// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: p_setup.c 1245 2016-08-04 14:21:00Z wesleyjohnson $
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2012 by DooM Legacy Team.
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
// $Log: p_setup.c,v $
// Include: DOS DJGPP Fixes
//
// Revision 1.49  2004/07/27 08:19:37  exl
// New fmod, fs functions, bugfix or 2, patrol nodes
//
// Revision 1.48  2003/06/11 03:38:09  ssntails
// THING Z definable in levels by using upper 9 bits
//
// Revision 1.47  2003/06/11 00:28:49  ssntails
// Big Blockmap Support (128kb+ ?)
//
// Revision 1.46  2003/05/04 02:37:47  sburke
// READSHORT now does byte-swapping on big-endian machines.
//
// Revision 1.45  2002/10/30 23:50:03  bock
//
// Revision 1.44  2002/09/27 16:40:09  tonyd
// First commit of acbot
//
// Revision 1.43  2002/07/24 19:03:07  ssntails
// Added support for things to retain spawned Z position.
//
// Revision 1.42  2002/07/20 03:24:45  mrousseau
// Copy 'side' from SEGS structure to seg_t's copy
//
// Revision 1.41  2002/01/12 12:41:05  hurdler
// Revision 1.40  2002/01/12 02:21:36  stroggonmeth
// Revision 1.39  2001/08/19 20:41:03  hurdler
//
// Revision 1.38  2001/08/13 16:27:44  hurdler
// Added translucency to linedef 300 and colormap to 3d-floors
//
// Revision 1.37  2001/08/12 22:08:40  hurdler
// Add alpha value for 3d water
//
// Revision 1.36  2001/08/12 17:57:15  hurdler
// Beter support of sector coloured lighting in hw mode
//
// Revision 1.35  2001/08/11 15:18:02  hurdler
// Add sector colormap in hw mode (first attempt)
//
// Revision 1.34  2001/08/08 20:34:43  hurdler
// Big TANDL update
//
// Revision 1.33  2001/08/06 23:57:09  stroggonmeth
// Removed portal code, improved 3D floors in hardware mode.
//
// Revision 1.32  2001/07/28 16:18:37  bpereira
// Revision 1.31  2001/06/16 08:07:55  bpereira
// Revision 1.30  2001/05/27 13:42:48  bpereira
//
// Revision 1.29  2001/05/14 19:02:58  metzgermeister
//   * Fixed floor not moving up with player on E3M1
//   * Fixed crash due to oversized string in screen message ... bad bug!
//   * Corrected some typos
//   * fixed sound bug in SDL
//
// Revision 1.28  2001/04/30 17:19:24  stroggonmeth
// HW fix and misc. changes
//
// Revision 1.27  2001/03/30 17:12:51  bpereira
//
// Revision 1.26  2001/03/19 21:18:48  metzgermeister
//   * missing textures in HW mode are replaced by default texture
//   * fixed crash bug with P_SpawnMissile(.) returning NULL
//   * deep water trick and other nasty thing work now in HW mode (tested with tnt/map02 eternal/map02)
//   * added cvar gr_correcttricks
//
// Revision 1.25  2001/03/13 22:14:19  stroggonmeth
// Long time no commit. 3D floors, FraggleScript, portals, ect.
//
// Revision 1.24  2001/01/25 22:15:43  bpereira
// added heretic support
//
// Revision 1.23  2000/11/04 16:23:43  bpereira
// Revision 1.22  2000/11/03 03:27:17  stroggonmeth
// Revision 1.21  2000/11/02 19:49:36  bpereira
//
// Revision 1.20  2000/11/02 17:50:08  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.19  2000/10/02 18:25:45  bpereira
// Revision 1.18  2000/08/31 14:30:56  bpereira
//
// Revision 1.17  2000/08/11 21:37:17  hurdler
// fix win32 compilation problem
//
// Revision 1.16  2000/08/11 19:10:13  metzgermeister
//
// Revision 1.15  2000/05/23 15:22:34  stroggonmeth
// Not much. A graphic bug fixed.
//
// Revision 1.14  2000/05/03 23:51:00  stroggonmeth
//
// Revision 1.13  2000/04/19 15:21:02  hurdler
// add SDL midi support
//
// Revision 1.12  2000/04/18 12:55:39  hurdler
// Revision 1.11  2000/04/16 18:38:07  bpereira
// Revision 1.10  2000/04/15 22:12:57  stroggonmeth
//
// Revision 1.9  2000/04/13 23:47:47  stroggonmeth
// See logs
//
// Revision 1.8  2000/04/12 16:01:59  hurdler
// ready for T&L code and true static lighting
//
// Revision 1.7  2000/04/11 19:07:24  stroggonmeth
// Finished my logs, fixed a crashing bug.
//
// Revision 1.6  2000/04/08 11:27:29  hurdler
// fix some boom stuffs
//
// Revision 1.5  2000/04/06 20:40:22  hurdler
// Mostly remove warnings under windows
//
// Revision 1.4  2000/04/04 19:28:43  stroggonmeth
// Global colormaps working. Added a new linedef type 272.
//
// Revision 1.3  2000/04/04 00:32:47  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Do all the WAD I/O, get map description,
//             set up initial state and misc. LUTs.
//
//-----------------------------------------------------------------------------

#include "doomincl.h"
#include "d_main.h"
#include "byteptr.h"
#include "g_game.h"

#include "p_local.h"
#include "p_setup.h"
#include "p_spec.h"

#include "i_sound.h"
  // I_PlayCD()..
#include "i_system.h"
  // I_Sleep
#include "r_sky.h"

#include "r_data.h"
#include "r_things.h"
#include "r_sky.h"

#include "s_sound.h"
#include "st_stuff.h"
#include "w_wad.h"
#include "z_zone.h"
#include "r_splats.h"
#include "p_info.h"
#include "t_array.h"
#include "t_func.h"
#include "t_script.h"

#include "hu_stuff.h"
#include "console.h"


#ifdef HWRENDER
#include "i_video.h"
  // rendermode
#include "hardware/hw_main.h"
#include "hardware/hw_light.h"
#endif

#include "b_game.h"
  // added by AC for acbot



//
// MAP related Lookup tables.
// Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
//
boolean         newlevel = false;
boolean         doom1level = false;    // doom 1 level running under doom 2

int             numvertexes;
vertex_t*       vertexes;

int             numsegs;
seg_t*          segs;

int             numsectors = 0;
sector_t*       sectors = NULL;

int             numsubsectors;
subsector_t*    subsectors;

int             numnodes;
node_t*         nodes;

int             numlines;
line_t*         lines;

int             numsides;
side_t*         sides;

int             nummapthings;
mapthing_t*     mapthings;

/*
typedef struct mapdata_s {
    int             numvertexes;
    vertex_t*       vertexes;
    int             numsegs;
    seg_t*          segs;
    int             numsectors;
    sector_t*       sectors;
    int             numsubsectors;
    subsector_t*    subsectors;
    int             numnodes;
    node_t*         nodes;
    int             numlines;
    line_t*         lines;
    int             numsides;
    side_t*         sides;
} mapdata_t;
*/


// BLOCKMAP
// Created from axis aligned bounding box
// of the map, a rectangular array of
// blocks of size ...
// Used to speed up collision detection
// by spatial subdivision in 2D.
//
// Blockmap size.
int             bmapwidth;
int             bmapheight;     // size in mapblocks

uint32_t *      blockmapindex;       // for large maps, wad is 16bit
// offsets in blockmap are from here
uint32_t *      blockmaphead; // Big blockmap, SSNTails

// origin of block map
fixed_t         bmaporgx;
fixed_t         bmaporgy;
// for thing chains
mobj_t**        blocklinks;


// REJECT
// For fast sight rejection.
// Speeds up enemy AI by skipping detailed
//  LineOf Sight calculation.
// Without special effect, this could be
//  used as a PVS lookup as well.
//
byte*           rejectmatrix;


// Maintain single and multi player starting spots.
mapthing_t      *deathmatchstarts[MAX_DM_STARTS];
int             numdmstarts;
//mapthing_t**    deathmatch_p;
mapthing_t      *playerstarts[MAXPLAYERS];


