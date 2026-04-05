#include "../doomtype.h"

#ifndef NOZOMI_TETRIS
#define NOZOMI_TETRIS

// Cache everything...
void D_InitNozomiTetris(void);

// To initialize/start a game of Nozomi Tetris!
void T_TetrisInit(void);

// For d_main.c
void T_TetrisTicker(void);
void T_TetrisDrawer(void);

extern CV_PossibleValue_t tetris_boardwidth_t[];
extern CV_PossibleValue_t tetris_boardheight_t[];
extern consvar_t cv_tetris_boardwidth;
extern consvar_t cv_tetris_boardheight;

extern byte* tetris_board;

// For our console variables and byte table above.
void T_ResizeBoard(void);

// Tetrominos below!

typedef struct
{
	// A tetromino is a shape composed
	// of four directly-connected squares.
	int x[4]; // x position for each mino in the tetromino
	int y[4]; // y position for each mino in the tetromino
	int xoff; // tetromino's xoffset from the spawn position
	int yoff; // tetromino's yoffset from the spawn position
	byte color; // the skincolor to use for this tetromino
} tetromino_t;

typedef enum
{
	i_piece,
	o_piece,
	t_piece,
	l_piece, // LJ SONIK???
	j_piece, // :3
	s_piece,
	z_piece,
	NUM_TETROMINOS
} tetromino_nums_t;

extern tetromino_t tetrominos[NUM_TETROMINOS];

// Variables and important stuffs for a game!

typedef struct
{
	// The "piece" is the current active
	// shape on the player's board.
	tetromino_t* type; // points to the piece's type definition
	byte x; // x position of this piece on the board
	byte y; // y position of this piece on the board
	byte rot; // the current rotation state of this piece
	byte falltimer; // timer to determine how long to wait before falling a cell
	boolean locked; // if true, this piece is locked!
	byte locktimer; // counts up when the piece is hitting something below
} piece_t;

extern piece_t tetris_playingpiece;

#endif