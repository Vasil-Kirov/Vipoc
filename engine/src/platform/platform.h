#pragma once
#include "include/defines.h"


typedef struct platform_state
{
    void *state;
} platform_state;

typedef struct vp_config vp_config;

bool32
platform_init(vp_config game, platform_state *pstate);


bool32
platform_handle_message();


void
platform_swap_buffers();


void
platform_output_string(char *str, uint8 color);


void
platform_allocate_console();


VP_API void
platform_read_entire_file(char *path, entire_file *e_file);


uint64
platform_get_size_of_file(char *path);


VP_API void *
platform_allocate_memory_chunk(uint64 size);


VP_API void
platform_get_absolute_path(char *output);


void
platform_exit(bool32 is_error);


int
platform_get_width();


int64
platform_get_perf_counter();


int64
platform_get_frequency();