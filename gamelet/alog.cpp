//
//  alog.cpp
//  full function of alog
//
//  Created by Dawen Rie on 12-4-5.
//  Copyright (c) 2012年 G4 Workshop. All rights reserved.
//
#include "alog.h"

#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>

static pthread_mutex_t alog_mutex = PTHREAD_MUTEX_INITIALIZER;

void alog(const char *file, long line, alogLevel level, const char *format, ...){
    pthread_mutex_lock(&alog_mutex);
    
    if(level == ALOG_LEVEL_ERROR)
        fprintf(stdout, "ERROR:%s(%ld):", file, line);
    
    va_list va;
    va_start(va, format);
    vfprintf(stdout, format, va);
    va_end(va);
    fprintf(stdout, "\n");
    fflush(stdout);
    pthread_mutex_unlock(&alog_mutex);
}
