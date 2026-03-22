// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: byteptr.h,v 1.4 2000/04/16 18:38:06 bpereira Exp $
//
// Copyright (C) 1998-2000 by DooM Legacy Team.
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
// $Log: byteptr.h,v $
// Revision 1.4  2000/04/16 18:38:06  bpereira
// no message
//
//
// DESCRIPTION:
//    Macro to read/write from/to a char*, used for packet cration and such...
//
//-----------------------------------------------------------------------------

// Replace this with a much better version from SRB2: Delta Quest
// Save 21-03-2026

// VS2002 and VS2003 are untested but this works for VS6 and VS8/VS2005
#ifndef _MSC_VER
#include <stdint.h>      /* C99+ */
#endif

#ifndef __BIG_ENDIAN__
//
// Little-endian machines
//

#if defined(_MSC_VER)
// Seems to work fine for VS6 and VS8
#define WRITEBYTE(p,b)      do { byte *p_tmp = (byte *)p; *p_tmp = (byte)(b); p_tmp++; p = (void *)p_tmp; } while (0)
#define WRITECHAR(p,b)      do { char *p_tmp = (char *)p; *p_tmp = (char)(b); p_tmp++; p = (void *)p_tmp; } while (0)
#define WRITESHORT(p,b)     do { short *p_tmp = (short *)p; *p_tmp = (short)(b); p_tmp++; p = (void *)p_tmp; } while (0)
#define WRITEUSHORT(p,b)    do { USHORT *p_tmp = (USHORT *)p; *p_tmp = (USHORT)(b); p_tmp++; p = (void *)p_tmp; } while (0)
#define WRITELONG(p,b)      do { long *p_tmp = (long *)p; *p_tmp = (long)(b); p_tmp++; p = (void *)p_tmp; } while (0)
#define WRITEULONG(p,b)     do { ULONG *p_tmp = (ULONG *)p; *p_tmp = (ULONG)(b); p_tmp++; p = (void *)p_tmp; } while (0)
#define WRITEFIXED(p,b)     do { fixed_t *p_tmp = (fixed_t *)p; *p_tmp = (fixed_t)(b); p_tmp++; p = (void *)p_tmp; } while (0)
#define WRITEANGLE(p,b)     do { angle_t *p_tmp = (angle_t *)p; *p_tmp = (angle_t)(b); p_tmp++; p = (void *)p_tmp; } while (0)
#else
// CMake + Ninja
#define WRITEBYTE(p, b)        \
    do {                               \
        *(p) = (b);                    \
        (p)++;                         \
    } while (0)
#define WRITECHAR(p, b)        \
do {                               \
	*(p) = (b);                    \
	(p)++;                         \
} while (0)
#define WRITESHORT(p, b)        \
do {                               \
	*(p) = (b);                    \
	(p)++;                         \
} while (0)
#define WRITEUSHORT(p, b)        \
do {                               \
	*(p) = (b);                    \
	(p)++;                         \
} while (0)
#define WRITELONG(p, b)        \
do {                               \
	*(p) = (b);                    \
	(p)++;                         \
} while (0)
#define WRITEULONG(p, b)        \
do {                               \
	*(p) = (b);                    \
	(p)++;                         \
} while (0)
#define WRITEFIXED(p, b)        \
do {                               \
	*(p) = (b);                    \
	(p)++;                         \
} while (0)
#define WRITEANGLE(p, b)        \
do {                               \
	*(p) = (b);                    \
	(p)++;                         \
} while (0)
#endif
#define WRITESTRING(p,b)    { int tmp_i = 0; do { WRITECHAR(p, b[tmp_i]); } while (b[tmp_i++]); }
#define WRITESTRINGN(p,b,n) { int tmp_i = 0; do { WRITECHAR(p, b[tmp_i]); if (!b[tmp_i]) break; tmp_i++; } while (tmp_i < n); }
#define WRITEMEM(p,s,n)     memcpy(p, s, n); p += n

#ifdef __GNUC__
//#define READBYTE(p)         ({ byte *p_tmp = (byte *)p; byte b = *p_tmp; p_tmp++; p = (void *)p_tmp; b; })
#define READBYTE(p)                    \
	({                                 \
		byte p_tmp = *(p);			   \
        (p)++;                         \
		p_tmp;						   \
    })
