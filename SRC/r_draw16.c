// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: r_draw16.c 925 2012-06-08 00:07:49Z wesleyjohnson $
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

#ifdef HIGHCOLORMAPS
  // [WDJ] 2012-2-10 HIGHCOLORMAPS are not working, not setup.
#endif

//hicolor composite (ENABLE_DRAW15 is 5:5:5), (ENABLE_DRAW16 is 5:6:5)
// [WDJ] (c>>1 & mask_01111) or ((c & mask_11110)>>1) both
// round off the LSB, and prevent it from becoming the MSB of the adjacent color.

//  standard upto 128high posts column drawer
//
void R_DrawColumn_16 (void)
{
    int                 count;
    byte *              dest;  // within screen buffer
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
    dest = ylookup[dc_yl] + columnofs[dc_x];

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
#ifdef HIGHCOLORMAPS
        *(uint16_t*)dest = hicolormaps[ ((uint16_t*)dc_source)[(frac>>FRACBITS)&127]>>1 ];
#else
        *(uint16_t*)dest = color8.to16[ dc_colormap[ dc_source[(frac>>FRACBITS)&127] ] ];
#endif
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
    byte *              dest;
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

    dest = ylookup[dc_yl] + columnofs[dc_x];

    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl-centery)*fracstep;

    do
    {
        // DUMMY, just to see it's active
//        *dest = (15<<10);
#ifdef HIGHCOLORMAPS
        hicolormaps[ ((uint16_t *)dc_source)[(frac>>FRACBITS)&255]>>1 ];
#else
        *(uint16_t*)dest = color8.to16[ dc_colormap[dc_source[(frac >> FRACBITS) & 255]] ];
#endif
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
    byte*               dest;
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
    dest = ylookup[dc_yl] + columnofs[dc_x];

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
// FIXME, reads dest as palette
        *(uint16_t*)dest = color8.to16[ reg_colormaps[ LIGHTTABLE(6) + dest[fuzzoffset[fuzzpos]]] ];

        // Clamp table lookup index.
        if (++fuzzpos == FUZZTABLE)
            fuzzpos = 0;

        dest += vid.ybytes;
        frac += fracstep;
    } while (count--);
}
//#endif


void R_DrawShadeColumn_16(void)
{
    register int count;
    register byte *dest;
    register fixed_t frac;
    register fixed_t fracstep;

    // [WDJ] This check has been added to all the callers of colfunc().
    // check out coords for src*
//    if ((dc_yl < 0) || (dc_x >= vid.width))
//        return;

    count = dc_yh - dc_yl;
    if (count < 0)
        return;

#ifdef RANGECHECK
    if ((unsigned) dc_x >= rdraw_viewwidth || dc_yl < 0 || dc_yh >= rdraw_viewheight)
    {
        I_SoftError("R_DrawColumn: %i to %i at %i\n", dc_yl, dc_yh, dc_x);
        return;
    }
#endif

    // FIXME. As above.
    //src  = ylookup[dc_yl] + columnofs[dc_x+2];
    dest = ylookup[dc_yl] + columnofs[dc_x];

    // Looks familiar.
    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl - centery) * fracstep;

    // Here we do an additional index re-mapping.
    do
    {
        // apply shading/translucent with existing showing through
        // Remap the existing dest color, dimming it through source LIGHTTABLE.
//        *dest = *(reg_colormaps + (dc_source[frac >> FRACBITS] << 8) + (*dest));
//        *dest = reg_colormaps[ LIGHTTABLE(dc_source[frac >> FRACBITS]) + (*dest) ];
        register byte sc = reg_colormaps[ LIGHTTABLE(dc_source[frac >> FRACBITS]) + 4 ]; // white
#if 1
	// 50/50 translucent
	register uint16_t dc = *(uint16_t*)dest;
	register uint16_t nc = color8.to16[sc];
        *(uint16_t*)dest=
	    ((nc & mask_11110)>>1)  // 50%
	    + ((dc & mask_11110)>>1);  // 50%
#endif
#if 0
	// 25/75 translucent
	register uint16_t dc = (*(uint16_t*)dest & mask_11110)>>1;
	register uint16_t nc = color8.to16[sc];
        *(uint16_t*)dest=
	    ((nc & mask_11100)>>2) // 25%
            + (dc + ((dc & mask_11110)>>1)); // 75%
#endif

        dest += vid.ybytes;
        frac += fracstep;
    }
    while (count--);
}


