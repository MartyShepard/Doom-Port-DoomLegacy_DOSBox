// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: p_inter.c 1405 2018-07-15 19:31:09Z wesleyjohnson $
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2016 by DooM Legacy Team.
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
// $Log: p_inter.c,v $
// Revision 1.30  2004/09/12 19:40:07  darkwolf95
// additional chex quest 1 support
//
// Revision 1.29  2002/11/30 18:39:58  judgecutor
// * Fix CR+LF problem
// * Fix FFW bug (player spawining istead of weapon)
//
// Revision 1.28  2002/09/27 16:40:09  tonyd
// First commit of acbot
//
// Revision 1.27  2002/08/24 22:42:03  hurdler
// Apply Robert Hogberg patches
//
// Revision 1.26  2002/07/26 15:21:36  hurdler
//
// Revision 1.25  2002/07/23 15:07:11  mysterial
// Messages to second player appear on his half of the screen
//
// Revision 1.24  2002/01/21 23:14:28  judgecutor
// Frag's Weapon Falling fixes
//
// Revision 1.23  2001/12/26 22:46:01  hurdler
// Revision 1.22  2001/12/26 22:42:52  hurdler
// Revision 1.18  2001/06/10 21:16:01  bpereira
// Revision 1.17  2001/05/27 13:42:47  bpereira
// Revision 1.16  2001/05/16 21:21:14  bpereira
//
// Revision 1.15  2001/05/14 19:02:58  metzgermeister
//   * Fixed floor not moving up with player on E3M1
//   * Fixed crash due to oversized string in screen message ... bad bug!
//   * Corrected some typos
//   * fixed sound bug in SDL
//
// Revision 1.14  2001/04/19 05:51:47  metzgermeister
// fixed 10 shells instead of 4 - bug
//
// Revision 1.13  2001/03/30 17:12:50  bpereira
// Revision 1.12  2001/02/24 13:35:20  bpereira
//
// Revision 1.11  2001/01/25 22:15:43  bpereira
// added heretic support
//
// Revision 1.10  2000/11/02 17:50:07  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.9  2000/10/02 18:25:45  bpereira
// Revision 1.8  2000/10/01 10:18:17  bpereira
// Revision 1.7  2000/09/28 20:57:16  bpereira
// Revision 1.6  2000/08/31 14:30:55  bpereira
// Revision 1.5  2000/04/16 18:38:07  bpereira
//
// Revision 1.4  2000/04/04 00:32:46  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.3  2000/02/27 00:42:10  hurdler
// Revision 1.2  2000/02/26 00:28:42  hurdler
// Mostly bug fix (see borislog.txt 23-2-2000, 24-2-2000)
//
//
// DESCRIPTION:
//      Handling interactions (i.e., collisions).
//
//-----------------------------------------------------------------------------

#include "doomincl.h"
#include "p_local.h"
#include "p_tick.h"
  // class-list
#include "p_inter.h"
#include "g_game.h"
#include "g_input.h"
  // cv_allowrocketjump
#include "i_system.h"
  //I_Tactile currently has no effect
#include "am_map.h"
#include "dstrings.h"
#include "m_random.h"
#include "s_sound.h"
#include "r_main.h"
#include "st_stuff.h"

#define BONUSADD        6
#define PICKUP_FLASH_TICS     10


// a weapon is found with two clip loads,
// a big item has five clip loads
int     maxammo[NUMAMMO] = {200, 50, 300, 50};
int     clipammo[NUMAMMO] = {10, 4, 20, 1};

consvar_t cv_fragsweaponfalling
  = {"fragsweaponfalling"   ,"0", CV_NETVAR, CV_YesNo};

// added 4-2-98 (Boris) for dehacked patch
// (i don't like that but do you see another solution ?)
int     MAXHEALTH= 100;

//--------------------------------------------------------------------------
//
// PROC P_SetMessage
//
//--------------------------------------------------------------------------

// [WDJ] Replaces the Heretic ultimatemsg flag for IDKFA and IDDQD.
// That made the IDKFA and IDDQD message block most other messages,
// until the next player tic.
// But Legacy message timing is different and separate.
// A user control limits the messages seen based on the message level,
// (Off, minimal, normal, verbose, debug).
// This also uses the new message level as priority for competing messages
// that are issued in the same tic.

// Player Message for cheats and object interactions.
// Subject to message level control.
// This does not attempt to duplicate Heretic message blocking identically,
// as it has finer control.
// msglevel:
//   0..9    debug
//   10..19  verbose
//   20..29  normal play
//   30..39  minimal msg play
//   40..49  major
//   50..59  god mode messages
//   60..64  mandatory (even when messages are off)
void P_SetMessage(player_t *player, char *message, byte msglevel)
{
    // This actually optimizes cheaper than a table.
    switch( cv_showmessages.EV )
    {
     case 5: // dev level
     case 4: // debug level
        break;
     case 3: // verbose level
        if( msglevel < 10 )  return;
        break;
     case 2: // play level
        if( msglevel < 20 )  return;
        break;
     case 1: // minimal msg play level
        if( msglevel < 30 )  return;
        break;
     default: // off
        // Does not block mandatory messages.
        if( msglevel < 60 )  return;
        break;
    }

    // Check competing messages in same tic.
    // Per player, to support splitplayer.
    if( player->msglevel > msglevel )
        return;

    player->msglevel = msglevel;
    player->message = message;
    //player->messageTics = MESSAGETICS;
    //BorderTopRefresh = true;
}

//
// GET STUFF
//

// added by Boris : preferred weapons order
void VerifFavoritWeapon (player_t *player)
{
    int newweapon;

    if (player->pendingweapon != wp_nochange)
        return;

    newweapon = FindBestWeapon(player);

    if (newweapon != player->readyweapon)
        player->pendingweapon = newweapon;
}

int FindBestWeapon(player_t *player)
{
    int actualprior, actualweapon = 0, i;

    actualprior = -1;

    for (i = 0; i < NUMWEAPONS; i++)
    {
        // skip super shotgun for non-Doom2
        if (gamemode!=doom2_commercial && i==wp_supershotgun)
            continue;
        // skip plasma-bfg in shareware
        if (gamemode==doom_shareware && (i==wp_plasma || i==wp_bfg))
            continue;

        if (player->weaponowned[i] &&
            actualprior < player->favoritweapon[i] &&
            player->ammo[player->weaponinfo[i].ammo] >= player->weaponinfo[i].ammopershoot)
        {
            actualweapon = i;
            actualprior = player->favoritweapon[i];
         }
    }
    
    return actualweapon;
}

static const weapontype_t GetAmmoChange[] =
{
        wp_goldwand,
        wp_crossbow,
        wp_blaster,
        wp_skullrod,
        wp_phoenixrod,
        wp_mace
};

//
// P_GiveAmmo
// Num is the number of clip loads,
// not the individual count (0= 1/2 clip).
// Returns false if the ammo can't be picked up at all
//

static
boolean P_GiveAmmo ( player_t*     player,
                     ammotype_t    ammo,
                     int           count )
{
    unsigned int oldammo, newammo;

    if (ammo == am_noammo)
        return false;

    if (ammo < 0 || ammo > NUMAMMO)
    {
        GenPrintf(EMSG_warn, "\2P_GiveAmmo: bad type %i", ammo);
        return false;
    }

    oldammo = player->ammo[ammo];
    if ( oldammo == player->maxammo[ammo]  )
        return false;
/*
    if (num)
        num *= clipammo[ammo];
    else
        num = clipammo[ammo]/2;
*/
    if (gameskill == sk_baby
        || gameskill == sk_nightmare)
    {
        if( EN_heretic )
            count += count>>1;  // add 50% more
        else
        {
            // give double ammo in trainer mode,
            // you'll need in nightmare
            count <<= 1;  // add 100% more
        }
    }


    newammo = oldammo + count;

    if (newammo > player->maxammo[ammo])
        newammo = player->maxammo[ammo];

    player->ammo[ammo] = newammo;

    if( newammo > oldammo )
    {
        // ammo pickup screen indication
        if(ammo == player->weaponinfo[player->readyweapon].ammo)
        {
            player->ammo_pickup = PICKUP_FLASH_TICS; // ammo for current weapon
        }
        else
        {
            player->ammo_pickup += PICKUP_FLASH_TICS/2;  // some other ammo
            if( player->ammo_pickup > 35 )
                player->ammo_pickup = 35;
        }
    }

    // If non zero ammo,
    // don't change up weapons,
    // player was lower on purpose.
    if (oldammo)
        return true;

    // We were down to zero,
    // so select a new weapon.
    // Preferences are not user selectable.

    // Boris hack for preferred weapons order...
    if(!player->originalweaponswitch)
    {
       if(player->ammo[player->weaponinfo[player->readyweapon].ammo]
                     < player->weaponinfo[player->readyweapon].ammopershoot)
         VerifFavoritWeapon(player);
       return true;
    }
    else //eof Boris
    if( EN_heretic )
    {
        if( ( player->readyweapon == wp_staff
           || player->readyweapon == wp_gauntlets) 
           && player->weaponowned[GetAmmoChange[ammo]])
             player->pendingweapon = GetAmmoChange[ammo];
    }
    else
    switch (ammo)
    {
      case am_clip:
        if (player->readyweapon == wp_fist)
        {
            if (player->weaponowned[wp_chaingun])
                player->pendingweapon = wp_chaingun;
            else
                player->pendingweapon = wp_pistol;
        }
        break;

      case am_shell:
        if (player->readyweapon == wp_fist
            || player->readyweapon == wp_pistol)
        {
            if (player->weaponowned[wp_shotgun])
                player->pendingweapon = wp_shotgun;
        }
        break;

      case am_cell:
        if (player->readyweapon == wp_fist
            || player->readyweapon == wp_pistol)
        {
            if (player->weaponowned[wp_plasma])
                player->pendingweapon = wp_plasma;
        }
        break;

      case am_misl:
        if (player->readyweapon == wp_fist)
        {
            if (player->weaponowned[wp_missile])
                player->pendingweapon = wp_missile;
        }
      default:
        break;
    }

    return true;
}

