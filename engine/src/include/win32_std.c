#ifdef VIPOC_WIN32

#include "std.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define V_WRITE_BUFFER_SIZE 8192



void _vstd_Printf(int FormatSize, const char* str, ...)
{
    HANDLE STDOUT_HANDLE = GetStdHandle((DWORD)-11);
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
    DWORD BytesWritten;
    WriteFile(STDOUT_HANDLE, Buffer, BufferIndex, &BytesWritten, NULL);
}


void
vstd_vsnsprintf(char *buff, size_t n, const char *str, va_list args)
{
    int FormatSize = vstd_strlen((char *)str);
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
                    char* ToCopy = va_arg(args, char*);
                    while (*ToCopy != 0)
                    {
                        buff[BufferIndex++] = *ToCopy;
                        ++ToCopy;
                    }
                }break;
                case 'd':
                {
                    int Number = va_arg(args, int);
                    char arr[20];
                    char* ToCopy = arr;
                    if (Number != 0)
                    {
                        _vstd_IntToStr(Number, ToCopy);
                        while (*ToCopy != 0)
                        {
                            buff[BufferIndex++] = *ToCopy;
                            ++ToCopy;
                        }
                    }
                    else buff[BufferIndex++] = '0';
                }break;
                case '\\':
                {
                    buff[BufferIndex++] = '%';
                }break;
                default:
                {
                    buff[BufferIndex++] = '%';
                    buff[BufferIndex++] = str[FormatIndex];
                }break;
            }
        }
        else
        {
            buff[BufferIndex++] = str[FormatIndex];
        }
    }
}

void _vstd_sPrintf(int FormatSize, char *buff, const char* str, ...)
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
                    char* ToCopy = va_arg(args, char*);
                    while (*ToCopy != 0)
                    {
                        buff[BufferIndex++] = *ToCopy;
                        ++ToCopy;
                    }
                }break;
                case 'd':
                {
                    int Number = va_arg(args, int);
                    char arr[20];
                    char* ToCopy = arr;
                    if (Number != 0)
                    {
                        _vstd_IntToStr(Number, ToCopy);
                        while (*ToCopy != 0)
                        {
                            buff[BufferIndex++] = *ToCopy;
                            ++ToCopy;
                        }
                    }
                    else buff[BufferIndex++] = '0';
                }break;
                case '\\':
                {
                    buff[BufferIndex++] = '%';
                }break;
                default:
                {
                    buff[BufferIndex++] = '%';
                    buff[BufferIndex++] = str[FormatIndex];
                }break;
            }
        }
        else
        {
            buff[BufferIndex++] = str[FormatIndex];
        }
    }
    va_end(args);
}


void Error(const char *error_msg)
{
	char buf[4096];
	vstd_sprintf(buf, "%s\nError Code: %d\n", error_msg, GetLastError());
	MessageBoxA(NULL, buf, NULL, MB_OK);
	ExitProcess(1);
}

#endif