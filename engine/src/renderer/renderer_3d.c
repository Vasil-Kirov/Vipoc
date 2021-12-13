#include "renderer/renderer.h"
#include "renderer/math_3d.h"
#include "platform/platform.h"
#include "input.h"

#define swapf(a, b) {float tmp = a; a = b; b = tmp;}

#define push_text_verts(target, num1, num2, color_in)	\
renderer.text_verts[renderer.last_text_index].position = (v4){target.world_position.x##num1, target.world_position.y##num2, 0.0f, 1.0f};	\
renderer.text_verts[renderer.last_text_index].texture = (v4){target.texture_position.x##num1, target.texture_position.y##num2, 0.0f, 0.0f};	\
renderer.text_verts[renderer.last_text_index].color = color_in

#define push_frame_vert(vert, tex, color_in)	\
renderer.frame_verts[renderer.last_frame_vert].position = (v4){vert.x, vert.y, vert.z, 1.0f};	\
renderer.frame_verts[renderer.last_frame_vert].texture = (v4){tex.x, tex.y,	0.0f, 1.0f};	\
renderer.frame_verts[renderer.last_frame_vert++].color = color_in

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
	uint64 allocate_from;
} objects_memory;

typedef struct transformation
{
	m4 rotation;
	m4 translation;
	m4 project;
	m4 world;
	m4 view;
} transformation;

internal objects_memory obj_manage;
internal gl_state renderer;
internal transformation matrices;
internal camera cm;
internal ascii_char ascii_char_map[256];
internal delta_time delta;


#define CHECK_GL_ERROR() if(glGetError() != 0) {VP_ERROR("OPENGL ERROR: %d", glGetError());}


f32 Scale = 0.6f;
// JUMP HERE FOR MAIN CODE
bool32
render_update()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	float current_frame = (float)platform_get_ms_since_start() / 1000.0f;
	delta.value = current_frame - delta.last_frame;
	delta.last_frame = current_frame;


	GLint uniform_location = glGetUniformLocation(renderer.shader_program, "texture1");
	glUniform1i(uniform_location, 0);
	
	v3 forward = v3_scale(cm.look_dir, 8.0f * delta.value);
	if(vp_is_keydown(VP_KEY_W))
		cm.position = v3_sub(cm.position, forward);
	if(vp_is_keydown(VP_KEY_S))
		cm.position = v3_add(cm.position, forward);
	if(vp_is_keydown(VP_KEY_A))
		cm.yaw -= 2.0f * delta.value;
	if(vp_is_keydown(VP_KEY_D))
		cm.yaw += 2.0f * delta.value;


	Scale += .01f;
	matrices.world = identity();
	matrices.project = projection((float)platform_get_width() / (float)platform_get_height(), 90.0f);


	v3 up = {0.0f, 1.0f, 0.0f};
	v3 target = {0.0f, 0.0f, 1.0f};
	m4 camera_y_rotation = y_rotation(cm.yaw);
	m4 camera_x_rotation = x_rotation(cm.pitch);
	m4 camera_rotation = mat4_multiply(camera_x_rotation, camera_y_rotation);
	cm.look_dir = mat4_v3_multiply(camera_rotation, target);
	target = v3_add(cm.position, cm.look_dir);
	
	m4 camera = point_at(cm.position, target, up);
	matrices.view = quick_inverse(camera);
	matrices.view = transpose(matrices.view);

	m4 MVP = mat4_multiply_multiple(3, matrices.project, matrices.view, matrices.world);
	MVP = transpose(MVP);
	set_shader_uniform_mat4("MVP", MVP);
	
	glBindVertexArray(renderer.vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, renderer.vbo);
	glBufferData(GL_ARRAY_BUFFER, renderer.last_frame_vert * sizeof(vertex), renderer.frame_verts, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer.ebo);		
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, renderer.last_index * sizeof(uint32), renderer.indexes, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(0));
	glEnableVertexAttribArray(0);
	
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(sizeof(v4)));
	glEnableVertexAttribArray(1);
	
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(sizeof(v4)*2));
	glEnableVertexAttribArray(2);
	
	glUseProgram(renderer.shader_program);
	glBindVertexArray(renderer.vao);
	glDrawElements(GL_TRIANGLES, renderer.last_index, GL_UNSIGNED_INT, vp_nullptr);
	
	// TEXT RENDERING
	if(renderer.last_text_index > 0)
	{
		
		m4 I = identity();
		set_shader_uniform_mat4("MVP", I);
		
		GLint uniform_location = glGetUniformLocation(renderer.shader_program, "texture1");
		glUniform1i(uniform_location, 1);
		
		glBindVertexArray(renderer.vao);
		
		glBindBuffer(GL_ARRAY_BUFFER, renderer.vbo);
		glBufferData(GL_ARRAY_BUFFER, renderer.last_text_index * sizeof(vertex), renderer.text_verts, GL_STATIC_DRAW);
		
		
		
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(0));
		glEnableVertexAttribArray(0);
		
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(sizeof(v4)));
		glEnableVertexAttribArray(1);
		
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(sizeof(v4)*2));
		glEnableVertexAttribArray(2);
		
		glUseProgram(renderer.shader_program);
		glBindVertexArray(renderer.vao);
		
		glDrawArrays(GL_TRIANGLES, 0, renderer.last_text_index);
		
		renderer.last_text_index = 0;
	}
	
	
	renderer.last_frame_vert = 0;
	renderer.last_index = 0;
	platform_swap_buffers();
	return true;	
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
/*	speed *= delta.value;
	switch(direction)
	{
		case VP_UP:
		{
			cm.position.y += speed;
		}break;
		case VP_DOWN:
		{
			cm.position.y -= speed;
		}break;
		case VP_LEFT:
		{
			cm.position.x -= speed;
		}break;
		case VP_RIGHT:
		{
			cm.position.x += speed;
		}break;
	}
*/
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
vp_object_pushback(int32 index, v4 color, v3 position)
{
	// 0 0 0 - center of the world
	// 10 meters = 2 gl (-1 to 1)
	position = normalize_v3(position, -10000, 10000, -1000, 1000);

	m4 translation = create_mat4(
				1.0f, 0.0f, 0.0f, position.x,
				0.0f, 1.0f, 0.0f, position.y,
				0.0f, 0.0f, 1.0f, position.z,
				0.0f, 0.0f, 0.0f, 1.0f
				);
	translation = transpose(translation);

	if(index < 0 || index >= renderer.last_object) return false;
	obj object = renderer.objects[index];
	v3 empty_tex = (v3){0.0f, 0.0f, 0.0f};

	int index_offset = renderer.last_frame_vert;

	for(int i = 0; object.verts[i].x != OBJ_FIELD_END; ++i)
	{
		v3 vert = object.verts[i];
		vert = mat4_v3_multiply(translation, vert);
		push_frame_vert(vert, empty_tex, color);
	}
	for(int i = 0; object.vert_indexes[i] != INT32_MAX; ++i)
	{		
		renderer.indexes[renderer.last_index++] = object.vert_indexes[i]+index_offset;
	}
	return true;
}