// ammo get with the weapon
int GetWeaponAmmo[NUMWEAPONS] =
{
    0,  // staff       fist
   20,  // gold wand   pistol
    8,  // crossbow    shotgun
   20,  // blaster     chaingun
    2,  // skull rod   missile    
   40,  // phoenix rod plasma     
   40,  // mace        bfg        
    0,  // gauntlets   chainsaw   
    8,  // beak        supershotgun
};

static int has_ammo_dropped = 0;
//
// P_GiveWeapon
// The weapon name may have a MF_DROPPED flag ored in.
//
static
boolean P_GiveWeapon ( player_t*     player,
                       weapontype_t  weapon,
                       boolean       dropped )
{
    boolean     gaveammo;
    boolean     gaveweapon;
    int         ammo_count;

    if( multiplayer && (cv_deathmatch.EV != 2) && !dropped )
    {
        // leave placed weapons forever on net games
        if (player->weaponowned[weapon])
            return false;

        player->bonuscount += BONUSADD;
        player->weapon_pickup = PICKUP_FLASH_TICS;
        player->weaponowned[weapon] = true;

        if( cv_deathmatch.EV )
            P_GiveAmmo (player, player->weaponinfo[weapon].ammo, 5*clipammo[player->weaponinfo[weapon].ammo]);
        else
            P_GiveAmmo (player, player->weaponinfo[weapon].ammo, GetWeaponAmmo[weapon]);

        // Boris hack preferred weapons order...
        if(player->originalweaponswitch
        || player->favoritweapon[weapon] > player->favoritweapon[player->readyweapon])
            player->pendingweapon = weapon;     // do like Doom2 original

        //added:16-01-98:changed consoleplayer to displayplayer
        //               (hear the sounds from the viewpoint)
        if (player == displayplayer_ptr
            || (cv_splitscreen.EV && player == displayplayer2_ptr))  // NULL when unused
            S_StartSound(sfx_wpnup);
        return false;
    }

    if (player->weaponinfo[weapon].ammo != am_noammo)
    {
        // give one clip with a dropped weapon,
        // two clips with a found weapon
        if (dropped)
        {
            ammo_count = has_ammo_dropped ? 
                (has_ammo_dropped < 0 ? 0 : has_ammo_dropped) : clipammo[player->weaponinfo[weapon].ammo];
            //gaveammo = P_GiveAmmo (player, player->weaponinfo[weapon].ammo, clipammo[player->weaponinfo[weapon].ammo]);
        }
        else
        {
            //gaveammo = P_GiveAmmo (player, player->weaponinfo[weapon].ammo, GetWeaponAmmo[weapon]);
            ammo_count = GetWeaponAmmo[weapon];
        }
        gaveammo = P_GiveAmmo (player, player->weaponinfo[weapon].ammo, ammo_count);
    }
    else
        gaveammo = false;

   if (player->weaponowned[weapon])
        gaveweapon = false;
    else
    {
        gaveweapon = true;
        player->weaponowned[weapon] = true;
        player->weapon_pickup = PICKUP_FLASH_TICS;
        if (player->originalweaponswitch
        || player->favoritweapon[weapon] > player->favoritweapon[player->readyweapon])
            player->pendingweapon = weapon;    // Doom2 original stuff
    }

    return (gaveweapon || gaveammo);
}



//
// P_GiveHealth  (P_GiveBody)
// Returns false if the health isn't needed at all
//
boolean P_GiveHealth ( player_t*     player,
                       int           num )
{
    int max;
    
    max = MAXHEALTH;
    if(player->chickenTics)
        max = MAXCHICKENHEALTH;

    if (player->health >= max)
        return false;

    player->health += num;
    if (player->health > max)
        player->health = max;
    player->mo->health = player->health;

    player->health_pickup = PICKUP_FLASH_TICS;
    return true;
}



//
// P_GiveArmor
// Returns false if the armor is worse
// than the current armor.
//
static
boolean P_GiveArmor ( player_t*     player,
                      int           armortype )
{
    int         hits;

    hits = armortype*100;
    if (player->armorpoints >= hits)
        return false;   // don't pick up

    player->armortype = armortype;
    player->armorpoints = hits;

    player->armor_pickup = PICKUP_FLASH_TICS;
    return true;
}



//
// P_GiveCard
//
static
boolean P_GiveCard ( player_t*     player,
                     card_t        card )
{
    if (player->cards & card )
        return false;

    player->cards |= card;
    player->bonuscount = BONUSADD;
    player->key_pickup = PICKUP_FLASH_TICS;
    return true;
}


//
// P_GivePower
//
//  power : powertype index
boolean P_GivePower ( player_t* player, int power )
{
    // Was using inventory to detect heretic and hexen rules,
    // but these rules have nothing to do with inventory.
    if (power == pw_invulnerability)
    {
        // Already have it
        if( EN_heretic_hexen && (player->powers[power] > BLINKTHRESHOLD ) )
            return false;

        player->powers[power] = INVULNTICS;
        return true;
    }
    if(power == pw_weaponlevel2)
    {
        // Already have it
        if( EN_heretic_hexen && (player->powers[power] > BLINKTHRESHOLD) )
            return false;

        player->powers[power] = WPNLEV2TICS;
        player->weaponinfo = wpnlev2info;
        return true;
    }
    if (power == pw_invisibility)
    {
        // Already have it
        if( EN_heretic_hexen && (player->powers[power] > BLINKTHRESHOLD) )
            return false;

        player->powers[power] = INVISTICS;
        player->mo->flags |= MF_SHADOW;
        return true;
    }
    if(power == pw_flight)
    {
        // Already have it
        if(player->powers[power] > BLINKTHRESHOLD)
            return(false);

        player->powers[power] = FLIGHTTICS;
        player->mo->flags2 |= MF2_FLY;
        player->mo->flags |= MF_NOGRAVITY;
        if(player->mo->z <= player->mo->floorz)
        {
            player->flyheight = 10; // thrust the player in the air a bit
        }
        return(true);
    }
    if (power == pw_infrared)
    {
        // Already have it
        if(player->powers[power] > BLINKTHRESHOLD)
            return(false);

        player->powers[power] = INFRATICS;
        return true;
    }

    if (power == pw_ironfeet)
    {
        player->powers[power] = IRONTICS;
        return true;
    }

    if (power == pw_strength)
    {
        P_GiveHealth (player, 100);
        player->powers[power] = 1;
        return true;
    }

    if (player->powers[power])
        return false;   // already got it

    player->powers[power] = 1;
    return true;
}

// Boris stuff : dehacked patches hack
int max_armor=200;
int green_armor_class=1;
int blue_armor_class=2;
int maxsoul=200;
int soul_health=100;
int mega_health=200;
// eof Boris

//---------------------------------------------------------------------------
//
// FUNC P_GiveArtifact
//
// Returns true if artifact accepted.
//
//---------------------------------------------------------------------------

boolean P_GiveArtifact(player_t *player, artitype_t arti, mobj_t *mo)
{
    int i;
    
    i = 0;
    while(player->inventory[i].type != arti && i < player->inventorySlotNum)
    {
        i++;
    }
    if(i == player->inventorySlotNum)
    {
        player->inventory[i].count = 1;
        player->inventory[i].type = arti;
        player->inventorySlotNum++;
    }
    else
    {
        if(player->inventory[i].count >= MAXARTECONT)
        { // Player already has 16 of this item
            return(false);
        }
        player->inventory[i].count++;
    }
    if( player->inventory[player->inv_ptr].count == 0 )
        player->inv_ptr = i;

    if(mo && (mo->flags&MF_COUNTITEM))
    {
        player->itemcount++;
    }
    return(true);
}


//---------------------------------------------------------------------------
//
// PROC P_SetDormantArtifact
//
// Removes the MF_SPECIAL flag, and initiates the artifact pickup
// animation.
//
//---------------------------------------------------------------------------

static
void P_SetDormantArtifact(mobj_t *arti)
{
        arti->flags &= ~MF_SPECIAL;
        if( cv_deathmatch.EV && (arti->type != MT_ARTIINVULNERABILITY )
                && (arti->type != MT_ARTIINVISIBILITY))
        {
                P_SetMobjState(arti, S_DORMANTARTI1);
        }
        else
        { // Don't respawn
                P_SetMobjState(arti, S_DEADARTI1);
        }
        S_StartObjSound(arti, sfx_artiup);
}

//---------------------------------------------------------------------------
//
// PROC A_RestoreArtifact
//
//---------------------------------------------------------------------------

void A_RestoreArtifact(mobj_t *arti)
{
        arti->flags |= MF_SPECIAL;
        P_SetMobjState(arti, arti->info->spawnstate);
        S_StartObjSound(arti, sfx_itmbk);
}

//----------------------------------------------------------------------------
//
// PROC P_HideSpecialThing
//
//----------------------------------------------------------------------------

void P_HideSpecialThing(mobj_t *thing)
{
        thing->flags &= ~MF_SPECIAL;
        thing->flags2 |= MF2_DONTDRAW;
        P_SetMobjState(thing, S_HIDESPECIAL1);
}

//---------------------------------------------------------------------------
//
// PROC A_RestoreSpecialThing1
//
// Make a special thing visible again.
//
//---------------------------------------------------------------------------

void A_RestoreSpecialThing1(mobj_t *thing)
{
        if(thing->type == MT_WMACE)
        { // Do random mace placement
                P_RepositionMace(thing);
        }
        thing->flags2 &= ~MF2_DONTDRAW;
        S_StartObjSound(thing, sfx_itmbk);
}

//---------------------------------------------------------------------------
//
// PROC A_RestoreSpecialThing2
//
//---------------------------------------------------------------------------

