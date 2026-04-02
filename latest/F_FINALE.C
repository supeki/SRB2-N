// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: f_finale.c,v 1.3 2000/08/03 17:57:41 bpereira Exp $
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
// $Log: f_finale.c,v $
// Revision 1.3  2000/08/03 17:57:41  bpereira
// no message
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Game completion, final screen animation.
//
//-----------------------------------------------------------------------------


#include "doomdef.h"
#include "doomstat.h"
#include "am_map.h"
#include "dstrings.h"
#include "d_main.h"
#include "f_finale.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "m_easing.h"
#include "p_tick.h"
#include "r_local.h"
#include "s_sound.h"
#include "i_video.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

// Stage of animation:
//  0 = text, 1 = art screen, 2 = character cast
int             finalestage;

int             finalecount;

static int animtimer; // Used for some animation timings

#define TEXTSPEED       3
#define TEXTWAIT        250

char*   finaletext;
char*   finaleflat;

//
// F_StartFinale
//
void F_StartFinale (void)
{
    gamestate = GS_FINALE;

    // Okay - IWAD dependend stuff.
    // This has been changed severly, and
    //  some stuff might have changed in the process.
    switch ( gamemode )
    {

      // DOOM 1 - E1, E3 or E4, but each nine missions
      // DOOM II and missions packs with E1, M34
      case commercial:
      {
		  if(xmasmode && gamemap == 5)
			S_ChangeMusic(mus_read_m, true);

          switch (gamemap)
          {
            case 4: // Tails 09-02-2001
              finaleflat = text[SLIME16_NUM];
              finaletext = C1TEXT;
              break;
            case 5: // Tails 09-02-2001
              finaleflat = text[RROCK14_NUM];
              finaletext = C2TEXT;
              break;
            case 24: // Tails 09-02-2001
              finaleflat = text[RROCK07_NUM];
              finaletext = C3TEXT;
              break;
            case 25: // Tails 09-02-2001
              finaleflat = text[RROCK17_NUM];
              finaletext = C4TEXT;
              break;
            case 26: // Tails 09-02-2001
              finaleflat = text[RROCK13_NUM];
              finaletext = C5TEXT;
              break;
            case 27: // Tails 09-02-2001
              finaleflat = text[RROCK19_NUM];
              finaletext = C6TEXT;
              break;
            default:
              // Ouch.
              break;
          }
          break;
      }


      // Indeterminate.
      default:
        S_ChangeMusic(mus_read_m, true);
        finaleflat = "F_SKY1"; // Not used anywhere else.
        finaletext = C1TEXT;  // FIXME - other text, music?
        break;
    }

    finalestage = 0;
    finalecount = 0;

}



boolean F_Responder (event_t *event)
{
    return false;
}


//
// F_Ticker
//
void F_Ticker (void)
{
    int         i;

    // check for skipping
    if ( (gamemode == commercial)
      && ( finalecount > 50) )
    {
      // go on to the next level
      for (i=0 ; i<MAXPLAYERS ; i++)
        if (players[i].cmd.buttons)
          break;

      if (i < MAXPLAYERS)
      {
           gameaction = ga_worlddone;
           finalecount = MININT;    // wait until map is lunched
      }
    }

    // advance animation
    finalecount++;

    if ( gamemode == commercial)
        return;

    if (!finalestage && (unsigned)finalecount>strlen (finaletext)*TEXTSPEED + TEXTWAIT)
    {
        finalecount = 0;
        finalestage = 1;
        wipegamestate = -1;             // force a wipe
    }
}



//
// F_TextWrite
//
void F_TextWrite (void)
{
    int         w;
    int         count;
    char*       ch;
    int         c;
    int         cx;
    int         cy;

    // erase the entire screen to a tiled background
    V_DrawFlatFill(0,0,vid.width,vid.height,W_GetNumForName(finaleflat));

    V_MarkRect (0, 0, vid.width, vid.height);

// Tails DRAW A FULL PIC INSTEAD OF FLAT!
    switch (gamemap)
	{
		case 4:
			V_DrawScaledPatch (0,0,0,W_CacheLumpName ("SEGALOGO", PU_CACHE));
			break;
        case 5:
			V_DrawScaledPatch (0,0,0,W_CacheLumpName ("BOSSBACK", PU_CACHE));
			break;
        default:
			break;
	}

    // draw some of the text onto the screen
    cx = 10;
    cy = 10;
    ch = finaletext;

    count = (finalecount - 10)/TEXTSPEED;
    if (count < 0)
        count = 0;
    for ( ; count ; count-- )
    {
        c = *ch++;
        if (!c)
            break;
        if (c == '\n')
        {
            cx = 10;
            cy += 11;
            continue;
        }

        c = toupper(c) - HU_FONTSTART;
        if (c < 0 || c> HU_FONTSIZE)
        {
            cx += 4;
            continue;
        }

        w = SHORT (hu_font[c]->width);
        if (cx+w > vid.width)
            break;
        V_DrawScaledPatch(cx, cy, 0, hu_font[c]); // Tails 12-12-2001
        cx+=w;
    }

}

