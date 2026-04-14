#include "debug_module.h"
#include "bsp_console.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "app_rtos.h"

#define DEBUG_BUF_SIZE 256

/* ANSI Color codes */
#define CLR_RESET  "\033[0m"
#define CLR_RED    "\033[31m"
#define CLR_YELLOW "\033[33m"
#define CLR_BLUE   "\033[34m"
#define CLR_GRAY   "\033[90m"

void debug_print(debug_level_t level, const char *tag, const char *fmt, ...) {
    // Check global log level
#ifdef APP_DEBUG_LEVEL
    if (level > APP_DEBUG_LEVEL) return;
#endif

    char buffer[DEBUG_BUF_SIZE];
    int len = 0;
    const char *level_str = "";
    const char *color_str = "";
    (void)color_str;

    switch (level) {
        case DEBUG_LEVEL_ERROR:
            level_str = "ERROR";
            color_str = CLR_RED;
            break;
        case DEBUG_LEVEL_WARN:
            level_str = "WARN ";
            color_str = CLR_YELLOW;
            break;
        case DEBUG_LEVEL_INFO:
            level_str = "INFO ";
            color_str = CLR_BLUE;
            break;
        case DEBUG_LEVEL_DEBUG:
            level_str = "DEBUG";
            color_str = CLR_GRAY;
            break;
        default:
            return;
    }

    // Header formatting: [COLOR][TIMESTAMP][LEVEL][TAG] 
    uint32_t ts = osal_get_tick();
#ifdef APP_DEBUG_USE_COLORS
    len += snprintf(buffer + len, DEBUG_BUF_SIZE - len, "%s[%08lu][%s][%s] ", color_str, (unsigned long)ts, level_str, tag);
#else
    len += snprintf(buffer + len, DEBUG_BUF_SIZE - len, "[%08lu][%s][%s] ", (unsigned long)ts, level_str, tag);
#endif

    // Message formatting
    va_list args;
    va_start(args, fmt);
    len += vsnprintf(buffer + len, DEBUG_BUF_SIZE - len, fmt, args);
    va_end(args);

    // Footer formatting
#ifdef APP_DEBUG_USE_COLORS
    len += snprintf(buffer + len, DEBUG_BUF_SIZE - len, "%s\r\n", CLR_RESET);
#else
    len += snprintf(buffer + len, DEBUG_BUF_SIZE - len, "\r\n");
#endif

    // Enqueue for transmission
    if (len > 0) {
        Console_Packet_t packet;
        packet.size = (len > 128) ? 128 : (uint16_t)len;
        memcpy(packet.data, buffer, packet.size);
        
        // Put in queue with 0 timeout to avoid blocking high priority tasks 
        // if the console is too slow.
        osal_queue_put(consoleTxQueueHandle, &packet, 0);
    }
}
