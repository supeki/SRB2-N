// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: r_plane.c,v 1.12 2000/11/11 13:59:46 bpereira Exp $
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
// $Log: r_plane.c,v $
// Revision 1.12  2000/11/11 13:59:46  bpereira
// no message
//
// Revision 1.11  2000/11/06 20:52:16  bpereira
// no message
//
// Revision 1.10  2000/11/02 17:50:09  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.9  2000/04/30 10:30:10  bpereira
// no message
//
// Revision 1.8  2000/04/18 17:39:39  stroggonmeth
// Bug fixes and performance tuning.
//
// Revision 1.7  2000/04/13 23:47:47  stroggonmeth
// See logs
//
// Revision 1.6  2000/04/08 17:29:25  stroggonmeth
// no message
//
// Revision 1.5  2000/04/06 21:06:19  stroggonmeth
// Optimized extra_colormap code...
// Added #ifdefs for older water code.
//
// Revision 1.4  2000/04/04 19:28:43  stroggonmeth
// Global colormaps working. Added a new linedef type 272.
//
// Revision 1.3  2000/04/04 00:32:48  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Here is a core component: drawing the floors and ceilings,
//       while maintaining a per column clipping list only.
//      Moreover, the sky areas have to be determined.
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "console.h"
#include "g_game.h"
#include "r_data.h"
#include "r_local.h"
#include "r_state.h"
#include "r_splats.h"   //faB(21jan):testing
#include "r_sky.h"
#include "p_tick.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

#include "p_setup.h"    // levelflats

planefunction_t         floorfunc;
planefunction_t         ceilingfunc;

//
// opening
//

// Here comes the obnoxious "visplane".
/*#define                 MAXVISPLANES 128 //SoM: 3/20/2000
visplane_t*             visplanes;
visplane_t*             lastvisplane;*/

//SoM: 3/23/2000: Use Boom visplane hashing.
#define           MAXVISPLANES      512

static visplane_t *visplanes[MAXVISPLANES];
static visplane_t *freetail;
static visplane_t **freehead = &freetail;


visplane_t*             floorplane;
visplane_t*             ceilingplane;
visplane_t*             waterplane;

visplane_t*             currentplane;

#ifdef R_FAKEFLOORS
planemgr_t              ffloor[MAXFFLOORS];
int                     numffloors;
#endif

//SoM: 3/23/2000: Boom visplane hashing routine.
#define visplane_hash(picnum,lightlevel,height) \
  ((unsigned)((picnum)*3+(lightlevel)+(height)*7) & (MAXVISPLANES-1))

// ?
/*#define MAXOPENINGS     MAXVIDWIDTH*128
short                   openings[MAXOPENINGS];
short*                  lastopening;*/

//SoM: 3/23/2000: Use boom opening limit removal
size_t maxopenings;
short *openings,*lastopening;



//
// Clip values are the solid pixel bounding the range.
//  floorclip starts out SCREENHEIGHT
//  ceilingclip starts out -1
//
short                   floorclip[MAXVIDWIDTH];
short                   ceilingclip[MAXVIDWIDTH];
fixed_t                 flatscale[MAXVIDWIDTH];

short                   waterclip[MAXVIDWIDTH];   //added:18-02-98:WATER!
boolean                 itswater;       //added:24-02-98:WATER!

//
// spanstart holds the start of a plane span
// initialized to 0 at start
//
int                     spanstart[MAXVIDHEIGHT];
//int                     spanstop[MAXVIDHEIGHT]; //added:08-02-98: Unused!!

//
// texture mapping
//
lighttable_t**          planezlight;
fixed_t                 planeheight;

//added:10-02-98: yslopetab is what yslope used to be,
//                yslope points somewhere into yslopetab,
//                now (viewheight/2) slopes are calculated above and
//                below the original viewheight for mouselook
//                (this is to calculate yslopes only when really needed)
//                (when mouselookin', yslope is moving into yslopetab)
//                Check R_SetupFrame, R_SetViewSize for more...
fixed_t                 yslopetab[MAXVIDHEIGHT*4];
fixed_t*                yslope;

fixed_t                 distscale[MAXVIDWIDTH];
fixed_t                 basexscale;
fixed_t                 baseyscale;

