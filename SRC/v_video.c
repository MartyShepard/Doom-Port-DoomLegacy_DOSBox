// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: v_video.c 1035 2013-08-14 00:38:40Z wesleyjohnson $
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2012 by DooM Legacy Team.
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
// $Log: v_video.c,v $
// Include: DOS DJGPP Fixes
//
// Revision 1.36  2004/07/27 08:19:37  exl
// New fmod, fs functions, bugfix or 2, patrol nodes
//
// Revision 1.35  2004/04/20 00:34:26  andyp
// Linux compilation fixes and string cleanups
//
// Revision 1.34  2003/08/11 13:50:00  hurdler
// go final + translucent HUD + fix spawn in net game
//
// Revision 1.33  2003/07/13 13:16:15  hurdler
//
// Revision 1.32  2003/06/11 04:45:17  ssntails
// High-res patch drawer added.
//
// Revision 1.31  2003/06/10 23:36:09  ssntails
// Variable flat support (32x32 to 2048x2048)
//
// Revision 1.30  2003/05/04 04:20:19  sburke
// Use SHORT macro for big-endian machines.
//
// Revision 1.29  2001/12/15 18:41:35  hurdler
// small commit, mainly splitscreen fix
//
// Revision 1.28  2001/07/28 16:18:37  bpereira
// Revision 1.27  2001/05/16 21:21:14  bpereira
// Revision 1.26  2001/04/28 14:33:41  metzgermeister
// Revision 1.25  2001/04/17 22:30:40  hurdler
// Revision 1.24  2001/04/09 20:20:46  metzgermeister
// fixed crash bug
//
// Revision 1.23  2001/04/01 17:35:07  bpereira
// Revision 1.22  2001/03/30 17:12:51  bpereira
//
// Revision 1.21  2001/03/13 22:14:20  stroggonmeth
// Long time no commit. 3D floors, FraggleScript, portals, ect.
//
// Revision 1.20  2001/02/28 17:50:55  bpereira
// Revision 1.19  2001/02/24 13:35:21  bpereira
//
// Revision 1.18  2001/02/19 17:40:34  hurdler
// Fix a bug with "chat on" in hw mode
//
// Revision 1.17  2001/02/10 13:05:45  hurdler
//
// Revision 1.16  2001/01/31 17:14:08  hurdler
// Add cv_scalestatusbar in hardware mode
//
// Revision 1.15  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.14  2000/11/06 20:52:16  bpereira
// Revision 1.13  2000/11/04 16:23:44  bpereira
// Revision 1.12  2000/11/02 19:49:37  bpereira
//
// Revision 1.11  2000/10/04 16:19:24  hurdler
// Change all those "3dfx names" to more appropriate names
//
// Revision 1.10  2000/08/31 14:30:56  bpereira
//
// Revision 1.9  2000/04/27 17:43:19  hurdler
// colormap code in hardware mode is now the default
//
// Revision 1.8  2000/04/24 20:24:38  bpereira
//
// Revision 1.7  2000/04/24 15:10:57  hurdler
// Support colormap for text
//
// Revision 1.6  2000/04/22 21:12:15  hurdler
//
// Revision 1.5  2000/04/06 20:47:08  hurdler
// add Boris' changes for coronas in doom3.wad
//
// Revision 1.4  2000/03/29 20:10:50  hurdler
//
// Revision 1.3  2000/03/12 23:16:41  linuxcub
// Fixed definition of VID_BlitLinearScreen (Well, it now compiles under RH61)
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Gamma correction LUT stuff.
//      Functions to draw patches (by post) directly to screen.
//      Functions to blit a block to the screen.
//
//-----------------------------------------------------------------------------

#include "doomincl.h"
#include "r_local.h"
#include "v_video.h"
#include "hu_stuff.h"
#include "r_draw.h"
#include "console.h"

#include "i_video.h"    //rendermode
#include "z_zone.h"

#ifdef HWRENDER
#include "hardware/hw_glob.h"
#endif

#if defined( ENABLE_DRAW15 ) || defined( ENABLE_DRAW16 ) || defined( ENABLE_DRAW24 ) || defined( ENABLE_DRAW32 )
#define ENABLE_DRAWEXT
#endif

// [WDJ] Interfaces to port video drivers, common to all

rendermode_t    rendermode=render_soft;

byte  req_bitpp = 8;  // set by d_main checks on command line
byte  req_drawmode = REQ_default;  // reqdrawmode_t

byte  graphics_started = 0; // Is used in console.c and screen.c

// To disable fullscreen at startup; is set in VID_PrepareModeList
boolean allow_fullscreen = false;

consvar_t cv_ticrate = { "vid_ticrate", "0", 0, CV_OnOff, NULL };
// synchronize page flipping with screen refresh
// unused and for compatibility reason
consvar_t cv_vidwait = {"vid_wait", "1", CV_SAVE, CV_OnOff};

// Each screen is vid.screen_size (which may be larger than width * height)
// width*height is wrong for the Mac, which pads buffer to power of 2
// someone stuck in an extra screen ptr
byte *screens[NUMSCREENS+1];


void CV_usegamma_OnChange();

#ifdef GAMMA_FUNCS
void CV_gammafunc_OnChange();
// In m_menu.c
void MenuGammaFunc_dependencies( byte gamma_en,
				 byte black_en, byte bright_en );

CV_PossibleValue_t gamma_func_t[] = {
   {0,"Gamma"},
   {1,"Gamma_black"},
#if !defined( __DJGPP__ )
   {2,"Gamma_bright_black"},
#else
   {2,"Bright Black"},
#endif
   {3,"Linear"},
   {0,NULL} };
consvar_t cv_gammafunc = { "gammafunc", "0", CV_SAVE | CV_CALL, gamma_func_t, CV_gammafunc_OnChange };
CV_PossibleValue_t gamma_bl_cons_t[] = { {-12, "MIN"}, {12, "MAX"}, {0, NULL} };
consvar_t cv_black = { "black", "0", CV_SAVE | CV_CALL, gamma_bl_cons_t, CV_usegamma_OnChange };
CV_PossibleValue_t gamma_br_cons_t[] = { {-12, "MIN"}, {12, "MAX"}, {0, NULL} };
consvar_t cv_bright = { "bright", "0", CV_SAVE | CV_CALL, gamma_br_cons_t, CV_usegamma_OnChange };
CV_PossibleValue_t gamma_cons_t[] = { {-12, "MIN"}, {12, "MAX"}, {0, NULL} };
consvar_t cv_usegamma = { "gamma", "0", CV_SAVE | CV_CALL, gamma_cons_t, CV_usegamma_OnChange };
#else
CV_PossibleValue_t gamma_cons_t[] = { {0, "MIN"}, {4, "MAX"}, {0, NULL} };
consvar_t cv_usegamma = { "gamma", "0", CV_SAVE | CV_CALL, gamma_cons_t, CV_usegamma_OnChange };
#endif

static byte gammatable[256];	// shared by all gamma table generators

/// Build a gamma table
static void R_BuildGammaTable(float gamma)
{
  int i;

  // calculate gammatable anew each time
  for (i=0; i<256; i++)
#ifdef __USE_ISOC99
    // round is ISOC99
    gammatable[i] = round(255.0*pow((i+1)/256.0, gamma));
#else
    gammatable[i] = rint(255.0*pow((i+1)/256.0, gamma));
#endif
}


#ifdef GAMMA_FUNCS

// table of gamma value for each slider position
float gamma_lookup_table[25] = {
   1.48, 1.44, 1.4, 1.36, 1.32, 1.28, 1.24, 1.2, 1.16, 1.12, 1.08, 1.04,
   1.0,		// doom gamma table 1   // at index 0
   0.96, 0.92,
   0.88,	// doom gamma table 2
   0.836, 0.793,
   0.75,	// doom gamma table 3
   0.706, 0.663,
   0.62,	// doom gamma table 4
   0.58, 0.54,
   0.50		// doom gamma table 5
};

inline float gamma_lookup( int ind )
{
   return gamma_lookup_table[ ind + 12 ];
}

static void put_gammatable( int i, float fv )
{
#ifdef __USE_ISOC99
    // roundf is ISOC99
   int gv = roundf( fv );
#else
   int gv = rint( fv );
#endif
   if( gv < 0 ) gv = 0; 
   if( gv > 255 ) gv = 255;
   gammatable[i] = gv;
}

// Generate a power law table from gamma, plus a black level offset
static void
  R_Generate_gamma_black_table( void )
{
   int i;
//   float b0 = ((float) cv_black.value ) * (16.0 / 12.0); // black
   float b0 = ((float) cv_black.value ) / 2.0; // black
   float pow_max = 255.0 - b0;
   float gam = gamma_lookup( cv_usegamma.value );  // gamma
   
   gammatable[0] = 0;	// absolute black

   for( i=1; i<=255; i++ ) {
      float fi = ((float) i) / 255.0;
      put_gammatable( i, b0 + (powf( fi, gam ) * pow_max) );
   }
}

