#include "../command.h"
#include "../doomdef.h"
#include "../i_system.h"
#include "../i_video.h"
#include "../screen.h"
#include "../z_zone.h"

#include "i_video.h"

rendermode_t rendermode = render_soft;

boolean highcolor = false;

boolean allow_fullscreen = false;

// Stores the current mode
int currentmode;

// Various unction protos
void I_ToggleFullscreen(void);
boolean VID_InitConsole(void);
void VID_Command_Vidmode(void);
void VID_Command_Listmodes(void);

// todo: un-hardcode this
#define NUM_SDLMODES 11

vmode_t window_modes[NUM_SDLMODES] = {
		// Fallback mode, 320x200 is gross
		{
			NULL,
			"320x200", //faB: W to make sure it's the windowed mode
			320, 200,   //(200.0/320.0)*(320.0/240.0),
			320, 1,     // rowbytes, bytes per pixel
			1, 2,       // windowed (TRUE), numpages
			NULL,
			NULL,
			0          // misc
		},
		// Non-fallback copy of 320x200W, if you WANT to use 320x200W for some reason
		{
			NULL,
			"320x200", //faB: W to make sure it's the windowed mode
			320, 200,   //(200.0/320.0)*(320.0/240.0),
			320, 1,     // rowbytes, bytes per pixel
			1, 2,       // windowed (TRUE), numpages
			NULL,
			NULL,
			0          // misc
		},
		{
			NULL,
			"320x240", //faB: W to make sure it's the windowed mode
			320, 240,   //(200.0/320.0)*(320.0/240.0),
			320, 1,     // rowbytes, bytes per pixel
			1, 2,       // windowed (TRUE), numpages
			NULL,
			NULL,
			0          // misc
		},
		{
			NULL,
			"640x400", //faB: W to make sure it's the windowed mode
			640, 400,   //(200.0/320.0)*(320.0/240.0),
			640, 1,     // rowbytes, bytes per pixel
			1, 2,       // windowed (TRUE), numpages
			NULL,
			NULL,
			0          // misc
		},
		{
			NULL,
			"640x480", //faB: W to make sure it's the windowed mode
			640, 480,   //(200.0/320.0)*(320.0/240.0),
			640, 1,     // rowbytes, bytes per pixel
			1, 2,       // windowed (TRUE), numpages
			NULL,
			NULL,
			0          // misc
		},
		{
			NULL,
			"800x600", //faB: W to make sure it's the windowed mode
			800, 600,   //(200.0/320.0)*(320.0/240.0),
			800, 1,     // rowbytes, bytes per pixel
			1, 2,       // windowed (TRUE), numpages
			NULL,
			NULL,
			0          // misc
		},
		{
			NULL,
			"1024x768", //faB: W to make sure it's the windowed mode
			1024, 768,   //(200.0/320.0)*(320.0/240.0),
			1024, 1,     // rowbytes, bytes per pixel
			1, 2,       // windowed (TRUE), numpages
			NULL,
			NULL,
			0          // misc
		},
		{
			NULL,
			"1280x720", //faB: W to make sure it's the windowed mode
			1280, 720,   //(200.0/320.0)*(320.0/240.0),
			1280, 1,     // rowbytes, bytes per pixel
			1, 2,       // windowed (TRUE), numpages
			NULL,
			NULL,
			0          // misc
		},
		{
			NULL,
			"1280x800", //faB: W to make sure it's the windowed mode
			1280, 800,   //(200.0/320.0)*(320.0/240.0),
			1280, 1,     // rowbytes, bytes per pixel
			1, 2,       // windowed (TRUE), numpages
			NULL,
			NULL,
			0          // misc
		},
		{
			NULL,
			"1920x1080", //faB: W to make sure it's the windowed mode
			1920, 1080,   //(200.0/320.0)*(320.0/240.0),
			1920, 1,     // rowbytes, bytes per pixel
			1, 2,       // windowed (TRUE), numpages
			NULL,
			NULL,
			0          // misc
		},
		{
			NULL,
			"1920x1200", //faB: W to make sure it's the windowed mode
			1920, 1200,   //(200.0/320.0)*(320.0/240.0),
			1920, 1,     // rowbytes, bytes per pixel
			1, 2,       // windowed (TRUE), numpages
			NULL,
			NULL,
			0          // misc
		},
};

