#include "../doomtype.h"

#ifndef NOZOMI_TETRIS
#define NOZOMI_TETRIS

// Cache everything...
void D_InitNozomiTetris(void);

// For d_main.c
void T_TetrisTicker(void);
void T_TetrisDrawer(void);

#endif