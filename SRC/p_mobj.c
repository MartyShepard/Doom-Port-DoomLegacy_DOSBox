// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: p_mobj.c 1397 2018-07-02 03:39:47Z wesleyjohnson $
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
// $Log: p_mobj.c,v $
// Revision 1.48  2005/12/20 14:58:26  darkwolf95
// Monster behavior CVAR - Affects how monsters react when they shoot each other
//
// Revision 1.47  2004/07/27 08:19:36  exl
// New fmod, fs functions, bugfix or 2, patrol nodes
//
// Revision 1.46  2003/11/21 22:50:47  darkwolf95
// FS spawned items and monsters retain their z coordinates for respawning
//
// Revision 1.45  2003/08/11 13:50:01  hurdler
// go final + translucent HUD + fix spawn in net game
//
// Revision 1.44  2003/07/21 16:47:13  hurdler
// Revision 1.42  2003/06/11 04:01:01  ssntails
//
// Revision 1.41  2003/06/11 03:38:09  ssntails
// THING Z definable in levels by using upper 9 bits
//
// Revision 1.40  2003/06/10 23:39:12  ssntails
// Any angle support for THINGS (0-360)
//
// Revision 1.39  2003/03/22 22:35:59  hurdler
//
// Revision 1.37  2002/09/27 16:40:09  tonyd
// First commit of acbot
//
// Revision 1.36  2002/09/15 12:15:41  hurdler
// Fix hanging bodies bug  properl
//
// Revision 1.35  2002/09/12 20:10:50  hurdler
// Added some cvars
//
// Revision 1.34  2002/08/16 17:35:51  judgecutor
// Fixed 'ceiling things on the floor' bug
//
// Revision 1.33  2002/07/24 19:03:08  ssntails
// Added support for things to retain spawned Z position.
//
// Revision 1.32  2002/06/14 02:49:46  ssntails
// Fix for 3dfloor bug with objects on them.
//
// Revision 1.31  2002/01/12 02:21:36  stroggonmeth
//
// Revision 1.30  2001/08/12 15:21:04  bpereira
// see my log
//
// Revision 1.29  2001/08/08 20:34:43  hurdler
// Big TANDL update
//
// Revision 1.28  2001/07/16 22:35:41  bpereira
// - fixed crash of e3m8 in heretic
// - fixed crosshair not drawed bug
//
// Revision 1.27  2001/04/02 18:54:32  bpereira
// Revision 1.26  2001/04/01 17:35:06  bpereira
// Revision 1.25  2001/03/30 17:12:50  bpereira
//
// Revision 1.24  2001/03/13 22:14:19  stroggonmeth
// Long time no commit. 3D floors, FraggleScript, portals, ect.
//
// Revision 1.23  2001/03/03 06:17:33  bpereira
// Revision 1.22  2001/02/24 13:35:20  bpereira
// Revision 1.21  2001/02/10 12:27:14  bpereira
//
// Revision 1.20  2001/01/25 22:15:43  bpereira
// added heretic support
//
// Revision 1.19  2000/11/04 16:23:43  bpereira
// Revision 1.18  2000/11/02 19:49:36  bpereira
//
// Revision 1.17  2000/11/02 17:50:08  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.16  2000/10/21 08:43:30  bpereira
// Revision 1.15  2000/10/16 20:02:30  bpereira
// Revision 1.14  2000/10/01 10:18:18  bpereira
// Revision 1.13  2000/09/28 20:57:16  bpereira
// Revision 1.12  2000/08/31 14:30:55  bpereira
// Revision 1.11  2000/08/11 19:10:13  metzgermeister
// Revision 1.10  2000/04/16 18:38:07  bpereira
//
// Revision 1.9  2000/04/11 19:07:24  stroggonmeth
// Finished my logs, fixed a crashing bug.
//
// Revision 1.8  2000/04/08 17:29:25  stroggonmeth
//
// Revision 1.7  2000/04/06 20:40:22  hurdler
// Mostly remove warnings under windows
//
// Revision 1.6  2000/04/05 15:47:46  stroggonmeth
// Added hack for Dehacked lumps. Transparent sprites are now affected by colormaps.
//
// Revision 1.5  2000/04/04 00:32:47  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.4  2000/03/29 19:39:48  bpereira
// Revision 1.3  2000/02/27 00:42:10  hurdler
// Revision 1.2  2000/02/26 00:28:42  hurdler
// Mostly bug fix (see borislog.txt 23-2-2000, 24-2-2000)
//
//
// DESCRIPTION:
//      Moving object handling. Spawn functions.
//
//-----------------------------------------------------------------------------

#include "doomincl.h"
  // memset
#include "p_local.h"
  // p_mobj.h
#include "p_tick.h"
  // think
#include "p_inter.h"
#include "p_fab.h"
#include "p_setup.h"
  //levelflats to test if mobj in water sector
#include "g_game.h"
#include "g_input.h"
#include "st_stuff.h"
#include "hu_stuff.h"
#include "r_main.h"
#include "r_things.h"
#include "r_sky.h"
#include "s_sound.h"
#include "z_zone.h"
#include "m_random.h"
#include "d_clisrv.h"
#include "r_splats.h"   //faB: in dev.

#include "b_game.h"     //added by AC for acbot


byte  EN_catch_respawn_0;  // enable catch Nightmare respawn at (0,0)
  // ! comp[comp_respawn]


// [WDJ] Voodoo doll 4/30/2009
// #define VOODOO_DEBUG

// A voodoo doll is a player mobj that is not under control of the player.
// 
// A voodoo doll is an accident of having multiple start points for a player.
// It has been used in levels as a token to trip linedefs and create
// sequenced actions, and thus are required to play some wads, like FreeDoom.

// [WDJ] Things a voodoo doll must do:
// * be carried by scrolling floors as a thing (FreeDoom, etc..)
// * trip a linedef as a player (FreeDoom, etc..)
// * pickup an item as if it was the player (Plutonia MAP28)
// * telefrag kills the player (TNT MAP30, ic2005)
// * crushed by crushing ceiling, and pass it to the player
// * take damage and pass it to the player

// Set an mobj to be a voodoo doll.
void P_Set_Voodoo( int playernum, mobj_t * voodoo_mobj )
{
#ifdef VOODOO_DEBUG   
    debug_Printf("Set Voodoo mobj\n");
#endif
    // Must have player set for P_XYMovement and P_CrossSpecialLine
    // NULL player will not trip W1 linedef, P_CrossSpecialLine uses test
    // on player field.
    voodoo_mobj->player = &players[playernum];	// point to real player
    // Code will intercept voodoo doll where it does not want side effects.
    // Detect by (voodoo_mobj->player->mo != voodoo_mobj)

    // Already spawned as player, set differences.
    // reasonable voodoo settings
    voodoo_mobj->flags &= ~(MF_COUNTKILL | MF_COUNTITEM | MF_CORPSE);
    voodoo_mobj->flags |= MF_NOBLOOD | MF_PICKUP | MF_SHOOTABLE;
    voodoo_mobj->flags2 |= MF2_NODMGTHRUST;
   
    voodoo_mobj->skin = 0;	// orig marine skin
    voodoo_mobj->angle = 0;
    voodoo_mobj->health = 100;
}

// Spawn voodoo doll at a playerspawn mapthing start point
void P_SpawnVoodoo( int playernum, mapthing_t * mthing )
{
    // Vanilla Doom does not voodoo when deathmatch.
    if( (voodoo_mode == VM_vanilla) && (cv_deathmatch.EV > 0) )
        return;

    if( playernum > 0 )
    {
        if(voodoo_mode == VM_auto)
            voodoo_mode = VM_multispawn;  // wad has multiple voodoo spawn
        // cannot create voodoo for player without mobj
        if( ! playeringame[playernum] )  // no player
        {
            if( voodoo_mode == VM_target )
                playernum = 0;  // will redirect target anyway
            else
                return;  // cannot create voodoo
                         // will have to create it later if player joins late
        }
    }

    CONS_Printf("Spawn Voodoo doll, player %d.\n", playernum);
    // A copy of P_SpawnPlayer, with the player removed
    // position
    fixed_t x = mthing->x << FRACBITS;
    fixed_t y = mthing->y << FRACBITS;
    fixed_t z = ONFLOORZ;
    
    mobj_t *mobj = P_SpawnMobj(x, y, z, MT_PLAYER);
       // does P_SetThingPosition
    mthing->mobj = mobj;
   
    P_Set_Voodoo( playernum, mobj );

    if( ! multiplayer )
    {
        spechit_player = &players[0];  // default picker
    }
}

// protos.
CV_PossibleValue_t viewheight_cons_t[] = { {16, "MIN"}, {56, "MAX"}, {0, NULL} };
CV_PossibleValue_t maxsplats_cons_t[] = { {1, "MIN"}, {MAXLEVELSPLATS, "MAX"}, {0, NULL} };

consvar_t cv_viewheight = { "viewheight", VIEWHEIGHTS, CV_VALUE, viewheight_cons_t, NULL };

// Needed by MBF, so use it elsewhere too.
#define GRAVITY1   FRACUNIT

//Fab:26-07-98:
consvar_t cv_gravity = { "gravity", "1", CV_NETVAR | CV_FLOAT | CV_SHOWMODIF };
consvar_t cv_splats = { "splats", "1", CV_SAVE, CV_OnOff };
consvar_t cv_maxsplats = { "maxsplats", "512", CV_SAVE, maxsplats_cons_t, NULL };

static const fixed_t FloatBobOffsets[64] = {
    0, 51389, 102283, 152192,
    200636, 247147, 291278, 332604,
    370727, 405280, 435929, 462380,
    484378, 501712, 514213, 521763,
    524287, 521763, 514213, 501712,
    484378, 462380, 435929, 405280,
    370727, 332604, 291278, 247147,
    200636, 152192, 102283, 51389,
    -1, -51390, -102284, -152193,
    -200637, -247148, -291279, -332605,
    -370728, -405281, -435930, -462381,
    -484380, -501713, -514215, -521764,
    -524288, -521764, -514214, -501713,
    -484379, -462381, -435930, -405280,
    -370728, -332605, -291279, -247148,
    -200637, -152193, -102284, -51389
};

//
// P_SetMobjState
// Returns true if the mobj is still present.
//
//SoM: 4/7/2000: Boom code...
boolean P_SetMobjState(mobj_t * mobj, statenum_t state)
{
    static statenum_t seenstate_tab[NUMSTATES]; // fast transition table
    static int recursion = 0;  // detects recursion

    boolean ret = true;         // return value
    state_t *st;

    //remember states seen, to detect cycles:

    statenum_t *seenstate = seenstate_tab;      // pointer to table
    statenum_t st1 = state;       // initial state
    statenum_t tempstate[NUMSTATES];    // for use with recursion

    if (recursion++)    // if recursion detected,
    {
        seenstate = tempstate;
        memset(tempstate, 0, sizeof tempstate);  // clear state table
    }

    do
    {
        if (state == S_NULL)
        {
            // [WDJ] This can be dereferenced, even after being removed.
            mobj->state = &states[S_NULL];
            P_RemoveMobj(mobj);
            ret = false;
            break;      // killough 4/9/98
        }

        st = &states[state];
        mobj->state = st;
        mobj->tics = st->tics;
        mobj->sprite = st->sprite;
        mobj->frame = st->frame;

        // Modified handling.
        // Call action functions when the state is set

        if (st->action.acp1)
            st->action.acp1(mobj);

        seenstate[state] = 1 + st->nextstate;   // killough 4/9/98
        state = st->nextstate;
        if( seenstate[state] )  break;  // killough 4/9/98

    } while(!mobj->tics);

    if (ret && !mobj->tics)     // killough 4/9/98: detect state cycles
        GenPrintf(EMSG_warn, "Warning: State Cycle Detected");

    if(--recursion == 0)
    {
        // Only upon seenstate_tab
        while( seenstate_tab[st1] )
        {
            register statenum_t st2 = seenstate_tab[st1] - 1;
            seenstate_tab[st1] = 0;   // killough 4/9/98: erase memory of states
            st1 = st2;
        }
    }

    return ret;
}

//----------------------------------------------------------------------------
//
// FUNC P_SetMobjStateNF
//
// Same as P_SetMobjState, but does not call the state function.
//
//----------------------------------------------------------------------------

boolean P_SetMobjStateNF(mobj_t * mobj, statenum_t state)
{
    state_t *st;

    if (state == S_NULL)
    {   // Remove mobj
        mobj->state = &states[S_NULL];
        P_RemoveMobj(mobj);
        return (false);
    }
    st = &states[state];
    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;
    return (true);
}

//
// P_ExplodeMissile
//
void P_ExplodeMissile(mobj_t * mo)
{
    if (mo->type == MT_WHIRLWIND)  // Heretic
    {
        if (++mo->special2 < 60)
            return;
    }

    mo->momx = mo->momy = mo->momz = 0;

    P_SetMobjState(mo, mobjinfo[mo->type].deathstate);

    if (EN_doom_etc)
    {
        mo->tics -= PP_Random(pr_explode) & 3;

        if (mo->tics < 1)
            mo->tics = 1;
    }

    mo->flags &= ~MF_MISSILE;

    if (mo->info->deathsound)
        S_StartObjSound(mo, mo->info->deathsound);
}

//----------------------------------------------------------------------------
//
// PROC P_FloorBounceMissile
// Heretic
//
//----------------------------------------------------------------------------

void P_FloorBounceMissile(mobj_t * mo)
{
    mo->momz = -mo->momz;
    P_SetMobjState(mo, mobjinfo[mo->type].deathstate);
}

//----------------------------------------------------------------------------
//
// PROC P_ThrustMobj
// Heretic
//
//----------------------------------------------------------------------------

void P_ThrustMobj(mobj_t * mo, angle_t angle, fixed_t move)
{
    mo->momx += FixedMul(move, cosine_ANG(angle));
    mo->momy += FixedMul(move, sine_ANG(angle));
}

//
// P_XYMovement
//
#define STOPSPEED               (0x1000/NEWTICRATERATIO)
// ORIG_FRICTION, FRICTION_NORM fixed_t 0xE800 = 0.90625
//#define FRICTION_NORM           0xe800
//#define FRICTION_LOW            0xf900
//#define FRICTION_FLY            0xeb00

