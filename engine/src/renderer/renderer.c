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
	if(last_tex_index == 0) return;
	memset(to_render_buffer, 0, sizeof(vp_texture) * (last_tex_index - 1));
}

void
GenGLBuffs()
{	
	glGenVertexArrays(1, &(renderer_state.vao));
	glGenBuffers(1, &(renderer_state.vbo));
	glGenBuffers(1, &(renderer_state.ebo));
}

void
process_texture(vp_texture texture, vec3 *verts, int *vert_index, unsigned int *texture_array, int *texture_index);

bool32 render_update()
{
	vp_memory vert_memory = vp_allocate_temp(KB(60));
	vec3 *verts = vert_memory.ptr;
	int vert_index = 0;
	unsigned int textures[last_tex_index];
	int texture_index;
	glGenTextures(last_tex_index, textures);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	for(int index = 0; index < last_tex_index; ++index)
	{
		process_texture(to_render_buffer[index], verts, &vert_index, textures, &texture_index);
	}

	glBindVertexArray(renderer_state.vao);

	glBindBuffer(GL_ARRAY_BUFFER, renderer_state.vbo);
	glBufferData(GL_ARRAY_BUFFER, vert_index * sizeof(vec3), verts, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vec3), (void *)0);
	glEnableVertexAttribArray(0);
	
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vec3), (void *)(sizeof(vec3)));
	glEnableVertexAttribArray(1);

	
	glBindBuffer(GL_ARRAY_BUFFER, 0); 


	glUseProgram(renderer_state.shader_program);

	glBindVertexArray(renderer_state.vao);
	// Since vert_index counts from 0 on every texture we adjust for that by adding the number of textures to it
	glDrawElements(GL_TRIANGLES, vert_index+last_tex_index, GL_UNSIGNED_INT, 0);

	platform_swap_buffers();
	return TRUE;
}

void
process_texture(vp_texture texture, vec3 *verts, int *vert_index, unsigned int *texture_array, int *texture_index)
{
	glBindTexture(GL_TEXTURE_2D, texture_array[*texture_index]);
	*texture_index += 1;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	
	// TODO: edit this when camera is added, change it to center origin
	int width_in_meters = pixels_to_meters(texture.width);
	int height_in_meters = pixels_to_meters(texture.height);

	GLfloat tex_gl_left_x = texture.x / 15;
	GLfloat tex_gl_right_x = tex_gl_left_x + (width_in_meters / 15);
	
	GLfloat tex_gl_top_y = texture.y / 8.4375;
	GLfloat tex_gl_bottom_y = tex_gl_top_y - (height_in_meters);

	// Top Right
	int tmp = *vert_index;
	verts[tmp++] = (struct vec3){tex_gl_right_x, tex_gl_top_y, 0.0f};
	verts[tmp++] = (struct vec3){1.0f, 1.0f, 0.0f}; 
	*vert_index += 2;

	// Bottom Right
	verts[tmp++] = (struct vec3){tex_gl_right_x, tex_gl_bottom_y, 0.0f};
	verts[tmp++] = (struct vec3){1.0f, 0.0f, 0.0f}; 
	*vert_index += 2;

	// Bottom Left
	verts[tmp++] = (struct vec3){tex_gl_left_x, tex_gl_bottom_y, 0.0f};
	verts[tmp++] = (struct vec3){0.0f, 0.0f, 0.0f}; 
	*vert_index += 2;

	// Top Left
	verts[tmp++] = (struct vec3){tex_gl_left_x, tex_gl_top_y, 0.0f};
	verts[tmp++] = (struct vec3){0.0f, 1.0f, 0.0f}; 
	*vert_index += 2;

}

void
load_bmp_file(char *path, vp_texture *texture)
{
	// Should be fine ?
	vp_allocate_temp(MB(10));
	platform_file_to_buffer(0, path);
}