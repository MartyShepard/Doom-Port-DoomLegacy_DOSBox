// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: p_user.c 834 2011-05-27 00:48:32Z wesleyjohnson $
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
// $Log: p_user.c,v $
// Revision 1.17  2004/07/27 08:19:37  exl
// New fmod, fs functions, bugfix or 2, patrol nodes
//
// Revision 1.16  2003/07/14 12:37:54  darkwolf95
// Fixed bug where frags don't display for Player 2 on death while in splitscreen.
//
// Revision 1.15  2001/05/27 13:42:48  bpereira
// no message
//
// Revision 1.14  2001/04/04 20:24:21  judgecutor
// Added support for the 3D Sound
//
// Revision 1.13  2001/03/03 06:17:33  bpereira
// no message
//
// Revision 1.12  2001/01/27 11:02:36  bpereira
// no message
//
// Revision 1.11  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.10  2000/11/04 16:23:43  bpereira
// no message
//
// Revision 1.9  2000/11/02 17:50:09  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.8  2000/10/21 08:43:31  bpereira
// no message
//
// Revision 1.7  2000/08/31 14:30:56  bpereira
// no message
//
// Revision 1.6  2000/08/03 17:57:42  bpereira
// no message
//
// Revision 1.5  2000/04/23 16:19:52  bpereira
// no message
//
// Revision 1.4  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.3  2000/03/29 19:39:48  bpereira
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
//      Player related stuff.
//      Bobbing POV/weapon, movement.
//      Pending weapon.
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "d_event.h"
#include "g_game.h"
#include "p_local.h"
#include "r_main.h"
#include "s_sound.h"
#include "p_setup.h"
#include "p_inter.h"
#include "m_random.h"

#include "hardware/hw3sound.h"


// Index of the special effects (INVUL inverse) map.
#define INVERSECOLORMAP         32


//
// Movement.
//

// 16 pixels of bob
#define MAXBOB  0x100000

//added:22-02-98: initial momz when player jumps (moves up)
fixed_t JUMPGRAVITY = (6*FRACUNIT/NEWTICRATERATIO);

boolean  onground;
int	 extramovefactor = 0;


//
// P_Thrust
// Moves the given origin along a given angle.
//
void P_Thrust(player_t *player, angle_t angle, fixed_t move)
{
    angle >>= ANGLETOFINESHIFT;
#if 0
    // friction and movefactor are now sector attributes
    if(player->mo->subsector->sector->special == 15  // heretic ice
       && !(player->powers[pw_flight] && (player->mo->z > player->mo->floorz)))
    {
        move>>=2;  // Friction_Low
    }
#endif   
    player->mo->momx += FixedMul(move, finecosine[angle]);
    player->mo->momy += FixedMul(move, finesine[angle]);
}

#ifdef CLIENTPREDICTION2
//
// P_ThrustSpirit
// Moves the given origin along a given angle.
//
void P_ThrustSpirit(player_t *player, angle_t angle, fixed_t move)
{
    angle >>= ANGLETOFINESHIFT;
#if 0   
    // friction and movefactor are now sector attributes
    if(player->spirit->subsector->sector->special == 15
    && !(player->powers[pw_flight] && !(player->spirit->z <= player->spirit->floorz))) // Friction_Low
    {
        move>>=2;  // Friction_Low
    }
#endif   
    player->spirit->momx += FixedMul(move, finecosine[angle]);
    player->spirit->momy += FixedMul(move, finesine[angle]);
}
#endif


//
// P_CalcHeight
// Calculate the walking / running height adjustment
//
void P_CalcHeight (player_t* player)
{
    int         angle;
    fixed_t     bob;
    fixed_t     viewheight;
    mobj_t      * pmo = player->mo;
    mobj_t      * smo = pmo;  // spirit (same as pmo unless using CLIENTPREDICTION2)

    // Regular movement bobbing
    // (needs to be calculated for gun swing
    // even if not on ground)
    // OPTIMIZE: tablify angle
    // Note: a LUT allows for effects
    //  like a ramp with low health.


#ifdef CLIENTPREDICTION2
    if( player->spirit )
        smo = player->spirit;
#endif

    player->bob = ((FixedMul (smo->momx, smo->momx)
                   +FixedMul (smo->momy, smo->momy))*NEWTICRATERATIO)>>2;

    if (player->bob>MAXBOB)
        player->bob = MAXBOB;

    if( pmo->flags2&MF2_FLY && !onground )
        player->bob = FRACUNIT/2;

    if ((player->cheats & CF_NOMOMENTUM) || smo->z > smo->floorz)
    {
        //added:15-02-98: it seems to be useless code!
        //player->viewz = pmo->z + (cv_viewheight.value<<FRACBITS);

        //if (player->viewz > pmo->ceilingz-4*FRACUNIT)
        //    player->viewz = pmo->ceilingz-4*FRACUNIT;
        player->viewz = smo->z + player->viewheight;
        return;
    }

    angle = (FINEANGLES/20*localgametic/NEWTICRATERATIO)&FINEMASK;
    bob = FixedMul ( player->bob/2, finesine[angle]);


    // move viewheight
    viewheight = cv_viewheight.value << FRACBITS; // default eye view height

    if (player->playerstate == PST_LIVE)
    {
        player->viewheight += player->deltaviewheight;

        if (player->viewheight > viewheight)
        {
            player->viewheight = viewheight;
            player->deltaviewheight = 0;
        }

        if (player->viewheight < viewheight/2)
        {
            player->viewheight = viewheight/2;
            if (player->deltaviewheight <= 0)
                player->deltaviewheight = 1;
        }

        if (player->deltaviewheight)
        {
            player->deltaviewheight += FRACUNIT/4;
            if (!player->deltaviewheight)
                player->deltaviewheight = 1;
        }
    }   

    if(player->chickenTics)
        player->viewz = smo->z + player->viewheight-(20*FRACUNIT);
    else
        player->viewz = smo->z + player->viewheight + bob;

    if(pmo->flags2&MF2_FEETARECLIPPED
        && player->playerstate != PST_DEAD
        && pmo->z <= pmo->floorz)
    {
        player->viewz -= FOOTCLIPSIZE;
    }

    if (player->viewz > smo->ceilingz-4*FRACUNIT)
        player->viewz = smo->ceilingz-4*FRACUNIT;
    if (player->viewz < smo->floorz+4*FRACUNIT)
        player->viewz = smo->floorz+4*FRACUNIT;

}


