// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: p_mobj.h 1367 2017-11-01 01:15:56Z wesleyjohnson $
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
// $Log: p_mobj.h,v $
// Revision 1.10  2004/07/27 08:19:37  exl
// New fmod, fs functions, bugfix or 2, patrol nodes
//
// Revision 1.9  2002/01/21 23:14:28  judgecutor
// Frag's Weapon Falling fixes
//
// Revision 1.8  2001/11/17 22:12:53  hurdler
// Ready to work on beta 4 ;)
//
// Revision 1.7  2001/02/24 13:35:20  bpereira
//
// Revision 1.6  2001/01/25 22:15:43  bpereira
// added heretic support
//
// Revision 1.5  2000/11/02 17:50:08  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.4  2000/04/30 10:30:10  bpereira
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
//      Map Objects, MObj, definition and handling.
//
//-----------------------------------------------------------------------------

#ifndef P_MOBJ_H
#define P_MOBJ_H

#include "doomdef.h"
  // NUMSKINCOLORS
#include "doomtype.h"

// We need the WAD data structure for Map things,
// from the THINGS lump.
#include "doomdata.h"
  // mapthing_t

// States are tied to finite states are
//  tied to animation frames.
// Needs precompiled tables/data structures.
#include "info.h"

// Basics.
#include "tables.h"
#include "m_fixed.h"

// We need the thinker_t stuff.
#include "d_think.h"



//
// NOTES: mobj_t
//
// mobj_ts are used to tell the refresh where to draw an image,
// tell the world simulation when objects are contacted,
// and tell the sound driver how to position a sound.
//
// The refresh uses the next and prev links to follow
// lists of things in sectors as they are being drawn.
// The sprite, frame, and angle elements determine which patch_t
// is used to draw the sprite if it is visible.
// The sprite and frame values are allmost allways set
// from state_t structures.
// The statescr.exe utility generates the states.h and states.c
// files that contain the sprite/frame numbers from the
// statescr.txt source file.
// The xyz origin point represents a point at the bottom middle
// of the sprite (between the feet of a biped).
// This is the default origin position for patch_ts grabbed
// with lumpy.exe.
// A walking creature will have its z equal to the floor
// it is standing on.
//
// The sound code uses the x,y, and subsector fields
// to do stereo positioning of any sound effited by the mobj_t.
//
// The play simulation uses the blocklinks, x,y,z, radius, height
// to determine when mobj_ts are touching each other,
// touching lines in the map, or hit by trace lines (gunshots,
// lines of sight, etc).
// The mobj_t->flags element has various bit flags
// used by the simulation.
//
// Every mobj_t is linked into a single sector
// based on its origin coordinates.
// The subsector_t is found with R_PointInSubsector(x,y),
// and the sector_t can be found with subsector->sector.
// The sector links are only used by the rendering code,
// the play simulation does not care about them at all.
//
// Any mobj_t that needs to be acted upon by something else
// in the play world (block movement, be shot, etc) will also
// need to be linked into the blockmap.
// If the thing has the MF_NOBLOCK flag set, it will not use
// the block links. It can still interact with other things,
// but only as the instigator (missiles will run into other
// things, but nothing can run into a missile).
// Each block in the grid is 128*128 units, and knows about
// every line_t that it contains a piece of, and every
// interactable mobj_t that has its origin contained.
//
// A valid mobj_t is a mobj_t that has the proper subsector_t
// filled in for its xy coordinates and is linked into the
// sector from which the subsector was made, or has the
// MF_NOSECTOR flag set (the subsector_t needs to be valid
// even if MF_NOSECTOR is set), and is linked into a blockmap
// block or has the MF_NOBLOCKMAP flag set.
// Links should only be modified by the P_[Un]SetThingPosition()
// functions.
// Do not change the MF_NO? flags while a thing is valid.
//
// Any questions?
//

