// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: screen.c 984 2012-12-04 04:11:53Z wesleyjohnson $
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
// $Log: screen.c,v $
// Revision 1.14  2004/05/16 20:34:47  hurdler
// remove unused stuff
//
// Revision 1.13  2002/11/12 00:06:05  ssntails
// Support for translated translucent columns in software mode.
//
// Revision 1.12  2001/08/20 20:40:39  metzgermeister
// *** empty log message ***
//
// Revision 1.11  2001/05/16 21:21:14  bpereira
// no message
//
// Revision 1.10  2001/03/13 22:14:20  stroggonmeth
// Long time no commit. 3D floors, FraggleScript, portals, ect.
//
// Revision 1.9  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.8  2000/11/02 19:49:37  bpereira
// no message
//
// Revision 1.7  2000/08/31 14:30:56  bpereira
// no message
//
// Revision 1.6  2000/08/11 19:10:13  metzgermeister
// *** empty log message ***
//
// Revision 1.5  2000/08/03 17:57:42  bpereira
// no message
//
// Revision 1.4  2000/04/22 20:27:35  metzgermeister
// support for immediate fullscreen switching
//
// Revision 1.3  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      handles multiple resolutions, 8bpp/16bpp(highcolor) modes
//
//-----------------------------------------------------------------------------

// [WDJ] If you need to use a debugger then disable fullscreeen
//#define DEBUG_WINDOWED


#include "doomdef.h"
#include "screen.h"
#include "console.h"
#include "am_map.h"
#include "i_system.h"
#include "i_video.h"
#include "r_local.h"
#include "r_sky.h"
#include "m_argv.h"
#include "v_video.h"
#include "st_stuff.h"
#include "hu_stuff.h"
#include "z_zone.h"
#include "d_main.h"



// --------------------------------------------
// assembly or c drawer routines for 8bpp/16bpp
// --------------------------------------------
void (*skycolfunc) (void);       //new sky column drawer draw posts >128 high
void (*colfunc) (void);          // standard column upto 128 high posts

#ifdef HORIZONTALDRAW
//Fab 17-06-98
void (*hcolfunc) (void);         // horizontal column drawer optimisation
#endif

void (*basecolfunc) (void);
void (*fuzzcolfunc) (void);      // standard fuzzy effect column drawer
void (*skincolfunc) (void);      // skin translation
void (*transcolfunc) (void);     // translucent column drawer
void (*shadecolfunc) (void);     // smokie test..
void (*fogcolfunc) (void);       // fog effects
void (*spanfunc) (void);         // span drawer, use a 64x64 tile
void (*basespanfunc) (void);     // default span func for color mode
void (*fogspanfunc) (void);      // Legacy Fog sheet
void (*transspanfunc) (void);    // translucent span drawer

// Tails 11-11-2002
void (*skintranscolfunc) (void); // skin translation translucent

// ------------------
// global video state
// ------------------
viddef_t  vid;
int       setmodeneeded= -1;   // video mode change needed, set by menu ( -1 = NOP )


// use original Doom fuzzy effect instead of translucency?
void CV_Fuzzymode_OnChange();
consvar_t   cv_fuzzymode = {"fuzzymode", "Off", CV_SAVE | CV_CALL, CV_OnOff, CV_Fuzzymode_OnChange};


CV_PossibleValue_t scr_depth_cons_t[]={{8,"8 bits"}, {15,"15 bits"}, {16,"16 bits"}, {24,"24 bits"}, {32,"32 bits"}, {0,NULL}};

//added:03-02-98: default screen mode, as loaded/saved in config
consvar_t   cv_scr_width  = {"scr_width",  "320", CV_SAVE, CV_Unsigned};
consvar_t   cv_scr_height = {"scr_height", "200", CV_SAVE, CV_Unsigned};
consvar_t   cv_scr_depth =  {"scr_depth",  "8 bits",   CV_SAVE, scr_depth_cons_t};
#if !defined( __DJGPP__ )	
consvar_t   cv_fullscreen = {"fullscreen",  "Yes",CV_SAVE | CV_CALL, CV_YesNo, SCR_ChangeFullscreen};
#endif