//added:22-02-98: adds friction on the xy plane
// Called from P_XYMovement
// Called from P_MobjThinker
void P_XYFriction(mobj_t * mo, fixed_t oldx, fixed_t oldy)
{
    fixed_t friction = FRICTION_NORM;
    //valid only if player avatar
    player_t *player = mo->player;

    // voodoo dolls do not depend on player cmd, and do not affect player
    boolean voodoo_mo = (player && (player->mo != mo));
    if( voodoo_mo )
       player = NULL;  // simplify the tests


    // [WDJ] Player bob and state are dependent upon bob effect, not world motion.
    // Standing on a conveyor should not keep the player sprite walking.
    // Watch a two player game on a really greasy floor to see the difference.
    if( player ) // not voodoo doll
    {
        if (player->bob_momx > -STOPSPEED && player->bob_momx < STOPSPEED
            && player->bob_momy > -STOPSPEED && player->bob_momy < STOPSPEED
            && player->cmd.forwardmove == 0
            && player->cmd.sidemove == 0 )
        {
            // [WDJ] stop player bobbing
            player->bob_momx = player->bob_momy = 0;

            // [WDJ] stop player walking sprite
            // if in a walking frame, stop moving
            if( mo->type != MT_SPIRIT )
            {
                if (player->chickenTics)
                {  // Heretic
                    if ((unsigned) ((player->mo->state - states) - S_CHICPLAY_RUN1) < 4)
                        P_SetMobjState(player->mo, S_CHICPLAY);
                }
                else
                {
                    if ((unsigned) ((player->mo->state - states) - S_PLAY_RUN1) < 4)
                        P_SetMobjState(player->mo, S_PLAY);
                }
            }
        }
        else
        {
            // [WDJ] The walking and bob have always taken too long to stop
            // slow down bob before player stops
#if 1
            fixed_t friction_bob = (EV_legacy < 145)? FRICTION_NORM : (FRICTION_NORM*15/16);
            player->bob_momx = FixedMul( player->bob_momx, friction_bob);
            player->bob_momy = FixedMul( player->bob_momy, friction_bob);
#else
# define FRICTION_BOB   (FRICTION_NORM*15/16)
            player->bob_momx = FixedMul( player->bob_momx, FRICTION_BOB);
            player->bob_momy = FixedMul( player->bob_momy, FRICTION_BOB);
#endif	   
        }
    }


    // Stop when below minimum.
    // Players without commands, no player, and voodoo doll
    // [WDJ] Restored deleted voodoo checks, originally made by Killough 10/98,
    // from examination of PrBoom and zdoom.
    if (mo->momx > -STOPSPEED && mo->momx < STOPSPEED
        && mo->momy > -STOPSPEED && mo->momy < STOPSPEED
        && (!player  // and voodoo_dolls
            || (player->cmd.forwardmove == 0 && player->cmd.sidemove == 0)))
    {
#ifndef BOB_MOM
        // [WDJ] Appearance of walking was moved above to be controlled
        // by bob_mom, instead of mo->mom.
        // if in a walking frame, stop moving
        if (player)  // not if voodoo doll (do not affect player mobj)
        {
          if( mo->type != MT_SPIRIT )
          {
            if (player->chickenTics)
            {
                // Heretic
                if ((unsigned) ((player->mo->state - states) - S_CHICPLAY_RUN1) < 4)
                    P_SetMobjState(player->mo, S_CHICPLAY);
            }
            else
            {
                if ((unsigned) ((player->mo->state - states) - S_PLAY_RUN1) < 4)
                    P_SetMobjState(player->mo, S_PLAY);
            }
          }
        } // player
#endif
        mo->momx = 0;
        mo->momy = 0;
    }
    else
    {
        // not stopped
        // [WDJ] 3/2011 new bobbing and friction here, mostly Killough etal. 10/98.
        if( (mo->eflags & MF_UNDERWATER)
            && (EV_legacy >= 128) )
        {
            // slow down in water, not too much for playability issues
            friction = FRICTION_NORM *3/4;
//	    mo->momx = FixedMul(mo->momx, FRICTION_NORM *3/4);
//	    mo->momy = FixedMul(mo->momy, FRICTION_NORM *3/4);
//	    return;
        }
        else if (mo->z > mo->floorz)
        { 
            // not standing on a floor
            // MF2_ONMOBJ has FRICTION_NORM
            if ( !(mo->flags2 & MF2_ONMOBJ))
            {
                // not on obj or monster
                if (mo->flags2 & MF2_FLY)
                {
                    // heretic fly, and fly cheat
                    friction = FRICTION_FLY;
                }
                else
                {
                    return; // jumping players and falling have no friction
                }
            }
        }
        // standing on a floor
        else if(friction_model == FR_heretic)
        {
#if 1
            friction = P_GetFriction( mo );  // heretic friction in sector
#else
            if (mo->subsector->sector->special == 15)      // Friction_Low
            {
                friction = FRICTION_LOW;
            }
            else
            {
                friction = FRICTION_NORM;
            }
#endif
        }
        else if(friction_model >= FR_mbf)
        {
            // latest sector based friction model in common use (MBF, PrBoom, zdoom)
            friction = P_GetFriction( mo );
        }
        else if(friction_model == FR_boom)
        {
            //SoM: 3/28/2000: Use boom friction.
            if ((oldx == mo->x) && (oldy == mo->y))     // Did you go anywhere?
            {
                // Use original friction to not bob so much when not moving
                // but enough to escape being stuck in wall.
                friction = ORIG_FRICTION;
            }
            else
#ifdef FRICTIONTHINKER
            {
                friction = mo->friction;  // from friction thinker
            }
            mo->friction = ORIG_FRICTION;
#else
            {
                friction = P_GetFriction( mo );  // a reasonable substitute
            }
#endif
        }
        else
        {
            friction = FRICTION_NORM; // FR_orig
        }
        mo->momx = FixedMul(mo->momx, friction);
        mo->momy = FixedMul(mo->momy, friction);
    }
}


// Called by P_MobjThinker
void P_XYMovement(mobj_t * mo)
{
    // Heretic wind, when up against walls.
    static int windTab[3] = { 2048 * 5, 2048 * 10, 2048 * 25 };

    int numsteps = 1;
    fixed_t ptryx, ptryy;
    player_t *player;
    fixed_t xmove, ymove, xmove_rep, ymove_rep;
    fixed_t oldx, oldy;         //reducing bobbing/momentum on ice

    //added:18-02-98: if it's stopped
    if (!mo->momx && !mo->momy)
    {
        // No momentum
        if (mo->flags & MF_SKULLFLY)
        {
            // the skull slammed into something
            mo->flags &= ~MF_SKULLFLY;
            // mo->momz = 0;  // Doom, but momx=0, momy=0 already.
            mo->momx = mo->momy = mo->momz = 0;  // Heretic

            //added:18-02-98: comment: set in 'search new direction' state?
            P_SetMobjState(mo, ((EN_heretic)? mo->info->seestate : mo->info->spawnstate));
        }
        return;
    }

    // heretic/hexen wind
    if( EN_heretic_hexen && (mo->flags2 & MF2_WINDTHRUST) )
    {
        int special = mo->subsector->sector->special;
        switch (special)
        {
            case 40:
            case 41:
            case 42:   // Wind_East
                P_ThrustMobj(mo, 0, windTab[special - 40]);
                break;
            case 43:
            case 44:
            case 45:   // Wind_North
                P_ThrustMobj(mo, ANG90, windTab[special - 43]);
                break;
            case 46:
            case 47:
            case 48:   // Wind_South
                P_ThrustMobj(mo, ANG270, windTab[special - 46]);
                break;
            case 49:
            case 50:
            case 51:   // Wind_West
                P_ThrustMobj(mo, ANG180, windTab[special - 49]);
                break;
        }
    }

    oldx = mo->x;  // for later comparison in Boom bobbing reduction
    oldy = mo->y;

    ptryx = mo->x;
    ptryy = mo->y;
   
    player = mo->player;        //valid only if player avatar
    if( player && (player->mo != mo))
        player = NULL;  // player cheats not for voodoo dolls 

    if (mo->momx > MAXMOVE)
        mo->momx = MAXMOVE;
    else if (mo->momx < -MAXMOVE)
        mo->momx = -MAXMOVE;

    if (mo->momy > MAXMOVE)
        mo->momy = MAXMOVE;
    else if (mo->momy < -MAXMOVE)
        mo->momy = -MAXMOVE;

    xmove_rep = xmove = mo->momx;
    ymove_rep = ymove = mo->momy;

    // To make this demo compatible requires some additional demo tests.
    // Demo compatiblity is not high priority for DoomLegacy, and this
    // would partially affect fireballs and some fast player movements.
    // The original Doom and Heretic used both xmove/2 and xmove>>1 expressions,
    // which give different results.  Those expressions must be duplicated
    // exactly, to avoid having position drift in the old demos.

    if( EV_legacy >= 145 )
    {
        // [WDJ] 3/2011 Moved out of loop and converted to stepping.
        // Fixes mancubus fireballs which were too fast for collision tests,
        // makes steps equal in size, and makes loop test faster and predictable.
        // Boom bug had only the positive tests.
        if (xmove > MAXMOVE/2 || xmove < -MAXMOVE/2
           || ymove > MAXMOVE/2 || ymove < -MAXMOVE/2 )
        {
           xmove >>= 1;
           ymove >>= 1;
           numsteps = 2;
        }
        if (mo->info->speed > (mo->radius*2)) // faster than radius*2
        {
           // Mancubus missiles and the like.
           xmove >>= 1;
           ymove >>= 1;
           numsteps *= 2;
        }

        xmove_rep = xmove;
        ymove_rep = ymove;

        if( EV_legacy >= 147 && (numsteps > 1))
        {
            // [WDJ] Account for odd amount of momentum.
            xmove = mo->momx - (xmove_rep * (numsteps-1));
            ymove = mo->momy - (ymove_rep * (numsteps-1));
        }
    }

    do
    {
        if( EV_legacy >= 145 )
        {
#if 1
            // [WDJ] Logically safer.
            ptryx = mo->x + xmove;
            ptryy = mo->y + ymove;
            xmove = xmove_rep;
            ymove = ymove_rep;
#else
            // [WDJ] Same result for now,
            // but depends on TryMove only taking whole step and not partial steps.
            ptryx += xmove;
            ptryy += ymove;
#endif
        }
        else
        {
            // [WDJ] If this is not done exactly this way,
            // then errors in the LSB accumulate.
            // [WDJ] Note:  (xmove/2  !=  xmove>>1), and it gets used both ways.

            // killough 8/9/98: fix bug in original Doom source:
            // Large negative displacements were never considered.
            // This explains the tendency for Mancubus fireballs
            // to pass through walls.
            // CPhipps - compatibility optioned
      
            if (xmove > MAXMOVE/2 || ymove > MAXMOVE/2
                || ( ! EN_doom_movestep_bug
                     && (xmove < -MAXMOVE/2 || ymove < -MAXMOVE/2))
               )
            {
                ptryx = mo->x + xmove/2;
                ptryy = mo->y + ymove/2;
                xmove >>= 1;
                ymove >>= 1;
                numsteps += numsteps;
            }
            else
            {
                ptryx = mo->x + xmove;
                ptryy = mo->y + ymove;
                numsteps = 1;
            }
        }

        if (P_TryMove(mo, ptryx, ptryy, true)) //SoM: 4/10/2000
        {   // success
            // hack for playability : walk in-air to jump over a small wall
            if (player)
                player->cheats &= ~CF_JUMPOVER;
        }
        else  // P_TryMove
        {
            // blocked move

            // gameplay issue : let the marine move forward while trying
            //                  to jump over a small wall
            //    (normally it can not 'walk' while in air)
            // BP:1.28 no more use CF_JUMPOVER, but i leave it for backward lmps compatibility
            if (player)
            {
                // tmr_floorz returned by P_TryMove
                if (tmr_floorz - mo->z > MAXSTEPMOVE)
                {
                    if (mo->momz > 0)
                        player->cheats |= CF_JUMPOVER;
                    else
                        player->cheats &= ~CF_JUMPOVER;
                }
            }

            if (mo->flags & MF_MISSILE)
            {
                // TODO: put missile bounce here
                goto missile_impact;
            }

            // [WDJ] MBF BOUNCES and Heretic SLIDE conflict, one or the other.
            // Boom has only player slides, but heretic has SLIDE attribute
            if (mo->flags2 & MF2_SLIDE)  // Heretic, and player.
            {   // try to slide along it
                // Alters momx,momy, and calls P_TryMove
                P_SlideMove(mo);
                continue;
            }

            // MBF bounce
            // [WDJ] From PrBoom and MBF source, rearranged, different order.
            // MF_MISSILE is already handled before this test, instead of
            // after as in PrBoom.
            // With EN_variable_friction enabled (normal), ice gives bounce
            // to all object and monsters that hit a lower wall.
            if( EN_mbf
                // && !(mo->flags & MF_MISSILE)  // already handled above
                && ( (mo->flags & MF_BOUNCES)
                     || ( EN_variable_friction
                          && tmr_blockingline  // solid wall
                          && !player  // player MF2_SLIDE has precedence, redundant
                          && mo->z <= mo->floorz  // hit lower wall
                          && (P_GetFriction(mo) > ORIG_FRICTION)  // ice
                        )
                   )
              )
            {
                if( ! tmr_blockingline )  goto zero_mom;

                // [WDJ] Bounce off wall, rewritten.
                int tbx = (tmr_blockingline->dx >> FRACBITS);
                int tby = (tmr_blockingline->dy >> FRACBITS);
                int tun = tbx*tbx + tby*tby;
                // [WDJ] Had no protection against short walls.
                if( tun > 0 )
                {
                    // Reflect momentum away from wall.
                    fixed_t mp = ((tbx * mo->momx) + (tby * mo->momy)) / tun;
                    fixed_t mwx = FixedMul( mp, tmr_blockingline->dx );
                    fixed_t mwy = FixedMul( mp, tmr_blockingline->dy );
                    mo->momx = mwx*2 - mo->momx;
                    mo->momy = mwy*2 - mo->momy;

                    if( !(mo->flags & MF_NOGRAVITY) )
                    {
                        // Under gravity. Slow it down in direction
                        // perpendicular to the wall.
                        mo->momx = (mo->momx + mwx)/2;
                        mo->momy = (mo->momy + mwy)/2;
                    }
                }
                continue;  // continue movement
            }

    zero_mom:
            mo->momx = mo->momy = 0;
            break;  // otherwise does not stop
        }
    } while ( --numsteps );

    if (mo->flags & (MF_MISSILE | MF_SKULLFLY))
        goto missile_fly;

    // slow down
    if (player)
    {
        if (player->cheats & CF_NOMOMENTUM)
        {
            // debug option for no sliding at all
            mo->momx = mo->momy = 0;
            player->bob_momx = player->bob_momy = 0;
            return;
        }
        else if (player->cheats & CF_FLYAROUND)  // fly cheat
        {
            // [WDJ] Heretic FLY was removed from here because it should be
            // subject to underwater and other tests, as in Legacy2.
            // Implement FLYAROUND using heretic fly, avoiding extra tests.
            uint32_t f2 = mo->flags2;
            mo->flags2 |= MF2_FLY;
            P_XYFriction(mo, oldx, oldy);
            mo->flags2 = f2;
            return;
        }
//        if(mo->z <= mo->subsector->sector->floorheight)
//          P_XYFriction (mo, oldx, oldy);
    }

    // No friction for missiles or skulls, when airborne.
    if ((mo->z > mo->floorz)
        && !(mo->flags2 & (MF2_FLY | MF2_ONMOBJ))
        && !(mo->eflags & MF_UNDERWATER))
    {
        return; // no friction when airborne
    }

    // tmr_dropoffz is set by P_CheckPosition() and P_TryMove(), called earlier.
    if( (mo->flags & MF_CORPSE)
        || ((mo->flags & MF_BOUNCES) && (mo->z > tmr_dropoffz))  // hanging
        || (mo->eflags & MF_FALLING)  // falling	
      )
    {
        // do not stop sliding
        //  if halfway off a step with some momentum
        if (mo->momx > FRACUNIT/4 || mo->momx < -FRACUNIT/4
            || mo->momy > FRACUNIT/4 || mo->momy < -FRACUNIT/4)
        {
            if( EV_legacy < 132 )
            {
                // original Vanilla, Heretic, and Boom test
                if (mo->z != mo->subsector->sector->floorheight)
                    return;
            }
            else
            {
                // Legacy
                if (mo->z != mo->floorz)
                    return;
            }
        }
    }

    P_XYFriction(mo, oldx, oldy); // thing friction
    return;

    // [WDJ] Special exit handling for missiles
missile_fly:
//    if (mo->flags & MF_SKULLFLY)  return;
    // no friction for missiles ever
    
    // Put missile bouncing here (hexen)
    return;
   

    // [WDJ] Exit taken out of loop to make it easier to read.
missile_impact:
    // explode a missile
    // tmr_ceilingline returned by P_TryMove
    if (tmr_ceilingline
        && tmr_ceilingline->backsector
        && tmr_ceilingline->backsector->ceilingpic == skyflatnum
        // Added tests, not in Boom, Heretic.
        && tmr_ceilingline->frontsector
        && tmr_ceilingline->frontsector->ceilingpic == skyflatnum
        && mo->subsector->sector->ceilingheight == mo->ceilingz)
    {
        if (!EN_boom   //SoM: 4/7/2000: DEMO'S
            || mo->z > tmr_ceilingline->backsector->ceilingheight)  // Boom
        {
            // Hack to prevent missiles exploding against the sky.
            // Does not handle sky floors.
            //SoM: 4/3/2000: Check frontsector as well..
            if (mo->type == MT_BLOODYSKULL)  // Heretic
            {
                mo->momx = mo->momy = 0;
                mo->momz = -GRAVITY1;
            }
            else
            {
                P_RemoveMobj(mo); // missile quietly dissappears
            }
            return;
        }
    }

    // draw damage on wall
    //SPLAT TEST ----------------------------------------------------------
#ifdef WALLSPLATS
    // tmr_blockingline returned by P_TryMove
    if( tmr_blockingline && (EV_legacy >= 129) ) //set by last P_TryMove() that failed
    {
        divline_t divl;
        divline_t misl;
        fixed_t frac;

        P_MakeDivline(tmr_blockingline, &divl);
        misl.x = mo->x;
        misl.y = mo->y;
        misl.dx = mo->momx;
        misl.dy = mo->momy;
        frac = P_InterceptVector(&divl, &misl);
        R_AddWallSplat( tmr_blockingline,
                        P_PointOnLineSide(mo->x, mo->y, tmr_blockingline),
                        "A_DMG3", mo->z, frac, SPLATDRAWMODE_SHADE);
    }
#endif
    // --------------------------------------------------------- SPLAT TEST
    // 
    P_ExplodeMissile(mo);
    return;
}


