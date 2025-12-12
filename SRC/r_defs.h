// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: r_defs.h 1180 2015-11-25 20:05:57Z wesleyjohnson $
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
// $Log: r_defs.h,v $
// Revision 1.35  2003/05/04 04:14:08  sburke
// Prevent misaligned access on Solaris/Sparc.
//
// Revision 1.34  2002/07/20 03:23:20  mrousseau
// Added 'side' to seg_t
//
// Revision 1.33  2002/01/12 02:21:36  stroggonmeth
// Revision 1.32  2001/08/19 20:41:04  hurdler
// Revision 1.31  2001/08/13 22:53:40  stroggonmeth
//
// Revision 1.30  2001/08/12 17:57:15  hurdler
// Beter support of sector coloured lighting in hw mode
//
// Revision 1.29  2001/08/11 15:18:02  hurdler
// Add sector colormap in hw mode (first attempt)
//
// Revision 1.28  2001/08/09 21:35:17  hurdler
// Add translucent 3D water in hw mode
//
// Revision 1.27  2001/08/08 20:34:43  hurdler
// Big TANDL update
//
// Revision 1.26  2001/08/06 23:57:09  stroggonmeth
// Removed portal code, improved 3D floors in hardware mode.
//
// Revision 1.25  2001/05/30 04:00:52  stroggonmeth
// Fixed crashing bugs in software with 3D floors.
//
// Revision 1.24  2001/05/14 19:02:58  metzgermeister
//   * Fixed floor not moving up with player on E3M1
//   * Fixed crash due to oversized string in screen message ... bad bug!
//   * Corrected some typos
//   * fixed sound bug in SDL
//
// Revision 1.23  2001/04/30 17:19:24  stroggonmeth
// HW fix and misc. changes
//
// Revision 1.22  2001/03/30 17:12:51  bpereira
// Revision 1.21  2001/03/21 18:24:39  stroggonmeth
//
// Revision 1.20  2001/03/19 21:18:48  metzgermeister
//   * missing textures in HW mode are replaced by default texture
//   * fixed crash bug with P_SpawnMissile(.) returning NULL
//   * deep water trick and other nasty thing work now in HW mode (tested with tnt/map02 eternal/map02)
//   * added cvar gr_correcttricks
//
// Revision 1.19  2001/03/13 22:14:20  stroggonmeth
// Long time no commit. 3D floors, FraggleScript, portals, ect.
//
// Revision 1.18  2001/02/28 17:50:55  bpereira
//
// Revision 1.17  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.16  2000/11/21 21:13:17  stroggonmeth
// Optimised 3D floors and fixed crashing bug in high resolutions.
//
// Revision 1.15  2000/11/09 17:56:20  stroggonmeth
//
// Revision 1.14  2000/11/02 17:50:09  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.13  2000/10/07 20:36:13  crashrl
// Added deathmatch team-start-sectors via sector/line-tag and linedef-type 1000-1031
//
// Revision 1.12  2000/10/04 16:19:24  hurdler
// Change all those "3dfx names" to more appropriate names
//
// Revision 1.11  2000/07/01 09:23:49  bpereira
// Revision 1.10  2000/04/18 17:39:39  stroggonmeth
// Revision 1.9  2000/04/18 12:55:39  hurdler
// Revision 1.7  2000/04/15 22:12:58  stroggonmeth
//
// Revision 1.6  2000/04/12 16:01:59  hurdler
// ready for T&L code and true static lighting
//
// Revision 1.5  2000/04/11 19:07:25  stroggonmeth
//
// Revision 1.4  2000/04/06 20:47:08  hurdler
// add Boris' changes for coronas in doom3.wad
//
// Revision 1.3  2000/04/04 00:32:47  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Refresh/rendering module, shared data struct definitions.
//
//-----------------------------------------------------------------------------


#ifndef R_DEFS_H
#define R_DEFS_H

#include "doomtype.h"
#include "m_fixed.h"

// We rely on the thinker data struct
// to handle sound origins in sectors.
#include "d_think.h"
// SECTORS do store MObjs anyway.
#include "p_mobj.h"

#include "screen.h"
  // MAXVIDWIDTH, MAXVIDHEIGHT


#define NULL_INDEX 0xFFFF  // or -1. Used in line_t::sidenum and maplinedef_t::sidenum.


