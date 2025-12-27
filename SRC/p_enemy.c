// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: p_enemy.c 1399 2018-07-02 03:41:01Z wesleyjohnson $
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2016 by DooM Legacy Team.
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
// $Log: p_enemy.c,v $
// Revision 1.20  2004/07/27 08:19:36  exl
// New fmod, fs functions, bugfix or 2, patrol nodes
//
// Revision 1.19  2002/11/30 18:41:20  judgecutor
//
// Revision 1.18  2002/09/27 16:40:09  tonyd
// First commit of acbot
//
// Revision 1.17  2002/09/17 21:20:03  hurdler
// Quick hack for hacx freeze
//
// Revision 1.16  2002/08/13 01:14:20  ssntails
// A_BossDeath fix (I hope!)
//
// Revision 1.15  2002/07/24 22:37:31  ssntails
// Fix for Keen deaths in custom maps.
//
// Revision 1.14  2001/07/28 16:18:37  bpereira
// Revision 1.13  2001/05/27 13:42:47  bpereira
//
// Revision 1.12  2001/04/04 20:24:21  judgecutor
// Added support for the 3D Sound
//
// Revision 1.11  2001/03/30 17:12:50  bpereira
//
// Revision 1.10  2001/03/19 21:18:48  metzgermeister
//   * missing textures in HW mode are replaced by default texture
//   * fixed crash bug with P_SpawnMissile(.) returning NULL
//   * deep water trick and other nasty thing work now in HW mode (tested with tnt/map02 eternal/map02)
//   * added cvar gr_correcttricks
//
// Revision 1.9  2001/01/25 22:15:43  bpereira
// added heretic support
//
// Revision 1.8  2000/10/21 08:43:30  bpereira
// Revision 1.7  2000/10/08 13:30:01  bpereira
// Revision 1.6  2000/10/01 10:18:17  bpereira
// Revision 1.5  2000/04/30 10:30:10  bpereira
//
// Revision 1.4  2000/04/11 19:07:24  stroggonmeth
// Finished my logs, fixed a crashing bug.
//
// Revision 1.3  2000/04/04 00:32:46  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Enemy thinking, AI.
//      Action Pointer Functions
//      that are associated with states/frames.
//
//-----------------------------------------------------------------------------

#include "doomincl.h"
#include "p_local.h"
#include "p_tick.h"
  // think
#include "p_inter.h"
  // P_KillMobj
#include "g_game.h"
#include "r_main.h"
#include "r_state.h"
#include "s_sound.h"
#include "m_random.h"
#include "t_script.h"


#include "hardware/hw3sound.h"

void A_Fall(mobj_t *actor);
static void FastMonster_OnChange(void);
void A_FaceTarget(mobj_t* actor);
static int P_IsUnderDamage(mobj_t *actor);
void CV_monster_OnChange(void);

// enable the solid corpses option : still not finished
consvar_t cv_solidcorpse =
  {"solidcorpse","0", CV_NETVAR | CV_SAVE, CV_OnOff};
consvar_t cv_fastmonsters =
  {"fastmonsters","0", CV_NETVAR | CV_CALL, CV_OnOff, FastMonster_OnChange};
consvar_t cv_predictingmonsters =
  {"predictingmonsters","0", CV_NETVAR | CV_SAVE, CV_OnOff};	//added by AC for predmonsters

CV_PossibleValue_t mongravity_cons_t[]={
   {0,"Off"},
   {1,"MBF"},
   {2,"On"},
   {0,NULL}};
consvar_t cv_monstergravity =
  {"monstergravity","2", CV_NETVAR | CV_SAVE, mongravity_cons_t };

// DarkWolf95: Monster Behavior
CV_PossibleValue_t monbehavior_cons_t[]={
   {0,"Normal"},
   {1,"Coop"},
   {2,"Infight"},
   {3,"Force Coop"},
   {4,"Force Infight"},
   {5,"No Infight"},
   {0,NULL}};
consvar_t cv_monbehavior =
  { "monsterbehavior", "0", CV_NETVAR | CV_SAVE | CV_CALL, monbehavior_cons_t, CV_monster_OnChange };

CV_PossibleValue_t monsterfriction_t[] = {
   {0,"None"},
   {1,"MBF"},
   {2,"Momentum"},
   {3,"Heretic"},
   {20,"Normal"},
   {0,NULL} };
consvar_t cv_monsterfriction =
  {"monsterfriction","2", CV_NETVAR | CV_SAVE | CV_CALL, monsterfriction_t, CV_monster_OnChange};

// Monster stuck on door edge
CV_PossibleValue_t doorstuck_t[] = {
   {0,"None"},
   {1,"MBF"},
   {2,"Boom"},
   {0,NULL} };
consvar_t cv_doorstuck =
  {"doorstuck","2", CV_NETVAR | CV_SAVE | CV_CALL, doorstuck_t, CV_monster_OnChange};



typedef enum
{
    DI_EAST,
    DI_NORTHEAST,
    DI_NORTH,
    DI_NORTHWEST,
    DI_WEST,
    DI_SOUTHWEST,
    DI_SOUTH,
    DI_SOUTHEAST,
    DI_NODIR,
    NUMDIRS
} dirtype_t;


//
// P_NewChaseDir related LUT.
//
static dirtype_t opposite[] =
{
  DI_WEST, DI_SOUTHWEST, DI_SOUTH, DI_SOUTHEAST,
  DI_EAST, DI_NORTHEAST, DI_NORTH, DI_NORTHWEST, DI_NODIR
};

static dirtype_t diags[] =
{
    DI_NORTHWEST, DI_NORTHEAST, DI_SOUTHWEST, DI_SOUTHEAST
};




static
void FastMonster_OnChange(void)
{
static boolean fast=false;
static const struct
    {
        mobjtype_t type;
        int speed[2];
    } MonsterMissileInfo[] =
    {
        // doom
        { MT_BRUISERSHOT, {15, 20}},
        { MT_HEADSHOT,    {10, 20}},
        { MT_TROOPSHOT,   {10, 20}},
        
        // heretic
        { MT_IMPBALL,     {10, 20}},
        { MT_MUMMYFX1,    { 9, 18}},
        { MT_KNIGHTAXE,   { 9, 18}},
        { MT_REDAXE,      { 9, 18}},
        { MT_BEASTBALL,   {12, 20}},
        { MT_WIZFX1,      {18, 24}},
        { MT_SNAKEPRO_A,  {14, 20}},
        { MT_SNAKEPRO_B,  {14, 20}},
        { MT_HEADFX1,     {13, 20}},
        { MT_HEADFX3,     {10, 18}},
        { MT_MNTRFX1,     {20, 26}},
        { MT_MNTRFX2,     {14, 20}},
        { MT_SRCRFX1,     {20, 28}},
        { MT_SOR2FX1,     {20, 28}},
        
        { -1, {-1, -1} } // Terminator
    };

    int i;
    if( cv_fastmonsters.EV && !fast )
    {
        for (i=S_SARG_RUN1 ; i<=S_SARG_PAIN2 ; i++)
            states[i].tics >>= 1;
        fast=true;
    }
    else
    if( !cv_fastmonsters.EV && fast )
    {
        for (i=S_SARG_RUN1 ; i<=S_SARG_PAIN2 ; i++)
            states[i].tics <<= 1;
        fast=false;
    }

    for(i = 0; MonsterMissileInfo[i].type != -1; i++)
    {
        mobjinfo[MonsterMissileInfo[i].type].speed
            = MonsterMissileInfo[i].speed[cv_fastmonsters.EV]<<FRACBITS;
    }
}


// MBF controls
static void mbf_OnChange( void );


consvar_t cv_monster_remember =
  {"mon_remember","1", CV_NETVAR | CV_SAVE, CV_OnOff};
  // Boom monsters_remember

consvar_t cv_mbf_dropoff = {"dropoff","1", CV_NETVAR | CV_SAVE, CV_OnOff};
  // !comp[comp_dropoff]
consvar_t cv_mbf_falloff = {"falloff","1", CV_NETVAR | CV_SAVE, CV_OnOff};
  // !comp[comp_falloff]

consvar_t cv_mbf_monster_avoid_hazard =
  {"mon_avoidhazard","1", CV_NETVAR | CV_SAVE, CV_OnOff};
  // MBF monster_avoid_hazards
consvar_t cv_mbf_monster_backing =
  {"mon_backing","0", CV_NETVAR | CV_SAVE, CV_OnOff};
  // MBF monster_backing
consvar_t cv_mbf_pursuit = {"pursuit","0", CV_NETVAR | CV_SAVE, CV_OnOff};
  // !comp[comp_pursuit]
consvar_t cv_mbf_distfriend =
  {"distfriend","128", CV_NETVAR | CV_VALUE | CV_SAVE | CV_CALL, CV_uint16, mbf_OnChange};
  // MBF distfriend (0..999)
consvar_t cv_mbf_staylift = {"staylift","1", CV_NETVAR | CV_SAVE, CV_OnOff};
  // !comp[comp_staylift]
consvar_t cv_mbf_help_friend = {"helpfriend","1", CV_NETVAR | CV_SAVE, CV_OnOff};
  // MBF help_friends
consvar_t cv_mbf_monkeys = {"monkeys","0", CV_NETVAR | CV_SAVE, CV_OnOff};
  // MBF monkeys (climb steep stairs)
#ifdef DOGS   
CV_PossibleValue_t dogs_cons_t[] = {{0,"MIN"}, {9,"MAX"}, {0,NULL} };
consvar_t cv_mbf_dogs = {"dogs_cnt","0", CV_NETVAR | CV_SAVE, dogs_cons_t};
  // MBF single player (and coop) dogs (0..9)
consvar_t cv_mbf_dog_jumping = {"dogjump","1", CV_NETVAR | CV_SAVE, CV_OnOff};
  // MBF dog_jumping
#endif

static void mbf_OnChange( void )
{
    // Demo sets EV_mbf_distfriend.
    EV_mbf_distfriend = cv_mbf_distfriend.value << FRACBITS;  // to fixed_t
}



// Infight settings translated to INFT_ values.  MBF demo sets infight.
byte  monster_infight; //DarkWolf95:November 21, 2003: Monsters Infight!
byte  monster_infight_deh; // DEH input.

       byte  EN_monster_friction;
static byte  EN_mbf_enemyfactor;
static byte  EN_monster_momentum;
       byte  EN_skull_limit;  // comp[comp_pain], enable pain skull limits
       byte  EN_old_pain_spawn;  // comp[comp_skull], can spit skull through walls
static byte  EN_doorstuck;  // !comp[comp_doorstuck], Boom doorstuck or MBF doorstuck
static byte  EN_mbf_doorstuck;  // MBF doorstuck
       byte  EN_mbf_speed;  // p_spec


// [WDJ] Friction got too complicated, use lookup table.
typedef enum {
  FRE_monster_friction = 0x01,
  FRE_monster_momentum = 0x02,
  FRE_mbf_enemyfactor = 0x04,
} friction_en_e;

static const byte friction_table[] =
{
  0,   // None
  FRE_monster_friction | FRE_mbf_enemyfactor,  // MBF
  FRE_monster_friction | FRE_monster_momentum,  // Momentum
  // Heretic demos have FR_orig friction, with special ice sector handling
  // in P_Thrust (so monsters slip only on ice sector)
  0,   // Heretic
};


// [WDJ] Monster friction, doorstuck, infight
void CV_monster_OnChange(void)
{
    // Set monster friction for Boom, MBF, prboom demo, by cv_monsterfriction.EV = 1.
    if( (demoplayback && (friction_model != FR_legacy))
        || ( cv_monsterfriction.EV > 3 ) )
    {
        EN_mbf_enemyfactor = (friction_model >= FR_mbf) && (friction_model <= FR_prboom);
        // Where monster friction is determined by friction model,
        // demo settings.
        // If cv_monsterfriction == 0x80, then EN_monster_friction has
        // already been set from the Boom demo compatiblity flag.
        if( cv_monsterfriction.EV < 0x80 )
            EN_monster_friction = EN_mbf_enemyfactor;  // MBF, PrBoom default
        EN_monster_momentum = 0;  // 2=momentum
    }
    else
    {
        // Legacy Demo, and User settings.
        // Legacy Demo sets through cv_monsterfriction.
        // default: 2= momentum
        register byte ft = friction_table[cv_monsterfriction.EV];
        EN_monster_friction = (ft & FRE_monster_friction);
        EN_mbf_enemyfactor = (ft & FRE_mbf_enemyfactor);
        EN_monster_momentum = (ft & FRE_monster_momentum);
    }
#ifdef FRICTIONTHINKER
    EN_boom_friction_thinker = (friction_model == FR_boom)
           && EN_variable_friction;
#endif

    // Demo sets through cv_doorstuck.
    EN_mbf_doorstuck = (cv_doorstuck.EV == 1);  // 1=MBF
    EN_doorstuck = (cv_doorstuck.EV > 0 );  // 0=none, 2=Boom

    // Demo sets infight through cv_monbehavior.
    // Monster Infight enables, can be changed during game.
    // Doom normal is infight, no coop (see Boom).
    monster_infight = monster_infight_deh;  // from DEH
    switch( cv_monbehavior.EV )  // from menu option, or demo
    {
     case 0: // Normal  (infight)
     case 2: // Infight default
       if( monster_infight_deh == INFT_none )  // no input
         monster_infight = INFT_infight;
       break;
     case 1: // Coop default
       if( monster_infight_deh == INFT_none )  // no input
         monster_infight = INFT_coop;
       break;
     case 3: // Coop forced
       monster_infight = INFT_coop;
       break;
     case 4: // Infight forced
       monster_infight = INFT_infight;
       break;
     case 5: // No Infight
       monster_infight = INFT_none;
       break;
    }
}


// local version control
void DemoAdapt_p_enemy( void )
{
    // Demo sets cv_monsterfriction.EV, and other cv_xxx.EV.
    CV_monster_OnChange();

    if( demoversion < 200 )
    {
        EN_skull_limit = ( demoversion <= 132 );  // doom demos
        EN_old_pain_spawn = ( demoversion < 143 );
    }

    EN_mbf_speed = EN_mbf || (EV_legacy >= 145);  // Legacy 1.45, 1.46

#if 1
    if( demoplayback && verbose > 1 )
    { 
        GenPrintf(EMSG_ver, "friction_model=%i, EN_monster_friction=%i\n",
                friction_model, EN_monster_friction );
        GenPrintf(EMSG_ver, "EN_mbf_enemyfactor=%i, EN_monster_momentum=%i\n",
                EN_mbf_enemyfactor,  EN_monster_momentum );
        GenPrintf(EMSG_ver, "EN_skull_limit=%i, EN_old_pain_spawn=%i, EN_doorstuck=%i, EN_mbf_doorstuck=%i\n",
                EN_skull_limit, EN_old_pain_spawn, EN_doorstuck, EN_mbf_doorstuck );
    }
#endif
}


//
// ENEMY THINKING
// Enemies are always spawned
// with targetplayer = -1, threshold = 0
// Most monsters are spawned unaware of all players,
// but some can be made preaware
//


