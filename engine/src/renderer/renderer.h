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
#define OBJ_FIELD_END -971.245f

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
	
	typedef struct obj
	{
		v3 *verts;
		v3 *texture;
		v3 *norms;
		uint32 *vert_indexes;
		uint32 *texture_indexes;
		uint32 *norms_indexes;
		uint32 last_vert;
		uint32 last_vert_index;
	} obj;
	
	typedef struct atlas
	{
		GLuint texture;
		uint32 width;
		uint32 height;
	} atlas;
	typedef struct vp_2d_render_target
	{
		int layer_id;
		m2 world_position;
		m2 texture_position;
	} vp_2d_render_target;
	
	
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
	
	
	typedef enum vp_direction
	{
		VP_UP=0,
		VP_DOWN=1,
		VP_LEFT=2,
		VP_RIGHT=3
	} vp_direction;
	

VP_API void
	vp_toggle_polygons();

VP_API bool32
	vp_object_pushback(int32 index, v4 color, v3 position, bool32 cachable, bool32 affected_by_light);

VP_API int32
	vp_load_simple_obj(char *path);

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
	vp_draw_text(char *text, float x, float y, v4 color, float scaler, int layer_id);

VP_API void
	vp_load_texture(char *path);

VP_API void
	vp_move_camera(vp_direction direction, f32 speed);
	
VP_API void
	vp_unlock_camera();

VP_API void
	vp_set_directional_light(v3 direction);

// Position = world position, Tex_Location = location in atlas
// values must be between -1 and 1
// function shouldn't be called before vp_load_texture
VP_API void
	vp_render_pushback(vp_2d_render_target target);

VP_API void
	vp_camera_mouse_callback(double xpos, double ypos);

VP_API void
	vp_force_2d(bool32 on);

/* location in meters 0 - 10 = -1 - 1 */
VP_API void
	vp_draw_rectangle(m2 location, v4 color, int layer_id);
	
VP_API void
	vp_lock_camera(float yaw, float pitch, v3 xyz);


void
	vp_update_mouse_pos(double xpos, double ypos);

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

vp_2d_render_target
	normalize_render_target(vp_2d_render_target target);

v3
normalize_v3(v3 target, float minx, float maxx, float from, float to);

void
	set_shader_uniform_f(char *str, float to_set);

void
	set_shader_uniform_vec3(char *str, v3 vector);

v3
	gl_to_meters(v3 target);
	

inline const char *
get_vert_shader_code()
{
	return
	"#version 330 core\n"
	"layout (location = 0) in vec4 \tPos;\n"
	"layout (location = 1) in vec4 \tTex;\n"
	"layout (location = 2) in vec4 \tColor;\n"
	"layout (location = 3) in vec4 \tNormal;\n"
	"layout (location = 4) in vec3 \tWorldPosition;\n"
	"layout (location = 5) in float \tInIsAffectedByLight;\n"
	"\n"
	"out vec2 \tTexCoord;\n"
	"out vec3 \tNormalOut;\n"
	"out vec4 \tColorOut;\n"
	"out vec3 \tFragPos;\n"
	"out float \tIsAffectedByLight;\n"
	"\n"
	"uniform mat4 MVP;\n"
	"\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// @Note: OpenGL wants matrices to be column major\n"
	"\tmat4 Translation = mat4(1.0, \t\t\t\t0.0, \t\t\t\t0.0, \t\t\t\t0,\n"
	"\t\t\t\t\t\t\t0.0, \t\t\t\t1.0, \t\t\t\t0.0, \t\t\t\t0,\n"
	"\t\t\t\t\t\t\t0.0, \t\t\t\t0.0, \t\t\t\t1.0, \t\t\t\t0,\n"
	"\t\t\t\t\t\t\tWorldPosition.x, \tWorldPosition.y, \tWorldPosition.z, \t1.0);\n"
	"\tvec4 position = Translation * Pos;\n"
	"\n"
	"\tgl_Position = MVP * position;\n"
	"\n"
	"\tTexCoord = vec2(Tex.x, Tex.y);\n"
	"\tNormalOut = vec3(Normal.x, Normal.y, Normal.z);\n"
	"\tColorOut = Color;\n"
	"\tIsAffectedByLight = InIsAffectedByLight;\n"
	"\tFragPos = vec3(position.x, position.y, position.z);\n"
	"}"
	"";
}

inline const char *
get_frag_shader_code()
{
	return
	"#version 330 core\n"
	"out vec4 FragColor;\n"
	"\n"
	"in vec2     TexCoord;\n"
	"in vec3     NormalOut;\n"
	"in vec4     ColorOut;\n"
	"in vec3     FragPos;\n"
	"in float    IsAffectedByLight;\n"
	"\n"
	"\n"
	"struct Light\n"
	"{\n"
	"\tvec3 position;\n"
	"\n"
	"\tvec3 ambient;\n"
	"\tvec3 diffuse;\n"
	"\tvec3 specular;\n"
	"};\n"
	"\n"
	"struct Material {\n"
	"\tvec3 ambient;\n"
	"\tvec3 diffuse;\n"
	"\tvec3 specular;\n"
	"\tfloat shininess;\n"
	"}; \n"
	"\n"
	"\n"
	"uniform sampler2D texture1;\n"
	"uniform vec3 view_pos;\n"
	"uniform Light light;\n"
	"uniform Material material;\n"
	"\n"
	"\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tvec3 light_result = vec3(1.0);\n"
	"\t\n"
	"\tif(IsAffectedByLight == 0.0)\n"
	"\t{\n"
	"\t\tlight_result = vec3(1.0, 1.0, 1.0);\n"
	"\t}\n"
	"\telse\n"
	"\t{\n"
	"\t\tvec3 ambient = light.ambient * material.ambient;\n"
	"\n"
	"\t\t// diffuse \n"
	"\t\tvec3 norm = normalize(NormalOut);\n"
	"\t\tvec3 light_dir = normalize(light.position - FragPos);\n"
	"\t\tfloat diff = max(dot(norm, light_dir), 0.0);\n"
	"\t\tvec3 diffuse = light.diffuse * (diff * material.diffuse);\n"
	"\t\t\n"
	"\t\t// specular\n"
	"\t\tvec3 view_dir = normalize(view_pos - FragPos);\n"
	"\t\tvec3 reflect_dir = reflect(-light_dir, norm);\n"
	"\n"
	"\t\tfloat spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);\n"
	"\t\tvec3 specular = light.specular * (spec * material.specular);  \n"
	"\t\t\t\n"
	"\t\tlight_result = ambient + diffuse + specular;\n"
	"\t}\n"
	"\t\n"
	"\tif(TexCoord.x < 0)\n"
	"\t{\n"
	"\t\tFragColor = ColorOut * vec4(light_result, 1.0f);\n"
	"\t}\n"
	"\telse\n"
	"\t{\n"
	"\t\tFragColor = (texture(texture1, TexCoord) * ColorOut) * vec4(light_result, 1.0f);\n"
	"\t}\n"
	"}\n"
	"";
}


#ifdef __cplusplus
}
#endif