#if 0
// Generate a power curve table from gamma,
// with a power curve black level adjustment
static void
  R_Generate_gamma_black_adj_table( void )
{
   // limits of black adjustment
#  define BLACK_SIZE  48
   int i, gv;
   float gvf;
   float gam = gamma_lookup( cv_usegamma.value );  // gamma
   float blkgam = gamma_lookup( cv_black.value ); // black
   
   gammatable[0] = 0;	// absolute black

   for( i=1; i<=255; i++ ) {
      float fi = ((float) i) / 255.0;
      gvf = powf( fi, gam ) * 255.0;
      if( i < BLACK_SIZE ) {
	 // Black adjustment, using a power function over the black range.
	 // At neutral, powf = i, so adj = powf - i.
	 fi = ((float) i) / BLACK_SIZE;
	 gvf += (powf( fi, blkgam ) * BLACK_SIZE) - ((float)i);
      }
      put_gammatable( i, gvf );
   }
}
#endif

// Generate a gamma with black adj, and bright adj
static void
  R_Generate_gamma_bright_black_table( void )
{
#  define BRIGHT_MIN  60
#  define BRIGHT_MID  130
   int i, di, start_index, end_index;
   float bf = ((float)cv_bright.value) * (256.0 / 6.0 / 12.0);
   float n3 = bf*bf*bf;
   float d2 = bf*bf;
   float gf, w0;
   
   R_Generate_gamma_black_table();
   
   // bright correct using curve: witch of agnesi
   // y = (d**3)/(x**2 + d**2)
   // MIN to MID
   start_index = BRIGHT_MIN;
   end_index = BRIGHT_MID;
   do {
      di = end_index - start_index;
      w0 = (n3 / ( (di*di) + d2 )) / di; 	// witch at low point / di
      for( i=start_index; i<=end_index; i++ ) {
	 di = abs(BRIGHT_MID - i);
	 gf = n3 / ( (di*di) + d2 );	// witch of agnesi
	 gf -= w0 * di; // smooth transition on tail
	 // add adjustment to table
	 put_gammatable( i, gammatable[i] + gf );
      }
      // MID to 255
      start_index = BRIGHT_MID + 1;
      end_index = 255;
   } while( i < 255 );
}

static void
  R_Generate_smooth5_linear_gamma_table( void )
{
   const int bl_index = 28;
   const int bl_ref_offset = 20; // (8 .. 28 .. 50);
   const int wl_index = 128;
   const int wl_ref_offset = 48; // (60 .. 128 .. 176);
   float bl_offset = ((float) cv_black.value ) * bl_ref_offset / 12.0;
   float wl_offset = ((float) cv_bright.value ) * wl_ref_offset / 12.0;
   float b0 = 0.0, lf = 1.0;
   int i, start_index, end_index, seg = 0;

   // monotonic checks
   if( (wl_offset + wl_index) < (bl_offset + bl_index + 5) ) {
      // enforce monotonic by altering wl
      wl_offset = bl_offset + bl_index + 5 - wl_index;
   }
   if( (wl_offset + wl_index) > 250.0 ) {
      // enforce monotonic by altering wl
      wl_offset = 250.0 - wl_index;
   }
   // eqn: bl_offset = ( b0 + (lf * bl_index))
   b0 = bl_offset * 5 / 16;
   if( b0 < 0.0 ) b0 = 0;
   gammatable[0] = 0;	// absolute black
   gammatable[1] = (b0 * 5)/16;	// near black
   gammatable[2] = (b0 * 11)/16;
   gammatable[3] = (b0 * 15)/16;

   // generate rest of table in three linear segments
   end_index = 3; // start at 4
   for( seg=0; seg<=2; seg++ ){
      start_index = end_index + 1;
      switch( seg ) {
       case 0:
         // linear from [1] to [bl_index]
	 end_index = bl_index;
	 lf = (bl_offset - b0) / bl_index;
	 break;
       case 1:
	 // linear from [bl_index+1] to [wl_index]
	 // eqn: bl_index + bl_offset = bl_index + ( b0 + (lf * bl_index))
	 // eqn: wl_index + wl_offset = wl_index + ( b0 + (lf * wl_index))
	 end_index = wl_index;
	 lf = ( wl_offset - bl_offset ) / ( wl_index - bl_index );
	 b0 = bl_offset - (lf * bl_index);
	 break;
       case 2:
	 // linear from [wl_index+1] to [255]
	 end_index = 255;
	 lf =  - wl_offset / (255 - wl_index);
	 b0 = wl_offset - (lf * wl_index);
	 break;
      }
      
      for( i=start_index; i<=end_index; i++ ) {
	 put_gammatable( i, (b0 + ( lf * i ) + i)); // linear
	 // smooth over 5 using weights 3 3 4 3 3
 	 gammatable[i-2] =
	   ((gammatable[i-4] + gammatable[i-3] + gammatable[i-1] + gammatable[i])*3
	      + gammatable[i-2]*4 ) / 16;
      }
   }
}

#endif


// local copy of the palette for V_GetColor()
RGBA_t *pLocalPalette = NULL;

// keep a copy of the palette so that we can get the RGB
// value for a color index at any time.
static void LoadPalette(char *lumpname)
{
  int i, palsize;

  i = W_GetNumForName(lumpname);
  palsize = W_LumpLength(i) / 3;
  if (pLocalPalette)
    Z_Free(pLocalPalette);

  pLocalPalette = Z_Malloc(sizeof(RGBA_t) * palsize, PU_STATIC, NULL);

  byte *pal = W_CacheLumpNum(i, PU_CACHE);
  for (i = 0; i < palsize; i++)
  {
      pLocalPalette[i].s.red = gammatable[*pal++];
      pLocalPalette[i].s.green = gammatable[*pal++];
      pLocalPalette[i].s.blue = gammatable[*pal++];
//        if( (i&0xff) == HWR_PATCHES_CHROMAKEY_COLORINDEX )
//            pLocalPalette[i].s.alpha = 0;
//        else
      pLocalPalette[i].s.alpha = 0xff;
  }
}

// -------------+
// V_SetPalette : Set the current palette to use for palettized graphics
//              : (that is, most if not all of Doom's original graphics)
// -------------+
// Called by D_Display, SCR_Startup, SCR_SetMode, SB_PaletteFlash
// Called by ST_doPaletteStuff, ST_Stop, CV_usegamma_OnChange, CV_Gammaxxx_ONChange
void V_SetPalette(int palettenum)
{
    if (!pLocalPalette)
        LoadPalette("PLAYPAL");

#ifdef HWRENDER
    if (rendermode != render_soft)
        HWR_SetPalette(&pLocalPalette[palettenum * 256]);
    else
#endif
    {
        if ( vid.bytepp > 1 )  // highcolor, truecolor
            R_Init_color8_translate(&pLocalPalette[palettenum * 256]);  // palette change
        else
            I_SetPalette(&pLocalPalette[palettenum * 256]);
    }
}

void V_SetPaletteLump(char *pal)
{
    LoadPalette(pal);
#ifdef HWRENDER
    if (rendermode != render_soft)
        HWR_SetPalette(pLocalPalette);
    else
#endif
    {
        if ( vid.bytepp > 1 )  // highcolor, truecolor
            R_Init_color8_translate(pLocalPalette);  // palette change
        else
            I_SetPalette(pLocalPalette);
    }
}

void CV_usegamma_OnChange(void)
{
#ifdef GAMMA_FUNCS
    switch( cv_gammafunc.value ){
     case 1:
        R_Generate_gamma_black_table();
        break;
     case 2:
        R_Generate_gamma_bright_black_table();
        break;
     case 3:
        R_Generate_smooth5_linear_gamma_table();
        break;
     case 0:
     default:
        R_BuildGammaTable( gamma_lookup( cv_usegamma.value));
        break;
    }
#else
    // old-style gamma levels are defined by gamma == 1-0.125*cv_usegamma.value
    R_BuildGammaTable(1.0 -0.125*cv_usegamma.value);
#endif
    // reload palette
    LoadPalette("PLAYPAL");
    V_SetPalette(0);
}

#ifdef GAMMA_FUNCS
enum{ GFU_GAMMA = 0x01, GFU_BLACK = 0x02, GFU_BRIGHT = 0x04 };
byte  gammafunc_usage[4] =
{
     GFU_GAMMA,  // gamma
     GFU_GAMMA | GFU_BLACK, // gamma_black
     GFU_GAMMA | GFU_BLACK | GFU_BRIGHT,  // gamma_bright_black
     GFU_BLACK | GFU_BRIGHT,  // Linear
};
  
void CV_gammafunc_OnChange(void)
{
    byte gu = gammafunc_usage[cv_gammafunc.value];
    MenuGammaFunc_dependencies( gu&GFU_GAMMA, gu&GFU_BLACK, gu&GFU_BRIGHT );
    CV_usegamma_OnChange();
}
#endif

