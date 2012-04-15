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
#include <netinet/tcp.h>

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
    
    static char address[64];
    sprintf(address, "@%p", this);
    description = address;
}

void Player::setPlayerId(std::string &playerid){
//    userid = playerid;
    this->playerid = playerid;
    
    static char address[64];
    sprintf(address, "@%p", this);
    description = playerid;
    description += address;
}
const char* Player::desc(){
    return description.c_str();
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
            ALOG_DEBUG("[%s] waiting command", desc());
            alog_buffer(commandBuffer);
            return ;
        }
        
        size_t removedSize = evbuffer_get_length(commandBuffer);
        if (header.indicator == SID_SERVER){
            // skip the length filed
            evbuffer_drain(commandBuffer, 2);
            Command cmd;
            cmd.parse(commandBuffer);
            ALOG_INFO("[%s] command(%d)", desc(), (int)cmd._packetId); 
            switch (cmd._packetId) {
                case G4_COMM_NPC_LOGIN:
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
                    ALOG_INFO("[%s] not handle %d", desc(), (int)header.indicator);
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
    if (group == NULL){
        ALOG_ERROR("[%s] forward to null group", desc());
        return ;
    }
    
    size_t sendLength = length+2;
    for (auto it = group->players.begin(); it != group->players.end(); ++it) {
        if( userid == (*it)->getPlayerId()){
            unsigned char *data = new unsigned char[sendLength];
            evbuffer_remove(commandBuffer, data, sendLength);
            bufferevent_write((*it)->bev, data, sendLength);
            delete data;
            ALOG_INFO("[%s] froward (%d) bytes data to player[%s] in group", 
                      desc(), sendLength, (*it)->desc());
            break;
        }
    }
}

void Player::forwardToGroup(short length){
    if (group == NULL)
    {
        ALOG_ERROR("[%s] forward to null group", desc());
        return ;
    }
    size_t sendLength = length+2;
    unsigned char *data = new unsigned char[sendLength];
    evbuffer_remove(commandBuffer, data, sendLength);
    for (auto it = group->players.begin(); it != group->players.end(); ++it) {
        if( *it == this)
            continue;
        bufferevent_write((*it)->bev, data, sendLength);
        ALOG_INFO("[%s] froward (%d) bytes data to group player[%s]", 
                  desc(), sendLength, (*it)->desc());
    }
    delete data;
}

void Player::login(Command &cmd){
    std::string userid;
    std::string passwd;
    
    cmd.gets(G4_KEY_PLAYER_ID, userid);
    cmd.gets(G4_KEY_PASSWORD, passwd);
    
    G4OutStream stream;
    Command resp(cmd._packetId);
    if (cmd._packetId == G4_COMM_NPC_LOGIN)
        NPC = true;
    else 
        NPC = false;
    
    if (PlayerManager::instance().login(this, userid, passwd)){
        resp._result = 0;
        ALOG_INFO("[%s] %slogin response", desc(), NPC ? "NPC ":"");
    }
    else {
        resp._result = 1;
        ALOG_INFO("[%s] %slogin failed", desc(), NPC ? "NPC ":"");
    }
    resp.encode(&stream);
    bufferevent_write(bev, stream.buffer(), stream.size());
}

void Player::logout(Command &){
    PlayerManager::instance().logout(this);
}

void Player::match(Command &cmd){
    unsigned int min = 2, max = 2;
    cmd.get32(G4_KEY_MIN_PLAYER, min);
    cmd.get32(G4_KEY_MAX_PLAYER, max);
    
    Group *group = PlayerManager::instance().matchGroup(this, min, max);
    if (!group) {
        ALOG_INFO("[%s] can't match a game", desc());
        G4OutStream stream;
        Command resp(cmd._packetId);
        resp._result = 1;
        resp.encode(&stream);
        bufferevent_write(bev, stream.buffer(), stream.size());
        return ;
    }
    else {
        ALOG_INFO("[%s] match a game group[%s]", desc(), group->desc());
        group->notifyPlayerMatched();

        if (group->isEnoughToPlay()){
            ALOG_INFO("[%s] game[%s] start", desc(), group->desc());
            group->startGame();
        }
    }
}

void Player::leaveMatch(Command &){
    PlayerManager::instance().leaveGroup(this);
}

Group::Group(){
    static char buf[64];
    sprintf(buf, "%p", this);
    description = buf;
}

const char *Group::desc(){
    return description.c_str();
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

void Group::notify(unsigned short cmd){
    G4OutStream stream;
    Command pjevent;
    pjevent._packetId = cmd;
    pjevent._result = 0;
    std::vector<std::string> playerid;
    for (auto it = players.begin(); it != players.end(); ++it) {
        playerid.push_back((*it)->getPlayerId());
    }
    pjevent.putss(G4_KEY_PLAYER_ID, playerid);
    pjevent.encode(&stream);
    
    for (auto it = players.begin(); it != players.end(); ++it) {
        ALOG_INFO("[%s] send notify(%d)", (*it)->desc(), cmd);
        bufferevent_write((*it)->bev, stream.buffer(), stream.size());
    }
}

void Group::notifyPlayerMatched(){
    notify(G4_COMM_PLAYER_MATCHED);
}

void Group::startGame(){
    notify(G4_COMM_MATCH_CREATED);
}

void Group::stopGame(){
    notify(G4_COMM_MATCH_DISMISSED);
    for (auto it = players.begin(); it != players.end(); ++it) {
        (*it)->group = NULL;
    }
    players.clear();
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
        bufferevent_enable(player->bev, EV_WRITE | EV_READ);
        int one = 1;
        setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);
        ALOG_INFO("[%s] new player", player->desc());
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
        ALOG_INFO("[%s] delete player", player->desc());
        delete player;
    }
}

