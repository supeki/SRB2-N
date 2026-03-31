#include "../doomtype.h"

#ifndef NOZOMI_PONG
#define NOZOMI_PONG

// Cache everything...
void D_InitNozomiPong(void);

// For d_main.c
void T_PongTicker(void);
void T_PongDrawer(void);

#endif