#include <stdlib.h>

#include "renderer/renderer.h"
#include "renderer/math_3d.h"
#include "platform/platform.h"
#include "input.h"
#include "entity.h"

#define swapf(a, b) { float reserved_variable_name = a; a = b; b = reserved_variable_name; }
#define swapv3(a, b) { v3 reserved_variable_name = a; a = b; b = reserved_variable_name; }


typedef struct gl_state
{
	GLuint shader_program;
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	GLuint text_vbo;
	atlas texture_atlas;
	atlas text_atlas;
} gl_state;

typedef struct transformation
{
	m4 rotation;
	m4 translation;
	m4 project;
	m4 world;
	m4 view;
} transformation;

typedef struct ui_targets
{
	vp_2d_render_target target;
	v4 color;
} ui_targets;

typedef struct
{
	v3 pos;
	b32 is_valid;
} positions;

internal gl_state renderer;
internal transformation matrices;
internal camera cm;
internal ascii_char ascii_char_map[256];
internal delta_time delta;
internal vp_light light;
internal i32 ebo_used_offset = 0;
internal i32 vbo_used_offset = 0;




#define CHECK_GL_ERROR() {int error = glGetError(); if(error != 0) {VP_ERROR("OPENGL ERROR at line %d: %d", __LINE__, error);}}
#define VERTEX_MEMORY MB(108)
#define INDEX_MEMORY MB(9)
#define QUICK_DRAW_VBO_OFFSET MB(72)    // NOTE(Vasko): Multiple of 36
#define QUICK_DRAW_EBO_OFFSET 7200000   // NOTE(Vasko): Multiple of 36


void
pushed_objects_to_verts();

void
draw_ui_targets();

#define SECONDS_SINCE_START() ((float)platform_get_ms_since_start() / 1000.0f)
// JUMP HERE FOR MAIN CODE
bool32
render_update()
{
	START_DTIMER();
	float current_frame = SECONDS_SINCE_START();
	delta.value = current_frame - delta.last_frame;
	delta.last_frame = current_frame;
    
	if(vbo_used_offset > QUICK_DRAW_VBO_OFFSET)
		VP_FATAL("VERTEX MEMORY OVERFLOW: %d bytes used", vbo_used_offset);
	
	if(ebo_used_offset > QUICK_DRAW_EBO_OFFSET)
		VP_FATAL("INDEX MEMORY OVERFLOW: %d bytes used", ebo_used_offset);
	
	platform_swap_buffers();
	
	STOP_DTIMER();
	return true;	
}

