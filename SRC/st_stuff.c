// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: st_stuff.c 1229 2016-05-24 17:04:54Z wesleyjohnson $
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
// $Log: st_stuff.c,v $
// Revision 1.22  2003/08/11 13:50:00  hurdler
// go final + translucent HUD + fix spawn in net game
//
// Revision 1.21  2001/08/20 21:37:35  hurdler
// fix palette in splitscreen + hardware mode
//
// Revision 1.20  2001/08/20 20:40:39  metzgermeister
//
// Revision 1.19  2001/08/08 20:34:43  hurdler
// Big TANDL update
//
// Revision 1.18  2001/05/16 21:21:14  bpereira
// Revision 1.17  2001/04/01 17:35:07  bpereira
// Revision 1.16  2001/03/03 06:17:34  bpereira
// Revision 1.15  2001/02/24 13:35:21  bpereira
// Revision 1.14  2001/02/10 13:05:45  hurdler
//
// Revision 1.13  2001/01/31 17:14:07  hurdler
// Add cv_scalestatusbar in hardware mode
//
// Revision 1.12  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.11  2000/11/02 19:49:37  bpereira
//
// Revision 1.10  2000/10/04 16:34:51  hurdler
// Change a little the presentation of monsters/secrets numbers
//
// Revision 1.9  2000/10/02 18:25:45  bpereira
// Revision 1.8  2000/10/01 10:18:19  bpereira
//
// Revision 1.7  2000/10/01 01:12:00  hurdler
// Add number of monsters and secrets in overlay
//
// Revision 1.6  2000/09/28 20:57:18  bpereira
//
// Revision 1.5  2000/09/25 19:28:15  hurdler
// Enable Direct3D support as OpenGL
//
// Revision 1.4  2000/09/21 16:45:09  bpereira
// Revision 1.3  2000/08/31 14:30:56  bpereira
// Revision 1.2  2000/02/27 00:42:11  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Status bar code.
//      Does the face/direction indicator animatin.
//      Does palette indicators as well (red pain/berserk, bright pickup)
//
//-----------------------------------------------------------------------------

#include "doomincl.h"

#include "am_map.h"

#include "g_game.h"
#include "m_cheat.h"

#include "screen.h"
#include "r_local.h"
#include "p_local.h"
#include "p_inter.h"
#include "m_random.h"

#include "st_stuff.h"
#include "st_lib.h"
#include "i_video.h"
#include "v_video.h"

#include "keys.h"

#include "z_zone.h"
#include "hu_stuff.h"
#include "d_main.h"

#ifdef HWRENDER
#include "hardware/hw_drv.h"
#include "hardware/hw_main.h"
#endif

//protos
void ST_createWidgets(void);


#define FLASH_COLOR  0x72

//
// STATUS BAR DATA
//

// Palette indices.
// For damage/bonus red-/gold-shifts
#define STARTREDPALS            1
#define STARTBONUSPALS          9
#define NUMREDPALS              8
#define NUMBONUSPALS            4
// Radiation suit, green shift.
#define RADIATIONPAL            13

// N/256*100% probability
//  that the normal face state will change
#define ST_FACEPROBABILITY              96

// For Responder
#define ST_TOGGLECHAT           KEY_ENTER

// Location of status bar
  //added:08-01-98:status bar position changes according to resolution.
#define ST_FX                     143
// This is now dynamic
#define ST_Y                    stbar_y

// Number of status faces.
#define ST_NUMPAINFACES         5
#define ST_NUMSTRAIGHTFACES     3
#define ST_NUMTURNFACES         2
#define ST_NUMSPECIALFACES      3

#define ST_FACESTRIDE \
          (ST_NUMSTRAIGHTFACES+ST_NUMTURNFACES+ST_NUMSPECIALFACES)

#define ST_NUMEXTRAFACES        2

#define ST_NUMFACES \
          (ST_FACESTRIDE*ST_NUMPAINFACES+ST_NUMEXTRAFACES)

#define ST_TURNOFFSET           (ST_NUMSTRAIGHTFACES)
#define ST_OUCHOFFSET           (ST_TURNOFFSET + ST_NUMTURNFACES)
#define ST_EVILGRINOFFSET       (ST_OUCHOFFSET + 1)
#define ST_RAMPAGEOFFSET        (ST_EVILGRINOFFSET + 1)
#define ST_GODFACE              (ST_NUMPAINFACES*ST_FACESTRIDE)
#define ST_DEADFACE             (ST_GODFACE+1)

#define ST_FACESX               143
#define ST_FACESY               0

#define ST_EVILGRINCOUNT        (2*TICRATE)
#define ST_STRAIGHTFACECOUNT    (TICRATE/2)
#define ST_TURNCOUNT            (1*TICRATE)
#define ST_OUCHCOUNT            (1*TICRATE)
#define ST_RAMPAGEDELAY         (2*TICRATE)

#define ST_MUCHPAIN             20


// Location and size of statistics,
//  justified according to widget type.
// Problem is, within which space? STbar? Screen?
// Note: this could be read in by a lump.
//       Problem is, is the stuff rendered
//       into a buffer,
//       or into the frame buffer?

// AMMO number pos.
#define ST_AMMOWIDTH            3
#define ST_AMMOX                44
#define ST_AMMOY                3

// HEALTH number pos.
#define ST_HEALTHWIDTH          3
#define ST_HEALTHX              90
#define ST_HEALTHY              3

// Weapon pos.
#define ST_ARMSX                111
#define ST_ARMSY                4
#define ST_ARMSBGX              104
#define ST_ARMSBGY              0
#define ST_ARMSXSPACE           12
#define ST_ARMSYSPACE           10

// Frags pos.
#define ST_FRAGSX               138
#define ST_FRAGSY               3
#define ST_FRAGSWIDTH           2

// ARMOR number pos.
#define ST_ARMORWIDTH           3
#define ST_ARMORX               221
#define ST_ARMORY               3

// Key icon positions.
#define ST_KEYSBOX_X   236
#define ST_KEYSBOX_Y   2
#define ST_KEYSBOX_W   13
#define ST_KEYSBOX_H   30
// They appear in a vertical column, so share x positions.
#define ST_KEY_WIDTH   6
#define ST_KEY_HEIGHT  5
#define ST_KEYX        239
#define ST_KEYDX       2
#define ST_KEYDY       2
static uint8_t  keybox_y[6] = { 3, 13, 23, 3, 13, 23 };
static uint8_t  keybox_dual_x[6] =
 { ST_KEYX-ST_KEYDX, ST_KEYX-ST_KEYDX, ST_KEYX-ST_KEYDX,
   ST_KEYX+ST_KEYDX, ST_KEYX+ST_KEYDX, ST_KEYX+ST_KEYDX };
