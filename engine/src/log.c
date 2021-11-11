#include "log.h"
#include "platform/platform.h"


void
logger_init()
{
    platform_allocate_console();
}


void vp_log(log_level level, const char *str, char *color, ...)
{
    const char *to_append[] = {"[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[INFO]: "};
    char to_print[4096];
    memset(to_print, 0, 4096);

//    vstd_strcat(to_print, color);
    vstd_strcat(to_print, to_append[level]);
    vstd_strcat(to_print, str);

    va_list args;
    va_start(args, color);

    vstd_vsnsprintf(to_print, 4096, to_print, args);

    va_end(args);
    
//    vstd_strcat(to_print, ANSI_COLOR_RESET); WINDOWS FUCKTARDS COMMAND LINE DOESNT SUPPORT ANSI COLORING CODES 

    vstd_strcat(to_print, "\n");

    platform_output_string(to_print, level);
    
}

