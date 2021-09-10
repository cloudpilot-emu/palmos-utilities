#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <PalmOS.h>

#ifdef DEBUG
    #define LOG DebugLog
#else
    #define LOG(ARGS...)
#endif

#ifdef DEBUG
void DebugLog(const char* fmt, ...);
const char* EventName(eventsEnum event);
#endif

#endif  // _DEBUG_H_
