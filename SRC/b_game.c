// Emacs style mode select -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: b_game.c 1361 2017-10-16 16:26:45Z wesleyjohnson $
//
// Copyright (C) 2002 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
//
// $Log: b_game.c,v $
// Revision 1.5  2004/07/27 08:19:34  exl
// New fmod, fs functions, bugfix or 2, patrol nodes
//
// Revision 1.4  2003/06/11 04:04:50  ssntails
// Rellik's Bot Code!
//
// Revision 1.3  2002/09/28 06:53:11  tonyd
// fixed CR problem, fixed game options crash
//
// Revision 1.2  2002/09/27 16:40:07  tonyd
// First commit of acbot
//
//-----------------------------------------------------------------------------

// Bot include
#include "b_bot.h"
#include "b_game.h"
#include "b_look.h"
#include "b_node.h"

// Doom include
#include "doomincl.h"
#include "doomstat.h"
//#include "r_defs.h"
#include "m_random.h"
#include "p_local.h"
#include "z_zone.h"

#include "command.h"
#include "r_state.h"
#include "v_video.h"
#include "m_argv.h"
#include "p_setup.h"
#include "r_main.h"
#include "r_things.h"
#include "g_game.h"
#include "d_net.h"

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

boolean B_FindNextNode(player_t* p);

BOTINFOTYPE botinfo[MAXPLAYERS];
fixed_t botforwardmove[2] = {25/NEWTICRATERATIO, 50/NEWTICRATERATIO};
fixed_t botsidemove[2]    = {24/NEWTICRATERATIO, 40/NEWTICRATERATIO};
fixed_t botangleturn[4]   = {500, 1000, 2000, 4000};

extern consvar_t cv_skill;
extern thinker_t thinkercap;
extern mobj_t*	tmthing;

char* botnames[MAXPLAYERS] = {
  "Frag-God",
  "Thresh",
  "Reptile",
  "Archer",
  "Freak",
  "TF-Master",
  "Carmack",
  "Quaker",
  "FragMaster",
  "Punisher",
  "Romero",
  "Xoleras",
  "Hurdlerbot",
  "Meisterbot",
  "Borisbot",
  "Tailsbot",
  "crackbaby",
  "yo momma",
  "crusher",
  "aimbot",
  "crash",
  "akira",
  "meiko",
  "undead",
  "death",
  "TonyD-bot",
  "unit",
  "fodder",
  "2-vile",
  "nitemare",
  "nos482",
  "billy"
};

int botcolors[NUMSKINCOLORS] = 
{
   0, // = Green
   1, // = Indigo
   2, // = Blue
   3, // = Deep Red
   4, // = White
   5, // = Bright Brown
   6, // = Red
   7, // = Blue
   8, // = Dark Blue
   9, // = Yellow
   10 // = Bleached Bone
};

void B_Init_Bots()
{  
    boolean duplicateBot;
    int botNum, i, j;

    for (i=0; i< MAXPLAYERS; i++)
    {
        do
        {
            botNum = B_Random()%MAXPLAYERS;
            botinfo[i].name = botnames[botNum];
            duplicateBot = false;
            j = 0;
            while((j < i) && !duplicateBot)
            {
                if( (j != botNum) && (botinfo[j].name == botinfo[botNum].name) )
                   duplicateBot = true;

                j++;
            }
        } while (duplicateBot);

        botinfo[i].colour = B_Random() % NUMSKINCOLORS;
    }
    botNodeArray = NULL;
}

//
// bot commands
//

void Command_AddBot(void)
{
    byte buf = 0;

    if( !server )
    {
        CONS_Printf("Only the server can add a bot\n");
        return;
    }

    while ((buf < MAXPLAYERS) && playeringame[buf])	//find free playerspot
       buf++;

    if( buf>=MAXPLAYERS )
    {
        CONS_Printf ("You can only have %d players.\n", MAXPLAYERS);
        return;
    }

    Send_NetXCmd(XD_ADDBOT, &buf, 1);
}

void B_Register_Commands()
{
    COM_AddCommand ("addbot", Command_AddBot);
}

static
void B_AvoidMissile(player_t* p, mobj_t* missile)
{
    fixed_t  missileAngle = R_PointToAngle2 (p->mo->x, p->mo->y,
                                             missile->x, missile->y);

    fixed_t  delta = p->mo->angle - missileAngle;

    if( delta >= 0)
        p->cmd.sidemove = -botsidemove[1];
    else if( delta < 0)
        p->cmd.sidemove = botsidemove[1];
}

