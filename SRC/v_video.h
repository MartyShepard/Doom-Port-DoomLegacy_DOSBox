// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: v_video.h 1201 2015-12-26 19:21:12Z wesleyjohnson $
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
// $Log: v_video.h,v $
// Revision 1.11  2003/08/11 13:50:00  hurdler
// go final + translucent HUD + fix spawn in net game
//
// Revision 1.10  2001/05/16 21:21:15  bpereira
// no message
//
// Revision 1.9  2001/04/17 22:26:07  calumr
// Initial Mac add
//
// Revision 1.8  2001/04/01 17:35:07  bpereira
// Revision 1.7  2001/02/24 13:35:21  bpereira
//
// Revision 1.6  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.5  2000/11/02 19:49:37  bpereira
// Revision 1.4  2000/08/31 14:30:56  bpereira
// Revision 1.3  2000/03/29 20:10:50  hurdler
// Revision 1.2  2000/02/27 00:42:11  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Gamma correction LUT.
//      Functions to draw patches (by post) directly to screen.
//      Functions to blit a block to the screen.
//
//-----------------------------------------------------------------------------


#ifndef V_VIDEO_H
#define V_VIDEO_H

#include "doomdef.h"
#include "doomtype.h"
#include "r_defs.h"
  // patch_t
#include "command.h"
  // consvar_t


//
// VIDEO
//

// Screen 0 is the screen updated by I_Update screen, (= vid.display).
// Screen 1 is an extra buffer, background, status bar.
// Screen 2,3 are used for wipes.

#define NUMSCREENS    4
// someone stuck in an extra screen ptr
extern  byte*   screens[NUMSCREENS+1];

#ifdef DIRTY_RECT
extern  int     dirtybox[4];
#endif

extern  consvar_t cv_ticrate;
extern  consvar_t cv_darkback;
extern  consvar_t cv_usegamma;
// Gamma Funcs
extern  consvar_t cv_gammafunc;
extern  consvar_t cv_black;	// input to gammafunc
extern  consvar_t cv_bright;	// input to gammafunc

// Early setup of video controls, register cv_ vars
void V_Init_VideoControl( void );

// Allocates buffer screens, call before R_Init.
// Must be called after every video Init and SetMode
void V_Setup_VideoDraw (void);

// Set the current RGB palette lookup to use for palettized graphics
void V_SetPalette( int palettenum );

void V_SetPaletteLump( char *pal );

extern RGBA_t  *pLocalPalette;

// Retrieve the ARGB value from a palette color index
#define V_GetColor(color)  (pLocalPalette[color&0xFF])

// Use the built-in font (font1), for Launch (before fonts are loaded)
extern byte  use_font1;
extern byte  ci_black, ci_white, ci_green, ci_grey;

// Font info for spacing lines and text on menus
typedef struct {
   int width, height;
   int xinc, yinc;
} fontinfo_t;

fontinfo_t * V_FontInfo( void );

// position and width is in src pixels
void V_CopyRect ( int srcx, int srcy, int srcscrn,
                  int width, int height,
                  int destx, int desty, int destscrn );


// V_Drawinfo flags (not uniformly supported by all functions (see src))
typedef enum {
  V_SCREENMASK =         0x0000FF,
  V_CENTERHORZ =         0x000100,   // center horizontally the whole screen
  V_CENTERMENU =         0x000200,   // menu centering, vert and horz.
  V_NOSCALESTART =       0x010000,   // dont scale x,y, start coords
                                     // console, statusbar, crosshair
  V_NOSCALEPATCH =       0x020000,   // don't scale patch
                                     // console, chat, statusbar
  V_SCALESTART =         0x040000,   // scale x,y, start coords
  V_SCALEPATCH =         0x080000,   // scale patch
// effects
  V_EFFECTMASK =        0xFF00000,
  V_WHITEMAP =           0x100000,   // draw white (for v_drawstring)
  V_FLIPPEDPATCH =       0x200000,   // flipped in y
                                     // finale
  V_TRANSLUCENTPATCH =   0x400000,   // draw patch translucent
                                     // statusbar
} drawflags_e;

typedef struct {
// drawing
    byte * drawp;  // screen buffer, with centering offsets
    byte * screen_start;  // screen buffer [0]
    unsigned int  start_offset; // offset, centering
    unsigned int  ybytes;   // dupy * ybytes, bytes per source line
    unsigned int  xbytes;   // dupx * bytepp, bytes per source pixel
    unsigned int  y0bytes;   // bytes per source line per SCALESTART
    unsigned int  x0bytes;   // bytes per source pixel per SCALESTART
    byte dupx, dupy; // dup pixels for some screen sizes
    byte dupx0;      // dup for x0, per SCALESTART
    fixed_t  y_unitfrac, x_unitfrac;
    byte  screen;
// externally setable
    uint32_t  effectflags;  // special effects
        // V_WHITEMAP, V_FLIPPEDPATCH, V_TRANSLUCENTPATCH
// save/restore
    uint32_t  screen_effectflags;  // special effects from screenflags
        // can restore by effectsflags = screen_effectsflags
    uint32_t  screenflags;   // screen and drawflags_e set by screenflags
    uint32_t  prev_screenflags;  // for restore
        // can restore by V_SetupDrawinfo( prev_screenflags );
} drawinfo_t;

