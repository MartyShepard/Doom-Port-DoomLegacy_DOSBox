// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: i_system.h 754 2010-09-26 11:49:59Z smite-meister $
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2010 by DooM Legacy Team.
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
// $Log: i_system.h,v $
// Revision 1.8  2001/04/27 13:32:14  bpereira
// no message
//
// Revision 1.7  2001/02/24 13:35:20  bpereira
// no message
//
// Revision 1.6  2000/10/21 08:43:29  bpereira
// no message
//
// Revision 1.5  2000/10/02 18:25:45  bpereira
// no message
//
// Revision 1.4  2000/04/25 19:49:46  metzgermeister
// support for automatic wad search
//
// Revision 1.3  2000/04/16 18:38:07  bpereira
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
//      System specific interface stuff.
//
//-----------------------------------------------------------------------------


#ifndef __I_SYSTEM__
#define __I_SYSTEM__

#include "d_ticcmd.h"
#include "d_event.h"


extern byte graphics_started;


// system initialization
void I_SysInit(void);

// return free and total physical memory in the system
uint64_t I_GetFreeMem(uint64_t *total);

// Called by D_DoomLoop,
// returns current time in tics.
tic_t I_GetTime (void);


void I_GetEvent (void);


//
// Called by D_DoomLoop,
// called before processing any tics in a frame
// (just after displaying a frame).
// Time consuming syncronous operations
// are performed here (joystick reading).
// Can call D_PostEvent.
//
void I_StartFrame (void);


//
// Called by D_DoomLoop,
// called before processing each tic in a frame.
// Quick syncronous operations are performed here.
// Can call D_PostEvent.
void I_OsPolling (void);

// Asynchronous interrupt functions should maintain private queues
// that are read by the synchronous functions
// to be converted into events.

// Either returns a null ticcmd,
// or calls a loadable driver to build it.
// This ticcmd will then be modified by the gameloop
// for normal input.
ticcmd_t* I_BaseTiccmd (void);

/// sleeps for the given amount of milliseconds
void I_Sleep(unsigned int ms);

// Called by M_Responder when quit is selected, return code 0.
void I_Quit (void);

void I_Error (const char *error, ...);

void I_Tactile (int on, int off, int total);

//added:18-02-98: write a message to stderr (use before I_Quit)
//                for when you need to quit with a msg, but need
//                the return code 0 of I_Quit();
void I_OutputMsg (char *error, ...);


/* list of functions to call at program cleanup */
void I_AddExitFunc (void (*func)(void));
void I_RemoveExitFunc (void (*func)(void));

// Setup signal handler, plus stuff for trapping errors and cleanly exit.
int  I_StartupSystem (void);
void I_ShutdownSystem (void);

uint64_t I_GetDiskFreeSpace(void);
char *I_GetUserName(void);
int  I_mkdir(const char *dirname, int unixright);

void I_StartupMouse(void);
void I_StartupMouse2(void);
void doUngrabMouse(void);


#if defined( __DJGPP__ )
// Called by DoomMain.
void I_InitJoystick (void);
extern byte sound_started;
byte* I_AllocLow (int length);
#endif
#endif