//
// Misc. mobj flags
//
typedef enum
{
    // Call P_SpecialThing when touched.
    MF_SPECIAL          = 0x0001,
    // Blocks.
    MF_SOLID            = 0x0002,
    // Can be hit.
    MF_SHOOTABLE        = 0x0004,
    // Don't use the sector links (invisible but touchable).
    MF_NOSECTOR         = 0x0008,
    // Don't use the blocklinks (inert but displayable)
    MF_NOBLOCKMAP       = 0x0010,

    // Not to be activated by sound, deaf monster.
    MF_AMBUSH           = 0x0020,
    // Will try to attack right back.
    MF_JUSTHIT          = 0x0040,
    // Will take at least one step before attacking.
    MF_JUSTATTACKED     = 0x0080,
    // On level spawning (initial position),
    //  hang from ceiling instead of stand on floor.
    MF_SPAWNCEILING     = 0x0100,
    // Don't apply gravity (every tic),
    //  that is, object will float, keeping current height
    //  or changing it actively.
    MF_NOGRAVITY        = 0x0200,

    // Movement flags.
    // This allows jumps from high places.
    MF_DROPOFF          = 0x0400,
    // For players, will pick up items.
    MF_PICKUP           = 0x0800,
    // Player cheat. ???
    MF_NOCLIP           = 0x1000,
    // Player: keep info about sliding along walls.
    MF_SLIDE            = 0x2000,
    // Allow moves to any height, no gravity.
    // For active floaters, e.g. cacodemons, pain elementals.
    MF_FLOAT            = 0x4000,
    // Don't cross lines
    //   ??? or look at heights on teleport.
    MF_TELEPORT         = 0x8000,
    // Don't hit same species, explode on block.
    // Player missiles as well as fireballs of various kinds.
    MF_MISSILE          = 0x10000,
    // Dropped by a demon, not level spawned.
    // E.g. ammo clips dropped by dying former humans.
    MF_DROPPED          = 0x20000,
    // DOOM2: Use fuzzy draw (shadow demons or spectres),
    //  temporary player invisibility powerup.
    // LEGACY: no more for translucency, but still makes targeting harder
    MF_SHADOW           = 0x40000,
    // Flag: don't bleed when shot (use puff),
    //  barrels and shootable furniture shall not bleed.
    MF_NOBLOOD          = 0x80000,
    // Don't stop moving halfway off a step,
    //  that is, have dead bodies slide down all the way.
    MF_CORPSE           = 0x100000,
    // Floating to a height for a move, ???
    //  don't auto float to target's height.
    MF_INFLOAT          = 0x200000,

    // On kill, count this enemy object
    //  towards intermission kill total.
    // Happy gathering.
    MF_COUNTKILL        = 0x400000,

    // On picking up, count this item object
    //  towards intermission item total.
    MF_COUNTITEM        = 0x800000,

    // Special handling: skull in flight.
    // Neither a cacodemon nor a missile.
    MF_SKULLFLY         = 0x1000000,

    // Don't spawn this object
    //  in death match mode (e.g. key cards).
    MF_NOTDMATCH        = 0x2000000,

    // MBF flags, in the position that MBF originally had them.
    // This makes it easier to handle them in combination with other flags.
    MF_TOUCHY           = 0x10000000, // killough 11/98: dies when solids touch it
    MF_BOUNCES          = 0x20000000, // killough 7/11/98: for beta BFG fireballs
    MF_FRIEND           = 0x40000000, // killough 7/18/98: friendly monsters
   
    MF_TRANSLUCENT      = 0x80000000,  // from boomdeh.txt, previously was FLOORHUGGER
} mobjflag_e;


typedef enum {
    MF2_LOGRAV         =     0x00000001,      // alternate gravity setting
    MF2_WINDTHRUST     =     0x00000002,      // gets pushed around by the wind
                                              // specials
    MF2_FLOORBOUNCE    =     0x00000004,      // bounces off the floor
    MF2_THRUGHOST      =     0x00000008,      // missile will pass through ghosts
    MF2_FLY            =     0x00000010,      // fly mode is active
    MF2_FOOTCLIP       =     0x00000020,      // if feet are allowed to be clipped
    MF2_SPAWNFLOAT     =     0x00000040,      // spawn random float z
    MF2_NOTELEPORT     =     0x00000080,      // does not teleport
    MF2_RIP            =     0x00000100,      // missile rips through solid
                                              // targets
    MF2_PUSHABLE       =     0x00000200,      // can be pushed by other moving
                                              // mobjs
    MF2_SLIDE          =     0x00000400,      // slides against walls
    MF2_ONMOBJ         =     0x00000800,      // mobj is resting on top of another
                                              // mobj
    MF2_PASSMOBJ       =     0x00001000,      // Enable z block checking.  If on,
                                              // this flag will allow the mobj to
                                              // pass over/under other mobjs.
    MF2_CANNOTPUSH     =     0x00002000,      // cannot push other pushable mobjs
    MF2_FEETARECLIPPED =     0x00004000,      // a mobj's feet are now being cut
    MF2_BOSS           =     0x00008000,      // mobj is a major boss
    MF2_FIREDAMAGE     =     0x00010000,      // does fire damage
    MF2_NODMGTHRUST    =     0x00020000,      // does not thrust target when
                                              // damaging        
    MF2_TELESTOMP      =     0x00040000,      // mobj can stomp another
    MF2_FLOATBOB       =     0x00080000,      // use float bobbing z movement
    MF2_DONTDRAW       =     0x00100000,      // don't generate a vissprite

    // extra
    MF2_FLOORHUGGER    =     0x00200000,      // stays on the floor

    // for chase camera, don't be blocked by things (partial clipping)
    MF2_NOCLIPTHING    =     0x00400000,
} mobjflag2_e;