void I_ToggleFullscreen(void) {
	VID_SetMode(currentmode);
}

void I_ShutdownGraphics(void){
	CONS_Printf("I_ShutdownGraphics...\n");
	free(vid.buffer);
	graphics_started = false;
}

// Translate Doom palette into SDL palette
void I_SetPalette(byte *palette){}

int VID_NumModes(void)
{
	// TODO: Find a way to get length of windowed_modes and return that
	return NUM_SDLMODES;
}

int VID_GetModeForSize(int w, int h)
{
	int diffx, diffy;
	int bestdiff, bestmode;

	bestdiff = 9999;

	// Iterate through (accessible) modes
	for (int i = 1; i < NUM_SDLMODES; i++) {

		// Get difference between current window mode's width and our requested width
		diffx = window_modes[i].width - w;
		if (diffx < 0) {
			diffx *= -1;
		}

		// Get difference between current window mode's height and our requested height
		diffy = window_modes[i].height - h;
		if (diffy < 0) {
			diffy *= -1;
		}

		// If the mode has the exact coords of our dimensions, use that!
		if (diffx == 0 && diffy == 0)
			return i;

		// If our current mode is the closest found so far to requested dimensions, record it as our best candidate so far
		if (bestdiff > diffx + diffy) {
			bestdiff = diffx + diffy;
			bestmode = i;
		}
	}

	// Well, we didn't find any exact matches. Darn.
	// Oh well, that's what that whole closest match system was for!
	return bestmode;
}

// TODO: Some of the stuff done when we change modes might not be needed. See how much we can keep between mode switches (For example, we might just be able to change the window's dimensions instead of destroying and remaking it)
int VID_SetMode(int modenum)
{
	return 0;
}

void I_StartupGraphics(void) {
	if (VID_InitConsole())
		I_Error("I_StartupGraphics(): Could not initialize commands / console variables!\n");

	VID_SetMode(3);

	graphics_started = true;
}

const char *VID_GetModeName(int modenum)
{
	return window_modes[modenum].name;
}

void I_UpdateNoBlit(void){}

void I_FinishUpdate(void){}

void I_WaitVBL(int count)
{
	count = 0;
}

void I_ReadScreen(byte *scr){}

void I_BeginRead(void){}

void I_EndRead(void){}


// COMMAND STUFF

boolean VID_InitConsole(void) {
	// Register cvars
	CV_RegisterVar(&cv_fullscreen);

	// Register commands
	COM_AddCommand("videomode", VID_Command_Vidmode);
	COM_AddCommand("listmodes", VID_Command_Listmodes);

	return 0;
}

void VID_Command_Vidmode(void) {

	int modenum;

	if (COM_Argc() != 2) {
		CONS_Printf("videomode <mode number>: Changes the video mode to the specified one. Number must be between 1 and %d.\n", NUM_SDLMODES - 1);
		return;
	}

	modenum = atoi(COM_Argv(1));

	if (modenum < 1 || modenum > NUM_SDLMODES - 1) {
		CONS_Printf("Invalid video mode \"%d\"! Must be between 1 and %d.\n", modenum, NUM_SDLMODES - 1);
		return;
	} else {
		VID_SetMode(modenum);
	}

	CONS_Printf("Changed to video mode %s (index %d).\n", VID_GetModeName(modenum), modenum);
}

void VID_Command_Listmodes(void) {
	for (int i = 1; i < NUM_SDLMODES; i++) {
		CONS_Printf("Video mode %d: %s.\n", i, VID_GetModeName(i));
	}
}