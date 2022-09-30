/* date = March 4th 2022 7:55 pm */

#ifndef ENTITY_H
#define ENTITY_H

#include <include/Core.h>
#include <renderer/math_3d.h>
#include <renderer/renderer.h>

#define MAX_ENTITIES 4096

typedef void (*entity_update)();

typedef struct
{
	u32 id;
	vp_mesh_identifier mesh_iden;
	f32 speed;
	b32 valid;
	u32 color;
	entity_update update;
	v3 position;
	m3 hitbox;
	f32 rough_distance_to_camera;
	b32 is_valid;
} vp_entity;


VP_API int
vp_create_entity(vp_mesh_identifier mesh_iden, v3 world_position, f32 speed, u32 color, entity_update update_func);

VP_API void
vp_move_entity(i32 index, v3 new_pos);

void
change_entity_color(int index, u32 new_color);

int
check_if_ray_collides_with_entity(v3 point, v3 dir);

void
update_entities();

void
infinitely_calculate_entity_distance_to_camera();

#endif //ENTITY_H
