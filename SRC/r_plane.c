// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: r_plane.c 644 2010-05-11 21:43:40Z wesleyjohnson $
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
// $Log: r_plane.c,v $
// Revision 1.20  2004/05/16 20:25:46  hurdler
// change version to 1.43
//
// Revision 1.19  2003/06/10 23:36:09  ssntails
// Variable flat support (32x32 to 2048x2048)
//
// Revision 1.18  2002/09/25 16:38:35  ssntails
// Alpha support for trans 3d floors in software
//
// Revision 1.17  2001/08/06 23:57:09  stroggonmeth
// Removed portal code, improved 3D floors in hardware mode.
//
// Revision 1.16  2001/05/30 04:00:52  stroggonmeth
// Fixed crashing bugs in software with 3D floors.
//
// Revision 1.15  2001/03/21 18:24:39  stroggonmeth
// Misc changes and fixes. Code cleanup
//
// Revision 1.14  2001/03/13 22:14:20  stroggonmeth
// Long time no commit. 3D floors, FraggleScript, portals, ect.
//
// Revision 1.13  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.12  2000/11/11 13:59:46  bpereira
// no message
//
// Revision 1.11  2000/11/06 20:52:16  bpereira
// no message
//
// Revision 1.10  2000/11/02 17:50:09  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.9  2000/04/30 10:30:10  bpereira
// no message
//
// Revision 1.8  2000/04/18 17:39:39  stroggonmeth
// Bug fixes and performance tuning.
//
// Revision 1.7  2000/04/13 23:47:47  stroggonmeth
// See logs
//
// Revision 1.6  2000/04/08 17:29:25  stroggonmeth
// no message
//
// Revision 1.5  2000/04/06 21:06:19  stroggonmeth
// Optimized extra_colormap code...
// Added #ifdefs for older water code.
//
// Revision 1.4  2000/04/04 19:28:43  stroggonmeth
// Global colormaps working. Added a new linedef type 272.
//
// Revision 1.3  2000/04/04 00:32:48  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Here is a core component: drawing the floors and ceilings,
//       while maintaining a per column clipping list only.
//      Moreover, the sky areas have to be determined.
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "console.h"
#include "g_game.h"
#include "r_data.h"
#include "r_local.h"
#include "r_state.h"
#include "r_splats.h"   //faB(21jan):testing
#include "r_sky.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

#include "p_setup.h"    // levelflats

planefunction_t         floorfunc = NULL;
planefunction_t         ceilingfunc = NULL;

//
// opening
//

// Here comes the obnoxious "visplane".
/*#define                 MAXVISPLANES 128 //SoM: 3/20/2000
visplane_t*             visplanes;
visplane_t*             lastvisplane;*/

//SoM: 3/23/2000: Use Boom visplane hashing.
#define           MAXVISPLANES      128

static visplane_t *visplanes[MAXVISPLANES];
static visplane_t *freetail;
static visplane_t **freehead = &freetail;


visplane_t*             floorplane;
visplane_t*             ceilingplane;

visplane_t*             currentplane;

planemgr_t              ffloor[MAXFFLOORS];  // this use 251 Kb memory (in Legacy 1.43)
int                     numffloors;

//SoM: 3/23/2000: Boom visplane hashing routine.
#define visplane_hash(picnum,lightlevel,height) \
  ((unsigned)((picnum)*3+(lightlevel)+(height)*7) & (MAXVISPLANES-1))

// ?
/*#define MAXOPENINGS     MAXVIDWIDTH*128
short                   openings[MAXOPENINGS];
short*                  lastopening;*/

//SoM: 3/23/2000: Use boom opening limit removal
size_t maxopenings = 0;
short *openings = NULL;
short *lastopening = NULL;



//
// Clip values are the solid pixel bounding the range.
//  floorclip starts out SCREENHEIGHT
//  ceilingclip starts out -1
//
short                   floorclip[MAXVIDWIDTH];
short                   ceilingclip[MAXVIDWIDTH];
fixed_t                 frontscale[MAXVIDWIDTH];


//
// spanstart holds the start of a plane span
// initialized to 0 at start
//
int                     spanstart[MAXVIDHEIGHT];
//int                     spanstop[MAXVIDHEIGHT]; //added:08-02-98: Unused!!

