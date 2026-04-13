#include "debug_module.h"
#include "bsp_console.h"
#include <stdarg.h>
#include <string.h>

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

    // Header formatting: [COLOR][LEVEL][TAG] 
#ifdef APP_DEBUG_USE_COLORS
    len += snprintf(buffer + len, DEBUG_BUF_SIZE - len, "%s[%s][%s] ", color_str, level_str, tag);
#else
    len += snprintf(buffer + len, DEBUG_BUF_SIZE - len, "[%s][%s] ", level_str, tag);
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

    // Send to BSP
    if (len > 0) {
        BSP_Console_Send((uint8_t *)buffer, (uint16_t)len);
    }
}