//
// F_CastDrawer
//
void V_DrawPatchFlipped (int x, int y, int scrn, patch_t *patch);

//
// F_Drawer
//
void F_Drawer (void)
{
    if (!finalestage)
        F_TextWrite ();
    else
    {
        switch (gameepisode)
        {
          case 1:
            if ( gamemode == retail )
              V_DrawScaledPatch (0,0,0,
                         W_CachePatchName(text[CREDIT_NUM],PU_CACHE));
            else
              V_DrawScaledPatch (0,0,0,
                         W_CachePatchName(text[HELP2_NUM],PU_CACHE));
            break;
          case 2:
            V_DrawScaledPatch(0,0,0,
                        W_CachePatchName(text[VICTORY2_NUM],PU_CACHE));
            break;
          case 4:
            V_DrawScaledPatch (0,0,0,
                         W_CachePatchName(text[ENDPIC_NUM],PU_CACHE));
            break;
        }
    }

}

static void F_DrawPatchCol(int x, patch_t *patch, int col)
{
	const column_t *column;
	const byte *source;
	byte *desttop, *dest = NULL;
	const byte *deststop, *destbottom;
	size_t count;

	desttop = screens[0] + x*vid.dupx;
	deststop = screens[0] + vid.rowbytes * vid.height;
	destbottom = desttop + vid.height*vid.width;

	do {
		int topdelta, prevdelta = -1;
		column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			topdelta = column->topdelta;
			if (topdelta <= prevdelta)
				topdelta += prevdelta;
			prevdelta = topdelta;
			source = (const byte *)column + 3;
			dest = desttop + topdelta*(vid.height/BASEVIDHEIGHT)*vid.width;
			count = column->length;

			while (count--)
			{
				int dupycount = vid.dupy;

				while (dupycount-- && dest < destbottom)
				{
					int dupxcount = vid.dupx;
					while (dupxcount-- && dest <= deststop)
						*dest++ = *source;

					dest += (vid.width - vid.dupx);
				}
				source++;
			}
			column = (const column_t *)((const byte *)column + column->length + 4);
		}

		desttop += patch->height*vid.dupy*vid.width;
	} while(dest < destbottom);
}

//
// F_SkyScroll
//
static void F_SkyScroll(void)
{
	int scrolled, x, mx, fakedwidth;
	patch_t *pat;

	pat = W_CachePatchName("TITLESKY", PU_CACHE);

	animtimer = ((finalecount*80)/16) % SHORT(pat->width);

	fakedwidth = vid.width / vid.dupx;

	if (rendermode == render_soft)
	{ // if only hardware rendering could be this elegant and complete
		scrolled = (SHORT(pat->width) - animtimer) - 1;
		for (x = 0, mx = scrolled; x < fakedwidth; x++, mx = (mx+1)%SHORT(pat->width))
			F_DrawPatchCol(x, pat, mx);
	}
}

static patch_t* ttbanner; // white banner with "robo blast" and "2"
static patch_t* ttwing; // wing background
static patch_t* ttsonic; // "SONIC"
static patch_t* ttswave1; // Title Sonics
static patch_t* ttswave2;
static patch_t* ttswip1;
static patch_t* ttsprep1;
static patch_t* ttsprep2;
static patch_t* ttspop1;
static patch_t* ttspop2;
static patch_t* ttspop3;
static patch_t* ttspop4;
static patch_t* ttspop5;
static patch_t* ttspop6;
static patch_t* ttspop7;