//
// texture mapping
//
lighttable_t**          planezlight;
fixed_t                 planeheight;

//added:10-02-98: yslopetab is what yslope used to be,
//                yslope points somewhere into yslopetab,
//                now (viewheight/2) slopes are calculated above and
//                below the original viewheight for mouselook
//                (this is to calculate yslopes only when really needed)
//                (when mouselookin', yslope is moving into yslopetab)
//                Check R_SetupFrame, R_SetViewSize for more...
fixed_t                 yslopetab[MAXVIDHEIGHT*4];
fixed_t*                yslope = NULL;

fixed_t                 distscale[MAXVIDWIDTH];
fixed_t                 basexscale;
fixed_t                 baseyscale;

fixed_t                 cachedheight[MAXVIDHEIGHT];
fixed_t                 cacheddistance[MAXVIDHEIGHT];
fixed_t                 cachedxstep[MAXVIDHEIGHT];
fixed_t                 cachedystep[MAXVIDHEIGHT];

fixed_t   xoffs, yoffs;

//
// R_InitPlanes
// Only at game startup.
//
void R_InitPlanes (void)
{
  // Doh!
}


//profile stuff ---------------------------------------------------------
//#define TIMING
#ifdef TIMING
#include "p5prof.h"
         long long mycount;
         long long mytotal = 0;
         unsigned long  nombre = 100000;
#endif
//profile stuff ---------------------------------------------------------


//
// R_MapPlane
//
// Uses global vars:
//  planeheight
//  ds_source
//  basexscale
//  baseyscale
//  viewx
//  viewy
//  xoffs
//  yoffs
//
// BASIC PRIMITIVE
//

// Draw plane span at row y, span=(x1..x2)
// at planeheight, using spanfunc
void R_MapPlane
( int           y,              // t1
  int           x1,
  int           x2 )
{
    angle_t     angle;
    fixed_t     distance;
    fixed_t     length;
    unsigned    index;

#ifdef RANGECHECK
    if (x2 < x1
        || x1<0
        || x2>=rdraw_viewwidth
        || (unsigned)y>rdraw_viewheight)    // [WDJ] ??  y>=rdraw_viewheight
    {
        I_Error ("R_MapPlane: %i, %i at %i",x1,x2,y);
    }
#endif

    if (planeheight != cachedheight[y])
    {
        cachedheight[y] = planeheight;
        distance = cacheddistance[y] = FixedMul (planeheight, yslope[y]);
        ds_xstep = cachedxstep[y] = FixedMul (distance,basexscale);
        ds_ystep = cachedystep[y] = FixedMul (distance,baseyscale);
    }
    else
    {
        distance = cacheddistance[y];
        ds_xstep = cachedxstep[y];
        ds_ystep = cachedystep[y];
    }
    length = FixedMul (distance,distscale[x1]);
    angle = (currentplane->viewangle + xtoviewangle[x1])>>ANGLETOFINESHIFT;
    // SoM: Wouldn't it be faster just to add viewx and viewy to the plane's
    // x/yoffs anyway?? (Besides, it serves my purpose well for portals!)
    ds_xfrac = /*viewx +*/ FixedMul(finecosine[angle], length) + xoffs;
    ds_yfrac = /*-viewy*/yoffs - FixedMul(finesine[angle], length);


    if (fixedcolormap)
        ds_colormap = fixedcolormap;
    else
    {
        index = distance >> LIGHTZSHIFT;

        if (index >= MAXLIGHTZ )
            index = MAXLIGHTZ-1;

        ds_colormap = planezlight[index];
    }
    if(currentplane->extra_colormap && !fixedcolormap)
      ds_colormap = currentplane->extra_colormap->colormap + (ds_colormap - colormaps);

    ds_y = y;
    ds_x1 = x1;
    ds_x2 = x2;
    // high or low detail

//added:16-01-98:profile hspans drawer.
#ifdef TIMING
  ProfZeroTimer();
#endif
   
  spanfunc ();

#ifdef TIMING
  RDMSR(0x10,&mycount);
  mytotal += mycount;   //64bit add
  if(nombre--==0)
  I_Error("spanfunc() CPU Spy reports: 0x%d %d\n", *((int*)&mytotal+1),
                                        (int)mytotal );
#endif

}