//
// P_LoadVertexes
//
void P_LoadVertexes (int lump)
{
    byte*               data;
    int                 i;
    mapvertex_t*        ml;
    vertex_t*           li;

    // Determine number of lumps:
    //  total lump length / vertex record length.
    numvertexes = W_LumpLength (lump) / sizeof(mapvertex_t);

    // Allocate zone memory for buffer.
    vertexes = Z_Malloc (numvertexes*sizeof(vertex_t), PU_LEVEL, NULL);

    // Load data into cache.
    data = W_CacheLumpNum (lump,PU_STATIC);  // vertex lump temp
    // [WDJ] Do endian as read from vertex lump temp

    ml = (mapvertex_t *)data;
    li = vertexes;

    // Copy and convert vertex coordinates,
    // internal representation as fixed.
    for (i=0 ; i<numvertexes ; i++, li++, ml++)
    {
        li->x = LE_SWAP16(ml->x)<<FRACBITS;
        li->y = LE_SWAP16(ml->y)<<FRACBITS;
    }

    // Free buffer memory.
    Z_Free (data);
}


//
// Computes the line length in frac units, the glide render needs this
//

float P_SegLength (seg_t* seg)
{
    double      dx,dy;

    // make a vector (start at origin)
    dx = FIXED_TO_FLOAT(seg->v2->x - seg->v1->x);
    dy = FIXED_TO_FLOAT(seg->v2->y - seg->v1->y);

    return sqrt(dx*dx+dy*dy)*FRACUNIT;
}


//
// P_LoadSegs
//
void P_LoadSegs (int lump)
{
    byte*               data;
    mapseg_t*           ml;
    seg_t*              li;
    line_t*             ldef;
    int                 linedef;
    int                 side, vn1, vn2;
    int                 i;

    // Load segments, from wad, as generated by nodebuilder.
    numsegs = W_LumpLength (lump) / sizeof(mapseg_t);
    segs = Z_Malloc (numsegs*sizeof(seg_t), PU_LEVEL, NULL);
    memset (segs, 0, numsegs*sizeof(seg_t));
    data = W_CacheLumpNum (lump,PU_STATIC);  // segs lump temp
    // [WDJ] Do endian as read from segs lump temp

    ml = (mapseg_t *)data;
    li = segs;
    for (i=0 ; i<numsegs ; i++, li++, ml++)
    {
        // [WDJ] Detect buggy wad, bad vertex number
        vn1 = LE_SWAP16(ml->v1);
        vn2 = LE_SWAP16(ml->v2);
        if( vn1 > numvertexes || vn2 > numvertexes )
        {
            I_SoftError("Seg vertex bad %d,%d\n", vn1,vn2 );
            // zero both out together, make seg safer (otherwise will cross another line)
            vn1 = vn2 = 0;
        }
        li->v1 = &vertexes[vn1];
        li->v2 = &vertexes[vn2];

#ifdef HWRENDER // not win32 only 19990829 by Kin
        // used for the hardware render
        if (rendermode != render_soft)
        {
            li->pv1 = li->pv2 = NULL;
            li->length = P_SegLength (li);
            //Hurdler: 04/12/2000: for now, only used in hardware mode
            li->lightmaps = NULL; // list of static lightmap for this seg
        }
#endif

        li->angle = (LE_SWAP16(ml->angle))<<16;
        li->offset = (LE_SWAP16(ml->offset))<<16;
        linedef = LE_SWAP16(ml->linedef);
        // [WDJ] Detect buggy wad, bad linedef number
        if( linedef > numlines ) {
            I_SoftError( "P_LoadSegs, linedef #%i, > numlines %i\n", linedef, numlines );
            linedef = 0; // default
        }
        ldef = &lines[linedef];
        li->linedef = ldef;
        side = LE_SWAP16(ml->side);
        if( side != 0 && side != 1 )
        {
            // [WDJ] buggy wad
            I_SoftError( "P_LoadSegs, bad side index\n");
            side = 0;  // assume was using wrong side
        }
        // side1 required to have sidenum != NULL_INDEX
        if( ldef->sidenum[side] == NULL_INDEX )
        {
            // [WDJ] buggy wad
            I_SoftError( "P_LoadSegs, using missing sidedef\n");
            side = 0;  // assume was using wrong side
        }
        li->side = side;
        li->sidedef = &sides[ldef->sidenum[side]];
        li->frontsector = sides[ldef->sidenum[side]].sector;
        if (ldef-> flags & ML_TWOSIDED)
            li->backsector = sides[ldef->sidenum[side^1]].sector;
        else
            li->backsector = NULL;

        li->numlights = 0;
        li->rlights = NULL;
    }

    Z_Free (data);
}


//
// P_LoadSubsectors
//
// Called by P_SetupLevel, after LoadLinedefs
void P_LoadSubsectors (int lump)
{
    byte*               data;
    int                 i;
    mapsubsector_t*     ms;
    subsector_t*        ss;

    // Load subsectors, from wad, as generated by nodebuilder.
    numsubsectors = W_LumpLength (lump) / sizeof(mapsubsector_t);
    subsectors = Z_Malloc (numsubsectors*sizeof(subsector_t), PU_LEVEL, NULL);
    data = W_CacheLumpNum (lump,PU_STATIC);  // subsectors lump temp
    // [WDJ] Do endian as read from subsectors temp lump

    ms = (mapsubsector_t *)data;
    memset (subsectors,0, numsubsectors*sizeof(subsector_t));
    ss = subsectors;

    for (i=0 ; i<numsubsectors ; i++, ss++, ms++)
    {
        ss->numlines = LE_SWAP16(ms->numsegs);
        ss->firstline = LE_SWAP16(ms->firstseg);  // unsigned
        // cannot check if valid, segs not loaded yet
    }

    Z_Free (data);
}



//
// P_LoadSectors
//

// Return the flat size_index.
//   flatsize : the flat lump size
// Called by P_PrecacheLevelFlats at level load time.
uint16_t P_flatsize_to_index( int flatsize, char * name )
{
  switch(flatsize)
  {
    case 2048*2048: // 2048x2048 lump
      return 7;
    case 1024*1024: // 1024x1024 lump
      return 6;
    case 512*512: // 512x512 lump
      return 5;
    case 256*256: // 256x256 lump
      return 4;
    case 128*128: // 128x128 lump
      return 3;
    case 64*64: // 64x64 lump
      return 2;
    case 32*32: // 32x32 lump
      return 1;
    default:
      if( verbose && name )
          GenPrintf( EMSG_warn, "Flat size not handled, %s, size=%i\n", name, flatsize );
  }
  return 0;
}

//
// levelflats
//
// usually fewer than 25 flats per level
#define LEVELFLAT_INC   32

unsigned int            levelflat_max = 0;  // num alloc levelflats
unsigned int            numlevelflats;  // actual in use
levelflat_t*            levelflats;

//SoM: Other files want this info.
int P_PrecacheLevelFlats( void )
{
  int flatmemory = 0;
  int i;
  int lump;

  //SoM: 4/18/2000: New flat code to make use of levelflats.
  for(i = 0; i < numlevelflats; i++)
  {
    lump = levelflats[i].lumpnum;
    levelflats[i].size_index = P_flatsize_to_index( W_LumpLength(lump), NULL );
    if(devparm)
      flatmemory += W_LumpLength(lump);
    R_GetFlat (lump);
  }
  return flatmemory;
}


// help function for P_LoadSectors, find a flat in the active wad files,
// allocate an id for it, and set the levelflat (to speedup search)
//
int P_AddLevelFlat ( char* flatname )
{
    union {
        char    s[9];
        int     x[2];
    } name8;

    levelflat_t* lfp;
    int         i;
    int         v1,v2;

    strncpy (name8.s,flatname,8);   // make it two ints for fast compares
    name8.s[8] = 0;                 // in case the name was a fill 8 chars
    strupr (name8.s);               // case insensitive
    v1 = name8.x[0];
    v2 = name8.x[1];

    if( levelflats )
    {
        // scan through the already found flats
        lfp = & levelflats[0];
        for (i=0; i<numlevelflats; i++)
        {
            if ( *(int *)lfp->name == v1
                 && *(int *)&lfp->name[4] == v2)
            {
                goto found_level_flat;  // return i
            }
            lfp ++;
        }
    }

    // create new flat entry in levelflats
    if (devparm)
        GenPrintf(EMSG_dev, "flat %#03d: %s\n", numlevelflats, name8.s);

    if (numlevelflats>=levelflat_max)
    {
        // grow number of levelflats
        // use Z_Malloc directly because it is usually a small number
        levelflat_max += LEVELFLAT_INC;  // alloc more levelflats
        levelflat_t* new_levelflats =
            Z_Malloc (levelflat_max*sizeof(levelflat_t), PU_LEVEL, NULL);
        // must zero because unanimated are left to defaults
        memset( &new_levelflats[numlevelflats], 0,
                (levelflat_max - numlevelflats)*sizeof(levelflat_t) );
        if( levelflats )
        {
            memcpy (new_levelflats, levelflats, numlevelflats*sizeof(levelflat_t));
            Z_Free ( levelflats );
        }
        levelflats = new_levelflats;
    }

    i = numlevelflats;
    lfp = & levelflats[numlevelflats];  // array moved
    numlevelflats++;

    // store the name
    *((int*)lfp->name) = v1;
    *((int*)&lfp->name[4]) = v2;

    // store the flat lump number
    lfp->lumpnum = R_FlatNumForName (flatname);
    lfp->size_index = P_flatsize_to_index( W_LumpLength(lfp->lumpnum), flatname );

 found_level_flat:
    return i;    // level flat id
}


