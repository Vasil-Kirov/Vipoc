#include "application.h"


// renderer.h loads vp_memory.h, platform.h and defines.h 
#include "renderer/renderer.h"
#include "log.h"



typedef struct app_state
{
	vp_game *game;
	bool32 is_running;
	bool32 is_suspended;
	platform_state pstate;
	int width;
	int height;
} app_state;


internal vp_arena *memory_arena;
internal app_state app;
internal bool32 initialized = FALSE;

// NOTE: this might be better as a pointer
void application_create(vp_game *game)
{
	if(initialized) 
	{
		// TODO: Error 
	}
	initialized = TRUE;
	
	// very random amount of memory
	memory_arena = memory_init(MB(4));
	app.game = game;
	app.is_running = true;
	app.is_suspended = false;
	app.width = app.game->config.w;
	app.height = app.game->config.h;
	if(!platform_init(app.game->config, &(app.pstate) ))
	{
		// TODO: Fatal Error
	}
	RendererInit();

	app.game->vp_on_resize(app.game, app.width, app.height);


	application_run(app.game);
}


void application_run(vp_game *game)
{
	while(app.is_running)
	{
		if(!platform_handle_message())
		{
			app.is_running = false;
		}

		if(!game->vp_update(game, 0.0f))
		{
			// TODO: Error
			VP_FATAL("vp_update has failed!");
			app.is_running = false;
			break;
		}
		if(!render_update())
		{
			VP_ERROR("A failure has occurred with internal rendering!");
		}
		renderer_buffer_reset();

	}
}

