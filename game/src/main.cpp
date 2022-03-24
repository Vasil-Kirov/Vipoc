#include "basic.h"
#include <chess.h>
#include <Vipoc.h>
#include <listeners.h>
#include <console.h>

#include <chess.cpp>
#include <listeners.cpp>
#include <console.cpp>


#define Allocate(Size, Storage) Storage.End; Storage.End = (char *)Storage.End + Size
#define Free(Storage) memset(Storage.Start, 0, (char *)Storage.End - (char *)Storage.Start); Storage.End = Storage.Start
#define SIZE(arr) ((int)(sizeof(arr), / sizeof(arr)[0]))

static vp_game Config;
#define TILE_MAP_WIDTH 8
#define TILE_MAP_HEIGHT 8
global_var uint32 Board[TILE_MAP_HEIGHT][TILE_MAP_WIDTH];

void
SetConfigRes(int Width, int Height) { Config.config.w = Width; Config.config.h = Height; }

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
	Config.config.vp_on_keyboard_key = OnKey;
	Config.vp_on_resize = OnResize;
	Config.config.vp_on_mouse_button = OnMouse;
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
    
	// NOTE(Vasko): Since the enum starts from 1 we need to +1 the last piece
	vp_mesh_identifier Cube;
	vp_mesh_identifier Pieces[W_QUEEN + 1] = {};
	{
		char EmptyString[MAX_PATH] = {};
		Cube = vp_load_simple_obj(OutputPathFromSource(EmptyString, PathToFolder, "assets/cube.obj"));
		Pieces[W_PAWN] = vp_load_simple_obj(OutputPathFromSource(EmptyString, PathToFolder, "assets/Pawn.obj"));
		Pieces[W_ROOK] = vp_load_simple_obj(OutputPathFromSource(EmptyString, PathToFolder, "assets/Rook.obj"));
		Pieces[W_KNIGHT] = vp_load_simple_obj(OutputPathFromSource(EmptyString, PathToFolder, "assets/Knight.obj"));
		Pieces[W_BISHOP] = vp_load_simple_obj(OutputPathFromSource(EmptyString, PathToFolder, "assets/Bishop.obj"));
		Pieces[W_KING] = vp_load_simple_obj(OutputPathFromSource(EmptyString, PathToFolder, "assets/King.obj"));
		Pieces[W_QUEEN] = vp_load_simple_obj(OutputPathFromSource(EmptyString, PathToFolder, "assets/Queen.obj"));
	}
    
	
	bool32 Running = true;
    
    
	//	int LastReloadCounter = 0;
	int FPSReloadCounter = 15;
	int32 FPS = 0;
	int32 MSPerFrame = 0;
	
#if 0
	reloader Game = {};
	Game = LoadSharable(PathToFolder);
#endif
	
	Free(TempStorage);
	
	
	uint64 RandomSeed = vp_random_seed();
	{
		uint32 tmp[TILE_MAP_HEIGHT][TILE_MAP_WIDTH] =
		{
			{B_ROOK, B_KNIGHT, B_BISHOP, B_QUEEN, B_KING,  B_BISHOP, B_KNIGHT, B_ROOK},
			{B_PAWN, B_PAWN,   B_PAWN,   B_PAWN,  B_PAWN,  B_PAWN,   B_PAWN,   B_PAWN},
			{NONE,   NONE,     NONE,     NONE,    NONE,    NONE,     NONE,     NONE},
			{NONE,   NONE,     NONE,     NONE,    NONE,    NONE,     NONE,     NONE},
			{NONE,   NONE,     NONE,     NONE,    NONE,    NONE,     NONE,     NONE},
			{NONE,   NONE,     NONE,     NONE,    NONE,    NONE,     NONE,     NONE},
			{W_PAWN, W_PAWN,   W_PAWN,   W_PAWN,  W_PAWN,  W_PAWN,   W_PAWN,   W_PAWN},
			{W_ROOK, W_KNIGHT, W_BISHOP, W_QUEEN, W_KING,  W_BISHOP, W_KNIGHT, W_ROOK}
		};
		memcpy(Board, tmp, sizeof(tmp));	
	}
	
	int PieceIDs[8][8] = {};
	memset(PieceIDs, -1, 8*8);
	
	for(int Y = 0; Y < TILE_MAP_HEIGHT; ++Y)
	{
		for(int X = 0; X < TILE_MAP_WIDTH; ++X)
		{
			u32 Piece = Board[Y][X];
			if(Piece == NONE) continue;
			
			u32 Color = 0xFFFFFFFF;
			if(Piece & 0xF000) Color = 0x2E2E2EFF;
			v3 Position = {X * 20.0f, 10.0f, Y * 20.0f};
			
			PieceIDs[X][Y] = vp_create_entity(Pieces[Piece & 0x000F], Position, 0, Color, (entity_update)vp_nullptr);
		}
	}
	
	
	
	u32 ParticleStartTimer = platform_get_ms_since_start();
    
	LockCamera();
	int64 PerfFrequency = platform_get_frequency();
	int64 StartCounter = platform_get_perf_counter();
	while(Running)
	{
		Running = vp_handle_messages();
        
		if(Console.IsStarting)
		{
			HandleConsoleProgression();
		}
		if(ConsoleIsOn() || Console.IsStarting)
		{
			vp_draw_rectangle((m2){0, Console.Position, 10, 5.625f}, (v4){0.2f, 0.7f, 0.4f, 0.6f}, 1);
			vp_draw_rectangle((m2){0, Console.Position + .2f, 10, Console.Position + .45f}, (v4){1.0, 1.0f, 1.0f, 1.0f}, 2);
			if(vp_is_keydown(VP_KEY_ENTER))
			{
				HandleCommand(Console.Command);
				StopConsole();
			}
			vp_draw_text(Console.Command, 0, Console.Position + .25f, 0x000000FF, 1.0f, 3);
		}
		
		//DrawChessBoard(Cube);
		
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
		vp_draw_text(ToDraw, .1f, 5.4, 0xFFFFFFFF, 0.6f, 0);
		vp_present();
		StartCounter = EndCounter;
	}
	return 0;
}
