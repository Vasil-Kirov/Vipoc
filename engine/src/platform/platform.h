#pragma once

#include "include/Core.h"
#include "log.h"


typedef struct platform_state
{
    void *state;
} platform_state;


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


void
platform_file_to_buffer(char *output, char *path);


uint64
platform_get_size_of_file(char *path);


void *
platform_allocate_memory_chunk(uint64 size);


void
platform_get_absolute_path(char *output);