//
// R_ClearPlanes
// At begining of frame.
//
//Fab:26-04-98:
// NOTE : uses con_clipviewtop, so that when console is on,
//        don't draw the part of the view hidden under the console
void R_ClearPlanes (player_t *player)
{
    int         i, p;
    angle_t     angle;


    // opening / clipping determination
    for (i=0 ; i<rdraw_viewwidth ; i++)
    {
        floorclip[i] = rdraw_viewheight;
        ceilingclip[i] = con_clipviewtop;       //Fab:26-04-98: was -1
        frontscale[i] = MAXINT;
        for(p = 0; p < MAXFFLOORS; p++)
        {
          ffloor[p].f_clip[i] = rdraw_viewheight;
          ffloor[p].c_clip[i] = con_clipviewtop;
        }
    }

    numffloors = 0;

    //lastvisplane = visplanes;

    //SoM: 3/23/2000
    for (i=0;i<MAXVISPLANES;i++)
      for (*freehead = visplanes[i], visplanes[i] = NULL; *freehead; )
        freehead = &(*freehead)->next;

    lastopening = openings;

    // texture calculation
    memset (cachedheight, 0, sizeof(cachedheight));

    // left to right mapping
    angle = (viewangle-ANG90)>>ANGLETOFINESHIFT;

    // scale will be unit scale at SCREENWIDTH/2 distance
    basexscale = FixedDiv (finecosine[angle],centerxfrac);
    baseyscale = -FixedDiv (finesine[angle],centerxfrac);
}


//SoM: 3/23/2000: New function, by Lee Killough
static visplane_t *new_visplane(unsigned hash)
{
  visplane_t *check = freetail;
  if (!check)
    check = calloc(1, sizeof *check);
  else
    if (!(freetail = freetail->next))
      freehead = &freetail;
  check->next = visplanes[hash];
  visplanes[hash] = check;
  return check;
}



//
// R_FindPlane : cherche un visplane ayant les valeurs identiques:
//               meme hauteur, meme flattexture, meme lightlevel.
//               Sinon en alloue un autre.
//
visplane_t* R_FindPlane( fixed_t height,
                         int     picnum,
                         int     lightlevel,
                         fixed_t xoff,
                         fixed_t yoff,
                         extracolormap_t* planecolormap,
                         ffloor_t* ffloor)
{
    visplane_t* check;
    unsigned    hash; //SoM: 3/23/2000

    xoff += viewx; // SoM
    yoff = -viewy + yoff;

    if (picnum == skyflatnum)
    {
        height = 0;                     // all skys map together
        lightlevel = 0;
    }


    //SoM: 3/23/2000: New visplane algorithm uses hash table -- killough
    hash = visplane_hash(picnum,lightlevel,height);

    for (check=visplanes[hash]; check; check=check->next)
    {
      if (height == check->height &&
          picnum == check->picnum &&
          lightlevel == check->lightlevel &&
          xoff == check->xoffs &&
          yoff == check->yoffs &&
          planecolormap == check->extra_colormap &&
          !ffloor && !check->ffloor &&
          check->viewz == viewz &&
          check->viewangle == viewangle)
        return check;
    }

    check = new_visplane(hash);

    check->height = height;
    check->picnum = picnum;
    check->lightlevel = lightlevel;
    check->minx = vid.width;
    check->maxx = -1;
    check->xoffs = xoff;
    check->yoffs = yoff;
    check->extra_colormap = planecolormap;
    check->ffloor = ffloor;
    check->viewz = viewz;
    check->viewangle = viewangle;

    memset (check->top, 0xff, sizeof(check->top));

    return check;
}




