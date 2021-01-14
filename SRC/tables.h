// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: tables.h 645 2010-05-11 21:47:18Z wesleyjohnson $
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2010 by DooM Legacy Team.
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
//-----------------------------------------------------------------------------

#ifndef __TABLES__
#define __TABLES__

#include "m_fixed.h"

// Binary Angle in fixed point
typedef uint32_t angle_t;

extern const angle_t ANG45;  // 0x20000000;  
extern const angle_t ANG90;  // 0x40000000;
extern const angle_t ANG180; // 0x80000000;
extern const angle_t ANG270; // 0xc0000000;

extern const angle_t ANGLE_MAX; // 0xffffffff;
extern const angle_t ANGLE_1;   // 0x20000000 / 45;
extern const angle_t ANGLE_60;  // 0x80000000 / 3;

/// Absolute value of angle difference, always in [0, ANG180].
static inline angle_t Abs(angle_t a)
{
  return (a <= ANG180) ? a : -a; // -a is effectively 2pi - a since it wraps
};


#define FINEANGLES              8192
#define FINEMASK                (FINEANGLES-1)
#define ANGLETOFINESHIFT        19      // 0x100000000 to 0x2000

// Effective size is 10240. [5*FINEANGLES/4]
extern const fixed_t finesine[5*FINEANGLES/4];

// Re-use data, is just PI/2 phase shift.
extern const fixed_t* const finecosine;

// Effective size is 4096. [FINEANGLES/2]
extern const fixed_t finetangent[FINEANGLES/2];

/// Encapsulation for tabulated sine, cosine and tangent
static inline fixed_t Sin(angle_t a) { return finesine[a >> ANGLETOFINESHIFT]; }
static inline fixed_t Cos(angle_t a) { return finecosine[a >> ANGLETOFINESHIFT]; }
static inline fixed_t Tan(angle_t a)
{
  a += ANG90; // wraps around like angles should
  return finetangent[a >> ANGLETOFINESHIFT];
}


// to get a global angle from cartesian coordinates, the coordinates are
// flipped until they are in the first octant of the coordinate system, then
// the y (<=x) is scaled and divided by x to get a tangent (slope) value
// which is looked up in the tantoangle[] table.
#define SLOPERANGE  2048
#define SLOPEBITS   11
#define DBITS       (FRACBITS-SLOPEBITS)

// The +1 size is to handle the case when x==y without additional checking.
extern const angle_t tantoangle[SLOPERANGE+1];

/// Encapsulation for arctangent (for the range 0 <= x <= 1)
static inline angle_t ArcTan(fixed_t x) { return tantoangle[x >> DBITS]; }


// Utility function, called by R_PointToAngle.
int SlopeDiv ( unsigned      num,
               unsigned      den);


#endif