void
vp_clear_screen()
{
	//	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClearColor(0.0f, 0.8f, 0.9f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

m4
calculate_3d_uniforms()
{
	START_DTIMER();
	
	GLint uniform_location = glGetUniformLocation(renderer.shader_program, "texture1");
	glUniform1i(uniform_location, 0);
    
	matrices.world = identity();
    
	matrices.project = projection((float)platform_get_width() / (float)platform_get_height(), 90.0f);
	
	matrices.view = calculate_view_matrix(&cm);
	
	m4 MVP = mat4_multiply_multiple(3, matrices.project, matrices.view, matrices.world);
	
	/* Light calculations */
	v3 light_color = {1.0f, 1.0f, 1.0f};
	
	//light_color.x = sin(SECONDS_SINCE_START() * 2.0f);
	//light_color.y = sin(SECONDS_SINCE_START() * 0.7f);
	//light_color.z = sin(SECONDS_SINCE_START() * 1.3f);
	
	v3 diffuseColor = v3_v3_scale(light_color, (v3){0.5f, 0.5f, 0.5f}); // decrease the influence
	v3 ambientColor = v3_v3_scale(light_color, (v3){0.35f, 0.35f, 0.35f});
    
	set_shader_uniform_vec3("light.position", light.position);
	set_shader_uniform_vec3("light.ambient", ambientColor);
	set_shader_uniform_vec3("light.diffuse", diffuseColor);
	set_shader_uniform_vec3("light.specular", (v3){1.0f, 1.0f, 1.0f});
    
	// material properties
	set_shader_uniform_vec3("material.ambient", (v3){1.0f, 1.0f, 1.0f});
	set_shader_uniform_vec3("material.diffuse", (v3){0.8f, 0.8f, 0.8f});
	set_shader_uniform_vec3("material.specular", (v3){0.5f, 0.5f, 0.5f}); // specular lighting doesn't have full effect on this object's material
	
	set_shader_uniform_f("material.shininess", 32.0f);
	
	set_shader_uniform_vec3("view_pos", cm.position);
	
	int is_3d_location = glGetUniformLocation(renderer.shader_program, "Is3D");
	glUniform1i(is_3d_location, 1);
	
	
	STOP_DTIMER();
	return MVP;
}

void
set_uniforms_for_ui()
{
	
	/* Undoing the light for Text and UI */
	set_shader_uniform_vec3("light.ambient", (v3){1.0f, 1.0f, 1.0f});
	set_shader_uniform_vec3("light.diffuse", (v3){1.0f, 1.0f, 1.0f});
	set_shader_uniform_vec3("material.ambient", (v3){1.0f, 1.0f, 1.0f});
	set_shader_uniform_vec3("material.diffuse", (v3){1.0f, 1.0f, 1.0f});
	set_shader_uniform_vec3("material.specular", (v3){1.0f, 1.0f, 1.0f});
	set_shader_uniform_f("material.shininess", 1.0f);
	
	m4 I = identity();
	set_shader_uniform_mat4("MVP", I);
	
	GLint uniform_location = glGetUniformLocation(renderer.shader_program, "texture2d");
	glUniform1i(uniform_location, 1);
	
	int is_3d_location = glGetUniformLocation(renderer.shader_program, "Is3D");
	glUniform1i(is_3d_location, 0);
}

void
make_draw_call(size_t offset, u32 num_of_elements)
{
	START_DTIMER();
	glBindVertexArray(renderer.vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, renderer.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer.ebo);
	
	u32 stride = sizeof(f32) * ( 3 + 3 + 3 );
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)(0));
	glEnableVertexAttribArray(0);
	
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(v3)));
	glEnableVertexAttribArray(1);
	
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(v3) + sizeof(v2)));
	glEnableVertexAttribArray(2);
    
	
	glUseProgram(renderer.shader_program);
	glDrawElements(GL_TRIANGLES, num_of_elements, GL_UNSIGNED_INT, (void *)(offset));
	STOP_DTIMER();
}

void
vp_lock_camera(float yaw, float pitch, v3 xyz)
{
	cm.position = xyz;
	cm.yaw = yaw;
	cm.pitch = pitch;
	cm.is_locked = true;
}

void
vp_unlock_camera()
{
	cm.is_locked = false;
}

void
vp_update_mouse_pos(double xpos, double ypos)
{
	cm.mouse_x = xpos;
	cm.mouse_y = ypos;
}

void
vp_move_camera(vp_direction direction, f32 speed)
{
	if(cm.is_locked) return;
	
    
    speed *= delta.value;
	v3 no_y_forward = {cm.look_dir.x, 0.0f, cm.look_dir.z};
    no_y_forward = v3_normalize(no_y_forward);
    
    v3 forward = v3_scale(cm.look_dir, speed);
	
    v3 up = (v3){0.0f, 1.0f, 0.0f};
	v3 sideways = v3_cross(no_y_forward, up);
    
	sideways = v3_scale(sideways, speed);
	
    switch(direction)
	{
		case VP_UP:
		{
			cm.position = v3_sub(cm.position, forward);
		}break;
		case VP_DOWN:
		{
			cm.position = v3_add(cm.position, forward);
		}break;
		case VP_LEFT:
		{
			cm.position = v3_add(cm.position, sideways);
		}break;
		case VP_RIGHT:
		{
			cm.position = v3_sub(cm.position, sideways);
		}break;
		default:
		{
			
		}break;
	}
    
}

static bool32 is_poly;

void
vp_toggle_polygons()
{
	if(is_poly)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	is_poly = !is_poly;
}

void
set_shader_uniform_vec3(char *str, v3 vector)
{
	GLint location = glGetUniformLocation(renderer.shader_program, str);
	if(glGetError() != 0)
		CHECK_GL_ERROR();
	glUniform3f(location, vector.x, vector.y, vector.z);
    CHECK_GL_ERROR();
}

