#include "../command.h"
#include "../doomdef.h"
#include "../i_system.h"
#include "../i_video.h"
#include "../screen.h"
#include "../z_zone.h"

#include "i_video.h"

#include <gl2d.h>

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

u16 ds_palette[256];

// Translate Doom palette into ??? palette
void I_SetPalette(byte *palette)
{
    for (int i = 0; i < 256; i++)
    {
        u8 r = palette[i*3+0];
        u8 g = palette[i*3+1];
        u8 b = palette[i*3+2];

        // convert 0–255 to 0–31
        r >>= 3;
        g >>= 3;
        b >>= 3;

        ds_palette[i] = RGB15(r, g, b);
    }
}

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
	// we dont use this anymore, atleast for now until the renderer is actually good/working
    return 0;
}

void I_StartupGraphics(void) {
    vid.modenum = 3; 
    vid.width = 256;
    vid.height = 192;
    vid.bpp = 1; // 8-bit buffer
    vid.rowbytes = vid.width;
    vid.dupx = vid.width / 320;
    vid.dupy = vid.height / 200;
    vid.recalc = 1;

    vid.buffer = malloc(vid.width * vid.height);
	
    if (!vid.buffer)
        I_Error("Could'nt allocate video buffer");

    memset(vid.buffer, 0, vid.width*vid.height);

    videoSetMode(MODE_FB0);
    vramSetBankA(VRAM_A_LCD);
}

const char *VID_GetModeName(int modenum)
{
	return window_modes[modenum].name;
}

void I_UpdateNoBlit(void){}

static u16 tempBuffer[256*192];

void I_FinishUpdate(void)
{
    // convert 8bit buffer to RGB15
    for (int i = 0; i < 256*192; i++)
        tempBuffer[i] = ds_palette[vid.buffer[i]];

    // copy to VRAM A
    u16* framebuffer = (u16*)VRAM_A;
    for (int i = 0; i < 256*192; i++)
        framebuffer[i] = tempBuffer[i];
}

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