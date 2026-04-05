// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: m_cheat.c,v 1.2 2000/02/27 00:42:10 hurdler Exp $
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
// $Log: m_cheat.c,v $
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Cheat sequence checking.
//
//-----------------------------------------------------------------------------
 

#include "doomdef.h"
#include "i_system.h" // for localtime!
#include "dstrings.h"

#include "am_map.h"
#include "m_cheat.h"
#include "g_game.h"

#include "r_local.h"
#include "p_local.h"
#include "p_inter.h"

#include "m_cheat.h"
#include "m_menu.h"

#include "i_sound.h" // for I_PlayCD()
#include "s_sound.h"

// ==========================================================================
//                             CHEAT Structures
// ==========================================================================

unsigned char   secret_bd_seq[] = // Demo Tails
{
    SCRAMBLE('b'), SCRAMBLE('e'), SCRAMBLE('e'), SCRAMBLE('d'), SCRAMBLE('e'), SCRAMBLE('e'), 0xff // id...
};

unsigned char   secret_jisk_seq[] = // srb2-nozomi v1.4 Nozomi
{
    SCRAMBLE('j'), SCRAMBLE('i'), SCRAMBLE('s'), SCRAMBLE('k'), 0xff // jisk! :D
};

unsigned char   secret_i_hate_bees_seq[] = // srb2-nozomi v1.4 Nozomi
{
    SCRAMBLE('i'),													 // This one's
	SCRAMBLE('h'), SCRAMBLE('a'), SCRAMBLE('t'), SCRAMBLE('e'),		 // for you
	SCRAMBLE('b'), SCRAMBLE('e'), SCRAMBLE('e'), SCRAMBLE('s'), 0xff // Sam.
};

cheatseq_t      secret_bd = { secret_bd_seq, 0 };
cheatseq_t      secret_jisk = { secret_jisk_seq, 0 };
cheatseq_t		secret_i_hate_bees = { secret_i_hate_bees_seq, 0 };

// ==========================================================================
//                        CHEAT SEQUENCE PACKAGE
// ==========================================================================

static int              firsttime = 1;
static unsigned char    cheat_xlate_table[256];


//
// Called in st_stuff module, which handles the input.
// Returns a 1 if the cheat was successful, 0 if failed.
//
int cht_CheckCheat ( cheatseq_t*   cht,     char           key )
{
    int i;
    int rc = 0;

    if (firsttime)
    {
        firsttime = 0;
        for (i=0;i<256;i++) cheat_xlate_table[i] = SCRAMBLE(i);
    }

    if (!cht->p)
        cht->p = cht->sequence; // initialize if first time

    if (*cht->p == 0)
        *(cht->p++) = key;
    else if
        (cheat_xlate_table[(unsigned char)key] == *cht->p) cht->p++;
    else
        cht->p = cht->sequence;

    if (*cht->p == 1)
        cht->p++;
    else if (*cht->p == 0xff) // end of sequence character
    {
        cht->p = cht->sequence;
        rc = 1;
    }

    return rc;
}

void cht_GetParam ( cheatseq_t*   cht,
                    char*         buffer )
{

    unsigned char *p, c;

    p = cht->sequence;
    while (*(p++) != 1);

    do
    {
        c = *p;
        *(buffer++) = c;
        *(p++) = 0;
    }
    while (c && *p!=0xff );

    if (*p==0xff)
        *buffer = 0;

}

extern player_t *plyr;

boolean cht_Responder (event_t* ev)
{
	// disable keyboard cheats Nozomi 03-05-2026
	//return false;

    if (ev->type == ev_keydown)
    {
        plyr = &players[consoleplayer];

		// Preserved with a minor tweak~ Nozomi 03-18-2026
		// It no longer closes the game if you're on Windows! :3
        if (cht_CheckCheat(&secret_bd, ev->data1))
        {
#ifdef WIN32
			MessageBoxA(NULL, "Hohohoho!!", "*B^D", 0x40000|0x10000);
#else
            I_Error("Hohohoho!! *B^D");
#endif
			// Demo Tails
        }

        if (cht_CheckCheat(&secret_jisk, ev->data1))
        {
			CONS_Printf("Go check Player Setup! *wink wink*\n");
			Color_cons_t[SKINCOLOR_FOREST+1].strvalue = "Jisk";
			setupm_cvcolor = &cv_playercolor;
			cv_playername.string = "Jisk";
			CV_SetValue(&cv_playercolor, SKINCOLOR_FOREST+1);
			setupm_cvname = &cv_playername;
			setupm_cvcolor->string = "Jisk";
        }

		if (cht_CheckCheat(&secret_i_hate_bees, ev->data1))
        {
			CONS_Printf("Activating Bee Replacement Protocol #31052007\n");
			nozo_antibee = true;
        }
    }
    return false;
}


