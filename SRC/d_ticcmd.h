// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: d_ticcmd.h 1268 2016-09-20 17:26:51Z wesleyjohnson $
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
// $Log: d_ticcmd.h,v $
// Revision 1.4  2001/03/03 06:17:33  bpereira
// Revision 1.3  2000/08/31 14:30:55  bpereira
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//     On Tick button and command interface stuff.
//
//-----------------------------------------------------------------------------


#ifndef D_TICCMD_H
#define D_TICCMD_H

#include "doomdef.h"
  // CLIENTPREDICTION
#include "doomtype.h"
#include "m_fixed.h"

#ifdef __GNUG__
#pragma interface
#endif


//
// Button/action code definitions.
//

//added:16-02-98: bit of value 64 doesnt seem to be used,
//                now its used to jump

typedef enum
{
    // Press "Fire".
    BT_ATTACK           = 1,
    // Use button, to open doors, activate switches.
    BT_USE              = 2,

    // Flag, weapon change pending.
    // If true, the next 3 bits hold weapon num.
    BT_CHANGE           = 4,
    // The 3bit weapon mask and shift, convenience.
    BT_WEAPONMASK       = (8+16+32),
    BT_WEAPONSHIFT      = 3,

    // Jump button.
    BT_JUMP             = 64,
    BT_EXTRAWEAPON      = 128
} buttoncode_t;


// The data sampled per tick (single player)
// and transmitted to other peers (multiplayer).
// Mainly movements/button commands per game tick,
// plus a checksum for internal state consistency.

// bits in angleturn
#define TICCMD_RECEIVED 1      
#define TICCMD_XY       2
#define BT_FLYDOWN      4
typedef struct
{
#ifdef CLIENTPREDICTION2
    fixed_t      x;
    fixed_t      y;
#endif
    int8_t       forwardmove;    // *2048 for move
    int8_t       sidemove;       // *2048 for move
    int16_t      angleturn;      // <<16 for angle delta
                                 // SAVED AS A BYTE into demos
    int16_t      aiming;    // pitch angle (up-down)
    byte         buttons;
} ticcmd_t;


#endif
