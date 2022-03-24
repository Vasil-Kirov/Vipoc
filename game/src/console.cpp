#include <console.h>

internal console Console;

void
ConsoleAddChar(char C)
{
	if(C != '\n' && C != '\r' && C != '\t' && C != '\v')
		Console.Command[Console.LastChar++] = C;
}

void
ConsoleDeleteChar()
{
	if(Console.LastChar > 0)
		Console.Command[--Console.LastChar] = 0;
}

b32
ConsoleIsOn()
{
	return Console.IsOn;
}


void
HandleConsoleProgression()
{
	Console.Position -= 4.0f * vp_get_dtime();
	if(Console.Position <= 3.5f)
	{
		Console.IsStarting = false;
		Console.IsOn = true;
	}
}

void
StopConsole()
{
	Console.Position = 0;
	Console.IsOn = false;
	memset(Console.Command, 0, Console.LastChar);
	Console.LastChar = 0;
}

void
StartConsole()
{
	Console.IsStarting = true;
	Console.Position = 5.625f;
}

void
HandleCommand(char *Command)
{
	VP_INFO("Command: %s", Command);
	if(memcmp(Command, (void *)"move_piece", sizeof("move_piece")) == 0)
	{
		vp_move_static_entity(3, (v3){ 35, 35, 35 });
	}
}

