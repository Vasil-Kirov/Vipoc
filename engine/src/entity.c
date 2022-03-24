#include <entity.h>
#include <renderer/renderer.h>

#include <stdlib.h>


internal vp_entity entities[MAX_ENTITIES];

int
vp_create_entity(u32 model_index, v3 world_position, f32 speed, u32 color, entity_update update_func)
{
	v3 h1 = world_position;
	v3 h2 = v3_f32_add(world_position, 20);
	m3 hitbox = { .m = {h1.x, h1.y, h1.z, h2.x, h2.y, h2.z} };
	
	vp_entity new_entity = {.model_index = model_index, .position = world_position, .color = color, .update = update_func, .valid = true, .hitbox = hitbox};
	
	for(i32 index = 0; index < MAX_ENTITIES; ++index)
	{
		if(!entities[index].valid)
		{
			new_entity.id = index;
			entities[index] = new_entity;
			return index;
		}
	}
	
	VP_FATAL("MAX ENTITIES REACHED!");
	return -1;
}


static v3 sorting_point;

int
sort_z(const void *a, const void *b)
{
	f32 distance_a =  get_distance_3d(sorting_point, ((vp_entity *)a)->position);
	f32 distance_b =  get_distance_3d(sorting_point, ((vp_entity *)b)->position);
	return (distance_a > distance_b) ? 1 : -1;
}

// NOTE(Vasko): look more into this code. Got it from:
/*
https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
*/
b32
calculate_ray_box_intersection(v3 ray_origin, v3 dir, m3 box)
{
	v3 min = {box.m[0], box.m[1], box.m[2] };
	v3 max = {box.m[3], box.m[4], box.m[5] };
	
	f32 tmin = (min.x - ray_origin.x) / dir.x;
	f32 tmax = (max.x - ray_origin.x) / dir.x;
	
	if(tmin > tmax) v_swapf(tmin, tmax);
	
	float tymin = (min.y - ray_origin.y) / dir.y; 
    float tymax = (max.y - ray_origin.y) / dir.y; 
	
	if(tymin > tymax) v_swapf(tymin, tmax);
	
	if ((tmin > tymax) || (tymin > tmax)) 
        return false; 
	
	if (tymin > tmin) 
        tmin = tymin; 
	
    if (tymax < tmax)
	{
        tmax = tymax;
	}
	
	float tzmin = (min.z - ray_origin.z) / dir.z; 
    float tzmax = (max.z - ray_origin.z) / dir.z; 
	
	
    if (tzmin > tzmax) v_swapf(tzmin, tzmax); 
	
    if ((tmin > tzmax) || (tzmin > tmax)) 
        return false; 
	
    if (tzmin > tmin) 
        tmin = tzmin; 
	
    if (tzmax < tmax) 
        tmax = tzmax; 
	
    return true; 
}

#if 0
int
check_if_ray_collides_with_entity(v3 point, v3 dir)
{
	size_t entities_size = sizeof(vp_entity) * MAX_ENTITIES;
	sorting_point = point;
	
	vp_entity *sorted_static_entities = vp_allocate_temp(entities_size).ptr;
	memcpy(sorted_static_entities, static_entities, entities_size);
	qsort(sorted_static_entities, MAX_ENTITIES, sizeof(vp_entity), sort_z);
	
	vp_entity *sorted_dynamic_entities = vp_allocate_temp(entities_size).ptr;
	memcpy(sorted_dynamic_entities, dynamic_entities, entities_size);
	qsort(sorted_dynamic_entities, MAX_ENTITIES, sizeof(vp_entity), sort_z);
	
	for(i32 index = 0; index < MAX_ENTITIES; ++index)
	{
		if(!sorted_static_entities[index].valid) continue;
		if(calculate_ray_box_intersection(point, dir, sorted_static_entities[index].hitbox))
		{
			static_entities[sorted_static_entities[index].id].color = 0x800080;
			for(i32 index = 0; index < MAX_ENTITIES; ++index)
			{
				if(!static_entities[index].valid) continue;
				
				vp_entity entity = static_entities[index];
				vp_object_pushback(entity.model_index, entity.color, entity.position, true, true);
			}
			return sorted_static_entities[index].id;
		}
	}
	
	for(i32 index = 0; index < MAX_ENTITIES; ++index)
	{
		if(!sorted_dynamic_entities[index].valid) continue;
		if(calculate_ray_box_intersection(point, dir, sorted_dynamic_entities[index].hitbox))
		{
			return sorted_dynamic_entities[index].id;
		}
	}
	return -1;
}
#endif

void
vp_move_entity(i32 index, v3 dir)
{
	if(index < 0 || index > MAX_ENTITIES)
	{
		VP_ERROR("Invalid index passed to function move_dynamic_entity: %d", index);
		return;
	}
	if(!entities[index].valid)
	{
		VP_ERROR("Tried to move invalid entity in function move_dynamic_entity");
		return;
	}
	
	dir = v3_normalize(dir);
	vp_entity entity = entities[index];
	entities[index].position = v3_add(entity.position, v3_scale(dir, entity.speed));
}

void
update_entities()
{
	for(i32 index = 0; index < MAX_ENTITIES; ++index)
	{
		vp_entity entity = entities[index];
		
		if(!entity.valid)
			continue;
		
		if(entity.update)
			entity.update();
		
		vp_object_pushback(entity.model_index, entity.color, entity.position, false, true);
	}
}