void
set_shader_uniform_vec4(char *str, v4 vector)
{
	GLint location = glGetUniformLocation(renderer.shader_program, str);
	if(glGetError() != 0)
		CHECK_GL_ERROR();
	glUniform4f(location, vector.x, vector.y, vector.z, vector.w);
    CHECK_GL_ERROR();
}

void
set_shader_uniform_f(char *str, float to_set)
{
	GLint location = glGetUniformLocation(renderer.shader_program, str);
	if(glGetError() != 0) 
		CHECK_GL_ERROR();
	glUniform1f(location, to_set);
    CHECK_GL_ERROR();
}

void
set_shader_uniform_mat4(char *str, m4 mat)
{
	GLint location = glGetUniformLocation(renderer.shader_program, str);
	if(glGetError() != 0) 
		VP_ERROR("FAILED TO GET UNIFORM LOCATION!");
	glUniformMatrix4fv(location, 1, GL_FALSE, &mat.m[0][0]);
	if(glGetError() != 0) 
		VP_ERROR("FAILED TO FILL UNIFORM WITH MATRIX!");
}

vp_texture
load_bmp_file(char *path)
{
	vp_texture result = {};
	// Should be fine ?
	vp_memory file_memory = vp_allocate_temp(MB(10));
	entire_file file = {};
	file.contents = file_memory.ptr;
	platform_read_entire_file(path, &file);
	if(file.size != 0)
	{
		bitmap_header *header = (bitmap_header *)file.contents;
		
		//													BYTES PER PIXEL NOT BITS
		int pixels_size = header->Width * header->Height * header->BitsPerPixel ;
		vp_memory asset_memory = vp_allocate_asset(pixels_size);
		char *pixels_ptr = file.contents + header->BitmapOffset;
		memcpy(asset_memory.ptr, pixels_ptr, pixels_size);
		result.pixels = asset_memory.ptr;
		result.height = header->Height;
		result.width = header->Width;
	}
	else
	{
		VP_ERROR("Failed to read a fail");
	}
	
	vp_free_temp_memory();
	return result;	
}

float normalize_coordinate(float x, float maxx, float minx)
{
	float Result;
	Result = (2.0f * ((x - minx) / (maxx - minx))) - 1.0f;
	return Result;
}

float normalize_tex_coordinate(float x, float maxx, float minx)
{
	float Result;
	Result = ((x - minx) / (maxx - minx));
	return Result;
}

v3
normalize_v3(v3 target, float minx, float maxx, float from, float to)
{	
	v3 result = {};
	result.x = normalize_between(target.x, minx, maxx, from, to);
	result.y = normalize_between(target.y, minx, maxx, from, to);
	result.z = normalize_between(target.z, minx, maxx, from, to);
	return result;
}

v4
normalize_v4(v4 target, float minx, float maxx, float from, float to)
{	
	v4 result = {};
	result.x = normalize_between(target.x, minx, maxx, from, to);
	result.y = normalize_between(target.y, minx, maxx, from, to);
	result.z = normalize_between(target.z, minx, maxx, from, to);
	result.w = normalize_between(target.w, minx, maxx, from, to);
	return result;
}


#if 0
void
vp_cast_ray(i32 x, i32 y)
{
	f32 xf = normalize_coordinate(x, 0, platform_get_width());
	f32 yf = normalize_coordinate(y, 0, platform_get_height());
	f32 zf = 0.0f;
	glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &zf);
	CHECK_GL_ERROR();
	
	m4 proj = projection((float)platform_get_width() / (float)platform_get_height(), 90.0f);
	m4 view = calculate_view_matrix(&cm);
	
	m4 inv_vp = identity();
	if(!inverse_matrix(mat4_multiply(proj, view), &inv_vp))
	{
		VP_ERROR("Failed to calculate inverse matrix");
		return;
	}
    
	v4 screen_pos = {-xf, yf, zf, 1.0f};
    v4 world_pos = mat4_v4_multiply(transpose(inv_vp), screen_pos);
	
    v3 dir = v3_normalize((v3){world_pos.x, world_pos.y, world_pos.z});
	
	v3 start_pos = {cm.position.x, cm.position.y, cm.position.z};
	int entity = check_if_ray_collides_with_entity(v3_scale(start_pos, 10.0f), dir);
	
	VP_INFO("entity id: %d", entity);
}
#endif