// [WDJ] Have multiple places where our more complicated gravity
// situations must be handled.
static
void  apply_gravity(mobj_t * mo)
{
    fixed_t gravityadd;

    //Fab: NOT SURE WHETHER IT IS USEFUL, just put it here too
    //     TO BE SURE there is no problem for the release..
    //     (this is done in P_Mobjthinker below normally)
    mo->eflags &= ~MF_JUSTHITFLOOR;

    gravityadd = -cv_gravity.value / NEWTICRATERATIO;

    // if waist under water, slow down the fall
    if( mo->eflags & MF_UNDERWATER )
    {
        if( mo->eflags & MF_SWIMMING )
            gravityadd = 0; // gameplay: no gravity while swimming
        else
            gravityadd >>= 2;
    }
    else if( mo->momz == 0 )
    {
        // mobj at stop, no floor, so feel the push of gravity!
        gravityadd <<= 1;
    }

    mo->momz += gravityadd;
}


//
// P_ZMovement
//
void P_ZMovement(mobj_t * mo)
{
    fixed_t dist;
    fixed_t delta;
    player_t *player = mo->player;
    boolean voodoo_mo = (player && (player->mo != mo));

    // Intercept the stupid 'fall through 3dfloors' bug SSNTails 06-13-2002
    if (mo->subsector->sector->ffloors)
    {
        ffloor_t *rovflr;
        fixed_t midfloor;
        int thingtop = mo->z + mo->height;

        for (rovflr = mo->subsector->sector->ffloors; rovflr; rovflr = rovflr->next)
        {
            if (!(rovflr->flags & FF_SOLID) || !(rovflr->flags & FF_EXISTS))
                continue;

            midfloor =
               *rovflr->bottomheight + ((*rovflr->topheight - *rovflr->bottomheight) / 2);
            if (abs(mo->z - midfloor) < abs(thingtop - midfloor))
            {
                // closer to feet
                if (*rovflr->topheight > mo->floorz)
                    mo->floorz = *rovflr->topheight;
            }
            else
            {
                // closer to head
                if (*rovflr->bottomheight < mo->ceilingz)
                    mo->ceilingz = *rovflr->bottomheight;
            }
        }
    }

    // MBF from PrBoom, modified.
    if( (mo->flags & MF_BOUNCES)  // MBF
        && mo->momz )  goto bouncer;

    // check for smooth step up
    if (player && (mo->z < mo->floorz)
        && !voodoo_mo  // voodoo does not pass this to player view
#ifdef CLIENTPREDICTION2
        && mo->type != MT_PLAYER
#else
        && mo->type != MT_SPIRIT
#endif
        )
    {
        player->viewheight -= mo->floorz - mo->z;

        player->deltaviewheight = ((((unsigned int)cv_viewheight.EV) << FRACBITS) - player->viewheight) >> 3;
    }

    // adjust height
    mo->z += mo->momz;

zmove_floater:
    if ((mo->flags & MF_FLOAT) && mo->target)
    {
        // float down towards target if too close
        if (!(mo->flags & MF_SKULLFLY) && !(mo->flags & MF_INFLOAT))
        {
            dist = P_AproxDistance(mo->x - mo->target->x, mo->y - mo->target->y);

            delta = (mo->target->z + (mo->height >> 1)) - mo->z;

            if (delta < 0 && dist < -(delta * 3))
                mo->z -= FLOATSPEED;
            else if (delta > 0 && dist < (delta * 3))
                mo->z += FLOATSPEED;
        }

    }

    if (player && (mo->flags2 & MF2_FLY)
        && !(mo->z <= mo->floorz) && (leveltime & 2))
    {
        // add the bobbing pattern
        mo->z += finesine[(FINEANGLES / 20 * leveltime >> 2) & FINEMASK];
    }

    // clip movement

    if (mo->z <= mo->floorz)
    {
        // hit the floor

        if (mo->flags & MF_MISSILE)
        {
            mo->z = mo->floorz;
            if (mo->flags2 & MF2_FLOORBOUNCE)  // Heretic
            {
                P_FloorBounceMissile(mo);
                return;
            }
            else if (mo->type == MT_MNTRFX2)  // Heretic
            {   // Minotaur floor fire can go up steps
                return;
            }
            else
            {
                if ((mo->flags & MF_NOCLIP) == 0)
                {
                    P_ExplodeMissile(mo);
                    return;
                }
            }
        }

        // Spawn splashes, etc.
        if ((mo->z - mo->momz) > mo->floorz)
            P_HitFloor(mo);

        mo->z = mo->floorz;

        // Note (id):
        //  somebody left this after the setting momz to 0,
        //  kinda useless there.
     // From PrBoom
     // cph - revised 2001/04/15 -
     // This was a bug in the Doom/Doom 2 source; the following code
     // is meant to make charging lost souls bounce off of floors, but it 
     // was incorrectly placed after momz was set to 0.
     // However, this bug was fixed in Doom95, Final/Ultimate Doom, and 
     // the v1.10 source release (which is one reason why it failed to sync 
     // some Doom2 v1.9 demos)
        if( EN_skull_bounce_floor && (mo->flags & MF_SKULLFLY) )
        {
            // the skull slammed into something
            mo->momz = -mo->momz;  // skull bounces
        }

        if (mo->momz < 0)       // falling
        {
            if( (mo->flags & MF_TOUCHY)  // MBF
                && (mo->health > 0) && (mo->eflags & MF_ARMED) )
            {
                // Explode on floor.
                P_DamageMobj( mo, NULL, NULL, mo->health );
                // That ought to kill it, but on EASY skill it won't.
                // PrBoom, EE, do not return here, probably for MISSILE test,
                // which we have already done (because of Heretic).
            }
            else
            if (player
                && !voodoo_mo  // voodoo does not pass this to player view
                && (mo->momz < (-8 * GRAVITY1))
                && !(mo->flags2 & MF2_FLY)
                )
            {
                // Squat down.
                // Decrease viewheight for a moment
                // after hitting the ground (hard),
                // and utter appropriate sound.
                player->deltaviewheight = mo->momz >> 3;
                if( mo->health > 0 )  // PrBoom, cph, no oof when dead.
                  S_StartObjSound(mo, sfx_oof);
            }

            // set it once and not continuously
            if (mo->z < mo->floorz)
                mo->eflags |= MF_JUSTHITFLOOR;

            mo->momz = 0;
        }

#if 0	   
        // [WDJ] Original Buggy skull bounce code.
        // Because momz has already been set to 0, this cannot do anything,
        // unless the floor hit the skull while the skull had momz > 0.
        // An important demo with this happening is needed to make this worth it.
        if( !EN_skull_bounce_floor && (mo->flags & MF_SKULLFLY) )
        {
            // the skull slammed into something
            mo->momz = -mo->momz;  // skull bounces
        }
#endif

        if (mo->info->crashstate && (mo->flags & MF_CORPSE))
        {
            P_SetMobjState(mo, mo->info->crashstate);
            mo->flags &= ~MF_CORPSE;
            return;
        }
    }
    else if (mo->flags2 & MF2_LOGRAV)
    {
        if (mo->momz == 0)
            mo->momz = -(cv_gravity.value >> 3) * 2;
        else
            mo->momz -= cv_gravity.value >> 3;
    }
    else if (!(mo->flags & MF_NOGRAVITY))       // Gravity here!
    {
        apply_gravity( mo );
    }

    if( mo->z > (mo->ceilingz - mo->height) )
    {
        mo->z = mo->ceilingz - mo->height;

        //added:22-02-98: player avatar hits his head on the ceiling, ouch!
        if (player
            && !voodoo_mo  // voodoo does not pass this to sound
            && (EV_legacy >= 112)
            && !(player->cheats & CF_FLYAROUND) && !(mo->flags2 & MF2_FLY)
            && (mo->momz > (8 * GRAVITY1)) )
        {
            S_StartObjSound(mo, sfx_ouch);
        }

        // PrBoom: cph 2001/04/15 -
        // Lost souls were meant to bounce off of ceilings;
        // PrBoom: if (!comp[comp_soul] && mo->flags & MF_SKULLFLY)
        if( EN_skull_bounce_fix && (mo->flags & MF_SKULLFLY) )
        {   // The skull slammed into something.
            mo->momz = -mo->momz;  // skull bounces
        }
       
        // hit the ceiling
        if (mo->momz > 0)
            mo->momz = 0;

        // Vanilla skull bounce in wrong place.
        // Upto DoomLegacy 1.46.3
        if( ! EN_skull_bounce_fix && (mo->flags & MF_SKULLFLY) )
        {   // the skull slammed into something
            // We might have hit a ceiling but had downward momentum
            // (e.g. ceiling is lowering onto skull), so for old demos we
            // must still do the buggy momentum reversal.
            // DoomLegacy (EV_legacy < 147)
            mo->momz = -mo->momz;  // skull bounces (now momz >= 0)
        }

        if ((mo->flags & MF_MISSILE) && !(mo->flags & MF_NOCLIP))
        {
            // Heretic has this sky test, but PrBoom and EE do not.
            //SoM: 4/3/2000: Don't explode on the sky!
            if ( mo->subsector->sector->ceilingpic == skyflatnum
                 && mo->subsector->sector->ceilingheight == mo->ceilingz
                 && (EV_legacy >= 129) )
            {
                if (mo->type == MT_BLOODYSKULL)
                {
                    mo->momx = mo->momy = 0;
                    mo->momz = -GRAVITY1;
                }
                else
                { 
                    P_RemoveMobj(mo);
                }
                return;
            }

            P_ExplodeMissile(mo);
            return;
        }
    }

    // z friction in water
    if( (EV_legacy >= 128)
        && ((mo->eflags & (MF_TOUCHWATER | MF_UNDERWATER)))
        && !(mo->flags & (MF_MISSILE | MF_SKULLFLY)) )
    {
        mo->momz = FixedMul(mo->momz, FRICTION_NORM * 3 / 4);
    }
    return;


bouncer:
    // [WDJ] From PrBoom and MBF source, modified.
    // MBF bounce off walls, ceiling, floor.
    mo->z += mo->momz;
    if( mo->z <= mo->floorz )
    {
        // Hit floor, check for collision and bounce.
        mo->z = mo->floorz;
        if( mo->momz < 0 )
        {
            // Hit floor with momentum, handle bounce conditions.
            mo->momz = -mo->momz;  // bounce off floor
            if( !(mo->flags & MF_NOGRAVITY) )
            {
                if (mo->eflags & MF_UNDERWATER)  // Legacy
                {
                    apply_gravity(mo);
                }
                else
                {
                    // MBF gravity for floor bouncers.
                    // Subject to gravity, bounce with decay.
                    mo->momz = FixedMul( mo->momz,
                        // Floaters fall slowly.
                        (mo->flags & MF_FLOAT)?   (fixed_t)(FRACUNIT*0.85)
                        // Dropoff modifies rate.
                       :(mo->flags & MF_DROPOFF)? (fixed_t)(FRACUNIT*0.70)
                       :                          (fixed_t)(FRACUNIT*0.45)
                    );
                }
                // Detect when below rest speed.
                if(abs(mo->momz) < ( mo->info->mass * (GRAVITY1*4/256)))
                {
                    mo->momz = 0;
                }
            }
            // TOUCHY objects explode on impact
            if( mo->flags & MF_TOUCHY
                && (mo->eflags & MF_ARMED) && (mo->health > 0) )
            {
                // Explode on floor.
                P_DamageMobj( mo, NULL, NULL, mo->health );
                // That ought to kill it, but on EASY skill it won't.
                // PrBoom did return here.
                return;
            }
            goto check_sentient_floater;       
        }
    }
    else if( mo->z > mo->ceilingz - mo->height )
    {
        // Hit ceiling, check for collision and bounce.
        mo->z = mo->ceilingz - mo->height;
        if( mo->momz > 0 )
        {
            // Hit ceiling with momentum, handle bounce conditions.
            // Interact with ceiling type.
            if( mo->subsector->sector->ceilingpic != skyflatnum )
            {
                // Normal bounce off building ceiling
                mo->momz = -mo->momz;
            }
            else if( mo->flags & MF_MISSILE )
            {
                // Missiles do not bounce off skies.
                P_RemoveMobj(mo);
                // PrBoom did not return here, but should not use mo anymore.
                // EE has return, but gave it a demo test.
                // Cannot be SENTIENT.
                return;
            }
            else if( mo->flags & MF_NOGRAVITY )
            {
                // Bounce off sky when gravity.
                mo->momz = -mo->momz;
            }
#if 1
            // From EternityEngine, without demo version test, level gravity.
            // haleyjd 05/22/03: kludge for bouncers sticking to sky --
            // if momz is still positive, the thing is still trying
            // to go up, which it should not be doing, so apply some
            // gravity to get it out of this code segment gradually
            if( /*demo_version >= 331 && */ mo->momz > 0)
            {
               mo->momz -= mo->info->mass * (GRAVITY1/256);
            }
#endif
            goto check_sentient_floater;       
        }
    }
    else
    {
        if( !(mo->flags & MF_NOGRAVITY ) )
        {
            // Gravity, let it fall.
            // Strange MBF bouncer gravity varies according to mass.
            // (mass * GRAVITY1/256)
            mo->momz -= mo->info->mass * (cv_gravity.value/(NEWTICRATERATIO*256));
        }
        goto check_sentient_floater;
    }
    
    // It came to a stop
    mo->momz = 0;

    if( mo->flags & MF_MISSILE )
    {
        // A BOUNCER MISSILE
        if( tmr_ceilingline
            && tmr_ceilingline->backsector
            && tmr_ceilingline->backsector->ceilingpic == skyflatnum
            && mo->z > tmr_ceilingline->backsector->ceilingheight )
        {
            // Sky, do not explode.
            P_RemoveMobj(mo);
        }
        else
        {
            P_ExplodeMissile(mo);
        }
        // PrBoom did not return here, but should not use mo anymore.
        // Even if you believe in SENTIENT MISSILE, it is gone.
        return;
    }

check_sentient_floater:
    // Give floating monsters a chance to move toward player.
    if( (mo->flags & MF_FLOAT) && SENTIENT(mo) )
        goto zmove_floater;  // re-enter ZMove to check floater

    return;
}