// [WDJ] Init before calling port video driver
// Common init to all port video drivers
// Register video interface controls
void V_Init_VideoControl( void )
{
    CV_RegisterVar(&cv_vidwait);
    CV_RegisterVar(&cv_ticrate);
    // Needs be done for config loading
    CV_RegisterVar(&cv_usegamma);
#ifdef GAMMA_FUNCS
    CV_RegisterVar(&cv_black);
    CV_RegisterVar(&cv_bright);
    CV_RegisterVar(&cv_gammafunc);
#endif   
}


#ifdef DIRTY_RECT
// [WDJ] Only kept in case want to put game on handheld device, limited CPU.
// V_MarkRect : this used to refresh only the parts of the screen
//              that were modified since the last screen update
//              it is useless today
//
int dirtybox[4];
void V_MarkRect(int x, int y, int width, int height)
{
    M_AddToBox(dirtybox, x, y);
    M_AddToBox(dirtybox, x + width - 1, y + height - 1);
}
#endif

// [WDJ] 2012-02-06 Draw functions for all bpp, bytepp, and padded lines.

// Return true if engine can draw using bitpp
boolean V_CanDraw( byte bitpp )
{
    if( bitpp==8
#ifdef ENABLE_DRAW15
	|| (bitpp==15)
#endif
#ifdef ENABLE_DRAW16
	|| (bitpp==16)
#endif
#ifdef ENABLE_DRAW24
	|| (bitpp==24)
#endif
#ifdef ENABLE_DRAW32
	|| (bitpp==32)
#endif
       ) return 1;
    return 0;
}

// [WDJ] Common calc of the display buffer address for an x and y
byte * V_GetDrawAddr( int x, int y )
{
    return  vid.display + (y * vid.ybytes) + (x * vid.bytepp);
}

#ifdef ENABLE_DRAWEXT
// [WDJ] Draw a palette color to a single pixel
void V_DrawPixel(byte * line, int x, byte color)
{
    switch(vid.drawmode)
    {
     default:
     case DRAW8PAL:
        line[x] = color;
        break;
#if defined( ENABLE_DRAW15 ) || defined( ENABLE_DRAW16 )
     case DRAW15:
     case DRAW16:
        {
            register uint16_t * s16 = (uint16_t*) line;
            s16[x] = color8.to16[ color ];
	}
        break;
#endif
#ifdef ENABLE_DRAW24
     case DRAW24:
        {
	    pixelunion32_t c32;
	    c32.ui32 = color8.to32[ color ];
	    register pixel24_t * s24 = (pixel24_t*) line;
	    s24[x] = c32.pix24;
        }
        break;
#endif       
#ifdef ENABLE_DRAW32
     case DRAW32:
        {
            register uint32_t * s32 = (uint32_t*) line;
            s32[x] = color8.to32[ color ];
        }
        break;
#endif
    }
}
#else
#if 0
// [WDJ] Draw a palette color to a single pixel
void V_DrawPixel(byte * line, int x, byte color)
{
   line[x] = color;
}
#else
// Degenerate case when only have DRAW8PAL, and want to save calls locally
# define  V_DrawPixel( line, x, color)     (line)[(x)]=(color)
#endif
#endif

// [WDJ] Draw a palette src to a screen line
void V_DrawPixels(byte * line, int x, int count, byte* src)
{
    switch(vid.drawmode)
    {
     default:
     case DRAW8PAL:
        memcpy( &line[x], src, count );
        break;
#if defined( ENABLE_DRAW15 ) || defined( ENABLE_DRAW16 )
     case DRAW15:
     case DRAW16:
        line += x * 2;
        while(count--)
        {
            *(uint16_t*)line = color8.to16[ *(src++) ];
	    line += 2;
        }
        break;
#endif
#ifdef ENABLE_DRAW24
     case DRAW24:
        line += x * 3;  // 3 byte per pixel
        while(count--)
        {
            pixelunion32_t c32;
            c32.ui32 = color8.to32[ *(src++) ];
	    *(pixel24_t*)line = c32.pix24;
	    line += 3;
        }
        break;
#endif
#ifdef ENABLE_DRAW32
     case DRAW32:
        line += x * 4;
        while(count--)
        {
            *(uint32_t*)line = color8.to32[ *(src++) ];
	    line += 4;
        }
        break;
#endif
    }
}


//
// V_CopyRect
//
// position and width is in src pixels
// srcsrcn, destscn include V_SCALESTART flag
void V_CopyRect(int srcx, int srcy, int srcscrn, int width, int height, int destx, int desty, int destscrn)
{
    byte *src;
    byte *dest;

    // WARNING don't mix
    if ((srcscrn & V_SCALESTART) || (destscrn & V_SCALESTART))
    {
        srcx *= vid.dupx;
        srcy *= vid.dupy;
        width *= vid.dupx;
        height *= vid.dupy;
        destx *= vid.dupx;
        desty *= vid.dupy;
    }
    srcscrn &= 0xffff;
    destscrn &= 0xffff;

#ifdef RANGECHECK
    if (srcx < 0 || srcx + width > vid.width || srcy < 0 || srcy + height > vid.height || destx < 0 || destx + width > vid.width || desty < 0 || desty + height > vid.height || (unsigned) srcscrn > 4
        || (unsigned) destscrn > 4)
    {
        I_Error("Bad V_CopyRect %d %d %d %d %d %d %d %d", srcx, srcy, srcscrn, width, height, destx, desty, destscrn);
    }
#endif
#ifdef DIRTY_RECT
    V_MarkRect(destx, desty, width, height);
#endif

#ifdef DEBUG
    CONS_Printf("V_CopyRect: vidwidth %d screen[%d]=%x to screen[%d]=%x\n", vid.width, srcscrn, screens[srcscrn], destscrn, screens[destscrn]);
    CONS_Printf("..........: srcx %d srcy %d width %d height %d destx %d desty %d\n", srcx, srcy, width, height, destx, desty);
#endif

    // [WDJ] Copy screens, by line, padded, 8bpp .. 32bpp
    src = screens[srcscrn] + (srcy * vid.ybytes) + (srcx * vid.bytepp);
    dest = screens[destscrn] + (desty * vid.ybytes) + (destx * vid.bytepp);
    width *= vid.bytepp;

    for (; height > 0; height--)
    {
        memcpy(dest, src, width);
        src += vid.ybytes;
        dest += vid.ybytes;
    }
}


#if !defined(USEASM) || defined(WIN_NATIVE_PLACEHOLDER)
// --------------------------------------------------------------------------
// Copy a rectangular area from one bitmap to another (8bpp)
// srcPitch, destPitch : width of source and destination bitmaps
// --------------------------------------------------------------------------
// width is in bytes (defined by ASM routine)
void VID_BlitLinearScreen(byte * srcptr, byte * destptr, int width, int height, int srcrowbytes, int destrowbytes)
{
    if (srcrowbytes == destrowbytes && width == vid.widthbytes)
        memcpy(destptr, srcptr, srcrowbytes * height);
    else
    {
        while (height--)
        {
            memcpy(destptr, srcptr, width);

            destptr += destrowbytes;
            srcptr += srcrowbytes;
        }
    }
}
#endif

// [WDJ] parameterized draw, used by V_DrawScaled, V_DrawMapped
drawinfo_t  V_drawinfo;
  
// [WDJ] setup V_drawinfo for window, scaling and flag options
// usage:
//  desttop = V_drawinfo.drawp + (y * V_drawinfo.y0bytes) + (x * V_drawinfo.x0bytes);
//  destend = desttop + (patch->width * V_drawinfo.xbytes);  // test against desttop
void V_SetupDraw( uint32_t screenflags )
{
    // save current draw
    V_drawinfo.prev_screenflags = V_drawinfo.screenflags;

    V_drawinfo.screenflags = screenflags;
    V_drawinfo.effectflags = V_drawinfo.screen_effectflags = screenflags & V_EFFECTMASK;

    if (screenflags & V_NOSCALEPATCH)
    {
        V_drawinfo.dupx = V_drawinfo.dupy = 1;  // unscaled
    }
    else
    {
        V_drawinfo.dupx = vid.dupx;   // scaled
        V_drawinfo.dupy = vid.dupy;
    }
    V_drawinfo.ybytes = V_drawinfo.dupy * vid.ybytes;  // bytes per source line
    V_drawinfo.xbytes = V_drawinfo.dupx * vid.bytepp;  // bytes per source pixel

    if (screenflags & V_NOSCALESTART)
    {
        V_drawinfo.y0bytes = vid.ybytes;  // unscaled
	V_drawinfo.x0bytes = vid.bytepp;
    }
    else
    {
        V_drawinfo.y0bytes = vid.dupy * vid.ybytes;  // scaled
	V_drawinfo.x0bytes = vid.dupx * vid.bytepp;
    }

    V_drawinfo.x_unitfrac = FixedDiv(FRACUNIT, V_drawinfo.dupx << FRACBITS);
    V_drawinfo.y_unitfrac = FixedDiv(FRACUNIT, V_drawinfo.dupy << FRACBITS);

    // The screen buffer, at an offset
    V_drawinfo.start_offset = 0;
    if (screenflags & V_CENTERSCREEN)
    {
        // center the menus, finale, and other screens in the fullscreen
        V_drawinfo.start_offset += (vid.widthbytes - (BASEVIDWIDTH * V_drawinfo.xbytes)) / 2;
    }
    if (screenflags & V_CENTER0)
    {
        // center the graphic in the menu, (0,0) at center
        V_drawinfo.start_offset += ( vid.centerofs * vid.bytepp );
        // as previously was performed by scaleofs.
        // Enabled when the menu is displayed, and crosshairs.
	// The menu is scaled, a round multiple of the original pixels to
        // keep the graphics clean, then it is centered a little.
        // Except the menu, scaled graphics don't have to be centered.
    }
    V_drawinfo.screen = screenflags & V_SCREENMASK;  // screen number (usually 0)
    V_drawinfo.drawp = screens[V_drawinfo.screen] + V_drawinfo.start_offset;
}


