// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: r_sky.c 1414 2018-12-06 22:01:48Z wesleyjohnson $
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
// $Log: r_sky.c,v $
// Revision 1.8  2001/08/06 23:57:09  stroggonmeth
// Removed portal code, improved 3D floors in hardware mode.
//
// Revision 1.7  2001/04/02 18:54:32  bpereira
//
// Revision 1.6  2001/03/21 18:24:39  stroggonmeth
// Misc changes and fixes. Code cleanup
//
// Revision 1.5  2001/03/13 22:14:20  stroggonmeth
// Long time no commit. 3D floors, FraggleScript, portals, ect.
//
// Revision 1.4  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.3  2000/09/21 16:45:08  bpereira
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Sky rendering. The DOOM sky is a texture map like any
//      wall, wrapping around. A 1024 columns equal 360 degrees.
//      The default sky map is 256 columns and repeats 4 times
//      on a 320 screen?
//  
//
//-----------------------------------------------------------------------------

#include "doomincl.h"
#include "r_local.h"
#include "w_wad.h"
#include "z_zone.h"

#include "p_maputl.h"
  // P_PointOnLineSide
#include "m_swap.h"
#include "r_sky.h"

// SoM: I know I should be moving portals out of r_sky.c and as soon
// as I have time and a I will... But for now, they are mostly used
// for sky boxes anyway so they have a mostly appropriate home here.


//
// sky mapping
//
int     skyflatnum;
int     skytexture = 0;
int     skytexturemid;

fixed_t skyscale;
byte    sky_240=0;  // 0=std 128 sky, 1=240 high sky

//
// R_Init_SkyMap called at startup, once.
//
void R_Init_SkyMap (void)
{
    // set at P_LoadSectors
    //skyflatnum = R_FlatNumForName ( SKYFLATNAME );
}


//  Setup sky draw for old or new skies (new skies = freelook 256x240)
//
//  Call at loadlevel after skytexture is set
//
//  NOTE: skycolfunc should be set at R_ExecuteSetViewSize ()
//        I dont bother because we don't use low detail no more
//
void R_Setup_SkyDraw (void)
{
    texpatch_t*  texpatch;
    patch_t      wpatch;
    int          count;
    int          max_height;
    int          i;

    if( skytexture == 0 )  return;

    // parse the patches composing sky texture for the tallest one
    // patches are usually RSKY1,RSKY2... and unique

    // note: the TEXTURES lump doesn't have the taller size of Legacy
    //       skies, but the patches it use will give the right size

    count   = textures[skytexture]->patchcount;
    texpatch = &textures[skytexture]->patches[0];
    max_height = 0;
    for (i=0;i<count;i++)
    {
        W_ReadLumpHeader (texpatch->patchnum, &wpatch, sizeof(patch_t));
        // [WDJ] Do endian fix as this is read.
        wpatch.height = LE_SWAP16(wpatch.height);
        if( wpatch.height > max_height )
            max_height = wpatch.height;
        texpatch++;
    }

    // DIRTY : should set the routine depending on colormode in screen.c
    if(max_height > 128)
    {
        // horizon line on 256x240 freelook textures of Legacy or heretic
        skytexturemid = 200<<FRACBITS;
        sky_240 = 1;
    }
    else
    {
        // the horizon line in a 256x128 sky texture
        skytexturemid = 100<<FRACBITS;
        sky_240 = 0;
    }

    // get the right drawer, it was set by screen.c, depending on the
    // current video mode bytes per pixel (quick fix)
    skycolfunc = skydrawerfunc[sky_240];

    R_SetSkyScale ();
}


// set the correct scale for the sky at setviewsize
void R_SetSkyScale (void)
{
    //fix this quick mess
    if( skytexturemid > (100<<FRACBITS))
    {
        // normal aspect ratio corrected scale
        skyscale = FixedDiv (FRACUNIT, pspriteyscale);
    }
    else
    {
        // double the texture vertically, bleeergh!!
        skyscale = FixedDiv (FRACUNIT, pspriteyscale)>>1;
    }
}




