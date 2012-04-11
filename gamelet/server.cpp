//
//  server.cpp
//  the server module
//
//  Created by Dawen Rie on 12-4-5.
//  Copyright (c) 2012年 G4 Workshop. All rights reserved.
//

#include "server.h"
#include "player.h"
#include "alog.h"

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>

#ifndef WIN32
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#endif

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>

static void listener_cb(struct evconnlistener *, evutil_socket_t, 
                        struct sockaddr *, int socklen, void *);
static void listener_errorcb(struct evconnlistener *listener, void *);
static void signal_cb(evutil_socket_t, short, void *);

Server::Server(){
}

Server::~Server(){
    if(base)
        event_base_free(base);
}

Server& Server::instance(){
    static Server s_server;
    return s_server;
}

bool Server::run(StartupConfigure &cfg){
	base = event_base_new();
	if (!base) {
        ALOG_ERROR("Could not initialize libevent!");
		return false;
	}
    
    struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(cfg.servicePort);
    
	struct evconnlistener *listener = evconnlistener_new_bind(base, 
                                        listener_cb, 
                                        (void *)base,
                                        LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, 
                                        -1,
                                        (struct sockaddr*)&sin,
                                        sizeof(sin));
    
	if (!listener){
		ALOG_ERROR("Could not create a listener!");
		return false;
	}
    evconnlistener_set_error_cb(listener, listener_errorcb);
    // the ctrl+c break event
    struct event *signal_event = evsignal_new(base, 
                                              SIGINT, 
                                              signal_cb, 
                                              (void *)base);
    
	if (!signal_event || event_add(signal_event, NULL)<0) {
		ALOG_ERROR("Could not create/add SIGINT event!");
		return false;
	}
    
    ALOG_INFO("Server running...");
	event_base_dispatch(base);
    ALOG_INFO("Server stoped.");
    
	evconnlistener_free(listener);
	event_free(signal_event);
    
	return true;
}

static void listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
            struct sockaddr *sa, int socklen, void *user_data)
{
	struct event_base *base = (struct event_base *)user_data;
    Player *client = PlayerManager::instance().newPlayer(base, fd);
	if (!client) {
        ALOG_ERROR("Error constructing bufferevent/evbuffer!");
		event_base_loopbreak(base);
		return;
	}
}

static void listener_errorcb(struct evconnlistener *listener, void *user_data){
    ALOG_ERROR("Got an error on the connection: %s", 
               evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
}

static void signal_cb(evutil_socket_t sig, short events, void *user_data){
    ALOG_INFO("Caught an interrupt signal; exiting cleanly.");
    struct event_base *base = (struct event_base *)user_data;
	struct timeval delay = { 2, 0 };  
	event_base_loopexit(base, &delay);
}
