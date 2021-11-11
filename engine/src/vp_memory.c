#include "vp_memory.h"


internal vp_arena memory_arena;


vp_arena *memory_init(uint64 size)
{
	if(memory_arena.isInitialized) return &memory_arena;
	memory_arena.isInitialized = true;
	memory_arena.start = platform_allocate_memory_chunk(size);
	memory_arena.end = memory_arena.start;
	return &memory_arena;
}


vp_memory vp_arena_allocate(uint64 size)
{
	vp_memory ret;
	ret.ptr = vp_nullptr;
	ret.size = 0;
	if(size == 0) return ret;
	memory_arena.end += size;
	*((int8 *)(memory_arena.end+1)) = 0;
	ret.ptr = memory_arena.end - size;
	ret.size = size;

	return ret;
}


void vp_arena_free_to_chunk(vp_memory mem)
{
	memory_arena.end = mem.ptr;
}