// [WDJ] Original flag positions, for deh, savegame fixes.
typedef enum {
    // Player sprites in multiplayer modes are modified
    //  using an internal color lookup table for re-indexing.
    // If 0x4 0x8 or 0xc,
    //  use a translation table for player colormaps
    MFO_TRANSLATION2     = 0x0C000000,    // original 2color
    MFO_TRANSLATION4     = 0x3C000000,    // 4color
    MFO_TRANSSHIFT       = 26,  // to shift MF_TRANSLATION bits to INT
 
    // Earlier Legacy savegames only.
    // for chase camera, don't be blocked by things (partial clipping)
    MFO_NOCLIPTHING      = 0x40000000,
} old_mobjflag_e;



// Translation Flags, and other displaced flags.
typedef enum {
    // Player sprites in multiplayer modes are modified
    // using an internal color lookup table for re-indexing.
    // Use a translation table for player colormaps.
    // Already shifted to indexing position, so MF_TO_SKINMAP has no shift.
    MFT_TRANSLATION6     = 0x00003F00,    // 6 bit color
    MFT_TRANSSHIFT       = 8,
    // If MFT_TRANSSHIFT is changed, defines in r_draw.h must be fixed too.
} mobjtflag_e;


// Same friendness
// Used often in MBF.
// True if both FRIEND, or if both not FRIEND.
#define SAME_FRIEND(s,t)  ((((s)->flags ^ (t)->flags) & MF_FRIEND) == 0)
#define BOTH_FRIEND(s,t)  ((s)->flags & (t)->flags & MF_FRIEND)


//
//  New mobj extra flags
//
//added:28-02-98:
typedef enum
{
    // The mobj stands on solid floor (not on another mobj or in air)
    MF_ONGROUND          = 0x0001,
    // The mobj just hit the floor while falling, this is cleared on next frame
    // (instant damage in lava/slime sectors to prevent jump cheat..)
    MF_JUSTHITFLOOR      = 0x0002,
    // The mobj stands in a sector with water, and touches the surface
    // this bit is set once and for all at the start of mobjthinker
    MF_TOUCHWATER        = 0x0004,
    // The mobj stands in a sector with water, and his waist is BELOW the water surface
    // (for player, allows swimming up/down)
    MF_UNDERWATER        = 0x0008,
    // Set by P_MovePlayer() to disable gravity add in P_MobjThinker() ( for gameplay )
    MF_SWIMMING          = 0x0010,
    // used for client prediction code, player can't be blocked in z by walls
    // it is set temporarely when player follow the spirit
    MF_NOZCHECKING       = 0x0020,
    // "Friendly"; the mobj ignores players
    MF_IGNOREPLAYER	 = 0x0040,
    // Actor will predict where the player will be
    MF_PREDICT		 = 0x0080,
  // MBF
    // Object is falling.
    MF_FALLING           = 0x0100,
    // Object is armed for TOUCHY explode.
    MF_ARMED             = 0x0200
} mobjeflag_e;


#if NUMSKINCOLORS > 16
#error MF_TRANSLATION can only handle NUMSKINCOLORS <= 16
#endif


