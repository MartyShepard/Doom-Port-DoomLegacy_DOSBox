// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: r_draw24.c 902 2012-02-29 07:30:00Z wesleyjohnson $
//
// Copyright (C) 2012-2012 by DooM Legacy Team.
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
// DESCRIPTION:
//      24bpp span/column drawer functions
//
//  NOTE: no includes because this is included as part of r_draw.c
//
//-----------------------------------------------------------------------------


// ==========================================================================
// COLUMNS
// ==========================================================================

// r_data.c
  // include: color8.to32, hicolormaps

#ifdef HIGHCOLORMAPS
  // [WDJ] 2012-2-10 HIGHCOLORMAPS are not working, not setup.
#endif

// 24bpp composite (ENABLE_DRAW24) is 8:8:8
// color_8.to32 color is aligned with pix24 for speed

//  standard upto 128high posts column drawer
//
void R_DrawColumn_24 (void)
{
    unsigned int heightmask = dc_texheight - 1;
    int     count;
    byte *  dest;  // within screen buffer
    fixed_t frac;
    fixed_t fracstep;
    pixelunion32_t c32;

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
    dest = ylookup[dc_yl] + columnofs[dc_x];

    // Determine scaling, which is the only mapping to be done.
    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl-centery)*fracstep;

    if( dc_texheight & heightmask )
    {
        // Odd size texture, use texheight
        fixed_t texheight = dc_texheight << FRACBITS;
        // Inner loop that does the actual texture mapping.
        do
        {
	    // Re-map color indices from wall texture column
	    //  using a lighting/special effects LUT.
#ifdef HIGHCOLORMAPS
	    c32.ui32 = hicolormaps[ ((uint16_t*)dc_source)[(frac>>FRACBITS)]>>1 ];
#else
	    c32.ui32 = color8.to32[ dc_colormap[ dc_source[(frac>>FRACBITS)] ] ];
#endif
	    *(pixel24_t*)dest = c32.pix24;
	    dest += vid.ybytes;
	    frac += fracstep;
	    if( frac >= texheight )
	        frac -= texheight;
	} while (--count);
    }
    else
    {
        // Inner loop that does the actual texture mapping.
        do
        {
	    // Re-map color indices from wall texture column
	    //  using a lighting/special effects LUT.
#ifdef HIGHCOLORMAPS
	    c32.ui32 = hicolormaps[ ((uint16_t*)dc_source)[(frac>>FRACBITS)&heightmask]>>1 ];
#else
	    c32.ui32 = color8.to32[ dc_colormap[ dc_source[(frac>>FRACBITS)&heightmask] ] ];
#endif
	    *(pixel24_t*)dest = c32.pix24;
	    dest += vid.ybytes;
	    frac += fracstep;
	} while (--count);
    }
}


//  Same as R_DrawColumn_24 but wraps around 256
//  instead of 128 for the tall sky textures (256x240)
//
void R_DrawSkyColumn_24 (void)
{
    int     count;
    byte *  dest;  // within screen buffer
    fixed_t frac;
    fixed_t fracstep;
    pixelunion32_t c32;

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
#ifdef HIGHCOLORMAPS
        c32.ui32 = hicolormaps[ ((uint16_t*)dc_source)[(frac>>FRACBITS)&255]>>1 ];
#else
        c32.ui32 = color8.to32[ dc_colormap[ dc_source[(frac>>FRACBITS)&255] ] ];
#endif
        *(pixel24_t*)dest = c32.pix24;
        dest += vid.ybytes;
        frac += fracstep;
    } while (--count);
}


void R_DrawFuzzColumn_24 (void)
{
    int     count;
    byte*   dest;

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

    do
    {
        // Retrieve a pixel that is either one column left or right
        // of the dest, dim it 8%, and write it as dest.
        // 8bpp: *dest = reg_colormaps[6*256+dest[fuzzoffset[fuzzpos]]];
        register pixel24_t * src = (pixel24_t*)(dest + fuzzoffset[fuzzpos]);
        ((pixel24_t*)dest)->b = src->b - (src->b >> 3);
        ((pixel24_t*)dest)->g = src->g - (src->g >> 3);
        ((pixel24_t*)dest)->r = src->r - (src->r >> 3);

        dest += vid.ybytes;
        // Clamp table lookup index.
        if (++fuzzpos == FUZZTABLE)
            fuzzpos = 0;
    } while (count--);
}