//
// Called by P_NoiseAlert.
// Recursively traverse adjacent sectors,
// sound blocking lines cut off traversal.
//

static mobj_t*         soundtarget;

//  soundblocks : 0..1 number of sound blocking linedefs
static void P_RecursiveSound ( sector_t*  sec, byte soundblocks )
{
    int         i;
    line_t*     check;
    sector_t*   other;

    // wake up all monsters in this sector
    if (sec->validcount == validcount
        && sec->soundtraversed <= soundblocks+1)
    {
        return;         // already flooded
    }

    sec->validcount = validcount;
    sec->soundtraversed = soundblocks+1;
    P_SetReference(sec->soundtarget, soundtarget);
    sec->soundtarget = soundtarget;

    for (i=0 ; i<sec->linecount ; i++)
    {
        // for each line of the sector linelist
        check = sec->linelist[i];
        if (! (check->flags & ML_TWOSIDED) )
            continue;  // nothing on other side

        P_LineOpening (check);

        if (openrange <= 0)
            continue;   // closed door

        if ( sides[ check->sidenum[0] ].sector == sec)
            other = sides[ check->sidenum[1] ].sector;
        else
            other = sides[ check->sidenum[0] ].sector;

        if (check->flags & ML_SOUNDBLOCK)
        {
            // Sound blocking linedef
            // If 0, then continue recursion with 1 soundblock.
            // If 1, then this is second, which blocks the sound.
            if (soundblocks == 0)
                P_RecursiveSound (other, 1);
        }
        else
            P_RecursiveSound (other, soundblocks);
    }
}



//
// P_NoiseAlert
// If a monster yells at a player,
// it will alert other monsters to the player.
//
void P_NoiseAlert ( mobj_t* target, mobj_t* emmiter )
{
    soundtarget = target;
    validcount++;
    P_RecursiveSound (emmiter->subsector->sector, 0);
}




//
// P_CheckMeleeRange
//
static boolean P_CheckMeleeRange (mobj_t* actor)
{
    mobj_t *    pl = actor->target;
    fixed_t     dist;

    if(!pl)
        goto ret_false;

    // [WDJ] prboom, killough, friend monsters do not attack other friend
    if( BOTH_FRIEND(actor, pl) )
        goto ret_false;

    dist = P_AproxDistance (pl->x-actor->x, pl->y-actor->y);

    // [WDJ] FIXME pl->info may be NULL (seen in phobiata.wad)
    if (pl->info == NULL )
        goto ret_false;
    if (dist >= (MELEERANGE - 20*FRACUNIT + pl->info->radius) )
        goto ret_false;

    //added:19-03-98: check height now, so that damn imps cant attack
    //                you if you stand on a higher ledge.
    if( (EV_legacy > 111)
         && ((pl->z > actor->z + actor->height)
             || (actor->z > pl->z + pl->height) )
      )
        goto ret_false;

    if (! P_CheckSight (actor, actor->target) )
        goto ret_false;

    return true;

ret_false:
    return false;
}


// [WDJ] MBF, from MBF, PrBoom, EternityEngine
//
// P_HitFriend()
//
// killough 12/98
// This function tries to prevent shooting at friends.
// Known that (actor->flags & MF_FRIEND), checked by caller.
// Return true if aiming would hit a friend.
static boolean P_HitFriend(mobj_t *actor)
{
    mobj_t * target = actor->target;

    if( target )
    {
        P_AimLineAttack(actor,
           R_PointToAngle2(actor->x, actor->y, target->x, target->y),
           P_AproxDistance(actor->x - target->x, actor->y - target->y),
           0 );
        // Because actor is already known to have MF_FRIEND (tested by caller),
        // replace SAME_FRIEND with simpler test of MF_FRIEND.
        if( lar_linetarget
            && lar_linetarget != target
            && lar_linetarget->flags & MF_FRIEND
// MBF:	    && SAME_FRIEND(lar_linetarget, actor)
          )
            return true;
    }
    return false;   
}


//
// P_CheckMissileRange
//
// Return true if good to fire missile.
static boolean P_CheckMissileRange (mobj_t* actor)
{
    mobj_t * target = actor->target;
    fixed_t  dist;

    if (! P_CheckSight (actor, actor->target) )
        goto ret_false;

    if ( actor->flags & MF_JUSTHIT )
    {
        // the target just hit the enemy,
        // so fight back!
        actor->flags &= ~MF_JUSTHIT;

        if( EN_heretic || (demoversion < 147) )
            goto ret_true;  // Old Legacy, Old Doom

        // [WDJ] MBF, from MBF, PrBoom
        // Boom has two calls of P_Random, which affect demos
        // killough 7/18/98: no friendly fire at corpses
        // killough 11/98: prevent too much infighting among friends
        return  !(actor->flags & MF_FRIEND)
         ||( (target->health > 0)
             &&( !(target->flags & MF_FRIEND)
                 || (target->player ?
                     // PrBoom, MBF, EternityEngine use P_Random(pr_defect)
                     ((monster_infight == INFT_infight) || (PP_Random(pr_defect)>128))
                     : !(target->flags & MF_JUSTHIT) && PP_Random(pr_defect)>128)
                )
           );
    }

    // [WDJ] MBF, from MBF, PrBoom
    // killough 7/18/98: friendly monsters don't attack other friendly
    // monsters or players (except when attacked, and then only once)
    if( BOTH_FRIEND(actor, target) )
        goto ret_false;

    if (actor->reactiontime)
        goto ret_false;  // do not attack yet

    // OPTIMIZE: get this from a global checksight
    dist = P_AproxDistance ( actor->x - actor->target->x,
                             actor->y - actor->target->y) - 64*FRACUNIT;

    if (!actor->info->meleestate)
        dist -= 128*FRACUNIT;   // no melee attack, so fire more

    dist >>= FRACBITS;

    if (actor->type == MT_VILE)
    {
        if (dist > 14*64)
            goto ret_false;  // too far away
    }

    if (actor->type == MT_UNDEAD)
    {
        if (dist < 196)
            goto ret_false;  // close for fist attack
        dist >>= 1;
    }


    if (actor->type == MT_CYBORG
        || actor->type == MT_SPIDER
        || actor->type == MT_SKULL
        || actor->type == MT_IMP) // heretic monster
    {
        dist >>= 1;
    }

    if (dist > 200)
        dist = 200;

    if (actor->type == MT_CYBORG && dist > 160)
        dist = 160;

    if (PP_Random(pr_missrange) < dist)
        goto ret_false;

    if( actor->flags & MF_FRIEND )  // call is only useful if MBF FRIEND
    {
        if( P_HitFriend(actor) )
            goto ret_false;
    }

ret_true:
    return true;

ret_false:
    return false;
}


// [WDJ] MBF, from MBF, PrBoom, EternityEngine
// P_IsOnLift
//
// killough 9/9/98:
//
// Returns true if the object is on a lift. Used for AI, since it may indicate
// the need for crowded conditions, or that a monster should stay on the lift
// for a while while it goes up or down.

static boolean P_IsOnLift( const mobj_t *actor )
{
    const sector_t *sec = actor->subsector->sector;
    line_t line;
    int l;

    // Short-circuit: it's on a lift which is active.
    if( sec->floordata
        && ((thinker_t *) sec->floordata)->function.acp1 == (actionf_p1) T_PlatRaise )
        return true;

    // Check to see if it is in a sector which can be activated as a lift.
    line.tag = sec->tag;
    if( line.tag == 0 )  return false;
   
    for( l = -1; (l = P_FindLineFromLineTag(&line, l)) >= 0; )
    {
// [WDJ] This is only MBF. Does not include Heretic or expansions.       
        switch( lines[l].special )
        {
            case  10: case  14: case  15: case  20: case  21: case  22:
            case  47: case  53: case  62: case  66: case  67: case  68:
            case  87: case  88: case  95: case 120: case 121: case 122:
            case 123: case 143: case 162: case 163: case 181: case 182:
            case 144: case 148: case 149: case 211: case 227: case 228:
            case 231: case 232: case 235: case 236:
            return true;
        }
    }
    return false;
}


// [WDJ] MBF, from MBF, PrBoom, EternityEngine
// P_IsUnderDamage
// killough 9/9/98:
// Used for AI.
// Returns nonzero if the object is under damage based on their current position.
// Returns 1 if the damage is moderate (ceiling crusher up),
//        -1 if it is serious (ceiling crusher down).
static int P_IsUnderDamage(mobj_t *actor)
{
    const msecnode_t * ts;  // touching sector
    const ceiling_t * cl;   // Crushing ceiling
    int dir = 0;

    for( ts = actor->touching_sectorlist; ts; ts = ts->m_tnext )
    {
        cl = ts->m_sector->ceilingdata;
        if( cl && (cl->thinker.function.acp1 == (actionf_p1) T_MoveCeiling) )
            dir |= cl->direction;
    }
    return dir;
}


//
// P_MoveActor
// Move in the current direction,
// returns false if the move is blocked.
//
static const fixed_t xspeed[8] = {FRACUNIT,47000,0,-47000,-FRACUNIT,-47000,0,47000};
static const fixed_t yspeed[8] = {0,47000,FRACUNIT,47000,0,-47000,-FRACUNIT,-47000};

// Called multiple times in one step, by P_TryWalk, while trying to find
// valid path for an actor.
// Called by P_SmartMove, indirectly P_TryWalk, A_Chase.
// Only called for actor things, not players, nor missiles.
//  dropoff : 0, 1, 2 dog jump
// Formerly P_Move.
static boolean P_MoveActor (mobj_t* actor, byte dropoff)
{
    fixed_t  tryx, tryy;
    fixed_t  old_momx, old_momy;
    line_t*  ld;
    boolean  good;
    int      hit_block = 0;
    int      speed = actor->info->speed;

    if (actor->movedir == DI_NODIR)
        return false;

#ifdef PARANOIA
    if ((unsigned)actor->movedir >= 8)
        I_Error ("Weird actor->movedir!");
#endif

    old_momx = actor->momx;
    old_momy = actor->momy;

    // [WDJ] Monsters too, killough 10/98
    // Otherwise they move too easily on mud and ice
    if(EN_monster_friction
       && (actor->z <= actor->floorz)  // must be on floor
       && !(actor->flags2&MF2_FLY)  // not heretic flying monsters
       )
    {
        // MBF, prboom: all into speed (except ice)
        // DoomLegacy: fixed proportion of momentum and speed
        fixed_t  dx = speed * xspeed[actor->movedir];
        fixed_t  dy = speed * yspeed[actor->movedir];   
        // [WDJ] Math: Part of speed goes into momentum, R=0.5.
        // fr = FRICTION_NORM/FRACUNIT = 0.90625
        // movement due to momentum (for i=0..)
        //         = Sum( momf * speed * (fr**i)) = momf * speed / (1-fr)
        // Thus: momf = (1- (FRICTION_NORM/FRACUNIT)) * R
        // For R=0.5, momf = 0.046875 = 3/64
        float  momf = ( EN_mbf_enemyfactor )? 0.0 : 0.046875f;  // default
        float  mdiffm = 1.0f;  // movefactor diff mult (to reduce effect)
       
        P_GetMoveFactor(actor);  // sets got_movefactor, got_friction
        if( got_friction < FRICTION_NORM )
        {   // mud
            if( EN_mbf_enemyfactor )
            {   // MBF, prboom: modify speed
                mdiffm = 0.5f;  // 1/2 by MBF
            }
            else
            {   // DoomLegacy:
                // movefactor is for cmd=25, but speed=8 for player size actor
                mdiffm = 0.64f;  // by experiment, larger is slower
            }
        }
        else if (got_friction > FRICTION_NORM )
        {   // ice
            // Avoid adding momentum into walls
            tryx = actor->x + dx;
            tryy = actor->y + dy;
            // [WDJ] Do not use TryMove to check position, as it has
            // too many side effects and is not reversible.
            // Cannot just reset actor x,y.
            if (P_CheckPosition (actor, tryx, tryy))
            { // success, give it momentum too
                if( EN_mbf_enemyfactor )
                {   // MBF, prboom:
                    speed = 0;  // put it all into momf
                    momf = 0.25f;  // becomes MBF equation
                }
                else
                {   // DoomLegacy:
                    // monsters a little better in slime than humans
                    mdiffm = 0.62f; // by experiment, larger is slower
#if 0
                    // [WDJ] proportional as movefactor decreases
                    // This worked better than MBF, but still showed friction transistion accel and decel.
                    fixed_t pro = FRACUNIT * (ORIG_FRICTION_FACTOR - got_movefactor) / (ORIG_FRICTION_FACTOR*69/100);
                    if( pro > FRACUNIT )  pro = FRACUNIT; // limit to 1
                    fixed_t anti_pro = (FRACUNIT - pro);
                    momf = FixedMul( momf, pro ); // pro to momentum
                    anti_pro = FixedMul(anti_pro, anti_pro); // (1-pro)**2 to speed
                    got_movefactor = FixedMul( got_movefactor, anti_pro);
#endif		       
                }
            }
            else
            {
                momf = 0.0f;  // otherwise they get stuck at walls
            }
        }
        // [WDJ] Apply mdiffm to difference between movefactor and normal.
        // movefactor has ORIG_FRICTION_FACTOR
        // movediff = (got_movefactor - ORIG_FRICTION_FACTOR) * mdiffm
        // ratio = (ORIG_FRICTION_FACTOR + movediff) / ORIG_FRICTION_FACTOR
        float mf_ratio = (
          (((float)(got_movefactor - ORIG_FRICTION_FACTOR)) * mdiffm)
            + ((float)ORIG_FRICTION_FACTOR)
          ) / ((float)ORIG_FRICTION_FACTOR);

        // modify speed by movefactor
        speed = (int) ( speed * mf_ratio ); // MBF, prboom: no momentum

        // [WDJ] Trying to use momentum for some sectors and not for others
        // results in stall when entering an icy momentum sector,
        // and speed surge when leaving an icy momentum sector.
        // Use it for all sectors to the same degree (except in demo compatibility).
        if( momf > 0.0 )  // not disabled
        {
            // It is necessary that there be full speed at dead stop to get
            // monsters unstuck from walls and each other.
            // Some wads like TNT have overlapping monster starting positions.
            if( actor->momx || actor->momy )  // if not at standstill
                speed /= 2;  // half of speed goes to momentum
            // apply momentum
            momf *= mf_ratio;
            actor->momx += (int) ( dx * momf );
            actor->momy += (int) ( dy * momf );
        }
        else
        {
            // if momf disabled, then minimum speed
            // Minimum speed with momf causes them to walk off ledges.
            if( speed == 0 )  speed = 1;
        }
    }
    
    tryx = actor->x + speed * xspeed[actor->movedir];
    tryy = actor->y + speed * yspeed[actor->movedir];

    if( !P_TryMove (actor, tryx, tryy, dropoff) )  // caller dropoff
    {
        // blocked move
        // Monsters will be here multiple times in each step while
        // trying to find sucessful path.
        actor->momx = old_momx;  // cancel any momentum changes for next try
        actor->momy = old_momy;

        // open any specials
        // tmr_floatok, tmr_floorz returned by P_TryMove
        if( (actor->flags & MF_FLOAT) && tmr_floatok )
        {
            // must adjust height
            if (actor->z < tmr_floorz)
                actor->z += FLOATSPEED;
            else
                actor->z -= FLOATSPEED;

            actor->flags |= MF_INFLOAT;
            return true;
        }

        if (!numspechit)
            return false;

        actor->movedir = DI_NODIR;
        good = false;
        while (numspechit--)
        {
            ld = &lines[ spechit[numspechit] ];
            // [WDJ] FIXME: Monsters get stuck in the door track when
            // they see the door activation and nothing else.
            // If the special is not a door that can be opened,
            // return false
            if( P_UseSpecialLine (actor, ld, 0) )
            {
                if( EN_mbf_doorstuck && (ld == tmr_blockingline))
                   hit_block = 1;
                good = true;
            }
        }

        if( good && EN_doorstuck )
        {
            // [WDJ] Calls of P_Random here in Boom, affects Demo sync.
            // A line blocking the monster got activated, a little randomness
            // to get unstuck from door frame.
            if (EN_mbf_doorstuck)
                good = (PP_Random(pr_opendoor) >= 230) ^ (hit_block);  // MBF
            else
                good = PP_Random(pr_trywalk) & 3;  // Boom jff, 25% fail
        }
        return good;
    }
    else  // TryMove
    {
        // successful move
        if( EN_monster_momentum && tmr_dropoffline )
        {
            // [WDJ] last move sensed dropoff
            // Reduce momentum near dropoffs, friction 0x4000 to 0xE000
#define  FRICTION_DROPOFF   0x6000
            actor->momx = FixedMul(actor->momx, FRICTION_DROPOFF);
            actor->momy = FixedMul(actor->momx, FRICTION_DROPOFF);
        }
        actor->flags &= ~MF_INFLOAT;
    }


    if(! (actor->flags & MF_FLOAT) )
    {
        // Monster gravity, or MBF felldown, blocks this vanilla instant fall.
        if( ! ( (cv_monstergravity.EV == 2) 
               || ((cv_monstergravity.EV == 1) && tmr_felldown) ) )
        {
            if(actor->z > actor->floorz)
               P_HitFloor(actor);
            actor->z = actor->floorz;
        }
    }
    return true;
}


