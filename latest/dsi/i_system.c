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
byte mb_used = 12;

JoyType_t   Joystick;

void I_GetFreeMem(void){}

#define timers2ms(tlow,thigh) ((tlow>>5)+(thigh<<11))

// Handy DSdev.org timer functions
u32 GetTicks(void)
{
	return timers2ms(TIMER0_DATA, TIMER1_DATA);
} 

void Pause(u32 ms)
{
	u32 now;
	now=timers2ms(TIMER0_DATA, TIMER1_DATA);
	while((u32)timers2ms(TIMER0_DATA, TIMER1_DATA)<now+ms);
}


void I_Sleep(unsigned long usecs)
{
	Pause(usecs/1000);
}

int ms_to_next_tick;

ULONG I_GetTime (void)
{
  int t = GetTicks();
  int i = t*(TICRATE/5)/200;
  ms_to_next_tick = (i+1)*200/(TICRATE/5) - t;
  if (ms_to_next_tick > 1000/TICRATE || ms_to_next_tick<1) ms_to_next_tick = 1;
  return i;
}

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

void I_StartTic(void)
{
}

void I_GetEvent(void){}

// Translate the DSi's events in Doom Legacy ones (keyboard and touch input)
void I_StartFrame(void)
{
	scanKeys();
	u16 keys = keysDown();
	
	event_t e_w;

	if (keys & KEY_A) {
		event_t event;
		event.type = ev_keydown;
		event.data1 = KEY_ENTER;
		D_PostEvent(&event);
	}
	
	if (keys & KEY_B) {
		event_t event;
		event.type = ev_keydown;
		event.data1 = KEY_ESCAPE;
		D_PostEvent(&event);
	}

	if (keys & KEY_UP) {
		event_t event;
		event.type = ev_keydown;
		event.data1 = KEY_UPARROW;
		D_PostEvent(&event);
	}
	
	if (keys & KEY_DOWN) {
		event_t event;
		event.type = ev_keydown;
		event.data1 = KEY_DOWNARROW;
		D_PostEvent(&event);
	}

	if (keys & KEY_LEFT) {
		event_t event;
		event.type = ev_keydown;
		event.data1 = KEY_LEFTARROW;
		D_PostEvent(&event);
	}
	
	if (keys & KEY_RIGHT) {
		event_t event;
		event.type = ev_keydown;
		event.data1 = KEY_RIGHTARROW;
		D_PostEvent(&event);
	}

	if (keys & KEY_TOUCH) { // not sure if this works yet
		touchPosition touch_pos;
		e_w.type = ev_mouse;
		e_w.data2 = touch_pos.px * (cv_mousesens.value + 1) / 10;
		e_w.data3 = touch_pos.py * (cv_mousesens.value + 1) / 10;
		D_PostEvent(&e_w);
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