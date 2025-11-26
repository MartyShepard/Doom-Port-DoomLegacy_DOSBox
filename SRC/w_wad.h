// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: w_wad.h 1035 2013-08-14 00:38:40Z wesleyjohnson $
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2012 by DooM Legacy Team.
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
// $Log: w_wad.h,v $
// Revision 1.13  2001/05/16 17:12:52  crashrl
// Added md5-sum support, removed recursiv wad search
//
// Revision 1.12  2001/02/28 17:50:55  bpereira
// Revision 1.11  2001/02/24 13:35:21  bpereira
//
// Revision 1.10  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.9  2000/10/04 16:19:24  hurdler
// Change all those "3dfx names" to more appropriate names
//
// Revision 1.8  2000/09/28 20:57:19  bpereira
// Revision 1.7  2000/08/31 14:30:56  bpereira
// Revision 1.6  2000/04/16 18:38:07  bpereira
// Revision 1.5  2000/04/13 23:47:48  stroggonmeth
// 
// Revision 1.4  2000/04/11 19:07:25  stroggonmeth
// Finished my logs, fixed a crashing bug.
//
// Revision 1.3  2000/04/04 00:32:48  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      WAD I/O functions, wad resource definitions (some).
//
//-----------------------------------------------------------------------------


#ifndef W_WAD_H
#define W_WAD_H

#include "doomtype.h"

#ifdef HWRENDER
#include "hardware/hw_data.h"
#else
typedef void MipPatch_t;
#endif

#ifdef __GNUG__
#pragma interface
#endif


// [WDJ] Indicates cache miss, new lump read requires endian fixing.
extern boolean lump_read;

// ==============================================================
//               WAD FILE STRUCTURE DEFINITIONS
// ==============================================================

// [WDJ] found unused 1/5/2010
//typedef long   lumpnum_t;           // 16:16 long (wad num: lump num)


// header of a wad file

typedef struct
{
    char       identification[4];   // should be "IWAD" or "PWAD"
    int        numlumps;            // how many resources
    uint32_t   infotableofs;        // the 'directory' of resources
} wadinfo_t;


// an entry of the wad directory

typedef struct
{
    uint32_t   filepos;             // file offset of the resource
    uint32_t   size;                // size of the resource
    char       name[8];             // name of the resource
} filelump_t;


// in memory : initted at game startup

typedef struct
{
    char        name[8];            // filelump_t name[]
    uint32_t    position;           // filelump_t filepos
    uint32_t    size;               // filelump_t size
} lumpinfo_t;


// =========================================================================
//                         DYNAMIC WAD LOADING
// =========================================================================

#define WADFILENUM(lump)       (lump>>16)   // wad file number in upper word
#define LUMPNUM(lump)          (lump&0xffff)    // lump number for this pwad

// MAX_WADPATH moved to doomdef.h, for other users
#define MAX_WADFILES  32       // maximum of wad files used at the same time
                               // (there is a max of simultaneous open files
                               // anyway, and this should be plenty)

#define lumpcache_t  void*

typedef struct wadfile_s
{
    char             *filename;
    lumpinfo_t*      lumpinfo;
    lumpcache_t*     lumpcache;
    MipPatch_t*      hwrcache;   // patches are cached in renderer's native format
    int              numlumps;          // this wad's number of resources
    int              handle;
    ULONG            filesize;          // for network
    unsigned char    md5sum[16];
} wadfile_t;

extern  int          numwadfiles;
extern  wadfile_t*   wadfiles[MAX_WADFILES];


// =========================================================================

void W_Shutdown(void);

// load and add a wadfile to the active wad files, return wad file number
// (you can get wadfile_t pointer: the return value indexes wadfiles[])
int     W_LoadWadFile (char *filename);

//added 4-1-98 initmultiplefiles now return 1 if all ok 0 else
//             so that it stops with a message if a file was not found
//             but not if all is ok.
int     W_InitMultipleFiles (char** filenames);
void    W_Reload (void);

int     W_CheckNumForName (char* name);
// this one checks only in one pwad
int     W_CheckNumForNamePwad (char* name, int wadid, int startlump);
int     W_GetNumForName (char* name);

// modified version that scan forwards
// used to get original lump instead of patched using -file
int     W_CheckNumForNameFirst (char* name);
int     W_GetNumForNameFirst (char* name);  

int     W_LumpLength (int lump);
//added:06-02-98: read all or a part of a lump size==0 meen read all
int     W_ReadLumpHeader (int lump, void* dest, int size);
//added:06-02-98: now calls W_ReadLumpHeader() with full lump size
void    W_ReadLump (int lump, void *dest);

void*   W_CacheLumpNum (int lump, int tag);
void*   W_CacheLumpName (char* name, int tag);

void*   W_CachePatchName (char* name, int tag);

void*   W_CachePatchNum (int lump, int tag);                        // return a patch_t
void*   W_CachePatchNum_Endian ( int lump, int tag );
#ifdef HWRENDER
// [WDJ] Called from hardware render for special mapped sprites
void*   W_CacheMappedPatchNum ( int lump, uint32_t drawflags );
#endif

void*   W_CacheRawAsPic( int lump, int width, int height, int tag); // return a pic_t

// Cache and endian convert a pic_t
void*   W_CachePicNum( int lumpnum, int tag );
void*   W_CachePicName( char* name, int tag );

// [WDJ] Return a sum unique to a lump, to detect replacements.
// The lumpptr must be to a Z_Malloc lump.
uint64_t  W_lump_checksum( void* lumpptr );

//SoM: 4/13/2000: Store lists of lumps for F_START/F_END ect.
typedef struct {
  int         wadfile;
  int         firstlump;
  int         numlumps;
} lumplist_t;
                    
void    W_LoadDehackedLumps( int wadnum );                    
#endif // __W_WAD__