//
// R_CheckPlane : return same visplane or alloc a new one if needed
//
visplane_t*  R_CheckPlane( visplane_t*   pl,
                           int           start,
                           int           stop )
{
    int         intrl;
    int         intrh;
    int         unionl;
    int         unionh;
    int         x;

    if (start < pl->minx)
    {
        intrl = pl->minx;
        unionl = start;
    }
    else
    {
        unionl = pl->minx;
        intrl = start;
    }

    if (stop > pl->maxx)
    {
        intrh = pl->maxx;
        unionh = stop;
    }
    else
    {
        unionh = pl->maxx;
        intrh = stop;
    }

    //added 30-12-97 : 0xff ne vaut plus -1 avec un short...
    for (x=intrl ; x<= intrh ; x++)
        if (pl->top[x] != 0xffff)
            break;

    //SoM: 3/23/2000: Boom code
    if (x > intrh)
      pl->minx = unionl, pl->maxx = unionh;
    else
      {
        unsigned hash = visplane_hash(pl->picnum, pl->lightlevel, pl->height);
        visplane_t *new_pl = new_visplane(hash);

        new_pl->height = pl->height;
        new_pl->picnum = pl->picnum;
        new_pl->lightlevel = pl->lightlevel;
        new_pl->xoffs = pl->xoffs;           // killough 2/28/98
        new_pl->yoffs = pl->yoffs;
        new_pl->extra_colormap = pl->extra_colormap;
        new_pl->ffloor = pl->ffloor;
        new_pl->viewz = pl->viewz;
        new_pl->viewangle = pl->viewangle;
        pl = new_pl;
        pl->minx = start;
        pl->maxx = stop;
        memset(pl->top, 0xff, sizeof pl->top);
      }
    return pl;
}


//
// R_ExpandPlane
//
// SoM: This function basically expands the visplane or I_Errors
// The reason for this is that when creating 3D floor planes, there is no
// need to create new ones with R_CheckPlane, because 3D floor planes
// are created by subsector and there is no way a subsector can graphically
// overlap.
void R_ExpandPlane(visplane_t*  pl, int start, int stop)
{
    int         intrl;
    int         intrh;
    int         unionl;
    int         unionh;
        int                     x;

    if (start < pl->minx)
    {
        intrl = pl->minx;
        unionl = start;
    }
    else
    {
        unionl = pl->minx;
        intrl = start;
    }

    if (stop > pl->maxx)
    {
        intrh = pl->maxx;
        unionh = stop;
    }
    else
    {
        unionh = pl->maxx;
        intrh = stop;
    }

    for (x = start ; x <= stop ; x++)
        if (pl->top[x] != 0xffff)
            break;

    //SoM: 3/23/2000: Boom code
    if (x > stop)
      pl->minx = unionl, pl->maxx = unionh;
//    else
//      I_Error("R_ExpandPlane: planes in same subsector overlap?!\nminx: %i, maxx: %i, start: %i, stop: %i\n", pl->minx, pl->maxx, start, stop);

    pl->minx = unionl, pl->maxx = unionh;
}


//
// R_MakeSpans
//
// Draw plane spans at rows (t1..b1), span=(spanstart..x-1)
//    except when disabled by t1>viewheight
// Setup spanstart for next span at rows (t2..b2),
//    except when disabled by t2>viewheight
// at planeheight, using spanfunc
void R_MakeSpans
( int           x,
  int           t1,
  int           b1,
  int           t2,
  int           b2 )
{
    // [WDJ] 11/10/2009  Fix crash in 3DHorror wad, sloppy limit checks on
    // spans caused writes to spanstart[] to overwrite yslope array.
    int lim;

    // Draw the spans over (t1..b1), skipping (t2..b2) which will be
    // drawn with (t2..b2) as one span.

    if( b1 >= rdraw_viewheight)
       b1 = rdraw_viewheight-1;
    if( b2 >= rdraw_viewheight)
       b2 = rdraw_viewheight-1;
   
    // Draw the spans over (t1..b1), up to but not including t2
    // If t2>rdraw_viewheight, then not valid and non-overlapping
    lim = b1+1; // not including
    if( t2 < lim )   lim = t2;		// valid and overlapping
    // unnecessary to limit lim to rdraw_viewheight if limit b1
    while (t1 < lim)  //  while (t1 < t2 && t1<=b1)
    {
        R_MapPlane (t1,spanstart[t1],x-1);  // y=t1, x=(spanstart[t1] .. x-1)
        t1++;
    }
   
    // Continue drawing (t1..b1), from b1, down to but not including b2.
    // If t2>rdraw_viewheight (disabled), then previous loop did it all
    // already and completed with t1<b1
    lim = t1-1;  // not including, if t1 invalid then is disabling
    if( b2 > lim )   lim = b2;		// valid and overlapping
    // unnecessary to limit lim to rdraw_viewheight, must limit b1 instead
    while (b1 > lim)  //  while (b1 > b2 && b1>=t1)
    {
       R_MapPlane (b1,spanstart[b1],x-1);	// y=b1, x=(spanstart[b1] .. x-1)
       b1--;
    }
   
    // Init spanstart over next span (t2..b2) that is not within (t1..b1)
    // Within (t1..b1) will use existing spanstart to draw this span
    // combined with other deferred span draws.

    // Init spanstart over (t2..b2) where less than t1.
    lim = b2+1;  // not including
    if( t1 < lim )   lim = t1;		// valid and overlapping
    // unnecessary to limit lim to rdraw_viewheight if limit b2
    // unnecessary to limit t2, as it is set from unsigned
    // loop only if t2<rdraw_viewheight, because b2 is limited
    while (t2 < lim)  // while (t2 < t1 && t2<=b2)
    {
        spanstart[t2] = x;
        t2++;
    }
   
    // Init spanstart over (t2..b2) where greater than b1.
    lim = t2-1;  // not including, if t2 invalid, then is disabling
    if( b1 > lim )   lim = b1;		// valid and overlapping
    if( lim < -1 )   lim = -1;
    while( b2 > lim )  //  while (b2 > b1 && b2>=t2)
    {
        spanstart[b2] = x;
        b2--;
    }
}



