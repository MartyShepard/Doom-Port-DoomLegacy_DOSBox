// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: p_tick.c 1512 2020-04-04 08:51:13Z wesleyjohnson $
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
// $Log: p_tick.c,v $
// Revision 1.6  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.5  2000/11/02 17:50:09  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.4  2000/10/21 08:43:31  bpereira
// Revision 1.3  2000/10/08 13:30:01  bpereira
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Archiving: SaveGame I/O.
//      Thinker, Ticker.
//
//-----------------------------------------------------------------------------


#include "doomstat.h"
#include "p_tick.h"
#include "g_game.h"
#include "p_local.h"
#include "z_zone.h"
#include "t_script.h"



tic_t     leveltime;

//
// THINKERS
// All thinkers should be allocated by Z_Malloc
// so they can be operated on uniformly.
// The actual structures will vary in size,
// but the first element must be thinker_t.
//



// Both the head and tail of the thinker list.
// Linked by next, prev.
thinker_t  thinkercap;
// MBF, class-lists.
// Linked by cnext, cprev.
thinker_t  thinkerclasscap[NUMTHCLASS];

#ifdef THINKER_INTERPOLATIONS
static byte  newthinkerpresent = true;
#endif


//
// P_Init_Thinkers
//
void P_Init_Thinkers (void)
{
    int i;

    // [WDJ] MBF, from MBF, PrBoom.
    // Init all class-list.
    for( i=0; i<NUMTHCLASS; i++ )
      thinkerclasscap[i].cprev = thinkerclasscap[i].cnext = &thinkerclasscap[i];

    thinkercap.prev = thinkercap.next  = &thinkercap;
}




//
// P_AddThinker
// Adds a new thinker at the end of the list.
//
void P_AddThinker (thinker_t* thinker)
{
    thinkercap.prev->next = thinker;
    thinker->next = &thinkercap;
    thinker->prev = thinkercap.prev;
    thinkercap.prev = thinker;
   
    // From MBF, PrBoom
#ifdef REFERENCE_COUNTING
    thinker->references = 0;    // killough 11/98: init reference counter to 0
#endif
    // killough 8/29/98: init pointers, and then add to appropriate list
    thinker->cnext = thinker->cprev = NULL;  // init
    P_UpdateClassThink(thinker, TH_unknown);

#ifdef THINKER_INTERPOLATIONS
    newthinkerpresent = true;
#endif
}


//
// P_RemoveThinker
// Deallocation is lazy -- it will not actually be freed
// until its thinking turn comes up.
//
void P_RemoveThinker (thinker_t* thinker)
{
    // Setup an action function that does removal.
    thinker->function.acp1 = (actionf_p1) T_RemoveThinker;

    // [WDJ] MBF, from MBF, PrBoom
    // killough 8/29/98: remove immediately from class-list
   
    // haleyjd 11/09/06: NO! Doing this here was always suspect to me, and
    // sure enough: if a monster's removed at the wrong time, it gets put
    // back into the list improperly and starts causing an infinite loop in
    // the AI code. We'll follow PrBoom's lead and create a th_delete class
    // for thinkers awaiting deferred removal.

    // [WDJ] Being in a delete list does nothing to stop being found.
    // Delete class links, and let acp1 block linking again.
    P_UpdateClassThink( thinker, TH_delete );
}

// Thinker function that removes the thinker.
// In PrBoom, MBF, this is called P_RemoveThinkerDelayed, and is more
// complicated, using reference counts and modifying currentthinker.
// Our RunThinker handles removal better.
void T_RemoveThinker( thinker_t* remthink )
{
    // [WDJ] MBF, from MBF, PrBoom
#ifdef REFERENCE_COUNTING
    if( remthink->references )  return;
#endif
    
    // Remove from current class-list, if in one.
    P_UpdateClassThink( remthink, TH_none );

    // Unlink and delete the thinker
    remthink->next->prev = remthink->prev;
    remthink->prev->next = remthink->next;
    Z_Free (remthink);  // mobj, etc.
}


// [WDJ] MBF, from MBF, PrBoom, EternityEngine.
// killough 8/29/98:
// [WDJ] Heavily rewritten to eliminate unused class-lists.
// Make readable.
//
// Maintain separate class-lists of friends and enemies, to permit more
// efficient searches.
//

void P_UpdateClassThink(thinker_t *thinker, int tclass )
{
    register thinker_t * th;

    if( tclass == TH_unknown )
    {
        // Find the class where the thinker belongs.
        // Most common case first.
        tclass = TH_misc;
        if( thinker->function.acp1 == (actionf_p1)P_MobjThinker )
        {
            register mobj_t * mo = (mobj_t *) thinker;
            if( mo->health > 0
                && ( mo->flags & MF_COUNTKILL || mo->type == MT_SKULL) )
            {
                tclass = (mo->flags & MF_FRIEND)? TH_friends : TH_enemies;
            }
        }
    }
   
    // Remove from current class-list, if in one.
    th = thinker->cnext;
    if( th != NULL)
    {
        th->cprev = thinker->cprev;
        th->cprev->cnext = th;
    }

    // Prevent linking dead mobj.
    if( thinker->function.acp1 == (actionf_p1)T_RemoveThinker
        || tclass >= NUMTHCLASS )  // TH_none, etc.
    {
        // Not in any class-list.
        // Prevent unlinking again.
        thinker->cprev = thinker->cnext = NULL;
        return;
    }

    // Add to the appropriate class-list.
    th = &thinkerclasscap[tclass];
    thinker->cnext = th;
    thinker->cprev = th->cprev;
    th->cprev->cnext = thinker;
    th->cprev = thinker;
    return;

}

