// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: p_mobj.c,v 1.10 2000/04/16 18:38:07 bpereira Exp $
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
// $Log: p_mobj.c,v $
// Revision 1.10  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.9  2000/04/11 19:07:24  stroggonmeth
// Finished my logs, fixed a crashing bug.
//
// Revision 1.8  2000/04/08 17:29:25  stroggonmeth
// no message
//
// Revision 1.7  2000/04/06 20:40:22  hurdler
// Mostly remove warnings under windows
//
// Revision 1.6  2000/04/05 15:47:46  stroggonmeth
// Added hack for Dehacked lumps. Transparent sprites are now affected by colormaps.
//
// Revision 1.5  2000/04/04 00:32:47  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.4  2000/03/29 19:39:48  bpereira
// no message
//
// Revision 1.3  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.2  2000/02/26 00:28:42  hurdler
// Mostly bug fix (see borislog.txt 23-2-2000, 24-2-2000)
//
//
// DESCRIPTION:
//      Moving object handling. Spawn functions.
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "g_game.h"
#include "g_input.h"
#include "st_stuff.h"
#include "hu_stuff.h"
#include "p_local.h"
#include "p_inter.h"
#include "p_setup.h"    //levelflats to test if mobj in water sector
#include "r_main.h"
#include "r_things.h"
#include "r_sky.h"
#include "s_sound.h"
#include "z_zone.h"
#include "m_random.h"
#include "info.h"
#include "d_clisrv.h"
#include "r_splats.h"   //faB: in dev.

	player_t  *plyr;
// protos.
void CV_ViewHeight_OnChange (void);

CV_PossibleValue_t viewheight_cons_t[]={{16,"MIN"},{56,"MAX"},{0,NULL}};

consvar_t cv_viewheight = {"viewheight","41",0,viewheight_cons_t,NULL};

//Fab:26-07-98:
consvar_t cv_gravity = {"gravity","0.5",CV_NETVAR|CV_FLOAT|CV_SHOWMODIF}; // No longer required in autoexec.cfg! Tails 12-01-99
consvar_t cv_splats  = {"splats","1",CV_SAVE,CV_OnOff};

//
// P_SetMobjState
// Returns true if the mobj is still present.
//
//SoM: 4/7/2000: Boom code...
boolean P_SetMobjState ( mobj_t*       mobj,
                         statenum_t    state )
{
  state_t*  st;

  //remember states seen, to detect cycles:

  static statenum_t seenstate_tab[NUMSTATES]; // fast transition table
  statenum_t *seenstate = seenstate_tab;      // pointer to table
  static int recursion;                       // detects recursion
  statenum_t i = state;                       // initial state
  boolean ret = true;                         // return value
  statenum_t tempstate[NUMSTATES];            // for use with recursion

  if (recursion++)                            // if recursion detected,
    memset(seenstate=tempstate,0,sizeof tempstate); // clear state table

  do
    {
    if (state == S_NULL)
      {
      mobj->state = (state_t *) S_NULL;
      P_RemoveMobj (mobj);
      ret = false;
      break;                 // killough 4/9/98
      }

    st = &states[state];
    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;

    // Modified handling.
    // Call action functions when the state is set

    if (st->action.acp1)
      st->action.acp1(mobj);

    seenstate[state] = 1 + st->nextstate;   // killough 4/9/98

    state = st->nextstate;
    } while (!mobj->tics && !seenstate[state]);   // killough 4/9/98

  if (ret && !mobj->tics)  // killough 4/9/98: detect state cycles
    CONS_Printf("Warning: State Cycle Detected");

  if (!--recursion)
    for (;(state=seenstate[i]);i=state-1)
      seenstate[i] = 0;  // killough 4/9/98: erase memory of states

  return ret;
}

static boolean P_SetPrecipMobjState(precipmobj_t* mobj, statenum_t state)
{
	state_t* st;

	if(state == S_NULL)
	{ // Remove mobj
		P_RemovePrecipMobj(mobj);
		return false;
	}
	st = &states[state];
	mobj->state = st;
	mobj->tics = st->tics;
	mobj->sprite = st->sprite;
	mobj->frame = st->frame;
	return true;
}

//
// P_ExplodeMissile
//
// Added some stuff here Tails 08-26-2001
void P_ExplodeMissile (mobj_t* mo)
{
	mobj_t*	explodemo;

    mo->momx = mo->momy = mo->momz = 0;

    P_SetMobjState (mo, mobjinfo[mo->type].deathstate);

	if(mo->type == MT_DETON)
	{
		explodemo = P_SpawnMobj(mo->x, mo->y, mo->z, MT_EXPLODE);
		explodemo->momx += (P_Random() % 32) * FRACUNIT/8;
		explodemo->momy += (P_Random() % 32) * FRACUNIT/8;
		S_StartSound(explodemo, sfx_pop);
		explodemo = P_SpawnMobj(mo->x, mo->y, mo->z, MT_EXPLODE);
		explodemo->momx += (P_Random() % 64) * FRACUNIT/8;
		explodemo->momy -= (P_Random() % 64) * FRACUNIT/8;
		S_StartSound(explodemo, sfx_dmpain);
		explodemo = P_SpawnMobj(mo->x, mo->y, mo->z, MT_EXPLODE);
		explodemo->momx -= (P_Random() % 128) * FRACUNIT/8;
		explodemo->momy += (P_Random() % 128) * FRACUNIT/8;
		S_StartSound(explodemo, sfx_pop);
		explodemo = P_SpawnMobj(mo->x, mo->y, mo->z, MT_EXPLODE);
		explodemo->momx -= (P_Random() % 96) * FRACUNIT/8;
		explodemo->momy -= (P_Random() % 96) * FRACUNIT/8;
		S_StartSound(explodemo, sfx_cybdth);
	}

    mo->tics -= P_Random()&3;

    if (mo->tics < 1)
        mo->tics = 1;

    mo->flags &= ~MF_MISSILE;

    if (mo->info->deathsound)
        S_StartSound (mo, mo->info->deathsound);
}


//
// P_XYMovement
//
#define STOPSPEED               0xffff
#define FRICTION                0xe800   //0.90625

//added:22-02-98: adds friction on the xy plane
void P_XYFriction (mobj_t* mo, fixed_t oldx, fixed_t oldy, boolean oldfriction)
{
    //valid only if player avatar
    player_t*   player = mo->player;

	if (player)
	{
		if (
			player->rmomx > -STOPSPEED
			&& player->rmomx < STOPSPEED
			&& player->rmomy > -STOPSPEED
			&& player->rmomy < STOPSPEED
			&& (
				player->cmd.forwardmove == 0
				&& player->cmd.sidemove == 0 
				&& !player->mfspinning
			)
		) {
			// if in a walking frame, stop moving
			if (player && player->walking == 1 && mo->type != MT_SPIRIT)
				P_SetMobjState (player->mo, S_PLAY);

			mo->momx = player->cmomx;
			mo->momy = player->cmomy;
		}
		else
		{
			if (oldfriction)
			{
				mo->momx = FixedMul (mo->momx, FRICTION);
				mo->momy = FixedMul (mo->momy, FRICTION);
			}
			else
			{
				if (oldx == mo->x && oldy == mo->y)
				{
					mo->momx = FixedMul(mo->momx,ORIG_FRICTION);
					mo->momy = FixedMul(mo->momy,ORIG_FRICTION);
				}
				else
				{
					mo->momx = FixedMul(mo->momx,mo->friction);
					mo->momy = FixedMul(mo->momy,mo->friction);
				}

				mo->friction = ORIG_FRICTION;
			}
		}
	}
	else
    {
		if (
			mo->momx > -STOPSPEED
			&& mo->momx < STOPSPEED
			&& mo->momy > -STOPSPEED
			&& mo->momy < STOPSPEED
		) {
			mo->momx = 0;
			mo->momy = 0;
		}
		else
		{
			if (oldfriction)
			{
				mo->momx = FixedMul (mo->momx, FRICTION);
				mo->momy = FixedMul (mo->momy, FRICTION);
			}
			else
			{
				if (oldx == mo->x && oldy == mo->y)
				{
					mo->momx = FixedMul(mo->momx,ORIG_FRICTION);
					mo->momy = FixedMul(mo->momy,ORIG_FRICTION);
				}
				else
				{
					mo->momx = FixedMul(mo->momx,mo->friction);
					mo->momy = FixedMul(mo->momy,mo->friction);
				}

				mo->friction = ORIG_FRICTION;
			}
		}
	}
}