fixed_t                 cachedheight[MAXVIDHEIGHT];
fixed_t                 cacheddistance[MAXVIDHEIGHT];
fixed_t                 cachedxstep[MAXVIDHEIGHT];
fixed_t                 cachedystep[MAXVIDHEIGHT];

fixed_t   xoffs, yoffs;

//
// R_InitPlanes
// Only at game startup.
//
void R_InitPlanes (void)
{
  // Doh!
}


//profile stuff ---------------------------------------------------------
//#define TIMING
#ifdef TIMING
#include "p5prof.h"
         long long mycount;
         long long mytotal = 0;
         unsigned long  nombre = 100000;
#endif
//profile stuff ---------------------------------------------------------


//
// R_MapPlane
//
// Uses global vars:
//  planeheight
//  ds_source
//  basexscale
//  baseyscale
//  viewx
//  viewy
//  xoffs
//  yoffs
//
// BASIC PRIMITIVE
//
static int bgofs;
static int wtofs=0;

void R_MapPlane
( int           y,              // t1
  int           x1,
  int           x2 )
{
    angle_t     angle;
    fixed_t     distance;
    fixed_t     length;
    unsigned    index;

#ifdef RANGECHECK
    if (x2 < x1
        || x1<0
        || x2>=viewwidth
        || (unsigned)y>viewheight)
    {
        I_Error ("R_MapPlane: %i, %i at %i",x1,x2,y);
    }
#endif

    if (planeheight != cachedheight[y])
    {
        cachedheight[y] = planeheight;
        distance = cacheddistance[y] = FixedMul (planeheight, yslope[y]);
        ds_xstep = cachedxstep[y] = FixedMul (distance,basexscale);
        ds_ystep = cachedystep[y] = FixedMul (distance,baseyscale);
    }
    else
    {
        distance = cacheddistance[y];
        ds_xstep = cachedxstep[y];
        ds_ystep = cachedystep[y];
    }
    length = FixedMul (distance,distscale[x1]);
    angle = (viewangle + xtoviewangle[x1])>>ANGLETOFINESHIFT;
    ds_xfrac = viewx + FixedMul(finecosine[angle], length) + xoffs;
    ds_yfrac = -viewy - FixedMul(finesine[angle], length)  + yoffs;

    if (itswater)
	{
		const INT32 yay = (wtofs + (distance>>9) ) & 8191;
		// ripples da water texture
		bgofs = FixedDiv(finesine[yay], (1<<12) + (distance>>11))>>FRACBITS;
		angle = (viewangle + xtoviewangle[x1])>>ANGLETOFINESHIFT;

		angle = (angle + 2048) & 8191;  // 90 degrees
		ds_xfrac += FixedMul(finecosine[angle], (bgofs<<FRACBITS));
		ds_yfrac += FixedMul(finesine[angle], (bgofs<<FRACBITS));

		if (y+bgofs>=viewheight)
			bgofs = viewheight-y-1;
		if (y+bgofs<0)
			bgofs = -y;
	}

    if (fixedcolormap)
        ds_colormap = fixedcolormap;
    else
    {
        index = distance >> LIGHTZSHIFT;

        if (index >= MAXLIGHTZ )
            index = MAXLIGHTZ-1;

        ds_colormap = planezlight[index];
    }
    if(currentplane->extra_colormap && !fixedcolormap)
      ds_colormap = &currentplane->extra_colormap[ds_colormap - colormaps];

    ds_y = y;
    ds_x1 = x1;
    ds_x2 = x2;
    // high or low detail

//added:16-01-98:profile hspans drawer.
#ifdef TIMING
  ProfZeroTimer();
#endif

  spanfunc ();

#ifdef TIMING
  RDMSR(0x10,&mycount);
  mytotal += mycount;   //64bit add
  if(nombre--==0)
  I_Error("spanfunc() CPU Spy reports: 0x%d %d\n", *((int*)&mytotal+1),
                                        (int)mytotal );
#endif

}


