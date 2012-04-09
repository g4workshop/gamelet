//
//  client_manager.h
//  gamelet
//
//  Created by Dawen Rie on 12-4-9.
//  Copyright (c) 2012年 G4 Workshop. All rights reserved.
//

#ifndef gamelet_client_manager_h
#define gamelet_client_manager_h

#include <event2/util.h>

#include <set>

class bufferevent;
class evbuffer;

// client connection data
struct Client{
   	// The bufferedevent for this client.
	struct bufferevent *bev;
	// The output buffer for this client.
	struct evbuffer *echoBuffer;
    
    // Other service specail information...
};


class ClientManager{
public:
    static ClientManager &instance();
    Client *newClient(struct event_base *base, evutil_socket_t fd);
    void delClient(Client *client);
    
protected:
    ClientManager();
    ~ClientManager();
    
private:

};

#endif
