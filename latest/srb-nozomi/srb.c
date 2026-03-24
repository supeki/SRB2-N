// Sonic Robo-Blast! Nozomi

#include "../doomdef.h"
#include "../g_input.h"
#include "../r_main.h"
#include "../s_sound.h"
#include "../v_video.h"
#include "../w_wad.h"
#include "../z_zone.h"
#include "srb.h"

// Prototypes!

static boolean SRBN_InPlatform(srbn_platform_t platform);
static void SRBN_CheckCollision(void);

// Global Variables

boolean play_srb_nozomi = false;

// Input Variables 
// The left and right inputs are used to mimic TGF's movement input stuff where if you hold an opposite dir it's ignored
int srbn_input_left, srbn_input_right, srbn_input_jump = 0;

// Sonic (Earless)
int srbn_sonic_x, srbn_sonic_y, srbn_sonic_speed, srbn_sonic_momy, srbn_sonic_rings, srbn_sonic_onground = 0; // These start at 0!
int srbn_sonic_idletimer, srbn_sonic_runtimer, srbn_sonic_jumptimer = 0; // These start at 0 as well! (Because they're animation timers....)
int srbn_sonic_lives = 3; // Sonic starts with 3 lives in Sonic Robo-Blast! unless you use the 99 lives cheat.
int srbn_sonic_dir = -1; // Sonic faces left by default in Sonic Robo-Blast!
int srbn_sonic_hitbox[4];

// Sonic Patches
static patch_t* srbn_earless[15];
static patch_t* srbn_hud[12];

// Platforms
// Structure:
// used - This platform number is used in this level.
// x - The on-screen x coordinate to put this platform.
// y - The on-screen y coordinate to put this platform.
// w - The width in number of patch tiles.
// h - The height in number of patch tiles.
// patch - The patch number to use, points to a patch in srbn_platformpatches.
// type - The platform type to use; see below for valid types.

// Platform Types:
// 0 - Background
// 1 - Solid
// 2 - Platform
// 3 - Unsure

#define PLATFORM_SOLID 1
#define PLATFORM_PLATFORM 2

static srbn_platform_t srbn_platforms[1024]; // We shouldn't really need this many platforms...?

// Platform Patches
static patch_t* srbn_platformpatches[1024]; // Same with the platform patches... blegh.

// Initialize a bunch of stuff yaya! Nozomi 03-10-2026
void D_InitSRBNozomi(void) {
	int i;

	// cache hedgehog patches
	for (i=1; i<15; i++)
		srbn_earless[i-1] = W_CachePatchName(va("EARLSS%d", i), PU_CACHE);

	// cache hud patches
	for (i=0; i<10; i++)
		srbn_hud[i] = W_CachePatchName(va("SRBNHUD%d", i), PU_CACHE);
	srbn_hud[10] = W_CachePatchName("SRBNHUDR", PU_CACHE);
	srbn_hud[11] = W_CachePatchName("SRBNHUDS", PU_CACHE);

	// set hedgehog default hitbox
	srbn_sonic_hitbox[0] = 0; // left
	srbn_sonic_hitbox[1] = 4; // top
	srbn_sonic_hitbox[2] = srbn_earless[0]->width-1; // right
	srbn_sonic_hitbox[3] = srbn_earless[0]->height-1; // bottom

	// init platforms
	memset(&srbn_platforms[i], 0, sizeof(srbn_platform_t));

	// cache platform patches
	for (i=0; i<8; i++)
		srbn_platformpatches[i] = W_CachePatchName(va("SRBP%04d", i), PU_CACHE);
}

