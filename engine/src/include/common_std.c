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
	while (*scanner != '\0')
	{
		if (*scanner == c)
		{
			if (hasExclude && *(scanner + 1) != exclude_next)
				count++;
		}
		scanner++;
	}
	return count;
}

void vstd_strcat(char *dst, const char *src)
{
	dst += vstd_strlen(dst);
	while (*src != '\0')
	{
		*dst = *src;
		dst++;
		src++;
	}
	*dst = '\0';
}

void _vstd_IntToStr(int num, char *arr_to_fill)
{
	if (num == 0)
	{
		arr_to_fill[0] = '0';
		arr_to_fill[1] = 0;
		return;
	}
	int CopyToDivide = num;
	int num_size = -1; // -1 cuz we want the size - 1 to get the last index of the array.
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

void _vstd_u32_to_x(u32 num, char *arr_to_fill, b32 lower_case)
{
	u32 digit_count = 0;
	u32 num_copy = num;

	if (num == 0)
		digit_count = 1;
	else
		while (num_copy != 0)
		{
			++digit_count;
			num_copy /= 16;
		}

	const char CONV_TABLE[] = {
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	int at = 0;
	arr_to_fill[at++] = '0';
	arr_to_fill[at++] = 'x';
	at = 1 + digit_count;
	do
	{
		u8 digit = num % 16;
		arr_to_fill[at--] = CONV_TABLE[digit] > '9' ? CONV_TABLE[digit] + (lower_case * 32) : CONV_TABLE[digit];
		num /= 16;
	} while (num != 0);
}

void _vstd_U64ToStr(uint64 num, char *arr_to_fill)
{
	if (num == 0)
	{
		arr_to_fill[0] = '0';
		arr_to_fill[1] = 0;
		return;
	}
	uint64 CopyToDivide = num;
	int num_size = -1; // -1 cuz we want the size - 1 to get the last index of the array.
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

void _vstd_FloatToStr(float num, char *arr_to_fill)
{
	int array_index = 0;
	if (num < 0.0f)
	{
		arr_to_fill[array_index++] = '-';
		num *= -1;
	}
	int whole_number = num;

	int decimal_part = (num - (float)whole_number) * 1000000.0f;
	char whole_number_str[100] = {};
	_vstd_IntToStr(whole_number, whole_number_str);

	char decimal_part_str[100] = {};
	_vstd_IntToStr(decimal_part, decimal_part_str);

	vstd_strcat(arr_to_fill, whole_number_str);
	vstd_strcat(arr_to_fill, ".");
	vstd_strcat(arr_to_fill, decimal_part_str);
}

// TODO: test
char *
vstd_strchr(char *str, char chr)
{
	__m128i set_chr = _mm_set1_epi8(chr);
	__m128i null_term = _mm_set1_epi8('\0');
	__m128i result = _mm_set1_epi32(0);

	int mask = 0;
	__m128i *restr = (__m128i *)str;
	for (;;)
	{
		__m128i to_cmp = _mm_lddqu_si128((const __m128i *)restr);
		result = _mm_cmpeq_epi8(to_cmp, set_chr);
		mask = _mm_movemask_epi8(result);
		if (mask != 0)
		{
			char *to_ret = (char *)restr;
			unsigned long index;
			index = __builtin_clz(mask);
			to_ret += index;
			return to_ret;
		}
		else
		{
			result = _mm_cmpeq_epi8(to_cmp, null_term);
			mask = _mm_movemask_epi8(result);
			if (mask != 0)
				return vp_nullptr;
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
	__m128i *mem = (__m128i *)str;

	for (/**/; /**/; mem++, result += 16)
	{

		const __m128i data = _mm_loadu_si128(mem);
		const __m128i cmp = _mm_cmpeq_epi8(data, zeros);

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

int _vstd_format(char *buffer, int format_size, const char *str, va_list args)
{
	//@Note: used for hex digits
	b32 lower_case = false;

	int buffer_index = 0;
	for (int format_index = 0; format_index < format_size; ++format_index)
	{
		char FormatChar = str[format_index];
		if (FormatChar == '%')
		{
			++format_index;
			switch (str[format_index])
			{
			case 's':
			{
				char *to_copy = va_arg(args, char *);
				while (*to_copy != 0)
				{
					buffer[buffer_index++] = *to_copy;
					++to_copy;
				}
			}
			break;
			case 'd':
			{
				int Number = va_arg(args, int);
				char arr[100] = {};
				char *to_copy = arr;
				_vstd_IntToStr(Number, to_copy);
				while (*to_copy != 0)
				{
					buffer[buffer_index++] = *to_copy;
					++to_copy;
				}
			}
			break;
			case 'f':
			{
				float Number = (float)va_arg(args, double);
				char arr[100] = {};
				char *to_copy = arr;
				_vstd_FloatToStr(Number, to_copy);
				while (*to_copy != 0)
				{
					buffer[buffer_index++] = *to_copy;
					++to_copy;
				}
			}
			case 'c':
			{
				char c = (char)va_arg(args, int);
				buffer[buffer_index++] = c;
			}
			break;
			case 'l':
			{
				if (str[format_index + 1] == 'l' && str[format_index + 2] == 'u')
				{
					uint64 Number = (uint64)va_arg(args, uint64);
					char arr[100] = {};
					char *to_copy = arr;
					_vstd_U64ToStr(Number, to_copy);
					while (*to_copy != 0)
					{
						buffer[buffer_index++] = *to_copy;
						++to_copy;
					}
					format_index += 2;
				}
				else
				{
					goto NORMAL_FORMAT;
				}
			}
			break;
			case 'x':
				lower_case = true;
			case 'X':
			{

				u32 number = (u32)va_arg(args, unsigned);
				char arr[100] = {};
				char *to_copy = arr;
				_vstd_u32_to_x(number, to_copy, lower_case);
				lower_case = false;
				while (*to_copy != 0)
				{
					buffer[buffer_index++] = *to_copy;
					++to_copy;
				}
			}
			break;
			case '\\':
			{
				buffer[buffer_index++] = '%';
			}
			break;
			default:
			{
			NORMAL_FORMAT:
				buffer[buffer_index++] = '%';
				buffer[buffer_index++] = str[format_index];
			}
			break;
			}
		}
		else
		{
			buffer[buffer_index++] = str[format_index];
		}
	}
	va_end(args);
	return buffer_index;
}

void _vstd_Printf(int format_size, const char *str, ...)
{

	char buffer[V_WRITE_BUFFER_SIZE];
	va_list args;
	va_start(args, str);
	int buffer_size = _vstd_format(buffer, format_size, str, args);

	platform_write_file(buffer, buffer_size, "0");
}

void _vstd_sPrintf(int format_size, char *buff, const char *str, ...)
{
	va_list args;
	va_start(args, str);
	_vstd_format(buff, format_size, str, args);
}

// TODO: move this in common_std
void vstd_vsnsprintf(char *buff, const char *str, va_list args)
{
	int format_size = vstd_strlen((char *)str);
	_vstd_format(buff, format_size, str, args);
}