//
// P_NightmareRespawn
// Monsters
//
void P_NightmareRespawn(mobj_t * mobj)
{
    uint32_t old_flags;
    fixed_t  old_height;
    fixed_t x,y,z;
    fixed_t fog_height;
    mobj_t *fog_mo;
    mobj_t *mo;  // new mobj for respawn
    mapthing_t * mthing = mobj->spawnpoint;
    byte  at_mobj_position = 0;

#if 1
    // [WDJ] Teleport from corpse position, which may be on 3d-floor.
    z = mobj->z;
#else
    // From Boom
    z = mobj->subsector->sector->floorheight;  // for teleport from
#endif
    
    // Hurdler: respawn FS spawned mobj at their last position (they have no mapthing)
    // [WDJ] Since 2002/9/7, FS spawn have a extra mapthing as spawnpoint
    if (!mthing)
    {
        // Also fixes Nightmare respawn at (0,0) bug, as in Eternity Engine.
        x = mobj->x;
        y = mobj->y;
        at_mobj_position = 1;
    }
    else
    {
        x = mthing->x << FRACBITS;
        y = mthing->y << FRACBITS;

        // [WDJ] The above fix will not work when respawn have mthing
        // Nightmare respawn at (0,0) bug fix, as in PrBoom, Eternity
        if( EN_catch_respawn_0 && x==0 && y==0 )
        {
            x = mobj->x;
            y = mobj->y;
        }

        if(mthing->options & MTF_FS_SPAWNED)
        {
            at_mobj_position = 2;
            z = mobj->z;  // FS spawn
        }
    }

    // [WDJ] Modify mobj for position test.
    // Must consider solid corpses that modify the mobj.
    old_flags = mobj->flags;
    old_height = mobj->height;
    mobj->flags |= MF_SOLID;  // [WDJ] must be solid to check position (Boom bug)
    mobj->height = mobj->info->height;  // will have to fit this height
    // somthing is occupying it's position?
    if (!P_CheckPosition(mobj, x, y))  goto no_respawn;

    // spawn a teleport fog at old spot
    // because of removal of the body?
    fog_height = (EN_heretic)? TELEFOGHEIGHT : 0;
    mo = P_SpawnMobj(mobj->x, mobj->y, z + fog_height, MT_TFOG);
    // initiate teleport sound
    S_StartObjSound(mo, sfx_telept);

    // Spawn a teleport fog at the new spot.
    // [WDJ] It should be over the new object, but cannot move this spawn
    // after the object spawn because they may use P_Random,
    // and that would affect demos.
#if 1
    // The fog z will be moved later to be over the monster.
    fog_mo = P_SpawnMobj(x, y, 0, MT_TFOG);
#else
    // Spawn the fog on the floor.
    subsector_t * ss = R_PointInSubsector(x, y);

    fog_mo = P_SpawnMobj(x, y, ss->sector->floorheight + fog_height, MT_TFOG);
#endif

    // spawn it
    if (mobj->info->flags & MF_SPAWNCEILING)
        z = ONCEILINGZ;
    else if( at_mobj_position == 2 )
        z = mobj->z;  // FS spawn
    else
        z = ONFLOORZ;

    // inherit attributes from deceased one
    mo = P_SpawnMobj(x, y, z, mobj->type);
    mo->spawnpoint = mthing;
    // [WDJ] clean up mthing handling
    if (mthing)
    {
        if (mthing->options & (MTF_FS_SPAWNED|MTF_EXTRA))
            mobj->spawnpoint = NULL;  // prevent free of extra mapthing
        mthing->mobj = mo;  // [WDJ] replace ref to old monster (missing in PrBoom)
    }

    if ( at_mobj_position )
        mo->angle = mobj->angle;
    else
        mo->angle = wad_to_angle(mthing->angle);

    // [WDJ] since 2002/9/7, even FS spawned mobj have loose mapthing
    if ( at_mobj_position )
//        mo->flags |= mobj->flags & (MTF_AMBUSH ? MF_AMBUSH : 0);
        mo->flags |= mobj->flags & MF_AMBUSH;
    else if (mthing->options & MTF_AMBUSH)
        mo->flags |= MF_AMBUSH;

    // [WDJ] From MBF
    // killough 11/98: transfer friendliness from deceased
    mo->flags = (mo->flags & ~MF_FRIEND) | (mobj->flags & MF_FRIEND);

    mo->reactiontime = 18;

    // [WDJ] Move fog z over the monster.
    fog_mo->z = mo->z + fog_height;
    S_StartObjSound(fog_mo, sfx_telept);  // fog sound

    // remove the old monster,
    P_RemoveMobj(mobj);  // does Z_Free
    return;

no_respawn:
    // put it back the way it was.
    mobj->flags = old_flags;
    mobj->height = old_height;
    return;
}


consvar_t cv_respawnmonsters = { "respawnmonsters", "0", CV_NETVAR, CV_OnOff };
consvar_t cv_respawnmonsterstime = { "respawnmonsterstime", "12", CV_NETVAR|CV_VALUE, CV_Unsigned };

