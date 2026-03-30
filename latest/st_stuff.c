// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: st_stuff.c,v 1.2 2000/02/27 00:42:11 hurdler Exp $
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
// $Log: st_stuff.c,v $
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Status bar code.
//      Does the face/direction indicator animatin.
//      Does palette indicators as well (red pain/berserk, bright pickup)
//
//-----------------------------------------------------------------------------

#include "doomdef.h"

#include "am_map.h"

#include "g_game.h"
#include "m_cheat.h"

#include "screen.h"
#include "r_local.h"
#include "p_local.h"
#include "p_inter.h"
#include "m_random.h"

#include "st_stuff.h"
#include "st_lib.h"
#include "i_video.h"
#include "v_video.h"

#include "keys.h"

#include "z_zone.h"

#include "stdlib.h" // Tails 06-06-2000

#include "m_easing.h"

#ifdef HWRENDER
#include "hardware/hw_drv.h"
#include "hardware/hw_main.h"
#endif

//protos
void ST_createWidgets(void);

extern fixed_t waterheight;

//
// STATUS BAR DATA
//

// Palette indices.
// For damage/bonus red-/gold-shifts
#define STARTREDPALS            1
#define STARTBONUSPALS          9
#define NUMREDPALS              8
#define NUMBONUSPALS            4
// Radiation suit, green shift.
#define RADIATIONPAL			13

// N/256*100% probability
//  that the normal face state will change
#define ST_FACEPROBABILITY              96

// For Responder
#define ST_TOGGLECHAT           KEY_ENTER

// Location of status bar
  //added:08-01-98:status bar position changes according to resolution.
#define ST_FX                     143

// Should be set to patch width
//  for tall numbers later on
#define ST_TALLNUMWIDTH         (tallnum[0]->width)

// Number of status faces.
#define ST_NUMPAINFACES         5
#define ST_NUMSTRAIGHTFACES     3
#define ST_NUMTURNFACES         2
#define ST_NUMSPECIALFACES      3

#define ST_FACESTRIDE \
          (ST_NUMSTRAIGHTFACES+ST_NUMTURNFACES+ST_NUMSPECIALFACES)

#define ST_NUMEXTRAFACES        2

#define ST_NUMFACES \
          (ST_FACESTRIDE*ST_NUMPAINFACES+ST_NUMEXTRAFACES)

#define ST_TURNOFFSET           (ST_NUMSTRAIGHTFACES)
#define ST_OUCHOFFSET           (ST_TURNOFFSET + ST_NUMTURNFACES)
#define ST_EVILGRINOFFSET       (ST_OUCHOFFSET + 1)
#define ST_RAMPAGEOFFSET        (ST_EVILGRINOFFSET + 1)
#define ST_GODFACE              (ST_NUMPAINFACES*ST_FACESTRIDE)
#define ST_DEADFACE             (ST_GODFACE+1)

#define ST_FACESX               143
#define ST_FACESY               (ST_Y+0)

#define ST_EVILGRINCOUNT        (2*TICRATE)
#define ST_STRAIGHTFACECOUNT    (TICRATE/2)
#define ST_TURNCOUNT            (1*TICRATE)
#define ST_OUCHCOUNT            (1*TICRATE)
#define ST_RAMPAGEDELAY         (2*TICRATE)

#define ST_MUCHPAIN             20


// Location and size of statistics,
//  justified according to widget type.
// Problem is, within which space? STbar? Screen?
// Note: this could be read in by a lump.
//       Problem is, is the stuff rendered
//       into a buffer,
//       or into the frame buffer?

// AMMO number pos.
#define ST_AMMOWIDTH            3
#define ST_AMMOX                44
#define ST_AMMOY                (ST_Y+3)

// HEALTH number pos.
#define ST_HEALTHWIDTH          3
#define ST_HEALTHX              90
#define ST_HEALTHY              (ST_Y+3)

// Weapon pos.
#define ST_ARMSX                111
#define ST_ARMSY                (ST_Y+4)
#define ST_ARMSBGX              104
#define ST_ARMSBGY              (ST_Y)
#define ST_ARMSXSPACE           12
#define ST_ARMSYSPACE           10

// Frags pos.
#define ST_FRAGSX               138
#define ST_FRAGSY               (ST_Y+3)
#define ST_FRAGSWIDTH           2

// ARMOR number pos.
#define ST_ARMORWIDTH           3
#define ST_ARMORX               221
#define ST_ARMORY               (ST_Y+3)

// Key icon positions.
#define ST_KEY0WIDTH            8
#define ST_KEY0HEIGHT           5
#define ST_KEY0X                239
#define ST_KEY0Y                (ST_Y+3)
#define ST_KEY1WIDTH            ST_KEY0WIDTH
#define ST_KEY1X                239
#define ST_KEY1Y                (ST_Y+13)
#define ST_KEY2WIDTH            ST_KEY0WIDTH
#define ST_KEY2X                239
#define ST_KEY2Y                (ST_Y+23)

// Ammunition counter.
#define ST_AMMO0WIDTH           3
#define ST_AMMO0HEIGHT          6
#define ST_AMMO0X               288
#define ST_AMMO0Y               (ST_Y+5)
#define ST_AMMO1WIDTH           ST_AMMO0WIDTH
#define ST_AMMO1X               288
#define ST_AMMO1Y               (ST_Y+11)
#define ST_AMMO2WIDTH           ST_AMMO0WIDTH
#define ST_AMMO2X               288
#define ST_AMMO2Y               (ST_Y+23)
#define ST_AMMO3WIDTH           ST_AMMO0WIDTH
#define ST_AMMO3X               288
#define ST_AMMO3Y               (ST_Y+17)

// Indicate maximum ammunition.
// Only needed because backpack exists.
#define ST_MAXAMMO0WIDTH        3
#define ST_MAXAMMO0HEIGHT       5
#define ST_MAXAMMO0X            314
#define ST_MAXAMMO0Y            (ST_Y+5)
#define ST_MAXAMMO1WIDTH        ST_MAXAMMO0WIDTH
#define ST_MAXAMMO1X            314
#define ST_MAXAMMO1Y            (ST_Y+11)
#define ST_MAXAMMO2WIDTH        ST_MAXAMMO0WIDTH
#define ST_MAXAMMO2X            314
#define ST_MAXAMMO2Y            (ST_Y+23)
#define ST_MAXAMMO3WIDTH        ST_MAXAMMO0WIDTH
#define ST_MAXAMMO3X            314
#define ST_MAXAMMO3Y            (ST_Y+17)

//faB: unused stuff from the Doom alpha version ?
// pistol
//#define ST_WEAPON0X           110
//#define ST_WEAPON0Y           (ST_Y+4)
// shotgun
//#define ST_WEAPON1X           122
//#define ST_WEAPON1Y           (ST_Y+4)
// chain gun
//#define ST_WEAPON2X           134
//#define ST_WEAPON2Y           (ST_Y+4)
// missile launcher
//#define ST_WEAPON3X           110
//#define ST_WEAPON3Y           (ST_Y+13)
// plasma gun
//#define ST_WEAPON4X           122
//#define ST_WEAPON4Y           (ST_Y+13)
// bfg
//#define ST_WEAPON5X           134
//#define ST_WEAPON5Y           (ST_Y+13)

// WPNS title
//#define ST_WPNSX              109
//#define ST_WPNSY              (ST_Y+23)

 // DETH title
//#define ST_DETHX              109
//#define ST_DETHY              (ST_Y+23)

//Incoming messages window location
// #define ST_MSGTEXTX     (viewwindowx)
// #define ST_MSGTEXTY     (viewwindowy+viewheight-18)
//#define ST_MSGTEXTX             0
//#define ST_MSGTEXTY             0     //added:08-01-98:unused
// Dimensions given in characters.
#define ST_MSGWIDTH             52
// Or shall I say, in lines?
#define ST_MSGHEIGHT            1

#define ST_OUTTEXTX             0
#define ST_OUTTEXTY             6

// Width, in characters again.
#define ST_OUTWIDTH             52
 // Height, in lines.
#define ST_OUTHEIGHT            1

#define ST_MAPWIDTH     \
    (strlen(mapnames[(gameepisode-1)*9+(gamemap-1)]))

//added:24-01-98:unused ?
//#define ST_MAPTITLEX  (vid.width - ST_MAPWIDTH * ST_CHATFONTWIDTH)

#define ST_MAPTITLEY            0
#define ST_MAPHEIGHT            1


//added:02-02-98: set true if widgets coords need to be recalculated
boolean     st_recalc;

// main player in game
player_t*        plyr;

// ST_Start() has just been called
boolean          st_firsttime;

// used to execute ST_Init() only once
static int              veryfirsttime = 1;

// lump number for PLAYPAL
static int              lu_palette;

// used for timing
static unsigned int     st_clock;

// used for making messages go away
static int              st_msgcounter=0;

// used when in chat
static st_chatstateenum_t       st_chatstate;

// whether left-side main status bar is active
static boolean          st_statusbaron;

// whether status bar chat is active
static boolean          st_chat;

// value of st_chat before message popped up
static boolean          st_oldchat;

// whether chat window has the cursor on
static boolean          st_cursoron;

// !deathmatch
static boolean          st_notdeathmatch;

// !deathmatch && st_statusbaron
static boolean          st_armson;

// !deathmatch
static boolean          st_fragson;

// main bar left
static patch_t*         sbar;

// 0-9, tall numbers
static patch_t*         tallnum[10];

// tall % sign
static patch_t*         tallpercent;

// 0-9, short, yellow (,different!) numbers
static patch_t*         shortnum[10];

// 3 key-cards, 3 skulls
static patch_t*         keys[NUMCARDS];

// face status patches
static patch_t*         faces[ST_NUMFACES];

// face background
static patch_t*         faceback;

 // main bar right
static patch_t*         armsbg;

// weapon ownership patches
static patch_t*         arms[6][2];

// ready-weapon widget
static st_number_t      w_ready;

 // in deathmatch only, summary of frags stats
static st_number_t      w_frags;

// health widget
static st_percent_t     w_health;

// arms background
static st_binicon_t     w_armsbg;


// weapon ownership widgets
static st_multicon_t    w_arms[6];

// face status widget
static st_multicon_t    w_faces;

// keycard widgets
static st_multicon_t    w_keyboxes[3];

// armor widget
static st_percent_t     w_armor;