void SRBN_Init(void) {
	srbn_sonic_x = srbn_sonic_y = srbn_sonic_speed = srbn_sonic_momy = srbn_sonic_rings = srbn_sonic_onground = 0; // These start at 0!
	srbn_sonic_idletimer = srbn_sonic_runtimer = srbn_sonic_jumptimer = 0; // These start at 0 as well! (Because they're animation timers....)
	srbn_sonic_lives = 3; // Sonic starts with 3 lives in Sonic Robo-Blast! unless you use the 99 lives cheat.
	srbn_sonic_dir = -1; // Sonic faces left by default in Sonic Robo-Blast!

	// TO-DO: Replace the placeholders below with a level loading function! Nozomi 03-24-2026
	srbn_platforms[0].used = true; // this platform id is used
	srbn_platforms[0].x = 0; // x coordinate on screen
	srbn_platforms[0].y = 100; // y coordinate on screen
	srbn_platforms[0].w = 4; // number of patch tiles in width
	srbn_platforms[0].h = 1; // number of patch tiles in height
	srbn_platforms[0].patch = 1; // see nozomi-gfx.wad for patch order
	srbn_platforms[0].type = PLATFORM_SOLID; // full solid, see above for details

	srbn_platforms[1].used = true; // this platform id is used
	srbn_platforms[1].x = 96; // x coordinate on screen
	srbn_platforms[1].y = 144; // y coordinate on screen
	srbn_platforms[1].w = 16; // number of patch tiles in width
	srbn_platforms[1].h = 1; // number of patch tiles in height
	srbn_platforms[1].patch = 0; // see nozomi-gfx.wad for patch order
	srbn_platforms[1].type = PLATFORM_PLATFORM; // platform, see above for details

	srbn_platforms[2].used = true; // this platform id is used
	srbn_platforms[2].x = 128; // x coordinate on screen
	srbn_platforms[2].y = 128; // y coordinate on screen
	srbn_platforms[2].w = 2; // number of patch tiles in width
	srbn_platforms[2].h = 2; // number of patch tiles in height
	srbn_platforms[2].patch = 1; // see nozomi-gfx.wad for patch order
	srbn_platforms[2].type = PLATFORM_SOLID; // full solid, see above for details

	srbn_platforms[3].used = true; // this platform id is used
	srbn_platforms[3].x = 160; // x coordinate on screen
	srbn_platforms[3].y = 128; // y coordinate on screen
	srbn_platforms[3].w = 10; // number of patch tiles in width
	srbn_platforms[3].h = 1; // number of patch tiles in height
	srbn_platforms[3].patch = 1; // see nozomi-gfx.wad for patch order
	srbn_platforms[3].type = PLATFORM_SOLID; // full solid, see above for details

	srbn_platforms[4].used = true; // this platform id is used
	srbn_platforms[4].x = 272; // x coordinate on screen
	srbn_platforms[4].y = 0; // y coordinate on screen
	srbn_platforms[4].w = 3; // number of patch tiles in width
	srbn_platforms[4].h = 8; // number of patch tiles in height
	srbn_platforms[4].patch = 1; // see nozomi-gfx.wad for patch order
	srbn_platforms[4].type = PLATFORM_SOLID; // full solid, see above for details

	srbn_platforms[5].used = true; // this platform id is used
	srbn_platforms[5].x = 240; // x coordinate on screen
	srbn_platforms[5].y = 96; // y coordinate on screen
	srbn_platforms[5].w = 2; // number of patch tiles in width
	srbn_platforms[5].h = 2; // number of patch tiles in height
	srbn_platforms[5].patch = 1; // see nozomi-gfx.wad for patch order
	srbn_platforms[5].type = 0; // background, see above for details

	srbn_platforms[6].used = true; // this platform id is used
	srbn_platforms[6].x = 240; // x coordinate on screen
	srbn_platforms[6].y = 96; // y coordinate on screen
	srbn_platforms[6].w = 16; // number of patch tiles in width
	srbn_platforms[6].h = 1; // number of patch tiles in height
	srbn_platforms[6].patch = 0; // see nozomi-gfx.wad for patch order
	srbn_platforms[6].type = PLATFORM_PLATFORM; // platform, see above for details

	S_ChangeMusicName("KNOTHOLE", 1);

	play_srb_nozomi = true;
}

// Let's separate our stuff... Nozomi 03-10-2026
static void SRBN_InputHandle(void) 
{
	// movement!! Nozomi 03-10-2026
	// ...Not yet Nozomi! This is for input now! ~ Future Nozomi
	if (gamekeydown[gamecontrol[gc_strafeleft][0]] ||
		gamekeydown[gamecontrol[gc_strafeleft][1]])
		srbn_input_left++;
	else
		srbn_input_left = 0;

	if (gamekeydown[gamecontrol[gc_straferight][0]] ||
		gamekeydown[gamecontrol[gc_straferight][1]])
		srbn_input_right++;
	else
		srbn_input_right = 0;

	if (gamekeydown[gamecontrol[gc_jump][0]] ||
		gamekeydown[gamecontrol[gc_jump][1]])
		srbn_input_jump++;
	else
		srbn_input_jump = 0;

	// For that "TGF-like" input detection :3 Nozomi 03-10-2026
	if (srbn_input_left > srbn_input_right)
		srbn_input_right = 0;
	else if (srbn_input_right > srbn_input_left)
		srbn_input_left = 0;
}