// [WDJ] From MBF, PrBoom
// P_SmartMove
// killough 9/12/98: Same as P_Move, except smarter
static boolean P_SmartMove(mobj_t *actor)
{
    mobj_t * target = actor->target;
    byte  on_lift;
    byte  under_damage = 0;
    byte  dropoff = 0;  // 0,1,2

    // killough 9/12/98: Stay on a lift if target is on one
    on_lift = cv_mbf_staylift.EV
      && target
      && target->health > 0
      && target->subsector->sector->tag == actor->subsector->sector->tag
      && P_IsOnLift(actor);

    if( cv_mbf_monster_avoid_hazard.EV )
        under_damage = P_IsUnderDamage(actor);  // 1, 0, -1

#ifdef DOGS
    // killough 10/98: allow dogs to drop off of taller ledges sometimes.
    // dropoff==1 means always allow it,
    // dropoff==2 means only up to 128 high,
    // and only if the target is immediately on the other side of the line.

    // haleyjd: allow all friends of Helper_MT Type to also jump down

    if( cv_mbf_dog_jumping.EV
        && (actor->type == MT_DOG
            || ((actor->type == helper_MT) && (actor->flags & MF_FRIEND)) )
        && target
        && SAME_FRIEND(target, actor)
        && P_AproxDistance(actor->x - target->x, actor->y - target->y)
             < FRACUNIT*144
        && (PP_Random(pr_dropoff) < 235)
      )
        dropoff = 2;
#endif

    if( !P_MoveActor(actor, dropoff) )
        return false;

    // killough 9/9/98: avoid crushing ceilings or other damaging areas
    if( on_lift
        && (PP_Random(pr_stayonlift) < 230) // Stay on lift
        && !P_IsOnLift(actor)
      )
        goto avoid_damage;
    
    if( cv_mbf_monster_avoid_hazard.EV
        && (under_damage == 0) )
    {
        // Get away from damage
        under_damage = P_IsUnderDamage(actor);
        if( (under_damage < 0) || (PP_Random(pr_avoidcrush) < 200) )
            goto avoid_damage;
    }

    return true;

avoid_damage:
    actor->movedir = DI_NODIR;  // avoid the area (most of the time anyway)
    return true;
}


line_t * trywalk_dropoffline;

//
// TryWalk
// Attempts to move actor on
// in its current (ob->moveangle) direction.
// If blocked by either a wall or an actor
// returns FALSE
// If move is either clear or blocked only by a door,
// returns TRUE and sets...
// If a door is in the way,
// an OpenDoor call is made to start it opening.
// Called multiple times in one step, while trying to find valid path.
//
static boolean P_TryWalk (mobj_t* actor)
{
    if( !P_SmartMove(actor) )  // P_MoveActor()
    {
        // record if failing due to dropoff
        if( tmr_dropoffline )
            trywalk_dropoffline = tmr_dropoffline;
        return false;
    }
    actor->movecount = PP_Random(pr_trywalk) & 0x0F;
    return true;
}



// [WDJ] MBF, From MBF, PrBoom, EternityEngine
// killough 11/98:
//
// Monsters try to move away from tall dropoffs.
//
// In Doom, they were never allowed to hang over dropoffs,
// and would remain stuck if involuntarily forced over one.
// This logic, combined with p_map.c (P_TryMove), allows
// monsters to free themselves without making them tend to
// hang over dropoffs.

static fixed_t dropoff_r_deltax, dropoff_r_deltay, dropoff_floorz;

// Always returns true, searches all lines.
static boolean PIT_AvoidDropoff(line_t *line)
{
    if( line->backsector // Ignore one-sided linedefs
        && tm_bbox[BOXRIGHT]  > line->bbox[BOXLEFT]
        && tm_bbox[BOXLEFT]   < line->bbox[BOXRIGHT]
        && tm_bbox[BOXTOP]    > line->bbox[BOXBOTTOM] // Linedef must be contacted
        && tm_bbox[BOXBOTTOM] < line->bbox[BOXTOP]
        && P_BoxOnLineSide(tm_bbox, line) == -1
      )
    {
        // Touching the line.
        fixed_t front = line->frontsector->floorheight;
        fixed_t back  = line->backsector->floorheight;
        angle_t angle;

        // The monster must contact one of the two floors,
        // and the other must be a tall dropoff (more than 24).
        if( back == dropoff_floorz
            && front < dropoff_floorz - FRACUNIT*24 )
        {
            // front side dropoff
            angle = R_PointToAngle2(0,0,line->dx,line->dy);
        }
        else if( front == dropoff_floorz
             && back < dropoff_floorz - FRACUNIT*24 )
        {
            // back side dropoff       
            angle = R_PointToAngle2(line->dx,line->dy,0,0);
        }
        else
            return true;  // not a dropoff

        // Move away from dropoff at a standard speed.
        // Multiple contacted linedefs are cumulative
        // (e.g. hanging over corner)
        dropoff_r_deltax -= finesine[angle >> ANGLETOFINESHIFT]*32;
        dropoff_r_deltay += finecosine[angle >> ANGLETOFINESHIFT]*32;
    }
    return true;
}

//
// Driver for above
//
static fixed_t P_AvoidDropoff(mobj_t *actor)
{
    int yh, yl, xh, xl;
    int bx, by;

    tm_bbox[BOXTOP] = actor->y + actor->radius;
    tm_bbox[BOXBOTTOM] = actor->y - actor->radius;
    tm_bbox[BOXRIGHT] = actor->x + actor->radius;
    tm_bbox[BOXLEFT] = actor->x - actor->radius;
    yh = (tm_bbox[BOXTOP] - bmaporgy) >> MAPBLOCKSHIFT;
    yl = (tm_bbox[BOXBOTTOM] - bmaporgy) >> MAPBLOCKSHIFT;
    xh = (tm_bbox[BOXRIGHT] - bmaporgx) >> MAPBLOCKSHIFT;
    xl = (tm_bbox[BOXLEFT] - bmaporgx) >> MAPBLOCKSHIFT;

    // PIT_AvoidDropoff parameters
    dropoff_floorz = actor->z;  // remember floor height
    dropoff_r_deltax = dropoff_r_deltay = 0;  // outputs

    // check lines

    validcount++;
    for( bx=xl ; bx<=xh ; bx++ )
    {
        for( by=yl ; by<=yh ; by++ )
            P_BlockLinesIterator(bx, by, PIT_AvoidDropoff);  // all contacted lines
    }

    // Non-zero if movement prescribed, caller uses dropoff vars.
    return dropoff_r_deltax | dropoff_r_deltay;
}


static
void P_NewChaseDir_P2 (mobj_t * actor, fixed_t deltax, fixed_t deltay )
{
    dirtype_t   d[3];

    int         tdir;
    dirtype_t   olddir = actor->movedir;
    dirtype_t   turnaround = opposite[olddir];

    trywalk_dropoffline = NULL;  // clear dropoff record

    if( deltax > 10*FRACUNIT )
        d[1]= DI_EAST;
    else if( deltax < -10*FRACUNIT )
        d[1]= DI_WEST;
    else
        d[1]= DI_NODIR;

    if( deltay < -10*FRACUNIT )
        d[2]= DI_SOUTH;
    else if( deltay > 10*FRACUNIT )
        d[2]= DI_NORTH;
    else
        d[2]= DI_NODIR;

    // try direct route
    if (   d[1] != DI_NODIR
        && d[2] != DI_NODIR)
    {
        actor->movedir = diags[((deltay<0)<<1)+(deltax>0)];
        if( actor->movedir != turnaround
            && P_TryWalk(actor) )
            goto accept_move;
    }

    // try other directions
    if( PP_Random(pr_newchase) > 200
        ||  abs(deltay) > abs(deltax))
    {
        tdir=d[1];
        d[1]=d[2];
        d[2]=tdir;
    }

    if( d[1] == turnaround)
        d[1] = DI_NODIR;
    if( d[2] == turnaround)
        d[2] = DI_NODIR;

    if( d[1] != DI_NODIR )
    {
        actor->movedir = d[1];
        if( P_TryWalk(actor) )
        {
            // either moved forward or attacked
            goto accept_move;
        }
    }

    if( d[2] != DI_NODIR )
    {
        actor->movedir = d[2];
        if( P_TryWalk(actor) )
            goto accept_move;
    }

    // there is no direct path to the player, so pick another direction.
    if( olddir != DI_NODIR )
    {
        actor->movedir = olddir;
        if( P_TryWalk(actor) )
            goto accept_move;
    }

    // randomly determine direction of search
    if( PP_Random(pr_newchasedir) & 0x01 )
    {
        for( tdir = DI_EAST; tdir <= DI_SOUTHEAST; tdir++ )
        {
            if( tdir != turnaround )
            {
                actor->movedir = tdir;
                if( P_TryWalk(actor) )
                    goto accept_move;
            }
        }
    }
    else
    {
        for( tdir = DI_SOUTHEAST; tdir >= DI_EAST; tdir-- )
        {
            if( tdir != turnaround )
            {
                actor->movedir = tdir;
                if( P_TryWalk(actor) )
                    goto accept_move;
            }
        }
    }

    if( turnaround != DI_NODIR )
    {
        actor->movedir = turnaround;
        if( P_TryWalk(actor) )
            goto accept_move;
    }

    // [WDJ] to not glide off ledges, unless conveyor
    if( EN_monster_momentum && trywalk_dropoffline )
    {
        // [WDJ] Momentum got actor stuck on edge,
        // but just reversing momentum is too much for conveyor.
        // Move perpendicular to dropoff line to get unstuck.
        fixed_t  dax, day;
        if( actor->subsector->sector == trywalk_dropoffline->frontsector )
        {
            // vector to frontsector
            dax = trywalk_dropoffline->dy;
            day = -trywalk_dropoffline->dx;
        }
        else if( actor->subsector->sector == trywalk_dropoffline->backsector )
        {
            // vector to backsector
            dax = -trywalk_dropoffline->dy;
            day = trywalk_dropoffline->dx;
        }
        else
        {
            actor->momx = actor->momy = 0;
            goto no_move;  // don't move across the dropoff
        }
        // Observe monsters on ledge, how long they stay stuck,
        // and on conveyor, how long they take to fall off.
        // Tuned backpedal speed constant.
        register int  backpedal = (actor->info->speed * 0x87BB);
//        debug_Printf( "backpedal 0x%X ", backpedal );
        // Vector away from line
        fixed_t  dal = P_AproxDistance(dax,day);
        dal = FixedDiv( dal, backpedal );
        if( dal > 1 )
        {
            dax = FixedDiv( dax, dal );  // shorten vector
            day = FixedDiv( day, dal );
        }

//        debug_Printf( "stuck delta (0x%X,0x%X)\n", dax, day );
        if (P_TryMove (actor, actor->x + dax, actor->y + day, true))  // allow cross dropoff
            goto accept_move;

    no_move:
        // must fall off conveyor end, but not off high ledges
        actor->momx = FixedMul(actor->momx, 0xA000);
        actor->momy = FixedMul(actor->momx, 0xA000);
    }

    actor->movedir = DI_NODIR;  // can not move
   
accept_move:
    return;
}


// [WDJ] MBF, from MBF, PrBoom, EternityEngine
// Most of the original P_NewChaseDir is now in P_NewChaseDir_P2.
static void P_NewChaseDir(mobj_t *actor)
{
    mobj_t *  target = actor->target;
    fixed_t deltax = target->x - actor->x;
    fixed_t deltay = target->y - actor->y;

    if( !target )
    {
        I_SoftError ("P_NewChaseDir: called with no target");
        return;
    }

    // killough 8/8/98: sometimes move away from target, keeping distance
    //
    // 1) Stay a certain distance away from a friend, to avoid being in their way
    // 2) Take advantage over an enemy without missiles, by keeping distance
    
    actor->strafecount = 0;  // MBF

    if( EN_mbf )
    {
        // CheckPosition has not been called, so tmr_dropoffz is not valid.
        // MBF has mobj keep dropoffz field.
        if( cv_mbf_dropoff.EV
            && actor->floorz - actor->dropoffz > FRACUNIT*24
            && actor->z <= actor->floorz
            && !(actor->flags & (MF_DROPOFF|MF_FLOAT))
            && P_AvoidDropoff(actor) // Move away from dropoff
          )
        {
            P_NewChaseDir_P2(actor, dropoff_r_deltax, dropoff_r_deltay);

            // If moving away from dropoff, set movecount to 1 so that
            // small steps are taken to get monster away from dropoff.

            actor->movecount = 1;
            return;
        }
        else
        {
            fixed_t dist = P_AproxDistance(deltax, deltay);

            // Move away from friends when too close, except
            // in certain situations (e.g. a crowded lift)

            if( BOTH_FRIEND(actor, target)
                && dist < EV_mbf_distfriend
                && !P_IsOnLift(target)
                && !P_IsUnderDamage(actor)
              )
            {
                deltax = -deltax;
                deltay = -deltay;
            }
            else if( target->health > 0
                     && ! SAME_FRIEND(actor, target) )
            {   // Live enemy target
                if( cv_mbf_monster_backing.EV
                    && actor->info->missilestate
                    && actor->type != MT_SKULL
                    && ( (!target->info->missilestate
                          && dist < MELEERANGE*2 )
                        || (target->player
                            && dist < MELEERANGE*3
                            && (target->player->readyweapon == wp_fist
                                || target->player->readyweapon == wp_chainsaw) ))
                  )
                {   // Back away from melee attacker
                    actor->strafecount = PP_Random(pr_enemystrafe) & 0x0F;
                    deltax = -deltax;
                    deltay = -deltay;
                }
            }
        }
    }

    P_NewChaseDir_P2(actor, deltax, deltay);

    // If strafing, set movecount to strafecount so that old Doom
    // logic still works the same, except in the strafing part

    if (actor->strafecount)
        actor->movecount = actor->strafecount;
}


