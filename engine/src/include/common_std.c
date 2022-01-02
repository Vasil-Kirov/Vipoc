#include "include/std.h"
#include <immintrin.h>
#include <stdarg.h>
#include "platform/platform.h"
#define V_WRITE_BUFFER_SIZE 8192



int strcount(char *str, char c, char exclude_next)
{
	bool32 hasExclude = true;
	if (exclude_next == 0)
	{
		hasExclude = false;
	}
	int count = 0;
	char *scanner = str;
	while(*scanner!='\0')
	{
		if(*scanner == c) 
		{
			if(hasExclude && *(scanner+1)!=exclude_next) count++;
		}
		scanner++;
	}
	return count;
}


void vstd_strcat(char *dst, const char *src)
{
	dst += vstd_strlen(dst);
	while(*src != '\0')
	{
		*dst = *src;
		dst++;
		src++;
	}
	*dst = '\0';
}

void _vstd_IntToStr(int num, char* arr_to_fill)
{
	if(num == 0)
	{
		arr_to_fill[0] = '0';
		arr_to_fill[1] =  0;
		return;
	}
	int CopyToDivide = num;
	int num_size = -1;           // -1 cuz we want the size - 1 to get the last index of the array.
	while (CopyToDivide != 0)
	{
		CopyToDivide /= 10;
		num_size++;
	}
	if (num < 0)
	{
		arr_to_fill[0] = '-';
		num_size++;
		num = -num;
	}
	arr_to_fill[num_size + 1] = '\0';
	for (int i = num_size; num != 0; --i)
	{
		int tmp = (int)(num % 10);
		num /= 10;
		arr_to_fill[i] = (char)tmp + '0';
	}
}

void
_vstd_U64ToStr(uint64 num, char *arr_to_fill)
{
	if(num == 0)
	{
		arr_to_fill[0] = '0';
		arr_to_fill[1] =  0;
		return;
	}
	uint64 CopyToDivide = num;
	int num_size = -1;           // -1 cuz we want the size - 1 to get the last index of the array.
	while (CopyToDivide != 0)
	{
		CopyToDivide /= 10;
		num_size++;
	}
	if (num < 0)
	{
		arr_to_fill[0] = '-';
		num_size++;
		num = -num;
	}
	arr_to_fill[num_size + 1] = '\0';
	for (int i = num_size; num != 0; --i)
	{
		int tmp = (uint64)(num % 10);
		num /= 10;
		arr_to_fill[i] = (char)tmp + '0';
	}
}

void
_vstd_FloatToStr(float num, char *arr_to_fill)
{
	int array_index = 0;
	if(num < 0.0f)
	{
		arr_to_fill[array_index++] = '-';
		num *= -1;
	}
	int whole_number = num;
	
	int decimal_part = (num-(float)whole_number) * 1000000.0f;
	char whole_number_str[100] = {};
	_vstd_IntToStr(whole_number, whole_number_str);
    
	char decimal_part_str[100] = {};
	_vstd_IntToStr(decimal_part, decimal_part_str);
	
	vstd_strcat(arr_to_fill, whole_number_str);
	vstd_strcat(arr_to_fill, ".");
	vstd_strcat(arr_to_fill, decimal_part_str);
}


// TODO: test 
char*
vstd_strchr(char* str, char chr)
{
	__m128i set_chr = _mm_set1_epi8(chr);
    __m128i null_term = _mm_set1_epi8('\0');
    __m128i result = _mm_set1_epi32(0);
    
    int mask = 0;
    __m128i* restr = (__m128i *)str;
    for (;;)
    {
        __m128i to_cmp = _mm_lddqu_si128((const __m128i *)restr);
        result = _mm_cmpeq_epi8(to_cmp, set_chr);
        mask = _mm_movemask_epi8(result);
        if (mask != 0)
        {
            char* to_ret = (char *)restr;
            unsigned long index;
            index = __builtin_clz(mask);
            to_ret += index;
            return to_ret;
        }
        else
        {
            result = _mm_cmpeq_epi8(to_cmp, null_term);
            mask = _mm_movemask_epi8(result);
            if (mask != 0) return vp_nullptr;
        }
        restr++;
    }
    
	// This shouldn't happen
	return vp_nullptr;
}

size_t
vstd_strlen(char *str)
{
    
    size_t result = 0;
    
    const __m128i zeros = _mm_setzero_si128();
    __m128i* mem = (__m128i*)str;
    
    for (/**/; /**/; mem++, result += 16) 
	{
        
        const __m128i data = _mm_loadu_si128(mem);
        const __m128i cmp  = _mm_cmpeq_epi8(data, zeros);
        
        if (!_mm_testc_si128(zeros, cmp)) 
		{
            int mask = _mm_movemask_epi8(cmp);
            
            return result + __builtin_ctz(mask);
        }
    }
    
    assert(false);
    return 0;
}

char *
vstd_strstr(char *str1, char *str2)
{
	return vp_nullptr;
	// TODO: Finish this, look at the Test++ vs project for the (not-working) implementation
}


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
					char arr[100] = {};
					char* ToCopy = arr;
					_vstd_IntToStr(Number, ToCopy);
					while (*ToCopy != 0)
					{
						Buffer[BufferIndex++] = *ToCopy;
						++ToCopy;
					}
				}break;
				case 'f':
				{
					float Number = (float)va_arg(args, double);
					char arr[100] = {};
					char *ToCopy = arr;
					_vstd_FloatToStr(Number, ToCopy);
					while (*ToCopy != 0)
					{
						Buffer[BufferIndex++] = *ToCopy;
						++ToCopy;
					}
				}
				case 'c':
				{
					char c = (char)va_arg(args, int);
					Buffer[BufferIndex++] = c;
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
    platform_write_file(Buffer, BufferIndex, "0");
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
				case 'f':
				{
					float Number = (float)va_arg(args, double);
					char arr[100] = {};
					char *ToCopy = arr;
					_vstd_FloatToStr(Number, ToCopy);
					while (*ToCopy != 0)
					{
						buff[BufferIndex++] = *ToCopy;
						++ToCopy;
					}
				}break;
				case 'c':
				{
					char c = (char)va_arg(args, int);
					buff[BufferIndex++] = c;
				}break;
				case 'l':
				{
					if(str[FormatIndex+1] == 'l' && str[FormatIndex + 2] == 'u')
					{
						uint64 Number = (uint64)va_arg(args, uint64);
						char arr[100] = {};
						char *ToCopy = arr;
						_vstd_U64ToStr(Number, ToCopy);
						while (*ToCopy != 0)
						{
							buff[BufferIndex++] = *ToCopy;
							++ToCopy;
						}
						FormatIndex += 2;
					}
					else
					{
						goto NORMAL_FORMAT;
					}
				}break;
				case '\\':
				{
					buff[BufferIndex++] = '%';
				}break;
				default:
				{
					NORMAL_FORMAT:
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


// TODO: move this in common_std
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
				case 'f':
				{
					float Number = (float)va_arg(args, double);
					char arr[100] = {};
					char *ToCopy = arr;
					_vstd_FloatToStr(Number, ToCopy);
					while (*ToCopy != 0)
					{
						buff[BufferIndex++] = *ToCopy;
						++ToCopy;
					}
				}break;
				case 'c':
				{
					char c = (char)va_arg(args, int);
					buff[BufferIndex++] = c;
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