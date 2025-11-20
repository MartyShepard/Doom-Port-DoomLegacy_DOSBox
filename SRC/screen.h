// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: screen.h 905 2012-02-29 19:31:37Z wesleyjohnson $
//
// Copyright (C) 1998-2000 by DooM Legacy Team.
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
// $Log: screen.h,v $
// Revision 1.14  2004/05/16 20:34:47  hurdler
// remove unused stuff
//
// Revision 1.13  2004/05/16 19:11:44  hurdler
// that should fix issues some people were having in 1280x1024 mode (and now support up to 1600x1200)
//
// Revision 1.12  2003/05/30 22:44:08  hurdler
// add checkcvar function to FS
//
// Revision 1.11  2003/05/04 04:16:25  sburke
// Allow 1280x1024 for full-screen video on Solaris.
//
// Revision 1.10  2002/11/12 00:06:05  ssntails
// Support for translated translucent columns in software mode.
//
// Revision 1.9  2001/05/16 21:21:14  bpereira
// no message
//
// Revision 1.8  2001/04/17 22:26:07  calumr
// Initial Mac add
//
// Revision 1.7  2001/03/13 22:14:20  stroggonmeth
// Long time no commit. 3D floors, FraggleScript, portals, ect.
//
// Revision 1.6  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.5  2000/11/02 19:49:37  bpereira
// no message
//
// Revision 1.4  2000/08/31 14:30:56  bpereira
// no message
//
// Revision 1.3  2000/04/22 20:27:35  metzgermeister
// support for immediate fullscreen switching
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------


#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "command.h"

#if defined( __DJGPP__ )
#define NUMSCREENS    4
#endif

// Size of statusbar.
#define ST_HEIGHT    32
#define ST_WIDTH     320
// heretic one's
#define SBARHEIGHT      42                      // status bar height at bottom of screen


extern int stbarheight;     // when scaled

//added:20-01-98: used now as a maximum video mode size for extra vesa modes.

// we try to re-allocate a minimum of buffers for stability of the memory,
// so all the small-enough tables based on screen size, are allocated once
// and for all at the maximum size.
#define MAXVIDWIDTH    3840
#define MAXVIDHEIGHT   2160

#define BASEVIDWIDTH    320   //NEVER CHANGE THIS! this is the original
#define BASEVIDHEIGHT   200  // resolution of the graphics.


// WDJ 2012-2-6, provide structure to complete the draw capability
typedef enum {
  DRAW8PAL, DRAW15, DRAW16, DRAW24, DRAW32
} drawmode_t;


// WDJ 2012-2-6, Provide structure and isolation of driver problems.
// Mac has padded video buffers, some drivers can provide direct access and
// some cannot, some have 15, 24, or 32 bpp drawing.
// [WDJ] do not need union, minimal wastage, do not make access difficult
// global video state
typedef struct viddef_s
{
    // If hardware allows direct access then display could be video memory,
    // but usually is in buffer allocation.  Decided by driver.
    // Horz. scanlines could be padded (Mac).
    // Screens have same size as display, and same padding.
    byte      * display;         // buffer that gets displayed, screen[0]
    byte      * screen1;         // allocated memory screen[1]

 // Display and screen drawing with correct padding
 // Sometimes is not same as direct_size
 // Use these, they are corrected for padded scanlines and multibyte pixels
    unsigned int widthbytes;     // width * bytepp, to save multiplies
    unsigned int ybytes;         // addr line = & display[ y * ybytes ]
    unsigned int screen_size;    // screens and display
 // basic video mode attributes
 // [WDJ] signed width, height for easy math, (draw errors if unsigned)
    int         width;          // PIXELS per scanline
    int         height;
    int         bytepp;          // BYTES per pixel: 1=256color, 2, 4
    int         modenum;         // vidmode num indexes videomodes list
    byte        drawmode;        // drawing mode, optimized for tables and switch stmts
    byte        bitpp;		 // BITS per pixel: 8, 15, 16, 24, 32
    byte        numpages;        // always 1, PAGE FLIPPING TODO!!!
//    byte        windowed;        // windowed or fullscreen mode ?
    byte        fullscreen;      // windowed or fullscreen mode ?
    byte        recalc;          // if true, recalc vid-based stuff
 // special uses
    int         dupx,dupy;       // scale 1,2,3 value for menus & overlays
    float       fdupx,fdupy;     // same as dupx,dupy but exact value when aspect ratio isn't 320/200
    int         centerofs;       // centering for the scaled menu gfx

    //int         baseratio;       // SoM: Used to get the correct value for lighting walls //Hurdler: not used anymore
 // PRIVATE TO DRIVER
    byte        *buffer;         // allocated invisible buffers
    byte        *direct;         // linear frame buffer, or vga base mem.
    unsigned int direct_rowbytes; // bytes per scanline of the VIDEO mode
    unsigned int direct_size;    // correct size for copy (up to 6400x4800x64)
 // END PRIVATE
} viddef_t;

