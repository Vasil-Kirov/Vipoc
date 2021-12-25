#ifdef VIPOC_WIN32

#define WIN32_LEAN_AND_MEAN
// might be able to remove Windows.h
#include <Windows.h>
#include <hidsdi.h>

#include "platform/platform.h"
#include "log.h"
#include "renderer/renderer.h"
#include "application.h"
#include "input.h"
#include "include/Core.h"

typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC)(int interval);
typedef BOOL (WINAPI * PFNWGLCHOOSEPIXELFORMATARBPROC)(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef HGLRC WINAPI wglCreateContextAttribsARB_type(HDC hdc, HGLRC hShareContext,
        const int *attribList);
wglCreateContextAttribsARB_type *wglCreateContextAttribsARB;
wglCreateContextAttribsARB_type *wglCreateContextAttribsARB;

#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001

typedef BOOL WINAPI wglChoosePixelFormatARB_type(HDC hdc, const int *piAttribIList,
        const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
wglChoosePixelFormatARB_type *wglChoosePixelFormatARB;

// See https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_pixel_format.txt for all values
#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_ACCELERATION_ARB                      0x2003
#define WGL_SUPPORT_OPENGL_ARB                    0x2010
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_PIXEL_TYPE_ARB                        0x2013
#define WGL_COLOR_BITS_ARB                        0x2014
#define WGL_DEPTH_BITS_ARB                        0x2022
#define WGL_STENCIL_BITS_ARB                      0x2023

#define WGL_FULL_ACCELERATION_ARB                 0x2027
#define WGL_TYPE_RGBA_ARB                         0x202B


typedef void (*vp_on_button_down)(vp_keys key, bool32 is_down);


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
internal vp_on_button_down button_callback;

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
	button_callback = game.vp_on_key_down;
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


bool32
platform_read_entire_file(char *path, entire_file *e_file)
{
	HANDLE File = CreateFileA(path, GENERIC_READ, 0, vp_nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, vp_nullptr);
	if(File == INVALID_HANDLE_VALUE) return false;
	e_file->size = GetFileSize(File, vp_nullptr);
	ReadFile(File, e_file->contents, e_file->size, vp_nullptr, vp_nullptr);
	CloseHandle(File);
	if (e_file->size > 0) return true;
	return false;
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
	if(!vp_is_mbdown(VP_MOUSE_BUTTON_LEFT))
	{
		POINT MousePos = {};
		GetCursorPos(&MousePos);
		vp_update_mouse_pos(MousePos.x, MousePos.y);
	}


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
			if(button_callback != vp_nullptr) button_callback(raw_input->data.keyboard.VKey, is_down);
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


/* Microsoft are the devil incarnate */ 
void
LoadOpenGLExtensions()
{
	WNDCLASSA window_class = {
        .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .lpfnWndProc = DefWindowProcA,
        .hInstance = GetModuleHandle(0),
        .lpszClassName = "Dummy_WGL_djuasiodwa",
    };

    if (!RegisterClassA(&window_class)) {
        Error("Failed to register dummy OpenGL window.");
    }
	    HWND dummy_window = CreateWindowExA(
        0,
        window_class.lpszClassName,
        "Dummy OpenGL Window",
        0,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        window_class.hInstance,
        0);

    if (!dummy_window) {
        Error("Failed to create dummy OpenGL window.");
    }

    HDC dummy_dc = GetDC(dummy_window);

	PIXELFORMATDESCRIPTOR pfd = {
        .nSize = sizeof(pfd),
        .nVersion = 1,
        .iPixelType = PFD_TYPE_RGBA,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER ,
        .cColorBits = 32,
        .cAlphaBits = 8,
        .iLayerType = PFD_MAIN_PLANE,
        .cDepthBits = 24,
        .cStencilBits = 8,
    };
	int SuggestedPixelFormatIndex = ChoosePixelFormat(dummy_dc, &pfd);
	DescribePixelFormat(dummy_dc, SuggestedPixelFormatIndex, sizeof(PIXELFORMATDESCRIPTOR), 
						&pfd);
	if(!SetPixelFormat(dummy_dc, SuggestedPixelFormatIndex, &pfd)) Error("Failed to set pixel format for the dummy context");
	HGLRC OpenGLRC = wglCreateContext(dummy_dc);
	if(!wglMakeCurrent(dummy_dc, OpenGLRC)) Error("Failed to make current the dummy context");

	wglCreateContextAttribsARB = (wglCreateContextAttribsARB_type*)wglGetProcAddress(
        "wglCreateContextAttribsARB");
    wglChoosePixelFormatARB = (wglChoosePixelFormatARB_type*)wglGetProcAddress(
        "wglChoosePixelFormatARB");

	wglMakeCurrent(dummy_dc, 0);
	wglDeleteContext(OpenGLRC);
	ReleaseDC(dummy_window, dummy_dc);
	DestroyWindow(dummy_window);
}


void
Win32LoadOpenGL(HWND Window)
{
	LoadOpenGLExtensions();

	HDC WindowDC = GetDC(Window);

	int pixel_format_attribs[] = {
		WGL_DRAW_TO_WINDOW_ARB,		GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB,		GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB,		GL_TRUE,
		WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
		WGL_ACCELERATION_ARB,		WGL_FULL_ACCELERATION_ARB,
		WGL_PIXEL_TYPE_ARB,			WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB, 		32,
		WGL_DEPTH_BITS_ARB, 		24,
		WGL_STENCIL_BITS_ARB, 		0,
		WGL_SAMPLES_ARB, 			4,
		0
	};
	int pixel_format;
	UINT num_formats;

	wglChoosePixelFormatARB(WindowDC, pixel_format_attribs, 0, 1, &pixel_format, &num_formats);
	if(!num_formats)
	{
		Error("Failed to choose pixelformat!");
	}

	PIXELFORMATDESCRIPTOR pfd;
	DescribePixelFormat(WindowDC, pixel_format, sizeof(pfd), &pfd);
	
	if(!SetPixelFormat(WindowDC, pixel_format, &pfd))
	{
		Error("Failed to set the OpenGL 3.3 pixel format.");
	}
	int gl33_attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };
	HGLRC gl33_context = wglCreateContextAttribsARB(WindowDC, 0, gl33_attribs);
	if (!gl33_context)
	{
        Error("Failed to create OpenGL 3.3 context.");
    }

	if(!wglMakeCurrent(WindowDC, gl33_context))
	{
		Error("Failed to activate OpenGL 3.3 rendering context.");
	}

	VP_INFO("GL_VERSION %s\n", glGetString(GL_VERSION));

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
	WriteFile(STDOUT, str, vstd_strlen(str), &written, 0);
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


const char *
platform_get_sharable_extension()
{
	return ".dll";
}

void
platform_free_sharable(platform_sharable sharable)
{
	FreeLibrary((HMODULE)sharable.sharable);
}

void *
platform_get_function_from_sharable(platform_sharable sharable, const char *func_name)
{
	return GetProcAddress((HMODULE)sharable.sharable, func_name);
}

platform_sharable
platform_load_sharable(const char *path)
{
	platform_sharable sharable = {};
	sharable.sharable = (void *)LoadLibraryA(path);
	return sharable;
}

bool32
platform_copy_file(const char *old_path, const char *new_path)
{
	if(CopyFileA(old_path, new_path, FALSE) == 0) return false;
	return true;
}


platform_thread
platform_create_thread(void *func, void *parameter)
{
	platform_thread thread;
	thread.data = (void *)CreateThread(vp_nullptr, 0, func, parameter, 0, 0);
	return thread;
}

bool32
platform_wait_for_thread(platform_thread thread)
{
	if(thread.data == vp_nullptr) return false;

	DWORD Result = WaitForSingleObject((HANDLE)thread.data, INFINITE);
	if(Result == WAIT_FAILED || Result == WAIT_ABANDONED) return false;
	return true;
}


// I trust you Reymond Chen
void
platform_switch_fullscreen()
{
	WINDOWPLACEMENT window_placement = {};
	window_placement.length	= sizeof(WINDOWPLACEMENT);
	window_placement.flags	= WPF_RESTORETOMAXIMIZED;
	window_placement.showCmd= SW_SHOWMAXIMIZED;

	DWORD dwStyle = GetWindowLong(Win32State->Window, GWL_STYLE);
	if (dwStyle & WS_OVERLAPPEDWINDOW) {
		MONITORINFO mi = { sizeof(mi) };
		if (GetWindowPlacement(Win32State->Window, &window_placement) &&
			GetMonitorInfo(MonitorFromWindow(Win32State->Window,
						MONITOR_DEFAULTTOPRIMARY), &mi)) {
		SetWindowLong(Win32State->Window, GWL_STYLE,
						dwStyle & ~WS_OVERLAPPEDWINDOW);
		SetWindowPos(Win32State->Window, HWND_TOP,
					mi.rcMonitor.left, mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left,
					mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	} else {
		SetWindowLong(Win32State->Window, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(Win32State->Window, &window_placement);
		SetWindowPos(Win32State->Window, NULL, 0, 0, 0, 0,
					SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
					SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
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
			// TODO: make a way for the engine user to set the cursor
			// probably an internal variable that can be changed by a function
			HCURSOR Cursor = LoadCursor(NULL, IDC_ARROW);
			SetCursor(Cursor);
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
			if(GetActiveWindow() == Win32State->Window && vp_is_mbdown(VP_MOUSE_BUTTON_LEFT))
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
                
				if(x != new_pos_x && y != new_pos_y)
				{
					mouse.x += x;
					mouse.y += y;
                    
					if(vp_is_keydown(VP_KEY_R))
					{
						mouse.x = 600;
						mouse.y = 400;
					}
                    
//					SetCursorPos(new_pos_x, new_pos_y);
                    
					vp_camera_mouse_callback(x, y);
					
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
