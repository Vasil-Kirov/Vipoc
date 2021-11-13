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
