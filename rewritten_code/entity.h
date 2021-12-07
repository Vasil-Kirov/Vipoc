#ifdef __cplusplus
	extern "C"{
#endif

#pragma once
#include "include/defines.h"
#include "renderer/renderer.h"

#define MAX_ENTITIES 2048
#define DEFINE_TAG(name, code) name = code


/* REMOVED CODE FROM MAIN
void
MoveEntity(vp_entity *entity, v2 to_move)
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
OnPlayerCollision(vp_entity *ThisEntity, vp_entity *CollidesWith)
{
	if(CollidesWith->tag == WALL)
	{

		v2 ToMove = {};

		float MoveDirection = ThisEntity->direction - 180;
		ToMove.x = ThisEntity->speed * 1.5f * cos(MoveDirection * (PI / 180.0f));
		ToMove.y = ThisEntity->speed * 1.5f * sin(MoveDirection * (PI / 180.0f));
		ThisEntity->hitbox += ToMove;
		ThisEntity->render.world_position += ToMove;
	}
}
*/

typedef enum tags
{
	NO_TAG,
	WALL,
	UI,
	DAMAGE,
	DOOR,
	NUMBER_OF_TAGS
} vp_tags;

struct entity;
typedef void (*entity_func)(struct entity *this_entity);
typedef void (*collision_func)(struct entity *this_entity, struct entity *collides_with);

typedef struct entity
{
	int id;
	vp_tags tag;
	float speed;
	float direction;
	vp_render_target render;
	m2 hitbox;
	v2 prev_pos;
	entity_func execute;
	bool32 collides[MAX_ENTITIES];
	collision_func on_collision;
	bool32 is_valid;
} vp_entity;

void
init_entity_system();

// Entity execute function can be vp_nullptr
VP_API int
vp_create_entity(vp_tags tag, vp_render_target render, m2 hitbox, entity_func execute, collision_func on_collision);

VP_API vp_entity *
vp_get_entity(int id);

VP_API void
vp_destroy_entity(int id);

VP_API bool32
vp_check_collision(int id1, int id2);

void
entities_to_renderer();


#ifdef __cplusplus
	}
#endif