//
//
//#ifndef USEASM
void R_DrawTranslucentColumn_16 (void)
{
    int                 count;
    byte*               dest;  // within screen buffer
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
    dest = ylookup[dc_yl] + columnofs[dc_x];


    // Looks familiar.
    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl-centery)*fracstep;

    // Here we do an additional index re-mapping.
        // Remap the existing dest color, dimming it through something?.
	// color is in 5,5,5 format
	// (c>>1 & mask_01110) ==> multiply R,G,B by 0.5 and mask lsb
	// perhaps meant: (c>>1 & mask_01111) ==> multiply R,G,B by 0.5
	// It takes an extra bit to add two values and divide by 2, so
	// need to prevent overflow into the adjacent field.
	// This divides by 2 first, and masks off the least bit.
	// An OR of selected bits, dependent upon a translucent mask, would be more stable.
        // *dest = dc_colormap[ dc_translucentmap[ (dc_source[frac >> FRACBITS] << 8) + (*dest) ]];
    switch( dc_translucent_index )
    {
     case TRANSLU_more: // 20 80  puffs
        do
        {
	    // 25/75 translucent
	    register uint16_t dc = (*(uint16_t*)dest & mask_11110)>>1;
	    register uint16_t nc = color8.to16[dc_source[frac>>FRACBITS]];
//	    register uint16_t nc = color8.to16[dc_colormap[dc_source[frac>>FRACBITS]]];
	    *(uint16_t*)dest=
	     ((nc & mask_11100)>>2) // 25%
	     + (dc + ((dc & mask_11110)>>1)); // 75%
	    dest += vid.ybytes;
	    frac += fracstep;
	} while (count--);
        break;
     case TRANSLU_hi:   // 10 90  blur effect
        do
        {
	    // 15/85 translucent
	    register uint16_t dc = *(uint16_t*)dest;
	    register uint16_t dc85 = dc - ((dc & mask_11000)>>3);
	    register uint16_t nc = color8.to16[dc_colormap[dc_source[frac>>FRACBITS]]];
//	    register uint16_t nc = color8.to16[dc_source[frac>>FRACBITS]];
	    *(uint16_t*)dest=
	     ((nc & mask_11000)>>3) // 15%
	     + dc85; // 85%
	    dest += vid.ybytes;
	    frac += fracstep;
	} while (count--);
        break;
     case TRANSLU_med:  // sprite 50 backg 50
     default:
        do
        {
	    // 50/50 translucent
	    register uint16_t dc = *(uint16_t*)dest;
	    register uint16_t nc = color8.to16[dc_source[frac>>FRACBITS]];
//	    register uint16_t nc = color8.to16[dc_colormap[dc_source[frac>>FRACBITS]]];
	    *(uint16_t*)dest=
	     ((nc & mask_11110)>>1)  // 50%
	     + ((dc & mask_11110)>>1);  // 50%
	    dest += vid.ybytes;
	    frac += fracstep;
	} while (count--);
        break;
     case TRANSLU_fire: // 50 50 but brighter for fireballs, shots..
     case TRANSLU_fx1:  // 50 50 brighter some colors, else opaque for torches
        do
        {
	    // 50/50 translucent
	    // *dest = dc_colormap[ dc_translucentmap[ (dc_source[frac >> FRACBITS] << 8) + (*dest) ]];
	    register uint16_t dc = *(uint16_t*)dest;
//	    register uint16_t nc = color8.to16[dc_source[frac>>FRACBITS]];
	    register uint16_t nc = color8.to16[dc_colormap[dc_translucentmap[dc_source[frac>>FRACBITS]<<8]]];
	    *(uint16_t*)dest=
	     ((nc & mask_11110)>>1)  // 50%
	     + ((dc & mask_11110)>>1);  // 50%
	    dest += vid.ybytes;
	    frac += fracstep;
	} while (count--);
        break;
     case TRANSLU_75: // 75 25
        do
        {
	    // 75/25 translucent
	    register uint16_t dc = *(uint16_t*)dest;
	    register uint16_t nc = (color8.to16[dc_source[frac>>FRACBITS]] & mask_11110)>>1;
	    *(uint16_t*)dest=
	     (nc + ((nc & mask_11110)>>1)) // 75%
	     + ((dc & mask_11100)>>2); // 25%
	    dest += vid.ybytes;
	    frac += fracstep;
	} while (count--);
        break;
    }
