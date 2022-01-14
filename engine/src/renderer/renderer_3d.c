#include <stdlib.h>

#include "renderer/renderer.h"
#include "renderer/math_3d.h"
#include "platform/platform.h"
#include "input.h"

#define swapf(a, b) {float tmp = a; a = b; b = tmp;}

#define push_text_verts(target, num1, num2, color_in, pos_in_world, z)																			\
renderer.text_verts[renderer.last_text_index].position = (v4){target.world_position.x##num1, target.world_position.y##num2, z, 1.0f};			\
renderer.text_verts[renderer.last_text_index].texture = (v4){target.texture_position.x##num1, target.texture_position.y##num2, 0.0f, 0.0f};		\
renderer.text_verts[renderer.last_text_index].color = color_in;																					\
renderer.text_verts[renderer.last_text_index++].world_pos = pos_in_world

#define push_frame_vert(vert, tex, color_in, normal_in, pos_in_world, affected_by_light)		\
renderer.frame_verts[renderer.last_frame_vert].position = (v4){vert.x, vert.y, vert.z, 1.0f};	\
renderer.frame_verts[renderer.last_frame_vert].texture = (v4){tex.x, tex.y,	0.0f, 1.0f};		\
renderer.frame_verts[renderer.last_frame_vert].color = color_in;								\
renderer.frame_verts[renderer.last_frame_vert].normal = normal_in;								\
renderer.frame_verts[renderer.last_frame_vert].world_pos = pos_in_world;						\
renderer.frame_verts[renderer.last_frame_vert++].is_affected_by_light = affected_by_light

typedef struct gl_state
{
	GLuint shader_program;
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	atlas texture_atlas;
	atlas text_atlas;
	vertex *frame_verts;
	uint32 last_frame_vert;
	void *frame_verts_end;
	vertex *text_verts;
	uint32 last_text_index;
	uint32 *indexes;
	uint32 last_index;
	obj objects[1024];
	uint32 last_object;
} gl_state;

typedef struct objects_memory
{
	vp_memory memory;
	void *start;
	void *end;
	uint64 allocate_from;
} objects_memory;

typedef struct obj_identifier
{
	v3 position;
	int object_index;
	v4 color;
	float affected_by_light;
} obj_identifier;

typedef struct cached
{
	obj_identifier in[1024];
	int last_cached;
	int last_cached_vert;
	int last_cached_index;
} cached;

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


internal objects_memory obj_manage;
internal gl_state renderer;
internal transformation matrices;
internal camera cm;
internal ascii_char ascii_char_map[256];
internal delta_time delta;
internal vp_light light;
internal cached cache;
internal obj_identifier *to_render_objects;
internal int last_pushed_object;
internal ui_targets *to_render_2d;
internal int last_2d_target;




#define CHECK_GL_ERROR() {int error = glGetError(); if(error != 0) {VP_ERROR("OPENGL ERROR at line %d: %d", __LINE__, error);}}
#define VERTEX_MEMORY MB(100)
#define INDEX_MEMORY MB(10)


void
pushed_objects_to_verts();

void
draw_ui_targets();

#define SECONDS_SINCE_START() (float)platform_get_ms_since_start() / 1000.0f
// JUMP HERE FOR MAIN CODE
bool32
render_update()
{
//	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClearColor(0.0f, 0.8f, 0.9f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFrontFace(GL_CW);
    
	float current_frame = SECONDS_SINCE_START();
	delta.value = current_frame - delta.last_frame;
	delta.last_frame = current_frame;
    
	pushed_objects_to_verts();
	last_pushed_object = 0;
    
    START_DTIMER();
    if(renderer.last_frame_vert * sizeof(vertex) > VERTEX_MEMORY) VP_FATAL("VERTECIES OVERFLOW!");
	if(renderer.last_index * sizeof(unsigned int) > INDEX_MEMORY) VP_FATAL("INDICES OVERFLOW!");
    
    
	GLint uniform_location = glGetUniformLocation(renderer.shader_program, "texture1");
	glUniform1i(uniform_location, 0);
    
	matrices.world = identity();
    
	matrices.project = projection((float)platform_get_width() / (float)platform_get_height(), 90.0f);
    
	matrices.view = calculate_view_matrix(&cm);
    
	m4 MVP = mat4_multiply_multiple(3, matrices.project, matrices.view, matrices.world);
	MVP = transpose(MVP);
    
	set_shader_uniform_mat4("MVP", MVP);
    
    
	/* Light calculations */
#if 0	
	vp_object_pushback(1, (v4){1.0f, 1.0f, 1.0f, 1.0f}, gl_to_meters(light.position), true, true);
#endif
    
    
    
	v3 light_color = {1.0f, 1.0f, 1.0f};
    //	light_color.x = sin(SECONDS_SINCE_START() * 2.0f);
    //	light_color.y = sin(SECONDS_SINCE_START() * 0.7f);
    //	light_color.z = sin(SECONDS_SINCE_START() * 1.3f);
	v3 diffuseColor = v3_v3_scale(light_color, (v3){0.5f, 0.5f, 0.5f}); // decrease the influence
	v3 ambientColor = v3_v3_scale(light_color, (v3){0.2f, 0.2f, 0.2f}); // low influence
    
	set_shader_uniform_vec3("light.position", light.position);
	set_shader_uniform_vec3("light.ambient", ambientColor);
	set_shader_uniform_vec3("light.diffuse", diffuseColor);
	set_shader_uniform_vec3("light.specular", (v3){1.0f, 1.0f, 1.0f});
    
	// material properties
	set_shader_uniform_vec3("material.ambient", (v3){1.0f, 1.0f, 1.0f});
	set_shader_uniform_vec3("material.diffuse", (v3){1.0f, 0.5f, 0.31f});
	set_shader_uniform_vec3("material.specular", (v3){0.5f, 0.5f, 0.5f}); // specular lighting doesn't have full effect on this object's material
	
	set_shader_uniform_f("material.shininess", 32.0f);
	
	set_shader_uniform_vec3("view_pos", cm.position);
    
	
	
	
	int stride = sizeof(v3) + (sizeof(v4) * 4) + sizeof(float);
    
	glBindVertexArray(renderer.vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, renderer.vbo);
	glBufferData(GL_ARRAY_BUFFER, renderer.last_frame_vert * sizeof(vertex), renderer.frame_verts, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer.ebo);		
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, renderer.last_index * sizeof(uint32), renderer.indexes, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, stride, (void*)(0));
	glEnableVertexAttribArray(0);
	
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(v4)));
	glEnableVertexAttribArray(1);
	
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(v4)*2));
	glEnableVertexAttribArray(2);
    
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(v4)*3));
	glEnableVertexAttribArray(3);
    
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(v4)*4));
	glEnableVertexAttribArray(4);
	
	glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(v4)*4 + sizeof(v3)));
	glEnableVertexAttribArray(5);
	
	
	glUseProgram(renderer.shader_program);
	glBindVertexArray(renderer.vao);
	glDrawElements(GL_TRIANGLES, renderer.last_index, GL_UNSIGNED_INT, vp_nullptr);
	
	// TEXT RENDERING
	if(last_2d_target > 0)
	{
		STOP_DTIMER();
		draw_ui_targets();
		START_DTIMER();
		last_2d_target = 0;
        
		/* Undoing the light for Text and UI */
		set_shader_uniform_vec3("light.ambient", (v3){1.0f, 1.0f, 1.0f});
		set_shader_uniform_vec3("light.diffuse", (v3){1.0f, 1.0f, 1.0f});
		set_shader_uniform_vec3("material.ambient", (v3){1.0f, 1.0f, 1.0f});
		set_shader_uniform_vec3("material.diffuse", (v3){1.0f, 1.0f, 1.0f});
		set_shader_uniform_vec3("material.specular", (v3){1.0f, 1.0f, 1.0f}); // specular lighting doesn't have full effect on this object's material
		set_shader_uniform_f("material.shininess", 1.0f);
        
        
		m4 I = identity();
		set_shader_uniform_mat4("MVP", I);
		
		GLint uniform_location = glGetUniformLocation(renderer.shader_program, "texture1");
		glUniform1i(uniform_location, 1);
		
		glBindVertexArray(renderer.vao);
		
		glBindBuffer(GL_ARRAY_BUFFER, renderer.vbo);
		glBufferData(GL_ARRAY_BUFFER, renderer.last_text_index * sizeof(vertex), renderer.text_verts, GL_STATIC_DRAW);
		
		
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, stride, (void*)(0));
		glEnableVertexAttribArray(0);
		
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(v4)));
		glEnableVertexAttribArray(1);
		
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(v4)*2));
		glEnableVertexAttribArray(2);
        
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(v4)*3));
		glEnableVertexAttribArray(3);
		
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(v4)*4));
		glEnableVertexAttribArray(4);
        
		glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(v4)*4 + sizeof(v3)));
		glEnableVertexAttribArray(5);
        
		
		
		/* NOTE: If you add another one it needs to calculate that the last is an m4 not a v4 */
        
        
		glUseProgram(renderer.shader_program);
		glBindVertexArray(renderer.vao);
		
		glDrawArrays(GL_TRIANGLES, 0, renderer.last_text_index);
		
		renderer.last_text_index = 0;
	}
	
	
	renderer.last_frame_vert = cache.last_cached_vert;
	renderer.last_index = cache.last_cached_index;
	STOP_DTIMER();
	platform_swap_buffers();
	
	return true;	
}