void P_XYMovement (mobj_t* mo)
{
    fixed_t     ptryx;
    fixed_t     ptryy;
    player_t*   player;
    fixed_t     xmove;
    fixed_t     ymove;
    fixed_t     oldx, oldy;

	if ( // Remove CTF flag if in death pit
		(mo->type == MT_REDFLAG || mo->type == MT_BLUEFLAG) 
		&& (mo->subsector->sector->special == 16 || mo->subsector->sector->special == 5) // Replace this with a single general Death Pit special Nozomi
		&& mo->z == mo->floorz
	)
		mo->fuse = 1;

    if (!mo->momx && !mo->momy)
    {
        if (mo->flags2 & MF2_SKULLFLY)
        {
            mo->flags2 &= ~MF2_SKULLFLY;
            mo->momx = mo->momy = mo->momz = 0;

			if(mo->type != MT_EGGMOBILE)
				P_SetMobjState (mo, mo->info->spawnstate);
        }
        return;
    }

    player = mo->player;

    if (mo->momx > MAXMOVE)
        mo->momx = MAXMOVE;
    else if (mo->momx < -MAXMOVE)
        mo->momx = -MAXMOVE;

    if (mo->momy > MAXMOVE)
        mo->momy = MAXMOVE;
    else if (mo->momy < -MAXMOVE)
        mo->momy = -MAXMOVE;

    xmove = mo->momx;
    ymove = mo->momy;

    oldx = mo->x;
    oldy = mo->y;

    do
    {
		if (xmove > MAXMOVE/2 || ymove > MAXMOVE/2)
		{
			ptryx = mo->x + xmove/2;
			ptryy = mo->y + ymove/2;
			xmove >>= 1;
			ymove >>= 1;
		}
		else
		{
			ptryx = mo->x + xmove;
			ptryy = mo->y + ymove;
			xmove = ymove = 0;
		}

		if (!P_TryMove (mo, ptryx, ptryy, true))
		{
			if ( // THIS NEEDS TO BE REPLACED WITH A MOBJFLAG ASAP Nozomi
				mo->player 
				|| mo->type==MT_CHASECAM 
				|| mo->type == MT_JETTBOMBER 
				|| mo->type == MT_JETTGUNNER 
				|| mo->type == MT_MISC2 
				|| mo->type == MT_FLINGRING 
				|| mo->type == MT_GARGOYLE
			) {
				P_SlideMove (mo);
			}
			else if (mo->flags & MF_MISSILE)
			{
				// Explode Missile objects.

				// Check to prevent exploding missiles hitting the sky boundaries.
				if (ceilingline &&
					(
						(
							(
								(ceilingline->backsector && ceilingline->backsector->ceilingpic == skyflatnum) 
								|| (ceilingline->frontsector && ceilingline->frontsector->ceilingpic == skyflatnum)
							)
							&& mo->subsector->sector->ceilingheight <= mo->ceilingz
						) || ( // floor support since it didn't exist before or something Nozomi
							(
								(ceilingline->backsector && ceilingline->backsector->floorpic == skyflatnum) 
								|| (ceilingline->frontsector && ceilingline->frontsector->floorpic == skyflatnum)
							)
							&& mo->subsector->sector->floorheight >= mo->floorz
						)
					)

				)
					if (
							(
								ceilingline->backsector
								&& (
									mo->z > ceilingline->backsector->ceilingheight 
									|| mo->z < ceilingline->backsector->floorheight
								)
							) || (
								ceilingline->frontsector
								&& (
									mo->z > ceilingline->frontsector->ceilingheight 
									|| mo->z < ceilingline->frontsector->floorheight
								)
							)
					) {
						// This mobj hit a sky boundary, so lets just remove it!
						P_RemoveMobj (mo);
						return;
					}

				// Otherwise explode!
				P_ExplodeMissile (mo);
			}
			else
				mo->momx = mo->momy = 0;
		}
		else
			// Walk over small walls, we should remove cheat flags and make this a mobjflag maybe... Nozomi
			if (mo->player)
				mo->player->cheats &= ~CF_JUMPOVER;
    } while (xmove || ymove);

    // Player friction, now in one general area...? Nozomi
    if (player)
	{
		// If we're homing, we have NO FRICTION.
		if (player->homing)
			return;

		// Ground friction.
		if (mo->friction != ORIG_FRICTION && mo->z <= mo->subsector->sector->floorheight)
			P_XYFriction (mo, oldx, oldy, false);

		// Rolling friction.
		if (
			player->mfspinning
			&& (player->rmomx || player->rmomy) 
			&& !player->mfstartdash
			) {
			mo->momx = FixedMul (mo->momx, FRICTION * 1.1); // Why 1.1, and not 1.1f?
			mo->momy = FixedMul (mo->momy, FRICTION * 1.1); // I won't question it honestly. Nozomi
			return;
		}
	}

	// Tails why...
	// Also I'm removing all the old XMAS support for new stuff.
	// Anyways, no friction for missiles or objects with SKULLFLY (like Eggman) Nozomi
    if ((mo->flags & MF_MISSILE || mo->flags2 & MF2_SKULLFLY) && !mo->type == MT_DETON)
        return;

	// ORIGINAL SSN COMMENT: Ice on a ledge Tails 11-29-2000
	// I'll let the above do the explaining... Nozomi

	// This is for FOFs with Ice friction. Nozomi
	if (mo->subsector->sector->ffloors)
	{
		ffloor_t* rover;
		for(rover = mo->subsector->sector->ffloors; rover; rover = rover->next)
		{
			if(mo->z == *rover->topheight && !mo->momz && *rover->special == 256) // Remind me to make constants for all the specials. Nozomi
			{ // I think this is supposed to keep you moving at the same speed? Odd. Nozomi
				mo->momx = FixedMul (mo->momx, FRICTION*1.1);
				mo->momy = FixedMul (mo->momy, FRICTION*1.1);
				return;
			}
		}
	}

	// No airborne friction...
	// Unless you're a Crawla Commander. Why??? Nozomi
	if (mo->z > mo->floorz && mo->type != MT_CRAWLACOMMANDER)
		return;

	// Corpses...???? We don't need that. Nozomi
	// Yeah, so like there used to be code for corpses here. Nozomi

	// Apply our friction. Nozomi
	P_XYFriction (mo, oldx, oldy, true);
}

//
// P_ZMovement
//
void P_ZMovement (mobj_t* mo)
{
    fixed_t     dist;
    fixed_t     delta;

    // check for smooth step up
#ifdef CLIENTPREDICTION2
    if (mo->player && mo->z < mo->floorz && mo->type!=MT_PLAYER)
#else
    if (mo->player && mo->z < mo->floorz && mo->type!=MT_SPIRIT)
#endif
    {
        mo->player->viewheight -= mo->floorz - mo->z;

        mo->player->deltaviewheight
            = ((cv_viewheight.value<<FRACBITS) - mo->player->viewheight)>>3;
    }

	// IDK what the above does but it scares me. Nozomi

	// OMG YOU DID IT TAILS YOU MADE SOMETHING SIMPLE THAT JUST WORKS!! Nozomi
	// Snowflake Tails 12-02-2001
	if (mo->type == MT_SNOWFLAKE)
	{
		if(mo->z + mo->momz <= mo->floorz || mo->z < mo->waterz)
			P_RemoveMobj(mo);
		else
			mo->momz = -2*FRACUNIT; // Why hardcode the momentum though Tails? Nozomi
	}

	// Add Z momentum to our Z coordinate. Nozomi
    mo->z += mo->momz;

	// Ignore still rings Tails 09-02-2001
	if(mo->type == MT_MISC2 && !(mo->momx || mo->momy || mo->momz))
		return;

	// Have player fall through floor? 10-05-2001 Tails
	if(mo->player && mo->player->playerstate == PST_DEAD)
		goto playergravity;

    if ( mo->flags & MF_FLOAT
         && mo->target && mo->health && !(mo->type == MT_DETON || mo->type == MT_JETTBOMBER || mo->type == MT_JETTGUNNER || mo->type == MT_CRAWLACOMMANDER)) // Tails 07-21-2001
    {
        // float down towards target if too close
        if ( !(mo->flags2 & MF2_SKULLFLY)
             && !(mo->flags & MF_INFLOAT) )
        {
            dist = P_AproxDistance (mo->x - mo->target->x,
                                    mo->y - mo->target->y);

            delta =(mo->target->z + (mo->height>>1)) - mo->z;

            if (delta<0 && dist < -(delta*3) )
                mo->z -= FLOATSPEED;
            else if (delta>0 && dist < (delta*3) )
                mo->z += FLOATSPEED;
        }

    }

    // clip movement

    if (mo->z <= mo->floorz)
    {
        // hit the floor

        // Note (id):
        //  somebody left this after the setting momz to 0,
        //  kinda useless there.
        if (mo->flags2 & MF2_SKULLFLY)
			mo->momz = -mo->momz;

		// Mine explodes upon ground contact Tails 06-13-2000
		if((mo->type==MT_MINE) && (mo->z <= mo->floorz) && !(mo->state == &states[S_MINE_BOOM1]
		   || mo->state == &states[S_MINE_BOOM2] || mo->state == &states[S_MINE_BOOM3]
		   || mo->state == &states[S_MINE_BOOM4] || mo->state == &states[S_DISS]))
			P_ExplodeMissile(mo);

        if (mo->momz < 0) // falling
        {
            if (mo->player && (mo->momz < -8*FRACUNIT))
                mo->player->deltaviewheight = mo->momz>>3;

			if(tmfloorthing)
			{
				// Bouncing boxes Tails 09-28-2001
				if(tmfloorthing->z > tmfloorthing->floorz)
				{
					switch(mo->type)
					{
						case MT_GARGOYLE: // Deep Sea Gargoyle
						case MT_MISC50: // Blue shield box
						case MT_MISC48: // Yellow shield box
						case MT_MISC31: // Green shield box
						case MT_BKTV: // Black shield box
						case MT_MISC74: // Super Sneaker box
						case MT_PRUP: // 1-Up box
						case MT_MISC10: // 10-Ring box
						case MT_MISC11: // 25-Ring box
						case MT_INV: // Invincibility box
							mo->momz = 4*FRACUNIT;
							break;
						default:
							break;
					}
				}

				switch(tmfloorthing->type)
				{
					case MT_GARGOYLE: // Deep Sea Gargoyle
					case MT_MISC50: // Blue shield box
					case MT_MISC48: // Yellow shield box
					case MT_MISC31: // Green shield box
					case MT_BKTV: // Black shield box
					case MT_MISC74: // Super Sneaker box
					case MT_PRUP: // 1-Up box
					case MT_MISC10: // 10-Ring box
					case MT_MISC11: // 25-Ring box
					case MT_INV: // Invincibility box
						if(mo->player)
							if(!(mo->player->mfjumped))
								tmfloorthing = 0;
						break;
					default:
						break;
				}
			}

            if ((mo->z <= mo->floorz) && !(tmfloorthing))
            {
				mo->eflags |= MF_JUSTHITFLOOR;

				if(mo->player)
				{
					mo->player->scoreadd = 0;
					mo->player->mfjumped = 0;
					mo->player->gliding = 0;
					mo->player->glidetime = 0;
					mo->player->climbing = 0;
				}
			}

			if(mo->player && mo->player->mfspinning == 0)
				mo->player->mfstartdash = 0;

			//SOM: Flingrings bounce
			if(mo->type == MT_FLINGRING)
				mo->momz = -mo->momz * 0.85;
			else if (!(tmfloorthing) || (tmfloorthing->type == MT_GARGOYLE || tmfloorthing->type == MT_PLAYER))
				mo->momz = 0;
        }

		if(mo->type == MT_STEAM)
			return;

        mo->z = mo->floorz;


        if ( (mo->flags & MF_MISSILE)
             && !(mo->flags & MF_NOCLIP) )
        {
            P_ExplodeMissile (mo);
            return;
        }
    }
    else if (! (mo->flags & MF_NOGRAVITY) )             // Gravity here!
    {
        fixed_t     gravityadd;
        
        //Fab: NOT SURE WHETHER IT IS USEFUL, just put it here too
        //     TO BE SURE there is no problem for the release..
        //     (this is done in P_Mobjthinker below normally)
        mo->eflags &= ~MF_JUSTHITFLOOR;

        gravityadd = -cv_gravity.value;

		if(mo->eflags & MF_UNDERWATER) // Tails
			gravityadd = -cv_gravity.value/3; // Tails

		if (mo->momz==0)
            // mobj at stop, no floor, so feel the push of gravity!
            gravityadd <<= 1;

playergravity:
		if(mo->player)
		{
			if ((mo->player->charability==1) && ((mo->player->powers[pw_tailsfly]) || (mo->player->mo->state == &states[S_PLAY_SPC1]) || (mo->player->mo->state == &states[S_PLAY_SPC2]) || (mo->player->mo->state == &states[S_PLAY_SPC3]) || (mo->player->mo->state == &states[S_PLAY_SPC4])))
				gravityadd = -cv_gravity.value/3; // less gravity while flying
			if(mo->player->gliding)
				gravityadd = -cv_gravity.value/3; // less gravity while gliding
			if(mo->player->climbing)
				gravityadd = 0;

			if(mo->player->playerstate == PST_DEAD) // Added crash check Tails 11-16-2001)
			{
				gravityadd = -cv_gravity.value;
				mo->momz += gravityadd;
				return;
			}
		}

		mo->momz += gravityadd;
    }

    if (mo->z + mo->height > mo->ceilingz)
    {
        mo->z = mo->ceilingz - mo->height;

        // hit the ceiling
        if (mo->momz > 0)
			mo->momz = 0;

        if (mo->flags2 & MF2_SKULLFLY)
        {       // the skull slammed into something
            mo->momz = -mo->momz;
        }

        if ( (mo->flags & MF_MISSILE)
             && !(mo->flags & MF_NOCLIP) )
        {
            //SoM: 4/3/2000: Don't explode on the sky!
            if(demoversion >= 129 && mo->subsector->sector->ceilingpic == skyflatnum &&
               mo->subsector->sector->ceilingheight == mo->ceilingz)
            {
              P_RemoveMobj(mo);
              return;
            }

            P_ExplodeMissile (mo);
            return;
        }
    }
}



