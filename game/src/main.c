#include <Vipoc.h>
#include <math.h>


/* Struct section */
struct res
{
	int w;
	int h;
};


typedef struct vec2
{
	float x;
	float y;
} vec2;

void scale_vector(vec2 *vector, float scaler)
{
	vector->x *= scaler;
	vector->y *= scaler;
}

void add_vec_to_tex(vp_texture *tex, vec2 vector)
{
	tex->x += vector.x;
	tex->y += vector.y;
}

bool32
OnResize(vp_game *game, int w, int h)
{
	return TRUE;
}

bool32
Update(vp_game *game, float delta_time)
{
	static bool32 isFirst = true;
	static vp_texture player = (vp_texture){50, 50};
	if(isFirst)
	{
		isFirst = false;
		player = vp_load_texture("E:\\Project\\Vipoc\\image.bmp");
	}

	float speed = 1.0f;
	vec2 to_move = {};
	if(vp_is_keydown(VP_KEY_RIGHT))
	{
		to_move.x += speed;
	}
	if (vp_is_keydown(VP_KEY_LEFT))
	{
		to_move.x -= speed;
	}
	if (vp_is_keydown(VP_KEY_UP))
	{
		to_move.y += speed;
	}
	if (vp_is_keydown(VP_KEY_DOWN))
	{
		to_move.y -= speed;
	}
	if(to_move.x && to_move.y)
	{
		scale_vector(&to_move, 0.70710678118f);
	}
	add_vec_to_tex(&player, to_move);
	
	vp_render_pushback(player);

	return TRUE;
}

void
vp_start(vp_game *game)
{
	game->config.name = "vipoc_game";
	game->config.x = 0;
	game->config.y = 0;
	game->config.w = 1600;
	game->config.h = 900;
	game->vp_update=Update;
	game->vp_on_resize=OnResize;
}
