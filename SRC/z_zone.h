// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: z_zone.h 593 2010-02-07 17:58:53Z wesleyjohnson $
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
// $Log: z_zone.h,v $
// Revision 1.9  2001/03/13 22:14:20  stroggonmeth
// Long time no commit. 3D floors, FraggleScript, portals, ect.
//
// Revision 1.8  2000/11/02 17:50:10  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.7  2000/10/08 13:30:01  bpereira
// no message
//
// Revision 1.6  2000/10/04 16:19:24  hurdler
// Change all those "3dfx names" to more appropriate names
//
// Revision 1.5  2000/10/02 18:25:46  bpereira
// no message
//
// Revision 1.4  2000/07/01 09:23:49  bpereira
// no message
//
// Revision 1.3  2000/04/30 10:30:10  bpereira
// no message
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Zone Memory Allocation, perhaps NeXT ObjectiveC inspired.
//      Remark: this was the only stuff that, according
//       to John Carmack, might have been useful for
//       Quake.
//
//---------------------------------------------------------------------



#ifndef __Z_ZONE__
#define __Z_ZONE__

#include <stdio.h>

//
// ZONE MEMORY
// PU - purge tags.
// Order is important, inequality tests are used.
typedef enum
{
// Internal use tags, do NOT use for user allocations
  PU_FREE = 0, // free, unallocated block
  PU_ZONE,     // head of a zone allocation, exclude from some checks
  PU_INVALID,  // no longer a valid memory block
// Do not move PU_INVALID, it is used to protect tags below it.

// User allocation tags
// Non-purgable tags.
  PU_STATIC,   // static entire execution time
  PU_SOUND,    // static while playing
  PU_MUSIC,    // static while playing
  PU_DAVE,     // anything else Dave wants static
  PU_HWRPATCHINFO,      // Hardware GlidePatch_t struct for OpenGl/Glide texture cache
  PU_HWRPATCHCOLMIPMAP, // Hardware GlideMipmap_t struct colormap variation of patch
// Tags that convert to PU_CACHE at level exit.
// Will not override more restrictive existing tag.
  PU_LUMP,      // Generic temp Lump.
  PU_IN_USE,    // Temp in use, user degraded to
		// PU_CACHE using Z_ChangeTags_To.
// Tags purged at level exit.
  PU_LEVEL,    // static until level exited
  PU_LEVSPEC,  // a special thinker in a level
  PU_HWRPLANE,

// Tags >= PU_PURGELEVEL are purgable whenever needed.
  PU_PURGELEVEL, // Tags >= PU_PURGELEVEL are purgable whenever needed.
  PU_PRIV_CACHE,
  PU_HWRCACHE,   // 'second-level' cache for graphics stored in hardware format and downloaded as needed
  PU_CACHE,
  PU_STALE_CACHE,	// not referenced recently
     
// Tag param, conditional on existing tag
  PU_CACHE_DEFAULT	// set to PU_CACHE, but not when
     			// already < PU_PURGELEVEL
} memtag_e;

//#define ZDEBUG


void    Z_Init (void);
void    Z_FreeTags(memtag_e lowtag, memtag_e hightag);
void    Z_DumpHeap(memtag_e lowtag, memtag_e hightag);
void    Z_FileDumpHeap (FILE *f);
void    Z_CheckHeap (int i);
void    Z_ChangeTag2 (void *ptr, memtag_e tag);

// Change all allocations of old_tag to new_tag.
void	Z_ChangeTags_To( memtag_e old_tag, memtag_e new_tag );

// returns number of bytes allocated for one tag type
int     Z_TagUsage(memtag_e tag);

void    Z_FreeMemory (int *realfree, int *cachemem, int *usedmem, int *largefreeblock);

#ifdef ZDEBUG
#define Z_Free(p) Z_Free2(p,__FILE__,__LINE__)
void    Z_Free2 (void *ptr,char *file,int line);
#define Z_Malloc(s,t,p) Z_Malloc2(s,t,p,0,__FILE__,__LINE__)
#define Z_MallocAlign(s,t,p,a) Z_Malloc2(s,t,p,a,__FILE__,__LINE__)
void*   Z_Malloc2 (int reqsize, memtag_e tag, void **user, int alignbits, char *file,int line);
#else
void    Z_Free (void *ptr);
void*   Z_MallocAlign(int reqsize, memtag_e tag, void **user, int alignbits);
#define Z_Malloc(s,t,p) Z_MallocAlign(s,t,p,0)
#endif

char *Z_Strdup(const char *s, memtag_e tag, void **user);


typedef struct memblock_s
{
   // [WDJ] only works for int >= 32bit, or else havoc in Z_ALLOC
    int                 size;   // including the header and possibly tiny fragments
    memtag_e            tag;    // purgelevel
    int                 id;     // should be ZONEID
    void**              user;   // NULL if a free block
    struct memblock_s*  next;
    struct memblock_s*  prev;
#ifdef ZDEBUG
    char             *ownerfile;
    int               ownerline; 
#endif
} memblock_t;

//
// This is used to get the local FILE:LINE info from CPP
// prior to really call the function in question.
//
#ifdef PARANOIA
#define Z_ChangeTag(p,t) \
{ \
      if (( (memblock_t *)( (byte *)(p) - sizeof(memblock_t)))->id!=0x1d4a11) \
          I_Error("Z_CT at "__FILE__":%i",__LINE__); \
      Z_ChangeTag2(p,t); \
};
#else
#define Z_ChangeTag(p,t)  Z_ChangeTag2(p,t)
#endif

#endif
