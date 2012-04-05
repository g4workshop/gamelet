//
//  server.cpp
//  the server module
//
//  Created by Dawen Rie on 12-4-5.
//  Copyright (c) 2012年 G4 Workshop. All rights reserved.
//

#include "server.h"

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

#include "alog.h"

static void listener_cb(struct evconnlistener *, evutil_socket_t, 
                        struct sockaddr *, int socklen, void *);
static void conn_writecb(struct bufferevent *, void *);
static void conn_eventcb(struct bufferevent *, short, void *);
static void signal_cb(evutil_socket_t, short, void *);

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
    
	struct evconnlistener *listener = evconnlistener_new_bind(base, listener_cb, (void *)base,
                                       LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1,
                                       (struct sockaddr*)&sin,
                                       sizeof(sin));
    
	if (!listener) {
		ALOG_ERROR("Could not create a listener!");
		return false;
	}
    
//	if (workqueue_init(&workqueue, NUM_THREADS)) {
//		perror("Failed to create work queue");
//		close(listenfd);
//		workqueue_shutdown(&workqueue);
//		return 1;
//	}
//    
    struct event *signal_event = evsignal_new(base, SIGINT, signal_cb, (void *)base);
    
	if (!signal_event || event_add(signal_event, NULL)<0) {
		ALOG_ERROR("Could not create/add a signal event!\n");
		return 1;
	}
    ALOG_INFO("Server running...");
	event_base_dispatch(base);
	evconnlistener_free(listener);
	event_free(signal_event);
    
	return true;
}

void Server::stop(){
	struct timeval delay = { 2, 0 };	
    event_base_loopexit(base, &delay);
}

Server::~Server(){
    if(base)
        event_base_free(base);
}

static void signal_cb(evutil_socket_t sig, short events, void *user_data){
	struct event_base *base = (struct event_base*)user_data;
	struct timeval delay = { 2, 0 };
    
	ALOG_INFO("Caught an interrupt signal; exiting cleanly in two seconds.");
	
    event_base_loopexit(base, &delay);
}