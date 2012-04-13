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

/*
struct LoginCommand : public Command{
    bool parsetlv();
    std::string userid;
    std::string passwd;
    unsigned char mode;
    std::string gameid;
};

struct LoginResponse : public Command{
    LoginResponse() { _packetId = G4_COMM_PLAYER_LOGIN; };
    void packet(G4OutStream &stream);
};

struct LogoutCommand : public Command{
};

struct MatchCommand : public Command{
    bool parsetlv();
    unsigned int minimum;
    unsigned int maxima;
};

struct LeaveMatchCommand : public Command {
    bool parse(evbuffer *evbuff);
};

struct PlayerJoinEvent : public Command{
    PlayerJoinEvent() { _packetId = G4_COMM_MATCH_CREATED; };
    void packet(G4OutStream &stream);
    std::vector<std::string> playerid;
};
 */
#endif