//
// R_ClearPlanes
// At begining of frame.
//
//Fab:26-04-98:
// NOTE : uses con_clipviewtop, so that when console is on,
//        don't draw the part of the view hidden under the console
void R_ClearPlanes (player_t *player)
{
    int         i, p;
    angle_t     angle;

    // opening / clipping determination
    for (i=0 ; i<viewwidth ; i++)
    {
        floorclip[i] = viewheight;
        ceilingclip[i] = con_clipviewtop;       //Fab:26-04-98: was -1
        flatscale[i] = MAXINT;
#ifdef R_FAKEFLOORS
        for(p = 0; p < MAXFFLOORS; p++)
        {
          ffloor[p].f_clip[i] = viewheight;
          ffloor[p].c_clip[i] = con_clipviewtop;
        }
#endif
    }

    numffloors = 0;

    //lastvisplane = visplanes;

    //SoM: 3/23/2000
    for (i=0;i<MAXVISPLANES;i++)
      for (*freehead = visplanes[i], visplanes[i] = NULL; *freehead; )
        freehead = &(*freehead)->next;

    lastopening = openings;

    // texture calculation
    memset (cachedheight, 0, sizeof(cachedheight));

    // left to right mapping
    angle = (viewangle-ANG90)>>ANGLETOFINESHIFT;

    // scale will be unit scale at SCREENWIDTH/2 distance
    basexscale = FixedDiv (finecosine[angle],centerxfrac);
    baseyscale = -FixedDiv (finesine[angle],centerxfrac);
}


//SoM: 3/23/2000: New function, by Lee Killough
static visplane_t *new_visplane(unsigned hash)
{
  visplane_t *check = freetail;
  if (!check)
    check = calloc(1, sizeof *check);
  else
    if (!(freetail = freetail->next))
      freehead = &freetail;
  check->next = visplanes[hash];
  visplanes[hash] = check;
  return check;
}



//
// R_FindPlane : cherche un visplane ayant les valeurs identiques:
//               meme hauteur, meme flattexture, meme lightlevel.
//               Sinon en alloue un autre.
//
visplane_t* R_FindPlane( fixed_t height,
                         int     picnum,
                         int     lightlevel,
                         fixed_t xoff,
                         fixed_t yoff,
                         lighttable_t* planecolormap,
                         ffloor_t* ffloor)
{
    visplane_t* check;
    unsigned    hash; //SoM: 3/23/2000

    if (picnum == skyflatnum && ffloor)
    {
        height = 0;                     // all skys map together
        lightlevel = 0;
    }


    //SoM: 3/23/2000: New visplane algorithm uses hash table -- killough
    hash = visplane_hash(picnum,lightlevel,height);

    for (check=visplanes[hash]; check; check=check->next)
      if (height == check->height &&
          picnum == check->picnum &&
          lightlevel == check->lightlevel &&
          xoff == check->xoffs &&
          yoff == check->yoffs &&
          planecolormap == check->extra_colormap &&
          ffloor == check->ffloor)
        return check;

    check = new_visplane(hash);

/*    for (check=visplanes; check<lastvisplane; check++)
    {
        if (height == check->height
            && picnum == check->picnum
            && lightlevel == check->lightlevel
            && xoff == check->xoffs
            && yoff == check->yoffs)
        {
          return check;
        }
    }


    if (lastvisplane - visplanes == MAXVISPLANES)
        ExpandVisplanes();
        //I_Error ("R_FindPlane: no more visplanes");*/

    /*if (devparm &&
        (lastvisplane-visplanes) > maxusedvisplanes)
    {
        maxusedvisplanes = lastvisplane - visplanes;
        CONS_Printf ("maxusedvisplanes: %i\n", maxusedvisplanes);
    }*/


    //lastvisplane++;

    check->height = height;
    check->picnum = picnum;
    check->lightlevel = lightlevel;
    check->minx = vid.width;
    check->maxx = -1;
    check->xoffs = xoff;
    check->yoffs = yoff;
    check->extra_colormap = planecolormap;
    check->ffloor = ffloor;

    memset (check->top, 0xff, sizeof(check->top));
    memset (check->flatscale, 0xff, sizeof(fixed_t) * MAXVIDWIDTH);

    return check;
}