// ammo widgets
static st_number_t      w_ammo[4];

// max ammo widgets
static st_number_t      w_maxammo[4];



 // number of frags so far in deathmatch
static int      st_fragscount;

// used to use appopriately pained face
static int      st_oldhealth = -1;

// used for evil grin
static boolean  oldweaponsowned[NUMWEAPONS];

 // count until face changes
static int      st_facecount = 0;

// current face index, used by w_faces
static int      st_faceindex = 0;

// holds key-type for each key box on bar
static int      keyboxes[3];

// a random number per tick
static int      st_randomnumber;



// ------------------------------------------
//             status bar overlay
// ------------------------------------------

// icons for overlay
static   patch_t*   sbohealth;
static   patch_t*   sbofrags;
static   patch_t*   sboarmor;
//static   patch_t*   sboammo[NUMWEAPONS];
static   patch_t*   sboover; // Tails 03-11-2000
static   patch_t*   sboslife; // Tails 03-12-2000
static   patch_t*   sbotlife; // Tails 03-12-2000
static   patch_t*   sboklife; // Nozomi 02-23-2026
static   patch_t*   stsonic; // Tails 03-12-2000
static   patch_t*   sttails; // Tails 03-12-2000
static   patch_t*   stknux; // Tails 03-12-2000
static   patch_t*   stlivex; // Tails 03-12-2000
static   patch_t*   rrings; // Tails 03-14-2000
static   patch_t*   stuser; // Temporary user icon Tails 04-08-2000
static   patch_t*   sboulife; // Temporary user icon Tails 04-08-2000
static   patch_t*   sbotime; // Time logo Tails 06-12-2000
static   patch_t*   sbocolon; // Colon for time Tails 01-03-2001
static   patch_t*   ttlone; // Tails 11-01-2000
static   patch_t*   ttltwo; // Tails 11-01-2000
static   patch_t*   ttlthree; // Tails 11-01-2000
static   patch_t*   ttlfour; // Nozomi 03-02-2026
static   patch_t*   ttlfive; // Nozomi 03-02-2026
static   patch_t*   ttlsix; // Nozomi 03-02-2026
static   patch_t*   ttlseven; // Nozomi 03-02-2026
static   patch_t*   ttleight; // Nozomi 03-02-2026
static   patch_t*   ttlzone; // Tails 11-01-2000
static   patch_t*   lvlttl1; // Tails 11-01-2000
static   patch_t*   lvlttl2; // Tails 11-01-2000
static   patch_t*   lvlttl3; // Tails 11-01-2000
static   patch_t*   lvlttl4; // Tails 11-01-2000
static   patch_t*   lvlttl5; // Tails 11-01-2000
static   patch_t*   lvlttl6; // Tails 11-01-2000
static   patch_t*   lvlttl7; // Tails 11-01-2000
static   patch_t*   lvlttl8; // Tails 11-01-2000
static   patch_t*   lvlttl9; // Tails 11-01-2000
static   patch_t*   ttbanner; // Title stuff white banner with "robo blast" and "2" Tails 01-06-2001
static   patch_t*   ttwing; // Title stuff wing background Tails 01-06-2001
static   patch_t*   ttsonic; // Title stuff "SONIC" Tails 01-06-2001
static   patch_t*   ttswave1; // Title Sonics Tails 01-06-2001
static   patch_t*   ttswave2; // Title Sonics Tails 01-06-2001
static   patch_t*   ttswip1; // Title Sonics Tails 01-06-2001
static   patch_t*   ttsprep1; // Title Sonics Tails 01-06-2001
static   patch_t*   ttsprep2; // Title Sonics Tails 01-06-2001
static   patch_t*   ttspop1; // Title Sonics Tails 01-06-2001
static   patch_t*   ttspop2; // Title Sonics Tails 01-06-2001
static   patch_t*   ttspop3; // Title Sonics Tails 01-06-2001
static   patch_t*   ttspop4; // Title Sonics Tails 01-06-2001
static   patch_t*   ttspop5; // Title Sonics Tails 01-06-2001
static   patch_t*   ttspop6; // Title Sonics Tails 01-06-2001
static   patch_t*   ttspop7; // Title Sonics Tails 01-06-2001
static	 patch_t*	getall; // Special Stage HUD Tails 08-11-2001
static	 patch_t*	timeup; // Special Stage HUD Tails 08-11-2001
static   patch_t*   homing1; // Emerald hunt indicators Tails 12-20-2001
static   patch_t*   homing2; // Emerald hunt indicators Tails 12-20-2001
static   patch_t*   homing3; // Emerald hunt indicators Tails 12-20-2001
static   patch_t*   homing4; // Emerald hunt indicators Tails 12-20-2001
static   patch_t*   homing5; // Emerald hunt indicators Tails 12-20-2001
static   patch_t*   homing6; // Emerald hunt indicators Tails 12-20-2001

void S_StartSound(); // Tails 12-21-2001

//
// STATUS BAR CODE
//
void ST_Stop(void);


//added:05-02-98:quickie routine to fill the space around the statusbar when
//               the screen is larger than the stbar.
void ST_drawStatusBarBorders(void)
{
    byte*       src;
    byte*       dest;
    int         x;
    int         y;
    int         i;

    i = vid.height-ST_HEIGHT;   //match with the backtile around the viewwindow
    src  = scr_borderpatch;
    dest = screens[BG];
    for (y=0 ; y<ST_HEIGHT ; y++)
    {
        for (x=0 ; x<vid.width/64 ; x++)
        {
            memcpy (dest, src+((i&63)<<6), 64);
            dest += 64;
        }
        if (vid.width&63)
        {
            memcpy (dest, src+((i&63)<<6), vid.width&63);
            dest += (vid.width&63);
        }
        i++;
    }
}


void ST_refreshBackground(void)
{
    byte*       colormap;
    int         st_x;
    int         st_y;

    if (st_statusbaron)
    {
        if (rendermode==render_soft)
        {
            // stbar is centered in BG buffer
            st_x = ((vid.width-ST_WIDTH)>>1);
            st_y = 0;
            // fill the gaps around the statusbar when sbar is not scaled
            ST_drawStatusBarBorders ();
        }
        else {
            //stbar is scaled to the original aspect ratio, direct to screen
            st_x = 0;
            st_y = BASEVIDHEIGHT-ST_HEIGHT;
        }

        // software mode copies patch to BG buffer,
        // hardware modes directly draw the statusbar to the screen
        V_DrawPatch(st_x, st_y, BG, sbar);

        // draw the faceback for the statusbarplayer
            if (plyr->skincolor==0)
                colormap = colormaps;
            else
                colormap = translationtables - 256 + (plyr->skincolor<<8);

            V_DrawTranslationPatch (st_x+ST_FX, st_y, BG, faceback, colormap);

        // copy the statusbar buffer to the screen
        if ( rendermode==render_soft )
            V_CopyRect(0, 0, BG, vid.width, ST_HEIGHT, 0, ST_Y, FG);
    }
}


// Respond to keyboard input events,
//  intercept cheats.
boolean ST_Responder (event_t* ev)
{

  if (ev->type == ev_keyup)
  {
    // Filter automap on/off : activates the statusbar while automap is active
    if( (ev->data1 & 0xffff0000) == AM_MSGHEADER )
    {
        switch(ev->data1)
        {
          case AM_MSGENTERED:
            st_firsttime = true;        // force refresh of status bar
            break;

          case AM_MSGEXITED:
            break;
        }
    }

  }
  return false;
}



int ST_calcPainOffset(void)
{
    int         health;
    static int  lastcalc;
    static int  oldhealth = -1;

    health = plyr->health > 100 ? 100 : plyr->health;

    //SOM: Show one less ring than acutally have
    health --;

    if (health != oldhealth)
    {
        lastcalc = ST_FACESTRIDE * (((100 - health) * ST_NUMPAINFACES) / 101);
        oldhealth = health;
    }
    return lastcalc;
}


//
// This is a not-very-pretty routine which handles
//  the face states and their timing.
// the precedence of expressions is:
//  dead > evil grin > turned head > straight ahead
//
void ST_updateFaceWidget(void)
{
    int         i;
    angle_t     badguyangle;
    angle_t     diffang;
    static int  lastattackdown = -1;
    static int  priority = 0;
    boolean     doevilgrin;

    if (priority < 10)
    {
        // dead
        if (!plyr->health)
        {
            priority = 9;
            st_faceindex = ST_DEADFACE;
            st_facecount = 1;
        }
    }

    if (priority < 9)
    {
        if (plyr->bonuscount)
        {
            // picking up bonus
            doevilgrin = false;

            for (i=0;i<NUMWEAPONS;i++)
            {
                if (oldweaponsowned[i] != plyr->weaponowned[i])
                {
                    doevilgrin = true;
                    oldweaponsowned[i] = plyr->weaponowned[i];
                }
            }
            if (doevilgrin)
            {
                // evil grin if just picked up weapon
                priority = 8;
                st_facecount = ST_EVILGRINCOUNT;
                st_faceindex = ST_calcPainOffset() + ST_EVILGRINOFFSET;
            }
        }

    }

    if (priority < 8)
    {
        if (plyr->damagecount
            && plyr->attacker
            && plyr->attacker != plyr->mo)
        {
            // being attacked
            priority = 7;

            if (plyr->health - st_oldhealth > ST_MUCHPAIN)
            {
                st_facecount = ST_TURNCOUNT;
                st_faceindex = ST_calcPainOffset() + ST_OUCHOFFSET;
            }
            else
            {
                badguyangle = R_PointToAngle2(plyr->mo->x,
                                              plyr->mo->y,
                                              plyr->attacker->x,
                                              plyr->attacker->y);

                if (badguyangle > plyr->mo->angle)
                {
                    // whether right or left
                    diffang = badguyangle - plyr->mo->angle;
                    i = diffang > ANG180;
                }
                else
                {
                    // whether left or right
                    diffang = plyr->mo->angle - badguyangle;
                    i = diffang <= ANG180;
                } // confusing, aint it?


                st_facecount = ST_TURNCOUNT;
                st_faceindex = ST_calcPainOffset();

                if (diffang < ANG45)
                {
                    // head-on
                    st_faceindex += ST_RAMPAGEOFFSET;
                }
                else if (i)
                {
                    // turn face right
                    st_faceindex += ST_TURNOFFSET;
                }
                else
                {
                    // turn face left
                    st_faceindex += ST_TURNOFFSET+1;
                }
            }
        }
    }

    if (priority < 7)
    {
        // getting hurt because of your own damn stupidity
        if (plyr->damagecount)
        {
            if (plyr->health - st_oldhealth > ST_MUCHPAIN)
            {
                priority = 7;
                st_facecount = ST_TURNCOUNT;
                st_faceindex = ST_calcPainOffset() + ST_OUCHOFFSET;
            }
            else
            {
                priority = 6;
                st_facecount = ST_TURNCOUNT;
                st_faceindex = ST_calcPainOffset() + ST_RAMPAGEOFFSET;
            }

        }

    }

    if (priority < 6)
    {
        // rapid firing
        if (plyr->attackdown)
        {
            if (lastattackdown==-1)
                lastattackdown = ST_RAMPAGEDELAY;
            else if (!--lastattackdown)
            {
                priority = 5;
                st_faceindex = ST_calcPainOffset() + ST_RAMPAGEOFFSET;
                st_facecount = 1;
                lastattackdown = 1;
            }
        }
        else
            lastattackdown = -1;

    }

    if (priority < 5)
    {
        // invulnerability
        if ((plyr->cheats & CF_GODMODE)
            || plyr->powers[pw_invulnerability])
        {
            priority = 4;

            st_faceindex = ST_GODFACE;
            st_facecount = 1;

        }

    }

    // look left or look right if the facecount has timed out
    if (!st_facecount)
    {
        st_faceindex = ST_calcPainOffset() + (st_randomnumber % 3);
        st_facecount = ST_STRAIGHTFACECOUNT;
        priority = 0;
    }

    st_facecount--;

}