bool32
check_for_cache(obj_identifier object)
{
	START_DTIMER();
	for(int index = 0; index < cache.last_cached; ++index)
	{
		if(object.object_index == cache.in[index].object_index)
		{
			if(v3_check_equality(object.position, cache.in[index].position))
			{
				STOP_DTIMER();
				return true;
			}
		}
	}
	STOP_DTIMER();
	return false;
}

void
add_verts_from_obj(obj object, v3 position, v4 color, bool32 affected_by_light)
{
//	START_DTIMER();
	v3 empty_tex = (v3){-1.0f, -1.0f, -1.0f};
    
	int index_offset = renderer.last_frame_vert;
	
	for(int i = 0; i < object.last_vert_index; ++i)
	{
		v3 tmp = object.norms[object.norms_indexes[i]];
        v4 normal = {tmp.x, tmp.y, tmp.z, 1.0f};
        v3 tmp2 = object.verts[object.vert_indexes[i]];
        v4 vert = {tmp2.x, tmp2.y, tmp2.z, 1.0f};
        
#ifdef VIPOC_DEBUG
		if ((void *)(renderer.frame_verts + renderer.last_frame_vert) > renderer.frame_verts_end) { VP_FATAL("FRAME BUFFER OVERFLOW!"); }
#endif
		push_frame_vert(vert, empty_tex, color, normal, position, affected_by_light);
	}
	for(int i = 0; i < object.last_vert_index; ++i)
	{
        
#ifdef VIPOC_DEBUG
		if(renderer.last_index > INDEX_MEMORY * (sizeof(uint32) / sizeof(char))) VP_FATAL("INDEX MEMORY OVERFLOW");
#endif
		renderer.indexes[renderer.last_index++] = i+index_offset;
    }
//	STOP_DTIMER();
}