// [WDJ] MBF, From MBF, PrBoom, EternityEngine
//
// P_IsVisible
//
// killough 9/9/98: whether a target is visible to a monster
//

static boolean P_IsVisible(mobj_t *actor, mobj_t *mo, boolean allaround)
{
    if( !allaround )
    {
        // Not visible if not within 90 degree of facing, and outside MELEE range.
        angle_t an = R_PointToAngle2(actor->x, actor->y, mo->x, mo->y)
                     - actor->angle;

        if( ( an > ANG90 && an < ANG270 )
            && ( P_AproxDistance((mo->x - actor->x), (mo->y - actor->y))
                 > MELEERANGE )
          )
            return false;
        // possibly visible, check sight
    }
    return P_CheckSight(actor, mo);
}


// [WDJ] MBF, From MBF, PrBoom, EternityEngine
//
// PIT_FindTarget
//
// killough 9/5/98
//
// Finds monster targets for other monsters
//

static mobj_t * ft_current_actor;
static int ft_current_allaround;

static boolean PIT_FindTarget(mobj_t *mo)
{
    mobj_t *actor = ft_current_actor;

    if( SAME_FRIEND(mo, actor)  // Invalid target
        || ! (mo->health > 0)
        || ! (mo->flags & MF_COUNTKILL || mo->type == MT_SKULL)
      )
        return true;  // reject, keep searching

    // If the monster is already engaged in a one-on-one attack
    // with a healthy friend, don't attack around 60% the time
    {
        const mobj_t * t2 = mo->target;  // target of this monster
        if( t2
            && t2->target == mo  // both targeting each other
            && PP_Random(pr_skiptarget) > 100  // stuck with test order
          )
        {
            // Already known that mo is not a friend.
            if( ! SAME_FRIEND(t2, mo)  // is attacking a friend
                && t2->health*2 >= t2->info->spawnhealth ) // friend health ok
                return true;  // find someone else
        }
    }

    if( !P_IsVisible(actor, mo, ft_current_allaround) )
        return true;

    P_SetReference(actor->lastenemy, actor->target);  // Remember previous target
    actor->lastenemy = actor->target;
    P_SetReference(actor->target, mo);                // Found target
    actor->target = mo;

    // Move the selected monster to the end of its class-list,
    // so that it gets searched last next time.
    P_MoveClassThink(&mo->thinker, 0);  // Last

    return false;  // Found target
}


//
// P_LookForPlayers
// If allaround is false, only look 180 degrees in front.
// Modifies actor.
// Returns true if a player is targeted.
//
static boolean P_LookForPlayers ( mobj_t*       actor,
                                  boolean       allaround )
{
    int  c;
    int  stop, stopc;
    int  lastlook = actor->lastlook;
    player_t*   player;

    if( EN_heretic
        && !multiplayer && players[0].health <= 0 )
    { // Single player game and player is dead, look for monsters
        return(PH_LookForMonsters(actor));  // Heretic version
    }

    // [WDJ] MBF, From MBF, PrBoom, EternityEngine.
    if( actor->flags & MF_FRIEND )
    {   // killough 9/9/98: friendly monsters go about players differently
        int anyone;

#if 0
        // If you want friendly monsters not to awaken unprovoked
        if( !allaround )
            return false;
#endif

        // Go back to a player, no matter whether it's visible or not
        // First time look for visible player, then look for anyone.
        for( anyone=0; anyone<=1; anyone++ )
        {
            for( c=0; c<MAXPLAYERS; c++ )
            {
                if( playeringame[c]
                    && players[c].playerstate == PST_LIVE
                    && (anyone
                        || P_IsVisible(actor, players[c].mo, allaround) )
                  )
                {
                    P_SetReference(actor->target, players[c].mo);
                    actor->target = players[c].mo;

                    // killough 12/98:
                    // get out of refiring loop, to avoid hitting player accidentally
                    if( actor->info->missilestate )
                    {
                        P_SetMobjState(actor, actor->info->seestate);
                        actor->flags &= ~MF_JUSTHIT;
                    }

                    return true;
                }
            }
        }
        return false;
    }

    // Monster looking for a player to attack!

    // Don't look for a player if ignoring
    if( actor->eflags & MF_IGNOREPLAYER )
        return false;

//        sector = actor->subsector->sector;

    // This is not in PrBoom, EternityEngine, and it uses P_Random !!!
    // BP: first time init, this allow minimum lastlook changes
    if( (lastlook < 0) && (EV_legacy >= 129) )
        lastlook = P_Random () % MAXPLAYERS;

    actor->lastlook = lastlook;
    stop = (lastlook-1)&PLAYERSMASK;
    stopc = ( !EN_mbf
              && EN_boom  // !demo_compatibility
              && cv_monster_remember.EV )? MAXPLAYERS : 2;  // killough 9/9/98

    for ( ; ; lastlook = (lastlook+1)&PLAYERSMASK )
    {
        actor->lastlook = lastlook;

        // [WDJ] The test for stop is ignored in PrBoom, MBF,
        // if that player is not present (broken test).
        // This test just gets to done_looking sooner,
        // which is not affected by playeringame.
        // But it affects SyncDebug.
        // done looking
        if( (EV_legacy >= 147 )
            && (lastlook == stop) )  goto done_looking;

        if( !playeringame[lastlook] )
            continue;

        if( --stopc < 0 )  goto done_looking;

        // Where PrBoom has this test, ignored when stop player is not present.
        if( lastlook == stop )  goto done_looking;  // broken test as in PrBoom

        player = &players[lastlook];
        if (player->health <= 0)
            continue;           // dead

        // MBF, PrBoom: IsVisible does same as previous code.
        if( !P_IsVisible( actor, player->mo, allaround) )
            continue;           // out of sight

        // Player found.

        if( EN_heretic && player->mo->flags&MF_SHADOW)
        { // Player is invisible
            if((P_AproxDistance(player->mo->x-actor->x,
                player->mo->y-actor->y) > 2*MELEERANGE)
                && P_AproxDistance(player->mo->momx, player->mo->momy)
                < 5*FRACUNIT)
            { // Player is sneaking - can't detect
                goto  none_found;
            }
            if(P_Random() < 225)
            { // Player isn't sneaking, but still didn't detect
                goto  none_found;
            }
        }

        // Remember old target node for later
        if (actor->target)
        {
            if(actor->target->type == MT_NODE)
               actor->targetnode = actor->target;
        }

        // New target found
        P_SetReference(actor->target, player->mo);
        actor->target = player->mo;

        // killough 9/9/98: give monsters a threshold towards getting players
        // we don't want it to be too easy for a player with dogs :)
        if( cv_mbf_pursuit.EV )  // !comp[comp_pursuit]
            actor->threshold = 60;

        return true;
    }
    goto  none_found;

done_looking:
    // [WDJ] MBF, From MBF, PrBoom, EternityEngine.
    // e6y
    // Fixed Boom incompatibilities. The following code was missed.
    // There are no more desyncs on Donce's demos on horror.wad (in PrBoom).

    if( !EN_mbf
        && EN_boom  // !demo_compatibility
        && cv_monster_remember.EV )
    {
        // Use last known enemy if no players sighted -- killough 2/15/98:
        if( actor->lastenemy && (actor->lastenemy->health > 0) )
        {
            actor->target = actor->lastenemy;
            actor->lastenemy = NULL;
            return true;
        }
    }

none_found:
    return false;
}


// [WDJ] MBF, From MBF, PrBoom, EternityEngine.
//
// Friendly monsters, by Lee Killough 7/18/98
//
// Friendly monsters go after other monsters first, but also return to owner
// if they cannot find any targets.
// A marine's best friend :)  killough 7/18/98, 9/98

static boolean P_LookForMonsters(mobj_t *actor, boolean allaround)
{
    int x, y, d;
    thinker_t *cap, *th;

    if( ! EN_boom )
        return false;

    // Boom: This test is at the end of P_LookForPlayers.
    if( cv_monster_remember.EV   // Boom smartypants = monsters_remember && EN_boom
        && actor->lastenemy
        && actor->lastenemy->health > 0
        && ! BOTH_FRIEND(actor->lastenemy, actor) // not friends
      )
    {
        // Boom
        // Use last known enemy if no players sighted.
        P_SetReference(actor->target, actor->lastenemy);
        actor->target = actor->lastenemy;
        P_SetReference(actor->lastenemy, NULL);
        actor->lastenemy = NULL;
        return true;
    }

    // Old demos do not support monster-seeking bots
    if( ! EN_mbf )
        return false;

    // Search the class-list corresponding to this actor's potential targets
    cap = &thinkerclasscap[actor->flags & MF_FRIEND ? TH_enemies : TH_friends];
    if( cap->cnext == cap )  // When empty list, bail out early
        return false;

    // Search for new enemy
    x = (actor->x - bmaporgx)>>MAPBLOCKSHIFT;
    y = (actor->y - bmaporgy)>>MAPBLOCKSHIFT;

    // Parameters PIT_FindTarget
    ft_current_actor = actor;
    ft_current_allaround = allaround;
   
    // Search first in the immediate vicinity.
    if( !P_BlockThingsIterator(x, y, PIT_FindTarget) )
        return true;

    for (d=1; d<5; d++)
    {
        int i = 1 - d;

        do{
            if( !P_BlockThingsIterator(x+i, y-d, PIT_FindTarget )
                || !P_BlockThingsIterator(x+i, y+d, PIT_FindTarget) )
                return true;
        }while (++i < d);

        do{
            if( !P_BlockThingsIterator(x-d, y+i, PIT_FindTarget )
                || !P_BlockThingsIterator(x+d, y+i, PIT_FindTarget) )
                return true;
        }while (--i + d >= 0);
    }

    {   // Random number of monsters, to prevent patterns from forming
        int n = (PP_Random(pr_friends) & 31) + 15;

        for( th = cap->cnext; th != cap; th = th->cnext )
        {
            if( --n < 0 )
            {
                // Only a subset of the monsters were searched. Move all of
                // the ones which were searched so far, to the end of the list.
                P_MoveClasslistRangeLast( cap, th );
                break;
            }

            if( !PIT_FindTarget((mobj_t *) th) )  // If target sighted
                return true;
        }
    }

    return false;  // No monster found
}


// [WDJ] MBF, From MBF, PrBoom, EternityEngine.
//
// P_LookForTargets
//
// killough 9/5/98: look for targets to go after, depending on kind of monster
//

static boolean P_LookForTargets(mobj_t *actor, int allaround)
{
    if( actor->flags & MF_FRIEND )
    {
        // MBF only
        return P_LookForMonsters(actor, allaround)  // Boom, MBF
               || P_LookForPlayers (actor, allaround);
    }

    return P_LookForPlayers (actor, allaround)
           || P_LookForMonsters(actor, allaround);  // Boom, MBF
}


// [WDJ] MBF, From MBF, PrBoom, EternityEngine.
//
// P_HelpFriend
//
// killough 9/8/98: Help friends in danger of dying
//

static boolean P_HelpFriend(mobj_t *actor)
{
    thinker_t *cap, *th;

    // If less than 33% health, self-preservation rules
    if( actor->health*3 < actor->info->spawnhealth )
        return false;

    // Parameters PIT_FindTarget
    ft_current_actor = actor;
    ft_current_allaround = true;

    // Possibly help a friend under 50% health
    cap = &thinkerclasscap[actor->flags & MF_FRIEND ? TH_friends : TH_enemies];

    for (th = cap->cnext; th != cap; th = th->cnext)
    {
        mobj_t * mo = (mobj_t *) th;  // thinker mobj 
        if( mo->health*2 >= mo->info->spawnhealth )
        {
            if( PP_Random(pr_helpfriend) < 180 )
                break;
        }
        else
        {
            if( mo->flags & MF_JUSTHIT
                && mo->target
                && mo->target != actor->target
                && !PIT_FindTarget( mo->target)
              )
            {
                // Ignore any attacking monsters, while searching for friend
                actor->threshold = BASETHRESHOLD;
                return true;
            }
        }
    }

    return false;
}


//
// ACTION ROUTINES
//

//
// A_Look
// Stay in state until a player is sighted.
//
void A_Look (mobj_t* actor)
{
    mobj_t*     targ;

    
    // Is there a node we must follow?
    if( actor->targetnode )
    {
        actor->target = actor->targetnode;
        P_SetMobjState(actor, actor->info->seestate);
        return;
    }

    actor->threshold = 0;       // any shot will wake up

    // [WDJ] From Prboom, MBF, EternityEngine
    // killough 7/18/98:
    // Friendly monsters go after other monsters first, but also return to
    // player, without attacking them, if they cannot find any targets.
    actor->pursuecount = 0;

    if( actor->flags & MF_FRIEND )
    {
        if( P_LookForTargets(actor, false) )  goto seeyou;
    }

    targ = actor->subsector->sector->soundtarget;
    if( targ && (targ->flags & MF_SHOOTABLE) )
    {
        P_SetReference( actor->target, targ );       
        actor->target = targ;

        if( !(actor->flags & MF_AMBUSH) )   goto seeyou;
        if( P_CheckSight(actor, targ) )     goto seeyou;
    }

#if 0   
    if( !EN_mbf )
    {
        if( P_LookForPlayers (actor, false) )   goto seeyou;
        return;
    }
#endif

    if( !(actor->flags & MF_FRIEND) )
    {
        // Look for Players, then monsters.
        if( P_LookForTargets(actor, false) )  goto seeyou;
    }
    return;

    // go into chase state
  seeyou:
    if (actor->info->seesound)
    {
        int             sound;

        switch (actor->info->seesound)
        {
          case sfx_posit1:
          case sfx_posit2:
          case sfx_posit3:
            sound = sfx_posit1+PP_Random(pr_see)%3;
            break;

          case sfx_bgsit1:
          case sfx_bgsit2:
            sound = sfx_bgsit1+PP_Random(pr_see)%2;
            break;

          default:
            sound = actor->info->seesound;
            break;
        }

        if (actor->type==MT_SPIDER
         || actor->type == MT_CYBORG
         || (actor->flags2&MF2_BOSS))
        {
            // full volume
            S_StartSound(sound);
        }
        else
            S_StartScreamSound(actor, sound);

    }

    P_SetMobjState (actor, actor->info->seestate);
}


// [WDJ] Functions from PrBoom, MBF, EternityEngine.

