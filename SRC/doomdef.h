// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: doomdef.h 1070 2013-12-14 00:27:19Z wesleyjohnson $
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2013 by DooM Legacy Team.
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
// DESCRIPTION:
//      Defines to control options in the code, tune, and select debugging.
//
//-----------------------------------------------------------------------------

#ifndef DOOMDEF_H
#define DOOMDEF_H

// =========================================================================
// Compile settings, configuration, tuning, and options

// Uncheck this to compile debugging code
//#define RANGECHECK
#define PARANOIA                // do some test that never happens but maybe
#if !defined( __DJGPP__ )
#define LOGMESSAGES             // write message in log.txt (win32 and Linux only for the moment)
#ifdef LOGMESSAGES
extern FILE *logstream;
#endif
#define LOGLINELEN  80
#endif
#define SHOW_DEBUG_MESSAGES

// [WDJ] Machine speed limitations.
// Leave undefined for netplay, or make sure all machines have same setting.
//#define MACHINE_MHZ  1500

// some tests, enable or disable it
//#define HORIZONTALDRAW        // abandoned : too slow
//#define TILTVIEW              // not finished
//#define PERSPCORRECT          // not finished
#define SPLITSCREEN
#define ABSOLUTEANGLE           // work fine, soon #ifdef and old code remove
//#define CLIENTPREDICTION2     // differant methode
#define NEWLIGHT                // compute lighting with bsp (in construction)
#define FRAGGLESCRIPT           // SoM: Activate FraggleScript
#define FIXROVERBUGS // Fix some 3dfloor bugs. SSNTails 06-13-2002

// For Boom demo compatibility, spawns friction thinkers
#define FRICTIONTHINKER

// Select type of bob code
#define BOB_MOM

// [WDJ] Voodoo doll 4/30/2009
// A voodoo doll is an accident of having multiple start points for a player.
// It has been used in levels as a token to trip linedefs and create
// sequenced actions, and thus are required to play some wads, like FreeDoom.
#define VOODOO_DOLL

// [WDJ] Gives a menu item that allows adjusting the time a door waits open.
// A few of the timed doors in doom2 are near impossible to get thru in time,
// and I have to use cheats to get past that part of the game.
// This is for us old people don't have super-twitch fingers anymore, or don't
// want to repeat from save game 20 times to get past these bad spots.
#define DOORDELAY_CONTROL
  // See p_fab.c, giving it NETVAR status causes saved games to crash program.

// [WDJ] 6/22/2009  Generate gamma table using two settings,
// and a selected function.
#define GAMMA_FUNCS

// [WDJ] 3/25/2010  Savegame slots 0..99
#define SAVEGAME99
#define SAVEGAMEDIR

// [WDJ] 8/26/2011  recover DEH string memory
// Otherwise will just abandon replaced DEH/BEX strings.
// Enable if you are short on memory, or just like clean execution.
// Disable if it gives you trouble.
#define DEH_RECOVER_STRINGS

#if defined PCDOS && ! defined DEH_RECOVER_STRINGS
#define DEH_RECOVER_STRINGS
#endif

// [WDJ] 9/5/2011
// Enable to allow BEX to change SAVEGAMENAME
// This is a security risk, trojan wads could use it to corrupt arbitrary files.
//#define BEX_SAVEGAMENAME

// [WDJ] 9/2/2011  French language controls
// Put french strings inline (from d_french.h)
// #define FRENCH_INLINE

#ifdef FRENCH
#define FRENCH_INLINE
#endif

// [WDJ] 9/2/2011  BEX language controls
// Load language BEX file
//#define BEX_LANGUAGE
// Automatic loading of lang.bex file.
//#define BEX_LANG_AUTO_LOAD


// [WDJ] 2/6/2012 Drawing enables
// To save code size, can turn off some drawing bpp that you cannot use.
#define ENABLE_DRAW15
#define ENABLE_DRAW16
#ifdef PC_DOS
#define ENABLE_DRAW24
#define ENABLE_DRAW32
#endif

// [WDJ] 6/5/2012 Boom global colormap
// Optional for now
#define BOOM_GLOBAL_COLORMAP

// If IPX network code is to be included
// This may be overridden for some ports.
#define USE_IPX

// Set the initial window size (width).
// Expected sizes are 320x200, 640x480, 800x600, 1024x768.
#if !defined( __DJGPP__ )
#define INITIAL_WINDOW_WIDTH   800
#define INITIAL_WINDOW_HEIGHT  600
#else
#define INITIAL_WINDOW_WIDTH   320
#define INITIAL_WINDOW_HEIGHT  200
#endif
// [WDJ] Built-in Launcher
#if !defined( __DJGPP__ )
#define LAUNCHER
#endif
// especially for Window 7,8
#if defined( WIN32 ) && !defined( LAUNCHER )
#define LAUNCHER
#endif
// =========================================================================

// Name of local directory for config files and savegames
#ifdef LINUX
#define DEFAULTDIR1 ".doomlegacy"
#define DEFAULTDIR2 ".legacy"
#endif
#ifdef __APPLE__
#define DEFAULTDIR1 ".doomlegacy"
#define DEFAULTDIR2 ".legacy"
#endif
#ifdef PC_DOS
#define DEFAULTDIR1 "dmlegacy"
#define DEFAULTDIR2 "legacy"
#endif
#ifndef DEFAULTDIR1
#define DEFAULTDIR1 "doomlegacy"
#endif
#ifndef DEFAULTDIR2
#define DEFAULTDIR2 "legacy"
#endif

#if defined PC_DOS || defined WIN32
#define DEFWADS1  "\\doomwads"
#define DEFWADS2  "\\games\\doomwads"
#define DEFHOME   "\\legacyhome"
#else
#define DEFWADS1  "/usr/local/share/games/doomwads"
#define DEFWADS2  "/usr/local/games/doomwads"
#define DEFHOME   "/usr/local/games/legacyhome"
#endif

#if defined(__APPLE__) && defined(__MACH__)
// Use defined Mac resources (app folder)
//#define EXT_MAC_DIR_SPEC

// Legacy wad for Mac
//#define  LEGACYWADDIR  ".app"
#define  LEGACYWADDIR  "/usr/local/share/games/doomlegacy"
#else
#define  LEGACYWADDIR  "/usr/local/share/games/doomlegacy"
#endif

// =========================================================================

// The maximum number of players, multiplayer/networking.
// NOTE: it needs more than this to increase the number of players...

#define MAXPLAYERS              32      // TODO: ... more!!!
#define MAXSKINS                MAXPLAYERS
#define PLAYERSMASK             (MAXPLAYERS-1)
#define MAXPLAYERNAME           21
#define MAXTEAMS		32

// Determined by skin color tables
#define NUMSKINCOLORS           11

#define SAVESTRINGSIZE          24

// Used for many file path buffer sizes
#ifdef PC_DOS
#define MAX_WADPATH   128
#else
// was too short for network systems
#define MAX_WADPATH   256
#endif

// =========================================================================

// State updates, number of tics / second.
// NOTE: used to setup the timer rate, see I_StartupTimer().
#define OLDTICRATE       35
#define NEWTICRATERATIO   1  // try 4 for 140 fps :)
#define TICRATE         (OLDTICRATE*NEWTICRATERATIO)


#endif  // DOOMDEF_H