boolean ST_SameTeam(player_t *a,player_t *b)
{
    switch (cv_teamplay.value) {
       case 0 : return false;
       case 1 : return (a->skincolor == b->skincolor);
       case 2 : return (a->skin == b->skin);
    }
    return false;
}

// count the frags of the playernum player
//Fab: made as a tiny routine so ST_overlayDrawer() can use it
//Boris: rename ST_countFrags in to ST_PlayerFrags for use anytime
//       when we need the frags
int ST_PlayerFrags (int playernum)
{
    int    i,frags;

    frags = players[playernum].addfrags;
    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if ((cv_teamplay.value==0 && i != playernum)
         || (cv_teamplay.value && !ST_SameTeam(&players[i],&players[playernum])) )
            frags += players[playernum].frags[i];
        else
            frags -= players[playernum].frags[i];
    }

    return frags;
}


void ST_updateWidgets(void)
{
    static int  largeammo = 1994; // means "n/a"
    int         i;

#ifdef PARANOIA
    if(!plyr) I_Error("plyr==NULL\n");
#endif
    // must redirect the pointer if the ready weapon has changed.
    //  if (w_ready.data != plyr->readyweapon)
    //  {
    if (weaponinfo[plyr->readyweapon].ammo == am_noammo)
        w_ready.num = &largeammo;
    else
        w_ready.num = &plyr->ammo[weaponinfo[plyr->readyweapon].ammo];
    //{
    // static int tic=0;
    // static int dir=-1;
    // if (!(tic&15))
    //   plyr->ammo[weaponinfo[plyr->readyweapon].ammo]+=dir;
    // if (plyr->ammo[weaponinfo[plyr->readyweapon].ammo] == -100)
    //   dir = 1;
    // tic++;
    // }
    w_ready.data = plyr->readyweapon;

    // if (*w_ready.on)
    //  STlib_updateNum(&w_ready, true);
    // refresh weapon change
    //  }

    // update keycard multiple widgets
    for (i=0;i<3;i++)
    {
        keyboxes[i] = (plyr->cards & (1<<i)) ? i : -1;

        if (plyr->cards & (1<<(i+3)) )
            keyboxes[i] = i+3;
    }

    // refresh everything if this is him coming back to life
    ST_updateFaceWidget();

    // used by the w_armsbg widget
    st_notdeathmatch = !cv_deathmatch.value;

    // used by w_arms[] widgets
    st_armson = st_statusbaron && !cv_deathmatch.value;

    // used by w_frags widget
    st_fragson = cv_deathmatch.value && st_statusbaron;

    st_fragscount = ST_PlayerFrags(statusbarplayer);

    // get rid of chat window if up because of message
    if (!--st_msgcounter)
        st_chat = st_oldchat;

}

void ST_Ticker (void)
{

    st_clock++;
    st_randomnumber = M_Random();
    ST_updateWidgets();
    st_oldhealth = plyr->health;

}

static int st_palette = 0;


void ST_doPaletteStuff(void)
{

    int         palette;
    byte*       pal;
//    int         cnt;
//    int         bzc;
// NO PALETTE CHANGES! Tails
//    cnt = plyr->damagecount;

/*    if (plyr->powers[pw_strength])
    {
        // slowly fade the berzerk out
        bzc = 12 - (plyr->powers[pw_strength]>>6);

        if (bzc > cnt)
            cnt = bzc;
    }

    if (cnt)
    {
        palette = (cnt+7)>>3;

        if (palette >= NUMREDPALS)
            palette = NUMREDPALS-1;

        palette += STARTREDPALS;
    }
    else*/
    if (plyr->bonuscount)
    {
        palette = (plyr->bonuscount+7)>>3;

        if (palette >= NUMBONUSPALS)
            palette = NUMBONUSPALS-1;

        palette += STARTBONUSPALS;
    }
//    else
//    if ( plyr->powers[pw_ironfeet] > 4*32
//      || plyr->powers[pw_ironfeet]&8)
//        palette = RADIATIONPAL;

    //added:28-02-98:quick hack underwater palette
    /*if (plyr->mo &&
        (plyr->mo->z + (cv_viewheight.value<<FRACBITS) < plyr->mo->waterz) )
        palette = RADIATIONPAL;*/
 /*     else if (camera.chase)
	  {
	if(plyr->camwater)
	{
//		  if(camera.mo->z + camera.mo->height/2 < plyr->mo->waterz)
       palette = RADIATIONPAL;
	  }
	  }*/
	else if(plyr->camunder)
		palette = RADIATIONPAL;
// Fixing dumb bug from previous underwater palette - Tails 10-31-99
    else
         palette = 0;

    if (palette != st_palette)
    {
        st_palette = palette;

        #ifdef HWRENDER // not win32 only 19990829 by Kin
        if (rendermode == render_opengl)
        
        //faB - NOW DO ONLY IN SOFTWARE MODE, LETS FIX THIS FOR GOOD OR NEVER
        //      idea : use a true color gradient from frame to frame, because we
        //             are in true color in HW3D, we can have smoother palette change
        //             than the palettes defined in the wad

        {
            //CONS_Printf("palette: %d\n", palette);
            switch (palette) {
                case 0x00: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0x0); break;  // pas de changement
                case 0x01: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff373797); break; // red
                case 0x02: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff373797); break; // red
                case 0x03: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff3030a7); break; // red
                case 0x04: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff2727b7); break; // red
                case 0x05: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff2020c7); break; // red
                case 0x06: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff1717d7); break; // red
                case 0x07: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff1010e7); break; // red
                case 0x08: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff0707f7); break; // red
                case 0x09: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xffff6060); break; // blue

                //case 0x0a: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff579787); break; // light green
                //case 0x0b: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff47a077); break; // light green
                //case 0x0c: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff37b767); break; // light green

                case 0x0a: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff70a090); break; // light green
                case 0x0b: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff67b097); break; // light green
                case 0x0c: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff60c0a0); break; // light green

                //case 0x0a: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff829371); break; // light green
                //case 0x0b: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff809070); break; // light green
                //case 0x0c: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff788769); break; // light green

                case 0x0d: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff60ff60); break; // green
                case 0x0e: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xffff6060); break; // blue
                case 0x0f: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xffff6060); break; // blue
            }
        }
        else
        #endif
        {
			if(palette == RADIATIONPAL)
				lu_palette = W_GetNumForName ("WATERPAL");
			else if (palette >= STARTBONUSPALS && palette <= STARTBONUSPALS+NUMBONUSPALS)
				lu_palette = W_GetNumForName ("FLASHPAL");
			else
				lu_palette = W_GetNumForName ("PLAYPAL");

            pal = (byte *) W_CacheLumpNum (lu_palette, PU_CACHE)+palette*768;
            V_SetPalette (pal);
        }
    }
}

void ST_drawWidgets(boolean refresh)
{
    int         i;

    // used by w_arms[] widgets
    st_armson = st_statusbaron && !cv_deathmatch.value;

    // used by w_frags widget
    st_fragson = cv_deathmatch.value && st_statusbaron;

    STlib_updateNum(&w_ready, refresh);

    for (i=0;i<4;i++)
    {
        STlib_updateNum(&w_ammo[i], refresh);
        STlib_updateNum(&w_maxammo[i], refresh);
    }

    STlib_updatePercent(&w_health, refresh);
    STlib_updatePercent(&w_armor, refresh);

    STlib_updateBinIcon(&w_armsbg, refresh);

    for (i=0;i<6;i++)
        STlib_updateMultIcon(&w_arms[i], refresh);

    STlib_updateMultIcon(&w_faces, refresh);

    for (i=0;i<3;i++)
        STlib_updateMultIcon(&w_keyboxes[i], refresh);

    STlib_updateNum(&w_frags, refresh);

}

void ST_doRefresh(void)
{

    // draw status bar background to off-screen buff
    ST_refreshBackground();

    // and refresh all widgets
    ST_drawWidgets(true);
}

void ST_diffDraw(void)
{
    // update all widgets
    ST_drawWidgets(false);
}


