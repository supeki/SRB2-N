// Nozomi Tetris!

#include "../console.h"
#include "../doomdef.h"
#include "../d_main.h"
#include "../g_input.h"
#include "../g_state.h"
#include "../r_main.h"
#include "../s_sound.h"
#include "../v_video.h"
#include "../w_wad.h"
#include "../z_zone.h"
#include "tetris.h"

// Command that starts our Tetris gamestate.
static void Command_StartTetris_f(void)
{
	if (gamestate != GS_NOZOMITETRIS && wipegamestate != GS_NOZOMITETRIS) {
		gamestate = GS_NOZOMITETRIS;
		wipegamestate = -1;
	}

	CON_ToggleOff();
}

// Initialize a bunch of stuff yaya! Nozomi 03-10-2026
void D_InitNozomiTetris(void) 
{
	COM_AddCommand ("tetris", Command_StartTetris_f);
}

void T_TetrisTicker(void)
{

}


void T_TetrisDrawer(void)
{
	// Draw a black background so we don't have HOMs.
	V_DrawFill(0, 0, vid.width, vid.height, 0); 

	// Placeholder.
	V_DrawStringWhite(160 - (4*15) - 2, 8, "PLACEHOLDER TEXT");
}
