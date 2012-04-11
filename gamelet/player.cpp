//
//  player_manager.cpp
//  gamelet
//
//  Created by Dawen Rie on 12-4-9.
//  Copyright (c) 2012年 G4 Workshop. All rights reserved.
//

#include "player.h"
#include "alog.h"

#include <errno.h>

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
    int count = 0;
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
        if(player->bev)
            bufferevent_free(player->bev);
        if(player->commandBuffer)
            evbuffer_free(player->commandBuffer);
        ALOG_INFO("[%p] delete player", player);
        delete player;
    }
}

bool PlayerManager::login(Player *player){
    if (player != NULL){
        player->loginTime = time(NULL);
        players.insert(player);
        ALOG_INFO("[%p] player login, userid(%s) gameid(%s)", 
                  player, 
                  player->userid.c_str(),
                  player->gameid.c_str());
    }
    
    return true;
}

bool PlayerManager::logout(Player *player){
    if (player != NULL){
        ALOG_INFO("[%p] player login", player);
        leaveGroup(player);
        players.erase(player);
    }
    return true;
}

Group *PlayerManager::matchGroup(Player* player){
    return NULL;
}

bool PlayerManager::leaveGroup(Player *player){
    if(player->group != NULL){
        player->group->remove(player);
        if (player->group->isEmpty())
            groups.erase(player->group);
    }
    return true;
}

// Called by libevent when there is data to read.
static void conn_readcb(struct bufferevent *bev, void *user_data){
	Player *player = (Player *)user_data;
    
    evbuffer_add_buffer(player->commandBuffer, bufferevent_get_input(bev));
    if (bufferevent_write_buffer(bev, player->commandBuffer)) {
        ALOG_ERROR("Error sending data to player!");
	}
}

// Called by libevent when there is data to write.
static void conn_writecb(struct bufferevent *bev, void *user_data) {
}

static void conn_eventcb(struct bufferevent *bev, short events, void *user_data)
{
	if (events & BEV_EVENT_EOF) {
		ALOG_INFO("Connection closed.");
	} else if (events & BEV_EVENT_ERROR) {
		ALOG_ERROR("Got an error on the connection: %s.", 
                  evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
	}
    
	// None of the other events can happen here, 
    // since we haven't enabled timeouts
	Player *player = (Player *)user_data;
    PlayerManager::instance().deletePlayer(player);
}