void A_RestoreSpecialThing2(mobj_t *thing)
{
        thing->flags |= MF_SPECIAL;
        P_SetMobjState(thing, thing->info->spawnstate);
}


//----------------------------------------------------------------------------
//
// PROC A_HideThing
//
//----------------------------------------------------------------------------

void A_HideThing(mobj_t *actor)
{
        //P_UnsetThingPosition(actor);
        actor->flags2 |= MF2_DONTDRAW;
}

//----------------------------------------------------------------------------
//
// PROC A_UnHideThing
//
//----------------------------------------------------------------------------

void A_UnHideThing(mobj_t *actor)
{
        //P_SetThingPosition(actor);
        actor->flags2 &= ~MF2_DONTDRAW;
}


//
// P_TouchSpecialThing
//
void P_TouchSpecialThing ( mobj_t*       special,
                           mobj_t*       toucher )
{                  
    player_t*   player;
    char *      msg = NULL;
    byte        msglevel = 20;  // normal play for common ammo
    spritenum_t group = 0xFFFF;  // combined handling by spritenum
    boolean     special_dropped;  // special item was dropped
    int         val, i;
    fixed_t     delta;
    int         sound;

    delta = special->z - toucher->z;

    //SoM: 3/27/2000: For some reason, the old code allowed the player to
    //grab items that were out of reach...
    if (delta > toucher->height
        || delta < -special->height)
    {
        // out of reach
        return;
    }

    // Dead thing touching.
    // Can happen with a sliding player corpse.
    if (toucher->health <= 0 || toucher->flags&MF_CORPSE)
        return;

    sound = sfx_itemup;
    player = toucher->player;

    if( player &&
        toucher != player->mo )  // voodoo doll toucher
    {
        if( voodoo_mode >= VM_target )
        {
            // Target last player to trigger a switch or linedef.
            if( spechit_player && spechit_player->mo )
            {
                player = spechit_player;
                toucher = player->mo;
            }
        }
        if( !player || !player->mo )
            return; // player left or voodoo multiplayer spawn
    }

#ifdef PARANOIA
    if( !player )
        I_Error("P_TouchSpecialThing: without player\n");
#endif


    // FWF support
    has_ammo_dropped = special->dropped_ammo_count;

    // Avoid muiltiple conversions to boolean.
    special_dropped = (boolean)(special->flags & MF_DROPPED);

    // Identify by sprite.
    switch (special->sprite)
    {
      case SPR_SHLD: // Item_Shield1
        // armor
      case SPR_ARM1:  // common armor
        if (!P_GiveArmor (player, green_armor_class))
            return;
        msg = GOTARMOR;
        msglevel = 28;
        break;

      case SPR_SHD2: // Item_Shield2
      case SPR_ARM2:
        if (!P_GiveArmor (player, blue_armor_class))
            return;
        msg = GOTMEGA;
        msglevel = 37;
        break;

        // bonus items
      case SPR_BON1:  // common health inc
        val = player->health + 1;               // can go over 100%
        if (val > 2*MAXHEALTH)
            val = 2*MAXHEALTH;
        if((val > player->health) && (player->health_pickup < 35))
            player->health_pickup += PICKUP_FLASH_TICS/2;
        player->mo->health = player->health = val;
        msg = GOTHTHBONUS;
        msglevel = 22;
        break;

      case SPR_BON2:  // common armor inc
        val = player->armorpoints + 1;          // can go over 100%
        if (val > max_armor)
            val = max_armor;
        if((val > player->armorpoints) && (player->armor_pickup < 35))
            player->armor_pickup += PICKUP_FLASH_TICS/2;
        player->armorpoints = val;       
        if (!player->armortype)
            player->armortype = 1;
        msg = GOTARMBONUS;
        msglevel = 22;
        break;

      case SPR_SOUL:
        val = player->health + soul_health;
        if (val > maxsoul)
            val = maxsoul;
        if((val > player->health) && (player->health_pickup < 35))
            player->health_pickup += PICKUP_FLASH_TICS;
        player->mo->health = player->health = val;
        msg = GOTSUPER;
        msglevel = 38;
        sound = sfx_getpow;
        break;

      case SPR_MEGA:
        if (gamemode != doom2_commercial)
            return;
        player->health = mega_health;
        player->mo->health = player->health;
        if(player->health_pickup < 35)
            player->health_pickup += PICKUP_FLASH_TICS;
        P_GiveArmor (player,2);
        msg = GOTMSPHERE;
        msglevel = 38;
        sound = sfx_getpow;
        break;

        // cards
        // leave cards for everyone
      case SPR_BKYY: // Key_Blue
      case SPR_BKEY:
        group = SPR_BKEY;        
        if( P_GiveCard (player, it_bluecard) )
        {
            msg = GOTBLUECARD;
            msglevel = 45;
        }
        break;

      case SPR_CKYY: // Key_Yellow
      case SPR_YKEY:
        group = SPR_BKEY;        
        if( P_GiveCard (player, it_yellowcard) )
        {
            msg = GOTYELWCARD;
            msglevel = 45;
        }
        break;

      case SPR_AKYY: // Key_Green
      case SPR_RKEY:
        group = SPR_BKEY;        
        if (P_GiveCard (player, it_redcard))
        {
            msg = GOTREDCARD;
            msglevel = 45;
        }
        break;

      case SPR_BSKU:
        group = SPR_BKEY;        
        if (P_GiveCard (player, it_blueskull))
        {
            msg = GOTBLUESKUL;
            msglevel = 45;
        }
        break;

      case SPR_YSKU:
        group = SPR_BKEY;        
        if (P_GiveCard (player, it_yellowskull))
        {
            msg = GOTYELWSKUL;
            msglevel = 45;
        }
        break;

      case SPR_RSKU:
        group = SPR_BKEY;        
        if (P_GiveCard (player, it_redskull))
        {
            msg = GOTREDSKULL;
            msglevel = 45;
        }
        break;

        // medikits, heals
      case SPR_PTN1: // Item_HealingPotion
      case SPR_STIM:
        if (!P_GiveHealth (player, 10))
            return;
        msg = GOTSTIM;
        break;

      case SPR_MEDI:
        // [WDJ] fix medkit message
        // DoomWiki fix would put messages first, but that would give
        // message even when not using the medkit
        if (!P_GiveHealth (player, 25))  // add 25 to health
            return;
        // if health was used, then give message
        // use fix from prboom, thanks to Quasar
        if (player->health < 50) // old health was < 25, before adding 25
        {
            msg = GOTMEDINEED;
            msglevel = 31;
        }
        else
        {
            msg = GOTMEDIKIT;
            msglevel = 23;
        }
        break;

        // heretic Artifacts :
      case SPR_PTN2: // Arti_HealingPotion
          if(P_GiveArtifact(player, arti_health, special))
          {
              P_SetMessage(player, TXT_ARTIHEALTH, 28);
              P_SetDormantArtifact(special);
          }
          return;
      case SPR_SOAR: // Arti_Fly
          if(P_GiveArtifact(player, arti_fly, special))
          {
              P_SetMessage(player, TXT_ARTIFLY, 31);
              P_SetDormantArtifact(special);
          }
          return;
      case SPR_INVU: // Arti_Invulnerability
          if(P_GiveArtifact(player, arti_invulnerability, special))
          {
              P_SetMessage(player, TXT_ARTIINVULNERABILITY, 31);
              P_SetDormantArtifact(special);
          }
          return;
      case SPR_PWBK: // Arti_TomeOfPower
          if(P_GiveArtifact(player, arti_tomeofpower, special))
          {
              P_SetMessage(player, TXT_ARTITOMEOFPOWER, 31);
              P_SetDormantArtifact(special);
          }
          return;
      case SPR_INVS: // Arti_Invisibility
          if(P_GiveArtifact(player, arti_invisibility, special))
          {
              P_SetMessage(player, TXT_ARTIINVISIBILITY, 31);
              P_SetDormantArtifact(special);
          }
          return;
      case SPR_EGGC: // Arti_Egg
          if(P_GiveArtifact(player, arti_egg, special))
          {
              P_SetMessage(player, TXT_ARTIEGG, 31);
              P_SetDormantArtifact(special);
          }
          return;
      case SPR_SPHL: // Arti_SuperHealth
          if(P_GiveArtifact(player, arti_superhealth, special))
          {
              P_SetMessage(player, TXT_ARTISUPERHEALTH, 31);
              P_SetDormantArtifact(special);
          }
          return;
      case SPR_TRCH: // Arti_Torch
          if(P_GiveArtifact(player, arti_torch, special))
          {
              P_SetMessage(player, TXT_ARTITORCH, 31);
              P_SetDormantArtifact(special);
          }
          return;
      case SPR_FBMB: // Arti_FireBomb
          if(P_GiveArtifact(player, arti_firebomb, special))
          {
              P_SetMessage(player, TXT_ARTIFIREBOMB, 31);
              P_SetDormantArtifact(special);
          }
          return;
      case SPR_ATLP: // Arti_Teleport
          if(P_GiveArtifact(player, arti_teleport, special))
          {
              P_SetMessage(player, TXT_ARTITELEPORT, 31);
              P_SetDormantArtifact(special);
          }
          return;

        // power ups
      case SPR_PINV:
        if (!P_GivePower (player, pw_invulnerability))
            return;
        msg = GOTINVUL;
        msglevel = 34;
        sound = sfx_getpow;
        break;

      case SPR_PSTR:
        if (!P_GivePower (player, pw_strength))
            return;
        msg = GOTBERSERK;
        msglevel = 34;
        if (player->readyweapon != wp_fist)
            player->pendingweapon = wp_fist;
        sound = sfx_getpow;
        break;

      case SPR_PINS:
        if (!P_GivePower (player, pw_invisibility))
            return;
        msg = GOTINVIS;
        msglevel = 34;
        sound = sfx_getpow;
        break;

      case SPR_SUIT:
        if (!P_GivePower (player, pw_ironfeet))
            return;
        msg = GOTSUIT;
        msglevel = 32;
        sound = sfx_getpow;
        break;

      case SPR_SPMP: // Item_SuperMap
      case SPR_PMAP:
        if (!P_GivePower (player, pw_allmap))
            return;
        msg = GOTMAP;
        msglevel = 31;
        if( EN_doom_etc )
            sound = sfx_getpow;
        break;

      case SPR_PVIS:
        if (!P_GivePower (player, pw_infrared))
            return;
        msg = GOTVISOR;
        msglevel = 32;
        sound = sfx_getpow;
        break;

        // heretic Ammo
      case SPR_AMG1: // Ammo_GoldWandWimpy
        if(!P_GiveAmmo(player, am_goldwand, special->health))
            return;
        msg = TXT_AMMOGOLDWAND1;
        break;
      case SPR_AMG2: // Ammo_GoldWandHefty
        if(!P_GiveAmmo(player, am_goldwand, special->health))
            return;
        msg = TXT_AMMOGOLDWAND2;
        break;
      case SPR_AMM1: // Ammo_MaceWimpy
        if(!P_GiveAmmo(player, am_mace, special->health))
            return;
        msg = TXT_AMMOMACE1;
        break;
      case SPR_AMM2: // Ammo_MaceHefty
        if(!P_GiveAmmo(player, am_mace, special->health))
            return;
        msg = TXT_AMMOMACE2;
        break;
      case SPR_AMC1: // Ammo_CrossbowWimpy
        if(!P_GiveAmmo(player, am_crossbow, special->health))
            return;
        msg = TXT_AMMOCROSSBOW1;
        break;
      case SPR_AMC2: // Ammo_CrossbowHefty
        if(!P_GiveAmmo(player, am_crossbow, special->health))
            return;
        msg = TXT_AMMOCROSSBOW2;
        break;
      case SPR_AMB1: // Ammo_BlasterWimpy
        if(!P_GiveAmmo(player, am_blaster, special->health))
            return;
        msg = TXT_AMMOBLASTER1;
        break;
      case SPR_AMB2: // Ammo_BlasterHefty
        if(!P_GiveAmmo(player, am_blaster, special->health))
            return;
        msg = TXT_AMMOBLASTER2;
        break;
      case SPR_AMS1: // Ammo_SkullRodWimpy
        if(!P_GiveAmmo(player, am_skullrod, special->health))
            return;
        msg = TXT_AMMOSKULLROD1;
        break;
      case SPR_AMS2: // Ammo_SkullRodHefty
        if(!P_GiveAmmo(player, am_skullrod, special->health))
            return;
        msg = TXT_AMMOSKULLROD2;
        break;
      case SPR_AMP1: // Ammo_PhoenixRodWimpy
        if(!P_GiveAmmo(player, am_phoenixrod, special->health))
            return;
        msg = TXT_AMMOPHOENIXROD1;
        break;
      case SPR_AMP2: // Ammo_PhoenixRodHefty
        if(!P_GiveAmmo(player, am_phoenixrod, special->health))
            return;
        msg = TXT_AMMOPHOENIXROD2;
        break;

        // ammo
      case SPR_CLIP:
        if (!P_GiveAmmo (player, am_clip,
               ((special_dropped)? clipammo[am_clip]/2 : clipammo[am_clip]) ))
        msg = GOTCLIP;
        break;

      case SPR_AMMO:
        if (!P_GiveAmmo (player, am_clip,5*clipammo[am_clip]))
            return;
        msg = GOTCLIPBOX;
        break;

      case SPR_ROCK:
        if (!P_GiveAmmo (player, am_misl,clipammo[am_misl]))
            return;
        msg = GOTROCKET;
        break;

      case SPR_BROK:
        if (!P_GiveAmmo (player, am_misl,5*clipammo[am_misl]))
            return;
        msg = GOTROCKBOX;
        break;

      case SPR_CELL:
        if (!P_GiveAmmo (player, am_cell,clipammo[am_cell]))
            return;
        msg = GOTCELL;
        break;

      case SPR_CELP:
        if (!P_GiveAmmo (player, am_cell,5*clipammo[am_cell]))
            return;
        msg = GOTCELLBOX;
        break;

      case SPR_SHEL:
        if (!P_GiveAmmo (player, am_shell,clipammo[am_shell]))
            return;
        msg = GOTSHELLS;
        break;

      case SPR_SBOX:
        if (!P_GiveAmmo (player, am_shell,5*clipammo[am_shell]))
            return;
        msg = GOTSHELLBOX;
        break;

      case SPR_BPAK:
        if (!player->backpack)
        {
            for (i=0 ; i<NUMAMMO ; i++)
                player->maxammo[i] *= 2;
            player->backpack = true;
        }
        for (i=0 ; i<NUMAMMO ; i++)
            P_GiveAmmo (player, i, clipammo[i]);
        msg = GOTBACKPACK;
        msglevel = 27;
        break;

      case SPR_BAGH: // Item_BagOfHolding
        if(!player->backpack)
        {
            for(i = 0; i < NUMAMMO; i++)
                player->maxammo[i] *= 2;
            player->backpack = true;
        }
        P_GiveAmmo(player, am_goldwand, AMMO_GWND_WIMPY);
        P_GiveAmmo(player, am_blaster, AMMO_BLSR_WIMPY);
        P_GiveAmmo(player, am_crossbow, AMMO_CBOW_WIMPY);
        P_GiveAmmo(player, am_skullrod, AMMO_SKRD_WIMPY);
        P_GiveAmmo(player, am_phoenixrod, AMMO_PHRD_WIMPY);
        msg = TXT_ITEMBAGOFHOLDING;
        msglevel = 27;
        break;

        // weapons
      case SPR_BFUG:
        if (!P_GiveWeapon (player, wp_bfg, special_dropped) )
            return;
        msg = GOTBFG9000;
        msglevel = 38;
        sound = sfx_wpnup;
        break;

      case SPR_MGUN:
        if (!P_GiveWeapon (player, wp_chaingun, special_dropped) )
            return;
        msg = GOTCHAINGUN;
        msglevel = 29;
        sound = sfx_wpnup;
        break;

      case SPR_CSAW:
        if (!P_GiveWeapon (player, wp_chainsaw, false) )
            return;
        msg = GOTCHAINSAW;
        msglevel = 21;
        sound = sfx_wpnup;
        break;

      case SPR_LAUN:
        if (!P_GiveWeapon (player, wp_missile, special_dropped) )
            return;
        msg = GOTLAUNCHER;
        msglevel = 32;
        sound = sfx_wpnup;
        break;

      case SPR_PLAS:
        if (!P_GiveWeapon (player, wp_plasma, special_dropped) )
            return;
        msg = GOTPLASMA;
        msglevel = 32;
        sound = sfx_wpnup;
        break;

      case SPR_SHOT:
        if (!P_GiveWeapon (player, wp_shotgun, special_dropped) )
            return;
        msg = GOTSHOTGUN;
        msglevel = 24;
        sound = sfx_wpnup;
        break;

      case SPR_SGN2:
        if (!P_GiveWeapon (player, wp_supershotgun, special_dropped) )
            return;
        msg = GOTSHOTGUN2;
        msglevel = 32;
        sound = sfx_wpnup;
        break;

      // heretic weapons
      case SPR_WMCE: // Weapon_Mace
        if(!P_GiveWeapon(player, wp_mace, special_dropped))
            return;
        msg = TXT_WPNMACE;
        msglevel = 32;
        sound = sfx_wpnup;
        break;
      case SPR_WBOW: // Weapon_Crossbow
        if(!P_GiveWeapon(player, wp_crossbow, special_dropped))
            return;
        msg = TXT_WPNCROSSBOW;
        msglevel = 24;
        sound = sfx_wpnup;
        break;
      case SPR_WBLS: // Weapon_Blaster
        if(!P_GiveWeapon(player, wp_blaster, special_dropped))
            return;
        msg = TXT_WPNBLASTER;
        msglevel = 32;
        sound = sfx_wpnup;
        break;
      case SPR_WSKL: // Weapon_SkullRod
        if(!P_GiveWeapon(player, wp_skullrod, special_dropped))
            return;
        msg = TXT_WPNSKULLROD;
        msglevel = 36;
        sound = sfx_wpnup;
        break;
      case SPR_WPHX: // Weapon_PhoenixRod
        if(!P_GiveWeapon(player, wp_phoenixrod, special_dropped))
            return;
        msg = TXT_WPNPHOENIXROD;
        msglevel = 38;
        sound = sfx_wpnup;
        break;
      case SPR_WGNT: // Weapon_Gauntlets
        if(!P_GiveWeapon(player, wp_gauntlets, false))
            return;
        msg = TXT_WPNGAUNTLETS;
        msglevel = 21;
        sound = sfx_wpnup;
        break;

      default:
        // SoM: New gettable things with FraggleScript!
        //debug_Printf ("\2P_TouchSpecialThing: Unknown gettable thing\n");
        return;
    }

    if( msg )
    {
        P_SetMessage( player, msg, msglevel );
    }
    if( group == SPR_BKEY )  // all keys
    {
        // keycard
        if( EN_heretic )
            sound = sfx_keyup;
        if (multiplayer)  return;  // leave keys in multiplayer
    }
   
    if (special->flags & MF_COUNTITEM)
        player->itemcount++;
    P_RemoveMobj ( special );
    player->bonuscount += BONUSADD;

    //added:16-01-98:consoleplayer -> displayplayer (hear sounds from viewpoint)
    if (player == displayplayer_ptr
        || (cv_splitscreen.EV && player == displayplayer2_ptr))  // NULL when unused
        S_StartSound(sound);
}



