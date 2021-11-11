#pragma once

#include "renderer/load_gl_black_magic.h"

// vp_memory loads platform.h
#include "vp_memory.h"
#include "log.h"


// NOTE: no padding
#pragma pack(push, 1)
typedef struct vec3
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
} vec3;
#pragma pack(pop)


void RendererInit();

bool32 render_update();