extern int ticruned,ticmiss;

byte  EN_move_doom = 0;
#ifdef ABSOLUTEANGLE
byte  EN_cmd_abs_angle = 1;  // legacy absolute angle commands
#endif

// local version control
void DemoAdapt_p_user( void )
{
    EN_move_doom =
       (gamemode != heretic)
       && ( (demoversion<128)  // legacy demo and orig doom
	    || (demoversion>=200 && demoversion <=202) // boom demo
	    );

#ifdef ABSOLUTEANGLE
    // abs angle in legacy demos only
    EN_cmd_abs_angle = (demoversion >= 125) && (demoversion < 200);
#endif
}


//
// P_MovePlayer
//
void P_MovePlayer (player_t* player)
{
    mobj_t *   pmo = player->mo;
    ticcmd_t*  cmd = &player->cmd;
    int  movefactor = ORIG_FRICTION_FACTOR; // default

#ifdef ABSOLUTEANGLE
    if(EN_cmd_abs_angle)
        pmo->angle = (cmd->angleturn<<16);
    else
        pmo->angle += (cmd->angleturn<<16);
#else
    pmo->angle += (cmd->angleturn<<16);
#endif

    ticruned++;
    if( (cmd->angleturn & TICCMD_RECEIVED) == 0)
        ticmiss++;
    // Do not let the player control movement
    //  if not onground.
    onground = (pmo->z <= pmo->floorz) 
               || (player->cheats & CF_FLYAROUND)   // cheat
               || (pmo->flags2&(MF2_ONMOBJ|MF2_FLY));  // heretic

    if(variable_friction && onground)
    {
        movefactor = P_GetMoveFactor(pmo); // gets got_movefactor, got_friction
//        CONS_Printf("friction: %X, movefactor: %i\n", got_friction, movefactor);
    }

    if( EN_move_doom )
    {
        // Doom and Boom movement
        boolean  jumpover = player->cheats & CF_JUMPOVER;
        if (cmd->forwardmove && (onground || jumpover))
        {
            // dirty hack to let the player avatar walk over a small wall
            // while in the air
            if (jumpover)
	    {
	        if(pmo->momz > 0)
		    P_Thrust (player, pmo->angle, 5*movefactor);
	    }
            else
	    {
	        P_Thrust (player, pmo->angle, cmd->forwardmove*movefactor);
	    }
        }
    
        if (cmd->sidemove && onground)
        {
            P_Thrust (player, pmo->angle-ANG90, cmd->sidemove*movefactor);
	}

        player->aiming = (signed char)cmd->aiming;
    }
    else
    {
        // most current
        fixed_t   movepushforward=0, movepushside=0;
        player->aiming = cmd->aiming<<16;
        if( player->chickenTics )
        {
	    // [WDJ] Moved to after other movefactor, so it can have some effect.
            // movefactor = 2500;  // heretic chicken
	    // Modify movefactor to chicken size, (chicken on ice)
	    movefactor = movefactor * 625 / 512;
	      // * 2500 / 2048
	}

        if (cmd->forwardmove)
        {
            movepushforward = cmd->forwardmove * (movefactor + extramovefactor);
        
            if (pmo->eflags & MF_UNDERWATER)
            {
                // half forward speed when waist under water
                // a little better grip if feets touch the ground
                if (!onground)
                    movepushforward >>= 1;
                else
                    movepushforward = movepushforward *3/4;
            }
            else
            {
                // allow very small movement while in air for gameplay
                if (!onground)
                    movepushforward >>= 3;
            }

            P_Thrust (player, pmo->angle, movepushforward);
        }

        if (cmd->sidemove)
        {
            movepushside = cmd->sidemove * (movefactor + extramovefactor);
            if (pmo->eflags & MF_UNDERWATER)
            {
                if (!onground)
                    movepushside >>= 1;
                else
                    movepushside = movepushside *3/4;
            }
            else
	    {
                if (!onground)
                    movepushside >>= 3;
	    }

            P_Thrust (player, pmo->angle-ANG90, movepushside);
        }

        // mouselook swim when waist underwater
        pmo->eflags &= ~MF_SWIMMING;
        if (pmo->eflags & MF_UNDERWATER)
        {
            fixed_t a;
            // swim up/down full move when forward full speed
            a = FixedMul( movepushforward*50, finesine[ (player->aiming>>ANGLETOFINESHIFT) ] >>5 );
            
            if ( a != 0 ) {
                pmo->eflags |= MF_SWIMMING;
                pmo->momz += a;
            }
        }
    }

    //added:22-02-98: jumping
    if (cmd->buttons & BT_JUMP)
    {
        if( pmo->flags2&MF2_FLY )
            player->flyheight = 10;
        else 
        if(pmo->eflags & MF_UNDERWATER)
            //TODO: goub gloub when push up in water
            pmo->momz = JUMPGRAVITY/2;
        else 
        // can't jump while in air, can't jump while jumping
        if( onground && !(player->jumpdown & 1))
        {
            pmo->momz = JUMPGRAVITY;
            if( !(player->cheats & CF_FLYAROUND) )
            {
                S_StartScreamSound (pmo, sfx_jump);
                // keep jumping ok if FLY mode.
                player->jumpdown |= 1;
            }
        }
    }
    else
        player->jumpdown &= ~1;


    if (cmd->forwardmove || cmd->sidemove)
    {
        if( player->chickenTics )
        {
            if( pmo->state == &states[S_CHICPLAY])
                P_SetMobjState(pmo, S_CHICPLAY_RUN1);
        }
        else
            if(pmo->state == &states[S_PLAY])
                P_SetMobjState(pmo, S_PLAY_RUN1);
    }
    if( (gamemode == heretic) && (cmd->angleturn & BT_FLYDOWN) )
    {
        player->flyheight = -10;
    }
/* HERETODO
    fly = cmd->lookfly>>4;
    if(fly > 7)
        fly -= 16;
    if(fly && player->powers[pw_flight])
    {
        if(fly != TOCENTER)
        {
            player->flyheight = fly*2;
            if(!(pmo->flags2&MF2_FLY))
            {
                pmo->flags2 |= MF2_FLY;
                pmo->flags |= MF_NOGRAVITY;
            }
        }
        else
        {
            pmo->flags2 &= ~MF2_FLY;
            pmo->flags &= ~MF_NOGRAVITY;
        }
    }
    else if(fly > 0)
    {
        P_PlayerUseArtifact(player, arti_fly);
    }*/
    if(pmo->flags2&MF2_FLY)
    {
        pmo->momz = player->flyheight*FRACUNIT;
        if(player->flyheight)
            player->flyheight /= 2;
    }
}



