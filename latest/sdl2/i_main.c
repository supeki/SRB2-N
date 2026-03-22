#include "../doomdef.h"
#include "../d_main.h"
#include "../m_argv.h"
#include <SDL2/SDL_rwops.h>

int mb_used = 32;

SDL_RWops* logstream;

#ifndef FORCESDLMAIN
int main(int argc, char **argv)
#else
int SDL_main(int argc, char** argv)
#endif
{
	myargc = argc;
	myargv = argv; /// \todo pull out path to exe from this string

	// startup SRB2
	CONS_Printf ("Setting up SRB2-Nozomi...\n");
	D_DoomMain();
	CONS_Printf ("Entering main game loop...\n");
	// never return
	D_DoomLoop();

	// init logstream
	logstream = SDL_RWFromFile("sdllog.txt", "w");

	// return to OS
#ifndef __GNUC__
	return 0;
#endif
}

void I_FPrintf(HANDLE fileHandle, LPCTSTR lpFmt, ...)
{
	char    str[1999];
	va_list arglist;

	va_start(arglist, lpFmt);
	wvsprintf(str, lpFmt, arglist);
	va_end(arglist);

	SDL_RWwrite(logstream, &str, sizeof(char), strlen(str));
}
