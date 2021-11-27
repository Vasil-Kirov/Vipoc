#include "basic.h"
#include <Vipoc.h>
#define Allocate(Size, Storage) Storage.End; Storage.End += Size
#define Free(Storage) memset(Storage.Start, 0, Storage.End - Storage.Start); Storage.End = Storage.Start

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
	Config.config.x = VP_USE_DEFAULT;
	Config.config.y = VP_USE_DEFAULT;
	Config.config.name = "test game";
	Config.vp_on_resize = OnResize;
	vp_init(Config);
	char PathToFolder[MAX_PATH] = {};
	platform_get_absolute_path(PathToFolder);


	allocator PermanentStorage = {};
	PermanentStorage.Start = platform_allocate_memory_chunk(MB(10));
	allocator TempStorage = {};
	TempStorage.Start = platform_allocate_memory_chunk(MB(125));

	PermanentStorage.End = PermanentStorage.Start;
	TempStorage.End = TempStorage.Start;

	rectangle AtlasSize = {};
	entire_file AtlasInfoFile = {};
	AtlasInfoFile.contents = Allocate(MB(1), TempStorage);
	{
		char AtlasInfoFileLocation[MAX_PATH] = {};
		vstd_strcat(AtlasInfoFileLocation, PathToFolder);
		vstd_strcat(AtlasInfoFileLocation, "assets/test_atlas.vat");
		platform_read_entire_file(AtlasInfoFileLocation, &AtlasInfoFile);
		AtlasSize = GetAtlasRect(AtlasInfoFile);

		char AtlasFileLocation[MAX_PATH] = {};
		vstd_strcat(AtlasFileLocation, PathToFolder);
		vstd_strcat(AtlasFileLocation, "assets/test.bmp");

		vp_load_texture(AtlasFileLocation, AtlasSize.w, AtlasSize.h);
	}
	entire_file FontInfoFile = {};
	{
		FontInfoFile.contents = Allocate(MB(1), TempStorage);

		char FontInfoFileLocation[MAX_PATH] = {};
		vstd_strcat(FontInfoFileLocation, PathToFolder);
		vstd_strcat(FontInfoFileLocation, "assets/consolas.fnt");
		platform_read_entire_file(FontInfoFileLocation, &FontInfoFile);

		char FontFileLocation[MAX_PATH] = {};
		vstd_strcat(FontFileLocation, PathToFolder);
		vstd_strcat(FontFileLocation, "assets/consolas.bmp");

		vp_parse_font_fnt(FontInfoFile);
		vp_load_text_atlas(FontFileLocation);
		
	}



	
	atlas_member Textures[1024];
	ParseVATFile(Textures, AtlasInfoFile);

	bool32 Running = true;




	float PlayerX = 400;
	float PlayerY = 400;
	int LastReloadCounter = 0;
	int FPSReloadCounter = 15;
	int32 FPS = 0;
	int32 MSPerFrame = 0;
	reloader Game = {};
	vp_render_target Targets[1024];
	Game = LoadDLL(PathToFolder);
	int XOffset = 0;
	int YOffset = 0;
	float PlayerSpeed = 4.0f;

	Free(TempStorage);

	int64 PerfFrequency = platform_get_frequency();
	int64 StartCounter = platform_get_perf_counter();
	while(Running)
	{
		
		if(LastReloadCounter++ >= 600000)
		{
			if(Game.DLL != vp_nullptr) FreeLibrary(Game.DLL);
			Game = LoadDLL(PathToFolder);
			LastReloadCounter = 0;
		}
		if (vp_is_keydown(VP_KEY_W))
			PlayerY += PlayerSpeed;
		if (vp_is_keydown(VP_KEY_S))
			PlayerY -= PlayerSpeed;
		if (vp_is_keydown(VP_KEY_A))
			PlayerX -= PlayerSpeed;
		if (vp_is_keydown(VP_KEY_D))
			PlayerX += PlayerSpeed;
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



		int64 EndCounter = platform_get_perf_counter();
		int64 Elapsed = EndCounter - StartCounter;
		
		if(FPSReloadCounter++ >= 3)
		{
			MSPerFrame = (int32)(((1000 * Elapsed) / PerfFrequency));
			FPS = PerfFrequency / Elapsed;
			FPSReloadCounter = 0;
		}
		
		char ToDraw[2048] = {};
		vstd_sprintf(ToDraw, "%dms FPS: %d\nnew line", MSPerFrame, FPS);
		vp_draw_text(ToDraw, 5, Config.config.h - 35);

		vp_present();
		StartCounter = EndCounter;
	}
	return 0;
}