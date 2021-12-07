#include "renderer/renderer.h"
#include "renderer/math_3d.h"
#include "platform/platform.h"


#define swapf(a, b) {float tmp = a; a = b; b = tmp;}

#define push_text_verts(target, num1, num2, color)	\
		renderer.text_verts[renderer.last_text_index].position = (v4){target.world_position.x##num1, target.world_position.y##num2, 0.0f, 1.0f};	\
		renderer.text_verts[renderer.last_text_index].texture = (v4){target.texture_position.x##num1, target.texture_position.y##num2, 0.0f, 0.0f};	\
		renderer.text_verts[renderer.last_text_index].color = color


typedef struct gl_state
{
	GLuint shader_program;
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	atlas texture_atlas;
	atlas text_atlas;
	vertex *frame_verts;
	vertex *text_verts;
	uint32 last_text_index;
	uint32 *indexes;
	uint32 last_index;
} gl_state;

internal gl_state renderer;
internal ascii_char ascii_char_map[256];


#define CHECK_GL_ERROR() if(glGetError() != 0) {VP_ERROR("OPENGL ERROR: %d", glGetError());}

bool32
render_update()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);


	// TEXT RENDERING
	GLint uniform_location = glGetUniformLocation(renderer.shader_program, "texture1");
	CHECK_GL_ERROR();

	glUniform1i(uniform_location, 1);
	CHECK_GL_ERROR();

	glBindVertexArray(renderer.vao);
	CHECK_GL_ERROR();

	glBindBuffer(GL_ARRAY_BUFFER, renderer.vbo);
	CHECK_GL_ERROR();

	glBufferData(GL_ARRAY_BUFFER, renderer.last_text_index * sizeof(vertex), renderer.text_verts, GL_STATIC_DRAW);
	CHECK_GL_ERROR();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer.ebo);
	CHECK_GL_ERROR();
	
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, renderer.last_index * sizeof(uint32), renderer.indexes, GL_STATIC_DRAW);
	CHECK_GL_ERROR();

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(0));
	CHECK_GL_ERROR();

	glEnableVertexAttribArray(0);
	CHECK_GL_ERROR();

	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(sizeof(v4)));
	CHECK_GL_ERROR();

	glEnableVertexAttribArray(1);
	CHECK_GL_ERROR();

	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(sizeof(v4)*2));
	CHECK_GL_ERROR();

	glEnableVertexAttribArray(2);
	CHECK_GL_ERROR();


	glUseProgram(renderer.shader_program);
	CHECK_GL_ERROR();

	glBindVertexArray(renderer.vao);
	CHECK_GL_ERROR();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer.ebo);
	CHECK_GL_ERROR();

	glDrawElements(GL_TRIANGLES, renderer.last_index, GL_UNSIGNED_INT, vp_nullptr);
	CHECK_GL_ERROR();

	renderer.last_index = 0;
	renderer.last_text_index = 0;
	platform_swap_buffers();
	return TRUE;
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

void
vp_draw_text(char *text, float x, float y, v4 color)
{
	int space_from_last_char = 0;
	int vert_from_last_char = 0;
	for(int index = 0; text[index] != '\0'; ++index)
	{
		int char_width = ascii_char_map[(int)text[index]].rect.x2 - ascii_char_map[(int)text[index]].rect.x1;
		int char_height = ascii_char_map[(int)text[index]].rect.y2 - ascii_char_map[(int)text[index]].rect.y1;
		vp_render_target target = {};
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
		renderer.indexes[renderer.last_index++] = renderer.last_text_index++;
		// Top Left
		push_text_verts(target, 1, 2, color);
		renderer.indexes[renderer.last_index++] = renderer.last_text_index++;
		// Top Right
		push_text_verts(target, 2, 2, color);
		renderer.indexes[renderer.last_index++] = renderer.last_text_index++;

		// Right triangle

		// Bottom Left
		push_text_verts(target, 1, 1, color);
		renderer.indexes[renderer.last_index++] = renderer.last_text_index++;
		// Top Right
		push_text_verts(target, 2, 2, color);
		renderer.indexes[renderer.last_index++] = renderer.last_text_index++;
		// Bottom Right
		push_text_verts(target, 2, 1, color);
		renderer.indexes[renderer.last_index++] = renderer.last_text_index++;

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
vp_camera_mouse_callback()
{

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


	// TODO: Change this to temp memory
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
}


void
GenGLBuffs()
{	
	glGenVertexArrays(1, &(renderer.vao));
	glGenBuffers(1, &(renderer.vbo));
	glGenBuffers(1, &(renderer.ebo));
}
