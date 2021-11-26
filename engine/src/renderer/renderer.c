#include "renderer/renderer.h"
#include "platform/platform.h"


// temp function
void GenGLBuffs();


typedef struct gl_state
{
	GLuint shader_program;
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	GLuint texture;
	vec3 *frame_verts;
	GLuint last_frame_index;
} gl_state;

#define VP_MAX_TEXTURES 2048

internal gl_state renderer_state;

vp_texture
load_bmp_file(char *path);

void RendererInit()
{
	LoadGLExtensions();

	vp_memory frame_vert_memory = vp_arena_allocate(KB(60));
	renderer_state.frame_verts = (vec3 *)frame_vert_memory.ptr;
	
	// TODO: Change this to temp memory
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
			VP_ERROR("Failed to compile shader: %s", info_log);
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


void
vp_load_texture(char *path, int width, int height)
{
	vp_texture result = {};
	if(strstr(path, ".bmp") != vp_nullptr)
	{
		result = load_bmp_file(path);
	}
	glGenTextures(1, &renderer_state.texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderer_state.texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, result.pixels);

}

void
vp_render_pushback(vec4 position, vec4 tex_location)
{
	// Left Triangle

	// Bottom left
	renderer_state.frame_verts[renderer_state.last_frame_index++] = (vec3){position.x1, position.y1, 0.0f};
	renderer_state.frame_verts[renderer_state.last_frame_index++] = (vec3){tex_location.x1, tex_location.y1, 0.0f};

	// Top Left
	renderer_state.frame_verts[renderer_state.last_frame_index++] = (vec3){position.x1, position.y2, 0.0f};
	renderer_state.frame_verts[renderer_state.last_frame_index++] = (vec3){tex_location.x1, tex_location.y2, 0.0f};
	
	// Top Right
	renderer_state.frame_verts[renderer_state.last_frame_index++] = (vec3){position.x2, position.y2, 0.0f};
	renderer_state.frame_verts[renderer_state.last_frame_index++] = (vec3){tex_location.x2, tex_location.y2, 0.0f};

	// Right triangle

	// Bottom Left
	renderer_state.frame_verts[renderer_state.last_frame_index++] = (vec3){position.x1, position.y1, 0.0f};
	renderer_state.frame_verts[renderer_state.last_frame_index++] = (vec3){tex_location.x1, tex_location.y1, 0.0f};
	
	// Top Right
	renderer_state.frame_verts[renderer_state.last_frame_index++] = (vec3){position.x2, position.y2, 0.0f};
	renderer_state.frame_verts[renderer_state.last_frame_index++] = (vec3){tex_location.x2, tex_location.y2, 0.0f};
	
	// Bottom Right
	renderer_state.frame_verts[renderer_state.last_frame_index++] = (vec3){position.x2, position.y1, 0.0f};
	renderer_state.frame_verts[renderer_state.last_frame_index++] = (vec3){tex_location.x2, tex_location.y1, 0.0f};

}


void
GenGLBuffs()
{	
	glGenVertexArrays(1, &(renderer_state.vao));
	glGenBuffers(1, &(renderer_state.vbo));
}


bool32 render_update()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	if(renderer_state.last_frame_index == 0) return TRUE;

	glBindVertexArray(renderer_state.vao);

	glBindBuffer(GL_ARRAY_BUFFER, renderer_state.vbo);

	// NOTE: might be last_frame_index -1? Probably not, since it counts from 0
	glBufferData(GL_ARRAY_BUFFER, renderer_state.last_frame_index * sizeof(vec3), renderer_state.frame_verts, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vec3), (void *)0);
	glEnableVertexAttribArray(0);
	
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vec3), (void *)(sizeof(vec3)));
	glEnableVertexAttribArray(1);

	 
	glUseProgram(renderer_state.shader_program);

	glBindVertexArray(renderer_state.vao);
	if (glGetError() != 0)
	{
		VP_ERROR("GL ERROR: %d", glGetError());
	}

	glDrawArrays(GL_TRIANGLES, 0, renderer_state.last_frame_index * 3);
	renderer_state.last_frame_index = 0;

	platform_swap_buffers();
	return TRUE;
}

float
pixels_to_meters(int pixels)
{
	float pixels_per_meter = platform_get_width() / 100;
	return(pixels / pixels_per_meter);
	// 16:9 = 100 meters width; 56.25 meters height
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