#ifdef thatsbuggycode
//
//  Tell each supported thing to check again its position,
//  because the 'base' thing has vanished or diminished,
//  the supported things might fall.
//
//added:28-02-98:
void P_CheckSupportThings (mobj_t* mobj)
{
    fixed_t   supportz = mobj->z + mobj->height;

    while ((mobj = mobj->supportthings))
    {
        // only for things above support thing
        if (mobj->z > supportz)
            mobj->eflags |= MF_CHECKPOS;
    }
}


//
//  If a thing moves and supportthings,
//  move the supported things along.
//
//added:28-02-98:
void P_MoveSupportThings (mobj_t* mobj, fixed_t xmove, fixed_t ymove, fixed_t zmove)
{
    fixed_t   supportz = mobj->z + mobj->height;
    mobj_t    *mo = mobj->supportthings;

    while (mo)
    {
        //added:28-02-98:debug
        if (mo==mobj)
        {
            mobj->supportthings = NULL;
            break;
        }

        // only for things above support thing
        if (mobj->z > supportz)
        {
            mobj->eflags |= MF_CHECKPOS;
            mobj->momx += xmove;
            mobj->momy += ymove;
            mobj->momz += zmove;
        }

        mo = mo->supportthings;
    }
}


//
//  Link a thing to it's 'base' (supporting) thing.
//  When the supporting thing will move or change size,
//  the supported will then be aware.
//
//added:28-02-98:
void P_LinkFloorThing(mobj_t*   mobj)
{
    mobj_t*     mo;
    mobj_t*     nmo;

    // no supporting thing
    if (!(mo = mobj->floorthing))
        return;

    // link mobj 'above' the lower mobjs, so that lower supporting
    // mobjs act upon this mobj
    while ( (nmo = mo->supportthings) &&
            (nmo->z<=mobj->z) )
    {
        // dont link multiple times
        if (nmo==mobj)
            return;

        mo = nmo;
    }
    mo->supportthings = mobj;
    mobj->supportthings = nmo;
}