bool32
check_cw_culling(v3 points[3])
{
	v3 edge_a = {};
	v3 edge_b = {};
	edge_a.x = points[1].x - points[0].x;
	edge_a.y = points[1].y - points[0].y;
	edge_a.z = points[1].z - points[0].z;
    
	edge_b.x = points[2].x - points[0].x;
	edge_b.y = points[2].y - points[0].y;
	edge_b.z = points[2].z - points[0].z;
    
	return (edge_a.x * edge_b.y - edge_a.y *edge_b.x);
}

v3
gl_to_meters(v3 target)
{
	float minx = -1000;
	float maxx = 1000;
	float from = -10000;
	float to = 10000;
    
	v3 result = {};
	result.x = normalize_between(target.x, minx, maxx, from, to);
	result.y = normalize_between(target.y, minx, maxx, from, to);
	result.z = normalize_between(target.z, minx, maxx, from, to);
	return result;
}

int
compare_targets(void const* a, void const* b)
{
	return( ((ui_targets *)a)->target.layer_id - ((ui_targets *)b)->target.layer_id );
}

void
sort_2d_target(vp_2d_render_target *targets, int size)
{
	START_DTIMER();
	qsort(targets, size, sizeof(ui_targets), compare_targets);
	STOP_DTIMER();
}



vp_mesh_identifier
render_targets_to_gl_buffers(vp_2d_render_target *targets, int size)
{
	START_DTIMER();
	float base_z = 0.0f;
	
	mesh_vertex verts[size*6];
	memset(verts, 0, size*6*sizeof(mesh_vertex));
	
	int last_vert = 0;
	
	for(int index = 0; index < size; ++index)
	{
        base_z += .000001f;
		vp_2d_render_target target = targets[index];
		
		float z = base_z - (float)(target.layer_id/100.0f);
		m2 pos = target.world_position;
		m2 tex = target.texture_position;
		
		verts[last_vert].position = (v3){pos.x2, pos.y1, z};
		verts[last_vert].texture  = (v3){tex.x2, tex.y1, z};
		last_vert++;
		
		verts[last_vert].position = (v3){pos.x1, pos.y2, z};
		verts[last_vert].texture  = (v3){tex.x1, tex.y2, z};
		last_vert++;
		
		verts[last_vert].position = (v3){pos.x1, pos.y1, z};
		verts[last_vert].texture  = (v3){tex.x1, tex.y1, z};
		last_vert++;
		
		
		verts[last_vert].position = (v3){pos.x2, pos.y2, z};
		verts[last_vert].texture  = (v3){tex.x2, tex.y2, z};
		last_vert++;
		
		verts[last_vert].position = (v3){pos.x1, pos.y2, z};
		verts[last_vert].texture  = (v3){tex.x1, tex.y2, z};
		last_vert++;
		
		verts[last_vert].position = (v3){pos.x2, pos.y1, z};
		verts[last_vert].texture  = (v3){tex.x2, tex.y1, z};
		last_vert++;
	}
	

	
	u32 indexes[last_vert];
	for(i32 i = 0; i < last_vert; ++i)
	{
		indexes[i] = i + (QUICK_DRAW_VBO_OFFSET / sizeof(mesh_vertex));
	}
	
	
	glBindBuffer(GL_ARRAY_BUFFER, renderer.vbo);
	glBufferSubData(GL_ARRAY_BUFFER, QUICK_DRAW_VBO_OFFSET, sizeof(mesh_vertex) * last_vert, verts);
	assert(glGetError() == 0);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer.ebo);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, QUICK_DRAW_EBO_OFFSET, sizeof(u32) * last_vert, indexes);
	assert(glGetError() == 0);
	
	STOP_DTIMER();
	return (vp_mesh_identifier){.element_count = last_vert, .ebo_offset = QUICK_DRAW_EBO_OFFSET};
}

