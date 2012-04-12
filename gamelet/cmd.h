//
//  cmd.h
//  gamelet
//
//  Created by Dawen Rie on 12-4-12.
//  Copyright (c) 2012年 G4 Workshop. All rights reserved.
//

#ifndef gamelet_cmd_h
#define gamelet_cmd_h

enum PlayerCommandID{
    CMD_LOGIN           = 1,
    CMD_NPC_LOGIN       = 2,
    CMD_LOGOUT          = 3,
    CMD_MATCH           = 4,
    CMD_LEAVE_MATCH     = 5,
    EVT_PLAYER_JOIN     = 6,
    EVT_PLAYER_LEAVE    = 7,
};

#endif