// Silhouette, needed for clipping Segs (mainly)
// and sprites representing things.
// OR bits for silhouette
typedef enum {
   SIL_BOTTOM  = 0x01,
   SIL_TOP     = 0x02
} Silhouette_e;

//faB: was upped to 512, but still people come with levels that break the
//     limits, so had to do an ugly re-alloc to get rid of the overflow.
//#define MAXDRAWSEGS             256        // see r_segs.c for more

// SoM: Moved this here...
// This could be wider for >8 bit display.
// Indeed, true color support is possible
//  precalculating 24bpp lightmap/colormap LUT.
//  from darkening PLAYPAL to all black.
// Could even use more than 32 levels.
typedef byte    lighttable_t;  // light map table
   // can be an array of map tables [256], or just one
// index a lighttable by mult by sizeof lighttable ( *256  =>  <<8 )
#define LIGHTTABLE(t)   ((t)<<8)

// right shift to convert 0..255 to 0..(NUM_RGBA_LEVELS-1)
//#define NUM_RGBA_LEVELS  4
//#define LIGHT_TO_RGBA_SHIFT  6
//#define NUM_RGBA_LEVELS  8
//#define LIGHT_TO_RGBA_SHIFT  5
#define NUM_RGBA_LEVELS  16
#define LIGHT_TO_RGBA_SHIFT  4
//#define NUM_RGBA_LEVELS  32
//#define LIGHT_TO_RGBA_SHIFT  3

// SoM: ExtraColormap type. Use for extra_colormaps from now on.
typedef struct
{
  uint32_t        maskcolor;  // 16 bit bright color
  uint32_t        fadecolor;  // 16 bit dark color
  double          maskalpha;  // 0.0 .. 1.0
  uint16_t        fadestart, fadeend;
  int             fog;

  //Hurdler: rgba is used in hw mode for coloured sector lighting
  // [WDJ] Separate rgba for light levels [0]=darkest, [NUM-1]=brightest
  // This is cast into a union of byte components.
  uint32_t        rgba[NUM_RGBA_LEVELS]; // similar to maskcolor in sw mode
     // alpha=0..255, 0=black/white tint, 255=saturated color
     // r,g,b are the saturated color, 0..255

  lighttable_t*   colormap; // colormap tables [32][256]
} extracolormap_t;


//
// INTERNAL MAP TYPES
//  used by play and refresh
//

//
// Your plain vanilla vertex.
// Note: transformed values not buffered locally,
//  like some DOOM-alikes ("wt", "WebView") did.
//
typedef struct
{
    fixed_t     x;
    fixed_t     y;

} vertex_t;


// Forward of LineDefs, for Sectors.
struct line_s;

// Each sector has a degenmobj_t in its center
//  for sound origin purposes.
// I suppose this does not handle sound from
//  moving objects (doppler), because
//  position is prolly just buffered, not updated.
typedef struct
{
    thinker_t           thinker;        // not used for anything
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;

} degenmobj_t;

//SoM: 3/23/2000: Store fake planes in a resizable array instead of just by
//heightsec field. Allows for multiple fake planes.
typedef enum
{
  FF_EXISTS            = 0x1,    //MAKE SURE IT'S VALID
  FF_SOLID             = 0x2,    //It clips things
  FF_OUTER_SIDES       = 0x4,    //Render the outside view of sides
  FF_OUTER_PLANES      = 0x8,    //Render the outside view of floor/ceiling
  FF_SLAB_SHADOW       = 0x10,   //Make two lightlist entries to contain light
  FF_NOSHADE           = 0x20,   //No light effect
  // Cut instead of Occlude
  FF_CUTSOLIDS         = 0x40,   //Must cut hidden solid pixels
  FF_CUTEXTRA          = 0x80,   //Must cut hidden translucent pixels
  FF_CUTSPRITES        = 0x100,  //Must cut sprites, Final Step in 3D water
  FF_EXTRA             = 0x200,  //Translucent, water, and fog,
                                 //It gets cut by FF_CUTEXTRAS
  FF_FLUID             = 0x400,  //Fluid surface
  FF_TRANSLUCENT       = 0x800,  //Translucent (see through)
  FF_FOG               = 0x1000, //Fog area
  FF_INNER_PLANES      = 0x2000, //Render the inside view of planes (FOG/WATER)
  FF_INNER_SIDES       = 0x4000, //Render the inside view of sides (FOG/WATER)
  FF_JOIN_SIDES        = 0x8000, //Render the join side between similar sectors
  FF_FOGFACE           = 0x10000,//Render a fogsheet in face
  FF_SWIMMABLE         = 0x20000,//Player can swim
//  FF_unused          = 0x40000,//
//  FF_unused          = 0x80000,//
} ffloortype_e;


