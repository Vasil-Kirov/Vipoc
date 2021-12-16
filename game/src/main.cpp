#include "basic.h"
#include <Vipoc.h>
#define Allocate(Size, Storage) Storage.End; Storage.End = (char *)Storage.End + Size
#define Free(Storage) memset(Storage.Start, 0, (char *)Storage.End - (char *)Storage.Start); Storage.End = Storage.Start
#define SIZE(arr) ((int)(sizeof(arr), / sizeof(arr)[0]))

static vp_game Config;
global_var console Console;
#define TILE_MAP_WIDTH 16
#define TILE_MAP_HEIGHT 9

static bool32 IsCameraLocked;
void
LockCamera()
{
	vp_lock_camera(3.14f, 1.45f, (v3){16.5f, 11.8f, 5.7f});
	IsCameraLocked = true;
}

void HandleInput()
{
	if(Console.IsOn) return;
	
    float CamSpeed = 10.0f;
    if(vp_is_keydown(VP_KEY_W))
    {
        vp_move_camera(VP_UP, CamSpeed);
    }
    if(vp_is_keydown(VP_KEY_S))
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
StopConsole();

void
OnKeyDown(vp_keys Key, bool32 IsDown)
{
	if(IsDown)
	{
		if(Console.IsOn)
		{
			if(Key == VP_KEY_MINUS) Console.Command[Console.LastChar++] = '-';
			else if(Key == VP_KEY_BACKSPACE)
			{
				if(Console.LastChar > 0) Console.Command[--Console.LastChar] = 0;
			}
			else if(Key >= 0x41 && Key <= 0x5A)
			{
				int modifier = 32;
				if(vp_is_keydown(VP_KEY_SHIFT)) modifier = 0;
				// 65 - 90
				Console.Command[Console.LastChar++] = Key+modifier;
			}
			else if(Key >= 0x30 && Key <= 0x39) Console.Command[Console.LastChar++] = Key;
			else if(Key >= 0x60 && Key <= 0x69) Console.Command[Console.LastChar++] = Key - 48;
			else if(Key == VP_KEY_SPACE) Console.Command[Console.LastChar++] = ' ';
		}
		if(Key == VP_KEY_DIVIDE)
		{
			if(Console.IsOn)
			{
				StopConsole();
			}
			else
			{
				Console.IsStarting = true;
				Console.Position = 5.625f;
			}
		}
		if(vp_is_keydown(VP_KEY_CONTROL))
		{
			if(Key == VP_KEY_O)
			{
				if(IsCameraLocked)
				{
					vp_unlock_camera();
					IsCameraLocked = false;
				}
				else
				{
					LockCamera();
				}
			}
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

void
GenerateYOffsetForTileMap(uint32 YOffsetOfMap[TILE_MAP_HEIGHT][TILE_MAP_WIDTH], uint64 *RandomSeed)
{
	for(int y = 0; y < TILE_MAP_HEIGHT; ++y)
	{
		for(int x = 0; x < TILE_MAP_WIDTH; ++x)
		{
			YOffsetOfMap[y][x] = vp_random_from_seed(RandomSeed) % 15;
		}
	} 
}

void
HandleConsoleProgression()
{
	Console.Position -= 4.0f * vp_get_dtime();
	if(Console.Position <= 3.5f)
	{
		Console.IsStarting = false;
		Console.IsOn = true;
	}
}

void
StopConsole()
{
	Console.Position = 0;
	Console.IsOn = false;
	memset(Console.Command, 0, Console.LastChar);
	Console.LastChar = 0;
}

void
HandleCommand(char *Command)
{
	
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
		Objects[LastObject++] = vp_load_simple_obj(OutputPathFromSource(EmptyString, PathToFolder, "assets/cube.obj"));
		Objects[LastObject++] = vp_load_simple_obj(OutputPathFromSource(EmptyString, PathToFolder, "assets/lamb.obj"));
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
	
	uint64 RandomSeed = vp_random_seed();
    uint32 YOffsetOfMap[TILE_MAP_HEIGHT][TILE_MAP_WIDTH] = {};
	GenerateYOffsetForTileMap(YOffsetOfMap, &RandomSeed);
	

	uint32 TileMap[TILE_MAP_HEIGHT][TILE_MAP_WIDTH] =
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

	LockCamera();
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
        
		if(Console.IsStarting)
		{
			HandleConsoleProgression();
		}
		if(Console.IsOn || Console.IsStarting)
		{
			vp_draw_rectangle((m2){0, 5.625f, 10, Console.Position}, (v4){0.2f, 0.7f, 0.4f, 0.6f}, 10);
			vp_draw_rectangle((m2){0, Console.Position + .3f, 10, Console.Position + .1f}, (v4){1.0, 1.0f, 1.0f, 1.0f}, 11);
			if(vp_is_keydown(VP_KEY_ENTER))
			{
				HandleCommand(Console.Command);
				StopConsole();
			}
			vp_draw_text(Console.Command, 0, Console.Position + .1f, (v4){0.0f, 0.0f, 0.0f, 1.0f}, 1.0f);
		}
		
		for(int row = 0; row < TILE_MAP_HEIGHT; ++row)
		{
			for(int column = 0; column < TILE_MAP_WIDTH; ++column)
			{
				v4 TileColor = (v4){0.3f, 0.6f, 0.8f, 1.0f};
				if (TileMap[row][column] == 0)
				{
					TileColor = (v4){ 0.5f, 0.5f, 0.5f, 1.0f };
				}
				float column_size = 20;
				float row_size = 20;
				v3 OBJPosition = (v3){column_size * column, (float)YOffsetOfMap[row][column], row_size * row};
				vp_object_pushback(Objects[0], TileColor, OBJPosition);
			}
		}
		
	#if 0
		v3 AxisPosition = {-100, 0, 100};
		v3 CubePosition = {0, 0, -10};
		vp_object_pushback(Objects[0], (v4){1.0f, 1.0f, 1.0f, 1.0f}, AxisPosition);
		vp_object_pushback(Objects[1], (v4){1.0f, 0.0f, 0.5f, 0.1f}, CubePosition);
		vp_object_pushback(Objects[1], (v4){1.0f, 0.0f, 0.5f, 0.1f}, (v3){-20, 0, -30});
		vp_object_pushback(Objects[1], (v4){1.0f, 0.0f, 0.5f, 0.1f}, (v3){-40, 0, -50});
		vp_object_pushback(Objects[1], (v4){1.0f, 0.0f, 0.5f, 0.1f}, (v3){-60, 0, -70});
	#endif
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
		vstd_sprintf(ToDraw, "%dms FPS: %d", MSPerFrame, FPS);
		
		vp_draw_text(ToDraw, .1f, 5.4, (v4){1.0f, 1.0f, 1.0f, 1.0f}, 0.6f);
		vp_present();
		StartCounter = EndCounter;
	}
	return 0;
}