//
// P_DeathThink
// Fall on your face when dying.
// Decrease POV height to floor height.
//
#define ANG5    (ANG90/18)

void P_DeathThink (player_t* player)
{
    mobj_t * pmo = player->mo;
    angle_t  angle;

    P_MovePsprites (player);

    // fall to the ground
    if (player->viewheight > 6*FRACUNIT)
        player->viewheight -= FRACUNIT;

    if (player->viewheight < 6*FRACUNIT)
        player->viewheight = 6*FRACUNIT;

    player->deltaviewheight = 0;
    onground = pmo->z <= pmo->floorz;

    P_CalcHeight (player);

    mobj_t *attacker = player->attacker;

    // watch my killer (if there is one)
    if (attacker && attacker != pmo)
    {
        angle = R_PointToAngle2 (pmo->x,
                                 pmo->y,
                                 player->attacker->x,
                                 player->attacker->y);

        angle_t delta = angle - pmo->angle;

        if (delta < ANG5 || delta > (unsigned)-ANG5)
        {
            // Looking at killer,
            //  so fade damage flash down.
            pmo->angle = angle;

            if (player->damagecount)
                player->damagecount--;
        }
        else if (delta < ANG180)
            pmo->angle += ANG5;
        else
            pmo->angle -= ANG5;

        //added:22-02-98:
        // change aiming to look up or down at the attacker (DOESNT WORK)
        // FIXME : the aiming returned seems to be too up or down... later
	/*
	fixed_t dist = P_AproxDistance(attacker->x - player->mo->x, attacker->y - player->mo->y);
	fixed_t dz = attacker->z +(attacker->height>>1) -player->mo->z;
	angle_t pitch = 0;
	if (dist)
	  pitch = ArcTan(FixedDiv(dz, dist));
	*/
	int32_t pitch = (attacker->z - player->mo->z)>>17;
	player->aiming = G_ClipAimingPitch(pitch);

    }
    else if (player->damagecount)
        player->damagecount--;

    if (player->cmd.buttons & BT_USE)
    {
        player->playerstate = PST_REBORN;
        pmo->special2 = 666;
    }
}

//----------------------------------------------------------------------------
//
// PROC P_ChickenPlayerThink
//
//----------------------------------------------------------------------------

void P_ChickenPlayerThink(player_t *player)
{
        mobj_t * pmo = player->mo;

        if(player->health > 0)
        { // Handle beak movement
                P_UpdateBeak(player, &player->psprites[ps_weapon]);
        }
        if(player->chickenTics&15)
        {
                return;
        }
        if(!(pmo->momx+pmo->momy) && P_Random() < 160)
        { // Twitch view angle
                pmo->angle += P_SignedRandom()<<19;
        }
        if((pmo->z <= pmo->floorz) && (P_Random() < 32))
        { // Jump and noise
                pmo->momz += FRACUNIT;
                P_SetMobjState(pmo, S_CHICPLAY_PAIN);
                return;
        }
        if(P_Random() < 48)
        { // Just noise
                S_StartScreamSound(pmo, sfx_chicact);
        }
}

//----------------------------------------------------------------------------
//
// FUNC P_UndoPlayerChicken
//
//----------------------------------------------------------------------------

