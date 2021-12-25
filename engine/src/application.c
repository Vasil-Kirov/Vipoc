#include "application.h"


#include "platform/platform.h"
// renderer.h loads vp_memory.h, platform.h and defines.h 
#include "renderer/renderer.h"
#include "log.h"
#include "renderer/particle.h"

internal bool32 is_particle_update_off;


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
	particles_init();

	app.game->vp_on_resize(app.game, app.width, app.height);
}

void
vp_toggle_particle_update()
{
	is_particle_update_off = !is_particle_update_off;
}

bool32
vp_is_particle_update_off()
{
	return is_particle_update_off;
}

bool32
vp_handle_messages()
{

	if(!platform_handle_message()) return FALSE;
	return TRUE;
}

void
vp_present()
{
	platform_thread particle_update = {};
	if(!is_particle_update_off)
		particle_update = platform_create_thread(update_particles, vp_nullptr);
	
	platform_thread particle_draw 		= platform_create_thread(draw_particles, vp_nullptr);
	platform_thread particle_rewrite 	= platform_create_thread(rewrite_particle_buffer, vp_nullptr);
	platform_wait_for_thread(particle_draw);
	if(!render_update())
	{
		VP_ERROR("A failure has occurred with internal rendering!");
	}
	platform_wait_for_thread(particle_update);
	platform_wait_for_thread(particle_rewrite);

//	renderer_buffer_reset();

	vp_free_temp_memory();
}

uint64
vp_random_seed()
{
	uint64 seed = 0;
	__asm__(
	"RDRAND %0;"
	:"=r"(seed)
	);
	
	return seed;
}


uint64
vp_random_from_seed(uint64 *seed)
{
	*seed = *seed * 1103515245 + 12345; 
    return (*seed % ((unsigned int)VP_RAND_MAX + 1)); 
}