//
// P_NightmareRespawn
//
void
P_NightmareRespawn (mobj_t* mobj)
{
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;
    mobj_t*             mo;
    mapthing_t*         mthing;

    x = mobj->spawnpoint->x << FRACBITS;
    y = mobj->spawnpoint->y << FRACBITS;

    // somthing is occupying it's position?
    if (!P_CheckPosition (mobj, x, y) )
        return; // no respwan

    // No more teleport fog. Nozomi

    // spawn the new monster
    mthing = mobj->spawnpoint;

    // spawn it
    if (mobj->info->flags & MF_SPAWNCEILING)
        z = ONCEILINGZ;
    else
        z = ONFLOORZ;

    // inherit attributes from deceased one
    mo = P_SpawnMobj (x,y,z, mobj->type);
    mo->spawnpoint = mobj->spawnpoint;
    mo->angle = ANG45 * (mthing->angle/45);

    if (mthing->options & MTF_AMBUSH)
        mo->flags |= MF_AMBUSH;

    mo->reactiontime = 18;

    // remove the old monster,
    P_RemoveMobj (mobj);
}


consvar_t cv_respawnmonsters = {"respawnmonsters","0",CV_NETVAR,CV_OnOff};
consvar_t cv_respawnmonsterstime = {"respawnmonsterstime","12",CV_NETVAR,CV_Unsigned};


//
// P_MobjCheckWater : check for water, set stuff in mobj_t struct for
//                    movement code later, this is called either by P_MobjThinker() or P_PlayerThink()
void P_MobjCheckWater (mobj_t* mobj)
{
    sector_t* sector;
    int       oldeflags;
	int z;

    if( demoversion<128 || mobj->type==MT_SPLASH) // splash don't do splash
        return;
    //
    // see if we are in water, and set some flags for later
    //
    sector = mobj->subsector->sector;
    oldeflags = mobj->eflags;
	mobj->waterz = mobj->floorz - 10000*FRACUNIT;

	if ((sector->heightsec > -1 && sector->altheightsec == 1) ||
        (levelflats[sector->floorpic].iswater && sector->heightsec == -1))
    {
        if (sector->heightsec > -1)  //water hack
            z = (sectors[sector->heightsec].floorheight);
        else
            z = sector->floorheight + (FRACUNIT/4); // water texture

        if (z && mobj->z+(mobj->height>>1) <= z) // Added crash check Tails 11-16-2001
        { // Tails 03-06-2000
            mobj->eflags |= MF_UNDERWATER;
			if(mobj->player)
			{
         if(!((mobj->player->powers[pw_super]) || (mobj->player->powers[pw_invulnerability])))
            mobj->player->powers[pw_yellowshield] = false;
        if (mobj->player->powers[pw_underwater] <= 0 && !(mobj->player->powers[pw_greenshield])) // Tails 03-06-2000
            {// Tails 03-06-2000
            mobj->player->powers[pw_underwater] = 30*TICRATE + 1; // Tails 03-06-2000
            }// Tails 03-06-2000
			}
		}
        else
         {
            mobj->eflags &= ~MF_UNDERWATER;
          } // Tails 03-06-2000 (I guess I'm just comment-happy today!)

    } else if(sector->ffloors) {
      ffloor_t*  rover;

      mobj->eflags &= ~(MF_UNDERWATER|MF_TOUCHWATER);

      for(rover = sector->ffloors; rover; rover = rover->next)
      {
        if(!(rover->flags & FF_SWIMMABLE) || rover->flags & FF_SOLID)
          continue;

		if (*rover->topheight <= mobj->z)
			mobj->waterz = *rover->topheight;

        if(*rover->topheight < mobj->z || *rover->bottomheight > (mobj->z + (mobj->height / 2)))
          continue;

		mobj->waterz = *rover->topheight;

        if(mobj->z + mobj->height > *rover->topheight)
            mobj->eflags |= MF_TOUCHWATER;
        else
            mobj->eflags &= ~MF_TOUCHWATER;

        if(mobj->z + mobj->height < *rover->topheight)
		{ // Tails
            mobj->eflags |= MF_UNDERWATER;

			if(mobj->player) {
				if(!((mobj->player->powers[pw_super]) || (mobj->player->powers[pw_invulnerability])))
					mobj->player->powers[pw_yellowshield] = false;
				if (mobj->player->powers[pw_underwater] <= 0 && !(mobj->player->powers[pw_greenshield])) // Tails 03-06-2000
					mobj->player->powers[pw_underwater] = 30*TICRATE + 1; // Tails 03-06-2000
			}
		} // Tails
        else
            mobj->eflags &= ~MF_UNDERWATER;
      }
      return;
    }
    else
        mobj->eflags &= ~(MF_UNDERWATER|MF_TOUCHWATER);

	if(mobj->subsector->sector->heightsec != -1 && mobj->subsector->sector->altheightsec == 1)
		mobj->waterz = sectors[mobj->subsector->sector->heightsec].floorheight;
}