/* location in meters 0 - 10 = -1 - 1 */ 
void
vp_draw_rectangle(m2 position, v4 color, int layer_id)
{
	
	START_DTIMER();
	
	vp_2d_render_target location = {};
	location.texture_position = (m2){-1.0f, -1.0f, -1.0f, -1.0f};
	location.layer_id = layer_id;
	location.world_position = position;
    
	location.world_position.x1 = normalize_between(location.world_position.x1, 0, 10.0f,  -1, 1);
	location.world_position.x2 = normalize_between(location.world_position.x2, 0, 10.0f,  -1, 1);
    
	location.world_position.y1 = normalize_between(location.world_position.y1, 0, 5.625f, -1, 1);
	location.world_position.y2 = normalize_between(location.world_position.y2, 0, 5.625f, -1, 1);
	
	vp_mesh_identifier info = render_targets_to_gl_buffers(&location, 1);
	set_uniforms_for_ui();
	set_shader_uniform_vec4("Color", color);
	make_draw_call(info.ebo_offset, info.element_count);
	STOP_DTIMER();

}

void
vp_draw_text(char *text, float x, float y, u32 in_color, float scaler, int layer_id)
{
	
	START_DTIMER();
	
	v4 color = {};
	
	// RGBA
	color.x = normalize_between((in_color >> 24) & 0xFF, 0, 255, -1, 1);
	color.y = normalize_between((in_color >> 16) & 0xFF, 0, 255, -1, 1);
	color.z = normalize_between((in_color >> 8)  & 0xFF, 0, 255, -1, 1);
	color.w = normalize_between((in_color >> 0)  & 0xFF, 0, 255, -1, 1);
	
	x = normalize_between(x, 0, 10, 0, platform_get_width());
	y = normalize_between(y, 0, 5.625f, 0, platform_get_height());
	int space_from_last_char = 0;
	int vert_from_last_char = 0;
	
	int size = vstd_strlen(text) + 1;
	vp_2d_render_target character_verts[size];
	int last_character = 0;
	
	for(int index = 0; index < size; ++index)
	{
		int char_width = ascii_char_map[(int)text[index]].rect.x2 - ascii_char_map[(int)text[index]].rect.x1;
		int char_height = ascii_char_map[(int)text[index]].rect.y2 - ascii_char_map[(int)text[index]].rect.y1;
        
        int x_offset = ascii_char_map[(int)text[index]].offset_x;
        int y_offset = ascii_char_map[(int)text[index]].offset_y;
        
        
		char_width *= scaler;
		char_height *= scaler;
		
        vp_2d_render_target target = {};
		
        
        target.world_position.x1 = x + space_from_last_char;
		target.world_position.x2 = target.world_position.x1 + char_width;
		target.world_position.y1 = y - vert_from_last_char;
		target.world_position.y2 = target.world_position.y1 + char_height;
		
        target.texture_position = ascii_char_map[(int)text[index]].rect;
		target.layer_id = layer_id;
		
        target.world_position.x1 += x_offset*scaler;
        target.world_position.x2 += x_offset*scaler;
        
        target.world_position.y1 += 25*scaler - (char_height+y_offset*scaler);
        target.world_position.y2 += 25*scaler - (char_height+y_offset*scaler);
		
		/* Normalize Coordinates */
		target.world_position.x1 = normalize_coordinate(target.world_position.x1, platform_get_width(), 0);
		target.world_position.x2 = normalize_coordinate(target.world_position.x2, platform_get_width(), 0);
		target.world_position.y1 = normalize_coordinate(target.world_position.y1, platform_get_height(), 0);
		target.world_position.y2 = normalize_coordinate(target.world_position.y2, platform_get_height(), 0);
		
		target.texture_position.x1 = normalize_tex_coordinate(target.texture_position.x1, renderer.text_atlas.width, 0);
		target.texture_position.x2 = normalize_tex_coordinate(target.texture_position.x2, renderer.text_atlas.width, 0);
		target.texture_position.y1 = normalize_tex_coordinate(target.texture_position.y1, renderer.text_atlas.height, 0);
		target.texture_position.y2 = normalize_tex_coordinate(target.texture_position.y2, renderer.text_atlas.height, 0);
		
        
		if(is_poly)
		{
			target.texture_position.x1 = -1;
			target.texture_position.y1 = -1;
			target.texture_position.x2 = -1;
			target.texture_position.y2 = -1;
		}
        
		character_verts[last_character++] = target;
		
		space_from_last_char += ascii_char_map[(int)text[index]].advance_x * scaler;
		if(text[index] == '\n')
		{
			vert_from_last_char += 30 * scaler;;
			space_from_last_char = 0;
		} 
	}
	
	vp_mesh_identifier info = render_targets_to_gl_buffers(character_verts, last_character);
	set_uniforms_for_ui();
	set_shader_uniform_vec4("Color", color);
	make_draw_call(info.ebo_offset, info.element_count);
	
	STOP_DTIMER();
}