static void SRBN_PlayerHandle(void) 
{
	if (srbn_input_left)
		srbn_sonic_dir = -1;
	else if (srbn_input_right)
		srbn_sonic_dir = 1;

	if (srbn_input_left || srbn_input_right)
		srbn_sonic_speed++;
	else if (srbn_sonic_speed > 0)
		srbn_sonic_speed--;

	if (srbn_input_jump == 1)
		S_StartSound(NULL, sfx_jump);

	if (srbn_sonic_speed > 16)
		srbn_sonic_speed = 16;

	if (srbn_sonic_onground) {
		if (srbn_input_left || srbn_input_right || srbn_sonic_speed > 0) {
			srbn_sonic_runtimer += srbn_sonic_speed;
			srbn_sonic_idletimer = 0;
		} else {
			srbn_sonic_idletimer++;
			srbn_sonic_runtimer = 0;
		}

		srbn_sonic_jumptimer = 0;

		if (srbn_input_jump) {
			srbn_sonic_momy = -11;
			srbn_sonic_onground = 0;
		}
	} else {
		srbn_sonic_idletimer = 0;
		srbn_sonic_runtimer = 0;

		{
			int spd = abs(srbn_sonic_momy) + srbn_sonic_speed;
			if (spd > 28) 
				spd = 28;
			if (spd < 7)
				spd = 7;
			srbn_sonic_jumptimer += spd;
		}
	}

	// set current hitbox

	if (!srbn_sonic_onground) {
		srbn_sonic_hitbox[0] = 5; // left
		srbn_sonic_hitbox[1] = 24; // top
		srbn_sonic_hitbox[2] = srbn_earless[0]->width-6; // right
		srbn_sonic_hitbox[3] = srbn_earless[0]->height-1; // bottom
	} else if (srbn_sonic_speed) {
		srbn_sonic_hitbox[0] = 2; // left
		srbn_sonic_hitbox[1] = 8; // top
		srbn_sonic_hitbox[2] = srbn_earless[0]->width-3; // right
		srbn_sonic_hitbox[3] = srbn_earless[0]->height-1; // bottom
	} else {
		srbn_sonic_hitbox[0] = 0; // left
		srbn_sonic_hitbox[1] = 4; // top
		srbn_sonic_hitbox[2] = srbn_earless[0]->width-1; // right
		srbn_sonic_hitbox[3] = srbn_earless[0]->height-1; // bottom
	}

	srbn_sonic_momy++;

	SRBN_CheckCollision();
}

static boolean SRBN_InPlatform(srbn_platform_t platform)
{
	patch_t* patch = srbn_platformpatches[platform.patch];

	return (
		srbn_sonic_x + srbn_sonic_hitbox[2] >= platform.x &&
		srbn_sonic_x + srbn_sonic_hitbox[0] <= platform.x + (platform.w * patch->width) - 1 &&
		srbn_sonic_y + srbn_sonic_hitbox[3] >= platform.y &&
		srbn_sonic_y + srbn_sonic_hitbox[1] <= platform.y + (platform.h * patch->height) - 1
	);
}

static boolean SRBN_CheckEdge(void)
{
	boolean result = false;

	if (srbn_sonic_x + srbn_sonic_hitbox[0] < 0) {
		srbn_sonic_speed = 0;
		srbn_sonic_x = 0 - srbn_sonic_hitbox[0];
		result = true;
	}

	if (srbn_sonic_x + srbn_sonic_hitbox[2] >= 320) {
		srbn_sonic_speed = 0;
		srbn_sonic_x = 320 - (srbn_sonic_hitbox[2]+1);
		result = true;
	}

	if (srbn_sonic_y + srbn_sonic_hitbox[3] >= 199) {
		srbn_sonic_y = 199 - (srbn_sonic_hitbox[3]+1);
		srbn_sonic_momy = 0;
		srbn_sonic_onground = 1;
		result = true;
	}

	return result;
}

