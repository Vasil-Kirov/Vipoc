#include "renderer/renderer.h"
#include "platform/platform.h"

#define swapf(a, b) {float tmp = a; a = b; b = tmp;}

// temp function
void GenGLBuffs();


typedef struct gl_state
{
	GLuint shader_program;
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	GLuint texture;
	GLuint text_atlas;
	GLuint text_atlas_width;
	GLuint text_atlas_height;
	vec3 *frame_verts;
	GLuint last_frame_index;
	vec3 *text_verts;
	GLuint last_text_index;
} gl_state;

#define VP_MAX_TEXTURES 2048

internal gl_state renderer_state;
global_var vp_render_target to_render[1024];
global_var int last_tex_index;
global_var ascii_char ascii_char_map[256];

vp_texture
load_bmp_file(char *path);

void RendererInit()
{
	LoadGLExtensions();

	vp_memory frame_vert_memory = vp_arena_allocate(KB(60));
	renderer_state.frame_verts = (vec3 *)frame_vert_memory.ptr;

	vp_memory text_vert_memory = vp_arena_allocate(KB(60));
	renderer_state.text_verts = (vec3 *)text_vert_memory.ptr;

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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}



void
vp_load_text_atlas(char *path)
{

	vp_texture result = {};
	if (strstr(path, ".bmp") != vp_nullptr)
	{
		result = load_bmp_file(path);
	}
	glGenTextures(1, &renderer_state.text_atlas);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, renderer_state.text_atlas);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, result.width, result.height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, result.pixels);
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
vp_draw_text(char *text, float x, float y)
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

		target.texture_position.x1 = normalize_tex_coordinate(target.texture_position.x1, renderer_state.text_atlas_width, 0);
		target.texture_position.x2 = normalize_tex_coordinate(target.texture_position.x2, renderer_state.text_atlas_width, 0);
		target.texture_position.y1 = normalize_tex_coordinate(target.texture_position.y1, renderer_state.text_atlas_height, 0);
		target.texture_position.y2 = normalize_tex_coordinate(target.texture_position.y2, renderer_state.text_atlas_height, 0);

		// Left Triangle

		// Bottom left
		renderer_state.text_verts[renderer_state.last_text_index++] = (vec3){target.world_position.x1, target.world_position.y1, 0.0f};
		renderer_state.text_verts[renderer_state.last_text_index++] = (vec3){target.texture_position.x1, target.texture_position.y1, 0.0f};

		// Top Left
		renderer_state.text_verts[renderer_state.last_text_index++] = (vec3){target.world_position.x1, target.world_position.y2, 0.0f};
		renderer_state.text_verts[renderer_state.last_text_index++] = (vec3){target.texture_position.x1, target.texture_position.y2, 0.0f};

		// Top Right
		renderer_state.text_verts[renderer_state.last_text_index++] = (vec3){target.world_position.x2, target.world_position.y2, 0.0f};
		renderer_state.text_verts[renderer_state.last_text_index++] = (vec3){target.texture_position.x2, target.texture_position.y2, 0.0f};

		// Right triangle

		// Bottom Left
		renderer_state.text_verts[renderer_state.last_text_index++] = (vec3){target.world_position.x1, target.world_position.y1, 0.0f};
		renderer_state.text_verts[renderer_state.last_text_index++] = (vec3){target.texture_position.x1, target.texture_position.y1, 0.0f};

		// Top Right
		renderer_state.text_verts[renderer_state.last_text_index++] = (vec3){target.world_position.x2, target.world_position.y2, 0.0f};
		renderer_state.text_verts[renderer_state.last_text_index++] = (vec3){target.texture_position.x2, target.texture_position.y2, 0.0f};

		// Bottom Right
		renderer_state.text_verts[renderer_state.last_text_index++] = (vec3){target.world_position.x2, target.world_position.y1, 0.0f};
		renderer_state.text_verts[renderer_state.last_text_index++] = (vec3){target.texture_position.x2, target.texture_position.y1, 0.0f};

		space_from_last_char += ascii_char_map[(int)text[index]].advance_x;
		if(text[index] == '\n')
		{
			vert_from_last_char += 30;
			space_from_last_char = 0;
		} 
	}
}

unsigned char *
to_next_line(unsigned char *at)
{
	while (*at != '\n')
		at++;
	at++;

	return at;
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
	renderer_state.text_atlas_width = atoi(at);
	ToAndPast(at, "scaleH=");
	renderer_state.text_atlas_height = atoi(at);

	while(true)
	{
		vec4 result = {};
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

		result.y1 = renderer_state.text_atlas_height - result.y1;
		result.y2 = renderer_state.text_atlas_height - result.y2;
		swapf(result.y1, result.y2);

		ascii_char_map[(int)id].rect = result;
		ascii_char_map[(int)id].advance_x = advance_x;
		
		// NOTE: Might be better to have some more concrete way to define the end of a file
		if(id == 126) break;
	}
}