// killough 10/98:
// Allows monsters to continue movement while attacking
// EternityEngine makes it BEX available.
//
void A_KeepChasing(mobj_t *actor)
{
  if (actor->movecount)
  {
      actor->movecount--;
      if (actor->strafecount)
        actor->strafecount--;
      P_SmartMove(actor);
  }
}

//
// A_Chase
// Actor has a melee attack,
// so it tries to close as fast as possible
//
void A_Chase (mobj_t*   actor)
{
    int         delta;

    if (actor->reactiontime)
    {
        actor->reactiontime--;

        // We are pausing at a node, just look for players
        if ( actor->target && actor->target->type == MT_NODE)
        {
                        P_LookForPlayers(actor, false);
                        return;
        }
    }


    // modify target threshold
    if( actor->threshold )
    {
        if (EN_doom_etc 
            && (!actor->target
              || actor->target->health <= 0
//              || (actor->target->flags & MF_CORPSE)  // corpse health < 0
            ) )
        {
            actor->threshold = 0;
        }
        else
            actor->threshold--;
    }

    if( EN_heretic && cv_fastmonsters.EV)
    { // Monsters move faster in nightmare mode
        actor->tics -= actor->tics/2;
        if(actor->tics < 3)
            actor->tics = 3;
    }

    // Turn towards movement direction if not there yet.
    // killough 9/7/98: keep facing towards target if strafing or backing out
    if( actor->strafecount )  // MBF
    {
        A_FaceTarget(actor);
    }
    else if( actor->movedir < 8 )
    {
        actor->angle &= (7<<29);
        delta = actor->angle - (actor->movedir << 29);

        if (delta > 0)
            actor->angle -= ANG90/2;
        else if (delta < 0)
            actor->angle += ANG90/2;
    }

// [WDJ] compiler complains, "suggest parenthesis"
#if 0
   // Original code was
        if (!actor->target
        || !(actor->target->flags&MF_SHOOTABLE)
                && actor->target->type != MT_NODE
                && !(actor->eflags & MF_IGNOREPLAYER))
#else     
// but, based on other tests, the last two tests were added later.
// [WDJ] I think they meant:
    if ( !( actor->target &&
           ( actor->target->flags&MF_SHOOTABLE
             || actor->target->type == MT_NODE
           ))
         && !(actor->eflags & MF_IGNOREPLAYER)
       )
#endif       
    {
        // [WDJ] Recursion check from EternityEngine.
        // haleyjd 07/26/04: Detect and prevent infinite recursion if
        // Chase is called from a thing's spawnstate.
        static byte  chase_recursion = 0;
        static byte  chase_recursion_count = 0;  // [WDJ] too many messages

        // If recursion is true at this point, P_SetMobjState sent
        // us back here -- print an error message and return.
        if( chase_recursion )
        {
            if( verbose > 1 && (chase_recursion_count++ == 0) )
                GenPrintf( EMSG_warn, "Chase recursion detected\n");
            return;
        }

        // set the flag true before calling P_SetMobjState
        chase_recursion = 1;
#if 1
        // MBF
        // look for a new target
        if( ! P_LookForTargets(actor,true) )
#else       
        // look for a new target
        if( ! P_LookForPlayers(actor,true) )
#endif
        {
            // None found
            // This monster will start waiting again
            P_SetMobjState (actor, actor->info->spawnstate);
        }

        chase_recursion = 0; // Must clear this, in either case.
        return;
    }

    // do not attack twice in a row
    if (actor->flags & MF_JUSTATTACKED)
    {
        actor->flags &= ~MF_JUSTATTACKED;
        // MBF: if( gameskill != nightmare && !fastparm )
        if( !cv_fastmonsters.EV )
            P_NewChaseDir (actor);
        return;
    }

    // check for melee attack
    if (actor->info->meleestate
        && P_CheckMeleeRange (actor)
        && (actor->target && actor->target->type != MT_NODE)
        && !(actor->eflags & MF_IGNOREPLAYER))
    {
        if (actor->info->attacksound)
            S_StartAttackSound(actor, actor->info->attacksound);

        P_SetMobjState (actor, actor->info->meleestate);
       
        //MBF: killough 8/98: remember an attack
        // cph - DEMOSYNC?
        if( EN_mbf
            && !actor->info->missilestate )
            actor->flags |= MF_JUSTHIT;

        return;
    }

    // check for missile attack
    if (actor->info->missilestate
        && (actor->target && actor->target->type != MT_NODE)
        && !(actor->eflags & MF_IGNOREPLAYER))
    {
        if( !cv_fastmonsters.EV && actor->movecount )
        {
            goto nomissile;
        }

        if (!P_CheckMissileRange (actor))
            goto nomissile;

        P_SetMobjState (actor, actor->info->missilestate);
        actor->flags |= MF_JUSTATTACKED;
        return;
    }

    // ?
  nomissile:
    // possibly choose another target
#if 0
    // DoomLegacy 1.46.3
    if (multiplayer
        && !actor->threshold
        && !P_CheckSight (actor, actor->target)
        && (actor->target && actor->target->type != MT_NODE)
        && !(actor->eflags & MF_IGNOREPLAYER))
    {
        if (P_LookForPlayers(actor,true))
            return;     // got a new target

    }
#endif
    if( !actor->threshold )
    {
        // [WDJ] MBF, from MBF, PrBoom, EternityEngine.
        if( ! EN_mbf )
        {
            // Doom, Boom, Legacy
            if( multiplayer
                && !P_CheckSight (actor, actor->target)
                && (actor->target && actor->target->type != MT_NODE)
                && !(actor->eflags & MF_IGNOREPLAYER))
            {
                if( P_LookForPlayers(actor,true) )
                    return;     // got a new target
            }
        }
        // MBF
        // killough 7/18/98, 9/9/98: new monster AI
        else if( cv_mbf_help_friend.EV && P_HelpFriend(actor) )
            return;  // killough 9/8/98: Help friends in need.
        // Look for new targets if current one is bad or is out of view.
        else if( actor->pursuecount )
            actor->pursuecount--;
        else
        {
            // Our pursuit time has expired. We're going to think about
            // changing targets.
            actor->pursuecount = BASETHRESHOLD;

            // Unless (we have a live target, and it's not friendly
            // and we can see it)
            // try to find a new one; return if sucessful.
            if( !actor->target
                || ! (actor->target->health > 0)
                ||( (cv_mbf_pursuit.EV || multiplayer)
                    && ! ( ( ! SAME_FRIEND(actor->target, actor)
                             ||( !(actor->flags & MF_FRIEND)
                                 && monster_infight == INFT_infight
                               )
                            )
                            && P_CheckSight(actor, actor->target)
                         )
                  )
              )
            {
                if( P_LookForTargets(actor, true) )
                    return;
            }

            // Current target was good, or no new target was found.
            // If monster is a missile-less friend, give up pursuit and
            // return to player, if no attacks have occurred recently.

            if( (actor->flags & MF_FRIEND) && !actor->info->missilestate )
            {
                // if recent action, keep fighting, else return to player.
                if( actor->flags & MF_JUSTHIT)
                    actor->flags &= ~MF_JUSTHIT;
                else if( P_LookForPlayers(actor, true) )
                    return;
            }
        }
    }

    if( actor->strafecount )  // MBF
        actor->strafecount--;

    // Patrolling nodes
    if (actor->target && actor->target->type == MT_NODE)
    {
        // Check if a player is near
        if (P_LookForPlayers(actor, false))
        {
            // We found one, let him know we saw him!
            S_StartScreamSound(actor, actor->info->seesound);
            return;
        }

        // Did we touch a node as target?
        if (R_PointToDist2(actor->x, actor->y, actor->target->x, actor->target->y)
            <= actor->target->info->radius )
        {

            // Execute possible FS script
            if (actor->target->nodescript)
            {
                T_RunScript((actor->target->nodescript - 1), actor);
            }

            // Do we wait here?
            if (actor->target->nodewait)
                actor->reactiontime = actor->target->nodewait;

            // Set next node, if any
            if (actor->target->nextnode)
            {
                // Also remember it, if we will encounter an enemy			   
                actor->target = actor->target->nextnode;
                actor->targetnode = actor->target->nextnode;
            }
            else
            {
                actor->target = NULL;
                actor->targetnode = NULL;
            }

            return;
        }
    }


    // chase towards player
    if (--actor->movecount<0
        || !P_SmartMove(actor) )
    {
        P_NewChaseDir (actor);
    }


    // make active sound
    if (actor->info->activesound
        && PP_Random(pr_see) < 3)
    {
        if(actor->type == MT_WIZARD && P_Random() < 128)
            S_StartScreamSound(actor, actor->info->seesound);
        else if(actor->type == MT_SORCERER2)
            S_StartSound( actor->info->activesound );
        else
            S_StartScreamSound(actor, actor->info->activesound);
    }

}


//
// A_FaceTarget
//
void A_FaceTarget (mobj_t* actor)
{
    if (!actor->target)
        return;

    actor->flags &= ~MF_AMBUSH;

    actor->angle = R_PointToAngle2 (actor->x, actor->y,
                                    actor->target->x, actor->target->y );

    if (actor->target->flags & MF_SHADOW)
        actor->angle += PP_SignedRandom(pr_facetarget)<<21;
}


//
// A_PosAttack
//
void A_PosAttack (mobj_t* actor)
{
    int         angle;
    int         damage;
    int         slope;

    if (!actor->target)
        return;

    A_FaceTarget (actor);
    angle = actor->angle;
    slope = P_AimLineAttack (actor, angle, MISSILERANGE, 0);

    S_StartAttackSound(actor, sfx_pistol);
    angle += PP_SignedRandom(pr_posattack)<<20;
    damage = ((PP_Random(pr_posattack)%5)+1)*3;
    P_LineAttack (actor, angle, MISSILERANGE, slope, damage);
}

void A_SPosAttack (mobj_t* actor)
{
    int         i;
    int         angle;
    int         bangle;
    int         damage;
    int         slope;

    if (!actor->target)
        return;
    S_StartAttackSound(actor, sfx_shotgn);
    A_FaceTarget (actor);
    bangle = actor->angle;
    slope = P_AimLineAttack (actor, bangle, MISSILERANGE, 0);

    for (i=0 ; i<3 ; i++)
    {
        angle  = (PP_SignedRandom(pr_sposattack)<<20)+bangle;
        damage = ((PP_Random(pr_sposattack)%5)+1)*3;
        P_LineAttack (actor, angle, MISSILERANGE, slope, damage);
    }
}

void A_CPosAttack (mobj_t* actor)
{
    int         angle;
    int         bangle;
    int         damage;
    int         slope;

    if (!actor->target)
        return;
    S_StartAttackSound(actor, sfx_shotgn);
    A_FaceTarget (actor);
    bangle = actor->angle;
    slope = P_AimLineAttack (actor, bangle, MISSILERANGE, 0);

    angle  = (PP_SignedRandom(pr_cposattack)<<20)+bangle;

    damage = ((PP_Random(pr_cposattack)%5)+1)*3;
    P_LineAttack (actor, angle, MISSILERANGE, slope, damage);
}

void A_CPosRefire (mobj_t* actor)
{
    // keep firing unless target got out of sight
    A_FaceTarget (actor);

    if( PP_Random(pr_cposrefire) < 40 )
        return;

    if (!actor->target
        || actor->target->health <= 0
//        || actor->target->flags & MF_CORPSE  // corpse health < 0
        || !P_CheckSight (actor, actor->target) )
    {
        P_SetMobjState (actor, actor->info->seestate);
    }
}


void A_SpidRefire (mobj_t* actor)
{
    // keep firing unless target got out of sight
    A_FaceTarget (actor);

    if( PP_Random(pr_spidrefire) < 10 )
        return;

    if (!actor->target
        || actor->target->health <= 0
//        || actor->target->flags & MF_CORPSE  // corpse health < 0
        || !P_CheckSight (actor, actor->target) )
    {
        P_SetMobjState (actor, actor->info->seestate);
    }
}

void A_BspiAttack (mobj_t *actor)
{
    if (!actor->target)
        return;

    A_FaceTarget (actor);

    // launch a missile
    P_SpawnMissile (actor, actor->target, MT_ARACHPLAZ);
}


//
// A_TroopAttack
//
void A_TroopAttack (mobj_t* actor)
{
    int         damage;

    if (!actor->target)
        return;

    A_FaceTarget (actor);
    if (P_CheckMeleeRange (actor))
    {
        S_StartAttackSound(actor, sfx_claw);
        damage = (PP_Random(pr_troopattack)%8+1)*3;
        P_DamageMobj (actor->target, actor, actor, damage);
        return;
    }


    // launch a missile
    P_SpawnMissile (actor, actor->target, MT_TROOPSHOT);
}


void A_SargAttack (mobj_t* actor)
{
    int         damage;

    if (!actor->target)
        return;

    A_FaceTarget (actor);
    if (P_CheckMeleeRange (actor))
    {
        damage = ((PP_Random(pr_sargattack)%10)+1)*4;
        P_DamageMobj (actor->target, actor, actor, damage);
    }
}

void A_HeadAttack (mobj_t* actor)
{
    int         damage;

    if (!actor->target)
        return;

    A_FaceTarget (actor);
    if (P_CheckMeleeRange (actor))
    {
        damage = (PP_Random(pr_headattack)%6+1)*10;
        P_DamageMobj (actor->target, actor, actor, damage);
        return;
    }

    // launch a missile
    P_SpawnMissile (actor, actor->target, MT_HEADSHOT);
}

void A_CyberAttack (mobj_t* actor)
{
    if (!actor->target)
        return;

    A_FaceTarget (actor);
    P_SpawnMissile (actor, actor->target, MT_ROCKET);
}


void A_BruisAttack (mobj_t* actor)
{
    int         damage;

    if (!actor->target)
        return;

    if (P_CheckMeleeRange (actor))
    {
        S_StartAttackSound(actor, sfx_claw);
        damage = (PP_Random(pr_bruisattack)%8+1)*10;
        P_DamageMobj (actor->target, actor, actor, damage);
        return;
    }

    // launch a missile
    P_SpawnMissile (actor, actor->target, MT_BRUISERSHOT);
}


//
// A_SkelMissile
//
void A_SkelMissile (mobj_t* actor)
{
    mobj_t*     mo;

    if (!actor->target)
        return;

    A_FaceTarget (actor);
    actor->z += 16*FRACUNIT;    // so missile spawns higher
    mo = P_SpawnMissile (actor, actor->target, MT_TRACER);
    actor->z -= 16*FRACUNIT;    // back to normal

    if(mo)
    {
        mo->x += mo->momx;
        mo->y += mo->momy;
        P_SetReference(mo->tracer, actor->target);       
        mo->tracer = actor->target;
    }
}

angle_t  TRACEANGLE = 0x0c000000;

