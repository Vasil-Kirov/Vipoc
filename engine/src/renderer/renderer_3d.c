#include "renderer/renderer.h"
#include "renderer/math_3d.h"
#include "platform/platform.h"


#define swapf(a, b) {float tmp = a; a = b; b = tmp;}

#define push_text_verts(target, num1, num2, color)	\
		renderer.text_verts[renderer.last_text_index].position = (v4){target.world_position.x##num1, target.world_position.y##num2, 0.0f, 1.0f};	\
		renderer.text_verts[renderer.last_text_index].texture = (v4){target.texture_position.x##num1, target.texture_position.y##num2, 0.0f, 0.0f};	\
		renderer.text_verts[renderer.last_text_index].color = color
#define push_frame_vert(vert, tex, color_in)	\
		renderer.frame_verts[renderer.last_frame_vert].position = (v4){vert.x, vert.y, vert.z, 1.0f};	\
		renderer.frame_verts[renderer.last_frame_vert].texture = (v4){tex.x, tex.y,	0.0f, 1.0f};	\
		renderer.frame_verts[renderer.last_frame_vert++].color = color_in;

void
draw_test_cube();

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
} gl_state;

internal gl_state renderer;
internal camera cm;
internal ascii_char ascii_char_map[256];


#define CHECK_GL_ERROR() if(glGetError() != 0) {VP_ERROR("OPENGL ERROR: %d", glGetError());}


f32 Scale = 0.6f;
// JUMP HERE FOR MAIN CODE
bool32
render_update()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);


//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	GLint uniform_location = glGetUniformLocation(renderer.shader_program, "texture1");
	glUniform1i(uniform_location, 0);

	draw_test_cube();
	m4 translation = create_mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,-2.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	m4 rotation_y = y_rotation(Scale);
	m4 rotation_x = x_rotation(Scale);
	Scale += .01f;
	m4 world = mat4_multiply_multiple(3, translation, rotation_x, rotation_y);

	v3 U = {1.0f, 0.0f, 0.0f};
	v3 V = {0.0f, 1.0f, 0.0f};
	v3 N = {0.0f, 0.0f, 1.0f};
	
	m4 project = projection((float)platform_get_width() / (float)platform_get_height(), 90.0f);


	m4 camera_transform = create_mat4(
		U.x,  U.y, 	U.z, -cm.position.x,
		V.x,  V.y, 	V.z, -cm.position.y,
		N.x,  N.y, 	N.z, -cm.position.z,
		0.0f, 0.0f, 0.0, 1.0f	
	);

	m4 MVP = mat4_multiply_multiple(3, project, camera_transform, world);
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
vp_move_camera(v3 by)
{
	cm.position.x += by.x;
	cm.position.y += by.y;
	cm.position.z += by.z;
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
draw_test_cube()
{
	v3 tex = {0.0f, 0.0f, 0.0f};
	v4 r = {1.0f, 0.0f , 0.0f, 1.0f};
	v4 g = {0.0f, 1.0f , 0.0f, 1.0f};
	v4 b = {0.0f, 0.0f , 1.0f, 1.0f};
	v3 pos[8];
	pos[0] = (v3){0.5f, 0.5f, 0.5f };		// 0
	pos[1] = (v3){ -0.5f, 0.5f, -0.5f };	// 1
	pos[2] = (v3){ -0.5f, 0.5f, 0.5f };		// 2
	pos[3] = (v3){ 0.5f, -0.5f, -0.5f };	// 3
	pos[4] = (v3){ -0.5f, -0.5f, -0.5f };	// 4
	pos[5] = (v3){ 0.5f, 0.5f, -0.5f };		// 5
	pos[6] = (v3){ 0.5f, -0.5f, 0.5f };		// 6
	pos[7] = (v3){ -0.5f, -0.5f, 0.5f };	// 7

	int indexes_offset = renderer.last_frame_vert;
	push_frame_vert(pos[0], tex, r);
	push_frame_vert(pos[1], tex, g);
	push_frame_vert(pos[2], tex, b);
	push_frame_vert(pos[3], tex, r);
	push_frame_vert(pos[4], tex, g);
	push_frame_vert(pos[5], tex, b);
	push_frame_vert(pos[6], tex, r);
	push_frame_vert(pos[7], tex, g);
	
	unsigned int Indexes[] = {
		// Front
		4, 5, 1,
		4, 3, 5, 

		// Right 
		5, 3, 0,
		3, 6, 0,

		// Back
		6, 7, 2,
		6, 2, 0,

		// Left
		7, 4, 1, 
		7, 1, 2,

		// Bottom
		7, 6, 4,
		6, 3, 4,

		// Top
		1, 5, 0,
		1, 0, 2
	};
	
	for(int i = 0; i < sizeof(Indexes)/sizeof(unsigned int); ++i)
	{
		renderer.indexes[renderer.last_index++] = Indexes[i] + indexes_offset;
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
	
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_FRONT);
}


void
GenGLBuffs()
{	
	glGenVertexArrays(1, &(renderer.vao));
	glGenBuffers(1, &(renderer.vbo));
	glGenBuffers(1, &(renderer.ebo));
}
