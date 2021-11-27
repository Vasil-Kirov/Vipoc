#include <Vipoc.h>
#include "basic.h"
#define Allocate(Size, Storage) Storage.End; Storage.End += Size

static vp_game Game;


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

bool32 OnResize(vp_game *internal_game, int Width, int Height)
{
	if(Width == 0 || Height == 0) return TRUE;
	Game.config.w = Width;
	Game.config.h = Height;
	return TRUE;
}

float
NormalizeCoordinate(float X, float MaxX, float MinX)
{
	float Result;
	Result = (2.0f * ( (X - MinX) / (MaxX - MinX) )) - 1.0f;
	return Result;
}
float
NormalizeTexCoordinate(float X, float MaxX, float MinX)
{
	float Result;
	Result = ((X - MinX) / (MaxX - MinX));
	return Result;
}

vec4
CalculateAtlasPos(atlas_member Member, int AtlasWidth, int AtlasHeight)
{
	vec4 AtlasPos = {};
	AtlasPos.x1 = NormalizeTexCoordinate(Member.x, Member.w, 0);
	AtlasPos.y1 = NormalizeTexCoordinate(Member.y, Member.h, 0);
	AtlasPos.x2 = NormalizeTexCoordinate(Member.x + Member.w, AtlasWidth, 0);
	AtlasPos.y2 = NormalizeTexCoordinate(Member.y + Member.h, AtlasHeight, 0);
	return AtlasPos;
}

vec4 CalculateWorldPos(atlas_member Member, float X, float Y)
{
	vec4 Pos;
	Pos.x1 = NormalizeCoordinate(X, Game.config.w, 0);
	Pos.y1 = NormalizeCoordinate(Y, Game.config.h, 0);
	Pos.x2 = NormalizeCoordinate(X + Member.w, Game.config.w, 0);
	Pos.y2 = NormalizeCoordinate(Y + Member.h, Game.config.h, 0);
	return Pos;
}

vp_render_target
CalculateRenderTarget(atlas_member Member, float X, float Y, int AtlasWidth, int AtlasHeight, int LayerID)
{
	vp_render_target Result = {};
	Result.layer_id = LayerID;
	Result.world_position = CalculateWorldPos(Member, X, Y);
	Result.texture_position = CalculateAtlasPos(Member, AtlasWidth, AtlasHeight);
	return Result;
}


int main()
{
	Game.config.w = 1280;
	Game.config.h = 720;
	Game.config.x = 50;
	Game.config.y = 50;
	Game.config.name = "test game";
	Game.vp_on_resize = OnResize;
	vp_init(Game);


	// TODO: add relative path
	// TODO: read vat file for width and height

	allocator PermanentStorage = {};
	PermanentStorage.Start = platform_allocate_memory_chunk(MB(10));
	allocator TempStorage = {};
	TempStorage.Start = platform_allocate_memory_chunk(MB(125));

	PermanentStorage.End = PermanentStorage.Start;
	TempStorage.End = TempStorage.Start;

	rectangle AtlasSize = {};
	entire_file AtlasInfoFile;
	AtlasInfoFile.contents = Allocate(KB(10), PermanentStorage);
	platform_read_entire_file("E:/Project/Vipoc/assets/test_atlas.vat", &AtlasInfoFile);
	AtlasSize = GetAtlasRect(AtlasInfoFile);

	vp_load_texture("E:/Project/Vipoc/assets/test.bmp", AtlasSize.w, AtlasSize.h);


	atlas_member Textures[1024];
	ParseVATFile(Textures, AtlasInfoFile);

	bool32 Running = true;

	float tex_x = 200;
	float tex_y = 200;
	while(Running)
	{
		if(vp_is_keydown(VP_KEY_UP)) tex_y += 2.0f;
		if(vp_is_keydown(VP_KEY_LEFT)) tex_x -= 2.0f;
		if (vp_is_keydown(VP_KEY_DOWN)) tex_y -= 2.0f;
		if (vp_is_keydown(VP_KEY_RIGHT)) tex_x += 2.0f;
		Running = vp_handle_messages();

		vp_render_target TransparentCircle = CalculateRenderTarget(Textures[0], tex_x, tex_y, AtlasSize.w, AtlasSize.h, 0);
		
		vp_render_target GreenSquare = CalculateRenderTarget(Textures[1], 100, 100, AtlasSize.w, AtlasSize.h, 1);

		vp_render_pushback(GreenSquare);
		vp_render_pushback(TransparentCircle);

		vp_present();
	}
	return 0;
}