//
//  V_DrawMappedPatch : like V_DrawScaledPatch, but with a colormap.
//  per V_drawinfo
//
//
//added:05-02-98:
// [WDJ] all patches are cached endian fixed 1/5/2010
// Called by draw char/string, menu, wi_stuff (screen0, scaled)
// Called by ST_refreshBackground to draw face on status bar (with flags)
void V_DrawMappedPatch(int x, int y, patch_t * patch, byte * colormap)
{
    column_t *column;
    byte *source;  // within column
    byte *desttop, *dest;  // within video buffer

    int count;
    fixed_t col, wf, ofs;

    // draw an hardware converted patch
#ifdef HWRENDER
    if (rendermode != render_soft)
    {
        HWR_DrawMappedPatch((MipPatch_t *) patch, x, y, V_drawinfo.screenflags|V_drawinfo.effectflags, colormap);
        return;
    }
#endif

    y -= patch->topoffset;
    x -= patch->leftoffset;

    // [WDJ] Draw to screens, by line, padded, 8bpp .. 32bpp
    desttop = V_drawinfo.drawp + (y * V_drawinfo.y0bytes) + (x * V_drawinfo.x0bytes);
//    destend = desttop + (patch->width * V_drawinfo.xbytes);  // test against desttop

#ifdef DIRTY_RECT
    if (V_drawinfo.screen == 0)
        V_MarkRect(x, y, patch->width * V_drawinfo.dupx, patch->height * V_drawinfo.dupy);
#endif

    wf = patch->width << FRACBITS;

    for (col=0; col < wf; col += V_drawinfo.x_unitfrac)
    {
        column = (column_t *) ((byte *) patch + patch->columnofs[col >> FRACBITS]);

        while (column->topdelta != 0xff)
        {
            source = (byte *) column + 3;
            dest = desttop + (column->topdelta * V_drawinfo.ybytes);
            count = column->length * V_drawinfo.dupy;

            ofs = 0;
#ifdef ENABLE_DRAWEXT
	    if(vid.drawmode != DRAW8PAL)
	    {
	        while (count--)
	        {
		    V_DrawPixel( dest, 0, colormap[ source[ofs >> FRACBITS]] );
		    dest += vid.ybytes;
		    ofs += V_drawinfo.y_unitfrac;
		}
	    }
	    else
#endif
	    {
	        // DRAW8PAL
		while (count--)
	        { 
		    *dest = colormap[ source[ofs >> FRACBITS]];
		    dest += vid.ybytes;
		    ofs += V_drawinfo.y_unitfrac;
		}
            }
            column = (column_t *) ((byte *) column + column->length + 4);
        }
        desttop += vid.bytepp;
    }

}

//  per V_drawinfo
// with temp patch load to cache
void V_DrawMappedPatch_Name ( int x, int y, char* name, byte* colormap )
{
   // The patch is used only in this function
   V_DrawMappedPatch ( x, y,
                       W_CachePatchName( name, PU_CACHE ),  // endian fix
		       colormap );
}


//
// V_DrawScaledPatch
//   like V_DrawPatch, but scaled 2,3,4 times the original size and position
//   this is used for menu and title screens, with high resolutions
//  per V_drawinfo, with V_SCALEDSTART, V_SCALEDPATCH
//
//added:05-02-98:
// default params : scale patch and scale start
// [WDJ] all patches are cached endian fixed 1/5/2010
// Called by menu, status bar, and wi_stuff
void V_DrawScaledPatch(int x, int y, patch_t * patch)
{
    int count;
    fixed_t col = 0;
    column_t *column;
    byte *source;  // within column
    byte *dest, *desttop, *destend;  // within video buffer

    fixed_t ofs;
    fixed_t colfrac;

    // draw an hardware converted patch
#ifdef HWRENDER
    if (rendermode != render_soft)
    {
        HWR_DrawPatch((MipPatch_t *) patch, x, y, V_drawinfo.screenflags|V_drawinfo.effectflags );
        return;
    }
#endif

    y -= patch->topoffset;
    x -= patch->leftoffset;

    colfrac = V_drawinfo.x_unitfrac;

    // [WDJ] Draw to screens, by line, padded, 8bpp .. 32bpp
    desttop = V_drawinfo.drawp + (y * V_drawinfo.y0bytes) + (x * V_drawinfo.x0bytes);
    destend = desttop + (patch->width * V_drawinfo.xbytes);  // test against desttop

    // only used in f_finale:F_CastDrawer
    if (V_drawinfo.effectflags & V_FLIPPEDPATCH)
    {
        colfrac = -colfrac;
        col = (patch->width << FRACBITS) + colfrac;
    }
    else
        col = 0;

    while( desttop < destend )
    {
        column = (column_t *) ((byte *) patch + patch->columnofs[col >> FRACBITS]);
        col += colfrac;

        while (column->topdelta != 0xff)
        {
            source = (byte *) column + 3;
            dest = desttop + (column->topdelta * V_drawinfo.ybytes);
            count = column->length * V_drawinfo.dupy;

            ofs = 0;
#ifdef ENABLE_DRAWEXT
	    if(vid.drawmode != DRAW8PAL)
	    {
	        while (count--)
	        {
		    V_DrawPixel( dest, 0, source[ofs >> FRACBITS] );
		    dest += vid.ybytes;
		    ofs += V_drawinfo.y_unitfrac;
		}
	    }
	    else
#endif
	    {
	        while (count--)
	        {
		    *dest = source[ofs >> FRACBITS];
		    dest += vid.ybytes;
		    ofs += V_drawinfo.y_unitfrac;
		}
	    }

            column = (column_t *) ((byte *) column + column->length + 4);
        }
        desttop += vid.bytepp;
    }
}

//  per V_drawinfo, with V_SCALEDSTART, V_SCALEDPATCH
// with temp patch load to cache
void V_DrawScaledPatch_Name(int x, int y, char * name )
{
   // The patch is used only in this function
   V_DrawScaledPatch ( x, y,
                       W_CachePatchName( name, PU_CACHE ) );  // endian fix
}

//  per V_drawinfo, with V_SCALEDSTART, V_SCALEDPATCH
// with temp patch load to cache
void V_DrawScaledPatch_Num(int x, int y, int patch_num )
{
   // The patch is used only in this function
   V_DrawScaledPatch ( x, y,
                       W_CachePatchNum( patch_num, PU_CACHE ) );  // endian fix
}

#if 0
//[WDJ] 2012-02-06 DrawSmallPatch found to be unused