//
// R_CheckPlane : return same visplane or alloc a new one if needed
//
visplane_t*  R_CheckPlane( visplane_t*   pl,
                           int           start,
                           int           stop )
{
    int         intrl;
    int         intrh;
    int         unionl;
    int         unionh;
    int         x;

    if (start < pl->minx)
    {
        intrl = pl->minx;
        unionl = start;
    }
    else
    {
        unionl = pl->minx;
        intrl = start;
    }

    if (stop > pl->maxx)
    {
        intrh = pl->maxx;
        unionh = stop;
    }
    else
    {
        unionh = pl->maxx;
        intrh = stop;
    }

    //added 30-12-97 : 0xff ne vaut plus -1 avec un short...
    for (x=intrl ; x<= intrh ; x++)
        if (pl->top[x] != 0xffff)
            break;

    //SoM: 3/23/2000: Boom code
    if (x > intrh && !pl->ffloor)
      pl->minx = unionl, pl->maxx = unionh;
    else
      {
        unsigned hash = visplane_hash(pl->picnum, pl->lightlevel, pl->height);
        visplane_t *new_pl = new_visplane(hash);
  
        new_pl->height = pl->height;
        new_pl->picnum = pl->picnum;
        new_pl->lightlevel = pl->lightlevel;
        new_pl->xoffs = pl->xoffs;           // killough 2/28/98
        new_pl->yoffs = pl->yoffs;
        new_pl->extra_colormap = pl->extra_colormap;
        new_pl->ffloor = pl->ffloor;
        pl = new_pl;
        pl->minx = start;
        pl->maxx = stop;
        memset(pl->top, 0xff, sizeof pl->top);
        memset(pl->flatscale, 0xff, sizeof(fixed_t) * MAXVIDWIDTH);
      }
    return pl;

//SoM: Old code
    /*if (x > intrh)
    {
        pl->minx = unionl;
        pl->maxx = unionh;

        // use the same one
        return pl;
    }

    if(lastvisplane == (visplanes + MAXVISPLANES))
      ExpandVisplanes();
      //I_Error("Ran out of visplanes\n");

    // make a new visplane
    lastvisplane->height = pl->height;
    lastvisplane->picnum = pl->picnum;
    lastvisplane->lightlevel = pl->lightlevel;
    lastvisplane->xoffs = pl->xoffs;
    lastvisplane->yoffs = pl->yoffs;

    pl = lastvisplane++;
    pl->minx = start;
    pl->maxx = stop;

    memset (pl->top,0xff,sizeof(pl->top));

    return pl;*/
}


//
// R_MakeSpans
//
void R_MakeSpans
( int           x,
  int           t1,
  int           b1,
  int           t2,
  int           b2 )
{
    while (t1 < t2 && t1<=b1)
    {
        R_MapPlane (t1,spanstart[t1],x-1);
        t1++;
    }
    while (b1 > b2 && b1>=t1)
    {
        R_MapPlane (b1,spanstart[b1],x-1);
        b1--;
    }

    while (t2 < t1 && t2<=b2)
    {
        spanstart[t2] = x;
        t2++;
    }
    while (b2 > b1 && b2>=t2)
    {
        spanstart[b2] = x;
        b2--;
    }
}


static int waterofs;