void ST_Drawer (boolean fullscreen, boolean refresh)
{
    st_statusbaron = (!fullscreen) || automapactive;

    //added:30-01-98:force a set of the palette by doPaletteStuff()
    if (vid.recalc)
        st_palette = -1;

    // Do red-/gold-shifts from damage/items
#ifdef HWRENDER // not win32 only 19990829 by Kin
//25/08/99: Hurdler: palette changes is done for all players,
//                   not only player1 ! That's why this part 
//                   of code is moved somewhere else.
    if (rendermode==render_soft)
#endif
        ST_doPaletteStuff();

#ifdef HWRENDER // not win32 only 19990829 by Kin
    if ( rendermode != render_soft )
        HWR_ScalePatch ( TRUE );
#endif
    
    if (!cv_splitscreen.value)
    {
        // after ST_Start(), screen refresh needed, or vid mode change
        if (st_firsttime || refresh || st_recalc ||
            rendermode!=render_soft )    //faB: always refresh for now, other priorities for now
        {
            if (st_recalc)  //recalc widget coords after vid mode change
            {
                ST_createWidgets ();
                st_recalc = false;
            }
            st_firsttime = false;
            ST_doRefresh();
        }
        else
            // Otherwise, update as little as possible
            ST_diffDraw();
    }
}


static void ST_loadGraphics(void)
{

    int         i;
    char        namebuf[9];

    // Load the numbers, tall and short
    for (i=0;i<10;i++)
    {
        sprintf(namebuf, "STTNUM%d", i);
        tallnum[i] = (patch_t *) W_CachePatchName(namebuf, PU_STATIC);

        sprintf(namebuf, "STTNUM%d", i);
        shortnum[i] = (patch_t *) W_CachePatchName(namebuf, PU_STATIC);
    }

    // Load percent key.
    //Note: why not load STMINUS here, too?
    tallpercent = (patch_t *) W_CachePatchName("STTPRCNT", PU_STATIC);

    // key cards
    for (i=0;i<NUMCARDS;i++)
    {
        sprintf(namebuf, "STKEYS%d", i);
        keys[i] = (patch_t *) W_CachePatchName(namebuf, PU_STATIC);
    }

    // arms background
    armsbg = (patch_t *) W_CachePatchName("STARMS", PU_STATIC);

    // arms ownership widgets
    for (i=0;i<6;i++)
    {
        sprintf(namebuf, "STGNUM%d", i+2);

        // gray #
        arms[i][0] = (patch_t *) W_CachePatchName(namebuf, PU_STATIC);

        // yellow #
        arms[i][1] = shortnum[i+2];
    }

    // status bar background bits
    sbar = (patch_t *) W_CachePatchName("STBAR", PU_STATIC);

    // the original Doom uses 'STF' as base name for all face graphics
    ST_loadFaceGraphics ("STF");
}


// made separate so that skins code can reload custom face graphics
void ST_loadFaceGraphics (char *facestr)
{
    int   i,j;
    int   facenum;
    char  namelump[9];
    char* namebuf;

    //hack: make sure base face name is no more than 3 chars
    // bug: core dump fixed 19990220 by Kin
    if(strlen(facestr)>3)
    facestr[3]='\0';
    strcpy (namelump, facestr);  // copy base name
    namebuf = namelump;
    while (*namebuf>' ') namebuf++;

    // face states
    facenum = 0;
    for (i=0;i<ST_NUMPAINFACES;i++)
    {
        for (j=0;j<ST_NUMSTRAIGHTFACES;j++)
        {
            sprintf(namebuf, "ST%d%d", i, j);
            faces[facenum++] = W_CachePatchName(namelump, PU_STATIC);
        }
        sprintf(namebuf, "TR%d0", i);        // turn right
        faces[facenum++] = W_CachePatchName(namelump, PU_STATIC);
        sprintf(namebuf, "TL%d0", i);        // turn left
        faces[facenum++] = W_CachePatchName(namelump, PU_STATIC);
        sprintf(namebuf, "OUCH%d", i);       // ouch!
        faces[facenum++] = W_CachePatchName(namelump, PU_STATIC);
        sprintf(namebuf, "EVL%d", i);        // evil grin ;)
        faces[facenum++] = W_CachePatchName(namelump, PU_STATIC);
        sprintf(namebuf, "KILL%d", i);       // pissed off
        faces[facenum++] = W_CachePatchName(namelump, PU_STATIC);
    }
    strcpy (namebuf, "GOD0");
    faces[facenum++] = W_CachePatchName(namelump, PU_STATIC);
    strcpy (namebuf, "DEAD0");
    faces[facenum++] = W_CachePatchName(namelump, PU_STATIC);

    // face backgrounds for different player colors
    //added:08-02-98: uses only STFB0, which is remapped to the right
    //                colors using the player translation tables, so if
    //                you add new player colors, it is automatically
    //                used for the statusbar.
    strcpy (namebuf, "B0");
    faceback = (patch_t *) W_CachePatchName(namelump, PU_STATIC);
}


static void ST_loadData(void)
{
    lu_palette = W_GetNumForName ("PLAYPAL");
    ST_loadGraphics();
}

void ST_unloadGraphics(void)
{

    int i;

    //faB: GlidePatch_t are always purgeable
    if (rendermode==render_soft)
    {
    // unload the numbers, tall and short
    for (i=0;i<10;i++)
    {
        Z_ChangeTag(tallnum[i], PU_CACHE);
        Z_ChangeTag(shortnum[i], PU_CACHE);
    }
    // unload tall percent
    Z_ChangeTag(tallpercent, PU_CACHE);

    // unload arms background
    Z_ChangeTag(armsbg, PU_CACHE);

    // unload gray #'s
    for (i=0;i<6;i++)
        Z_ChangeTag(arms[i][0], PU_CACHE);

    // unload the key cards
    for (i=0;i<NUMCARDS;i++)
        Z_ChangeTag(keys[i], PU_CACHE);

    Z_ChangeTag(sbar, PU_CACHE);
    }

    ST_unloadFaceGraphics ();

    // Note: nobody ain't seen no unloading
    //   of stminus yet. Dude.

}

// made separate so that skins code can reload custom face graphics
void ST_unloadFaceGraphics (void)
{
    int    i;

    //faB: GlidePatch_t are always purgeable
    if (rendermode==render_soft)
    {
    for (i=0;i<ST_NUMFACES;i++)
        Z_ChangeTag(faces[i], PU_CACHE);

    // face background
    Z_ChangeTag(faceback, PU_CACHE);
    }
}


void ST_unloadData(void)
{
    ST_unloadGraphics();
}

void ST_initData(void)
{

    int         i;

    st_firsttime = true;

    //added:16-01-98:'link' the statusbar display to a player, which could be
    //               another player than consoleplayer, for example, when you
    //               change the view in a multiplayer demo with F12.
    if (singledemo || cv_gametype.value == 2) // Tails 04-25-2001
        statusbarplayer = displayplayer;
    else
        statusbarplayer = consoleplayer;

    plyr = &players[statusbarplayer];

    st_clock = 0;
    st_chatstate = StartChatState;

    st_statusbaron = true;
    st_oldchat = st_chat = false;
    st_cursoron = false;

    st_faceindex = 0;
    st_palette = -1;

    st_oldhealth = -1;

    for (i=0;i<NUMWEAPONS;i++)
        oldweaponsowned[i] = plyr->weaponowned[i];

    for (i=0;i<3;i++)
        keyboxes[i] = -1;

    STlib_init();

}


//added:30-01-98: NOTE: this is called at any level start, view change,
//                      and after vid mode change.
void ST_createWidgets(void)
{
    int i;
    int st_x;
    
    if (rendermode == render_soft)
        st_x = ((vid.width-ST_WIDTH)>>1);
    else
        st_x = 0;   //currently we scale the statusbar

    // ready weapon ammo
    STlib_initNum(&w_ready,
                  st_x + ST_AMMOX,
                  ST_AMMOY,
                  tallnum,
                  &plyr->ammo[weaponinfo[plyr->readyweapon].ammo],
                  &st_statusbaron,
                  ST_AMMOWIDTH );

    // the last weapon type
    w_ready.data = plyr->readyweapon;

    // health percentage
    STlib_initPercent(&w_health,
                      st_x + ST_HEALTHX,
                      ST_HEALTHY,
                      tallnum,
                      &plyr->health,
                      &st_statusbaron,
                      tallpercent);

    // arms background
    STlib_initBinIcon(&w_armsbg,
                      st_x + ST_ARMSBGX,
                      ST_ARMSBGY,
                      armsbg,
                      &st_notdeathmatch,
                      &st_statusbaron);

    // weapons owned
    for(i=0;i<6;i++)
    {
        STlib_initMultIcon(&w_arms[i],
                           st_x + ST_ARMSX+(i%3)*ST_ARMSXSPACE,
                           ST_ARMSY+(i/3)*ST_ARMSYSPACE,
                           arms[i], (int *) &plyr->weaponowned[i+1],
                           &st_armson);
    }

    // frags sum
    STlib_initNum(&w_frags,
                  st_x + ST_FRAGSX,
                  ST_FRAGSY,
                  tallnum,
                  &st_fragscount,
                  &st_fragson,
                  ST_FRAGSWIDTH);

    // faces
    STlib_initMultIcon(&w_faces,
                       st_x + ST_FACESX,
                       ST_FACESY,
                       faces,
                       &st_faceindex,
                       &st_statusbaron);

    // armor percentage - should be colored later
    STlib_initPercent(&w_armor,
                      st_x + ST_ARMORX,
                      ST_ARMORY,
                      tallnum,
                      &plyr->armorpoints,
                      &st_statusbaron, tallpercent);

    // keyboxes 0-2
    STlib_initMultIcon(&w_keyboxes[0],
                       st_x + ST_KEY0X,
                       ST_KEY0Y,
                       keys,
                       &keyboxes[0],
                       &st_statusbaron);

    STlib_initMultIcon(&w_keyboxes[1],
                       st_x + ST_KEY1X,
                       ST_KEY1Y,
                       keys,
                       &keyboxes[1],
                       &st_statusbaron);

    STlib_initMultIcon(&w_keyboxes[2],
                       st_x + ST_KEY2X,
                       ST_KEY2Y,
                       keys,
                       &keyboxes[2],
                       &st_statusbaron);

    // ammo count (all four kinds)
    STlib_initNum(&w_ammo[0],
                  st_x + ST_AMMO0X,
                  ST_AMMO0Y,
                  shortnum,
                  &plyr->ammo[0],
                  &st_statusbaron,
                  ST_AMMO0WIDTH);

    STlib_initNum(&w_ammo[1],
                  st_x + ST_AMMO1X,
                  ST_AMMO1Y,
                  shortnum,
                  &plyr->ammo[1],
                  &st_statusbaron,
                  ST_AMMO1WIDTH);

    STlib_initNum(&w_ammo[2],
                  st_x + ST_AMMO2X,
                  ST_AMMO2Y,
                  shortnum,
                  &plyr->ammo[2],
                  &st_statusbaron,
                  ST_AMMO2WIDTH);

    STlib_initNum(&w_ammo[3],
                  st_x + ST_AMMO3X,
                  ST_AMMO3Y,
                  shortnum,
                  &plyr->ammo[3],
                  &st_statusbaron,
                  ST_AMMO3WIDTH);

    // max ammo count (all four kinds)
    STlib_initNum(&w_maxammo[0],
                  st_x + ST_MAXAMMO0X,
                  ST_MAXAMMO0Y,
                  shortnum,
                  &plyr->maxammo[0],
                  &st_statusbaron,
                  ST_MAXAMMO0WIDTH);

    STlib_initNum(&w_maxammo[1],
                  st_x + ST_MAXAMMO1X,
                  ST_MAXAMMO1Y,
                  shortnum,
                  &plyr->maxammo[1],
                  &st_statusbaron,
                  ST_MAXAMMO1WIDTH);

    STlib_initNum(&w_maxammo[2],
                  st_x + ST_MAXAMMO2X,
                  ST_MAXAMMO2Y,
                  shortnum,
                  &plyr->maxammo[2],
                  &st_statusbaron,
                  ST_MAXAMMO2WIDTH);

    STlib_initNum(&w_maxammo[3],
                  st_x + ST_MAXAMMO3X,
                  ST_MAXAMMO3Y,
                  shortnum,
                  &plyr->maxammo[3],
                  &st_statusbaron,
                  ST_MAXAMMO3WIDTH);

}