void HWR_DrawSmallPatch(MipPatch_t * gpatch, int x, int y, int option, byte * colormap);
// Draws a patch 2x as small. SSNTails 06-10-2003
// [WDJ] all patches are cached endian fixed 1/5/2010
void V_DrawSmallScaledPatch(int x, int y, int scrn, patch_t * patch, byte * colormap)
{
    int count;
    int col;
    column_t *column;
    byte *source;  // within column
    byte *desttop, *dest, *destend;  // within video buffer

    int dupx=1, dupy=1;
    int count_dupy, dup_ybytes;
    int ofs;
    fixed_t colfrac, rowfrac, colfrac_inc, rowfrac_inc;
//    boolean skippixels = false;

    // draw an hardware converted patch
#ifdef HWRENDER
    if (rendermode != render_soft)
    {
        HWR_DrawSmallPatch((MipPatch_t *) patch, x, y, scrn, colormap);
        return;
    }
#endif

    y -= patch->topoffset;
    x -= patch->leftoffset;

    colfrac = FixedDiv(FRACUNIT, dupx << FRACBITS);
    rowfrac = FixedDiv(FRACUNIT, dupy << FRACBITS);

    if (scrn & V_FLIPPEDPATCH)
    {
        colfrac = -colfrac;
        col = (patch->width << FRACBITS) + colfrac;
    }
    else
        col = 0;

    colfrac_inc = colfrac;
    rowfrac_inc = rowfrac;

    desttop = screens[scrn & 0xFF] + (y * vid.ybytes) + (x * vid.bytepp);
    destend = desttop;

//    if( (scrn & V_NOSCALEPATCH) )
    if (vid.dupx > 1 && vid.dupy > 1)
    {
        destend += (patch->width * dupx * vid.bytepp);
        count_dupy = dupy << 1;  // count_dupy = dupy * 2, will be dupy after >> 1
    }
    else
    {
//        skippixels = true;
        // double the inc, halve the count
        destend += (patch->width / 2 * dupx * vid.bytepp);
        colfrac_inc += colfrac_inc;  // * 2
        rowfrac_inc += rowfrac_inc;  // * 2
        count_dupy = dupy; // will be dupy/2 after >> 1
    }
    dup_ybytes = dupy * vid.ybytes;

    // [WDJ] Use same loop for normal and skippixels, with some predefined inc
    for (  ; desttop < destend; desttop+=vid.bytepp)
    {
        column = (column_t *) ((byte *) patch + patch->columnofs[col >> FRACBITS]);
        col += colfrac_inc;
        while (column->topdelta != 0xff)
        {
	    source = (byte *) column + 3;
	    dest = desttop + (column->topdelta * dup_ybytes);
	    count = (column->length * count_dupy) >> 1;  // dupy or dupy/2
	    ofs = 0;
	    while (count--)
	    {
	        V_DrawPixel( dest, 0, colormap[source[ofs >> FRACBITS]] );
	        dest += vid.ybytes;
	        ofs += rowfrac_inc;
	    }
	    column = (column_t *) ((byte *) column + column->length + 4);
	}
    }
}
#endif

//added:16-02-98: now used for crosshair
//
//  This draws a patch over a background with translucency
//  per V_drawinfo
//
// [WDJ] all patches are cached endian fixed 1/5/2010
void V_DrawTranslucentPatch(int x, int y, patch_t * patch)
{
    int count;
    column_t *column;
    byte *source;  // within column
    byte *desttop, *dest;  // within video buffer
    fixed_t ofs;
    fixed_t col, wf;

    // draw an hardware converted patch
#ifdef HWRENDER
    if (rendermode != render_soft)
    {
        HWR_DrawPatch((MipPatch_t *) patch, x, y, V_drawinfo.screenflags|V_drawinfo.effectflags );
        return;
    }
#endif

    y -= patch->topoffset * V_drawinfo.dupy;
    x -= patch->leftoffset * V_drawinfo.dupx;

#ifdef DIRTY_RECT
    if (!(scrn & 0xff))
        V_MarkRect(x, y, patch->width * V_drawinfo.dupx, patch->height * V_drawinfo.dupy);
#endif

    // [WDJ] Draw to screens, by line, padded, 8bpp .. 32bpp
    desttop = V_drawinfo.drawp + (y * V_drawinfo.y0bytes) + (x * V_drawinfo.x0bytes);
//    destend = desttop + (patch->width * V_drawinfo.xbytes);  // test against desttop

    wf = patch->width << FRACBITS;

    for ( col=0; col < wf; col += V_drawinfo.x_unitfrac)
    {
        column = (column_t *) ((byte *) patch + patch->columnofs[col >> FRACBITS]);

        while (column->topdelta != 0xff)
        {
            source = (byte *) column + 3;
            dest = desttop + (column->topdelta * V_drawinfo.ybytes);
            count = column->length * V_drawinfo.dupy;

            ofs = 0;
#ifdef ENABLE_DRAWEXT
	    switch(vid.drawmode)
	    {
	     default:
	     case DRAW8PAL:
	        while (count--)
	        {
		    register unsigned int color = source[ofs >> FRACBITS];
		    *dest = translucenttables[ ((color << 8) & 0xFF00) + (*dest & 0xFF) ];
		    dest += vid.ybytes;
		    ofs += V_drawinfo.y_unitfrac;
		}
	        break;
#if defined( ENABLE_DRAW15 ) || defined( ENABLE_DRAW16 )
	     case DRAW15:
	     case DRAW16:
	        while (count--)
	        {
		    register unsigned int color = source[ofs >> FRACBITS];
		    register uint16_t * s16 = (uint16_t*) dest;
		    *s16 =( ((color8.to16[color]>>1) & mask_01111) +
			    (((*s16)>>1) & mask_01111) );
		    dest += vid.ybytes;
		    ofs += V_drawinfo.y_unitfrac;
		}
	        break;
#endif
#ifdef ENABLE_DRAW24
	     case DRAW24:
	        while (count--)
	        {
		    register unsigned int color = source[ofs >> FRACBITS];
		    pixelunion32_t c32;
		    c32.ui32 = (color8.to32[ color ]>>1) & 0x7F7F7F; // 01111111 on pix24
		    register pixel24_t * s24 = (pixel24_t*) dest;
		    s24->r = c32.pix24.r + (s24->r>>1);
		    s24->g = c32.pix24.g + (s24->g>>1);
		    s24->b = c32.pix24.b + (s24->b>>1);
		    dest += vid.ybytes;
		    ofs += V_drawinfo.y_unitfrac;
		}
	        break;
#endif
#ifdef ENABLE_DRAW32
	     case DRAW32:
	        while (count--)
	        {
		    register unsigned int color = source[ofs >> FRACBITS];
		    register uint32_t * s32 = (uint32_t*) dest;
		    *s32 = ((color8.to32[ color ]>>1) & 0x007F7F7F)
		         + (((*s32)>>1) & 0x007F7F7F) + (*s32 & 0xFF000000);
		    dest += vid.ybytes;
		    ofs += V_drawinfo.y_unitfrac;
		}
	        break;
#endif
	    }
#else
	    // Degenerate DRAW8PAL only
	    while (count--)
	    {
	        register unsigned int color = source[ofs >> FRACBITS];
	        *dest = translucenttables[ ((color << 8) & 0xFF00) + (*dest & 0xFF) ];
	        dest += vid.ybytes;
	        ofs += V_drawinfo.y_unitfrac;
	    }
#endif

            column = (column_t *) ((byte *) column + column->length + 4);
        }
        desttop += vid.bytepp;
    }
}

//
// V_DrawPatch
// Masks a column based masked pic to the screen. NO SCALING!!!
//
// [WDJ] all patches are cached endian fixed 1/5/2010
// Called by R_FillBackScreen, map
void V_DrawPatch(int x, int y, int scrn, patch_t * patch)
{

    column_t *column;
    byte *source;  // within column
    byte *desttop, *dest;  // within video buffer
    int count;
    int col, wi;

    // draw an hardware converted patch
#ifdef HWRENDER
    if (rendermode != render_soft)
    {
        HWR_DrawPatch((MipPatch_t *) patch, x, y, V_NOSCALESTART | V_NOSCALEPATCH);
        return;
    }
#endif

    y -= patch->topoffset;
    x -= patch->leftoffset;
#ifdef RANGECHECK
    if (x < 0 || x + patch->width > vid.width || y < 0 || y + patch->height > vid.height || (unsigned) scrn > 4)
    {
        fprintf(stderr, "Patch at %d,%d exceeds LFB\n", x, y);
        // No I_Error abort - what is up with TNT.WAD?
        fprintf(stderr, "V_DrawPatch: bad patch (ignored)\n");
        return;
    }
#endif

#ifdef DIRTY_RECT
    if (!scrn)
        V_MarkRect(x, y, patch->width, patch->height);
#endif

    desttop = screens[scrn] + (y * vid.ybytes) + (x * vid.bytepp);

    wi = patch->width;

    for ( col=0; col < wi; col++)
    {
        column = (column_t *) ((byte *) patch + patch->columnofs[col]);

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            source = (byte *) column + 3;
            dest = desttop + (column->topdelta * vid.ybytes);
            count = column->length;

            while (count--)
            {
                V_DrawPixel(dest, 0, *source++);
                dest += vid.ybytes;
            }
            column = (column_t *) ((byte *) column + column->length + 4);
        }
        desttop += vid.bytepp;
    }
}


#if 0
// [WDJ] Replaced by VID_BlitLinearScreen and V_CopyRect because
// were being used to copy screens
//
// V_DrawBlock
// Draw a linear block of pixels into the view buffer.
//
// src: is not a screen
// dest: scrn is a screen, x,y in pixel coord
void V_DrawBlock(int x, int y, int scrn, int width, int height, byte * src)
{
    byte *dest;  // within video buffer

#ifdef RANGECHECK
    if (x < 0 || x + width > vid.width || y < 0 || y + height > vid.height || (unsigned) scrn > 4)
    {
        I_Error("Bad V_DrawBlock");
    }
#endif

#ifdef DIRTY_RECT
    //V_MarkRect (x, y, width, height);
#endif

    // [WDJ] Copy screens, by line, padded, 8bpp .. 32bpp
    width *= vid.bytepp;
    dest = screens[scrn] + (y * vid.ybytes) + (x * vid.bytepp);

    while (height--)
    {
        memcpy(dest, src, width);

        src += width;
        dest += vid.ybytes;
    }
}

