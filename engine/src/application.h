#pragma once

// This is needed for VP_API to work
#include "include/Core.h"


void application_create(vp_game *game);
void application_run();


VP_API bool32 
vp_handle_messages();

VP_API void 
vp_present();