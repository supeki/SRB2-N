// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: r_draw.c,v 1.8 2000/11/09 17:56:20 stroggonmeth Exp $
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
// $Log: r_draw.c,v $
// Revision 1.8  2000/11/09 17:56:20  stroggonmeth
// Hopefully fixed a few bugs and did a few optimizations.
//
// Revision 1.7  2000/11/03 03:48:54  stroggonmeth
// Fix a few warnings when compiling.
//
// Revision 1.6  2000/11/02 17:50:09  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.5  2000/07/01 09:23:49  bpereira
// no message
//
// Revision 1.4  2000/04/07 18:47:09  hurdler
// There is still a problem with the asm code and boom colormap
// At least, with this little modif, it compiles on my Linux box
//
// Revision 1.3  2000/04/06 21:06:19  stroggonmeth
// Optimized extra_colormap code...
// Added #ifdefs for older water code.
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      span / column drawer functions, for 8bpp and 16bpp
//
//      All drawing to the view buffer is accomplished in this file.
//      The other refresh files only know about ccordinates,
//      not the architecture of the frame buffer.
//      The frame buffer is a linear one, and we need only the base address.
//
//-----------------------------------------------------------------------------


#include "doomdef.h"
#include "r_local.h"
#include "st_stuff.h"   //added:24-01-98:need ST_HEIGHT
#include "i_video.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"
#include "info.h" // For skincolor numbers.

#ifdef HWRENDER
#include "hardware/hw_main.h"
#endif

// ==========================================================================
//                     COMMON DATA FOR 8bpp AND 16bpp
// ==========================================================================

byte*           viewimage;
int             viewwidth;
int             scaledviewwidth;
int             viewheight;
int             viewwindowx;
int             viewwindowy;

                // pointer to the start of each line of the screen,
byte*           ylookup[MAXVIDHEIGHT];

byte*           ylookup1[MAXVIDHEIGHT]; // for view1 (splitscreen)
byte*           ylookup2[MAXVIDHEIGHT]; // for view2 (splitscreen)

                 // x byte offset for columns inside the viewwindow
                // so the first column starts at (SCRWIDTH-VIEWWIDTH)/2
int             columnofs[MAXVIDWIDTH];

#ifdef HORIZONTALDRAW
//Fab 17-06-98: horizontal column drawer optimisation
byte*           yhlookup[MAXVIDWIDTH];
int             hcolumnofs[MAXVIDHEIGHT];
#endif

// =========================================================================
//                      COLUMN DRAWING CODE STUFF
// =========================================================================

lighttable_t*           dc_colormap;
int                     dc_x;
int                     dc_yl;
int                     dc_yh;

//Hurdler: 04/06/2000: asm code still use it
//#ifdef OLDWATER
int                     dc_yw;          //added:24-02-98: WATER!
lighttable_t*           dc_wcolormap;   //added:24-02-98:WATER!
//#endif

fixed_t                 dc_iscale;
fixed_t                 dc_texturemid;

byte*                   dc_source;


// -----------------------
// translucency stuff here
// -----------------------
#define NUMTRANSTABLES  5     // how many translucency tables are used

byte*                   transtables;    // translucency tables

// R_DrawTransColumn uses this
byte*                   dc_transmap;    // one of the translucency tables


// ----------------------
// translation stuff here
// ----------------------

byte*                   translationtables;

byte*					fadetables;

// R_DrawTranslatedColumn uses this
byte*                   dc_translation;

struct r_lightlist_s*   dc_lightlist = NULL;
int                     dc_numlights = 0;
int                     dc_maxlights;

int     dc_texheight;

// =========================================================================
//                      SPAN DRAWING CODE STUFF
// =========================================================================

int                     ds_y;
int                     ds_x1;
int                     ds_x2;

lighttable_t*           ds_colormap;

fixed_t                 ds_xfrac;
fixed_t                 ds_yfrac;
fixed_t                 ds_xstep;
fixed_t                 ds_ystep;

byte*                   ds_source;      // start of a 64*64 tile image


// ==========================================================================
//                        OLD DOOM FUZZY EFFECT
// ==========================================================================

//
// Spectre/Invisibility.
//
#define FUZZTABLE     50
#define FUZZOFF       (1)

static  int fuzzoffset[FUZZTABLE] =
{
    FUZZOFF,-FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
    FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
    FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,
    FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
    FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,
    FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,
    FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF
};

static  int fuzzpos = 0;     // move through the fuzz table


//  fuzzoffsets are dependend of vid width, for optimising purpose
//  this is called by SCR_Recalc() whenever the screen size changes
//
void R_RecalcFuzzOffsets (void)
{
    int i;
    for (i=0;i<FUZZTABLE;i++)
    {
        fuzzoffset[i] = (fuzzoffset[i] < 0) ? -vid.width : vid.width;
    }
}


// =========================================================================
//                   TRANSLATION COLORMAP CODE
// =========================================================================

char *Color_Names[MAXSKINCOLORS+1]={
   "",
   "White",
   "Silver",
   "Gray", 
   "Pink",
   "Cherry_Blossom", // I made this color! :D Nozomi Date Unknown
   "Bright_Red", // Someone asked for this... Nozomi Date Unknown
   "Red",
   "Crimson", 
   "Peach",
   "Apricot", // used to be Orange Nozomi 03-25-2026
   "Beige",
   "Lemon",
   "Yellow",
   "Green",
   "Forest", // Take a hint :)
   "Light_Blue",
   "Blue", // An in-between of Light and Deep! Nozomi Date Unknown
   "Deep_Blue",
   "Purple",
   "Legacy_Army", // MIDIMan jokingly asked me to port this, I did it :D Nozomi Date Unknown
};