static void R_DrawTranslucentWaterSpan_8(void)
{
	UINT32 xposition;
	UINT32 yposition;
	UINT32 xstep, ystep;

	byte *source;
	byte *colormap;
	byte *dest;
	byte *dsrc;

	size_t count;

	// SoM: we only need 6 bits for the integer part (0 thru 63) so the rest
	// can be used for the fraction part. This allows calculation of the memory address in the
	// texture with two shifts, an OR and one AND. (see below)
	// for texture sizes > 64 the amount of precision we can allow will decrease, but only by one
	// bit per power of two (obviously)
	// Ok, because I was able to eliminate the variable spot below, this function is now FASTER
	// than the original span renderer. Whodathunkit?
	xposition = ds_xfrac << nflatshiftup; yposition = (ds_yfrac + waterofs) << nflatshiftup;
	xstep = ds_xstep << nflatshiftup; ystep = ds_ystep << nflatshiftup;

	source = ds_source;
	colormap = ds_colormap;
	dest = ylookup[ds_y] + columnofs[ds_x1];
	dsrc = screens[1] + (ds_y+bgofs)*vid.width + ds_x1;
	count = ds_x2 - ds_x1 + 1;

	while (count >= 8)
	{
		// SoM: Why didn't I see this earlier? the spot variable is a waste now because we don't
		// have the uber complicated math to calculate it now, so that was a memory write we didn't
		// need!
		dest[0] = *(ds_transmap + (colormap[source[((yposition >> nflatyshift) & nflatmask) | (xposition >> nflatxshift)]] << 8) + dest[0]);
		xposition += xstep;
		yposition += ystep;

		dest[1] = *(ds_transmap + (colormap[source[((yposition >> nflatyshift) & nflatmask) | (xposition >> nflatxshift)]] << 8) + dest[1]);
		xposition += xstep;
		yposition += ystep;

		dest[2] = *(ds_transmap + (colormap[source[((yposition >> nflatyshift) & nflatmask) | (xposition >> nflatxshift)]] << 8) + dest[2]);
		xposition += xstep;
		yposition += ystep;

		dest[3] = *(ds_transmap + (colormap[source[((yposition >> nflatyshift) & nflatmask) | (xposition >> nflatxshift)]] << 8) + dest[3]);
		xposition += xstep;
		yposition += ystep;

		dest[4] = *(ds_transmap + (colormap[source[((yposition >> nflatyshift) & nflatmask) | (xposition >> nflatxshift)]] << 8) + dest[4]);
		xposition += xstep;
		yposition += ystep;

		dest[5] = *(ds_transmap + (colormap[source[((yposition >> nflatyshift) & nflatmask) | (xposition >> nflatxshift)]] << 8) + dest[5]);
		xposition += xstep;
		yposition += ystep;

		dest[6] = *(ds_transmap + (colormap[source[((yposition >> nflatyshift) & nflatmask) | (xposition >> nflatxshift)]] << 8) + dest[6]);
		xposition += xstep;
		yposition += ystep;

		dest[7] = *(ds_transmap + (colormap[source[((yposition >> nflatyshift) & nflatmask) | (xposition >> nflatxshift)]] << 8) + dest[7]);
		xposition += xstep;
		yposition += ystep;

		dest += 8;
		count -= 8;
	}
	while (count--)
	{
		*dest = *(ds_transmap + (colormap[source[((yposition >> nflatyshift) & nflatmask) | (xposition >> nflatxshift)]] << 8) + *dest);
		dest++;
		xposition += xstep;
		yposition += ystep;
	}
}



static int wateranim;


byte* R_GetFlat (int  flatnum);

void R_DrawPlanes (void)
{
    visplane_t*         pl;
    int                 x;
    int                 angle;
    int                 i; //SoM: 3/23/2000

    //
    // DRAW NON-WATER VISPLANES FIRST
    //

    spanfunc = basespanfunc;
	wallcolfunc = walldrawerfunc;

    for (i=0;i<MAXVISPLANES;i++, pl++)
    for (pl=visplanes[i]; pl; pl=pl->next)
    {

        // sky flat
		if (pl->picnum == skyflatnum)
		{
			// use correct aspect ratio scale
			dc_iscale = skyscale;

			// Sky is always drawn full bright,
			//  i.e. colormaps[0] is used.
			// Because of this hack, sky is not affected
			//  by INVUL inverse mapping.
			dc_colormap = colormaps;
			dc_texturemid = skytexturemid;
			dc_texheight = textureheight[skytexture]
				>>FRACBITS;
			for (x = pl->minx; x <= pl->maxx; x++)
			{
				dc_yl = pl->top[x];
				dc_yh = pl->bottom[x];

				if (dc_yl <= dc_yh)
				{
					angle = (viewangle + xtoviewangle[x])>>ANGLETOSKYSHIFT;
					dc_x = x;
					dc_source =
						R_GetColumn(skytexture,
							angle);
					wallcolfunc();
				}
			}
			continue;
		}

        if(pl->ffloor)
          continue;

        R_DrawSinglePlane(pl, true);
    }

	waterofs = (leveltime & 1)*16384;
	wtofs = leveltime * 140;
}