//
// V_GetBlock
// Gets a linear block of pixels from the view buffer.
//
// src: scrn is a screen, x,y in pixel coord
// dest: is not a screen
void V_GetBlock(int x, int y, int scrn, int width, int height, byte * dest)
{
    byte *src;  // within video buffer

    if (rendermode != render_soft)
        I_Error("V_GetBlock: called in non-software mode");

#ifdef RANGECHECK
    if (x < 0 || x + width > vid.width || y < 0 || y + height > vid.height || (unsigned) scrn > 4)
    {
        I_Error("Bad V_GetBlock");
    }
#endif

    src = screens[scrn] + (y * vid.ybytes) + (x * vid.bytepp);

    while (height--)
    {
        memcpy(dest, src, width);
        src += vid.ybytes;
        dest += width;
    }
}
#endif



//  per V_drawinfo, scaled, abs start coord.
// [WDJ] all pic are cached endian fixed 1/5/2010
static void V_BlitScalePic(int x1, int y1, pic_t * pic)
{
    int dupx, dupy;
    int x, y;
    byte *src, *dest;
    int pic_width = pic->width;
    int pic_height = pic->height;
   
    if (pic->mode != 0)
    {
        CONS_Printf("pic mode %d not supported in Software\n", pic->mode);
        return;
    }

    // scaled, with x centering
    dest = V_drawinfo.drawp + (max(0, y1) * vid.ybytes) + (max(0, x1) * vid.bytepp);
    // y clipping to the screen
    if (y1 + (pic_height * vid.dupy) >= vid.width)
        pic_height = (vid.width - y1) / vid.dupy - 1;
    // WARNING no x clipping (not needed for the moment)

    for (y = max(0, -y1 / vid.dupy); y < pic_height; y++)
    {
        for (dupy = vid.dupy; dupy; dupy--)
        {
	    int xb = 0;
            src = pic->data + (y * pic_width);
            for (x = 0; x < pic_width; x++)
            {
                for (dupx = vid.dupx; dupx; dupx--)
		    V_DrawPixel(dest, xb++, *src);
                src++;
            }
	    dest += vid.ybytes;
        }
    }
}

//  Draw a linear pic, scaled
//  CURRENTLY USED FOR StatusBarOverlay, scale pic but not starting coords
//  per V_drawinfo, scaled, abs start coord.
//
void V_DrawScalePic_Num(int x1, int y1, int lumpnum)
{
#ifdef HWRENDER
    if (rendermode != render_soft)
    {
        HWR_DrawPic(x1, y1, lumpnum);
        return;
    }
#endif

    // [WDJ] Get pic and fix endian, then display
    V_BlitScalePic(x1, y1, W_CachePicNum(lumpnum, PU_CACHE));
}

// Heretic raw pic
//  per V_drawinfo, scaled, abs start coord.
void V_DrawRawScreen_Num(int x1, int y1, int lumpnum, int width, int height)
{
#ifdef HWRENDER
    if (rendermode != render_soft)
    {
        // save size somewhere and mark lump as a raw pic !
        MipPatch_t *grpatch = &(wadfiles[lumpnum >> 16]->hwrcache[lumpnum & 0xffff]);
        grpatch->width = width;
        grpatch->height = height;
        grpatch->mipmap.tfflags |= TF_Her_Raw_Pic;  // Heretic Raw Pic
        HWR_DrawPic(x1, y1, lumpnum);
        return;
    }
#endif

    V_BlitScalePic(x1, y1,
		   W_CacheRawAsPic(lumpnum, width, height, PU_CACHE));
}


//
//  Fills a box of pixels with a single color
//
// per V_drawinfo centering, always screen 0, V_SCALEPATCH, V_SCALESTART
//added:05-02-98:
void V_DrawFill(int x, int y, int w, int h, byte color)
{
    byte *dest;  // within screen buffer
    int u, v;

#ifdef HWRENDER
    if (rendermode != render_soft)
    {
        HWR_DrawFill(x, y, w, h, color);
        return;
    }
#endif

    dest = screens[0] + (y * vid.dupy * vid.ybytes) + (x * vid.dupx * vid.bytepp);
    dest += V_drawinfo.start_offset;

    w *= vid.dupx;
    h *= vid.dupy;

    for (v = 0; v < h; v++, dest += vid.ybytes)
        for (u = 0; u < w; u++)
            V_DrawPixel(dest, u, color);
}

//
//  Fills a box of pixels using a flat texture as a pattern,
//  scaled to screen size.
//
//added:06-02-98:
// per V_drawinfo, scaled, centering
// Called by WI_slamBackground, F_TextWrite (entire screen), M_DrawTextBox
void V_DrawFlatFill(int x, int y, int w, int h, int flatnum)
{
    byte *dest;  // within screen buffer
    int u, v;
    fixed_t dx, dy, xfrac, yfrac;
    byte *src;
    byte *flat;
    int size;
    int flatsize, flatshift;

#ifdef HWRENDER
    if (rendermode != render_soft)
    {
        HWR_DrawFlatFill(x, y, w, h, flatnum);
        return;
    }
#endif

    size = W_LumpLength(flatnum);

    switch (size)
    {
        case 4194304:  // 2048x2048 lump
            flatsize = 2048;
            flatshift = 10;
            break;
        case 1048576:  // 1024x1024 lump
            flatsize = 1024;
            flatshift = 9;
            break;
        case 262144:   // 512x512 lump
            flatsize = 512;
            flatshift = 8;
            break;
        case 65536:    // 256x256 lump
            flatsize = 256;
            flatshift = 7;
            break;
        case 16384:    // 128x128 lump
            flatsize = 128;
            flatshift = 7;
            break;
        case 1024:     // 32x32 lump
            flatsize = 32;
            flatshift = 5;
            break;
        default:       // 64x64 lump
            flatsize = 64;
            flatshift = 6;
            break;
    }

    flat = W_CacheLumpNum(flatnum, PU_CACHE);

#if 1
    // Draw per V_drawinfo
    dest = V_drawinfo.drawp + (y * V_drawinfo.y0bytes) + (x * V_drawinfo.x0bytes);
    dx = V_drawinfo.x_unitfrac;
    dy = V_drawinfo.y_unitfrac;
    w *= V_drawinfo.dupx;
    h *= V_drawinfo.dupy;
#else
    // Draw to screen0, scaled
    int dupx = vid.dupx;
    int dupy = vid.dupy;

    dest = screens[0] + (y * dupy * vid.ybytes) + (x * dupx * vid.bytepp);
    dest += V_drawinfo.start_offset;

    dx = FixedDiv(FRACUNIT, dupx << FRACBITS);
    dy = FixedDiv(FRACUNIT, dupy << FRACBITS);

    w *= dupx;
    h *= dupy;
#endif

    yfrac = 0;
    for (v = 0; v < h; v++, dest += vid.ybytes)
    {
        xfrac = 0;
        src = flat + (((yfrac >> (FRACBITS - 1)) & (flatsize - 1)) << flatshift);
        for (u = 0; u < w; u++)
        {
            V_DrawPixel(dest, u, src[(xfrac >> FRACBITS) & (flatsize - 1)]);
            xfrac += dx;
        }
        yfrac += dy;
    }
}

//
//  Fade all the screen buffer, so that the menu is more readable,
//  especially now that we use the small hufont in the menus...
//
void V_DrawFadeScreen(void)
{
    int x, y, w4;
    uint32_t *buf;  // within video buffer
    uint32_t quad;
#ifdef ENABLE_DRAWEXT
    uint32_t mask;
#endif
    byte p1, p2, p3, p4;
    byte *fadetable = (byte*) & reg_colormaps[ LIGHTTABLE(16) ];

#ifdef HWRENDER // not win32 only 19990829 by Kin
    if (rendermode != render_soft)
    {
        HWR_FadeScreenMenuBack(0x01010160, (0xff/2), 0);  //faB: hack, 0 means full height :o
        return;
    }
#endif

    w4 = (vid.widthbytes >> 2);  // 4 bytes at a time
    switch(vid.drawmode)
    {
     case DRAW8PAL:
        // 8 bpp palette fade
        for (y = 0; y < vid.height; y++)
        {
	    buf = (uint32_t *) (screens[0] + (y * vid.ybytes));
	    for (x = 0; x < w4; x++)
	    {
	        // fade four at a time
	        quad = buf[x];
	        p1 = fadetable[quad & 255];
	        p2 = fadetable[(quad >> 8) & 255];
	        p3 = fadetable[(quad >> 16) & 255];
	        p4 = fadetable[quad >> 24];
	        buf[x] = (p4 << 24) | (p3 << 16) | (p2 << 8) | p1;
	    }
	}
        break;
#ifdef ENABLE_DRAW15
     case DRAW15:
        mask = 0x3DEF3DEF;  // 0 01111 01111 01111
        goto fade_loop;
#endif
#ifdef ENABLE_DRAW16
     case DRAW16:
        mask = 0x7BEF7BEF;  // 01111 011111 01111
        goto fade_loop;
#endif
#ifdef ENABLE_DRAW24
     case DRAW24:
        mask = 0x7F7F7F7F;
        goto fade_loop;
#endif
#ifdef ENABLE_DRAW32
     case DRAW32:
        mask = 0xFF7F7F7F;  // alpha unchanged
#endif
#ifdef ENABLE_DRAWEXT
     fade_loop:
        for (y = 0; y < vid.height; y++)
        {
	    buf = (uint32_t *) (screens[0] + (y * vid.ybytes)); 
            for (x = 0; x < w4; x++)
	    {
	        *buf = (*buf >> 1) & mask;
	        buf++;
             }
	}
        break;
#endif       
     default:
        break;
    }
}