//
// P_MobjCheckWater : check for water, set stuff in mobj_t struct for
//                    movement code later, this is called either by P_MobjThinker() or P_PlayerThink()
void P_MobjCheckWater(mobj_t * mobj)
{
    sector_t *sector;
    int oldeflags;
    fixed_t mo_top = mobj->z + mobj->info->height;
    fixed_t mo_half = mobj->z + (mobj->info->height>>1);
    fixed_t z;

    if( (EV_legacy < 128)
        || mobj->type == MT_SPLASH || mobj->type == MT_SPIRIT)        // splash don't do splash
        return;
    //
    // see if we are in water, and set some flags for later
    //
    sector = mobj->subsector->sector;
    z = sector->floorheight;
    oldeflags = mobj->eflags;

    //SoM: 3/28/2000: Only use 280 water type of water. Some boom levels get messed up.
    if ((sector->model == SM_Legacy_water)
        || (sector->floortype == FLOOR_WATER && sector->modelsec == -1))
    {
        if (sector->model == SM_Legacy_water)     // special sector water
            z = (sectors[sector->modelsec].floorheight);
        else
            z = sector->floorheight + (FRACUNIT / 4);   // water texture

        if (mobj->z <= z && mo_top > z)
            mobj->eflags |= MF_TOUCHWATER;
        else
            mobj->eflags &= ~MF_TOUCHWATER;

        if (mo_half <= z)
            mobj->eflags |= MF_UNDERWATER;
        else
            mobj->eflags &= ~MF_UNDERWATER;
    }
    else if (sector->ffloors)
    {
        ffloor_t *rover;

        mobj->eflags &= ~(MF_UNDERWATER | MF_TOUCHWATER);

        for (rover = sector->ffloors; rover; rover = rover->next)
        {
            if (!(rover->flags & FF_SWIMMABLE) || rover->flags & FF_SOLID)
                continue;
            if (*rover->topheight <= mobj->z || *rover->bottomheight > mo_half)
                continue;

            if (mo_top > *rover->topheight)
                mobj->eflags |= MF_TOUCHWATER;
            else
                mobj->eflags &= ~MF_TOUCHWATER;

            if (mo_half < *rover->topheight)
                mobj->eflags |= MF_UNDERWATER;
            else
                mobj->eflags &= ~MF_UNDERWATER;

            if (EN_doom_etc
                && !(oldeflags & (MF_TOUCHWATER | MF_UNDERWATER))
                && ((mobj->eflags & MF_TOUCHWATER)
                    || (mobj->eflags & MF_UNDERWATER))
                && mobj->type != MT_BLOOD
                && mobj->type != MT_SMOK
               )
                P_SpawnSplash(mobj, *rover->topheight);
        }
        return;
    }
    else
    {
        mobj->eflags &= ~(MF_UNDERWATER | MF_TOUCHWATER);
    }

#if 0
    // [WDJ] From EternityEngine, missile splash.
    if( (mobj->flags & (MF_MISSILE | MF_BOUNCES) )
    {
        // Any time a missile or bouncer crosses, splash.
        if( oldeflags != mobj->eflags )
            P_SpawnSplash(mobj, z);
    }
#endif

/*
    if( (mobj->eflags ^ oldeflags) & MF_TOUCHWATER)
        CONS_Printf("touchwater %d\n",mobj->eflags & MF_TOUCHWATER ? 1 : 0);
    if( (mobj->eflags ^ oldeflags) & MF_UNDERWATER)
        CONS_Printf("underwater %d\n",mobj->eflags & MF_UNDERWATER ? 1 : 0);
*/
    // blood doesnt make noise when it falls in water
    if( !(oldeflags & (MF_TOUCHWATER | MF_UNDERWATER))
        && ((mobj->eflags & (MF_TOUCHWATER | MF_UNDERWATER)) )
        && mobj->type != MT_BLOOD
        && (EV_legacy < 132) )
        P_SpawnSplash(mobj, z); //SoM: 3/17/2000
}

//===========================================================================
//
// PlayerLandedOnThing
//
//===========================================================================
static void PlayerLandedOnThing(mobj_t * mo, mobj_t * onmobj)
{
    mo->player->deltaviewheight = mo->momz >> 3;
    if (mo->momz < (-23 * GRAVITY1))
    {
        //P_FallingDamage(mo->player);
        P_NoiseAlert(mo, mo);
    }
    else if( (mo->momz < (-8 * GRAVITY1)) && !mo->player->chickenTics)
    {
        S_StartObjSound(mo, sfx_oof);
    }
}

//
// P_MobjThinker
//
void P_MobjThinker(mobj_t * mobj)
{
    boolean checkedpos = false; //added:22-02-98:
    player_t * player = mobj->player;

    // check mobj against possible water content, before movement code
    P_MobjCheckWater(mobj);

    //
    // momentum movement
    //
#ifdef CLIENTPREDICTION2
    // move player mobj (not the spirit) to spirit position (sent by ticcmd)
    if ((mobj->type == MT_PLAYER)
        && (player)
        && ((player->cmd.angleturn & (TICCMD_XY | TICCMD_RECEIVED)) == (TICCMD_XY | TICCMD_RECEIVED)) && (mobj->player->playerstate == PST_LIVE)
        && (EV_legacy > 130) )
    {
        int oldx = mobj->x, oldy = mobj->y;

        if (oldx != player->cmd.x || oldy != player->cmd.y)
        {
            mobj->eflags |= MF_NOZCHECKING;
            // cross special lines and pick up things
            if (!P_TryMove(mobj, player->cmd.x, player->cmd.y, true))
            {
                // P_TryMove fail mean cannot change mobj position to requested position
                // the mobj is blocked by something
                if (player == consoleplayer_ptr)
                {
                    // reset spirit position
                    CL_ResetSpiritPosition(mobj);

                    if(verbose > 1)
                        GenPrintf(EMSG_ver, "\2MissPrediction\n");
                }
            }
            mobj->eflags &= ~MF_NOZCHECKING;
        }
        P_XYFriction(mobj, oldx, oldy);  // thing friction
    }
    else
#endif
    if (mobj->momx || mobj->momy || (mobj->flags & MF_SKULLFLY))
    {
        P_XYMovement(mobj);
        checkedpos = true;

        if (mobj->thinker.function.acp1 == (actionf_p1) T_RemoveThinker)
            goto done;     // mobj was removed
    }
    if (mobj->flags2 & MF2_FLOATBOB)
    {   // Floating item bobbing motion
        mobj->z = mobj->floorz + FloatBobOffsets[(mobj->health++) & 63];
    }
    else
        //added:28-02-98: always do the gravity bit now, that's simpler
        //                BUT CheckPosition only if wasn't do before.
    if( (mobj->eflags & MF_ONGROUND) == 0
        || (mobj->z != mobj->floorz) || mobj->momz)
    {
        // BP: since version 1.31 we use heretic z-checking code
        //     kept old code for backward demo compatibility
        if( EV_legacy < 131 )
        {

            // if didnt check things Z while XYMovement, do the necessary now
            if( !checkedpos && (EV_legacy >= 112) )
            {
                // FIXME : should check only with things, not lines
                P_CheckPosition(mobj, mobj->x, mobj->y);

                // tmr_floorz, tmr_ceilingz, tmr_floorthing returned by P_CheckPosition
                mobj->floorz = tmr_floorz;
                mobj->ceilingz = tmr_ceilingz;
                if (tmr_floorthing)
                    mobj->eflags &= ~MF_ONGROUND;       //not on real floor
                else
                    mobj->eflags |= MF_ONGROUND;

                // now mobj->floorz should be the current sector's z floor
                // or a valid thing's top z
            }

            P_ZMovement(mobj);
        }
        else if (mobj->flags2 & MF2_PASSMOBJ)
        {
            mobj_t *onmo;
            onmo = P_CheckOnmobj(mobj);
            if (!onmo)
            {
                P_ZMovement(mobj);
                if (player && mobj->flags & MF2_ONMOBJ)
                    mobj->flags2 &= ~MF2_ONMOBJ;
            }
            else
            {
                if (player && (player->mo == mobj))
                {
                    // Player, not a voodoo doll
                    if( (mobj->momz < (-8 * GRAVITY1))
                        && !(mobj->flags2 & MF2_FLY))
                    {
                        PlayerLandedOnThing(mobj, onmo);
                    }

                    if (onmo->z + onmo->height - mobj->z <= 24 * FRACUNIT)
                    {
                        player->viewheight -= onmo->z + onmo->height - mobj->z;
                        player->deltaviewheight = (VIEWHEIGHT - player->viewheight) >> 3;
                        mobj->z = onmo->z + onmo->height;
                        mobj->flags2 |= MF2_ONMOBJ;
                        mobj->momz = 0;
                    }
                    else
                    {   // hit the bottom of the blocking mobj
                        mobj->momz = 0;
                    }
                }
            }
        }
        else
            P_ZMovement(mobj);

        if (mobj->thinker.function.acp1 == (actionf_p1) T_RemoveThinker)
            goto done;     // mobj was removed
    }
    else
    {
        mobj->eflags &= ~MF_JUSTHITFLOOR;

        // MBF
        if( ((mobj->momx | mobj->momy) == 0) && !SENTIENT(mobj) )
        {
            // At rest
            mobj->eflags |= MF_ARMED;  // arm a mine which has come to rest.

            // killough 9/12/98: objects fall off ledges if they are hanging off
            // slightly push off of ledge if hanging more than halfway off
            if( cv_mbf_falloff.EV
                && (mobj->z > mobj->dropoffz)  // Only objects contacting dropoff
                && !(mobj->flags & MF_NOGRAVITY)  // Only objects which fall
              )
            {
                P_ApplyTorque(mobj);               // Apply torque
            }
            else
            {
                mobj->eflags &= ~MF_FALLING;
                mobj->tipcount = 0;  // Reset torque
            }
        }
    }


    // SoM: Floorhuggers stay on the floor always...
    // BP: tested here but never set ?!
    if (mobj->info->flags2 & MF2_FLOORHUGGER)
    {
        mobj->z = mobj->floorz;
    }

    // cycle through states,
    // calling action functions at transitions
    if (mobj->tics != -1)
    {
        mobj->tics--;

        // you can cycle through multiple states in a tic
        if (!mobj->tics)
        {
            // [WDJ] This would segfault if mobj had been removed.
            if( mobj->state == &states[S_NULL] )
               goto done;
            if (!P_SetMobjState(mobj, mobj->state->nextstate))
               goto done; // freed itself
        }
    }
    else
    {
        // check for nightmare respawn
        if( !cv_respawnmonsters.EV )
            goto done;

        if (!(mobj->flags & MF_COUNTKILL))
            goto done;

        mobj->movecount++;

        if (mobj->movecount < cv_respawnmonsterstime.value * TICRATE)
            goto done;

        if (leveltime % (32 * NEWTICRATERATIO))
            goto done;

        if( PP_Random(pr_respawn) > 4 )
            goto done;

        P_NightmareRespawn(mobj);
    }

done:
    return;
}

void P_MobjNullThinker(mobj_t * mobj)
{
}

//
// P_SpawnMobj
//
// Does not set mapthing
mobj_t * P_SpawnMobj(fixed_t x, fixed_t y, fixed_t z, mobjtype_t type)
{
    mobj_t *mobj;
    state_t *st;
    mobjinfo_t *info;

    mobj = Z_Malloc(sizeof(*mobj), PU_LEVEL, NULL);
    memset(mobj, 0, sizeof(*mobj));
    info = &mobjinfo[type];

    mobj->type = type;
    mobj->info = info;
    mobj->x = x;
    mobj->y = y;
    mobj->radius = info->radius;
    mobj->height = info->height;
    mobj->flags = info->flags;
    mobj->flags2 = info->flags2;
    mobj->tflags = info->tflags;

    // MBF from PrBoom.
    // killough 8/23/98: no friends, bouncers, or touchy things in old demos.
    if(! EN_mbf )    // Vanilla or Boom demo
    {
        mobj->flags &= ~(MF_BOUNCES | MF_FRIEND | MF_TOUCHY);
    }
    // EternityEngine: new EE demo has different method.
    else if((demoversion >= 147) && (cv_deathmatch.EV == 0))
    {
        // DoomLegacy >= 147, or MBF or Boom demo, and not deathmatch
        // Boom demo: 201, 202
        // MBF demo: 203
        // Players are always friends.
        if(type == MT_PLAYER)
        {
            // MBF player in Old MBF demo.
            mobj->flags |= MF_FRIEND;
        }
    }

    mobj->health = info->spawnhealth;

    if (gameskill != sk_nightmare)
        mobj->reactiontime = info->reactiontime;

    if( (EV_legacy >= 129) || (mobj->type == MT_CHASECAM) )
        mobj->lastlook = -1;    // stuff moved in P_enemy.P_LookForPlayer
    else
        mobj->lastlook = PP_Random(pr_lastlook) % MAXPLAYERS;  // Boom, MBF

    // do not set the state with P_SetMobjState,
    // because action routines can not be called yet
    st = &states[info->spawnstate];

    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;    // FF_FRAMEMASK for frame, and other bits..
    mobj->touching_sectorlist = NULL;   //SoM: 4/7/2000
    mobj->friction = ORIG_FRICTION;     //SoM: 4/7/2000

    // BP: SoM right ? if not ajust in p_saveg line 625 and 979
    mobj->movefactor = ORIG_FRICTION_FACTOR;

    mobj->target = NULL;
    mobj->tracer = NULL;

    // set subsector and/or block links
    P_SetThingPosition(mobj);

    mobj->floorz = mobj->subsector->sector->floorheight;
    mobj->ceilingz = mobj->subsector->sector->ceilingheight;

    //added:27-02-98: if ONFLOORZ, stack the things one on another
    //                so they do not occupy the same 3d space
    //                allow for some funny thing arrangements!
    if (z == ONFLOORZ)
    {
        //if (!P_CheckPosition(mobj,x,y))
        // we could send a message to the console here, saying
        // "no place for spawned thing"...

        //added:28-02-98: defaults onground
        mobj->eflags |= MF_ONGROUND;

        //added:28-02-98: dirty hack : dont stack monsters coz it blocks
        //                moving floors and anyway whats the use of it?
        /*
           if (mobj->flags & MF_NOBLOOD)
           {
           mobj->z = mobj->floorz;

           // first check the tmfloorz
           P_CheckPosition(mobj,x,y);
           mobj->z = tmfloorz+FRACUNIT;

           // second check at the good z pos
           P_CheckPosition(mobj,x,y);

           mobj->floorz = tmfloorz;
           mobj->ceilingz = tmsectorceilingz;
           mobj->z = tmfloorz;
           // thing not on solid ground
           if (tmfloorthing)
           mobj->eflags &= ~MF_ONGROUND;

           //if (mobj->type == MT_BARREL)
           //   debug_Printf("barrel at z %d floor %d ceiling %d\n",mobj->z,mobj->floorz,mobj->ceilingz);

           }
           else
         */
        mobj->z = mobj->floorz;

    }
    else if (z == ONCEILINGZ)
        mobj->z = mobj->ceilingz - mobj->info->height;
    else if (z == FLOATRANDZ)
    {
        fixed_t space = ((mobj->ceilingz) - (mobj->height)) - mobj->floorz;
        if (space > 48 * FRACUNIT)
        {
            space -= 40 * FRACUNIT;
            mobj->z = ((space * P_Random()) >> 8) + mobj->floorz + 40 * FRACUNIT;
        }
        else
            mobj->z = mobj->floorz;
    }
    else
    {
        //debug_Printf("P_SpawnMobj: mobj spawned at z %d\n",z>>16);
        mobj->z = z;
    }

    // [WDJ] Spawnpoint set by last Spawn (monsters and objects).
    if (mobj->spawnpoint)
    {
        if ((mobj->spawnpoint->options >> 7) != 0 && !mobj->spawnpoint->z)
        {
            if (z == ONFLOORZ)
                mobj->z = R_PointInSubsector(x, y)->sector->floorheight + ((mobj->spawnpoint->options >> 7) << FRACBITS);
            else if (z == ONCEILINGZ)
                mobj->z = R_PointInSubsector(x, y)->sector->ceilingheight - (((mobj->spawnpoint->options >> 7) << FRACBITS) - mobj->height);
        }
        else if (mobj->spawnpoint->z)
            mobj->z = mobj->spawnpoint->z << FRACBITS;
    }

    if (EN_heretic
        && mobj->flags2 & MF2_FOOTCLIP
        && P_GetThingFloorType(mobj) != FLOOR_SOLID
        && mobj->floorz == mobj->subsector->sector->floorheight
       )
        mobj->flags2 |= MF2_FEETARECLIPPED;
    else
        mobj->flags2 &= ~MF2_FEETARECLIPPED;

    // added 16-6-98: special hack for spirit
    if (mobj->type == MT_SPIRIT)
        mobj->thinker.function.acp1 = (actionf_p1) P_MobjNullThinker;
    else
    {
        mobj->thinker.function.acp1 = (actionf_p1) P_MobjThinker;
        P_AddThinker(&mobj->thinker);
    }

    if (mobj->spawnpoint)
        mobj->spawnpoint->z = mobj->z >> FRACBITS;

    return mobj;
}


// [WDJ] Changes the mobj type.
// Does not set mapthing
// Returns false if fails the test.
boolean P_MorphMobj( mobj_t * mo, mobjtype_t type, int mmflags, int keepflags )
{
    mobjtype_t  current_type = mo->type;
    uint32_t old_flags = mo->flags;  // to restore
    state_t *st;
    mobjinfo_t *info;

    info = &mobjinfo[type];
    mo->height = info->height;
    mo->radius = info->radius;
    mo->flags = info->flags;

    if( mmflags & MM_testsize )
    {
        // Test if the new size fits in current location.
        // Requires x,y,z, floorz, ceilingz, 
        // height, radius, flags (for pickup, noclip, solid)
        if( ! P_TestMobjLocation(mo) )
        {
            // does not fit, restore any changed fields
            info = &mobjinfo[current_type];
            mo->height = info->height;
            mo->radius = info->radius;
            mo->flags = info->flags;
            return false;
        }
    }
       
    // commit by updating all the fields
    mo->special2 = current_type; // save type for restore
    mo->type = type;
    mo->info = info;
    mo->flags |= old_flags & keepflags;  // keep some flags
    mo->flags2 = info->flags2;
    mo->health = info->spawnhealth;
   
    // Other things that SpawnMobj did that may be relevant
    mo->reactiontime = (gameskill != sk_nightmare)? info->reactiontime : 0;

    if( ((EV_legacy < 129) && mo->type != MT_CHASECAM )
//       || EN_heretic  // if played heretic demo this would be important
        )
    {
        // Heretic use of P_Random
        mo->lastlook = P_Random() % MAXPLAYERS;
    }
    else
        mo->lastlook = -1;    // stuff moved in P_enemy.P_LookForPlayer

    // because action routines can not be called yet
    st = &states[info->spawnstate];

    mo->state = st;
    mo->tics = st->tics;
    mo->sprite = st->sprite;
    mo->frame = st->frame;    // FF_FRAMEMASK for frame, and other bits..

    // MorphMobj should only be callable from Heretic, used for chicken.
    if ( EN_heretic
        && mo->flags2 & MF2_FOOTCLIP
        && P_GetThingFloorType(mo) != FLOOR_SOLID
        && mo->floorz == mo->subsector->sector->floorheight
       )
        mo->flags2 |= MF2_FEETARECLIPPED;
    else
        mo->flags2 &= ~MF2_FEETARECLIPPED;

    if (mo->type == MT_SPIRIT)
        mo->thinker.function.acp1 = (actionf_p1) P_MobjNullThinker;
    else
    {
        mo->thinker.function.acp1 = (actionf_p1) P_MobjThinker;
        if( mo->thinker.next == NULL )  // Not currently linked into thinker
            P_AddThinker(&mo->thinker);
    }
   
    if( mmflags & MM_telefog )
    { 
        mobj_t * fog = P_SpawnMobj(mo->x, mo->y, mo->z+TELEFOGHEIGHT, MT_TFOG);
        S_StartObjSound(fog, sfx_telept);
    }

    return true;
}

//
// P_RemoveMobj
//
mapthing_t *itemrespawnque[ITEMQUESIZE];
tic_t itemrespawntime[ITEMQUESIZE];
int iquehead;
int iquetail;

void P_RemoveMobj(mobj_t * mobj)
{
    // Do not respawn: missiles, fire, cube monsters
    // Respawn: weapons, ammo, health, armor, powerups
    if ( mobj->spawnpoint  // [WDJ] no more respawn without mapthing
        && (mobj->flags & MF_SPECIAL) && !(mobj->flags & MF_DROPPED)
        && (mobj->type != MT_INV) && (mobj->type != MT_INS) )
    {
        // Respawn
        itemrespawnque[iquehead] = mobj->spawnpoint;  // mapthing or Extra_MapThing
        itemrespawntime[iquehead] = leveltime;
        iquehead = (iquehead + 1) & (ITEMQUESIZE - 1);

        // lose one off the end?
        if (iquehead == iquetail)
            iquetail = (iquetail + 1) & (ITEMQUESIZE - 1);
    }
    else if ( mobj->spawnpoint
              && mobj->spawnpoint->options & (MTF_FS_SPAWNED|MTF_EXTRA))
    {
        // extra mapthing
        P_Free_Extra_Mapthing( mobj->spawnpoint );
        mobj->spawnpoint = NULL;
    }

    // unlink from sector and block lists
    P_UnsetThingPosition(mobj);

    //SoM: 4/7/2000: Remove touching_sectorlist from mobj.
    if (sector_list)
    {
        P_DelSeclist(sector_list);
        sector_list = NULL;
    }

    // stop any playing sound
    S_StopObjSound(mobj);

#ifdef REFERENCE_COUNTING
 // [WDJ] From PrBoom.
 // DoomLegacy is not implementing reference counting, so this does nothing.
 // killough 11/98:
 //
 // Remove any references to other mobjs.
 //
 // Older demos might depend on the fields being left alone, however,
 // if multiple thinkers reference each other indirectly before the
 // end of the current tic.
    P_SetReference(mobj->target,    NULL);
    P_SetReference(mobj->tracer,    NULL);
    P_SetReference(mobj->lastenemy, NULL);
#endif

    // free block
    P_RemoveThinker((thinker_t *) mobj);   // does Z_Free() mobj
}

consvar_t cv_itemrespawntime = { "respawnitemtime", "30", CV_NETVAR|CV_VALUE, CV_Unsigned };
consvar_t cv_itemrespawn = { "respawnitem", "0", CV_NETVAR, CV_OnOff };

//
// P_RespawnSpecials
//
void P_RespawnSpecials(void)
{
    fixed_t x, y, z;
    mobj_t *mo;
    mobj_t *fog_mo = NULL;
    mapthing_t *mthing;

    int i;

    // only respawn items in deathmatch
    if( !cv_itemrespawn.EV )
        return; //

    // nothing left to respawn?
    if (iquehead == iquetail)
        return;

    // the first item in the queue is the first to respawn
    // wait at least 30 seconds
    if (leveltime - itemrespawntime[iquetail] < (tic_t) cv_itemrespawntime.value * TICRATE)
        return;

    mthing = itemrespawnque[iquetail];

    if (!mthing)
    {
        // [WDJ] No NULL in itemrespawnque, should no longer happen
        GenPrintf(EMSG_warn, "Warning: NULL respawn ptr.\n");
        // pull it from the que
        iquetail = (iquetail + 1) & (ITEMQUESIZE - 1);
        return;
    }

    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;

    if (EN_doom_etc)
    {
        // Spawn a teleport fog at the new spot.
        // Will move later to correct z, so that it is over the object,
        // even if it is on 3d-floor.
#if 1
        // Spawn does not check z.
        fog_mo = P_SpawnMobj(x, y, 0, MT_IFOG);  // teleport fog
#else
        if (mthing->options & MTF_FS_SPAWNED)
        {
            z = mthing->z << FRACBITS;
        }
        else
        {
            // [WDJ] at floor height, even when mobj spawns on the ceiling.
            // Actual height is too difficult and too late (like other ports).
            subsector_t * ss = R_PointInSubsector(x, y);
            z = ss->sector->floorheight;
        }
        fog_mo = P_SpawnMobj(x, y, z, MT_IFOG);  // teleport fog
#endif
    }

    // find which type to spawn
    for (i = 0; i < NUMMOBJTYPES; i++)
        if (mthing->type == mobjinfo[i].doomednum)
            break;

    // spawn it
    if (mobjinfo[i].flags & MF_SPAWNCEILING)
        z = ONCEILINGZ;
    else if(mthing->options & MTF_FS_SPAWNED)
        z = mthing->z << FRACBITS;	//DarkWolf95:This still wasn't fixed?! Keep Z for FS stuff.
    else
        z = ONFLOORZ;

    mo = P_SpawnMobj(x, y, z, i);
    mo->spawnpoint = mthing;
    mthing->mobj = mo;  // [WDJ] replace ref to old mobj (missing in PrBoom)
    mo->angle = wad_to_angle(mthing->angle);

    if (EN_heretic)
        S_StartObjSound(mo, sfx_itmbk);
    else
    {
        // Move fog z over the new object.
        fog_mo->z = mo->z;
        S_StartObjSound(fog_mo, sfx_itmbk);
    }

    // pull it from the que
    iquetail = (iquetail + 1) & (ITEMQUESIZE - 1);
}

// used when we are going from deathmatch 2 to deathmatch 1
void P_RespawnWeapons(void)
{
    fixed_t x;
    fixed_t y;
    fixed_t z;

    subsector_t *ss;
    mobj_t *mo;
    mapthing_t *mthing;

    int i, j, freeslot;

    freeslot = iquetail;
    for (j = iquetail; j != iquehead; j = (j + 1) & (ITEMQUESIZE - 1))
    {
        mthing = itemrespawnque[j];

        i = 0;
        switch (mthing->type)
        {
            case 2001: //mobjinfo[MT_SHOTGUN].doomednum  :
                i = MT_SHOTGUN;
                break;
            case 82:   //mobjinfo[MT_SUPERSHOTGUN].doomednum :
                i = MT_SUPERSHOTGUN;
                break;
            case 2002: //mobjinfo[MT_CHAINGUN].doomednum :
                i = MT_CHAINGUN;
                break;
            case 2006: //mobjinfo[MT_BFG9000].doomednum   : // bfg9000
                i = MT_BFG9000;
                break;
            case 2004: //mobjinfo[MT_PLASMAGUN].doomednum   : // plasma launcher
                i = MT_PLASMAGUN;
                break;
            case 2003: //mobjinfo[MT_ROCKETLAUNCH].doomednum   : // rocket launcher
                i = MT_ROCKETLAUNCH;
                break;
            case 2005: //mobjinfo[MT_SHAINSAW].doomednum   : // shainsaw
                i = MT_SHAINSAW;
                break;
            default:
                if (freeslot != j)
                {
                    itemrespawnque[freeslot] = itemrespawnque[j];
                    itemrespawntime[freeslot] = itemrespawntime[j];
                }

                freeslot = (freeslot + 1) & (ITEMQUESIZE - 1);
                continue;
        }
        // respwan it
        x = mthing->x << FRACBITS;
        y = mthing->y << FRACBITS;

        // spawn a teleport fog at the new spot
        ss = R_PointInSubsector(x, y);
        if(mthing->options & MTF_FS_SPAWNED)
            mo = P_SpawnMobj(x, y, mthing->z << FRACBITS, MT_IFOG);
        else
            mo = P_SpawnMobj(x, y, ss->sector->floorheight, MT_IFOG);
        S_StartObjSound(mo, sfx_itmbk);

        // spawn it
        if (mobjinfo[i].flags & MF_SPAWNCEILING)
            z = ONCEILINGZ;
        else if(mthing->options & MTF_FS_SPAWNED)
            z = mthing->z << FRACBITS;
        else
            z = ONFLOORZ;

        mo = P_SpawnMobj(x, y, z, i);
        mo->spawnpoint = mthing;
        mthing->mobj = mo;  // [WDJ] replace ref to old mobj (missing in PrBoom)
        mo->angle = wad_to_angle(mthing->angle);
        // here don't increment freeslot
    }
    iquehead = freeslot;
}

extern byte weapontobutton[NUMWEAPONS];

//
// P_SpawnPlayer
// Called when a player is spawned on the level.
// Most of the player structure stays unchanged
//  between levels.
//
// BP: spawn it at a playerspawn mapthing, [WDJ] as playernum
void P_SpawnPlayer( mapthing_t * mthing, int playernum )
{
    player_t *p;
    fixed_t x, y, z;
    mobj_t *mobj;

//    [WDJ] 2/8/2011 relied on mangled type field, prevented re-searching for voodoo
//    int playernum = mthing->type - 1;

    // not playing?
    if (!playeringame[playernum])
        return;

#ifdef PARANOIA
    if (playernum < 0 && playernum >= MAXPLAYERS)
        I_Error("P_SpawnPlayer : bad playernum (%d)", playernum);
#endif

    p = &players[playernum];

    if (p->playerstate == PST_REBORN)
        G_PlayerReborn(playernum);

    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;

    z = ONFLOORZ;

    // [WDJ] If there is already an mobj for this player, then it is the
    // zombie of a player.  It can be made corpse or removed.
    // It cannot be reused because of all the fields that are setup by P_SpawnMobj.
    // Voodoo dolls are detected elsewhere.
    if( p->mo )  // player already has an mobj
    {
        if( voodoo_mode != VM_vanilla )
        {
            if( cv_solidcorpse.EV )
            {
                // convert to corpse
                p->mo->flags |= MF_CORPSE|MF_SOLID;
                p->mo->player = NULL;  // no voodoo, zombie
            }
            else
            {
                P_RemoveMobj( p->mo );
            }
        }
    }

    mobj = P_SpawnMobj(x, y, z, MT_PLAYER);
    //SoM:
    mthing->mobj = mobj;

    // set color translations for player sprites
    // added 6-2-98 : change color : now use skincolor (befor is mthing->type-1
    mobj->tflags |= (p->skincolor) << MFT_TRANSSHIFT;

    //
    // set 'spritedef' override in mobj for player skins.. (see ProjectSprite)
    // (usefulness : when body mobj is detached from player (who respawns),
    //  the dead body mobj retain the skin through the 'spritedef' override).
    mobj->skin = skins[p->skin];
    if( mobj->skin == NULL )  mobj->skin = skins[0];

    mobj->angle = wad_to_angle(mthing->angle);
    if (playernum == consoleplayer)
    {
        localangle = mobj->angle;
        localaiming = 0;
    }
    else if (playernum == displayplayer2)  // player 2
    {
        localangle2 = mobj->angle;
        localaiming2 = 0;
    }
    else if (p->bot)    //added by AC for acbot
    {
        B_SpawnBot(p->bot);
    }

    mobj->player = p;
    mobj->health = p->health;
   
    p->mo = mobj;
    p->playerstate = PST_LIVE;
    p->refire = 0;
    p->message = NULL;
    p->damagecount = 0;
    p->bonuscount = 0;
    p->chickenTics = 0;
    p->rain1 = NULL;
    p->rain2 = NULL;
    p->extralight = 0;
    p->fixedcolormap = 0;
    p->viewheight = ((unsigned int)cv_viewheight.EV) << FRACBITS;
    // added 2-12-98
    p->viewz = p->mo->z + p->viewheight;
    p->aiming = 0;  // reset freelook 

    p->flamecount = 0;
    p->flyheight = 0;
    p->bob_momx = 0;
    p->bob_momy = 0;

    // setup gun psprite
    P_SetupPsprites(p);

    // give all cards in death match mode
    if( cv_deathmatch.EV )
        p->cards = it_allkeys;

    if (playernum == consoleplayer)
    {
        // wake up the status bar
        ST_Start();
        // wake up the heads up text
        HU_Start();
    }

#ifdef CLIENTPREDICTION2
    if( EV_legacy > 132 )
    {
        //added 1-6-98 : for movement prediction
        if (p->spirit)
            CL_ResetSpiritPosition(mobj);       // reset spirit possition
        else
            p->spirit = P_SpawnMobj(x, y, z, MT_SPIRIT);

        p->spirit->skin = mobj->skin;
        p->spirit->angle = mobj->angle;
        p->spirit->player = mobj->player;
        p->spirit->health = mobj->health;
        p->spirit->movedir = weapontobutton[p->readyweapon];
        p->spirit->flags2 |= MF2_DONTDRAW;
    }
#endif
    // notify network
    SV_SpawnPlayer(playernum, mobj->x, mobj->y, mobj->angle);

    if (camera.chase == p)
        P_ResetCamera(p);

   if( ! ( voodoo_mode == VM_vanilla && cv_deathmatch.EV ) )
   {
       // [WDJ] Create any missing personal voodoo dolls for this player
       // But not the last spawnpoint, otherwise deathmatch gets extraneous voodoo dolls.
       int mtpn = (playernum < 4)? (playernum+1) : (playernum - 4 + 4001);
       int i;
       for (i=0 ; i<nummapthings ; i++)
       {
           mapthing_t* mt = &mapthings[i];
           if( mt->type == mtpn ) // a spawnpoint for this player
           {
               // if not the last playerstart, then spawn as voodoo doll
               if( mt != playerstarts[playernum] && mt->mobj == NULL )
               {
                   P_SpawnVoodoo( playernum, mt );
               }
           }
       }
   }
}

//
// P_SpawnMapthing
// The fields of the mapthing should
// already be in host byte order.
//


void P_SpawnMapthing (mapthing_t* mthing)
{
    int i;
    int bit;
    mobj_t *mobj;
    fixed_t x, y, z;

    if (!mthing->type)
        return; //SoM: 4/7/2000: Ignore type-0 things as NOPs

#if 1   
   // From PrBoom, MBF, EternityEngine, (so it must be a good thing).
   // killough 11/98: clear flags unused by Doom
   //
   // We clear the flags unused in Doom if we see flag mask 256 set, since
   // it is reserved to be 0 under the new scheme. A 1 in this reserved bit
   // indicates it's a Doom wad made by a Doom editor which puts 1's in
   // bits that weren't used in Doom (such as HellMaker wads). So we should
   // then simply ignore all upper bits.
   if( !EN_boom
       || ((demoversion >= 203) && mthing->options & MTF_RESERVED))
   {
      // Clear bits that would be unused.
      mthing->options &= MTF_EASY|MTF_NORMAL|MTF_HARD|MTF_AMBUSH|MTF_MPSPAWN;
   }
#endif

    // count deathmatch start positions
    if (mthing->type == 11)
    {
        if (numdmstarts < MAX_DM_STARTS)
        {
            deathmatchstarts[numdmstarts] = mthing;
            mthing->type = 0;
            numdmstarts++;
        }
        return;
    }

    // check for players specially
    // added 9-2-98 type 5 -> 8 player[x] starts for cooperative
    //              support ctfdoom cooperative playerstart
    //SoM: 4/7/2000: Fix crashing bug.
    if ((mthing->type > 0 && mthing->type <= 4) || (mthing->type >= 4001 && mthing->type <= 4028 ))
    {
       // [WDJ] 2/8/2011  Pass playernum to SpawnPlayer, instead of mangling the thing->type.
       // This means that we can search the mthing list later without errors.
       // Map player starts 1..4 to playernum 0..3
       // Map expanded player starts 4001..4028 to playernum 4..31
       int playernum = (mthing->type < 4000)? (mthing->type - 1) : (mthing->type - 4001 + 4);
       
        // [WDJ] Detect voodoo doll as multiple start points.
        // Extra player start points spawn voodoo dolls,
        // the last is the actual player start point.
        // For all playernum, seen in Plutonia MAP06.
        if( playerstarts[playernum] ) {
            // Spawn the previous player start point as a voodoo doll.
            // Such a voodoo doll can trip linedefs, but is not counted as
            // a monster nor as a player.
            P_SpawnVoodoo( playernum, playerstarts[playernum] );
        }

        // save spots for respawning in network games
        playerstarts[playernum] = mthing;

        // old version spawn player now, new version spawn player when level is 
        // loaded, or in network event later when player join game
        if( cv_deathmatch.EV == 0 && (EV_legacy < 128) )
        {
#ifdef DOGS
            // [WDJ] From MBF, PrBoom, EternityEngine
            // Kept for demo compatibility.
            // killough 7/19/98: Marine's best friend :)
            // Spawn a dog for player starts (2..), up to cv_mbf_dogs.
            // Avoid multiple dogs in case of multiple starts, using secretcount,
            // which the missing players apparantly do not need.
            // Playerstart will not be set.
            if( !netgame
                && playernum > 0 && playernum <= cv_mbf_dogs.EV
                && (players[playernum].secretcount == 0) )
            {
                players[playernum].secretcount = 1;
                // killough 10/98: force it to be a friend
                mthing->options |= MTF_FRIEND;
                // haleyjd 9/22/99: deh, bex substitution	       
                i = ( helper_MT < ENDDOOM_MT )? helper_MT : MT_DOG;
                goto spawnit;
            }
#endif
            P_SpawnPlayer(mthing, playernum);
        }

        return;
    }

    // Ambient sound sequences
    if (mthing->type >= 1200 && mthing->type < 1300)
    {
        P_AddAmbientSfx(mthing->type - 1200);
        return;
    }

    // Check for boss spots
    if (EN_heretic_hexen && (mthing->type == 56) )    // Monster_BossSpot
    {
#if 1
        // [WDJ] Gives same result as Heretic source.
        P_AddBossSpot(mthing->x << FRACBITS, mthing->y << FRACBITS, wad_to_angle(mthing->angle));
#else
        // [WDJ] ANGLE_1 has a round-off error.  I do not know what this was trying to accomplish.
        P_AddBossSpot(mthing->x << FRACBITS, mthing->y << FRACBITS, mthing->angle * ANGLE_1);   // SSNTails 06-10-2003
#endif
        return;
    }

    // check for apropriate skill level
    if (!multiplayer && (mthing->options & MTF_MPSPAWN))
         return;


    //SoM: 4/7/2000: Implement "not deathmatch" thing flag
    if( netgame && cv_deathmatch.EV && (mthing->options & MTF_NODM))
        return;

    //SoM: 4/7/2000: Implement "not cooperative" thing flag
    if( netgame && !cv_deathmatch.EV && (mthing->options & MTF_NOCOOP))
        return;

    if (gameskill == sk_baby)
        bit = 1;
    else if (gameskill == sk_nightmare)
        bit = 4;
    else
        bit = 1 << (gameskill - 1);

    if (!(mthing->options & bit))
        return;

    // find which type to spawn
    for (i = 0; i < NUMMOBJTYPES; i++)
        if (mthing->type == mobjinfo[i].doomednum)
            break;

    if (i == NUMMOBJTYPES)
    {
        I_SoftError("\2P_SpawnMapthing: Unknown type %i at (%i, %i)\n", mthing->type, mthing->x, mthing->y);
        return;
    }

    // don't spawn keycards and players in deathmatch
    if( cv_deathmatch.EV && (mobjinfo[i].flags & MF_NOTDMATCH) )
        return;

    // don't spawn any monsters if -nomonsters
    if (nomonsters && (i == MT_SKULL || (mobjinfo[i].flags & MF_COUNTKILL)))
    {
        return;
    }

    if (i == MT_WMACE)
    {
        P_AddMaceSpot(mthing);
        return;
    }

#ifdef DOGS
spawnit:
#endif
    // spawn it
    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;

    if (mobjinfo[i].flags & MF_SPAWNCEILING)
        z = ONCEILINGZ;
    else if (mobjinfo[i].flags2 & MF2_SPAWNFLOAT)
        z = FLOATRANDZ;
    else
        z = ONFLOORZ;

    mobj = P_SpawnMobj(x, y, z, i);
    mobj->spawnpoint = mthing;

    // Seed random starting index for bobbing motion
    if( EN_heretic && (mobj->flags2 & MF2_FLOATBOB) )
        mobj->health = P_Random();

    if (mobj->tics > 0)
        mobj->tics = 1 + (PP_Random(pr_spawnthing) % mobj->tics);

    if (EN_mbf
        && !(mobj->flags & MF_FRIEND)
        && (mthing->options & MTF_FRIEND)
        )
    {
        mobj->flags |= MF_FRIEND;  // killough 10/98:
        P_UpdateClassThink(&mobj->thinker, TH_friends); // transfer friendliness flag
    }

    if( (mobj->flags & MF_COUNTKILL) && !(mobj->flags & MF_FRIEND))
        totalkills++;

    if (mobj->flags & MF_COUNTITEM)
        totalitems++;

    if( demoplayback && EV_legacy && (EV_legacy < 147) )
    {
        // [WDJ] ANGLE_1 (from Heretic) has a significant round off error.
        // 0x10e * ANGLE_1 -> 0xbfffff40,  it should be 0xc0000000
        // When used for positioning, it leads to demo sync problems.
        mobj->angle = mthing->angle * ANGLE_1;      // SSNTails 06-10-2003
    }
    else
    {
        // Like every other Doom.
        // Heretic also spawns using this calc.
        mobj->angle = wad_to_angle(mthing->angle);
    }
   
    if (mthing->options & MTF_AMBUSH)
        mobj->flags |= MF_AMBUSH;

    mthing->mobj = mobj;
}

//
// GAME SPAWN FUNCTIONS
//

//
// P_SpawnSplash
//
// when player moves in water
// SoM: Passing the Z height saves extra calculations...
void P_SpawnSplash(mobj_t * mo, fixed_t z)
{
    mobj_t *th;
    //fixed_t     z;

    if( EV_legacy < 125 )
        return;

#if 0   
    //SoM: disabled 3/17/2000
    // flatwater : old water FWATER flat texture
    // we are supposed to be in water sector and my current
    // hack uses negative tag as water height
    if (flatwater)
       z = mo->subsector->sector->floorheight + (FRACUNIT/4);
    else
       z = sectors[mo->subsector->sector->modelsec].floorheight; 
#endif   

    // need to touch the surface because the splashes only appear at surface
    if (mo->z > z || mo->z + mo->height < z)
        return;

    // note pos +1 +1 so it doesn't eat the sound of the player..
    th = P_SpawnMobj(mo->x + 1, mo->y + 1, z, MT_SPLASH);
    //if( z - mo->subsector->sector->floorheight > 4*FRACUNIT)
    S_StartObjSound(th, sfx_gloop);
    //else
    //    S_StartObjSound (th,sfx_splash);
    th->tics -= P_Random() & 3;

    if (th->tics < 1)
        th->tics = 1;

    // get rough idea of speed
    /*
       thrust = (mo->momx + mo->momy) >> FRACBITS+1;

       if (thrust >= 2 && thrust<=3)
       P_SetMobjState (th,S_SPLASH2);
       else
       if (thrust < 2)
       P_SetMobjState (th,S_SPLASH3);
     */
}

// --------------------------------------------------------------------------
// P_SpawnSmoke
// --------------------------------------------------------------------------
// when player gets hurt by lava/slime, spawn at feet
void P_SpawnSmoke(fixed_t x, fixed_t y, fixed_t z)
{
    mobj_t *th;

    if( EV_legacy < 125 )
        return;

    x = x - ((P_Random() & 8) * FRACUNIT) - 4 * FRACUNIT;
    y = y - ((P_Random() & 8) * FRACUNIT) - 4 * FRACUNIT;
    z += (P_Random() & 3) * FRACUNIT;

    th = P_SpawnMobj(x, y, z, MT_SMOK);
    th->momz = FRACUNIT;
    th->tics -= P_Random() & 3;

    if (th->tics < 1)
        th->tics = 1;
}

// --------------------------------------------------------------------------
// P_SpawnPuff
// --------------------------------------------------------------------------
// Heretic must have set PuffType before calling routines that call SpawnPuff.
// Otherwise you get a player mobj stuck on the thing the player just punched.
void P_SpawnPuff(fixed_t x, fixed_t y, fixed_t z)
{
    mobj_t *puff;

    z += PP_SignedRandom(pr_spawnpuff) << 10;

    if (EN_heretic)
    {
        puff = P_SpawnMobj(x, y, z, PuffType);
        if (puff->info->attacksound)
        {
            S_StartObjSound(puff, puff->info->attacksound);
        }
        switch (PuffType)
        {
            case MT_BEAKPUFF:
            case MT_STAFFPUFF:
                puff->momz = FRACUNIT;
                break;
            case MT_GAUNTLETPUFF1:
            case MT_GAUNTLETPUFF2:
                puff->momz = .8 * FRACUNIT;
            default:
                break;
        }
    }
    else
    {

        puff = P_SpawnMobj(x, y, z, MT_PUFF);
        puff->momz = FRACUNIT;
        puff->tics -= PP_Random(pr_spawnpuff) & 3;

        if (puff->tics < 1)
            puff->tics = 1;

        // don't make punches spark on the wall
        // la_attackrange is global var param of LineAttack
        if (la_attackrange == MELEERANGE)
            P_SetMobjState(puff, S_PUFF3);
    }
}

// --------------------------------------------------------------------------
// P_SpawnBlood
// --------------------------------------------------------------------------

static mobj_t *bloodthing;
static fixed_t bloodspawnpointx, bloodspawnpointy;

#ifdef WALLSPLATS
boolean PTR_BloodTraverse(intercept_t * in)
{
    line_t *li;
    divline_t divl;
    fixed_t frac;

    fixed_t z;

    if (in->isaline)
    {
        li = in->d.line;

        z = bloodthing->z + (P_SignedRandom() << (FRACBITS - 3));
        if (!(li->flags & ML_TWOSIDED))
            goto hitline;

        P_LineOpening(li);

        // hit lower texture ?
        if (li->frontsector->floorheight != li->backsector->floorheight)
        {
            if (openbottom > z)
                goto hitline;
        }

        // hit upper texture ?
        if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
        {
            if (opentop < z)
                goto hitline;
        }

        // else don't hit
        return true;

      hitline:
        P_MakeDivline(li, &divl);
        frac = P_InterceptVector(&divl, &trace);
        // Chexquest: has green splats, BLUDA0, BLUDB0, and BLUDC0
        if (EN_heretic)
        {
            // BLODC0 from heretic wad
            R_AddWallSplat(li, P_PointOnLineSide(bloodspawnpointx, bloodspawnpointy, li), "BLODC0", z, frac, SPLATDRAWMODE_TRANS);
        }
        else
        {
            // BLUDC0 from wad or legacy.wad, green splat for chexquest
            R_AddWallSplat(li, P_PointOnLineSide(bloodspawnpointx, bloodspawnpointy, li), "BLUDC0", z, frac, SPLATDRAWMODE_TRANS);
        }
        return false;
    }

    //continue
    return true;
}
#endif

// P_SpawnBloodSplats
// the new SpawnBlood : this one first calls P_SpawnBlood for the usual blood sprites
// then spawns blood splats around on walls
//
void P_SpawnBloodSplats(fixed_t x, fixed_t y, fixed_t z, int damage, fixed_t momx, fixed_t momy)
{
#ifdef WALLSPLATS
//static int  counter =0;
    fixed_t x2, y2;
    angle_t angle, anglesplat;
    int distance;
    angle_t anglemul = 1;
    int numsplats;
    int i;
#endif

    if ( ! cv_splats.EV )  // obey splats option
        return; 

    // spawn the usual falling blood sprites at location
    // Creates bloodthing passed to PTR_BloodTraverse
    P_SpawnBlood(x, y, z, damage);
    //debug_Printf ("spawned blood counter %d\n", counter++);

    if( EV_legacy < 129 )
        return;

#ifdef WALLSPLATS
    // traverse all linedefs and mobjs from the blockmap containing t1,
    // to the blockmap containing the dest. point.
    // Call the function for each mobj/line on the way,
    // starting with the mobj/linedef at the shortest distance...

    if (!momx && !momy)
    {
        // from inside
        angle = 0;
        anglemul = 2;
    }
    else
    {
        // get direction of damage
        x2 = x + momx;
        y2 = y + momy;
        angle = R_PointToAngle2(x, y, x2, y2);
    }
    distance = damage * 6;
    numsplats = damage / 3 + 1;
    // BFG is funy without this check
    if (numsplats > 20)
        numsplats = 20;

    if (gamemode == chexquest1)
    {
        distance /=8;  // less violence to splats
    }

    //debug_Printf ("spawning %d bloodsplats at distance of %d\n", numsplats, distance);
    //debug_Printf ("damage %d\n", damage);
    bloodspawnpointx = x;
    bloodspawnpointy = y;
    //uses 'bloodthing' set by P_SpawnBlood()
    for (i = 0; i < numsplats; i++)
    {
        // find random angle between 0-180deg centered on damage angle
        anglesplat = angle + (((P_Random() - 128) * FINEANGLES / 512 * anglemul) << ANGLETOFINESHIFT);
        x2 = x + distance * cosine_ANG(anglesplat);
        y2 = y + distance * sine_ANG(anglesplat);

        P_PathTraverse(x, y, x2, y2, PT_ADDLINES, PTR_BloodTraverse);
    }
#endif

#ifdef FLOORSPLATS
    // add a test floor splat
    R_AddFloorSplat(bloodthing->subsector, "STEP2", x, y, bloodthing->floorz, SPLATDRAWMODE_SHADE);
#endif
}

// P_SpawnBlood
// spawn a blood sprite with falling z movement, at location
// the duration and first sprite frame depends on the damage level
// the more damage, the longer is the sprite animation
void P_SpawnBlood(fixed_t x, fixed_t y, fixed_t z, int damage)
{
    mobj_t *th;

    z += PP_SignedRandom(pr_spawnblood) << 10;
    th = P_SpawnMobj(x, y, z, MT_BLOOD);
    if( EV_legacy >= 128 )
    {
        th->momx = P_SignedRandom() << 12;      //faB:19jan99
        th->momy = P_SignedRandom() << 12;      //faB:19jan99
    }
    th->momz = FRACUNIT * 2;
    th->tics -= PP_Random(pr_spawnblood) & 3;

    if (th->tics < 1)
        th->tics = 1;

    if (gamemode == chexquest1)
    {
        // less violence to splats
        th->momx /=8;
        th->momy /=8;
        th->momz /=4;
    }

    if (damage <= 12 && damage >= 9)
        P_SetMobjState(th, S_BLOOD2);
    else if (damage < 9)
        P_SetMobjState(th, S_BLOOD3);

    bloodthing = th;
}

//---------------------------------------------------------------------------
//
// FUNC P_HitFloor
//
//---------------------------------------------------------------------------

int P_HitFloor(mobj_t * thing)
{
    mobj_t *mo;
    int floortype;

    if (thing->floorz != thing->subsector->sector->floorheight)
    {   // don't splash if landing on the edge above water/lava/etc....
        return (FLOOR_SOLID);
    }
    floortype = P_GetThingFloorType(thing);
    if (EN_heretic)
    {

        if (thing->type != MT_BLOOD)
        {
            switch (floortype)
            {
                case FLOOR_WATER:
                    P_SpawnMobj(thing->x, thing->y, ONFLOORZ, MT_SPLASHBASE);
                    mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, MT_HSPLASH);
                    P_SetReference(mo->target, thing);
                    mo->target = thing;
                    mo->momx = P_SignedRandom() << 8;
                    mo->momy = P_SignedRandom() << 8;
                    mo->momz = 2 * FRACUNIT + (P_Random() << 8);
                    S_StartObjSound(mo, sfx_gloop);
                    break;
                case FLOOR_LAVA:
                    P_SpawnMobj(thing->x, thing->y, ONFLOORZ, MT_LAVASPLASH);
                    mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, MT_LAVASMOKE);
                    mo->momz = FRACUNIT + (P_Random() << 7);
                    S_StartObjSound(mo, sfx_burn);
                    break;
                case FLOOR_SLUDGE:
                    P_SpawnMobj(thing->x, thing->y, ONFLOORZ, MT_SLUDGESPLASH);
                    mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, MT_SLUDGECHUNK);
                    P_SetReference(mo->target, thing);
                    mo->target = thing;
                    mo->momx = P_SignedRandom() << 8;
                    mo->momy = P_SignedRandom() << 8;
                    mo->momz = FRACUNIT + (P_Random() << 8);
                    break;
            }
        }
        return floortype;
    }
    else if (floortype == FLOOR_WATER)
        P_SpawnSplash(thing, thing->floorz);

    // do not down the viewpoint
    return (FLOOR_SOLID);
}