boolean P_UndoPlayerChicken(player_t *player)
{
        mobj_t *fog;
        mobj_t *mo;
        mobj_t * pmo = player->mo;
        fixed_t x;
        fixed_t y;
        fixed_t z;
        angle_t angle;
        int playerNum;
        weapontype_t weapon;
        int oldFlags;
        int oldFlags2;

        x = pmo->x;
        y = pmo->y;
        z = pmo->z;
        angle = pmo->angle;
        weapon = pmo->special1;
        oldFlags = pmo->flags;
        oldFlags2 = pmo->flags2;
        P_SetMobjState(pmo, S_FREETARGMOBJ);
        mo = P_SpawnMobj(x, y, z, MT_PLAYER);
        if(P_TestMobjLocation(mo) == false)
        { // Didn't fit
                P_RemoveMobj(mo);
                mo = P_SpawnMobj(x, y, z, MT_CHICPLAYER);
                mo->angle = angle;
                mo->health = player->health;
                mo->special1 = weapon;
                mo->player = player;
                mo->flags = oldFlags;
                mo->flags2 = oldFlags2;
                player->mo = mo;
                player->chickenTics = 2*35;
                return(false);
        }
        playerNum = player-players;
        if(playerNum != 0)
        { // Set color translation
                mo->flags |= playerNum<<MF_TRANSSHIFT;
        }
        mo->angle = angle;
        mo->player = player;
        mo->reactiontime = 18;
        if(oldFlags2&MF2_FLY)
        {
                mo->flags2 |= MF2_FLY;
                mo->flags |= MF_NOGRAVITY;
        }
        player->chickenTics = 0;
        player->powers[pw_weaponlevel2] = 0;
        player->weaponinfo = wpnlev1info;
        player->health = mo->health = MAXHEALTH;
        player->mo = mo;
        angle >>= ANGLETOFINESHIFT;
        fog = P_SpawnMobj(x+20*finecosine[angle],
                y+20*finesine[angle], z+TELEFOGHEIGHT, MT_TFOG);
        S_StartSound(fog, sfx_telept);
        P_PostChickenWeapon(player, weapon);
        return(true);
}

//
// P_MoveCamera : make sure the camera is not outside the world
//                and looks at the player avatar
//

camera_t camera;

//#define VIEWCAM_DIST    (128<<FRACBITS)
//#define VIEWCAM_HEIGHT  (20<<FRACBITS)

consvar_t cv_cam_dist   = {"cam_dist"  ,"128"  ,CV_FLOAT,NULL};
consvar_t cv_cam_height = {"cam_height", "20"   ,CV_FLOAT,NULL};
consvar_t cv_cam_speed  = {"cam_speed" ,  "0.25",CV_FLOAT,NULL};

void P_ResetCamera (player_t *player)
{
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;

    camera.chase = true;
    x = player->mo->x;
    y = player->mo->y;
    z = player->mo->z + (cv_viewheight.value<<FRACBITS);

    // hey we should make sure that the sounds are heard from the camera
    // instead of the marine's head : TO DO

    // set bits for the camera
    if (!camera.mo)
        camera.mo = P_SpawnMobj (x,y,z, MT_CHASECAM);
    else
    {
        camera.mo->x = x;
        camera.mo->y = y;
        camera.mo->z = z;
    }

    camera.mo->angle = player->mo->angle;
    camera.aiming = 0;
}

boolean PTR_FindCameraPoint (intercept_t* in)
{
/*    int         side;
    fixed_t             slope;
    fixed_t             dist;
    line_t*             li;

    li = in->d.line;

    if ( !(li->flags & ML_TWOSIDED) )
        return false;

    // crosses a two sided line
    //added:16-02-98: Fab comments : sets opentop, openbottom, openrange
    //                lowfloor is the height of the lowest floor
    //                         (be it front or back)
    P_LineOpening (li);

    dist = FixedMul (attackrange, in->frac);

    if (li->frontsector->floorheight != li->backsector->floorheight)
    {
        //added:18-02-98: comments :
        // find the slope aiming on the border between the two floors
        slope = FixedDiv (openbottom - cameraz , dist);
        if (slope > aimslope)
            return false;
    }

    if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
    {
        slope = FixedDiv (opentop - shootz , dist);
        if (slope < aimslope)
            goto hitline;
    }

    return true;

    // hit line
  hitline:*/
    // stop the search
    return false;
}


fixed_t cameraz;

void P_MoveChaseCamera (player_t *player)
{
    angle_t             angle;
    fixed_t             x,y,z ,viewpointx,viewpointy;
    fixed_t             dist;
    mobj_t*             pmo = player->mo;
    subsector_t*        newsubsec;
    float               f1,f2;

    if (!camera.mo)
        P_ResetCamera (player);

    angle = pmo->angle;

    // sets ideal cam pos
    dist  = cv_cam_dist.value;
    x = pmo->x - FixedMul( finecosine[(angle>>ANGLETOFINESHIFT) & FINEMASK], dist);
    y = pmo->y - FixedMul(   finesine[(angle>>ANGLETOFINESHIFT) & FINEMASK], dist);
    z = pmo->z + (cv_viewheight.value<<FRACBITS) + cv_cam_height.value;

/*    P_PathTraverse ( pmo->x, pmo->y, x, y, PT_ADDLINES, PTR_UseTraverse );*/

    // move camera down to move under lower ceilings
    newsubsec = R_IsPointInSubsector ((pmo->x + camera.mo->x)>>1,(pmo->y + camera.mo->y)>>1);
              
    if (!newsubsec)
    {
        // use player sector 
        if (pmo->subsector->sector->ceilingheight - camera.mo->height < z)
            z = pmo->subsector->sector->ceilingheight - camera.mo->height-11*FRACUNIT; // don't be blocked by a opened door
    }
    else
    // camera fit ?
    if (newsubsec->sector->ceilingheight - camera.mo->height < z)
        // no fit
        z = newsubsec->sector->ceilingheight - camera.mo->height-11*FRACUNIT;
        // is the camera fit is there own sector
    newsubsec = R_PointInSubsector (camera.mo->x,camera.mo->y);
    if (newsubsec->sector->ceilingheight - camera.mo->height < z)
        z = newsubsec->sector->ceilingheight - camera.mo->height-11*FRACUNIT;


    // point viewed by the camera
    // this point is just 64 unit forward the player
    dist = 64 << FRACBITS;
    viewpointx = pmo->x + FixedMul( finecosine[(angle>>ANGLETOFINESHIFT) & FINEMASK], dist);
    viewpointy = pmo->y + FixedMul( finesine[(angle>>ANGLETOFINESHIFT) & FINEMASK], dist);

    camera.mo->angle = R_PointToAngle2(camera.mo->x,camera.mo->y,
                                       viewpointx  ,viewpointy);

    // folow the player
    camera.mo->momx = FixedMul(x - camera.mo->x,cv_cam_speed.value);
    camera.mo->momy = FixedMul(y - camera.mo->y,cv_cam_speed.value);
    camera.mo->momz = FixedMul(z - camera.mo->z,cv_cam_speed.value);

    // compute aming to look the viewed point
    f1=FIXED_TO_FLOAT(viewpointx-camera.mo->x);
    f2=FIXED_TO_FLOAT(viewpointy-camera.mo->y);
    dist=sqrt(f1*f1+f2*f2)*FRACUNIT;
    angle=R_PointToAngle2(0, camera.mo->z, dist,
                         pmo->z+(pmo->height>>1)+finesine[(player->aiming>>ANGLETOFINESHIFT) & FINEMASK] * 64);

    angle = G_ClipAimingPitch(angle);
    dist=camera.aiming-angle;
    camera.aiming-=(dist>>3);
}


