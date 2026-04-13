#ifndef __DEBUG_MODULE_H
#define __DEBUG_MODULE_H

#include <stdint.h>
#include <stdio.h>
#include "config.h"

/* Trace levels */
typedef enum {
    DEBUG_LEVEL_NONE  = 0,
    DEBUG_LEVEL_ERROR = 1,
    DEBUG_LEVEL_WARN  = 2,
    DEBUG_LEVEL_INFO  = 3,
    DEBUG_LEVEL_DEBUG = 4
} debug_level_t;

/* Standard log function */
void debug_print(debug_level_t level, const char *tag, const char *fmt, ...);

/* Macros for easy usage */
#define LOG_ERROR(tag, fmt, ...) debug_print(DEBUG_LEVEL_ERROR, tag, fmt, ##__VA_ARGS__)
#define LOG_WARNING(tag, fmt, ...) debug_print(DEBUG_LEVEL_WARN,  tag, fmt, ##__VA_ARGS__)
#define LOG_INFO(tag, fmt, ...) debug_print(DEBUG_LEVEL_INFO,  tag, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(tag, fmt, ...) debug_print(DEBUG_LEVEL_DEBUG, tag, fmt, ##__VA_ARGS__)

#endif /* __DEBUG_MODULE_H */