// command that can be typed at the console !

void Command_CheatNoClip_f (void)
{
    player_t*   plyr;
    if (multiplayer || nozo_timeattack)
        return;

    plyr = &players[consoleplayer];

    plyr->cheats ^= CF_NOCLIP;

    if (plyr->cheats & CF_NOCLIP)
        CONS_Printf (STSTR_NCON);
    else
        CONS_Printf (STSTR_NCOFF);

}

void Command_CheatGod_f (void)
{
    player_t*   plyr;

    if (multiplayer || nozo_timeattack)
        return;

    plyr = &players[consoleplayer];

    plyr->cheats ^= CF_GODMODE;
    if (plyr->cheats & CF_GODMODE)
    {
        if (plyr->mo)
            plyr->mo->health = 999;

        plyr->health = 999;
        CONS_Printf ("%s\n", STSTR_DQDON);
    }
    else
        CONS_Printf ("%s\n", STSTR_DQDOFF);
}

void Command_CheatGimme_f (void)
{
    char*     s;
    int       i;
    player_t* plyr;

    if (multiplayer || nozo_timeattack)
        return;

    if (COM_Argc()<2)
    {
        CONS_Printf ("gimme - gives you stuff\nstuff:\n- rings\n- emerald\n- shoes\n- milk\n");
        return;
    }

    plyr = &players[consoleplayer];

    for (i=1; i<COM_Argc(); i++)
    {
        s = COM_Argv(i);

        if (!strncmp(s,"rings",5))
        {
            if (plyr->mo)
                plyr->mo->health += 10;
			plyr->health += 10;

            CONS_Printf("got rings\n");
        }
        else
        if (!strncmp(s,"emerald",7))
        {
            if(!(plyr->emerald1))
				plyr->emerald1 = true;
			else if((plyr->emerald1) && !(plyr->emerald2))
				plyr->emerald2 = true;
			else if((plyr->emerald2) && !(plyr->emerald3))
				plyr->emerald3 = true;
			else if((plyr->emerald3) && !(plyr->emerald4))
				plyr->emerald4 = true;
			else if((plyr->emerald4) && !(plyr->emerald5))
				plyr->emerald5 = true;
			else if((plyr->emerald5) && !(plyr->emerald6))
				plyr->emerald6 = true;
			else if((plyr->emerald6) && !(plyr->emerald7))
				plyr->emerald7 = true;

            CONS_Printf("got emerald\n");
        }
        else
        if (!strncmp(s,"milk",4))
        {
            CONS_Printf("got milk?\n");

			if (!plyr->emerald8)
				plyr->emerald8 = true;
        }
        else
        if (!strncmp(s,"shoes",5))
        {
            if (!plyr->powers[pw_strength])
                P_GivePower( plyr, pw_strength);
            CONS_Printf("got speed shoes\n");
        }
		else
            CONS_Printf ("can't give '%s' : unknown\n", s);


    }
}

void Command_Music_f(void)
{
	char*     s;

	if (COM_Argc()<2)
    {
        CONS_Printf ("music [music name]\n");
        return;
    }

	s = COM_Argv(1);

	if (strlen(s) < 1)
		return;

	S_ChangeMusicName(s, true);
}

void Command_Time_f(void)
{
	localtime_t lt;

	lt = I_GetLocalTime();

	CONS_Printf("%02d-%02d-%04d %02d:%02d:%02d\n", lt.day, lt.month, lt.year, lt.hour, lt.minute, lt.second);
}