// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: doomdata.h 1356 2017-07-29 18:37:15Z wesleyjohnson $
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2011 by DooM Legacy Team.
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
// $Log: doomdata.h,v $
// Revision 1.6  2003/07/14 21:19:40  hurdler
//
// Revision 1.5  2002/07/24 19:03:09  ssntails
// Added support for things to retain spawned Z position.
//
// Revision 1.4  2000/11/02 17:50:06  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.3  2000/04/04 00:32:45  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      all external data is defined here
//      most of the data is loaded into different structures at run time
//      some internal structures shared by many modules are here
//
//-----------------------------------------------------------------------------

#ifndef DOOMDATA_H
#define DOOMDATA_H

// The most basic types we use, portability.
#include "doomtype.h"

// Some global defines, that configure the game.
#include "doomdef.h"


//
// Map level types.
// The following data structures define the persistent format
// used in the lumps of the WAD files.
//

// Lump order in a map WAD: each map needs a couple of lumps
// to provide a complete scene geometry description.
enum
{
  ML_LABEL,             // A separator, name, ExMx or MAPxx
  ML_THINGS,            // Monsters, items..
  ML_LINEDEFS,          // LineDefs, from editing
  ML_SIDEDEFS,          // SideDefs, from editing
  ML_VERTEXES,          // Vertices, edited and BSP splits generated
  ML_SEGS,              // LineSegs, from LineDefs split by BSP
  ML_SSECTORS,          // SubSectors, list of LineSegs
  ML_NODES,             // BSP nodes
  ML_SECTORS,           // Sectors, from editing
  ML_REJECT,            // LUT, sector-sector visibility        
  ML_BLOCKMAP,          // LUT, motion clipping, walls/grid element
  ML_BEHAVIOR,          // Hexen
};


// A single Vertex.
// WAD lump structure
typedef struct
{
  int16_t       x;
  int16_t       y;
} mapvertex_t;


// A SideDef, defining the visual appearance of a wall,
// by setting textures and offsets.
// WAD lump structure
typedef struct
{
  int16_t       textureoffset;
  int16_t       rowoffset;
  char          toptexture[8];		// 8 char name
  char          bottomtexture[8];	// 8 char name
  char          midtexture[8];		// 8 char name
  // Front sector, towards viewer.
  uint16_t       sector;
} mapsidedef_t;



// A LineDef, as used for editing, and as input
// to the BSP builder.
// WAD lump structure
typedef struct
{
  uint16_t      v1, v2;
  int16_t       flags;
  int16_t       special;
  uint16_t      tag;
  uint16_t      sidenum[2]; // sidenum[1] will be NULL_INDEX if one sided
} maplinedef_t;


//
// LineDef attributes.
//

// Solid, is an obstacle.
#define ML_BLOCKING             1

// Blocks monsters only.
#define ML_BLOCKMONSTERS        2

// Backside will not be present at all
//  if not two sided.
#define ML_TWOSIDED             4

// If a texture is pegged, the texture will have
// the end exposed to air held constant at the
// top or bottom of the texture (stairs or pulled
// down things) and will move with a height change
// of one of the neighbor sectors.
// Unpegged textures allways have the first row of
// the texture at the top pixel of the line for both
// top and bottom textures (use next to windows).

// upper texture unpegged
#define ML_DONTPEGTOP           8

// lower texture unpegged
#define ML_DONTPEGBOTTOM        16      

// In AutoMap: don't map as two sided: IT'S A SECRET!
#define ML_SECRET               32

// Sound rendering: don't let sound cross two of these.
#define ML_SOUNDBLOCK           64

// Don't draw on the automap at all.
#define ML_DONTDRAW             128

// Set if already seen, thus drawn in automap.
#define ML_MAPPED               256

//SoM: 3/29/2000: If flag is set, the player can use through it.
#define ML_PASSUSE              512

//SoM: 4/1/2000: If flag is set, anything can trigger the line.
#define ML_ALLTRIGGER           1024



// Sector definition, from editing.
// WAD lump structure
typedef struct
{
  int16_t       floorheight;
  int16_t       ceilingheight;
  char          floorpic[8];	// 8 char name
  char          ceilingpic[8];  // 8 char name
  int16_t       lightlevel;
  int16_t       special;
  uint16_t      tag;
} mapsector_t;

// SubSector, as generated by BSP.
// WAD lump structure
typedef struct
{
  uint16_t       numsegs;
  // Index of first one, segs are stored sequentially.
  uint16_t       firstseg;       
} mapsubsector_t;


// LineSeg, generated by splitting LineDefs
// using partition lines selected by BSP builder.
// WAD lump structure
typedef struct
{
  uint16_t      v1, v2;
  uint16_t      angle;          
  uint16_t      linedef;
  int16_t       side;
  int16_t       offset;
} mapseg_t;



// BSP node structure.

// Indicate a leaf.
#define NF_SUBSECTOR    0x8000

// WAD lump structure
typedef struct
{
  // Partition line from (x,y) to x+dx,y+dy)
  int16_t       x;
  int16_t       y;
  int16_t       dx;
  int16_t       dy;

  // Bounding box for each child,
  // clip against view frustum.
  int16_t       bbox[2][4];

  // If NF_SUBSECTOR its a subsector,
  // else it's a node of another subtree.
  uint16_t      children[2];

} mapnode_t;




// Thing definition, position, orientation and type,
// plus skill/visibility flags and attributes.
// WAD lump structure, almost.  P_LoadThings has actual WAD struct.
typedef struct
{
    int16_t             x;
    int16_t             y;
    int16_t             z; // Z support for objects SSNTails 07-24-2002
    int16_t             angle;  // normally (0,90,180,270), reported neg sometimes
    int16_t             type;  // DoomEd id number
    int16_t             options;  // flags
    struct mobj_s*      mobj;  // Extra MapThing, voodoo, and FS tests
} mapthing_t;


extern char *Color_Names[NUMSKINCOLORS];


#endif  // DOOMDATA_H