#if 0
	// original
        // [WDJ] The original is not a balanced translucent, and there are 5 different
	// translucent effects in the 8 bit code.
        *(uint16_t*)dest=
	    ( ((color8.to16[dc_source[frac>>FRACBITS]]>>1) & mask_01110)
	    + (*(uint16_t*)dest & mask_11110) ) /*>> 1*/ & 0x7fff;
#endif
#if 0
	// 75/25 translucent
	register uint16_t dc = *(uint16_t*)dest;
	register uint16_t nc = (color8.to16[dc_source[frac>>FRACBITS]] & mask_11110)>>1;
//	register uint16_t nc = (color8.to16[dc_colormap[dc_source[frac>>FRACBITS]]] & mask_11110)>>1;
        *(uint16_t*)dest=
            (nc + ((nc & mask_11110)>>1))  // 75%
            + ((dc & mask_11100)>>2); // 25%
#endif
#if 0
        // by 256 entry translucent table (16x16) of uint16_t
	register unsigned int nc = color8.to16[dc_source[frac>>FRACBITS]];
	register unsigned int dc = *(uint16_t*)dest;
        nc <<=3; // must be 32 bit register or will lose high bits
        dc >>=1;
        *(uint16_t*)dest= dc16_translucent[(nc&0xF0)|(dc&0x0F)];
        nc >>=5;
        dc >>=5;
	*(uint16_t*)dest|= dc16_translucent[(nc&0xF0)|(dc&0x0F)];
        nc >>=5;
        dc >>=5;
	*(uint16_t*)dest|= dc16_translucent[(nc&0xF0)|(dc&0x0F)];
#endif
#if 0
        // by 1024 entry translucent table (32x32) of uint16_t
	register unsigned int nc = color8.to16[dc_source[frac>>FRACBITS]];
	register unsigned int dc = *(uint16_t*)dest;
        nc <<=4; // must be 32 bit register or will lose high bits
        *(uint16_t*)dest = dc16_translucent[(nc&0x02E0)|(dc&0x001F)];
        nc >>=5;
        dc >>=5;
        *(uint16_t*)dest |= dc16_translucent[(nc&0x02E0)|(dc&0x001F)];
        nc >>=5;
        dc >>=5;
        *(uint16_t*)dest |= dc16_translucent[(nc&0x02E0)|(dc&0x001F)];
#endif
}
//#endif



