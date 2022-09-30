#include "application.h"

#include "platform/platform.h"
// renderer.h loads vp_memory.h, platform.h and defines.h
#include "renderer/renderer.h"
#include "log.h"
#include "renderer/particle.h"
#include "entity.h"

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

internal app_state app;
internal bool32 initialized = FALSE;

// NOTE: this might be better as a pointer
void application_create(vp_game *game)
{
	if (initialized)
	{
		// TODO: Error
	}
	initialized = TRUE;

	memory_init(MB(10));
	app.game = game;
	app.is_running = true;
	app.is_suspended = false;
	app.width = app.game->config.w;
	app.height = app.game->config.h;

	VP_INFO("Initializing platform layer...");
	if (!platform_init(app.game->config, &(app.pstate)))
	{
		// TODO: Fatal Error
	}
	VP_INFO("Initializing renderer...");
	renderer_init();
	VP_INFO("Initializing particle system...");
	particles_init();

	app.game->vp_on_resize(app.game, app.width, app.height);
	//	platform_create_thread(infinitely_calculate_entity_distance_to_camera, vp_nullptr);
	VP_INFO("Everything has been initialized!");
}

void vp_toggle_particle_update()
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
	vp_clear_screen();
	if (!platform_handle_message())
		return FALSE;
	return TRUE;
}

void vp_present()
{
#if 0
	vp_start_debug_timer("Particles", STRHASH("Particles"));
	platform_thread particle_update = {};
	
	if(!is_particle_update_off)
		particle_update   				= platform_create_thread(update_particles, vp_nullptr);
	platform_thread particle_draw 		= platform_create_thread(draw_particles, vp_nullptr);
	platform_thread particle_rewrite  	= platform_create_thread(rewrite_particle_buffer, vp_nullptr);
	
	vp_stop_debug_timer(STRHASH("Particles"));
	
	platform_wait_for_thread(particle_draw);
	platform_wait_for_thread(particle_update);
	platform_wait_for_thread(particle_rewrite);

#endif
	update_entities();
	vp_draw_diagrams();
	render_update();

	//	renderer_buffer_reset();

	vp_free_temp_memory();
	vp_reset_debug_timers();
}

uint64
vp_random_seed()
{
	uint64 seed = 0;
	__asm__(
		"RDRAND %0;"
		: "=r"(seed));

	return seed;
}

uint64
vp_random_from_seed(uint64 *seed)
{
	*seed = *seed * 1103515245 + 12345;
	return (*seed % ((unsigned int)VP_RAND_MAX + 1));
}