void
vp_draw_text(char *text, float x, float y, v4 color)
{
	int space_from_last_char = 0;
	int vert_from_last_char = 0;
	for(int index = 0; text[index] != '\0'; ++index)
	{
		int char_width = ascii_char_map[(int)text[index]].rect.x2 - ascii_char_map[(int)text[index]].rect.x1;
		int char_height = ascii_char_map[(int)text[index]].rect.y2 - ascii_char_map[(int)text[index]].rect.y1;
		vp_2d_render_target target = {};
		target.world_position.x1 = x + space_from_last_char;
		target.world_position.x2 = target.world_position.x1 + char_width;
		target.world_position.y1 = y - vert_from_last_char;
		target.world_position.y2 = target.world_position.y1 + char_height;
		target.texture_position = ascii_char_map[(int)text[index]].rect;
		
		
		/* Normalize Coordinates */
		target.world_position.x1 = normalize_coordinate(target.world_position.x1, platform_get_width(), 0);
		target.world_position.x2 = normalize_coordinate(target.world_position.x2, platform_get_width(), 0);
		target.world_position.y1 = normalize_coordinate(target.world_position.y1, platform_get_height(), 0);
		target.world_position.y2 = normalize_coordinate(target.world_position.y2, platform_get_height(), 0);
		
		target.texture_position.x1 = normalize_tex_coordinate(target.texture_position.x1, renderer.text_atlas.width, 0);
		target.texture_position.x2 = normalize_tex_coordinate(target.texture_position.x2, renderer.text_atlas.width, 0);
		target.texture_position.y1 = normalize_tex_coordinate(target.texture_position.y1, renderer.text_atlas.height, 0);
		target.texture_position.y2 = normalize_tex_coordinate(target.texture_position.y2, renderer.text_atlas.height, 0);
		
		// Left Triangle
		
		// Bottom left
		push_text_verts(target, 1, 1, color);
		renderer.last_text_index++;
		// Top Left
		push_text_verts(target, 1, 2, color);
		renderer.last_text_index++;
		// Top Right
		push_text_verts(target, 2, 2, color);
		renderer.last_text_index++;
		
		// Right triangle
		
		// Bottom Left
		push_text_verts(target, 1, 1, color);
		renderer.last_text_index++;
		// Top Right
		push_text_verts(target, 2, 2, color);
		renderer.last_text_index++;
		// Bottom Right
		push_text_verts(target, 2, 1, color);
		renderer.last_text_index++;
		
		space_from_last_char += ascii_char_map[(int)text[index]].advance_x;
		if(text[index] == '\n')
		{
			vert_from_last_char += 30;
			space_from_last_char = 0;
		} 
	}
}