byte weapontobutton[NUMWEAPONS]={wp_fist    <<BT_WEAPONSHIFT,
                                 wp_pistol  <<BT_WEAPONSHIFT,
                                 wp_shotgun <<BT_WEAPONSHIFT,
                                 wp_chaingun<<BT_WEAPONSHIFT,
                                 wp_missile <<BT_WEAPONSHIFT,
                                 wp_plasma  <<BT_WEAPONSHIFT,
                                 wp_bfg     <<BT_WEAPONSHIFT,
                                (wp_fist    <<BT_WEAPONSHIFT) | BT_EXTRAWEAPON,// wp_chainsaw
                                (wp_shotgun <<BT_WEAPONSHIFT) | BT_EXTRAWEAPON};//wp_supershotgun

#ifdef CLIENTPREDICTION2

void CL_ResetSpiritPosition(mobj_t *mobj)
{
    P_UnsetThingPosition(mobj->player->spirit);
    mobj->player->spirit->x=mobj->x;
    mobj->player->spirit->y=mobj->y;
    mobj->player->spirit->z=mobj->z;
    mobj->player->spirit->momx=0;
    mobj->player->spirit->momy=0;
    mobj->player->spirit->momz=0;
    mobj->player->spirit->angle=mobj->angle;
    P_SetThingPosition(mobj->player->spirit);
}

void P_ProcessCmdSpirit (player_t* player,ticcmd_t *cmd)
{
    fixed_t   movepushforward=0, movepushside=0;
#ifdef PARANOIA
    if(!player)
        I_Error("P_MoveSpirit : player null");
    if(!player->spirit)
        I_Error("P_MoveSpirit : player->spirit null");
    if(!cmd)
        I_Error("P_MoveSpirit : cmd null");
#endif

    // don't move if dead
    if( player->playerstate != PST_LIVE )
    {
        cmd->angleturn &= ~TICCMD_XY;
        return;
    }
    onground = (player->spirit->z <= player->spirit->floorz) ||
               (player->cheats & CF_FLYAROUND);

    if (player->spirit->reactiontime)
    {
        player->spirit->reactiontime--;
        return;
    }

    player->spirit->angle = cmd->angleturn<<16;
    cmd->angleturn |= TICCMD_XY;
/*
    // now weapon is allways send change is detected at receiver side
    if(cmd->buttons & BT_CHANGE) 
    {
        player->spirit->movedir = cmd->buttons & (BT_WEAPONMASK | BT_EXTRAWEAPON);
        cmd->buttons &=~BT_CHANGE;
    }
    else
    {
        if( player->pendingweapon!=wp_nochange )
            player->spirit->movedir=weapontobutton[player->pendingweapon];
        cmd->buttons&=~(BT_WEAPONMASK | BT_EXTRAWEAPON);
        cmd->buttons|=player->spirit->movedir;
    }
*/
    if (cmd->forwardmove)
    {
        movepushforward = cmd->forwardmove * movefactor;
        
        if (player->spirit->eflags & MF_UNDERWATER)
        {
            // half forward speed when waist under water
            // a little better grip if feets touch the ground
            if (!onground)
                movepushforward >>= 1;
            else
                movepushforward = movepushforward *3/4;
        }
        else
        {
            // allow very small movement while in air for gameplay
            if (!onground)
                movepushforward >>= 3;
        }
        
        P_ThrustSpirit (player->spirit, player->spirit->angle, movepushforward);
    }
    
    if (cmd->sidemove)
    {
        movepushside = cmd->sidemove * movefactor;
        if (player->spirit->eflags & MF_UNDERWATER)
        {
            if (!onground)
                movepushside >>= 1;
            else
                movepushside = movepushside *3/4;
        }
        else 
            if (!onground)
                movepushside >>= 3;
            
        P_ThrustSpirit (player->spirit, player->spirit->angle-ANG90, movepushside);
    }
    
    // mouselook swim when waist underwater
    player->spirit->eflags &= ~MF_SWIMMING;
    if (player->spirit->eflags & MF_UNDERWATER)
    {
        fixed_t a;
        // swim up/down full move when forward full speed
        a = FixedMul( movepushforward*50, finesine[ (cmd->aiming>>(ANGLETOFINESHIFT-16)) ] >>5 );
        
        if ( a != 0 ) {
            player->spirit->eflags |= MF_SWIMMING;
            player->spirit->momz += a;
        }
    }

    //added:22-02-98: jumping
    if (cmd->buttons & BT_JUMP)
    {
        // can't jump while in air, can't jump while jumping
        if (!(player->jumpdown & 2) &&
             (onground || (player->spirit->eflags & MF_UNDERWATER)) )
        {
            if (onground)
                player->spirit->momz = JUMPGRAVITY;
            else //water content
                player->spirit->momz = JUMPGRAVITY/2;

            //TODO: goub gloub when push up in water
            
            if ( !(player->cheats & CF_FLYAROUND) && onground && !(player->spirit->eflags & MF_UNDERWATER))
            {
                S_StartScreamSound(player->spirit, sfx_jump);

                // keep jumping ok if FLY mode.
                player->jumpdown |= 2;
            }
        }
    }
    else
        player->jumpdown &= ~2;

}