// PLACEHOLDER:
void
vp_camera_mouse_callback(double xpos, double ypos)
{
	if(cm.is_locked) return;
	float delta_x = xpos - cm.mouse_x;
	float delta_y = ypos - cm.mouse_y;
	cm.mouse_x = xpos;
	cm.mouse_y = ypos;
	cm.yaw += (delta_x * .02f);
	cm.pitch += (delta_y * .02f);
    if (cm.pitch > 1.5f) cm.pitch = 1.5f;
    if (cm.pitch < -1.5f) cm.pitch = -1.5f;
    
}


void
vp_load_text_atlas(char *path)
{
	vp_texture result = {};
	if (strstr(path, ".bmp") != vp_nullptr)
	{
		result = load_bmp_file(path);
	}
	
	renderer.text_atlas.width = result.width;
	renderer.text_atlas.height = result.height;
	
	glGenTextures(1, &renderer.text_atlas.texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, renderer.text_atlas.texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, result.width, result.height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, result.pixels);
	CHECK_GL_ERROR();
}


#define ToAndPast(at, str) at = strstr(at, str); at+=sizeof(str)-1

void
vp_parse_font_fnt(entire_file file)
{
	if(file.contents == vp_nullptr)
	{
		VP_ERROR("Passing invalid file to vp_parse_font_fnt!");
		return;
	}
	const char *at = (const char *)file.contents;
	ToAndPast(at, "scaleW=");
	renderer.text_atlas.width = atoi(at);
	ToAndPast(at, "scaleH=");
	renderer.text_atlas.height = atoi(at);
	
	while(true)
	{
		m2 result = {};
		ToAndPast(at, "char id=");
		char id = atoi(at);
		ToAndPast(at, "x=");
		result.x1 = atoi(at);
		ToAndPast(at, "y=");
		result.y1 = atoi(at);
		ToAndPast(at, "width=");
		result.x2 = result.x1 + atoi(at);
		ToAndPast(at, "height=");
        result.y2 = result.y1 + atoi(at);
        
        
        ToAndPast(at, "xoffset=");
		int x_offset = atoi(at);
		ToAndPast(at, "yoffset=");
		int y_offset = atoi(at);
        
		ToAndPast(at, "xadvance=");
		int advance_x = atoi(at);
		
		result.y1 = renderer.text_atlas.height - result.y1;
		result.y2 = renderer.text_atlas.height - result.y2;
		swapf(result.y1, result.y2);
		
		ascii_char_map[(int)id].rect = result;
		ascii_char_map[(int)id].advance_x = advance_x;
		ascii_char_map[(int)id].offset_x = x_offset;
		ascii_char_map[(int)id].offset_y = y_offset;
		
		// NOTE: Might be better to have some more concrete way to define the end of a file
		if(id == 126) break;
	}
}





#define catof(str) atof((const char *)str)
#define ToAndPastC(at, chr) at = strchr(at, (int)chr); at++



