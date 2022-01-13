#include "basic.h"
#include <Vipoc.h>
#define Allocate(Size, Storage) Storage.End; Storage.End = (char *)Storage.End + Size
#define Free(Storage) memset(Storage.Start, 0, (char *)Storage.End - (char *)Storage.Start); Storage.End = Storage.Start
#define SIZE(arr) ((int)(sizeof(arr), / sizeof(arr)[0]))

static vp_game Config;
global_var console Console;
#define TILE_MAP_WIDTH 16
#define TILE_MAP_HEIGHT 9
global_var v2 PlayerPosition = {1, 1};
global_var uint32 TileMap[TILE_MAP_HEIGHT][TILE_MAP_WIDTH];

static bool32 IsCameraLocked;
void
LockCamera()
{
	vp_lock_camera(3.14f, 1.45f, (v3){15.25f, 12.5f, 5.7f});
	IsCameraLocked = true;
}


void
HandleInput()
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
HandlePlayerMovement(vp_keys Key)
{
    if(Console.IsOn) return;
	if(!IsCameraLocked) return;
	uint32 PosX = (uint32)PlayerPosition.x;
    uint32 PosY = (uint32)PlayerPosition.y;
    
	if(Key == VP_KEY_W)
    {
		if(TileMap[PosY + 1][PosX] != 1)
		{
			PlayerPosition.y++;
		}
    }
    if(Key == VP_KEY_S)
    {
		if(TileMap[PosY - 1][PosX] != 1)
		{
			PlayerPosition.y--;
		}
    }
    if(Key == VP_KEY_A)
    {
		if(TileMap[PosY][PosX + 1] != 1)
		{
			PlayerPosition.x++;
		}
	}
    if(Key == VP_KEY_D)
    {
		if(TileMap[PosY][PosX - 1] != 1)
		{
			PlayerPosition.x--;
		}
	}
}

