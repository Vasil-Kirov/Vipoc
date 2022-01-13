#include "renderer/particle.h"
#include "application.h"

internal particle *particle_buffer;
internal uint32 last_particle;
internal uint32 ms_last_frame;
internal uint32 invalid_particle_counter;

global_var particle *particle_buffer_end;
#define PARTICLE_BUFFER_MEMORY MB(1)
void
particles_init()
{
	vp_memory particle_memory = vp_arena_allocate(PARTICLE_BUFFER_MEMORY);
	particle_buffer = (particle *)particle_memory.ptr;
	particle_buffer_end = particle_buffer + PARTICLE_BUFFER_MEMORY / sizeof(particle);
	memset(particle_buffer, 0, PARTICLE_BUFFER_MEMORY);
	
}

void
update_particles()
{
	uint32 ms_this_frame = platform_get_ms_since_start();
	uint32 to_subtract = ms_this_frame - ms_last_frame;
	for(int index = 0; index < last_particle; ++index)
	{
		particle_buffer[index].life_time -= to_subtract;
		if(particle_buffer[index].life_time <= 0)
		{
			particle_buffer[index].is_valid = false;
			invalid_particle_counter++;
		}
		else
		{
			v3 to_move = v3_scale(particle_buffer[index].direction, particle_buffer[index].speed);
			to_move = v3_scale(to_move, vp_get_dtime());
			particle_buffer[index].position = v3_add(particle_buffer[index].position, to_move);
		}
	}
	ms_last_frame = ms_this_frame;
}

void
draw_particles()
{
	if(vp_is_particle_update_off()) ms_last_frame = platform_get_ms_since_start();
	for(int index = 0; index < last_particle; ++index)
	{
		if(particle_buffer[index].is_valid)
		{
			particle to_push = particle_buffer[index];
			vp_object_pushback(0, to_push.color, to_push.position, false, false);
		}
	}
}

void
rewrite_particle_buffer()
{
	if(invalid_particle_counter < 100) return;
	
	vp_memory temp_buffer_memory = vp_allocate_temp(PARTICLE_BUFFER_MEMORY);
	particle *temp_buffer = temp_buffer_memory.ptr;
	int temp_buffer_index = 0;
	for(int index = 0; index < last_particle; ++index)
	{
		if(particle_buffer[index].is_valid)
		{
			temp_buffer[temp_buffer_index++] = particle_buffer[index];
		}
	}
	memset(particle_buffer, 0, PARTICLE_BUFFER_MEMORY);
	memcpy(particle_buffer, temp_buffer, PARTICLE_BUFFER_MEMORY);
    
	last_particle = temp_buffer_index;	
	invalid_particle_counter = 0;
}

void
vp_create_particle(int32 life_time, v3 position, v3 direction, float speed, v4 color)
{
	assert(particle_buffer + last_particle < particle_buffer_end);
	particle_buffer[last_particle].life_time 	= life_time;
	particle_buffer[last_particle].position 	= position;
	particle_buffer[last_particle].direction 	= direction;
	particle_buffer[last_particle].speed 		= speed;
	particle_buffer[last_particle].color 		= color;
	particle_buffer[last_particle].is_valid 	= true;
	last_particle++;
}