#define MAGIC_FLOAT -47213.4576
vp_mesh_identifier
vp_load_simple_obj(char *path)
{
	u32 first_vert_offset = vbo_used_offset / sizeof(mesh_vertex);
	
	entire_file obj_file = {};
	obj_file.contents = vp_allocate_temp(platform_get_size_of_file(path)).ptr;
	if(!platform_read_entire_file(path, &obj_file)) VP_FATAL("MISSING FILE %s", path);
	int size = obj_file.size;
	
#define MAX_VERTS (obj_file.size / sizeof(v3))
#define MAX_INDEXES MAX_VERTS
#define MAX_NORM_INDEXES MAX_INDEXES
#define MAX_NORMS MAX_VERTS
	
	// TODO: Change this when implementing dynamic arrays
	v3 *mesh_verts = vp_allocate_temp(size).ptr;
	
	u32 last_vert = 0;
	u32 last_norm = 0;
	
	u32 *indexes = vp_allocate_temp(MAX_INDEXES * sizeof(u32)).ptr;
	u32 last_index = 0;
	
	u32 *norm_indexes = vp_allocate_temp(MAX_NORM_INDEXES * sizeof(u32)).ptr;
	u32 last_norm_index = 0;
	
	v3 *normals = vp_allocate_temp(MAX_NORMS * sizeof(f32)).ptr;
	char *at = (char *)obj_file.contents;
	while(size > 0)
	{
		char *start_at = at;
		if(*at == 'v')
		{
			at++;
			if(*at == 'n')
			{
				at+=2;
				normals[last_norm].x = catof(at);
				ToAndPastC(at, ' ');
				normals[last_norm].y = catof(at);
				ToAndPastC(at, ' ');
				normals[last_norm].z = catof(at);
				last_norm++;
			}
			else if(*at == ' ')
			{
				at++;
				mesh_verts[last_vert].x = catof(at);
				ToAndPastC(at, ' ');
				mesh_verts[last_vert].y = catof(at);
				ToAndPastC(at, ' ');
				mesh_verts[last_vert].z = catof(at);
				last_vert++;	
			}
		}
		else if(*at == 'f')
		{
			at+=2;
			while(true)
			{
				indexes[last_index++] = catof(at) - 1;
				
				while(*at >= '0' && *at <= '9') at++;
				
				if(*at=='/')
				{
                    if(*at+1 != '/') at++;
					ToAndPastC(at, '/');
					while(*at == '/') at++;
					norm_indexes[last_norm_index++] = catof(at) - 1;
				}
				while(*at >= '0' && *at <= '9') at++;
				if(*at == '\n' || *at == VP_NEW_LINE[0]) break;
				at++;
			}
		}
        
		ToAndPast(at, "\n");
		size -= (at - start_at); 
	}
	
	mesh_vertex *verts = vp_allocate_temp(last_index * sizeof(mesh_vertex)).ptr;
	memset(verts, 0, last_index * sizeof(mesh_vertex));
	for(i32 i = 0; i < last_index; i++)
	{
		v3 normal = normals[norm_indexes[i]];	
		v3 vert = mesh_verts[indexes[i]];
		
		// TODO(Vasko): texture support
		v3 no_texture = {-1, -1, -1};
		
		verts[i].position = vert;
		verts[i].normal = normal;
		verts[i].texture = no_texture;
	}
	
	u32 *fin_indexes = vp_allocate_temp(last_index * sizeof(u32)).ptr;
	memset(fin_indexes, 0, last_index * sizeof(u32));
	for(i32 i = 0; i < last_index; ++i)
	{
		fin_indexes[i] = i + first_vert_offset;
	}
	
	if(last_index > MAX_INDEXES) { VP_ERROR("Indexes in obj file exceed maximum"); assert(false); }
	if(last_norm_index > MAX_NORM_INDEXES) { VP_ERROR("Norm indexes in obj file exceed maximum"); assert(false); }
	if(last_norm > MAX_NORMS) { VP_ERROR("Norms in obj file exceed maximum"); assert(false); }
	if(last_vert > MAX_VERTS) { VP_ERROR("Verts in obj file exceed maximum"); assert(false); }
	
	glBindBuffer(GL_ARRAY_BUFFER, renderer.vbo);
	glBufferSubData(GL_ARRAY_BUFFER, vbo_used_offset, sizeof(mesh_vertex) * last_index, verts);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer.ebo);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, ebo_used_offset, sizeof(u32) * last_index, fin_indexes);
	
	vp_mesh_identifier result = {.element_count = last_index, .ebo_offset = ebo_used_offset};
	
	ebo_used_offset += last_index * sizeof(u32);
	vbo_used_offset += last_index * sizeof(mesh_vertex);
	
	vp_free_temp_memory();
	return result;
}

void
vp_load_texture(char *path)
{
	vp_texture result = {};
	if(strstr(path, ".bmp") != vp_nullptr)
	{
		result = load_bmp_file(path);
	}
	
	renderer.texture_atlas.width = result.width;
	renderer.texture_atlas.height = result.height;
	glGenTextures(1, &renderer.texture_atlas.texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderer.texture_atlas.texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, result.width, result.height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, result.pixels);
	
}