void
OnKeyDown(vp_keys Key, bool32 IsDown)
{
	if(IsDown)
	{
		if(Console.IsOn)
		{
			if(Key == VP_KEY_MINUS)
            {
                if(vp_is_keydown(VP_KEY_SHIFT))
                    Console.Command[Console.LastChar++] = '_';
                else
                    Console.Command[Console.LastChar++] = '-';
            }
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
			else if(Key == VP_KEY_X)
				vp_toggle_polygons();
			
			else if(Key == VP_KEY_V)
			{
				platform_toggle_vsync(VSyncToggle);
				VSyncToggle = !VSyncToggle;
			}
			else if(Key == VP_KEY_P)
				vp_toggle_particle_update();
		}
		if(vp_is_keydown(VP_KEY_ALT))
		{
			if(Key == VP_KEY_ENTER)
			{
				platform_switch_fullscreen();
			}
		}
		
		HandlePlayerMovement(Key);
		
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
LoadSharable(char *PathToFolder)
{
	reloader Result = {};
	char DLLPath[MAX_PATH] = {};
	strcpy(DLLPath, PathToFolder);
	vstd_strcat(DLLPath, "bin\\hot_reload");
	vstd_strcat(DLLPath, platform_get_sharable_extension());
	
	char TempDLLPath[MAX_PATH] = {};
	strcpy(TempDLLPath, PathToFolder);
	vstd_strcat(TempDLLPath, "bin\\hot_reload_temp");
	vstd_strcat(TempDLLPath, platform_get_sharable_extension());
    
	if(platform_copy_file(DLLPath, TempDLLPath) == FALSE) VP_ERROR("Failed to copy sharable!");
	Result.DLL = platform_load_sharable(TempDLLPath);
	if(Result.DLL.sharable == vp_nullptr)
	{
		VP_ERROR("Failed to load sharable");
		return (reloader){};
	}
	// TODO: Default UpdateAndRender function if this one fails
	Result.UpdateAndRender = (game_update_and_render *)platform_get_function_from_sharable(Result.DLL, "GameUpdateAndRender");
	if(Result.UpdateAndRender == vp_nullptr) VP_ERROR("Failed to get GameUpdateAndRender function!");
	return Result;
}

void
CreateSnowParticles(uint64 *RandomSeed)
{
	
	// 16 : 9
	// 20 : 20
	for(int index = 0; index < 20; ++index)
	{
		v3 ParticlePosition = {};
		ParticlePosition.x = vp_random_from_seed(RandomSeed) % 320;
		ParticlePosition.y = vp_random_from_seed(RandomSeed) % 10000;
		ParticlePosition.z = vp_random_from_seed(RandomSeed) % 180;
        
		ParticlePosition.y = normalize_between(vp_random_from_seed(RandomSeed) % 10000, 0, 10000, 100, 150);
        
		v3 FallingDirection = (v3){0.0f, -1.0f, 0.0f};
		vp_create_particle(1100, ParticlePosition, FallingDirection, 100.0f, (v4){1.0f, 0.9f, 0.9f, 1.0f});
	}
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
		Objects[LastObject++] = vp_load_simple_obj(OutputPathFromSource(EmptyString, PathToFolder, "assets/snow_ball.obj"));
		Objects[LastObject++] = vp_load_simple_obj(OutputPathFromSource(EmptyString, PathToFolder, "assets/cube.obj"));
		Objects[LastObject++] = vp_load_simple_obj(OutputPathFromSource(EmptyString, PathToFolder, "assets/HumanLowPoly.obj"));
	}
    
	bool32 Running = true;
    
    
	int LastReloadCounter = 0;
	int FPSReloadCounter = 15;
	int32 FPS = 0;
	int32 MSPerFrame = 0;
	reloader Game = {};
	Game = LoadSharable(PathToFolder);
    
	Free(TempStorage);
	
	uint64 RandomSeed = vp_random_seed();
    uint32 YOffsetOfMap[TILE_MAP_HEIGHT][TILE_MAP_WIDTH] = {};
	GenerateYOffsetForTileMap(YOffsetOfMap, &RandomSeed);
	
	{
		uint32 tmp[TILE_MAP_HEIGHT][TILE_MAP_WIDTH] =
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
		memcpy(TileMap, tmp, sizeof(tmp));	
	}
    
	//uint32 ParticleStartTimer = platform_get_ms_since_start();
    
	LockCamera();
	int64 PerfFrequency = platform_get_frequency();
	int64 StartCounter = platform_get_perf_counter();
	while(Running)
	{
#if 0
		if(platform_get_ms_since_start() - ParticleStartTimer > 100)
		{
			ParticleStartTimer = platform_get_ms_since_start();
			if(!vp_is_particle_update_off())
				CreateSnowParticles(&RandomSeed);
		}
#endif
		if(LastReloadCounter++ >= 120)
		{
			if(Game.DLL.sharable != vp_nullptr) platform_free_sharable(Game.DLL);
			Game = LoadSharable(PathToFolder);
			LastReloadCounter = 0;
		}
        
		Running = vp_handle_messages();
        
		if(Console.IsStarting)
		{
			HandleConsoleProgression();
		}
		if(Console.IsOn || Console.IsStarting)
		{
			vp_draw_rectangle((m2){0, Console.Position, 10, 5.625f}, (v4){0.2f, 0.7f, 0.4f, 0.6f}, 1);
			vp_draw_rectangle((m2){0, Console.Position + .2f, 10, Console.Position + .45f}, (v4){1.0, 1.0f, 1.0f, 1.0f}, 2);
			if(vp_is_keydown(VP_KEY_ENTER))
			{
				HandleCommand(Console.Command);
				StopConsole();
			}
			vp_draw_text(Console.Command, 0, Console.Position + .25f, (v4){0.0f, 0.0f, 0.0f, 1.0f}, 1.0f, 3);
		}
		
        
		for(int row = 0; row < TILE_MAP_HEIGHT; ++row)
		{
			for(int column = 0; column < TILE_MAP_WIDTH; ++column)
			{
				v4 TileColor = (v4){0.3f, 0.6f, 0.8f, 1.0f};
				float y_bonus = 5;
				if (TileMap[row][column] == 0)
				{
					TileColor = (v4){1.0f, 1.0f, 1.0f, 1.0f};
					y_bonus = 0;
				}
				float column_size = 20;
				float row_size = 20;
				v3 OBJPosition = (v3){column_size * column, (float)YOffsetOfMap[row][column]+y_bonus, row_size * row};
				if(column == PlayerPosition.x && row == PlayerPosition.y)
				{
					vp_object_pushback(Objects[2], (v4){0.4f, 0.7f, 1.0f, 1.0f}, (v3){OBJPosition.x, OBJPosition.y+10, OBJPosition.z}, false, true);					
				}
				vp_object_pushback(Objects[1], TileColor, OBJPosition, true, true);
			}
		}
        
		if(vp_is_keydown(VP_KEY_R))
		{
#if 0
			GenerateYOffsetForTileMap(YOffsetOfMap, &RandomSeed);
#endif
		}
        
        vp_object_pushback(Objects[1], (v4){1.0f, 0.0f, 1.0f, 1.0f}, (v3){10, 50, 10}, true, true);
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
		vp_draw_text(ToDraw, .1f, 5.4, (v4){1.0f, 1.0f, 1.0f, 1.0f}, 0.6f, 0);
		vp_present();
		StartCounter = EndCounter;
	}
	return 0;
}
