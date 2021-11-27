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
typedef struct vec4
{
	float x1;
	float y1;
	float x2;
	float y2;
} vec4;
#pragma pack(push, 1)
typedef struct bitmap_header
{
	uint16 FileType;		/* File type, always 4D42h ("BM") */
	uint32 FileSize;		/* Size of the file in bytes */
	uint16 Reserved1;		/* Always 0 */
	uint16 Reserved2;		/* Always 0 */
	uint32 BitmapOffset; /* Starting position of image data in bytes */

	uint32 Size;		 /* Size of this header in bytes */
	int32 Width;		 /* Image width in pixels */
	int32 Height;		 /* Image height in pixels */
	uint16 Planes;		 /* Number of color planes */
	uint16 BitsPerPixel; /* Number of bits per pixel */
} bitmap_header;
#pragma pack(pop)


typedef struct vp_render_target
{
	int layer_id;
	vec4 world_position;
	vec4 texture_position;
} vp_render_target;

typedef struct vp_texture
{
	meters x;
	meters y;
	int width;
	int height;
	unsigned char *pixels;
} vp_texture;



VP_API void
vp_load_texture(char *path, int width, int height);


// Position = world position, Tex_Location = location in atlas
// values must be between -1 and 1
// function shouldn't be called before vp_load_texture
VP_API void
vp_render_pushback(vp_render_target target);

void
renderer_buffer_reset();

void
RendererInit();

void
GenGLBuffs();

bool32
render_update();