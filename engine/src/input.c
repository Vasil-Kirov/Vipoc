#include "input.h"
#include "platform/platform.h"


typedef struct vp_mouse
{
	int x;
	int y;
	bool32 button_list[6];
} vp_mouse;

internal bool32 input_list[256];
internal vp_mouse mouse;


bool32 vp_is_keydown(keys key)
{
	return input_list[key];
}


void vp_get_mouse_pos(int *x, int *y)
{
	*x = mouse.x;
	*y = mouse.y;
}


bool32 vp_is_mbdown(buttons button)
{
	return mouse.button_list[button];
}


void input_keyboard_key(keys key, bool32 is_down)
{
	input_list[key] = is_down;
}


void input_mouse_button(buttons button, bool32 is_down)
{
	mouse.button_list[button] = is_down;
}
void input_mouse_position(int x, int y)
{
	mouse.x = x;
	mouse.y = y;
}