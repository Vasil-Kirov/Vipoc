#ifdef VIPOC_WIN32

#include "std.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define V_WRITE_BUFFER_SIZE 8192




void Error(const char *error_msg)
{
	char buf[4096];
	vstd_sprintf(buf, "%s\nError Code: %d\n", error_msg, GetLastError());
	MessageBoxA(NULL, buf, NULL, MB_OK);
	ExitProcess(1);
}

#endif