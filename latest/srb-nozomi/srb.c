// Sonic Robo-Blast! Nozomi

#include "../doomdef.h"
#include "../g_input.h"
#include "../r_main.h"
#include "../s_sound.h"
#include "../v_video.h"
#include "../w_wad.h"
#include "../z_zone.h"
#include "srb.h"

// Global Variables

boolean play_srb_nozomi = false;

// Input Variables 
// The left and right inputs are used to mimic TGF's movement input stuff where if you hold an opposite dir it's ignored
int srbn_input_left, srbn_input_right, srbn_input_jump = 0;

// Sonic (Earless) Variables
int srbn_sonic_x, srbn_sonic_y, srbn_sonic_speed, srbn_sonic_momy, srbn_sonic_rings, srbn_sonic_onground = 0; // These start at 0!
int srbn_sonic_idletimer, srbn_sonic_runtimer, srbn_sonic_jumptimer = 0; // These start at 0 as well! (Because they're animation timers....)
int srbn_sonic_lives = 3; // Sonic starts with 3 lives in Sonic Robo-Blast! unless you use the 99 lives cheat.
int srbn_sonic_dir = -1; // Sonic faces left by default in Sonic Robo-Blast!
int srbn_sonic_hitbox[4];

// Sonic Patches
static patch_t* srbn_earless[15];
static patch_t* srbn_hud[12];

// Initialize a bunch of stuff yaya! Nozomi 03-10-2026
void D_InitSRBNozomi(void) {
	int i;

	for (i=1; i<15; i++)
		srbn_earless[i-1] = W_CachePatchName(va("EARLSS%d", i), PU_CACHE);

	for (i=0; i<10; i++)
		srbn_hud[i] = W_CachePatchName(va("SRBNHUD%d", i), PU_CACHE);
	srbn_hud[10] = W_CachePatchName("SRBNHUDR", PU_CACHE);
	srbn_hud[11] = W_CachePatchName("SRBNHUDS", PU_CACHE);

	srbn_sonic_hitbox[0] = 0; // left
	srbn_sonic_hitbox[1] = 4; // top
	srbn_sonic_hitbox[2] = srbn_earless[0]->width-1; // right
	srbn_sonic_hitbox[3] = srbn_earless[0]->height-1; // bottom
}

void SRBN_Init(void) {
	srbn_sonic_x = srbn_sonic_y = srbn_sonic_speed = srbn_sonic_momy = srbn_sonic_rings = srbn_sonic_onground = 0; // These start at 0!
	srbn_sonic_idletimer = srbn_sonic_runtimer = srbn_sonic_jumptimer = 0; // These start at 0 as well! (Because they're animation timers....)
	srbn_sonic_lives = 3; // Sonic starts with 3 lives in Sonic Robo-Blast! unless you use the 99 lives cheat.
	srbn_sonic_dir = -1; // Sonic faces left by default in Sonic Robo-Blast!

	play_srb_nozomi = true;
}

// Let's separate our stuff... Nozomi 03-10-2026
static void SRBN_InputHandle(void) {
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

static void SRBN_PlayerHandle(void) {
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

		srbn_sonic_momy++;
	}

	{
		int i;

		for (i=0; i < srbn_sonic_speed/2; i++) {
			srbn_sonic_x += 1*srbn_sonic_dir;

			if (srbn_sonic_x < 0) {
				srbn_sonic_speed = 0;
				srbn_sonic_x = 0;
				break;
			}

			if (srbn_sonic_x >= 320-srbn_sonic_hitbox[2]) {
				srbn_sonic_speed = 0;
				srbn_sonic_x = 320-(srbn_sonic_hitbox[2]+1);
				break;
			}
		}

		for (i=0; i < abs(srbn_sonic_momy); i++) {
			srbn_sonic_y += srbn_sonic_momy/abs(srbn_sonic_momy);

			if (srbn_sonic_y + srbn_sonic_hitbox[3] >= 200) {
				srbn_sonic_y = 200 - (srbn_sonic_hitbox[3]+1);
				srbn_sonic_momy = 0;
				srbn_sonic_onground = 1;
				break;
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
	int earless_patch = 0;

	if (srbn_sonic_jumptimer > 0)
		earless_patch = (srbn_sonic_jumptimer / 17) % 4 + 10;
	else if (srbn_sonic_runtimer > 0)
		earless_patch = ((srbn_sonic_runtimer / 12) % 4) + 6;
	else if (srbn_sonic_idletimer > 6.65f)
		earless_patch = (int)(srbn_sonic_idletimer / 6.65f) % 5 + 1;

	if (srbn_sonic_dir > 0)
		V_DrawScaledPatch(srbn_sonic_x, srbn_sonic_y, 0, srbn_earless[earless_patch]);
	else
		V_DrawScaledPatchFlipped(srbn_sonic_x, srbn_sonic_y, 0, srbn_earless[earless_patch]);

	V_DrawStringWhite(0, 0, va("%d,%d", srbn_sonic_x, srbn_sonic_y));
}