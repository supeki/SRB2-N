#include "../doomtype.h"

#ifndef NOZOMI_SRB
#define NOZOMI_SRB

// Boolean that when set to true, starts a session of SRB-N!
extern boolean play_srb_nozomi;

// Cache everything...
void D_InitSRBNozomi(void);

// SRBN formats~!

// Platforms
typedef struct
{
	boolean used; // is this platform being used?
	int x; // position in the game screen
	int y; // position in the game screen
	int w; // number of patch tiles
	int h; // number of patch tiles
	int patch; // patch number to use
	int type; // platform type, see srb.c for more details
} srbn_platform_t;

// Functions for srb.c!
extern void SRBN_Init(void);
extern void SRBN_GameplayLoop(void);
extern void SRBN_Draw(void);

#endif