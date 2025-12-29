// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: g_game.h 1422 2019-01-29 08:05:39Z wesleyjohnson $
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
// $Log: g_game.h,v $
// Revision 1.13  2004/07/27 08:19:35  exl
// New fmod, fs functions, bugfix or 2, patrol nodes
//
// Revision 1.12  2003/03/22 22:35:59  hurdler
//
// Revision 1.11  2002/09/27 16:40:08  tonyd
// First commit of acbot
//
// Revision 1.10  2001/12/15 18:41:35  hurdler
// small commit, mainly splitscreen fix
//
// Revision 1.9  2001/08/12 15:21:04  bpereira
// see my log
//
// Revision 1.8  2001/01/25 22:15:42  bpereira
// added heretic support
//
// Revision 1.7  2000/11/26 20:36:14  hurdler
// Adding autorun2
//
// Revision 1.6  2000/11/02 19:49:35  bpereira
// Revision 1.5  2000/10/21 08:43:29  bpereira
// Revision 1.4  2000/10/01 10:18:17  bpereira
//
// Revision 1.3  2000/04/07 23:11:17  metzgermeister
// added mouse move
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//   
// 
//-----------------------------------------------------------------------------


#ifndef G_GAME_H
#define G_GAME_H

#include "doomdef.h"
  // SAVEGAMEDIR
#include "doomtype.h"
  // PACKED_ATTR
#include "doomstat.h"
#include "d_event.h"

// ---- Teams

// [WDJ] cannot write to const team_names
// Created basic team record
typedef struct {
    char * name;  // always allocated string
} PACKED_ATTR  team_info_t;

extern team_info_t*  team_info[MAXTEAMS];
extern byte          num_teams;  // limited to MAXTEAMS (32)

team_info_t*  get_team( int team_num );
void  set_team_name( int team_num, const char * str );
char * get_team_name( int team_num );

extern  player_t  players[MAXPLAYERS];
extern  boolean   playeringame[MAXPLAYERS];
//added:11-02-98: yeah now you can change it!
// changed to 2d array 19990220 by Kin
extern char       player_names[MAXPLAYERS][MAXPLAYERNAME];



extern  char      game_map_filename[MAX_WADPATH];
extern  boolean   nomonsters;   // checkparm of -nomonsters

extern  boolean   gameplay_msg;  // True during gameplay

// --- Event actions

typedef enum
{
    ga_nothing,
    ga_completed,
    ga_worlddone,
    ga_playdemo,
    //HeXen
/*
    ga_initnew,
    ga_newgame,
    ga_loadgame,
    ga_savegame,
    ga_leavemap,
    ga_singlereborn
*/
} gameaction_e;

extern  gameaction_e    gameaction;

// ======================================
// DEMO playback/recording related stuff.
// ======================================

// demoplaying back and demo recording
extern  boolean   demoplayback;
extern  boolean   demorecording;
extern  boolean   timingdemo;       

// Quit after playing a demo from cmdline.
extern  boolean   singledemo;

// gametic at level start
extern  tic_t     levelstarttic;  

// used in game menu
extern consvar_t  cv_crosshair;
//extern consvar_t  cv_crosshairscale;
extern consvar_t  cv_showmessages;
extern consvar_t  cv_pickupflash;
extern consvar_t  cv_weapon_recoil;  // Boom weapon recoil
extern consvar_t  cv_fastmonsters;
extern consvar_t  cv_predictingmonsters;  //added by AC for predmonsters

extern consvar_t  cv_allowjump;
extern consvar_t  cv_allowrocketjump;
extern consvar_t  cv_allowautoaim;
extern consvar_t  cv_allowmlook;
extern consvar_t  cv_allowturbo ;
extern consvar_t  cv_allowexitlevel;


void Command_Turbo_f (void);

// build an internal map name ExMx MAPxx from episode,map numbers
char* G_BuildMapName (int episode, int map);
void G_BuildTiccmd (ticcmd_t* cmd, int realtics, int which_player);

//added:22-02-98: clip the console player aiming to the view
angle_t G_ClipAimingPitch(angle_t aiming);

extern angle_t localangle,localangle2;
extern angle_t localaiming,localaiming2; // should be a angle_t but signed

extern int extramovefactor;		// Extra speed to move at


// ---- Player Spawn
void G_DoReborn (int playernum);
boolean G_DeathMatchSpawnPlayer (int playernum);
void G_CoopSpawnPlayer (int playernum);
void G_PlayerReborn (int player);

void G_AddPlayer( int playernum );

#ifdef DOGS
extern byte  extra_dog_count;
boolean  G_SpawnExtraDog( mapthing_t * spot );
void  G_KillDog( mobj_t * mo );
#endif

// ---- Game load

void G_InitNew (skill_e skill, const char* mapname, boolean resetplayer);

// Can be called by the startup code or M_Responder.
// A normal game starts at map 1,
// but a warp test can start elsewhere
void G_DeferedInitNew (skill_e skill, const char* mapname, boolean StartSplitScreenGame);
void G_DoLoadLevel (boolean resetplayer);

void G_DeferedPlayDemo (const char* demo);

// [WDJ] Set the gamemode, and all EN_ that are dependent upon it.
void G_set_gamemode( byte new_gamemode );
boolean G_Downgrade(int version);
void G_setup_VERSION( void );

// --- Save Games

// Can be called by the startup code or M_Responder,
// calls P_SetupLevel or W_EnterWorld.
#ifdef SAVEGAMEDIR
//void G_Load_Game ( const char * savegamedir, int slot );
#endif
void G_Load_Game (int slot);
void G_DoLoadGame (int slot);

// Called by M_Responder.
void G_Save_Game  (int slot, const char* description);
void G_DoSaveGame(int slot, const char* description);

extern char savegamename[MAX_WADPATH];

void G_Savegame_Name( /*OUT*/ char * namebuf, /*IN*/ int slot );

void CheckSaveGame(size_t size);

// --- Demo
// Only called by startup code.
void G_RecordDemo (const char* name);

void G_BeginRecording (void);

void G_DoPlayDemo (const char *defdemoname);
void G_TimeDemo (const char* name);
void G_DoneLevelLoad(void);
void G_StopDemo(void);
boolean G_CheckDemoStatus (void);

// --- Level Func
void G_ExitLevel (void);
void G_SecretExitLevel (void);
void G_NextLevel (void);

void G_Ticker (void);
boolean G_Responder (event_t*   ev);



// [WDJ] 8/2011 Par times can now be modified.
extern int pars[4][10];
extern int cpars[32];

#endif