static
void B_ChangeWeapon (player_t* p)
{
    boolean  hasWeaponAndAmmo[NUMWEAPONS];
    byte  i;
    byte  numWeapons = 0;
    byte  weaponChance;

    for (i=0; i<NUMWEAPONS; i++)
    {
        switch (i)
        {
         case wp_fist:
            hasWeaponAndAmmo[i] = false;//true;
            break;
         case wp_pistol:
            hasWeaponAndAmmo[i] = p->ammo[am_clip];
            break;
         case wp_shotgun:
            hasWeaponAndAmmo[i] = (p->weaponowned[i] && p->ammo[am_shell]);
            break;
         case wp_chaingun:
            hasWeaponAndAmmo[i] = (p->weaponowned[i] && p->ammo[am_clip]);
            break;
         case wp_missile:
            hasWeaponAndAmmo[i] = (p->weaponowned[i] && p->ammo[am_misl]);
            break;
         case wp_plasma:
            hasWeaponAndAmmo[i] = (p->weaponowned[i] && p->ammo[am_cell]);
            break;
         case wp_bfg:
            hasWeaponAndAmmo[i] = (p->weaponowned[i] && (p->ammo[am_cell] >= 40));
            break;
         case wp_chainsaw:
            hasWeaponAndAmmo[i] = p->weaponowned[i];
            break;
         case wp_supershotgun:
            hasWeaponAndAmmo[i] = (p->weaponowned[i] && (p->ammo[am_shell] >= 2));
        }
        if( hasWeaponAndAmmo[i])// || ((i == wp_fist) && p->powers[pw_strength]))
            numWeapons++;
    }

    //or I have just picked up a new weapon
    if( !p->bot->weaponchangetimer || !hasWeaponAndAmmo[p->readyweapon]
        || (numWeapons != p->bot->lastNumWeapons))
    {
        if( (hasWeaponAndAmmo[wp_shotgun] && (p->readyweapon != wp_shotgun))
            || (hasWeaponAndAmmo[wp_chaingun] && (p->readyweapon != wp_chaingun))
            || (hasWeaponAndAmmo[wp_missile] && (p->readyweapon != wp_missile))
            || (hasWeaponAndAmmo[wp_plasma] && (p->readyweapon != wp_plasma))
            || (hasWeaponAndAmmo[wp_bfg] && (p->readyweapon != wp_bfg))
            || (hasWeaponAndAmmo[wp_supershotgun] && (p->readyweapon != wp_supershotgun)))
        {
            p->cmd.buttons &= ~BT_ATTACK;	//stop rocket from jamming;
            do
            {
                weaponChance = B_Random();
                if( (weaponChance < 30) && hasWeaponAndAmmo[wp_shotgun]
                     && (p->readyweapon != wp_shotgun))//has shotgun and shells
                    p->cmd.buttons |= (BT_CHANGE | (wp_shotgun<<BT_WEAPONSHIFT));
                else if( (weaponChance < 80) && hasWeaponAndAmmo[wp_chaingun]
                     && (p->readyweapon != wp_chaingun))//has chaingun and bullets
                    p->cmd.buttons |= (BT_CHANGE | (wp_chaingun<<BT_WEAPONSHIFT));
                else if( (weaponChance < 130) && hasWeaponAndAmmo[wp_missile]
                     && (p->readyweapon != wp_missile))//has rlauncher and rocket
                    p->cmd.buttons |= (BT_CHANGE | (wp_missile<<BT_WEAPONSHIFT));
                else if( (weaponChance < 180) && hasWeaponAndAmmo[wp_plasma]
                     && (p->readyweapon != wp_plasma))//has plasma and cells
                    p->cmd.buttons |= (BT_CHANGE | (wp_plasma<<BT_WEAPONSHIFT));
                else if( (weaponChance < 200) && hasWeaponAndAmmo[wp_bfg]
                     && (p->readyweapon != wp_bfg))//has bfg and cells
                    p->cmd.buttons |= (BT_CHANGE | (wp_bfg<<BT_WEAPONSHIFT));
                else if( hasWeaponAndAmmo[wp_supershotgun]
                     && (p->readyweapon != wp_supershotgun))
                    p->cmd.buttons |= (BT_CHANGE | BT_EXTRAWEAPON | (wp_shotgun<<BT_WEAPONSHIFT));
            } while (!(p->cmd.buttons & BT_CHANGE));
        }
        else if( hasWeaponAndAmmo[wp_pistol]
                 && (p->readyweapon != wp_pistol))//has pistol and bullets
            p->cmd.buttons |= (BT_CHANGE | wp_pistol<<BT_WEAPONSHIFT);
        else if( p->weaponowned[wp_chainsaw] && !p->powers[pw_strength]
                 && (p->readyweapon != wp_chainsaw))//has chainsaw, and not powered
            p->cmd.buttons |= (BT_CHANGE | BT_EXTRAWEAPON | (wp_fist<<BT_WEAPONSHIFT));
        else	//resort to fists, if have powered fists, better with fists then chainsaw
            p->cmd.buttons |= (BT_CHANGE | wp_fist<<BT_WEAPONSHIFT);

        p->bot->weaponchangetimer = (B_Random()<<7)+10000;	//how long until I next change my weapon
    }
    else if( p->bot->weaponchangetimer)
        p->bot->weaponchangetimer--;

    if( numWeapons != p->bot->lastNumWeapons)
        p->cmd.buttons &= ~BT_ATTACK;	//stop rocket from jamming;
    p->bot->lastNumWeapons = numWeapons;

    //debug_Printf("p->bot->weaponchangetimer is %d\n", p->bot->weaponchangetimer);
}