void P_MoveSpirit (player_t* p,ticcmd_t *cmd, int realtics)
{
    if( gamestate != GS_LEVEL )
        return;
    if(p->spirit)
    {
        extern boolean supdate;
        int    i;

        p->spirit->flags|=MF_SOLID;
        for(i=0;i<realtics;i++)
        {
            P_ProcessCmdSpirit(p,cmd);
            P_MobjThinker(p->spirit);
        }                 
        p->spirit->flags&=~MF_SOLID;
        P_CalcHeight (p);                 // z-bobing of player
        A_TicWeapon(p, &p->psprites[0]);  // bobing of weapon
        cmd->x=p->spirit->x;
        cmd->y=p->spirit->y;
        supdate=true;
    }
    else
    if(p->mo)
    {
        cmd->x=p->mo->x;
        cmd->y=p->mo->y;
    }
}

#endif


//
// P_PlayerThink
//

boolean playerdeadview; //Fab:25-04-98:show dm rankings while in death view

void P_PlayerThink (player_t* player)
{
    ticcmd_t*           cmd;
    weapontype_t        newweapon;
    mobj_t *  pmo = player->mo;

#ifdef PARANOIA
    if(!pmo) I_Error("p_playerthink : players[%d].mo == NULL",player-players);
#endif

    // fixme: do this in the cheat code
    if (player->cheats & CF_NOCLIP)
        pmo->flags |= MF_NOCLIP;
    else
        pmo->flags &= ~MF_NOCLIP;

    // chain saw run forward
    cmd = &player->cmd;
    if (pmo->flags & MF_JUSTATTACKED)
    {
// added : now angle turn is a absolute value not relative
#ifndef ABSOLUTEANGLE
        cmd->angleturn = 0;
#endif
        cmd->forwardmove = 0xc800/512;
        cmd->sidemove = 0;
        pmo->flags &= ~MF_JUSTATTACKED;
    }

    if (player->playerstate == PST_REBORN)
    {
#ifdef PARANOIA
        I_SoftError("player %d is in PST_REBORN\n");
#endif
        // it is not "normal" but far to be critical
        return;
    }

    if (player->playerstate == PST_DEAD)
    {
        //Fab:25-04-98: show the dm rankings while dead, only in deathmatch
		//DarkWolf95:July 03, 2003:fixed bug where rankings only show on player1's death
        if (player== displayplayer_ptr
	    || player== displayplayer2_ptr ) // NULL when unused
            playerdeadview = true;

        P_DeathThink (player);

        //added:26-02-98:camera may still move when guy is dead
        if (camera.chase)
            P_MoveChaseCamera ( displayplayer_ptr );
        return;
    }
    else
    {
        if ( player== displayplayer_ptr )
            playerdeadview = false;
    }
    if( player->chickenTics )
        P_ChickenPlayerThink(player);

    // check water content, set stuff in mobj
    P_MobjCheckWater (pmo);

    // Move around.
    // Reactiontime is used to prevent movement
    //  for a bit after a teleport.
    if (pmo->reactiontime)
        pmo->reactiontime--;
    else
        P_MovePlayer (player);

    //added:22-02-98: bob view only if looking by the marine's eyes
#ifndef CLIENTPREDICTION2
    if (!camera.chase)
        P_CalcHeight (player);
#endif

    //added:26-02-98: calculate the camera movement
    if (camera.chase && player== displayplayer_ptr)
        P_MoveChaseCamera ( displayplayer_ptr );

    // check special sectors : damage & secrets
    P_PlayerInSpecialSector (player);

    //
    // TODO water splashes
    //
#if 0
    if (demoversion>=125 && player->specialsector == )
    {
        if ((pmo->momx >  (2*FRACUNIT) ||
             pmo->momx < (-2*FRACUNIT) ||
             pmo->momy >  (2*FRACUNIT) ||
             pmo->momy < (-2*FRACUNIT) ||
             pmo->momz >  (2*FRACUNIT)) &&  // jump out of water
             !(gametic % (32 * NEWTICRATERATIO)) )
        {
            //
            // make sure we disturb the surface of water (we touch it)
            //
	    int waterz = pmo->subsector->sector->floorheight + (FRACUNIT/4);

            // half in the water
            if(pmo->eflags & MF_TOUCHWATER)
            {
                if (pmo->z <= pmo->floorz) // onground
                {
                    fixed_t whater_height = waterz - pmo->subsector->sector->floorheight;

                    if( whater_height < (pmo->height>>2))
                        S_StartSound (pmo, sfx_splash);
                    else
                        S_StartSound (pmo, sfx_floush);
                }
                else
                    S_StartSound (pmo, sfx_floush);
            }                   
        }
    }
#endif

    // Check for weapon change.
//#ifndef CLIENTPREDICTION2
    if (cmd->buttons & BT_CHANGE)
//#endif
    {

        // The actual changing of the weapon is done
        //  when the weapon psprite can do it
        //  (read: not in the middle of an attack).
        newweapon = (cmd->buttons&BT_WEAPONMASK)>>BT_WEAPONSHIFT;
        if(demoversion<128)
        {
            if (newweapon == wp_fist
                && player->weaponowned[wp_chainsaw]
                && !(player->readyweapon == wp_chainsaw
                     && player->powers[pw_strength]))
            {
                newweapon = wp_chainsaw;
            }
        
            if ( (gamemode == doom2_commercial)
                && newweapon == wp_shotgun
                && player->weaponowned[wp_supershotgun]
                && player->readyweapon != wp_supershotgun)
            {
                newweapon = wp_supershotgun;
            }
        }
        else
        {
            if(cmd->buttons&BT_EXTRAWEAPON)
               switch(newweapon) {
                  case wp_shotgun : 
                       if( gamemode == doom2_commercial && player->weaponowned[wp_supershotgun])
                           newweapon = wp_supershotgun;
                       break;
                  case wp_fist :
                       if( player->weaponowned[wp_chainsaw])
                           newweapon = wp_chainsaw;
                       break;
                  default:
                       break;
               }
        }

        if (player->weaponowned[newweapon]
            && newweapon != player->readyweapon)
        {
            // Do not go to plasma or BFG in shareware,
            //  even if cheated.
            if ((newweapon != wp_plasma
                 && newweapon != wp_bfg)
                || (gamemode != doom_shareware) )
            {
                player->pendingweapon = newweapon;
            }
        }
    }

    // check for use
    if (cmd->buttons & BT_USE)
    {
        if (!player->usedown)
        {
            P_UseLines (player);
            player->usedown = true;
        }
    }
    else
        player->usedown = false;
    // Chicken counter
    if(player->chickenTics)
    {
        // Chicken attack counter
        if(player->chickenPeck)
            player->chickenPeck -= 3;
        // Attempt to undo the chicken
        if(!--player->chickenTics)
            P_UndoPlayerChicken(player);
    }

    // cycle psprites
    P_MovePsprites (player);
    // Counters, time dependend power ups.

    // Strength counts up to diminish fade.
    if (player->powers[pw_strength])
        player->powers[pw_strength]++;

    if (player->powers[pw_invulnerability])
        player->powers[pw_invulnerability]--;

    // the MF_SHADOW activates the tr_transhi translucency while it is set
    // (it doesnt use a preset value through FF_TRANSMASK)
    if (player->powers[pw_invisibility])
        if (! --player->powers[pw_invisibility] )
            pmo->flags &= ~MF_SHADOW;

    if (player->powers[pw_infrared])
        player->powers[pw_infrared]--;

    if (player->powers[pw_ironfeet])
        player->powers[pw_ironfeet]--;
    if (player->powers[pw_flight])
    {
        if(!--player->powers[pw_flight])
        {
/* HERETODO
            if(pmo->z != pmo->floorz)
                player->centering = true;
*/
            // timed out heretic fly power
            pmo->flags2 &= ~MF2_FLY;
            pmo->flags &= ~MF_NOGRAVITY;
           // BorderTopRefresh = true; //make sure the sprite's cleared out
        }
    }
    if(player->powers[pw_weaponlevel2])
    {
        if(!--player->powers[pw_weaponlevel2])
        {
            player->weaponinfo = wpnlev1info;
            // end of weaponlevel2 power
            if((player->readyweapon == wp_phoenixrod)
                && (player->psprites[ps_weapon].state
                != &states[S_PHOENIXREADY])
                && (player->psprites[ps_weapon].state
                != &states[S_PHOENIXUP]))
            {
                P_SetPsprite(player, ps_weapon, S_PHOENIXREADY);
                player->ammo[am_phoenixrod] -= USE_PHRD_AMMO_2;
                player->refire = 0;
            }
            else if((player->readyweapon == wp_gauntlets)
                || (player->readyweapon == wp_staff))
            {
                player->pendingweapon = player->readyweapon;
            }
            //BorderTopRefresh = true;
        }
    }

    if (player->damagecount)
        player->damagecount--;

    if (player->bonuscount)
        player->bonuscount--;

    // Handling colormaps.
    if (player->powers[pw_invulnerability])
    {
        if (player->powers[pw_invulnerability] > BLINKTHRESHOLD
            || (player->powers[pw_invulnerability]&8) )
            player->fixedcolormap = INVERSECOLORMAP;
        else
            player->fixedcolormap = 0;
    }
    else if (player->powers[pw_infrared])
    {
        if (player->powers[pw_infrared] > BLINKTHRESHOLD
            || (player->powers[pw_infrared]&8) )
        {
            // almost full bright
            player->fixedcolormap = 1;
        }
        else
            player->fixedcolormap = 0;
    }
    else
        player->fixedcolormap = 0;
}