void
pushed_objects_to_verts()
{
	for(int index = 0; index < last_pushed_object; ++index)
	{
		add_verts_from_obj(renderer.objects[to_render_objects[index].object_index], to_render_objects[index].position, to_render_objects[index].color, to_render_objects[index].affected_by_light);
	}
    
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

bool32
vp_object_pushback(int32 index, v4 color, v3 position, bool32 cachable, bool32 affected_by_light)
{
	START_DTIMER();
	// 0 0 0 - center of the world
	// 10 meters = 2 gl (-1 to 1)
	position = normalize_v3(position, -10000, 10000, -1000, 1000);
    
    
	if(index < 0 || index >= renderer.last_object)
	{
		VP_ERROR("INVALID OBJECT INDEX!");
		return false;
	}
	to_render_objects[last_pushed_object].object_index = index;
	to_render_objects[last_pushed_object].position = position;
	to_render_objects[last_pushed_object].color = color;
	to_render_objects[last_pushed_object++].affected_by_light = affected_by_light;
	
    
	obj_identifier identify = {};
	identify.object_index = index;
	identify.position = position;
	identify.color = color;
	if(cachable && !check_for_cache(identify))
	{
		cache.in[cache.last_cached++] = identify;
		add_verts_from_obj(renderer.objects[index], position, color, affected_by_light);
		cache.last_cached_vert 	= renderer.last_frame_vert;
		cache.last_cached_index	= renderer.last_index;
	}
	return true;
	STOP_DTIMER();
}



int
compare_targets(void const* a, void const* b)
{
	return( ((ui_targets *)a)->target.layer_id - ((ui_targets *)b)->target.layer_id );
}

void
sort_2d_target()
{
	START_DTIMER();
	#if 0
	for(int i = 0; i < last_2d_target; ++i)
	{
		for(int j = 0; j < last_2d_target; ++j)
		{
			if(to_render_2d[i].target.layer_id < to_render_2d[j].target.layer_id)
			{
				ui_targets tmp = to_render_2d[i];
				to_render_2d[i] = to_render_2d[j];
				to_render_2d[j] = tmp;
			}
		}
	}
	#else
		qsort(to_render_2d, last_2d_target, sizeof(ui_targets), compare_targets);
	#endif
	STOP_DTIMER();
}

void
draw_ui_targets()
{
	sort_2d_target();
	
	START_DTIMER();


	v3 I = {0.0f, 0.0f, 0.0f};
    float base_z = 0.0f;
	for(int index = 0; index < last_2d_target; ++index)
	{
        base_z += .000001f;
		vp_2d_render_target location = to_render_2d[index].target;
		v4 color = to_render_2d[index].color;
		
		float z = base_z - (float)(to_render_2d[index].target.layer_id/100.0f);
		// Top Left
		push_text_verts(location, 2, 1, color, I, z);
		// Top Right
		push_text_verts(location, 1, 2, color, I, z);
		// Bottom left
		push_text_verts(location, 1, 1, color, I, z);
        
		// Right triangle
        
		// Top Right
		push_text_verts(location, 2, 2, color, I, z);
		// Bottom Left
		push_text_verts(location, 1, 2, color, I, z);
		// Bottom Right
		push_text_verts(location, 2, 1, color, I, z);
	}
	STOP_DTIMER();
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
	
	to_render_2d[last_2d_target].target = location;
	to_render_2d[last_2d_target++].color = color;

	STOP_DTIMER();
}


void
vp_draw_text(char *text, float x, float y, v4 color, float scaler, int layer_id)
{
	START_DTIMER();

	x = normalize_between(x, 0, 10, 0, platform_get_width());
	y = normalize_between(y, 0, 5.625f, 0, platform_get_height());
	int space_from_last_char = 0;
	int vert_from_last_char = 0;
	for(int index = 0; text[index] != '\0'; ++index)
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
        
		to_render_2d[last_2d_target].target = target;
		to_render_2d[last_2d_target++].color = color;
		
		space_from_last_char += ascii_char_map[(int)text[index]].advance_x * scaler;
		if(text[index] == '\n')
		{
			vert_from_last_char += 30 * scaler;;
			space_from_last_char = 0;
		} 
	}

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




void *
obj_allocate_memory_for(void **something, uint64 size_to_allocate)
{
	*something = ((char *)obj_manage.memory.ptr + obj_manage.allocate_from);
	void *memory_end = (char *)*something + size_to_allocate;
	if(memory_end > obj_manage.end) VP_FATAL("OBJECT MEMORY OVERFLOW");
	obj_manage.allocate_from += size_to_allocate;
	*(float *)(*something + (size_to_allocate-4)) = OBJ_FIELD_END;
	return memory_end;
}

#define catof(str) atof((const char *)str)
#define ToAndPastC(at, chr) at = strchr(at, (int)chr); at++


int32
vp_load_simple_obj(char *path)
{
	entire_file obj_file = {};
	vp_memory obj_file_memory = vp_allocate_temp(platform_get_size_of_file(path));
	obj_file.contents = obj_file_memory.ptr;
	if(!platform_read_entire_file(path, &obj_file)) return -1;
	
	uint64 size_to_allocate = (obj_file.size + KB(10));
	size_to_allocate /= 2;
	obj new_object = {}; 
	void *vert_memory_end 			= obj_allocate_memory_for((void **)&new_object.verts, size_to_allocate);
	void *vert_index_memory_end 	= obj_allocate_memory_for((void **)&new_object.vert_indexes, size_to_allocate*10);
	void *norms_memory_end 			= obj_allocate_memory_for((void **)&new_object.norms, size_to_allocate);
	void *norms_index_memory_end 	= obj_allocate_memory_for((void **)&new_object.norms_indexes, size_to_allocate);
    
    
    
	char *at = (char *)obj_file.contents;
	int64 size = obj_file.size;
	uint64 last_vert = 0;
	uint64 last_vert_index = 0;
	uint64 last_norm = 0;
	uint64 last_norm_index = 0;
	while(size > 0)
	{
		char *start_at = at;
		if(*at == 'v')
		{
			at++;
			if(*at == 'n')
			{
				at+=2;
				new_object.norms[last_norm].x = catof(at);
				ToAndPastC(at, ' ');
				new_object.norms[last_norm].y = catof(at);
				ToAndPastC(at, ' ');
				new_object.norms[last_norm++].z = catof(at);
				if((void *)((char *)new_object.norms + last_norm) > norms_memory_end) VP_FATAL("MEMORY OVERFLOW AT LINE: %d", __LINE__);
			}
			else if(*at == ' ')
			{
				at++;
				new_object.verts[last_vert].x = catof(at);
				ToAndPastC(at, ' ');
				new_object.verts[last_vert].y = catof(at);
				ToAndPastC(at, ' ');
				new_object.verts[last_vert++].z = catof(at);
				if((void *)((char *)new_object.verts + last_vert) > vert_memory_end) VP_FATAL("MEMORY OVERFLOW AT LINE: %d", __LINE__);
			}
		}
		else if(*at == 'f')
		{
			at+=2;
			while(true)
			{
				new_object.vert_indexes[last_vert_index++] = catof(at)-1;
				if((void *)((char *)new_object.vert_indexes + last_vert_index) > vert_index_memory_end) VP_FATAL("MEMORY OVERFLOW AT LINE: %d", __LINE__);
				while(*at >= '0' && *at <= '9') at++;
				if(*at=='/')
				{
                    if(*at+1 != '/') at++;
					ToAndPastC(at, '/');
					while(*at == '/') at++;
					new_object.norms_indexes[last_norm_index++] = catof(at)-1;
					if((void *)((char *)new_object.norms_indexes + last_norm_index) > norms_index_memory_end) VP_FATAL("MEMORY OVERFLOW AT LINE: %d", __LINE__);
				}
				while(*at >= '0' && *at <= '9') at++;
				if(*at == '\n' || *at == VP_NEW_LINE) break;
				at++;
			}
		}
        
		ToAndPast(at, "\n");
		size -= (at - start_at); 
	}
	new_object.verts[last_vert].x = OBJ_FIELD_END;
	new_object.norms[last_norm].x = OBJ_FIELD_END;
	new_object.vert_indexes[last_vert_index] = INT32_MAX;
	new_object.norms_indexes[last_norm_index] = INT32_MAX;
    
	new_object.last_vert = last_vert;
	new_object.last_vert_index = last_vert_index;
    
	renderer.objects[renderer.last_object] = new_object;
	return renderer.last_object++;
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
	
	renderer.frame_verts = (vertex *)platform_allocate_memory_chunk(VERTEX_MEMORY);
	
	renderer.frame_verts_end = (void *)((char *)renderer.frame_verts + VERTEX_MEMORY);
	
	vp_memory text_vert_memory = vp_arena_allocate(MB(1));
	renderer.text_verts = (vertex *)text_vert_memory.ptr;
	
	vp_memory indexes_memory = vp_arena_allocate(INDEX_MEMORY);
	renderer.indexes = (uint32 *)indexes_memory.ptr;
    
	vp_memory targets_2d_memory = vp_arena_allocate(KB(100));
	to_render_2d = targets_2d_memory.ptr;
    
	vp_memory render_objects_memory = vp_arena_allocate(MB(1));
	to_render_objects = render_objects_memory.ptr;
    
	obj_manage.memory = vp_arena_allocate(MB(100));
	obj_manage.end = (void *)((char *)obj_manage.memory.ptr + MB(100));
    
    
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
			VP_FATAL("Failed to compile shader at line %d: %s", __LINE__, info_log);
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
			VP_FATAL("Failed to compile shader at line %d: %s", __LINE__, info_log);
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
    
	GenGLBuffs();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
	glEnable(GL_MULTISAMPLE);
	if(glGetError() != 0) VP_ERROR("GL ERROR: %d\n", glGetError());
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_FRONT);
    
	cm.position = (v3){0.0f, 5.0f, 5.0f};
	cm.is_locked = false;
    
	light.position = (v3){15.0f, 7.780f, 7.614f};
	
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
}
