#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <string.h>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define LOG_INFO(msg) __MKlog(LT_INFO, msg, __FILENAME__, __LINE__)
#define LOG_WARNING(msg) __MKlog(LT_WARNING, msg, __FILENAME__, __LINE__)
#define LOG_ERROR(msg) __MKlog(LT_ERROR, msg, __FILENAME__, __LINE__)
#define LOG_FATAL(msg) __MKlog(LT_FATAL, msg, __FILENAME__, __LINE__)

typedef enum {
    LT_INFO=0,
    LT_WARNING=1,
    LT_ERROR=2,
    LT_FATAL=3,
} LogType;

void __MKlog(LogType type, const char* msg, const char* file, unsigned int line);

#endif //LOGGING_H
