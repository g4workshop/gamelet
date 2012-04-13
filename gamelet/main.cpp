//
//  main.cpp
//  gamelet
//
//  Created by Dawen Rie on 12-4-5.
//  Copyright (c) 2012年 G4 Workshop. All rights reserved.
//


#include "cfgdata.h"
#include "alog.h"
#include "server.h"

#include "cmd.h"
#include <event2/buffer.h>

#define CFG_FILE "./gamelet.conf"

int main(int argc, const char * argv[])
{
    StartupConfigure cfg;
    if (!cfg.loadFromFile(CFG_FILE)) {
        ALOG_ERROR("failed to load configure from file:%s", CFG_FILE);
        return -1;
    }

    if (!Server::instance().run(cfg)){
        ALOG_ERROR("can't run the server");
    }
    ALOG_INFO("Server exited");
    return 0;
}

