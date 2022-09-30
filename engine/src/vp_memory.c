#include "vp_memory.h"
#include "platform/platform.h"

internal vp_arena memory_arenas[ARENAS_COUNT];

// Currently passing 10 megabytes
void memory_init(uint64 size)
{
	if (memory_arenas[0].isInitialized)
		return;

	memory_arenas[0].isInitialized = true;

	// Permanent memory should never be 0ed
	// OPTIMIZE: Currently allocating a lot of memory for entities that might not be needed
	memory_arenas[PERM_MEM].start = platform_allocate_memory_chunk(size * 20);

	// Temporary memory 0ed all the time
	memory_arenas[TEMP_MEM].start = platform_allocate_memory_chunk(size * 4);

	// Asset memory 0ed on call
	memory_arenas[ASSET_MEM].start = platform_allocate_memory_chunk(size * 50);

	memory_arenas[PERM_MEM].end = memory_arenas[PERM_MEM].start;
	memory_arenas[TEMP_MEM].end = memory_arenas[TEMP_MEM].start;
	memory_arenas[ASSET_MEM].end = memory_arenas[ASSET_MEM].start;
}

void *vp_allocate(uint64 size, unsigned int index)
{
	// TODO: Check for remaining memory -1 for the null terminator

	if (size == 0)
		return vp_nullptr;
	memory_arenas[index].end += size;

	// Null-terminate
	*((int8 *)(memory_arenas[index].end + 1)) = 0;

	return memory_arenas[index].end - size;
}

void vp_free_asset_memory()
{
	memory_arenas[ASSET_MEM].end = memory_arenas[ASSET_MEM].start;
}

void vp_free_temp_memory()
{
	memory_arenas[TEMP_MEM].end = memory_arenas[TEMP_MEM].start;
}