//
// P_MobjThinker
//
void P_MobjThinker (mobj_t* mobj)
{
    boolean   checkedpos = false;  //added:22-02-98:

    // check mobj against possible water content, before movement code
    P_MobjCheckWater (mobj);

// Start Level end sign stuff Tails 01-14-2001

	if (mobj->type == MT_SIGN && plyr->exiting)
		if (mobj->state == &states[S_SIGN49])
		{
			P_SetMobjState (mobj, S_SIGN1);
			S_StartSound(mobj, sfx_lvpass);
		}

// End Level end sign stuff Tails 01-14-2001

// Fans spawn bubbles underwater Tails 02-28-2001
// ONLY with MF_AMBUSH! Nozomi 03-13-2026
	if((mobj->type == MT_MISC34 || mobj->type == MT_REDFAN) && mobj->flags & MF_AMBUSH)
	{
		int dist = 0;

		if (!P_LookForPlayers(mobj, true))
			return;

		dist = R_PointToDist2(mobj->x, mobj->y, mobj->target->x, mobj->target->y);

		if (dist > 1024*FRACUNIT) {
			mobj->target = NULL;
			return;
		}

		if (mobj->z + mobj->height < mobj->waterz) {
			if(!(P_Random() % 16))
			{
				P_SpawnMobj (mobj->x, mobj->y, mobj->z + (mobj->height / 1.25), MT_SMALLBUBBLE);
			}
			if(!(P_Random() % 96))
			{
				P_SpawnMobj (mobj->x, mobj->y, mobj->z + (mobj->height / 1.25), MT_MEDIUMBUBBLE);
			}
		}
		else if (!(leveltime % (4 + (((byte)dist) % 7)))) // air particles!!! Nozomi 03-13-2026
		{
			if(!(P_Random() % 16))
			{
				if (mobj->type == MT_REDFAN)
					P_SpawnMobj (mobj->x, mobj->y, mobj->z + (mobj->height / 1.25), MT_AIRPARTICLE3);
				else
					P_SpawnMobj (mobj->x, mobj->y, mobj->z + (mobj->height / 1.25), MT_AIRPARTICLE);
			}
			if(!(P_Random() % 32))
			{
				if (mobj->type == MT_REDFAN)
					P_SpawnMobj (mobj->x, mobj->y, mobj->z + (mobj->height / 1.25), MT_AIRPARTICLE4);
				else
					P_SpawnMobj (mobj->x, mobj->y, mobj->z + (mobj->height / 1.25), MT_AIRPARTICLE2);
			}
		}
	}

    if(mobj->player)
	{
		if(mobj->eflags & MF_JUSTHITFLOOR && mobj->z<=mobj->floorz && mobj->health)
		{
			// This is fucking stupid, but let it exist. It has a purpose... and I'll let it use the runspeed var I made. :3 Nozomi
			if(mobj->player->cmomx || mobj->player->cmomy)
			{
				if(mobj->player->speed > mobj->player->runspeed && !mobj->player->running)
					P_SetMobjState (mobj, S_PLAY_SPD1);
				else if ((mobj->player->rmomx > STOPSPEED || mobj->player->rmomy > STOPSPEED) && (mobj->player->cmomx || mobj->player->cmomy) && !mobj->player->walking)
					P_SetMobjState (mobj, S_PLAY_RUN1);
				else if ((mobj->momx > STOPSPEED || mobj->momy > STOPSPEED) && !mobj->player->walking)
					P_SetMobjState (mobj, S_PLAY_RUN1);
				else if ((mobj->player->rmomx < FRACUNIT || mobj->player->rmomy < FRACUNIT) && (mobj->player->cmomx || mobj->player->cmomy) && !(mobj->player->walking || mobj->player->running))
					P_SetMobjState (mobj, S_PLAY);
			}
			else
			{
				if(mobj->player->speed > mobj->player->runspeed && !mobj->player->running)
					P_SetMobjState (mobj, S_PLAY_SPD1);
				else if ((mobj->momx || mobj->momy) && !mobj->player->walking)
					P_SetMobjState (mobj, S_PLAY_RUN1);
				else if (!(mobj->momx && mobj->momy) && !(mobj->player->walking || mobj->player->running))
					P_SetMobjState (mobj, S_PLAY);
			}

			mobj->player->mfjumped = 0;
			mobj->player->mfspinning = 0;
			mobj->player->gliding = 0;
			mobj->player->glidetime = 0;
			mobj->player->climbing = 0;
		}
	}
	
	//SOM: Check fuse
	if(mobj->fuse) {
		mobj->fuse--;

		if(!mobj->fuse) {

			subsector_t* ss;
			fixed_t             x;
			fixed_t             y;
			fixed_t             z;
			mobj_t*			flagmo;

			if(mobj->type == MT_BLUEFLAG)
			{
				x = mobj->spawnpoint->x << FRACBITS;
				y = mobj->spawnpoint->y << FRACBITS;
				ss = R_PointInSubsector(x, y);
				z = ss->sector->floorheight;
				flagmo = P_SpawnMobj(x, y, z, MT_BLUEFLAG);
				flagmo->spawnpoint = mobj->spawnpoint;
			}
			else if(mobj->type == MT_REDFLAG)
			{
				x = mobj->spawnpoint->x << FRACBITS;
				y = mobj->spawnpoint->y << FRACBITS;
				ss = R_PointInSubsector(x, y);
				z = ss->sector->floorheight;
				flagmo = P_SpawnMobj(x, y, z, MT_REDFLAG);
				flagmo->spawnpoint = mobj->spawnpoint;
			}

			switch(mobj->type)
			{
					case MT_MISC50: // Blue shield box
					case MT_MISC48: // Yellow shield box
					case MT_MISC31: // Green shield box
					case MT_BKTV: // Black shield box
					case MT_MISC74: // Super Sneaker box
					case MT_PRUP: // 1-Up box
					case MT_MISC10: // 10-Ring box
					case MT_MISC11: // 25-Ring box
					case MT_INV: // Invincibility box
						P_SetMobjState(mobj, S_DISS); // make sure they dissapear tails
						break;
					default:
						if(mobj->info->deathstate)
							P_ExplodeMissile(mobj);
						else
							P_SetMobjState(mobj, S_DISS); // make sure they dissapear tails
						break;
			}
		}
	}

    //
    // momentum movement
    //
    if ( mobj->momx ||
         mobj->momy ||
        (mobj->flags2&MF2_SKULLFLY) )
    {
        P_XYMovement (mobj);
        checkedpos = true;

        // FIXME: decent NOP/NULL/Nil function pointer please.
        if ((mobj->thinker.function.acv == (actionf_v) (-1)))
            return;             // mobj was removed
    }

    //added:28-02-98: always do the gravity bit now, that's simpler
    //                BUT CheckPosition only if wasn't do before.
    if ( !( (mobj->eflags & MF_ONGROUND) &&
            (mobj->z == mobj->floorz) &&
            !mobj->momz
          ) )
    {
        // if didnt check things Z while XYMovement, do the necessary now
        if (!checkedpos)
        {
            P_CheckPosition (mobj, mobj->x, mobj->y);

            mobj->floorz = tmfloorz;
            mobj->ceilingz = tmceilingz;

            if (tmfloorthing)
                mobj->eflags &= ~MF_ONGROUND;  //not on real floor
            else
                mobj->eflags |= MF_ONGROUND;
        }

        P_ZMovement (mobj);

        // FIXME: decent NOP/NULL/Nil function pointer please.
        if (mobj->thinker.function.acv == (actionf_v) (-1))
            return;             // mobj was removed
    }
    else
        mobj->eflags &= ~MF_JUSTHITFLOOR;

	if (!mobj->player)
		mobj->eflags &= ~MF_SPRUNG;

    // cycle through states,
    // calling action functions at transitions
    if (mobj->tics != -1)
    {
        mobj->tics--;

        // you can cycle through multiple states in a tic
        if (!mobj->tics)
            if (!P_SetMobjState (mobj, mobj->state->nextstate) )
                return;         // freed itself
    }
    else
    {
        if (!cv_respawnmonsters.value)
            return;

        // check for nightmare respawn
        if (! (mobj->flags & MF_COUNTKILL) )
            return;

        mobj->movecount++;

        if (mobj->movecount < cv_respawnmonsterstime.value*TICRATE)
            return;

        if ( leveltime&31 )
            return;

        if (P_Random () > 4)
            return;

        P_NightmareRespawn (mobj);
    }

	mobj->eflags &= ~MF_SPRUNG;

	if(mobj->type == MT_EGGMOBILE && mobj->health < 3 && leveltime & 1 && mobj->health > 0)
		P_SpawnMobj(mobj->x, mobj->y, mobj->z, MT_SMOK);

	if(mobj->type == MT_EGGMOBILE && mobj->flags2 & MF2_SKULLFLY)
	{
		mobj_t* thok;
		thok = P_SpawnMobj(mobj->x, mobj->y, mobj->z, MT_THOK);
		thok->color = SKINCOLOR_GREY;
	}

	// Some black shield code Tails 04-08-2000
	if (mobj->type==MT_BFG)
		P_SetMobjState (mobj, S_BFGLAND3);

	if (mobj->state == &states[S_BFGLAND3])
		P_SetMobjState (mobj, S_DISS);

	// start bubble dissipate Tails
	if((mobj->type==MT_SMALLBUBBLE || mobj->type==MT_MEDIUMBUBBLE || mobj->type==MT_EXTRALARGEBUBBLE) && (mobj->z >= mobj->waterz || mobj->z + mobj->height >= mobj->ceilingz))
	{
		P_SetMobjState (mobj, S_DISS);
	}
	// end bubble dissipate Tails

	// air particle dissipate Nozomi
	if((mobj->type==MT_AIRPARTICLE || mobj->type==MT_AIRPARTICLE2 || mobj->type==MT_AIRPARTICLE3 || mobj->type==MT_AIRPARTICLE4) && (mobj->z + mobj->height >= mobj->ceilingz))
	{
		P_SetMobjState (mobj, S_DISS);
	}
	// air particle dissipate Nozomi

	// start make sure player shows dead Tails 03-15-2000
	if(mobj->player)
	{
	   if(mobj->health <= 0)
	   {
		 P_SetMobjState (mobj, S_PLAY_DIE3);
	   }
	}
	// end make sure player shows dead Tails 03-15-2000

	// Keep Skim at water surface Tails 06-13-2000
	if((mobj->type==MT_SKIM) && ((mobj->z > mobj->waterz) || (mobj->z < mobj->waterz)))
		mobj->z = mobj->waterz;
}

static void CalculatePrecipFloor(precipmobj_t* mobj)
{
	// recalculate floorz each time
	mobj->floorz = mobj->subsector->sector->floorheight;
	if(mobj->subsector->sector->ffloors)
	{
		ffloor_t* rover;

		for(rover = mobj->subsector->sector->ffloors; rover; rover = rover->next)
		{
			// If it exists, it'll get rained on.
			if(!(rover->flags & FF_EXISTS))
				continue;

			if(*rover->topheight > mobj->floorz)
				mobj->floorz = *rover->topheight;
		}
	}
}

void P_RecalcPrecipInSector(sector_t* sector)
{
	/// \todo Why doesn't this work?!
/*	precipmobj_t* precipthing;

	for(precipthing = sector->preciplist; precipthing; precipthing = precipthing->snext)
	{
		CalculatePrecipFloor(precipthing);
	}*/
	sector = NULL; // warning C4100: 'sector' : unreferenced formal parameter
}

void P_SnowThinker(precipmobj_t* mobj)
{
	// adjust height
	mobj->z += mobj->momz;

	if(mobj->z <= mobj->floorz)
		mobj->z = mobj->subsector->sector->ceilingheight;

	return;
}

void P_RainThinker(precipmobj_t* mobj)
{
	// adjust height
	mobj->z += mobj->momz;

	if(mobj->state != &states[S_RAIN1])
	{
		// cycle through states,
		// calling action functions at transitions
		if(mobj->tics != -1)
		{
			mobj->tics--;

			// you can cycle through multiple states in a tic
			if(!mobj->tics)
				if(!P_SetPrecipMobjState(mobj, mobj->state->nextstate))
					return; // freed itself
		}

		if(mobj->state == &states[S_RAINRETURN])
		{
			mobj->z = mobj->subsector->sector->ceilingheight;
			mobj->momz = mobjinfo[MT_RAIN].speed;
			P_SetPrecipMobjState(mobj, S_RAIN1);
		}
	}
	else if(mobj->z <= mobj->floorz && mobj->momz)
	{
		// no splashes on sky or bottomless pits
		if(mobj->z <= mobj->subsector->sector->floorheight
			&& (mobj->subsector->sector->special == 5 || mobj->subsector->sector->special == 16
			|| mobj->subsector->sector->floorpic == skyflatnum))
			mobj->z = mobj->subsector->sector->ceilingheight;
		else
		{
			mobj->momz = 0;
			mobj->z = mobj->floorz;
			P_SetPrecipMobjState(mobj, S_SPLASH1);
		}
	}

	return;
}

void P_MobjNullThinker (mobj_t* mobj)
{}

