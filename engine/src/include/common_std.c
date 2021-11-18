#include "include/std.h"



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


int vstd_strlen(char *str)
{
	char *cpy = str;
	while (*cpy != '\0') cpy++;
	return (int)(cpy - str);
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