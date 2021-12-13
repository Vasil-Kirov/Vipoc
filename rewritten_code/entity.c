#include "entity.h"


// TODO: This whole thing can be made MUCH MUCH MUCH better

internal vp_entity entities[MAX_ENTITIES];
internal vp_entity **chunks[8];
internal int32 last_entity = 1;

// 10  5.625

void
init_entity_system()
{
	for(int index = 0; index < 8; ++index)
	{
		vp_memory chunk_memory = vp_arena_allocate(sizeof(vp_entity *) * MAX_ENTITIES);
		chunks[index] = chunk_memory.ptr;
	}
}


int
vp_create_entity(vp_tags tag, vp_2d_render_target render, m2 hitbox, entity_func execute, collision_func on_collision)
{
	vp_entity new_entity = {};
	new_entity.id = last_entity;
	new_entity.tag = tag;
	new_entity.render = render;
	new_entity.hitbox = hitbox;
	new_entity.prev_pos = (v2){hitbox.x1, hitbox.y1};
	new_entity.direction = 0;
	new_entity.speed = 0;
	new_entity.execute = execute;
	new_entity.on_collision = on_collision;
	memset(new_entity.collides, 0, MAX_ENTITIES);
	new_entity.is_valid = true;
	entities[last_entity++] = new_entity;
	return last_entity-1;
}

void
vp_destroy_entity(int id)
{
	entities[id].is_valid = false;
}

vp_entity *
vp_get_entity(int id)
{
	vp_entity *entity = vp_nullptr;
	if(id > 0 && entities[id].is_valid)
	{
		entity = &(entities[id]);
	}
	return entity;
}

bool32
rectangles_overlap(m2 rect1, m2 rect2)
{
	if(rect1.x2 < rect2.x1 || rect2.y1 > rect1.y2 ||
	rect1.x1 > rect2.x2 || rect2.y2 < rect1.y1)
	{
		return false;
	}
	return true;
}


// OPTIMIZATION: This is possibly the worst piece of code I have ever written, please future me... write it better
void
vp_sort_entities_in_chunks()
{
	// 2.5f wide, 2.8125f high
	for(int index = 0; index < 8; ++index)
	{
		memset(chunks[index], 0, MAX_ENTITIES);
	}

	int32 chunk_index = 0;
	for(int chunk_row = 0; chunk_row < 2; ++chunk_row)
	{
		for(int chunk_column = 0; chunk_column < 4; ++chunk_column)
		{
			m2 chunk_location = {};
			chunk_location.x1 = 2.5f * (float)chunk_column;
			chunk_location.x2 = chunk_location.x1 + 2.5f;
			chunk_location.y1 = 2.8125f * chunk_row;
			chunk_location.y2 = chunk_location.y1 + 2.8125f;
			for(int index = 1; index < last_entity; ++index)
			{
				
				if(rectangles_overlap(entities[index].hitbox, chunk_location))
				{
					int chunk_to_insert_index = 0;
					while (chunks[chunk_index][chunk_to_insert_index] != vp_nullptr && chunks[chunk_index][chunk_to_insert_index]->id != entities[index].id)
					{
						chunk_to_insert_index++;
					}
					chunks[chunk_index][chunk_to_insert_index] = &entities[index];
				}
			}
			++chunk_index;		
		}
	}
}


bool32
vp_check_collision(int id1, int id2)
{
	return(entities[id1].collides[id2]);
}

void
vp_update_collisions()
{
	for(int chunk_index = 0; chunk_index < 8; ++chunk_index)
	{
		vp_entity *entity = *chunks[chunk_index];
		vp_entity *start = entity;
		while(entity != vp_nullptr && entity < start+MAX_ENTITIES)
		{
			if(entity->is_valid == true)
			{
				vp_entity *second_entity = entity + 1;
				while(second_entity != vp_nullptr && second_entity < start+MAX_ENTITIES)
				{
					if(second_entity->is_valid == true)
					{
						if(rectangles_overlap(entity->hitbox, second_entity->hitbox))
						{
							entities[entity->id].collides[second_entity->id] = true;
							entities[second_entity->id].collides[entity->id] = true;
							if(entity->on_collision != vp_nullptr)
							{
								entity->on_collision(entity, second_entity);
							}
						}
						else
						{
							entities[entity->id].collides[second_entity->id] = false;
							entities[second_entity->id].collides[entity->id] = false;
						}
					}

					second_entity++;
				}
			}
			entity++;
		}
	}
}

float
deg_to_rad(float degrees)
{
	return (degrees * (PI/180.0f));
}

void
move_entity(vp_entity *entity, v2 to_move)
{
	entity->render.world_position.x1 += to_move.x;
	entity->render.world_position.x2 += to_move.x;
	entity->render.world_position.y1 += to_move.y;
	entity->render.world_position.y2 += to_move.y;

	entity->hitbox.x1 += to_move.x;
	entity->hitbox.x2 += to_move.x;
	entity->hitbox.y1 += to_move.y;
	entity->hitbox.y2 += to_move.y;
}

void
entities_to_renderer()
{
	vp_sort_entities_in_chunks();
	vp_update_collisions();
	for(int index = 1; index < MAX_ENTITIES; ++index)
	{
		if(entities[index].is_valid)
		{
			if(entities[index].execute != vp_nullptr)
			{
				entities[index].execute(&entities[index]);
			}
			vp_2d_render_target to_render = normalize_render_target(entities[index].render);
			vp_render_pushback(to_render);
			
			entities[index].prev_pos.x = entities[index].hitbox.x1 ;
			entities[index].prev_pos.y = entities[index].hitbox.y1 ;
			v2 to_move = {};
			float cosine = cos(deg_to_rad(entities[index].direction));
			float sine = sin(deg_to_rad(entities[index].direction));

			to_move.x = cosine * entities[index].speed;
			to_move.y = sine * entities[index].speed;
			move_entity(&entities[index], to_move);
		}
	}
	
}