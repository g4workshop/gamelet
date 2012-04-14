//
//  player_manager.cpp
//  gamelet
//
//  Created by Dawen Rie on 12-4-9.
//  Copyright (c) 2012年 G4 Workshop. All rights reserved.
//

#include "player.h"
#include "alog.h"
#include "cmd.h"

#include <errno.h>
#include <string.h>

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/event.h>


static void conn_readcb(struct bufferevent *,  void *);
static void conn_writecb(struct bufferevent *, void *);
static void conn_eventcb(struct bufferevent *, short, void *);

Player::Player(){
    NPC = false;
    bev = NULL;
    commandBuffer = NULL;
    group = NULL;
}

bool Player::isNPC(){
    return NPC;
}

bool Player::attributeMatch(Player *other){
    if(other == NULL)
        return false;
    // @TODO
    return attributes == other->attributes;
}

void Player::handleCommand(){
    while (true) {
        Header header;
        // loop until all command handled
        if (!header.decode(commandBuffer)){
            ALOG_DEBUG("[%p] waiting command", this);
            alogbuffer(commandBuffer);
            return ;
        }
        
        size_t removedSize = evbuffer_get_length(commandBuffer);
        if (header.indicator == SID_SERVER){
            // skip the length filed
            evbuffer_drain(commandBuffer, 2);
            Command cmd;
            cmd.parse(commandBuffer);
            ALOG_INFO("[%p] command(%d)", this, (int)cmd._packetId); 
            switch (cmd._packetId) {
                case G4_COMM_PLAYER_LOGIN:
                    login(cmd);
                    break;
                case G4_COMM_PLAYER_LOGOUT:
                    logout(cmd);
                    break;
                case G4_COMM_MATCH_REQUEST:
                    match(cmd);
                    break;
                case G4_COMM_PLAYER_LEAVE:
                    leaveMatch(cmd);
                    break;
                default:
                    break;
            }
        }
        else {
            switch (header.indicator) {
                case SID_PLAYER:
                    forwardToPlayer(header.targetPlayer, header.length);
                    break;
                case SID_GROUP:
                    forwardToGroup(header.length);
                    break;
                default:
                    ALOG_INFO("not handle %d", (int)header.indicator);
                    break;
            }
        }
        removedSize -= evbuffer_get_length(commandBuffer);
        size_t eatSize = header.length+2;
        if (eatSize > removedSize){
            eatSize -= removedSize;
            evbuffer_drain(commandBuffer, eatSize);
        }
    }
}

void Player::forwardToPlayer(std::string &userid, short length){
    ALOG_INFO("[%p] forward to %s", this, userid.c_str());
}

void Player::forwardToGroup(short length){
    if (group == NULL)
    {
        ALOG_ERROR("[%p] forward to null group", this);
        return ;
    }
    size_t sendLength = length+2;
    unsigned char *data = new unsigned char[sendLength];
    evbuffer_remove(commandBuffer, data, sendLength);
    for (auto it = group->players.begin(); it != group->players.end(); ++it) {
        if( *it == this)
            continue;
        bufferevent_write((*it)->bev, data, sendLength);
        ALOG_INFO("[%p] froward (%d)data to group player[%p]", 
                  this, sendLength, *it);
        alogbin(data, sendLength);
    }
    delete data;
}

void Player::login(Command &cmd){
    G4TLV *tlv = NULL;
    std::string userid;
    std::string passwd;
    
    if ( (tlv = cmd.find(G4_KEY_PLAYER_ID)) )
        tlv->gets(userid);
    if ( (tlv = cmd.find(G4_KEY_PASSWORD)) )
        tlv->gets(passwd);
    
    G4OutStream stream;
    Command resp(G4_COMM_PLAYER_LOGIN);
    if (PlayerManager::instance().login(this, userid, passwd)){
        resp._result = 0;
        ALOG_INFO("[%p] login response", this);
    }
    else {
        resp._result = 1;
        ALOG_INFO("[%p] login failed", this);
    }
    resp.encode(&stream);
    bufferevent_write(bev, stream.buffer(), stream.size());
}

void Player::logout(Command &){
    PlayerManager::instance().logout(this);
}

void Player::match(Command &cmd){
    G4TLV *tlv = NULL;
    unsigned int min = 2, max = 2;
    if ( (tlv = cmd.find(G4_KEY_MIN_PLAYER)) )
        tlv->gets(userid);
    if ( (tlv = cmd.find(G4_KEY_MAX_PLAYER)) )
        tlv->gets(passwd);
    
    Group *group = PlayerManager::instance().matchGroup(this, min, max);
    if (!group) {
        ALOG_INFO("[%p] can't match a game", this);
        // response failed?
        return ;
    }
    else {
        ALOG_INFO("[%p] match a game group[%p]", this, group);
        G4OutStream stream;
        Command pjevent(G4_COMM_PLAYER_MATCHED);
        pjevent._result = 0;
        std::vector<std::string> playerid;
        for (auto it = group->players.begin(); it != group->players.end(); ++it) {
            playerid.push_back((*it)->userid);
        }
        pjevent.encode(&stream);
        
        for (auto it = group->players.begin(); it != group->players.end(); ++it) {
            ALOG_INFO("[%p] send matched notify", (*it));
            bufferevent_write((*it)->bev, stream.buffer(), stream.size());
        }

        if (group->isEnoughToPlay()){
            // just modify the exsting message
            G4OutStream stream;
            pjevent._packetId = G4_COMM_MATCH_CREATED;
            pjevent.encode(&stream);
            for (auto it = group->players.begin(); it != group->players.end(); ++it) {
                ALOG_INFO("[%p] send created notify", (*it));
                bufferevent_write((*it)->bev, stream.buffer(), stream.size());
            }
        }
    }
}