void R_DrawSinglePlane(visplane_t* pl, boolean handlesource)
{
  int                 light;
  int                 x;
  int                 stop;
  size_t size;

  if (pl->minx > pl->maxx)
    return;

  itswater = false;
  spanfunc = basespanfunc;

  if (pl->ffloor)
  {
	  if (pl->ffloor->flags & (FF_TRANSLUCENT|FF_SWIMMABLE))
	  {
		spanfunc = R_DrawTranslucentSpan_8;
		ds_transmap = ((1)<<FF_TRANSSHIFT) - 0x10000 + transtables;

		if (!pl->extra_colormap)
			light = (pl->lightlevel >> LIGHTSEGSHIFT)+extralight;
		else
			light = LIGHTLEVELS-1;
	  }
	  else light = (pl->lightlevel >> LIGHTSEGSHIFT)+extralight;

	  if (pl->ffloor->flags & FF_SWIMMABLE) {
		itswater = true;
		if (spanfunc == R_DrawTranslucentSpan_8) {
			ds_transmap = (tr_translo<<FF_TRANSSHIFT) - 0x10000 + transtables;
			spanfunc = R_DrawTranslucentWaterSpan_8;
		}
	  }
  }
  else light = (pl->lightlevel >> LIGHTSEGSHIFT)+extralight;

  currentplane = pl;

  ds_source = (byte *) W_CacheLumpNum (levelflats[pl->picnum].lumpnum, PU_STATIC);

  size = W_LumpLength(levelflats[pl->picnum].lumpnum);

  switch (size)
	{
		case 4194304: // 2048x2048 lump
			nflatmask = 0x3FF800;
			nflatxshift = 21;
			nflatyshift = 10;
			nflatshiftup = 5;
			break;
		case 1048576: // 1024x1024 lump
			nflatmask = 0xFFC00;
			nflatxshift = 22;
			nflatyshift = 12;
			nflatshiftup = 6;
			break;
		case 262144:// 512x512 lump'
			nflatmask = 0x3FE00;
			nflatxshift = 23;
			nflatyshift = 14;
			nflatshiftup = 7;
			break;
		case 65536: // 256x256 lump
			nflatmask = 0xFF00;
			nflatxshift = 24;
			nflatyshift = 16;
			nflatshiftup = 8;
			break;
		case 16384: // 128x128 lump
			nflatmask = 0x3F80;
			nflatxshift = 25;
			nflatyshift = 18;
			nflatshiftup = 9;
			break;
		case 1024: // 32x32 lump
			nflatmask = 0x3E0;
			nflatxshift = 27;
			nflatyshift = 22;
			nflatshiftup = 11;
			break;
		default: // 64x64 lump
			nflatmask = 0xFC0;
			nflatxshift = 26;
			nflatyshift = 20;
			nflatshiftup = 10;
			break;
	}

  xoffs = pl->xoffs;
  yoffs = pl->yoffs;
  planeheight = abs(pl->height-viewz);

  if (light >= LIGHTLEVELS)
      light = LIGHTLEVELS-1;

  if (light < 0)
      light = 0;

  planezlight = zlight[light];

  //set the MAXIMUM value for unsigned
  pl->top[pl->maxx+1] = 0xffff;
  pl->top[pl->minx-1] = 0xffff;
  pl->bottom[pl->maxx+1] = 0x0000;
  pl->bottom[pl->minx-1] = 0x0000;

  stop = pl->maxx + 1;

  for (x=pl->minx ; x<= stop ; x++)
  {
    R_MakeSpans(x,pl->top[x-1],
                pl->bottom[x-1],
                pl->top[x],
                pl->bottom[x]);
  }

  Z_ChangeTag (ds_source, PU_CACHE);
}


void R_PlaneBounds(visplane_t* plane, int *hi, int *low)
{
  int  i;
  *hi = plane->top[plane->minx];
  *low = plane->bottom[plane->minx];

  for (i = plane->minx + 1; i <= plane->maxx; i++)
  {
  	if (plane->top[i] < *hi)
  	*hi = plane->top[i];
  	if (plane->bottom[i] > *low)
  	*low = plane->bottom[i];
  }
}
