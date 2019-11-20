#ifndef __SYSLOG_H__
#define __SYSLOG_H__
#include <stdio.h>

#define MODULE_TAG    ""
#define DBG     "D"
#define WARNING "W"
#define ERROR   "E"
#define INFO    "I"
#define LOG(level, format, args...) \
do { \
    fprintf(stderr, "(%s )[%s] %s:%d] " format, \
    level, MODULE_TAG, __FUNCTION__, __LINE__, ## args ); \
} while(0)

#endif