// =========================================================================
//                           SCREEN VARIABLES
// =========================================================================

byte*  scr_borderflat; // flat used to fill the reduced view borders
                       // set at ST_Init ()

#if defined( ENABLE_DRAW15 ) || defined( ENABLE_DRAW16 ) || defined( ENABLE_DRAW24 ) || defined( ENABLE_DRAW32 )
#define ENABLE_DRAWEXT

#if defined( ENABLE_DRAW15 ) || defined( ENABLE_DRAW16 )
// hicolor masks  15 bit / 16 bit
uint16_t mask_01111 = 0, mask_01110 = 0, mask_11110 = 0, mask_11100 = 0, mask_11000 = 0;
uint16_t mask_r = 0, mask_g = 0, mask_b = 0, mask_rb = 0;
#endif
#endif


// =========================================================================

//added:27-01-98: tell asm code the new rowbytes value.
void ASMCALL ASM_PatchRowBytes(int rowbytes);


//  Set the video mode right now,
//  the video mode change is delayed until the start of the next refresh
//  by setting setmodeneeded to a value >= 0
//
int  VID_SetMode(int modenum);

//  Short and Tall sky drawer, for the current color mode
void (*skydrawerfunc[2]) (void);

// Called from D_DoomLoop, to init
// Called from D_Display, when setmodeneeded
void SCR_SetMode (void)
{
    if(dedicated)
        return;

    if (setmodeneeded < 0)
        return;                 //should never happen

#ifdef DEBUG_WINDOWED
    {
      // Disable fullscreen so can switch to debugger at breakpoints.
      int VID_GetModeForSize( int, int );  //vid_vesa.c
      int cvfs = cv_fullscreen.value; // preserve config.cfg
      cv_fullscreen.value = 0;
      int modenum = VID_GetModeForSize(800,600);  // debug window
      VID_SetMode(modenum);
      cv_fullscreen.value = cvfs;
      vid.modenum = setmodeneeded; // fix the display
    }
#else
    // video system interface, sets vid.recalc
    VID_SetMode(setmodeneeded);
#endif

    // No longer can use display, must fail
    if( rendermode == render_soft && vid.display == NULL )
        I_Error( "VID_SetMode failed to provide any display\n" );

    //
    //  setup the right draw routines for either 8bpp or 16bpp
    //
    //CONS_Printf ("SCR_SetMode : vid.bitpp is %d\n", vid.bitpp);
    // set the apprpriate drawer for the sky (tall or short)
    // vid.bitpp is already protected by V_Init_Draw
    switch( vid.bitpp )
    {
     case 8:
        vid.drawmode = DRAW8PAL;
        colfunc = basecolfunc = R_DrawColumn_8;
#ifdef HORIZONTALDRAW
        hcolfunc = R_DrawHColumn_8;
#endif
        skincolfunc = R_DrawTranslatedColumn_8;  // skin translation
        transcolfunc = R_DrawTranslucentColumn_8;
        shadecolfunc = R_DrawShadeColumn_8;  //R_DrawColumn_8;
        fogcolfunc = R_DrawFogColumn_8;

        spanfunc = basespanfunc = R_DrawSpan_8;
        fogspanfunc = R_DrawFogSpan_8;
        transspanfunc = R_DrawTranslucentSpan_8;

        // SSNTails 11-11-2002
        skintranscolfunc = R_DrawTranslatedTranslucentColumn_8;

        // FIXME: quick fix
        skydrawerfunc[0] = R_DrawColumn_8;      //old skies
        skydrawerfunc[1] = R_DrawSkyColumn_8;   //tall sky
        break;
#ifdef ENABLE_DRAW15
     case 15:
        vid.drawmode = DRAW15;
        mask_01111 = 0x3DEF;  // 0 01111 01111 01111 mask out the upper bit of R,G,B
        mask_01110 = 0x39CE;  // 0 01110 01110 01110 mask out the upper and lowest bit of R,G,B
        mask_11110 = 0x7BDE;  // 0 11110 11110 11110 mask out the lowest bit of R,G,B
        mask_11100 = 0x739C;  // 0 11100 11100 11100 mask out the lowest bits of R,G,B
        mask_11000 = 0x6318;  // 0 11000 11000 11000 mask out the lowest bits of R,G,B
        mask_r = 0x7C00;  // 0 11111 00000 00000 mask of R
	mask_g = 0x03E0;  // 0 00000 11111 00000 mask of G
        mask_b = 0x001F;  // 0 00000 00000 11111 mask of B
        mask_rb = 0x7C1F;  // 0 11111 00000 11111 mask of R and B
        goto highcolor_common;
#endif
#ifdef ENABLE_DRAW16
     case 16:
        vid.drawmode = DRAW16;
#if 0
        // by definition, but extra bit in green gives whites a green cast
        mask_01111 = 0x7BEF;  // 01111 011111 01111 mask out the upper bit of R,G,B
        mask_01110 = 0x73CE;  // 01110 011110 01110 mask out the upper bit of R,G,B
        mask_11110 = 0xF7DE;  // 11110 111110 11110 mask out the lowest bit of R,G,B
        mask_11110 = 0xE79C;  // 11100 111100 11100 mask out the lowest bits of R,G,B
        mask_11100 = 0xC718;  // 11000 111000 11000 mask out the lowest bits of R,G,B
        mask_r = 0xF800;  // 11111 000000 00000 mask of R
	mask_g = 0x07E0;  // 00000 111111 00000 mask of G (green cast due to 1 more bit)
        mask_b = 0x001F;  // 00000 000000 11111 mask of B
        mask_rb = 0xF81F;  // 11111 000000 11111 mask of R and B
#else
        // equal number of bits, 5 bits to each color
        mask_01111 = 0x7BCF;  // 01111 011110 01111 mask out the upper bit of R,G,B
        mask_01110 = 0x738E;  // 01110 011100 01110 mask out the upper bit of R,G,B
        mask_11110 = 0xF79E;  // 11110 111100 11110 mask out the lowest bit of R,G,B
        mask_11100 = 0xE71C;  // 11100 111000 11100 mask out the lowest bits of R,G,B
        mask_11000 = 0xC618;  // 11000 110000 11000 mask out the lowest bits of R,G,B
        mask_r = 0xF800;  // 11111 000000 00000 mask of R
	mask_g = 0x07C0;  // 00000 111110 00000 mask of G (5 bit each)
        mask_b = 0x001F;  // 00000 000000 11111 mask of B
        mask_rb = 0xF81F;  // 11111 000000 11111 mask of R and B
#endif
        goto highcolor_common;
#endif

#if defined( ENABLE_DRAW15 ) || defined( ENABLE_DRAW16 )
     highcolor_common:
        CONS_Printf ("using highcolor mode\n");

        colfunc = basecolfunc = R_DrawColumn_16;
        skincolfunc = R_DrawTranslatedColumn_16;
        transcolfunc = R_DrawTranslucentColumn_16;
	shadecolfunc = R_DrawShadeColumn_16;
        fogcolfunc = R_DrawFogColumn_16;

        spanfunc = basespanfunc = R_DrawSpan_16;
        fogspanfunc = R_DrawFogSpan_16;
        transspanfunc = R_DrawTranslucentSpan_16;

        skintranscolfunc = R_DrawTranslatedTranslucentColumn_16;

        // FIXME: quick fix to think more..
        skydrawerfunc[0] = R_DrawColumn_16;
        skydrawerfunc[1] = R_DrawSkyColumn_16;
        break;
#endif
#ifdef ENABLE_DRAW24
     case 24:
        vid.drawmode = DRAW24;
        colfunc = basecolfunc = R_DrawColumn_24;
        skincolfunc = R_DrawTranslatedColumn_24;
        transcolfunc = R_DrawTranslucentColumn_24;
	shadecolfunc = R_DrawShadeColumn_24;
        fogcolfunc = R_DrawFogColumn_24;

        spanfunc = basespanfunc = R_DrawSpan_24;
        fogspanfunc = R_DrawFogSpan_24;
        transspanfunc = R_DrawTranslucentSpan_24;

        skintranscolfunc = R_DrawTranslatedTranslucentColumn_24;

        // FIXME: quick fix to think more..
        skydrawerfunc[0] = R_DrawColumn_24;
        skydrawerfunc[1] = R_DrawSkyColumn_24;
        break;
#endif
#ifdef ENABLE_DRAW32
     case 32:
        vid.drawmode = DRAW32;
        colfunc = basecolfunc = R_DrawColumn_32;
        skincolfunc = R_DrawTranslatedColumn_32;
        transcolfunc = R_DrawTranslucentColumn_32;
	shadecolfunc = R_DrawShadeColumn_32;
        fogcolfunc = R_DrawFogColumn_32;

        spanfunc = basespanfunc = R_DrawSpan_32;
        fogspanfunc = R_DrawFogSpan_32;
        transspanfunc = R_DrawTranslucentSpan_32;

        skintranscolfunc = R_DrawTranslatedTranslucentColumn_32;

        // FIXME: quick fix to think more..
        skydrawerfunc[0] = R_DrawColumn_32;
        skydrawerfunc[1] = R_DrawSkyColumn_32;
        break;
#endif
     default:
        goto bpp_err;
    }
    vid.widthbytes = vid.width * vid.bytepp;  // to save multiplies

    V_SetPalette (0);

    // set fuzzcolfunc
    CV_Fuzzymode_OnChange();

    setmodeneeded = -1;
    return;

 bpp_err:
    I_Error ("SetMode: cannot draw %i bits per pixel\n", vid.bitpp);
}


