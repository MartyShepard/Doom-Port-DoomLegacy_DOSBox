// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: p_tick.h 1035 2013-08-14 00:38:40Z wesleyjohnson $
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
// $Log: p_tick.h,v $
// Revision 1.3  2000/10/21 08:43:31  bpereira
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      
//
//-----------------------------------------------------------------------------


#ifndef P_TICK_H
#define P_TICK_H

#include "doomtype.h"
  // tic_t

#ifdef __GNUG__
#pragma interface
#endif

extern tic_t leveltime;

// Called by C_Ticker,
// can call G_PlayerExited.
// Carries out all thinking of monsters and players.
void P_Ticker (void);


#endif
