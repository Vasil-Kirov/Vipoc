#pragma once

#include "platform/platform.h"


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

vp_arena *memory_init(uint64 size);

vp_memory vp_arena_allocate(uint64 size);


void vp_arena_free_to_chunk(vp_memory mem);