void A_Tracer (mobj_t* actor)
{
    angle_t     exact;
    fixed_t     dist;
    fixed_t     slope;
    mobj_t    * dest;
    mobj_t    * th;

    // killough 1/18/98: this is why some missiles do not have smoke and some do.
    // Also, internal demos start at random gametics, thus the bug in which
    // revenants cause internal demos to go out of sync.
    //
    // [WDJ] Use a proper tic, demo_comp_tic, that only runs while demo plays,
    // instead of the contrived basetic correction.
    //
    // It would have been better to use leveltime to start with in Doom,
    // but since old demos were recorded using gametic, we must stick with it,
    // and improvise around it (using leveltime causes desync across levels).

    // [WDJ] As NEWTICRATERATIO = 1, this is same as gametic & 3, 
    // PrBoom, MBF: use (gametic - basetic), which is same as demo_comp_tick.
    if( ((EV_legacy && (EV_legacy < 147))? gametic : game_comp_tic) % (4 * NEWTICRATERATIO) )
        return;
   
    // spawn a puff of smoke behind the rocket
    P_SpawnPuff (actor->x, actor->y, actor->z);

    th = P_SpawnMobj ((actor->x - actor->momx), (actor->y - actor->momy),
                      actor->z, MT_SMOKE);

    th->momz = FRACUNIT;
    th->tics -= PP_Random(pr_tracer)&3;
    if (th->tics < 1)
        th->tics = 1;

    // adjust direction
    dest = actor->tracer;

    if (!dest || dest->health <= 0)
        return;

    // change angle
    exact = R_PointToAngle2 (actor->x, actor->y,
                             dest->x, dest->y);

    if (exact != actor->angle)
    {
        if (exact - actor->angle > 0x80000000) // (actor->angle > exact)
        {
            actor->angle -= TRACEANGLE;  // correct towards exact
            if (exact - actor->angle < 0x80000000)  // (actor->angle < exact)
                actor->angle = exact;
        }
        else
        {
            actor->angle += TRACEANGLE;  // correct towards exact
            if (exact - actor->angle > 0x80000000)  // (actor->angle > exact)
                actor->angle = exact;
        }
    }

    int angf = ANGLE_TO_FINE(actor->angle);
    actor->momx = FixedMul (actor->info->speed, finecosine[angf]);
    actor->momy = FixedMul (actor->info->speed, finesine[angf]);

    // change slope
    dist = P_AproxDistance (dest->x - actor->x,
                            dest->y - actor->y);

    dist = dist / actor->info->speed;

    if (dist < 1)
        dist = 1;
    slope = (dest->z+40*FRACUNIT - actor->z) / dist;

    if (slope < actor->momz)
        actor->momz -= FRACUNIT/8;
    else
        actor->momz += FRACUNIT/8;
}


void A_SkelWhoosh (mobj_t*      actor)
{
    if (!actor->target)
        return;
    A_FaceTarget (actor);
    // judgecutor:
    // CHECK ME!
    S_StartAttackSound(actor, sfx_skeswg);
}

void A_SkelFist (mobj_t*        actor)
{
    int         damage;

    if (!actor->target)
        return;

    A_FaceTarget (actor);

    if (P_CheckMeleeRange (actor))
    {
        damage = ((PP_Random(pr_skelfist)%10)+1)*6;
        S_StartAttackSound(actor, sfx_skepch);
        P_DamageMobj (actor->target, actor, actor, damage);
    }
}



//
// PIT_VileCheck
// Detect a corpse that could be raised.
//
// PIT_VileCheck Global parameter
static mobj_t * vile_r_corpse;  // OUT
static mobj_t * vileobj;
static xyz_t    vile_pos;

boolean PIT_VileCheck (mobj_t*  thing)
{
    int         maxdist;
    boolean     check;

    if (!(thing->flags & MF_CORPSE) )
        return true;    // not a monster

    if (thing->tics != -1)
        return true;    // not lying still yet

    if (thing->info->raisestate == S_NULL)
        return true;    // monster doesn't have a raise state

    maxdist = thing->info->radius + mobjinfo[MT_VILE].radius;

    if ( abs(thing->x - vile_pos.x) > maxdist
         || abs(thing->y - vile_pos.y) > maxdist )
        return true;            // not actually touching

    // [WDJ] Prevent raise of corpse on another 3d floor.
    // Because corpse may have 0 height, use only vile reach.
    if( thing->z > (vile_pos.z + mobjinfo[MT_VILE].height)
        || thing->z < (vile_pos.z - 20*FRACUNIT) )  // reasonable reach down
        return true;

    vile_r_corpse = thing;
    vile_r_corpse->momx = vile_r_corpse->momy = 0;

    if( EN_vile_revive_bug )
    {
        // [WDJ] The original code.  Corpse heights are not this simple.
        // Would touch another monster and get stuck.
        // In PrBoom this is enabled by comp[comp_vile].
        vile_r_corpse->height <<= 2;
        check = P_CheckPosition (vile_r_corpse, vile_r_corpse->x, vile_r_corpse->y);
        vile_r_corpse->height >>= 2;
    }
    else
    {
        // [WDJ] Test with revived sizes from info, to fix monsters stuck together bug.
        // Must test as it would be revived, and then restore after the check
        // (because a collision could be found).
        // From considering the same fix in zdoom and prboom.
        fixed_t corpse_height = vile_r_corpse->height;
        vile_r_corpse->height = vile_r_corpse->info->height; // revived height
        fixed_t corpse_radius = vile_r_corpse->radius;
        vile_r_corpse->radius = vile_r_corpse->info->radius; // revived radius
        int corpse_flags = vile_r_corpse->flags;
        vile_r_corpse->flags |= MF_SOLID; // revived would be SOLID
        check = P_CheckPosition (vile_r_corpse, vile_r_corpse->x, vile_r_corpse->y);
        vile_r_corpse->height = corpse_height;
        vile_r_corpse->radius = corpse_radius;
        vile_r_corpse->flags = corpse_flags;
    }

    return !check;	// stop searching when no collisions found
}



//
// A_VileChase
// Check for ressurecting a body
//
void A_VileChase (mobj_t* actor)
{
    int    xl, xh;
    int    yl, yh;

    int    bx, by;

    mobjinfo_t * info;
    mobj_t     * temp;

    if (actor->movedir != DI_NODIR)
    {
        // check for corpses to raise
        // Parameters to VileCheck.
        vile_pos.x =
            actor->x + actor->info->speed * xspeed[actor->movedir];
        vile_pos.y =
            actor->y + actor->info->speed * yspeed[actor->movedir];
        vile_pos.z = actor->z;

        xl = (vile_pos.x - bmaporgx - MAXRADIUS*2)>>MAPBLOCKSHIFT;
        xh = (vile_pos.x - bmaporgx + MAXRADIUS*2)>>MAPBLOCKSHIFT;
        yl = (vile_pos.y - bmaporgy - MAXRADIUS*2)>>MAPBLOCKSHIFT;
        yh = (vile_pos.y - bmaporgy + MAXRADIUS*2)>>MAPBLOCKSHIFT;

        vileobj = actor;
        for (bx=xl ; bx<=xh ; bx++)
        {
            for (by=yl ; by<=yh ; by++)
            {
                // Call PIT_VileCheck to check whether object is a corpse
                // that can be raised.
                if (!P_BlockThingsIterator(bx,by,PIT_VileCheck))
                    goto raise_corpse;
            }
        }
    }

    // Return to normal attack.
    A_Chase (actor);
    return;

raise_corpse:
    // got one!
    temp = actor->target;
    actor->target = vile_r_corpse;  // do not change reference
    A_FaceTarget (actor);
    actor->target = temp;  // reference is still correct

    P_SetMobjState (actor, S_VILE_HEAL1);
    S_StartObjSound( vile_r_corpse, sfx_slop );
    info = vile_r_corpse->info;

    P_SetMobjState (vile_r_corpse,info->raisestate);
    if( demoversion<129 )
    {
        // original code, with ghost bug
        // does not work when monster has been crushed
        vile_r_corpse->height <<= 2;
    }
    else
    {
        // fix vile revives crushed monster as ghost bug
        vile_r_corpse->height = info->height;
        vile_r_corpse->radius = info->radius;
    }
    vile_r_corpse->flags = info->flags & ~MF_FRIEND;
    vile_r_corpse->health = info->spawnhealth;
    P_SetReference(vile_r_corpse->target, NULL);
    vile_r_corpse->target = NULL;

    if( EN_mbf )
    {
        // killough 7/18/98:
        // friendliness is transferred from vile to raised corpse
        vile_r_corpse->flags =
            (info->flags & ~(MF_FRIEND|MF_JUSTHIT)) | (actor->flags & MF_FRIEND);

#if 0
        if( vile_r_corpse->flags & (MF_FRIEND | MF_COUNTKILL) == MF_FRIEND)
            totallive++;
#endif

        P_SetReference(vile_r_corpse->lastenemy, NULL);
        vile_r_corpse->lastenemy = NULL;

        // killough 8/29/98: add to appropriate thread.
        P_UpdateClassThink( &vile_r_corpse->thinker, TH_unknown );
    }
    return;
}


//
// A_VileStart
//
void A_VileStart (mobj_t* actor)
{
    S_StartAttackSound(actor, sfx_vilatk);
}


//
// A_Fire
// Keep fire in front of player unless out of sight
//
void A_Fire (mobj_t* actor);

void A_StartFire (mobj_t* actor)
{
    S_StartObjSound(actor, sfx_flamst);
    A_Fire(actor);
}

void A_FireCrackle (mobj_t* actor)
{
    S_StartObjSound(actor, sfx_flame);
    A_Fire(actor);
}

void A_Fire (mobj_t* actor)
{
    mobj_t*     dest;

    dest = actor->tracer;
    if (!dest)
        return;

    // don't move it if the vile lost sight
    if (!P_CheckSight (actor->target, dest) )
        return;

    int angf = ANGLE_TO_FINE(dest->angle);

    P_UnsetThingPosition (actor);
    actor->x = dest->x + FixedMul (24*FRACUNIT, finecosine[angf]);
    actor->y = dest->y + FixedMul (24*FRACUNIT, finesine[angf]);
    actor->z = dest->z;
    P_SetThingPosition (actor);
}



//
// A_VileTarget
// Spawn the hellfire
//
void A_VileTarget (mobj_t*      actor)
{
    mobj_t*     fog;

    if (!actor->target)
        return;

    A_FaceTarget (actor);

#if 0
    // Original bug
    fog = P_SpawnMobj (actor->target->x,
                       actor->target->x,           // Bp: shoul'nt be y ?
                       actor->target->z, MT_FIRE);
#endif

    // [WDJ] Found by Bp: Fix Vile fog bug, similar to prboom (Killough 12/98).
    fog = P_SpawnMobj (actor->target->x,
                       actor->target->y,
                       actor->target->z, MT_FIRE);

    P_SetReference(actor->tracer, fog);
    actor->tracer = fog;
    P_SetReference(fog->target, actor);
    fog->target = actor;
    P_SetReference(fog->tracer, actor->target);
    fog->tracer = actor->target;

    A_Fire (fog);
}




//
// A_VileAttack
//
void A_VileAttack (mobj_t* actor)
{
    mobj_t*     fire;

    if (!actor->target)
        return;

    A_FaceTarget (actor);

    if (!P_CheckSight (actor, actor->target) )
        return;

    S_StartObjSound(actor, sfx_barexp);
    P_DamageMobj (actor->target, actor, actor, 20);
    actor->target->momz = 1000*FRACUNIT/actor->target->info->mass;

    int angf = ANGLE_TO_FINE(actor->angle);

    fire = actor->tracer;
    if (!fire)
        return;

    // move the fire between the vile and the player
    fire->x = actor->target->x - FixedMul (24*FRACUNIT, finecosine[angf]);
    fire->y = actor->target->y - FixedMul (24*FRACUNIT, finesine[angf]);
    P_RadiusAttack (fire, actor, 70 );
}




//
// Mancubus attack,
// firing three missiles (bruisers)
// in three different directions?
// Doesn't look like it.
//
#define FATSPREAD       (ANG90/8)

void A_FatRaise (mobj_t *actor)
{
    A_FaceTarget (actor);
    S_StartAttackSound(actor, sfx_manatk);
}


void A_FatAttack1 (mobj_t* actor)
{
    mobj_t*     mo;

    A_FaceTarget (actor);
    // Change direction  to ...
    actor->angle += FATSPREAD;
    P_SpawnMissile (actor, actor->target, MT_FATSHOT);

    mo = P_SpawnMissile (actor, actor->target, MT_FATSHOT);
    if(mo)
    {
        mo->angle += FATSPREAD;
        int angf = ANGLE_TO_FINE(mo->angle);
        mo->momx = FixedMul (mo->info->speed, finecosine[angf]);
        mo->momy = FixedMul (mo->info->speed, finesine[angf]);
    }
}

void A_FatAttack2 (mobj_t* actor)
{
    mobj_t*     mo;

    A_FaceTarget (actor);
    // Now here choose opposite deviation.
    actor->angle -= FATSPREAD;
    P_SpawnMissile (actor, actor->target, MT_FATSHOT);

    mo = P_SpawnMissile (actor, actor->target, MT_FATSHOT);
    if(mo)
    {
        mo->angle -= FATSPREAD*2;
        int angf = ANGLE_TO_FINE(mo->angle);
        mo->momx = FixedMul (mo->info->speed, finecosine[angf]);
        mo->momy = FixedMul (mo->info->speed, finesine[angf]);
    }
}

void A_FatAttack3 (mobj_t* actor)
{
    mobj_t*     mo;
    int         angf;

    A_FaceTarget (actor);

    mo = P_SpawnMissile (actor, actor->target, MT_FATSHOT);
    if(mo)
    {
        mo->angle -= FATSPREAD/2;
        angf = ANGLE_TO_FINE(mo->angle);
        mo->momx = FixedMul (mo->info->speed, finecosine[angf]);
        mo->momy = FixedMul (mo->info->speed, finesine[angf]);
    }
    
    mo = P_SpawnMissile (actor, actor->target, MT_FATSHOT);
    if(mo)
    {
        mo->angle += FATSPREAD/2;
        angf = ANGLE_TO_FINE(mo->angle);
        mo->momx = FixedMul (mo->info->speed, finecosine[angf]);
        mo->momy = FixedMul (mo->info->speed, finesine[angf]);
    }
}


//
// SkullAttack
// Fly at the player like a missile.
//
#define SKULLSPEED              (20*FRACUNIT)

