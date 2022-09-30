#ifdef __cplusplus
extern "C"
{
#endif

#pragma once

#include "include/defines.h"

	enum MEMORY_ARENAS
	{
		PERM_MEM,
		TEMP_MEM,
		ASSET_MEM,
		ARENAS_COUNT
	};

	typedef struct vp_arena
	{
		bool32 isInitialized;
		void *start;
		void *end;
	} vp_arena;

	void
	memory_init(uint64 size);

	void *
	vp_allocate(uint64 size, unsigned int index);

	void
	vp_free_temp_memory();

	void
	vp_free_asset_memory();

#define vp_allocate_temp(size) vp_allocate(size, TEMP_MEM)
#define vp_allocate_perm(size) vp_allocate(size, PERM_MEM)
#define vp_allocate_asset(size) vp_allocate(size, ASSET_MEM)

#ifdef __cplusplus
}
#endif