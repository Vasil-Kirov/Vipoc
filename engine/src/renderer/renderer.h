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

typedef struct vp_texture
{
	meters x;
	meters y;
	int width;
	int height;
	unsigned char *pixels;
} vp_texture;


VP_API vp_texture
vp_load_texture(char *path);

VP_API void
vp_render_pushback(vp_texture texutre);

void
renderer_buffer_reset();

void
RendererInit();

void
GenGLBuffs();

bool32
render_update();