//
// P_SpawnMobj
//
mobj_t* P_SpawnMobj ( fixed_t       x,
                      fixed_t       y,
                      fixed_t       z,
                      mobjtype_t    type )
{
    mobj_t*     mobj;
    state_t*    st;
    mobjinfo_t* info;

    mobj = Z_Malloc (sizeof(*mobj), PU_LEVEL, NULL);
    memset (mobj, 0, sizeof (*mobj));
    info = &mobjinfo[type];

    mobj->type = type;
    mobj->info = info;
    mobj->x = x;
    mobj->y = y;
    mobj->radius = info->radius;
    mobj->height = info->height;
    mobj->flags = info->flags;
	mobj->flags2 = info->flags2;
    mobj->health = info->spawnhealth;

    if (gameskill != sk_nightmare)
        mobj->reactiontime = info->reactiontime;

    // added 4-9-98: dont get out of synch
    if (mobj->type == MT_SPIRIT || mobj->type == MT_CHASECAM)
        mobj->lastlook = 0;
    else
        if( demoversion<129 )
            mobj->lastlook = P_Random () % MAXPLAYERS;
        else
            mobj->lastlook = -1;  // stuff moved in P_enemy.P_LookForPlayer

    // do not set the state with P_SetMobjState,
    // because action routines can not be called yet
    st = &states[info->spawnstate];

    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame; // FF_FRAMEMASK for frame, and other bits..
    mobj->touching_sectorlist = NULL; //SoM: 4/7/2000
    mobj->friction = ORIG_FRICTION; //SoM: 4/7/2000

    // BP: SoM right ? if not ajust in p_saveg line 625 and 979
    mobj->movefactor = ORIG_FRICTION_FACTOR;

    // set subsector and/or block links
    P_SetThingPosition (mobj);

    mobj->floorz = mobj->subsector->sector->floorheight;
    mobj->ceilingz = mobj->subsector->sector->ceilingheight;

    //added:27-02-98: if ONFLOORZ, stack the things one on another
    //                so they do not occupy the same 3d space
    //                allow for some funny thing arrangements!
    if (z == ONFLOORZ)
    {
        //if (!P_CheckPosition(mobj,x,y))
            // we could send a message to the console here, saying
            // "no place for spawned thing"...

        //added:28-02-98: defaults onground
        mobj->eflags |= MF_ONGROUND;

		if((mobj->type == MT_MISC2 && (mobj->flags & MF_AMBUSH)) || mobj->type == MT_DETON || mobj->type == MT_JETTBOMBER || mobj->type == MT_JETTGUNNER) // Special flag for rings Tails 06-03-2001
			mobj->z = mobj->floorz + 32*FRACUNIT;
		else
            mobj->z = mobj->floorz;

    }
    else if (z == ONCEILINGZ)
        mobj->z = mobj->ceilingz - mobj->height;
    else
    {
        //CONS_Printf("mobj spawned at z %d\n",z>>16);
        mobj->z = z;
    }

    // added 16-6-98: special hack for spirit
    if(mobj->type == MT_SPIRIT)
        mobj->thinker.function.acv = (actionf_p1)P_MobjNullThinker;
    else
    {
        mobj->thinker.function.acp1 = (actionf_p1)P_MobjThinker;
        P_AddThinker (&mobj->thinker);
    }

    //SOM: Fuse for bunnies, squirls, and flingrings
      if(mobj->type == MT_BIRD || mobj->type == MT_SQRL || mobj->type == MT_MOUSE)
        mobj->fuse = 300 + (P_Random() % 50);

    return mobj;
}

static inline precipmobj_t* P_SpawnRainMobj(fixed_t x, fixed_t y, fixed_t z, mobjtype_t type)
{
	precipmobj_t* mobj;
	state_t* st;

	mobj = Z_Malloc(sizeof(*mobj), PU_LEVEL, NULL);
	memset(mobj, 0, sizeof(*mobj));

	mobj->x = x;
	mobj->y = y;
	mobj->flags = mobjinfo[type].flags;

	// do not set the state with P_SetMobjState,
	// because action routines can not be called yet
	st = &states[mobjinfo[type].spawnstate];

	mobj->state = st;
	mobj->tics = st->tics;
	mobj->sprite = st->sprite;
	mobj->frame = st->frame; // FF_FRAMEMASK for frame, and other bits..
	mobj->touching_sectorlist = NULL;

	// set subsector and/or block links
	P_SetPrecipitationThingPosition(mobj);

	mobj->floorz = mobj->subsector->sector->floorheight;

	mobj->z = z;
	mobj->momz = mobjinfo[type].speed;

	mobj->thinker.function.acp1 = (actionf_p1)P_RainThinker;
	P_AddThinker(&mobj->thinker);

	CalculatePrecipFloor(mobj);

	return mobj;
}

static precipmobj_t* P_SpawnSnowMobj(fixed_t x, fixed_t y, fixed_t z, mobjtype_t type)
{
	precipmobj_t* mobj;
	state_t* st;

	mobj = Z_Malloc(sizeof(*mobj), PU_LEVEL, NULL);
	memset(mobj, 0, sizeof(*mobj));

	mobj->x = x;
	mobj->y = y;
	mobj->flags = mobjinfo[type].flags;

	// do not set the state with P_SetMobjState,
	// because action routines can not be called yet
	st = &states[mobjinfo[type].spawnstate];

	mobj->state = st;
	mobj->tics = st->tics;
	mobj->sprite = st->sprite;
	mobj->frame = st->frame; // FF_FRAMEMASK for frame, and other bits..
	mobj->touching_sectorlist = NULL;

	// set subsector and/or block links
	P_SetPrecipitationThingPosition(mobj);

	mobj->floorz = mobj->subsector->sector->floorheight;

	mobj->z = z;
	mobj->momz = mobjinfo[type].speed;

	mobj->thinker.function.acp1 = (actionf_p1)P_SnowThinker;
	P_AddThinker(&mobj->thinker);

	CalculatePrecipFloor(mobj);

	return mobj;
}

//
// P_RemoveMobj
//
mapthing_t     *itemrespawnque[ITEMQUESIZE];
int             itemrespawntime[ITEMQUESIZE];
int             iquehead;
int             iquetail;

void P_RemoveMobj (mobj_t* mobj)
{
	int random; // Tails 08-09-2001

		switch(mobj->type)
		{
				case MT_MISC50: // Blue shield box
				case MT_MISC48: // Yellow shield box
				case MT_MISC31: // Green shield box
				case MT_BKTV: // Black shield box
				case MT_MISC74: // Super Sneaker box
				case MT_PRUP: // 1-Up box
				case MT_MISC10: // 10-Ring box
				case MT_MISC11: // 25-Ring box
				case MT_INV: // Invincibility box
					random = P_Random() / 32;
					if(random == 0)
						P_SpawnMobj(mobj->x, mobj->y, mobj->z, MT_MISC50);
					else if(random == 1)
						P_SpawnMobj(mobj->x, mobj->y, mobj->z, MT_MISC48);
					else if(random == 2)
						P_SpawnMobj(mobj->x, mobj->y, mobj->z, MT_MISC31);
					else if(random == 3)
						P_SpawnMobj(mobj->x, mobj->y, mobj->z, MT_BKTV);
					else if(random == 4)
						P_SpawnMobj(mobj->x, mobj->y, mobj->z, MT_MISC74);
					else if(random == 5)
						P_SpawnMobj(mobj->x, mobj->y, mobj->z, MT_MISC10);
					else if(random == 6)
						P_SpawnMobj(mobj->x, mobj->y, mobj->z, MT_MISC11);
					else
						P_SpawnMobj(mobj->x, mobj->y, mobj->z, MT_INV);
					break;
				case MT_MISC2:
					itemrespawnque[iquehead] = mobj->spawnpoint;
					itemrespawntime[iquehead] = leveltime;
					iquehead = (iquehead+1)&(ITEMQUESIZE-1);
					// lose one off the end?
					if (iquehead == iquetail)
						iquetail = (iquetail+1)&(ITEMQUESIZE-1);
					break;
				default:
					break;
		}

    // unlink from sector and block lists
    P_UnsetThingPosition (mobj);

    //SoM: 4/7/2000: Remove touching_sectorlist from mobj.
    if(sector_list)
    {
      P_DelSeclist(sector_list);
      sector_list = NULL;
    }

    // stop any playing sound
    S_StopSound (mobj);

    // free block
    P_RemoveThinker ((thinker_t*)mobj);
}

void P_RemovePrecipMobj(precipmobj_t* mobj)
{
	// unlink from sector and block lists
	P_UnsetPrecipThingPosition(mobj);

	if(precipsector_list)
	{
		P_DelPrecipSeclist(precipsector_list);
		precipsector_list = NULL;
	}

	// free block
	P_RemoveThinker((thinker_t*)mobj);
}

void P_SpawnPrecipitation(void)
{
	int i;
	fixed_t x, y, height;

	if(cv_snow.value)
	{
		int z;
		subsector_t* snowsector;
		z = 0;

		for(i = 0; i < 1048576 / cv_numsnow.value; i++)
		{
			x = ((rand() * (65536 / (int)RAND_MAX)) - 32768) << FRACBITS;
			y = ((rand() * (65536 / (int)RAND_MAX)) - 32768) << FRACBITS;
			height = ((rand() * (65536 / (int)RAND_MAX)) - 32768) << FRACBITS;

			snowsector = R_IsPointInSubsector(x, y);

			if(!snowsector)
				continue;

			{
				if(snowsector->sector->ceilingpic == skyflatnum &&
					snowsector->sector->floorheight <= snowsector->sector->ceilingheight - 32)
					// don't do it if sector height is less than 32
				{
					while(height < snowsector->sector->floorheight ||
						height >= snowsector->sector->ceilingheight)
						height = ((rand() * (65536 / (int)RAND_MAX)) - 32768) << FRACBITS;

					z = rand() % 256;
					if(z < 64)
						P_SetPrecipMobjState(P_SpawnSnowMobj(x, y, height, MT_SNOWFLAKE), S_SNOW3);
					else if(z < 144)
						P_SetPrecipMobjState(P_SpawnSnowMobj(x, y, height, MT_SNOWFLAKE), S_SNOW2);
					else
						P_SpawnSnowMobj(x, y, height, MT_SNOWFLAKE);
				}
			}
		}
	}
	else if(cv_storm.value || cv_rain.value)
	{

		subsector_t* rainsector;

		for(i = 0; i < 1048576 / cv_raindensity.value; i++)
		{
			x = ((rand() * (65536 / (int)RAND_MAX)) - 32768) << FRACBITS;
			y = ((rand() * (65536 / (int)RAND_MAX)) - 32768) << FRACBITS;
			height = ((rand() * (65536 / (int)RAND_MAX)) - 32768) << FRACBITS;

			rainsector = R_IsPointInSubsector(x, y);

			if(!rainsector)
				continue;

			if(rainsector->sector->ceilingpic == skyflatnum && rainsector->sector->floorheight < rainsector->sector->ceilingheight)
			{
				while(!(height < rainsector->sector->ceilingheight &&
					height > rainsector->sector->floorheight))
					height = ((rand() * (65536 / (int)RAND_MAX)) - 32768) << FRACBITS;

				P_SpawnRainMobj(x, y, height, MT_RAIN);
			}
		}
	}
}

consvar_t cv_itemrespawntime={"respawnitemtime","30",CV_NETVAR,CV_Unsigned};
consvar_t cv_itemrespawn    ={"respawnitem"    , "0",CV_NETVAR,CV_OnOff};
consvar_t cv_flagtime={"flagtime","30",CV_NETVAR,CV_Unsigned}; // Tails 08-03-2001

