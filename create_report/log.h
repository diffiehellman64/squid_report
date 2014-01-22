#ifndef __LOG_H_
#define __LOG_H_

#define IS_DEBUG 0
#define LOG_LEVEL LOG_VERBOSE

#define LOG_DEFAULT 1
#define LOG_VERBOSE 2

//WIP debug macro
#if IS_DEBUG == 1

#define DEBUG(MSG_LOG_LVL, fmt, arg...) \
do {\
    if (MSG_LOG_LVL <= LOG_LEVEL) {\
        fprintf(stderr, fmt,\
        ##arg); \
    } \
} while (0)

#else

#define DEBUG(LOG_LVL, fmt, arg...)

#endif

#endif