static uint8_t  keybox_dual_y[6] =
 {  3-ST_KEYDY, 13-ST_KEYDY, 23-ST_KEYDY,
    3+ST_KEYDY, 13+ST_KEYDY, 23+ST_KEYDY };

// Ammunition counter, and max ammo, in two columns in status bar.
// Max ammo changes when backpack aquired.
#define ST_AMMOS_WIDTH          3
#define ST_MAXAMMOS_WIDTH       3
#define ST_AMMOSX               288
#define ST_MAXAMMOSX            314
static uint8_t  ammobox_y[4] = { 5, 11, 23, 17 };


//faB: unused stuff from the Doom alpha version ?
// pistol
//#define ST_WEAPON0X           110
//#define ST_WEAPON0Y           4
// shotgun
//#define ST_WEAPON1X           122
//#define ST_WEAPON1Y           4
// chain gun
//#define ST_WEAPON2X           134
//#define ST_WEAPON2Y           4
// missile launcher
//#define ST_WEAPON3X           110
//#define ST_WEAPON3Y           13
// plasma gun
//#define ST_WEAPON4X           122
//#define ST_WEAPON4Y           13
// bfg
//#define ST_WEAPON5X           134
//#define ST_WEAPON5Y           13

// WPNS title
//#define ST_WPNSX              109
//#define ST_WPNSY              23

 // DETH title
//#define ST_DETHX              109
//#define ST_DETHY              23

//Incoming messages window location
// #define ST_MSGTEXTX     (viewwindowx)
// #define ST_MSGTEXTY     (viewwindowy+viewheight-18)
//#define ST_MSGTEXTX             0
//#define ST_MSGTEXTY             0     //added:08-01-98:unused
// Dimensions given in characters.
#define ST_MSGWIDTH             52
// Or shall I say, in lines?
#define ST_MSGHEIGHT            1

#define ST_OUTTEXTX             0
#define ST_OUTTEXTY             6

// Width, in characters again.
#define ST_OUTWIDTH             52
 // Height, in lines.
#define ST_OUTHEIGHT            1

#define ST_MAPWIDTH     \
    (strlen(mapnames[(gameepisode-1)*9+(gamemap-1)]))

//added:24-01-98:unused ?
//#define ST_MAPTITLEX  (vid.width - ST_MAPWIDTH * ST_CHATFONTWIDTH)

#define ST_MAPTITLEY            0
#define ST_MAPHEIGHT            1


int stbar_height = ST_HEIGHT;
int stbar_y = BASEVIDHEIGHT - ST_HEIGHT;
int stbar_x = 0;
int stbar_scalex, stbar_scaley;
int stbar_fg = FG | V_TRANSLUCENTPATCH;


//added:02-02-98: set true if widgets coords need to be recalculated
boolean     stbar_recalc;

// main player in game
//Hurdler: no not static!
player_t*   plyr;

// ST_Start() has just been called
boolean     st_force_refresh;

// used to execute ST_Init() only once
static int  veryfirsttime = 1;

// used for timing
static unsigned int     st_clock;

// used for making messages go away
static int              st_msgcounter=0;

// used when in chat
static st_chatstateenum_t       st_chatstate;

// whether left-side main status bar is active
boolean                 stbar_on;

// whether status bar chat is active
static boolean          st_chat;

// value of st_chat before message popped up
static boolean          st_oldchat;

// whether chat window has the cursor on
static boolean          st_cursor_on;

// !deathmatch
static boolean          st_notdeathmatch;

// main bar left
static patch_t*         sbar;

// 0-9, tall numbers
static patch_t*         tallnum[10];

// tall % sign
static patch_t*         tallpercent;

// 0-9, short, yellow (,different!) numbers
static patch_t*         shortnum[10];

// 3 key-cards, 3 skulls
static patch_t*         keys[NUMCARDS];

// face status patches
static patch_t*         faces[ST_NUMFACES];

// face background
static patch_t*         faceback;

 // main bar right
static patch_t*         armsbg;

// weapon ownership patches
static patch_t*         arms[6][2];

// ready-weapon widget
static st_number_t      w_ready;

 // in deathmatch only, summary of frags stats
static st_number_t      w_frags;

// health widget
static st_percent_t     w_health;

// arms background
static st_binicon_t     w_armsbg;

// weapon ownership widgets
static st_multicon_t    w_arms[6];

// face status widget
static st_multicon_t    w_faces;

// keycard widgets
static st_multicon_t    w_keyboxes[6];

// armor widget
static st_percent_t     w_armor;

// ammo widgets
static st_number_t      w_ammo[4];

// max ammo widgets
static st_number_t      w_maxammo[4];



 // number of frags so far in deathmatch
static int      st_fragscount;

// used to use appopriately pained face
static int      st_oldhealth = -1;

// used for evil grin
static boolean  oldweaponsowned[NUMWEAPONS];

 // count until face changes
static int      st_facecount = 0;

// current face index, used by w_faces
static int      st_faceindex = 0;

// holds key-type for each key box on bar
static byte     st_card;  // card state displayed
static byte     st_num_keyboxes;
static int      keyboxes[6];  // 0..3 keycards skulls, 4..6 dual display

// a random number per tick
static int      st_randomnumber;

// ------------------------------------------
//             status bar overlay
// ------------------------------------------

// icons for overlay
static   int   sbohealth;
static   int   sbofrags;
static   int   sboarmor;
static   int   sboammo[NUMWEAPONS];


//
// STATUS BAR CODE
//
static void ST_refreshBackground(void)
{
    byte*       colormap;

    if (stbar_on)
    {
        // Draw background, with status bar flag settings
        V_SetupDraw( BG | (stbar_fg & ~V_SCREENMASK) );

        // software mode copies patch to BG buffer,
        // hardware modes directly draw the statusbar to the screen
        V_DrawScaledPatch(stbar_x, stbar_y, sbar);

        // draw the faceback for the statusbarplayer
        colormap = (plyr->skincolor) ?
             SKIN_TO_SKINMAP( plyr->skincolor )
           : & reg_colormaps[0]; // default green skin

        V_DrawMappedPatch (stbar_x+ST_FX, stbar_y, faceback, colormap);

        // copy the statusbar buffer to the screen
        if ( rendermode==render_soft )
            V_CopyRect(0, vid.height-stbar_height, BG, vid.width, stbar_height, 0, vid.height-stbar_height, FG);
    }
}