#if MAXSKINCOLORS > 32
#error MAXSKINCOLORS is greater than 32, please update r_draw.c to reflect this!
#endif
CV_PossibleValue_t Color_cons_t[]={{0,NULL}, {1,NULL}, {2,NULL}, {3,NULL},
                                   {4,NULL}, {5,NULL}, {6,NULL}, {7,NULL},
                                   {8,NULL}, {9,NULL}, {10,NULL},{11,NULL},
                                   {12,NULL},{13,NULL},{14,NULL},{15,NULL},
                                   {16,NULL},{17,NULL},{18,NULL},{19,NULL},
                                   {20,NULL},{21,NULL},{22,NULL},{23,NULL},
                                   {24,NULL},{25,NULL},{26,NULL},{27,NULL},
                                   {28,NULL},{29,NULL},{30,NULL},{31,NULL},
                                                                 {0,NULL}};

//  Creates the translation tables to map the green color ramp to
//  another ramp (gray, brown, red, ...)
//
//  This is precalculated for drawing the player sprites in the player's
//  chosen color
//
void R_InitTranslationTables (void)
{
    int         i,j;

    //added:11-01-98: load here the transparency lookup tables 'TINTTAB'
    // NOTE: the TINTTAB resource MUST BE aligned on 64k for the asm optimised
    //       (in other words, transtables pointer low word is 0)
    transtables = Z_MallocAlign (NUMTRANSTABLES*0x10000, PU_STATIC, 0, 16);

    // load in translucency tables
    W_ReadLump( W_GetNumForName("TRANSMED"), transtables );
    W_ReadLump( W_GetNumForName("TRANSMOR"), transtables+0x10000 );
    W_ReadLump( W_GetNumForName("TRANSHI"),  transtables+0x20000 );
    W_ReadLump( W_GetNumForName("TRANSFIR"), transtables+0x30000 );
    W_ReadLump( W_GetNumForName("TRANSFX1"), transtables+0x40000 );

    translationtables = Z_MallocAlign (256*(MAXSKINCOLORS+MAXSUPERCOLORS), PU_STATIC, 0, 8);
	fadetables = Z_MallocAlign(256*10, PU_STATIC, 0, 8);

    // translate just the 16 green colors
    for (i=0 ; i<256 ; i++)
    {
        if (i >= 0x70 && i<= 0x7f)
        {
            translationtables [i + SKINCOLOR_GREY*256] = 0x60 + (i&0xf); // Gray
			translationtables [i + SKINCOLOR_PINK*256] = 0x10 + (i&0xf); // Pink
			translationtables [i + SKINCOLOR_CHERRYBLOSSOM*256] = 0xa8 + (i&0xf)/3;   // Cherry Blossom by Nozomi
			translationtables [i + SKINCOLOR_RED*256] = 0xb0 + (i&0xf); // Red (renamed from Light Red)
            translationtables [i + SKINCOLOR_CRIMSON*256] = 0x20 + (i&0xf); // Crimson (renamed from Red)
			translationtables [i + SKINCOLOR_PEACH*256] = 0x30 + (i&0xf); // Peach
			translationtables [i + SKINCOLOR_APRICOT*256] = 0xd0 + (i&0xf); // Apricot (renamed from Orange)
			translationtables [i + SKINCOLOR_BEIGE*256] = 0x80 + (i&0xf); // Beige
			translationtables [i + SKINCOLOR_GREEN*256] = 0x70 + (i&0xf); // Green
			translationtables [i + SKINCOLOR_FOREST*256] = 0x78 + (i&0xf)/2; // Forest by Nozomi
			translationtables [i + SKINCOLOR_LIGHTBLUE*256] = 0xc0 + (i&0xf); // Light Blue

			if ((i&0xf) <4)
				translationtables [i + SKINCOLOR_BRIGHTRED*256] = 0xac + (i&0xf);   // Bright Red by Nozomi
            else
				translationtables [i + SKINCOLOR_BRIGHTRED*256] = 0xb0-4 + (i&0xf);

			if ((i&0xf) < 8)
				translationtables [i + SKINCOLOR_LEMON*256] = 0xe0 + (i&0xf); // Lemon by Nozomi
			else
				translationtables [i + SKINCOLOR_LEMON*256] = 0xa0-8 + (i&0xf);

			if ((i&0xf) < 13)
				translationtables [i + SKINCOLOR_BLUE*256] = 0xc3 + (i&0xf); // Blue
			else
				translationtables [i + SKINCOLOR_BLUE*256] = 0xf0-13 + (i&0xf);

            if ((i&0xf) <9)
               translationtables [i + SKINCOLOR_DEEPBLUE*256] = 0xc7 + (i&0xf);   // Deep Blue (renamed from Dark Blue)
            else
               translationtables [i + SKINCOLOR_DEEPBLUE*256] = 0xf0-9 + (i&0xf);

			if ((i&0xf) <8)
               translationtables [i + SKINCOLOR_LEGACYARMY*256] = 0x98 + (i&0xf);   // Army by SSNTails circa. 02-19-2000
            else												                  // Renamed to Legacy Army by Nozomi
               translationtables [i + SKINCOLOR_LEGACYARMY*256] = 0x90-8 + (i&0xf);
        }
        else
        {
            // Keep all other colors as is.
            for (j=0;j<(MAXSKINCOLORS+MAXSUPERCOLORS)*256;j+=256)
                translationtables [i+j] = i;
        }
    }

	// White
	// Provided by Nozomi
	translationtables [0x70 + SKINCOLOR_WHITE*256] = 4;
	translationtables [0x71 + SKINCOLOR_WHITE*256] = 4;
	translationtables [0x72 + SKINCOLOR_WHITE*256] = 4;
	translationtables [0x73 + SKINCOLOR_WHITE*256] = 4;
	translationtables [0x74 + SKINCOLOR_WHITE*256] = 4;
	translationtables [0x75 + SKINCOLOR_WHITE*256] = 4;
	translationtables [0x76 + SKINCOLOR_WHITE*256] = 80;
	translationtables [0x77 + SKINCOLOR_WHITE*256] = 81;
	translationtables [0x78 + SKINCOLOR_WHITE*256] = 83;
	translationtables [0x79 + SKINCOLOR_WHITE*256] = 85;
	translationtables [0x7a + SKINCOLOR_WHITE*256] = 87;
	translationtables [0x7b + SKINCOLOR_WHITE*256] = 89;
	translationtables [0x7c + SKINCOLOR_WHITE*256] = 91;
	translationtables [0x7d + SKINCOLOR_WHITE*256] = 93;
	translationtables [0x7e + SKINCOLOR_WHITE*256] = 95;
	translationtables [0x7f + SKINCOLOR_WHITE*256] = 97;

	// Silver
	// Provided by Nozomi
	translationtables [0x70 + SKINCOLOR_SILVER*256] = 4;
	translationtables [0x71 + SKINCOLOR_SILVER*256] = 80;
	translationtables [0x72 + SKINCOLOR_SILVER*256] = 82;
	translationtables [0x73 + SKINCOLOR_SILVER*256] = 85;
	translationtables [0x74 + SKINCOLOR_SILVER*256] = 87;
	translationtables [0x75 + SKINCOLOR_SILVER*256] = 90;
	translationtables [0x76 + SKINCOLOR_SILVER*256] = 93;
	translationtables [0x77 + SKINCOLOR_SILVER*256] = 95; // Windows 95
	translationtables [0x78 + SKINCOLOR_SILVER*256] = 98; // Windows 98 :pink_heart:
	translationtables [0x79 + SKINCOLOR_SILVER*256] = 101;
	translationtables [0x7a + SKINCOLOR_SILVER*256] = 103;
	translationtables [0x7b + SKINCOLOR_SILVER*256] = 106;
	translationtables [0x7c + SKINCOLOR_SILVER*256] = 108;
	translationtables [0x7d + SKINCOLOR_SILVER*256] = 111;
	translationtables [0x7e + SKINCOLOR_SILVER*256] = 6;
	translationtables [0x7f + SKINCOLOR_SILVER*256] = 0;

	// Yellow
	// Provided by Nozomi
	translationtables [0x70 + SKINCOLOR_YELLOW*256] = 227;
	translationtables [0x71 + SKINCOLOR_YELLOW*256] = 230;
	translationtables [0x72 + SKINCOLOR_YELLOW*256] = 231;
	translationtables [0x73 + SKINCOLOR_YELLOW*256] = 160;
	translationtables [0x74 + SKINCOLOR_YELLOW*256] = 160;
	translationtables [0x75 + SKINCOLOR_YELLOW*256] = 161;
	translationtables [0x76 + SKINCOLOR_YELLOW*256] = 162;
	translationtables [0x77 + SKINCOLOR_YELLOW*256] = 163;
	translationtables [0x78 + SKINCOLOR_YELLOW*256] = 164;
	translationtables [0x79 + SKINCOLOR_YELLOW*256] = 164;
	translationtables [0x7a + SKINCOLOR_YELLOW*256] = 164;
	translationtables [0x7b + SKINCOLOR_YELLOW*256] = 149;
	translationtables [0x7c + SKINCOLOR_YELLOW*256] = 150;
	translationtables [0x7d + SKINCOLOR_YELLOW*256] = 151;
	translationtables [0x7e + SKINCOLOR_YELLOW*256] = 7;
	translationtables [0x7f + SKINCOLOR_YELLOW*256] = 0;

	// Purple
	// Provided by ova pico
	translationtables [0x70 + SKINCOLOR_PURPLE*256] = 4;
	translationtables [0x71 + SKINCOLOR_PURPLE*256] = 51;
	translationtables [0x72 + SKINCOLOR_PURPLE*256] = 250;
	translationtables [0x73 + SKINCOLOR_PURPLE*256] = 251;
	translationtables [0x74 + SKINCOLOR_PURPLE*256] = 251;
	translationtables [0x75 + SKINCOLOR_PURPLE*256] = 252;
	translationtables [0x76 + SKINCOLOR_PURPLE*256] = 252;
	translationtables [0x77 + SKINCOLOR_PURPLE*256] = 253;
	translationtables [0x78 + SKINCOLOR_PURPLE*256] = 253;
	translationtables [0x79 + SKINCOLOR_PURPLE*256] = 254;
	translationtables [0x7a + SKINCOLOR_PURPLE*256] = 254;
	translationtables [0x7b + SKINCOLOR_PURPLE*256] = 254;
	translationtables [0x7c + SKINCOLOR_PURPLE*256] = 109;
	translationtables [0x7d + SKINCOLOR_PURPLE*256] = 79;
	translationtables [0x7e + SKINCOLOR_PURPLE*256] = 7;
	translationtables [0x7f + SKINCOLOR_PURPLE*256] = 0;

	// Start Super Skincolors! Nozomi

	// deadass copying lemon first, i'm sowwi
	translationtables [0x70 + SKINCOLOR_SUPER*256] = 0xe0;
	translationtables [0x71 + SKINCOLOR_SUPER*256] = 0xe1;
	translationtables [0x72 + SKINCOLOR_SUPER*256] = 0xe2;
	translationtables [0x73 + SKINCOLOR_SUPER*256] = 0xe3;
	translationtables [0x74 + SKINCOLOR_SUPER*256] = 0xe4;
	translationtables [0x75 + SKINCOLOR_SUPER*256] = 0xe5;
	translationtables [0x76 + SKINCOLOR_SUPER*256] = 0xe6;
	translationtables [0x77 + SKINCOLOR_SUPER*256] = 0xe7;
	translationtables [0x78 + SKINCOLOR_SUPER*256] = 0xa0;
	translationtables [0x79 + SKINCOLOR_SUPER*256] = 0xa1;
	translationtables [0x7a + SKINCOLOR_SUPER*256] = 0xa2;
	translationtables [0x7b + SKINCOLOR_SUPER*256] = 0xa3;
	translationtables [0x7c + SKINCOLOR_SUPER*256] = 0xa4;
	translationtables [0x7d + SKINCOLOR_SUPER*256] = 0xa5;
	translationtables [0x7e + SKINCOLOR_SUPER*256] = 0xa6;
	translationtables [0x7f + SKINCOLOR_SUPER*256] = 0xa7;

	translationtables [0x70 + SKINCOLOR_SUPER2*256] = 0xe2;
	translationtables [0x71 + SKINCOLOR_SUPER2*256] = 0xe2;
	translationtables [0x72 + SKINCOLOR_SUPER2*256] = 0xe3;
	translationtables [0x73 + SKINCOLOR_SUPER2*256] = 0xe4;
	translationtables [0x74 + SKINCOLOR_SUPER2*256] = 0xe5;
	translationtables [0x75 + SKINCOLOR_SUPER2*256] = 0xe5;
	translationtables [0x76 + SKINCOLOR_SUPER2*256] = 0xe6;
	translationtables [0x77 + SKINCOLOR_SUPER2*256] = 0xe7;
	translationtables [0x78 + SKINCOLOR_SUPER2*256] = 0xa0;
	translationtables [0x79 + SKINCOLOR_SUPER2*256] = 0xa1;
	translationtables [0x7a + SKINCOLOR_SUPER2*256] = 0xa2;
	translationtables [0x7b + SKINCOLOR_SUPER2*256] = 0xa3;
	translationtables [0x7c + SKINCOLOR_SUPER2*256] = 0xa4;
	translationtables [0x7d + SKINCOLOR_SUPER2*256] = 0xa5;
	translationtables [0x7e + SKINCOLOR_SUPER2*256] = 0xa6;
	translationtables [0x7f + SKINCOLOR_SUPER2*256] = 0xa7;

	translationtables [0x70 + SKINCOLOR_SUPER3*256] = 0xe5;
	translationtables [0x71 + SKINCOLOR_SUPER3*256] = 0xe6;
	translationtables [0x72 + SKINCOLOR_SUPER3*256] = 0xe7;
	translationtables [0x73 + SKINCOLOR_SUPER3*256] = 0xe7;
	translationtables [0x74 + SKINCOLOR_SUPER3*256] = 0xa0;
	translationtables [0x75 + SKINCOLOR_SUPER3*256] = 0xa0;
	translationtables [0x76 + SKINCOLOR_SUPER3*256] = 0xa1;
	translationtables [0x77 + SKINCOLOR_SUPER3*256] = 0xa1;
	translationtables [0x78 + SKINCOLOR_SUPER3*256] = 0xa2;
	translationtables [0x79 + SKINCOLOR_SUPER3*256] = 0xa2;
	translationtables [0x7a + SKINCOLOR_SUPER3*256] = 0xa3;
	translationtables [0x7b + SKINCOLOR_SUPER3*256] = 0xa4;
	translationtables [0x7c + SKINCOLOR_SUPER3*256] = 0xa5;
	translationtables [0x7d + SKINCOLOR_SUPER3*256] = 0xa6;
	translationtables [0x7e + SKINCOLOR_SUPER3*256] = 0xa7;
	translationtables [0x7f + SKINCOLOR_SUPER3*256] = 0xa7;

	translationtables [0x70 + SKINCOLOR_SUPER4*256] = 0xe7;
	translationtables [0x71 + SKINCOLOR_SUPER4*256] = 0xe7;
	translationtables [0x72 + SKINCOLOR_SUPER4*256] = 0xa0;
	translationtables [0x73 + SKINCOLOR_SUPER4*256] = 0xa0;
	translationtables [0x74 + SKINCOLOR_SUPER4*256] = 0xa1;
	translationtables [0x75 + SKINCOLOR_SUPER4*256] = 0xa1;
	translationtables [0x76 + SKINCOLOR_SUPER4*256] = 0xa2;
	translationtables [0x77 + SKINCOLOR_SUPER4*256] = 0xa2;
	translationtables [0x78 + SKINCOLOR_SUPER4*256] = 0xa3;
	translationtables [0x79 + SKINCOLOR_SUPER4*256] = 0xa4;
	translationtables [0x7a + SKINCOLOR_SUPER4*256] = 0xa4;
	translationtables [0x7b + SKINCOLOR_SUPER4*256] = 0xa5;
	translationtables [0x7c + SKINCOLOR_SUPER4*256] = 0xa6;
	translationtables [0x7d + SKINCOLOR_SUPER4*256] = 0xa6;
	translationtables [0x7e + SKINCOLOR_SUPER4*256] = 0xa7;
	translationtables [0x7f + SKINCOLOR_SUPER4*256] = 0xa7;

	translationtables [0x70 + SKINCOLOR_SUPER5*256] = 0xa0;
	translationtables [0x71 + SKINCOLOR_SUPER5*256] = 0xa0;
	translationtables [0x72 + SKINCOLOR_SUPER5*256] = 0xa1;
	translationtables [0x73 + SKINCOLOR_SUPER5*256] = 0xa1;
	translationtables [0x74 + SKINCOLOR_SUPER5*256] = 0xa2;
	translationtables [0x75 + SKINCOLOR_SUPER5*256] = 0xa2;
	translationtables [0x76 + SKINCOLOR_SUPER5*256] = 0xa3;
	translationtables [0x77 + SKINCOLOR_SUPER5*256] = 0xa3;
	translationtables [0x78 + SKINCOLOR_SUPER5*256] = 0xa4;
	translationtables [0x79 + SKINCOLOR_SUPER5*256] = 0xa4;
	translationtables [0x7a + SKINCOLOR_SUPER5*256] = 0xa5;
	translationtables [0x7b + SKINCOLOR_SUPER5*256] = 0xa5;
	translationtables [0x7c + SKINCOLOR_SUPER5*256] = 0xa6;
	translationtables [0x7d + SKINCOLOR_SUPER5*256] = 0xa6;
	translationtables [0x7e + SKINCOLOR_SUPER5*256] = 0xa7;
	translationtables [0x7f + SKINCOLOR_SUPER5*256] = 0xa7;

	// End Super Skincolors! Nozomi

	// Start Hyper Skincolors! Nozomi

	// Yellow, swapped indexes with White for ease of access.
	translationtables [0x70 + SKINCOLOR_HYPER2*256] = 0xe2;
	translationtables [0x71 + SKINCOLOR_HYPER2*256] = 0xe2;
	translationtables [0x72 + SKINCOLOR_HYPER2*256] = 0xe3;
	translationtables [0x73 + SKINCOLOR_HYPER2*256] = 0xe4;
	translationtables [0x74 + SKINCOLOR_HYPER2*256] = 0xe5;
	translationtables [0x75 + SKINCOLOR_HYPER2*256] = 0xe5;
	translationtables [0x76 + SKINCOLOR_HYPER2*256] = 0xe6;
	translationtables [0x77 + SKINCOLOR_HYPER2*256] = 0xe7;
	translationtables [0x78 + SKINCOLOR_HYPER2*256] = 0xa0;
	translationtables [0x79 + SKINCOLOR_HYPER2*256] = 0xa1;
	translationtables [0x7a + SKINCOLOR_HYPER2*256] = 0xa2;
	translationtables [0x7b + SKINCOLOR_HYPER2*256] = 0xa3;
	translationtables [0x7c + SKINCOLOR_HYPER2*256] = 0xa4;
	translationtables [0x7d + SKINCOLOR_HYPER2*256] = 0xa5;
	translationtables [0x7e + SKINCOLOR_HYPER2*256] = 0xa6;
	translationtables [0x7f + SKINCOLOR_HYPER2*256] = 0xa7;

	// White
	translationtables [0x70 + SKINCOLOR_HYPER*256] = 168;
	translationtables [0x71 + SKINCOLOR_HYPER*256] = 168;
	translationtables [0x72 + SKINCOLOR_HYPER*256] = 0x50;
	translationtables [0x73 + SKINCOLOR_HYPER*256] = 0x51;
	translationtables [0x74 + SKINCOLOR_HYPER*256] = 0x52;
	translationtables [0x75 + SKINCOLOR_HYPER*256] = 0x52;
	translationtables [0x76 + SKINCOLOR_HYPER*256] = 0x53;
	translationtables [0x77 + SKINCOLOR_HYPER*256] = 0x54;
	translationtables [0x78 + SKINCOLOR_HYPER*256] = 0x55;
	translationtables [0x79 + SKINCOLOR_HYPER*256] = 0x56;
	translationtables [0x7a + SKINCOLOR_HYPER*256] = 0x57;
	translationtables [0x7b + SKINCOLOR_HYPER*256] = 0x58;
	translationtables [0x7c + SKINCOLOR_HYPER*256] = 0x59;
	translationtables [0x7d + SKINCOLOR_HYPER*256] = 0x5a;
	translationtables [0x7e + SKINCOLOR_HYPER*256] = 0x5b;
	translationtables [0x7f + SKINCOLOR_HYPER*256] = 0x5c;

	// Pink (I'm just going to copy Cherry Blossom a bit...)
	translationtables [0x70 + SKINCOLOR_HYPER3*256] = translationtables [0x71 + SKINCOLOR_CHERRYBLOSSOM*256];
	translationtables [0x71 + SKINCOLOR_HYPER3*256] = translationtables [0x71 + SKINCOLOR_CHERRYBLOSSOM*256];
	translationtables [0x72 + SKINCOLOR_HYPER3*256] = translationtables [0x72 + SKINCOLOR_CHERRYBLOSSOM*256];
	translationtables [0x73 + SKINCOLOR_HYPER3*256] = translationtables [0x72 + SKINCOLOR_CHERRYBLOSSOM*256];
	translationtables [0x74 + SKINCOLOR_HYPER3*256] = translationtables [0x72 + SKINCOLOR_CHERRYBLOSSOM*256];
	translationtables [0x75 + SKINCOLOR_HYPER3*256] = translationtables [0x73 + SKINCOLOR_CHERRYBLOSSOM*256];
	translationtables [0x76 + SKINCOLOR_HYPER3*256] = translationtables [0x73 + SKINCOLOR_CHERRYBLOSSOM*256];
	translationtables [0x77 + SKINCOLOR_HYPER3*256] = translationtables [0x73 + SKINCOLOR_CHERRYBLOSSOM*256];
	translationtables [0x78 + SKINCOLOR_HYPER3*256] = translationtables [0x73 + SKINCOLOR_CHERRYBLOSSOM*256];
	translationtables [0x79 + SKINCOLOR_HYPER3*256] = translationtables [0x73 + SKINCOLOR_CHERRYBLOSSOM*256];
	translationtables [0x7a + SKINCOLOR_HYPER3*256] = translationtables [0x73 + SKINCOLOR_CHERRYBLOSSOM*256];
	translationtables [0x7b + SKINCOLOR_HYPER3*256] = translationtables [0x74 + SKINCOLOR_CHERRYBLOSSOM*256];
	translationtables [0x7c + SKINCOLOR_HYPER3*256] = translationtables [0x74 + SKINCOLOR_CHERRYBLOSSOM*256];
	translationtables [0x7d + SKINCOLOR_HYPER3*256] = translationtables [0x74 + SKINCOLOR_CHERRYBLOSSOM*256];
	translationtables [0x7e + SKINCOLOR_HYPER3*256] = translationtables [0x75 + SKINCOLOR_CHERRYBLOSSOM*256];
	translationtables [0x7f + SKINCOLOR_HYPER3*256] = translationtables [0x76 + SKINCOLOR_CHERRYBLOSSOM*256];

	// Pastel...? I can't really make this color with what I have... We're using purple.

	translationtables [0x70 + SKINCOLOR_HYPER4*256] = 168;
	translationtables [0x71 + SKINCOLOR_HYPER4*256] = 168;
	translationtables [0x72 + SKINCOLOR_HYPER4*256] = 169;
	translationtables [0x73 + SKINCOLOR_HYPER4*256] = 169;
	translationtables [0x74 + SKINCOLOR_HYPER4*256] = 169;
	translationtables [0x75 + SKINCOLOR_HYPER4*256] = 250;
	translationtables [0x76 + SKINCOLOR_HYPER4*256] = 250;
	translationtables [0x77 + SKINCOLOR_HYPER4*256] = 250;
	translationtables [0x78 + SKINCOLOR_HYPER4*256] = 250;
	translationtables [0x79 + SKINCOLOR_HYPER4*256] = 250;
	translationtables [0x7a + SKINCOLOR_HYPER4*256] = 250;
	translationtables [0x7b + SKINCOLOR_HYPER4*256] = 251;
	translationtables [0x7c + SKINCOLOR_HYPER4*256] = 251;
	translationtables [0x7d + SKINCOLOR_HYPER4*256] = 251;
	translationtables [0x7e + SKINCOLOR_HYPER4*256] = 252;
	translationtables [0x7f + SKINCOLOR_HYPER4*256] = 253;

	// Blue

	translationtables [0x70 + SKINCOLOR_HYPER5*256] = 192;
	translationtables [0x71 + SKINCOLOR_HYPER5*256] = 192;
	translationtables [0x72 + SKINCOLOR_HYPER5*256] = 193;
	translationtables [0x73 + SKINCOLOR_HYPER5*256] = 193;
	translationtables [0x74 + SKINCOLOR_HYPER5*256] = 193;
	translationtables [0x75 + SKINCOLOR_HYPER5*256] = 194;
	translationtables [0x76 + SKINCOLOR_HYPER5*256] = 194;
	translationtables [0x77 + SKINCOLOR_HYPER5*256] = 194;
	translationtables [0x78 + SKINCOLOR_HYPER5*256] = 194;
	translationtables [0x79 + SKINCOLOR_HYPER5*256] = 194;
	translationtables [0x7a + SKINCOLOR_HYPER5*256] = 194;
	translationtables [0x7b + SKINCOLOR_HYPER5*256] = 195;
	translationtables [0x7c + SKINCOLOR_HYPER5*256] = 195;
	translationtables [0x7d + SKINCOLOR_HYPER5*256] = 195;
	translationtables [0x7e + SKINCOLOR_HYPER5*256] = 196;
	translationtables [0x7f + SKINCOLOR_HYPER5*256] = 197;

	// Green

	translationtables [0x70 + SKINCOLOR_HYPER6*256] = 192;
	translationtables [0x71 + SKINCOLOR_HYPER6*256] = 192;
	translationtables [0x72 + SKINCOLOR_HYPER6*256] = 112;
	translationtables [0x73 + SKINCOLOR_HYPER6*256] = 112;
	translationtables [0x74 + SKINCOLOR_HYPER6*256] = 112;
	translationtables [0x75 + SKINCOLOR_HYPER6*256] = 113;
	translationtables [0x76 + SKINCOLOR_HYPER6*256] = 113;
	translationtables [0x77 + SKINCOLOR_HYPER6*256] = 113;
	translationtables [0x78 + SKINCOLOR_HYPER6*256] = 113;
	translationtables [0x79 + SKINCOLOR_HYPER6*256] = 113;
	translationtables [0x7a + SKINCOLOR_HYPER6*256] = 113;
	translationtables [0x7b + SKINCOLOR_HYPER6*256] = 114;
	translationtables [0x7c + SKINCOLOR_HYPER6*256] = 114;
	translationtables [0x7d + SKINCOLOR_HYPER6*256] = 114;
	translationtables [0x7e + SKINCOLOR_HYPER6*256] = 115;
	translationtables [0x7f + SKINCOLOR_HYPER6*256] = 116;

	// Merky Yellow

	translationtables [0x70 + SKINCOLOR_HYPER7*256] = 209;
	translationtables [0x71 + SKINCOLOR_HYPER7*256] = 209;
	translationtables [0x72 + SKINCOLOR_HYPER7*256] = 160;
	translationtables [0x73 + SKINCOLOR_HYPER7*256] = 160;
	translationtables [0x74 + SKINCOLOR_HYPER7*256] = 160;
	translationtables [0x75 + SKINCOLOR_HYPER7*256] = 161;
	translationtables [0x76 + SKINCOLOR_HYPER7*256] = 161;
	translationtables [0x77 + SKINCOLOR_HYPER7*256] = 161;
	translationtables [0x78 + SKINCOLOR_HYPER7*256] = 161;
	translationtables [0x79 + SKINCOLOR_HYPER7*256] = 161;
	translationtables [0x7a + SKINCOLOR_HYPER7*256] = 161;
	translationtables [0x7b + SKINCOLOR_HYPER7*256] = 162;
	translationtables [0x7c + SKINCOLOR_HYPER7*256] = 162;
	translationtables [0x7d + SKINCOLOR_HYPER7*256] = 162;
	translationtables [0x7e + SKINCOLOR_HYPER7*256] = 163;
	translationtables [0x7f + SKINCOLOR_HYPER7*256] = 164;


	// End Hyper Skincolors! Nozomi

	// Start Fade Tables! Nozomi

	// Initializing them because lazy nya~ Nozomi
	// Valid fadenums are 0-9...
	// 0 is pitch black
	// 9 is almost normal
	// Nozomi 04-02-2026
	for (i=0; i<256; i++)
		for (j=0; j<256*9; j+=256)
			fadetables[i+j] = i;

	// Fade Table 0: Pitch Black
	for (i=0; i<256; i++)
		fadetables[i] = 0;

	// Greyscale
	for (i=80; i<112; i++)
		for (j=1; j<9; j++)
			fadetables[i+j*256] = floor((float)i + ((float)i - ((float)i * ((float)i / 112.0f) ) )/((float)(j)));

	// Red
	for (i=168; i<192; i++)
		for (j=1; j<9; j++)
			fadetables[i+j*256] = floor((float)i + ((float)i - ((float)i * ((float)i / 192.0f) ) )/((float)(j)));

	// Orange
	for (i=208; i<224; i++)
		for (j=1; j<9; j++)
			fadetables[i+j*256] = floor((float)i + ((float)i - ((float)i * ((float)i / 224.0f) ) )/((float)(j)));

	for (i=232; i<236; i++)
		for (j=1; j<9; j++)
			fadetables[i+j*256] = floor((float)i + ((float)i - ((float)i * ((float)i / 236.0f) ) )/((float)(j)));

	for (j=1; j<9; j++)
			fadetables[248+j*256] = floor((float)214 + ((float)214 - ((float)214 * ((float)214 / 224.0f) ) )/((float)(j)));

	// Yellow
	for (i=224; i<232; i++) // This one is ugly... Nozomi
		for (j=1; j<9; j++)
			fadetables[i+j*256] = floor((float)i + ((float)i - ((float)i * ((float)i / 232.0f) ) )/((float)(j)));

	for (i=160; i<168; i++)
		for (j=1; j<9; j++)
			fadetables[i+j*256] = floor((float)i + ((float)i - ((float)i * ((float)i / 168.0f) ) )/((float)(j)));

	for (j=1; j<9; j++) // I'm cheating for this one.
			fadetables[249+j*256] = floor((float)160 + ((float)160 - ((float)160 * ((float)160 / 168.0f) ) )/((float)(j)));

	// Green
	for (i=112; i<128; i++)
		for (j=1; j<9; j++)
			fadetables[i+j*256] = floor((float)i + ((float)i - ((float)i * ((float)i / 128.0f) ) )/((float)(j)));

	// Blue
	for (i=192; i<208; i++)
		for (j=1; j<9; j++)
			fadetables[i+j*256] = floor((float)i + ((float)i - ((float)i * ((float)i / 208.0f) ) )/((float)(j)));

	// Violet
	for (i=250; i<255; i++)
		for (j=1; j<9; j++)
			fadetables[i+j*256] = floor((float)i + ((float)i - ((float)i * ((float)i / 255.0f) ) )/((float)(j)));

	// Armor
	for (i=152; i<160; i++)
		for (j=1; j<9; j++)
			fadetables[i+j*256] = floor((float)i + ((float)i - ((float)i * ((float)i / 160.0f) ) )/((float)(j)));
	
	for (i=9; i<13; i++)
		for (j=1; j<9; j++)
			fadetables[i+j*256] = floor((float)i + ((float)i - ((float)i * ((float)i / 13.0f) ) )/((float)(j)));

	// Leather
	for (i=128; i<152; i++)
		for (j=1; j<9; j++)
			fadetables[i+j*256] = floor((float)i + ((float)i - ((float)i * ((float)i / 152.0f) ) )/((float)(j)));

	for (i=236; i<240; i++)
		for (j=1; j<9; j++)
			fadetables[i+j*256] = floor((float)i + ((float)i - ((float)i * ((float)i / 240.0f) ) )/((float)(j)));

	for (i=13; i<16; i++)
		for (j=1; j<9; j++)
			fadetables[i+j*256] = floor((float)i + ((float)i - ((float)i * ((float)i / 16.0f) ) )/((float)(j)));

	// Skin
	for (i=48; i<80; i++)
		for (j=1; j<9; j++)
			fadetables[i+j*256] = floor((float)i + ((float)i - ((float)i * ((float)i / 80.0f) ) )/((float)(j)));

	// Flesh
	for (i=16; i<48; i++)
		for (j=1; j<9; j++)
			fadetables[i+j*256] = floor((float)i + ((float)i - ((float)i * ((float)i / 48.0f) ) )/((float)(j)));

	// Deep Water
	for (i=240; i<248; i++)
		for (j=1; j<9; j++)
			fadetables[i+j*256] = floor((float)i + ((float)i - ((float)i * ((float)i / 248.0f) ) )/((float)(j)));

	// End Fade Tables! Nozomi
}


