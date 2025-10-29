// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: r_data.h 665 2010-06-03 12:51:23Z wesleyjohnson $
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
// $Log: r_data.h,v $
// Revision 1.6  2001/03/13 22:14:20  stroggonmeth
// Long time no commit. 3D floors, FraggleScript, portals, ect.
//
// Revision 1.5  2000/11/02 17:50:09  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.4  2000/04/13 23:47:47  stroggonmeth
// See logs
//
// Revision 1.3  2000/04/04 00:32:47  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Refresh module, data I/O, caching, retrieval of graphics
//      by name.
//
//-----------------------------------------------------------------------------


#ifndef __R_DATA__
#define __R_DATA__

#include "r_defs.h"
#include "r_state.h"

#ifdef __GNUG__
#pragma interface
#endif


// moved here for r_sky.c (texture_t is used)

//
// Texture definition.
// Each texture is composed of one or more patches,
// with patches being lumps stored in the WAD.
// The lumps are referenced by number, and patched
// into the rectangular texture space using origin
// and possibly other attributes.
//
#if 1
// Used to read texture patch info from wad, sizes must be correct.
typedef struct
{
    // The patches leftoffset and topoffset are ignored.
    int16_t	originx;	// from top left of texture area
    int16_t	originy;
    uint16_t    patchnum;	// index [0..] of the entry in PNAMES
    int16_t	stepdir;	// 1
    uint16_t    colormap;	// 0
} mappatch_t;
#else
// Used for reading texture patches from wad.
typedef struct
{
    short       originx;
    short       originy;
    short       patchnum;
    short       stepdir;
    short       colormap;
} mappatch_t;
#endif



//
// Texture definition.
// A DOOM wall texture is a list of patches
// which are to be combined in a predefined order.
//
#if 1
// Used to read texture lump from wad, sizes must be correct.
// UDS is unclear on the exact size of some of these fields.
typedef struct
{
    char                name[8];
    uint32_t		masked;		// [8] must be 4 bytes
   					// boolean size cannot be trusted
    uint16_t            width;		// [12]
    uint16_t            height;		// [14]
    char                columndirectory[4]; //void **columndirectory; // OBSOLETE 
    uint16_t            patchcount;	// [20]
    mappatch_t		patches[1];	// [22] array
} maptexture_t;
#else
typedef struct
{
    char                name[8];
    boolean             masked;
    short               width;
    short               height;
    char                columndirectory[4]; //void **columndirectory; // OBSOLETE 
    short               patchcount;
    mappatch_t  patches[1];
} maptexture_t;
#endif

// A single patch from a texture definition,
//  basically a rectangular area within
//  the texture rectangle.
#if 1
// Used only in internal texture struct. Wad read uses mappatch_t, which has more fields.
// There is clipping code for originx<0 and originy<0, which occur in doom wads.
// The original doom has a clipping bug when originy < 0.
typedef struct
{
    int32_t     originx;
    int32_t     originy;
    int		patchnum;
} texpatch_t;
#else
typedef struct
{
    // Block origin (allways UL),
    // which has already accounted
    // for the internal origin of the patch.
    int         originx;
    int         originy;
    int         patchnum;
} texpatch_t;
#endif

// [WDJ] 2/8/2010
typedef enum {
   TM_none,
   TM_patch,	// original single patch texture  (has draw)
   TM_picture,	// drawn into picture buffer  (has draw)
   TM_combine_patch,  // transparent combined multi-patch texture  (has draw)
   TM_multi_patch, // original multi-patch texture
   TM_masked,   // detect masked flag (hint)
   TM_invalid	// disabled for some internal reason
} texture_model_e;

// A maptexturedef_t describes a rectangular texture,
//  which is composed of one or more mappatch_t structures
//  that arrange graphic patches.
//  Internal structure, maptexture_t is used for reading wad.
typedef struct
{
    // Keep name for switch changing, etc.
    char        name[8];
    short       width;
    short       height;
    texture_model_e  texture_model;	// [WDJ] drawing and storage models

    // All the patches[patchcount]
    //  are drawn back to front into the cached texture.
    short       patchcount;
    texpatch_t  patches[1];

} texture_t;


// All loaded and prepared textures from the start of the game,
// Contains info from the TEXTURE1 and TEXTURE2 lumps.
// [WDJ] Original Doom bug: conflict between texture[0] and 0=no-texture.
// Their solution was to not use the first texture.
// See BUGFIX_TEXTURE0 in r_data.c.
//   array[ 0.. numtextures-1 ] of texture_t*,
//   but [0] is unusable because it conflicts with 0=no-texture.
extern texture_t**     textures;

//extern lighttable_t    *colormaps;
extern CV_PossibleValue_t Color_cons_t[];

// Load TEXTURE1/TEXTURE2/PNAMES definitions, create lookup tables
void  R_LoadTextures (void);
void  R_FlushTextureCache (void);

// Generate a texture from texture desc. and patches.
byte* R_GenerateTexture (int texnum);

// Retrieve column data for span blitting.
byte* R_GetColumn (int texnum, int col);

byte* R_GetFlat (int  flatnum);

// I/O, setting up the stuff.
void R_InitData (void);
void R_PrecacheLevel (void);


// Retrieval.
// Floor/ceiling opaque texture tiles,
// lookup by name. For animation?
int R_GetFlatNumForName (char *name);
int P_FlagNumForName (char *flatname);
#define R_FlatNumForName(x)    R_GetFlatNumForName(x)


// Called by P_Ticker for switches and animations,
// returns the texture number for the texture name.
int R_TextureNumForName (char *name);
int R_CheckTextureNumForName (char *name);


void R_ClearColormaps();
int R_ColormapNumForName(char *name);

// The colorstr is the toptexture name.
// The ctrlstr is the midtexture name.
// The fadestr is the bottomtexture name.
int R_CreateColormap(char *colorstr, char *ctrlstr, char *fadestr);

char *R_ColormapNameForNum(int num);
#endif