byte* R_GetFlat (int  flatnum);

void R_DrawPlanes (void)
{
    visplane_t*         pl;
    int                 x;
    int                 angle;
    int                 i; //SoM: 3/23/2000

    spanfunc = basespanfunc;

    for (i=0;i<MAXVISPLANES;i++, pl++)
    for (pl=visplanes[i]; pl; pl=pl->next)
    {
        // sky flat
        if (pl->picnum == skyflatnum)
        {
            //added:12-02-98: use correct aspect ratio scale
            //dc_iscale = FixedDiv (FRACUNIT, pspriteyscale);
            dc_iscale = skyscale;

// Kik test non-moving sky .. weird
// cy = centery;
// centery = (rdraw_viewheight/2);

            // Sky is allways drawn full bright,
            //  i.e. colormaps[0] is used.
            // Because of this hack, sky is not affected
            //  by INVUL inverse mapping.
#if 0
            // BP: this fix sky not inversed in invuln but it is a original doom2 feature (bug?)
            if(fixedcolormap)
                dc_colormap = fixedcolormap;
            else
#endif
            dc_colormap = colormaps;
            dc_texturemid = skytexturemid;
            dc_texheight = textureheight[skytexture] >> FRACBITS;
            for (x=pl->minx ; x <= pl->maxx ; x++)
            {
                dc_yl = pl->top[x];
                dc_yh = pl->bottom[x];

                if (dc_yl <= dc_yh && dc_yh >= 0 && dc_yl < rdraw_viewheight )
                {
		   //[WDJ] phobiata.wad has many views that need clipping
		    if ( dc_yl < 0 )   dc_yl = 0;
		    if ( dc_yh >= rdraw_viewheight )   dc_yh = rdraw_viewheight - 1;
                    angle = (viewangle + xtoviewangle[x])>>ANGLETOSKYSHIFT;
                    dc_x = x;
                    dc_source = R_GetColumn(skytexture, angle);
                    skycolfunc ();
                }
            }
// centery = cy;
            continue;
        }

        if(pl->ffloor)
          continue;

        R_DrawSinglePlane(pl);
    }
}




