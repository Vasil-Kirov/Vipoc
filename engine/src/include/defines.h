#pragma once

#include <stdint.h>


#ifdef VIPOC_DEBUG
#define assert(expression) if(!expression) {*(int *)0 = 0;}
#else
#define assert(expression)
#endif


#ifdef VIPOC_EXPORT
#define VP_API __declspec(dllexport)
#else
#define VP_API __declspec(dllimport)
#endif


#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"

#define ANSI_COLOR_RESET   "\x1b[0m"



typedef int32_t bool32;
typedef wchar_t wchar;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;


#define _KB(b) b << 10
#define _MB(b) b << 20
#define _GB(b) b << 30

#define KB(b) _KB((int64)b)
#define MB(b) _MB((int64)b)
#define GB(b) _GB((int64)b)


#define internal static
#define local_var static
#define global_var static

#define vp_nullptr		(void *)0
#define PI				3.14159265358979323846

typedef struct render_buffer
{
	uint8 *Memory;
	int32 Width;
	int32 Height;
	int32 BytesPerPixel;
} render_buffer;

#define true 1
#define false 0