// Respond to keyboard input events,
//  intercept cheats.
boolean ST_Responder (event_t* ev)
{

  if (ev->type == ev_keyup)
  {
    // Filter automap on/off : activates the statusbar while automap is active
    if( (ev->data1 & 0xffff0000) == AM_MSGHEADER )
    {
        switch(ev->data1)
        {
          case AM_MSGENTERED:
            st_force_refresh = true;        // force refresh of status bar
            break;

          case AM_MSGEXITED:
            break;
        }
    }

  }
  return false;
}



static int ST_calcPainOffset(void)
{
    int         health;
    static int  lastcalc;
    static int  oldhealth = -1;

    health = plyr->health > 100 ? 100 : plyr->health;

    if (health != oldhealth)
    {
        lastcalc = ST_FACESTRIDE * (((100 - health) * ST_NUMPAINFACES) / 101);
        oldhealth = health;
    }
    return lastcalc;
}


//
// This is a not-very-pretty routine which handles
//  the face states and their timing.
// the precedence of expressions is:
//  dead > evil grin > turned head > straight ahead
//
static void ST_updateFaceWidget(void)
{
    static int  fw_last_attackdown_cnt = -1;
    static int  fw_priority = 0;  // priority of face effects

    int         i;
    angle_t     badguyangle;
    angle_t     diffang;
    boolean     doevilgrin;

    // Highest priority first
    if (fw_priority < 10)
    {
        if (!plyr->health)
        {
            // dead
            fw_priority = 9;
            st_faceindex = ST_DEADFACE;
            st_facecount = 1;
        }
    }

    if (fw_priority < 9)
    {
        if (plyr->bonuscount)
        {
            // picking up bonus
            doevilgrin = false;

            for (i=0;i<NUMWEAPONS;i++)
            {
                if (oldweaponsowned[i] != plyr->weaponowned[i])
                {
                    doevilgrin = true;
                    oldweaponsowned[i] = plyr->weaponowned[i];
                }
            }
            if (doevilgrin)
            {
                // evil grin if just picked up weapon
                fw_priority = 8;  // allow retrigger evil grin
                st_facecount = ST_EVILGRINCOUNT;
                st_faceindex = ST_calcPainOffset() + ST_EVILGRINOFFSET;
            }
        }

    }

    if (fw_priority < 8)
    {
        if (plyr->damagecount
            && plyr->attacker
            && plyr->attacker != plyr->mo)
        {
            // being attacked
            fw_priority = 7;  // allow attack test retrigger

            // [WDJ] Ouch-face when damage>20, fix from DoomWiki, same as prboom
//            if (plyr->health - st_oldhealth > ST_MUCHPAIN) // orig bug
            if (st_oldhealth - plyr->health > ST_MUCHPAIN)
            {
                st_facecount = ST_TURNCOUNT;
                st_faceindex = ST_calcPainOffset() + ST_OUCHOFFSET;
                fw_priority = 8; // [WDJ] Necessary to keep ouchface visible
            }
            else
            {
                badguyangle = R_PointToAngle2(plyr->mo->x,
                                              plyr->mo->y,
                                              plyr->attacker->x,
                                              plyr->attacker->y);

                if (badguyangle > plyr->mo->angle)
                {
                    // whether right or left
                    diffang = badguyangle - plyr->mo->angle;
                    i = diffang > ANG180;
                }
                else
                {
                    // whether left or right
                    diffang = plyr->mo->angle - badguyangle;
                    i = diffang <= ANG180;
                } // confusing, aint it?


                st_facecount = ST_TURNCOUNT;
                st_faceindex = ST_calcPainOffset();

                if (diffang < ANG45)
                {
                    // head-on
                    st_faceindex += ST_RAMPAGEOFFSET;
                }
                else if (i)
                {
                    // turn face right
                    st_faceindex += ST_TURNOFFSET;
                }
                else
                {
                    // turn face left
                    st_faceindex += ST_TURNOFFSET+1;
                }
            }
        }
    }

    if (fw_priority < 7)
    {
        // getting hurt because of your own damn stupidity
        if (plyr->damagecount)
        {
            // [WDJ] Ouch-face when damage>20, fix from DoomWiki, same as prboom
//            if (plyr->health - st_oldhealth > ST_MUCHPAIN)
            if (st_oldhealth - plyr->health > ST_MUCHPAIN)
            {
                fw_priority = 7;  // no pain retrigger
                st_facecount = ST_TURNCOUNT;
                st_faceindex = ST_calcPainOffset() + ST_OUCHOFFSET;
            }
            else
            {
                fw_priority = 6;  // allow pain test retrigger
                st_facecount = ST_TURNCOUNT;
                st_faceindex = ST_calcPainOffset() + ST_RAMPAGEOFFSET;
            }
        }
    }

    if (fw_priority < 6)
    {
        // rapid firing
        if (plyr->attackdown)
        {
            if (fw_last_attackdown_cnt < 0)
                fw_last_attackdown_cnt = ST_RAMPAGEDELAY;
            else if (--fw_last_attackdown_cnt == 0)
            {
                fw_last_attackdown_cnt = 1;
                fw_priority = 5;  // continual retrigger, no timer
                st_facecount = 1;
                st_faceindex = ST_calcPainOffset() + ST_RAMPAGEOFFSET;
            }
        }
        else
            fw_last_attackdown_cnt = -1;
    }

    if (fw_priority < 5)
    {
        // invulnerability
        if ((plyr->cheats & CF_GODMODE)
            || plyr->powers[pw_invulnerability])
        {
            fw_priority = 4;  // continual retrigger, no timer
            st_facecount = 1;
            st_faceindex = ST_GODFACE;
        }
    }

    // look left or look right if the facecount has timed out
    if (st_facecount == 0)
    {
        fw_priority = 0;  // clear
        st_faceindex = ST_calcPainOffset() + (st_randomnumber % 3);
        st_facecount = ST_STRAIGHTFACECOUNT;
    }

    st_facecount--;
}

boolean ST_SameTeam(player_t *a,player_t *b)
{
    switch (cv_teamplay.value) {
       case 0 : return false;
       case 1 : return (a->skincolor == b->skincolor);
       case 2 : return (a->skin == b->skin);
    }
    return false;
}

// count the frags of the playernum player
//Fab: made as a tiny routine so ST_overlayDrawer() can use it
//Boris: rename ST_countFrags in to ST_PlayerFrags for use anytime
//       when we need the frags
int ST_PlayerFrags (int playernum)
{
    player_t * player = &players[playernum];
    int    i,frags;

    frags = player->addfrags;
    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if ((cv_teamplay.value==0 && i != playernum)
         || (cv_teamplay.value && !ST_SameTeam(&players[i], player)) )
            frags += player->frags[i];
        else
            frags -= player->frags[i];
    }

    return frags;
}


