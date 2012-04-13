//
//  cmd.cpp
//  gamelet
//
//  Created by Dawen Rie on 12-4-12.
//  Copyright (c) 2012年 G4 Workshop. All rights reserved.
//

#include "cmd.h"
#include "alog.h"

#include <event2/buffer.h>

short Command::get16(unsigned char *buf){
    return (buf[0] | buf[1] << 8);
}

int Command::get32(unsigned char *buf){
    return (buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24);
}

void Command::set16(evbuffer *evbuff, short val){
    unsigned char buf[2];
    buf[0] = val & 0xFF;
    buf[1] = (val & 0xFF00)>>8;
    evbuffer_add(evbuff, buf, 2);
}

void Command::set32(evbuffer *evbuff, int val){
    unsigned char buf[4];
    buf[0] = val & 0xFF;
    buf[1] = (val & 0xFF00)>>8;
    buf[2] = (val & 0xFF0000)>>16;
    buf[3] = (val & 0xFF000000)>>24;
    evbuffer_add(evbuff, buf, 4);
}

bool Command::isCommandComplete(evbuffer *evbuf){
    size_t buflen = evbuffer_get_length(evbuf);
    if (buflen < 2)
        return false;

    unsigned char lenbuf[2];
    evbuffer_copyout(evbuf, lenbuf, 2);
    short len = get16(lenbuf);
    if (buflen-2 < (size_t)len)
        return false;
    return true;
}

void Command::decodeHeader(evbuffer *evbuf){
    unsigned char buf[7];
    evbuffer_copyout(evbuf, buf, 5);
    length = get16(buf);
    indicator = buf[2];
    short strlen = get16(buf+3);
    if(strlen != 0){
        unsigned char *strbuf = new unsigned char[strlen];
        evbuffer_copyout(evbuf, strbuf, strlen);
        targetPlayer.assign(strbuf, strbuf+strlen);
        delete strbuf;
    }
}

bool Command::parse(evbuffer *evbuf){
    unsigned char lenbuf[2];
    if (evbuffer_remove(evbuf, lenbuf, 2) != 2)
        return false;
    length = get16(lenbuf);

    if (evbuffer_remove(evbuf, &indicator, 1) != 1)
        return false;

    if (!parsePlayer(evbuf))
        return false;
    if (!parseMsgid(evbuf))
        return false;
    if (evbuffer_remove(evbuf, &result, 1) != 1)
        return false;
    return true;
}

bool Command::parsePlayer(evbuffer *evbuf){
    return parseString(evbuf, targetPlayer);
}

bool Command::parseMsgid(evbuffer *evbuf){
    unsigned char msgidbuf[2];
    if (evbuffer_remove(evbuf, msgidbuf, 2) != 2)
        return false;
    msgid = get16(msgidbuf);
    return true;
}

bool Command::parseString(evbuffer *evbuf, std::string &str){
    unsigned char lenbuf[2];
    if (evbuffer_remove(evbuf, lenbuf, 2) != 2)
        return false;
    short strlen = get16(lenbuf);
    if(strlen == 0)
        return true;
    unsigned char *strbuf = new unsigned char[strlen];
    if (evbuffer_remove(evbuf, strbuf, strlen) != strlen)
        return false;
    str.assign(strbuf, strbuf+strlen);
    delete strbuf;
    return true;
}

bool Command::parseObjString(evbuffer *evbuf, std::string &str){
    unsigned char objtype;
    if (evbuffer_remove(evbuf, &objtype, 1) != 1)
        return false;
    if (objtype != OBJ_STRING)
        return false;
    return parseString(evbuf, str);
}

bool Command::parseObjNumber(evbuffer *evbuf, int &num){
    unsigned char objtype;
    if (evbuffer_remove(evbuf, &objtype, 1) != 1)
        return false;
    if (objtype != OBJ_NUMBER)
        return false;
    if (evbuffer_remove(evbuf, &objtype, 1) != 1)
        return false;
    if (objtype == NUM_C){
        if (evbuffer_remove(evbuf, &objtype, 1) != 1)
            return false;
        num = objtype;
        return true;
    }
    else if(objtype == NUM_I){
        unsigned char numbuf[4];
        if (evbuffer_remove(evbuf, numbuf, 4) != 4)
            return false;

        num = get32(numbuf);
    }
    return false;
}

void Command::encodeString(evbuffer *evbuff, std::string &str){
    set16(evbuff, str.length()+1);
    evbuffer_add(evbuff, str.c_str(), str.length());
    unsigned char end = '\0';
    evbuffer_add(evbuff, &end, 1);
}

void Command::encodeObjString(evbuffer *evbuff, std::string &str){
    unsigned char objtype = OBJ_STRING;
    evbuffer_add(evbuff, &objtype, 1);
    encodeString(evbuff, str);
}

void Command::encodeObjNumber(evbuffer *evbuff, char num){

}

void Command::encodeObjNumber(evbuffer *evbuff, int num){

}

evbuffer *Command::encode(){
    evbuffer *evbuff = evbuffer_new();
    unsigned char indicator = 0;
    evbuffer_add(evbuff, &indicator, 1);
    encodeString(evbuff, targetPlayer);
    set16(evbuff, msgid);
    evbuffer_add(evbuff, &result, 1);
    return evbuff;
}

void Command::finalize(evbuffer *evfinal, evbuffer *evbuff){
    size_t length = evbuffer_get_length(evbuff);
    set16(evfinal, (short)length);
    evbuffer_add_buffer(evfinal, evbuff);
    evbuffer_free(evbuff);
}

bool LoginCommand::parse(evbuffer *evbuff){
    unsigned char keycount;
    if (evbuffer_remove(evbuff, &keycount, 1) != 1)
        return false;

    // skip ObjectType and 标志
    evbuffer_drain(evbuff, 3);
    parseObjString(evbuff, userid);
    evbuffer_drain(evbuff, 3);
    parseObjString(evbuff, passwd);
    evbuffer_drain(evbuff, 3);
    parseObjNumber(evbuff, mode);
    return true;
}

void LoginResponse::encode(evbuffer *evbuff){
    evbuffer *tmp = Command::encode();
    // @ TODO encode username
    unsigned char count = 0;
    evbuffer_add(tmp, &count, 1);
    //
    finalize(evbuff, tmp);
}

bool MatchCommand::parse(evbuffer *evbuff){
    unsigned char keycount;
    if (evbuffer_remove(evbuff, &keycount, 1) != 1)
        return false;
    evbuffer_drain(evbuff, 3);
    parseObjNumber(evbuff, minimum);
    evbuffer_drain(evbuff, 3);
    parseObjNumber(evbuff, maxima);
    return true;
}

void MatchResponse::encode(evbuffer *evbuff){
    evbuffer *tmp = Command::encode();
    // @ TODO encode username
    unsigned char count = 0;
    evbuffer_add(tmp, &count, 1);
    //
    finalize(evbuff, tmp);
}

void PlayerJoinEvent::encode(evbuffer *evbuff){
    evbuffer *tmp = Command::encode();
    unsigned char arraycount = 1;
    evbuffer_add(tmp, &arraycount, 1);

    // netpair
    unsigned char dummy[] = {0x04, 0x01, 0x00, 0x03};
    evbuffer_add(tmp, dummy, 4);

    unsigned char count = (unsigned char)playerid.size();
    evbuffer_add(tmp, &count, 1);
    for (auto it = playerid.begin(); it != playerid.end(); ++it){
        encodeObjString(tmp, *it);
    }

    finalize(evbuff, tmp);
}
