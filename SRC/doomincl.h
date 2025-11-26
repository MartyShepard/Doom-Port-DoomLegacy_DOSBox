// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: doomincl.h 835 2011-05-27 00:49:51Z wesleyjohnson $
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
//      Internally used data structures for virtually everything,
//      key definitions, lots of other stuff.
//      Not used in headers.
//
//-----------------------------------------------------------------------------

#ifndef DOOMINCL_H
#define DOOMINCL_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>

#if defined( __DJGPP__ )
#include <io.h>
#endif

#ifdef PC_DOS
#include <conio.h>
#endif

#include "doomdef.h"
#include "doomtype.h"


// commonly used routines - moved here for include convenience

// i_system.h
void  I_Error (const char *error, ...);
void  I_SoftError (char *error, ...);

// console.h
void  CONS_Printf (const char *fmt, ...);
void  CONS_Printf_va (const char *fmt, va_list ap );

// m_misc.h
char  *va(char *format, ...);
char  *Z_StrDup (const char *in);

// g_game.h
extern  boolean devparm;                // development mode (-devparm)

extern  byte    verbose;   // 1, 2

// demo version when playback demo, or the current VERSION
// used to enable/disable selected features for backward compatibility
// (where possible)
extern  byte    demoversion;

// version numbering
extern const int  VERSION;
extern const int  REVISION;
extern char VERSION_BANNER[];

// =======================
// Log and Debug stuff
// =======================

// File handling stuff.
//#define DEBUGFILE
#ifdef DEBUGFILE
#define DEBFILE(msg) { if(debugfile) fputs(msg,debugfile); }
extern  FILE*           debugfile;
#else
#define DEBFILE(msg) {}
//extern  FILE*           debugfile;
#endif

#ifdef LOGMESSAGES
extern  FILE  *logstream;
#endif


#endif  /* DOOMINCL_H */

