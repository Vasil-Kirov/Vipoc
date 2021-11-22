#pragma once

#include "include/defines.h"


#define vstd_printf(str, ...) _vstd_Printf(sizeof(str) - 1, str, __VA_ARGS__)
#define vstd_sprintf(buff, str, ...) _vstd_sPrintf(sizeof(str) - 1, buff, str, __VA_ARGS__)

typedef struct vstdRect 
{
	int x1;
	int y1;
	int x2;
	int y2;
} vstdRect;


void
vstd_vsnsprintf(char *buff, size_t n, const char *str, va_list args);


int
strcount(char *str, char c, char exclude_next);


char *
vstd_strchr(char *str, char chr);


char *
vstd_strstr(char *str1, char *str2);


VP_API size_t
vstd_strlen(char *str);


VP_API void
vstd_strcat(char *dst, const char *src);


VP_API void
_vstd_IntToStr(int num, char* arr_to_fill);

//@Note: make it so it works with long and chars
VP_API void
_vstd_Printf(int FormatSize, const char* str, ...);


VP_API void
_vstd_sPrintf(int FormatSize, char *buff, const char* str, ...);


VP_API void
Error(const char *error_msg);

