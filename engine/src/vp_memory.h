#ifdef __cplusplus
	extern "C"{
#endif

#pragma once

#include "include/defines.h"

typedef struct vp_memory
{
	void *ptr;
	uint64 size;
} vp_memory;

typedef struct vp_arena
{
	bool32 isInitialized;
	void *start;
	void *end;
} vp_arena;

vp_arena *
memory_init(uint64 size);

vp_memory
vp_arena_allocate(uint64 size);

vp_memory
vp_allocate_temp(uint64 size);

void
vp_free_temp_memory();

void
vp_arena_free_to_chunk(vp_memory mem);

vp_memory
vp_allocate_asset(uint64 size);

void
vp_free_asset_memory();

#ifdef __cplusplus
	}
#endif