//
// P_CheckMissileSpawn
// Moves the missile forward a bit
//  and possibly explodes it right there.
//
boolean P_CheckMissileSpawn(mobj_t * th)
{
    if (EN_doom_etc)
    {
        th->tics -= PP_Random(pr_missile) & 3;
        if (th->tics < 1)
            th->tics = 1;
    }

    // move a little forward so an angle can
    // be computed if it immediately explodes
    th->x += (th->momx >> 1);
    th->y += (th->momy >> 1);
    th->z += (th->momz >> 1);

    if (!P_TryMove(th, th->x, th->y, false))
    {
        P_ExplodeMissile(th);
        return false;
    }
    return true;
}

//
// P_SpawnMissile
//
mobj_t *P_SpawnMissile(mobj_t * source, mobj_t * dest, mobjtype_t type)
{
    mobj_t *th;
    angle_t ang;
    int dist;
    fixed_t z;

#ifdef PARANOIA
    if (!source)
        I_Error("P_SpawnMissile : no source");
    if (!dest)
        I_Error("P_SpawnMissile : no dest");
#endif
    switch (type)
    {
        case MT_MNTRFX1:       // Minotaur swing attack missile
            z = source->z + 40 * FRACUNIT;
            break;
        case MT_MNTRFX2:       // Minotaur floor fire missile
            z = ONFLOORZ;
            break;
        case MT_SRCRFX1:       // Sorcerer Demon fireball
            z = source->z + 48 * FRACUNIT;
            break;
        case MT_KNIGHTAXE:     // Knight normal axe
        case MT_REDAXE:        // Knight red power axe
            z = source->z + 36 * FRACUNIT;
            break;
        default:
            z = source->z + 32 * FRACUNIT;
            break;
    }
    if (source->flags2 & MF2_FEETARECLIPPED)
        z -= FOOTCLIPSIZE;

    th = P_SpawnMobj(source->x, source->y, z, type);

    if (th->info->seesound)
        S_StartObjSound(th, th->info->seesound);

    P_SetReference(th->target, source);
    th->target = source;        // where it came from

    if( cv_predictingmonsters.EV || (source->eflags & MF_PREDICT))  //added by AC for predmonsters
    {
        boolean canHit;
        fixed_t px, py, pz;
        int mtime, t;
        subsector_t *sec;

        dist = P_AproxDistance(dest->x - source->x, dest->y - source->y);
        mtime = dist / th->info->speed;
        mtime = P_AproxDistance(dest->x + dest->momx * mtime - source->x, dest->y + dest->momy * mtime - source->y) / th->info->speed;

        canHit = false;
        t = mtime + 4;
        do
        {
            t -= 4;
            if (t < 1)
                t = 1;
            px = dest->x + dest->momx * t;
            py = dest->y + dest->momy * t;
            pz = dest->z + dest->momz * t;
            canHit = P_CheckSight2(source, dest, px, py, pz);
        } while (!canHit && (t > 1));
        pz = dest->z + dest->momz * mtime;

        sec = R_PointInSubsector(px, py);
        if (!sec)
            sec = dest->subsector;

        if (pz < sec->sector->floorheight)
            pz = sec->sector->floorheight;
        else if (pz > sec->sector->ceilingheight)
            pz = sec->sector->ceilingheight - dest->height;

        ang = R_PointToAngle2(source->x, source->y, px, py);

        // fuzzy player
        if (dest->flags & MF_SHADOW)
        {
            if (EN_heretic)
                ang += P_SignedRandom() << 21;
            else
                ang += PP_SignedRandom(pr_shadow) << 20;
        }

        th->angle = ang;
        int angf = ANGLE_TO_FINE(ang); 
        th->momx = FixedMul(th->info->speed, finecosine[angf]);
        th->momy = FixedMul(th->info->speed, finesine[angf]);

        if (t < 1)
            t = 1;

        th->momz = (pz - source->z) / t;
    }
    else
    {
        ang = R_PointToAngle2(source->x, source->y, dest->x, dest->y);

        // fuzzy player
        if (dest->flags & MF_SHADOW)
        {
            if (EN_heretic)
                ang += P_SignedRandom() << 21;
            else
                ang += P_SignedRandom() << 20;
        }

        th->angle = ang;
        int angf = ANGLE_TO_FINE(ang); 
        th->momx = FixedMul(th->info->speed, finecosine[angf]);
        th->momy = FixedMul(th->info->speed, finesine[angf]);

        dist = P_AproxDistance(dest->x - source->x, dest->y - source->y);
        dist = dist / th->info->speed;

        if (dist < 1)
            dist = 1;

        th->momz = (dest->z - source->z) / dist;
    }

    dist = P_CheckMissileSpawn(th);
    if( EV_legacy < 131 )
        return th;

    return dist ? th : NULL;
}

