// Nozomi Tetris!

#include "../console.h"
#include "../doomdef.h"
#include "../doomstat.h"
#include "../d_main.h"
#include "../g_input.h"
#include "../g_state.h"
#include "../info.h"
#include "../m_random.h"
#include "../r_draw.h"
#include "../r_main.h"
#include "../s_sound.h"
#include "../v_video.h"
#include "../w_wad.h"
#include "../z_zone.h"
#include "tetris.h"

byte* tetris_board;
piece_t tetris_playingpiece;
tetromino_t tetrominos[NUM_TETROMINOS];

static patch_t* patch_mino;

// Initialize some Console Variables!
CV_PossibleValue_t tetris_boardwidth_t[] =  {{4,"MIN"},{10,"MAX"},{0, NULL}};
CV_PossibleValue_t tetris_boardheight_t[] = {{4,"MIN"},{20,"MAX"},{0, NULL}};
consvar_t cv_tetris_boardwidth =  {"tetris_boardwidth",  "10", CV_CALL, tetris_boardwidth_t, T_ResizeBoard};
consvar_t cv_tetris_boardheight = {"tetris_boardheight", "20", CV_CALL, tetris_boardheight_t, T_ResizeBoard};

// Command that starts our Tetris gamestate.
static void Command_StartTetris_f(void)
{
	if (gamestate != GS_NOZOMITETRIS && wipegamestate != GS_NOZOMITETRIS) {
		gamestate = GS_NOZOMITETRIS;
		wipegamestate = -1;
	}

	T_TetrisInit();
	menuactive = false;
	CON_ToggleOff();
}

// Initialize a bunch of stuff yaya! Nozomi 03-29-2026
void D_InitNozomiTetris(void) 
{
	int i;

	CV_RegisterVar(&cv_tetris_boardwidth);
	CV_RegisterVar(&cv_tetris_boardheight);
	COM_AddCommand ("tetris", Command_StartTetris_f);

	for (i=0; i<NUM_TETROMINOS; i++)
		memset(&tetrominos[i], 0, sizeof(tetromino_t));

	for (i=0; i<4; i++) {
		tetrominos[i_piece].x[i] = i;
		tetrominos[i_piece].y[i] = 1;
	}

	tetrominos[i_piece].xoff = 0;
	tetrominos[i_piece].yoff = 0;
	tetrominos[i_piece].color = SKINCOLOR_WHITE+1;

	tetrominos[o_piece].x[0] = tetrominos[o_piece].x[2] = 1;
	tetrominos[o_piece].x[1] = tetrominos[o_piece].x[3] = 2;
	tetrominos[o_piece].y[0] = tetrominos[o_piece].y[1] = 1;
	tetrominos[o_piece].y[2] = tetrominos[o_piece].y[3] = 2;

	tetrominos[o_piece].xoff = 0;
	tetrominos[o_piece].yoff = 0;
	tetrominos[o_piece].color = SKINCOLOR_LEMON+1;

	patch_mino = W_CachePatchName("TET_MINO", PU_CACHE);
}

// Resize the game board when one of our console variables is changed.
void T_ResizeBoard(void)
{
	free(tetris_board);
	tetris_board = malloc(cv_tetris_boardwidth.value*cv_tetris_boardheight.value);
	memset(tetris_board, 0, cv_tetris_boardwidth.value*cv_tetris_boardheight.value);	
	CONS_Printf("Resized Tetris Board!\nNew Size: %dx%d\n", cv_tetris_boardwidth.value, cv_tetris_boardheight.value);
}

void T_TetrisInit(void)
{
	// First, resize the board (which also resets it)!
	T_ResizeBoard();

	// Then set our statistics and variables! (when they exist)
	memset(&tetris_playingpiece, 0, sizeof(piece_t));
	tetris_playingpiece.type = &tetrominos[o_piece];

	// Then finally, play some nice tunes! :3
	S_ChangeMusicName("TLVLZERO", true);
}

void T_TetrisTicker(void)
{

}


void T_TetrisDrawer(void)
{
	int x, y, piece_x, piece_y, px, py;
	byte* colormap;
	tetromino_t* piece;

	// Draw a black background so we don't have HOMs.
	V_DrawFill(0, 0, vid.width, vid.height, 0); 

	// Placeholder text.
	V_DrawStringWhite(160 - (4*15) - 2, 8, "PLACEHOLDER TEXT");

	// Placeholder mino draw.
	colormap = translationtables - 256 + (tetris_playingpiece.type->color<<8);
	piece = tetris_playingpiece.type; 

	for (y=0; y<4; y++)
	{
		for (x=0; x<4; x++)
		{
			for (py=0;py<4;py++) {
				for (px=0;px<4;px++) {
					piece_x = tetris_playingpiece.x + piece->x[x] + piece->xoff;
					piece_y = tetris_playingpiece.y + piece->y[x] + piece->yoff;
					if (piece->x[x] == px && piece->y[y] == py)
						V_DrawCustomScaledTranslationPatch(piece_x*8, piece_y*8, FRACUNIT/2, 0, patch_mino, colormap);
				}
			}
		}
	}
}
