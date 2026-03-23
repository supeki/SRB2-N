// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: s_sound.h,v 1.5 2000/05/13 19:52:10 metzgermeister Exp $
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
// $Log: s_sound.h,v $
// Revision 1.5  2000/05/13 19:52:10  metzgermeister
// cd vol jiggle
//
// Revision 1.4  2000/04/21 08:23:47  emanne
// To have SDL working.
// Makefile: made the hiding by "@" optional. See the CC variable at
// the begining. Sorry, but I like to see what's going on while building
//
// qmus2mid.h: force include of qmus2mid_sdl.h when needed.
// s_sound.c: ??!
// s_sound.h: with it.
// (sorry for s_sound.* : I had problems with cvs...)
//
// Revision 1.3  2000/03/12 23:21:10  linuxcub
// Added consvars which hold the filenames and arguments which will be used
// when running the soundserver and musicserver (under Linux). I hope I
// didn't break anything ... Erling Jacobsen, linuxcub@email.dk
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      The not so system specific sound interface.
//
//-----------------------------------------------------------------------------


#ifndef __S_SOUND__
#define __S_SOUND__

#include "sounds.h"

// killough 4/25/98: mask used to indicate sound origin is player item pickup
#define PICKUP_SOUND (0x8000)

extern consvar_t stereoreverse;

extern consvar_t cv_soundvolume;
extern consvar_t cv_musicvolume;
extern consvar_t cv_numChannels;

extern consvar_t cv_usepitch;
extern consvar_t cv_ringpitch;
extern consvar_t cv_underwaterpitch;

extern consvar_t cv_oldsoundbehavior;

#ifdef SNDSERV
extern consvar_t sndserver_cmd;
extern consvar_t sndserver_arg;
#endif
#ifdef MUSSERV
extern consvar_t musserver_cmd;
extern consvar_t musserver_arg;
#endif

extern CV_PossibleValue_t soundvolume_cons_t[];
//part of i_cdmus.c
extern consvar_t cd_volume;
extern consvar_t cdUpdate;
#ifdef LINUX
extern consvar_t cv_jigglecdvol;
#endif

typedef enum
{
	SF_TOTALLYSINGLE =  1,
	SF_MULTIPLESOUND =  2,
	SF_OUTSIDESOUND  =  4,
	SF_X4AWAYSOUND   =  8,
	SF_X8AWAYSOUND   = 16,
} soundflags_t;

// register sound vars and commands at game startup
void S_RegisterSoundStuff (void);


//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//
void S_Init (int sfxVolume, int musicVolume);


//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//
void S_Start(void);

//
// basicaly a wgetnumforname with adding "ds" at the begin of string
// return a lumpnum
//
int S_GetSfxLumpNum (sfxinfo_t* sfx);

//
// Start sound for thing at <origin>
//  using <sound_id> from sounds.h
//
void
S_StartSound
( void*         origin,
  int           sound_id );



// Will start a sound at a given volume.
void
S_StartSoundAtVolume
( void*         origin,
  int           sound_id,
  int           volume );

void
S_StartSoundAtVolumeAndPitch
( void*         origin,
  int           sound_id,
  int           volume, 
  int			pitch );


// Stop sound for thing at <origin>
void S_StopSound(void* origin);


// Start music using <music_id> from sounds.h
void S_StartMusic(int music_id);

// Start music using <music_id> from sounds.h,
//  and set whether looping
void S_ChangeMusic (int music_id, int looping);
void S_ChangeMusicName (char *music_name, int looping);

// Stops the music fer sure.
void S_StopMusic(void);

// Stop and resume music, during game PAUSE.
void S_PauseSound(void);
void S_ResumeSound(void);


//
// Updates music & sounds
//
void S_UpdateSounds(void);

void S_SetMusicVolume(int volume);
void S_SetSfxVolume(int volume);

// Moving these from S_Sound.c Nozomi 03-19-2026
#define S_MAX_VOLUME            127

// when to clip out sounds
// Does not fit the large outdoor areas.
// added 2-2-98 in 8 bit volume control (befort  (1200*0x10000))
#define S_CLIPPING_DIST         (1200*0x10000)

// Distance tp origin when sounds should be maxed out.
// This should relate to movement clipping resolution
// (see BLOCKMAP handling).
// Originally: (200*0x10000).
// added 2-2-98 in 8 bit volume control (befort  (160*0x10000))
#define S_CLOSE_DIST            (160*0x10000)

// added 2-2-98 in 8 bit volume control (befort  remove the +4)
#define S_ATTENUATOR            ((S_CLIPPING_DIST-S_CLOSE_DIST)>>(FRACBITS+4))

// Adjustable by menu.
#define NORM_VOLUME             snd_MaxVolume

#define NORM_PITCH              127
#define NORM_PRIORITY           64
#define NORM_SEP                128

#define S_PITCH_PERTURB         1
#define S_STEREO_SWING          (96*0x10000)

// percent attenuation from front to back
#define S_IFRACVOL              30


#endif