static boolean  st_stopped = true;


void ST_Start (void)
{
    if (!st_stopped)
        ST_Stop();

    ST_initData();
    ST_createWidgets();
    st_stopped = false;
    st_recalc = false;  //added:02-02-98: widgets coords have been setup
                        // see ST_drawer()
}

void ST_Stop (void)
{
    if (st_stopped)
        return;

    V_SetPalette (W_CacheLumpNum (lu_palette, PU_CACHE));

    st_stopped = true;
}

//
//  Initializes the status bar,
//  sets the defaults border patch for the window borders.
//

//faB: used by Glide mode, holds lumpnum of flat used to fill space around the viewwindow
int  st_borderpatchnum;

void ST_Init (void)
{
//    int     i;

    veryfirsttime = 0;
    ST_loadData();

    //added:26-01-98:screens[4] is allocated at videomode setup, and
    //               set at V_Init(), the first time being at SCR_Recalc()

    // choose and cache the default border patch
    if (gamemode == commercial)
        // DOOM II border patch, original was GRNROCK
        st_borderpatchnum = W_GetNumForName ("FLOOR0_3"); // Tails 10-26-99
    else
        // DOOM border patch.
        st_borderpatchnum = W_GetNumForName ("FLOOR0_3"); // Tails 10-26-99

    scr_borderpatch = W_CacheLumpNum (st_borderpatchnum, PU_STATIC);

    //
    // cache the status bar overlay icons  (fullscreen mode)
    //
    sbohealth = W_CachePatchName ("SBOHEALT", PU_STATIC); // Tails
    sbofrags  = W_CachePatchName ("SBOFRAGS", PU_STATIC); // Tails
    sboarmor  = W_CachePatchName ("SBOARMOR", PU_STATIC); // Tails
    sboover   = W_CachePatchName ("SBOOVER", PU_STATIC); // Tails 03-11-2000
    sboslife  = W_CachePatchName ("SBOSLIFE", PU_STATIC); // Tails 03-12-2000
    sbotlife  = W_CachePatchName ("SBOTLIFE", PU_STATIC); // Tails 03-12-2000
	sboklife  = W_CachePatchName ("SBOKLIFE", PU_STATIC);
    stsonic   = W_CachePatchName ("STSONIC", PU_STATIC); // Tails 03-12-2000
    sttails   = W_CachePatchName ("STTAILS", PU_STATIC); // Tails 03-12-2000
    stknux    = W_CachePatchName ("STKNUX", PU_STATIC); // Tails 03-12-2000
    stlivex   = W_CachePatchName ("STLIVEX", PU_STATIC); // Tails 03-12-2000
    rrings    = W_CachePatchName ("SBORINGS", PU_STATIC); // Tails 03-14-2000
//    colon     = W_CachePatchName ("WICOLON", PU_STATIC); // Tails 03-14-2000
    stuser    = W_CachePatchName ("STUSER", PU_STATIC); // Temorary User icon Tails 04-08-2000
    sboulife  = W_CachePatchName ("SBOULIFE", PU_STATIC); // Temorary User icon Tails 04-08-2000
    sbotime     = W_CachePatchName ("SBOTIME", PU_STATIC); // Time logo Tails 06-12-2000
	sbocolon = W_CachePatchName ("SBOCOLON", PU_STATIC); // Colon for time Tails 01-03-2001
	ttlone     = W_CachePatchName ("TTLONE", PU_STATIC); // Tails 11-01-2000
	ttltwo     = W_CachePatchName ("TTLTWO", PU_STATIC); // Tails 11-01-2000
	ttlthree     = W_CachePatchName ("TTLTHREE", PU_STATIC); // Tails 11-01-2000
	ttlfour     = W_CachePatchName ("TTLFOUR", PU_STATIC); // Nozomi 03-02-2026
	ttlfive     = W_CachePatchName ("TTLFIVE", PU_STATIC); // Nozomi 03-02-2026
	ttlsix     = W_CachePatchName ("TTLSIX", PU_STATIC); // Nozomi 03-02-2026
	ttlseven     = W_CachePatchName ("TTLSEVEN", PU_STATIC); // Nozomi 03-02-2026
	ttleight     = W_CachePatchName ("TTLEIGHT", PU_STATIC); // Nozomi 03-02-2026
	ttlzone     = W_CachePatchName ("TTLZONE", PU_STATIC); // Tails 11-01-2000
	lvlttl1     = W_CachePatchName ("LVLTTL1", PU_STATIC); // Tails 11-01-2000
	lvlttl2     = W_CachePatchName ("LVLTTL2", PU_STATIC); // Tails 11-01-2000
	lvlttl3     = W_CachePatchName ("LVLTTL3", PU_STATIC); // Tails 11-01-2000
	lvlttl4     = W_CachePatchName ("LVLTTL4", PU_STATIC); // Tails 11-01-2000
	lvlttl5     = W_CachePatchName ("LVLTTL5", PU_STATIC); // Tails 11-01-2000
	lvlttl6     = W_CachePatchName ("LVLTTL6", PU_STATIC); // Tails 11-01-2000
	lvlttl7     = W_CachePatchName ("LVLTTL7", PU_STATIC); // Tails 11-01-2000
	lvlttl8     = W_CachePatchName ("LVLTTL8", PU_STATIC); // Tails 11-01-2000
	lvlttl9     = W_CachePatchName ("LVLTTL9", PU_STATIC); // Tails 11-01-2000
	ttbanner    = W_CachePatchName ("TTBANNER", PU_STATIC); // Tails 01-06-2001
	ttwing    = W_CachePatchName ("TTWING", PU_STATIC); // Tails 01-06-2001
	ttsonic    = W_CachePatchName ("TTSONIC", PU_STATIC); // Tails 01-06-2001
	ttswave1    = W_CachePatchName ("TTSWAVE1", PU_STATIC); // Tails 01-06-2001
	ttswave2    = W_CachePatchName ("TTSWAVE2", PU_STATIC); // Tails 01-06-2001
	ttswip1    = W_CachePatchName ("TTSWIP1", PU_STATIC); // Tails 01-06-2001
	ttsprep1    = W_CachePatchName ("TTSPREP1", PU_STATIC); // Tails 01-06-2001
	ttsprep2    = W_CachePatchName ("TTSPREP2", PU_STATIC); // Tails 01-06-2001
	ttspop1    = W_CachePatchName ("TTSPOP1", PU_STATIC); // Tails 01-06-2001
	ttspop2    = W_CachePatchName ("TTSPOP2", PU_STATIC); // Tails 01-06-2001
	ttspop3    = W_CachePatchName ("TTSPOP3", PU_STATIC); // Tails 01-06-2001
	ttspop4    = W_CachePatchName ("TTSPOP4", PU_STATIC); // Tails 01-06-2001
	ttspop5    = W_CachePatchName ("TTSPOP5", PU_STATIC); // Tails 01-06-2001
	ttspop6    = W_CachePatchName ("TTSPOP6", PU_STATIC); // Tails 01-06-2001
	ttspop7    = W_CachePatchName ("TTSPOP7", PU_STATIC); // Tails 01-06-2001
	getall     = W_CachePatchName ("GETALL", PU_STATIC); // Special Stage HUD Tails 08-11-2001
	timeup	   = W_CachePatchName ("TIMEUP", PU_STATIC); // Special Stage HUD Tails 08-11-2001
	homing1	   = W_CachePatchName ("HOMING1", PU_STATIC); // Emerald hunt indicators Tails 12-20-2001
	homing2	   = W_CachePatchName ("HOMING2", PU_STATIC); // Emerald hunt indicators Tails 12-20-2001
	homing3	   = W_CachePatchName ("HOMING3", PU_STATIC); // Emerald hunt indicators Tails 12-20-2001
	homing4	   = W_CachePatchName ("HOMING4", PU_STATIC); // Emerald hunt indicators Tails 12-20-2001
	homing5	   = W_CachePatchName ("HOMING5", PU_STATIC); // Emerald hunt indicators Tails 12-20-2001
	homing6	   = W_CachePatchName ("HOMING6", PU_STATIC); // Emerald hunt indicators Tails 12-20-2001

/*    for (i=0;i<NUMWEAPONS;i++)
    {
        if (i>0 && i!=7)
            sboammo[i] = W_GetNumForName (va("SBOAMMO%c",'0'+i));
        else
            sboammo[i] = 0;
    }*/
}

 //added:16-01-98: change the status bar too, when pressing F12 while viewing
//                 a demo.
void ST_changeDemoView (void)
{
    //the same routine is called at multiplayer deathmatch spawn
    // so it can be called multiple times
    ST_Start();
}


// =========================================================================
//                         STATUS BAR OVERLAY
// =========================================================================