void A_SkullAttack (mobj_t* actor)
{
    mobj_t*             dest;
    angle_t             ang;
    int                 dist;

    if (!actor->target)
        return;
   
    if (actor->target->health <= 0)
    {
       actor->target = NULL;
       return;
    }

    dest = actor->target;
    actor->flags |= MF_SKULLFLY;
    S_StartScreamSound(actor, actor->info->attacksound);
    A_FaceTarget (actor);

    if( cv_predictingmonsters.EV || (actor->eflags & MF_PREDICT) ) //added by AC for predmonsters
    {

                boolean canHit;
                fixed_t	px, py, pz;
                int	t, time;
                subsector_t *sec;

                dist = P_AproxDistance (dest->x - actor->x, dest->y - actor->y);
                time = dist/SKULLSPEED;
                time = P_AproxDistance (dest->x + dest->momx*time - actor->x,
                                                                dest->y + dest->momy*time - actor->y)/SKULLSPEED;

                canHit = 0;
                t = time + 4;
                do
                {
                        t-=4;
                        if (t < 1)
                                t = 1;
                        px = dest->x + dest->momx*t;
                        py = dest->y + dest->momy*t;
                        pz = dest->z + dest->momz*t;
                        canHit = P_CheckSight2(actor, dest, px, py, pz);
                } while (!canHit && (t > 1));

                sec = R_PointInSubsector(px, py);
                if (!sec)
                        sec = dest->subsector;

                if (pz < sec->sector->floorheight)
                        pz = sec->sector->floorheight;
                else if (pz > sec->sector->ceilingheight)
                        pz = sec->sector->ceilingheight - dest->height;

                ang = R_PointToAngle2 (actor->x, actor->y, px, py);

                // fuzzy player
                if (dest->flags & MF_SHADOW)
                {
                        if( EN_heretic )
                            ang += P_SignedRandom()<<21;
                        else
                            ang += P_SignedRandom()<<20;
                }

                actor->angle = ang;
                actor->momx = FixedMul (SKULLSPEED, cosine_ANG(ang));
                actor->momy = FixedMul (SKULLSPEED, sine_ANG(ang));

                actor->momz = (pz+(dest->height>>1) - actor->z) / t;
    }
    else
    {
                ang = actor->angle;
                actor->momx = FixedMul (SKULLSPEED, cosine_ANG(ang));
                actor->momy = FixedMul (SKULLSPEED, sine_ANG(ang));
                dist = P_AproxDistance (dest->x - actor->x, dest->y - actor->y);
                dist = dist / SKULLSPEED;

                if (dist < 1)
                        dist = 1;
                actor->momz = (dest->z+(dest->height>>1) - actor->z) / dist;
    }
}


//
// A_PainShootSkull
// Spawn a lost soul and launch it at the target
//
void
A_PainShootSkull( mobj_t* actor, angle_t angle )
{
    fixed_t     x, y, z;
    mobj_t*     newmobj;
    int         prestep;


    if( EN_skull_limit )  // Boom comp_pain
    {
    //  --------------- SKULL LIMIT CODE -----------------
//  Original Doom code that limits the number of skulls to 20
    int         count;
    thinker_t*  currentthinker;

    // count total number of skull currently on the level
    count = 0;

    currentthinker = thinkercap.next;
    while (currentthinker != &thinkercap)
    {
        if (   (currentthinker->function.acp1 == (actionf_p1)P_MobjThinker)
            && ((mobj_t *)currentthinker)->type == MT_SKULL)
            count++;
        currentthinker = currentthinker->next;
    }

    // if there are already 20 skulls on the level,
    // don't spit another one
    if (count > 20)
        goto no_skull;

    }

    // okay, there's place for another one
    prestep =
        4*FRACUNIT
        + 3*(actor->info->radius + mobjinfo[MT_SKULL].radius)/2;

    x = actor->x + FixedMul (prestep, cosine_ANG(angle));
    y = actor->y + FixedMul (prestep, sine_ANG(angle));
    z = actor->z + 8*FRACUNIT;

    if( EN_old_pain_spawn )  // Boom comp_skull
    {
       newmobj = P_SpawnMobj (x, y, z, MT_SKULL);
    }
    else
    {
       // Check before spawning if spawn spot is valid, not in a wall,
       // not crossing any lines that monsters could not cross.
       if( P_CheckCrossLine( actor, x, y ) )
           goto no_skull;
       
       newmobj = P_SpawnMobj (x, y, z, MT_SKULL);
       
       // [WDJ] Could not think of better way to check this.
       // So modified from prboom (by phares).
       {
           register sector_t * nmsec = newmobj->subsector->sector;
           // check for above ceiling or below floor
           // skull z may be modified by SpawnMobj, so check newmobj itself
           if( ( (newmobj->z + newmobj->height) > nmsec->ceilingheight )
               || ( newmobj->z < nmsec->floorheight ) )
               goto remove_skull;
       }
    }

    // [WDJ] From PrBoom, MBF, EternityEngine.
    // killough 7/20/98: PEs shoot lost souls with the same friendliness
    newmobj->flags = (newmobj->flags & ~MF_FRIEND) | (actor->flags & MF_FRIEND);
    P_UpdateClassThink(&newmobj->thinker, TH_unknown);

    // Check for movements.
    if (!P_TryMove (newmobj, newmobj->x, newmobj->y, false))
       goto remove_skull;

    if( actor->target && (actor->target->health > 0) )
    {
        P_SetReference( newmobj->target, actor->target );
        newmobj->target = actor->target;
    }
    
    A_SkullAttack (newmobj);
    return;

remove_skull:   
    // kill it immediately
#define RMSKULL  2
#if RMSKULL == 0
    // The skull dives to the floor and dies
    P_DamageMobj (newmobj,actor,actor,10000);
#endif
#if RMSKULL == 1      
    // The skull dies less showy, like prboom
    newmobj->health = 0;
    P_KillMobj (newmobj,NULL,actor);  // no death messages
#endif
#if RMSKULL == 2
    // The skull does not appear, like Edge
    P_RemoveMobj (newmobj);
#endif
no_skull:
    return;
}


//
// A_PainAttack
// Spawn a lost soul and launch it at the target
//
void A_PainAttack (mobj_t* actor)
{
    if (!actor->target)
        return;
   
    if (actor->target->health <= 0 )
    {
       P_SetReference(actor->target, NULL);
       actor->target = NULL;
       return;
    }

    A_FaceTarget (actor);
    A_PainShootSkull (actor, actor->angle);
}


void A_PainDie (mobj_t* actor)
{
    A_Fall (actor);
    A_PainShootSkull (actor, actor->angle+ANG90);
    A_PainShootSkull (actor, actor->angle+ANG180);
    A_PainShootSkull (actor, actor->angle+ANG270);
}






void A_Scream (mobj_t* actor)
{
    int         sound;

    switch (actor->info->deathsound)
    {
      case 0:
        return;

      case sfx_podth1:
      case sfx_podth2:
      case sfx_podth3:
        sound = sfx_podth1 + PP_Random(pr_scream)%3;
        break;

      case sfx_bgdth1:
      case sfx_bgdth2:
        sound = sfx_bgdth1 + PP_Random(pr_scream)%2;
        break;

      default:
        sound = actor->info->deathsound;
        break;
    }

    // Check for bosses.
    if (actor->type==MT_SPIDER
        || actor->type == MT_CYBORG)
    {
        // full volume
        S_StartSound(sound);
    }
    else
        S_StartScreamSound(actor, sound);
}


void A_XScream (mobj_t* actor)
{
    S_StartScreamSound(actor, sfx_slop);
}

void A_Pain (mobj_t* actor)
{
    if (actor->info->painsound)
        S_StartScreamSound(actor, actor->info->painsound);
}


//
//  A dying thing falls to the ground (monster deaths)
//
// Invoked by state during death sequence, after P_KillMobj.
void A_Fall (mobj_t *actor)
{
    // actor is on ground, it can be walked over
    if (!cv_solidcorpse.EV)
        actor->flags &= ~MF_SOLID;  // not solid (vanilla doom)

    if( EV_legacy >= 131 )
    {
        // Before version 131 this is done later in P_KillMobj.
        actor->flags   |= MF_CORPSE|MF_DROPOFF;
        actor->height = actor->info->height>>2;
        actor->radius -= (actor->radius>>4);      //for solid corpses
        // [WDJ] Corpse health must be < 0.
        // Too many health checks all over the place, like BossDeath.
        if( actor->health >= 0 )
            actor->health = -1;
        if( actor->health > -actor->info->spawnhealth )
        {
            // Not gibbed yet.
            // Determine health until gibbed, keep some of the damage.
            actor->health = (actor->health - actor->info->spawnhealth)/2;
        }
    }

    // So change this if corpse objects
    // are meant to be obstacles.
}


//
// A_Explode
//
void A_Explode (mobj_t* actor)
{
    int damage = 128;

    switch(actor->type)
    {
    case MT_FIREBOMB: // Time Bombs
        actor->z += 32*FRACUNIT;
        actor->flags &= ~MF_SHADOW;
        break;
    case MT_MNTRFX2: // Minotaur floor fire
        damage = 24;
        break;
    case MT_SOR2FX1: // D'Sparil missile
        damage = 80+(P_Random()&31);
        break;
    default:
        break;
    }

    P_RadiusAttack ( actor, actor->target, damage );
    P_HitFloor(actor);
}

static state_t *P_FinalState(statenum_t state)
{
    static char final_state[NUMSTATES]; //Hurdler: quick temporary hack to fix hacx freeze

    memset(final_state, 0, NUMSTATES);
    while (states[state].tics!=-1)
    {
        final_state[state] = 1;
        state=states[state].nextstate;
        if (final_state[state])
            return NULL;
    }

    return &states[state];
}

//
// A_BossDeath
// Possibly trigger special effects
// if on first boss level
//
// Triggered by actor state change action, last in death sequence.
// A_Fall usually occurs before this.
// Heretic: see A_HBossDeath() in p_henemy.c
// [WDJ]  Keen death does not have tests for mo->type and thus allows
// Dehacked monsters to trigger Keen death and BossDeath effects.
// Should duplicate that ability in Doom maps.
void A_Bosstype_Death (mobj_t* mo, int boss_type)
{
    thinker_t*  th;
    mobj_t*     mo2;
    line_t      lineop;  // operation performed when all bosses are dead.
    int         i;
   
    if ( gamemode == doom2_commercial)
    {
        // Doom2 MAP07: When last Mancubus is dead,
        //   execute lowerFloortoLowest(sectors tagged 666).
        // Doom2 MAP07: When last Arachnotron is dead,
        //   execute raisetoTexture(sectors tagged 667).
        // Doom2 MAP32: When last Keen is dead,
        //   execute doorOpen(doors tagged 666).
          if((boss_type != MT_FATSO)
            && (boss_type != MT_BABY)
            && (boss_type != MT_KEEN))
                goto no_action;
    }
    // [WDJ] Untested
    // This could be done with compatibility switch (comp_666), as in prboom.
    else if( (gamemode == doom_shareware || gamemode == doom_registered)
             && gameepisode < 4 )
    {
        // [WDJ] Revert to behavior before UltimateDoom,
        // to fix "Doomsday of UAC" bug.
        if (gamemap != 8)
            goto no_action;
        // Allow all boss types in each episode, (for PWAD)
        // E1: all, such as Baron and Cyberdemon
        // E2,E3,E4: all except Baron
        // [WDJ] Logic from prboom
        if (gameepisode != 1)
            if (boss_type == MT_BRUISER)
                goto no_action;
    }
    else
    {
        switch(gameepisode)
        {
          case 1:
            // Doom E1M8: When all Baron are dead,
            //   execute lowerFloortoLowest(sectors tagged 666).
            if (gamemap != 8)
                goto no_action;

            // This test was added in UltimateDoom,
            // some PWAD from before then, such as "Doomsday of UAC" which
            // requires death of last Baron and last Cyberdemon, will fail.
            if (boss_type != MT_BRUISER)
                goto no_action;
            break;

          case 2:
            // Doom E2M8: When last Cyberdemon is dead, level ends.
            if (gamemap != 8)
                goto no_action;

            if (boss_type != MT_CYBORG)
                goto no_action;
            break;

          case 3:
            // Doom E3M8: When last Spidermastermind is dead, level ends.
            if (gamemap != 8)
                goto no_action;

            if (boss_type != MT_SPIDER)
                goto no_action;

            break;

          case 4:
            switch(gamemap)
            {
              case 6:
                // Doom E4M6: When last Cyberdemon is dead,
                //   execute blazeOpen(doors tagged 666).
                if (boss_type != MT_CYBORG)
                    goto no_action;
                break;

              case 8:
                // Doom E4M8: When last Spidermastermind is dead,
                //   execute lowerFloortoLowest(sectors tagged 666).
                if (boss_type != MT_SPIDER)
                    goto no_action;
                break;

              default:
                goto no_action;
            }
            break;

          default:
            if (gamemap != 8)
                goto no_action;
            break;
        }

    }


    // make sure there is a player alive for victory
    for (i=0 ; i<MAXPLAYERS ; i++)
        if (playeringame[i] && players[i].health > 0)
            break;

    if (i==MAXPLAYERS)
        return; // no one left alive, so do not end game

    // scan the remaining thinkers to see
    // if all bosses are dead
    for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    {
        if (th->function.acp1 != (actionf_p1)P_MobjThinker)
            continue;

        // Fixes MAP07 bug where if last arachnotron is killed while first
        // still in death sequence, then both would trigger this code
        // and the floor would be raised twice (bad).
        mo2 = (mobj_t *)th;
        // Check all boss of the same type
        if ( mo2 != mo
            && mo2->type == boss_type )
        {
            // Check if really dead and finished the death sequence.
            // [WDJ] Corpse has health < 0.
            // If two monsters are killed at the same time, this test may occur
            // while first is corpse and second is not.  But the simple health
            // test may trigger twice because second monster already has
            // health < 0 during the first death test.
            if( mo2->health > 0  // the old test (doom original 1.9)
                || !(mo2->flags & MF_CORPSE)
                || mo2->state != P_FinalState(mo2->info->deathstate) )
            {
                // other boss not dead
                goto no_action;
            }
        }
    }

    // victory!
    if ( gamemode == doom2_commercial)
    {
        if (boss_type == MT_FATSO)
        {
            if(gamemap == 7)
            {
                // Doom2 MAP07: When last Mancubus is dead, execute lowerFloortoLowest.
                //   execute lowerFloortoLowest(sectors tagged 666).
                lineop.tag = 666;
                EV_DoFloor( &lineop, FT_lowerFloorToLowest);
            }
            goto done;
        }
        if (boss_type == MT_BABY)
        {
            if(gamemap == 7)
            {
                // Doom2 MAP07: When last Arachnotron is dead,
                //   execute raisetoTexture(sectors tagged 667).
                lineop.tag = 667;
                EV_DoFloor( &lineop, FT_raiseToTexture);
            }
            goto done;
        }
        else if(boss_type == MT_KEEN)
        {
            // Doom2 MAP32: When last Keen is dead,
            //   execute doorOpen(doors tagged 666).
            lineop.tag = 666;
            EV_DoDoor( &lineop, VD_dooropen, VDOORSPEED);
            goto done;
        }
    }
    else
    {
        switch(gameepisode)
        {
          case 1:
            // Doom E1M8: When all Baron are dead, execute lowerFloortoLowest
            //   on all sectors tagged 666.
            lineop.tag = 666;
            EV_DoFloor( &lineop, FT_lowerFloorToLowest);
            goto done;

          case 4:
            switch(gamemap)
            {
              case 6:
                // Doom E4M6: When last Cyberdemon is dead, execute blazeOpen.
                //   on all doors tagged 666.
                lineop.tag = 666;
                EV_DoDoor( &lineop, VD_blazeOpen, 4*VDOORSPEED);
                goto done;

              case 8:
                // Doom E4M8: When last Spidermastermind is dead, execute lowerFloortoLowest.
                //   on all sectors tagged 666.
                lineop.tag = 666;
                EV_DoFloor( &lineop, FT_lowerFloorToLowest);
                goto done;
            }
        }
    }
    if( cv_allowexitlevel.EV )
        G_ExitLevel ();
done:
    return;

no_action:
    return;
}