// DONT CALL BEFORE LOADING THE ATLAS
void
vp_parse_font_xml(entire_file file)
{
	vec4 result = {};
	unsigned char *at = (unsigned char *)file.contents;
	while(true)
	{
		/* Check if end was reached */
		if (!strcmp((const char *)at, "</Font>\n")) break;

		/* Get Offset */
		at = (unsigned char *)strstr((const char *)at, "offset=\"");
		at += sizeof("offset=\"")-1;
		int advance_x = atoi((const char *)at);
		while (*at != ' ')
			++at;
		++at;
		int offset_y = atoi((const char *)at);

		/* Get Char Size & Position */
		at = (unsigned char *)strstr((const char *)at, "rect=\"");
		at += sizeof("rect=\"")-1;
		
		result.x1 = atoi((const char *)at);
		while(*at != ' ') 
			++at;
		++at;

		result.y1 = atoi((const char *)at);
		while (*at != ' ')
			++at;
		++at;
		
		result.x2 = atoi((const char *)at);
		while (*at != ' ')
			++at;
		++at;
		
		result.y2 = atoi((const char *)at);
		at = (unsigned char *)strstr((const char *)at, "code=\"");
		at += sizeof("code=\"")-1;
		
		result.x2 += result.x1;
		result.y2 += result.y1;

		result.y1 = renderer_state.text_atlas_height - result.y1;
		result.y2 = renderer_state.text_atlas_height - result.y2;
		swapf(result.y1, result.y2);

		ascii_char_map[*at].rect = result;
		ascii_char_map[*at].advance_x = advance_x;
		ascii_char_map[*at].offset_y = offset_y;

		at = to_next_line(at);
	}
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

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, result.pixels);

}

void
vp_render_pushback(vp_render_target target)
{
	to_render[last_tex_index++] = target;
}

void
FrameBufferToVerts(vp_render_target target)
{
	// Left Triangle

	// Bottom left
	renderer_state.frame_verts[renderer_state.last_frame_index++] = (vec3){target.world_position.x1, target.world_position.y1, 0.0f};
	renderer_state.frame_verts[renderer_state.last_frame_index++] = (vec3){target.texture_position.x1, target.texture_position.y1, 0.0f};

	// Top Left
	renderer_state.frame_verts[renderer_state.last_frame_index++] = (vec3){target.world_position.x1, target.world_position.y2, 0.0f};
	renderer_state.frame_verts[renderer_state.last_frame_index++] = (vec3){target.texture_position.x1, target.texture_position.y2, 0.0f};

	// Top Right
	renderer_state.frame_verts[renderer_state.last_frame_index++] = (vec3){target.world_position.x2, target.world_position.y2, 0.0f};
	renderer_state.frame_verts[renderer_state.last_frame_index++] = (vec3){target.texture_position.x2, target.texture_position.y2, 0.0f};

	// Right triangle

	// Bottom Left
	renderer_state.frame_verts[renderer_state.last_frame_index++] = (vec3){target.world_position.x1, target.world_position.y1, 0.0f};
	renderer_state.frame_verts[renderer_state.last_frame_index++] = (vec3){target.texture_position.x1, target.texture_position.y1, 0.0f};

	// Top Right
	renderer_state.frame_verts[renderer_state.last_frame_index++] = (vec3){target.world_position.x2, target.world_position.y2, 0.0f};
	renderer_state.frame_verts[renderer_state.last_frame_index++] = (vec3){target.texture_position.x2, target.texture_position.y2, 0.0f};

	// Bottom Right
	renderer_state.frame_verts[renderer_state.last_frame_index++] = (vec3){target.world_position.x2, target.world_position.y1, 0.0f};
	renderer_state.frame_verts[renderer_state.last_frame_index++] = (vec3){target.texture_position.x2, target.texture_position.y1, 0.0f};
}

void
GenGLBuffs()
{	
	glGenVertexArrays(1, &(renderer_state.vao));
	glGenBuffers(1, &(renderer_state.vbo));
}
void
SortRenderEntries(vp_render_target *Textures, int Size)
{
	for(int Index = 0; Index < Size; ++Index)
	{
		for(int Index2 = Index; Index2 < Size; ++Index2)
		{
			if(Textures[Index].layer_id > Textures[Index2].layer_id)
			{
				vp_render_target tmp = Textures[Index];
				Textures[Index] = Textures[Index2];
				Textures[Index2] = tmp;
			}
		}
	}
}


bool32
render_update()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	GLint uniform_location = glGetUniformLocation(renderer_state.shader_program, "texture1");
	glUniform1i(uniform_location, 0);
	/* Draw Textures */
	if (last_tex_index == 0) return TRUE;

	// TODO: Change this to a good sorting algorithm
	SortRenderEntries(to_render, last_tex_index);
	for(int Index = 0; Index < last_tex_index; ++Index)
	{
		FrameBufferToVerts(to_render[Index]);
	}

	glBindVertexArray(renderer_state.vao);

	glBindBuffer(GL_ARRAY_BUFFER, renderer_state.vbo);

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
	last_tex_index = 0;


	/* Draw Text */ 
	if(renderer_state.last_text_index != 0)
	{
		glUniform1i(uniform_location, 1);
		glBindVertexArray(renderer_state.vao);

		glBindBuffer(GL_ARRAY_BUFFER, renderer_state.vbo);

		glBufferData(GL_ARRAY_BUFFER, renderer_state.last_text_index * sizeof(vec3), renderer_state.text_verts, GL_STATIC_DRAW);

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
		glDrawArrays(GL_TRIANGLES, 0, renderer_state.last_text_index * 3);
		renderer_state.last_text_index = 0;
	} 

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