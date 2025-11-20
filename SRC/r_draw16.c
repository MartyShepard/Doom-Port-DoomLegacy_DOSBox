// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: r_draw16.c 897 2012-02-29 19:21:08Z wesleyjohnson $
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
  // include: color8.to16, hicolormaps

//hicolor composite (we're in 5:5:5)
#define HIMASK_11110   0x7bde     //mask out the lowest bit of R,G,B
#define HIMASK_01111   0x3def     //mask out the upper bit of R,G,B
#define HIMASK_01110   0x39ce	  //mask out the upper and lowest bit of R,G,B
// [WDJ] (c>>1 & HIMASK_01111) or ((c & HIMASK_11110)>>1) both
// round off the LSB, and prevent it from becoming the MSB of the adjacent color.

//  standard upto 128high posts column drawer
//
void R_DrawColumn_16 (void)
{
    int                 count;
    uint16_t*           dest;  // within screen buffer
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

    // Screen buffer destination address.
    // Use ylookup LUT to avoid multiply with ScreenWidth.
    // Use columnofs LUT for subwindows?
    dest = (uint16_t *) (ylookup[dc_yl] + columnofs[dc_x]);

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
        //*dest = *( (uint16_t *)dc_colormap + dc_source[(frac>>FRACBITS)&127] );
        *dest = hicolormaps[ ((uint16_t*)dc_source)[(frac>>FRACBITS)&127]>>1 ];

        dest += vid.ybytes;
        frac += fracstep;

    } while (--count);
}


//  LAME cutnpaste : same as R_DrawColumn_16 but wraps around 256
//  instead of 128 for the tall sky textures (256x240)
//
void R_DrawSkyColumn_16 (void)
{
    int                 count;
    uint16_t *          dest;
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

    dest = (uint16_t *) (ylookup[dc_yl] + columnofs[dc_x]);

    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl-centery)*fracstep;

    do
    {
        // DUMMY, just to see it's active
        *dest = (15<<10);
        //hicolormaps[ ((uint16_t *)dc_source)[(frac>>FRACBITS)&255]>>1 ];

        dest += vid.ybytes;
        frac += fracstep;

    } while (--count);
}


//
//
//#ifndef USEASM
void R_DrawFuzzColumn_16 (void)
{
    int                 count;
    uint16_t *          dest;
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
    dest = (uint16_t *) (ylookup[dc_yl] + columnofs[dc_x]);

    // Looks familiar.
    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl-centery)*fracstep;

    do
    {
        // Lookup framebuffer, and retrieve a pixel that is either one column
        //  left or right of the current one.
        // Add index from colormap to index.
	// Remap existing dest, modify position, dim through LIGHTTABLE[6].
//        *dest = color8.to16[reg_colormaps[6*256+dest[fuzzoffset[fuzzpos]]]];
        *dest = color8.to16[ reg_colormaps[ LIGHTTABLE(6) + dest[fuzzoffset[fuzzpos]]] ];

        // Clamp table lookup index.
        if (++fuzzpos == FUZZTABLE)
            fuzzpos = 0;

        dest += vid.ybytes;

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
    uint16_t*           dest;  // within screen buffer
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
    dest = (uint16_t *) (ylookup[dc_yl] + columnofs[dc_x]);


    // Looks familiar.
    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl-centery)*fracstep;

    // Here we do an additional index re-mapping.
    do
    {
        // Remap the existing dest color, dimming it through something?.
	// color is in 5,5,5 format
	// (c>>1 & HIMASK_01110) ==> multiply R,G,B by 0.5 and mask lsb
	// perhaps meant: (c>>1 & HIMASK_01111) ==> multiply R,G,B by 0.5
	// It takes an extra bit to add two values and divide by 2, so
	// need to prevent overflow into the adjacent field.
	// This divides by 2 first, and masks off the least bit.
	// An OR of selected bits, dependent upon a translucent mask, would be more stable.
        *dest =( ((color8.to16[dc_source[frac>>FRACBITS]]>>1) & HIMASK_01110) +
                 (*dest & HIMASK_11110) ) /*>> 1*/ & 0x7fff;

#if 0       
        // [WDJ] The above is not a true translucent, and there are 5 different
	// translucent effects in the 8 bit code.
	// 50/50 translucent
        *dest =((color8.to16[dc_source[frac>>FRACBITS]] & HIMASK_11110)>>1)
	       + ((*dest & HIMASK_11110)>>1);
	// 75/25 translucent
	register (uint16_t) nc = color8.to16[dc_source[frac>>FRACBITS]] & HIMASK_11110)>>1;
        *dest =(nc + ((nc & HIMASK_11110)>>1))  // 75%
	       + ((*dest & HIMASK_11110)>>2); // 25%
	// 25/75 translucent
	register (uint16_t) dc = (*dest & HIMASK_11110)>>1;
        *dest =((color8.to16[dc_source[frac>>FRACBITS]] & HIMASK_11110)>>2) // 25%
	       + (dc + ((dc & HIMASK_11110)>>1)); // 75%
        // by 256 entry translucent table (16x16) of uint16_t
	register unsigned int nc = color8.to16[dc_source[frac>>FRACBITS]];
	register unsigned int dc = *dest;
        nc <<=3; // must be 32 bit register or will lose high bits
        dc >>=1;
        *dest = dc16_translucent[(nc&0xF0)|(dc&0x0F)];
        nc >>=5;
        dc >>=5;
        *dest |= dc16_translucent[(nc&0xF0)|(dc&0x0F)];
        nc >>=5;
        dc >>=5;
        *dest |= dc16_translucent[(nc&0xF0)|(dc&0x0F)];
        // by 1024 entry translucent table (32x32) of uint16_t
	register unsigned int nc = color8.to16[dc_source[frac>>FRACBITS]];
	register unsigned int dc = *dest;
        nc <<=4; // must be 32 bit register or will lose high bits
        *dest = dc16_translucent[(nc&0x02E0)|(dc&0x001F)];
        nc >>=5;
        dc >>=5;
        *dest |= dc16_translucent[(nc&0x02E0)|(dc&0x001F)];
        nc >>=5;
        dc >>=5;
        *dest |= dc16_translucent[(nc&0x02E0)|(dc&0x001F)];
#endif
        dest += vid.ybytes;
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
    uint16_t*           dest;  // within screen buffer
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


    dest = (uint16_t *) (ylookup[dc_yl] + columnofs[dc_x]);

    // Looks familiar.
    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl-centery)*fracstep;

    // Here we do an additional index re-mapping.
    do
    {
        *dest = color8.to16[ dc_colormap[ dc_skintran[ dc_source[frac>>FRACBITS]]] ];
        dest += vid.ybytes;

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
    uint16_t *          dest;
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

    dest = (uint16_t *)(ylookup[ds_y] + columnofs[ds_x1]);

    // We do not check for zero spans here?
    count = ds_x2 - ds_x1;

    do
    {
        // Current texture index in u,v.
        spot = ((yfrac>>(16-6))&(63*64)) + ((xfrac>>16)&63);

        // Lookup pixel from flat texture tile,
        //  re-index using light/colormap.
        *dest++ = hicolormaps[ ((uint16_t *)ds_source)[spot]>>1 ];

        // Next step in u,v.
        xfrac += ds_xstep;
        yfrac += ds_ystep;

    } while (count--);
}
//#endif
