#pragma once

#include <stdlib.h>
#include <Vipoc.h>

struct console
{
	bool32 IsOn;
	bool32 IsStarting;
	char Command[1024];
	int LastChar;
	float Position;
};

typedef struct allocator
{
    void *Start;
    void *End;
} allocator;

m2 operator+(m2 A, v2 B)
{
	m2 Result = {};
	Result.x1 = A.x1 + B.x;
	Result.x2 = A.x2 + B.x;
	Result.y1 = A.y1 + B.y;
	Result.y2 = A.y2 + B.y;
	return Result;
}

m2 & operator+=(m2 &A, v2 B)
{
	A = A + B;
	return(A);
}

v2 operator*(v2 A, float B)
{
	v2 Result = {};
	Result.x = A.x * B;
	Result.y = A.y * B;
	return Result;
}

v2 operator*(float B, v2 A)
{
	return(A * B);
}

v2 operator+(v2 A, v2 B)
{
	v2 Result = {};
	Result.x = A.x + B.x;
	Result.y = A.y + B.y;
	return Result;
}

v2 & operator*=(v2 &A, float B)
{
	A = A * B;
	return(A);
}

typedef struct atlas_member
{
    float x;
    float y;
    float w;
    float h;
} atlas_member; 

typedef struct rectangle
{
    int w;
    int h;
} rectangle;

typedef struct hot_render_target
{
    atlas_member Member;
    float X;
    float Y;
    int AtlasWidth;
    int AtlasHeight;
    int LayerID;
} hot_render_target;

#define GAME_UPDATE_AND_RENDER(name) int name(atlas_member *Members, vp_game Game, float PlayerX, float PlayerY, rectangle AtlasSize, vp_2d_render_target *Targets, int XOffset, int YOffset)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);
GAME_UPDATE_AND_RENDER(GameUpdateAndRenderStub)
{
    return 0;
}

typedef struct reloader
{
    HMODULE DLL;
    game_update_and_render *UpdateAndRender;
} reloader;


/* NOTE: the char array EmptyString must be allocated to size of MAX_PATH before the call*/
inline char *
OutputPathFromSource(char *EmptyString, char *SourcePath, const char *ToAdd)
{
	memset(EmptyString, 0, MAX_PATH);
	vstd_strcat(EmptyString, SourcePath);
	vstd_strcat(EmptyString, ToAdd);
	return EmptyString;
}

rectangle
GetAtlasRect(entire_file File)
{
	rectangle Result=  {};
	unsigned char *At = (unsigned char *)File.contents;
	while(*At!='-') At++;
	At++;
	while(true)
	{
		if(*At == '\n') break;
		if(*At == 'w')
		{
			At++;
			Result.w = atoi((const char *)At);
		}
		else if(*At == 'h')
		{
			At++;
			Result.h = atoi((const char *)At);
		}
		At++;
	}
	return Result;
}

unsigned char *
ToNextLine(unsigned char *At)
{
	while(*At!='\n') At++;
	At++;

	return At;
}

void
ParseVATFile(atlas_member *Textures, entire_file File)
{
	unsigned char *At = (unsigned char *)File.contents;
	while(true)
	{
		if(*At == '!') break;
		else if(*At !='#') At = ToNextLine(At);
		else
		{
			atlas_member new_member = {};
			At++; // #
			int insert_index = atoi((const char *)At++);
			while(*At!='#' && *At != '!')
			{
				if(*At== 'x')
				{
					At++;
					new_member.x = atoi((const char *)At);
				}
				else if(*At == 'y')
				{
					At++;
					new_member.y = atoi((const char *)At);
				}
				else if (*At == 'w')
				{
					At++;
					new_member.w = atoi((const char *)At);
				}
				else if (*At == 'h')
				{
					At++;
					new_member.h = atoi((const char *)At);
				}
				At = ToNextLine(At);
			}
			Textures[insert_index] = new_member;
		}
	}
}