void RendererInit()
{
	LoadGLExtensions();
	GenGLBuffs();
	
	GLenum usage = GL_STATIC_DRAW;
	
	glBindBuffer(GL_ARRAY_BUFFER, renderer.vbo);
	glBufferData(GL_ARRAY_BUFFER, VERTEX_MEMORY, vp_nullptr, usage);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, INDEX_MEMORY, vp_nullptr, usage);
	
    
#if VIPOC_DEBUG	
	vp_memory vertex_shader_source;
	vertex_shader_source = vp_allocate_temp(MB(1));
	
	vp_memory fragment_shader_source;
	fragment_shader_source = vp_allocate_temp(MB(1));
	
	
	// NOTE: 4096 is the max path on Linux
	char vertex_shader_location[4096];
	char fragment_shader_location[4096];
	
	platform_get_absolute_path(vertex_shader_location);
	platform_get_absolute_path(fragment_shader_location);
	
	// TODO: HARD CODE THE SHADERS INTO STRINGS WHEN RELEASING ! ! !
	vstd_strcat(vertex_shader_location, "engine\\src\\renderer\\shader.vert");
	vstd_strcat(fragment_shader_location, "engine\\src\\renderer\\shader.frag");
	
	entire_file vertex_code_file = {};
	vertex_code_file.contents = vertex_shader_source.ptr;
	
	entire_file fragment_code_file = {};
	fragment_code_file.contents = fragment_shader_source.ptr;
	
	platform_read_entire_file(vertex_shader_location, &vertex_code_file);
	platform_read_entire_file(fragment_shader_location, &fragment_code_file);
	
	
	const char *vertex_source = vertex_code_file.contents;
#else
	const char *vertex_source = get_vert_shader_code();
#endif
	GLuint vertex_shader;
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, (const char * const *)&vertex_source, vp_nullptr);
	glCompileShader(vertex_shader);
	
	{
		int  success;
		char info_log[512];
		glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
		if(!success)
		{
			glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
			VP_FATAL("Failed to compile vertex shader:\n%s", info_log);
		}
	}
#if VIPOC_DEBUG
	const char *fragment_source = fragment_code_file.contents;
#else
	const char *fragment_source = get_frag_shader_code();
#endif
	GLuint fragment_shader;
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, (const char * const *)&fragment_source, vp_nullptr);
	glCompileShader(fragment_shader);
	
	{
		int  success;
		char info_log[512];
		glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
		if(!success)
		{
			glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
			VP_FATAL("Failed to compile fragment shader:\n%s", info_log);
		}
	}
	
	renderer.shader_program = glCreateProgram();
	glAttachShader(renderer.shader_program, vertex_shader);
	glAttachShader(renderer.shader_program, fragment_shader);
	glLinkProgram(renderer.shader_program);
	glUseProgram(renderer.shader_program);
	
	
	{
		int  success;
		char info_log[512];
		glGetProgramiv(renderer.shader_program, GL_LINK_STATUS, &success);
		
		if(!success) 
		{
			glGetProgramInfoLog(renderer.shader_program, 512, NULL, info_log);
			VP_FATAL("Failed to create the shader program: %s", info_log);
		}
	}
	
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	
	
	// First free call is redundant but it feels more correct, it will also work if I ever change the free ( probably won't but still )
    
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
	glEnable(GL_MULTISAMPLE);
	if(glGetError() != 0) VP_ERROR("GL ERROR: %d\n", glGetError());
	
	glEnable(GL_DEPTH_TEST);
	
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	
	cm.position = (v3){0.0f, 5.0f, 5.0f};
	cm.is_locked = false;
    
	light.position = (v3){0.0f, 5.000f, 0.0f};
	
}

float
vp_get_dtime()
{
	return delta.value;
}


void
GenGLBuffs()
{	
	glGenVertexArrays(1, &(renderer.vao));
	glGenBuffers(1, &(renderer.vbo));
	glGenBuffers(1, &(renderer.ebo));
	glGenBuffers(1, &(renderer.text_vbo));
	
	
	glBindBuffer(GL_ARRAY_BUFFER, renderer.vbo);
	glBufferData(GL_ARRAY_BUFFER, VERTEX_MEMORY, vp_nullptr, GL_STREAM_DRAW );
	CHECK_GL_ERROR();
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, INDEX_MEMORY, vp_nullptr, GL_STREAM_DRAW );
	CHECK_GL_ERROR();
}
