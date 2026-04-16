#include "debug_module.h"
#include "bsp_console.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "app_rtos.h"

#define DEBUG_BUF_SIZE 256

void debug_print(debug_level_t level, const char *tag, const char *fmt, ...) {
    // Check global log level
#ifdef APP_DEBUG_LEVEL
    if (level > APP_DEBUG_LEVEL) return;
#endif

    char buffer[DEBUG_BUF_SIZE];
    memset(buffer, 0, DEBUG_BUF_SIZE);
    int len = 0;
    const char *level_str = "";

    switch (level) {
        case DEBUG_LEVEL_ERROR: level_str = "ERROR"; break;
        case DEBUG_LEVEL_WARN:  level_str = "WARN "; break;
        case DEBUG_LEVEL_INFO:  level_str = "INFO "; break;
        case DEBUG_LEVEL_DEBUG: level_str = "DEBUG"; break;
        default: return;
    }

    // Header formatting: [COLOR][TIMESTAMP][LEVEL][TAG] 
    uint32_t ts = osal_get_tick();
    len += snprintf(buffer + len, DEBUG_BUF_SIZE - len, "[%08lu][%s][%s] ", (unsigned long)ts, level_str, tag);

    // Message formatting
    va_list args;
    va_start(args, fmt);
    len += vsnprintf(buffer + len, DEBUG_BUF_SIZE - len, fmt, args);
    va_end(args);

    // Enqueue for transmission
    if (len > 0 && len < DEBUG_BUF_SIZE) {
        Console_Packet_t packet;
        memset(&packet, 0, sizeof(Console_Packet_t));
        
        packet.size = (uint16_t)len;
        memcpy(packet.data, buffer, packet.size);
        
        osal_queue_put(consoleTxQueueHandle, &packet, 0);
    }
}