// Called by: ST_Ticker
static void ST_updateWidgets(void)
{
    static int  largeammo = NON_NUMBER; // means "n/a"
    int         i;

#ifdef PARANOIA
    if(!plyr)  return;  // not likely, but have soft fail
#endif
    // must redirect the pointer if the ready weapon has changed.
    //  if (w_ready.data != plyr->readyweapon)
    //  {
    if (plyr->weaponinfo[plyr->readyweapon].ammo == am_noammo)
        w_ready.num = &largeammo;
    else
        w_ready.num = &plyr->ammo[plyr->weaponinfo[plyr->readyweapon].ammo];
    //{
    // static int tic=0;
    // static int dir=-1;
    // if (!(tic&15))
    //   plyr->ammo[weaponinfo[plyr->readyweapon].ammo]+=dir;
    // if (plyr->ammo[weaponinfo[plyr->readyweapon].ammo] == -100)
    //   dir = 1;
    // tic++;
    // }
//    w_ready.data = plyr->readyweapon;

    // if (*w_ready.on)
    //  STlib_updateNum(&w_ready, true);
    // refresh weapon change
    //  }

    // update keycard multiple widgets
    if( plyr->cards != st_card )
    {
        // fraggle script can take a key, so keyboxes must not be sticky.
        if( ~plyr->cards & st_card )
        {
             st_force_refresh = true;  // a card was taken
             st_card = 0;
        }
        if( (plyr->cards & 0x07) && (plyr->cards & 0x38) )
        {
            // Have both keycards and skulls.
            if(((st_card & 0x07) == 0) || ((st_card & 0x38) == 0))
            {
                // Enable display of both skulls and keycards.
                st_num_keyboxes = 6;
                // Shift keybox[ 0..6 ] positions, into two vertical columns
		// Skulls will now be in [3..6]
                for( i=0; i<6; i++ )
                {
                    keyboxes[i] = -1;  // clear previous recorded skulls
                    w_keyboxes[i].x = stbar_x + keybox_dual_x[i],
                    w_keyboxes[i].y = stbar_y + keybox_dual_y[i],
                    w_keyboxes[i].command = STLIB_REFRESH;   // to clear old card positions
                }
            }
        }
        st_card = plyr->cards;

        for (i=0;i<3;i++)
        {
            // keycards
            keyboxes[i] = ((st_card >> i) & 0x01) ? i : -1;

            // skull keys
            if ((st_card >> i) & 0x08)
            {
                keyboxes[i+3] = i+3;  // dual display
                if( st_num_keyboxes == 3 )
                    keyboxes[i] = i+3;  // only skull display
            }
            else
            {
                keyboxes[i+3] = -1;  // skull off in dual display
            }
        }
    }

    // refresh everything if this is him coming back to life
    ST_updateFaceWidget();

    // used by the w_armsbg widget
    st_notdeathmatch = !cv_deathmatch.value;

    st_fragscount = ST_PlayerFrags(statusbarplayer);

    // get rid of chat window if up because of message
    if (!--st_msgcounter)
        st_chat = st_oldchat;

}

static boolean  st_stopped = true;

void ST_Ticker (void)
{
    if( st_stopped )
        return;

    if( gamemode == heretic )
    {
        SB_Heretic_Ticker();
        return;
    }

    st_clock++;
    st_randomnumber = M_Random();
    ST_updateWidgets();
    st_oldhealth = plyr->health;

}

static int st_palette = 0;
// Used by Heretic too.
byte pickupflash_table[ 4 ] = { 6, 5, 4, 3 }; // Vanilla=[3]=3

// Called by: ST_Drawer
void ST_doPaletteStuff(void)
{
    int  palette;
    int  red_cnt;

    red_cnt = plyr->damagecount;

    if (plyr->powers[pw_strength])
    {
        // slowly fade the berzerk out
        int bzc = 12 - (plyr->powers[pw_strength]>>6);

        if (bzc > red_cnt)
            red_cnt = bzc;
    }

    if (red_cnt)
    {
        palette = STARTREDPALS + ((red_cnt+7)>>3);

        if (palette >= (STARTREDPALS+NUMREDPALS))
            palette = STARTREDPALS+NUMREDPALS-1;
    }
    else
    if (plyr->bonuscount && (cv_pickupflash.value>=2))
    {
        // Pickup object palette flash.
        palette = STARTBONUSPALS
           + ((plyr->bonuscount+7)>>(pickupflash_table[cv_pickupflash.value]));

        if (palette >= (STARTBONUSPALS+NUMBONUSPALS))
            palette = STARTBONUSPALS+NUMBONUSPALS-1;
    }
    else
    if ( plyr->powers[pw_ironfeet] > 4*32
      || plyr->powers[pw_ironfeet]&8)
        palette = RADIATIONPAL;
    else
        palette = 0;


    //added:28-02-98:quick hack underwater palette
    /*if (plyr->mo &&
        (plyr->mo->z + (cv_viewheight.value<<FRACBITS) < plyr->mo->waterz) )
        palette = RADIATIONPAL;*/

    if (palette != st_palette)
    {
        st_palette = palette;

#ifdef HWRENDER // not win32 only 19990829 by Kin
        if ( (rendermode == render_opengl) || (rendermode == render_d3d) )
        
        //faB - NOW DO ONLY IN SOFTWARE MODE, LETS FIX THIS FOR GOOD OR NEVER
        //      idea : use a true color gradient from frame to frame, because we
        //             are in true color in HW3D, we can have smoother palette change
        //             than the palettes defined in the wad

        {
            // Imitate the special object screen tints for each special palette.
            //CONS_Printf("palette: %d\n", palette);
            switch (palette) {
                case 0x00: HWD.pfnSetSpecialState(HWD_SET_TINT_COLOR, 0x0); break;  // pas de changement
                case 0x01: HWD.pfnSetSpecialState(HWD_SET_TINT_COLOR, 0xff373797); break; // red
                case 0x02: HWD.pfnSetSpecialState(HWD_SET_TINT_COLOR, 0xff373797); break; // red
                case 0x03: HWD.pfnSetSpecialState(HWD_SET_TINT_COLOR, 0xff3030a7); break; // red
                case 0x04: HWD.pfnSetSpecialState(HWD_SET_TINT_COLOR, 0xff2727b7); break; // red
                case 0x05: HWD.pfnSetSpecialState(HWD_SET_TINT_COLOR, 0xff2020c7); break; // red
                case 0x06: HWD.pfnSetSpecialState(HWD_SET_TINT_COLOR, 0xff1717d7); break; // red
                case 0x07: HWD.pfnSetSpecialState(HWD_SET_TINT_COLOR, 0xff1010e7); break; // red
                case 0x08: HWD.pfnSetSpecialState(HWD_SET_TINT_COLOR, 0xff0707f7); break; // red
                case 0x09: HWD.pfnSetSpecialState(HWD_SET_TINT_COLOR, 0xffff6060); break; // blue
                case 0x0a: HWD.pfnSetSpecialState(HWD_SET_TINT_COLOR, 0xff70a090); break; // light green
                case 0x0b: HWD.pfnSetSpecialState(HWD_SET_TINT_COLOR, 0xff67b097); break; // light green
                case 0x0c: HWD.pfnSetSpecialState(HWD_SET_TINT_COLOR, 0xff60c0a0); break; // light green
                case 0x0d: HWD.pfnSetSpecialState(HWD_SET_TINT_COLOR, 0xff60ff60); break; // green
                case 0x0e: HWD.pfnSetSpecialState(HWD_SET_TINT_COLOR, 0xffff6060); break; // blue
                case 0x0f: HWD.pfnSetSpecialState(HWD_SET_TINT_COLOR, 0xffff6060); break; // blue
            }
        }
        else
#endif
        {
            if( !cv_splitscreen.value || !palette )
                V_SetPalette (palette);
        }
    }
}

