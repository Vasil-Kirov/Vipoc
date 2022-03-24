#include <debug.h>
#include <log.h>
#include <renderer/renderer.h>

typedef struct timer_info
{
	u32 id;
	u64 value;
	const char *name;
} timer_info;


typedef struct debug_state
{
	timer_info timers[MAX_TIMERS];
	timer_info draw_timers[MAX_TIMERS];
	u16 number_of_diagrams;
} debug_state;
internal debug_state debug;



u64
vp_debug_timer()
{
#if defined(VIPOC_DEBUG)
	
#if defined(__clang__) || defined(__GNUC__)
	return __builtin_ia32_rdtsc();
#else
	return __rdtsc();
#endif
#else
	return 0;
#endif
}


i32
find_valid_index(u32 id, const char *name)
{
	i32 index = id % MAX_TIMERS;
	if(debug.timers[index].id != 0 && debug.timers[index].id != id)
	{
		VP_ERROR("A HASH COLLISION HAS OCCURRED: %s with %s! THIS MIGHT RESULT IN A MEMORY LEAK", name, debug.timers[index].name);
		while(debug.timers[index].id != 0 && debug.timers[index].id != id)
		{
			index++;
		}
	}
	return index;
}

void
vp_start_debug_timer(const char *name, u32 id)
{
#if defined(VIPOC_DEBUG)
	i32 index = find_valid_index(id, name);
	debug.timers[index].value = vp_debug_timer();
	debug.timers[index].id = id;
	debug.timers[index].name = name;
#endif
}

void
vp_stop_debug_timer(u32 id)
{
#if defined(VIPOC_DEBUG)
	i32 index = find_valid_index(id, "");
	debug.draw_timers[index].value += vp_debug_timer() - debug.timers[index].value;
	debug.draw_timers[index].id = debug.timers[index].id;
	debug.draw_timers[index].name = debug.timers[index].name;
#endif
}


u16
count_diagrams()
{
	u16 counter = 0;
	for(u16 index = 0; index < MAX_TIMERS; ++index)
	{
		if(debug.draw_timers[index].id != 0) counter++;
	}
	return counter;
}

void
vp_draw_diagrams()
{
#if defined(VIPOC_DEBUG)
	debug.number_of_diagrams = count_diagrams();
	if(debug.number_of_diagrams <= 0) return;
	START_DTIMER();
	u64 total_timer = 0;
	for(int index = 0; index < MAX_TIMERS; ++index)
	{
		total_timer += debug.draw_timers[index].value;
	}
	
	f32 diagram_height = (5.0f * 0.25f)/debug.number_of_diagrams;
	f32 diagram_width = 5.0f;
	
	vp_draw_rectangle((m2){0, 0, diagram_width, 5.0f*.25f}, (v4){0.7f, 0.1f, 0.1f, 0.3f}, 0);
	
	int number_of_drawn = 0;
	for(int index = 0; index < MAX_TIMERS; ++index)
	{
		if(debug.draw_timers[index].id == 0) continue;
		else number_of_drawn++;
		
		m2 location = {};
		location.y1 = diagram_height * (number_of_drawn-1);
		location.y2 = diagram_height * (number_of_drawn);
		location.x2 = double_normalize_between((f64)debug.draw_timers[index].value, 0.0f, (f64)total_timer, 0.0f, diagram_width);
		char text_to_draw[1024] = {};
		vstd_sprintf(text_to_draw, "%s %llu clock cycles", debug.draw_timers[index].name, debug.draw_timers[index].value);
		
		vp_draw_rectangle(location, (v4){0.0f, 0.3f, 0.7f, 1.0f}, 1);
		vp_draw_text(text_to_draw, location.x1+0.1f, location.y1+diagram_height/4.0f, 0xFFFFFFFF, 0.5f, 2);
		debug.draw_timers[index].id		= 0;
		debug.draw_timers[index].value 	= 0;
		debug.draw_timers[index].name	= 0;
	}
	debug.number_of_diagrams = 0;
	STOP_DTIMER();
#endif
}

void
vp_reset_debug_timers()
{
	memset(debug.timers, 0, sizeof(timer_info) * MAX_TIMERS);
}

