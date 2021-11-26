#include "application.h"


#include "platform/platform.h"
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
	memory_arena = memory_init(MB(10));
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
}

bool32 vp_handle_messages()
{
	if(!platform_handle_message()) return FALSE;
	return TRUE;
}

void vp_present()
{
	if(!render_update())
	{
		VP_ERROR("A failure has occurred with internal rendering!");
	}
//	renderer_buffer_reset();
	vp_free_temp_memory();
}