// Called by: ST_doRefresh, ST_diffDraw, when stbar_on
// STlib refresh enable is now setup by caller.
// Only called when stbar_on == true, so more tests are pointless.
static void ST_drawWidgets( void )
{
    int  i;

    // Draw stbar_fg, screen0 status bar
    V_SetupDraw( stbar_fg );  // for all STlib

    if( cv_pickupflash.value == 1 )
    {
        // Pickup flash on the status bar.
        if( plyr->ammo_pickup )
        {
            w_ready.command = STLIB_FLASH;
        }
        if( plyr->armor_pickup )
        {
            w_armor.ni.command = STLIB_FLASH;
        }
        if( plyr->health_pickup )
        {
            w_health.ni.command = STLIB_FLASH;
        }
        if( plyr->key_pickup )
        {
            // Flash entire box.
            // Do not know which one was picked up.
            V_DrawFill(stbar_x + ST_KEYSBOX_X, stbar_y + ST_KEYSBOX_Y,
                       ST_KEYSBOX_W, ST_KEYSBOX_H, FLASH_COLOR);
            // Prevent the key icons from performing background refresh.
            for (i=0;i<st_num_keyboxes;i++)
                w_keyboxes[i].command = STLIB_FLASH;
        }
        else if( w_keyboxes[0].command == STLIB_FLASH_CLEAR )
        {
            // Restore the background
            V_CopyRect(stbar_x + ST_KEYSBOX_X, stbar_y + ST_KEYSBOX_Y, BG,
                       ST_KEYSBOX_W, ST_KEYSBOX_H,
                       stbar_x + ST_KEYSBOX_X, stbar_y + ST_KEYSBOX_Y, stbar_fg);
            // Refresh the key icons.
            for (i=0;i<st_num_keyboxes;i++)
                w_keyboxes[i].command = STLIB_REFRESH;
        }
    }

    STlib_updateNum(&w_ready);  // current weapon ammo

    for (i=0;i<4;i++)
    {
        STlib_updateNum(&w_ammo[i]);
        STlib_updateNum(&w_maxammo[i]);
    }

    STlib_updatePercent(&w_health);
    STlib_updatePercent(&w_armor);

    STlib_updateBinIcon(&w_armsbg);

    if( cv_deathmatch.value )
    {
        // frags on
        STlib_updateNum(&w_frags);
    }
    else   
    {
        // arms on
        for (i=0;i<6;i++)
            STlib_updateMultIcon(&w_arms[i]);
    }

    STlib_updateMultIcon(&w_faces);

    for (i=0;i<st_num_keyboxes;i++)
        STlib_updateMultIcon(&w_keyboxes[i]);
}

// Called by ST_Drawer when stbar_on
static void ST_doRefresh(void)
{
    // This is not executed as frequently as ST_diffDraw, so it is more
    // complicated, in order to keep ST_diffDraw and ST_drawWidgets simpler.
    
    // Draw status bar background to off-screen buff
    ST_refreshBackground();

    stlib_enable_erase = !st_overlay && rendermode==render_soft;
    // Command to force global refresh from BG
    stlib_force_refresh = true;

    // and refresh all widgets
    ST_drawWidgets();
}

// Called by ST_Drawer when stbar_on
static void ST_diffDraw(void)
{
    stlib_force_refresh = false;
    // update all widgets
    ST_drawWidgets();
}

void ST_Invalidate(void)
{
    st_force_refresh = true;
    st_card = 0;
}

void ST_overlayDrawer ();

void ST_Drawer ( boolean refresh )
{
    stbar_on = (cv_viewsize.value<11) || automapactive;

    if( gamemode == heretic )
    {
        SB_Heretic_Drawer( refresh );
        return;
    }

    //added:30-01-98:force a set of the palette by doPaletteStuff()
    if (vid.recalc)
        st_palette = -1;

    // Do red-/gold-shifts from damage/items
#ifdef HWRENDER // not win32 only 19990829 by Kin
//25/08/99: Hurdler: palette changes is done for all players,
//                   not only player1 ! That's why this part 
//                   of code is moved somewhere else.
    if (rendermode==render_soft)
#endif
        ST_doPaletteStuff();

    if( stbar_on )
    {
        // after ST_Start(), screen refresh needed, or vid mode change
        if (st_force_refresh || refresh || stbar_recalc )
        {
            if (stbar_recalc)  //recalc widget coords after vid mode change
            {
                ST_createWidgets ();
                stbar_recalc = false;
            }
            st_force_refresh = false;
            st_card = 0;
            ST_doRefresh();
        }
        else
        {
            // Otherwise, update as little as possible
            ST_diffDraw();
        }
    }
    else
    {
        if( st_overlay )
        {
            if( !playerdeadview || cv_splitscreen.value)
            {
                plyr= displayplayer_ptr;
                ST_overlayDrawer ();
            }
            if( cv_splitscreen.value && displayplayer2_ptr )
            {
                plyr= displayplayer2_ptr;
                ST_overlayDrawer ();
            }
        }
    }
}