void Player::leaveMatch(Command &){
    ALOG_INFO("[%p] leave a game group[%]", this, group);
    PlayerManager::instance().leaveGroup(this);
}

Group::Group(){
}

bool Group::add(Player *player){
    if (isFull()){
        // try to replace the NPC
        for (auto it = players.begin(); it != players.end(); ++it) {
            if ((*it)->isNPC()){
                players.erase(it);
                players.push_back(player);
                return true;
            }
        }
        // full and no NPC
        return false;
    }
    else{
        players.push_back(player);
        return true;
    }
    return false;
}

bool Group::remove(Player *player){
    players.remove(player);
    return true;
}

bool Group::isEmpty(){
    return players.empty();
}

bool Group::isEnoughToPlay(){
    return players.size() >= minimum;
}

bool Group::isEnoughPlayer(){
    size_t count = 0;
    for (auto it = players.begin(); it != players.end(); ++it) {
        if (!(*it)->isNPC()) {
            count ++;
        }
    }
    return count >= minimum;
}

bool Group::isFull(){
    return players.size() >= maxima;
}

PlayerManager::PlayerManager(){
}

PlayerManager::~PlayerManager(){
}

PlayerManager &PlayerManager::instance(){
    static PlayerManager s_instance;
    return s_instance;
}

Player *PlayerManager::newPlayer(struct event_base *base, evutil_socket_t fd) {
    Player*player = new Player;
    player->bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    player->commandBuffer = evbuffer_new();
    if (player->bev && player->commandBuffer) {
        bufferevent_setcb(player->bev,
                          conn_readcb,
                          conn_writecb,
                          conn_eventcb,
                          player);

        //bufferevent_enable(player->bev, EV_WRITE);
        bufferevent_enable(player->bev, EV_READ);
        ALOG_INFO("[%p] new player", player);
    }
    else {
        deletePlayer(player);
        player = NULL;
    }
    return player;
}

void PlayerManager::deletePlayer(Player *player){
    if(player) {
        logout(player);
        if(player->bev)
            bufferevent_free(player->bev);
        if(player->commandBuffer)
            evbuffer_free(player->commandBuffer);
        ALOG_INFO("[%p] delete player", player);
        delete player;
    }
}

Group *PlayerManager::newGroup(int min, int max){
    Group *group = new Group();
    groups.insert(group);
    group->minimum = min;
    group->maxima = max; 
    ALOG_INFO("[%p] group created(%d, %d)", 
              group, 
              group->minimum,
              group->maxima);
    return group;
}

void PlayerManager::deleteGroup(Group *group){
    if (group != NULL){
        groups.erase(group);
        delete group;
    }
}

bool PlayerManager::login(Player *player, 
                          std::string &userid, std::string &passwd){
    if (player != NULL){
        player->userid = userid;
        player->passwd = passwd;

        player->loginTime = time(NULL);
        players.insert(player);
        ALOG_INFO("[%p] player login, userid(%s) password(%s)",
                  player,
                  player->userid.c_str(),
                  player->passwd.c_str());
    }

    return true;
}

bool PlayerManager::logout(Player *player){
    if (player != NULL){
        ALOG_INFO("[%p] player logout", player);
        leaveGroup(player);
        players.erase(player);
    }
    return true;
}

Group *PlayerManager::matchGroup(Player* player, unsigned int min, unsigned int max){
    Group *group = NULL;
    for (auto it = groups.begin(); it != groups.end(); ++it){
        if ( !(*it)->isEnoughPlayer()){
            group = *it;
            break;
        }
    }
    if (group == NULL){
        group = newGroup(min, max);
    }
    group->add(player);
    player->group = group;
    return group;
}

bool PlayerManager::leaveGroup(Player *player){
    if(player->group != NULL){
        player->group->remove(player);
        if (player->group->isEmpty()){
            deleteGroup(player->group);
        }
        player->group = NULL;
    }
    return true;
}

// Called by libevent when there is data to read.
static void conn_readcb(struct bufferevent *bev, void *user_data){
	Player *player = (Player *)user_data;

    evbuffer_add_buffer(player->commandBuffer, bufferevent_get_input(bev));
    player->handleCommand();
}

// Called by libevent when there is data to write.
static void conn_writecb(struct bufferevent *bev, void *user_data) {
}

static void conn_eventcb(struct bufferevent *bev, short events, void *user_data)
{
    if (events & BEV_EVENT_ERROR) {
		ALOG_ERROR("Got an error on the connection: %s.", 
                   evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
	}

	// None of the other events can happen here,
    // since we haven't enabled timeouts
	Player *player = (Player *)user_data;
    PlayerManager::instance().deletePlayer(player);
}
