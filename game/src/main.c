#include <Vipoc.h>
#include "basic.h"
#define Allocate(Size, Storage) Storage.End; Storage.End += Size

static vp_game Game;


unsigned char *ToNextLine(unsigned char *At)
{
	while(*At!='\n') At++;
	At++;

	return At;
}
void ParseVATFile(atlas_member *Textures, entire_file File)
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

vec4 GetAtlasPos(atlas_member member, int AtlasWidth, int AtlasHeight)
{
	vec4 AtlasPos = {};
	AtlasPos.x1 = member.x / member.w;
	AtlasPos.y1 = member.y / member.h;
	AtlasPos.x2 = AtlasPos.x1 + (member.w / (float)AtlasWidth);
	AtlasPos.y2 = AtlasPos.y1 + (member.h / (float)AtlasHeight);
	return AtlasPos;
}
vec4 CalculatePos(float x, float y, atlas_member member)
{
	vec4 Pos;
	Pos.x1 = x/Game.config.w;
	Pos.y1 = y/Game.config.h;
	Pos.x2 = Pos.x1 + (member.w / Game.config.w);
	Pos.y2 = Pos.y1 + (member.h / Game.config.h);
	return Pos;
}

int main()
{
	Game.config.w = 1600;
	Game.config.h = 900;
	Game.config.x = 200;
	Game.config.y = 200;
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

	int AtlasWidth = 600;
	int AtlasHeight = 200;

	vp_load_texture("E:/Project/Vipoc/assets/test.bmp", AtlasWidth, AtlasHeight);
	entire_file AtlasInfoFile;
	AtlasInfoFile.contents = Allocate(KB(10), PermanentStorage);
	platform_read_entire_file("E:/Project/Vipoc/assets/test_atlas.vat", &AtlasInfoFile);
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

		vec4 Pos = CalculatePos(tex_x, tex_y, Textures[1]);
		
		vec4 AtlasPos = GetAtlasPos(Textures[1], AtlasWidth, AtlasHeight);
		
		vp_render_pushback(Pos, AtlasPos);

		vp_present();
	}
	return 0;
}