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

// [WDJ] Enables for messages to various outputs
// Many choices so can be individually configured.
typedef enum {
   EMSG_text = 0x01,  // stderr
   EMSG_CONS = 0x02,
   EMSG_log  = 0x04,
   EMSG_debtst = 0x10, // subject to debug enable
   EMSG_now  = 0x20, // immediate update
   EMSG_error = 0x40,
#if defined(PC_DOS) || defined(WIN32) || defined(OS2_NATIVE)
   EMSG_warn = EMSG_text|EMSG_CONS|EMSG_log,
   EMSG_info = EMSG_text|EMSG_CONS|EMSG_log,
   EMSG_ver = EMSG_text|EMSG_CONS|EMSG_log,
   EMSG_dev = EMSG_text|EMSG_CONS|EMSG_log,
   EMSG_debug = EMSG_text|EMSG_CONS|EMSG_log|EMSG_debtst,
#else
   // Linux, Mac
   EMSG_warn = EMSG_text|EMSG_CONS|EMSG_log,
   EMSG_info = EMSG_text|EMSG_CONS|EMSG_log,
   EMSG_ver = EMSG_text|EMSG_CONS|EMSG_log,
   EMSG_dev = EMSG_text|EMSG_log,
   EMSG_debug = EMSG_text|EMSG_log|EMSG_debtst,
#endif
   EMSG_all = EMSG_text|EMSG_CONS|EMSG_log,
} EMSG_e;

extern  byte  EMSG_flags;  // EMSG_e
extern  byte  fatal_error;

// i_system.h
void  I_Error (const char *error, ...);
void  I_SoftError (const char *errmsg, ...);

// console.h
void  CONS_Printf (const char *fmt, ...);
void  CONS_Printf_va (const char *fmt, va_list ap );
// For info, debug, dev, verbose messages
// print to text, console, and logs
void  GenPrintf (byte emsgflags, const char *fmt, ...);

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
// [WDJ] For separate libs that cannot access VERSION var
// 1.45 beta1
#define DOOMLEGACY_COMPONENT_VERSION   14500

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