#define ANG5 (ANG90/18)

// returns the difference between the angle mobj is facing,
// and the angle from mo to x,y

fixed_t B_AngleDiff(mobj_t* mo, fixed_t x, fixed_t y)
{
    return ((R_PointToAngle2 (mo->x, mo->y, x, y)) - mo->angle);
}

static
void B_TurnTowardsPoint(player_t* p, fixed_t x, fixed_t y)
{
    int  botspeed;
    fixed_t  angle = R_PointToAngle2 (p->mo->x, p->mo->y, x, y);
    fixed_t  delta = angle - p->mo->angle;

    if( abs(delta) < (ANG45>>2))
        botspeed = 0;
    else if( abs(delta) < ANG45)
        botspeed = 1;
    else
        botspeed = 1;

    if( abs(delta) < ANG5)
        p->cmd.angleturn = angle>>FRACBITS;	//perfect aim
    else if( delta > 0)
        p->cmd.angleturn += botangleturn[botspeed];
    else
        p->cmd.angleturn -= botangleturn[botspeed];
}

static
void B_AimWeapon(player_t* p)
{
    mobj_t  *dest = p->bot->closestEnemy;
    mobj_t  *source = p->mo;

    int  botspeed = 0;
    int  angle, delta, dist, missileSpeed, realAngle;
    int  mtime, t;

    fixed_t  px, py, pz;
    subsector_t	*sec;
    boolean  canHit;

    switch (p->readyweapon)	// changed so bot projectiles don't lead targets at lower skills
    {
     case wp_fist: case wp_chainsaw:			//must be close to hit with these
     case wp_pistol: case wp_shotgun: case wp_chaingun:	//instant hit weapons, aim directly at enemy
        missileSpeed = 0;
        break;
     case wp_missile:
        if( gameskill == sk_baby || gameskill == sk_easy || gameskill == sk_hard)
        {
            missileSpeed = 0;
            //debug_Printf("rocketspeed zero\n");
            break;
        }
        else
        {
            missileSpeed = mobjinfo[MT_ROCKET].speed;
            //debug_Printf("rocketspeed perfect\n");
        }
        break;
     case wp_plasma:
        if( gameskill == sk_baby || gameskill == sk_easy || gameskill == sk_hard)
        {
            missileSpeed = 0;
            //debug_Printf("plasmaspeed = zero\n");
            break;
        }
        else
        {
            missileSpeed = mobjinfo[MT_PLASMA].speed;
            //debug_Printf("plasmaspeed perfect\n");
            break;
        }
     case wp_bfg:
        if( gameskill == sk_baby || gameskill == sk_easy || gameskill == sk_hard)
        {
            missileSpeed = 0;
            //debug_Printf("BFGspeed = zero\n");
            break;
        }
        else
        {
            missileSpeed = mobjinfo[MT_BFG].speed;
            //debug_Printf("BFGspeed perfect\n");
            break;
        }
     default:
        missileSpeed = 0;
    }

    dist = P_AproxDistance (dest->x - source->x, dest->y - source->y);
    if( (p->readyweapon != wp_missile) || (dist > (100<<FRACBITS)))
    {
        if( missileSpeed)
        {
            mtime = dist/missileSpeed;
            mtime = P_AproxDistance ( dest->x + dest->momx*mtime - source->x,
                                      dest->y + dest->momy*mtime - source->y)
                            / missileSpeed;

            t = mtime + 4;
            do
            {
                t-=4;
                if( t < 0)
                    t = 0;
                px = dest->x + dest->momx*t;
                py = dest->y + dest->momy*t;
                pz = dest->z + dest->momz*t;
                canHit = P_CheckSight2(source, dest, px, py, pz);
            } while (!canHit && (t > 0));

            sec = R_PointInSubsector(px, py);
            if( !sec)
                sec = dest->subsector;

            if( pz < sec->sector->floorheight)
                pz = sec->sector->floorheight;
            else if( pz > sec->sector->ceilingheight)
                pz = sec->sector->ceilingheight - dest->height;
        }
        else
        {
            px = dest->x;
            py = dest->y;
            pz = dest->z;
        }

        realAngle = angle = R_PointToAngle2 (source->x, source->y, px, py);
        p->cmd.aiming = ((int)((atan ((pz - source->z + (dest->height - source->height)/2) / (double)dist)) * ANG180/M_PI))>>FRACBITS;

        if( (P_AproxDistance(dest->momx, dest->momy)>>FRACBITS) > 8)	//enemy is moving reasonably fast, so not perfectly acurate
        {
            if( dest->flags & MF_SHADOW)
                angle += P_SignedRandom()<<23;
            else if( !missileSpeed)
                angle += P_SignedRandom()<<22;
        }
        else
        {
            if( dest->flags & MF_SHADOW)
                angle += P_SignedRandom()<<22;
            else if( !missileSpeed)
                angle += P_SignedRandom()<<21;
        }

        delta = angle - source->angle;
        if( abs(delta) < (ANG45>>1))
            botspeed = 0;
        else if( abs(delta) < ANG45)
            botspeed = 1;
        else
            botspeed = 3;

        if( abs(delta) < ANG45)
        {
            if( (p->readyweapon == wp_chaingun) || (p->readyweapon == wp_plasma)
                || (p->readyweapon == wp_pistol))
                 p->cmd.buttons |= BT_ATTACK;

            if( abs(delta) <= ANG5)
            {
                // check skill, if anything but nightmare bot aim is imperfect
                if( gameskill == sk_baby || gameskill == sk_easy || gameskill == sk_medium)
                     p->cmd.angleturn = angle>>FRACBITS;  // not so perfect aim
                else if( gameskill == sk_hard || gameskill == sk_nightmare)
                     p->cmd.angleturn = realAngle>>FRACBITS; // perfect aim
                delta = 0;
                p->cmd.buttons |= BT_ATTACK;
            }
        }

        if( delta > 0)
            p->cmd.angleturn += botangleturn[botspeed];	//turn right
        else if( delta < 0)
            p->cmd.angleturn -= botangleturn[botspeed]; //turn left
    }
} 