// SoM: Do I really need to comment this?
char * P_FlatNameForNum(int num)
{
  if(num < 0 || num > numlevelflats)
    I_Error("P_FlatNameForNum: Invalid flatnum\n");

  return Z_Strdup(va("%.8s", levelflats[num].name), PU_STATIC, 0);
}


void P_LoadSectors (int lump)
{
    byte*               data;
    int                 i;
    mapsector_t*        ms;
    sector_t*           ss;

    numsectors = W_LumpLength (lump) / sizeof(mapsector_t);
    sectors = Z_Malloc (numsectors*sizeof(sector_t), PU_LEVEL, NULL);
    memset (sectors, 0, numsectors*sizeof(sector_t));
    data = W_CacheLumpNum (lump,PU_STATIC);  // mapsector lump temp
    // [WDJ] Fix endian as transfer from temp to internal.

    // [WDJ] init growing flats array
    numlevelflats = 0;
    levelflat_max = 0;
    levelflats = NULL;

    ms = (mapsector_t *)data;  // ms will be ++
    ss = sectors;
    for (i=0 ; i<numsectors ; i++, ss++, ms++)
    {
        ss->floorheight = LE_SWAP16(ms->floorheight)<<FRACBITS;
        ss->ceilingheight = LE_SWAP16(ms->ceilingheight)<<FRACBITS;

        //
        //  flats
        //
        if( strncasecmp(ms->floorpic,"FWATER",6)==0 || 
            strncasecmp(ms->floorpic,"FLTWAWA1",8)==0 ||
            strncasecmp(ms->floorpic,"FLTFLWW1",8)==0 )
            ss->floortype = FLOOR_WATER;
        else
        if( strncasecmp(ms->floorpic,"FLTLAVA1",8)==0 ||
            strncasecmp(ms->floorpic,"FLATHUH1",8)==0 )
            ss->floortype = FLOOR_LAVA;
        else
        if( strncasecmp(ms->floorpic,"FLTSLUD1",8)==0 )
            ss->floortype = FLOOR_SLUDGE;
        else
            ss->floortype = FLOOR_SOLID;

        ss->floorpic = P_AddLevelFlat (ms->floorpic);
        ss->ceilingpic = P_AddLevelFlat (ms->ceilingpic);

        ss->lightlevel = LE_SWAP16(ms->lightlevel);
        ss->special = LE_SWAP16(ms->special);
        ss->tag = LE_SWAP16(ms->tag);

#ifdef DCK_WATER_TEST
        //added:31-03-98: quick hack to test water with DCK
/*        if (ss->tag < 0)
            CONS_Printf("Level uses dck-water-hack\n");*/
#endif

        ss->thinglist = NULL;
        ss->touching_thinglist = NULL; //SoM: 4/7/2000

        ss->stairlock = 0;
        ss->nextsec = -1;
        ss->prevsec = -1;

        ss->modelsec = -1; //SoM: 3/17/2000: This causes some real problems
                           // [WDJ] Is now dependent upon model
        ss->model = SM_normal; //SoM: 3/20/2000, [WDJ] 11/14/2009
        ss->friction = ORIG_FRICTION;  // normal friction
        ss->movefactor = ORIG_FRICTION_FACTOR;
        ss->floorlightsec = -1;
        ss->ceilinglightsec = -1;
        ss->ffloors = NULL;
        ss->lightlist = NULL;
        ss->numlights = 0;
        ss->attached = NULL;
        ss->numattached = 0;
        ss->extra_colormap = NULL;
        ss->moved = true;  // force init of light lists
        ss->floor_xoffs = ss->ceiling_xoffs = ss->floor_yoffs = ss->ceiling_yoffs = 0;
        ss->bottommap = ss->midmap = ss->topmap = -1;
        
        // ----- for special tricks with HW renderer -----
        ss->pseudoSector = false;
        ss->virtualFloor = false;
        ss->virtualCeiling = false;
        ss->sectorLines = NULL;
        ss->stackList = NULL;
        ss->lineoutLength = -1.0;
        // ----- end special tricks -----
        
    }

    Z_Free (data);

    // whoa! there is usually no more than 25 different flats used per level!!
    //debug_Printf("Load Sectors: %d flats found\n", numlevelflats);

    // set the sky flat num
    skyflatnum = P_AddLevelFlat ("F_SKY1");

    // search for animated flats and set up
    P_SetupLevelFlatAnims ();
}


//
// P_LoadNodes
//
void P_LoadNodes (int lump)
{
    byte*       data;
    int         i;
    int         j;
    int         k;
    mapnode_t*  mn;
    node_t*     no;

    numnodes = W_LumpLength (lump) / sizeof(mapnode_t);
    nodes = Z_Malloc (numnodes*sizeof(node_t), PU_LEVEL, NULL);
    data = W_CacheLumpNum (lump,PU_STATIC);  // mapnode_t array temp
    // [WDJ] Fix endian as transfer from temp to internal.

    mn = (mapnode_t *)data;
    no = nodes;

    for (i=0 ; i<numnodes ; i++, no++, mn++)
    {
        no->x = LE_SWAP16(mn->x)<<FRACBITS;
        no->y = LE_SWAP16(mn->y)<<FRACBITS;
        no->dx = LE_SWAP16(mn->dx)<<FRACBITS;
        no->dy = LE_SWAP16(mn->dy)<<FRACBITS;
        for (j=0 ; j<2 ; j++)
        {
            no->children[j] = LE_SWAP16(mn->children[j]);
            for (k=0 ; k<4 ; k++)
                no->bbox[j][k] = LE_SWAP16(mn->bbox[j][k])<<FRACBITS;
        }
    }

    Z_Free (data);
}

//
// P_LoadThings
//
void P_LoadThings (int lump)
{
  // Doom mapthing template (internally we use a modified version)
  typedef struct
  {
    int16_t   x, y;   ///< coordinates
    int16_t  angle;   ///< orientation
    uint16_t  type;   ///< DoomEd number
    uint16_t flags;
  } doom_mapthing_t;

    int                 i;
    mapthing_t*         mt;
//    boolean             spawn;
    byte               *data;

    data = W_CacheLumpNum (lump,PU_LEVEL);  // temp things lump
    // [WDJ] Do endian as read from temp things lump
    nummapthings     = W_LumpLength(lump) / sizeof(doom_mapthing_t);
    mapthings        = Z_Malloc(nummapthings * sizeof(mapthing_t), PU_LEVEL, NULL);

    //SoM: Because I put a new member into the mapthing_t for use with
    //fragglescript, the format has changed and things won't load correctly
    //using the old method.

    doom_mapthing_t *dmt = (doom_mapthing_t *)data;
    mt = mapthings;
    for (i=0 ; i<nummapthings ; i++, mt++, dmt++)
    {
//        spawn = true;

        // Do spawn all other stuff.
        // SoM: Do this first so all the mapthing slots are filled!
        mt->x = LE_SWAP16(dmt->x);
        mt->y = LE_SWAP16(dmt->y);
        mt->angle   = LE_SWAP16(dmt->angle);
        mt->type    = LE_SWAP16(dmt->type);
        mt->options = LE_SWAP16(dmt->flags);
        mt->mobj = NULL; //SoM:

        if( gamedesc_id == GDESC_tnt && gamemap == 31)
        {
            // Fix TNT MAP31 bug: yellow keycard is multiplayer only
            // Released a fixed copy of TNT later, but CDROM have this bug.
            if( mt->type == 6 )  // Yellow keycard
               mt->options &= ~MTF_MPSPAWN;  // Remove multiplayer only flag
        }

        P_SpawnMapthing (mt);
    }

    Z_Free(data);
}


