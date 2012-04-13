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
#include <list>

class evbuffer;

// indicator of server how to handle a client event
enum ServiceIndicator{
    SID_SERVER          = 1,    // server handle only
    SID_PLAYER          = 2,    // to splecial player
    SID_GROUP           = 3,    // to all player in the same game group
    SID_GAME            = 4,    // to all player in this game
    SID_ALL             = 5,    // to all login player
};

// player action command
enum PlayerCommandID{
    CMD_LOGIN           = 1,    // player login
    CMD_NPC_LOGIN       = 2,    // NPC login
    CMD_LOGOUT          = 3,    // player logou
    CMD_MATCH           = 4,    // try to match a game
    CMD_LEAVE_MATCH     = 5,    // leave the match game
    EVT_PLAYER_JOIN     = 6,    // casting event of one player joined
    EVT_PLAYER_LEAVE    = 7,    // casting event of one player leave
    EVT_GAME_STOP       = 8,    // it's enough player to open a game
    EVT_GAME_START      = 9,    // the started game became not enough player
};

enum ObjectType{
    OBJ_STRING = 1,
    OBJ_NUMBER = 2,
    OBJ_ARRAY  = 3,
};

enum NumberType{
    NUM_C = 'c',
    NUM_I = 'i',
};

struct Command{
    static short get16(unsigned char *buf);
    static int get32(unsigned char *buf);
    static void set16(evbuffer *evbuff, short val);
    static void set32(evbuffer *evbuff, int val);
    
    static bool isCommandComplete(evbuffer *evbuf);
    static ServiceIndicator serviceIndicator(evbuffer *evbuf);

    void decodeHeader(evbuffer *evbuf);
    // parse the command from buffer, and remove the parsed data fro evbuffer.
    bool parse(evbuffer *evbuf);
    bool parsePlayer(evbuffer *evbuf);
    bool parseMsgid(evbuffer *evbuf);
    
    //
    static bool parseString(evbuffer *evbuf, std::string &str);
    static bool parseObjString(evbuffer *evbuf, std::string &str);
    static bool parseObjNumber(evbuffer *evbuf, int &num);
    
    
    evbuffer* encode();
    void finalize(evbuffer *evfinal, evbuffer *tmp);
    
    void encodeString(evbuffer *evbuff, std::string &str);
    void encodeObjString(evbuffer *evbuff, std::string &str);
    void encodeObjNumber(evbuffer *evbuff, char num);
    void encodeObjNumber(evbuffer *evbuff, int num);
    
    short length;
    unsigned char indicator;
    std::string targetPlayer;
    short msgid;
    unsigned char result;
};

struct LoginCommand : public Command{
    bool parse(evbuffer *evbuff);
    
    std::string userid;
    std::string passwd;
    int mode;
    std::string gameid;
};

struct LoginResponse : public Command{
    LoginResponse(){ msgid = CMD_LOGIN; }
    void encode(evbuffer *evbuff);
    std::string username;
};

struct LogoutCommand : public Command{
    bool parse(evbuffer *evbuf){ return true; };
};

struct MatchCommand : public Command{
    bool parse(evbuffer *evbuff);
    
    int minimum;
    int maxima;
};

struct MatchResponse : public Command{
    MatchResponse(){ msgid = CMD_MATCH; }
    void encode(evbuffer *evbuff);
};

struct PlayerJoinEvent : public Command{
    PlayerJoinEvent() { msgid = EVT_PLAYER_JOIN; };
    void encode(evbuffer *evbuff);
    std::list<std::string> playerid;
};
#endif
