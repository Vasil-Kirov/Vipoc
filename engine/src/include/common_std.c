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

    for (/**/; /**/; mem++, result += 16) {

        const __m128i data = _mm_loadu_si128(mem);
        const __m128i cmp  = _mm_cmpeq_epi8(data, zeros);

        if (!_mm_testc_si128(zeros, cmp)) {

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
	// TODO: Finish this, look at the Test++ vs project for the (not-working) implementation
}

