// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: p_fab.c 837 2011-05-27 00:52:45Z wesleyjohnson $
//
// Copyright (C) 1998-2000 by DooM Legacy Team.
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
// $Log: p_fab.c,v $
// Revision 1.5  2000/10/21 08:43:30  bpereira
// no message
//
// Revision 1.4  2000/07/01 09:23:49  bpereira
// no message
//
// Revision 1.3  2000/04/24 20:24:38  bpereira
// no message
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      some new action routines, separated from the original doom
//      sources, so that you can include it or remove it easy.
//
//-----------------------------------------------------------------------------


#include "doomdef.h"
#include "g_game.h"
#include "p_local.h"
#include "r_state.h"
#include "p_fab.h"
#include "m_random.h"

#ifdef DOORDELAY_CONTROL
// [WDJ] 1/15/2009 support control of door and event delay
// see p_doors.c
extern  int  adj_ticks_per_sec;

CV_PossibleValue_t doordelay_cons_t[]={{0,"0.9x Fast"}, {1,"Normal"}, {2,"1.2x Slow"}, {3,"1.6x Slow"}, {4,"2x Slow"}, {5,"3x Slow"}, {0,NULL}};
int  doordelay_table[6] = { 32, 35, 42, 56, 70, 105 };  // 35 is normal

void DoorDelay_OnChange( void )
{
   adj_ticks_per_sec = doordelay_table[ cv_doordelay.value ];
}

//consvar_t cv_doordelay = {"doordelay","1",CV_NETVAR|CV_CALL|CV_SAVE,doordelay_cons_t,DoorDelay_OnChange};
consvar_t cv_doordelay = {"doordelay","1",CV_CALL|CV_SAVE,doordelay_cons_t,DoorDelay_OnChange};

#endif


#ifdef VOODOO_DOLL
// [WDJ] 2/3/2011  Insta-death by voodoo doll, subverted.
CV_PossibleValue_t instadeath_cons_t[]={{0,"Die"}, {1,"Damage"}, {2,"Zap"}, {0,NULL}};
consvar_t cv_instadeath = {"instadeath", "0", CV_SAVE|CV_NETVAR, instadeath_cons_t, NULL};

voodoo_mode_e  voodoo_mode;

// [WDJ] 2/7/2011  Voodoo doll behavior
void VoodooMode_OnChange( void )
{
   // config file saves cv_voodoo_mode but changes to voodoo_mode do not get saved.
   voodoo_mode = (voodoo_mode_e) cv_voodoo_mode.value;
}

CV_PossibleValue_t voodoo_mode_cons_t[]={{0,"Vanilla"}, {1,"Multispawn"}, {2,"Target"}, {3, "Auto"}, {0,NULL}};
consvar_t cv_voodoo_mode = {"voodoo_mode", "3", CV_CALL|CV_SAVE|CV_NETVAR, voodoo_mode_cons_t, VoodooMode_OnChange};
#endif

// Translucency

void Translucency_OnChange(void);

consvar_t cv_translucency  = {"translucency" ,"1",CV_CALL|CV_SAVE,CV_OnOff, Translucency_OnChange};

//
// Action routine, for the ROCKET thing.
// This one adds trails of smoke to the rocket.
// The action pointer of the S_ROCKET state must point here to take effect.
// This routine is based on the Revenant Fireball Tracer code A_Tracer()
//
void A_SmokeTrailer (mobj_t* actor)
{
    mobj_t*     th;

    if (gametic % (4 * NEWTICRATERATIO))
        return;

    // spawn a puff of smoke behind the rocket
    if (demoversion<125 && // rocket trails spawnpuff from v1.11 to v1.24
        demoversion>=111) // skull trails since v1.25
        P_SpawnPuff (actor->x, actor->y, actor->z);

    // add the smoke behind the rocket
    th = P_SpawnMobj (actor->x-actor->momx,
                      actor->y-actor->momy,
                      actor->z, MT_SMOK);

    th->momz = FRACUNIT;
    th->tics -= P_Random()&3;
    if (th->tics < 1)
        th->tics = 1;
}