//
// P_SpawnPlayerMissile
// Tries to aim at a nearby monster
//
mobj_t *P_SPMAngle(mobj_t * source, mobjtype_t type, angle_t angle)
{
    mobj_t *th;
    angle_t ang;

    fixed_t x, y, z;
    fixed_t slope = 0;
    byte  friend_protect;

    // angle at which you fire, is player angle
    ang = angle;

    //added:16-02-98: autoaim is now a toggle
    if( source->player->autoaim_toggle && cv_allowautoaim.EV )
    {
        // Try first with friend_protect, then without friend_protect.
        for( friend_protect = EN_mbf; ; )
        {
            // see which target is to be aimed at
            slope = P_AimLineAttack(source, ang, 16 * 64 * FRACUNIT, friend_protect);
            // lar_linetarget returned by P_AimLineAttack
            if( lar_linetarget )  break;

            ang += 1 << 26;
            slope = P_AimLineAttack(source, ang, 16 * 64 * FRACUNIT, friend_protect);
            if( lar_linetarget )  break;

            ang -= 2 << 26;
            slope = P_AimLineAttack(source, ang, 16 * 64 * FRACUNIT, friend_protect);
            if( lar_linetarget )  break;

            // no target
            ang = angle;
            slope = 0;
            if( ! friend_protect )  break;
            // MBF retry without friend protect
            friend_protect = 0;
        }
    }

    //added:18-02-98: if not autoaim, or if the autoaim didnt aim something,
    //                use the mouseaiming
    // lar_linetarget returned by P_AimLineAttack
    if( !(source->player->autoaim_toggle && cv_allowautoaim.EV )
        || (!lar_linetarget && (EV_legacy > 111)) )
    {
        // Manual aiming
        if( EV_legacy >= 128 )
            slope = AIMINGTOSLOPE(source->player->aiming);
        else
            slope = (source->player->aiming << FRACBITS) / 160;
    }

    x = source->x;
    y = source->y;
    z = source->z + 4 * 8 * FRACUNIT;
    if (source->flags2 & MF2_FEETARECLIPPED)
        z -= FOOTCLIPSIZE;

    th = P_SpawnMobj(x, y, z, type);

    if (th->info->seesound)
        S_StartObjSound(th, th->info->seesound);

    P_SetReference( th->target, source );
    th->target = source;

    th->angle = ang;
    int angf = ANGLE_TO_FINE( ang );
    fixed_t speed = th->info->speed;
    th->momx = FixedMul(speed, finecosine[angf]);
    th->momy = FixedMul(speed, finesine[angf]);
    th->momz = FixedMul(speed, slope);

    if( EV_legacy >= 128 )
    {   // 1.28 fix, allow full aiming must be much precise
        fixed_t aimcosine = cosine_ANG( source->player->aiming );
        th->momx = FixedMul(th->momx, aimcosine);
        th->momy = FixedMul(th->momy, aimcosine);
    }

    if (th->type == MT_BLASTERFX1)
    {   // Ultra-fast ripper spawning missile
        th->x += (th->momx >> 3) - (th->momx >> 1);
        th->y += (th->momy >> 3) - (th->momy >> 1);
        th->z += (th->momz >> 3) - (th->momz >> 1);
    }

    slope = P_CheckMissileSpawn(th);

    if( EV_legacy < 131 )
        return th;

    return slope ? th : NULL;
}