Group *PlayerManager::newGroup(int min, int max){
    Group *group = new Group();
    groups.insert(group);
    group->minimum = min;
    group->maxima = max; 
    ALOG_INFO("[%s] group created(%d, %d)", 
              group->desc(), 
              group->minimum,
              group->maxima);
    return group;
}

void PlayerManager::deleteGroup(Group *group){
    if (group != NULL){
        groups.erase(group);
        ALOG_INFO("[%s] group deleted", group->desc());
        delete group;
    }
}

bool PlayerManager::login(Player *player, 
                          std::string &userid, std::string &passwd){
    if (player != NULL){
        player->setPlayerId(userid);
        player->passwd = passwd;

        player->loginTime = time(NULL);
        players.insert(player);
        ALOG_INFO("[%s] player login, userid(%s) password(%s)",
                  player->desc(),
                  player->getPlayerId(),
                  player->passwd.c_str());
    }

    return true;
}

bool PlayerManager::logout(Player *player){
    if (player != NULL){
        ALOG_INFO("[%s] player logout", player->desc());
        leaveGroup(player);
        players.erase(player);
    }
    return true;
}

Group *PlayerManager::matchGroup(Player* player, unsigned int min, unsigned int max){
    leaveGroup(player);
    Group *group = NULL;
    for (auto it = groups.begin(); it != groups.end(); ++it){
        if (player->isNPC() && (*it)->isEnoughToPlay()){
            // NPC do not try to replase other NPC
            continue;
        }
        if ( !(*it)->isEnoughToPlay()){
            group = *it;
            break;
        }
    }
    // NPC can't create game group
    if (group == NULL && player->isNPC())
        return NULL;
    
    if (group == NULL){
        group = newGroup(min, max);
    }
    group->add(player);
    player->group = group;
    return group;
}

bool PlayerManager::leaveGroup(Player *player){
    if(player->group != NULL){
        ALOG_INFO("[%s] leave group[%s]", player->desc(), player->group->desc());
        player->group->remove(player);
        if (!player->group->isEnoughToPlay()){
            ALOG_INFO("[%s] game[%s] stop", player->desc(), player->group->desc());
            player->group->stopGame();
        }
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
