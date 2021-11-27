#include "basic.h"
#include <Vipoc.h>
#define Allocate(Size, Storage) Storage.End; Storage.End += Size

static vp_game Config;


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
	Config.config.w = Width;
	Config.config.h = Height;
	return TRUE;
}

reloader
LoadDLL(char *PathToFolder)
{
	reloader Result = {};
	char DLLPath[MAX_PATH] = {};
	strcpy(DLLPath, PathToFolder);
	vstd_strcat(DLLPath, "bin\\hot_reload.dll");
	
	char TempDLLPath[MAX_PATH] = {};
	strcpy(TempDLLPath, PathToFolder);
	vstd_strcat(TempDLLPath, "bin\\hot_reload_temp.dll");
	
	if(CopyFile(DLLPath, TempDLLPath, FALSE) == FALSE) VP_ERROR("Failed to copy DLL!");
	Result.DLL = LoadLibraryA(TempDLLPath);
	if(Result.DLL == vp_nullptr)
	{
		VP_ERROR("Failed to load DLL");
		return (reloader){};
	}
	// TODO: Default UpdateAndRender function if this one fails
	Result.UpdateAndRender = (game_update_and_render *)GetProcAddress(Result.DLL, "GameUpdateAndRender");
	if(Result.UpdateAndRender == vp_nullptr) VP_ERROR("Failed to get GameUpdateAndRender function!");
	return Result;
}



int main()
{
	Config.config.w = 1280;
	Config.config.h = 720;
	Config.config.x = 50;
	Config.config.y = 50;
	Config.config.name = "test game";
	Config.vp_on_resize = OnResize;
	vp_init(Config);


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


	char PathToFolder[MAX_PATH] = {};
	platform_get_absolute_path(PathToFolder);

	float PlayerX = 400;
	float PlayerY = 400;
	int LastReloadCounter = 0;
	reloader Game = {};
	vp_render_target Targets[1024];
	Game = LoadDLL(PathToFolder);
	int XOffset = 0;
	int YOffset = 0;
	while(Running)
	{
		if(LastReloadCounter++ >= 60)
		{
			if(Game.DLL != vp_nullptr) FreeLibrary(Game.DLL);
			Game = LoadDLL(PathToFolder);
		}
		if (vp_is_keydown(VP_KEY_UP))
			YOffset += 2.0f;
		if (vp_is_keydown(VP_KEY_LEFT))
			XOffset -= 2.0f;
		if (vp_is_keydown(VP_KEY_DOWN))
			YOffset -= 2.0f;
		if (vp_is_keydown(VP_KEY_RIGHT))
			XOffset += 2.0f;

		Running = vp_handle_messages();
		
		int TargetsSize = 0;
		if(Game.UpdateAndRender != vp_nullptr)
		{
			TargetsSize = Game.UpdateAndRender(Textures, Config, PlayerX, PlayerY, AtlasSize, Targets, XOffset, YOffset);
		}
		for (int Index = 0; Index < TargetsSize; ++Index)
		{
			vp_render_pushback(Targets[Index]);
		}

		vp_present();
	}
	return 0;
}