// ==========================================================================
//               COMMON DRAWER FOR 8 AND 16 BIT COLOR MODES
// ==========================================================================

// in a perfect world, all routines would be compatible for either mode,
// and optimised enough
//
// in reality, the few routines that can work for either mode, are
// put here


// R_InitViewBuffer
// Creates lookup tables for getting the framebuffer address
//  of a pixel to draw.
//
void R_InitViewBuffer ( int   width,
                        int   height )
{
    int         i;
    int         bytesperpixel = vid.bpp;

    if (bytesperpixel<1 || bytesperpixel>4)
        I_Error ("R_InitViewBuffer : wrong bytesperpixel value %d\n",
                 bytesperpixel);

    // Handle resize,
    //  e.g. smaller view windows
    //  with border and/or status bar.
    viewwindowx = (vid.width-width) >> 1;

    // Column offset for those columns of the view window, but
    // relative to the entire screen
    for (i=0 ; i<width ; i++)
        columnofs[i] = (viewwindowx + i) * bytesperpixel;

    // Same with base row offset.
    if (width == vid.width)
        viewwindowy = 0;
    else
        viewwindowy = (vid.height-ST_HEIGHT-height) >> 1;

    // Precalculate all row offsets.
    for (i=0 ; i<height ; i++)
    {
        ylookup[i] = ylookup1[i] = vid.buffer + (i+viewwindowy)*vid.width*bytesperpixel;
                     ylookup2[i] = vid.buffer + (i+(vid.height>>1))*vid.width*bytesperpixel;
    }
        

#ifdef HORIZONTALDRAW
    //Fab 17-06-98
    // create similar lookup tables for horizontal column draw optimisation

    // (the first column is the bottom line)
    for (i=0; i<width; i++)
        yhlookup[i] = screens[2] + ((width-i-1) * bytesperpixel * height);

    for (i=0; i<height; i++)
        hcolumnofs[i] = i * bytesperpixel;
#endif
}


