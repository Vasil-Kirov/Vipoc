#pragma once
#include "renderer/math_3d.h"
#include "renderer/renderer.h"
#include "vp_memory.h"


typedef struct particle
{
	int32 life_time;
	v3 position;
	v3 direction;
	float speed;
	v4 color;
	bool32 is_valid;
} particle;

void
particles_init();

void
update_particles();

void
draw_particles();

void
rewrite_particle_buffer();

// Lifetime is supplied in milliseconds
VP_API void
vp_create_particle(int32 life_time, v3 position, v3 direction, float speed, v4 color);