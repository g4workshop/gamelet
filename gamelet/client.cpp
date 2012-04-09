//
//  client_manager.cpp
//  gamelet
//
//  Created by Dawen Rie on 12-4-9.
//  Copyright (c) 2012年 G4 Workshop. All rights reserved.
//

#include "client.h"
#include "alog.h"

#include <errno.h>

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/event.h>


static void conn_readcb(struct bufferevent *,  void *);
static void conn_writecb(struct bufferevent *, void *);
static void conn_eventcb(struct bufferevent *, short, void *);

ClientManager::ClientManager(){
}

ClientManager::~ClientManager(){
}

ClientManager &ClientManager::instance(){
    static ClientManager s_instance;
    return s_instance;
}

Client *ClientManager::newClient(struct event_base *base, evutil_socket_t fd) {
    Client*client = new Client;
    client->bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    client->echoBuffer = evbuffer_new();
    if (client->bev && client->echoBuffer) {
        bufferevent_setcb(client->bev, 
                          conn_readcb, 
                          conn_writecb, 
                          conn_eventcb, 
                          client);
        
        //bufferevent_enable(client->bev, EV_WRITE);
        bufferevent_enable(client->bev, EV_READ);
    }
    else {
        delClient(client);
        client = NULL;
    }
    return client;
}

void ClientManager::delClient(Client *client){
    if(client) {
        if(client->bev)
            bufferevent_free(client->bev);
        if(client->echoBuffer)
            evbuffer_free(client->echoBuffer);
        delete client;
    }
}

// Called by libevent when there is data to read.
static void conn_readcb(struct bufferevent *bev, void *user_data){
	Client *client = (Client *)user_data;
    
    evbuffer_add_buffer(client->echoBuffer, bufferevent_get_input(bev));
    if (bufferevent_write_buffer(bev, client->echoBuffer)) {
        ALOG_ERROR("Error sending data to client!");
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
	Client *client = (Client *)user_data;
    ClientManager::instance().delClient(client);
    ALOG_INFO("[%p] Client freeze.", client);
}
