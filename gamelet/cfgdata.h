//
//  cfgdata.h
//  gamelet
//
//  Created by Dawen Rie on 12-4-5.
//  Copyright (c) 2012年 G4 Workshop. All rights reserved.
//

#ifndef gamelet_cfgdata_h
#define gamelet_cfgdata_h

#include <string>

class StartupConfigure
{
public:
    StartupConfigure();
    bool loadFromFile(const char* filepath);

    std::string serviceAddress;
    int servicePort;
    int blockCount;
    int numOfThreads;
};

#endif