consvar_t cv_hidehud = {"hidehud", "Off", 0, CV_OnOff, NULL};
consvar_t cv_stbaroverlay = {"overlay","kahmf",CV_SAVE,NULL};

boolean   st_overlay;


void ST_AddCommands (void)
{
	CV_RegisterVar (&cv_hidehud);
    CV_RegisterVar (&cv_stbaroverlay);
}


//  Draw a number, scaled, over the view
//  Always draw the number completely since it's overlay
//
void ST_drawOverlayNum (int       x,            // right border!
                        int       y,
                        int       num,
                        patch_t** numpat,
                        patch_t*  percent )
{
    int       w = SHORT(numpat[0]->width);
    boolean   neg;

    // in the special case of 0, you draw 0
    if (!num)
    {
        V_DrawScaledPatch(x - (w*vid.dupx), y, FG|V_NOSCALESTART, numpat[ 0 ]);
        return;
    }

    neg = num < 0;

    if (neg)
        num = -num;

    // draw the number
    while (num)
    {
        x -= (w * vid.dupx);
        V_DrawScaledPatch(x, y, FG|V_NOSCALESTART, numpat[ num % 10 ]);
        num /= 10;
    }

    // draw a minus sign if necessary
    if (neg)
        V_DrawScaledPatch(x - (8*vid.dupx), y, FG|V_NOSCALESTART, sttminus);
}


static inline int SCY( int y )
{ 
    //31/10/99: fixed by Hurdler so it _works_ also in hardware mode
    // do not scale to resolution for hardware accelerated
    // because these modes always scale by default
    if ( rendermode == render_soft ) {
        y = ( y * vid.height ) / BASEVIDHEIGHT;     // scale to resolution
        if ( cv_splitscreen.value ) {
            y >>= 1;
            if (plyr != &players[statusbarplayer])
                y += vid.height / 2;
        }
    } else {// hardware mode
        if ( cv_splitscreen.value ) {
            y >>= 1;
            if (plyr != &players[statusbarplayer])
                y += BASEVIDHEIGHT / 2; // hardware mode is scaled!!!
        }
    }
    return y;
}


static inline int SCX( int x )
{
    // hardware accelerated modes scale automatically
    if ( rendermode == render_soft )
        return ( ( x * vid.width ) / BASEVIDWIDTH );
    else
        return x;
}


