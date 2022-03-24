/* date = March 3rd 2022 3:34 pm */

#ifndef CONSOLE_H
#define CONSOLE_H

void
ConsoleAddChar(char c);

void
ConsoleDeleteChar();

b32
ConsoleIsOn();

void
HandleConsoleProgression();

void
StopConsole();

void
HandleCommand(char *Command);

void
StartConsole();

#endif //CONSOLE_H