// Simple translucence with one color, coords are resolution dependent
//
//added:20-03-98: console test
void V_DrawFadeConsBack(int x1, int y1, int x2, int y2)
{
#ifdef ENABLE_DRAWEXT
    int w4 = x2 - x1;
    uint32_t mask, green_tint, alpha=0;
# define GREEN_TINT_255  0x16
#endif
    int x, y;
    byte p1, p2, p3, p4;
    uint32_t *buf;

#ifdef HWRENDER // not win32 only 19990829 by Kin
    if (rendermode != render_soft)
    {
        HWR_FadeScreenMenuBack(0x00500000, (0xff/2), y2);
        return;
    }
#endif

    switch(vid.drawmode)
    {
     case DRAW8PAL:
        // 8bpp palette, accessed 4 bytes at a time
        x1 >>= 2;
        x2 >>= 2;
        for (y = y1; y < y2; y++)
        {
            buf = (uint32_t *) (screens[0] + (y * vid.ybytes));
            for (x = x1; x < x2; x++)
            {
                uint32_t quad = buf[x];
                p1 = greenmap[quad & 255];
                p2 = greenmap[(quad >> 8) & 255];
                p3 = greenmap[(quad >> 16) & 255];
                p4 = greenmap[quad >> 24];
                buf[x] = (p4 << 24) | (p3 << 16) | (p2 << 8) | p1;
            }
        }
        break;
#ifdef ENABLE_DRAW15
     case DRAW15:
//        mask = 0x3DEF3DEF;  // 0 01111 01111 01111
        mask = 0x1CE71CE7;  // 0 00111 00111 00111
        green_tint = (GREEN_TINT_255 >> 3) * 0x00200020;  // 0 00000 00001 00000
        w4 >>= 1;
        goto fade_loop;
#endif
#ifdef ENABLE_DRAW16
     case DRAW16:
//        mask = 0x7BEF7BEF;  // 01111 011111 01111
        mask = 0x39E739E7;  // 00111 001111 00111
        green_tint = (GREEN_TINT_255 >> 2) * 0x00200020;  // 0 00000 00001 00000
        w4 >>= 1;
        goto fade_loop;
#endif
#ifdef ENABLE_DRAW32
     case DRAW32:
        // assume ARGB format
//        mask = 0x007F7F7F;  // alpha unchanged
        mask = 0x003F3F3F;  // alpha unchanged
        alpha = 0xFF000000;
        green_tint = GREEN_TINT_255 << 8;
        goto fade_loop;
#endif
#if defined( ENABLE_DRAW15 ) || defined( ENABLE_DRAW16 ) || defined( ENABLE_DRAW32 )
     fade_loop:
        for (y = y1; y < y2; y++)
        {
            buf = (uint32_t *) (screens[0] + (y * vid.ybytes) + (x1 * vid.bytepp));
            for (x = 0; x < w4; x++)
            {
                *buf = ((((*buf >> 2) & mask) + green_tint) )
		        | (*buf & alpha);
	        buf++;  // compiler complains when combined above
            }
        }
        break;
#endif       
#ifdef ENABLE_DRAW24
     case DRAW24:
        // RGB
        for (y = y1; y < y2; y++)
        {
            pixel24_t * p24 = (pixel24_t*)( screens[0] + (y * vid.ybytes) + (x1 * vid.bytepp) );
            for (x = w4; x > 0; x--)
            { 
	        p24->b >>= 2; // blue
	        p24->g = ((uint16_t)(p24->g) >> 2) + GREEN_TINT_255; // green
	        p24->r >>= 2; // red
	        p24 ++;
            }
        }
        break;
#endif
     default:
        break;
    }
}

// Writes a single character (draw WHITE if bit 7 set)
//
//added:20-03-98:
void V_DrawCharacter(int x, int y, byte c)
{
    int w;
    boolean white = c & 0x80;
    c &= 0x7f;

    c = toupper(c) - HU_FONTSTART;
    if (c >= HU_FONTSIZE)
        return;

    w = (hu_font[c]->width);
    if ((x + w) > vid.width)
        return;

    if (white)
        // draw with colormap, WITHOUT scale
        V_DrawMappedPatch(x, y, hu_font[c], whitemap);
    else
        V_DrawScaledPatch(x, y, hu_font[c]);
}

//
//  Write a string using the hu_font
//  NOTE: the text is centered for screens larger than the base width
//
//added:05-02-98:
void V_DrawString(int x, int y, int option, char *string)
{
    int w;
    char *ch;
    int c;
    int cx, cy;
    int dupx, dupy, scrwidth = BASEVIDWIDTH;

    ch = string;
    cx = x;
    cy = y;
    if (option & V_NOSCALESTART)
    {
        dupx = vid.dupx;
        dupy = vid.dupy;
        scrwidth = vid.width;
    }
    else
        dupx = dupy = 1;

    while (1)
    {
        c = *ch++;
        if (!c)
            break;
        if (c == '\n')
        {
            cx = x;
            cy += 12 * dupy;
            continue;
        }

        c = toupper(c) - HU_FONTSTART;
        if (c < 0 || c >= HU_FONTSIZE)
        {
            cx += 4 * dupx;
            continue;
        }

	//[segabor]
        w = hu_font[c]->width * dupx;	// hu_font is endian fixed
        if (cx + w > scrwidth)
            break;
        if (option & V_WHITEMAP)
            V_DrawMappedPatch(cx, cy, hu_font[c], whitemap);
        else
            V_DrawScaledPatch(cx, cy, hu_font[c]);
        cx += w;
    }
}

// Handy utility function.
// SSNTails 06-10-2003
// unused
void V_DrawCenteredString(int x, int y, int option, char *string)
{
    int w;
    char *ch;
    int c;
    int cx;
    int cy;
    int dupx, dupy, scrwidth = BASEVIDWIDTH;

    x -= V_StringWidth(string) / 2;

    ch = string;
    cx = x;
    cy = y;
    if (option & V_NOSCALESTART)
    {
        dupx = vid.dupx;
        dupy = vid.dupy;
        scrwidth = vid.width;
    }
    else
        dupx = dupy = 1;

    while (1)
    {
        c = *ch++;
        if (!c)
            break;
        if (c == '\n')
        {
            cx = x;
            cy += 12 * dupy;
            continue;
        }

        c = toupper(c) - HU_FONTSTART;
        if (c < 0 || c >= HU_FONTSIZE)
        {
            cx += 4 * dupx;
            continue;
        }

        w = (hu_font[c]->width) * dupx;
        if (cx + w > scrwidth)
            break;

        if (option & V_WHITEMAP)
            V_DrawMappedPatch(cx, cy, hu_font[c], whitemap);
        else
            V_DrawScaledPatch(cx, cy, hu_font[c]);
        cx += w;
    }
}

//
// Find string width from hu_font chars
//
int V_StringWidth(char *string)
{
    int i;
    int w = 0;
    int c;

    for (i = 0; i < (int) strlen(string); i++)
    {
        c = toupper(string[i]) - HU_FONTSTART;
        if (c < 0 || c >= HU_FONTSIZE)
            w += 4;
        else
	    //[segabor]
            w += hu_font[c]->width;  // hu_font is endian fixed
    }

    return w;
}

//
// Find string height from hu_font chars
//
int V_StringHeight(char *string)
{
    return (hu_font[0]->height);
}

//---------------------------------------------------------------------------
//
// PROC MN_DrTextB
//
// Draw text using font B.
//
//---------------------------------------------------------------------------
int FontBBaseLump;

// per V_drawinfo
void V_DrawTextB(char *text, int x, int y)
{
    char c;
    patch_t *p;

    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            x += 8;
        }
        else
        {
            p = W_CachePatchNum(FontBBaseLump + toupper(c) - 33, PU_CACHE);  // endian fix
            V_DrawScaledPatch(x, y, p);
            x += p->width - 1;
        }
    }
}