// created by P_AddFakeFloor
typedef struct ffloor_s
{
  // references to model sector, to pass through changes immediately
  fixed_t          *topheight;  // model sector ceiling
  short            *toppic;
  short            *toplightlevel;
  fixed_t          *topxoffs;
  fixed_t          *topyoffs;

  fixed_t          *bottomheight;  // model sector floor
  short            *bottompic;
  //short            *bottomlightlevel;
  fixed_t          *bottomxoffs;
  fixed_t          *bottomyoffs;

  int              model_secnum; // model sector num used in linedef
  ffloortype_e     flags;  // draw and property flags set by special linedef

  struct line_s  * master; // the special linedef generating this floor

  struct sector_s* taggedtarget; // tagged sector that is affected

  // double linked list of ffloor_t in sector
  struct ffloor_s* next;
  struct ffloor_s* prev;

  uint16_t         fw_effect;		// index to fweff, 0 is unused
                                        // FF_FOG, FF_TRANSLUCENT, alpha
  uint8_t          alpha;
  uint8_t          lastlight;		// light index, FF_SLAB_SHADOW
} ffloor_t;


// SoM: This struct holds information for shadows casted by 3D floors.
// This information is contained inside the sector_t and is used as the base
// information for casted shadows.
typedef struct lightlist_s {
  fixed_t                 height;
  int                     flags;
  short *                 lightlevel;
  extracolormap_t*        extra_colormap;
  ffloor_t*               caster;
} ff_lightlist_t;


// SoM: This struct is used for rendering walls with shadows casted on them...
typedef struct r_lightlist_s {
  fixed_t                 height;
  fixed_t                 heightstep;
  fixed_t                 botheight;
  fixed_t                 botheightstep;
  short                   lightlevel;
  short                   vlight;  // visible light 0..255
  extracolormap_t*        extra_colormap;
  lighttable_t*           rcolormap;
  int                     flags;
} r_lightlist_t;





typedef enum {
   FLOOR_SOLID,
   FLOOR_WATER,  
   FLOOR_LAVA,   
   FLOOR_SLUDGE, 
   FLOOR_ICE,
} floortype_e;

// ----- for special tricks with HW renderer -----

//
// For creating a chain with the lines around a sector
//
typedef struct linechain_s
{
    struct line_s        *line;
    struct linechain_s   *next;
} linechain_t;
// ----- end special tricks -----

// sector model	[WDJ] 11/14/2009
typedef enum{
   SM_normal,		// normal sector
   SM_colormap,		// Legacy colormap generation
   SM_fluid,		// start of fluid sectors
   SM_Boom_deep_water,	// special Boom sector
   SM_Legacy_water,	// special Legacy sector
     // Legacy 3D floors are handled through FFloor list
} sector_model_e;