// To draw FF_SMOKESHADE things, sprite is an alpha channel
void R_DrawShadeColumn_24(void)
{
    fixed_t texheight = dc_texheight << FRACBITS;  // any texture size
    int count;
    register byte *dest;
    register fixed_t frac;
    register fixed_t fracstep;
    unsigned int  alpha;

    // [WDJ] Source check has been added to all the callers of colfunc().

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
    dest = ylookup[dc_yl] + columnofs[dc_x];

    // Looks familiar.
    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl - centery) * fracstep;
    if (texheight > 0)  // hangs when texheight==0
    {
        // From Boom, to fix the odd frac
        if (frac < 0)
	    while ((frac += texheight) < 0);
        else
	    while (frac >= texheight)  frac -= texheight;
    }

    do
    {
        // 8bpp: *dest = reg_colormaps[ LIGHTTABLE(dc_source[frac >> FRACBITS]) + (*dest) ];
	// [WDJ] The source determines light level.
	// light level values are 0..NUMCOLORMAPS-1, which is 0..32
//        alpha = dc_source[frac >> FRACBITS] >> 3;  // reduce 0..255 to 0..32
        alpha = dc_source[frac >> FRACBITS];
        register pixel24_t * p24 = (pixel24_t*)dest;
        p24->b = (p24->b * alpha) >> 5;
        p24->g = (p24->g * alpha) >> 5;
        p24->r = (p24->r * alpha) >> 5;

        dest += vid.ybytes;
        frac += fracstep;
        if( frac >= texheight )
	    frac -= texheight;
    }
    while (count--);
}


