#include "vp_memory.h"
#include "platform/platform.h"


internal vp_arena memory_arena;
internal vp_arena temp_memory;


vp_arena *memory_init(uint64 size)
{
	if(memory_arena.isInitialized) return &memory_arena;
	memory_arena.isInitialized = true;
	memory_arena.start = platform_allocate_memory_chunk(size);
	temp_memory.start = platform_allocate_memory_chunk(size*4);
	
	memory_arena.end = memory_arena.start;
	return &memory_arena;
}


vp_memory
vp_allocate_temp(uint64 size)
{
	// TODO: Check for remaining memory -1 for the null terminator
	
	// Initialize for some reason?
	vp_memory ret;
	ret.ptr = vp_nullptr;
	ret.size = 0;
	if(size == 0) return ret;
	temp_memory.end += size;

	// Null-terminate
	*((int8 *)(temp_memory.end+1)) = 0;

	// Set return values
	ret.ptr = temp_memory.end - size;
	ret.size = size;

	return ret;
}


void
vp_free_temp_memory()
{
	memset(temp_memory.start, 0, temp_memory.end - temp_memory.start);
	temp_memory.end = temp_memory.start;
}

vp_memory vp_arena_allocate(uint64 size)
{
	// TODO: Check for remaining memory -1 for the null terminator
	
	// Initialize for some reason?
	vp_memory ret;
	ret.ptr = vp_nullptr;
	ret.size = 0;
	if(size == 0) return ret;
	memory_arena.end += size;

	// Null-terminate
	*((int8 *)(memory_arena.end+1)) = 0;

	// Set return values
	ret.ptr = memory_arena.end - size;
	ret.size = size;

	return ret;
}


void vp_arena_free_to_chunk(vp_memory mem)
{
	memory_arena.end = mem.ptr;
}