//
// The SECTORS record, at runtime.
// Stores things/mobjs.
//
typedef struct sector_s
{
    fixed_t     floorheight;
    fixed_t     ceilingheight;
    short       floorpic;
    short       ceilingpic;
    short       lightlevel;
    short       special;	 // special type code (highly encoded with fields)
    short       oldspecial;      //SoM: 3/6/2000: Remember if a sector was secret (for automap)
    short       tag;
    int nexttag,firsttag;        //SoM: 3/6/2000: by killough: improves searches for tags.

    // 0 = untraversed, 1,2 = sndlines -1
    short       soundtraversed;
    short       floortype;  // see floortype_t beffor 

    // thing that made a sound (or null)
    mobj_t*     soundtarget;

    // mapblock bounding box for height changes
    int         blockbox[4];

    // origin for any sounds played by the sector
    degenmobj_t soundorg;

    // if == validcount, already checked
    int         validcount;

    // list of mobjs in sector
    mobj_t*     thinglist;

    //SoM: 3/6/2000: Start boom extra stuff
    // thinker_t for reversable actions
    // make thinkers on floors, ceilings, lighting, independent of one another
    void *floordata;
                     // ZMalloc PU_LEVSPEC, in EV_DoFloor
    void *ceilingdata;
    void *lightingdata;
  
    // lockout machinery for stairbuilding
    int stairlock;   // -2 on first locked -1 after thinker done 0 normally
    int prevsec;     // -1 or number of sector for previous step
    int nextsec;     // -1 or number of next step sector
  
    // floor and ceiling texture offsets
    fixed_t   floor_xoffs,   floor_yoffs;
    fixed_t ceiling_xoffs, ceiling_yoffs;

    // [WDJ] 4/20/2010  modelsec is model sector for special linedefs.
    // It will be valid when model != SM_normal.
    // Testing modelsec for water is invalid, it is also used for colormap.
    int modelsec;    // other sector number, or -1 if no other sector
    sector_model_e  model;  // Boom or Legacy special sector  [WDJ] 11/14/2009
    
    // [WDJ] 3/2011, (killough 8/28/98 Friction as sector property).
    // friction=MAXINT when unused
    fixed_t  friction;
    int movefactor;
  
    int floorlightsec, ceilinglightsec;
    int teamstartsec;

    int bottommap, midmap, topmap; // dynamic colormaps
        // -1 is invalid, valid 0..
  
    // list of mobjs that are at least partially in the sector
    // thinglist is a subset of touching_thinglist
    struct msecnode_s *touching_thinglist;               // phares 3/14/98  
                                    // nodes are ZMalloc PU_LEVEL, by P_GetSecnode
    //SoM: 3/6/2000: end stuff...

    // list of ptrs to lines that have this sector as a side
    int                 linecount;
    struct line_s**     linelist;  // [linecount] size

    //SoM: 2/23/2000: Improved fake floor hack
    ffloor_t *          ffloors;    // 3D floor list
                                    // ZMalloc PU_LEVEL, in P_AddFakeFloor
    int  *              attached;   // list of control sectors (by secnum)
                                    // realloc in P_AddFakeFloor
                                    // [WDJ] 7/2010 deallocate in P_SetupLevel
    int                 numattached;
    ff_lightlist_t *    lightlist;  // fake floor lights
                                    // ZMalloc PU_LEVEL, in R_Prep3DFloors
    int                 numlights;
    boolean             moved;  // floor was moved

    int                 validsort; //if == validsort already been sorted
    boolean             added;

    // SoM: 4/3/2000: per-sector colormaps!
    extracolormap_t*    extra_colormap;  // (ref) using colormap for this frame
         // selected from bottommap,midmap,topmap, from special linedefs

    // ----- for special tricks with HW renderer -----
    boolean             pseudoSector;
    boolean             virtualFloor;
    fixed_t             virtualFloorheight;
    boolean             virtualCeiling;
    fixed_t             virtualCeilingheight;
    linechain_t *       sectorLines;
    struct sector_s **  stackList;
#ifdef SOLARIS
    // Until we get Z_MallocAlign sorted out, make this a float
    // so that we don't get alignment problems.
    float               lineoutLength;
#else
    double              lineoutLength;
#endif
    // ----- end special tricks -----
} sector_t;



//
// The SideDef.
//

typedef struct
{
    // add this to the calculated texture column
    fixed_t     textureoffset;

    // add this to the calculated texture top
    fixed_t     rowoffset;

    // Texture indices.
    // We do not maintain names here.
    // 0= no-texture, will never have -1
    short       toptexture;
    short       bottomtexture;
    short       midtexture;

    //SoM: 3/6/2000: This is the special of the linedef this side belongs to.
    short       linedef_special;

    // Sector the SideDef is facing.
    sector_t*   sector;
} side_t;



//
// Move clipping aid for LineDefs.
//
typedef enum
{
    ST_HORIZONTAL,
    ST_VERTICAL,
    ST_POSITIVE,
    ST_NEGATIVE

} slopetype_t;