void R_DrawTranslucentColumn_24 (void)
{
    fixed_t texheight = dc_texheight << FRACBITS;  // any texture size
    int     count;
    byte*   dest;  // within screen buffer
    fixed_t frac;
    fixed_t fracstep;
    pixelunion32_t c32;

    // [WDJ] Source check has been added to all the callers of colfunc().

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
    if (texheight > 0)  // hangs when texheight==0
    {
        // From Boom, to fix the odd frac
        if (frac < 0)
	    while ((frac += texheight) < 0);
        else
	    while (frac >= texheight)  frac -= texheight;
    }

    // 8bpp: *dest = dc_colormap[ dc_translucentmap[ (dc_source[frac >> FRACBITS] << 8) + (*dest) ]];
    switch( dc_translucent_index )
    {
     case TRANSLU_more: // 20 80  puffs, Linedef 285
        do
        {
	    // 25/75 translucent
#ifdef HIGHCOLORMAPS
	    c32.ui32 = hicolormaps[ dc_source[frac>>FRACBITS] ];
#else
	    c32.ui32 = color8.to32[dc_colormap[dc_source[frac>>FRACBITS]]];
//	    c32.ui32 = color8.to32[dc_source[frac>>FRACBITS]];
#endif
	    register uint16_t dc0, dc1, dc2; // for overlapped execution
	    register pixel24_t * p24 = (pixel24_t*)dest;
	    dc0 = p24->b;
	    p24->b = (dc0 + dc0 + dc0 + c32.pix24.b) >> 2;
	    dc1 = p24->g;
	    p24->g = (dc1 + dc1 + dc1 + c32.pix24.g) >> 2;
	    dc2 = p24->r;
	    p24->r = (dc2 + dc2 + dc2 + c32.pix24.r) >> 2;
	    dest += vid.ybytes;
	    frac += fracstep;
	    if( frac >= texheight )
	        frac -= texheight;
	} while (count--);
        break;
     case TRANSLU_hi:   // 10 90  blur effect, Linedef 286
        do
        {
	    // 15/85 translucent
#ifdef HIGHCOLORMAPS
	    c32.ui32 = hicolormaps[ dc_source[frac>>FRACBITS] ];
#else
	    c32.ui32 = color8.to32[dc_colormap[dc_source[frac>>FRACBITS]]];
//	    c32.ui32 = color8.to32[dc_source[frac>>FRACBITS]];
#endif
	    register uint16_t dc0, dc1, dc2; // for overlapped execution
	    register pixel24_t * p24 = (pixel24_t*)dest;
	    dc0 = p24->b;
	    p24->b = ((dc0<<3) - dc0 + c32.pix24.b) >> 3;
	    dc1 = p24->g;
	    p24->g = ((dc1<<3) - dc1 + c32.pix24.g) >> 3;
	    dc2 = p24->r;
	    p24->r = ((dc2<<3) - dc2 + c32.pix24.r) >> 3;
	    dest += vid.ybytes;
	    frac += fracstep;
	    if( frac >= texheight )
	        frac -= texheight;
	} while (count--);
        break;
     case TRANSLU_med:  // sprite 50 backg 50, Linedef 260, 284, 300
     default:
        do
        {
	    // 50/50 translucent
#ifdef HIGHCOLORMAPS
	    c32.ui32 = hicolormaps[ dc_source[frac>>FRACBITS] ];
#else
	    c32.ui32 = color8.to32[dc_colormap[dc_source[frac>>FRACBITS]]];
//	    c32.ui32 = color8.to32[dc_source[frac>>FRACBITS]];
#endif
	    register uint16_t dc0, dc1, dc2; // for overlapped execution
	    register pixel24_t * p24 = (pixel24_t*)dest;
	    dc0 = p24->b;
	    p24->b = (dc0 + c32.pix24.b) >> 1;
	    dc1 = p24->g;
	    p24->g = (dc1 + c32.pix24.g) >> 1;
	    dc2 = p24->r;
	    p24->r = (dc2 + c32.pix24.r) >> 1;
	    dest += vid.ybytes;
	    frac += fracstep;
	    if( frac >= texheight )
	        frac -= texheight;
	} while (count--);
        break;
     case TRANSLU_fire: // Linedef 287
        // 50 50 but brighter for fireballs, shots
#define TFIRE_OPTION  1
        do
        {
	    // 50/50 translucent with modifications
	    register int sb = dc_source[frac>>FRACBITS];
#if TFIRE_OPTION==4
//	    sb = dc_translucentmap[ (sb << 8) + 80 ]; // grays 80..111; // faded
//	    sb = dc_translucentmap[ (sb << 8) + 110 ]; // grays 80..111;
	    sb = dc_translucentmap[ (sb << 8) ]; // black
#endif
#ifdef HIGHCOLORMAPS
	    c32.ui32 = hicolormaps[ sb  ];
#else
//	    c32.ui32 = color8.to32[dc_colormap[ sb ]];
	    c32.ui32 = color8.to32[ sb ];
#endif
#if TFIRE_OPTION==1
	    // dark background is enhanced before avg, to give light add effect
	    register pixelunion32_t sh32;
	    sh32.ui32 = (c32.ui32 >> 1) & 0x70707070;  // tint
//	    sh32.ui32 = (c32.ui32 >> 1) & 0x7F7F7F7F;  // heavy
	    register uint16_t dc0, dc1, dc2; // for overlapped execution
	    register pixel24_t * p24 = (pixel24_t*)dest;
	    dc0 = p24->b;
	    p24->b = ((dc0 | sh32.pix32.b) + c32.pix24.b) >> 1;
	    dc1 = p24->g;
	    p24->g = ((dc1 | sh32.pix32.g) + c32.pix24.g) >> 1;
	    dc2 = p24->r;
	    p24->r = ((dc2 | sh32.pix32.r) + c32.pix24.r) >> 1;
#endif
#if TFIRE_OPTION==2
	    // Bright and nearly opaque
	    // additive light (not average)
	    register uint16_t dc0, dc1, dc2; // for overlapped execution
	    register pixel24_t * p24 = (pixel24_t*)dest;
	    dc0 = p24->b;
	    p24->b = (dc0 | c32.pix24.b);
	    dc1 = p24->g;
	    p24->g = (dc1 | c32.pix24.g);
	    dc2 = p24->r;
	    p24->r = (dc2 | c32.pix24.r);
#endif
	    dest += vid.ybytes;
	    frac += fracstep;
	    if( frac >= texheight )
	        frac -= texheight;
	} while (count--);
        break;
     case TRANSLU_fx1:  // Linedef 288
        // 50 50 brighter some colors, else opaque for torches
        // fire translucent texture
#define FX1_OPTION 1
        do
        {
	    // 50/50 translucent with modifications
	    register int sb = dc_source[frac>>FRACBITS];
#ifdef HIGHCOLORMAPS
	    c32.ui32 = hicolormaps[ sb  ];
#else
//	    c32.ui32 = color8.to32[dc_colormap[ sb ]];
	    c32.ui32 = color8.to32[ sb ];
#endif
#if FX1_OPTION==1
	    // check translucent map for opaque, sprite vrs white bkg
	    register byte twht = dc_translucentmap[ (sb << 8) + 4 ];
	    register int  tdiff = sb - twht;  // any change
#else
	    // check translucent map for opaque, blk bkg vrs white bkg
	    register byte tblk = dc_translucentmap[ (sb << 8) ];
	    register byte twht = dc_translucentmap[ (sb << 8) + 4 ];
	    register int  tdiff = tblk - twht;
#endif

	    if( tdiff > 2 || tdiff < -2 )
	    {
	        // 50/50
	        register uint16_t dc0, dc1, dc2;
	        register pixel24_t * p24 = (pixel24_t*)dest;
	        dc0 = p24->b;
	        p24->b = (dc0 + c32.pix24.b) >> 1;
	        dc1 = p24->g;
	        p24->g = (dc1 + c32.pix24.g) >> 1;
	        dc2 = p24->r;
	        p24->r = (dc2 + c32.pix24.r) >> 1;
	    }
	    else
	    {
		*(pixel24_t*)dest = c32.pix24;  // opaque
	    }
	    dest += vid.ybytes;
	    frac += fracstep;
	    if( frac >= texheight )
	        frac -= texheight;
	} while (count--);
        break;
     case TRANSLU_75: // 75 25
        do
        {
	    // 75/25 translucent
#ifdef HIGHCOLORMAPS
	    c32.ui32 = hicolormaps[ dc_source[frac>>FRACBITS] ];
#else
	    c32.ui32 = color8.to32[dc_colormap[dc_source[frac>>FRACBITS]]];
#endif
	    register uint16_t dc0, dc1, dc2; // for overlapped execution
	    register pixel24_t * p24 = (pixel24_t*)dest;
	    dc0 = c32.pix24.b;
	    p24->b = (dc0 + dc0 + dc0 + p24->b) >> 2;
	    dc1 = c32.pix24.g;
	    p24->g = (dc1 + dc1 + dc1 + p24->g) >> 2;
	    dc2 = c32.pix24.r;
	    p24->r = (dc2 + dc2 + dc2 + p24->r) >> 2;
	    dest += vid.ybytes;
	    frac += fracstep;
	    if( frac >= texheight )
	        frac -= texheight;
	} while (count--);
        break;
    }
}


