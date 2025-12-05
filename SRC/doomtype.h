// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: doomtype.h 1069 2013-12-14 00:26:30Z wesleyjohnson $
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2013 by DooM Legacy Team.
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
// Include: DOS DJGPP Fixes
//
// Revision 1.8  2001/05/16 22:33:34  bock
// Initial FreeBSD support.
//
// Revision 1.7  2001/04/17 22:26:07  calumr
// Initial Mac add
//
// Revision 1.6  2001/02/24 13:35:19  bpereira
// Revision 1.5  2000/11/02 19:49:35  bpereira
// Revision 1.4  2000/10/21 08:43:28  bpereira
//
// Revision 1.3  2000/08/10 14:53:57  ydario
// OS/2 port
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      doom games standard types
//      Simple basic typedefs, isolated here to make it easier
//      separating modules.
//      Dependent upon system and compile flags, but not dependent upon
//      doomdef.h.
//    
//-----------------------------------------------------------------------------

#ifndef DOOMTYPE_H
#define DOOMTYPE_H
// General type defines, not dependent upon doomdef.h

#include <stdint.h>

#if defined( __WATCOMC__) && defined( _M_I386)
// _M_I386  means 32bit Intel
#ifndef WIN32
# define WIN32
#endif
#ifndef __WIN32__
# define __WIN32__
#endif
#endif

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
#if defined( __APPLE_CC__ ) && ! defined( __GNUC__ )
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
typedef uint32_t   tic_t;

// Return values
// Positive are positive indicators
// 0 is NULL result
// Negative are failure indicators
typedef enum {
    FAIL = -1,   // general fail
    FAIL_end = -2,  // end of valid input
    FAIL_invalid_input = -3,  // invalid input to the function
    FAIL_create = -100,
    FAIL_memory = -101,
} status_return_e;


#ifdef __APPLE_CC__
  // Apple GNAT, GNU C 4.5, __APPLE_CC__ == 1
  // Apple C 4.2, __APPLE_CC__ == 5666
  //   They define __MACH__, __GNUC__, and conditionally __BIG_ENDIAN__
  //   Do not use __LITTLE_ENDIAN__, it is not defined on WIN, LINUX
# ifdef SDL
   // Mac on SDL, is like Linux
   // Can also test for __APPLE_CC__ or __MACH__
#  define MAC_SDL
# else
   // Hardware direct interface using macos directory (NOT SDL)
#  define MACOS_DI
#  define DEBUG_LOG
#  ifndef HWRENDER
#   define HWRENDER
#  endif
# endif
#endif

#ifdef __GNUC__
#define PACKED_ATTR  __attribute__((packed))
#else
#define PACKED_ATTR
#endif

#ifdef WIN32
# define ASMCALL __cdecl
#else
# define ASMCALL
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
		
    #ifdef __DJGPP__
        #include <dos.h>
        #include <sys\types.h>
        #include <direct.h>
        #include <malloc.h>
        #define strcmpi                 strcasecmp
    #endif		
#endif

#if 0
// [WDJ] changed all stricmp to strcasecmp
// [smite] we should use standard funcs
#define stricmp(x,y) strcasecmp(x,y)
#define strnicmp(x,y,n) strncasecmp(x,y,n)
#endif


#if !defined(WIN32) && !defined(__WINDOWS__)
#define min(x,y) ( ((x)<(y)) ? (x) : (y) )
#define max(x,y) ( ((x)>(y)) ? (x) : (y) )

int strupr(char *n);
int strlwr(char *n);
#endif

#ifndef O_BINARY
#define O_BINARY 0 // stupid windows text files
#endif


// Predefined with some OS.
#ifdef __WIN32__
#include <limits.h>
#elif defined( MACOS_DI ) || defined( __MACH__ ) || defined( FREEBSD )
#include <limits.h>
#else
// Linux GNU, which also includes limits.h
// obsolete header file
#include <values.h>
//#include <limits.h>
#endif

// [WDJ] This is very dangerous considering 32 bit and 64 bit systems,
// should use stdint.h values instead.
// These are obsolete defines from values.h.
#ifndef MAXCHAR
// unused
#define MAXCHAR   ((char)0x7f)
#endif

#ifndef MAXSHORT
// defined in values.h
// used in r_segs.c
#define MAXSHORT  ((short)0x7fff)
#endif

#ifndef MAXINT
// defined in values.h
// used in many places
#define MAXINT    ((int)0x7fffffff)
#endif

#ifndef MINCHAR
// unused
#define MINCHAR   ((char)0x80)
#endif

#ifndef MINSHORT
// defined in values.h
// unused
#define MINSHORT  ((short)0x8000)
#endif

#ifndef MININT
// defined in values.h
// used in many places
#define MININT    ((int)0x80000000)
#endif

typedef union {
    uint32_t  rgba;
    struct {
        byte  red;
        byte  green;
        byte  blue;
        byte  alpha;
    } s;
} RGBA_t;

#ifdef __BIG_ENDIAN__
    // __BIG_ENDIAN__ is defined on MAC compilers, not on WIN, nor LINUX
#define UINT2RGBA(a) a
#else
#define UINT2RGBA(a) ((a&0xff)<<24)|((a&0xff00)<<8)|((a&0xff0000)>>8)|(((ULONG)a&0xff000000)>>24)
#endif

#endif  //__DOOMTYPE__
