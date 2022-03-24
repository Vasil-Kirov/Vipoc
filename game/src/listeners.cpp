#include <listeners.h>

static bool32 IsCameraLocked;
static bool32 VSyncToggle = false;

void
OnKey(vp_keys Key, bool32 IsDown)
{
	if(IsDown)
	{
		if(ConsoleIsOn())
		{
			char C;
			if(Key == VP_KEY_BACKSPACE)
			{
				ConsoleDeleteChar();
			}
			else if(platform_key_to_char(Key, &C))
			{
				ConsoleAddChar(C);
			}
		}
		if(Key == VP_KEY_DIVIDE)
		{
			if(ConsoleIsOn())
			{
				StopConsole();
			}
			else
			{
				StartConsole();
			}
		}
		if(vp_is_keydown(VP_KEY_CONTROL))
		{
			if(Key == VP_KEY_O)
			{
				if(IsCameraLocked)
				{
					vp_unlock_camera();
					IsCameraLocked = false;
				}
				else
				{
					LockCamera();
				}
			}
			else if(Key == VP_KEY_X)
				vp_toggle_polygons();
			
			else if(Key == VP_KEY_V)
			{
				platform_toggle_vsync(VSyncToggle);
				VSyncToggle = !VSyncToggle;
			}
			else if(Key == VP_KEY_P)
				vp_toggle_particle_update();
		}
		if(vp_is_keydown(VP_KEY_ALT))
		{
			if(Key == VP_KEY_ENTER)
			{
				platform_switch_fullscreen();
			}
		}
	}
}


bool32
OnResize(vp_game *internal_game, int Width, int Height)
{
	if(Width == 0 || Height == 0) return TRUE;
	SetConfigRes(Width, Height);
	return TRUE;
}

void
OnMouse(vp_buttons Button, bool32 IsDown, i32 X, i32 Y)
{
	if(IsDown)
	{
		if(Button == VP_MOUSE_BUTTON_RIGHT)
		{
			//vp_cast_ray(X, Y);
		}
	}
}


void
HandleInput()
{
	if(ConsoleIsOn()) return;
	
    float CamSpeed = 10.0f;
	if(vp_is_keydown(VP_KEY_W))
    {
		vp_move_camera(VP_UP, CamSpeed);
    }
    if(vp_is_keydown(VP_KEY_S))
    {
        vp_move_camera(VP_DOWN, CamSpeed);
	}
    if(vp_is_keydown(VP_KEY_A))
    {
        vp_move_camera(VP_LEFT, CamSpeed);
	}
    if(vp_is_keydown(VP_KEY_D))
    {
        vp_move_camera(VP_RIGHT, CamSpeed);
	}
}

void
ToggleCameraLock(b32 toggle) { IsCameraLocked = toggle; }

b32
CameraLockedIsLocked() { return IsCameraLocked; }

void
LockCamera()
{
	vp_lock_camera(3.14f, 1.45f, (v3){15.25f, 12.5f, 5.7f});
	ToggleCameraLock(true);
}
