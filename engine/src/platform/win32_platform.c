#ifdef VIPOC_WIN32

#define WIN32_LEAN_AND_MEAN
// might be able to remove Windows.h
#include <Windows.h>
#include <hidsdi.h>

#include "platform/platform.h"
#include "log.h"
#include "renderer/renderer.h"
#include "input.h"
#include "include/Core.h"

typedef BOOL(WINAPI * PFNWGLSWAPINTERVALEXTPROC)(int interval);

typedef struct win32_state
{
	HINSTANCE Instance;
	HWND Window;
	int w;
	int h;
} win32_state;

typedef struct cursor
{
	int32 x;
	int32 y;
} cursor;


internal win32_state *Win32State;
internal vp_memory raw_input_memory;
internal uint32 ms_at_start;
internal cursor mouse; 

LRESULT CALLBACK
WindowProc(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam);
void Win32LoadOpenGL(HWND Window);
void Win32LoadRawInput(HWND Window);


bool32
platform_init(vp_config game, platform_state *pstate)
{   
	int x = game.x;
	int y = game.y; 
	int w = game.w; 
	int h = game.h;
	pstate->state = platform_allocate_memory_chunk(sizeof(win32_state));
	Win32State = (win32_state *)pstate->state; 
	Win32State->w = game.w;
	Win32State->h = game.h;

	Win32State->Instance = GetModuleHandleA(0);

	WNDCLASSA wnd = {};
	wnd.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wnd.lpfnWndProc = WindowProc;
	wnd.hInstance = Win32State->Instance;
	wnd.hCursor = LoadCursor(wnd.hInstance, IDC_ARROW);
	wnd.lpszClassName = "VipocProgramWindowClass";
	if(RegisterClassA(&wnd))
	{
		Win32State->Window = CreateWindowExA(0, wnd.lpszClassName, game.name, WS_OVERLAPPEDWINDOW | WS_VISIBLE, x, y, w, h, 0, 0, Win32State->Instance, 0);
		if(Win32State->Window)
		{
			raw_input_memory = vp_arena_allocate(MB(1));
			Win32LoadRawInput(Win32State->Window);
			Win32LoadOpenGL(Win32State->Window);
			glViewport(0, 0, game.w, game.h);
			ms_at_start = GetTickCount();
			
			RECT WindowRect = {};
			GetWindowRect(Win32State->Window, &WindowRect);
			MapWindowPoints(HWND_DESKTOP, GetParent(Win32State->Window), (LPPOINT)&WindowRect, 2);

			int new_pos_x = WindowRect.left + ((WindowRect.right - WindowRect.left) / 2);
			int new_pos_y = WindowRect.top + ((WindowRect.bottom - WindowRect.top) / 2);

			mouse.x = new_pos_x;
			mouse.y = new_pos_y;

			SetCursorPos(new_pos_x, new_pos_y);


			return TRUE;
		} 
	}
	return FALSE;
}


void Win32LoadRawInput(HWND Window)
{
	RAWINPUTDEVICE rids[5];

	rids[0].usUsage = 0x02; // Mouse
	rids[0].usUsagePage = 0x01;
	rids[0].dwFlags = RIDEV_INPUTSINK; 
	rids[0].hwndTarget = Window;

	rids[1].usUsage = 0x06; // Keyboard
	rids[1].usUsagePage = 0x01;
	rids[1].dwFlags = RIDEV_INPUTSINK; 
	rids[1].hwndTarget = Window;
	
	rids[2].usUsage = 0x04; // Controller
	rids[2].usUsagePage = 0x01;
	rids[2].dwFlags = RIDEV_INPUTSINK; 
	rids[2].hwndTarget = Window;

	rids[3].usUsage = 0x05; // Controller
	rids[3].usUsagePage = 0x01;
	rids[3].dwFlags = RIDEV_INPUTSINK; 
	rids[3].hwndTarget = Window;

	rids[4].usUsage = 0x01; // Mouse
	rids[4].usUsagePage = 0x01;
	rids[4].dwFlags = RIDEV_INPUTSINK; 
	rids[4].hwndTarget = Window;


	if (RegisterRawInputDevices(rids, 5, sizeof(RAWINPUTDEVICE)) == FALSE) Error("RegisterRawInputDevice failed!\n");
}

