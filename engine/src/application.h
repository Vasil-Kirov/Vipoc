#ifdef __cplusplus
	extern "C"{
#endif

#pragma once

// This is needed for VP_API to work
#include "include/Core.h"
#include "renderer/math_3d.h"


void application_create(vp_game *game);
void application_run();


VP_API bool32 
vp_handle_messages();

VP_API void 
vp_present();

#ifdef __cplusplus
	}
#endif