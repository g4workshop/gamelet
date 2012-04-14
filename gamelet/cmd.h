//
//  cmd.h
//  gamelet
//
//  Created by Dawen Rie on 12-4-12.
//  Copyright (c) 2012年 G4 Workshop. All rights reserved.
//

#ifndef gamelet_cmd_h
#define gamelet_cmd_h

#include <string>
#include <vector>

#include "packet.h"
#include "sysdefine.h"

class evbuffer;
enum ServiceIndicator {
    SID_SERVER = 1,
    SID_PLAYER = 2,
    SID_GROUP  = 3,
    SID_GAME   = 4,
    SID_ALL    = 5,
};

class EvBufferStream : public G4InStreamIF{
public:
    EvBufferStream(evbuffer *evbuff);
    virtual bool getbytes(unsigned char* buffer, unsigned short size);
    virtual bool skip(unsigned short offset);
private:
    evbuffer *evbuff;
};

struct Header{
    bool decode(evbuffer *evbuff);
    size_t length;
    unsigned char indicator;
    std::string targetPlayer;
};

class Command : public G4NextPacket{
public:
    Command(){};
    Command(unsigned short packetId) { _packetId = packetId; };
    bool parse(evbuffer *evbuff);    
};

#endif