void
platform_get_absolute_path(char *output)
{
	if (!GetModuleFileNameA(NULL, output, MAX_PATH))
	{
		Error("Failed to get directory location, file path might be too long");
	}
	int size = vstd_strlen(output);
	
	// twice to remove the exe name and then to leave the bin directory
	for(int i = size-1; i >= 0; --i)
	{
		if(output[i] == '\\')
		{
			output[i] = '\0';
			break;
		}
	}

	size = vstd_strlen(output);
	for(int i = size-1; i >= 0; --i)
	{
		if(output[i] == '\\')
		{
			output[i+1] = '\0';
			break;
		}
	}
}


void
platform_read_entire_file(char *path, entire_file *e_file)
{
	HANDLE File = CreateFileA(path, GENERIC_READ, 0, vp_nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, vp_nullptr);
	if(File == INVALID_HANDLE_VALUE) return;
	e_file->size = GetFileSize(File, vp_nullptr);
	ReadFile(File, e_file->contents, e_file->size, vp_nullptr, vp_nullptr);
	CloseHandle(File);
}

uint64
platform_get_size_of_file(char *path)
{
	HANDLE File = CreateFileA(path, GENERIC_READ, 0, vp_nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, vp_nullptr);
	if(File == INVALID_HANDLE_VALUE) return 0;
	uint32 FileSize = GetFileSize(File, vp_nullptr);
	CloseHandle(File);
	return FileSize;
}

void *
platform_allocate_memory_chunk(uint64 size)
{
	return VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);

}

uint32
platform_get_ms_since_start()
{
	return GetTickCount() - ms_at_start;
}
int64
platform_get_frequency()
{
	LARGE_INTEGER result;
	QueryPerformanceFrequency(&result);
	return result.QuadPart;
}

int64
platform_get_perf_counter()
{
	LARGE_INTEGER result;
	QueryPerformanceCounter(&result);
	return result.QuadPart;
}


void
HandleRawInput(RAWINPUT *raw_input);

bool32
platform_handle_message()
{

	MSG Message;
	while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
	{
		if(Message.message == WM_QUIT)
		{
			return false;
		}
		if(Message.message == WM_INPUT)
		{
			UINT DataSize;
			if (GetRawInputData((HRAWINPUT)Message.lParam, RID_INPUT, NULL, &DataSize, sizeof(RAWINPUTHEADER)) == -1)
			{
				Error("Failed getting raw input amount\n");
			}
			
			if (GetRawInputData((HRAWINPUT)Message.lParam, RID_INPUT, (PRAWINPUT)raw_input_memory.ptr, &DataSize, sizeof(RAWINPUTHEADER)) == -1)
			{
				Error("Failed getting raw input\n");
			}
			HandleRawInput(raw_input_memory.ptr);
			
		}
		
		TranslateMessage(&Message);
		DispatchMessageA(&Message);
	
		
	}

	return true;

}


int
platform_get_width()
{
	return Win32State->w;
}

int platform_get_height()
{
	return Win32State->h;
}

void platform_exit(bool32 is_error)
{
	ExitProcess(is_error);
}



