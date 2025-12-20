// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: d_think.h 1325 2017-05-23 14:29:10Z wesleyjohnson $
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
// $Log: d_think.h,v $
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//  Mobj, and some mapthings, as thinkers.
//  MapObj data. Map Objects or mobjs are actors, entities,
//  thinker, take-your-pick... anything that moves, acts, or
//  suffers state changes of more or less violent nature.
//
//-----------------------------------------------------------------------------

#ifndef D_THINK_H
#define D_THINK_H


#ifdef __GNUG__
#pragma interface
#endif


//
// Experimental stuff.
// To compile this as "ANSI C with classes"
//  we will need to handle the various
//  action functions cleanly.
//
typedef  void (*actionf_v)();
typedef  void (*actionf_p1)( void* );
typedef  void (*actionf_p2)( void*, void* );

typedef union
{
  actionf_v     acv;
  actionf_p1    acp1;
  actionf_p2    acp2;

} actionf_t;





// Historically, "think_t" is yet another
//  function pointer to a routine to handle
//  an actor.
typedef actionf_t  think_t;


// Doubly linked list of actors.
typedef struct thinker_s
{
    struct thinker_s*   prev;
    struct thinker_s*   next;
    think_t             function;

    // killough 8/29/98: maintain thinkers in several equivalence classes,
    // according to various criteria, so as to allow quicker searches.
    struct thinker_s *  cnext, *cprev; // linked list thinkers in same class

#ifdef REFERENCE_COUNTING
    // killough 11/98: count of how many other objects reference
    // this one using pointers. Used for garbage collection.
    uint16_t  references;
#endif
} thinker_t;



#endif
