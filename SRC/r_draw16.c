// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: r_draw16.c 572 2009-11-29 01:14:35Z wesleyjohnson $
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
// $Log: r_draw16.c,v $
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      16bpp (HIGHCOLOR) span/column drawer functions
//
//  NOTE: no includes because this is included as part of r_draw.c
//
// THIS IS THE VERY BEGINNING OF DOOM LEGACY's HICOLOR MODE (or TRUECOLOR)
// Doom Legacy will use HICOLOR textures, rendered through software, and
// may use MMX, or WILL use MMX to get a decent speed.
//
//-----------------------------------------------------------------------------


// ==========================================================================
// COLUMNS
// ==========================================================================

// r_data.c
extern short    color8to16[256];        //remap color index to highcolor
extern short*   hicolormaps;            //remap high colors to high colors..

#define HIMASK1   0x7bde     //kick out the upper bit of each
                                        //composant (we're in 5:5:5)

//  standard upto 128high posts column drawer
//
void R_DrawColumn_16 (void)
{
    int                 count;
    short*              dest;
    fixed_t             frac;
    fixed_t             fracstep;

    count = dc_yh - dc_yl+1;

    // Zero length, column does not exceed a pixel.
    if (count <= 0)
        return;

#ifdef RANGECHECK
    if ((unsigned)dc_x >= vid.width
        || dc_yl < 0
        || dc_yh >= vid.height)
        I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif

    // Framebuffer destination address.
    // Use ylookup LUT to avoid multiply with ScreenWidth.
    // Use columnofs LUT for subwindows?
    dest = (short *) (ylookup[dc_yl] + columnofs[dc_x]);

    // Determine scaling,
    //  which is the only mapping to be done.
    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl-centery)*fracstep;

    // Inner loop that does the actual texture mapping,
    //  e.g. a DDA-lile scaling.
    // This is as fast as it gets.

    do
    {
        // Re-map color indices from wall texture column
        //  using a lighting/special effects LUT.
        //*dest = *( (short *)dc_colormap + dc_source[(frac>>FRACBITS)&127] );
        *dest = hicolormaps[ ((short*)dc_source)[(frac>>FRACBITS)&127]>>1 ];

        dest += vid.width;
        frac += fracstep;

    } while (--count);
}


//  LAME cutnpaste : same as R_DrawColumn_16 but wraps around 256
//  instead of 128 for the tall sky textures (256x240)
//
void R_DrawSkyColumn_16 (void)
{
    int                 count;
    short*              dest;
    fixed_t             frac;
    fixed_t             fracstep;

    count = dc_yh - dc_yl+1;

    // Zero length, column does not exceed a pixel.
    if (count <= 0)
        return;

#ifdef RANGECHECK
    if ((unsigned)dc_x >= vid.width
        || dc_yl < 0
        || dc_yh >= vid.height)
        I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif

    dest = (short *) (ylookup[dc_yl] + columnofs[dc_x]);

    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl-centery)*fracstep;

    do
    {
        // DUMMY, just to see it's active
        *dest = (15<<10);
        //hicolormaps[ ((short*)dc_source)[(frac>>FRACBITS)&255]>>1 ];

        dest += vid.width;
        frac += fracstep;

    } while (--count);
}


//
//
//#ifndef USEASM
void R_DrawFuzzColumn_16 (void)
{
    int                 count;
    short*              dest;
    fixed_t             frac;
    fixed_t             fracstep;

    // Adjust borders. Low...
    if (!dc_yl)
        dc_yl = 1;

    // .. and high.
    if (dc_yh == rdraw_viewheight-1)
        dc_yh = rdraw_viewheight - 2;

    count = dc_yh - dc_yl;

    // Zero length.
    if (count < 0)
        return;


#ifdef RANGECHECK
    if ((unsigned)dc_x >= vid.width
        || dc_yl < 0 || dc_yh >= vid.height)
    {
        I_Error ("R_DrawFuzzColumn: %i to %i at %i",
                 dc_yl, dc_yh, dc_x);
    }
#endif


    // Does not work with blocky mode.
    dest = (short*) (ylookup[dc_yl] + columnofs[dc_x]);

    // Looks familiar.
    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl-centery)*fracstep;

    do
    {
        // Lookup framebuffer, and retrieve
        //  a pixel that is either one column
        //  left or right of the current one.
        // Add index from colormap to index.
        *dest = color8to16[colormaps[6*256+dest[fuzzoffset[fuzzpos]]]];

        // Clamp table lookup index.
        if (++fuzzpos == FUZZTABLE)
            fuzzpos = 0;

        dest += vid.width;

        frac += fracstep;
    } while (count--);
}
//#endif