// Move to be first or last.
//  first : 0=last, 1=first.
void P_MoveClassThink(thinker_t *thinker, byte first)
{
    register thinker_t * th;

    // Remove from current thread, if in one.
    th = thinker->cnext;
    if( th != NULL)
    {
        th->cprev = thinker->cprev;
        thinker->cprev->cnext = th;
    }

    // prevent linking dead mobj
    if( thinker->function.acp1 == (actionf_p1)T_RemoveThinker )
    {
        // Not in any class-list.
        // Prevent unlinking again.
        thinker->cprev = thinker->cnext = NULL;
        return;
    }
   
    // Add to appropriate thread list.
    register mobj_t * mo = (mobj_t *) thinker;
    th = &thinkerclasscap[ (mo->flags & MF_FRIEND)? TH_friends : TH_enemies ];
    if( first )
    {
        thinker->cprev = th;
        thinker->cnext = th->cnext;
        th->cnext->cprev = thinker;
        th->cnext = thinker;
    }
    else
    {   // Last
        thinker->cnext = th;
        thinker->cprev = th->cprev;
        th->cprev->cnext = thinker;
        th->cprev = thinker;
    }
}


// Move range cap to th, to be last in class-list.
//  cap: is a class-list.
//  thnext: becomes new first in class-list.
void P_MoveClasslistRangeLast( thinker_t * cap, thinker_t * thnext )
{
    // cap is head of a class-list.
    // Link first in class-list to end of class-list.
    cap->cnext->cprev = cap->cprev;
    cap->cprev->cnext = cap->cnext;
    // Break list before th.  Make thnext first in class-list.
    register thinker_t * tp = thnext->cprev;
    cap->cprev = tp;
    tp->cnext = cap;
    thnext->cprev = cap;
    cap->cnext = thnext;
}


#ifdef REFERENCE_COUNTING
//
// P_SetReference
//
// This function is used to keep track of pointer references to mobj thinkers.
// In Doom, objects such as lost souls could sometimes be removed despite
// their still being referenced. In Boom, 'target' mobj fields were tested
// during each gametic, and any objects pointed to by them would be prevented
// from being removed. But this was incomplete, and was slow (every mobj was
// checked during every gametic). Now, we keep a count of the number of
// references, and delay removal until the count is 0.

//  rm_mo: remove reference
//  add_mo:  add reference
void P_SetReference(mobj_t *rm_mo, mobj_t *add_mo)
{
  if(rm_mo)  // If there was a target already, decrease its refcount
    rm_mo->thinker.references--;
  if(targ)  // Set new target and if non-NULL, increase its counter
    add_mo->thinker.references++;
}
#endif


//
// P_RunThinkers
//
void P_RunThinkers (void)
{
    thinker_t*  currentthinker;
    thinker_t*  next_thinker;

    currentthinker = thinkercap.next;
    while (currentthinker != &thinkercap)
    {
        next_thinker = currentthinker->next;  // because of T_RemoveThinker
#ifdef THINKER_INTERPOLATIONS
        if (newthinkerpresent)
            R_ActivateThinkerInterpolations(currentthinker);
#endif
        if (currentthinker->function.acp1)
        {
            currentthinker->function.acp1 (currentthinker);
        }
        currentthinker = next_thinker;
    }
#ifdef THINKER_INTERPOLATIONS
    newthinkerpresent = false;
#endif
}



//
// P_Ticker
//

void P_Ticker (void)
{
    int  i;

    // [WDJ] From PrBoom, adapted to game_comp_tic.
    // Pause if in menu and at least one tic has been run.
    // killough 9/29/98: note that this ties in with basetic,
    // since G_Ticker does the pausing during recording or playback,
    // and compensates by incrementing basetic (not incrementing game_comp_tic).
    // All of this complicated mess is used to preserve demo sync.
    // PrBoom and EternityEngine test (players[consoleplayer].viewz != 1)
    // as test that one tic has run.
    // Heretic only has paused.
    if (paused
        || (menuactive && !netgame && !demoplayback
            && (players[consoleplayer].viewz != 1) ))
        return;

#ifdef THINKER_INTERPOLATIONS
    R_UpdateInterpolations();
#endif

    // From PrBoom, EternityEngine, may affect demo sync.
    // Not if this is an intermission screen.
    if( gamestate == GS_LEVEL || gamestate == GS_DEDICATEDSERVER )
    {
        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (playeringame[i])
                P_PlayerThink (&players[i]);
        }
    }

    P_RunThinkers ();
    P_UpdateSpecials ();
    if( cv_itemrespawn.EV )  P_RespawnSpecials ();

    P_AmbientSound();

    // for par times
    leveltime++;

#ifdef FRAGGLESCRIPT
    // SoM: Update FraggleScript...
    T_DelayedScripts();
#endif
}