// PLACEHOLDER:
void
vp_camera_mouse_callback(double xpos, double ypos)
{
	float delta_x = xpos - cm.mouse_x;
	float delta_y = ypos - cm.mouse_y;
	cm.mouse_x = xpos;
	cm.mouse_y = ypos;
	cm.yaw += (delta_x * delta.value);
	cm.pitch += (delta_y * delta.value);
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
		ToAndPast(at, "xadvance=");
		int advance_x = atoi(at);
		
		result.y1 = renderer.text_atlas.height - result.y1;
		result.y2 = renderer.text_atlas.height - result.y2;
		swapf(result.y1, result.y2);
		
		ascii_char_map[(int)id].rect = result;
		ascii_char_map[(int)id].advance_x = advance_x;
		
		// NOTE: Might be better to have some more concrete way to define the end of a file
		if(id == 126) break;
	}
}




void
obj_allocate_memory_for(void **something, uint64 size_to_allocate)
{
	*something = ((char *)obj_manage.memory.ptr + obj_manage.allocate_from);
	obj_manage.allocate_from += size_to_allocate;
	*(float *)(*something + (size_to_allocate-4)) = OBJ_FIELD_END;
}

#define catof(str) atof((const char *)str)
#define ToAndPastC(at, chr) at = strchr(at, (int)chr); at++


int32
vp_load_simple_obj(char *path)
{
	entire_file obj_file = {};
	vp_memory obj_file_memory = vp_allocate_temp(MB(1));
	obj_file.contents = obj_file_memory.ptr;
	if(!platform_read_entire_file(path, &obj_file)) return -1;
	
	uint64 size_to_allocate = (obj_file.size + KB(10))/6;
	obj new_object = {}; 
	obj_allocate_memory_for((void **)&new_object.verts, size_to_allocate);
	obj_allocate_memory_for((void **)&new_object.vert_indexes, size_to_allocate);


	char *at = (char *)obj_file.contents;
	uint64 size = obj_file.size;
	uint64 last_vert = 0;
	uint64 last_vert_index = 0;
	while(size > 0)
	{
		char *start_at = at;
		if(*at == 'v')
		{
			at+=2;
			new_object.verts[last_vert].x = catof(at);
			ToAndPastC(at, ' ');
			new_object.verts[last_vert].y = catof(at);
			ToAndPastC(at, ' ');
			new_object.verts[last_vert++].z = catof(at);
		}
		else if(*at == 'f')
		{
			at+=2;
			new_object.vert_indexes[last_vert_index++] = catof(at)-1;
			ToAndPastC(at, ' ');
			new_object.vert_indexes[last_vert_index++] = catof(at)-1;
			ToAndPastC(at, ' ');
			new_object.vert_indexes[last_vert_index++] = catof(at)-1;
		}

		ToAndPast(at, "\n");
		size -= (at - start_at); 
	}
	new_object.verts[last_vert].x = OBJ_FIELD_END;
	new_object.vert_indexes[last_vert_index++] = INT32_MAX;

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
	
	vp_memory frame_vert_memory = vp_arena_allocate(KB(60));
	renderer.frame_verts = (vertex *)frame_vert_memory.ptr;
	
	vp_memory text_vert_memory = vp_arena_allocate(KB(60));
	renderer.text_verts = (vertex *)text_vert_memory.ptr;
	
	vp_memory indexes_memory = vp_arena_allocate(KB(60));
	renderer.indexes = (uint32 *)indexes_memory.ptr;


	obj_manage.memory = vp_arena_allocate(MB(100));
	

	
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
	
	const char *fragment_source = fragment_code_file.contents;
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
	vp_arena_free_to_chunk(fragment_shader_source);
	vp_arena_free_to_chunk(vertex_shader_source);
	GenGLBuffs();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_MULTISAMPLE);
	if(glGetError() != 0) VP_ERROR("GL ERROR: %d\n", glGetError());

//	glEnable(GL_CULL_FACE);
//	glFrontFace(GL_CCW);
//	glCullFace(GL_FRONT);
	
	/* CAMERA INITIALIZATION */
//	camera_init(&cm);
	cm.position = (v3){0.0f, 5.0f, 5.0f};
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
