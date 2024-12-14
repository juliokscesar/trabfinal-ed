#include "logging.h"

static const char* LOG_TYPE_STR[] = {"INFO", "WARNING", "ERROR", "FATAL"};
void __MKlog(const LogType type, const char* msg, const char* file, unsigned int line) {
    const char* typeStr = LOG_TYPE_STR[type];
    fprintf(stderr, "%s:%u - [%s] %s\n", file, line, typeStr, msg);
}