static boolean reset_translucent=false;
//  Set the translucency map for each frame state of mobj
//
void R_SetTrans (statenum_t state1, statenum_t state2, translucent_e transel)
{
    state_t*   state = &states[state1];
    do
    {
        state->frame &= ~FF_TRANSMASK;
       if(!reset_translucent)
           state->frame |= (transel<<FF_TRANSSHIFT);
        state++;
    } while (state1++<state2);
}


//  hack the translucency in the states for a set of standard doom sprites
//
void P_SetTranslucencies (void)
{

    //revenant fireball
    R_SetTrans (S_TRACER    , S_TRACER2    , TRANSLU_fire);
    R_SetTrans (S_TRACEEXP1 , S_TRACEEXP3  , TRANSLU_med);
                                           
    //rev. fireball. smoke trail           
    R_SetTrans (S_SMOKE1    , S_SMOKE5     , TRANSLU_med);
                                           
    //imp fireball                         
    R_SetTrans (S_TBALL1    , S_TBALL2     , TRANSLU_fire);
    R_SetTrans (S_TBALLX1   , S_TBALLX3    , TRANSLU_med);
                                           
    //archvile attack                      
    R_SetTrans (S_FIRE1     , S_FIRE30     , TRANSLU_fire);
                                           
    //bfg ball                             
    R_SetTrans (S_BFGSHOT   , S_BFGSHOT2   , TRANSLU_fire);
    R_SetTrans (S_BFGLAND   , S_BFGLAND3   , TRANSLU_med);
    R_SetTrans (S_BFGLAND4  , S_BFGLAND6   , TRANSLU_more);
    R_SetTrans (S_BFGEXP    , 0            , TRANSLU_med);
    R_SetTrans (S_BFGEXP2   , S_BFGEXP4    , TRANSLU_more);
                                           
    //plasma bullet                        
    R_SetTrans (S_PLASBALL  , S_PLASBALL2  , TRANSLU_fire);
    R_SetTrans (S_PLASEXP   , S_PLASEXP2   , TRANSLU_med);
    R_SetTrans (S_PLASEXP3  , S_PLASEXP5   , TRANSLU_more);
                                           
    //bullet puff                          
    R_SetTrans (S_PUFF1     , S_PUFF4      , TRANSLU_more);
                                           
    //teleport fog                         
    R_SetTrans (S_TFOG      , S_TFOG5      , TRANSLU_med);
    R_SetTrans (S_TFOG6     , S_TFOG10     , TRANSLU_more);
                                           
    //respawn item fog                     
    R_SetTrans (S_IFOG      , S_IFOG5      , TRANSLU_med);
                                           
    //soulsphere                           
    R_SetTrans (S_SOUL      , S_SOUL6      , TRANSLU_med);
    //invulnerability                      
    R_SetTrans (S_PINV      , S_PINV4      , TRANSLU_med);
    //blur artifact                        
    R_SetTrans (S_PINS      , S_PINS4      , TRANSLU_med);
    //megasphere                           
    R_SetTrans (S_MEGA      , S_MEGA4      , TRANSLU_med);
                            
    R_SetTrans (S_GREENTORCH, S_REDTORCH4  , TRANSLU_fx1); // blue torch
    R_SetTrans (S_GTORCHSHRT, S_RTORCHSHRT4, TRANSLU_fx1); // short blue torch

    // flaming barrel !!
    R_SetTrans (S_BBAR1     , S_BBAR3      , TRANSLU_fx1);

    //lost soul
    R_SetTrans (S_SKULL_STND, S_SKULL_DIE6 , TRANSLU_fx1);
    //baron shot
    R_SetTrans (S_BRBALL1   , S_BRBALL2    , TRANSLU_fire);
    R_SetTrans (S_BRBALLX1  , S_BRBALLX3   , TRANSLU_med);
    //demon spawnfire
    R_SetTrans (S_SPAWNFIRE1, S_SPAWNFIRE3 , TRANSLU_fire);
    R_SetTrans (S_SPAWNFIRE4, S_SPAWNFIRE8 , TRANSLU_med);
    //caco fireball
    R_SetTrans (S_RBALL1    , S_RBALL2     , TRANSLU_fire);
    R_SetTrans (S_RBALLX1   , S_RBALLX3    , TRANSLU_med);

    //arachno shot
    R_SetTrans (S_ARACH_PLAZ, S_ARACH_PLAZ2, TRANSLU_fire);
    R_SetTrans (S_ARACH_PLEX, S_ARACH_PLEX2, TRANSLU_med);
    R_SetTrans (S_ARACH_PLEX3,S_ARACH_PLEX4, TRANSLU_more);
    R_SetTrans (S_ARACH_PLEX5,            0, TRANSLU_hi);

    //blood puffs!
    //R_SetTrans (S_BLOOD1   ,            0, TRANSLU_med);
    //R_SetTrans (S_BLOOD2   , S_BLOOD3    , TRANSLU_more);

    //eye in symbol
    R_SetTrans (S_EVILEYE    , S_EVILEYE4  , TRANSLU_med);
                                          
    //mancubus fireball
    R_SetTrans (S_FATSHOT1   , S_FATSHOT2  , TRANSLU_fire);
    R_SetTrans (S_FATSHOTX1  , S_FATSHOTX3 , TRANSLU_med);

    // rockets explosion
    R_SetTrans (S_EXPLODE1   , S_EXPLODE2  , TRANSLU_fire);
    R_SetTrans (S_EXPLODE3   ,            0, TRANSLU_med);

    //Fab: lava/slime damage smoke test
    R_SetTrans (S_SMOK1      , S_SMOK5     , TRANSLU_med);
    R_SetTrans (S_SPLASH1    , S_SPLASH3   , TRANSLU_more);
}

