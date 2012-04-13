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
<<<<<<< HEAD
    if (!Command::isCommandComplete(commandBuffer)){
        ALOG_DEBUG("[%p] command not complete", this);
        alogbuffer(commandBuffer);
        return ;
    }

    Command cmd;
    cmd.decodeHeader(commandBuffer);
    if (cmd.indicator == SID_SERVER){
        size_t removedSize = evbuffer_get_length(commandBuffer);
        if (!cmd.parse(commandBuffer)){
            ALOG_INFO("parse command error");
            return ;
        }
        ALOG_INFO("[%p] command(%d)", this, (int)cmd.msgid);
        switch (cmd.msgid) {
            case CMD_LOGIN:
                login();
                break;
            case CMD_LOGOUT:
                logout();
                break;
            case CMD_MATCH:
                match();
                break;
            case CMD_LEAVE_MATCH:
                leaveMatch();
                break;
            default:
                break;
=======
    while (true) {
        // loop until all command handled
        if (!Command::isCommandComplete(commandBuffer)){
            ALOG_DEBUG("[%p] waiting command", this);
            alogbuffer(commandBuffer);
            return ;
        }
        
        Command cmd;
        cmd.decodeHeader(commandBuffer);
        size_t removedSize = evbuffer_get_length(commandBuffer);
        
        if (cmd.indicator == SID_SERVER){
            if (!cmd.parse(commandBuffer)){
                ALOG_INFO("parse command error");
                return ;
            }
            ALOG_INFO("[%p] command(%d)", this, (int)cmd.msgid); 
            switch (cmd.msgid) {
                case CMD_LOGIN:
                    login();
                    break;
                case CMD_LOGOUT:
                    logout();
                    break;
                case CMD_MATCH:
                    match();
                    break;
                case CMD_LEAVE_MATCH:
                    leaveMatch();
                    break;
                default:
                    break;
            }
        }
        else {
            switch (cmd.indicator) {
                case SID_PLAYER:
                    forwardToPlayer(cmd.targetPlayer, cmd.length);
                    break;
                case SID_GROUP:
                    forwardToGroup(cmd.length);
                    break;
                default:
                    ALOG_INFO("not handle %d", cmd.indicator);
                    break;
            }
>>>>>>> loop the handleCommand until all command was handle
        }
        removedSize -= evbuffer_get_length(commandBuffer);
        size_t eatSize = cmd.length+2;
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

void Player::login(){
    LoginCommand loginCmd;
    loginCmd.parse(commandBuffer);
    if (PlayerManager::instance().login(this, loginCmd))
    {
        LoginResponse resp;
        resp.targetPlayer = loginCmd.targetPlayer;
        resp.result = 0;
        evbuffer *respevbuff = evbuffer_new();
        resp.encode(respevbuff);
        bufferevent_write_buffer(bev, respevbuff);
        evbuffer_free(respevbuff);
        ALOG_INFO("[%p] login response", this);
    }
}

void Player::logout(){
    LogoutCommand logouCmd;
    logouCmd.parse(commandBuffer);
    PlayerManager::instance().logout(this);
}

void Player::match(){
    MatchCommand matchCmd;
    matchCmd.parse(commandBuffer);
    Group *group = PlayerManager::instance().matchGroup(this, matchCmd);
    if (!group) {
        ALOG_INFO("[%p] can't match a game", this);
        // response failed?
        return ;
    }
    else {
        ALOG_INFO("[%p] match a game group[%p]", this, group);
        PlayerJoinEvent pjevent;
        pjevent.result = 0;
        for (auto it = group->players.begin(); it != group->players.end(); ++it) {
            pjevent.playerid.push_back((*it)->userid);
        }
        evbuffer *evtjoinbuff = evbuffer_new();
        pjevent.encode(evtjoinbuff);
        size_t length = evbuffer_get_length(evtjoinbuff);
        unsigned char *data = new unsigned char[length];
        evbuffer_copyout(evtjoinbuff, data, length);
        for (auto it = group->players.begin(); it != group->players.end(); ++it) {
            //alogbin(data, length);
            bufferevent_write((*it)->bev, data, length);
        }
        delete data;
        evbuffer_free(evtjoinbuff);

        if (group->isEnoughToPlay()){
            // just modify the exsting message
            pjevent.msgid = EVT_GAME_START;

            evbuffer *evtjoinbuff = evbuffer_new();
            pjevent.encode(evtjoinbuff);
            size_t length = evbuffer_get_length(evtjoinbuff);
            unsigned char *data = new unsigned char[length];
            evbuffer_copyout(evtjoinbuff, data, length);
            for (auto it = group->players.begin(); it != group->players.end(); ++it) {
                ALOG_INFO("[%p] send start notify", (*it));
                bufferevent_write((*it)->bev, data, length);
            }
            delete data;
            evbuffer_free(evtjoinbuff);
<<<<<<< HEAD

=======
            
>>>>>>> loop the handleCommand until all command was handle
        }
    }
}

void Player::leaveMatch(){
<<<<<<< HEAD

=======
    ALOG_INFO("[%p] leave a game group[%]", this, group);
    PlayerManager::instance().leaveGroup(this);
>>>>>>> loop the handleCommand until all command was handle
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
    return (int)players.size() >= minimum;
}

bool Group::isEnoughPlayer(){
    int count = 0;
    for (auto it = players.begin(); it != players.end(); ++it) {
        if (!(*it)->isNPC()) {
            count ++;
        }
    }
    return count >= minimum;
}

bool Group::isFull(){
    return (int)players.size() >= maxima;
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

bool PlayerManager::login(Player *player, LoginCommand &cmd){
    if (player != NULL){
        player->userid = cmd.userid;
        player->passwd = cmd.passwd;
        player->gameid = cmd.gameid;

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

Group *PlayerManager::matchGroup(Player* player, MatchCommand &cmd){
    Group *group = NULL;
    for (auto it = groups.begin(); it != groups.end(); ++it){
        if ( !(*it)->isEnoughPlayer()){
            group = *it;
            break;
        }
    }
    if (group == NULL){
<<<<<<< HEAD
        group = new Group();
        groups.insert(group);
        group->minimum = cmd.minimum;
        group->maxima = cmd.maxima;
        ALOG_INFO("[%p] group created(%d, %d)",
                  group,
                  group->minimum,
                  group->maxima);
=======
        group = newGroup(cmd.minimum, cmd.maxima);
>>>>>>> loop the handleCommand until all command was handle
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
<<<<<<< HEAD
		ALOG_ERROR("Got an error on the connection: %s.",
                  evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
=======
		ALOG_ERROR("Got an error on the connection: %s.", 
                   evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
>>>>>>> loop the handleCommand until all command was handle
	}

	// None of the other events can happen here,
    // since we haven't enabled timeouts
	Player *player = (Player *)user_data;
    PlayerManager::instance().deletePlayer(player);
}
