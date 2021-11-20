#include "renderer/renderer.h"


// temp function
void GenGLBuffs();


typedef struct gl_state
{
	GLuint shader_program;
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
} gl_state;

#define VP_MAX_TEXTURES 2048

internal gl_state renderer_state;
internal vp_texture to_render_buffer[VP_MAX_TEXTURES];
internal int last_tex_index;

void
load_bmp_file(char *path, vp_texture *texture);

void RendererInit()
{
	LoadGLExtensions();

	vp_memory vertex_shader_source;
	vertex_shader_source = vp_arena_allocate(MB(1));

	vp_memory fragment_shader_source;
	fragment_shader_source = vp_arena_allocate(MB(1));


	// NOTE: 4096 is the max path on Linux
	char vertex_shader_location[4096];
	char fragment_shader_location[4096];

	platform_get_absolute_path(vertex_shader_location);
	platform_get_absolute_path(fragment_shader_location);

	// TODO: HARD CODE THE SHADERS INTO STRINGS WHEN RELEASING ! ! !
	vstd_strcat(vertex_shader_location, "engine\\src\\renderer\\shader.vert");
	vstd_strcat(fragment_shader_location, "engine\\src\\renderer\\shader.frag");

	platform_file_to_buffer((char *)vertex_shader_source.ptr, vertex_shader_location);
	platform_file_to_buffer((char *)fragment_shader_source.ptr, fragment_shader_location);
	

	GLuint vertex_shader;
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, (const char * const *)&vertex_shader_source.ptr, vp_nullptr);
	glCompileShader(vertex_shader);
	
	{
		int  success;
		char info_log[512];
		glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
		if(!success)
		{
			glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
			VP_ERROR("Failed to compile shader: %s", info_log);
		}
	}

	GLuint fragment_shader;
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, (const char * const *)&fragment_shader_source.ptr, vp_nullptr);
	glCompileShader(fragment_shader);
	
	{
		int  success;
		char info_log[512];
		glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
		if(!success)
		{
			glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
			VP_ERROR("Failed to compile shader: %s", info_log);
		}
	}

	renderer_state.shader_program = glCreateProgram();
	glAttachShader(renderer_state.shader_program, vertex_shader);
	glAttachShader(renderer_state.shader_program, fragment_shader);
	glLinkProgram(renderer_state.shader_program);
	glUseProgram(renderer_state.shader_program);


	{
		int  success;
		char info_log[512];
		glGetProgramiv(renderer_state.shader_program, GL_LINK_STATUS, &success);
		
		if(!success) 
		{
			glGetProgramInfoLog(renderer_state.shader_program, 512, NULL, info_log);
			VP_ERROR("Failed to create the shader program: %S", info_log);
		}
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	

	// First free call is redundant but it feels more correct, it will also work if I ever change the free ( probably won't but still )
	vp_arena_free_to_chunk(fragment_shader_source);
	vp_arena_free_to_chunk(vertex_shader_source);
	GenGLBuffs();
}


vp_texture
renderer_load_texture(char *path)
{
	vp_texture result = {}; 
	if(strstr(path, ".bmp") != vp_nullptr)
	{
		load_bmp_file(path, &result);
	}
	return result;
}

void
renderer_pushback(vp_texture texture)
{
	to_render_buffer[last_tex_index++] = texture;
}

void
renderer_buffer_reset()
{
	memset(to_render_buffer, 0, sizeof(vp_texture) * (last_tex_index - 1));
}

void GenGLBuffs()
{
	vec3 cube[] = 
	{
		// [0] Front Bottom Left 	
		{0-0.35f, 0-0.35f, 0},
		// [1] Front Top left 
		{0-0.35f, 1-0.35f, 0}, 
		// [2] Front Top Right
		{1-0.35f, 1-0.35f, 0},
		// [3] Front Bottom Right
		{1-0.35f, 0-0.35f, 0},
		// [4] Top Left
		{0-0.35f, 1-0.35f, 1},
		// [5] Top Right 
		{1-0.35f, 1-0.35f, 1},
		// [6] Right side Bottom right
		{1-0.35f, 0-0.35f, 1},
		// [7] Bottom Top Left
		{0-0.35f, 0-0.35f, 1}
	};
	GLuint indices[] = 
	{
		0, 1, 2,
		0, 2, 3,
		3, 2, 5,
		3, 5, 6,
		1, 4, 5,
		1, 5, 2,
		7, 4, 5,
		7, 5, 6,
		0, 1, 4,
		0, 4, 7,
		0, 7, 6,
		0, 6, 3
	};
	
	
	glGenVertexArrays(1, &(renderer_state.vao));
	glGenBuffers(1, &(renderer_state.vbo));
	glGenBuffers(1, &(renderer_state.ebo));


	glBindVertexArray(renderer_state.vao);

	glBindBuffer(GL_ARRAY_BUFFER, renderer_state.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_state.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
	glEnableVertexAttribArray(0);
	
	
	glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0); 
}

bool32 render_update()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(renderer_state.shader_program);
	glBindVertexArray(renderer_state.vao);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	platform_swap_buffers();
	return TRUE;
}


void
load_bmp_file(char *path, vp_texture *texture)
{

}