//
// P_LoadLineDefs
// Also counts secret lines for intermissions.
//
void P_LoadLineDefs (int lump)
{
    byte*               data;
    int                 i;
    maplinedef_t*       mld;
    line_t*             ld;
    vertex_t*           v1;
    vertex_t*           v2;

    numlines = W_LumpLength (lump) / sizeof(maplinedef_t);
    lines = Z_Malloc (numlines*sizeof(line_t), PU_LEVEL, NULL);
    memset (lines, 0, numlines*sizeof(line_t));
    data = W_CacheLumpNum (lump,PU_STATIC);  // temp linedefs array
    // [WDJ] Fix endian as transfer from lump temp to internal.

    mld = (maplinedef_t *)data;
    ld = lines;
    for (i=0 ; i<numlines ; i++, mld++, ld++)
    {
        ld->flags = (uint16_t) LE_SWAP16(mld->flags);
        ld->special = LE_SWAP16(mld->special);
        ld->tag = LE_SWAP16(mld->tag);
        v1 = ld->v1 = &vertexes[ LE_SWAP16(mld->v1) ];
        v2 = ld->v2 = &vertexes[ LE_SWAP16(mld->v2) ];
        ld->dx = v2->x - v1->x;
        ld->dy = v2->y - v1->y;

        if (!ld->dx)
            ld->slopetype = ST_VERTICAL;
        else if (!ld->dy)
            ld->slopetype = ST_HORIZONTAL;
        else
        {
            if (FixedDiv (ld->dy , ld->dx) > 0)
                ld->slopetype = ST_POSITIVE;
            else
                ld->slopetype = ST_NEGATIVE;
        }

        if (v1->x < v2->x)
        {
            ld->bbox[BOXLEFT] = v1->x;
            ld->bbox[BOXRIGHT] = v2->x;
        }
        else
        {
            ld->bbox[BOXLEFT] = v2->x;
            ld->bbox[BOXRIGHT] = v1->x;
        }

        if (v1->y < v2->y)
        {
            ld->bbox[BOXBOTTOM] = v1->y;
            ld->bbox[BOXTOP] = v2->y;
        }
        else
        {
            ld->bbox[BOXBOTTOM] = v2->y;
            ld->bbox[BOXTOP] = v1->y;
        }

        // NULL_INDEX = no sidedef
        ld->sidenum[0] = LE_SWAP16(mld->sidenum[0]);
        ld->sidenum[1] = LE_SWAP16(mld->sidenum[1]);

        // [WDJ] detect common wad errors and make playable, similar to prboom
        if( ld->sidenum[0] == NULL_INDEX )
        {
            // linedef is required to always have valid sidedef1
            I_SoftError( "Linedef %i is missing sidedef1\n", i );
            ld->sidenum[0] = 0;  // arbitrary valid sidedef
        }
        else if ( ld->sidenum[0] >= numsides )
        {
            I_SoftError( "Linedef %i has sidedef1 bad index\n", i );
            ld->sidenum[0] = 0;  // arbitrary valid sidedef
        }
        if( ld->sidenum[1] == NULL_INDEX )
        {
            if( ld->flags & ML_TWOSIDED )
            {
                // two-sided linedef is required to always have valid sidedef2
                I_SoftError( "Linedef %i is missing sidedef2\n", i );
                // fix one or the other
//              ld->sidenum[1] = 0;  // arbitrary valid sidedef
                ld->flags &= ~ML_TWOSIDED;
            }
        }
        else if ( ld->sidenum[1] >= numsides )
        {
            I_SoftError( "Linedef %i has sidedef2 bad index\n", i );
            ld->sidenum[1] = 0;  // arbitrary valid sidedef
        }

       
        // special linedef has special sidedef1
        if (ld->sidenum[0] != NULL_INDEX && ld->special)
          sides[ld->sidenum[0]].linedef_special = ld->special;
    }

    Z_Free (data);
}


void P_LoadLineDefs2()
{
  int i;
  line_t* ld = lines;
  for(i = 0; i < numlines; i++, ld++)
  {
      if (ld->sidenum[0] != NULL_INDEX)
        ld->frontsector = sides[ld->sidenum[0]].sector;
      else
        ld->frontsector = 0;

      if (ld->sidenum[1] != NULL_INDEX)
        ld->backsector = sides[ld->sidenum[1]].sector;
      else
        ld->backsector = 0;
      
      // special linedef setup after sidedefs are loaded
      switch( ld->special ) 
      {
       case 260:  // Boom transparency
         // sidedef1 of master linedef has the transparency map
         // Similar effect to Boom, no alternatives
         {
             int eff = ld->translu_eff;  // TRANSLU_med, or TRANSLU_ext + lumpid
             short tag = ld->tag;
             if( tag )
             {
                 // Same tagged linedef get it too (both sidedefs).
                 int li;
                 for( li=numlines-1; li>=0; li-- )
                 {
                     if( lines[li].tag == tag )
                        lines[li].translu_eff = eff;
                     // Cannot use special because Boom allows tagged lines
                     // to have the transparent effect simultaneously with
                     // other linedef effects.
                 }
             }
         }
      }
  }
}

#if 0
// See two part load of sidedefs, with special texture interpretation
//
// P_LoadSideDefs
//
void P_LoadSideDefs (int lump)
{
    byte*               data;
    int                 i;
    mapsidedef_t*       msd;
    side_t*             sd;

    numsides = W_LumpLength (lump) / sizeof(mapsidedef_t);
    sides = Z_Malloc (numsides*sizeof(side_t), PU_LEVEL, NULL);
    memset (sides, 0, numsides*sizeof(side_t));
    data = W_CacheLumpNum (lump,PU_STATIC);  // sidedefs temp lump
    // [WDJ] Do endian as read from temp sidedefs lump

    msd = (mapsidedef_t *)data;
    sd = sides;
    for (i=0 ; i<numsides ; i++, msd++, sd++)
    {
        sd->textureoffset = LE_SWAP16(msd->textureoffset)<<FRACBITS;
        sd->rowoffset = LE_SWAP16(msd->rowoffset)<<FRACBITS;
        // 0= no-texture, never -1
        sd->toptexture = R_TextureNumForName(msd->toptexture);
        sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
        sd->midtexture = R_TextureNumForName(msd->midtexture);

        sd->sector = &sectors[LE_SWAP16(msd->sector)];
    }

    Z_Free (data);
}
#endif

static
void Report_sidedef_bad_sector( int sdnum, int secnum )
{
    int li;

    // When WAD has not been cleaned, some editors leave sidedefs around
    // with sector_num=65535.
    if( (! verbose) && (secnum == 65535) )   return;

    for( li=numlines-1; li>=0; li-- )
    {
        if( lines[li].sidenum[0] == sdnum
            || lines[li].sidenum[1] == sdnum )
        {
            // found parent linedef
            I_SoftError( "SideDef %i has bad sector number %i, used by line %i\n",
                          sdnum, secnum, li );
            return;
        }
    }
    I_SoftError( "SideDef %i has bad sector number %i, UNUSED\n", sdnum, secnum );
}


// Two part load of sidedefs
// [WDJ] Do endian conversion in part2
void P_LoadSideDefs (int lump)
{
  numsides = W_LumpLength(lump) / sizeof(mapsidedef_t);
  sides = Z_Malloc(numsides*sizeof(side_t), PU_LEVEL, NULL);
  memset(sides, 0, numsides*sizeof(side_t));
}

// SoM: 3/22/2000: Delay loading texture names until after loaded linedefs.

//Hurdler: 04/04/2000: proto added
int R_ColormapNumForName(char *name);

