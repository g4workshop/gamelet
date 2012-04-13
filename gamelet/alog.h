//
//  alog.h
//  just a simple log
//
//  Created by Dawen Rie on 12-4-5.
//  Copyright (c) 2012年 G4 Workshop. All rights reserved.
//

#ifndef gamelet_alog_h
#define gamelet_alog_h

#include <stdio.h>

typedef enum {
    ALOG_LEVEL_DEBUG = 20,		// < debug-level message
    ALOG_LEVEL_INFO = 40,	    // < informational message
    ALOG_LEVEL_ERROR = 100,		// < error conditions, maybe application fail
} alogLevel;

void alog(const char *file, long line, alogLevel level, const char *format, ...);
void alogbin(unsigned char *buf, size_t length);

class evbuffer;
void alogbuffer(evbuffer *evbuff);

#define ALOG_ERROR(format, args...) \
alog(__FILE__, __LINE__, ALOG_LEVEL_ERROR, format, ##args)

#define ALOG_INFO(format, args...) \
alog(__FILE__, __LINE__, ALOG_LEVEL_INFO, format, ##args)

#ifdef NON_ALOG_DEBUG
#define ALOG_DEBUG(format, args...)
#else
#define ALOG_DEBUG(format, args...) \
alog(__FILE__, __LINE__, ALOG_LEVEL_DEBUG, format, ##args)
#endif

#endif