typedef struct line_s
{
    // Vertices, from v1 to v2.
    vertex_t*   v1;  // linedef start vertex
    vertex_t*   v2;  // linedef end vertex
       // side1 is right side when looking from v1 to v2  (start to end)

    // Precalculated v2 - v1 for side checking.
    fixed_t     dx;
    fixed_t     dy;

    // Animation related.
    uint16_t	flags;
        // [WDJ] flags should be unsigned, but binary gets larger??
        // test shows that unsigned costs 4 more bytes per (flag & ML_bit)
    short       special;  // special linedef code
    short       tag;	  // special affects sectors with same tag id

    // Visual appearance: SideDefs.
    uint16_t    sidenum[2]; //  sidenum[1] will be NULL_INDEX if one sided
    // [smite] TODO actually they should be side_t pointers...

    // Neat. Another bounding box, for the extent
    //  of the LineDef.
    fixed_t     bbox[4];

    // To aid move clipping.
    slopetype_t slopetype;

    // Front and back sector.
    // Note: redundant? Can be retrieved from SideDefs.
    sector_t*   frontsector; // sidedef[0] sector (right side, required)
    sector_t*   backsector;  // sidedef[1] sector (left side, optional)

    // if == validcount, already checked
    int         validcount;

    // thinker_t for reversable actions
    void*       specialdata;

    // wallsplat_t list
    void*       splats;
    
    //SoM: 3/6/2000
    int translu_eff;       // translucency effect table, 0 == none 
                           // TRANSLU_med or (TRANSLU_ext + translu_store index)
    int firsttag,nexttag;  // improves searches for tags.

//    int ecolormap;         // SoM: Used for 282 linedefs
} line_t;




//
// A SubSector.
// References a Sector or portion of a sector.  Is a convex polygon.
// When the original sector is not convex, the nodebuilder divides it into
// subsectors until it has convex polygons.
// Basically, this is a list of LineSegs,
//  indicating the visible walls that define
//  (all or some) sides of a convex BSP leaf.
//
typedef struct subsector_s
{
    sector_t*   sector;   // (ref) part of this sector, from segs->sector of firstline
    // numlines and firstline are from the subsectors lump (nodebuilder)
    // [WDJ] some wad may be large enough to overflow signed short.
    unsigned short  numlines;   // number of segs in this subsector
    unsigned short  firstline;  // index into segs lump (loaded from wad)
    // floorsplat_t list
    void*       splats;
    //Hurdler: added for optimized mlook in hw mode
    int         validcount; 
} subsector_t;


// SoM: 3/6/200
//
// Sector list node showing all sectors an object appears in.
//
// There are two threads that flow through these nodes. The first thread
// starts at touching_thinglist in a sector_t and flows through the m_snext
// links to find all mobjs that are entirely or partially in the sector.
// The second thread starts at touching_sectorlist in an mobj_t and flows
// through the m_tnext links to find all sectors a thing touches. This is
// useful when applying friction or push effects to sectors. These effects
// can be done as thinkers that act upon all objects touching their sectors.
// As an mobj moves through the world, these nodes are created and
// destroyed, with the links changed appropriately.
//
// For the links, NULL means top or end of list.

typedef struct msecnode_s
{
  sector_t          *m_sector; // a sector containing this object
  struct mobj_s     *m_thing;  // this object
  struct msecnode_s *m_tprev;  // prev msecnode_t for this thing
  struct msecnode_s *m_tnext;  // next msecnode_t for this thing
  struct msecnode_s *m_sprev;  // prev msecnode_t for this sector
  struct msecnode_s *m_snext;  // next msecnode_t for this sector
  boolean visited; // killough 4/4/98, 4/7/98: used in search algorithms
} msecnode_t;


//Hurdler: 04/12/2000: for now, only used in hardware mode
//                     maybe later for software as well?
//                     that's why it's moved here
typedef struct light_s 
{
    USHORT  type;           // light,... (cfr #define in hwr_light.c)

    float   light_xoffset;
    float   light_yoffset;  // y offset to adjust corona's height

    ULONG   corona_color;   // color of the light for static lighting
    float   corona_radius;  // radius of the coronas

    ULONG   dynamic_color;  // color of the light for dynamic lighting
    float   dynamic_radius; // radius of the light ball
    float   dynamic_sqrradius; // radius^2 of the light ball

} light_t;


typedef struct lightmap_s 
{
    float               s[2], t[2];
    light_t             *light;
    struct lightmap_s   *next;
} lightmap_t;

#ifdef HWRENDER
# include "hardware/hw_poly.h"
#endif