// Interpret Linedef specials in sidedefs,
// after other supporting data has been loaded.
void P_LoadSideDefs2(int lump)
{
  mapsidedef_t * msdlump = W_CacheLumpNum(lump, PU_IN_USE);  // sidedefs lump
  // [WDJ] Do endian as read from temp sidedefs lump
  int  sdnum;
  int  num, eff;

  for (sdnum=0; sdnum<numsides; sdnum++)
  {
      register mapsidedef_t *msd = & msdlump[sdnum]; // map sidedef
      register side_t *sd = & sides[sdnum];
      register sector_t *sec;

      sd->textureoffset = LE_SWAP16(msd->textureoffset)<<FRACBITS;
      sd->rowoffset = LE_SWAP16(msd->rowoffset)<<FRACBITS;

      // Refined to allow special linedef colormap (in texture name) to instead
      // be a wall texture (wall texture name instead of colormap name)
      // using the normal colormap.
      // Check if valid texture first, on failure check if valid colormap,
      // because we have func that can check texture without error.

      uint16_t secnum = LE_SWAP16(msd->sector);
      // [WDJ] Check for buggy wad, like prboom
      if( secnum >= numsectors )
      {
          Report_sidedef_bad_sector( sdnum, secnum );
          secnum = 0; // arbitrary use of sector 0
      }
      sd->sector = sec = &sectors[secnum];

      // original linedef types are 1..141, higher values are extensions
      switch (sd->linedef_special)
      {
        case 242:  // Boom deep water, sidedef1 texture is colormap
        case 280:  //SoM: 3/22/2000: Legacy water type.
          // Sets topmap,midmap,bottommap colormaps, in the tagged sectors.
          // Uses the model sector lightlevel underwater, and over ceiling.
          // [WDJ] There is no good reason for HWRENDER to block recording the
          // colormaps if the worse that the hardware renderer does is ignore them.
          {
            num = R_CheckTextureNumForName(msd->toptexture);
            if(num == -1)  // if not texture
            {
              // must be colormap
              sec->topmap = R_ColormapNumForName(msd->toptexture);
              num = 0;
            }
            sd->toptexture = num; // never set to -1

            num = R_CheckTextureNumForName(msd->midtexture);
            if(num == -1)
            {
              sec->midmap = R_ColormapNumForName(msd->midtexture);
              num = 0;
            }
            sd->midtexture = num; // never set to -1


            num = R_CheckTextureNumForName(msd->bottomtexture);
            if(num == -1)
            {
              sec->bottommap = R_ColormapNumForName(msd->bottomtexture);
              num = 0;
            }
            sd->bottomtexture = num; // never set to -1
          }
          break;   // [WDJ]  no fall through

        case 282:                       //SoM: 4/4/2000: Just colormap transfer
          // Set the colormap of all tagged sectors.

          // SoM: R_CreateColormap will only create a colormap in software mode...
          // [WDJ] Expanded to hardware mode too.
          {
            if(msd->toptexture[0] == '#' || msd->bottomtexture[0] == '#')
            {
              // generate colormap from sidedef1 texture text strings
              sec->midmap = R_CreateColormap(msd->toptexture, msd->midtexture, msd->bottomtexture);
              sd->toptexture = sd->bottomtexture = 0;
              sec->extra_colormap = &extra_colormaps[sec->midmap];
            }
            else
            {
              // textures never set to -1
              if((num = R_CheckTextureNumForName(msd->toptexture)) == -1)
                sd->toptexture = 0;
              else
                sd->toptexture = num;
              if((num = R_CheckTextureNumForName(msd->midtexture)) == -1)
                sd->midtexture = 0;
              else
                sd->midtexture = num;
              if((num = R_CheckTextureNumForName(msd->bottomtexture)) == -1)
                sd->bottomtexture = 0;
              else
                sd->bottomtexture = num;
            }
          }
          break;  // [WDJ]  no fall through
                  // case 282, if(render_soft), was falling through,
                  // but as 260 has same tests, the damage was benign


        case 260:  // Boom transparency
          // When tag=0: this sidedef middle texture is made translucent using TRANMAP.
          // When tag!=0: all same tagged linedef have their middle texture
          // made translucent using sidedef1.
          // If this sidedef middle texture is a TRANMAP, size=64K, then it
          // is used for the transluceny.
          // This always affects both sidedefs.
          // never set to -1, 0=no_texture
          // Do not mangle special because Boom allows tagged lines
          // to have the transparent effect simultaneously with
          // other linedef effects.
          eff = TRANSLU_med;  // default TRANMAP
          sd->midtexture = 0;  // default, no texture
          // texture name = "TRANMAP" means use TRANSLU_med
          if( strncasecmp("TRANMAP", msd->midtexture, 8) != 0 )
          {
              // From Boom, any lump can be transparency map if it is right size
              int spec_num = W_CheckNumForName( msd->midtexture );
              if( spec_num > 0 && W_LumpLength(spec_num) == 65536 )
              {
                  int translu_map_num = R_setup_translu_store( spec_num );
                  eff = TRANSLU_ext + translu_map_num;
                  // LoadLineDefs2 will propagate it to other linedefs
              }
              else
              {
                  // Not lump name or not right size
                  // midtexture is texture
                  num = R_CheckTextureNumForName(msd->midtexture);
                  sd->midtexture = (num == -1) ? 0 : num;
              }
          }
          {
              // Find parent linedef and update, otherwise would have to use
              // some field to pass translu_eff back up to it.
              int li;
              for( li=numlines-1; li>=0; li-- )
              {
                  if( lines[li].sidenum[0] == sdnum )  // found parent linedef
                  {
                      lines[li].translu_eff = eff;
                      break;
                  }
              }
          }

          num = R_CheckTextureNumForName(msd->toptexture);
          sd->toptexture = (num == -1) ? 0 : num;

          num = R_CheckTextureNumForName(msd->bottomtexture);
          sd->bottomtexture = (num == -1) ? 0 : num;
          break;
/*        case 260: // killough 4/11/98: apply translucency to 2s normal texture
          sd->midtexture = strncasecmp("TRANMAP", msd->midtexture, 8) ?
            (sd->special = W_CheckNumForName(msd->midtexture)) < 0 ||
            W_LumpLength(sd->special) != 65536 ?
            sd->special=0, R_TextureNumForName(msd->midtexture) :
              (sd->special++, 0) : (sd->special=0);
          sd->toptexture = R_TextureNumForName(msd->toptexture);
          sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
          break;*/ //This code is replaced.. I need to fix this though


       //Hurdler: added for alpha value with translucent 3D-floors/water
        case 300:  // Legacy solid translucent 3D floor in tagged
        case 301:  // Legacy translucent 3D water in tagged
        case 302:  // Legacy 3D fog in tagged
        case 304:  // Legacy opaque fluid (because of inside)
            // 3Dfloor slab uses model sector ceiling and floor, heights and flats.
            // Upper texture encodes the translucent alpha: #nnn  => 0..255
            // Uses model sector colormap and lightlevel
            // Interpret texture name string as decimal alpha and fogwater effect
            sd->toptexture = R_Create_FW_effect( sd->linedef_special,
                                                 msd->toptexture );
            sd->bottomtexture = 0;
            sd->midtexture = R_TextureNumForName(msd->midtexture); // side texture
            break;

        default:                        // normal cases
          // SoM: Lots of people are sick of texture errors. 
          // Hurdler: see r_data.c for my suggestion
          // [WDJ] 0=no-texture, texture not found returns default texture.
          // Textures never set to -1, so these texture num are safe to use as array index.
          sd->midtexture = R_TextureNumForName(msd->midtexture);
          sd->toptexture = R_TextureNumForName(msd->toptexture);
          sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
          break;
      }
  }
  Z_Free (msdlump);
}