// transparent with skin translations
// Although the vissprite has capability for any transparency,
// this is only called with TRANSLU_hi
void R_DrawTranslatedTranslucentColumn_16(void)
{
#if 1
    R_DrawTranslucentColumn_16();
#else
    register int count;
    register byte *dest;
    register fixed_t frac;
    register fixed_t fracstep;

    count = dc_yh - dc_yl;

    if (count < 0)     // Zero length, column does not exceed a pixel.
        return;

    dest = ylookup[dc_yl] + columnofs[dc_x];

    // Looks familiar.
    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl - centery) * fracstep;

    // Here we do an additional index re-mapping.
    switch( dc_translucent_index )
    {
     case TRANSLU_hi:   // 10 90  blur effect
        do
        {
	    // Makes player visible in dark rooms, but so does draw8 version
	    // *dest = dc_colormap[ dc_translucentmap[ (dc_colormap[dc_skintran[dc_source[frac >> FRACBITS]]] << 8) + (*dest) ]];

	    // 15/85 translucent
	    register uint16_t dc = *(uint16_t*)dest;
	    register uint16_t dc85 = dc - ((dc & mask_11000)>>3);
//	    register uint16_t nc = color8.to16[dc_skintran[dc_source[frac>>FRACBITS]]];
	    register uint16_t nc = color8.to16[dc_colormap[dc_skintran[dc_source[frac>>FRACBITS]]]];
	    *(uint16_t*)dest=
	     ((nc & mask_11000)>>3) // 15%
	     + dc85; // 85%
	    dest += vid.ybytes;
	    frac += fracstep;
	} while (count--);
        break;
     case TRANSLU_med:  // sprite 50 backg 50
     default:
        // Seems to be unused
        do
        {
	    // 50/50 translucent
	    register uint16_t dc = *(uint16_t*)dest;
//	    register uint16_t nc = color8.to16[dc_skintran[dc_source[frac>>FRACBITS]]];
	    register uint16_t nc = color8.to16[dc_colormap[dc_skintran[dc_source[frac>>FRACBITS]]]];
	    *(uint16_t*)dest=
	     ((nc & mask_11110)>>1)  // 50%
	     + ((dc & mask_11110)>>1);  // 50%
	    dest += vid.ybytes;
	    frac += fracstep;
	} while (count--);
        break;
    }
#endif
}