//  Draw the status bar overlay, customisable : the user choose which
//  kind of information to overlay
//
void ST_overlayDrawer (int playernum)
{
    char*  cmds;
    char   c;
    int    i;

	if (cv_hidehud.value)
		return;

    cmds = cv_stbaroverlay.string;

    while ((c=*cmds++))
    {
       if (c>='A' && c<='Z')
           c = c + 'a' - 'A';
       switch (c)
       {
         case 'h': // draw health
			// start GAME OVER pic Tails 03-11-2000
			if(plyr->lives <= 0)
			{
				V_DrawScaledPatch (SCX(32),SCY(100)-(sboover->height*vid.dupy), FG | V_NOSCALESTART,sboover); // Tails 03-11-2000
			}
			// end GAME OVER pic Tails 03-11-2000

			// start lives status Tails 03-12-2000
			if ((!cv_gametype.value || cv_gametype.value == 2) && !nozo_timeattack)
			{
				patch_t* face_patch;
				patch_t* name_patch;
				byte* colormap;

				if (plyr->skincolor==0)
					colormap = colormaps;
				else
					colormap = translationtables - 256 + (plyr->skincolor<<8);

				if (plyr->skin == 0)
				{
				   name_patch = stsonic;
				   face_patch = sboslife;
				}
				else if (plyr->skin == 1)
				{
				   name_patch = sttails;
				   face_patch = sbotlife;
				}
				else if (plyr->skin == 2)
				{
				   name_patch = stknux;
				   face_patch = sboklife;
				}
				else
				{
					if (strlen(skins[plyr->skin].face) > 0) 
						face_patch = W_CachePatchName(skins[plyr->skin].face, PU_STATIC);
					else
						face_patch = sboulife;

					if (strlen(skins[plyr->skin].hudname) > 0)
						name_patch = W_CachePatchName(skins[plyr->skin].hudname, PU_STATIC);
					else
						name_patch = stuser;
				}

				if (skins[plyr->skin].facescale != FRACUNIT) {
					fixed_t scale = skins[plyr->skin].facescale;

					V_DrawScaledPatch(SCX(52),SCY(192)-(FixedMul(face_patch->height<<FRACBITS, vid.dupy*scale)>>FRACBITS), FG | V_NOSCALESTART,name_patch);
					V_DrawCustomScaledTranslationPatch(SCX(16),SCY(192)-(FixedMul(face_patch->height<<FRACBITS, vid.dupy*scale)>>FRACBITS), scale, FG | V_NOSCALESTART,face_patch,colormap);
				}
				else {
					V_DrawScaledPatch(SCX(52),SCY(192)-(face_patch->height*vid.dupy), FG | V_NOSCALESTART,name_patch);
					V_DrawScaledTranslationPatch(SCX(16),SCY(192)-(face_patch->height*vid.dupy), FG | V_NOSCALESTART,face_patch,colormap);
				}

				// draw the number of lives
				ST_drawOverlayNum(SCX(88), // was 50 Tails 10-31-99
								SCY(197)-(16*vid.dupy),
								plyr->lives,
								tallnum,NULL);

				// now draw the "x"
				V_DrawScaledPatch (SCX(56),SCY(184), FG | V_NOSCALESTART, stlivex);
			}
			// end lives status Nozomi 03-01-2026
			
			// start rings status Nozomi 03-01-2026
			{
				int hud_rings = plyr->health-1;
				int x_pos = 112;
				int y_pos = 58;

				if (hud_rings < 0)
					hud_rings++;

				if (mapheaders[gamemap].special) {
					y_pos -= 16;

					if (mapheaders[gamemap].special == 2)
						V_DrawStringWhite(116, y_pos-14, "x 100");
				}

				ST_drawOverlayNum(SCX(x_pos), SCY(y_pos)-(16*vid.dupy), hud_rings, tallnum, NULL);

				if(plyr->health <= 1 && leveltime/5 & 1)
					V_DrawScaledPatch (SCX(16),SCY(y_pos)-(16*vid.dupy), FG | V_NOSCALESTART,rrings); // Tails 03-14-2000
				else
					V_DrawScaledPatch (SCX(16),SCY(y_pos)-(16*vid.dupy), FG | V_NOSCALESTART,sbohealth); // Was a number I forget and 198 =) Tails 10-31-99
			}
			// end rings status Nozomi 03-01-2026
			break;

		case 'f': // draw score Nozomi 03-01-2026
			{
				int hud_score = plyr->score;

				if (mapheaders[gamemap].special == 2) // SCORE ATTACK!!
					hud_score = plyr->sp_score + (plyr->mo->health-1)*100;
			
				ST_drawOverlayNum(SCX(128), SCY(10), hud_score, tallnum, NULL); // Draw Score Num Nozomi 03-01-2026
				V_DrawScaledPatch (SCX(16),SCY(10), FG | V_NOSCALESTART,sbofrags); // Draw SCORE Tails 03-01-2000
			}
			break;
		case 'a': // draw ammo
           break;

        case 'k': // draw keys
           break;

        case 'm': // Draw time hud Nozomi 03-01-2026
			if (!(mapheaders[gamemap].special)) {
				if(cv_timetic.value) // Special option to show tics instead of MM:SS
				{
					ST_drawOverlayNum(SCX(112), // Tails 02-29-2000
							 SCY(42)-(16*vid.dupy), // Draw the current single seconds time Tails 02-29-2000
							 leveltime,
							 tallnum,NULL);
				}
				else
				{
					if(plyr->seconds < 10)
					{
						ST_drawOverlayNum(SCX(104), // Tails 02-29-2000
							 SCY(42)-(16*vid.dupy), // Draw the current single seconds time Tails 02-29-2000
							 0,
							 tallnum,NULL);
					}

					ST_drawOverlayNum(SCX(112), // Tails 02-29-2000
							 SCY(42)-(16*vid.dupy), // Draw the current single seconds time Tails 02-29-2000
							 plyr->seconds,
							 tallnum,NULL);

					ST_drawOverlayNum(SCX(88), // Tails 02-29-2000
							 SCY(42)-(16*vid.dupy), // Draw the current single seconds time Tails 02-29-2000
							 plyr->minutes,
							 tallnum,NULL);

					V_DrawScaledPatch (SCX(88),SCY(42)-(16*vid.dupy), FG | V_NOSCALESTART,sbocolon); // colon location Tails 02-29-2000
				}

			   V_DrawScaledPatch (SCX(17),SCY(26), FG | V_NOSCALESTART,sbotime); // TIME location Tails 02-29-2000

			}

			if(plyr->redxvi > 1)
			{
				V_DrawString(52,80, "That's for buggering around");
				V_DrawString(56,96, "in my fangame, MAdventure!");
				V_DrawString(112,112, "Now sod off!");
			}

			// Countdown timer for Race Mode Tails 04-25-2001
			if(plyr->countdown)
			{
				char scountdown[33];
				sprintf(scountdown, "%i", plyr->countdown/TICRATE);
				V_DrawString(154, 176, scountdown);
			}
			// End Countdown timer for Race Mode Tails 04-25-2001

			// Start Tag display for Tag Mode Tails 05-09-2001
			if(plyr->tagit && cv_gametype.value == 3)
			{
				char stagit[33];
				sprintf(stagit, "%i", plyr->tagit/TICRATE);

				V_DrawString(120, 176, "YOU'RE IT!");
				V_DrawString(158-((int)strlen(stagit)*8)/2, 184, stagit);
			}

			if(plyr->tagzone && cv_gametype.value == 3)
			{
				char stagzone[33];
				sprintf(stagzone, "%i", plyr->tagzone/TICRATE);
				V_DrawString(104, 160, "IN NO-TAG ZONE");
				V_DrawString(158-((int)strlen(stagzone)*8)/2, 168, stagzone);
			}

			if(plyr->taglag && cv_gametype.value == 3)
			{
				char staglag[33];
				sprintf(staglag, "%i", plyr->taglag/TICRATE);
				V_DrawString(120, 160, "NO-TAG LAG");
				V_DrawString(158-((int)strlen(staglag)*8)/2, 168, staglag);
			}
			// End Tag display for Tag Mode Tails 05-09-2001


			// CTF HUD Stuff Tails 07-31-2001
			if(cv_gametype.value == 4)
			{
				int team;
				int whichflag;
				team = whichflag = 0;

				for(i=0; i<MAXPLAYERS; i++)
				{
					if(players[i].gotflag == 1)
					{
						team = players[i].ctfteam;
						whichflag = players[i].gotflag;
						break; // break, don't continue.
					}
				}

				if(plyr->ctfteam != team && team > 0 && plyr->ctfteam == whichflag)
					V_DrawStringWhite(128, 168, "OTHER TEAM HAS YOUR FLAG!");
				else if (plyr->ctfteam == team && team > 0)
				{
					if(plyr->ctfteam == whichflag)
						V_DrawString(128, 168, "YOUR TEAM HAS YOUR FLAG!");
					else
						V_DrawString(128, 176, "YOUR TEAM HAS ENEMY FLAG!");
				}

				team = whichflag = 0;

				for(i=0; i<MAXPLAYERS; i++)
				{
					if(players[i].gotflag == 2)
					{
						team = players[i].ctfteam;
						whichflag = players[i].gotflag;
						break; // break, don't continue.
					}
				}
				if(plyr->ctfteam != team && team > 0 && plyr->ctfteam == whichflag)
					V_DrawStringWhite(128, 168, "OTHER TEAM HAS YOUR FLAG!");
				else if (plyr->ctfteam == team && team > 0)
				{
					if(plyr->ctfteam == whichflag)
						V_DrawString(128, 168, "YOUR TEAM HAS YOUR FLAG!");
					else
						V_DrawString(128, 176, "YOUR TEAM HAS ENEMY FLAG!");
				}

				if(plyr->ctfteam == 1)
					V_DrawString(128, 192, "YOU'RE ON THE RED TEAM");
				else if(plyr->ctfteam == 2)
					V_DrawString(128, 192, "YOU'RE ON THE BLUE TEAM");

				if(plyr->gotflag == 1)
					V_DrawString(128, 184, "YOU HAVE THE RED FLAG");
				else if (plyr->gotflag == 2)
					V_DrawString(128, 184, "YOU HAVE THE BLUE FLAG");
			}

			// Special Stage HUD Tails 08-11-2001
			if(mapheaders[gamemap].special)
			{
				int pos_x = 112;
				int pos_y = 56;

				if (mapheaders[gamemap].special == 2) {
					pos_x += 16;
					pos_y += 16;
					V_DrawStringWhite(16, 44, "Req. Score:");
				}

				ST_drawOverlayNum(SCX(pos_x),SCY(pos_y)-(16*vid.dupy), totalrings, tallnum, NULL);
				
				// leaving this here for reference Nozomi 03-03-2026
				/*if(leveltime < 5*TICRATE)
				{
					V_DrawScaledPatch (SCX(100),SCY(90), FG | V_NOSCALESTART,getall); // Tails 08-11-2001
					ST_drawOverlayNum(SCX(160), SCY(93), totalrings, tallnum, NULL);
				}*/

				// mine, disappears when the titlecard does Nozomi 03-03-2026
				if (leveltime < 110) {
					if (mapheaders[gamemap].special == 2)
						V_DrawString(160 - (strlen("score attack!")-1)*4, 140, "score attack!");
					else
						V_DrawString(160 - (strlen(va("get %d rings!", totalrings))-1)*4, 140, va("get %d rings!", totalrings));
				}

				if(plyr->sstimer)
				{
					V_DrawString(124,160,"TIME LEFT");
					   ST_drawOverlayNum(SCX(168), // Tails 02-29-2000
										 SCY(192)-(16*vid.dupy), // Draw the current single seconds time Tails 02-29-2000
										 plyr->sstimer/TICRATE,
										 tallnum,NULL);
				}
				else
					V_DrawScaledPatch (SCX(125),SCY(90), FG | V_NOSCALESTART,timeup); // Tails 08-11-2001

			}

			// Emerald Hunt Indicators Tails 12-20-2001
			if(plyr->hunt1 && plyr->hunt1->health)
			{
				fixed_t dist;
				dist = P_AproxDistance(P_AproxDistance(plyr->mo->x - plyr->hunt1->x, plyr->mo->y - plyr->hunt1->y), plyr->mo->z - plyr->hunt1->z);

				if(dist < 128*FRACUNIT)
				{
					V_DrawScaledPatch(SCX(132), SCY(176), FG | V_NOSCALESTART, homing6);
					if(leveltime % 5 == 1)
						S_StartSound(0, sfx_shotgn);
				}
				else if(dist < 512*FRACUNIT)
				{
					V_DrawScaledPatch(SCX(132), SCY(176), FG | V_NOSCALESTART, homing5);
					if(leveltime % 10 == 1)
						S_StartSound(0, sfx_shotgn);
				}
				else if(dist < 1024*FRACUNIT)
				{
					V_DrawScaledPatch(SCX(132), SCY(176), FG | V_NOSCALESTART, homing4);
					if(leveltime % 20 == 1)
						S_StartSound(0, sfx_shotgn);
				}
				else if(dist < 2048*FRACUNIT)
				{
					V_DrawScaledPatch(SCX(132), SCY(176), FG | V_NOSCALESTART, homing3);
					if(leveltime % 30 == 1)
						S_StartSound(0, sfx_shotgn);
				}
				else if(dist < 3072*FRACUNIT)
				{
					V_DrawScaledPatch(SCX(132), SCY(176), FG | V_NOSCALESTART, homing2);
					if(leveltime % 35 ==1)
						S_StartSound(0, sfx_shotgn);
				}
				else
				{
					V_DrawScaledPatch(SCX(132), SCY(176), FG | V_NOSCALESTART, homing1);
				}
			}
			if(plyr->hunt2 && plyr->hunt2->health)
			{
				fixed_t dist;
				dist = P_AproxDistance(P_AproxDistance(plyr->mo->x - plyr->hunt2->x, plyr->mo->y - plyr->hunt2->y), plyr->mo->z - plyr->hunt2->z);
				if(dist < 128*FRACUNIT)
				{
					V_DrawScaledPatch(SCX(152), SCY(176), FG | V_NOSCALESTART, homing6);
					if(leveltime % 5 == 1)
						S_StartSound(0, sfx_shotgn);
				}
				else if(dist < 512*FRACUNIT)
				{
					V_DrawScaledPatch(SCX(152), SCY(176), FG | V_NOSCALESTART, homing5);
					if(leveltime % 10 == 1)
						S_StartSound(0, sfx_shotgn);
				}
				else if(dist < 1024*FRACUNIT)
				{
					V_DrawScaledPatch(SCX(152), SCY(176), FG | V_NOSCALESTART, homing4);
					if(leveltime % 20 == 1)
						S_StartSound(0, sfx_shotgn);
				}
				else if(dist < 2048*FRACUNIT)
				{
					V_DrawScaledPatch(SCX(152), SCY(176), FG | V_NOSCALESTART, homing3);
					if(leveltime % 30 == 1)
						S_StartSound(0, sfx_shotgn);
				}
				else if(dist < 3072*FRACUNIT)
				{
					V_DrawScaledPatch(SCX(152), SCY(176), FG | V_NOSCALESTART, homing2);
					if(leveltime % 35 == 1)
						S_StartSound(0, sfx_shotgn);
				}
				else
				{
					V_DrawScaledPatch(SCX(152), SCY(176), FG | V_NOSCALESTART, homing1);
				}
			}
			if(plyr->hunt3 && plyr->hunt3->health)
			{
				fixed_t dist;
				dist = P_AproxDistance(P_AproxDistance(plyr->mo->x - plyr->hunt3->x, plyr->mo->y - plyr->hunt3->y), plyr->mo->z - plyr->hunt3->z);
				if(dist < 128*FRACUNIT)
				{
					V_DrawScaledPatch(SCX(172), SCY(176), FG | V_NOSCALESTART, homing6);
					if(leveltime % 5 == 1)
						S_StartSound(0, sfx_shotgn);
				}
				else if(dist < 512*FRACUNIT)
				{
					V_DrawScaledPatch(SCX(172), SCY(176), FG | V_NOSCALESTART, homing5);
					if(leveltime % 10 == 1)
						S_StartSound(0, sfx_shotgn);
				}
				else if(dist < 1024*FRACUNIT)
				{
					V_DrawScaledPatch(SCX(172), SCY(176), FG | V_NOSCALESTART, homing4);
					if(leveltime % 20 == 1)
						S_StartSound(0, sfx_shotgn);
				}
				else if(dist < 2048*FRACUNIT)
				{
					V_DrawScaledPatch(SCX(172), SCY(176), FG | V_NOSCALESTART, homing3);
					if(leveltime % 30 == 1)
						S_StartSound(0, sfx_shotgn);
				}
				else if(dist < 3072*FRACUNIT)
				{
					V_DrawScaledPatch(SCX(172), SCY(176), FG | V_NOSCALESTART, homing2);
					if(leveltime % 35 == 1)
						S_StartSound(0, sfx_shotgn);
				}
				else
				{
					V_DrawScaledPatch(SCX(172), SCY(176), FG | V_NOSCALESTART, homing1);
				}
			}

#define titledrawtime 146
			// draw level title Tails
			if(leveltime < titledrawtime)
			{
				static patch_t*  lvlttl;
				static patch_t*  ttlnum;

				int lvlttlxpos;
				int ttlnumxpos;
				int zonexpos;

				int start_ttl_posy = 0;
				int end_ttl_posy = 80;
				int end_ttl_posy2 = 200;

				int start_zone_posy = 200;
				int end_zone_posy = 104;
				int end_zone_posy2 = 0;

				if (strlen(mapheaders[gamemap].ttlcardlump) > 0)
					lvlttl = W_CachePatchName (mapheaders[gamemap].ttlcardlump, PU_STATIC);
				else if (mapheaders[gamemap].special)
					lvlttl = W_CachePatchName ("LVLTTLS", PU_STATIC);
				else
					lvlttl = W_CachePatchName ("LVLTTLM", PU_STATIC);

				if (mapheaders[gamemap].act > 0)
					switch (mapheaders[gamemap].act) {
						case 1:
						default:
							ttlnum = ttlone;
							break;
						case 2:
							ttlnum = ttltwo;
							break;
						case 3:
							ttlnum = ttlthree;
							break;
						case 4:
							ttlnum = ttlfour;
							break;
						case 5:
							ttlnum = ttlfive;
							break;
						case 6:
							ttlnum = ttlsix;
							break;
						case 7:
							ttlnum = ttlseven;
							break;
						case 8:
							ttlnum = ttleight;
							break;
					}

				lvlttlxpos = 160 - (lvlttl->width/2);
				ttlnumxpos = 160 + (ttlzone->width/3*2);
				zonexpos = 160 - (ttlzone->width/2);

				// Smooth Animation - Jisk 03-19-26
#define anim1end 40 // OLD: 7
#define anim2start 120 //OLD: 104
#define anim2end 145 //OLD: 109

				if (leveltime < anim1end)
				{
					int div = FixedDiv(leveltime*FRACUNIT, (anim1end+1)*FRACUNIT);
					int lvlttlypos = Easing_OutQuint(div, 0, 80);
					int ztypos = Easing_OutQuint(div, 200, 104);

					V_DrawScaledPatch (SCX(lvlttlxpos),SCY(lvlttlypos), FG | V_NOSCALESTART,lvlttl);
					V_DrawScaledPatch (SCX(zonexpos),SCY(ztypos), FG | V_NOSCALESTART,ttlzone);
					if (mapheaders[gamemap].act > 0)
						V_DrawScaledPatch (SCX(ttlnumxpos),SCY(ztypos), FG | V_NOSCALESTART,ttlnum); 
				}
				else if ((leveltime >= anim1end) && (leveltime < anim2start))
				{
					int lvlttlypos = 80;
					int ztypos = 104;

					V_DrawScaledPatch (SCX(lvlttlxpos),SCY(lvlttlypos), FG | V_NOSCALESTART,lvlttl);
					V_DrawScaledPatch (SCX(zonexpos),SCY(ztypos), FG | V_NOSCALESTART,ttlzone);
					if (mapheaders[gamemap].act > 0)
						V_DrawScaledPatch (SCX(ttlnumxpos),SCY(ztypos), FG | V_NOSCALESTART,ttlnum);
				}
				else if (leveltime >= anim2start)
				{
					int div = FixedDiv((leveltime-anim2start)*FRACUNIT, (anim2end-anim2start)*FRACUNIT);
					int lvlttlypos = Easing_InQuint(div, 80, 200);
					int ztypos = Easing_InQuint(div, 104, 0);

					V_DrawScaledPatch (SCX(lvlttlxpos),SCY(lvlttlypos), FG | V_NOSCALESTART,lvlttl);
					V_DrawScaledPatch (SCX(zonexpos),SCY(ztypos), FG | V_NOSCALESTART,ttlzone);
					if (mapheaders[gamemap].act > 0)
						V_DrawScaledPatch (SCX(ttlnumxpos),SCY(ztypos), FG | V_NOSCALESTART,ttlnum); 
				}
#undef anim1end
#undef anim2start
#undef anim2end

				/* OLD LEVEL TITLE CODE
				if(leveltime == 1)
				{
					V_DrawScaledPatch (SCX(lvlttlxpos),SCY(0), FG | V_NOSCALESTART,lvlttl); // Tails 11-01-2000
					V_DrawScaledPatch (SCX(zonexpos),SCY(200), FG | V_NOSCALESTART,ttlzone); // Tails 11-01-2000
					if (mapheaders[gamemap].act > 0)
						V_DrawScaledPatch (SCX(ttlnumxpos),SCY(200), FG | V_NOSCALESTART,ttlnum); // Tails 11-01-2000
				}
				else if (leveltime == 2)
				{
					V_DrawScaledPatch (SCX(lvlttlxpos),SCY(12), FG | V_NOSCALESTART,lvlttl); // Tails 11-01-2000
					V_DrawScaledPatch (SCX(zonexpos),SCY(188), FG | V_NOSCALESTART,ttlzone); // Tails 11-01-2000
					if (mapheaders[gamemap].act > 0)
						V_DrawScaledPatch (SCX(ttlnumxpos),SCY(188), FG | V_NOSCALESTART,ttlnum); // Tails 11-01-2000
				}
				else if (leveltime == 3)
				{
					V_DrawScaledPatch (SCX(lvlttlxpos),SCY(24), FG | V_NOSCALESTART,lvlttl); // Tails 11-01-2000
					V_DrawScaledPatch (SCX(zonexpos),SCY(176), FG | V_NOSCALESTART,ttlzone); // Tails 11-01-2000
					if (mapheaders[gamemap].act > 0)
						V_DrawScaledPatch (SCX(0),SCY(176), FG | V_NOSCALESTART,ttlnum); // Tails 11-01-2000
				}
				else if (leveltime == 4)
				{
					V_DrawScaledPatch (SCX(lvlttlxpos),SCY(36), FG | V_NOSCALESTART,lvlttl); // Tails 11-01-2000
					V_DrawScaledPatch (SCX(zonexpos),SCY(164), FG | V_NOSCALESTART,ttlzone); // Tails 11-01-2000
					if (mapheaders[gamemap].act > 0)
						V_DrawScaledPatch (SCX(ttlnumxpos),SCY(164), FG | V_NOSCALESTART,ttlnum); // Tails 11-01-2000
				}
				else if (leveltime == 5)
				{
					V_DrawScaledPatch (SCX(lvlttlxpos),SCY(48), FG | V_NOSCALESTART,lvlttl); // Tails 11-01-2000
					V_DrawScaledPatch (SCX(zonexpos),SCY(152), FG | V_NOSCALESTART,ttlzone); // Tails 11-01-2000
					if (mapheaders[gamemap].act > 0)
						V_DrawScaledPatch (SCX(ttlnumxpos),SCY(152), FG | V_NOSCALESTART,ttlnum); // Tails 11-01-2000
				}
				else if (leveltime == 6)
				{
					V_DrawScaledPatch (SCX(lvlttlxpos),SCY(60), FG | V_NOSCALESTART,lvlttl); // Tails 11-01-2000
					V_DrawScaledPatch (SCX(zonexpos),SCY(140), FG | V_NOSCALESTART,ttlzone); // Tails 11-01-2000
					if (mapheaders[gamemap].act > 0)
						V_DrawScaledPatch (SCX(ttlnumxpos),SCY(140), FG | V_NOSCALESTART,ttlnum); // Tails 11-01-2000
				}
				else if (leveltime == 7)
				{
					V_DrawScaledPatch (SCX(lvlttlxpos),SCY(72), FG | V_NOSCALESTART,lvlttl); // Tails 11-01-2000
					V_DrawScaledPatch (SCX(zonexpos),SCY(128), FG | V_NOSCALESTART,ttlzone); // Tails 11-01-2000
					if (mapheaders[gamemap].act > 0)
						V_DrawScaledPatch (SCX(ttlnumxpos),SCY(128), FG | V_NOSCALESTART,ttlnum); // Tails 11-01-2000
				}
				else if (leveltime > 7 && leveltime < 105)
				{
					V_DrawScaledPatch (SCX(lvlttlxpos),SCY(80), FG | V_NOSCALESTART,lvlttl); // Tails 11-01-2000
					V_DrawScaledPatch (SCX(zonexpos),SCY(104), FG | V_NOSCALESTART,ttlzone); // Tails 11-01-2000
					if (mapheaders[gamemap].act > 0)
						V_DrawScaledPatch (SCX(ttlnumxpos),SCY(104), FG | V_NOSCALESTART,ttlnum); // Tails 11-01-2000
				}
				else if (leveltime == 105)
				{
					V_DrawScaledPatch (SCX(lvlttlxpos),SCY(104), FG | V_NOSCALESTART,lvlttl); // Tails 11-01-2000
					V_DrawScaledPatch (SCX(zonexpos),SCY(80), FG | V_NOSCALESTART,ttlzone); // Tails 11-01-2000
					if (mapheaders[gamemap].act > 0)
						V_DrawScaledPatch (SCX(ttlnumxpos),SCY(80), FG | V_NOSCALESTART,ttlnum); // Tails 11-01-2000
				}
				else if (leveltime == 106)
				{
					V_DrawScaledPatch (SCX(lvlttlxpos),SCY(128), FG | V_NOSCALESTART,lvlttl); // Tails 11-01-2000
					V_DrawScaledPatch (SCX(zonexpos),SCY(56), FG | V_NOSCALESTART,ttlzone); // Tails 11-01-2000
					if (mapheaders[gamemap].act > 0)
						V_DrawScaledPatch (SCX(ttlnumxpos),SCY(56), FG | V_NOSCALESTART,ttlnum); // Tails 11-01-2000
				}
				else if (leveltime == 107)
				{
					V_DrawScaledPatch (SCX(lvlttlxpos),SCY(152), FG | V_NOSCALESTART,lvlttl); // Tails 11-01-2000
					V_DrawScaledPatch (SCX(zonexpos),SCY(32), FG | V_NOSCALESTART,ttlzone); // Tails 11-01-2000
					if (mapheaders[gamemap].act > 0)
						V_DrawScaledPatch (SCX(ttlnumxpos),SCY(32), FG | V_NOSCALESTART,ttlnum); // Tails 11-01-2000
				}
				else if (leveltime == 108)
				{
					V_DrawScaledPatch (SCX(lvlttlxpos),SCY(176), FG | V_NOSCALESTART,lvlttl); // Tails 11-01-2000
					V_DrawScaledPatch (SCX(zonexpos),SCY(8), FG | V_NOSCALESTART,ttlzone); // Tails 11-01-2000
					if (mapheaders[gamemap].act > 0)
						V_DrawScaledPatch (SCX(ttlnumxpos),SCY(8), FG | V_NOSCALESTART,ttlnum); // Tails 11-01-2000
				}
				else if (leveltime == 109)
				{
					V_DrawScaledPatch (SCX(lvlttlxpos),SCY(200), FG | V_NOSCALESTART,lvlttl); // Tails 11-01-2000
					V_DrawScaledPatch (SCX(zonexpos),SCY(0), FG | V_NOSCALESTART,ttlzone); // Tails 11-01-2000
					if (mapheaders[gamemap].act > 0)
						V_DrawScaledPatch (SCX(ttlnumxpos),SCY(0), FG | V_NOSCALESTART,ttlnum); // Tails 11-01-2000
				}
				*/
			}
			break;
		}
	}
}

#undef titledrawtime