//
// P_LoadBlockMap
//
// Read wad blockmap using int16_t wadblockmaplump[].
// Expand from 16bit wad to internal 32bit blockmap.
void P_LoadBlockMap (int lump)
{
  int count = W_LumpLength(lump)/2;  // number of 16 bit blockmap entries
  uint16_t * wadblockmaplump = W_CacheLumpNum (lump, PU_LEVEL); // blockmap lump temp
  uint32_t firstlist, lastlist;  // blockmap block list bounds
  uint32_t overflow_corr = 0;
  uint32_t prev_bme = 0;  // for detecting overflow wrap
  int i;
   
  // [WDJ] when zennode has not been run, this code will corrupt Zone memory.
  // It assumes a minimum size blockmap.
  if( count < 5 )
      I_Error( "Missing blockmap, node builder has not been run.\n" );

  // [WDJ] Do endian as read from blockmap lump temp
  blockmaphead = Z_Malloc(sizeof(*blockmaphead) * count, PU_LEVEL, NULL);

      // killough 3/1/98: Expand wad blockmap into larger internal one,
      // by treating all offsets except -1 as unsigned and zero-extending
      // them. This potentially doubles the size of blockmaps allowed,
      // because Doom originally considered the offsets as always signed.
      // [WDJ] They are unsigned in Unofficial Doom Spec.

  blockmaphead[0] = LE_SWAP16(wadblockmaplump[0]);  // map orgin_x
  blockmaphead[1] = LE_SWAP16(wadblockmaplump[1]);  // map orgin_y
  blockmaphead[2] = LE_SWAP16(wadblockmaplump[2]);  // number columns (x size)
  blockmaphead[3] = LE_SWAP16(wadblockmaplump[3]);  // number rows (y size)

  bmaporgx = blockmaphead[0]<<FRACBITS;
  bmaporgy = blockmaphead[1]<<FRACBITS;
  bmapwidth = blockmaphead[2];
  bmapheight = blockmaphead[3];
  blockmapindex = & blockmaphead[4];
  firstlist = 4 + (bmapwidth*bmapheight);
  lastlist = count - 1;

  if( firstlist >= lastlist || bmapwidth < 1 || bmapheight < 1 )
      I_Error( "Blockmap corrupt, must run node builder on wad.\n" );

  // read blockmap index array
  for (i=4 ; i<firstlist ; i++)  // for all entries in wad offset index
  {
      uint32_t  bme = LE_SWAP16(wadblockmaplump[i]);  // offset
      // upon overflow, the bme will wrap to low values
      if ( (bme < firstlist)  // too small to be valid
           && (bme < 0x1000) && (prev_bme > 0xf000))  // wrapped
      {
          // first or repeated overflow
          overflow_corr += 0x00010000;
          GenPrintf(EMSG_warn,"Correct blockmap offset[%i...] overflow by adding 0x%X\n",
                   i, overflow_corr );
      }
      prev_bme = bme;  // uncorrected
      // correct for overflow, or else try without correction
      if ( overflow_corr )
      {
          uint32_t bmec = bme + overflow_corr;
          // First entry of list is 0, but high odds of hitting one randomly.
          // Check for valid blockmap offset, and offset overflow
          if ( bmec <= lastlist
               && wadblockmaplump[bmec] == 0      // valid start list
               && ((bmec - blockmaphead[i-1]) < 1000))  // reasonably close sequentially
          {
              bme = bmec;
          }
      }
     
      if ( bme > lastlist )
          I_Error("Blockmap offset[%i]= %i, exceeds bounds.\n", i, bme);
      if ( bme < firstlist
           || wadblockmaplump[bme] != 0 )  // not start list
          I_Error("Bad blockmap offset[%i]= %i.\n", i, bme);
      blockmaphead[i] = bme;
  }
  // read blockmap lists
  for (i=firstlist ; i<count ; i++)  // for all list entries in wad blockmap
  {
      // killough 3/1/98
      // keep -1 (0xffff), but other values are unsigned
      uint16_t  bme = LE_SWAP16(wadblockmaplump[i]);
      blockmaphead[i] = (bme == 0xffff)? ((uint32_t) -1) : ((uint32_t) bme);
  }

  Z_Free(wadblockmaplump);


  // clear out mobj chains
  count = sizeof(*blocklinks)* bmapwidth*bmapheight;
  blocklinks = Z_Malloc (count, PU_LEVEL, NULL);
  memset (blocklinks, 0, count);
/* Original
                blockmaplump = W_CacheLumpNum (lump,PU_LEVEL);
                blockmap = blockmaplump+4;
                count = W_LumpLength (lump)/2;

                for (i=0 ; i<count ; i++)
                        blockmaplump[i] = LE_SWAP16(blockmaplump[i]);

                bmaporgx = blockmaplump[0]<<FRACBITS;
                bmaporgy = blockmaplump[1]<<FRACBITS;
                bmapwidth = blockmaplump[2];
                bmapheight = blockmaplump[3];
        }

        // clear out mobj chains
        count = sizeof(*blocklinks)*bmapwidth*bmapheight;
        blocklinks = Z_Malloc (count, PU_LEVEL, NULL);
        memset (blocklinks, 0, count);
 */
}



//
// P_GroupLines
// Builds sector line lists and subsector sector numbers.
// Finds block bounding boxes for sectors.
//
void P_GroupLines (void)
{
    line_t**            linebuffer;  // to build line list
    line_t*             li;
    sector_t*           sector;
    subsector_t*        ss;
    seg_t*              seg;
    fixed_t             bbox[4];
    int                 block;
    int                 total;
    int                 i, j;

    // look up sector number for each subsector
    ss = subsectors;
    for (i=0 ; i<numsubsectors ; i++, ss++)
    {
        // [WDJ] Detct buggy wad
        if( ss->firstline > numsegs )
        {
            I_Error( "P_GroupLines: subsector firstline %i > numsegs %i\n",
                         ss->firstline, numsegs );
        }
        seg = &segs[ss->firstline];  // first seg of the subsector
        if( seg->sidedef )
        {
            // normal
            ss->sector = seg->sidedef->sector;
        }
        else
        {
            // [WDJ] prboom can play Europe.wad, but Legacy segfaults, cannot have that.
            // buggy wad, try to recover
            I_SoftError( "P_GroupLines, subsector without sidedef1 sector\n");
            // find one good reference in all the segs, like in prboom does
            for(j=0; j < ss->numlines; j++ )
            {
                if( seg->sidedef )
                {
                    ss->sector = seg->sidedef->sector;
                    GenPrintf(EMSG_error, " found sector in seg #%d\n", j );
                    goto continue_subsectors;
                }
                seg++;  // step through segs from firstline
            }
            // subsector has no sector
            I_SoftError( "P_GroupLines, subsector defaulted to sector[0]\n");
            // [WDJ] arbitrarily use sector[0], because these usually are not visible.
            ss->sector = & sectors[0];
        }
       continue_subsectors:
        continue;
    }

    // count number of lines in each sector
    li = lines;
    total = 0;
    for (i=0 ; i<numlines ; i++, li++) // for each line
    {
        // [WDJ] some wads ( like blowup.wad, and zdoom formats)
        // have missing frontsector, but other ports (like prboom) cope
        if( li->frontsector )
        {
            total++;
            li->frontsector->linecount++;
        }
        else
        {
            I_SoftError( "P_GroupLines, linedef #%d missing frontsector\n", i );
        }

        if (li->backsector && li->backsector != li->frontsector)
        {
            li->backsector->linecount++;
            total++;
        }
    }

    // build line tables for each sector
    // One allocation for all, each sector using a segment of the list
    linebuffer = Z_Malloc(total*sizeof(line_t *), PU_LEVEL, NULL);
    sector = sectors;  // &sectors[0]
    for (i=0 ; i<numsectors ; i++, sector++)
    {
        // for each sector
        M_ClearBox (bbox);
        sector->linelist = linebuffer;  // the next segment
        li = lines;  // &lines[0]
        for (j=0 ; j<numlines ; j++, li++)
        {
            // for each line
            // check if it has this sector as a side
            if (li->frontsector == sector || li->backsector == sector)
            {
                *linebuffer++ = li; // add it to this segment
                M_AddToBox (bbox, li->v1->x, li->v1->y);
                M_AddToBox (bbox, li->v2->x, li->v2->y);
            }
        }
        // check that added lines in this segment equal the sector linecount
        if (linebuffer - sector->linelist != sector->linecount)
            I_Error ("P_GroupLines: miscounted");

        // set the degenmobj_t to the middle of the bounding box
        sector->soundorg.x = (bbox[BOXRIGHT]+bbox[BOXLEFT])/2;
        sector->soundorg.y = (bbox[BOXTOP]+bbox[BOXBOTTOM])/2;

        // adjust bounding box to map blocks
        block = (bbox[BOXTOP]-bmaporgy+MAXRADIUS)>>MAPBLOCKSHIFT;
        block = block >= bmapheight ? bmapheight-1 : block;
        sector->blockbox[BOXTOP]=block;

        block = (bbox[BOXBOTTOM]-bmaporgy-MAXRADIUS)>>MAPBLOCKSHIFT;
        block = block < 0 ? 0 : block;
        sector->blockbox[BOXBOTTOM]=block;

        block = (bbox[BOXRIGHT]-bmaporgx+MAXRADIUS)>>MAPBLOCKSHIFT;
        block = block >= bmapwidth ? bmapwidth-1 : block;
        sector->blockbox[BOXRIGHT]=block;

        block = (bbox[BOXLEFT]-bmaporgx-MAXRADIUS)>>MAPBLOCKSHIFT;
        block = block < 0 ? 0 : block;
        sector->blockbox[BOXLEFT]=block;
    }

}