//
// The LineSeg.
//
typedef struct
{
    // v1, v2, side, angle, offset, linedef are from wad segs lump
    vertex_t*   v1;  // start vertex  (derived from vertex index in wad)
    vertex_t*   v2;  // end vertex
       // side1 is right side when looking from v1 to v2  (start to end)

    int         side;
        // 0= seg is on right side of linedef
        // 1= seg is on left side of linedef (seg direction is opposite linedef)

    fixed_t     offset;
        // offset from linedef start or end, to segment vertex v1
        // when side=0, is offset from start of linedef to start of seg
        // when side=1, is offset from end of linedef to start of seg

    angle_t     angle;	// Binary Angle wad angle converted
        // EAST  = 0x00000000
        // NORTH = 0x40000000
        // WEST  = 0x80000000
        // SOUTH = 0xC0000000

    line_t*     linedef;  // (derived from linedef index in wad)
    side_t*     sidedef;  // segment sidedef (derived from linedef and side)

    // Sector references.
    // Could be retrieved from linedef, too.
    // backsector is NULL for one sided lines
    // (dervived from linedef and side)
    sector_t*   frontsector;  // sidedef sector, the segment sector/subsector, required
    sector_t*   backsector;   // side of linedef away from sector, optional

#ifdef HWRENDER   
    polyvertex_t  *pv1, *pv2; // float polygon vertex
    // length of the seg : used by the hardware renderer
    float       length;

    //Hurdler: 04/12/2000: added for static lightmap
    // hardware renderer
    lightmap_t  *lightmaps;
#endif

    // SoM: Why slow things down by calculating lightlists for every
    // thick side.
    int               numlights;
    r_lightlist_t*    rlights;
} seg_t;



//
// BSP node.
//
typedef struct
{
    // Partition line from (x,y) to x+dx,y+dy)
    fixed_t     x, y;
    fixed_t     dx, dy;

    // Bounding box for each child.
    fixed_t     bbox[2][4];
        // bbox[0]= right child, all segs of right must be within the box
        // bbox[1]= left child, all segs of left must be within the box

    // If NF_SUBSECTOR is set then rest of it is a subsector index,
    // otherwise it is another node index.
    unsigned short children[2];
        // children[0]= right
        // children[1]= left

} node_t;


// Example of column data:
//  post_t, bytes[length], post_t, bytes[length], 0xFF

// posts are runs of non masked source pixels
// Post format: post_t header, bytes[length] pixels
typedef struct
{
    byte                topdelta; 	// y offset within patch of this post
        // reads (0xFF) at column termination (not a valid post_t)
        // BP: humf, -1 with byte ! (unsigned char) test WARNING
    byte                length;         // length data bytes follows
} post_t;

// column_t is a list of 0 or more post_t, (0xFF) terminated
typedef post_t  column_t;



//
// OTHER TYPES
//




#ifndef MAXFFLOORS
#define MAXFFLOORS    40
#endif

//
// Drawseg for floor and 3D floor thickseg
//
typedef struct drawseg_s
{
    seg_t*              curline;
    int                 x1, x2;  // x1..x2

    fixed_t             scale1, scale2;  // scale x1..x2
    fixed_t             scalestep;

    // silhouette is where a drawseg can overlap a sprite
    int                 silhouette;	    // bit flags, Silhouette_e
    fixed_t             sil_top_height;     // do not clip sprites below this
    fixed_t             sil_bottom_height;  // do not clip sprites above this

    // Pointers to lists for sprite clipping,
    //  all three adjusted so [x1] is first value.
    short*              spr_topclip;     // owned array [x1..x2]
    short*              spr_bottomclip;  // owned array [x1..x2]
    short*              maskedtexturecol;  // ref to array [x1..x2]

    // 3D floors, only use what is needed, often none
    struct visplane_s*  ffloorplanes[MAXFFLOORS];
    int                 numffloorplanes;
    struct ffloor_s*    thicksides[MAXFFLOORS];
    short*              thicksidecol;
    int                 numthicksides;
    fixed_t             backscale[MAXVIDWIDTH]; // z check for sprite clipping
//    fixed_t             frontscale[MAXVIDWIDTH]; // z check for sprite clipping
} drawseg_t;


