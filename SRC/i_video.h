// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: i_video.h 1069 2013-12-14 00:26:30Z wesleyjohnson $
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
// Revision 1.5  2001/06/10 21:16:01  bpereira
// Revision 1.4  2001/02/24 13:35:20  bpereira
// Revision 1.3  2000/11/02 19:49:35  bpereira
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      System specific video interface stuff.
//
//-----------------------------------------------------------------------------

#ifndef I_VIDEO_H
#define I_VIDEO_H


#include "doomtype.h"
#include "command.h"
  // consvar_t

#ifdef __GNUG__
#pragma interface
#endif

typedef enum {
    render_soft   = 1,
    render_glide  = 2,
    render_d3d    = 3,
    render_opengl = 4, //Hurdler: the same for render_minigl
    render_none   = 5  // for dedicated server
} rendermode_e;

extern rendermode_e    rendermode;

// Structure for passing modenums with the necessary context
typedef enum {
    MODE_NOP,
    MODE_window,
    MODE_fullscreen,
    MODE_voodoo,
    MODE_either,  // window or fullscreen
    MODE_other
} modetype_e;

extern const char * modetype_string[ MODE_other + 1 ];

typedef struct {
    byte  modetype;  // from modetype_e
    byte  index;
} modenum_t;


extern boolean  allow_fullscreen;  // controlled by i_video
extern boolean  mode_fullscreen;   // can window before going to cv_fullscreen
#if defined( __DJGPP__ )
boolean    highcolor; // local
int        BitsColor; // Definiert die Bit Palette
extern void vsync(void);        //Allegro
extern void _vsync_in(void);    //Allegro
extern void _vsync_out_h(void); //Allegro
#endif
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

// Initial windowed graphics
void I_StartupGraphics (void);
// Full game playing graphics, with hardware drawing, and options
void I_RequestFullGraphics( byte select_fullscreen );
// Restore original video mode.
void I_ShutdownGraphics(void);

// Takes full 8 bit values.
void I_SetPalette (RGBA_t* palette);

#ifdef MACOS_DI
// in macos directory
void macConfigureInput(void);

void VID_Pause(int pause);
#endif

typedef struct {
   uint16_t  first;
   uint16_t  last;
} range_t;

// modetype is of modetype_e
range_t  VID_ModeRange( byte modetype );
char  *  VID_GetModeName(modenum_t modenum);

// rmodetype is of modetype_e
// Returns MODE_NOP when none found
modenum_t  VID_GetModeForSize( int rw, int rh, byte rmodetype );

//  By setting setmodeneeded to a value > 0,
//  the video mode change is delayed until the start of the next refresh
//
//  Set the video mode right now.
//  Returns FAIL_end, FAIL_create, of status_return_e, 1 on success;
int  VID_SetMode( modenum_t modenum );

void I_UpdateNoBlit (void);
void I_FinishUpdate (void);

void I_ReadScreen (byte* scr);

#if defined(WIN_NATIVE) || defined(OS2_NATIVE) 
// printf to loading screen
void I_LoadingScreen ( const char * msg );
#endif

// debug in color
// unused
void IO_Color( unsigned char color, unsigned char r, unsigned char g, unsigned char b );

#endif