//
// P_RespawnSpecials
//
void P_RespawnSpecials (void)
{
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;

    subsector_t*        ss;
    mobj_t*             mo;
    mapthing_t*         mthing;

    int                 i;

	// Rain spawning
	if(cv_storm.value || cv_rain.value)
	{
		int volume;

		volume = 255;

		if(players[displayplayer].mo->subsector->sector->ceilingpic == skyflatnum);
		else
		{
			fixed_t x, y, yl, yh, xl, xh;
			fixed_t closex, closey, closedist, newdist, adx, ady;

			// Essentially check in a 1024 unit radius of the player for an outdoor area.
			yl = players[displayplayer].mo->y - 1024*FRACUNIT;
			yh = players[displayplayer].mo->y + 1024*FRACUNIT;
			xl = players[displayplayer].mo->x - 1024*FRACUNIT;
			xh = players[displayplayer].mo->x + 1024*FRACUNIT;
			closex = players[displayplayer].mo->x + 2048*FRACUNIT;
			closey = players[displayplayer].mo->y + 2048*FRACUNIT;
			closedist = 2048*FRACUNIT;
			for(y = yl; y <= yh; y += FRACUNIT*64)
				for(x = xl; x <= xh; x += FRACUNIT*64)
				{
					if(R_PointInSubsector(x, y)->sector->ceilingpic == skyflatnum) // Found the outdoors!
					{
						adx = abs(players[displayplayer].mo->x - x);
						ady = abs(players[displayplayer].mo->y - y);
						newdist = adx + ady - ((adx < ady ? adx : ady)>>1);
						if(newdist < closedist)
						{
							closex = x;
							closey = y;
							closedist = newdist;
						}
					}
				}
			volume = 255 - (closedist>>FRACBITS)/4;
		}
		if(volume < 0)
			volume = 0;
		else if(volume > 255)
			volume = 255;

		if(!leveltime || leveltime % 80 == 1)
			S_StartSoundAtVolume(players[displayplayer].mo, sfx_rainin, volume);

		if(cv_storm.value)
		{
			if(netgame ? (P_Random() < 2) : (M_Random() < 2))
			{
				sector_t* ss;
				int i;
				ss = sectors;

				for(i = 0; i < numsectors; i++, ss++)
					if(ss->ceilingpic == skyflatnum) // Only for the sky.
						P_SpawnLightningFlash(ss); // Spawn a quick flash thinker

				i = rand() % 256; // This doesn't need to use P_Random().

				if(i < 128 && leveltime & 1)
					S_StartSoundAtVolume(players[displayplayer].mo, sfx_litng1, volume);
				else if(i < 128)
					S_StartSoundAtVolume(players[displayplayer].mo, sfx_litng2, volume);
				else if(leveltime & 1)
					S_StartSoundAtVolume(players[displayplayer].mo, sfx_litng3, volume);
				else
					S_StartSoundAtVolume(players[displayplayer].mo, sfx_litng4, volume);
			}
			else if(leveltime & 1)
			{
				int random;

				random = rand() % 256; // This doesn't need to use P_Random().

				if(random > 253)
				{
					if(random & 1)
						S_StartSoundAtVolume(players[displayplayer].mo, sfx_athun1, volume);
					else
						S_StartSoundAtVolume(players[displayplayer].mo, sfx_athun2, volume);
				}
			}
		}
	}

    // only respawn items in deathmatch
    if (!cv_itemrespawn.value || !netgame)
        return; //

    // nothing left to respawn?
    if (iquehead == iquetail)
        return;

    // the first item in the queue is the first to respawn
    // wait at least 30 seconds
    if (leveltime - itemrespawntime[iquetail] < cv_itemrespawntime.value*TICRATE)
        return;

    mthing = itemrespawnque[iquetail];

    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;

    // spawn a teleport fog at the new spot
    ss = R_PointInSubsector (x,y);
    mo = P_SpawnMobj (x, y, ss->sector->floorheight , MT_IFOG);
    S_StartSound (mo, sfx_itmbk);

    // find which type to spawn
    for (i=0 ; i< NUMMOBJTYPES ; i++)
    {
        if (mthing->type == mobjinfo[i].doomednum)
            break;
    }

    // spawn it
    if (mobjinfo[i].flags & MF_SPAWNCEILING)
        z = ONCEILINGZ;
	else if(mthing->type == 2014 && mthing->options & MTF_AMBUSH) // Tails 08-05-2001
		z = ss->sector->floorheight + 32*FRACUNIT; // Tails 08-05-2001
    else
        z = ONFLOORZ;

    mo = P_SpawnMobj (x,y,z, i);
    mo->spawnpoint = mthing;
    mo->angle = ANG45 * (mthing->angle/45);

    // pull it from the que
    iquetail = (iquetail+1)&(ITEMQUESIZE-1);
}

// used when we are going from deathmatch 2 to deathmatch 1
void P_RespawnWeapons(void)
{
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;

    subsector_t*        ss;
    mobj_t*             mo;
    mapthing_t*         mthing;

    int                 i,j,freeslot;

    freeslot=iquetail;
    for(j=iquetail;j!=iquehead;j=(j+1)&(ITEMQUESIZE-1))
    {
        mthing = itemrespawnque[j];

        i=0;
        switch(mthing->type) {
            case 2001 : //mobjinfo[MT_SHOTGUN].doomednum  :
                 i=MT_SHOTGUN;
                 break;
            case 82   : //mobjinfo[MT_SUPERSHOTGUN].doomednum :
                 i=MT_SUPERSHOTGUN;
                 break;
            case 2002 : //mobjinfo[MT_CHAINGUN].doomednum :
                 i=MT_CHAINGUN;
                 break;
            case 2006 : //mobjinfo[MT_BFG9000].doomednum   : // bfg9000
                 i=MT_BFG9000;
                 break;
            case 2004 : //mobjinfo[MT_PLASMAGUNMISC28].doomednum   : // plasma launcher
                 i=MT_PLASMAGUN;
                 break;
            case 2003 : //mobjinfo[MT_ROCKETLAUNCH].doomednum   : // rocket launcher
                 i=MT_ROCKETLAUNCH;
                 break;
            case 2005 : //mobjinfo[MT_SHAINSAW].doomednum   : // shainsaw
                 i=MT_SHAINSAW;
                 break;
            default:
                 if(freeslot!=j)
                 {
                     itemrespawnque[freeslot]=itemrespawnque[j];
                     itemrespawntime[freeslot]=itemrespawntime[j];
                 }

                 freeslot=(freeslot+1)&(ITEMQUESIZE-1);
                 continue;
        }
        // respwan it
        x = mthing->x << FRACBITS;
        y = mthing->y << FRACBITS;

        // spawn a teleport fog at the new spot
        ss = R_PointInSubsector (x,y);
        mo = P_SpawnMobj (x, y, ss->sector->floorheight , MT_IFOG);
        S_StartSound (mo, sfx_itmbk);

        // spawn it
        if (mobjinfo[i].flags & MF_SPAWNCEILING)
            z = ONCEILINGZ;
        else
            z = ONFLOORZ;

        mo = P_SpawnMobj (x,y,z, i);
        mo->spawnpoint = mthing;
        mo->angle = ANG45 * (mthing->angle/45);
        // here don't increment freeslot
    }
    iquehead=freeslot;
}

extern byte weapontobutton[NUMWEAPONS];

//
// P_SpawnPlayer
// Called when a player is spawned on the level.
// Most of the player structure stays unchanged
//  between levels.
//
// BP: spawn it at a playerspawn mapthing
void P_SpawnPlayer (mapthing_t* mthing)
{
    player_t*           p;
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;

    mobj_t*             mobj;

    int                 i=mthing->type-1;

    // not playing?
    if (!playeringame[i])
        return;

#ifdef PARANOIA
    if(i<0 && i>=MAXPLAYERS)
        I_Error("P_SpawnPlayer : playernum not in bound (%d)",i);
#endif

    p = &players[i];

    if (p->playerstate == PST_REBORN)
        G_PlayerReborn (mthing->type-1);

    x           = mthing->x << FRACBITS;
    y           = mthing->y << FRACBITS;
    z           = ONFLOORZ;
    mobj        = P_SpawnMobj (x,y,z, MT_PLAYER);

#ifdef CLIENTPREDICTION
    //added 1-6-98 : for movement prediction
    p->spirit = P_SpawnMobj (x,y,z, MT_SPIRIT);
#endif

    // set color translations for player sprites
    // added 6-2-98 : change color : now use skincolor (befor is mthing->type-1)
	// Some new stuff here Tails 06-10-2001

		mobj->flags |= MF_TRANSLATION;
		mobj->color = p->skincolor;

    //
    // set 'spritedef' override in mobj for player skins.. (see ProjectSprite)
    // (usefulness : when body mobj is detached from player (who respawns),
    //  the dead body mobj retain the skin through the 'spritedef' override).
    mobj->skin = &skins[p->skin];

    mobj->angle = ANG45 * (mthing->angle/45);
    if (p==&players[consoleplayer])
        localangle = mobj->angle;
    else
    if (p==&players[secondarydisplayplayer])
        localangle2 = mobj->angle;
    mobj->player = p;
    mobj->health = p->health;

    p->mo = mobj;
    p->playerstate = PST_LIVE;
    p->refire = 0;
    p->message = NULL;
    p->damagecount = 0;
    p->bonuscount = 0;
    p->extralight = 0;
    p->fixedcolormap = 0;
	p->ringtimer = 0;
    p->viewheight = cv_viewheight.value<<FRACBITS;
	if (p == &players[consoleplayer])
		p->autobrake = cv_playerautobrake.value;
	if (cv_splitscreen.value && p == &players[secondarydisplayplayer])
		p->autobrake = cv_playerautobrake2.value;
    p->viewz = p->mo->z + p->viewheight;

    // setup gun psprite
    P_SetupPsprites (p);

    // give all cards in death match mode
    if (cv_deathmatch.value)
        p->cards = it_allkeys;

    if (mthing->type-1 == consoleplayer)
    {
        // wake up the status bar
        ST_Start ();
        // wake up the heads up text
        HU_Start ();
    }

#ifdef CLIENTPREDICTION2
    //added 1-6-98 : for movement prediction
    if(p->spirit)
        CL_ResetSpiritPosition(mobj);   // reset spirit possition
    else
        p->spirit = P_SpawnMobj (x,y,z, MT_SPIRIT);
    if( p==&players[consoleplayer] )
        mobj->eflags |= MF_INVISIBLE;   // don't show self

    p->spirit->skin    = mobj->skin;
    p->spirit->angle   = mobj->angle;
    p->spirit->player  = mobj->player;
    p->spirit->health  = mobj->health;
    p->spirit->movedir = weapontobutton[p->readyweapon];
#endif
    SV_SpawnPlayer(mthing->type-1,mobj);

    if (camera.chase && displayplayer==mthing->type-1)
       P_ResetCamera(p);
}