// transparent with skin translations
// Although the vissprite has capability for any transparency,
// this is only called with TRANSLU_hi
void R_DrawTranslatedTranslucentColumn_24(void)
{
    R_DrawTranslucentColumn_24();
}


// Skin
void R_DrawTranslatedColumn_24 (void)
{
    fixed_t texheight = dc_texheight << FRACBITS;  // any texture size
    int     count;
    byte*   dest;  // within screen buffer
    fixed_t frac;
    fixed_t fracstep;
    pixelunion32_t c32;

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
    if (texheight > 0)  // hangs when texheight==0
    {
        // From Boom, to fix the odd frac
        if (frac < 0)
	    while ((frac += texheight) < 0);
        else
	    while (frac >= texheight)  frac -= texheight;
    }

    // Here we do an additional skin re-mapping.
    do
    {
#ifdef HIGHCOLORMAPS
        c32.ui32 = hicolormaps[ dc_skintran[ dc_source[frac>>FRACBITS]]] ];
#else
        c32.ui32 = color8.to32[ dc_colormap[ dc_skintran[ dc_source[frac>>FRACBITS]]] ];
#endif
        *(pixel24_t*)dest = c32.pix24;  // opaque
        dest += vid.ybytes;
        frac += fracstep;
        if( frac >= texheight )
	    frac -= texheight;
    } while (count--);
}



// ==========================================================================
// SPANS
// ==========================================================================

void R_DrawSpan_24 (void)
{
    fixed_t xfrac;
    fixed_t yfrac;
    pixel24_t *  p24;
    int     count;
    pixelunion32_t c32;

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

    p24 = (pixel24_t*)( ylookup[ds_y] + columnofs[ds_x1] );

    // We do not check for zero spans here?
    count = ds_x2 - ds_x1;

    do
    {
#ifdef HICOLORMAPS
        // Current texture index in u,v.
        register int spot = ((yfrac>>(16-6))&(63*64)) + ((xfrac>>16)&63);

        // Lookup pixel from flat texture tile,
        //  re-index using light/colormap.
        c32.ui32 = hicolormaps[ ds_source[spot]>>1 ];
#else
        xfrac &= 0x3fFFff;
        register int spot = ((yfrac >> (16 - flatsubtract)) & flatmask) | (xfrac >> 16);
//        register int spot = ((yfrac >> (16 - 6)) & (0x3f << 6)) | (xfrac >> 16);
        c32.ui32 = color8.to32[ ds_colormap[ds_source[spot]] ];
#endif
        *(p24++) = c32.pix24;  // opaque
        // Next step in u,v.
        xfrac += ds_xstep;
        yfrac += ds_ystep;
    } while (count--);
}