//#define READCHAR(p)         ({ char *p_tmp = (char *)p; char b = *p_tmp; p_tmp++; p = (void *)p_tmp; b; })
#define READCHAR(p)                    \
	({                                 \
		char p_tmp = *(p);			   \
        (p)++;                         \
		p_tmp;						   \
    })
//#define READSHORT(p)        ({ short *p_tmp = (short *)p; short b = *p_tmp; p_tmp++; p = (void *)p_tmp; b; })
#define READSHORT(p)                   \
	({                                 \
		short p_tmp = *(p);			   \
        (p)++;                         \
		p_tmp;						   \
    })
//#define READUSHORT(p)       ({ USHORT *p_tmp = (USHORT *)p; USHORT b = *p_tmp; p_tmp++; p = (void *)p_tmp; b; })
#define READUSHORT(p)                  \
	({                                 \
		USHORT p_tmp = *(p);		   \
        (p)++;                         \
		p_tmp;						   \
    })
//#define READLONG(p)         ({ long *p_tmp = (long *)p; long b = *p_tmp; p_tmp++; p = (void *)p_tmp; b; })
#define READLONG(p)                    \
	({                                 \
		long p_tmp = *(p);			   \
        (p)++;                         \
		p_tmp;						   \
    })
//#define READULONG(p)        ({ ULONG *p_tmp = (ULONG *)p; ULONG b = *p_tmp; p_tmp++; p = (void *)p_tmp; b; })
#define READULONG(p)                   \
	({                                 \
		ULONG p_tmp = *(p);		       \
        (p)++;                         \
		p_tmp;						   \
    })
//#define READFIXED(p)        ({ fixed_t *p_tmp = (fixed_t *)p; fixed_t b = *p_tmp; p_tmp++; p = (void *)p_tmp; b; })
#define READFIXED(p)                   \
	({                                 \
		fixed_t p_tmp = *(p);		   \
        (p)++;                         \
		p_tmp;						   \
    })
//#define READANGLE(p)        ({ angle_t *p_tmp = (angle_t *)p; angle_t b = *p_tmp; p_tmp++; p = (void *)p_tmp; b; })
#define READANGLE(p)                   \
	({                                 \
		angle_t p_tmp = *(p);		   \
        (p)++;                         \
		p_tmp;						   \
    })
#else
#define READBYTE(p)         *((byte *)p)++
#define READCHAR(p)         *((char *)p)++
#define READSHORT(p)        *((short *)p)++
#define READUSHORT(p)       *((USHORT *)p)++
#define READLONG(p)         *((long *)p)++
#define READULONG(p)        *((ULONG *)p)++
#define READFIXED(p)        *((fixed_t *)p)++
#define READANGLE(p)        *((angle_t *)p)++
#endif

#define READSTRING(p,s)     { int tmp_i = 0; do { s[tmp_i] = READBYTE(p); } while (s[tmp_i++]); }
#define READSTRINGN(p,s,n)     { int tmp_i = 0; do { s[tmp_i] = READBYTE(p); if (!s[tmp_i]) break;tmp_i++; } while (tmp_i < n); }
#define SKIPSTRING(p)       while (READBYTE(p))
#define READMEM(p,s,n)      memcpy(s, p, n); p += n

#else //__BIG_ENDIAN__
//
// definitions for big-endian machines with alignment constraints.
//
// Write a value to a little-endian, unaligned destination.
//
FUNCINLINE static ATTRINLINE void writeshort(void *ptr, int val)
{
	char *cp = ptr;
	cp[0] = val; val >>= 8;
	cp[1] = val;
}

FUNCINLINE static ATTRINLINE void writelong(void *ptr, int val)
{
	char *cp = ptr;
	cp[0] = val; val >>= 8;
	cp[1] = val; val >>= 8;
	cp[2] = val; val >>= 8;
	cp[3] = val;
}