//
//  Unlink a thing from it's support,
//  when it's 'floorthing' has changed,
//  before linking with the new 'floorthing'.
//
//added:28-02-98:
void P_UnlinkFloorThing(mobj_t*   mobj)
{
    mobj_t*     mo;

    if (!(mo = mobj->floorthing))      // just to be sure (may happen)
       return;

    while (mo->supportthings)
    {
        if (mo->supportthings == mobj)
        {
            mo->supportthings = NULL;
            break;
        }
        mo = mo->supportthings;
    }
}
#endif


#define BUFFSIZE  512
// Death messages relating to the target (dying) player
//
static
void P_DeathMessages ( mobj_t*       target,
                       mobj_t*       inflictor,
                       mobj_t*       source )
{
    char txt[BUFFSIZE+1];
    int     w;
    char    *str;

    if (!target || !target->player)
        return;

    if (target->player->mo != target )  // voodoo doll died
        return;
   
    if (source && source->player)
    {
        if (source->player==target->player)
        {
            str = text[DEATHMSG_SUICIDE];
            snprintf(txt, BUFFSIZE, str, player_names[target->player - players]);
            txt[BUFFSIZE-1] = 0;
            GenPrintf(EMSG_playmsg, txt);
            if( cv_splitscreen.EV )
                GenPrintf(EMSG_playmsg2, txt);
        }
        else
        {
            if (target->health < -9000) // telefrag !
                str = text[DEATHMSG_TELEFRAG];
            else
            {
                w = source->player->readyweapon;
                if( inflictor )
                {
                    switch(inflictor->type) {
                    case MT_ROCKET   : w = wp_missile; break;
                    case MT_PLASMA   : w = wp_plasma;  break;
                    case MT_EXTRABFG :
                    case MT_BFG      : w = wp_bfg;     break;
                    default : break;
                    }
                }

                switch(w)
                {
                case wp_fist:
                    str = text[DEATHMSG_FIST];
                    break;
                case wp_pistol:
                    str = text[DEATHMSG_GUN];
                    break;
                case wp_shotgun:
                    str = text[DEATHMSG_SHOTGUN];
                    break;
                case wp_chaingun:
                    str = text[DEATHMSG_MACHGUN];
                    break;
                case wp_missile:
                    str = text[DEATHMSG_ROCKET];
                    if (target->health < -target->info->spawnhealth &&
                        target->info->xdeathstate)
                        str = text[DEATHMSG_GIBROCKET];
                    break;
                case wp_plasma:
                    str = text[DEATHMSG_PLASMA];
                    break;
                case wp_bfg:
                    str = text[DEATHMSG_BFGBALL];
                    break;
                case wp_chainsaw:
                    str = text[DEATHMSG_CHAINSAW];
                    break;
                case wp_supershotgun:
                    str = text[DEATHMSG_SUPSHOTGUN];
                    break;
                default:
                    str = text[DEATHMSG_PLAYUNKNOW];
                    break;
                }
            }

            snprintf(txt, BUFFSIZE, str,
                     player_names[target->player - players],
                     player_names[source->player - players]);
            txt[BUFFSIZE-1] = 0;
            GenPrintf(EMSG_playmsg, txt);
            if( cv_splitscreen.EV )
                GenPrintf(EMSG_playmsg2, txt);
        }
    }
    else
    {
        if (!source)
        {
            // environment kills
            w = target->player->specialsector;      //see p_spec.c

            if (w==5)
                str = text[DEATHMSG_HELLSLIME];
            else if (w==7)
                str = text[DEATHMSG_NUKE];
            else if (w==16 || w==4)
                str = text[DEATHMSG_SUPHELLSLIME];
            else
                str = text[DEATHMSG_SPECUNKNOW];
        }
        else
        {
            // check for lava,slime,water,crush,fall,monsters..
            if (source->type == MT_BARREL)
            {
                if (source->target->player)
                {
                    GenPrintf(EMSG_playmsg, text[DEATHMSG_BARRELFRAG],
                                player_names[target->player - players],
                                player_names[source->target->player - players]);
                    return;
                }
                else
                    str = text[DEATHMSG_BARREL];
            }
            else
            switch (source->type)
            {
              case MT_POSSESSED: str = text[DEATHMSG_POSSESSED]; break;
              case MT_SHOTGUY:   str = text[DEATHMSG_SHOTGUY];   break;
              case MT_VILE:      str = text[DEATHMSG_VILE];      break;
              case MT_FATSO:     str = text[DEATHMSG_FATSO];     break;
              case MT_CHAINGUY:  str = text[DEATHMSG_CHAINGUY];  break;
              case MT_TROOP:     str = text[DEATHMSG_TROOP];     break;
              case MT_SERGEANT:  str = text[DEATHMSG_SERGEANT];  break;
              case MT_SHADOWS:   str = text[DEATHMSG_SHADOWS];   break;
              case MT_HEAD:      str = text[DEATHMSG_HEAD];      break;
              case MT_BRUISER:   str = text[DEATHMSG_BRUISER];   break;
              case MT_UNDEAD:    str = text[DEATHMSG_UNDEAD];    break;
              case MT_KNIGHT:    str = text[DEATHMSG_KNIGHT];    break;
              case MT_SKULL:     str = text[DEATHMSG_SKULL];     break;
              case MT_SPIDER:    str = text[DEATHMSG_SPIDER];    break;
              case MT_BABY:      str = text[DEATHMSG_BABY];      break;
              case MT_CYBORG:    str = text[DEATHMSG_CYBORG];    break;
              case MT_PAIN:      str = text[DEATHMSG_PAIN];      break;
              case MT_WOLFSS:    str = text[DEATHMSG_WOLFSS];    break;
              default:           str = text[DEATHMSG_DEAD];      break;
            }
        }
        GenPrintf(EMSG_playmsg, str, player_names[target->player - players]);
    }
}

// WARNING : check cv_fraglimit>0 before call this function !
void P_CheckFragLimit(player_t *p)
{
    int fragteam = 0;
    if( cv_teamplay.EV )
    {
        int i;
        for(i=0;i<MAXPLAYERS;i++)
        {
            if(ST_SameTeam(p,&players[i]))
                fragteam += ST_PlayerFrags(i);
        }
    }
    else
    {
        fragteam = ST_PlayerFrags(p - players);
    }
    // CV_VALUE, may be too large for EV
    if( fragteam >= cv_fraglimit.value )
        G_ExitLevel();
}


/************************************************************
 *
 *  Returns ammo count in current weapon
 *
 ************************************************************
 */
static
int P_AmmoInWeapon(player_t *player)
{
    ammotype_t  ammo = player->weaponinfo[player->readyweapon].ammo;
    int         ammo_count = player->ammo[ammo];
    
    return ammo == am_noammo ? 0
        : ammo_count ? ammo_count : -1;
}


