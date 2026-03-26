#include "../doomdef.h"
#include "../d_main.h"
#include "../m_argv.h"

#include <nds.h>
#include <filesystem.h>

int mb_used = 32;

int main(int argc, char **argv)
{
	myargc = argc;
	myargv = argv; /// \todo pull out path to exe from this string

	consoleDemoInit();

	// start NitroFS
	nitroFSInit(NULL);
    chdir("nitrofs:/");

	// startup SRB2
	CONS_Printf("Setting up SRB2-Nozomi...\n");
	D_DoomMain();
	CONS_Printf("Entering main game loop...\n");
	// never return
	D_DoomLoop();


	// return to OS
#ifndef __GNUC__
	return 0;
#endif
}

void I_FPrintf(FILE *fileHandle, const char *lpFmt, ...)
{
	char    str[1999];
	va_list arglist;

	va_start(arglist, lpFmt);
	vsprintf(str, lpFmt, arglist);
	va_end(arglist);
}
