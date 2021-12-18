#if defined VIPOC_LINUX

#define _XOPEN_SOURCE 500
#define GLX_GLXEXT_PROTOTYPES

#include <X11/Xlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <dlfcn.h>
#include <errno.h>


#include "platform/platform.h"
#include "application.h"
#include "renderer/renderer.h"
#include <GL/glx.h>



#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"

#define ANSI_COLOR_RESET   "\x1b[0m"


typedef struct linux_state
{
	Display *display;
	Window window;
	XEvent event;
	int screen;
	struct timespec start_time;
} linux_state;
internal linux_state *internal_state;

int error_handler(Display * d, XErrorEvent * e);

bool32
platform_init(vp_config config, platform_state *pstate)
{
	clock_gettime(CLOCK_REALTIME, &internal_state->start_time);

	pstate->state = platform_allocate_memory_chunk(sizeof(linux_state));
	internal_state = (linux_state *)pstate->state; 


	internal_state->display = XOpenDisplay(NULL);
	if(!internal_state->display)
	{
		ERROR("Failed to open display");
	}
	
	internal_state->screen = DefaultScreen(internal_state->display);

	internal_state->window = XCreateSimpleWindow(internal_state->display, RootWindow(internal_state->display, internal_state->screen), 
												config.x, config.y, config.w, config.h, 1, 
												BlackPixel(internal_state->display, internal_state->screen),
												WhitePixel(internal_state->display, internal_state->screen));


	XSelectInput(internal_state->display,  internal_state->window, ExposureMask | KeyPressMask | KeyReleaseMask | LeaveWindowMask);
	XMapWindow(internal_state->display, internal_state->window);


	if(!LinuxLoadOpenGL(internal_state->display, internal_state->screen)) return false;
	XSetErrorHandler(error_handler);

	return true;

}

int error_handler(Display * d, XErrorEvent * e)
{
	VP_ERROR("XError: %d\nRequest: %d\n", e->error_code, e->request_code);
    return 0;
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


void
platform_swap_buffers()
{
	glXSwapBuffers(internal_state->display, glXGetCurrentDrawable());
}

void
platform_output_string(char *str, uint8 color)
{
	char output[4096] = {};
	char *colors[4] = {ANSI_COLOR_MAGENTA, ANSI_COLOR_RED, ANSI_COLOR_YELLOW, ANSI_COLOR_CYAN};
	vstd_strcat(output, colors[color]);
	vstd_strcat(output, str);
	vstd_strcat(output, ANSI_COLOR_RESET);
	vstd_printf(output);
}

void
platform_allocate_console()
{

}

uint64
platform_get_size_of_file(char *path)
{
    struct stat stat_buf;
    int rc = stat(path, &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}


bool32
platform_read_entire_file(char *path, entire_file *e_file)
{
	int file = open(path, O_RDONLY);
	if(file == -1) return false;
	e_file->size = platform_get_size_of_file(path);
	if(read(file, e_file->contents, e_file->size) == -1) return false;
	close(file);
	return true;
}

void
platform_get_absolute_path(char *output)
{
	readlink("/proc/self/exe", output, PATH_MAX);

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
platform_exit(bool32 is_error)
{
	_exit(is_error);
}

int
platform_get_width()
{
	XWindowAttributes status = {};
	XGetWindowAttributes(internal_state->display, internal_state->window, &status);	
	return(status.width);
}

int
platform_get_height()
{
	XWindowAttributes status = {};
	XGetWindowAttributes(internal_state->display, internal_state->window, &status);	
	return(status.height);
}

uint32
platform_get_ms_since_start()
{
	struct timespec time = {};
	clock_gettime(CLOCK_REALTIME, &time);
	return (internal_state->start_time.tv_nsec - time.tv_nsec) / 1000000;
}

bool32
platform_toggle_vsync(bool32 toggle)
{
	glXSwapIntervalEXT(internal_state->display, internal_state->window, toggle);
}

platform_sharable
platform_load_sharable(const char *path)
{
	platform_sharable sharable;
	sharable.sharable = dlopen(path, RTLD_NOW);
	return sharable;
};

void *
platform_get_function_from_sharable(platform_sharable sharable, const char *func_name)
{
	return glXGetProcAddress(func_name);
}

void
platform_free_sharable(platform_sharable sharable)
{
	dlclose(sharable.sharable);
}

const char *
platform_get_sharable_extension()
{
	return ".so";
}


int _read_write_copy(const char *to, const char *from)
{
    int fd_to, fd_from;
    char buf[4096];
    ssize_t nread;
    int saved_errno;

    fd_from = open(from, O_RDONLY);
    if (fd_from < 0)
        return -1;

    fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd_to < 0)
        goto out_error;

    while (nread = read(fd_from, buf, sizeof buf), nread > 0)
    {
        char *out_ptr = buf;
        ssize_t nwritten;

        do {
            nwritten = write(fd_to, out_ptr, nread);

            if (nwritten >= 0)
            {
                nread -= nwritten;
                out_ptr += nwritten;
            }
            else if (errno != EINTR)
            {
                goto out_error;
            }
        } while (nread > 0);
    }

    if (nread == 0)
    {
        if (close(fd_to) < 0)
        {
            fd_to = -1;
            goto out_error;
        }
        close(fd_from);

        /* Success! */
        return 0;
    }

  out_error:
    saved_errno = errno;

    close(fd_from);
    if (fd_to >= 0)
        close(fd_to);

    errno = saved_errno;
    return -1;
}


VP_API bool32
platform_copy_file(const char *old_path, const char *new_path)
{
	if(_read_write_copy(new_path, old_path) == -1) return false;
	return true;
}

#endif
