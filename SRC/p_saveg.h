// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: p_saveg.h 628 2010-04-03 02:23:33Z wesleyjohnson $
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
// $Log: p_saveg.h,v $
// Revision 1.4  2000/11/11 13:59:45  bpereira
// no message
//
// Revision 1.3  2000/11/02 17:50:08  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Savegame I/O, archiving, persistence.
//
//-----------------------------------------------------------------------------


#ifndef __P_SAVEG__
#define __P_SAVEG__


#ifdef __GNUG__
#pragma interface
#endif


// Persistent storage/archiving.
// These are the load / save game routines.

// Allocate malloc an appropriately sized buffer
byte *  P_Alloc_savebuffer( boolean header, boolean data );

// return -1 if overrun the buffer
size_t  P_Savegame_length( void );

// Write savegame header to savegame buffer.
void	P_Write_Savegame_Header( const char * description );

typedef struct {
   char * name;	// description
   char * game; // game name
   char * wad;  // wad name containing level
   char * map;  // level MAP01, E1M2, etc.
   char * levtime; // time into this level, as string
   boolean  have_game, have_wad;  // validity checks on read
   char   msg[60];  // currently uses 46 chars max
} savegame_info_t;

// Read savegame header from savegame buffer.
// Returns header info in infop, as pointers into savebuffer.
// Returns 1 when header is correct.
boolean P_Read_Savegame_Header( savegame_info_t * infop );

void    P_SaveGame (void);
boolean P_LoadGame (void);

extern byte*		savebuffer;
extern byte*            save_p; 
extern int		savebuffer_size;

#endif
