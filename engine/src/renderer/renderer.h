#pragma once
#ifdef __cplusplus
	extern "C"{
#endif



#include "renderer/load_gl_black_magic.h"

// vp_memory loads platform.h
#include "vp_memory.h"
#include "log.h"

#include "renderer/math_3d.h"


typedef float meters;

// NOTE(Vasko): no padding (is it needed?)

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


typedef struct atlas
{
	GLuint texture;
	uint32 width;
	uint32 height;
} atlas;
typedef struct vp_render_target
{
	int layer_id;
	m2 world_position;
	m2 texture_position;
} vp_render_target;


typedef struct vp_texture
{
	meters x;
	meters y;
	int width;
	int height;
	unsigned char *pixels;
} vp_texture;

typedef struct ascii_char
{
	m2 rect;
	int advance_x;
	int offset_y;
} ascii_char;

typedef struct delta_time
{
	float value;
	float last_frame;
} delta_time;

typedef struct camera
{
	v3 position;
	v3 U;
	v3 V;
	v3 N;
} camera;

typedef enum vp_direction
{
	VP_UP=0,
	VP_DOWN=1,
	VP_LEFT=2,
	VP_RIGHT=3
} vp_direction;

VP_API float
vp_get_dtime();

VP_API void
vp_draw_cube(v3 position, v4 color);

VP_API void
vp_parse_font_fnt(entire_file file);

VP_API void
vp_parse_font_xml(entire_file file);

VP_API void
vp_load_text_atlas(char *path);

VP_API void
vp_draw_text(char *text, float x, float y, v4 color);

VP_API void
vp_load_texture(char *path);

VP_API void
vp_move_camera(v3 by);

// Position = world position, Tex_Location = location in atlas
// values must be between -1 and 1
// function shouldn't be called before vp_load_texture
VP_API void
vp_render_pushback(vp_render_target target);

VP_API void
vp_camera_mouse_callback(double xpos, double ypos);

VP_API void
vp_force_2d(bool32 on);

VP_API void
vp_draw_rectangle(m2 location, v4 color, int layer_id);

void
renderer_buffer_reset();

void
RendererInit();

void
GenGLBuffs();

bool32
render_update();

float
meters_to_gl(float num, bool32 is_horizontal);

vp_render_target
normalize_render_target(vp_render_target target);


#ifdef __cplusplus
	}
#endif