// SoM: 6/27: Don't restrict maps to MAPxx/ExMx any more!
char *levellumps[] =
{
  "label",        // ML_LABEL,    A separator, name, ExMx or MAPxx
  "THINGS",       // ML_THINGS,   Monsters, items..
  "LINEDEFS",     // ML_LINEDEFS, LineDefs, from editing
  "SIDEDEFS",     // ML_SIDEDEFS, SideDefs, from editing
  "VERTEXES",     // ML_VERTEXES, Vertices, edited and BSP splits generated
  "SEGS",         // ML_SEGS,     LineSegs, from LineDefs split by BSP
  "SSECTORS",     // ML_SSECTORS, SubSectors, list of LineSegs
  "NODES",        // ML_NODES,    BSP nodes
  "SECTORS",      // ML_SECTORS,  Sectors, from editing
  "REJECT",       // ML_REJECT,   LUT, sector-sector visibility
  "BLOCKMAP"      // ML_BLOCKMAP  LUT, motion clipping, walls/grid element
};


//
// P_CheckLevel
// Checks a lump and returns whether or not it is a level header lump.
boolean P_CheckLevel(int lumpnum)
{
  int  i;
  int  file, lump;
  
  for(i=ML_THINGS; i<=ML_BLOCKMAP; i++)
  {
      file = lumpnum >> 16;
      lump = (lumpnum & 0xffff) + i;
      if(file > numwadfiles || lump > wadfiles[file]->numlumps ||
         strncmp(wadfiles[file]->lumpinfo[lump].name, levellumps[i], 8) )
        return false;
  }
  return true;    // all right
}


//
// Setup sky texture to use for the level, actually moved the code
// from G_DoLoadLevel() which had nothing to do there.
//
// - in future, each level may use a different sky.
//
// The sky texture to be used instead of the F_SKY1 dummy.
void P_SetupLevelSky (void)
{
    char       skytexname[12];

    // DOOM determines the sky texture to be used
    // depending on the current episode, and the game version.

    if(*info_skyname)
      skytexture = R_TextureNumForName(info_skyname);
    else
    if (gamemode == doom2_commercial)  // includes doom2, plut, tnt
      // || (gamemode == pack_tnt) he ! is not a mode is a episode !
      //    || ( gamemode == pack_plut )
    {
        if (gamemap < 12)
            skytexture = R_TextureNumForName ("SKY1");
        else
        if (gamemap < 21)
            skytexture = R_TextureNumForName ("SKY2");
        else
            skytexture = R_TextureNumForName ("SKY3");
    }
    else
    if ( (gamemode==ultdoom_retail) ||
         (gamemode==doom_registered) )
    {
        if (gameepisode<1 || gameepisode>4)     // useful??
            gameepisode = 1;

        sprintf (skytexname,"SKY%d",gameepisode);
        skytexture = R_TextureNumForName (skytexname);
    }
    else // who knows?
    if (gamemode==heretic)
    {
        static char *skyLumpNames[5] = {
            "SKY1", "SKY2", "SKY3", "SKY1", "SKY3" };

        if(gameepisode > 5)
            skytexture = R_TextureNumForName("SKY1");
        else
            skytexture = R_TextureNumForName(skyLumpNames[gameepisode-1]);
    }
    else
        skytexture = R_TextureNumForName ("SKY1");

    // scale up the old skies, if needed
    R_SetupSkyDraw ();
}


//
// P_SetupLevel
//
// Load the level from an existing lump or from a external wad !
// Purge all previous PU_LEVEL memory.
int    level_lumpnum = 0;  // for info and comparative savegame
char*  level_mapname = NULL;  // to savegame and info

//  to_episode : change to episode num
//  to_map : change to map number
//  to_skill : change to skill
//  map_wadname : map command, load wad file
boolean P_SetupLevel (int      to_episode,
                      int      to_map,
                      skill_e  to_skill,
                      char*    map_wadname)      // for wad files
{
    char  *sl_mapname = NULL;
    int   i;

    GenPrintf( (verbose? (EMSG_ver|EMSG_now) : (EMSG_console|EMSG_now)),
               "Setup Level\n" );

    //Initialize sector node list.
    P_Initsecnode();

    // Clear existing level variables and reclaim memory.
    totalkills = totalitems = totalsecret = wminfo.maxfrags = 0;
    wminfo.partime = 180;
    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        players[i].killcount = players[i].secretcount
            = players[i].itemcount = 0;
        players[i].mo = NULL;  // will be freed with PU_LEVEL
#ifdef CLIENTPREDICTION2
        players[i].spirit = NULL;
#endif
    }

    // Initial height of PointOfView
    // will be set by player think.

    players[consoleplayer].viewz = 1;

    // [WDJ] 7/2010 Free allocated memory in sectors before PU_LEVEL purge
    for (i=0 ; i<numsectors ; i++)
    {
        sector_t * sp = &sectors[i];
        if( sp )
        {
            if( sp->attached )  // from realloc in P_AddFakeFloor
            {
                free( sp->attached );
                sp->attached = NULL;
            }
        }
    }

    I_Sleep( 100 );  // give menu sound a chance to finish
    // Make sure all sounds are stopped before Z_FreeTags.
    // This will kill the last menu pistol sound too.
    S_StopSounds();

#if 0 // UNUSED
    if (debugfile)
    {
        Z_FreeTags (PU_LEVEL, 255);  // all purge tags
        Z_FileDumpHeap (debugfile);
    }
#endif

    Z_FreeTags (PU_LEVEL, PU_PURGELEVEL-1);
   
    // [WDJ] all temp lumps are unlocked, to be freed unless they are accessed first
    Z_ChangeTags_To (PU_LUMP, PU_CACHE);
    Z_ChangeTags_To (PU_IN_USE, PU_CACHE);  // for any missed otherwise

#ifdef WALLSPLATS
    // clear the splats from previous level
    R_ClearLevelSplats ();
#endif
    P_Clear_Extra_Mapthing();  // remove FS mapthings 

    script_camera_on = false;
    HU_ClearTips();

    if (camera.chase)
        camera.mo = NULL;

    // UNUSED W_Profile ();
    
    P_InitThinkers ();

    // Loading new level map.
    // if working with a devlopment map, reload it
    W_Reload ();

    // Load the map from existing game resource or external wad file.
    if (map_wadname && map_wadname[0] )
    {
        // External wad file load.
        level_id_t level_id;

        if (!P_AddWadFile (map_wadname, &level_id) ||
            level_id.mapname==NULL)            // no maps were found
        {
            // go back to title screen if no map is loaded
            return false;
        }

        // From the added wad, returned by P_AddWadFile().
        to_episode = gameepisode = level_id.episode;
        to_map = gamemap = level_id.map;
        sl_mapname = level_id.mapname;  // mapname from P_AddWadFile
    }
    else
    {
        // Existing game map
        sl_mapname = G_BuildMapName(to_episode,to_map);
    }

    // Determine this level map name for savegame and info next level.
    if(level_mapname)   Z_Free(level_mapname);
    level_mapname = Z_Strdup(sl_mapname, PU_STATIC, 0);  // MAP01 or E1M1, etc.
    level_lumpnum = W_GetNumForName(sl_mapname);

    leveltime = 0;

    // textures are needed first
//    R_LoadTextures ();
//    R_FlushTextureCache();

    R_Clear_FW_effect();  // clear and init of fog store
    R_ClearColormaps();  // colormap ZMalloc cleared by Z_FreeTags(PU_LEVEL, PU_PURGELEVEL-1)

#ifdef FRAGGLESCRIPT
    // load level lump info(level name etc)
    // Dependent upon level_mapname, level_lumpnum.
    P_LoadLevelInfo();
