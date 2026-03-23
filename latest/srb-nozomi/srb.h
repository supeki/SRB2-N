#include "../doomtype.h"

#ifndef NOZOMI_SRB
#define NOZOMI_SRB

// Boolean that when set to true, starts a session of SRB-N!
boolean play_srb_nozomi;

// Cache everything...
void D_InitSRBNozomi(void);

void SRBN_Init(void);
void SRBN_GameplayLoop(void);
void SRBN_Draw(void);

#endif