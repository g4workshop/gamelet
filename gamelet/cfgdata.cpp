//
//  cfgdata.cpp
//  gamelet
//
//  Created by Dawen Rie on 12-4-5.
//  Copyright (c) 2012年 G4 Workshop. All rights reserved.
//

#include "cfgdata.h"

StartupConfigure::StartupConfigure(){
    // @TODO read it from file
    servicePort = 9090;
    blockCount = 9;
    // normally the worker count is coresponse to the CPU(kernel) count, read it from the system
    numOfThreads = 4;
}

bool StartupConfigure::loadFromFile(const char *filepath){
    // @TODO
    return true;
}