//----------------------------------------------------------------------------
//
// PROC P_PlayerNextArtifact
//
//----------------------------------------------------------------------------

void P_PlayerNextArtifact(player_t *player)
{
    player->inv_ptr--;
    if(player->inv_ptr < 6)
    {
        player->st_curpos--;
        if(player->st_curpos < 0)
            player->st_curpos = 0;
    }
    if(player->inv_ptr < 0)
    {
        player->inv_ptr = player->inventorySlotNum-1;
        if(player->inv_ptr < 6)
            player->st_curpos = player->inv_ptr;
        else
            player->st_curpos = 6;
    }
}

//----------------------------------------------------------------------------
//
// PROC P_PlayerRemoveArtifact
//
//----------------------------------------------------------------------------

static void P_PlayerRemoveArtifact(player_t *player, int slot)
{
    int i;
    
    if(!(--player->inventory[slot].count))
    { // Used last of a type - compact the artifact list
        player->inventory[slot].type = arti_none;
        for(i = slot+1; i < player->inventorySlotNum; i++)
            player->inventory[i-1] = player->inventory[i];
        player->inventorySlotNum--;

        // Set position markers and get next readyArtifact
        player->inv_ptr--;
        if(player->inv_ptr < 6)
        {
            player->st_curpos--;
            if( player->st_curpos < 0 )
                player->st_curpos = 0;
        }
        if( player->inv_ptr >= player->inventorySlotNum)
            player->inv_ptr = player->inventorySlotNum-1;
        if( player->inv_ptr < 0)
            player->inv_ptr = 0;
    }
}

