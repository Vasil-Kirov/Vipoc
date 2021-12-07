#ifdef __cplusplus
	extern "C"{
#endif

#pragma once
#include "include/defines.h"


typedef struct vp_config
{
    char const *name;
    int x;
    int y;
    int w;
    int h;
} vp_config;


typedef struct vp_game
{
    vp_config config;
    // Game on_resize function
    bool32 (*vp_on_resize)(struct vp_game *game, int w, int h);

} vp_game;


#ifdef __cplusplus
	}
#endif