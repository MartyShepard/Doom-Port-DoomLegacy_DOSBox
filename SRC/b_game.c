// Emacs style mode select -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: b_game.c 1512 2020-04-04 08:51:13Z wesleyjohnson $
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
#include "byteptr.h"


// [WDJ] New Bot code is not compatible with old versions, is not old demo compatible.
// #define BOT_VERSION_DETECT

// Persistant random number, that changes after each use.  Only used to initialize.
// Only CV_NETVAR in case someone uses the B_Gen_Random outside of initializing names.
consvar_t  cv_bot_random = { "botrandom", "1333", CV_NETVAR | CV_SAVE, CV_Unsigned };

// User set random seed.  Only used to initialize.
// Only CV_NETVAR in case someone uses the B_Gen_Random outside of initializing names.
static void CV_botrandom_OnChange( void );
consvar_t  cv_bot_randseed = { "botrandseed", "0", CV_NETVAR | CV_SAVE | CV_CALL, CV_Unsigned, CV_botrandom_OnChange };

CV_PossibleValue_t botgen_cons_t[]={ {0,"Plain"}, {1,"Seed"}, {2,"Seed Random"}, {3,"Cfg Random"}, {4,"Sys Random"}, {0,NULL}};
consvar_t  cv_bot_gen = { "botgen", "0", CV_NETVAR | CV_SAVE | CV_CALL, botgen_cons_t, CV_botrandom_OnChange };

CV_PossibleValue_t botskin_cons_t[]={ {0,"Color"}, {1,"Skin"}, {0,NULL}};
consvar_t  cv_bot_skin = { "botskin", "0", CV_NETVAR | CV_SAVE, botskin_cons_t };

CV_PossibleValue_t botrespawn_cons_t[]={
  {5,"MIN"},
  {255,"MAX"},
  {0,NULL}};
consvar_t  cv_bot_respawn_time = { "botrespawntime", "8", CV_NETVAR | CV_SAVE, botrespawn_cons_t };

CV_PossibleValue_t botskill_cons_t[]={
  {0,"crippled"},
  {1,"baby"},
  {2,"easy"},
  {3,"medium"},
  {4,"hard"},
  {5,"nightmare"},
  {6,"randmed"},
  {7,"randgame"},
  {8,"gamemed"},
  {9,"gameskill"},
  {0,NULL}};
consvar_t  cv_bot_skill = { "botskill", "gamemed", CV_NETVAR | CV_SAVE, botskill_cons_t };

static void CV_botspeed_OnChange( void );

CV_PossibleValue_t botspeed_cons_t[]={
  {0,"walk"},
  {1,"trainer"},
  {2,"slow"},
  {3,"medium"},
  {4,"fast"},
  {5,"run"},
  {8,"gamemed"},
  {9,"gameskill"},
  {10,"botskill"},
  {0,NULL}};
consvar_t  cv_bot_speed = { "botspeed", "botskill", CV_NETVAR | CV_SAVE | CV_CALL, botspeed_cons_t, CV_botspeed_OnChange };


// [WDJ] Tables just happened to be this way for now, they may change later.
static byte bot_botskill_to_speed[ 6 ] = { 0, 1, 2, 3, 4, 5 };
static byte bot_gameskill_to_speed[ 5 ] = { 1, 2, 3, 4, 5 };  // lowest value must be >= 1 (see gamemed)
static byte bot_speed_frac_table[ 6 ] = { 110, 90, 102, 112, 122, 128 };  // 128=full
static uint32_t bot_run_tics_table[ 6 ] = { TICRATE/4, 4*TICRATE, 6*TICRATE, 12*TICRATE, 24*TICRATE, 128*TICRATE };  // tics
static byte bot_speed_frac;
static byte bot_run_tics;

// A function of gameskill and cv_bot_speed.
// Must be called when either changes.
static void CV_botspeed_OnChange( void )
{
    byte bot_speed_index;
    switch( cv_bot_speed.EV )
    {
      case 8:  // gamemed
        // One step slower than gameskill.
        bot_speed_index = bot_gameskill_to_speed[ gameskill ] - 1;
        break;
      case 10: // botskill (temp value, deferred determination)
      case 9:  // gameskill
        bot_speed_index = bot_gameskill_to_speed[ gameskill ];
        break;
      default:
        bot_speed_index = cv_bot_speed.EV;  // 0..5
        break;
    }
#ifdef BOT_VERSION_DETECT
    if( demoversion < 148 )  bot_speed_index = 5; // always run
#endif

    bot_speed_frac = bot_speed_frac_table[ bot_speed_index ];
    bot_run_tics = bot_run_tics_table[ bot_speed_index ];
}


