#include <entry.h>
#define true 1
#define false 0



/* Struct section */
struct res
{
	int w;
	int h;
};




/* Functions  */

#if 0
// DOES NOT WORK !!!
internal void
GetExeDirectory(char *Buffer)
{
	if (!GetModuleFileNameA(NULL, Buffer, MAX_PATH))
	{
		Error("Failed to get directory location, file path might be too long");
	}
	int size = vstd_strlen(Buffer);
	for(int i = size-1; i >= 0; --i)
	{
		if(Buffer[i] == '\\')
		{
			Buffer[i+1] = '\0';
			break;
		}
	}
	
}
#endif

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