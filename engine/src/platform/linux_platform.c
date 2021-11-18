#ifdef VIPOC_LINUX


#include <X11/Xlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>


#include "platform/platform.h"
#include "renderer/renderer.h"
#include <GL/glx.h>


typedef struct linux_state
{
	Display *display;
	Window window;
	XEvent event;
	int screen;
} linux_state;
internal linux_state *internal_state;


bool32
platform_init(vp_config game, platform_state *pstate)
{
	
	pstate->state = platform_allocate_memory_chunk(sizeof(linux_state));
	internal_state = (linux_state *)pstate->state; 


	internal_state->display = XOpenDisplay(NULL);
	if(!internal_state->display)
	{
		ERROR("Failed to open display");
	}
	
	internal_state->screen = DefaultScreen(internal_state->display);

	internal_state->window = XCreateSimpleWindow(internal_state->display, RootWindow(internal_state->display, internal_state->screen), 
												game.x, game.y, game.w, game.h, 1, 
												BlackPixel(internal_state->display, internal_state->screen),
												WhitePixel(internal_state->display, internal_state->screen));


	XSelectInput(internal_state->display,  internal_state->window, ExposureMask | KeyPressMask | KeyReleaseMask | LeaveWindowMask);
	XMapWindow(internal_state->display, internal_state->window);


	if(!LinuxLoadOpenGL(internal_state->display, internal_state->screen)) return false;

	return true;

}


bool32
platform_handle_message()
{
	XNextEvent( internal_state->display, &(internal_state->event) );
	if(internal_state->event.type == LeaveNotify) 
	{
		return false;
	}

	return true;
}


#define MAP_ANONYMOUS 0x20
void *
platform_allocate_memory_chunk(uint64 size)
{
	// @NOTE: Might be a good idea to use MAP_PRIVATE instead of MAP_SHARED idk
	return mmap(vp_nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
}


uint64
platform_get_size_of_file(char *path)
{
	struct stat status;
	stat(path, &status);
	return status.st_size;
}


bool32
LinuxLoadOpenGL(Display *display, int screen)
{
		const int fbCfgAttribslist[] =
		{
			GLX_RENDER_TYPE, GLX_RGBA_BIT,
			GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT ,
			None
		};
		// GLX_WINDOW_BIT
	int nElements = 0;
	GLXFBConfig * glxfbCfg = glXChooseFBConfig( display,
									  screen,
									  fbCfgAttribslist,
									  & nElements );


	if(glxfbCfg == vp_nullptr) return false;

	const int pfbCfg[] =
	{
		GLX_PBUFFER_WIDTH, 600,
		GLX_PBUFFER_HEIGHT, 400,
		GLX_PRESERVED_CONTENTS, True,
		GLX_LARGEST_PBUFFER, False,
		None
	};

	GLXPbuffer pBufferId = glXCreatePbuffer( display, glxfbCfg[ 0 ], pfbCfg );

	if(pBufferId == vp_nullptr) return false;

	XVisualInfo * visInfo = glXGetVisualFromFBConfig( display, glxfbCfg[ 0 ] );

	GLXContext  glCtx = glXCreateContext( display, visInfo, NULL, True );

	if(glCtx == vp_nullptr) return false;

	glXMakeContextCurrent( display,
					   pBufferId,
					   pBufferId,
					   glCtx );
	return true;
}


#endif