// P_KillMobj
//
//      source is the attacker, (for revenge, frags)
//      target is the 'target' of the attack, target dies...
//      inflictor is the weapon, missile, creature melee, or NULL, (for messages)
//                                          113
void P_KillMobj ( mobj_t*  target,
                  mobj_t*  inflictor,
                  mobj_t*  source )
{
    mobjtype_t  item = 0;
    mobj_t*     mo;
    int         drop_ammo_count = 0;

    // dead target is no more shootable
    if( ! cv_solidcorpse.EV )
        target->flags &= ~MF_SHOOTABLE;

    target->flags &= ~(MF_FLOAT|MF_SKULLFLY);

    if (target->type != MT_SKULL)
        target->flags &= ~MF_NOGRAVITY;

#ifdef DOGS
    // [WDJ] MBF dogs, extension for DoomLegacy.
    if( (target->type == MT_DOG) || (target->type == helper_MT) )
        G_KillDog( target );     
#endif

    // scream a corpse :)
    if( target->flags & MF_CORPSE )
    {
        // Turn it to gibs.
        P_SetMobjState (target, S_GIBS);

        target->flags &= ~MF_SOLID;
        target->height = 0;
        target->radius<<= 1;
        target->skin = 0;

        //added:22-02-98: lets have a neat 'crunch' sound!
        S_StartObjSound(target, sfx_slop);
        return;
    }

    //added:22-02-98: remember who exploded the barrel, so that the guy who
    //                shot the barrel which killed another guy, gets the frag!
    //                (source is passed from barrel to barrel also!)
    //                (only for multiplayer fun, does not remember monsters)
    if ((target->type == MT_BARREL || target->type == MT_POD)
	&& source && source->player)
    {
        P_SetReference(target->target, source);
        target->target = source;
    }

    if( EV_legacy < 131 )  // old Legacy, Boom, MBF
    {
        // Version 131 and after this is done later in A_Fall.
        // (this fix the stepping monster)
        target->flags   |= MF_CORPSE|MF_DROPOFF;
        target->height >>= 2;
        if( EV_legacy >= 112 )
            target->radius -= (target->radius>>4);      //for solid corpses
    }
    // show death messages, only if it concern the console player
    // (be it an attacker or a target)
    if (target->player && (target->player == consoleplayer_ptr) )
        P_DeathMessages (target, inflictor, source);
    else
    if (source && source->player && (source->player == consoleplayer_ptr) )
        P_DeathMessages (target, inflictor, source);
    else
    if (target->player && target->player->bot)	//added by AC for acbot
       P_DeathMessages (target, inflictor, source);



    // if killed by a player
    if (source && source->player)
    {
        // count for intermission
        if (target->flags & MF_COUNTKILL)
            source->player->killcount++;

        // count frags if player killed player
        if (target->player)
        {
            source->player->frags[target->player-players]++;
            if( EN_heretic )
            {
                if(source->player == displayplayer_ptr
                || source->player == displayplayer2_ptr )
                    S_StartSound(sfx_gfrag);

                // Make a super chicken
                if(source->player->chickenTics)
                    P_GivePower(source->player, pw_weaponlevel2);
            }
            // check fraglimit cvar
            if (cv_fraglimit.value)
                P_CheckFragLimit(source->player);
        }
    }
    else if (!multiplayer && (target->flags & MF_COUNTKILL))
    {
        // count all monster deaths,
        // even those caused by other monsters
        players[0].killcount++;
    }

    // if a player avatar dies...
    if (target->player)
    {
        // count environment kills against you (you fragged yourself!)
        if (!source)
            target->player->frags[target->player-players]++;

        if( ! cv_solidcorpse.EV )
            target->flags &= ~MF_SOLID;                     // does not block
        target->flags2 &= ~MF2_FLY;
        target->player->powers[pw_flight] = 0;
        target->player->powers[pw_weaponlevel2] = 0;
        target->player->playerstate = PST_DEAD;
        P_DropWeapon (target->player);                  // put weapon away
        if (target->player == consoleplayer_ptr )
        {
            // don't die in auto map,
            // switch view prior to dying
            if (automapactive)
                AM_Stop ();

            //added:22-02-98: recenter view for next live...
            localaiming = 0;
        }
        if (target->player == displayplayer2_ptr) // NULL when unused
        {
            // player 2
            //added:22-02-98: recenter view for next live...
            localaiming2 = 0;
        }
/* HERETODO
        if(target->flags2&MF2_FIREDAMAGE)
        { // Player flame death
            P_SetMobjState(target, S_PLAY_FDTH1);
            //S_StartObjSound(target, sfx_hedat1); // Burn sound
            goto done;
        }
*/
    }

    if ( target->info->xdeathstate
         && ( target->health < -(
               (EN_heretic)? (target->info->spawnhealth>>1)  // heretic
               : target->info->spawnhealth  // doom
            ) )
       )
    {
        P_SetMobjState (target, target->info->xdeathstate);
    }
    else
        P_SetMobjState (target, target->info->deathstate);

    target->tics -= PP_Random(pr_killtics)&3;

    if (target->tics < 1)
        target->tics = 1;

    // Drop stuff.
    // This determines the kind of object spawned
    // during the death frame of a thing.

    // Frags Weapon Falling support
    if( target->player && cv_fragsweaponfalling.EV )
    {
        drop_ammo_count = P_AmmoInWeapon(target->player);
        //if (!drop_ammo_count)
        //    goto done;
        
        if (EN_heretic)
        {
            switch (target->player->readyweapon)
            {
                case wp_crossbow:
                    item = MT_HMISC15;
                    break;

                case wp_blaster:
                    item = MT_RIPPER;
                    break;

                case wp_skullrod:
                    item = MT_WSKULLROD;
                    break;

                case wp_phoenixrod:
                    item = MT_WPHOENIXROD;
                    break;

                case wp_mace:
                    item = MT_WMACE;
                    break;

                default:
                    //debug_Printf("Unknown weapon %d\n", target->player->readyweapon);
                    goto done;
            }
        }
        else
        {
            switch (target->player->readyweapon)
            {
                case wp_shotgun:
                    item = MT_SHOTGUN;
                    break;

                case wp_supershotgun:
                    item = MT_SUPERSHOTGUN;
                    break;

                case wp_chaingun:
                    item = MT_CHAINGUN;
                    break;

                case wp_missile:
                    item = MT_ROCKETLAUNCH;
                    break;

                case wp_plasma:
                    item = MT_PLASMAGUN;
                    break;

                case wp_bfg:
                    item = MT_BFG9000;
                    break;

                default:
                    //debug_Printf("Unknown weapon %d\n", target->player->readyweapon);
                    goto done;
            }
        }
    }
    else
    {
        //DarkWolf95: Support for Chex Quest
        if(gamemode == chexquest1)  //don't drop monster ammo in chex quest
           goto done;

        switch (target->type)
        {
            case MT_WOLFSS:
            case MT_POSSESSED:
                item = MT_CLIP;
                break;

            case MT_SHOTGUY:
                item = MT_SHOTGUN;
                break;

            case MT_CHAINGUY:
                item = MT_CHAINGUN;
                break;

            default:
                goto done;
        }
    }

    // SoM: Damnit! Why not use the target's floorz?
    // Doom, Boom, MBF use ONFLOORZ.
    mo = P_SpawnMobj (target->x, target->y,
                      ((EV_legacy < 132) ? ONFLOORZ : target->floorz), item);
    mo->flags |= MF_DROPPED;    // special versions of items

    if( !cv_fragsweaponfalling.EV )
        drop_ammo_count = 0;    // Doom default ammo count

    mo->dropped_ammo_count = drop_ammo_count;

done:
    return;
}


//---------------------------------------------------------------------------
//
// FUNC P_MinotaurSlam
//
//---------------------------------------------------------------------------

static
void P_MinotaurSlam(mobj_t *source, mobj_t *target)
{
    angle_t angle;
    fixed_t thrust;
    
    angle = R_PointToAngle2(source->x, source->y, target->x, target->y);
    thrust = 16*FRACUNIT+(P_Random()<<10);
    target->momx += FixedMul(thrust, cosine_ANG(angle));
    target->momy += FixedMul(thrust, sine_ANG(angle));
    P_DamageMobj(target, NULL, NULL, HITDICE(6));
    if(target->player)
    {
        target->reactiontime = 14+(P_Random()&7);
    }
}

//---------------------------------------------------------------------------
//
// FUNC P_TouchWhirlwind
//
//---------------------------------------------------------------------------

static
boolean P_TouchWhirlwind(mobj_t *target)
{
    int randVal;
    
    target->angle += P_SignedRandom()<<20;
    target->momx += P_SignedRandom()<<10;
    target->momy += P_SignedRandom()<<10;
    if(leveltime&16 && !(target->flags2&MF2_BOSS))
    {
        randVal = P_Random();
        if(randVal > 160)
        {
            randVal = 160;
        }
        target->momz += randVal<<10;
        if(target->momz > 12*FRACUNIT)
        {
            target->momz = 12*FRACUNIT;
        }
    }
    if(!(leveltime&7))
    {
        return P_DamageMobj(target, NULL, NULL, 3);
    }
    return false;
}


//---------------------------------------------------------------------------
//
// FUNC P_ChickenMorphPlayer
//
// Returns true if the player gets turned into a chicken.
//
//---------------------------------------------------------------------------

// [WDJ] Fixed to keep the same player mobj.
// Used to change the player mobj, and hide the prev as a corpse above
// the ceiling using S_FREETARGMOBJ.  This could happen in Line attack or Damage.
boolean P_ChickenMorphPlayer(player_t *player)
{
    mobj_t *pmo;
    int oldflags2;
    
    if(player->chickenTics)
    {
        if((player->chickenTics < CHICKENTICS-TICRATE)
            && !player->powers[pw_weaponlevel2])
        { // Make a super chicken
            P_GivePower(player, pw_weaponlevel2);
        }
        return false;
    }
    if(player->powers[pw_invulnerability])
    { // Immune when invulnerable
        return false;
    }
    pmo = player->mo;
    oldflags2 = pmo->flags2;
    P_MorphMobj(pmo, MT_CHICPLAYER, MM_telefog,
#ifdef PLAYER_CHICKEN_KEEPS_SHADOW
                      MF_SHADOW
#else
                      0
#endif
                );
    pmo->special1 = player->readyweapon;  // save for later restore
    pmo->flags2 |= oldflags2&MF2_FLY;  // preserve fly
    // Clear skin so it does not override chicken.
    pmo->skin = NULL;  // Chickens all look alike.
    pmo->tflags &= ~MFT_TRANSLATION6; // no color translation for chicken
    // spawnhealth for chicken is 100, this is 30
    player->health = pmo->health = MAXCHICKENHEALTH;
    player->armorpoints = player->armortype = 0;
#ifndef PLAYER_CHICKEN_KEEPS_SHADOW
    // If keep MF_SHADOW and cancel invisibility, then MF_SHADOW is permananet.
    player->powers[pw_invisibility] = 0;
#endif
    player->powers[pw_weaponlevel2] = 0;
    player->weaponinfo = wpnlev1info;
    player->chickenTics = CHICKENTICS;  // start chicken timer
    P_ActivateBeak(player);
    return true;
}

//---------------------------------------------------------------------------
//
// FUNC P_ChickenMorph
//
//---------------------------------------------------------------------------