// current draw info
extern drawinfo_t  drawinfo;

// setup for draw screen, scaled, and centering
// Can use saved V_drawinfo.screenflags or use V_drawinfo.prev_screenflags
void V_SetupDraw( uint32_t screenflags );  // screen + drawflags_e


//added:03-02-98:like V_DrawPatch, + using a colormap.
void V_DrawMappedPatch ( int x, int y,
                         patch_t*      patch,
                         byte*         colormap );

// with temp patch load to cache
void V_DrawMappedPatch_Name ( int x, int y,
                              char*         name,
                              byte*         colormap );

//added:05-02-98:V_DrawPatch scaled 2,3,4 times size and position.
// default params : scale patch and scale start
void V_DrawScaledPatch ( int x, int y,
                         patch_t*   patch );

// with temp patch load to cache
void V_DrawScaledPatch_Name(int x, int y, char * name );
void V_DrawScaledPatch_Num(int x, int y, int patch_num );

//added:16-02-98: like V_DrawScaledPatch, plus translucency
void V_DrawTranslucentPatch ( int x, int y,
                              patch_t*  patch );


void V_DrawPatch ( int x, int y,
                   int       scrn,
                   patch_t*  patch);

#if 0
// [WDJ] Replaced by VID_BlitLinearScreen and V_CopyRect because
// were being used to copy screens

// Draw a linear block of pixels into the view buffer.
// src: is not a screen
void V_DrawBlock ( int x, int y,
                   int    scrn,
                   int width, int height,
                   byte*  src );

// Reads a linear block of pixels into the view buffer.
// dest: is not a screen
void V_GetBlock ( int x, int y,
                  int    scrn,
                  int width, int height,
                  byte*  dest );
#endif

// draw a pic_t, SCALED
void V_DrawScalePic_Num ( int x1, int y1, int lumpnum );

// Heretic raw pic
void V_DrawRawScreen_Num(int x, int y,
                         int lumpnum,
                         int width, int height);

#ifdef DIRTY_RECT
void V_MarkRect ( int x, int y,
                  int width, int height );
#endif

//added:05-02-98: fill a box with a single color
void V_DrawFill(int x, int y, int w, int h, byte color);
//added:06-02-98: fill a box with a flat as a pattern
void V_DrawFlatFill (int x, int y, int w, int h, int flatnum);

//added:10-02-98: fade down the screen buffer before drawing the menu over
void V_DrawFadeScreen (void);

// Fade the console background with fade alpha and green tint per cv_darkback.
//added:20-03-98: test console
//   x1, x2, y2 : affected ranges in pixels,  (always y1 = 0)
void V_DrawFadeConsBack (int x1, int x2, int y2);

//  General Draw Fade.
//   x1, x2, y2 : affected ranges in pixels,  (always y1 = 0)
//   fade_alpha : 1 (no fade) .. 255 (faded)
//   fade_index : from fadescreen_draw8, or fadecons_draw8 table
//   tint_rgba : added color tint, small color values only
void V_DrawFade(int x1, int x2, int y2,
                uint32_t fade_alpha, unsigned int fade_index,
                uint32_t tint_rgba );

//added:20-03-98: draw a single character
void V_DrawCharacter (int x, int y, byte c);

//added:05-02-98: draw a string using the hu_font
void V_DrawString (int x, int y, int option, char* string);

// Find string width from hu_font chars
int V_StringWidth (char* string);

// Find string height from hu_font chars
int V_StringHeight (char* string);

// draw text with fontB (big font)
extern int FontBBaseLump;
void V_DrawTextB(char *text, int x, int y);
void V_DrawTextBGray(char *text, int x, int y);
int V_TextBWidth(char *text);
int V_TextBHeight(char *text);

//added:12-02-98:
void V_DrawTiltView (byte *viewbuffer);

//added:05-04-98: test persp. correction !!
void V_DrawPerspView (byte *viewbuffer, int aiming);

// width is in bytes (defined by ASM routine)
void VID_BlitLinearScreen (byte *srcptr, byte *destptr, int width,
                           int height, int srcrowbytes, int destrowbytes);

// clear to black
void V_ClearDisplay( void );
  
// [WDJ] 2012-02-06 Draw functions for all bpp, bytepp, and padded lines.

// [WDJ] Common calc of the display buffer address for an x and y
byte * V_GetDrawAddr( int x, int y );

// [WDJ] Draw a palette color to a single pixel
void V_DrawPixel(byte * line, int x, byte color);

// [WDJ] Draw a palette src to a screen line
void V_DrawPixels(byte * line, int x, int count, byte* src);

void V_Draw_ticrate_graph( void );

#endif