// change drawer function when fuzzymode is changed
void CV_Fuzzymode_OnChange()
{
  switch(vid.drawmode)
  {
   case DRAW8PAL:
     fuzzcolfunc = (cv_fuzzymode.value) ? R_DrawFuzzColumn_8 : R_DrawTranslucentColumn_8;
     break;
   default:
#if defined(ENABLE_DRAW15) || defined(ENABLE_DRAW16)
   case DRAW15:
   case DRAW16:
     fuzzcolfunc = (cv_fuzzymode.value) ? R_DrawFuzzColumn_16 : R_DrawTranslucentColumn_16;
     break;
#endif
#ifdef ENABLE_DRAW24
   case DRAW24:
     fuzzcolfunc = (cv_fuzzymode.value) ? R_DrawFuzzColumn_24 : R_DrawTranslucentColumn_24;
     break;
#endif
#ifdef ENABLE_DRAW32
   case DRAW32:
     fuzzcolfunc = (cv_fuzzymode.value) ? R_DrawFuzzColumn_32 : R_DrawTranslucentColumn_32;
     break;
#endif
  }
}



//  do some initial settings for the game loading screen
//
void SCR_Startup (void)
{
    if(dedicated)
        return;

    vid.modenum = 0;

    vid.fdupx = (float)vid.width/BASEVIDWIDTH;//1.0;
    vid.fdupy = (float)vid.height/BASEVIDHEIGHT; //1.0f;
    vid.dupx = (int)vid.fdupx; //1;
    vid.dupy = (int)vid.fdupy; //1;

    //vid.baseratio = FRACUNIT; //Hurdler: not used anymore

    scaledofs = 0;
    vid.centerofs = 0;

#ifdef USEASM
    ASM_PatchRowBytes(vid.width);
#endif

    V_Init_Draw();

    V_SetPalette (0);
}