static void ST_loadGraphics(void)
{

    int         i;
    char        namebuf[9];
    // [WDJ] all ST graphics are loaded endian fixed
    // [WDJ] Lock the status bar graphics against other texture users.

    // Load the numbers, tall and short
    for (i=0;i<10;i++)
    {
        sprintf(namebuf, "STTNUM%d", i);
        tallnum[i] = (patch_t *) W_CachePatchName(namebuf, PU_LOCK_SB);

        sprintf(namebuf, "STYSNUM%d", i);
        shortnum[i] = (patch_t *) W_CachePatchName(namebuf, PU_LOCK_SB);
    }

    // Load percent key.
    //Note: why not load STMINUS here, too?
    tallpercent = (patch_t *) W_CachePatchName("STTPRCNT", PU_LOCK_SB);

    // key cards
    // FreeDoom and DoomII have STKEYS 0..5.
    for (i=0;i<NUMCARDS;i++)
    {
        sprintf(namebuf, "STKEYS%d", i);
        keys[i] = (patch_t *) W_CachePatchName(namebuf, PU_LOCK_SB);
    }

    // arms background
    armsbg = (patch_t *) W_CachePatchName("STARMS", PU_LOCK_SB);

    // arms ownership widgets
    for (i=0;i<6;i++)
    {
        sprintf(namebuf, "STGNUM%d", i+2);

        // gray #
        arms[i][0] = (patch_t *) W_CachePatchName(namebuf, PU_LOCK_SB);

        // yellow #
        arms[i][1] = shortnum[i+2];
    }

    // status bar background bits
    sbar = (patch_t *) W_CachePatchName("STBAR", PU_LOCK_SB);

    // the original Doom uses 'STF' as base name for all face graphics
    ST_loadFaceGraphics ("STF");
}


// made separate so that skins code can reload custom face graphics
void ST_loadFaceGraphics (char *facestr)
{
    int   i,j;
    int   facenum;
    char  namelump[9];
    char* namebuf;
    // [WDJ] all ST graphics are loaded endian fixed

    //hack: make sure base face name is no more than 3 chars
    // bug: core dump fixed 19990220 by Kin
    if( strlen(facestr)>3 )
        facestr[3]='\0';
    strcpy (namelump, facestr);  // copy base name
    namebuf = namelump;
    while (*namebuf>' ') namebuf++;

    // face states
    facenum = 0;
    for (i=0;i<ST_NUMPAINFACES;i++)
    {
        for (j=0;j<ST_NUMSTRAIGHTFACES;j++)
        {
            sprintf(namebuf, "ST%d%d", i, j);
            faces[facenum++] = W_CachePatchName(namelump, PU_LOCK_SB);
        }
        sprintf(namebuf, "TR%d0", i);        // turn right
        faces[facenum++] = W_CachePatchName(namelump, PU_LOCK_SB);
        sprintf(namebuf, "TL%d0", i);        // turn left
        faces[facenum++] = W_CachePatchName(namelump, PU_LOCK_SB);
        sprintf(namebuf, "OUCH%d", i);       // ouch!
        faces[facenum++] = W_CachePatchName(namelump, PU_LOCK_SB);
        sprintf(namebuf, "EVL%d", i);        // evil grin ;)
        faces[facenum++] = W_CachePatchName(namelump, PU_LOCK_SB);
        sprintf(namebuf, "KILL%d", i);       // pissed off
        faces[facenum++] = W_CachePatchName(namelump, PU_LOCK_SB);
    }
    strcpy (namebuf, "GOD0");
    faces[facenum++] = W_CachePatchName(namelump, PU_LOCK_SB);
    strcpy (namebuf, "DEAD0");
    faces[facenum++] = W_CachePatchName(namelump, PU_LOCK_SB);

    // face backgrounds for different player colors
    //added:08-02-98: uses only STFB0, which is remapped to the right
    //                colors using the player translation tables, so if
    //                you add new player colors, it is automatically
    //                used for the statusbar.
    strcpy (namebuf, "B0");
    i = W_CheckNumForName(namelump);
    if( i!=-1 )
        faceback = (patch_t *) W_CachePatchNum(i, PU_LOCK_SB);
    else
        faceback = (patch_t *) W_CachePatchName("STFB0", PU_LOCK_SB);

    ST_Invalidate();
}


static void ST_loadData(void)
{
    ST_loadGraphics();
}

void ST_unloadGraphics(void)
{

    int i;

    //faB: GlidePatch_t are always purgeable
    if (rendermode==render_soft)
    {
        // unload the numbers, tall and short
        for (i=0;i<10;i++)
        {
            Z_ChangeTag(tallnum[i], PU_UNLOCK_CACHE);
            Z_ChangeTag(shortnum[i], PU_UNLOCK_CACHE);
        }
        // unload tall percent
        Z_ChangeTag(tallpercent, PU_UNLOCK_CACHE);
        
        // unload arms background
        Z_ChangeTag(armsbg, PU_UNLOCK_CACHE);
        
        // unload gray #'s
        for (i=0;i<6;i++)
            Z_ChangeTag(arms[i][0], PU_UNLOCK_CACHE);
        
        // unload the key cards
        for (i=0;i<NUMCARDS;i++)
            Z_ChangeTag(keys[i], PU_UNLOCK_CACHE);
        
        Z_ChangeTag(sbar, PU_UNLOCK_CACHE);
    }

    ST_unloadFaceGraphics ();

    // Note: nobody ain't seen no unloading
    //   of stminus yet. Dude.

}

// made separate so that skins code can reload custom face graphics
void ST_unloadFaceGraphics (void)
{
    int    i;

    //faB: GlidePatch_t are always purgeable
    if (rendermode==render_soft)
    {
        for (i=0;i<ST_NUMFACES;i++)
            Z_ChangeTag(faces[i], PU_UNLOCK_CACHE);
        
        // face background
        Z_ChangeTag(faceback, PU_UNLOCK_CACHE);
    }
}


void ST_unloadData(void)
{
    ST_unloadGraphics();
}

void ST_initData(void)
{

    int         i;

    st_force_refresh = true;

    //added:16-01-98:'link' the statusbar display to a player, which could be
    //               another player than consoleplayer, for example, when you
    //               change the view in a multiplayer demo with F12.
    if (singledemo)
        statusbarplayer = displayplayer;
    else
        statusbarplayer = consoleplayer;

    plyr = &players[statusbarplayer];

    st_clock = 0;
    st_chatstate = StartChatState;

    stbar_on = true;
    st_oldchat = st_chat = false;
    st_cursor_on = false;

    st_faceindex = 0;
    st_palette = -1;

    st_oldhealth = -1;

    for (i=0;i<NUMWEAPONS;i++)
        oldweaponsowned[i] = plyr->weaponowned[i];

    st_card = 0;  // no keys
    st_num_keyboxes = 3;
    for (i=0;i<6;i++)
        keyboxes[i] = -1;

    STlib_init();
}