void R_DrawTranslucentSpan_24(void)
{
    fixed_t xfrac;
    fixed_t yfrac;
    fixed_t xstep;
    fixed_t ystep;
    pixel24_t *  p24;
    int count;
    pixelunion32_t c32;

#ifdef RANGECHECK
    if (ds_x2 < ds_x1 || ds_x1 < 0 || ds_x2 >= rdraw_viewwidth || (unsigned) ds_y > rdraw_viewheight)
    {
        I_SoftError("R_DrawSpan: %i to %i at %i\n", ds_x1, ds_x2, ds_y);
        return;
    }
#endif

    xfrac = ds_xfrac & ((flatsize << FRACBITS) - 1);
    yfrac = ds_yfrac;

    p24 = (pixel24_t*)( ylookup[ds_y] + columnofs[ds_x1] );

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
        c32.ui32 = hicolormaps[ ((uint16_t *)ds_source)[spot]>>1 ];
#else
        xfrac &= 0x3fFFff;
        register int spot = ((yfrac >> (16 - flatsubtract)) & flatmask) | (xfrac >> 16);
	// 50/50 translucent
	c32.ui32 = color8.to32[ ds_colormap[ds_source[spot]] ];
#endif
        p24->b = ((uint16_t)p24->b + c32.pix24.b) >> 1;
        p24->g = ((uint16_t)p24->g + c32.pix24.g) >> 1;
        p24->r = ((uint16_t)p24->r + c32.pix24.r) >> 1;
        p24 ++;  // *3

        // Next step in u,v.
        xfrac += xstep;
        yfrac += ystep;
        xfrac &= ((flatsize << FRACBITS) - 1);
    }
    while (--count);
}


#define FOGCOLOR
// Used for Legacy linetype 302, ceiling and floor of 3D fog in tagged
void R_DrawFogSpan_24(void)
{
    pixel24_t *  p24;
    unsigned int count;
#ifdef FOGCOLOR
    pixelunion32_t fogcolor;
    fogcolor.ui32 = color8.to32[ ds_colormap[ ds_source[0] ]];
//    fogcolor.ui32 = color8.to32[ ds_colormap[ 104 ]]; // gray
    fogcolor.pix24.r >>= 1;
    fogcolor.pix24.g >>= 1;
    fogcolor.pix24.b >>= 1;
#endif

    p24 = (pixel24_t*)( ylookup[ds_y] + columnofs[ds_x1] );
    count = ds_x2 - ds_x1 + 1;

    while (count--)
    {
        // 25/0/12 translucent
#ifdef FOGCOLOR
        register uint16_t dc0, dc1, dc2; // for overlapped execution
        dc0 = p24->b;
	p24->b = (dc0 + dc0 + fogcolor.pix24.b) >> 2;
	dc1 = p24->g;
	p24->g = (dc1 + dc1 + fogcolor.pix24.g) >> 2;
	dc2 = p24->r;
	p24->r = (dc2 + dc2 + fogcolor.pix24.r) >> 2;
#else
# define FOGGRAY_SPAN   0x08
        register uint16_t dc0, dc1, dc2; // for overlapped execution
        dc0 = p24->b;
	p24->b = (dc0 + dc0 + FOGGRAY_SPAN) >> 3;
	dc1 = p24->g;
	p24->g = (dc1 + dc1 + FOGGRAY_SPAN) >> 3;
	dc2 = p24->r;
	p24->r = (dc2 + dc2 + FOGGRAY_SPAN) >> 3;
#endif
        p24 ++;  // *3
    }
}