// Info table call, as in Heretic.
void A_BossDeath (mobj_t* mo)
{
    A_Bosstype_Death( mo, mo->type );
}

//
// A_KeenDie
// DOOM II special, map 32.
// Uses special tag 666.
//
void A_KeenDie (mobj_t* mo)
{
    A_Fall (mo);

    // Doom2 MAP32: When last Keen is dead,
    //   execute doorOpen(doors tagged 666).
    // Some Dehacked mods use Keen death to trigger 666 tagged door.
    // Cannot use mo->type when dehacked may have only changed death frame.
    A_Bosstype_Death(mo, MT_KEEN);
}

void A_Hoof (mobj_t* mo)
{
    S_StartObjSound(mo, sfx_hoof);
    A_Chase (mo);
}

void A_Metal (mobj_t* mo)
{
    S_StartObjSound(mo, sfx_metal);
    A_Chase (mo);
}

void A_BabyMetal (mobj_t* mo)
{
    S_StartObjSound(mo, sfx_bspwlk);
    A_Chase (mo);
}

void A_OpenShotgun2 ( player_t*     player,
                      pspdef_t*     psp )
{
    S_StartAttackSound(player->mo, sfx_dbopn);
}

void A_LoadShotgun2 ( player_t*     player,
                      pspdef_t*     psp )
{
    S_StartAttackSound(player->mo, sfx_dbload);
}

void A_ReFire ( player_t*     player,
                pspdef_t*     psp );

void A_CloseShotgun2 ( player_t*     player,
                       pspdef_t*     psp )
{
    S_StartAttackSound(player->mo, sfx_dbcls);
    A_ReFire(player,psp);
}

// [WDJ] Remove hard limits. Similar to prboom, killough 3/26/98.
// Dynamic allocation.
static mobj_t* * braintargets; // dynamic array of ptrs
static int     braintargets_max = 0; // allocated
static int     numbraintargets;
static int     braintargeton;

// Original was 32 max.
// Non-fatal - limited for useability, otherwise can have too many thinkers.
// This affects compatibility in netplay.
#ifdef MACHINE_MHZ
#define MAX_BRAINTARGETS  (32*MACHINE_MHZ/100)
#endif

// return 1 on success
boolean  expand_braintargets( void )
{
    int needed = braintargets_max += 32;
#ifdef MAX_BRAINTARGETS
    if( needed > MAX_BRAINTARGETS )
    {
        I_SoftError( "Expand braintargets exceeds MAX_BRAINTARGETS %d.\n", MAX_BRAINTARGETS );
        return 0;
    }
#endif
    // realloc to new size, copying contents
    mobj_t ** new_braintargets =
     realloc( braintargets, sizeof(mobj_t *) * needed );
    if( new_braintargets )
    {
        braintargets = new_braintargets;
        braintargets_max = needed;
    }
    else
    {
        // non-fatal protection, allow savegame or continue play
        // realloc fail does not disturb existing allocation
        numbraintargets = braintargets_max;
        I_SoftError( "Expand braintargets realloc failed at $d.\n", needed );
        return 0;  // fail to expand
    }
    return 1;
}

void P_Init_BrainTarget()
{
    thinker_t*  thinker;

    // find all the target spots
    numbraintargets = 0;
    braintargeton = 0;

    thinker = thinkercap.next;
    for (thinker = thinkercap.next ;
         thinker != &thinkercap ;
         thinker = thinker->next)
    {
        if (thinker->function.acp1 != (actionf_p1)P_MobjThinker)
            continue;   // not a mobj

        register mobj_t* m = (mobj_t *)thinker;

        if (m->type == MT_BOSSTARGET )
        {
            if( numbraintargets >= braintargets_max )
            {
                if( ! expand_braintargets() )  break;
            }
            braintargets[numbraintargets] = m;
            numbraintargets++;
        }
    }
}


void A_BrainAwake (mobj_t* mo)
{
    S_StartSound(sfx_bossit);
}


void A_BrainPain (mobj_t*       mo)
{
    S_StartSound(sfx_bospn);
}


void A_BrainScream (mobj_t*     mo)
{
    int         x;
    int         y;
    int         z;
    mobj_t*     th;

    for (x=mo->x - 196*FRACUNIT ; x< mo->x + 320*FRACUNIT ; x+= FRACUNIT*8)
    {
        y = mo->y - 320*FRACUNIT;
        z = 128 + PP_Random(pr_brainscream)*2*FRACUNIT;
        th = P_SpawnMobj (x,y,z, MT_ROCKET);
        th->momz = PP_Random(pr_brainscream)*512;

        P_SetMobjState (th, S_BRAINEXPLODE1);

        th->tics -= PP_Random(pr_brainscream)&7;
        if (th->tics < 1)
            th->tics = 1;
    }

    S_StartSound(sfx_bosdth);
}



void A_BrainExplode (mobj_t* mo)
{
    int         x;
    int         y;
    int         z;
    mobj_t*     th;

    x = (PP_SignedRandom(pr_brainexp)<<11)+mo->x;
    y = mo->y;
    z = 128 + PP_Random(pr_brainexp)*2*FRACUNIT;
    th = P_SpawnMobj (x,y,z, MT_ROCKET);
    th->momz = PP_Random(pr_brainexp)*512;

    P_SetMobjState (th, S_BRAINEXPLODE1);

    th->tics -= PP_Random(pr_brainexp)&7;
    if (th->tics < 1)
        th->tics = 1;
}


void A_BrainDie (mobj_t*        mo)
{
    if( cv_allowexitlevel.EV )
       G_ExitLevel ();
}

void A_BrainSpit (mobj_t*       mo)
{
    mobj_t*     targ;
    mobj_t*     newmobj;

    static int  easy = 0;

    easy ^= 1;
    if (gameskill <= sk_easy && (!easy))
        return;

    if( numbraintargets>0 ) 
    {
        // shoot a cube at current target
        targ = braintargets[braintargeton];
        braintargeton = (braintargeton+1)%numbraintargets;
        
        // spawn brain missile
        newmobj = P_SpawnMissile (mo, targ, MT_SPAWNSHOT);
        if(newmobj)
        {
            P_SetReference( newmobj->target, targ);
            newmobj->target = targ;
            newmobj->reactiontime =
                ((targ->y - mo->y)/newmobj->momy) / newmobj->state->tics;
            
            // [WDJ] MBF: From PrBoom, MBF, EternityEngine.
            // killough 7/18/98: brain friendliness is transferred
            newmobj->flags = (newmobj->flags & ~MF_FRIEND) | (mo->flags & MF_FRIEND);
            P_UpdateClassThink(&newmobj->thinker, TH_unknown);
        }

        S_StartSound(sfx_bospit);
    }
}



void A_SpawnFly (mobj_t* mo);

// travelling cube sound
void A_SpawnSound (mobj_t* mo)
{
    S_StartObjSound(mo,sfx_boscub);
    A_SpawnFly(mo);
}

void A_SpawnFly (mobj_t* mo)
{
    mobj_t*     newmobj;
    mobj_t*     fog;
    mobj_t*     targ;
    int         r;
    mobjtype_t  type;

    if (--mo->reactiontime)
        return; // still flying

    targ = mo->target;
    if( targ == NULL )
    {
        // Happens if save game with cube flying.
        // targ should be the previous braintarget.
        int bt = ((braintargeton == 0)? numbraintargets : braintargeton) - 1;
        targ = braintargets[bt];
    }

    // First spawn teleport fog.
    fog = P_SpawnMobj (targ->x, targ->y, targ->z, MT_SPAWNFIRE);
    S_StartObjSound(fog, sfx_telept);

    // Randomly select monster to spawn.
    r = PP_Random(pr_spawnfly);

    // Probability distribution (kind of :),
    // decreasing likelihood.
    if ( r<50 )
        type = MT_TROOP;
    else if (r<90)
        type = MT_SERGEANT;
    else if (r<120)
        type = MT_SHADOWS;
    else if (r<130)
        type = MT_PAIN;
    else if (r<160)
        type = MT_HEAD;
    else if (r<162)
        type = MT_VILE;
    else if (r<172)
        type = MT_UNDEAD;
    else if (r<192)
        type = MT_BABY;
    else if (r<222)
        type = MT_FATSO;
    else if (r<246)
        type = MT_KNIGHT;
    else
        type = MT_BRUISER;

    newmobj     = P_SpawnMobj (targ->x, targ->y, targ->z, type);

    // MBF: killough 7/18/98: brain friendliness is transferred
    newmobj->flags = (newmobj->flags & ~MF_FRIEND) | (mo->flags & MF_FRIEND);
    P_UpdateClassThink(&newmobj->thinker, TH_unknown);

    if( P_LookForTargets(newmobj, true) )
        P_SetMobjState (newmobj, newmobj->info->seestate);
    // cube monsters have no mapthing (spawnpoint=NULL), do not respawn

    // telefrag anything in this spot
    P_TeleportMove (newmobj, newmobj->x, newmobj->y, true);

    // remove self (i.e., cube).
    P_RemoveMobj (mo);
}



void A_PlayerScream (mobj_t* mo)
{
    // Default death sound.
    int         sound = sfx_pldeth;

    if ( (gamemode == doom2_commercial)
        &&      (mo->health < -50))
    {
        // IF THE PLAYER DIES
        // LESS THAN -50% WITHOUT GIBBING
        sound = sfx_pdiehi;
    }
    S_StartScreamSound(mo, sound);
}


// Exl: More Toxness :)
// Running scripts from states (both mobj and weapon)
void A_StartFS(mobj_t *actor)
{
   // load script number from misc1
   int misc1 = actor->tics;
   actor->tics = 0;
   T_RunScript(misc1, actor);
}

void A_StartWeaponFS(player_t *player, pspdef_t *psp)
{
   int misc1;

   // check all pointers for validacy
   if(player && psp && psp->state)
   {
                misc1 = psp->tics;
                psp->tics = 0;
                T_RunScript(misc1, player->mo);
   }
}


// [WDJ] From MBF, PrBoom, EternityEngine.
// cph - MBF-added codepointer functions.

// killough 11/98: kill an object
void A_Die(mobj_t *actor)
{
    P_DamageMobj(actor, NULL, NULL, actor->health);
}

//
// A_Detonate
// killough 8/9/98: same as A_Explode, except that the damage is variable
//
void A_Detonate(mobj_t *mo)
{
    P_RadiusAttack(mo, mo->target, mo->info->damage);
}

//
// killough 9/98: a mushroom explosion effect, sorta :)
// Original idea: Linguica
//
void A_Mushroom(mobj_t *actor)
{
    int i, j, n = actor->info->damage;

    A_Explode(actor);  // First make normal explosion

    // Now launch mushroom cloud
    for (i = -n; i <= n; i += 8)
    {
        for (j = -n; j <= n; j += 8)
        {
            mobj_t target = *actor, *mo;
            // Aim in many directions from source, up fairly high.
            target.x += i << FRACBITS;
            target.y += j << FRACBITS;
            target.z += P_AproxDistance(i,j) << (FRACBITS+2);
            // Launch fireball
            mo = P_SpawnMissile(actor, &target, MT_FATSHOT);
            mo->momx >>= 1;
            mo->momy >>= 1;  // Slow it down a bit
            mo->momz >>= 1;
            mo->flags &= ~MF_NOGRAVITY;  // Make debris fall under gravity
        }
    }
}

//
// killough 11/98
//
// The following were inspired by Len Pitre
// A small set of highly-sought-after code pointers


void A_Spawn(mobj_t *mo)
{
    state_ext_t * sep = P_state_ext( mo->state );
    if( sep->parm1 )
    {
//        mobj_t *newmobj = 
        P_SpawnMobj(mo->x, mo->y, (sep->parm2 << FRACBITS) + mo->z,
                    sep->parm1 - 1);
// CPhipps - no friendlyness (yet)
//      newmobj->flags = (newmobj->flags & ~MF_FRIEND) | (mo->flags & MF_FRIEND);
    }
}

void A_Turn(mobj_t *mo)
{
    state_ext_t * sep = P_state_ext( mo->state );
    mo->angle += (uint32_t)((((uint64_t) sep->parm1) << 32) / 360);
}

void A_Face(mobj_t *mo)
{
    state_ext_t * sep = P_state_ext( mo->state );
    mo->angle = (uint32_t)((((uint64_t) sep->parm1) << 32) / 360);
}

// Melee attack
void A_Scratch(mobj_t *mo)
{
    state_ext_t * sep = P_state_ext( mo->state );

    // [WDJ] Unraveled from horrible one-liner.
    A_FaceTarget(mo);       
    if( ! P_CheckMeleeRange(mo) )  return;

    if( sep->parm2 )
    {
       S_StartObjSound(mo, sep->parm2);
    }

    P_DamageMobj(mo->target, mo, mo, sep->parm1);
}

void A_PlaySound(mobj_t *mo)
{
    state_ext_t * sep = P_state_ext( mo->state );
    if( sep->parm2 )
        S_StartSound( sep->parm1 );
    else
        S_StartObjSound( mo, sep->parm1 );
}

void A_RandomJump(mobj_t *mo)
{
    state_ext_t * sep = P_state_ext( mo->state );

    // As in EternityEngine, test first, then use Random Number.
    statenum_t si = deh_frame_to_state( sep->parm1 );
    if( si == S_NULL )  return;
    
    if( PP_Random(pr_randomjump) < sep->parm2 )
        P_SetMobjState(mo, si);
}

//
// This allows linedef effects to be activated inside deh frames.
//
void A_LineEffect(mobj_t *mo)
{
    // [WDJ] From PrBoom, EternityEngine.
    // haleyjd 05/02/04: bug fix:
    // This line can end up being referred to long after this function returns,
    // thus it must be made static or memory corruption is possible.
    static line_t  fake_line;

    player_t fake_player;
    player_t * oldplayer;
    state_ext_t * sep = P_state_ext( mo->state );

#if 0
    // From EternityEngine
    if( mo->eflags & MIF_LINEDONE )  return; // Unless already used up
#endif

    fake_line = *lines;

    fake_line.special = (short)sep->parm1;  // param linedef type
    if( !fake_line.special )
        return;

    fake_line.tag = (short)sep->parm2;  // param sector tag

    // Substitute a fake player to absorb effects.
    // Do this after tests that return.  Do not leave fake player in mo.
    fake_player.health = 100;  // make em alive
    oldplayer = mo->player;
    mo->player = &fake_player;  // fake player

    // Invoke the fake line, first by invoke, then by crossing.
    if( !P_UseSpecialLine(mo, &fake_line, 0) )
        P_CrossSpecialLine(&fake_line, 0, mo);

    mo->player = oldplayer;  // restore player

#if 0
    // From EternityEngine
    // If type cleared, no more for this thing.
    if( !fake_line.special )
        mo->eflags |= MIF_LINEDONE;
#else
    // If type cleared, no more for this state line type.
    sep->parm1 = fake_line.special;
#endif
}

#include "p_henemy.c"
