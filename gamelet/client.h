//
//  client_manager.h
//  client manager
//
//  Created by Dawen Rie on 12-4-9.
//  Copyright (c) 2012年 G4 Workshop. All rights reserved.
//

#ifndef gamelet_client_manager_h
#define gamelet_client_manager_h

#include <event2/util.h>

#include <string>
#include <map>

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

typedef std::map<std::string, Client*> ClientMap;

class ClientManager{
public:
    static ClientManager &instance();
    Client *newClient(struct event_base *base, evutil_socket_t fd);
    void delClient(Client *client);
    
private:
    ClientManager();
    ~ClientManager();
    
private:
    ClientMap clients;
};

#endif