void ST_CalcPos(void)
{
    if( cv_scalestatusbar.value || cv_viewsize.value>=11 )
    {
        // large scaled status bar
        stbar_fg = FG | V_SCALEPATCH | V_SCALESTART | V_TRANSLUCENTPATCH;
        stbar_scalex = vid.dupx;
        stbar_scaley = vid.dupy;

#ifdef HWRENDER
        if( rendermode != render_soft )
        {
            stbar_x = 0;
            stbar_y = BASEVIDHEIGHT - stbar_height/vid.fdupy;
        }
        else
#endif

        {
            stbar_x = ((vid.width - ST_WIDTH*vid.dupx)>>1)/vid.dupx;
            stbar_y = (vid.height - stbar_height)/vid.dupy;
        }
    }
    else
    {
        // smaller unscaled status bar in center
        stbar_fg = FG | V_NOSCALE | V_TRANSLUCENTPATCH;
        stbar_scalex = stbar_scaley = 1;
        stbar_x = (vid.width - ST_WIDTH)>>1;  // center
        stbar_y = vid.height - stbar_height;
    }
}

//added:30-01-98: NOTE: this is called at any level start, view change,
//                      and after vid mode change.
void ST_createWidgets(void)
{
    int i;

    ST_CalcPos();

    // ready weapon ammo
    STlib_initNum(&w_ready,
                  stbar_x + ST_AMMOX,
                  stbar_y + ST_AMMOY,
                  tallnum,
                  &plyr->ammo[plyr->weaponinfo[plyr->readyweapon].ammo],
                  ST_AMMOWIDTH );

    // the last weapon type
//    w_ready.data = plyr->readyweapon;

    // health percentage
    STlib_initPercent(&w_health,
                      stbar_x + ST_HEALTHX,
                      stbar_y + ST_HEALTHY,
                      tallnum,
                      &plyr->health,
                      tallpercent);

    // arms background
    STlib_initBinIcon(&w_armsbg,
                      stbar_x + ST_ARMSBGX,
                      stbar_y + ST_ARMSBGY,
                      armsbg,
                      &st_notdeathmatch );

    // weapons owned, draw enabled by !cv_deathmatch
    for(i=0;i<6;i++)
    {
        STlib_initMultIcon(&w_arms[i],
                      stbar_x + ST_ARMSX + (i%3)*ST_ARMSXSPACE,
                      stbar_y + ST_ARMSY + (i/3)*ST_ARMSYSPACE,
                      arms[i], (int *) &plyr->weaponowned[i+1] );
    }

    // frags sum, draw enabled by cv_deathmatch
    STlib_initNum(&w_frags,
                  stbar_x + ST_FRAGSX,
                  stbar_y + ST_FRAGSY,
                  tallnum,
                  &st_fragscount,
                  ST_FRAGSWIDTH);

    // faces
    STlib_initMultIcon(&w_faces,
                       stbar_x + ST_FACESX,
                       stbar_y + ST_FACESY,
                       faces,
                       &st_faceindex );

    // armor percentage - should be colored later
    STlib_initPercent(&w_armor,
                      stbar_x + ST_ARMORX,
                      stbar_y + ST_ARMORY,
                      tallnum,
                      &plyr->armorpoints,
                      tallpercent);

    // keyboxes 0-6, in vertical column
    for( i=0; i<6; i++ )
    {
        STlib_initMultIcon(&w_keyboxes[i],
                       stbar_x + ST_KEYX,
                       stbar_y + keybox_y[i],
                       keys,  // patches
                       &keyboxes[i] );
    }

    for( i=0; i<4; i++ )
    {
        // In vertical column.
        // ammo count (all four kinds)
        STlib_initNum(&w_ammo[i],
                  stbar_x + ST_AMMOSX,
                  stbar_y + ammobox_y[i],
                  shortnum,
                  &plyr->ammo[i],
                  ST_AMMOS_WIDTH);
        // max ammo count (all four kinds)
        STlib_initNum(&w_maxammo[i],
                  stbar_x + ST_MAXAMMOSX,
                  stbar_y + ammobox_y[i],
                  shortnum,
                  &plyr->maxammo[i],
                  ST_MAXAMMOS_WIDTH);
    }
}

static void ST_Stop (void)
{
    if (st_stopped)
        return;

    V_SetPalette (0);

    st_stopped = true;
}

void ST_Start (void)
{
    if( gamemode == heretic )
    {
        plyr = &players[statusbarplayer];
        st_stopped = false;
        return;
    }
    if (!st_stopped)
        ST_Stop();

    ST_initData();
    ST_createWidgets();
    st_stopped = false;
    stbar_recalc = false;  //added:02-02-98: widgets coords have been setup
                           // see ST_drawer()
}

//
//  Initializes the status bar,
//  sets the defaults border patch for the window borders.
//

//faB: used by Glide mode, holds lumpnum of flat used to fill space around the viewwindow
int  st_borderflat_num;  // extern in r_draw.h

void ST_Init (void)
{
    int     i;

    if(dedicated)
        return;
    
    //added:26-01-98:screens[4] is allocated at videomode setup, and
    //               set at V_Init(), the first time being at SCR_Recalc()

    // choose and cache the default border patch
    switch(gamemode) {
        case doom2_commercial :
            // DOOM II border patch, original was GRNROCK
            st_borderflat_num = W_GetNumForName ("GRNROCK");
            break;
        case heretic :
            if(W_CheckNumForName("e2m1")==-1)
                // GDESC_heretic_shareware
                st_borderflat_num = W_GetNumForName ("FLOOR04");
            else
                st_borderflat_num = W_GetNumForName ("FLAT513");
            break;
        case hexen :
            st_borderflat_num = W_GetNumForName ("F_022");
            break;
        default :
            // DOOM border patch.
            st_borderflat_num = W_GetNumForName ("FLOOR7_2");
    }
    // [WDJ] Lock against other users of same patch releasing it!.
    scr_borderflat = W_CacheLumpNum (st_borderflat_num, PU_LOCK_SB);
    if( gamemode == heretic )
    {
        SB_Heretic_Init();
        return;
    }
    veryfirsttime = 0;
    ST_loadData();

    //
    // cache the status bar overlay icons  (fullscreen mode)
    //
    sbohealth = W_GetNumForName ("SBOHEALT");
    sbofrags  = W_GetNumForName ("SBOFRAGS");
    sboarmor  = W_GetNumForName ("SBOARMOR");

    for (i=0;i<NUMWEAPONS;i++)
    {
        if (i>0 && i!=7)
            sboammo[i] = W_GetNumForName (va("SBOAMMO%c",'0'+i));
        else
            sboammo[i] = 0;
    }
}

 //added:16-01-98: change the status bar too, when pressing F12 while viewing