#endif

    //SoM: We've loaded the music lump, start the music.
    S_StartSounds();

    //faB: now part of level loading since in future each level may have
    //     its own anim texture sequences, switches etc.
    P_InitSwitchList ();
    P_InitPicAnims ();
    P_InitLava ();
    P_SetupLevelSky ();

    // SoM: WOO HOO!
    // SoM: DOH!
    //R_InitPortals ();

    // note: most of this ordering is important
    P_LoadBlockMap (level_lumpnum+ML_BLOCKMAP);
    P_LoadVertexes (level_lumpnum+ML_VERTEXES);
    P_LoadSectors  (level_lumpnum+ML_SECTORS);
    P_LoadSideDefs (level_lumpnum+ML_SIDEDEFS);

    P_LoadLineDefs (level_lumpnum+ML_LINEDEFS);
    P_LoadSideDefs2(level_lumpnum+ML_SIDEDEFS);
    P_LoadLineDefs2();
    P_LoadSubsectors (level_lumpnum+ML_SSECTORS);
    P_LoadNodes (level_lumpnum+ML_NODES);
    P_LoadSegs (level_lumpnum+ML_SEGS);
    rejectmatrix = W_CacheLumpNum (level_lumpnum+ML_REJECT,PU_LEVEL);
    P_GroupLines ();

#ifdef HWRENDER // not win32 only 19990829 by Kin
    if (rendermode != render_soft)
    {
        // BP: reset light between levels (we draw preview frame lights on current frame)
        HWR_ResetLights();
        // Correct missing sidedefs & deep water trick
        HWR_CorrectSWTricks();
        HWR_CreatePlanePolygons();
    }
#endif

    bodyqueslot = 0;

    numdmstarts = 0;
    // added 25-4-98 : reset the players starts
    //SoM: Set pointers to NULL
    for(i=0;i<MAXPLAYERS;i++)
       playerstarts[i] = NULL;

    P_InitAmbientSound ();
    P_InitMonsters ();
    P_OpenWeapons ();
    P_LoadThings (level_lumpnum+ML_THINGS);
    P_CloseWeapons ();

    // set up world state
    P_SpawnSpecials ();
    P_InitBrainTarget();

    //BP: spawnplayers after all structures are inititialized
    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if (playeringame[i])
        {
            if (cv_deathmatch.value)
            {
                G_DoReborn(i);
            }
            else if( demoversion>=128 )
            {
                G_CoopSpawnPlayer(i);
            }
        }
    }

    // clear special respawning que
    iquehead = iquetail = 0;

    // build subsector connect matrix
    //  UNUSED P_ConnectSubsectors ();

#ifdef CDMUS
    //Fab:19-07-98:start cd music for this level (note: can be remapped)
    if (gamemode==doom2_commercial)
#if defined (__DJGPP__)			
        I_PlayCD (to_map+1, true);                // Doom2, 32 maps
#else
        I_PlayCD (to_map, true);                // Doom2, 32 maps
#endif			
    else
#if defined (__DJGPP__)				
        I_PlayCD ((to_episode-1)*9+ to_map+1, true);  // Doom1, 9maps per episode
#else
        I_PlayCD ((to_episode-1)*9+ to_map, true);  // Doom1, 9maps per episode
#endif
#endif

    // preload graphics
#ifdef HWRENDER // not win32 only 19990829 by Kin
    if (rendermode != render_soft)
    {
        HWR_PrepLevelCache (numtextures);
        HWR_CreateStaticLightmaps();
    }
#endif

    if (precache)
        R_PrecacheLevel ();


#ifdef FRAGGLESCRIPT
    T_Init_FSArrayList();         // Setup FS array list
    T_PreprocessScripts();        // preprocess FraggleScript scripts
#endif

    script_camera_on = false;

    B_InitNodes();  //added by AC for acbot

    //debug_Printf("P_SetupLevel: %d vertexs %d segs %d subsector\n",numvertexes,numsegs,numsubsectors);
    return true;
}


// Add a wadfile to the active wad files,
// replace sounds, musics, patches, textures, sprites and maps
//
//  wadfilename : filename of wad to be loaded 
//  firstmap_out : /*OUT*/  info about the first level map
boolean P_AddWadFile (char* wadfilename, /*OUT*/ level_id_t * firstmap_out )
{
    int         wadfilenum;
    wadfile_t*  wadfile;
    lumpinfo_t* lumpinfo;
    char*       name;
    int         firstmapreplaced;
    int         i,j,num;
    int         replace_cnt;
    boolean     texturechange;

    if( firstmap_out )
       firstmap_out->mapname = NULL;

    if ((wadfilenum = W_LoadWadFile (wadfilename))==-1)
    {
        CONS_Printf ("couldn't load wad file %s\n", wadfilename);
        return false;
    }
    wadfile = wadfiles[wadfilenum];

    //
    // search for sound replacements
    //
    lumpinfo = wadfile->lumpinfo;
    replace_cnt = 0;
    texturechange=false;
    for (i=0; i<wadfile->numlumps; i++,lumpinfo++)
    {
        name = lumpinfo->name;
        if (name[0]=='D' && name[1]=='S')
        {
            for (j=1 ; j<NUMSFX ; j++)
            {
                if ( S_sfx[j].name &&
                    !S_sfx[j].link &&
                    !strncasecmp(S_sfx[j].name,name+2,6) )
                {
                    // the sound will be reloaded when needed,
                    // since sfx->data will be NULL
                    if (devparm)
                        GenPrintf(EMSG_dev, "Sound %.8s replaced\n", name);

                    S_FreeSfx (&S_sfx[j]);

                    replace_cnt++;
                }
            }
        }
        else
        if( memcmp(name,"TEXTURE1",8)==0    // find texture replacement too
         || memcmp(name,"TEXTURE2",8)==0
         || memcmp(name,"PNAMES",6)==0)
            texturechange=true;
    }
    if (!devparm && replace_cnt)
        CONS_Printf ("%d sounds replaced\n", replace_cnt);

    //
    // search for music replacements
    //
    lumpinfo = wadfile->lumpinfo;
    replace_cnt = 0;
    for (i=0; i<wadfile->numlumps; i++,lumpinfo++)
    {
        name = lumpinfo->name;
        if (name[0]=='D' && name[1]=='_')
        {
            if (devparm)
                GenPrintf(EMSG_dev, "Music %.8s replaced\n", name);
            replace_cnt++;
        }
    }
    if (!devparm && replace_cnt)
        CONS_Printf ("%d musics replaced\n", replace_cnt);

    //
    // search for sprite replacements
    //
    R_AddSpriteDefs (sprnames, numwadfiles-1);

    //
    // search for texturechange replacements
    //
    if( texturechange ) // inited in the sound check
        R_LoadTextures();       // numtexture changes
    else
        R_FlushTextureCache();  // just reload it from file

    //
    // look for skins
    //
    R_AddSkins (wadfilenum);      //faB: wadfile index in wadfiles[]

    //
    // search for maps
    //
    lumpinfo = wadfile->lumpinfo;
    firstmapreplaced = MAXINT;  // invalid
    for (i=0; i<wadfile->numlumps; i++,lumpinfo++)
    {
        name = lumpinfo->name;
        num = 0;  // invalid
        if (gamemode==doom2_commercial)       // Doom2
        {
            if (name[0]=='M' &&
                name[1]=='A' &&
                name[2]=='P')
            {
                num = (name[3]-'0')*10 + (name[4]-'0');
                CONS_Printf ("Map %d\n", num);
            }
        }
        else
        {
            if (name[0]=='E' &&
                ((unsigned)name[1]-'0')<='9' &&   // a digit
                name[2]=='M' &&
                ((unsigned)name[3]-'0')<='9' &&
                name[4]==0)
            {
                num = ((name[1]-'0')<<8) + (name[3]-'0');
                CONS_Printf ("Episode %d map %d\n", name[1]-'0',
                                                    name[3]-'0');
            }
        }
        // The lowest numbered map is the first map. No map 0.
        if ( (num>0) && (num < firstmapreplaced) )
        {
            firstmapreplaced = num;
            if(firstmap_out)  /* OUT */
            {
                firstmap_out->mapname = name;
                firstmap_out->episode = num >> 8;
                firstmap_out->map = num & 0xFF;
            }
        }
    }
    if ( firstmapreplaced >= 0xFFFF )  // invalid
        CONS_Printf ("no maps added\n");

    // reload status bar (warning should have valid player !)
    if( gamestate == GS_LEVEL )
        ST_Start();

    return true;
}
