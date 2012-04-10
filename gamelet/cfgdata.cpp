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
}

bool StartupConfigure::loadFromFile(const char *filepath){
    // @TODO
    return true;
}
