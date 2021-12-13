#include "basic.h"
#include <Vipoc.h>
#define Allocate(Size, Storage) Storage.End; Storage.End = (char *)Storage.End + Size
#define Free(Storage) memset(Storage.Start, 0, (char *)Storage.End - (char *)Storage.Start); Storage.End = Storage.Start

static vp_game Config;


void HandleInput()
{
    float CamSpeed = 10.0f;
    if(vp_is_keydown(VP_KEY_SPACE))
    {
        vp_move_camera(VP_UP, CamSpeed);
    }
    if(vp_is_keydown(VP_KEY_SHIFT))
    {
        vp_move_camera(VP_DOWN, CamSpeed);
    }
    if(vp_is_keydown(VP_KEY_A))
    {
        vp_move_camera(VP_LEFT, CamSpeed);
    }
    if(vp_is_keydown(VP_KEY_D))
    {
        vp_move_camera(VP_RIGHT, CamSpeed);
    }
}


static bool32 VSyncToggle = false;

void
OnKeyDown(vp_keys Key, bool32 IsDown)
{
	if(IsDown)
	{
		if(vp_is_keydown(VP_KEY_CONTROL))
		{
			if(Key == VP_KEY_X)
				vp_toggle_polygons();
			
			if(vp_is_keydown(VP_KEY_V))
			{
				platform_toggle_vsync(VSyncToggle);
				VSyncToggle = !VSyncToggle;
			}
		}
		
	}
}

bool32
OnResize(vp_game *internal_game, int Width, int Height)
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

int
main()
{
	Config.config.w = 1280;
	Config.config.h = 720;
	Config.config.x = VP_USE_DEFAULT;
	Config.config.y = VP_USE_DEFAULT;
	Config.config.name = "test game";
	Config.config.vp_on_key_down = OnKeyDown;
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
        
		vp_load_texture(AtlasFileLocation);
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

	int Objects[1024] = {};
	int LastObject = 0;
	memset(Objects, -1, 1024);
	{
		char EmptyString[MAX_PATH] = {};
		Objects[LastObject++] = vp_load_simple_obj(OutputPathFromSource(EmptyString, PathToFolder, "assets/simple_axis.obj"));
		Objects[LastObject++] = vp_load_simple_obj(OutputPathFromSource(EmptyString, PathToFolder, "assets/cube.obj"));
	}
	atlas_member Textures[1024];
	ParseVATFile(Textures, AtlasInfoFile);
    
	bool32 Running = true;
    
    
	int LastReloadCounter = 0;
	int FPSReloadCounter = 15;
	int32 FPS = 0;
	int32 MSPerFrame = 0;
	reloader Game = {};
	Game = LoadDLL(PathToFolder);
    
	Free(TempStorage);
    
#if 0
	uint32 TileMap[9][17] =
	{
		{1, 1, 1, 1,  1, 1, 1, 1,   1, 1, 1, 1,  1, 1, 1, 1}, 
		{1, 0, 0, 0,  0, 0, 0, 0,   0, 0, 0, 0,  0, 0, 0, 1}, 
		{1, 0, 0, 0,  0, 0, 0, 0,   0, 0, 0, 0,  0, 0, 0, 1}, 
		{1, 0, 0, 0,  0, 0, 0, 0,   0, 0, 0, 0,  0, 0, 0, 1}, 
		{1, 0, 0, 0,  0, 0, 0, 0,   0, 0, 0, 0,  0, 0, 0, 1}, 
		{1, 0, 0, 0,  0, 0, 0, 0,   0, 0, 0, 0,  0, 0, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,   0, 0, 0, 0,  0, 0, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,   0, 0, 0, 0,  0, 0, 0, 1},
		{1, 1, 1, 1,  1, 1, 1, 1,   1, 1, 1, 1,  1, 1, 1, 1}
	};
#endif
    
	int64 PerfFrequency = platform_get_frequency();
	int64 StartCounter = platform_get_perf_counter();
	while(Running)
	{
		if(LastReloadCounter++ >= 120)
		{
			if(Game.DLL != vp_nullptr) FreeLibrary(Game.DLL);
			Game = LoadDLL(PathToFolder);
			LastReloadCounter = 0;
		}
        
		Running = vp_handle_messages();
        
		v3 AxisPosition = {100, 0, -100};
		v3 CubePosition = {0, -20, 0};
		vp_object_pushback(Objects[0], (v4){1.0f, 1.0f, 1.0f, 1.0f}, AxisPosition);
		vp_object_pushback(Objects[1], (v4){1.0f, 0.0f, 0.5f, 0.1f}, CubePosition);
		HandleInput();
        
        
		/* END OF CODE! ONLY PERFORMANCE CALCULATIONS AFTER THIS LABEL */
		int64 EndCounter = platform_get_perf_counter();
		int64 Elapsed = EndCounter - StartCounter;
		
		if(FPSReloadCounter++ >= 3)
		{
			MSPerFrame = (int32)(((1000 * Elapsed) / PerfFrequency));
			FPS = PerfFrequency / Elapsed;
			FPSReloadCounter = 0;
		}
		
		char ToDraw[2048] = {};	
		vstd_sprintf(ToDraw, "%dms FPS: %d\nlines\nof\ntext", MSPerFrame, FPS);
		
		vp_draw_text(ToDraw, 5, Config.config.h - 35, (v4){1.0f, 1.0f, 1.0f, 1.0f});
		vp_present();
		StartCounter = EndCounter;
	}
	return 0;
}