// [WDJ] Extra mapthing for FS
#define EXTRA_MAPTHING_INC  64
typedef struct extra_mapthing_s {
    struct extra_mapthing_s *  link;  // to next allocation
    mapthing_t   mt_array[ EXTRA_MAPTHING_INC ];
} extra_mapthing_t;

static extra_mapthing_t * extra_mapthing_chunk = NULL;  // Z_Malloc
static mapthing_t * free_mapthing = NULL;

void P_Free_Extra_Mapthing( mapthing_t * mthing )
{
    mthing->options = 0;
    mthing->mobj = (mobj_t*) free_mapthing;  // link into free
    free_mapthing = mthing;
}

// Create an extra mapthing for FS spawn
mapthing_t * P_Get_Extra_Mapthing( uint16_t flags )
{
    int i;
    mapthing_t * mthing;

    if ( ! free_mapthing )
    {
        // allocate some mapthings as a chunk
        extra_mapthing_t * mapthing_chunk =
            Z_Malloc( sizeof(extra_mapthing_t), PU_LEVEL, NULL);
        memset( mapthing_chunk, 0, sizeof(extra_mapthing_t) );  // zeroed
        mapthing_chunk->link = extra_mapthing_chunk;  // link
        extra_mapthing_chunk = mapthing_chunk;
        for( i=EXTRA_MAPTHING_INC-1; i>=0; i-- )
        {
            P_Free_Extra_Mapthing( &mapthing_chunk->mt_array[i] );
        }
    }
    // get free mapthing
    mthing = free_mapthing;
    free_mapthing = (mapthing_t*) free_mapthing->mobj;  // unlink, reuse mobj field

    mthing->options = flags | MTF_EXTRA;  // in use
    return mthing;
}

void P_Clear_Extra_Mapthing( void )
{
    // All marked PU_LEVEL will be freed together, probably before this call.
    extra_mapthing_chunk = NULL;
    free_mapthing = NULL;
}

// Returns an index number for a mapthing, first index is 1
// Returns 0 if not found
unsigned int P_Extra_Mapthing_Index( mapthing_t * mtp )
{
    unsigned int index = 1;
    extra_mapthing_t * chunk = extra_mapthing_chunk;

    if (mtp==NULL)  goto not_found;
    if (chunk == NULL)  goto not_found;
    // find chunk that contains mthing
    while( ((mtp < &chunk->mt_array[0])
            || (mtp > &chunk->mt_array[EXTRA_MAPTHING_INC-1]) ))
    {
       chunk = chunk->link;
       if (chunk == NULL)  goto not_found;
       index += EXTRA_MAPTHING_INC;
    }
    return index + (mtp - &chunk->mt_array[0]);

 not_found:
    return 0;  // not found
}


// Traverse all Extra Mapthing that are in use
mapthing_t * P_Traverse_Extra_Mapthing( mapthing_t * prev )
{
    extra_mapthing_t * chunk = extra_mapthing_chunk;
    mapthing_t * mtp = prev;
    uint16_t  option_flags = 0;

    if (chunk == NULL)  goto done;
    if (prev == NULL)
    {
        mtp = &chunk->mt_array[0];  // first entry
        option_flags = mtp->options;
    }
    else
    {
        // find chunk that contains mthing
        while( ((prev < &chunk->mt_array[0])
                || (prev > &chunk->mt_array[EXTRA_MAPTHING_INC-1]) ))
        {
            chunk = chunk->link;
            if (chunk == NULL)  goto done;
        }
    }
    while( option_flags == 0 )   // skip unused
    {
        // advance to next mthing
        if( mtp < &chunk->mt_array[EXTRA_MAPTHING_INC-1] )
        {
            mtp ++;
        }
        else
        {
            // next chunk
            chunk = chunk->link;  // advance chunk
            if (chunk == NULL)  goto done;
            mtp = &chunk->mt_array[0];  // first entry
        }
        option_flags = mtp->options;
    }
    return mtp;
 done:
    return NULL;
}

