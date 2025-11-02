// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: doomtype.h 754 2010-09-26 11:49:59Z smite-meister $
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2009 by DooM Legacy Team.
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
// $Log: doomtype.h,v $
// Revision 1.8  2001/05/16 22:33:34  bock
// Initial FreeBSD support.
//
// Revision 1.7  2001/04/17 22:26:07  calumr
// Initial Mac add
//
// Revision 1.6  2001/02/24 13:35:19  bpereira
// no message
//
// Revision 1.5  2000/11/02 19:49:35  bpereira
// no message
//
// Revision 1.4  2000/10/21 08:43:28  bpereira
// no message
//
// Revision 1.3  2000/08/10 14:53:57  ydario
// OS/2 port
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      doom games standard types
//      Simple basic typedefs, isolated here to make it easier
//      separating modules.
//    
//-----------------------------------------------------------------------------


#ifndef __DOOMTYPE__
#define __DOOMTYPE__

#include <stdint.h>


#ifdef WIN32
#define WINVER 0x0500  // require windows 2k or later
#include <windows.h>
#else
// WIN32 standard headers already define these!
// TODO this is just a temporary measure, all USHORT / ULONG / INT64 instances 
// in the code should be changed to stdint.h types or basic types
typedef uint32_t ULONG;
typedef uint16_t USHORT;
typedef  int64_t INT64;
#endif

// boolean type
#ifdef __APPLE_CC__
# define boolean int
# define false 0
# define true  1
#elif defined(WIN32)
# define false   FALSE           // use windows types
# define true    TRUE
# define boolean BOOL
#else
typedef enum {false, true} boolean;
#endif


typedef uint8_t    byte;
typedef uint32_t  tic_t;


#ifdef WIN32
# define ASMCALL __cdecl
#else
# define ASMCALL
#endif


#ifdef __APPLE_CC__
#define __MACOS__
#define DEBUG_LOG
#define HWRENDER
#endif

#if defined( __MSC__) || defined( __OS2__)
    // Microsoft VisualC++
    #define strncasecmp             strnicmp
    #define strcasecmp              stricmp
    #define inline                  __inline
#else
    #ifdef __WATCOMC__
        #include <dos.h>
        #include <sys\types.h>
        #include <direct.h>
        #include <malloc.h>
        #define strncasecmp             strnicmp
        #define strcasecmp              strcmpi
    #endif
#endif

// [smite] we should use standard funcs
#define stricmp(x,y) strcasecmp(x,y)
#define strnicmp(x,y,n) strncasecmp(x,y,n)


#ifndef WIN32
#define min(x,y) ( ((x)<(y)) ? (x) : (y) )
#define max(x,y) ( ((x)>(y)) ? (x) : (y) )
#if !defined( __DJGPP__ )	
int strupr(char *n);
int strlwr(char *n);
#endif
#endif

#ifndef O_BINARY
#define O_BINARY 0 // stupid windows text files
#endif



// Predefined with some OS.
#ifndef __WIN32__
#ifndef __MACOS__
#ifndef FREEBSD
#include <values.h>
#else
#include <limits.h>
#endif
#endif
#endif

#ifndef MAXCHAR
#define MAXCHAR   ((char)0x7f)
#endif
#ifndef MAXSHORT
#define MAXSHORT  ((short)0x7fff)
#endif
#ifndef MAXINT
#define MAXINT    ((int)0x7fffffff)
#endif

#ifndef MINCHAR
#define MINCHAR   ((char)0x80)
#endif
#ifndef MINSHORT
#define MINSHORT  ((short)0x8000)
#endif
#ifndef MININT
#define MININT    ((int)0x80000000)
#endif

union FColorRGBA {
    uint32_t  rgba;
    struct {
        byte  red;
        byte  green;
        byte  blue;
        byte  alpha;
    } s;
};
typedef union FColorRGBA RGBA_t;

#ifdef __BIG_ENDIAN__
#define UINT2RGBA(a) a
#else
#define UINT2RGBA(a) ((a&0xff)<<24)|((a&0xff00)<<8)|((a&0xff0000)>>8)|(((ULONG)a&0xff000000)>>24)
#endif

#endif  //__DOOMTYPE__