//
//
//#ifndef USEASM
void R_DrawTranslucentColumn_16 (void)
{
    int                 count;
    short*              dest;
    fixed_t             frac;
    fixed_t             fracstep;

    //byte*               src;

    // check out coords for src*
    if((dc_yl<0)||(dc_x>=vid.width))
      return;

    count = dc_yh - dc_yl;
    if (count < 0)
        return;

#ifdef RANGECHECK
    if ((unsigned)dc_x >= vid.width
        || dc_yl < 0
        || dc_yh >= vid.height)
    {
        I_Error ( "R_DrawColumn: %i to %i at %i",
                  dc_yl, dc_yh, dc_x);
    }

#endif

    // FIXME. As above.
    //src  = ylookup[dc_yl] + columnofs[dc_x+2];
    dest = (short*) (ylookup[dc_yl] + columnofs[dc_x]);


    // Looks familiar.
    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl-centery)*fracstep;

    // Here we do an additional index re-mapping.
    do
    {
        *dest =( ((color8to16[dc_source[frac>>FRACBITS]]>>1) & 0x39ce) +
                 (*dest & HIMASK1) ) /*>> 1*/ & 0x7fff;

        dest += vid.width;
        frac += fracstep;
    } while (count--);
}
//#endif


//
//
//#ifndef USEASM
void R_DrawTranslatedColumn_16 (void)
{
    int                 count;
    short*              dest;
    fixed_t             frac;
    fixed_t             fracstep;

    count = dc_yh - dc_yl;
    if (count < 0)
        return;

#ifdef RANGECHECK
    if ((unsigned)dc_x >= vid.width
        || dc_yl < 0
        || dc_yh >= vid.height)
    {
        I_Error ( "R_DrawColumn: %i to %i at %i",
                  dc_yl, dc_yh, dc_x);
    }

#endif


    dest = (short *) (ylookup[dc_yl] + columnofs[dc_x]);

    // Looks familiar.
    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl-centery)*fracstep;

    // Here we do an additional index re-mapping.
    do
    {
        *dest = color8to16[ dc_colormap[dc_translation[dc_source[frac>>FRACBITS]]] ];
        dest += vid.width;

        frac += fracstep;
    } while (count--);
}
//#endif



// ==========================================================================
// SPANS
// ==========================================================================

//
//
//#ifndef USEASM
void R_DrawSpan_16 (void)
{
    fixed_t             xfrac;
    fixed_t             yfrac;
    short*              dest;
    int                 count;
    int                 spot;

#ifdef RANGECHECK
    if (ds_x2 < ds_x1
        || ds_x1<0
        || ds_x2>=vid.width
        || (unsigned)ds_y>vid.height)
    {
        I_Error( "R_DrawSpan: %i to %i at %i",
                 ds_x1,ds_x2,ds_y);
    }
#endif

    xfrac = ds_xfrac;
    yfrac = ds_yfrac;

    dest = (short *)(ylookup[ds_y] + columnofs[ds_x1]);

    // We do not check for zero spans here?
    count = ds_x2 - ds_x1;

    do
    {
        // Current texture index in u,v.
        spot = ((yfrac>>(16-6))&(63*64)) + ((xfrac>>16)&63);

        // Lookup pixel from flat texture tile,
        //  re-index using light/colormap.
        *dest++ = hicolormaps[ ((short*)ds_source)[spot]>>1 ];

        // Next step in u,v.
        xfrac += ds_xstep;
        yfrac += ds_ystep;

    } while (count--);
}
//#endif
