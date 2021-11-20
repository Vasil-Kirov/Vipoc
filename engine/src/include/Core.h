#pragma once
#include "include/defines.h"


typedef struct vp_config
{
    char *name;
    int x;
    int y;
    int w;
    int h;
} vp_config;


typedef struct vp_game
{
    vp_config config;

    // Game initialization function
    bool32 (*vp_create)(struct vp_game *game);

    // Game update function
    bool32 (*vp_update)(struct vp_game *game, float delta_time);

    // Game on_resize function
    bool32 (*vp_on_resize)(struct vp_game *game, int w, int h);

} vp_game;

VP_API void GameUpdateAndRender(render_buffer *Buffer, int XOffset, int YOffset);