// Map Object definition.
typedef struct mobj_s
{
    // List: thinker links.
    thinker_t           thinker;

    // Drawing position, and sound position.
    // Must be identical to xyz_t, replaces use of degenmobj_t.
    fixed_t             x, y, z;

    // Momentums, used to update position.
    fixed_t             momx, momy, momz;
   
    // The closest interval over all contacted Sectors (or Things).
    fixed_t             floorz;
    fixed_t             ceilingz;
    fixed_t             dropoffz;  // MBF, dropoff floor

    // For movement checking.
    fixed_t             radius;
    fixed_t             height;

    //More drawing info: to determine current sprite.
    angle_t             angle;  // orientation
    spritenum_t         sprite; // used to find patch_t and flip value
    uint32_t            frame;  // frame number, plus bits see p_pspr.h

    // If == validcount, already checked.
    //int                 validcount;

    uint32_t            flags;  // mobjflag_e
    uint32_t            flags2; // heretic mobjflag2_e
    uint32_t            tflags; // translation, drawing, settings, mobjtflag_e
    uint32_t            eflags; //added:28-02-98: mobjeflag_e
    int                 special1;
    int                 special2;
    int                 health;
    int                 tics;   // state tic counter

    mobjtype_t          type;   // MT_*  (MT_PLAYER, MT_VILE, MT_BFG, etc.)

  // [WDJ] Better alignment if ptrs are kept together, and int16 are together.

    mobjinfo_t        * info;   // &mobjinfo[mobj->type]
    state_t           * state;

    // More list: links in sector (if needed)
    struct mobj_s     * snext;
    struct mobj_s     * sprev;

    // Interaction info, by BLOCKMAP.
    // Links in blocks (if needed).
    struct mobj_s     * bnext;
    struct mobj_s     * bprev;

    struct subsector_s* subsector;

    // a linked list of sectors where this object appears
    struct msecnode_s * touching_sectorlist;

    //Fab:02-08-98
    // Skin overrides 'sprite' when non NULL (currently hack for player
    // bodies so they 'remember' the skin).
    // Secondary use is when player die and we play the die sound.
    // Problem is he has already respawn and want the corpse to
    // play the sound !!! (yeah it happens :\)
    void              * skin;
   
    // Thing being chased/attacked (or NULL),
    // also the originator for missiles.
  union {
    struct mobj_s     * target;
    uint32_t            target_id; // used during loading
  };

    // Thing being chased/attacked for tracers.
  union {
    struct mobj_s     * tracer;
    uint32_t            tracer_id; // used during loading
  };

  union {
    struct mobj_s     * lastenemy;    // MBF, last known enemy
    uint32_t            lastenemy_id; // used during loading
  };

    // Additional info record for player avatars only.
    // Only valid if type == MT_PLAYER
    struct player_s   * player;

    // For nightmare and itemrespawn respawn.
    mapthing_t        * spawnpoint;

    // Nodes
    struct mobj_s     * nextnode;   // Next node object to chase after touching
                                    // current target (which must be MT_NODE).
    struct mobj_s     * targetnode; // Target node to remember when encountering a player
    int			nodescript; // Script to run when this node is touched
    int			nodewait;   // How many ticks to wait at this node

    // Player number last looked for.
    int8_t              lastlook;

    // Movement direction, movement generation (zig-zagging).
    int8_t              movedir;      // 0-7
    int16_t             movecount;    // when 0, select a new dir

    // Reaction time: if non 0, don't attack yet.
    // Used by player to freeze a bit after teleporting.
    int16_t             reactiontime;

    // If >0, the target will be chased no matter what (even if shot).
    int16_t             threshold;
    int16_t             strafecount;  // MBF, monster strafing
    int16_t             pursuecount;  // MBF, how long to pursue
    int16_t             tipcount;     // MBF, torque simulation (gear)

    //SoM: Friction.
    int friction;
    int movefactor;

    // Support for Frag Weapon Falling
    // This field valid only for MF_DROPPED ammo and weapn objects
    int dropped_ammo_count;

    // WARNING : new field are not automaticely added to save game 
} mobj_t;


// check mobj against water content, before movement code
void P_MobjCheckWater (mobj_t* mobj);

void P_SpawnMapthing (mapthing_t*  mthing);
// [WJD] spawn as playernum
void P_SpawnPlayer(mapthing_t * mthing, int playernum );

int P_HitFloor(mobj_t *thing);


// Extra Mapthing
mapthing_t * P_Get_Extra_Mapthing( uint16_t flags );
void P_Free_Extra_Mapthing( mapthing_t * mthing );
void P_Clear_Extra_Mapthing( void );

// Returns an index number for a mapthing, first index is 1
// Returns 0 if not found
unsigned int P_Extra_Mapthing_Index( mapthing_t * mtp );

// Traverse all Extra Mapthing that are in use
mapthing_t * P_Traverse_Extra_Mapthing( mapthing_t * prev );

// MBF
#define  SENTIENT(mo)  (((mo)->health > 0) && ((mo)->info->seestate))

// killough 11/98:
// For torque simulation:
// MBF, PrBoom  TIPSHIFT=OVERDRIVE, MAXTIPCOUNT=MAXGEAR
#define TIPSHIFT     6
#define MAXTIPCOUNT  22

#endif