//
//
//#ifndef USEASM
void R_DrawTranslatedColumn_16 (void)
{
    int                 count;
    byte*               dest;  // within screen buffer
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


    dest = ylookup[dc_yl] + columnofs[dc_x];

    // Looks familiar.
    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl-centery)*fracstep;

    // Here we do an additional index re-mapping.
    do
    {
#ifdef HIGHCOLORMAPS
        *dest = hicolormaps[ ((uint16_t*)dc_skintran[ dc_source[frac>>FRACBITS]]];
#else
        *(uint16_t*)dest = color8.to16[ dc_colormap[ dc_skintran[ dc_source[frac>>FRACBITS]]] ];
#endif
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
    byte*               dest;
    int                 count;

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

    dest = ylookup[ds_y] + columnofs[ds_x1];

    // We do not check for zero spans here?
    count = ds_x2 - ds_x1;

    do
    {
#ifdef HICOLORMAPS
        // Current texture index in u,v.
        register int spot = ((yfrac>>(16-6))&(63*64)) + ((xfrac>>16)&63);

        // Lookup pixel from flat texture tile,
        //  re-index using light/colormap.
        *(uint16_t*)dest = hicolormaps[ ((uint16_t *)ds_source)[spot]>>1 ];

#else
        xfrac &= 0x3fFFff;
//        *(uint16_t*)dest = color8.to16[ ds_colormap[ds_source[((yfrac >> (16 - 6)) & (0x3f << 6)) | (xfrac >> 16)]] ];
        *(uint16_t*)dest = color8.to16[ ds_colormap[ds_source[((yfrac >> (16 - flatsubtract)) & flatmask) | (xfrac >> 16)]] ];
#endif
        dest += 2;
        // Next step in u,v.
        xfrac += ds_xstep;
        yfrac += ds_ystep;
    } while (count--);
}
//#endif


void R_DrawTranslucentSpan_16(void)
{
    fixed_t xfrac;
    fixed_t yfrac;
    fixed_t xstep;
    fixed_t ystep;
    byte *dest;
    int count;

#ifdef RANGECHECK
    if (ds_x2 < ds_x1 || ds_x1 < 0 || ds_x2 >= rdraw_viewwidth || (unsigned) ds_y > rdraw_viewheight)
    {
        I_SoftError("R_DrawSpan: %i to %i at %i\n", ds_x1, ds_x2, ds_y);
        return;
    }
#endif

    xfrac = ds_xfrac & ((flatsize << FRACBITS) - 1);
    yfrac = ds_yfrac;

    dest = ylookup[ds_y] + columnofs[ds_x1];

    // We do not check for zero spans here?
    count = ds_x2 - ds_x1 + 1;

    xstep = ds_xstep;
    ystep = ds_ystep;

    do
    {
        // Current texture index in u,v.
        // Lookup pixel from flat texture tile,
#ifdef HICOLORMAPS
        // Current texture index in u,v.
        register int spot = ((yfrac>>(16-6))&(63*64)) + ((xfrac>>16)&63);

        // Lookup pixel from flat texture tile,
        //  re-index using light/colormap.
        register uint16_t nc = hicolormaps[ ((uint16_t *)ds_source)[spot]>>1 ];

#else
        xfrac &= 0x3fFFff;
	// 50/50 translucent
	register uint16_t nc = color8.to16[ ds_colormap[ds_source[((yfrac >> (16 - flatsubtract)) & (flatmask)) | (xfrac >> 16)]] ];
#endif
	register uint16_t dc = *(uint16_t*)dest;
        *(uint16_t*)dest=
	    ((nc & mask_11110)>>1)  // 50%
	    + ((dc & mask_11110)>>1);  // 50%
        dest+=2;

        // Next step in u,v.
        xfrac += xstep;
        yfrac += ystep;
        xfrac &= ((flatsize << FRACBITS) - 1);
    }
    while (--count);
}

#define FOGCOLOR

void R_DrawFogSpan_16(void)
{
    byte * dest;
    uint16_t fogcolor = color8.to16[ ds_colormap[110] ]; // grays 80..111
    uint16_t fogcolor4 = (fogcolor & mask_11100)>>2;
    uint16_t fogcolor2 = (fogcolor & mask_11000)>>3;
    uint16_t fogcolor3 = ((fogcolor2 + fogcolor4) & mask_11110)>>1;
//    uint16_t fogcolor1 = (fogcolor4 & mask_11100)>>2;

    unsigned count;

//    colormap = ds_colormap;
    dest = ylookup[ds_y] + columnofs[ds_x1];
    count = ds_x2 - ds_x1 + 1;

#if 0   
    // partial unrolled loop, for speed
    while (count >= 4)
    {
        dest[0] = colormap[dest[0]];

        dest[1] = colormap[dest[1]];

        dest[2] = colormap[dest[2]];

        dest[3] = colormap[dest[3]];

        dest += 4;
        count -= 4;
    }
    // leftover, count = 0..3
#endif   

    while (count--)
    {
	// 25/75 translucent
        register uint16_t dc = *(uint16_t*)dest;
        register uint16_t dc2 = (dc & mask_11110)>>1;
        *(uint16_t*)dest=
	    fogcolor3
            + (dc2 + ((dc2 & mask_11110)>>1)); // 75%
        dest+=2;
    }
}

//SoM: Fog wall.
// Used for Legacy linetype 302, walls of 3D fog in tagged
// Used for Legacy linetype 283, fog sheet
void R_DrawFogColumn_16(void)
{
    int count;
    byte * dest;
#ifdef FOGCOLOR
   // fogcolor blur needs to be kept as 8 bit or else the blur is inadequate
static uint16_t fogcolor_r8 = 0x1000;
static uint16_t fogcolor_g8 = 0x0080;
static uint16_t fogcolor_b8 = 0x0002<<3;
    uint32_t fc, fc2;  // uint16_t + 3
    // fog_index 0.. column height
    fc = color8.to16[ dc_colormap[ dc_source[fog_index] ]];
//    fc = color8.to16[ dc_colormap[ 110 ]];
    int fogb1 = fog_bltic>>2;  // 8 
    int fogb2 = 8 - fogb1;  // fade proportionality
    fc2 = color8.to16[ dc_colormap[ dc_source[((fog_index==0)?fog_col_length:fog_index)-1] ]];
    register uint32_t fr1 = (((fc & mask_r) * fogb1) + ((fc2 & mask_r) * fogb2)) >> 3;
    register uint32_t fg1 = (((fc & mask_g) * fogb1) + ((fc2 & mask_g) * fogb2)) >> 3;
    register uint32_t fb1 = (((fc & mask_b) * fogb1) + ((fc2 & mask_b) * fogb2));  // kept << 3
    if( fog_init )
    {
        // init blur
        fogcolor_r8 = fr1;
        fogcolor_g8 = fg1;
        fogcolor_b8 = fb1;
        fog_init = 0;
    }
    else
    {
        // blur
        fogcolor_r8 = ((((uint32_t)fogcolor_r8)*31) + fr1) >> 5;
        fogcolor_g8 = ((((uint32_t)fogcolor_g8)*31) + fg1) >> 5;
	fogcolor_b8 = ((((uint32_t)fogcolor_b8)*31) + fb1) >> 5;
    }
    // limit each color blur to 8 bits so grays stay pure
    fogcolor_r8 &= (mask_r | (mask_r>>3));
    fogcolor_g8 &= (mask_g | (mask_g>>3));
    // the working fog color for this column
    uint32_t fogcolor_rb = (fogcolor_r8 & mask_r) | ((fogcolor_b8>>3) & mask_b);
    uint32_t fogcolor_g = fogcolor_g8 & mask_g;
#endif

    count = dc_yh - dc_yl;

    // Zero length, column does not exceed a pixel.
    if (count < 0)
        return;

#ifdef RANGECHECK
    // [WDJ] Draw window is actually rdraw_viewwidth and rdraw_viewheight
    if ((unsigned) dc_x >= rdraw_viewwidth || dc_yl < 0 || dc_yh >= rdraw_viewheight)
    {
        I_SoftError("R_DrawFogColumn: %i to %i at %i\n", dc_yl, dc_yh, dc_x);
        return;
    }
#endif

    // Framebuffer destination address.
    // Use ylookup LUT to avoid multiply with ScreenWidth.
    // Use columnofs LUT for subwindows?
    dest = ylookup[dc_yl] + columnofs[dc_x];

#ifdef FOGCOLOR
    do
    {
	// faint color + dark
        register uint16_t d16 = *(uint16_t*)dest;
	register uint32_t dc0 = d16 & mask_rb;  // for overlapped execution
	register uint32_t dc1 = d16 & mask_g;
	dc0 = ((dc0 + dc0 + fogcolor_rb) >> 2) & mask_rb;
	dc1 = ((dc1 + dc1 + fogcolor_g) >> 2) & mask_g;
	*(uint16_t*)dest = dc0 | dc1;
        dest += vid.ybytes;
    }
    while (count--);
#else
    fixed_t             frac;
    fixed_t             fracstep;

    // Determine scaling,
    //  which is the only mapping to be done.
    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl-centery)*fracstep;

#if 1
    uint16_t fogcolor = color8.to16[ dc_colormap[110] ]; // grays 80..111
//    uint16_t fogcolor4 = (fogcolor & mask_11100)>>2;
    uint16_t fogcolor2 = (fogcolor & mask_11000)>>3;
//    uint16_t fogcolor3 = ((fogcolor2 + fogcolor4) & mask_11110)>>1;
//    uint16_t fogcolor1 = (fogcolor4 & mask_11100)>>2;
    do
    {
        register uint16_t dc = *(uint16_t*)dest;
	// faint texture + dark
	register byte bc = dc_source[frac>>FRACBITS];
	register uint16_t nc = color8.to16[bc];
        register uint16_t dc2 = (dc & mask_11110)>>1;
//        register uint16_t fg = ( bc < 0x7F )? fogcolor1 : fogcolor2;
	    *(uint16_t*)dest = fogcolor2 + ((nc & mask_11000)>>3) 
		 + (dc2 + ((dc2 & mask_11110)>>1)); // 75%
        frac += fracstep;
        dest += vid.ybytes;
    }
    while (count--);
#endif
#if 0
    uint16_t fogcolor = color8.to16[ dc_colormap[110] ]; // grays 80..111
    uint16_t fogcolor4 = (fogcolor & mask_11100)>>2;
    uint16_t fogcolor2 = (fogcolor & mask_11000)>>3;
    uint16_t fogcolor3 = ((fogcolor2 + fogcolor4) & mask_11110)>>1;
    uint16_t fogcolor1 = (fogcolor4 & mask_11100)>>2;
    do
    {
        register uint16_t dc = *(uint16_t*)dest;
	// texture is alpha, no color, lumpy
	register byte nc = dc_source[frac>>FRACBITS];
        if ( nc < 0x7F )
        {
	    register uint16_t dc2 = (dc & mask_11110)>>1;
	    register uint16_t fg = ( nc < 0x3F )? fogcolor4 : fogcolor3;
	    *(uint16_t*)dest = fg
		 + (dc2 + ((dc2 & mask_11110)>>1)); // 75%
	}
        else
        {
	    register uint16_t dc85 = dc - ((dc & mask_11000)>>3);
	    register uint16_t fg = ( nc < 0xBF )? fogcolor2 : fogcolor1;
	        *(uint16_t*)dest= fg
		  + dc85; // 85%
	}
        frac += fracstep;
        dest += vid.ybytes;
    }
    while (count--);
#endif
#if 0       
    do
    {
	// 25/75 translucent
	// Colors the fog
        register uint16_t dc = *(uint16_t*)dest;
        register uint16_t dc2 = (dc & mask_11110)>>1;
	register uint16_t nc = color8.to16[dc_source[frac>>FRACBITS]];
        *(uint16_t*)dest=
	    ((nc & mask_11100)>>2) // 25%
            + (dc2 + ((dc2 & mask_11110)>>1)); // 75%
        frac += fracstep;
        dest += vid.ybytes;
    }
    while (count--);
#endif
#if 0
    do
    {
	// subtractive
        register uint16_t dc = *(uint16_t*)dest;
        register uint16_t dc2 = (dc & mask_11110)>>1;
	register uint16_t nc = color8.to16[dc_source[frac>>FRACBITS]];
        *(uint16_t*)dest=
            (dc2 + ((dc2 & mask_11110)>>1)); // 75%
	    - ((nc & mask_11100)>>2) // 25%
        frac += fracstep;
        dest += vid.ybytes;
    }
    while (count--);
#endif
#if 0
    uint16_t fogcolor = color8.to16[ dc_colormap[110] ]; // grays 80..111
    uint16_t fogcolor4 = (fogcolor & mask_11100)>>2;
    uint16_t fogcolor2 = (fogcolor & mask_11000)>>3;
    uint16_t fogcolor3 = ((fogcolor2 + fogcolor4) & mask_11110)>>1;
    do
    {
	// ignore texture
	// Closest to 8bit PAL version
        register uint16_t dc = *(uint16_t*)dest;
        register uint16_t dc2 = (dc & mask_11110)>>1;
        *(uint16_t*)dest=
	    fogcolor3
            + (dc2 + ((dc2 & mask_11110)>>1)); // 75%
        frac += fracstep;
        dest += vid.ybytes;
    }
    while (count--);
#endif
#if 0
    uint16_t fogcolor = color8.to16[ dc_colormap[110] ]; // grays 80..111
    uint16_t fogcolor2 = (fogcolor & mask_11000)>>3;
    do
    {
	// ignore texture, too light
        register uint16_t dc = *(uint16_t*)dest;
        register uint16_t dc85 = dc - ((dc & mask_11000)>>3);
          *(uint16_t*)dest= fogcolor2
	   + dc85; // 85%
        dest += vid.ybytes;
    }
    while (count--);
#endif
#endif
}

