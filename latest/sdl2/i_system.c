#include "../doomdef.h"
#include "../i_system.h"
#include "../i_joy.h"

#include "i_main.h"
#include <SDL2/SDL_messagebox.h>

byte graphics_started = 0;

byte keyboard_started = 0;

JoyType_t   Joystick;

void I_GetFreeMem(){}

ULONG I_GetTime(void) 
{
	return 0;
}

void I_Sleep(void){}

void I_GetEvent(void){}

void I_OsPolling(void){}

ticcmd_t *I_BaseTiccmd(void)
{
	return NULL;
}

ticcmd_t *I_BaseTiccmd2(void)
{
	return NULL;
}

void I_Quit(void)
{
	exit(0);
}

void I_Error(char *error, ...)
{
	char    str[1999];
	va_list arglist;

	va_start(arglist, error);
	vsprintf(str, error, arglist);
	va_end(arglist);

	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SRB2 Error", error, NULL);

	error = NULL;
	exit(-1);
}

byte *I_AllocLow(int length)
{
	length = 0;
	return NULL;
}

void I_Tactile(int on, int off, int total)
{
	on = 0;
	off = 0;
	total = 0;
	return;
}


void I_JoyScale(void){}

void I_JoyScale2(void){}

void I_InitJoystick(void){}

void I_InitJoystick2(void){}

int I_NumJoys(void)
{
	return 0;
}

const char *I_GetJoyName(int joyindex)
{
	joyindex = 0;
	return NULL;
}

void I_OutputMsg(char *error, ...)
{
	error = NULL;
}

void I_StartupMouse(void){}

void I_StartupMouse2(void){}

void I_StartupKeyboard(void){}

int I_GetKey(void)
{
	return 0;
}

void I_StartupTimer(void){}

void I_AddExitFunc(void (*func)())
{
	func = NULL;
}

void I_RemoveExitFunc(void (*func)())
{
	func = NULL;
}

int I_StartupSystem(void)
{
	return -1;
}

void I_ShutdownSystem(void){}

void I_GetDiskFreeSpace(INT64* freespace)
{
	freespace = NULL;
}

char *I_GetUserName(void)
{
	return NULL;
}

void I_SetWindowTitle(char* WNDTTL){}

// Stub function, just return 0s as localtime is not supported on SDL2 yet
localtime_t I_GetLocalTime(void){
	localtime_t nozo_localtime;
	memset(&nozo_localtime, 0, sizeof(localtime_t));
	return nozo_localtime; 
}

void I_StartTic(void){}

int I_mkdir(const char *dirname, int unixright)
{
	dirname = NULL;
	unixright = 0;
	return -1;
}

UINT64 I_FileSize(const char *filename)
{
	filename = NULL;
	return (UINT64)-1;
}

const char *I_LocateWad(void)
{
	return NULL;
}

void I_GetJoystickEvents(void){}

void I_GetJoystick2Events(void){}

void I_GetMouseEvents(void){}

char *I_GetEnv(const char *name)
{
	name = NULL;
	return NULL;
}

int I_PutEnv(char *variable)
{
	variable = NULL;
	return -1;
}

byte* I_ZoneBase(int* size)
{
	void* pmem;

	// do it the old way
	*size = mb_used * 1024 * 1024;
	pmem = malloc(*size);

	if (!pmem)
	{
		I_Error("Could not allocate %d megabytes.\n"
			"Please use -mb parameter and specify a lower value.\n", mb_used);
	}

	//TODO: lock the memory
	memset(pmem, 0, *size);

	return (byte*)pmem;
}
