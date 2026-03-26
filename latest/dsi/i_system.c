#include <filesystem.h>
#include <nds.h>
#include <stdio.h>

#include "../doomdef.h"
#include "../d_clisrv.h"
#include "../d_main.h"
#include "../m_misc.h"
#include "../i_system.h"
#include "../i_video.h"
#include "../i_sound.h"
#include "../i_joy.h"
#include "../z_zone.h"

#include "i_main.h"

byte graphics_started = 0;

byte keyboard_started = 0;

byte mb_used = 13;

JoyType_t   Joystick;

void I_GetFreeMem(void){}

ULONG I_GetTime (void)
{
	ULONG ticks = 0;

	ticks = (ticks*TICRATE);

	ticks = (ticks/1000);

	return ticks;
}

void I_Sleep(void){}

void I_OsPolling(void){}

// Apparently this isn't system specific so Ctrl+C,Ctrl+V it is
ticcmd_t        emptycmd;
ticcmd_t* I_BaseTiccmd(void)
{
	return &emptycmd;
}


ticcmd_t *I_BaseTiccmd2(void)
{
	return NULL;
}

void I_Quit(void)
{
	M_SaveConfig(NULL);
	D_QuitNetGame();
	I_ShutdownGraphics();
	I_ShutdownSound();
	I_ShutdownMusic();
	I_ShutdownSystem();
	exit(0);
}

void I_Error(char *error, ...)
{
    // Format the error string
    va_list args;
    va_start(args, error);

    int len = vsnprintf(NULL, 0, error, args);
    va_end(args);

    char* buffer = (char*)malloc(len + 1);

    va_start(args, error);
    vsnprintf(buffer, len + 1, error, args);
    va_end(args);

    printf("SRB2 Error:\n%s\n", buffer);


    M_SaveConfig(NULL);
    D_QuitNetGame();
    I_ShutdownGraphics();
    I_ShutdownSound();
    I_ShutdownMusic();
    I_ShutdownSystem();

    while(1) 
	{
        swiWaitForVBlank();
    }
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
	va_list args;
	va_start(args, error);

	int len = vsnprintf(NULL, 0, error, args);
	va_end(args);

	char* buffer = (char*)malloc(len + 1);

	va_start(args, error);
	vsnprintf(buffer, len + 1, error, args);
	va_end(args);
}

// Just print this to the console for now
void I_LoadingScreen(const char* msg)
{}

void I_SetWindowTitle(char *WNDTTL) {}

void I_DoStartupMouse(void){}	

// Apparently, despite its name this is called whenever cv_usemouse changes
// Why not I_ToggleMouse???
void I_StartupMouse(void){}

void I_StartupMouse2(void){}

void I_StartupKeyboard(void){}

int I_GetKey(void)
{
	return 0;
}

void I_StartTic(void){}

void I_GetEvent(void){}

// Translate SDL2's events in Doom Legacy ones (keyboard and mouse input)
void I_StartFrame(void)
{
	scanKeys();
	
	event_t e_w;
	const char *key_names[15] = {
        "A", "B", "Select", "Start", "Right", "Left", "Up", "Down", "R",
        "L", "X", "Y", "Touch", "Lid", "Debug"
    };

	for (int i = 0; i <= 14; i++)
    {
		switch(i)
		{
			case 7:
				e_w.data1 = KEY_UPARROW;
				break;
			case 8:
				e_w.data1 = KEY_DOWNARROW;
				break;
			case 6:
				e_w.data1 = KEY_LEFTARROW;
				break;
			case 5:
				e_w.data1 = KEY_RIGHTARROW;
				break;
			case 3:
				e_w.data1 = KEY_MENU;
				break;
			case 1:
				e_w.data1 = KEY_SPACE;
				break;
			case 2:
				e_w.data1 = KEY_SHIFT;
				break;
		}
    }
}

void I_GetDiskFreeSpace(INT64 *freespace){}
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
	videoSetMode(MODE_0_2D);
    videoSetModeSub(MODE_0_2D);

    vramSetBankA(VRAM_A_MAIN_BG);
    vramSetBankC(VRAM_C_SUB_BG);

	return 0;
}

void I_ShutdownSystem(void){
	CONS_Printf("I_ShutdownSystem...\n");
}

char *I_GetUserName(void)
{
	return NULL;
}

int I_mkdir(const char *dirname, int unixright)
{
	dirname = NULL;
	unixright = 0;
	return -1;
}


const char *I_LocateWad(void)
{
	return "nitrofs:/";
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

// Stub that returns a struct full of 0s
// Save 23-03-2026
localtime_t I_GetLocalTime(void) {
	localtime_t nozo_localtime;
	memset(&nozo_localtime, 0, sizeof(localtime_t));
	return nozo_localtime;
}