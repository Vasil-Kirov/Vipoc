#pragma once

#include <math.h>
#include "renderer/load_gl_black_magic.h"

#pragma pack(push, 1)
typedef struct v2
{
	GLfloat x;
	GLfloat y;
} v2;
typedef struct v3
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
} v3;
typedef struct v4
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat w;
} v4;
typedef struct m2
{
	float x1;
	float y1;
	float x2;
	float y2;
} m2;
typedef struct m4
{
    float m[4][4];
} m4;
typedef struct vertex
{
	v4 position;
    v4 texture;
	v4 color; 
} vertex;
#pragma pack(pop)

void
set_shader_uniform_mat4(char *str, m4 mat);

//typedef __declspec(align(32)) v4 m4[4];