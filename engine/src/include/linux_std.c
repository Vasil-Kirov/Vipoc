#ifdef VIPOC_LINUX

#include "std.h"
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#define V_WRITE_BUFFER_SIZE 8192



void _vstd_Printf(int FormatSize, const char* str, ...)
{
	va_list args;
	va_start(args, str);
	
	char Buffer[V_WRITE_BUFFER_SIZE];
	int BufferIndex = 0;
	for (int FormatIndex = 0; FormatIndex < FormatSize; ++FormatIndex)
	{
		char FormatChar = str[FormatIndex];
		if (FormatChar == '%')
		{
			++FormatIndex;
			switch (str[FormatIndex])
			{
				case 's':
				{
					char* ToCopy = va_arg(args, char *);
					while (*ToCopy != 0)
					{
						Buffer[BufferIndex++] = *ToCopy;
						++ToCopy;
					}
				}break;
				case 'd':
				{
					int Number = va_arg(args, int);
					char arr[20];
					char* ToCopy = arr;
					_vstd_IntToStr(Number, ToCopy);
					while (*ToCopy != 0)
					{
						Buffer[BufferIndex++] = *ToCopy;
						++ToCopy;
					}
				}break;
				case '\\':
				{
					Buffer[BufferIndex++] = '%';
				}break;
				default:
				{
					Buffer[BufferIndex++] = '%';
					Buffer[BufferIndex++] = str[FormatIndex];
				}break;
			}
		}
		else
		{
			Buffer[BufferIndex++] = str[FormatIndex];
		}
	}
	va_end(args);

	const int stdout_fd = 1;
	write(stdout_fd, Buffer, BufferIndex);	
}

// @ make sure the buffer has enough space for the string
void _vstd_sPrintf(int FormatSize, char *Buffer, const char* str, ...)
{
	va_list args;
	va_start(args, str);
	
	int BufferIndex = 0;
	for (int FormatIndex = 0; FormatIndex < FormatSize; ++FormatIndex)
	{
		char FormatChar = str[FormatIndex];
		if (FormatChar == '%')
		{
			++FormatIndex;
			switch (str[FormatIndex])
			{
				case 's':
				{
					char* ToCopy = va_arg(args, char *);
					while (*ToCopy != 0)
					{
						Buffer[BufferIndex++] = *ToCopy;
						++ToCopy;
					}
				}break;
				case 'd':
				{
					int Number = va_arg(args, int);
					char arr[20];
					char* ToCopy = arr;
					_vstd_IntToStr(Number, ToCopy);
					while (*ToCopy != 0)
					{
						Buffer[BufferIndex++] = *ToCopy;
						++ToCopy;
					}
				}break;
				case '\\':
				{
					Buffer[BufferIndex++] = '%';
				}break;
				default:
				{
					Buffer[BufferIndex++] = '%';
					Buffer[BufferIndex++] = str[FormatIndex];
				}break;
			}
		}
		else
		{
			Buffer[BufferIndex++] = str[FormatIndex];
		}
	}
	va_end(args);

}


// @ PLACE HOLDER, MAKE ERROR MESSAGE BOX!
void Error(const char *error_msg)
{
	vstd_printf("%s\nError Code: %s\n", error_msg, errno);
	exit(1);
}


#endif