void
HandleRawInput(RAWINPUT *raw_input)
{
	switch(raw_input->header.dwType)
	{
		case RIM_TYPEKEYBOARD:
		{
			unsigned short flags = raw_input->data.keyboard.Flags;
			bool32 is_down = ((flags & RI_KEY_BREAK) == 0);

			input_keyboard_key(raw_input->data.keyboard.VKey, is_down);
		} break;
		case RIM_TYPEMOUSE:
		{
			unsigned short flag = raw_input->data.mouse.usButtonFlags;
			switch(flag)
			{
				case RI_MOUSE_LEFT_BUTTON_DOWN:
				{
					input_mouse_button(VP_MOUSE_BUTTON_LEFT, true);
				} break;
				case RI_MOUSE_RIGHT_BUTTON_DOWN:
				{
					input_mouse_button(VP_MOUSE_BUTTON_RIGHT, true);
				} break;
				
				case RI_MOUSE_MIDDLE_BUTTON_DOWN:
				{
					input_mouse_button(VP_MOUSE_BUTTON_MIDDLE, true);
				} break;
				case RI_MOUSE_BUTTON_4_DOWN:
				{
					input_mouse_button(VP_MOUSE_BUTTON_XBUTTON1, true);
				} break;
				case RI_MOUSE_BUTTON_5_DOWN:
				{
					input_mouse_button(VP_MOUSE_BUTTON_XBUTTON2, true);
				} break;
				case RI_MOUSE_LEFT_BUTTON_UP:
				{
					input_mouse_button(VP_MOUSE_BUTTON_LEFT, false);
				} break;
				case RI_MOUSE_RIGHT_BUTTON_UP:
				{
					input_mouse_button(VP_MOUSE_BUTTON_RIGHT, false);
				} break;
				case RI_MOUSE_MIDDLE_BUTTON_UP:
				{
					input_mouse_button(VP_MOUSE_BUTTON_MIDDLE, false);
				} break;
				case RI_MOUSE_BUTTON_4_UP:
				{
					input_mouse_button(VP_MOUSE_BUTTON_XBUTTON1, false);
				} break;
				case RI_MOUSE_BUTTON_5_UP:
				{
					input_mouse_button(VP_MOUSE_BUTTON_XBUTTON2, false);
				} break;
				default:
				{
					// TODO: (IMPORTANT): Handle scrolling 
				} break;
			}
		} break;
		case RIM_TYPEHID:
		{
			
		} break;

	}
};

void
Win32LoadOpenGL(HWND Window)
{
	HDC WindowDC = GetDC(Window);
	// NOTE(Vasko): desired pixel format
	PIXELFORMATDESCRIPTOR dpf = {0}; // NOTE(Vasko): not sure if this works
	dpf.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	dpf.nVersion = 1;
	dpf.dwFlags = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
	dpf.cColorBits = 32;
	dpf.cAlphaBits = 8;
	dpf.iLayerType = PFD_MAIN_PLANE;
	
	int SuggestedPixelFormatIndex = ChoosePixelFormat(WindowDC, &dpf);
	PIXELFORMATDESCRIPTOR spf;
	DescribePixelFormat(WindowDC, SuggestedPixelFormatIndex, sizeof(PIXELFORMATDESCRIPTOR), 
						&spf);
	
	
	SetPixelFormat(WindowDC, SuggestedPixelFormatIndex, &spf);
	
	HGLRC OpenGLRC = wglCreateContext(WindowDC);
	if(wglMakeCurrent(WindowDC, OpenGLRC))
	{
		// NOTE: ??
		int attribs[] = {WGL_CONTEXT_MAJOR_VERSION_ARB, 3, WGL_CONTEXT_MINOR_VERSION_ARB, 3, WGL_CONTEXT_LAYER_PLANE_ARB, 0, 
						WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB, WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB, 0};
		
		PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
		HGLRC ExtendedContext = wglCreateContextAttribsARB(WindowDC, 0,
								attribs);

		if(!wglMakeCurrent(WindowDC, ExtendedContext)) Error("Failed to make current extended gl context");

		VP_INFO("GL_VERSION %s\n", glGetString(GL_VERSION));
		GLuint BlitTextureHandle = 1;
		glGenTextures(1, &BlitTextureHandle);
		PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
		if(wglSwapIntervalEXT)
		{
			wglSwapIntervalEXT(1);
		}
		else
		{
			VP_ERROR("FAILED TO ENABLE OPENGL!");
		}
	}
	else
	{
		// TODO(Vasko): probably never gonna happen but could do something with it
		Error("faild to get opengl context!");
	}
	
	ReleaseDC(Window, WindowDC);
}


void
platform_swap_buffers()
{
	HDC DeviceContext = GetDC(Win32State->Window);  
	SwapBuffers(DeviceContext);
}