// [WDJ] Do not hide what these are, it makes it more difficult to track effects.
//#define VIDWIDTH    vid.width
//#define VIDHEIGHT   vid.height


// internal additional info for vesa modes only
typedef struct {
    int         vesamode;         // vesa mode number plus LINEAR_MODE bit
    void        *plinearmem;      // linear address of start of frame buffer
} vesa_extra_t;
// a video modes from the video modes list,
// note: video mode 0 is always standard VGA320x200.
typedef struct vmode_s {

    struct vmode_s  *pnext;
    char         *name;
    unsigned int width;
    unsigned int height;
    unsigned int rowbytes;          //bytes per scanline
    unsigned int bytesperpixel;     // 1 for 256c, 2 for highcolor
    int          windowed;          // if true this is a windowed mode
    int          numpages;
    vesa_extra_t *pextradata;       //vesa mode extra data
    int          (*setmode)(viddef_t *lvid, struct vmode_s *pcurrentmode);
    int          misc;              //misc for display driver (r_glide.dll etc)
} vmode_t;


// [WDJ] definitions for 15bpp, 16bpp, 24bpp, 32bpp

typedef struct {
   byte r, g, b;
} pixel24_t;

typedef struct {
   byte alpha, r, g, b;
} pixel32_t;

typedef union {
   uint32_t      uint32;
   struct {
      byte unused;
      pixel24_t  pix24;
   } s24;
   pixel32_t     pix32;
} pixelunion32_t;

// hicolor masks  15 bit / 16 bit
extern uint16_t mask_01111, mask_01110, mask_11110, mask_11100, mask_11000;

// ---------------------------------------------
// color mode dependent drawer function pointers
// ---------------------------------------------

extern void     (*skycolfunc) (void);
extern void     (*colfunc) (void);
#ifdef HORIZONTALDRAW
extern void     (*hcolfunc) (void);    //Fab 17-06-98
#endif
extern void     (*basecolfunc) (void);
extern void     (*fuzzcolfunc) (void);  // fuzzy
extern void     (*skincolfunc) (void);  // skin translated
extern void     (*transcolfunc) (void);  // translucent
extern void     (*skintranscolfunc) (void); // SSNTails 11-11-2002
extern void     (*shadecolfunc) (void);
extern void     (*fogcolfunc) (void);
extern void     (*spanfunc) (void);
extern void     (*basespanfunc) (void);
extern void     (*fogspanfunc) (void);
extern void     (*transspanfunc) (void);


// ----------------
// screen variables
// ----------------
extern viddef_t vid;
extern int    setmodeneeded;   // mode number to set (+1), 0=NOP

extern byte*  scr_borderflat;  // flat used to fill the view borders

extern consvar_t cv_scr_width;
extern consvar_t cv_scr_height;
extern consvar_t cv_scr_depth;
extern consvar_t cv_fullscreen;

extern consvar_t cv_fuzzymode;

// quick fix for tall/short skies, depending on bytesperpixel
extern void (*skydrawerfunc[2]) (void);

// from vid_vesa.c : user config video mode decided at VID_Init ();
extern int      vid_modenum;

// Change video mode, only at the start of a refresh.
void SCR_SetMode (void);
// Recalc screen size dependent stuff
void SCR_Recalc (void);
// Check parms once at startup
void SCR_CheckDefaultMode (void);
// Set the mode number which is saved in the config
void SCR_SetDefaultMode (void);

void SCR_Startup (void);

#if !defined( __DJGPP__ )	
void SCR_ChangeFullscreen (void);
#endif
#endif //__SCREEN_H__