//added:24-01-98:
//
// Recalc settings for game drawing, trigger updates to view size and status bar.
// Called at new frame, if the video mode has changed
// Called from D_DoomLoop upon entry, after SCR_SetMode.
// Called from D_Display upon video mode change, after SCR_SetMode.
void SCR_Recalc (void)
{
    if(dedicated || ! vid.recalc)
        return;

    //added:18-02-98: scale 1,2,3 times in x and y the patches for the
    //                menus and overlays... calculated once and for all
    //                used by routines in v_video.c
    //if ( rendermode == render_soft )
    {
        // leave it be 1 in hardware accelerated modes
        vid.dupx = vid.width / BASEVIDWIDTH;
        vid.dupy = vid.height / BASEVIDHEIGHT;
        vid.fdupx = (float)vid.width / BASEVIDWIDTH;
        vid.fdupy = (float)vid.height / BASEVIDHEIGHT;
        //vid.baseratio = FixedDiv(vid.height << FRACBITS, BASEVIDHEIGHT << FRACBITS); //Hurdler: not used anymore
    }

    //added:18-02-98: calculate centering offset for the scaled menu
    scaledofs = 0;  //see v_video.c
    vid.centerofs = (((vid.height%BASEVIDHEIGHT)/2) * vid.width) +
                    (vid.width%BASEVIDWIDTH)/2;

    // patch the asm code depending on vid buffer rowbytes
#ifdef USEASM
    ASM_PatchRowBytes(vid.width);
#endif

    // toggle off automap because some screensize-dependent values will
    // be calculated next time the automap is activated.
    if (automapactive)
        AM_Stop();

    // fuzzoffsets for the 'spectre' effect,... this is a quick hack for
    // compatibility, because I don't use it anymore since transparency
    // looks much better.
    R_RecalcFuzzOffsets();

        // r_plane stuff : visplanes, openings, floorclip, ceilingclip, spanstart,
        //                 spanstop, yslope, distscale, cachedheight, cacheddistance,
        //                 cachedxstep, cachedystep
        //              -> allocated at the maximum vidsize, static.

    // r_main : xtoviewangle, allocated at the maximum size.
    // r_things : negonearray, screenheightarray allocated max. size.

    // set the screen[x] ptrs on the new vidbuffers
    V_Init_Draw();

    // scr_viewsize doesn't change, neither detailLevel, but the pixels
    // per screenblock is different now, since we've changed resolution.
    R_SetViewSize ();   //just set setsizeneeded true now ..

    // vid.recalc lasts only for the next refresh...
    con_recalc = true;
//    CON_ToggleOff ();  // make sure con height is right for new screen height

    st_recalc = true;
    am_recalc = true;
}


