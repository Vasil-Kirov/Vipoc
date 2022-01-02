#ifdef VIPOC_LINUX

#include "std.h"
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>



// @ PLACE HOLDER, MAKE ERROR MESSAGE BOX!
void Error(const char *error_msg)
{
	vstd_printf("%s\nError Code: %s\n", error_msg, errno);
	exit(1);
}


#endif