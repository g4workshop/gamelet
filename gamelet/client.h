//
//  client.h
//  client connection information
//
//  Created by Dawen Rie on 12-4-5.
//  Copyright (c) 2012年 G4 Workshop. All rights reserved.
//

#ifndef gamelet_client_h
#define gamelet_client_h

#include <event2/bufferevent.h>
#include <event2/buffer.h>

struct ClientInfo{
	// The client's socket.
	int fd;
	// The event_base for this client.
	struct event_base *evbase;
    
	// The bufferedevent for this client.
	struct bufferevent *buf_ev;
    
	// The output buffer for this client.
	struct evbuffer *output_buffer;
};

#endif