// Check for screen cmd-line parms : to force a resolution.
//
// Set the video mode to set at the 1st display loop (setmodeneeded)
//
int VID_GetModeForSize( int w, int h);  //vid_vesa.c

void SCR_CheckDefaultMode (void)
{
    int p;
    int scr_forcex;     // resolution asked from the cmd-line
    int scr_forcey;

    if(dedicated)
        return;

    // 0 means not set at the cmd-line
    scr_forcex = 0;
    scr_forcey = 0;

    p = M_CheckParm("-width");
    if (p && p < myargc-1)
        scr_forcex = atoi(myargv[p+1]);

    p = M_CheckParm("-height");
    if (p && p < myargc-1)
        scr_forcey = atoi(myargv[p+1]);

    if (scr_forcex && scr_forcey)
    {
        CONS_Printf("Using resolution: %d x %d\n",scr_forcex,scr_forcey);
        // returns -1 if not found, (no mode change)
        setmodeneeded = VID_GetModeForSize(scr_forcex,scr_forcey);
        //if (scr_forcex!=BASEVIDWIDTH || scr_forcey!=BASEVIDHEIGHT)
    }
    else
    {
        CONS_Printf("Default resolution: %d x %d (%d bits)\n",cv_scr_width.value,cv_scr_height.value,cv_scr_depth.value);
        // see note above
        setmodeneeded = VID_GetModeForSize(cv_scr_width.value,cv_scr_height.value);
    }
}


//added:03-02-98: sets the modenum as the new default video mode to be saved
//                in the config file
void SCR_SetDefaultMode (void)
{
    // remember the default screen size
    CV_SetValue (&cv_scr_width,  vid.width);
    CV_SetValue (&cv_scr_height, vid.height);
    CV_SetValue (&cv_scr_depth,  vid.bitpp);
    //    CV_SetValue (&cv_fullscreen, vid.fullscreen); // metzgermeister: unnecessary?
}

// Change fullscreen on/off according to cv_fullscreen
#if !defined( __DJGPP__ )
void SCR_ChangeFullscreen (void)
{
  // used to prevent switching to fullscreen during startup
  if (!allow_fullscreen)
    return;

  if(graphics_started) {
    setmodeneeded = VID_GetModeForSize(cv_scr_width.value,cv_scr_height.value);
  }
}
#endif