#define WRITEBYTE(p,b)      do { byte *p_tmp = (byte *)p; *p_tmp = (byte)(b); p_tmp++; p = (void *)p_tmp; } while (0)
#define WRITECHAR(p,b)      do { char *p_tmp = (char *)p; *p_tmp = (char)(b); p_tmp++; p = (void *)p_tmp; } while (0)
#define WRITESHORT(p,b)     do {short *p_tmp = (short *)p; writeshort (p, (short)(b)); p_tmp++; p = (void *)p_tmp;} while (0)
#define WRITEUSHORT(p,b)    do {USHORT *p_tmp = (USHORT *)p; writeshort (p,  (USHORT)(b)); p_tmp++; p = (void *)p_tmp;} while (0)
#define WRITELONG(p,b)      do {long *p_tmp = (long *)p; writelong (p, (long)(b)); p_tmp++; p = (void *)p_tmp;} while (0)
#define WRITEULONG(p,b)     do {ULONG *p_tmp = (ULONG *)p; writelong (p, (ULONG)(b)); p_tmp++; p = (void *)p_tmp;} while (0)
#define WRITEFIXED(p,b)     do {fixed_t *p_tmp = (fixed_t *)p; writelong (p, (fixed_t)(b)); p_tmp++; p = (void *)p_tmp;} while (0)
#define WRITEANGLE(p,b)     do {angle_t *p_tmp = (angle_t *)p; writelong (p, (angle_t)(b)); p_tmp++; p = (void *)p_tmp;} while (0)
#define WRITESTRING(p,b)    { int tmp_i = 0; do { WRITECHAR(p,b[tmp_i]); } while (b[tmp_i++]); }
#define WRITESTRINGN(p,b,n) { int tmp_i = 0; do { WRITECHAR(p,b[tmp_i]); if (!b[tmp_i]) break;tmp_i++; } while (tmp_i < n); }
#define WRITEMEM(p,s,n)     memcpy(p, s, n); p += n

// Read a signed quantity from little-endian, unaligned data.
//
FUNCINLINE static ATTRINLINE short readshort(void *ptr)
{
	char *cp  = ptr;
	u_char *ucp = ptr;
	return (cp[1] << 8) | ucp[0];
}

FUNCINLINE static ATTRINLINE USHORT readushort(void *ptr)
{
	u_char *ucp = ptr;
	return (ucp[1] << 8) | ucp[0];
}

FUNCINLINE static ATTRINLINE long readlong(void *ptr)
{
	char *cp = ptr;
	u_char *ucp = ptr;
	return (cp[3] << 24) | (ucp[2] << 16) | (ucp[1] << 8) | ucp[0];
}

FUNCINLINE static ATTRINLINE ULONG readulong(void *ptr)
{
	u_char *ucp = ptr;
	return (ucp[3] << 24) | (ucp[2] << 16) | (ucp[1] << 8) | ucp[0];
}

#define READBYTE(p)         ({ byte *p_tmp = (byte *)p; byte b = *p_tmp; p_tmp++; p = (void *)p_tmp; b; })
#define READCHAR(p)         ({ char *p_tmp = (char *)p; char b = *p_tmp; p_tmp++; p = (void *)p_tmp; b; })
#define READSHORT(p)        ({ short *p_tmp = (short *)p; short b = readshort(p); p_tmp++; p = (void *)p_tmp; b; })
#define READUSHORT(p)       ({ USHORT *p_tmp = (USHORT *)p; USHORT b = readushort(p); p_tmp++; p = (void *)p_tmp; b; })
#define READLONG(p)         ({ long *p_tmp = (long *)p; long b = readlong(p); p_tmp++; p = (void *)p_tmp; b; })
#define READULONG(p)        ({ ULONG *p_tmp = (ULONG *)p; ULONG b = readulong(p); p_tmp++; p = (void *)p_tmp; b; })
#define READFIXED(p)        ({ fixed_t *p_tmp = (fixed_t *)p; fixed_t b = readlong(p); p_tmp++; p = (void *)p_tmp; b; })
#define READANGLE(p)        ({ angle_t *p_tmp = (angle_t *)p; angle_t b = readulong(p); p_tmp++; p = (void *)p_tmp; b; })
#define READSTRING(p,s)     { int tmp_i = 0; do { s[tmp_i] = READBYTE(p); } while (s[tmp_i++]); }
#define SKIPSTRING(p)       while (READBYTE(p))
#define READMEM(p,s,n)      memcpy(s, p, n); p += n
#endif //__BIG_ENDIAN__
