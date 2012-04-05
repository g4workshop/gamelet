//
//  server.h
//  the server module
//
//  Created by Dawen Rie on 12-4-5.
//  Copyright (c) 2012年 G4 Workshop. All rights reserved.
//

#ifndef gamelet_server_h
#define gamelet_server_h

#include "cfgdata.h"

struct event_base;

class Server
{
public:
    static Server &instance();
    // run the server, this function will block until the server stop running
    bool run(StartupConfigure& cfg);
    void stop();
private:
    Server(){};
    ~Server();
    
private:
    struct event_base *base;
};

#endif
