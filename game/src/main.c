#include <entry.h>
#define true 1
#define false 0
#define TRUE true
#define FALSE false


static int SwitchKey = true; 

/* Struct section */
struct res
{
	int w;
	int h;
};


bool32
OnResize(vp_game *game, int w, int h)
{
	return TRUE;
}

bool32
Render(vp_game *game, float delta_time)
{
	return TRUE;
}


bool32
Update(vp_game *game, float delta_time)
{
	if(vp_is_keydown(VP_KEY_UP) && SwitchKey)
	{
		SwitchKey = false;
		VP_INFO("Detected up arrow!");
	}
	if(!vp_is_keydown(VP_KEY_UP))
	{
		SwitchKey = true;
	}
	return TRUE;
}

void
vp_start(vp_game *game)
{
	game->config.name = "vipoc_game";
	game->config.x = 0;
	game->config.y = 0;
	game->config.w = 800;
	game->config.h = 600;
	game->vp_update=Update;
	game->vp_on_resize=OnResize;
	game->vp_render=Render;
	VP_FATAL("FATAL!");
	VP_ERROR("ERROR!");
	VP_WARN("WARN!");
	VP_INFO("INFO!");
}

/*
int main()
{
	render_buffer RenderBuffer;
	
	LARGE_INTEGER PerfFreq;
	QueryPerformanceFrequency(&PerfFreq); 
	int64 PerfCountFreq = PerfFreq.QuadPart;
	
		
	wchar ExePath[MAX_PATH];
	GetExeDirectory(ExePath);

	LARGE_INTEGER LastCounter;
	QueryPerformanceCounter(&LastCounter);
		
	bool32 Running = true;
		
	int XOffset = 0;
	int YOffset = 0;
	while(Running)
	{
		
		
		GameUpdateAndRender(&RenderBuffer, XOffset, YOffset);
		++XOffset;
		++YOffset;
		
		LARGE_INTEGER EndCounter;
		QueryPerformanceCounter(&EndCounter);
		
		int64 CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
		
		// NOTE(Vasko): DEBUG FPS
		int32 FPS = (int32)(PerfCountFreq / CounterElapsed);
		char FPS_DEBUG[1024];
		vstd_sprintf(FPS_DEBUG, "FPS: %d\n", FPS);
		OutputDebugStringA(FPS_DEBUG);
		
		// NOTE(Vasko): DEBUG MS
		int64 MSPerFrame = (1000*CounterElapsed) / PerfCountFreq ;
		char MS_DEBUG[1024];
		vstd_sprintf(MS_DEBUG, "%dms since last frame\n", MSPerFrame);
		OutputDebugStringA(MS_DEBUG);
		
		QueryPerformanceCounter(&EndCounter);
		LastCounter = EndCounter; 
		
	}
}
*/