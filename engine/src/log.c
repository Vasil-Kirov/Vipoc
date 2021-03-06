#include "log.h"
#include "platform/platform.h"
#include <stdarg.h>
#include <string.h>




void
logger_init()
{
    platform_allocate_console();
}


void vp_log(log_level level, const char *str, const char *color, ...)
{
    const char *to_append[] = {"[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[INFO]: "};
    char to_print[4096] = {};
    memset(to_print, 0, 4096);

//    vstd_strcat(to_print, color);
    vstd_strcat(to_print, to_append[level]);
    vstd_strcat(to_print, str);

    va_list args;
    va_start(args, color);

    char copy[4096] = {};
    memcpy(copy, to_print, 4096);
    vstd_vsnsprintf(to_print, 4096, copy, args);

    va_end(args);
    
//    vstd_strcat(to_print, ANSI_COLOR_RESET); WINDOWS FUCKTARDS COMMAND LINE DOESNT SUPPORT ANSI COLORING CODES 

    vstd_strcat(to_print, "\n");

    platform_output_string(to_print, level);
    
    if(level == LOG_LEVEL_FATAL) platform_exit(1);
    
}