//
// P_SpawnMapThing
// The fields of the mapthing should
// already be in host byte order.
//
void P_SpawnMapThing (mapthing_t* mthing)
{
    int                 i;
    int                 bit;
    mobj_t*             mobj;
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;
    subsector_t*        ss; // Tails 08-30-2001
	int					r; // Vertical Rings Tails 08-05-2001

    if(!mthing->type)
      return; //SoM: 4/7/2000: Ignore type-0 things as NOPs

    // count deathmatch start positions
    if (mthing->type == 11)
    {
        if (deathmatch_p < &deathmatchstarts[MAX_DM_STARTS])
        {
            memcpy (deathmatch_p, mthing, sizeof(*mthing));
            deathmatch_p->type=0; // put it valide
            deathmatch_p++;
        }
        return;
    }

	if (mthing->type == 87) // CTF Startz! Tails 08-04-2001
    {
        if (redctfstarts_p < &redctfstarts[MAXPLAYERS])
        {
            memcpy (redctfstarts_p, mthing, sizeof(*mthing));
            redctfstarts_p->type=0; // put it valide
            redctfstarts_p++;
        }
        return;
    }

	if (mthing->type == 89) // CTF Startz! Tails 08-04-2001
    {
        if (bluectfstarts_p < &bluectfstarts[MAXPLAYERS])
        {
            memcpy (bluectfstarts_p, mthing, sizeof(*mthing));
            bluectfstarts_p->type=0; // put it valide
            bluectfstarts_p++;
        }
        return;
    }

    // check for players specially
    // added 9-2-98 type 5 -> 8 player[x] starts for cooperative
    //              support ctfdoom cooperative playerstart
    //SoM: 4/7/2000: Fix crashing bug.
    if ((mthing->type > 0 && mthing->type <=4) ||
        (mthing->type<=4028 && mthing->type>=4001))
    {
        if(mthing->type>4000)
             mthing->type=mthing->type-4001+5;

        // save spots for respawning in network games
        playerstarts[mthing->type-1] = *mthing;
        if ((cv_deathmatch.value==0 || cv_gametype.value==0) && demoversion<128) // Tails 03-13-2001
            P_SpawnPlayer (mthing);

        return;
    }

    // check for apropriate skill level
    if (!multiplayer && (mthing->options & 16))
        return;

    //SoM: 4/7/2000: Implement "not deathmatch" thing flag
    if (netgame && cv_deathmatch.value && (mthing->options & 32) )
      return;

    //SoM: 4/7/2000: Implement "not cooperative" thing flag
    if (netgame && !cv_deathmatch.value && (mthing->options & 64) )
      return;

    if (gameskill == sk_baby)
        bit = 1;
    else if (gameskill == sk_nightmare)
        bit = 4;
    else
        bit = 1<<(gameskill-1);

    if (!(mthing->options & bit) )
        return;

    // find which type to spawn
    for (i=0 ; i< NUMMOBJTYPES ; i++)
        if (mthing->type == mobjinfo[i].doomednum)
            break;

    if (i==NUMMOBJTYPES && (!i == 84 || !i == 44)) // Tails 08-05-2001
    {
        CONS_Printf ("\2P_SpawnMapThing: Unknown type %i at (%i, %i)\n",
                      mthing->type,
                      mthing->x, mthing->y);
        return;
    }

    // don't spawn any monsters if -nomonsters
    if (nomonsters
        && ((mobjinfo[i].flags & MF_ENEMY) || (mobjinfo[i].flags2 & MF2_BOSS))) // Tails 04-01-2001
//             || (mobjinfo[i].flags & MF_COUNTKILL)) )
    {
        return;
    }

	if (mobjinfo[i].flags2 & MF2_BOSS)
		level_has_bosses = true;

	if ((i == 84 || i == 44 || i == MT_MISC2 || i == MT_MISC10 || i == MT_MISC11 || i == MT_MISC50 || i == MT_MISC48 || i == MT_MISC31 || i == MT_BKTV)
		&& gameskill == sk_nightmare
		&& !(mapheaders[gamemap].special)) // Don't have rings in Very Hard mode Tails 03-26-2001
		return;

	if((i == MT_BLUEFLAG || i == MT_REDFLAG) && !cv_gametype.value == 4)
		return; // Don't spawn flags if you aren't in CTF Mode! Tails 09-03-2001

    // spawn it
    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;
    ss = R_PointInSubsector (x,y);

	z = ss->sector->floorheight;

    if (mobjinfo[i].flags & MF_SPAWNCEILING)
        z = ONCEILINGZ;
    else if (i == MT_MISC2 && (mthing->options & MTF_AMBUSH)) // Special flag for rings Tails 06-03-2001
		z += 32*FRACUNIT;
	else if (i == MT_DETON || i == MT_JETTBOMBER || i == MT_JETTGUNNER || i == MT_EGGMOBILE)
		z += 32*FRACUNIT;

	if(mthing->options >> 4)
		z += (mthing->options >> 4)*FRACUNIT;

	if(mthing->type == 84) // Vertical Rings - Stack of 5 Tails 08-05-2001
	{
		int oz = z;
		for(r=1; r<6; r++)
		{
			z = oz + (64*FRACUNIT*r);
			mobj = P_SpawnMobj (x,y,z, MT_MISC2);
			mobj->spawnpoint = mthing;

			if (mobj->tics > 0)
		      mobj->tics = 1 + (P_Random () % mobj->tics);

			mobj->angle = ANG45 * (mthing->angle/45);
			if (mthing->options & MTF_AMBUSH)
				mobj->flags |= MF_AMBUSH;
		}
	}
	if(mthing->type == 44) // Vertical Rings - Stack of 5 (suitable for Red Spring) Tails 08-05-2001
	{
		int oz = z;
		for(r=1; r<6; r++)
		{
			z = oz + (128*FRACUNIT*r);
			mobj = P_SpawnMobj (x,y,z, MT_MISC2);
			mobj->spawnpoint = mthing;

			if (mobj->tics > 0)
		      mobj->tics = 1 + (P_Random () % mobj->tics);

			mobj->angle = ANG45 * (mthing->angle/45);
			if (mthing->options & MTF_AMBUSH)
				mobj->flags |= MF_AMBUSH;
		}
	}
	else
	//P_SpawnMapThing tmp
	// Nozomi Fake Player Additions
	// 9995 - Standing Sonic
	// 9996 - Tired Tails
	// 9997 - Knuckles?
	if (mthing->type >= 9995 && mthing->type < 9998) {
		int skin = mthing->type-9995;
		mobj = P_SpawnMobj (x,y,z, MT_PLAYER);
		mobj->spawnpoint = mthing;

		mobj->skin = &skins[skin];

		mobj->color = (skin == 0) ? SKINCOLOR_BLUE+1 : (skin == 1) ? SKINCOLOR_APRICOT+1 : SKINCOLOR_BRIGHTRED+1;
		mobj->flags |= MF_TRANSLATION;
		P_SetMobjState(mobj, (skin == 0) ? S_DUMMY_STND : (skin == 1) ? S_PLAY_TAP1 : S_DUMMY_STND);
		mobj->angle = FixedAngle(mthing->angle*FRACUNIT);
	}
	else
	{
    mobj = P_SpawnMobj (x,y,z, i);
    mobj->spawnpoint = mthing;

	if(i == MT_EMMY)
		P_SpawnMobj(x,y,z, MT_TOKEN);
	else if(i == MT_EGGMOBILE && mapheaders[gamemap].hard_eggmobile)
	{
		mobj_t* spikemobj;
		spikemobj = P_SpawnMobj(x,y,z, MT_SPIKEBALL);
		spikemobj->target = mobj;
		spikemobj->angle = 0;
		spikemobj = P_SpawnMobj(x,y,z, MT_SPIKEBALL);
		spikemobj->target = mobj;
		spikemobj->angle = ANG90;
		spikemobj = P_SpawnMobj(x,y,z, MT_SPIKEBALL);
		spikemobj->target = mobj;
		spikemobj->angle = ANG180;
		spikemobj = P_SpawnMobj(x,y,z, MT_SPIKEBALL);
		spikemobj->target = mobj;
		spikemobj->angle = ANG270;
	}

    if (mobj->tics > 0)
        mobj->tics = 1 + (P_Random () % mobj->tics);
    if (mobj->flags & MF_COUNTKILL)
        totalkills++;
    if (mobj->flags & MF_COUNTITEM)
        totalitems++;

    mobj->angle = FixedAngle(mthing->angle*FRACUNIT);
    if (mthing->options & MTF_AMBUSH)
        mobj->flags |= MF_AMBUSH;
	}

	if (mobj->flags2 & MF2_PUSHABLE && mthing->options & MTF_AMBUSH)
		mobj->flags2 &= ~MF2_PUSHABLE;


	if (mobj->flags2 & MF2_SPRING) 
	{
		if (mobj->flags & MF_TRANSLATION)
			mobj->color = mobjinfo[i].speed+1;
	}
		
	if (mobj->type == MT_FLINGRING)
		mobj->flags |= MF_AMBUSH;
}



//
// GAME SPAWN FUNCTIONS
//

// --------------------------------------------------------------------------
// P_SpawnSmoke
// --------------------------------------------------------------------------
// when player gets hurt by lava/slime, spawn at feet
void P_SpawnSmoke ( fixed_t       x,
                    fixed_t       y,
                    fixed_t       z )
{
    mobj_t*     th;

    if (demoversion<125)
        return;

    x = x - ((P_Random()&8) * FRACUNIT) - 4*FRACUNIT;
    y = y - ((P_Random()&8) * FRACUNIT) - 4*FRACUNIT;
    z += (P_Random()&3) * FRACUNIT;


    th = P_SpawnMobj (x,y,z, MT_SMOK);
    th->momz = FRACUNIT;
    th->tics -= P_Random()&3;

    if (th->tics < 1)
        th->tics = 1;
}