void Translucency_OnChange(void)
{
    if( cv_translucency.value==0 )
        reset_translucent = true;
    if (!cv_fuzzymode.value)
        P_SetTranslucencies();
    reset_translucent = false;
}


// =======================================================================
//                    FUNKY DEATHMATCH COMMANDS
// =======================================================================

void BloodTime_OnChange (void);

CV_PossibleValue_t bloodtime_cons_t[]={{1,"MIN"},{3600,"MAX"},{0,NULL}};
// how much tics to last for the last (third) frame of blood (S_BLOODx)
consvar_t cv_bloodtime = {"bloodtime","20",CV_NETVAR|CV_CALL|CV_SAVE,bloodtime_cons_t,BloodTime_OnChange};

// Called when var. 'bloodtime' is changed : set the blood states duration
//
void BloodTime_OnChange (void)
{
    states[S_BLOOD1].tics = 8;
    states[S_BLOOD2].tics = 8;
    states[S_BLOOD3].tics = (cv_bloodtime.value*TICRATE) - 16;

    CONS_Printf ("blood lasts for %d seconds\n", cv_bloodtime.value);
}


// [WDJ] All misc init
void D_RegisterMiscCommands (void)
{
    // add commands for deathmatch rules and style (like more blood) :)
    CV_RegisterVar (&cv_solidcorpse);                 //p_enemy

    CV_RegisterVar (&cv_bloodtime);

    // BP:not realy in deathmatch but is just here
    CV_RegisterVar (&cv_translucency);
#ifdef DOORDELAY_CONTROL
    // [WDJ] 1/15/2009 support control of door and event delay
    // see p_doors.c
    CV_RegisterVar (&cv_doordelay);
#endif   
#ifdef VOODOO_DOLL
    // [WDJ] 2/7/2011 Voodoo
    CV_RegisterVar (&cv_instadeath);
    CV_RegisterVar (&cv_voodoo_mode);
#endif
    CV_RegisterVar (&cv_monsterfriction);
}

void  DemoAdapt_p_fab(void)  // local enables of p_fab
{
    if( ! demoplayback )
    {
        // restore player settings
#ifdef DOORDELAY_CONTROL
        DoorDelay_OnChange();
#endif
#ifdef VOODOO_DOLL
        // cv_instadeath not covered here
        VoodooMode_OnChange();
#endif
    }
}