// Patches.
// A patch holds one or more columns.
// Patches are used for sprites and all masked pictures,
// and we compose textures from the TEXTURE1/2 lists
// of patches.
//
//WARNING: this structure is cloned in GlidePatch_t
// [WDJ] This is used for reading patches from wad.
struct patch_s
{
    uint16_t            width;          // bounding box size
    uint16_t            height;
    int16_t             leftoffset;     // pixels to the left of origin
    int16_t             topoffset;      // pixels below the origin
    uint32_t            columnofs[8];   // actually [width]
       // offset of each column from start of patch header
       // the [0] is &columnofs[width]
    // This is used as the head of a patch, and columnofs[8] provides
    // access to an array that is usually [64], [128], or [256].
    // This would not work if the [8] was actually enforced.
};
typedef struct patch_s patch_t;


typedef enum {
    PALETTE         = 0,  // 1 byte is the index in the doom palette (as usual)
    INTENSITY       = 1,  // 1 byte intensity
    INTENSITY_ALPHA = 2,  // 2 byte : alpha then intensity
    RGB24           = 3,  // 24 bit rgb
    RGBA32          = 4,  // 32 bit rgba
} pic_mode_t;
// a pic is an unmasked block of pixels, stored in horizontal way
//
typedef struct
{
    uint16_t  width;
    byte      zero;   // set to 0 allow autodetection of pic_t 
                      // mode instead of patch or raw
    byte      mode;   // see pic_mode_t above
    uint16_t  height;
    uint16_t  reserved1;  // set to 0
    byte      data[0];
} pic_t;


typedef enum
{
  SC_NONE = 0,
  SC_TOP = 1,
  SC_BOTTOM = 2
} spritecut_e;

// A vissprite_t is a thing
//  that will be drawn during a refresh.
// I.e. a sprite object that is partly visible.
typedef struct vissprite_s
{
    // Doubly linked list.
    struct vissprite_s* prev;
    struct vissprite_s* next;

    int                 x1;
    int                 x2;

    // for line side calculation
    fixed_t             gx;
    fixed_t             gy;

    // global bottom / top for silhouette clipping, world coordinates
    fixed_t             gz_bot;
    fixed_t             gz_top;

    // Physical bottom / top for sorting with 3D floors, world coordinates.
    fixed_t		pz_bot;
    fixed_t		pz_top;

    // horizontal position of x1
    fixed_t             startfrac;

    fixed_t             scale;

    // negative if flipped
    fixed_t             xiscale;

    fixed_t             texturemid;
    int                 patch;

    // for color translation and shadow draw,
    //  maxbright frames as well
    lighttable_t*       colormap;

    //Fab:29-04-98: for MF_SHADOW sprites, which translucency table to use
    byte*               translucentmap;
    byte		translucent_index;

    int                 mobjflags;

    // SoM: 3/6/2000: height sector for underwater/fake ceiling support
    int                 heightsec;

    //SoM: 4/3/2000: Global colormaps!
    extracolormap_t*    extra_colormap;
    fixed_t             xscale;

    fixed_t             thingheight; //The actual height of the thing (for 3D floors)
    sector_t*           sector; //The sector containing the thing.

    //SoM: Precalculated top and bottom screen coords for the sprite.
    // [WDJ] sz_ only used in r_things.c, these are NOT fixed point nor frac
    int                 sz_bot;
    int                 sz_top;

    int                 cut;  //0 for none, bit 1 for top, bit 2 for bottom
                                // OR of spritecut_e
} vissprite_t;


//
// Sprites are patches with a special naming convention
//  so they can be recognized by R_InitSprites.
// The base name is NNNNFx or NNNNFxFx, with
//  x indicating the rotation, x = 0, 1-7.
// The sprite and frame specified by a thing_t
//  is range checked at run time.
// A sprite is a patch_t that is assumed to represent
//  a three dimensional object and may have multiple
//  rotations pre drawn.
// Horizontal flipping is used to save space,
//  thus NNNNF2F5 defines a mirrored patch.
// Some sprites will only have one picture used
// for all views: NNNNF0
//
typedef struct
{
    // If rotate is false, use rot=0 for any position.
    // Note: Then as all eight entries are always available,
    //  we will fill them all with the same values.
    boolean     rotate;

    // Lump to use for view angles 0-7.
    int         lumppat[8];   // lump number 16:16 wad:lump
    short       spritelump_id[8]; // into spritelumps[]

    // Flip bit (1 = flip) to use for view angles 0-7.
    byte        flip[8];

} spriteframe_t;



//
// A sprite definition:  a number of animation frames.
//
typedef struct
{
    int                 numframes;
    spriteframe_t*      spriteframes;

} spritedef_t;


#endif
