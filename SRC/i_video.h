// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: i_video.h 901 2012-02-29 19:26:57Z wesleyjohnson $
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
// $Log: i_video.h,v $
// Revision 1.6  2001/08/20 20:40:39  metzgermeister
// *** empty log message ***
//
// Revision 1.5  2001/06/10 21:16:01  bpereira
// no message
//
// Revision 1.4  2001/02/24 13:35:20  bpereira
// no message
//
// Revision 1.3  2000/11/02 19:49:35  bpereira
// no message
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      System specific interface stuff.
//
//-----------------------------------------------------------------------------


#ifndef __I_VIDEO__
#define __I_VIDEO__


#include "doomtype.h"
#include "command.h"
  // consvar

#ifdef __GNUG__
#pragma interface
#endif

typedef enum {
    render_soft   = 1,
    render_glide  = 2,
    render_d3d    = 3,
    render_opengl = 4, //Hurdler: the same for render_minigl
    render_none   = 5  // for dedicated server
} rendermode_t;

extern rendermode_t    rendermode;

extern boolean  allow_fullscreen;  // controlled by i_video

// added for OpenGL gamma correction
extern consvar_t cv_grgammared;
extern consvar_t cv_grgammagreen;
extern consvar_t cv_grgammablue;

// wait for page flipping to end or not
extern consvar_t cv_vidwait;

extern consvar_t cv_fullscreen; // for fullscreen support

// WDJ 2012-2-8, command line request to port driver
typedef enum {
  REQ_default, REQ_native, REQ_specific, REQ_highcolor, REQ_truecolor
} reqdrawmode_t;

extern byte req_bitpp;
extern byte req_drawmode;  // reqdrawmode_t

// Return true if engine can draw the using bitpp
boolean V_CanDraw( byte bitpp );

// ---------------------------------------------

void I_StartupGraphics (void);          //setup video mode
void I_ShutdownGraphics(void);          //restore old video mode

// Takes full 8 bit values.
void I_SetPalette (RGBA_t* palette);

#ifdef __MACOS__
void macConfigureInput(void);

void VID_Pause(int pause);
#endif

int   VID_NumModes(void);
char  *VID_GetModeName(int modenum);

void I_UpdateNoBlit (void);
void I_FinishUpdate (void);

// Wait for vertical retrace or pause a bit.
void I_WaitVBL(int count);

void I_ReadScreen (byte* scr);

#endif