// per V_drawinfo
void V_DrawTextBGray(char *text, int x, int y)
{
    char c;
    patch_t *p;

    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            x += 8;
        }
        else
        {
            p = W_CachePatchNum(FontBBaseLump + toupper(c) - 33, PU_CACHE);  // endian fix
            V_DrawMappedPatch(x, y, p, graymap);
            x += p->width - 1;
        }
    }
}

//---------------------------------------------------------------------------
//
// FUNC MN_TextBWidth
//
// Returns the pixel width of a string using font B.
//
//---------------------------------------------------------------------------

int V_TextBWidth(char *text)
{
    char c;
    int width;
    patch_t *p;

    width = 0;
    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            width += 5;
        }
        else
        {
            p = W_CachePatchNum(FontBBaseLump + toupper(c) - 33, PU_CACHE);  // endian fix
            width += p->width - 1;
        }
    }
    return (width);
}

int V_TextBHeight(char *text)
{
    return 16;
}

// V_Init
// old software stuff, buffers are allocated at video mode setup
// here we set the screens[x] pointers accordingly
// WARNING :
// - called at runtime (don't init cvar here)
// Must be called after every video Init and SetMode
void V_Init_Draw(void)
{
    int i;

    // Must init everything needed by DrawPixel, as that gets used for
    // many intro screens, before SCR_SetMode or SCR_Recalc are called.
    LoadPalette("PLAYPAL");
    FontBBaseLump = W_CheckNumForName("FONTB_S") + 1;
#ifdef HWRENDER // not win32 only 19990829 by Kin
    // hardware modes do not use screens[] pointers
    if (rendermode != render_soft)
    {
        // be sure to cause a NULL read/write error so we detect it, in case of..
        for (i = 0; i < NUMSCREENS; i++)
            screens[i] = NULL;
        return;
    }
#endif

    if( vid.display == NULL ) return;  // allocation failed

    // [WDJ] screens usage
    // [0] = display or direct video
    // [1] = background, status bar
    // [2] = wipe start screen, screenshot, (? Horz. draw)
    // [3] = wipe end screen
    screens[0] = vid.display;  // buffer or direct video
    // buffers allocated by video driver, 0..(NUMSCREENS-1)
    for (i = 1; i < NUMSCREENS; i++)
        screens[i] = vid.screen1 + ((i-1) * vid.screen_size);

    // [WDJ] statusbar buffer was not within driver allocated memory
    // and is not used.
    //added:26-01-98: statusbar buffer
//    screens[4] = base + NUMSCREENS * screensize;
    screens[4] = NULL;

    //!debug
#ifdef DEBUG
    CONS_Printf("V_Init done:\n");
    for (i = 0; i < NUMSCREENS + 1; i++)
        CONS_Printf(" screens[%d] = %x\n", i, screens[i]);
#endif

    // port video driver should check CanDraw before setting vid.bitpp
    switch( vid.bitpp )
    {
     case 8:
        vid.drawmode = DRAW8PAL;
        break;
#ifdef ENABLE_DRAW15
     case 15:
        vid.drawmode = DRAW15;
        break;
#endif
#ifdef ENABLE_DRAW16
     case 16:
        vid.drawmode = DRAW16;
        break;
#endif
#ifdef ENABLE_DRAW24
     case 24:
        vid.drawmode = DRAW24;
        break;
#endif
#ifdef ENABLE_DRAW32
     case 32:
        vid.drawmode = DRAW32;
        break;
#endif
     default:
        I_Error ("V_Init_Draw invalid bits per pixel: %d\n", vid.bitpp);
    }
    vid.widthbytes = vid.width * vid.bytepp;  // to save multiplies

#ifdef ENABLE_DRAWEXT
    //fab highcolor
    if ( vid.bytepp > 1 )  // highcolor, truecolor
    {
        R_Init_color8_translate( pLocalPalette );  // no palette change
    }
#endif
    return;
}

//
//
//
typedef struct
{
    int px;
    int py;
} modelvertex_t;

void R_DrawSpanNoWrap(void);    //tmap.S

//
//  Tilts the view like DukeNukem...
//
//added:12-02-98:
#ifdef TILTVIEW
#ifdef HWRENDER
void V_DrawTiltView(byte * viewbuffer)  // don't touch direct video I'll find something..
{
}
#else

static modelvertex_t vertex[4];

// Called instead of I_FinishUpdate
void V_DrawTiltView(byte * viewbuffer)
{
    fixed_t leftxfrac;
    fixed_t leftyfrac;
    fixed_t xstep;
    fixed_t ystep;

    int y;

    vertex[0].px = 0;   // tl
    vertex[0].py = 53;
    vertex[1].px = 212; // tr
    vertex[1].py = 0;
    vertex[2].px = 264; // br
    vertex[2].py = 144;
    vertex[3].px = 53;  // bl
    vertex[3].py = 199;

    // resize coords to screen
    for (y = 0; y < 4; y++)
    {
        vertex[y].px = (vertex[y].px * vid.width) / BASEVIDWIDTH;
        vertex[y].py = (vertex[y].py * vid.height) / BASEVIDHEIGHT;
    }

    ds_colormap = fixedcolormap;
    ds_source = viewbuffer;

    // starting points top-left and top-right
    leftxfrac = vertex[0].px << FRACBITS;
    leftyfrac = vertex[0].py << FRACBITS;

    // steps
    xstep = ((vertex[3].px - vertex[0].px) << FRACBITS) / vid.height;
    ystep = ((vertex[3].py - vertex[0].py) << FRACBITS) / vid.height;

#if 0
    // [WDJ] WRONG, ds_y is y line index, not a ptr
    // vid.direct not allowed without locking the video buffer
    ds_y = (int) vid.direct;
#else
    ds_y = 0;
#endif
    ds_x1 = 0;
    ds_x2 = vid.width - 1;
    ds_xstep = ((vertex[1].px - vertex[0].px) << FRACBITS) / vid.width;
    ds_ystep = ((vertex[1].py - vertex[0].py) << FRACBITS) / vid.width;

//    I_Error("ds_y %d ds_x1 %d ds_x2 %d ds_xstep %x ds_ystep %x \n"
//            "ds_xfrac %x ds_yfrac %x ds_source %x\n", ds_y,
//                      ds_x1,ds_x2,ds_xstep,ds_ystep,leftxfrac,leftyfrac,
//                      ds_source);

    // render spans
    for (y = 0; y < vid.height; y++)
    {
        // FAST ASM routine!
        ds_xfrac = leftxfrac;
        ds_yfrac = leftyfrac;
        R_DrawSpanNoWrap();
#if 0
        // [WDJ] WRONG, ds_y is y line index, not a ptr
        ds_y += vid.ybytes;
#else
        ds_y++; 
#endif

        // move along the left and right edges of the polygon
        leftxfrac += xstep;
        leftyfrac += ystep;
    }

}
#endif
#endif

//
// Test 'scrunch perspective correction' tm (c) ect.
//
//added:05-04-98:

#ifdef HWRENDER // not win32 only 19990829 by Kin
void V_DrawPerspView(byte * viewbuffer, int aiming)
{
}
#else

// Called by D_Display
// - instead of I_Finish update with page flip
void V_DrawPerspView(byte * viewbuffer, int aiming)
{
    byte *source;
    byte *dest;  // direct screen
    int y;
    int x1, w;
    int offs;

    fixed_t topfrac, bottomfrac, scale, scalestep;
    fixed_t xfrac, xfracstep;

    source = viewbuffer;

    //+16 to -16 fixed
    offs = ((aiming * 20) << 16) / 100;

    topfrac = ((vid.width - 40) << 16) - (offs * 2);
    bottomfrac = ((vid.width - 40) << 16) + (offs * 2);

    scalestep = (bottomfrac - topfrac) / vid.height;
    scale = topfrac;

    for (y = 0; y < vid.height; y++)
    {
        x1 = ((vid.width << 16) - scale) >> 17;
    // vid.direct not allowed without locking the video buffer
        dest = vid.direct + (y * vid.ybytes) + (x1 * vid.bytepp);

        xfrac = (20 << FRACBITS) + ((!x1) & 0xFFFF);
        xfracstep = FixedDiv((vid.width << FRACBITS) - (xfrac << 1), scale);
        w = scale >> 16;
#ifdef ENABLE_DRAWEXT
        if( vid.bytepp > 1 )
        {
          while (w--)
	  {
	    V_DrawPixel( dest, 0, source[xfrac >> FRACBITS] );
	    dest += vid.bytepp;
            xfrac += xfracstep;
	  }
	}else
#else
        while (w--)
        {
            *dest++ = source[xfrac >> FRACBITS];
            xfrac += xfracstep;
        }
#endif
        scale += scalestep;
        source += vid.ybytes;
    }

}
#endif
