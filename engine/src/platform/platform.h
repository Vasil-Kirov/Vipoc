#ifdef __cplusplus
	extern "C"{
#endif

#pragma once
#include "include/defines.h"

#if defined VIPOC_LINUX
	#define TRUE 	1
	#define FALSE 	0
	#include <limits.h>
	#define MAX_PATH PATH_MAX
#endif



typedef struct platform_state
{
    void *state;
} platform_state;

typedef struct platform_sharable
{
	void *sharable;
} platform_sharable;

typedef struct platform_thread
{
	void *data;
} platform_thread;

typedef struct vp_config vp_config;


// *
bool32
platform_init(vp_config game, platform_state *pstate);

// *
bool32
platform_handle_message();

// *
void
platform_swap_buffers();

// *
void
platform_output_string(char *str, uint8 color);


void
platform_allocate_console();

// *
VP_API bool32
platform_read_entire_file(char *path, entire_file *e_file);

// *
uint64
platform_get_size_of_file(char *path);

// *
VP_API void *
platform_allocate_memory_chunk(uint64 size);


VP_API void
platform_get_absolute_path(char *output);


void
platform_exit(bool32 is_error);

// *
// Get the width of the screen
int
platform_get_width();

// *
// Get the height of the screen
int
platform_get_height();

// todo
VP_API int64
platform_get_perf_counter();

// todo
VP_API int64
platform_get_frequency();


VP_API platform_sharable
platform_load_sharable(const char *path);

VP_API void *
platform_get_function_from_sharable(platform_sharable sharable, const char *func_name);

VP_API void
platform_free_sharable(platform_sharable sharable);

// INCLUDES DOT
VP_API const char *
platform_get_sharable_extension();


// *
VP_API uint32
platform_get_ms_since_start();

// *
VP_API bool32
platform_toggle_vsync(bool32 toggle);

VP_API bool32
platform_copy_file(const char *old_path, const char *new_path);


VP_API platform_thread
platform_create_thread(void *func, void *parameter);

VP_API bool32
platform_wait_for_thread(platform_thread thread);

VP_API void
platform_switch_fullscreen();

#ifdef __cplusplus
	}
#endif