//
// MAIN BOT AI
//
void B_BuildTiccmd(player_t* p, ticcmd_t* netcmd)
{
    boolean  blocked, notUsed = true;

    mobj_t * pmo = p->mo;
    bot_t  * pbot = p->bot;
    int  botspeed = 1;
    int  x, y;
    fixed_t  cmomx, cmomy;  //what the extra momentum added from this tick will be
    fixed_t  px, py;  //coord of where I will be next tick
    fixed_t  forwardmove = 0, sidemove = 0;
    int      forward_angf, side_angf;
    fixed_t  targetDistance;  //how far away is my enemy, wanted thing

    ticcmd_t*  cmd = &p->cmd;

    //needed so bot doesn't hold down use before reaching switch object
    if( cmd->buttons & BT_USE)
        notUsed = false;    //wouldn't be able to use switch

    memset (cmd,0,sizeof(*cmd));


    // Exit now if locked
    if( p->locked == true)
        return;

    if( p->playerstate == PST_LIVE)
    {
        cmd->angleturn = pmo->angle>>FRACBITS;
        cmd->aiming = 0;//p->aiming>>FRACBITS;

        B_LookForThings(p);
        B_ChangeWeapon(p);

        if( pbot->avoidtimer)
        {
            pbot->avoidtimer--;
            if( pmo->eflags & MF_UNDERWATER)
            {
                forwardmove = botforwardmove[1];
                cmd->buttons |= BT_JUMP;
            }
            else
            {
                if( netcmd->forwardmove > 0)
                    forwardmove = -botforwardmove[1];
                else
                    forwardmove = botforwardmove[1];
                sidemove = botsidemove[1];
            }
        }
        else
        {
            if( pbot->bestSeenItem)
            {
                targetDistance = P_AproxDistance (pmo->x - pbot->bestSeenItem->x, pmo->y - pbot->bestSeenItem->y)>>FRACBITS;
                if( targetDistance > 64)
                    botspeed = 1;
                else
                    botspeed = 0;
                B_TurnTowardsPoint(p, pbot->bestSeenItem->x, pbot->bestSeenItem->y);
                forwardmove = botforwardmove[botspeed];
                if( (((pbot->bestSeenItem->floorz - pmo->z)>>FRACBITS) > 24)
                    && (targetDistance <= 100))
                    cmd->buttons |= BT_JUMP;

                pbot->bestItem = NULL;
            }	//if a target exists and is alive
            else if( pbot->closestEnemy)
             // && (pbot->closestEnemy->flags & ~MF_CORPSE))
            {
                player_t * enemyp = pbot->closestEnemy->player;
                weapontype_t  enemy_readyweapon =
                 ( enemyp )? enemyp->readyweapon
                 : wp_nochange; // does not match anything
                boolean  enemy_linescan_weapon =
                   (enemy_readyweapon == wp_pistol)
                   || (enemy_readyweapon == wp_shotgun)
                   || (enemy_readyweapon == wp_chaingun);

                //debug_Printf("heading for an enemy\n");
                targetDistance = P_AproxDistance (pmo->x - pbot->closestEnemy->x, pmo->y - pbot->closestEnemy->y)>>FRACBITS;
                if( (targetDistance > 300)
                    || (p->readyweapon == wp_fist)
                    || (p->readyweapon == wp_chainsaw))
                    forwardmove = botforwardmove[botspeed];
                if( (p->readyweapon == wp_missile) && (targetDistance < 400))
                    forwardmove = -botforwardmove[botspeed];

                // gameskill setting determines likelyhood bot will start strafing
                switch(gameskill)
                {
                 case sk_baby:
                    if(targetDistance <=32)
                        sidemove = botsidemove[botspeed];
                    break;
                 case sk_easy:
                    if(targetDistance <=150)
                        sidemove = botsidemove[botspeed];
                    break;
                 case sk_medium:
                    if((targetDistance <= 150) || enemy_linescan_weapon )
                        sidemove = botsidemove[botspeed];
                    break;
                 case sk_hard:
                    if((targetDistance <= 350) || enemy_linescan_weapon )
                        sidemove = botsidemove[botspeed];
                    break;
                 case sk_nightmare:
                    if((targetDistance <= 650) || enemy_linescan_weapon )
                        sidemove = botsidemove[botspeed];
                    break;
                 default:
                    break;
                }

                B_AimWeapon(p);
                pbot->lastMobj = pbot->closestEnemy;
                pbot->lastMobjX = pbot->closestEnemy->x;
                pbot->lastMobjY = pbot->closestEnemy->y;
            }
            else
            {
                cmd->aiming = 0;
                //look for an unactivated switch/door
                if( B_LookForSpecialLine(p, &x, &y)
                    && B_ReachablePoint(p, pmo->subsector->sector, x, y))
                {
                    //debug_Printf("found a special line\n");
                    B_TurnTowardsPoint(p, x, y);
                    if( P_AproxDistance (pmo->x - x, pmo->y - y) <= USERANGE)
                    {
                        if( notUsed )
                            cmd->buttons |= BT_USE;
                    }
                    else
                        forwardmove = botforwardmove[1];
                }
                else if( pbot->teammate)
                {
                    mobj_t * tmate = pbot->teammate;
                    targetDistance =
                        P_AproxDistance (pmo->x - tmate->x, pmo->y - tmate->y)>>FRACBITS;
                    if( targetDistance > 100)
                    {
                        B_TurnTowardsPoint(p, tmate->x, tmate->y);
                        forwardmove = botforwardmove[botspeed];
                    }

                    pbot->lastMobj = tmate;
                    pbot->lastMobjX = tmate->x;
                    pbot->lastMobjY = tmate->y;
                }
                //since nothing else to do, go where last enemy/teamate was seen
                else if( pbot->lastMobj && (pbot->lastMobj->flags & MF_SOLID))
                  // && B_ReachablePoint(p, R_PointInSubsector(pbot->lastMobjX, pbot->lastMobjY)->sector, pbot->lastMobjX, pbot->lastMobjY))
                {
                    if( (pmo->momx == 0 && pmo->momy == 0)
                        || !B_NodeReachable(NULL, pmo->x, pmo->y, pbot->lastMobjX, pbot->lastMobjY))
                        pbot->lastMobj = NULL;	//just went through teleporter
                    else
                    {
                        //debug_Printf("heading towards last mobj\n");
                        B_TurnTowardsPoint(p, pbot->lastMobjX, pbot->lastMobjY);
                        forwardmove = botforwardmove[botspeed];
                    }
                }
                else
                {
                    pbot->lastMobj = NULL;

                    if( pbot->bestItem)
                    {
                        SearchNode_t* temp =
                            B_GetNodeAt(pbot->bestItem->x, pbot->bestItem->y);
                        //debug_Printf("found a best item at x:%d, y:%d\n", pbot->bestItem->x>>FRACBITS, pbot->bestItem->y>>FRACBITS);
                        if( pbot->destNode != temp)
                            B_LLClear(pbot->path);
                        pbot->destNode = temp;
                    }
                    else if( pbot->closestUnseenTeammate)
                    {
                        SearchNode_t* temp =
                            B_GetNodeAt(pbot->closestUnseenTeammate->x,
                                        pbot->closestUnseenTeammate->y);
                        //debug_Printf("found a best item at x:%d, y:%d\n", pbot->bestItem->x>>FRACBITS, pbot->bestItem->y>>FRACBITS);
                        if( pbot->destNode != temp)
                            B_LLClear(pbot->path);
                        pbot->destNode = temp;
                    }
                    else if( pbot->closestUnseenEnemy)
                    {
                        SearchNode_t* temp =
                            B_GetNodeAt(pbot->closestUnseenEnemy->x,
                                        pbot->closestUnseenEnemy->y);
                        //debug_Printf("found a best item at x:%d, y:%d\n", pbot->bestItem->x>>FRACBITS, pbot->bestItem->y>>FRACBITS);
                        if( pbot->destNode != temp)
                            B_LLClear(pbot->path);
                        pbot->destNode = temp;
                    }
                    else
                        pbot->destNode = NULL;

                    if( pbot->destNode)
                    {
                        if( !B_LLIsEmpty(pbot->path)
                            && P_AproxDistance(pmo->x - posX2x(pbot->path->first->x), pmo->y - posY2y(pbot->path->first->y)) < (BOTNODEGRIDSIZE<<1))//BOTNODEGRIDSIZE>>1))
                        {
#ifdef SHOWBOTPATH
                            SearchNode_t* temp = B_LLRemoveFirstNode(pbot->path);
                            P_RemoveMobj(temp->mo);
                            Z_Free(temp);
#else
                            Z_Free(B_LLRemoveFirstNode(pbot->path));
#endif
                        }


                        //debug_Printf("at x%d, y%d\n", pbot->wantedItemNode->x>>FRACBITS, pbot->wantedItemNode->y>>FRACBITS);
                        if( B_LLIsEmpty(pbot->path)
                            || !B_NodeReachable(NULL, pmo->x, pmo->y,
                                                posX2x(pbot->path->first->x),
                                                posY2y(pbot->path->first->y) )
                               // > (BOTNODEGRIDSIZE<<2))
                            )
                        {
                            if( !B_FindNextNode(p))	//search for next node
                            {
                                //debug_Printf("Bot stuck at x:%d y:%d could not find a path to x:%d y:%d\n",pmo->x>>FRACBITS, pmo->y>>FRACBITS, posX2x(pbot->destNode->x)>>FRACBITS, posY2y(pbot->destNode->y)>>FRACBITS);

                                pbot->destNode = NULL;	//can't get to it
                            }
                        }

                        if( !B_LLIsEmpty(pbot->path))
                        {
                            //debug_Printf("turning towards node at x%d, y%d\n", (pbot->nextItemNode->x>>FRACBITS), (pbot->nextItemNode->y>>FRACBITS));
                            //debug_Printf("it has a distance %d\n", (P_AproxDistance(pmo->x - pbot->nextItemNode->x, pmo->y - pbot->nextItemNode->y)>>FRACBITS));
                            B_TurnTowardsPoint(p,
                                               posX2x(pbot->path->first->x),
                                               posY2y(pbot->path->first->y));
                            forwardmove = botforwardmove[1];//botspeed];
                        }
                    }
                }
            }

            // proportional forward and side movement
            forward_angf = ANGLE_TO_FINE(pmo->angle);
            side_angf = ANGLE_TO_FINE(pmo->angle - ANG90);
            cmomx = FixedMul(forwardmove*2048, finecosine[forward_angf]) + FixedMul(sidemove*2048, finecosine[side_angf]);
            cmomy = FixedMul(forwardmove*2048, finesine[forward_angf]) + FixedMul(sidemove*2048, finesine[side_angf]);
            px = pmo->x + pmo->momx + cmomx;
            py = pmo->y + pmo->momy + cmomy;

            // tmr_floorz, tmr_ceilingz returned by P_CheckPosition
            blocked = !P_CheckPosition (pmo, px, py)
                 || (((tmr_floorz - pmo->z)>>FRACBITS) > 24)
                 || ((tmr_ceilingz - tmr_floorz) < pmo->height);
            //if its time to change strafe directions,
            if( sidemove && ((pmo->flags & MF_JUSTHIT) || blocked))
            {
                pbot->straferight = !pbot->straferight;
                pmo->flags &= ~MF_JUSTHIT;
            }

            if( blocked)
            {
                // tm_thing is global var of P_CheckPosition
                if( (++pbot->blockedcount > 20)
                    && ((P_AproxDistance(pmo->momx, pmo->momy) < (4<<FRACBITS))
                        || (tm_thing && (tm_thing->flags & MF_SOLID)))
                    )
                    pbot->avoidtimer = 20;

                if( (((tmr_floorz - pmo->z)>>FRACBITS) > 24)
                    && ((((tmr_floorz - pmo->z)>>FRACBITS) <= 37)
                        || ((((tmr_floorz - pmo->z)>>FRACBITS) <= 45)
                            && (pmo->subsector->sector->floortype != FLOOR_WATER))))
                    cmd->buttons |= BT_JUMP;

                for (x=0; x<numspechit; x++)
                {
                    if( lines[spechit[x]].backsector)
                    {
                        if( !lines[spechit[x]].backsector->ceilingdata && !lines[spechit[x]].backsector->floordata && (lines[spechit[x]].special != 11))	//not the exit switch
                            cmd->buttons |= BT_USE;
                    }
                }
            }
            else
                pbot->blockedcount = 0;
        }

        if( sidemove )
        {
            if( pbot->strafetimer )
                pbot->strafetimer--;
            else
            {
                pbot->straferight = !pbot->straferight;
                pbot->strafetimer = B_Random()/3;
            }
        }
        if( pbot->weaponchangetimer)
            pbot->weaponchangetimer--;

        p->cmd.forwardmove = forwardmove;
        p->cmd.sidemove = pbot->straferight ? sidemove : -sidemove;
        if( pbot->closestMissile )
            B_AvoidMissile(p, pbot->closestMissile);
    }
    else
    {
        // Dead
        if( demoversion < 146 )
        {
            cmd->buttons |= BT_USE;	//I want to respawn
        }
        else
        {
            // Version 1.46
            // [WDJ] Slow down bot respawn, so they are not so overwhelming.
            cmd->buttons = 0;
            if( p->damagecount )
            {
                p->damagecount = 0;
                pbot->avoidtimer = 6 * TICRATE; // wait
            }
            if( --pbot->avoidtimer <= 0 )
                cmd->buttons |= BT_USE;	//I want to respawn
        }
    }

    memcpy (netcmd, cmd, sizeof(*cmd));
} // end of BOT_Thinker


bot_t* B_Create_Bot()
{
    bot_t* bot = Z_Malloc (sizeof(*bot), PU_STATIC, 0);

    bot->path = B_LLCreate();

    return bot;
}


void B_SpawnBot(bot_t* bot)
{
    bot->avoidtimer = 0;
    bot->blockedcount = 0;
    bot->weaponchangetimer = 0;

    bot->bestItem = NULL;
    bot->lastMobj = NULL;
    bot->destNode = NULL;

    B_LLClear(bot->path);
}
