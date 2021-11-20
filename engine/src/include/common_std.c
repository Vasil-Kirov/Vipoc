#include "include/std.h"
#include <immintrin.h>



int strcount(char *str, char c, char exclude_next)
{
	bool32 hasExclude = true;;
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
	while(*dst != '\0') dst++;
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
	}
	arr_to_fill[num_size + 1] = '\0';
	for (int i = num_size; num != 0; --i)
	{
		int tmp = (int)(num % 10);
		num /= 10;
		arr_to_fill[i] = (char)tmp + '0';
	}
}


char*
vstd_strchr(char* str, char chr)
{
    __m256i set_chr = _mm256_set1_epi8(chr);
    __m256i null_term = _mm256_set1_epi8('\0');
    __m256i result = _mm256_set1_epi32(0);

    int mask = 0;
    __m256i* restr = (__m256i*)str;
    for (;;)
    {
        __m256i to_cmp = _mm256_lddqu_si256(restr);
        result = _mm256_cmpeq_epi8(to_cmp, set_chr);
        mask = _mm256_movemask_epi8(result);
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
            result = _mm256_cmpeq_epi8(to_cmp, null_term);
            mask = _mm256_movemask_epi8(result);
            if (mask != 0) return vp_nullptr;
        }
        restr++;
    }

	// This shouldn't happen
	return vp_nullptr;
}

int vstd_strlen(char *str)
{
    char *null_pos = vstd_strchr(str, '\0');
    return null_pos - str;
}


char *
vstd_strstr(char *str1, char *str2)
{
	// TODO: Finish this, look at the Test++ vs project for the (not-working) implementation
}

