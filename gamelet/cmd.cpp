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

EvBufferStream::EvBufferStream(evbuffer *evbuff){
    this->evbuff = evbuff;
}

bool EvBufferStream::getbytes(unsigned char* buffer, unsigned short size){
    if (evbuffer_get_length(evbuff) < (size_t)size)
        return false;
    evbuffer_remove(evbuff, buffer, (size_t)size);
    return true;
}

bool EvBufferStream::skip(unsigned short len){
    if (evbuffer_get_length(evbuff) < (size_t)len)
        return false;
    evbuffer_drain(evbuff, (size_t)len);
    return true;
}

// just decode from buffer, not drain it
bool Header::decode(evbuffer *evbuf){
    size_t buflen = evbuffer_get_length(evbuf);
    if (buflen < 4)
        return false;
    
    unsigned char buf[4];
    evbuffer_copyout(evbuf, &buf, 4);
    length = ((buf[0] | buf[1] << 8));
    
    if (buflen-2 < length)
        return false;
    
    indicator = buf[2];
    size_t strLen = buf[3];
    if (strLen != 0){
        unsigned char strbuf[260] = {0};
        evbuffer_copyout(evbuf, strbuf, strLen+4);
        targetPlayer.assign(strbuf+4, strbuf+strLen+4);
    }
    return true;
}

bool Command::parse(evbuffer *evbuff){
    EvBufferStream evstream(evbuff);
    G4InStream stream(evstream);
    
    if (!decode(&stream))
        return false;
    return true;
}