void F_StartTitleScreen(void)
{
	gamestate = GS_NOZOMITITLE;

	// IWAD dependent stuff.
	S_ChangeMusic(mus_dm2ttl, false);

	finalecount = 0;
	finalestage = 0;
	animtimer = 0;

	ttbanner = W_CachePatchName("TTBANNER", PU_LEVEL);
	ttwing = W_CachePatchName("TTWING", PU_LEVEL);
	ttsonic = W_CachePatchName("TTSONIC", PU_LEVEL);
	ttswave1 = W_CachePatchName("TTSWAVE1", PU_LEVEL);
	ttswave2 = W_CachePatchName("TTSWAVE2", PU_LEVEL);
	ttswip1 = W_CachePatchName("TTSWIP1", PU_LEVEL);
	ttsprep1 = W_CachePatchName("TTSPREP1", PU_LEVEL);
	ttsprep2 = W_CachePatchName("TTSPREP2", PU_LEVEL);
	ttspop1 = W_CachePatchName("TTSPOP1", PU_LEVEL);
	ttspop2 = W_CachePatchName("TTSPOP2", PU_LEVEL);
	ttspop3 = W_CachePatchName("TTSPOP3", PU_LEVEL);
	ttspop4 = W_CachePatchName("TTSPOP4", PU_LEVEL);
	ttspop5 = W_CachePatchName("TTSPOP5", PU_LEVEL);
	ttspop6 = W_CachePatchName("TTSPOP6", PU_LEVEL);
	ttspop7 = W_CachePatchName("TTSPOP7", PU_LEVEL);
}

void F_TitleScreenTicker(void)
{
	finalecount++;
	finalestage += 8;
}

static fixed_t ttlscale = FRACUNIT;
static int titletimer;

void Title_Drawer(void) 
{
	if (gamestate != GS_NOZOMITITLE)
		return;

	if (!demoplayback) {
		F_SkyScroll();
		titletimer = finalecount;
	} else
		titletimer = leveltime + 57;
	

	V_DrawCustomScaledTranslationPatch(30, 14, ttlscale, 0, ttwing, colormaps);

	if(titletimer < 57)
	{
		if(titletimer == 35)
			V_DrawCustomScaledTranslationPatch(115, 15, ttlscale, 0, ttspop1, colormaps);
		else if(titletimer == 36)
			V_DrawCustomScaledTranslationPatch(114, 15, ttlscale, 0,ttspop2, colormaps);
		else if(titletimer == 37)
			V_DrawCustomScaledTranslationPatch(113, 15, ttlscale, 0,ttspop3, colormaps);
		else if(titletimer == 38)
			V_DrawCustomScaledTranslationPatch(112, 15, ttlscale, 0,ttspop4, colormaps);
		else if(titletimer == 39)
			V_DrawCustomScaledTranslationPatch(111, 15, ttlscale, 0,ttspop5, colormaps);
		else if(titletimer == 40)
			V_DrawCustomScaledTranslationPatch(110, 15, ttlscale, 0, ttspop6, colormaps);
		else if(titletimer >= 41 && titletimer <= 44)
			V_DrawCustomScaledTranslationPatch(109, 15, ttlscale, 0, ttspop7, colormaps);
		else if(titletimer >= 45 && titletimer <= 48)
			V_DrawCustomScaledTranslationPatch(108, 12, ttlscale, 0, ttsprep1, colormaps);
		else if(titletimer >= 49 && titletimer <= 52)
			V_DrawCustomScaledTranslationPatch(107, 9, ttlscale, 0, ttsprep2, colormaps);
		else if(titletimer >= 53 && titletimer <= 56)
			V_DrawCustomScaledTranslationPatch(106, 6, ttlscale, 0, ttswip1, colormaps);
		V_DrawCustomScaledTranslationPatch(93, 106, ttlscale, 0, ttsonic, colormaps);
	}
	else
	{
		V_DrawCustomScaledTranslationPatch(93, 106, ttlscale, 0,ttsonic, colormaps);
		if(titletimer/5 & 1)
			V_DrawCustomScaledTranslationPatch(100, 3, ttlscale, 0,ttswave1, colormaps);
		else
			V_DrawCustomScaledTranslationPatch(100, 3, ttlscale, 0,ttswave2, colormaps);
	}

	V_DrawCustomScaledTranslationPatch(48, 142, ttlscale, 0,ttbanner, colormaps);
}
