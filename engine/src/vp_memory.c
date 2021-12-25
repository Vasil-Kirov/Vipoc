#include "vp_memory.h"
#include "platform/platform.h"


internal vp_arena memory_arena;
internal vp_arena temp_memory;
internal vp_arena asset_memory;


// Currently passing 10 megabytes
vp_arena *memory_init(uint64 size)
{
	if(memory_arena.isInitialized) return &memory_arena;
	memory_arena.isInitialized = true;

	// Permanent memory should never be 0ed
	// OPTIMIZE: Currently allocating a lot of memory for entities that might not be needed
	memory_arena.start = platform_allocate_memory_chunk(size*20);
	
	// Temporary memory 0ed all the time
	temp_memory.start = platform_allocate_memory_chunk(size*4);
	
	// Asset memory 0ed on call
	asset_memory.start = platform_allocate_memory_chunk(size*50);

	memory_arena.end = memory_arena.start;
	temp_memory.end = temp_memory.start;
	asset_memory.end = asset_memory.start;
	return &memory_arena;
}

vp_memory
vp_allocate_asset(uint64 size)
{
	// TODO: Check for remaining memory -1 for the null terminator
	
	vp_memory ret;
	ret.ptr = vp_nullptr;
	ret.size = 0;
	if(size == 0) return ret;
	asset_memory.end += size;

	// Null-terminate
	*((int8 *)(asset_memory.end+1)) = 0;

	// Set return values
	ret.ptr = asset_memory.end - size;
	ret.size = size;

	return ret;
}

vp_memory
vp_allocate_temp(uint64 size)
{
	// TODO: Check for remaining memory -1 for the null terminator
	
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
vp_free_asset_memory()
{
	memset(asset_memory.start, 0, asset_memory.end - asset_memory.start);
	asset_memory.end = asset_memory.start;
}

void
vp_free_temp_memory()
{
	temp_memory.end = temp_memory.start;
}

vp_memory
vp_arena_allocate(uint64 size)
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