#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

boolean B_FindNextNode(player_t* p);

bot_info_t  botinfo[MAXPLAYERS];
fixed_t botforwardmove[2] = {25/NEWTICRATERATIO, 50/NEWTICRATERATIO};
fixed_t botsidemove[2]    = {24/NEWTICRATERATIO, 40/NEWTICRATERATIO};
angle_t botangleturn[4]   = {500, 1000, 2000, 4000};

extern consvar_t cv_skill;
extern thinker_t thinkercap;
extern mobj_t*	tmthing;

// Player name that is seen for each bot.
#define NUM_BOT_NAMES 40
char* botnames[NUM_BOT_NAMES] = {
  "Frag-God",
  "TF-Master",
  "FragMaster",
  "Thresh",
  "Reptile",
  "Archer",
  "Freak",
  "Reeker",
  "Dranger",
  "Enrage",
  "Franciot",
  "Grimknot",
  "Fodder",
  "Rumble",
  "Crusher",
  "Crash",
  "Krunch",
  "Wreaker",
  "Punisher",
  "Quaker",
  "Reaper",
  "Slasher",
  "Tormenot",
  "Drat",
  "Labrat",
  "Nestor",
  "Akira",
  "Meikot",
  "Aliciot",
  "Leonardot",
  "Ralphat",
  "Xoleras",
  "Zetat",
  "Carmack",  // Hon
  "Romero",  // Hon
  "Hurdlerbot",  // Team member
  "Meisterbot",  // Team member
  "Borisbot",  // Team member
  "Tailsbot",  // Team member
  "TonyD-bot", // Team member
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

#if 1
// By Server, for bots.
//  skinrand : the botinfo skinrand value
static
char * bot_skinname( int skinrand )
{
    if( cv_bot_skin.EV && (numskins > 1) )
    {
        skinrand = (skinrand % (numskins-1)) + 1;
    }
    else
    {
        skinrand = 0; // marine
    }
    return skins[skinrand]->name;
}
#endif


// Random number source for bot generation.
uint32_t  B_Gen_Random( void )
{
    uint32_t r;
    switch( cv_bot_gen.EV )
    {
     case 0:  // Plain
        r = B_Random();
        break;
     default:
     case 1:  // Seed
        r = B_Random() + cv_bot_randseed.value;
        break;
     case 2:  // Seed Random
        r = E_Random() + cv_bot_randseed.value;
        break;
     case 3:  // Cfg Random
        r = E_Random() + cv_bot_random.value;
        break;
     case 4:  // Sys Random
        r = rand();
        break;
    }
    return r;
}

void B_Init_Names()
{
    int br, i, j;
    uint16_t color_used = 0;
    byte   botindex = B_Rand_GetIndex();

    CV_ValueIncDec( &cv_bot_random, 7237 ); // add a prime

    // Initialize botinfo.
    for (i=0; i< MAXPLAYERS; i++)
    {
        // Give each prospective bot a unique name.
        do
        {
            br = B_Gen_Random() % NUM_BOT_NAMES;
            for( j = 0; j < i; j++ )
            {
                if( botinfo[j].name_index == br )  break;
            }
        } while ( j < i );  // when j==i then did not find any duplicates
        botinfo[i].name_index = br;

        // Assign a skin color.  Make them unique until have used all colors.
        j = NUMSKINCOLORS;
        br = B_Gen_Random();
        for(;;)
        {
            br = br % NUMSKINCOLORS;
            if( ((1<<br) & color_used) == 0 )  break;
            br++;
            if( --j < 0 )  color_used = 0;
        }
        botinfo[i].colour = br;
        color_used |= (1<<br);

        botinfo[i].skinrand = B_Gen_Random();
    }
   
#if 1
    // Restore to keep reproducible bot gen in spite of multiple calls during setup,
    // and it minimizes effects of network races.
    if( cv_bot_gen.EV < 2 )
        B_Rand_SetIndex( botindex );
#endif

    // Existing bots keep their existing names and colors.
    // This only changes the tables used to create new bots, by the server.
    // Off-server clients will keep the original bot names that the server sent them.
}


static byte bot_init_done = 0;

static void CV_botrandom_OnChange( void )
{
#ifdef BOT_VERSION_DETECT
    if( demoversion < 148 )  return;
#endif

    // With so many NETVAR update paths, only let the server do this.
    // The bot names will be passed with the NetXCmd, so clients do not need this.
    // The random number generatators will be updated.
    if( ! server )
        return;

    // [WDJ] Updating the random number generators in the middle of a game, ugh.
    if( netgame )
        SV_network_wait_timer( 35 );  // pause everybody
    
    if( cv_bot_randseed.state & CS_MODIFIED )
        B_Rand_SetIndex( cv_bot_randseed.value );
   
    // Only change names after initial loading of config.
    if( bot_init_done )
        B_Init_Names();
   
    // Let network un-pause naturally.  Better timing.
}


void DemoAdapt_bots( void )
{
    CV_botspeed_OnChange();
}


// may be called multiple times
void B_Init_Bots()
{  
    DemoAdapt_bots();

    B_Init_Names();

    botNodeArray = NULL;

    bot_init_done = 1;
}

//
// bot commands
//

// By Server
void B_Send_bot_NameColor( byte pn )
{
    // From botinfo sources
    bot_info_t * bip = & botinfo[pn];
    Send_NameColor_pn( pn, botnames[bip->name_index], bip->colour, bot_skinname(bip->skinrand), 2 );  // server channel
}

// By Server.
// Send name, color, skin of all bots.
void B_Send_all_bots_NameColor( void )
{
    byte pn;

    for( pn=0; pn<MAXPLAYERS; pn++ )
    {
        if( playeringame[pn] && players[pn].bot )
        {
#if 1
            B_Send_bot_NameColor( pn );
#else
            // From player, which requires that bot have set the namecolor of player already.
            Send_NameColor_player( pn, 2 );  // server channel
#endif
        }
    }
}


void Command_AddBot(void)
{
    byte buf[10];
    byte pn = 0;

    if( !server )
    {
        CONS_Printf("Only the server can add a bot\n");
        return;
    }

    pn = SV_get_player_num();

    if( pn >= MAXPLAYERS )
    {
        CONS_Printf ("You can only have %d players.\n", MAXPLAYERS);
        return;
    }

    bot_info_t * bip = & botinfo[pn];
    byte * b = &buf[0];
    // AddBot format: pn, color, name:string0
    WRITEBYTE( b, pn );
    WRITEBYTE( b, bip->colour );
    b = write_stringn(b, botnames[bip->name_index], MAXPLAYERNAME);
    SV_Send_NetXCmd(XD_ADDBOT, buf, (b - buf));  // as server

    // Cannot send NameColor XCmd before the bot exists.
}

// Only call after console player and splitscreen players have grabbed their player slots.
void B_Regulate_Bots( int req_numbots )
{
    byte pn;
    for( pn = 0; pn < MAXPLAYERS; pn++ )
    {
        if( playeringame[pn] && players[pn].bot )  req_numbots--;
    }

    if( req_numbots > (MAXPLAYERS - num_game_players) )
        req_numbots = (MAXPLAYERS - num_game_players);

    while( req_numbots > 0 )
    {
        Command_AddBot();
        req_numbots --;
    }
}

void B_Register_Commands( void )
{
    COM_AddCommand ("addbot", Command_AddBot, CC_command);
}

static
void B_AvoidMissile(player_t* p, mobj_t* missile)
{
    angle_t  missile_angle = R_PointToAngle2 (p->mo->x, p->mo->y, missile->x, missile->y);
    signed_angle_t  delta = p->mo->angle - missile_angle;

    if( delta >= 0)
        p->cmd.sidemove = -botsidemove[1];
    else if( delta < 0)
        p->cmd.sidemove = botsidemove[1];
}

static
void B_ChangeWeapon (player_t* p)
{
    bot_t * pbot = p->bot;
    boolean  usable_weapon[NUMWEAPONS]; // weapons with ammo
    byte  num_weapons = 0;
    byte  weaponChance;
    byte  i;

    for (i=0; i<NUMWEAPONS; i++)
    {
        byte hw = false;
        switch (i)
        {
         case wp_fist:
            hw = false;//true;
            break;
         case wp_pistol:
            hw = p->ammo[am_clip];
            break;
         case wp_shotgun:
            hw = (p->weaponowned[i] && p->ammo[am_shell]);
            break;
         case wp_chaingun:
            hw = (p->weaponowned[i] && p->ammo[am_clip]);
            break;
         case wp_missile:
            hw = (p->weaponowned[i] && p->ammo[am_misl]);
            break;
         case wp_plasma:
            hw = (p->weaponowned[i] && p->ammo[am_cell]);
            break;
         case wp_bfg:
            hw = (p->weaponowned[i] && (p->ammo[am_cell] >= 40));
            break;
         case wp_chainsaw:
            hw = p->weaponowned[i];
            break;
         case wp_supershotgun:
            hw = (p->weaponowned[i] && (p->ammo[am_shell] >= 2));
        }
        usable_weapon[i] = hw;
        if( hw ) // || ((i == wp_fist) && p->powers[pw_strength]))
            num_weapons++;
    }

    //or I have just picked up a new weapon
    if( !pbot->weaponchangetimer || !usable_weapon[p->readyweapon]
        || (num_weapons != pbot->lastNumWeapons))
    {
        if( (usable_weapon[wp_shotgun] && (p->readyweapon != wp_shotgun))
            || (usable_weapon[wp_chaingun] && (p->readyweapon != wp_chaingun))
            || (usable_weapon[wp_missile] && (p->readyweapon != wp_missile))
            || (usable_weapon[wp_plasma] && (p->readyweapon != wp_plasma))
            || (usable_weapon[wp_bfg] && (p->readyweapon != wp_bfg))
            || (usable_weapon[wp_supershotgun] && (p->readyweapon != wp_supershotgun)))
        {
            p->cmd.buttons &= ~BT_ATTACK;	//stop rocket from jamming;
            do
            {
                weaponChance = B_Random();
                if( (weaponChance < 30) && usable_weapon[wp_shotgun]
                     && (p->readyweapon != wp_shotgun))//has shotgun and shells
                    p->cmd.buttons |= (BT_CHANGE | (wp_shotgun<<BT_WEAPONSHIFT));
                else if( (weaponChance < 80) && usable_weapon[wp_chaingun]
                     && (p->readyweapon != wp_chaingun))//has chaingun and bullets
                    p->cmd.buttons |= (BT_CHANGE | (wp_chaingun<<BT_WEAPONSHIFT));
                else if( (weaponChance < 130) && usable_weapon[wp_missile]
                     && (p->readyweapon != wp_missile))//has rlauncher and rocket
                    p->cmd.buttons |= (BT_CHANGE | (wp_missile<<BT_WEAPONSHIFT));
                else if( (weaponChance < 180) && usable_weapon[wp_plasma]
                     && (p->readyweapon != wp_plasma))//has plasma and cells
                    p->cmd.buttons |= (BT_CHANGE | (wp_plasma<<BT_WEAPONSHIFT));
                else if( (weaponChance < 200) && usable_weapon[wp_bfg]
                     && (p->readyweapon != wp_bfg))//has bfg and cells
                    p->cmd.buttons |= (BT_CHANGE | (wp_bfg<<BT_WEAPONSHIFT));
                else if( usable_weapon[wp_supershotgun]
                     && (p->readyweapon != wp_supershotgun))
                    p->cmd.buttons |= (BT_CHANGE | BT_EXTRAWEAPON | (wp_shotgun<<BT_WEAPONSHIFT));
            } while (!(p->cmd.buttons & BT_CHANGE));
        }
        else if( usable_weapon[wp_pistol]
                 && (p->readyweapon != wp_pistol))//has pistol and bullets
            p->cmd.buttons |= (BT_CHANGE | wp_pistol<<BT_WEAPONSHIFT);
        else if( p->weaponowned[wp_chainsaw] && !p->powers[pw_strength]
                 && (p->readyweapon != wp_chainsaw))//has chainsaw, and not powered
            p->cmd.buttons |= (BT_CHANGE | BT_EXTRAWEAPON | (wp_fist<<BT_WEAPONSHIFT));
        else	//resort to fists, if have powered fists, better with fists then chainsaw
            p->cmd.buttons |= (BT_CHANGE | wp_fist<<BT_WEAPONSHIFT);

        pbot->weaponchangetimer = (B_Random()<<7)+10000;	//how long until I next change my weapon
    }
    else if( pbot->weaponchangetimer)
        pbot->weaponchangetimer--;

    if( num_weapons != pbot->lastNumWeapons)
        p->cmd.buttons &= ~BT_ATTACK;	//stop rocket from jamming;
    pbot->lastNumWeapons = num_weapons;

    //debug_Printf("pbot->weaponchangetimer is %d\n", pbot->weaponchangetimer);
}

#define ANG5 (ANG90/18)

// returns the difference between the angle mobj is facing,
// and the angle from mo to x,y

#if 0
// Not Used
static
signed_angle_t  B_AngleDiff(mobj_t* mo, fixed_t x, fixed_t y)
{
    return ((R_PointToAngle2 (mo->x, mo->y, x, y)) - mo->angle);
}
#endif

static
void B_TurnTowardsPoint(player_t* p, fixed_t x, fixed_t y)
{
    angle_t  angle = R_PointToAngle2(p->mo->x, p->mo->y, x, y);
    signed_angle_t  delta =  angle - p->mo->angle;
    angle_t  abs_delta = abs(delta);

    if( abs_delta < ANG5 )
    {
        p->cmd.angleturn = angle>>FRACBITS;	//perfect aim
    }
    else
    {
        angle_t  turnspeed = botangleturn[ (abs_delta < (ANG45>>2))? 0 : 1 ];

        if( delta > 0)
            p->cmd.angleturn += turnspeed;
        else
            p->cmd.angleturn -= turnspeed;
    }
}

// Turn away from a danger, or friend.
static
void B_Turn_Away_Point(player_t* p, fixed_t x, fixed_t y)
{
    signed_angle_t  delta = R_PointToAngle2(p->mo->x, p->mo->y, x, y) - p->mo->angle;
    angle_t  abs_delta = abs(delta);

    if( abs_delta < (ANG45*3) )
    {
        angle_t  turnspeed = botangleturn[ (abs_delta < ANG45)? 1 : 0 ];

        // No perfect aim
        if( delta > 0 )
            p->cmd.angleturn -= turnspeed;
        else
            p->cmd.angleturn += turnspeed;
    }
}

static
void B_AimWeapon(player_t* p)
{
    bot_t  * pbot = p->bot;
    mobj_t * source = p->mo;
    mobj_t * dest = pbot->closestEnemy;

    int  botspeed = 0;
    int  mtime, t;
    angle_t angle, perfect_angle;
    signed_angle_t  delta;
    angle_t  abs_delta;
    byte botskill = pbot->skill;

    fixed_t  px, py, pz;
    fixed_t  weapon_range;
    fixed_t  dist, missile_speed;
    subsector_t	*sec;
    boolean  canHit;

    missile_speed = 0;  // default
    switch (p->readyweapon)	// changed so bot projectiles don't lead targets at lower skills
    {
     case wp_fist: case wp_chainsaw:			//must be close to hit with these
        missile_speed = 0;
        weapon_range = 20<<FRACBITS;
        break;
     case wp_pistol: case wp_shotgun: case wp_chaingun:	//instant hit weapons, aim directly at enemy
        missile_speed = 0;
        weapon_range = 512<<FRACBITS;
        break;
     case wp_missile:
        missile_speed = mobjinfo[MT_ROCKET].speed;
        weapon_range = 1024<<FRACBITS;
        break;
     case wp_plasma:
        missile_speed = mobjinfo[MT_PLASMA].speed;
        weapon_range = 1024<<FRACBITS;
        break;
     case wp_bfg:
        missile_speed = mobjinfo[MT_BFG].speed;
        weapon_range = 1024<<FRACBITS;
        break;
     default:
        missile_speed = 0;
        weapon_range = 4096<<FRACBITS;
        break;
    }

    // botskill 0 to 5
    if( botskill < 2 )
    {
        missile_speed = 0;  // no aim prediction
    }
    else if( botskill == 3 )
    {
        missile_speed *= 3;  // throw off aiming
    }
    else if( botskill == 4 )
    {
        missile_speed *= 2;  // throw off aiming
    }

    dist = P_AproxDistance (dest->x - source->x, dest->y - source->y);

    if( (dest->type == MT_BARREL) || (dest->type == MT_POD) || (dest->flags & MF_TOUCHY) )
    {
        // [WDJ] Do not attack exploding things with fists.	    
        if( weapon_range < (100<<FRACBITS))  goto reject_enemy;
        if( dist < (100<<FRACBITS))  goto reject_enemy;  // too close, must get distance first.
    }

    if( dist > weapon_range )
    {
        if( B_Random() < 240 )  return;  // wait till in range, most of the time
    }
   
    if( (p->readyweapon != wp_missile) || (dist > (100<<FRACBITS)))
    {
        if( missile_speed)
        {
            mtime = dist/missile_speed;
            mtime = P_AproxDistance ( dest->x + dest->momx*mtime - source->x,
                                      dest->y + dest->momy*mtime - source->y)
                            / missile_speed;

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

        perfect_angle = angle = R_PointToAngle2 (source->x, source->y, px, py);
        p->cmd.aiming = ((int)((atan ((pz - source->z + (dest->height - source->height)/2) / (double)dist)) * ANG180/M_PI))>>FRACBITS;

        // Random aiming imperfections.
        if( (P_AproxDistance(dest->momx, dest->momy)>>FRACBITS) > 8)	//enemy is moving reasonably fast, so not perfectly acurate
        {
            if( dest->flags & MF_SHADOW)
                angle += P_SignedRandom()<<23;
            else if( missile_speed == 0 )
                angle += P_SignedRandom()<<22;
        }
        else
        {
            if( dest->flags & MF_SHADOW)
                angle += P_SignedRandom()<<22;
            else if( missile_speed == 0 )
                angle += P_SignedRandom()<<21;
        }

        delta = angle - source->angle;
        abs_delta = abs(delta);

        if( abs_delta < ANG45 )
        {
            // Fire weapon when aim is best.
            if( abs_delta <= ANG5 )
            {
                // cmd.angleturn is 16 bit angle (angle is not fixed_t)
                // lower skill levels have imperfect aim
                p->cmd.angleturn = (( botskill < 4 )?  // < hard
                      angle  // not so perfect aim
                    : perfect_angle // perfect aim
                    ) >>16;  // 32 bit to 16 bit angle
                p->cmd.buttons |= BT_ATTACK;
                return;
            }

            // Fire some weapons when aim is just close.
            if( (p->readyweapon == wp_chaingun) || (p->readyweapon == wp_plasma)
                || (p->readyweapon == wp_pistol))
                 p->cmd.buttons |= BT_ATTACK;
        }

        // Still turning to face target.
        botspeed = ( abs_delta < (ANG45>>1) )? 0
           : ( abs_delta < ANG45 )? 1
           : 3;

        if( delta > 0)
            p->cmd.angleturn += botangleturn[botspeed];	//turn right
        else if( delta < 0)
            p->cmd.angleturn -= botangleturn[botspeed]; //turn left
    }
    return;

reject_enemy:
    pbot->closestEnemy = NULL;
    return;
} 

//
// MAIN BOT AI
//

static fixed_t bot_strafe_dist[6] = {
   (20<<FRACBITS),  // crippled
   (32<<FRACBITS),  // baby
   (150<<FRACBITS), // easy
   (150<<FRACBITS), // medium
   (350<<FRACBITS), // hard
   (650<<FRACBITS)  // nightmare
};

void B_BuildTiccmd(player_t* p, ticcmd_t* netcmd)
{
    mobj_t * pmo = p->mo;
    bot_t  * pbot = p->bot;
    ticcmd_t*  cmd = &p->cmd;

    int  x, y;
    fixed_t  cmomx, cmomy;  //what the extra momentum added from this tick will be
    fixed_t  px, py;  //coord of where I will be next tick
    fixed_t  forwardmove = 0, sidemove = 0;
    int      forward_angf, side_angf;
    fixed_t  target_dist;  //how far away is my enemy, wanted thing
    byte  botspeed = 1;
    boolean  blocked, notUsed = true;

    //needed so bot doesn't hold down use before reaching switch object
    if( cmd->buttons & BT_USE)
        notUsed = false;    //wouldn't be able to use switch

    memset (cmd,0,sizeof(*cmd));


    // Exit now if locked
    if( p->locked == true)
        return;

    if( p->playerstate == PST_LIVE)
    {
        cmd->angleturn = pmo->angle>>16;  // 32 bit angle to 16 bit angle
        cmd->aiming = 0;//p->aiming>>16;

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
            if( pbot->bestSeenItem )
            {
                // Move towards the item.
                target_dist = P_AproxDistance (pmo->x - pbot->bestSeenItem->x, pmo->y - pbot->bestSeenItem->y);
                botspeed = (target_dist > (64<<FRACBITS))? 1 : 0;
                B_TurnTowardsPoint(p, pbot->bestSeenItem->x, pbot->bestSeenItem->y);
                forwardmove = botforwardmove[botspeed];
                if( (((pbot->bestSeenItem->floorz - pmo->z)>>FRACBITS) > 24)
                    && (target_dist <= (100<<FRACBITS)))
                    cmd->buttons |= BT_JUMP;

                pbot->bestItem = NULL;
            }
            else if( pbot->closestEnemy && (pbot->closestEnemy->health > 0))
            {
                // Target exists and is still alive.
                // Prepare to attack the enemy.
                player_t * enemyp = pbot->closestEnemy->player;
                weapontype_t  enemy_readyweapon =
                 ( enemyp )? enemyp->readyweapon
                 : wp_nochange; // does not match anything
                boolean  enemy_linescan_weapon =
                   (enemy_readyweapon == wp_pistol)
                   || (enemy_readyweapon == wp_shotgun)
                   || (enemy_readyweapon == wp_chaingun);

                //debug_Printf("heading for an enemy\n");
                target_dist = P_AproxDistance (pmo->x - pbot->closestEnemy->x, pmo->y - pbot->closestEnemy->y);
                if( (target_dist > (300<<FRACBITS))
                    || (p->readyweapon == wp_fist)
                    || (p->readyweapon == wp_chainsaw))
                    forwardmove = botforwardmove[botspeed];
                if( (p->readyweapon == wp_missile) && (target_dist < (400<<FRACBITS)))
                    forwardmove = -botforwardmove[botspeed];

                // bot skill setting determines likelyhood bot will start strafing
                if(( target_dist <= bot_strafe_dist[ pbot->skill ])
                    || ( enemy_linescan_weapon && (pbot->skill >= 3) ))  // >= medium skill
                {
                    sidemove = botsidemove[botspeed];
                }

                B_AimWeapon(p);

                if( pbot->closestEnemy )
                {
                    pbot->lastMobj = pbot->closestEnemy;
                    pbot->lastMobjX = pbot->closestEnemy->x;
                    pbot->lastMobjY = pbot->closestEnemy->y;
                }
            }
            else
            {
                cmd->aiming = 0;
                //look for an unactivated switch/door
                if( (B_Random() > 190)  // not every time, so it does not obsess
                    && B_LookForSpecialLine(p, &x, &y)
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
                    target_dist =
                        P_AproxDistance (pmo->x - tmate->x, pmo->y - tmate->y);

                    // [WDJ]: Like MBF, Move away from friends when too close, except
                    // in certain situations.

                    // assume BOTH_FRIEND( p, tmate )
                    if( target_dist < EV_mbf_distfriend )
                    {
                        // Allowed to bump bot away, even in crusher.
                        if(( !P_IsOnLift( tmate )
                              && !P_IsUnderDamage( pmo ) )
                            || (target_dist <= (pmo->info->radius + tmate->info->radius + (FRACUNIT*35/16))) )  // bump
                        {
                            B_Turn_Away_Point(p, tmate->x, tmate->y);
                            forwardmove = botforwardmove[0];
                        }
                    }
                    else if( target_dist > (EV_mbf_distfriend + (8<<FRACBITS)) )
                    {
                        B_TurnTowardsPoint(p, tmate->x, tmate->y);
                        forwardmove = botforwardmove[botspeed];
                    }

                    pbot->lastMobj = tmate;
                    pbot->lastMobjX = tmate->x;
                    pbot->lastMobjY = tmate->y;
                }
                //since nothing else to do, go where last enemy/teammate was seen
                else if( pbot->lastMobj && (pbot->lastMobj->health > 0))
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
                    byte br = B_Random();
                    pbot->lastMobj = NULL;

                    if( pbot->bestItem && (br & 0x01) )  // do not obsess if cannot get to it
                    {
                        SearchNode_t* temp =
                            B_GetNodeAt(pbot->bestItem->x, pbot->bestItem->y);
                        //debug_Printf("found a best item at x:%d, y:%d\n", pbot->bestItem->x>>FRACBITS, pbot->bestItem->y>>FRACBITS);
                        if( pbot->destNode != temp)
                            B_LLClear(pbot->path);
                        pbot->destNode = temp;
                    }
                    else if( pbot->closestUnseenTeammate && (br & 0x02) )  // do not obsess if cannot get to it
                    {
                        SearchNode_t* temp =
                            B_GetNodeAt(pbot->closestUnseenTeammate->x, pbot->closestUnseenTeammate->y);
                        if( pbot->destNode != temp)
                            B_LLClear(pbot->path);
                        pbot->destNode = temp;
                    }
                    else if( pbot->closestUnseenEnemy && (br & 0x04) )  // do not obsess if cannot get to it
                    {
                        SearchNode_t* temp =
                            B_GetNodeAt(pbot->closestUnseenEnemy->x, pbot->closestUnseenEnemy->y);
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
                                                posX2x(pbot->path->first->x), posY2y(pbot->path->first->y) )
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
                            B_TurnTowardsPoint(p, posX2x(pbot->path->first->x), posY2y(pbot->path->first->y));
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

        if( cv_bot_speed.EV == 10 )  // botskill dependent speed
        {
            byte spd = bot_botskill_to_speed[ pbot->skill ];
            bot_speed_frac = bot_speed_frac_table[ spd ];
            bot_run_tics = bot_run_tics_table[ spd ];
        }

        // [WDJ] Limit the running.
        if( abs(forwardmove) > botforwardmove[0] )
        {
            // Running
            if( pbot->runtimer < bot_run_tics )
            {
                pbot->runtimer++;
                goto cmd_move;
            }

            // Tired, must walk.
            forwardmove = forwardmove>>1;  // walk
            if( pbot->runtimer == bot_run_tics )
            {
                pbot->runtimer += 10 * TICRATE;  // rest time
                goto cmd_move;
            }
        }
       
        if( pbot->runtimer > 0 )
        {
            pbot->runtimer--;
            // Run time needs to be proportional to bot_run_tics,
            // so reset timer to constant value.
            if( pbot->runtimer == bot_run_tics )
           {
                pbot->runtimer = (4 * TICRATE) - 2;  // reset hysterisis
           }
        }

    cmd_move:
        // [WDJ] Regulate the run speed.
        p->cmd.forwardmove = (forwardmove * bot_speed_frac) >> 7;
        p->cmd.sidemove = pbot->straferight ? sidemove : -sidemove;
        if( pbot->closestMissile )
            B_AvoidMissile(p, pbot->closestMissile);
    }
    else
    {
        // Dead
#ifdef BOT_VERSION_DETECT
        if( demoversion < 146 )
        {
            cmd->buttons |= BT_USE;	//I want to respawn
        }
        else
#endif
        {
            // Version 1.46
            // [WDJ] Slow down bot respawn, so they are not so overwhelming.
            cmd->buttons = 0;
            if( p->damagecount )
            {
                p->damagecount = 0;
                pbot->avoidtimer = cv_bot_respawn_time.EV * TICRATE; // wait
            }
            if( --pbot->avoidtimer <= 0 )
                cmd->buttons |= BT_USE;	//I want to respawn
        }
    }

    memcpy (netcmd, cmd, sizeof(*cmd));
} // end of BOT_Thinker


// Forget what cannot be saved.  To sync client and server bots.
void  B_forget_stuff( bot_t * bot )
{
    bot->destNode = NULL;
    B_LLClear( bot->path );
}

void  B_Destroy_Bot( player_t * player )
{
    bot_t * bot = player->bot;

    if( bot )
    {
        B_LLClear( bot->path );

        Z_Free( bot );

        player->bot = NULL;
    }
}

void  B_Create_Bot( player_t * player )
{
    bot_t * bot = player->bot;
    if( bot )
    {
        B_LLClear( bot->path );
        B_LLDelete( bot->path );
    }
    else
    {
        bot = Z_Malloc (sizeof(*bot), PU_STATIC, 0);
        player->bot = bot;
    }
    memset( bot, 0, sizeof(bot_t));
    bot->path = B_LLCreate();
}


void B_SpawnBot(bot_t* bot)
{
    byte sk;

    bot->avoidtimer = 0;
    bot->blockedcount = 0;
    bot->weaponchangetimer = 0;
    bot->runtimer = 0;

    bot->bestItem = NULL;
    bot->lastMobj = NULL;
    bot->destNode = NULL;

    // [WDJ] Bot skill = 0..5, Game skill = 0..4.
    switch( cv_bot_skill.EV )
    {
      case 6: // randmed
         sk = E_Random() % gameskill;  // random bots, 0 to gameskill-1
         break;
      case 7: // randgame
         sk = (E_Random() % gameskill) + 1;  // random bots, 1 to gameskill
         break;
      case 8: // gamemed
         sk = gameskill;  // bot lower skill levels
         break;
      case 9: // gameskill
         sk = gameskill + 1;  // bot upper skill levels
         break;
      default:  // fixed bot skill selection
         sk = cv_bot_skill.EV;  // 0..5
         break;
    }
    bot->skill = sk; // 0=crippled, 1=baby .. 5=nightmare

    B_LLClear(bot->path);
}