// --------------------------------------------------------------------------
// P_SpawnPuff
// --------------------------------------------------------------------------
void P_SpawnPuff ( fixed_t       x,
                   fixed_t       y,
                   fixed_t       z )
{
    mobj_t*     th;

    z += P_Random()<<10;
    z -= P_Random()<<10;

    th = P_SpawnMobj (x,y,z, MT_PUFF);
    th->momz = FRACUNIT;
    th->tics -= P_Random()&3;

    if (th->tics < 1)
        th->tics = 1;

    // don't make punches spark on the wall
    if (attackrange == MELEERANGE)
        P_SetMobjState (th, S_PUFF3);
}



// --------------------------------------------------------------------------
// P_SpawnBlood
// --------------------------------------------------------------------------

static mobj_t*  bloodthing;
// static fixed_t  bloodspawnpointx,bloodspawnpointy; // Tails 11-16-2001

#ifdef WALLSPLATS
boolean PTR_BloodTraverse (intercept_t* in)
{
    line_t*             li;
    divline_t   divl;
    fixed_t     frac;

    fixed_t     z;

    if (in->isaline)
    {
        li = in->d.line;

        z = bloodthing->z + (P_Random()<<(FRACBITS-3));
        z -= P_Random()<<(FRACBITS-3);
        if ( !(li->flags & ML_TWOSIDED) )
            goto hitline;

        P_LineOpening (li);

        // hit lower texture ?
        if (li->frontsector->floorheight != li->backsector->floorheight)
        {
            if( openbottom>z )
                goto hitline;
        }

        // hit upper texture ?
        if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
        {
            if( opentop<z )
                goto hitline;
        }

        // else don't hit
        return true;

hitline:
        P_MakeDivline (li, &divl);
        frac = P_InterceptVector (&divl, &trace);
        R_AddWallSplat (li, P_PointOnLineSide(bloodspawnpointx,bloodspawnpointy,li),"BLUDC0", z, frac, SPLATDRAWMODE_TRANS);
        return false;
    }

    //continue
    return true;
}
#endif

// P_SpawnBloodSplats
// the new SpawnBlood : this one first calls P_SpawnBlood for the usual blood sprites
// then spawns blood splats around on walls
//
void P_SpawnBloodSplats ( fixed_t       x,
                          fixed_t       y,
                          fixed_t       z,
                          int           damage,
                          fixed_t       momx,
                          fixed_t       momy)
{
#ifdef WALLSPLATS
//static int  counter =0;
    fixed_t x2,y2;
    angle_t angle, anglesplat;
    int     distance;
    angle_t anglemul=1;  
    int     numsplats;
    int     i;
#endif
    // spawn the usual falling blood sprites at location
//    P_SpawnBlood (x,y,z,damage);
    //CONS_Printf ("spawned blood counter %d\n", counter++);
    if( demoversion<129)
        return;


#ifdef WALLSPLATS
    // traverse all linedefs and mobjs from the blockmap containing t1,
    // to the blockmap containing the dest. point.
    // Call the function for each mobj/line on the way,
    // starting with the mobj/linedef at the shortest distance...

    if(!momx && !momy)
    {   
        // from inside
        angle=0;
        anglemul=2; 
    }
    else
    {
        // get direction of damage
        x2 = x + momx;
        y2 = y + momy;
        angle = R_PointToAngle2 (x,y,x2,y2);
    }
    distance = damage * 6;
    numsplats = damage / 3+1;
    // BFG is funy without this check
    if( numsplats > 20 )
        numsplats = 20;

    //CONS_Printf ("spawning %d bloodsplats at distance of %d\n", numsplats, distance);
    //CONS_Printf ("damage %d\n", damage);
    bloodspawnpointx = x;
    bloodspawnpointy = y;
    //uses 'bloodthing' set by P_SpawnBlood()
    for (i=0; i<numsplats; i++) {
        // find random angle between 0-180deg centered on damage angle
        anglesplat = angle + (((P_Random() - 128) * FINEANGLES/512*anglemul)<<ANGLETOFINESHIFT);
        x2 = x + distance*finecosine[anglesplat>>ANGLETOFINESHIFT];
        y2 = y + distance*finesine[anglesplat>>ANGLETOFINESHIFT];
        //CONS_Printf ("traversepath cangle %d angle %d fuck %d\n", (angle>>ANGLETOFINESHIFT)*360/FINEANGLES,
        //    (anglesplat>>ANGLETOFINESHIFT)*360/FINEANGLES, (P_Random() - 128));

        P_PathTraverse ( x, y,
                         x2, y2,
                         PT_ADDLINES, 
                         PTR_BloodTraverse );
    }
#endif

#ifdef FLOORSPLATS
    // add a test floor splat
    R_AddFloorSplat (bloodthing->subsector, "STEP2", x, y, bloodthing->floorz, SPLATDRAWMODE_SHADE);
#endif
}


// P_SpawnBlood
// spawn a blood sprite with falling z movement, at location
// the duration and first sprite frame depends on the damage level
// the more damage, the longer is the sprite animation
void P_SpawnBlood ( fixed_t       x,
                    fixed_t       y,
                    fixed_t       z,
                    int           damage )
{
    mobj_t*     th;

    z += P_Random()<<10;
    z -= P_Random()<<10;
    th = P_SpawnMobj (x,y,z, MT_BLOOD);
    if(demoversion>=128)
    {
        th->momx  = P_Random()<<12; //faB:19jan99
        th->momx -= P_Random()<<12; //faB:19jan99
        th->momy  = P_Random()<<12; //faB:19jan99
        th->momy -= P_Random()<<12; //faB:19jan99
    }
    th->momz = FRACUNIT*2;
    th->tics -= P_Random()&3;

    if (th->tics < 1)
        th->tics = 1;

    if (damage <= 12 && damage >= 9)
        P_SetMobjState (th,S_BLOOD2);
    else if (damage < 9)
        P_SetMobjState (th,S_BLOOD3);

    bloodthing = th;
}


//
// P_CheckMissileSpawn
// Moves the missile forward a bit
//  and possibly explodes it right there.
//
void P_CheckMissileSpawn (mobj_t* th)
{
    th->tics -= P_Random()&3;
    if (th->tics < 1)
        th->tics = 1;

    // move a little forward so an angle can
    // be computed if it immediately explodes
    th->x += (th->momx>>1);
    th->y += (th->momy>>1);
    th->z += (th->momz>>1);

    if (!P_TryMove (th, th->x, th->y, false))
        P_ExplodeMissile (th);
}


//
// P_SpawnMissile
//
mobj_t* P_SpawnMissile ( mobj_t*       source,
                         mobj_t*       dest,
                         mobjtype_t    type )
{
    mobj_t*     th;
    angle_t     an;
    int         dist;

#ifdef PARANOIA
    if(!source)
        I_Error("P_SpawnMissile : no source");
    if(!dest)
        I_Error("P_SpawnMissile : no dest");
#endif
	if(source->type == MT_JETTGUNNER) // Tails 08-25-2001
    th = P_SpawnMobj (source->x,
                      source->y,
                      source->z - 12*FRACUNIT, type); // Tails 08-25-2001
	else // Tails 08-25-2001
    th = P_SpawnMobj (source->x,
                      source->y,
                      source->z + 4*8*FRACUNIT, type);

    if (th->info->seesound)
        S_StartSound (th, th->info->seesound);

    th->target = source;        // where it came from
    an = R_PointToAngle2 (source->x, source->y, dest->x, dest->y);

// Invis shouldn't matter Tails 01-06-2001
/*
    // fuzzy player
    if (dest->flags & MF_SHADOW)
    {
        an += (P_Random()<<20); // WARNING: don't put this in one line 
        an -= (P_Random()<<20); // else this expretion is ambiguous (evaluation order not diffined)
    }
*/
    th->angle = an;
    an >>= ANGLETOFINESHIFT;
    th->momx = FixedMul (th->info->speed, finecosine[an]);
    th->momy = FixedMul (th->info->speed, finesine[an]);

    dist = P_AproxDistance (dest->x - source->x, dest->y - source->y);
    dist = dist / th->info->speed;

    if (dist < 1)
        dist = 1;

    th->momz = (dest->z - source->z) / dist;
    P_CheckMissileSpawn (th);

    return th;
}


//
// P_SpawnPlayerMissile
// Tries to aim at a nearby monster
//
void P_SpawnPlayerMissile ( mobj_t*       source,
                            mobjtype_t    type,
              //added:16-02-98: needed the player here for the aiming
              player_t*     player )
{
    mobj_t*     th;
    angle_t     an;

    fixed_t     x;
    fixed_t     y;
    fixed_t     z;
    fixed_t     slope;

    // angle at which you fire, is player angle
    an = source->angle;

    //added:16-02-98: autoaim is now a toggle
    if (player->autoaim_toggle && cv_allowautoaim.value)
    {
        // see which target is to be aimed at
        slope = P_AimLineAttack (source, an, 16*64*FRACUNIT);

        if (!linetarget)
        {
            an += 1<<26;
            slope = P_AimLineAttack (source, an, 16*64*FRACUNIT);

            if (!linetarget)
            {
                an -= 2<<26;
                slope = P_AimLineAttack (source, an, 16*64*FRACUNIT);
            }

            if (!linetarget)
            {
                an = source->angle;
                slope = 0;
            }
        }
    }

    //added:18-02-98: if not autoaim, or if the autoaim didnt aim something,
    //                use the mouseaiming
    if (!(player->autoaim_toggle && cv_allowautoaim.value)
                                || (!linetarget && demoversion>111))
    {
        if(demoversion>=128)
            slope = AIMINGTOSLOPE(player->aiming);
        else
            slope = (player->aiming<<FRACBITS)/160;
    }

    x = source->x;
    y = source->y;
    z = source->z + source->height/3; // Tails 03-25-2001

    th = P_SpawnMobj (x,y,z, type);

    if (th->info->seesound)
        S_StartSound (th, th->info->seesound);

    th->target = source;

    th->angle = an;
    th->momx = FixedMul( th->info->speed, finecosine[an>>ANGLETOFINESHIFT]);
    th->momy = FixedMul( th->info->speed, finesine[an>>ANGLETOFINESHIFT]);
    
    if( demoversion>=128 )
    {   // 1.28 fix, allow full aiming must be much precise
        th->momx = FixedMul(th->momx,finecosine[player->aiming>>ANGLETOFINESHIFT]);
        th->momy = FixedMul(th->momy,finecosine[player->aiming>>ANGLETOFINESHIFT]);
    }
	th->momz = FixedMul( th->info->speed, slope);

    P_CheckMissileSpawn (th);
}
