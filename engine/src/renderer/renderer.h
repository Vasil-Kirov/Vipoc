#pragma once

#include "renderer/load_gl_black_magic.h"

// vp_memory loads platform.h
#include "vp_memory.h"
#include "log.h"


typedef float meters;

// NOTE(Vasko): no padding (is it needed?)
#pragma pack(push, 1)
typedef struct vec3
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
} vec3;
#pragma pack(pop)


typedef struct vp_texture
{
	meters x;
	meters y;
	int width;
	int height;
	unsigned char *pixels;
} vp_texture;


VP_API vp_texture
renderer_load_texture(char *path);

VP_API void
renderer_pushback(vp_texture texutre);

void
renderer_buffer_reset();

void
RendererInit();

bool32
render_update();