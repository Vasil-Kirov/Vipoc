#pragma once

#ifdef __cplusplus
extern "C"{
#endif
	
#define VP_RAND_MAX 0x7fff
	// This is needed for VP_API to work
#include "include/Core.h"
#include "renderer/math_3d.h"
#include "input.h"
	
	typedef struct vp_config
	{
		char const *name;
		int x;
		int y;
		int w;
		int h;
		void (*vp_on_keyboard_key)(vp_keys key, bool32 is_down);
		void (*vp_on_mouse_button)(vp_buttons button, bool32 is_down, i32 x, i32 y);
	} vp_config;
	
	
	typedef struct vp_game
	{
		vp_config config;
		// Game on_resize function
		bool32 (*vp_on_resize)(struct vp_game *game, int w, int h);
	} vp_game;
	
	
	void application_create(vp_game *game);
	void application_run();
	
	VP_API uint64
		vp_random_seed();
	
	VP_API uint64
		vp_random_from_seed(uint64 *seed);
	
	VP_API bool32 
		vp_handle_messages();
	
	VP_API void 
		vp_present();
	
	VP_API void
		vp_toggle_particle_update();
	
	VP_API bool32
		vp_is_particle_update_off();
	
#ifdef __cplusplus
}
#endif