//SoM: Fog wall.
// Used for Legacy linetype 302, walls of 3D fog in tagged
// Used for Legacy linetype 283, fog sheet
void R_DrawFogColumn_24(void)
{
    int count;
    byte * dest;
#ifdef FOGCOLOR
static pixelunion32_t fogcolor = {.ui32=0x00101010};
    pixelunion32_t fc, fc2;
    // fog_index 0.. column height
    int fogb2 = 32 - fog_bltic; // fade proportionality
    fc.ui32 = color8.to32[ dc_colormap[ dc_source[fog_index] ]];
    fc2.ui32 = color8.to32[ dc_colormap[ dc_source[((fog_index==0)?fog_col_length:fog_index)-1] ]];
    fc.pix32.r = ((fc.pix32.r * fog_bltic) + (fc2.pix32.r * fogb2)) >> 5;
    fc.pix32.g = ((fc.pix32.g * fog_bltic) + (fc2.pix32.g * fogb2)) >> 5;
    fc.pix32.b = ((fc.pix32.b * fog_bltic) + (fc2.pix32.b * fogb2)) >> 5;
    if( fog_init )
    {
        // init blur
        fogcolor.pix32 = fc.pix32;
        fog_init = 0;
    }
    else
    {
        // blur
        fogcolor.pix32.r = ((((uint16_t)fogcolor.pix32.r)*31) + fc.pix32.r) >> 5;
        fogcolor.pix32.g = ((((uint16_t)fogcolor.pix32.g)*31) + fc.pix32.g) >> 5;
        fogcolor.pix32.b = ((((uint16_t)fogcolor.pix32.b)*31) + fc.pix32.b) >> 5;
    }
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
	register uint16_t dc0, dc1, dc2; // for overlapped execution
        register pixel24_t * p24 = (pixel24_t*)dest;
	dc0 = p24->b;
//	p24->b = (dc0 + dc0 + dc0 + fogcolor.pix24.b) >> 2;
	p24->b = (dc0 + dc0 + fogcolor.pix24.b) >> 2;
	dc1 = p24->g;
//	p24->g = (dc1 + dc1 + dc1 + fogcolor.pix24.g) >> 2;
	p24->g = (dc1 + dc1 + fogcolor.pix24.g) >> 2;
	dc2 = p24->r;
//	p24->r = (dc2 + dc2 + dc2 + fogcolor.pix24.r) >> 2;
	p24->r = (dc2 + dc2 + fogcolor.pix24.r) >> 2;
        dest += vid.ybytes;
    }
    while (count--);
#else

#if 0
#   define FOGGRAY_P    0x10
    do
    {
	// ignore texture
	// Closest to 8bit PAL version
	register uint16_t dc0, dc1, dc2; // for overlapped execution
        register pixel24_t * p24 = (pixel24_t*)dest;
        dc0 = p24->b;
	p24->b = ((dc0<<2) + FOGGRAY_P) >> 3;
	dc1 = p24->g;
	p24->g = ((dc1<<2) + FOGGRAY_P) >> 3;
	dc2 = p24->r;
	p24->r = ((dc2<<2) + FOGGRAY_P) >> 3;
        dest += vid.ybytes;
    }
    while (count--);
#else
   
    // Show texture in fog

    // Determine scaling,
    //  which is the only mapping to be done.
    pixelunion32_t c32;
    fixed_t texheight = dc_texheight << FRACBITS;  // any texture size
    fixed_t frac;
    fixed_t fracstep;

    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl-centery)*fracstep;
    if (texheight > 0)  // hangs when texheight==0
    {
        // From Boom, to fix the odd frac
        if (frac < 0)
	    while ((frac += texheight) < 0);
        else
	    while (frac >= texheight)  frac -= texheight;
    }

#   define FOGGRAY   0x04
    do
    {
	// faint texture + dark
	register byte bc = dc_source[frac>>FRACBITS];
#ifdef HICOLORMAPS
        c32.ui32 = hicolormaps[ bc ];
#else
        c32.ui32 = color8.to32[ bc ];
#endif
	register uint16_t dc0, dc1, dc2; // for overlapped execution
        register pixel24_t * p24 = (pixel24_t*)dest;
	dc0 = p24->b;
	p24->b = (dc0 + dc0 + c32.pix24.b + FOGGRAY) >> 3;
	dc1 = p24->g;
	p24->g = (dc1 + dc1 + c32.pix24.g + FOGGRAY) >> 3;
	dc2 = p24->r;
	p24->r = (dc2 + dc2 + c32.pix24.r + FOGGRAY) >> 3;
        dest += vid.ybytes;
        frac += fracstep;
        if( frac >= texheight )
	    frac -= texheight;
    }
    while (count--);
#endif
#endif
}