// Other actors, not players.
boolean P_ChickenMorph(mobj_t *actor)
{
    mobjtype_t moType;
    
    if(actor->player)
    {
        return false;
    }
    moType = actor->type;
    switch(moType)
    {
        case MT_POD:
        case MT_CHICKEN:
        case MT_HHEAD:
        case MT_MINOTAUR:
        case MT_SORCERER1:
        case MT_SORCERER2:
            return false;
        default:
            break;
    }
    
    // preserve position, angle, target, invisible
    P_MorphMobj(actor, MT_CHICKEN, MM_telefog, MF_SHADOW);
    actor->special1 = CHICKENTICS+P_Random();  // monster chickentics
    actor->special2 = moType;  // save type for restore
    return true;
}

//---------------------------------------------------------------------------
//
// FUNC P_AutoUseChaosDevice
//
//---------------------------------------------------------------------------

boolean P_AutoUseChaosDevice(player_t *player)
{
    int i;
    
    for(i = 0; i < player->inventorySlotNum; i++)
    {
        if(player->inventory[i].type == arti_teleport)
        {
            P_PlayerUseArtifact(player, arti_teleport);
            player->health = player->mo->health = (player->health+1)/2;
            return(true);
        }
    }
    return(false);
}

//---------------------------------------------------------------------------
//
// PROC P_AutoUseHealth
//
//---------------------------------------------------------------------------

// From Heretic
void P_AutoUseHealth(player_t *player, int saveHealth)
{
    int i;
    int count;
    int normalCount;
    int superCount;

    // Uses P_PlayerUseArtifact, so do not need to know inventory slot.
    normalCount = superCount = 0;
    for(i = 0; i < player->inventorySlotNum; i++)
    {
        if(player->inventory[i].type == arti_health)
        {
            normalCount = player->inventory[i].count;
        }
        else if(player->inventory[i].type == arti_superhealth)
        {
            superCount = player->inventory[i].count;
        }
    }
    if((gameskill == sk_baby) && (normalCount*25 >= saveHealth))
    { // Use quartz flasks
        count = (saveHealth+24)/25;
        for(i = 0; i < count; i++)
            P_PlayerUseArtifact( player, arti_health);
    }
    else if(superCount*100 >= saveHealth)
    { // Use mystic urns
        count = (saveHealth+99)/100;
        for(i = 0; i < count; i++)
            P_PlayerUseArtifact( player, arti_superhealth);
    }
    else if((gameskill == sk_baby)
        && (superCount*100+normalCount*25 >= saveHealth))
    { // Use mystic urns and quartz flasks
        count = (saveHealth+24)/25;
        for(i = 0; i < count; i++)
            P_PlayerUseArtifact( player, arti_health);

        saveHealth -= count*25;
        count = (saveHealth+99)/100;
        for(i = 0; i < count; i++)
            P_PlayerUseArtifact( player, arti_superhealth);
    }
    player->mo->health = player->health;
}