//
// Store the lumpnumber of the viewborder patches.
//
    int viewborderlump[8];
void R_InitViewBorder (void)
{
    viewborderlump[BRDR_T] = W_GetNumForName ("brdr_t");
    viewborderlump[BRDR_B] = W_GetNumForName ("brdr_b");
    viewborderlump[BRDR_L] = W_GetNumForName ("brdr_l");
    viewborderlump[BRDR_R] = W_GetNumForName ("brdr_r");
    viewborderlump[BRDR_TL] = W_GetNumForName ("brdr_tl");
    viewborderlump[BRDR_BL] = W_GetNumForName ("brdr_bl");
    viewborderlump[BRDR_TR] = W_GetNumForName ("brdr_tr");
    viewborderlump[BRDR_BR] = W_GetNumForName ("brdr_br");
}


//
// R_FillBackScreen
// Fills the back screen with a pattern for variable screen sizes
// Also draws a beveled edge.
//
void R_FillBackScreen (void)
{
    byte*       src;
    byte*       dest;
    int         x;
    int         y;
    patch_t*    patch;
    
    //faB: quickfix, don't cache lumps in both modes
    if (rendermode!=render_soft)
        return;

     //added:08-01-98:draw pattern around the status bar too (when hires),
    //                so return only when in full-screen without status bar.
    if ((scaledviewwidth == vid.width)&&(viewheight==vid.height))
        return;

    src  = scr_borderpatch;
    dest = screens[1];

    for (y=0 ; y<vid.height-ST_HEIGHT ; y++)
    {
        for (x=0 ; x<vid.width/64 ; x++)
        {
            memcpy (dest, src+((y&63)<<6), 64);
            dest += 64;
        }

        if (vid.width&63)
        {
            memcpy (dest, src+((y&63)<<6), vid.width&63);
            dest += (vid.width&63);
        }
    }

    //added:08-01-98:dont draw the borders when viewwidth is full vid.width.
    if (scaledviewwidth == vid.width)
       return;
    
    patch = W_CacheLumpNum (viewborderlump[BRDR_T],PU_CACHE);
    for (x=0 ; x<scaledviewwidth ; x+=8)
        V_DrawPatch (viewwindowx+x,viewwindowy-8,1,patch);
    patch = W_CacheLumpNum (viewborderlump[BRDR_B],PU_CACHE);
    for (x=0 ; x<scaledviewwidth ; x+=8)
        V_DrawPatch (viewwindowx+x,viewwindowy+viewheight,1,patch);
    patch = W_CacheLumpNum (viewborderlump[BRDR_L],PU_CACHE);
    for (y=0 ; y<viewheight ; y+=8)
        V_DrawPatch (viewwindowx-8,viewwindowy+y,1,patch);
    patch = W_CacheLumpNum (viewborderlump[BRDR_R],PU_CACHE);
    for (y=0 ; y<viewheight ; y+=8)
        V_DrawPatch (viewwindowx+scaledviewwidth,viewwindowy+y,1,patch);

    // Draw beveled corners.
    V_DrawPatch (viewwindowx-8,
                 viewwindowy-8,
                 1,
                 W_CacheLumpNum (viewborderlump[BRDR_TL],PU_CACHE));

    V_DrawPatch (viewwindowx+scaledviewwidth,
                 viewwindowy-8,
                 1,
                 W_CacheLumpNum (viewborderlump[BRDR_TR],PU_CACHE));

    V_DrawPatch (viewwindowx-8,
                 viewwindowy+viewheight,
                 1,
                 W_CacheLumpNum (viewborderlump[BRDR_BL],PU_CACHE));

    V_DrawPatch (viewwindowx+scaledviewwidth,
                 viewwindowy+viewheight,
                 1,
                 W_CacheLumpNum (viewborderlump[BRDR_BR],PU_CACHE));
}