// A LOT OF COPY-PASTING IN THIS FUNCTION
// I'M SORRY :<<<< Nozomi 03-24-2026
static void SRBN_CheckCollision(void)
{	
	int i,j;

	for (j=0; j < srbn_sonic_speed/4 + 1; j++) {
		boolean result = false;
		if (srbn_sonic_speed)
			srbn_sonic_x += 1*srbn_sonic_dir;

		for (i=0; i<1024; i++)
		{
			srbn_platform_t platform = srbn_platforms[i];
			patch_t* patch;

			if (!platform.used)
				continue;

			patch = srbn_platformpatches[platform.patch];

			if (SRBN_InPlatform(platform))
			{
				switch (platform.type)
				{
					case PLATFORM_SOLID:
						if (srbn_sonic_x + srbn_sonic_hitbox[0] > platform.x + (platform.w * patch->width)/2)
							srbn_sonic_x = platform.x + (platform.w * patch->width) - srbn_sonic_hitbox[0];
						else
							srbn_sonic_x = platform.x-(srbn_sonic_hitbox[2]+1);

						srbn_sonic_speed = 0;
						result = true;
						break;
					default:
						result = false;
						break;
				}
			}
		}

		if (SRBN_CheckEdge())
			result = true;

		if (result)
			break;
	}

	for (j=0; j < abs(srbn_sonic_momy); j++) {
		boolean result = false;
		srbn_sonic_y += srbn_sonic_momy/abs(srbn_sonic_momy);

		for (i=0; i<1024; i++)
		{
			srbn_platform_t platform = srbn_platforms[i];
			patch_t* patch;

			if (!platform.used)
				continue;

			patch = srbn_platformpatches[platform.patch];

			if (SRBN_InPlatform(platform))
			{
				switch (platform.type)
				{
					case PLATFORM_SOLID:
						if (srbn_sonic_y + srbn_sonic_hitbox[1] > platform.y + (platform.h * patch->height)/2)
							srbn_sonic_y = platform.y + (platform.h * patch->height) - srbn_sonic_hitbox[1];
						else {
							srbn_sonic_y = platform.y-(srbn_sonic_hitbox[3]+1);
							srbn_sonic_onground = 1;
						}

						srbn_sonic_momy = 0;
						result = true;
						break;
					case PLATFORM_PLATFORM:
						if (srbn_sonic_momy > 0 && srbn_sonic_y+srbn_sonic_hitbox[3] < platform.y+2) {
							srbn_sonic_y = platform.y-(srbn_sonic_hitbox[3]+1);

							srbn_sonic_momy = 0;
							srbn_sonic_onground = 1;

							result = true;
						}
						break;
					default:
						result = false;
						break;
				}
			}

			if (result)
				break;
		}

		if (SRBN_CheckEdge())
			result = true;

		if (result)
			break;
		else
			srbn_sonic_onground = 0;
	}
}

static void SRBN_DrawBackground(void)
{
	V_DrawFill(0, 0, 320, 200, 193);
}

static void SRBN_DrawSonikku(void)
{
	int earless_patch = 0;

	if (srbn_sonic_jumptimer > 0)
		earless_patch = (srbn_sonic_jumptimer / 17) % 4 + 10;
	else if (srbn_sonic_runtimer > 0)
		earless_patch = ((srbn_sonic_runtimer / 12) % 4) + 6;
	else if (srbn_sonic_idletimer > 6.65f)
		earless_patch = (int)(srbn_sonic_idletimer / 6.65f) % 5 + 1;

	if (srbn_sonic_dir > 0)
		V_DrawScaledPatch(srbn_sonic_x, srbn_sonic_y - srbn_sonic_momy, 0, srbn_earless[earless_patch]);
	else
		V_DrawScaledPatchFlipped(srbn_sonic_x, srbn_sonic_y - srbn_sonic_momy, 0, srbn_earless[earless_patch]);

	V_DrawStringWhite(0, 0, va("%d,%d", srbn_sonic_x, srbn_sonic_y));
}

static void SRBN_DrawPlatforms(void)
{
	patch_t* patch;
	int i,j,k,w,h;

	for (i=0; i<1024; i++)
	{
		srbn_platform_t platform = srbn_platforms[i];

		if (!platform.used)
			continue;

		patch = srbn_platformpatches[platform.patch];
		w = patch->width;
		h = patch->height;

		for (j=0; j<platform.h; j++)
		{
			for (k=0; k<platform.w; k++)
			{
				V_DrawScaledPatch(platform.x + (w*k), platform.y + (h*j), 0, patch);
			}
		}
	}
}

void SRBN_GameplayLoop(void)
{
	SRBN_InputHandle();
	SRBN_PlayerHandle();
}

void SRBN_Draw(void)
{
	SRBN_DrawBackground();
	SRBN_DrawPlatforms();
	SRBN_DrawSonikku();
}