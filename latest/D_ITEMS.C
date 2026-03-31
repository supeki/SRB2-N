// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: d_items.c,v 1.2 2000/02/27 00:42:10 hurdler Exp $
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
// $Log: d_items.c,v $
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION: 
//      holds the weapon info for now...
//
//-----------------------------------------------------------------------------



// We are referring to sprite numbers.
#include "info.h"
#include "d_items.h"


//
// PSPRITE ACTIONS for weapons.
// This struct controls the weapon animations.
//
// Each entry is:
//  ammo/amunition type
//  upstate
//  downstate
//  readystate
//  atkstate, i.e. attack/fire/hit frame
//  flashstate, muzzle flash
//

// Erm... this is Sonic the FREAKING hedgehog!!
// Remove allis later
// Save 01-04-2026
weaponinfo_t    weaponinfo[NUMWEAPONS] =
{
    {
        // fist
        am_noammo,
        0,
        S_NULL,
        S_NULL,
        S_NULL,
        S_NULL,
        S_NULL
    },
    {
        // pistol
        am_noammo,
        0,
        S_NULL,
        S_NULL,
        S_NULL,
        S_NULL,
        S_NULL
    },
    {
        // shotgun
        am_shell,
        1,
        S_NULL,
        S_NULL,
        S_NULL,
        S_NULL,
        S_NULL
    },
    {
        // chaingun
        am_clip,
        1,
        S_NULL,
        S_NULL,
        S_NULL,
        S_NULL,
        S_NULL
    },
    {
        // missile launcher
        am_misl,
        1,
        S_NULL,
        S_NULL,
        S_NULL,
        S_NULL,
        S_NULL
    },
    {
        // plasma rifle
        am_cell,
        1,
        S_NULL,
        S_NULL,
        S_NULL,
        S_NULL,
        S_NULL
    },
    {
        // bfg 9000
        am_cell,
        40,
        S_NULL,
        S_NULL,
        S_NULL,
        S_NULL,
        S_NULL
    },
    {
        // chainsaw
        am_noammo,
        0,
        S_NULL,
        S_NULL,
        S_NULL,
        S_NULL,
        S_NULL
    },
    {
        // super shotgun
        am_shell,
        2,
        S_NULL,
        S_NULL,
        S_NULL,
        S_NULL,
        S_NULL
    },
};
