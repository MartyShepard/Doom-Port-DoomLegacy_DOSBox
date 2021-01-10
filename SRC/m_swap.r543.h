// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: m_swap.h 543 2009-09-27 23:12:40Z smite-meister $
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
// $Log: m_swap.h,v $
// Revision 1.5  2003/05/07 03:03:03  sburke
// Make the SHORT and LONG macros cast their value to short and long.
//
// Revision 1.4  2001/04/17 22:26:07  calumr
// Initial Mac add
//
// Revision 1.3  2001/02/24 13:35:20  bpereira
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
//      Endianess handling, swapping 16bit and 32bit.
//
//-----------------------------------------------------------------------------


#ifndef __M_SWAP__
#define __M_SWAP__

#include <stdint.h>

// WAD files are always little-endian.
// Other files, such as MIDI files, are always big-endian.

#define SWAP_INT16(x) ((int16_t)( \
(((uint16_t)(x) & (uint16_t)0x00ffU) << 8) | \
(((uint16_t)(x) & (uint16_t)0xff00U) >> 8) ))

#define SWAP_INT32(x) ((int32_t)( \
(((uint32_t)(x) & (uint32_t)0x000000ffUL) << 24) | \
(((uint32_t)(x) & (uint32_t)0x0000ff00UL) <<  8) | \
(((uint32_t)(x) & (uint32_t)0x00ff0000UL) >>  8) | \
(((uint32_t)(x) & (uint32_t)0xff000000UL) >> 24) ))

#ifdef __BIG_ENDIAN__
# define LE_SHORT(x) SWAP_INT16(x)
# define LE_LONG(x)  SWAP_INT32(x)
# define BE_SHORT(x) (x)
# define BE_LONG(x)  (x)
#else // little-endian
# define LE_SHORT(x) (x)
# define LE_LONG(x)  (x)
# define BE_SHORT(x) SWAP_INT16(x)
# define BE_LONG(x)  SWAP_INT32(x)
#endif

// TODO FIXME convert all endianness handling to use the code above, remove code below.


#ifdef __BIG_ENDIAN__

#define SHORT(x) ((short)( \
(((unsigned short)(x) & (unsigned short)0x00ffU) << 8) \
| \
(((unsigned short)(x) & (unsigned short)0xff00U) >> 8) )) \

#define LONG(x) ((int)( \
(((unsigned int)(x) & (unsigned int)0x000000ffUL) << 24) \
| \
(((unsigned int)(x) & (unsigned int)0x0000ff00UL) <<  8) \
| \
(((unsigned int)(x) & (unsigned int)0x00ff0000UL) >>  8) \
| \
(((unsigned int)(x) & (unsigned int)0xff000000UL) >> 24) ))

#else
#define SHORT(x)  ((short)x)
#define LONG(x)	  ((long) x)
#endif





#endif