//                 a demo.
void ST_changeDemoView (void)
{
    //the same routine is called at multiplayer deathmatch spawn
    // so it can be called multiple times
    ST_Start();
}


// =========================================================================
//                         STATUS BAR OVERLAY
// =========================================================================

consvar_t cv_stbaroverlay = {"overlay","kahmf",CV_SAVE,NULL};

boolean   st_overlay;


void ST_AddCommands (void)
{
    CV_RegisterVar (&cv_stbaroverlay);
}


//  Draw a number, scaled, over the view
//  Always draw the number completely since it's overlay
//
//   x, y: scaled position
static
void ST_drawOverlayNum (int       x,            // right border!
                        int       y,
                        int       num,
                        patch_t** numpat,
                        patch_t*  percent,
                        byte      pickup_flash )
{
    int  hf = numpat[0]->height;
    int  wf = numpat[0]->width;
    int  wfv = wf * vid.dupx;
    boolean   neg;

    V_SetupDraw( FG | V_NOSCALE | V_SCALEPATCH | V_TRANSLUCENTPATCH );
   
    if( pickup_flash && (cv_pickupflash.value == 1))
    {
        // Assume 3 digits  0..200
        V_DrawVidFill(x - (wfv*3), y, wfv*3, hf*vid.dupy, FLASH_COLOR);
    }

    // in the special case of 0, you draw 0
    if (num == 0)
    {
        V_DrawScaledPatch(x - wfv, y, numpat[ 0 ]);
        return;
    }

    neg = num < 0;

    if (neg)
        num = -num;

    // draw the number
    while (num)
    {
        x -= wfv;
        V_DrawScaledPatch(x, y, numpat[ num % 10 ]);
        num /= 10;
    }

    // draw a minus sign if necessary
    if (neg)
        V_DrawScaledPatch(x - (8*vid.dupx), y, sttminus);
}


static inline int SCY( int y )
{ 
    //31/10/99: fixed by Hurdler so it _works_ also in hardware mode
    // do not scale to resolution for hardware accelerated
    // because these modes always scale by default
    y = y * vid.fdupy;     // scale to resolution
    if ( cv_splitscreen.value ) {
        y >>= 1;
        if (plyr != &players[statusbarplayer])
            y += vid.height / 2;
    }
    return y;
}


static inline int SCX( int x )
{
    return x * vid.fdupx;
}

static
void  ST_drawOverlayKeys( int x, int y, byte cards )
{
    int  i, yh, xinc, yinc;

    xinc = (int)((ST_KEY_WIDTH + 1) * vid.fdupx);
    yinc = (int)((ST_KEY_HEIGHT + 1) * vid.fdupy);
    yh = y;  // upper row is same as lower row when no skull keys
    // if both skull and cards, then move cards up a row	  
    if( cards & 0x38 )
        yh -= yinc;

    if( plyr->key_pickup && (cv_pickupflash.value == 1))
    {
        V_DrawVidFill(x - (xinc*3), yh, (xinc*3), y - yh + yinc, FLASH_COLOR);
    }
   
    for (i=0;i<3;i++)
    {
        x -= xinc;
        if( (cards >> i) & 0x08 ) // skull
        {
            V_DrawScaledPatch(x, y, keys[i+3]);  // skull graphic lower row
        }
        if( (cards >> i) & 0x01 ) // card
        {
            V_DrawScaledPatch(x, yh, keys[i]);  // keycard graphic upper row
        }
    }
}


//  Draw the status bar overlay, customisable : the user choose which
//  kind of information to overlay
//
void ST_overlayDrawer ()
{
    char*  cmds;
    char   c;
    int    i;
    // [WDJ] 8/2012 fix opengl overlay position to use fdupy
    float  sf_dupy = (rendermode == render_soft)? vid.dupy : vid.fdupy ;
    int    lowerbar_y = SCY(198) - (int)( 16 * sf_dupy );

    // Draw screen0, scaled, abs position
    V_SetupDraw( FG | V_NOSCALE | V_SCALEPATCH );
    // x, y are already scaled.

    cmds = cv_stbaroverlay.string;

    while ((c=*cmds++))
    {
       if (c>='A' && c<='Z')
           c = c + 'a' - 'A';
       switch (c)
       {
         case 'h': // draw health
           ST_drawOverlayNum(SCX(50), lowerbar_y,
                             plyr->health,
                             tallnum, NULL, plyr->health_pickup);

           V_DrawScalePic_Num (SCX(52), lowerbar_y, sbohealth);
           break;

         case 'f': // draw frags
           st_fragscount = ST_PlayerFrags(plyr-players);

           if (cv_deathmatch.value)
           {
               ST_drawOverlayNum(SCX(300), SCY(2),
                                 st_fragscount,
                                 tallnum, NULL, 0);

               V_DrawScalePic_Num (SCX(302), SCY(2), sbofrags);
           }
           break;

         case 'a': // draw ammo
           i = sboammo[plyr->readyweapon];
           if (i)
           {
               ST_drawOverlayNum(SCX(234), lowerbar_y,
                                 plyr->ammo[plyr->weaponinfo[plyr->readyweapon].ammo],
                                 tallnum, NULL, plyr->ammo_pickup);

               V_DrawScalePic_Num (SCX(236), lowerbar_y, i);
           }
           break;

         case 'k': // draw keys
           ST_drawOverlayKeys( SCX(318), lowerbar_y - (8 * sf_dupy), plyr->cards );
           break;

         case 'm': // draw armor
           ST_drawOverlayNum(SCX(300), lowerbar_y,
                             plyr->armorpoints,
                             tallnum, NULL, plyr->armor_pickup);

           V_DrawScalePic_Num (SCX(302), lowerbar_y, sboarmor);
           break;

         // added by Hurdler for single player only
         case 'e': // number of monster killed 
           if ( (!cv_deathmatch.value) && (!cv_splitscreen.value) )
           {
               char buf[16];
               sprintf(buf, "%d/%d", plyr->killcount, totalkills);
               V_DrawString(SCX(318-V_StringWidth(buf)), SCY(1), 0, buf);
           }
           break;

         case 's': // number of secrets found
           if ( (!cv_deathmatch.value) && (!cv_splitscreen.value) )
           {
               char buf[16];
               sprintf(buf, "%d/%d", plyr->secretcount, totalsecret);
               V_DrawString(SCX(318-V_StringWidth(buf)), SCY(11), 0, buf);
           }
           break;

           /* //TODO
         case 'r': // current frame rate
           {
               char buf[8];
               int framerate = 35;
               sprintf(buf, "%d FPS", framerate);
               V_DrawString(SCX(2), SCY(4), 0, buf);
           }
           break;
           */
       }
    }
}