//----------------------------------------------------------------------------
//
// PROC P_PlayerUseArtifact
//
//----------------------------------------------------------------------------
extern int ArtifactFlash;
void P_PlayerUseArtifact(player_t *player, artitype_t arti)
{
    int i;
    
    for(i = 0; i < player->inventorySlotNum; i++)
    {
        if(player->inventory[i].type == arti)
        { // Found match - try to use
            if(P_UseArtifact(player, arti))
            { // Artifact was used - remove it from inventory
                P_PlayerRemoveArtifact(player, i);
                if(player == consoleplayer_ptr 
		   || player == displayplayer2_ptr ) // NULL when unused
                {
                    S_StartSound(NULL, sfx_artiuse);
                    ArtifactFlash = 4;
                }
            }
            else
            { // Unable to use artifact, advance pointer
                P_PlayerNextArtifact(player);
            }
            break;
        }
    }
}

//----------------------------------------------------------------------------
//
// PROC P_ArtiTele
//
//----------------------------------------------------------------------------

void P_ArtiTele(player_t *player)
{
    int i;
    fixed_t destX;
    fixed_t destY;
    angle_t destAngle;
    mapthing_t * mtp;
    
    if(cv_deathmatch.value)
    {
        i = P_Random()%numdmstarts;
        mtp = deathmatchstarts[i];
    }
    else
    {
        mtp = playerstarts[0];
    }
    destX = mtp->x<<FRACBITS;
    destY = mtp->y<<FRACBITS;
    destAngle = ANG45*(mtp->angle/45);
    P_Teleport(player->mo, destX, destY, destAngle);
    S_StartSound(NULL, sfx_wpnup); // Full volume laugh
}


//----------------------------------------------------------------------------
//
// FUNC P_UseArtifact
//
// Returns true if artifact was used.
//
//----------------------------------------------------------------------------

boolean P_UseArtifact(player_t *player, artitype_t arti)
{
    mobj_t *mo;
    mobj_t * pmo = player->mo;
    angle_t angle;
    
    switch(arti)
    {
    case arti_invulnerability:
        if(!P_GivePower(player, pw_invulnerability))
	    goto ret_fail;
        break;
    case arti_invisibility:
        if(!P_GivePower(player, pw_invisibility))
	    goto ret_fail;
        break;
    case arti_health:
        if(!P_GiveBody(player, 25))
	    goto ret_fail;
        break;
    case arti_superhealth:
        if(!P_GiveBody(player, 100))
	    goto ret_fail;
        break;
    case arti_tomeofpower:
        if(player->chickenTics)
        { // Attempt to undo chicken
            if(P_UndoPlayerChicken(player) == false)
            { // Failed
                P_DamageMobj(pmo, NULL, NULL, 10000);
            }
            else
            { // Succeeded
                player->chickenTics = 0;
#ifdef XPEREMNTAL_HW3S
                S_StartScreamSound(pmo, sfx_wpnup);
#else
                S_StartSound(pmo, sfx_wpnup);
#endif
            }
        }
        else
        {
            if(!P_GivePower(player, pw_weaponlevel2))
	        goto ret_fail;
            if(player->readyweapon == wp_staff)
            {
                P_SetPsprite(player, ps_weapon, S_STAFFREADY2_1);
            }
            else if(player->readyweapon == wp_gauntlets)
            {
                P_SetPsprite(player, ps_weapon, S_GAUNTLETREADY2_1);
            }
        }
        break;
    case arti_torch:
        if(!P_GivePower(player, pw_infrared))
	    goto ret_fail;
        break;
    case arti_firebomb:
        angle = pmo->angle>>ANGLETOFINESHIFT;
        mo = P_SpawnMobj(pmo->x+24*finecosine[angle],
            pmo->y+24*finesine[angle], pmo->z - 15*FRACUNIT*
            ((pmo->flags2&MF2_FEETARECLIPPED) != 0), MT_FIREBOMB);
        mo->target = pmo;
        break;
    case arti_egg:
        P_SpawnPlayerMissile(pmo, MT_EGGFX);
        P_SPMAngle(pmo, MT_EGGFX, pmo->angle-(ANG45/6));
        P_SPMAngle(pmo, MT_EGGFX, pmo->angle+(ANG45/6));
        P_SPMAngle(pmo, MT_EGGFX, pmo->angle-(ANG45/3));
        P_SPMAngle(pmo, MT_EGGFX, pmo->angle+(ANG45/3));
        break;
    case arti_fly:
        if(!P_GivePower(player, pw_flight))
	    goto ret_fail;
        break;
    case arti_teleport:
        P_ArtiTele(player);
        break;
    default:
        goto ret_fail;
    }
    return(true);

ret_fail:   
    return(false);
}
