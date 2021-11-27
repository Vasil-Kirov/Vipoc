#pragma once

#include <stdlib.h>
#include <Vipoc.h>

typedef struct vec2
{
    float x;
    float y;
} vec2;

typedef struct allocator
{
    void *Start;
    void *End;
} allocator;

void scale_vector(vec2 *vector, float scaler)
{
    vector->x *= scaler;
    vector->y *= scaler;
}

typedef struct atlas_member
{
    float x;
    float y;
    float w;
    float h;
} atlas_member; 

typedef struct rectangle
{
    int w;
    int h;
} rectangle;

typedef struct hot_render_target
{
    atlas_member Member;
    float X;
    float Y;
    int AtlasWidth;
    int AtlasHeight;
    int LayerID;
} hot_render_target;

#define GAME_UPDATE_AND_RENDER(name) int name(atlas_member *Members, vp_game Game, float PlayerX, float PlayerY, rectangle AtlasSize, vp_render_target *Targets, int XOffset, int YOffset)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);
GAME_UPDATE_AND_RENDER(GameUpdateAndRenderStub)
{
    return 0;
}

typedef struct reloader
{
    HMODULE DLL;
    game_update_and_render *UpdateAndRender;
} reloader;