//
// P_DamageMobj
// Damages both enemies and players
// "inflictor" is the thing that caused the damage
//  creature or missile, can be NULL (slime, etc)
// "source" is the thing to target after taking damage
//  creature or NULL
// Source and inflictor are the same for melee attacks.
// Source can be NULL for slime, barrel explosions
// and other environmental stuff.
//
// Return true when damaged, for blood splats and other effects.
// Fixed to not change the player mobj.
boolean P_DamageMobj ( mobj_t*   target,
                       mobj_t*   inflictor,
                       mobj_t*   source,
                       int       damage )
{
    angle_t     ang;
    int         angf;
    int         saved;
    player_t*   player;  // always target->player
    fixed_t     thrust;
    boolean     voodoo_target = false;
    boolean     mbf_justhit = false;  // MBF, delayed MF_JUSTHIT
    boolean     takedamage = true;  // block damage between members of same team

    player = target->player;

    // [WDJ] 7/2017 Moved the voodoo intercept of damage to be tested earlier
    // because of the weapons and armor specific player checks that
    // can get applied to the wrong player otherwise.
    // This code can change the target of the damage.
    // If we ever implement the player as a monster, this code needs to be first.
    if( player )
    {
        // [WDJ] 2/7/2011 Intercept voodoo damage
        voodoo_target = (player->mo != target);
        if( voodoo_target )
        {
            mobj_t * voodoo_thing = target;
            if(voodoo_mode >= VM_target)
            {
                // Multiplayer and single player:
                // try to find someone appropriate, instead of spawn point player.
                // Target source player causing damage
                if( source && source->player
                    && (source->player->mo == source) )
                {
                    // Shooting any voodoo doll, select shooting player
                    player = source->player;
                }
                // Target last player to trigger a switch or linedef.
                else if( spechit_player && spechit_player->mo )
                {
                    player = spechit_player;
                }
            }

            if(! player->mo )  // this player is not present
            {
                if( voodoo_mode < VM_target )
                {
                    // remove this voodoo doll to avoid segfaults
                    P_RemoveMobj( voodoo_thing );
                }
                goto ret_false;
            }

            if( voodoo_mode == VM_vanilla )
            {
                target->health -= damage;  // damage the voodoo too
            }
            else
            {
                if( multiplayer && (damage > player->health))
                {
                    // Kill the voodoo, so it cannot kill after respawn.
                    // Voodoo doll in crusher is game fatal otherwise.
                    voodoo_thing->health = 0;
                    voodoo_thing->player = NULL;
                    P_KillMobj ( voodoo_thing, inflictor, source );
                    spechit_player = NULL;  // cancel voodoo damage
                }
                // let player mobj get the damage, no Zombies
                target = player->mo;
            }
        }

        if( gameskill == sk_baby )
            damage >>= 1;   // take half damage in trainer mode
    }
   
 
    // killough 8/31/98: allow bouncers to take damage
    if ( !(target->flags & (MF_SHOOTABLE | MF_BOUNCES)) )
        goto ret_false; // shouldn't happen...

    // [WDJ] Solid Corpse health < 0.
    if( (target->health <= 0) && !(target->flags & MF_CORPSE) )
        goto ret_false;

    if ( target->flags & MF_SKULLFLY )
    {
        // Minotaur is invulnerable during charge attack
        if(target->type == MT_MINOTAUR)
            goto ret_false;

        target->momx = target->momy = target->momz = 0;
    }

    // Special damage types
    if(inflictor)
    {
        switch(inflictor->type)
        {
        case MT_EGGFX:
            if(player)
            {
                // Fixed to not change the player mobj.
                P_ChickenMorphPlayer(player);
            }
            else
            {
                P_ChickenMorph(target);
            }
            goto ret_false; // Always return
        case MT_WHIRLWIND:
            takedamage = P_TouchWhirlwind(target);
            goto ret_damage;
        case MT_MINOTAUR:
            if(inflictor->flags&MF_SKULLFLY)
            { // Slam only when in charge mode
                P_MinotaurSlam(inflictor, target);
                goto ret_true;
            }
            break;
        case MT_MACEFX4: // Death ball
            if((target->flags2&MF2_BOSS) || target->type == MT_HHEAD)
            { // Don't allow cheap boss kills
                break;
            }
            else if(player)
            { // Player specific checks
                if(player->powers[pw_invulnerability])
                { // Can't hurt invulnerable players
                    break;
                }
                if(P_AutoUseChaosDevice(player))
                { // Player was saved using chaos device
                    goto ret_false;
                }
            }
            damage = 10000; // Something's gonna die
            break;
        case MT_PHOENIXFX2: // Flame thrower
            if(player && P_Random() < 128)
            { // Freeze player for a bit
                target->reactiontime += 4;
            }
            break;
        case MT_RAINPLR1: // Rain missiles
        case MT_RAINPLR2:
        case MT_RAINPLR3:
        case MT_RAINPLR4:
            if(target->flags2&MF2_BOSS)
            { // Decrease damage for bosses
                damage = (P_Random()&7)+1;
            }
            break;
        case MT_HORNRODFX2:
        case MT_PHOENIXFX1:
            if(target->type == MT_SORCERER2 && P_Random() < 96)
            { // D'Sparil teleports away
                P_DSparilTeleport(target);
                goto ret_false;
            }
            break;
        case MT_BLASTERFX1:
        case MT_RIPPER:
            if(target->type == MT_HHEAD)
            { // Less damage to Ironlich bosses
                damage = P_Random()&1;
                if(!damage)
                    goto ret_false;
            }
            break;
        default:
            break;
        }
    }

    // Some close combat weapons should not
    // inflict thrust and push the victim out of reach,
    // thus kick away unless using the chainsaw.
    if (inflictor
        && !(target->flags & MF_NOCLIP)  // unless target is NOCLIP
        && !(inflictor->flags2&MF2_NODMGTHRUST)  // unless inflictor cannot thrust
        && (!source  // not chainsaw
            || !source->player
            || source->player->readyweapon != wp_chainsaw))
    {
        // Impose thrust upon the target from the weapon
        fixed_t  amomx, amomy, amomz=0;//SoM: 3/28/2000

        ang = R_PointToAngle2 ( inflictor->x, inflictor->y,
                                target->x, target->y);

        if (EN_heretic )
            thrust = damage*(FRACUNIT>>3)*150/target->info->mass;
        else
            thrust = damage*(FRACUNIT>>3)*100/target->info->mass;

        // sometimes a target shot down might fall off a ledge forwards
        if ( damage < 40
             && damage > target->health
             && target->z - inflictor->z > 64*FRACUNIT
             && (PP_Random(pr_damagemobj) & 0x01)
           )
        {
            ang += ANG180;
            thrust *= 4;
        }

        angf = ANGLE_TO_FINE(ang);

        if( EN_heretic
            && source && (source == inflictor)
            && source->player
            && source->player->powers[pw_weaponlevel2]
            && source->player->readyweapon == wp_staff)
        {
            // Staff power level 2
            target->momx += FixedMul(10*FRACUNIT, finecosine[angf]);
            target->momy += FixedMul(10*FRACUNIT, finesine[angf]);
            if(!(target->flags&MF_NOGRAVITY))
            {
                target->momz += 5*FRACUNIT;
            }
        }
        else
        {
            // all other thrusting weapons
            amomx = FixedMul (thrust, finecosine[angf]);
            amomy = FixedMul (thrust, finesine[angf]);
            target->momx += amomx;
            target->momy += amomy;
            
            // added momz (do it better for missiles explosion)
            if ( source
                 && (EV_legacy >= 124)
                 && ((EV_legacy < 129) || !cv_allowrocketjump.EV))
            {
                // Legacy
                int dist,z;
                
                if(source==target) // rocket in yourself (suicide)
                {
                    viewx=inflictor->x;
                    viewy=inflictor->y;
                    z=inflictor->z;
                }
                else
                {
                    viewx=source->x;
                    viewy=source->y;
                    z=source->z;
                }
                dist=R_PointToDist(target->x,target->y);
                
                viewx=0;
                viewy=z;
                ang = R_PointToAngle(dist,target->z);
                amomz = FixedMul (thrust, sine_ANG(ang));
            }
            else //SoM: 2/28/2000: Added new function.
            if( (EV_legacy >= 129) && cv_allowrocketjump.EV )
            {
                // Legacy rocket jump.
                fixed_t delta1 = abs(inflictor->z - target->z);
                fixed_t delta2 = abs(inflictor->z - (target->z + target->height));
                amomz = (abs(amomx) + abs(amomy))>>1;
                
                if(delta1 >= delta2 && inflictor->momz < 0)
                    amomz = -amomz;
            }

            target->momz += amomz;
#ifdef CLIENTPREDICTION2
            if( player && player->spirit )
            {
                player->spirit->momx += amomx;
                player->spirit->momy += amomy;
                player->spirit->momz += amomz;
            }
#endif
            // [WDJ] MBF
            // killough 11/98: thrust objects hanging off ledges
            if( target->eflags & MF_FALLING && (target->tipcount >= MAXTIPCOUNT) )
                target->tipcount = 0;
        }
    }
   
    // Solid Corpse specific
    if( target->flags & MF_CORPSE )
    {
        target->health -= damage;
        // [WDJ] Corpse health < 0, so solid corpse test is < 0.
        if( target->health < -target->info->spawnhealth )
            P_KillMobj ( target, inflictor, source );  // to gibs
        // Keep corpse from ticking the P_Random in the pain test.
        goto ret_true;    
    }

    // target player specific
    if (player)
    {
        // end of game hell hack
        if (target->subsector->sector->special == 11
            && damage >= target->health)
        {
            damage = target->health - 1;
        }

        // Below certain threshold,
        // ignore damage in GOD mode, or with INVUL power.
        if( (player->cheats&CF_GODMODE) || player->powers[pw_invulnerability] )
        {
            // Boom, MBF: killough 3/26/98: make god mode 100%
            // !comp[comp_god]
            if( (player->cheats&CF_GODMODE) && EN_invul_god )
                goto ret_false;

            if( damage < 1000 )
                goto ret_false;
        }

        if (player->armortype)
        {
            if (player->armortype == 1)
                saved = (EN_heretic)? damage>>1 : damage/3;
            else
                saved = (EN_heretic)? (damage>>1)+(damage>>2) : damage/2;

            if (player->armorpoints <= saved)
            {
                // armor is used up
                saved = player->armorpoints;
                player->armortype = 0;
            }
            player->armorpoints -= saved;
            damage -= saved;
        }

        // added team play and teamdamage (view logboris at 13-8-98 to understand)
        // [WDJ] 2/7/2011  Allow damage to player when:
        // olddemo (version < 125) // because they did not have these restrictions
        // OR telefrag        // not subject to friendly fire tests
        // OR no source       // no source interaction, sector damage etc.
        // OR NOT sourceplayer  // monster attack
        // OR (source==target)  // self inflicted damage (missile launcher)
        // OR voodoo_target   // voodoo damage allowed by previous tests
        // OR NOT multiplayer  // single player
        // OR ( coop         // all on same team
        //     AND cv_teamdamage  // team members can hurt each other
        //    )
        // OR ( deathmatch   // teams or individual, not coop
        //     AND ( NOT teamplay
        //               // otherwise teamplay
        //           OR cv_teamdamage   // team members can hurt each other
        //           OR (target.team != source.team)
        //         )
        //    )
        // [WDJ] For readability and understanding, please do not try to reduce
        // these equations, they are not executed very often, and the
        // compiler will reduce them during optimization anyway.
        if( (! source)		   // no source interaction, sector damage etc.
            || (! source->player)  // monster attack
            || voodoo_target	   // allowed voodoo damage
            || (damage>1000)       // telefrag and death-ball
            || (EV_legacy < 125)   // old demoversion bypasses restrictions
            || (source==target)    // self-inflicted
            || (! multiplayer)     // single player
            || ( (cv_deathmatch.EV == 0) && cv_teamdamage.EV )  // coop
            || ( (cv_deathmatch.EV > 0)      // deathmatch 1,2,3
                 && ( (!cv_teamplay.EV)    // no teams
                      || cv_teamdamage.EV  // can damage within team
                      || ! ST_SameTeam(source->player,player) // diff team
                    )
                 )
            )
        {
            if(damage >= player->health
                && ((gameskill == sk_baby) || cv_deathmatch.EV)
                && !player->chickenTics)
            { // Try to use some inventory health
                P_AutoUseHealth(player, damage - player->health + 1);
            }

            // Update player health here, because they may die before
            // reaching the later player update.
            player->health -= damage;   // mirror mobj health here for Dave
            if (player->health < 0)
                player->health = 0;
            // [WDJ] If player->mo is updated here, it prevents player gibs.
            // target = player->mo, as set in voodoo logic.

            player->damagecount += damage;  // add damage after armor / invuln

            if (player->damagecount > 100)
                player->damagecount = 100;  // teleport stomp does 10k points...

            //added:22-02-98: force feedback ??? electro-shock???
            if (player == consoleplayer_ptr )
                I_Tactile (40,10,40+min(damage, 100)*2);
        }
        else
        {
            takedamage = false;  // block damage
        }
        player->attacker = source;
    }

    if( takedamage )
    {
        target->health -= damage;

        // check for kill
        if (target->health <= 0)
        {
            target->special1 = damage;
            if(player && inflictor && !player->chickenTics)
            { // Check for flame death
                if((inflictor->flags2&MF2_FIREDAMAGE)
                    || ((inflictor->type == MT_PHOENIXFX1)
                    && (target->health > -50) && (damage > 25)))
                {
                    target->flags2 |= MF2_FIREDAMAGE;
                }
            }

            P_KillMobj ( target, inflictor, source );
            goto ret_true;
        }
  
        // This must be after KillMobj, so target damage can be negative.
        if( player )
        {
            if( player->mo )
                player->mo->health = player->health; // keep mobj and player health same
        }

        // [WDJ] MBF, From MBF, PrBoom, EternityEngine.
        // killough 9/7/98: keep track of targets so that friends can help friends
        if( EN_mbf )
        {
            // If target is a player, set player's target to source,
            // so that a friend can tell who is hurting a player
            if(player)
            {
                P_SetReference(target->target, source);
                target->target = source;
            }

            // killough 9/8/98:
            // If target's health is less than 50%, move it to the front of its list.
            // This will slightly increase the chances that enemies will choose to
            // "finish it off", but its main purpose is to alert friends of danger.
            if( target->health*2 < target->info->spawnhealth )
            {
                P_MoveClassThink( &target->thinker, 1 );  // move first
            }
        }

        if( (PP_Random(pr_painchance) < target->info->painchance)
            && !(target->flags&(MF_SKULLFLY|MF_CORPSE)) )
        {
            if( EN_mbf )
                mbf_justhit = true;  // defer setting MF_JUSTHIT to below
            else
                target->flags |= MF_JUSTHIT;    // fight back!

            P_SetMobjState (target, target->info->painstate);
        }

        target->reactiontime = 0;           // we're awake now...
    }

    if ( (!target->threshold || target->type == MT_VILE)
         && source && source != target  // fixes bug where monster attacks self
         && source->type != MT_VILE
         && !(source->flags2 & MF2_BOSS)
         && !( EN_mbf
              && monster_infight != INFT_infight
              && SAME_FRIEND(source, target)
             )
         && !(target->type == MT_SORCERER2 && source->type == MT_WIZARD)
       )
    {
        // killough 2/15/98: remember last enemy, to prevent sleeping early
        // 2/21/98: Place priority on players
        // killough 9/9/98: cleaned up, made more consistent:
        if( !target->lastenemy
            || target->lastenemy->health <= 0
            ||( EN_mbf ?
                  ( target->target != source
                    && SAME_FRIEND(target, target->lastenemy) )
                : ! target->lastenemy->player
              )
          )
        {
            // remember last enemy - killough
            P_SetReference(target->lastenemy, target->target);
            target->lastenemy = target->target;
        }

        // if not intent on another player,
        // chase after this one
        P_SetReference(target->target, source);       // killough 11/98
        target->target = source;

        target->threshold = BASETHRESHOLD;
        if( target->state == &states[target->info->spawnstate]
            && target->info->seestate != S_NULL)
            P_SetMobjState (target, target->info->seestate);
    }

    // killough 11/98: Don't attack a friend, unless hit by that friend.
    // cph 2006/04/01 - implicitly this is only if mbf_features
    if( mbf_justhit   // set by MBF to defer MF_JUSTHIT to here
        && ( !target->target
             || target->target == source
             || !BOTH_FRIEND(target, target->target) )
      )
        target->flags |= MF_JUSTHIT;    // fight back!

ret_damage:   
    return takedamage;

ret_false:
    return false;

ret_true:
    return true;  // damaged
}