void R_DrawSinglePlane(visplane_t* pl)
{
  int                 light = 0;
  int                 x;
  int                 stop;
  int                 angle;

  if (pl->minx > pl->maxx)
    return;

  spanfunc = basespanfunc;
  if(pl->ffloor)
  {
    if(pl->ffloor->flags & FF_TRANSLUCENT)
    {
      spanfunc = R_DrawTranslucentSpan_8;

          // Hacked up support for alpha value in software mode SSNTails 09-24-2002
          if(pl->ffloor->alpha < 64)
                  ds_transmap = ((3)<<FF_TRANSSHIFT) - 0x10000 + transtables;
          else if(pl->ffloor->alpha < 128 && pl->ffloor->alpha > 63)
                  ds_transmap = ((2)<<FF_TRANSSHIFT) - 0x10000 + transtables;
          else
                  ds_transmap = ((1)<<FF_TRANSSHIFT) - 0x10000 + transtables;

      if(pl->extra_colormap && pl->extra_colormap->fog)
        light = (pl->lightlevel >> LIGHTSEGSHIFT);
      else
        light = LIGHTLEVELS-1;
    }
    else if(pl->ffloor->flags & FF_FOG)
    {
      spanfunc = R_DrawFogSpan_8;
      light = (pl->lightlevel >> LIGHTSEGSHIFT);
    }
    else if(pl->extra_colormap && pl->extra_colormap->fog)
      light = (pl->lightlevel >> LIGHTSEGSHIFT);
    else
      light = (pl->lightlevel >> LIGHTSEGSHIFT)+extralight;
  }
  else
  {
    if(pl->extra_colormap && pl->extra_colormap->fog)
      light = (pl->lightlevel >> LIGHTSEGSHIFT);
    else
      light = (pl->lightlevel >> LIGHTSEGSHIFT)+extralight;
  }

  if(viewangle != pl->viewangle)
  {
    memset (cachedheight, 0, sizeof(cachedheight));

    angle = (pl->viewangle-ANG90)>>ANGLETOFINESHIFT;

    basexscale = FixedDiv (finecosine[angle],centerxfrac);
    baseyscale = -FixedDiv (finesine[angle],centerxfrac);
    viewangle = pl->viewangle;
  }

  currentplane = pl;


  // [WDJ] Flat use is safe from alloc, change to PU_CACHE at function exit.
  ds_source = (byte *) R_GetFlat (levelflats[pl->picnum].lumpnum);

  int size = W_LumpLength(levelflats[pl->picnum].lumpnum);
  switch(size)
  {
    case 2048*2048: // 2048x2048 lump
      flatsize = 2048;
      flatmask = 2047<<11;
      flatsubtract = 11;
      break;
    case 1024*1024: // 1024x1024 lump
      flatsize = 1024;
      flatmask = 1023<<10;
      flatsubtract = 10;
      break;
    case 512*512:// 512x512 lump
      flatsize = 512;
      flatmask = 511<<9;
      flatsubtract = 9;
      break;
    case 256*256: // 256x256 lump
      flatsize = 256;
      flatmask = 255<<8;
      flatsubtract = 8;
      break;
    case 128*128: // 128x128 lump
      flatsize = 128;
      flatmask = 127<<7;
      flatsubtract = 7;
      break;
    case 32*32: // 32x32 lump
      flatsize = 32;
      flatmask = 31<<5;
      flatsubtract = 5;
      break;
    default: // 64x64 lump
      flatsize = 64;
      flatmask = 0x3f<<6;
      flatsubtract = 6;
      break;
  }


  xoffs = pl->xoffs;
  yoffs = pl->yoffs;
  planeheight = abs(pl->height - pl->viewz);

  if (light >= LIGHTLEVELS)
      light = LIGHTLEVELS-1;

  if (light < 0)
      light = 0;

  planezlight = zlight[light];

  //set the MAXIMUM value for unsigned short (but is not MAX for int)
  // mark the columns on either side of the valid area
  pl->top[pl->maxx+1] = 0xffff;		// disable setup spanstart
  pl->top[pl->minx-1] = 0xffff;		// disable drawing on first call
//  pl->bottom[pl->maxx+1] = 0;		// prevent interference from random value
//  pl->bottom[pl->minx-1] = 0;		// prevent interference from random value

  stop = pl->maxx + 1;

  for (x=pl->minx ; x<= stop ; x++)
  {
    R_MakeSpans( x,
		pl->top[x-1], pl->bottom[x-1],	// draw range (except first)
                pl->top[x], pl->bottom[x]	// setup spanstart range
		);
  }


  Z_ChangeTag (ds_source, PU_CACHE);
}


void R_PlaneBounds(visplane_t* plane)
{
  int  i;
  int  hi, low;

  hi = plane->top[plane->minx];
  low = plane->bottom[plane->minx];

  for(i = plane->minx + 1; i <= plane->maxx; i++)
  {
    if(plane->top[i] < hi)
      hi = plane->top[i];
    if(plane->bottom[i] > low)
      low = plane->bottom[i];
  }
  plane->high = hi;
  plane->low = low;
}