//
// Copy a screen buffer.
//
void R_VideoErase (unsigned ofs, int count)
{
    // LFB copy.
    // This might not be a good idea if memcpy
    //  is not optiomal, e.g. byte by byte on
    //  a 32bit CPU, as GNU GCC/Linux libc did
    //  at one point.
    memcpy (screens[0]+ofs, screens[1]+ofs, count);
}


//
// R_DrawViewBorder
// Draws the border around the view
//  for different size windows?
//
void R_DrawViewBorder (void)
{
    int         top;
    int         side;
    int         ofs;

#ifdef HWRENDER // not win32 only 19990829 by Kin
    if (rendermode != render_soft)
    {
        HWR_DrawViewBorder (0);
        return;
    }
#endif


#ifdef DEBUG
    fprintf(stderr,"RDVB: vidwidth %d vidheight %d scaledviewwidth %d viewheight %d\n",
             vid.width,vid.height,scaledviewwidth,viewheight);
#endif

     //added:08-01-98: draw the backtile pattern around the status bar too
    //                 (when statusbar width is shorter than vid.width)
    /*
    if( (vid.width>ST_WIDTH) && (vid.height!=viewheight) )
    {
        ofs  = (vid.height-ST_HEIGHT)*vid.width;
        side = (vid.width-ST_WIDTH)>>1;
        R_VideoErase(ofs,side);

        ofs += (vid.width-side);
        for (i=1;i<ST_HEIGHT;i++)
        {
            R_VideoErase(ofs,side<<1);  //wraparound right to left border
            ofs += vid.width;
        }
        R_VideoErase(ofs,side);
    }*/

    if (scaledviewwidth == vid.width)
        return;

    top  = (vid.height-ST_HEIGHT-viewheight) >>1;
    side = (vid.width-scaledviewwidth) >>1;

    // copy top and one line of left side
    R_VideoErase (0, top*vid.width+side);

    // copy one line of right side and bottom
    ofs = (viewheight+top)*vid.width-side;
    R_VideoErase (ofs, top*vid.width+side);

    // copy sides using wraparound
    ofs = top*vid.width + vid.width-side;
    side <<= 1;

    //added:05-02-98:simpler using our new VID_Blit routine
    VID_BlitLinearScreen(screens[1]+ofs, screens[0]+ofs,
                         side, viewheight-1, vid.width, vid.width);

    // useless, old dirty rectangle stuff
    //V_MarkRect (0,0,vid.width, vid.height-ST_HEIGHT);
}


// ==========================================================================
//                   INCLUDE 8bpp DRAWING CODE HERE
// ==========================================================================

#include "r_draw8.c"


// ==========================================================================
//                   INCLUDE 16bpp DRAWING CODE HERE
// ==========================================================================

#include "r_draw16.c"