void
platform_allocate_console()
{
	AllocConsole();
}


void
platform_output_string(char *str, uint8 color)
{
	
	//		MAGIC NUMBERS! (ored rgb)
	//		13 = r | b | intense
	//		4 = r
	//		6 = r | g
	//		8 = intense
	char colors[] = {13, 4, 6, 8};
	HANDLE STDOUT = GetStdHandle(STD_OUTPUT_HANDLE);
	//		FATAL = 0
	//		ERROR = 1
	//		WARN = 2
	//		INFO = 3

	int attrib = colors[color];
	SetConsoleTextAttribute(STDOUT, attrib);
	OutputDebugStringA(str);
	unsigned long written = 0;
	WriteConsoleA(STDOUT, str, vstd_strlen(str), &written, 0);
	SetConsoleTextAttribute(STDOUT, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

bool32
platform_toggle_vsync(bool32 toggle)
{
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	if(wglSwapIntervalEXT)
	{
		wglSwapIntervalEXT(toggle);
		return true;
	}
	else
	{
		return false;
	}

}


LRESULT CALLBACK WindowProc(
							HWND   Window,
							UINT   Message,
							WPARAM wParam,
							LPARAM lParam
							)
{
	LRESULT Result = 0;
	switch(Message)
	{
		case WM_SETCURSOR:
		{
			// TODO: make a way for the engine user to set the cursro
			// probably an internal variable that can be changed by a function
//			HCURSOR Cursor = LoadCursor(NULL, IDC_ARROW);
			SetCursor(0);
		}
		case WM_SIZE:
		{
			// TODO: Make a messaging system, let the user handle this
			RECT ClientRect; 
			GetClientRect(Window, &ClientRect);
			int Width = ClientRect.right - ClientRect.left;
			int Height = ClientRect.bottom - ClientRect.top;
			glViewport(0, 0, Width, Height);

#if 0
			if(RenderBuffer.Memory != 0) VirtualFree(RenderBuffer.Memory, 0, MEM_RELEASE);
			RenderBuffer.Memory = (unsigned char *)VirtualAlloc(NULL, 
																RenderBuffer.Width * RenderBuffer.Height * RenderBuffer.BytesPerPixel, 
																MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			#endif
		}break;
		case WM_MOUSEMOVE:
		{
			if(GetActiveWindow() == Win32State->Window)
			{
				RECT WindowRect = {};
				GetWindowRect(Win32State->Window, &WindowRect);
				MapWindowPoints(HWND_DESKTOP, GetParent(Win32State->Window), (LPPOINT)&WindowRect, 2);

				int new_pos_x = WindowRect.left + ((WindowRect.right - WindowRect.left) / 2);
				int new_pos_y = WindowRect.top + ((WindowRect.bottom - WindowRect.top) / 2);

				POINT MousePos = {};
				GetCursorPos(&MousePos);
				int32 x = MousePos.x;
				int32 y = MousePos.y;


				x -= new_pos_x;
				y -= new_pos_y;
				if(x && y)
				{
					mouse.x += x;
					mouse.y += y;

					if(vp_is_keydown(VP_KEY_R))
					{
						mouse.x = 600;
						mouse.y = 400;
					}

					SetCursorPos(new_pos_x, new_pos_y);

					vp_camera_mouse_callback(mouse.x, mouse.y);
					
				}
			}
		}break;
		case WM_PAINT:
		{
			PAINTSTRUCT Paint;
			if(BeginPaint(Win32State->Window, &Paint))
			{
				int Width = Paint.rcPaint.right - Paint.rcPaint.left;
				int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
				glViewport(0, 0, Width, Height);
				EndPaint(Win32State->Window, &Paint);
			}
			
		}break;
		case WM_CLOSE:
		{
			PostQuitMessage(0);
		} break;
		
		case WM_DESTROY:
		{
			PostQuitMessage(0);
		} break;

		default:
		{
			Result = DefWindowProcA(Window, Message, wParam, lParam);
		} break;
	}
	return Result;
	
}
#endif
