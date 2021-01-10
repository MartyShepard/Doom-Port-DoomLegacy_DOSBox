// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: byteptr.h 557 2009-11-14 19:45:32Z smite-meister $
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
// Revision 1.6  2003/05/04 02:26:39  sburke
// Fix problems in adapting to __BIG_ENDIAN__ machines.
//
// Revision 1.5  2000/10/21 08:43:28  bpereira
// no message
//
// Revision 1.4  2000/04/16 18:38:06  bpereira
// no message
//
//
// DESCRIPTION:
//    Macro to read/write from/to a char*, used for packet cration and such...
//
//-----------------------------------------------------------------------------

#include "m_swap.h"


// TODO FIXME the reliance on specific sizes for longs, shorts etc in this file is like asking for horrible bugs


static inline int16_t read_16(byte **p)
{
  int16_t temp = *(int16_t *)*p;
  *p += sizeof(int16_t);
  return LE_SHORT(temp);
}

static inline int32_t read_32(byte **p)
{
  int32_t temp = *(int32_t *)*p;
  *p += sizeof(int32_t);
  return LE_LONG(temp);
}


static inline void write_16(byte **p, int16_t val)
{
  *(int16_t *)*p = LE_SHORT(val);
  *p += sizeof(int16_t);
}

static inline void write_32(byte **p, int32_t val)
{
  *(int32_t *)*p = LE_LONG(val);
  *p += sizeof(int32_t);
}


#define WRITEBYTE(p,b)      *(p)++ = (b)
#define WRITECHAR(p,b)      *(p)++ = (byte)(b)
#define WRITESHORT(p,b)     write_16(&p, b)
#define WRITEUSHORT(p,b)    write_16(&p, b)
#define WRITELONG(p,b)      write_32(&p, b)
#define WRITEULONG(p,b)     write_32(&p, b)
#define WRITEFIXED(p,b)     write_32(&p, b)
#define WRITEANGLE(p,b)     write_32(&p, b)
#define WRITESTRING(p,b)    { int tmp_i=0; do { WRITECHAR((p), (b)[tmp_i]); } while ((b)[tmp_i++]); }
#define WRITESTRINGN(p,b,n) { int tmp_i=0; do { WRITECHAR((p), (b)[tmp_i]); if (!(b)[tmp_i]) break; tmp_i++; } while (tmp_i<(n)); }
#define WRITEMEM(p,s,n)     memcpy((p),(s),(n)); (p)+=(n)


#define READBYTE(p)         *(p)++
#define READCHAR(p)         (char)*(p)++
#define READSHORT(p)          (short)read_16(&p)
#define READUSHORT(p)        (USHORT)read_16(&p)
#define READLONG(p)            (long)read_32(&p)
#define READULONG(p)          (ULONG)read_32(&p)
#define READFIXED(p)        (fixed_t)read_32(&p)
#define READANGLE(p)        (angle_t)read_32(&p)
#define READSTRING(p,s)     { int tmp_i=0; do { (s)[tmp_i] = READBYTE(p); } while ((s)[tmp_i++]); }
#define SKIPSTRING(p)       while(READBYTE(p))
#define READMEM(p,s,n)      memcpy(s, p, n);p+=n
