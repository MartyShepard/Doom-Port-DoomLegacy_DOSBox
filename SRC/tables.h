// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: tables.h 1458 2019-09-11 12:27:47Z wesleyjohnson $
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

#ifndef TABLES_H
#define TABLES_H

#include "m_fixed.h"

// Binary Angle as fraction of PI*2, unsigned, wrap at 0.
typedef uint32_t angle_t;
typedef int32_t  signed_angle_t;  // for angle differences

extern const angle_t ANG45;  // 0x20000000;
extern const angle_t ANG90;  // 0x40000000;
extern const angle_t ANG180; // 0x80000000;
extern const angle_t ANG270; // 0xc0000000;

extern const angle_t ANGLE_MAX; // 0xffffffff;
extern const angle_t ANGLE_1;   // 0x20000000 / 45;
extern const angle_t ANGLE_60;  // 0x80000000 / 3;

// convert signed wad angle to unsigned angle_t
static inline angle_t wad_to_angle( fixed_t wad_angle )
{
  // wad angle in signed degrees (mapthing_t), should be (0,90,180,270)
  // other possibilities will cause angle_t to wrap
  return ANG45 * (wad_angle / 45);
}

/// Absolute value of angle difference, always in [0, ANG180].
static inline angle_t Abs(angle_t a)
{
  return (a <= ANG180) ? a : -a; // -a is effectively 2pi - a since it wraps
};

// FINE ANGLES are for lookup in the fine tables.
// They are finer than degrees, but more coarse than ANGLES.
#define FINEANGLES              8192
//#define FINEMASK                (FINEANGLES-1)
#define FINEMASK                0x1FFF
//#define FINE_ANG180             (FINEANGLES/2)
#define FINE_ANG180             4096
//#define FINE_ANG90              (FINEANGLES/4)
#define FINE_ANG90              2048

// Shift ANGLE (0xFFFFFFFF) so it fits within FINEANGLES (0x1FFF), (32-13)bits
#define ANGLETOFINESHIFT        19

// Does not need FINEMASK when a is unsigned, and angle_t is unsigned.
#define ANGLE_TO_FINE(a)    (((unsigned int)(a))>>ANGLETOFINESHIFT)

// Treat angle as signed, -ANG180 to +ANG180.
// Requires use of FINEMASK before use as index.
#define SIGNED_ANGLE_TO_FINE(a)   (((int)(a))>>ANGLETOFINESHIFT)

// Effective size is 10240. [5*FINEANGLES/4]
// Index Range is 0 to FINEANGLES-1.
// Overlapped cosine table extends that to (FINEANGLES + FINE_ANG90).
extern const fixed_t finesine[5*FINEANGLES/4];
// Range is 0 to ANGLE_MAX.
// Overlapped cosine table extends that to ANGLE_MAX + ANG90.
#define sine_ANG(a)  (finesine[ ANGLE_TO_FINE(a) ])

// Table of finesine and finecosine overlap, with PI/2 phase shift.
// Index Range is 0 to FINEANGLES-1.
extern const fixed_t* const finecosine;
// Range is 0 to ANGLE_MAX.
#define cosine_ANG(a)  (finecosine[ ANGLE_TO_FINE(a) ])

// Effective size is 4096, (0 to FINE_ANG180). [FINEANGLES/2]
// The table content index Range is -FINE_ANG90 to FINE_ANG90  (signed).
// Must add FINE_ANG90 to signed angle before indexing table, as in tangent_ANG.
extern const fixed_t finetangent[FINEANGLES/2];
// Range is ANG_270 to ANG_90, as signed angle.
#define tangent_ANG(a)  (finetangent[ ((((int)(a))>>ANGLETOFINESHIFT) + FINE_ANG90 ) & FINEMASK ])

// Encapsulation for tabulated sine, cosine and tangent
// [WDJ] Do not use names that only differ in cap from lib names, as it
// makes it difficult to distinguish later. ANG are doom angles.
// Unfortunatly, these compile to larger code than the define.
static inline fixed_t sine_ang(angle_t a) { return finesine[ ANGLE_TO_FINE(a) ]; }
static inline fixed_t cosine_ang(angle_t a) { return finecosine[ ANGLE_TO_FINE(a) ]; }
static inline fixed_t tan_ang(angle_t a)
{
  a += ANG90; // wraps around like angles should
  return finetangent[ ANGLE_TO_FINE(a) ];
}


// To get a global angle from cartesian coordinates, the coordinates are
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
int SlopeDiv ( unsigned num, unsigned den);

#endif
