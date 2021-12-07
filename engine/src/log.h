#ifdef __cplusplus
	extern "C"{
#endif

#pragma once

#include "include/std.h"


#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"

#define ANSI_COLOR_RESET   "\x1b[0m"

typedef enum log_level
{
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3
} log_level;


void
logger_init();


VP_API void
vp_log(log_level level, const char *str, const char *color, ...);



#define VP_FATAL(str, ...) vp_log(LOG_LEVEL_FATAL, str, ANSI_COLOR_RED, __VA_ARGS__)
#define VP_ERROR(str, ...) vp_log(LOG_LEVEL_ERROR, str, ANSI_COLOR_MAGENTA, __VA_ARGS__)
#define VP_WARN(str, ...) vp_log(LOG_LEVEL_WARN, str, ANSI_COLOR_YELLOW, __VA_ARGS__)
#define VP_INFO(str, ...) vp_log(LOG_LEVEL_INFO, str, ANSI_COLOR_CYAN, __VA_ARGS__)



#ifdef __cplusplus
	}
#endif