// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: r_data.c 896 2012-02-29 19:18:53Z wesleyjohnson $
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
// $Log: r_data.c,v $
// Revision 1.31  2003/05/04 04:12:54  sburke
// Replace memcpy with memmove, to prevent a misaligned access fault on sparc.
//
// Revision 1.30  2002/12/13 19:22:12  ssntails
// Fix for the Z_CheckHeap and random crashes! (I hope!)
//
// Revision 1.29  2002/01/12 12:41:05  hurdler
// very small commit
//
// Revision 1.28  2002/01/12 02:21:36  stroggonmeth
// Big commit
//
// Revision 1.27  2001/12/27 22:50:25  hurdler
// fix a colormap bug, add scrolling floor/ceiling in hw mode
//
// Revision 1.26  2001/08/13 22:53:40  stroggonmeth
// Small commit
//
// Revision 1.25  2001/03/21 18:24:39  stroggonmeth
// Misc changes and fixes. Code cleanup
//
// Revision 1.24  2001/03/19 18:52:01  hurdler
// lil fix
//
// Revision 1.23  2001/03/13 22:14:20  stroggonmeth
// Long time no commit. 3D floors, FraggleScript, portals, ect.
//
// Revision 1.22  2000/11/04 16:23:43  bpereira
// no message
//
// Revision 1.21  2000/11/02 17:50:09  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.20  2000/10/04 16:19:23  hurdler
// Change all those "3dfx names" to more appropriate names
//
// Revision 1.19  2000/09/28 20:57:17  bpereira
// no message
//
// Revision 1.18  2000/08/11 12:25:23  hurdler
// latest changes for v1.30
//
// Revision 1.17  2000/07/01 09:23:49  bpereira
// no message
//
// Revision 1.16  2000/05/03 23:51:01  stroggonmeth
// A few, quick, changes.
//
// Revision 1.15  2000/04/23 16:19:52  bpereira
// no message
//
// Revision 1.14  2000/04/18 17:39:39  stroggonmeth
// Bug fixes and performance tuning.
//
// Revision 1.13  2000/04/18 12:54:58  hurdler
// software mode bug fixed
//
// Revision 1.12  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.11  2000/04/15 22:12:58  stroggonmeth
// Minor bug fixes
//
// Revision 1.10  2000/04/13 23:47:47  stroggonmeth
// See logs
//
// Revision 1.9  2000/04/08 17:45:11  hurdler
// fix some boom stuffs
//
// Revision 1.8  2000/04/08 17:29:25  stroggonmeth
// no message
//
// Revision 1.7  2000/04/08 11:27:29  hurdler
// fix some boom stuffs
//
// Revision 1.6  2000/04/07 01:39:53  stroggonmeth
// Fixed crashing bug in Linux.
// Made W_ColormapNumForName search in the other direction to find newer colormaps.
//
// Revision 1.5  2000/04/06 21:06:19  stroggonmeth
// Optimized extra_colormap code...
// Added #ifdefs for older water code.
//
// Revision 1.4  2000/04/06 20:40:22  hurdler
// Mostly remove warnings under windows
//
// Revision 1.3  2000/04/04 00:32:47  stroggonmeth
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
//      Preparation of data for rendering,
//      generation of lookups, caching, retrieval by name.
//
//-----------------------------------------------------------------------------

#include <malloc.h>

#include "doomdef.h"
#include "g_game.h"
#include "i_video.h"
#include "r_local.h"
#include "r_sky.h"
#include "p_local.h"
#include "r_data.h"
#include "w_wad.h"
#include "z_zone.h"
#include "p_setup.h" //levelflats
#include "v_video.h" //pLocalPalette


// [WDJ] debug flat
//#define DEBUG_FLAT

// [WDJ] Generate texture controls

// [WDJ] For y clipping to be technically correct, the pixels in the source
// post must be skipped. To maintain compatibility with the original doom
// engines, which had this bug, other engines do not skip the post pixels either.
// Enabling corrected_clipping will make some textures slightly different
// than displayed in other engines.
// TEKWALL1 will have two boxes in the upper left corner with this off and one
// box with it enabled.  The differences in other textures are less noticable.
// This only affects software rendering, hardware rendering is correct.
boolean corrected_clipping = 0;

// Select texture generation for multiple-patch textures.
typedef enum {
   TGC_auto,		// logic will select picture or combine_patch
   TGC_picture,		// always picture format, no transparent multi-patch
   TGC_combine_patch	// always combine_patch format, uses more memory (usually)
} texgen_control_e;
texgen_control_e  texgen_control = TGC_auto;

//
// Graphics.
// DOOM graphics for walls and sprites
// is stored in vertical runs of opaque pixels (posts).
// A column is composed of zero or more posts,
// a patch or sprite is composed of zero or more columns.
//

int             firstflat, lastflat, numflats;
int             firstpatch, lastpatch, numpatches;


// textures
int             numtextures=0;      // total number of textures found,
// size of following tables

texture_t**     textures=NULL;
unsigned int**  texturecolumnofs;   // column offset lookup table for each texture
byte**          texturecache;       // graphics data for each generated full-size texture
int*            texturewidthmask;   // texture width is a power of 2, so it
                                    // can easily repeat along sidedefs using
                                    // a simple mask
fixed_t*        textureheight;      // needed for texture pegging

int       *flattranslation;             // for global animation
int       *texturetranslation;

// needed for pre rendering
spritelump_t *  spritelumps = NULL;  // array of sprite lump values, endian swapped
int             num_spritelump = 0;  // already allocated
int             num_free_spritelump = 0;  // free for allocation

// colormap lightmaps from wad COLORMAP lump
lighttable_t *  reg_colormaps;


//faB: for debugging/info purpose
int             flatmemory;
int             spritememory;
int             texturememory;	// all textures


//faB: highcolor stuff
short    color8to16[256];       //remap color index to highcolor rgb value
short*   hicolormaps;           // test a 32k colormap remaps high -> high

#if 0
// [WDJ] This description applied to previous code. It is kept for historical
// reasons. We may have to restore some of this functionality, but this scheme
// would not work with z_zone freeing cache blocks.
// 
// MAPTEXTURE_T CACHING
// When a texture is first needed,
//  it counts the number of composite columns
//  required in the texture and allocates space
//  for a column directory and any new columns.
// The directory will simply point inside other patches
//  if there is only one patch in a given column,
//  but any columns with multiple patches
//  will have new column_t generated.
//
#endif

// [WDJ] 2/5/2010
// See LITE96 originx=-1, LITERED originx=-4, SW1WOOD originx=-64
// See TEKWALL1 originy=-27, STEP2 originy=-112, and other such textures in doom wad.
// Patches leftoffset and topoffset are ignored when composing textures.

// The original doom has a clipping bug when originy < 0.
// The patch position is moved instead of clipped, the height is clipped.

//
// R_DrawColumnInCache
// Clip and draw a column from a patch into a cached post.
//

void R_DrawColumnInCache ( column_t*     colpost,	// source, list of 0 or more post_t
                           byte*         cache,		// dest
                           int           originy,
                           int           cacheheight )  // limit
{
    int         count;
    int         position;
    byte*       source;
    byte*       dest;

    dest = (byte *)cache;// + 3;

    // Assemble a texture from column post data from wad lump.
    // Column is a series of posts (post_t), terminated by 0xFF
    while (colpost->topdelta != 0xff)	// end of posts
    {
        // post has 2 byte header (post_t), 
        // and has extra byte before and after pixel data
        source = (byte *)colpost + 3;	// pixel data after post header
        count = colpost->length;
        position = originy + colpost->topdelta;  // position in dest

        if (position < 0)
        {
            count += position;  // skip pixels off top
	    // [WDJ] For this clipping to be technically correct, the pixels
	    // in the source must be skipped too.
	    // Other engines do not skip the post pixels either, to maintain
	    // compatibility with the original doom engines, which had this bug.
	    // Enabling this will make some textures slightly different
	    // than displayed in other engines.
	    // TEKWALL1 will have two boxes in the upper left corner with this
	    // off and one box with it enabled.  The differences in other
	    // textures are less noticable.
	    if( corrected_clipping )
	    {
	        source -= position; // [WDJ] 1/29/2010 skip pixels in post
	    }
            position = 0;
        }

        if (position + count > cacheheight)  // trim off bottom
            count = cacheheight - position;

        // copy column (full or partial) to dest cache at position
        if (count > 0)
            memcpy (cache + position, source, count);

        // next source colpost, adv by (length + 2 byte header + 2 extra bytes)
        colpost = (column_t *)(  (byte *)colpost + colpost->length + 4);
    }
}



//
// R_GenerateTexture
//
//   Allocate space for full size texture, either single patch or 'composite'
//   Build the full textures from patches.
//   The texture caching system is a little more hungry of memory, but has
//   been simplified for the sake of highcolor, dynamic lighting, & speed.
//
//   This is not optimized, but it's supposed to be executed only once
//   per level, when enough memory is available.

// Temp structure element used to combine patches into one dest patch.
typedef struct {
    int   nxt_y, bot_y;		// current post segment in dest coord.
    int   width;
    int   originx, originy;	// add to patch to get texture
    post_t *  postptr;		// post within that patch
    patch_t * patch;     	// patch source
} compat_t;
   

byte* R_GenerateTexture (int texnum)
{
    byte*               texgen;  // generated texture
    byte*               txcblock;
    texture_t*          texture; // texture info from wad
    texpatch_t*         texpatch;  // patch info to make texture
    patch_t*            realpatch;
    uint32_t*           colofs;  // to match size in wad
    int                 x, x1, x2;
    int                 i;
    int                 patchsize;
    int			txcblocksize;
    int			colofs_size;

    texture = textures[texnum];
    texture->texture_model = TM_invalid; // default in case this fails
    
    // Column offset table size as determined by wad specs.
    // Column pixel data starts after the table.
    colofs_size = texture->width * sizeof( uint32_t );  // width * 4

    // allocate texture column offset lookup

    // single-patch textures can have holes and may be used on
    // 2sided lines so they need to be kept in 'packed' format
    if (texture->patchcount==1)
    {
        // Texture patch format:
	//   patch header (8 bytes), ignored
        //   array[ texture_width ] of column offset
        //   concatenated column posts, of variable length, terminate 0xFF
        //        ( post header (topdelta,length), pixel data )

        // Single patch texture, simplify
        texpatch = texture->patches;
        patchsize = W_LumpLength (texpatch->patchnum);
       
        // [WDJ] Protect every alloc using PU_CACHE from all Z_Malloc that
        // follow it, as that can deallocate the PU_CACHE unexpectedly.
	
        // [WDJ] Only need patch lump for the following memcpy.
	// [WDJ] Must use common patch read to preserve endian consistency.
	// otherwise it will be in cache without endian changes.
        realpatch = W_CachePatchNum (texpatch->patchnum, PU_IN_USE);  // texture lump temp
#if 1
        // [WDJ] Detect PNG patches.
	if(    ((byte*)realpatch)[0]==137
	    && ((byte*)realpatch)[1]=='P'
	    && ((byte*)realpatch)[2]=='N'
	    && ((byte*)realpatch)[3]=='G' )
        {
	    // Found a PNG patch, which will crash the draw8 routine.
	    // Must assume could be used for top or bottom of wall,
	    // which require the patch run full height (no transparent).
#if 1
	    // Enable when want to know which textures are triggering this.
	    fprintf(stderr,"R_GenerateTexture: Texture %8s has PNG patch, using dummy texture.\n", texture->name );
#endif
	    // make a dummy texture
	    int head_size = colofs_size + 8;
	    txcblocksize = head_size + 4 + texture->height + 4;
            txcblock = Z_Malloc (txcblocksize,
                          PU_IN_USE,         // will change tag at end of this function
                          (void**)&texturecache[texnum]);
	    patch_t * txcpatch = (patch_t*) txcblock;
	    txcpatch->width = texture->width;
	    txcpatch->height = texture->height;
	    txcpatch->leftoffset = 0;
	    txcpatch->topoffset = 0;
	    post_t * destpost = (post_t*) ((byte*)txcblock + head_size);  // posting area;
	    destpost->topdelta = 0;
	    destpost->length = texture->height;
	    byte* destpixels = (byte*)destpost + 3;
	    destpixels[-1] = 0;	// pad 0
	    for( i=0; i>texture->height; i++ )
	    {
	        destpixels[i] = 8; // mono color
	    }
	    destpixels[i+1] = 0; // pad 0
	    destpixels[i+2] = 0xFF; // term
	    // all columns use the same post
	    colofs = (uint32_t*)&(txcpatch->columnofs);  // has patch header
	    for(i=0 ; i< texture->width ; i++ )
	         colofs[i] = head_size;
        }
        else
#endif
#if 1
	if( realpatch->width < texture->width )
	{
	    // [WDJ] Messy situation. Single patch texture where the patch
	    // width is smaller than the texture.  There will be segfaults when
	    // texture columns are accessed that are not in the patch.
	    // Rebuild the patch to meet draw expectations.
	    // This occurs in phobiata and maybe heretic.
	    // [WDJ] Too late to change texture size to match patch,
	    // the caller can already be violating the patch width.
	    // Cannot goto multi-patch because numpatches > 1 is used as the
	    // selector between texture formats, and single-sided textures
	    // need to be kept packed to preserve holes.
	    // The texture may be rebuilt several times due to cache.
            int patch_colofs_size = realpatch->width * sizeof( uint32_t );  // width * 4
	    int ofsdiff = colofs_size - patch_colofs_size;
	    // reserve 4 bytes at end for empty post handling
	    txcblocksize = patchsize + ofsdiff + 4;
#if 0
	    // Enable when want to know which textures are triggering this.
	    fprintf(stderr,"R_GenerateTexture: single patch width does not match texture width %8s\n",
		   texture->name );
#endif
            txcblock = Z_Malloc (txcblocksize,
                          PU_LEVEL,         // will change tag at end of this function
                          (void**)&texturecache[texnum]);
	    // patches have 8 byte patch header, part of patch_t
            memcpy (txcblock, realpatch, 8); // header
            memcpy (txcblock + colofs_size + 8, ((byte*)realpatch) + patch_colofs_size + 8, patchsize - patch_colofs_size - 8 ); // posts
	    // build new column offset table of texture width
	    {
	        // Copy patch columnofs table to texture, adjusted for new
		// length of columnofs table.
                uint32_t* pat_colofs = (uint32_t*)&(realpatch->columnofs); // to match size in wad
                colofs = (uint32_t*)&(((patch_t*)txcblock)->columnofs); // new
                for( i=0; i< realpatch->width; i++)
	                colofs[i] = pat_colofs[i] + ofsdiff;
                // put empty post for all columns not in the patch.
		// txcblock has 4 bytes reserved for this.
	        int empty_post = txcblocksize - 4;
	        txcblock[empty_post] = 0xFF;  // empty post list
	        txcblock[empty_post+3] = 0xFF;  // paranoid
	        for( ; i< texture->width ; i++ )
		        colofs[i] = empty_post;
	    }
	}
	else
#endif   
	{
            // Normal: Most often use patch as it is.
#if 1
            // texturecache gets copy so that PU_CACHE deallocate clears the
	    // texturecache automatically
            txcblock = Z_Malloc (patchsize,
                          PU_IN_USE,  // will change tag at end of this function
                          (void**)&texturecache[texnum]);
	    memcpy (txcblock, realpatch, patchsize);
	    txcblocksize = patchsize;
#else
        // FIXME: this version puts the z_block user as lumpcache,
        // instead of as texturecache, so deallocate by PU_CACHE leaves
        // texturecache with a bad ptr.
//        texturecache[texnum] = txcblock = W_CachePatchNum (texpatch->patchnum, PU_IN_USE);
        texturecache[texnum] = txcblock = realpatch;
	Z_ChangeOwner (realpatch, (void**)&texturecache[texnum]);
        Z_ChangeTag (realpatch, PU_IN_USE);
        txcblocksize = patchsize;
#endif
	}

        // Interface for texture picture format
        // use the single patch's, single column lookup
//        colofs = (unsigned int*)(txcblock + 8);
        colofs = (uint32_t*)&(((patch_t*)txcblock)->columnofs);
        // colofs from patch are relative to start of table
        for (i=0; i<texture->width; i++)
             colofs[i] = colofs[i] + 3;  // adjust colofs from wad
	      // offset to pixels instead of post header
	      // Many callers will use colofs-3 to get back to header, but
              // some drawing functions need pixels.
       
        Z_ChangeTag (realpatch, PU_CACHE);  // safe
        texturecolumnofs[texnum] = colofs;
        texture->texture_model = TM_patch;
        //CONS_Printf ("R_GenTex SINGLE %.8s size: %d\n",texture->name,patchsize);
        texgen = txcblock;
        goto done;
    }
    // End of Single-patch texture
   
 // TGC_combine_patch vrs TGC_picture: Multiple patch textures.
    // array to hold all patches
    #define MAXPATCHNUM 256
    compat_t   compat[MAXPATCHNUM];
    post_t   * srcpost, * destpost;
    patch_t  * txcpatch; // header of txcblock
    byte     * txcdata;  // posting area
    byte     * destpixels;  // post pixel area
    byte     * texture_end; // end of allocated texture area
    unsigned int patchcount = texture->patchcount;
    unsigned int compostsize = 0;
    int p;
    int postlength;  // length of current post
    int segnxt_y, segbot_y; // may be negative
    int bottom;		// next y in column

    if( patchcount >= MAXPATCHNUM ) {
       I_SoftError("R_GenerateTexture: Combine patch count %i exceeds %i, ignoring rest\n",
		   patchcount, MAXPATCHNUM);
       patchcount = MAXPATCHNUM - 1;
    }
   
    // First examination of the source patches
    texpatch = texture->patches;
    for (p=0; p<patchcount; p++, texpatch++)
    {
        compat_t * cp = &compat[p];
        cp->postptr = NULL;	// disable until reach starting column
        cp->nxt_y = MAXINT;	// disable
        cp->originx = texpatch->originx;
        cp->originy = texpatch->originy;
        realpatch = W_CachePatchNum(texpatch->patchnum, PU_IN_USE);  // patch temp
        cp->patch = realpatch;
        cp->width = realpatch->width;
        int patch_colofs_size = realpatch->width * sizeof( uint32_t );  // width * 4
        // add posts, without columnofs table and 8 byte patch header
        compostsize += W_LumpLength(texpatch->patchnum) - patch_colofs_size - 8;
    }
    // Decide TGC_ format
    // Combined patches + table + header
    compostsize += colofs_size + 8;	// combined patch size
    txcblocksize = colofs_size + (texture->width * texture->height); // picture format size
    // If cache was flushed then do not change model
    switch( texture->texture_model )
    {
     case TM_picture:  goto picture_format;
     case TM_combine_patch:  goto combine_format;
     default: break;
    }
    // new texture, decide on a model
    switch( texgen_control )
    {
     case TGC_auto:  break;
     case TGC_picture:  goto picture_format;
     case TGC_combine_patch:  goto combine_format;
     default: break;
    }
    // if patches would not cover picture, then must have transparent regions
    if( compostsize < txcblocksize )  goto combine_format;
    if( texture->texture_model == TM_masked )  goto combine_format; // hint
    // If many patches and high overlap, then picture format is best.
    // This will need to be tuned if it makes mistakes.
    if( texture->patchcount >= 4 && compostsize > txcblocksize*4 )  goto picture_format;
  
 combine_format:
    // TGC_combine_patch: Combine multiple patches into one patch.

    // Size the new texture.  It may be too big, but must not be too small.
    // Will always fit into compostsize because it does so as separate patches.
    // Overlap of posts will only reduce the size.
    // Worst case is (width * (height/2) * (1 pixel + 4 byte overhead))
    //   worstsize = colofs_size + 8 + (width * height * 5/2)
    // Usually the size will be much smaller, but it is not predictable.
    // all posts + new columnofs table + patch header

#if 1
    // Combined patches + table + header + 2 byte per empty column
    txcblocksize = compostsize + (2 * texture->width);
     // this stops failure in caesar.wad
#else
    // Combined patches + table + header + 1 byte per empty column
    txcblocksize = compostsize + texture->width;
#endif
    txcblock = Z_Malloc (txcblocksize, PU_IN_USE,
                      (void**)&texturecache[texnum]);
    txcpatch = (patch_t*) txcblock;
    txcpatch->width = texture->width;
    txcpatch->height = texture->height;
    txcpatch->leftoffset = 0;
    txcpatch->topoffset = 0;
    // column offset lookup table
    colofs = (uint32_t*)&(txcpatch->columnofs);  // has patch header
    txcdata = (byte*)txcblock + colofs_size + 8;  // posting area
    // starts as empty post, with 0xFF a possibility
    destpixels = txcdata;
    texture_end = txcblock + txcblocksize - 2; // do not exceed

    // Composite the columns together.
    for (x=0; x<texture->width; x++)
    {
        int nxtpat;	// patch number with next post
        int seglen, offset;
       
        // offset to pixels instead of post header
        // Many callers will use colofs-3 to get back to header, but
        // some drawing functions need pixels.
        colofs[x] = destpixels - (byte*)txcblock + 3;  // this column
        destpost = (post_t*)destpixels;	// first post in column
	postlength = 0;  // length of current post
        bottom = 0;	 // next y in column
        segnxt_y = MAXINT - 10;	// init to very large, but less than disabled
        segbot_y = MAXINT - 10;

        // setup the columns, active or inactive
        for (p=0; p<patchcount; p++ )
        {
	    compat_t * cp = &compat[p];
	    int patch_x = x - cp->originx;	// within patch
	    if( patch_x >= 0 && patch_x < cp->width )
	    {
	        realpatch = cp->patch;
	        uint32_t* pat_colofs = (uint32_t*)&(realpatch->columnofs); // to match size in wad
	        cp->postptr = (post_t*)( (byte*)realpatch + pat_colofs[patch_x] );  // patch column
	        if ( cp->postptr->topdelta == 0xFF )
		    goto patch_off;
	        cp->nxt_y = cp->originy + cp->postptr->topdelta;
	        cp->bot_y = cp->nxt_y + cp->postptr->length;
	    }else{
	       patch_off: // empty post
	        // clip left and right by turning this patch off
	        cp->postptr = NULL;
	        cp->nxt_y = MAXINT;
	        cp->bot_y = MAXINT;
	    }
	}  // for all patches
       
        for(;;) // all posts in column
        {
            // Find next post y in this column.
	    // Only need the last patch, as that overwrites every other patch.
	    nxtpat = -1;	// patch with next post
            segnxt_y = MAXINT-64;	// may be negative, must be < MAXINT
            compat_t * cp = &compat[0];
            for (p=0; p<patchcount; p++, cp++)
            {
	        // Skip over any patches that have been passed.
		// Includes those copied, or covered by those copied.
	        while( cp->bot_y <= bottom )
	        {
		    // this post has been passed, go to next post
		    cp->postptr = (post_t*)( ((byte*)cp->postptr) + cp->postptr->length + 4);
		    if( cp->postptr->topdelta == 0xFF )  // end of post column
		    {
		        // turn this patch off
		        cp->postptr = NULL;
		        cp->nxt_y = MAXINT;	// beyond texture size
		        cp->bot_y = MAXINT;
		        break;
		    }
		    cp->nxt_y = cp->originy + cp->postptr->topdelta;
		    cp->bot_y = cp->nxt_y + cp->postptr->length;
		}
	        if( cp->nxt_y <= segnxt_y )
	        {
		    // Found an active post
		    nxtpat = p;	// last draw into this segment
		    // Check for continuing in the middle of a post.
		    segnxt_y = (cp->nxt_y < bottom)?
		       bottom	// continue previous
		       : cp->nxt_y; // start at top of post
		    // Only a later drawn patch can overwrite this post.
		    segbot_y = cp->bot_y;
		}
	        else
	        {
		    // Limit bottom of this post segment by later drawn posts.
		    if( cp->nxt_y < segbot_y )   segbot_y = cp->nxt_y;
		}
	    }
	    // Exit test, end of column, which may be empty
	    if( segnxt_y >= texture->height )   break;
	   
	    // copy whole/remainder of post, or cut it short
	    // assert: segbot_y <= cp->bot_y+1  because it is set in loop
	    if( segbot_y > texture->height )   segbot_y = texture->height;

	    seglen = segbot_y - segnxt_y;

	    if( postlength != 0 )
	    {
	       // Check if next patch does not append to bottom of current patch
	       if( ((segnxt_y > bottom) && (bottom > 0))
		 || (postlength + seglen > 255) ) // if postlength would be too long
	       {
		   // does not append, start new post after existing post
		   destpost = (post_t*)((byte*)destpost + destpost->length + 4);
		   postlength = 0;
	       }
	    }
	   
	    // Only one patch is drawn last in this segment, copy that one
	    cp = &compat[nxtpat];
	    srcpost = cp->postptr;
            // offset>0 when copy starts part way into this source post
	    // NOTE: cp->nxt_y = cp->originy + srcpost->topdelta;
	    offset = ( segnxt_y > cp->nxt_y )? (segnxt_y - cp->nxt_y) : 0;
	    // consider y clipping problem
	    if( cp->nxt_y < 0  &&  !corrected_clipping )
	    {
                // Original doom had bug in y clipping, such that it
		// would clip the width but not skip the source pixels.
	        // Given that segnxt_y was already correctly clipped.
	        offset += cp->nxt_y; // reproduce original doom clipping
	    }
	   
	    if( postlength == 0 )
	    {
	        if( destpixels + 3 >= texture_end )  goto exceed_alloc_error;
	        if( segnxt_y > 254 )   goto exceed_topdelta;
	        // new post header and 0 pad byte
	        destpost->topdelta = segnxt_y;
	        // append at
	        destpixels = (byte*)destpost + 3;
	        destpixels[-1] = 0;	// pad 0
	    }

	    seglen = segbot_y - segnxt_y;
	    if( destpixels + seglen >= texture_end )  goto exceed_alloc_error;
	   
	    // append to existing post
	    memcpy( destpixels, ((byte*)srcpost + offset + 3), seglen );
	    destpixels += seglen;
	    postlength += seglen;
	    bottom = segbot_y;
	    // finish post bookkeeping so can terminate loop easily
	    *destpixels = 0;	// pad 0
	    if( postlength > 255 )   goto exceed_post_length;
	    destpost->length = postlength;
	} // for all posts in column
        destpixels++;		// skip pad 0
        *destpixels++ = 0xFF;	// mark end of column
        // may be empty column so do not reference destpost
    } // for x
    if( destpixels >= texture_end )  goto exceed_alloc_error;
    // unlock all the patches, no longer needed, but may be in another texture
    for (p=0; p<patchcount; p++)
    {
        Z_ChangeTag (compat[p].patch, PU_CACHE);
    }
    // Interface for texture picture format
    // texture data is after the offset table, no patch header
    texgen = txcblock;  // ptr to whole patch
    texturecolumnofs[texnum] = colofs;
    texture->texture_model = TM_combine_patch;  // transparent combined
#if 0
    // Enable to print memory usage
    I_SoftError("R_GenerateTexture: %8s allocated %i, used %i bytes\n",
	    texture->name, txcblocksize, destpixels - texgen );
#endif
    goto done;
   
 exceed_alloc_error:   
    I_SoftError("R_GenerateTexture: %8s exceeds allocated block, make picture\n", texture->name );
    goto error_redo_as_picture;
   
 exceed_topdelta:
    I_SoftError("R_GenerateTexture: %8s topdelta= %i exceeds 254, make picture\n",
	    texture->name, segnxt_y );
    goto error_redo_as_picture;
   
 exceed_post_length:
    I_SoftError("R_GenerateTexture: %8s post length= %i exceeds 255, make picture\n",
	    texture->name, postlength );

 error_redo_as_picture:
    Z_Free( txcblock );
   
 picture_format:   
    //
    // multi-patch textures (or 'composite')
    // These are stored in a picture format and use a different drawing routine,
    // which is flagged by TM_picture.
    // Format:
    //   array[ width ] of column offset
    //   array[ width ] of column ( array[ height ] pixels )

    txcblocksize = colofs_size + (texture->width * texture->height);
    //CONS_Printf ("R_GenTex MULTI  %.8s size: %d\n",texture->name,txcblocksize);

    txcblock = Z_Malloc (txcblocksize,
                      PU_IN_USE,  // will change at end of this function
                      (void**)&texturecache[texnum]);

    memset( txcblock + colofs_size, 0, txcblocksize - colofs_size ); // black background
   
    // columns lookup table
    colofs = (uint32_t*)txcblock;  // has no patch header
   
    // [WDJ] column offset must cover entire texture, not just under patches
    // and it does not vary with the patches.
    x1 = colofs_size;  // offset to first column, past colofs array
    for (x=0; x<texture->width; x++)
    {
       // generate column offset lookup
       // colofs[x] = (x * texture->height) + (texture->width*4);
       colofs[x] = x1;
       x1 += texture->height;
    }

    // Composite the columns together.
    texpatch = texture->patches;
    for (i=0; i<texture->patchcount; i++, texpatch++)
    {
        // [WDJ] patch only used in this loop, without any other Z_Malloc
	// [WDJ] Must use common patch read to preserve endian consistency.
	// otherwise it will be in cache without endian changes.
        realpatch = W_CachePatchNum (texpatch->patchnum, PU_CACHE);  // patch temp
        x1 = texpatch->originx;
        x2 = x1 + realpatch->width;

        x = (x1<0)? 0 : x1;

        if (x2 > texture->width)
            x2 = texture->width;

        for ( ; x<x2 ; x++)
        {
	    // source patch column from wad, to be copied
	    column_t* patchcol =
	     (column_t *)( (byte *)realpatch + realpatch->columnofs[x-x1] );

            R_DrawColumnInCache (patchcol,		// source
                                 txcblock + colofs[x],	// dest
                                 texpatch->originy,
                                 texture->height);	// limit
        }
    }
    // Interface for texture picture format
    // texture data is after the offset table, no patch header
    texgen = txcblock + colofs_size;  // ptr to first column
    texturecolumnofs[texnum] = colofs;
    texture->texture_model = TM_picture;  // non-transparent picture format

done:
    // Now that the texture has been built in column cache,
    //  texturecache is purgable from zone memory.
    Z_ChangeTag (txcblock, PU_PRIV_CACHE);  // priority because of expense
    texturememory += txcblocksize;  // global

    return texgen;
}


//
// R_GetColumn
//


//
// new test version, short!
//
byte* R_GetColumn ( int           texnum,
                    int           col )
{
    byte*       data;

    col &= texturewidthmask[texnum];
    data = texturecache[texnum];

    if (!data)
        data = R_GenerateTexture (texnum);

    return data + texturecolumnofs[texnum][col];
}


//  convert flat to hicolor as they are requested
//
//byte**  flatcache;

byte* R_GetFlat (int  flatlumpnum)
{
   // [WDJ] Checking all callers shows that they might tolerate PU_CACHE,
   // but this is safer, and has less reloading of the flats
    return W_CacheLumpNum (flatlumpnum, PU_LUMP);	// flat image
//    return W_CacheLumpNum (flatlumpnum, PU_CACHE);

/*  // this code work but is useless
    byte*    data;
    short*   wput;
    int      i,j;

    //FIXME: work with run time pwads, flats may be added
    // lumpnum to flatnum in flatcache
    if ((data = flatcache[flatlumpnum-firstflat])!=0)
                return data;

    data = W_CacheLumpNum (flatlumpnum, PU_CACHE);
    i=W_LumpLength(flatlumpnum);

    Z_Malloc (i,PU_STATIC,&flatcache[flatlumpnum-firstflat]);
    memcpy (flatcache[flatlumpnum-firstflat], data, i);

    return flatcache[flatlumpnum-firstflat];
*/

/*  // this code don't work because it don't put a proper user in the z_block
    if ((data = flatcache[flatlumpnum-firstflat])!=0)
       return data;

    data = (byte *) W_CacheLumpNum(flatlumpnum,PU_LEVEL);
    flatcache[flatlumpnum-firstflat] = data;
    return data;

    flatlumpnum -= firstflat;

    if (vid.drawmode==DRAW8PAL)
    {
                flatcache[flatlumpnum] = data;
                return data;
    }

    // allocate and convert to high color

    wput = (short*) Z_Malloc (64*64*2,PU_STATIC,&flatcache[flatlumpnum]);
    //flatcache[flatlumpnum] =(byte*) wput;

    for (i=0; i<64; i++)
       for (j=0; j<64; j++)
                        wput[i*64+j] = ((color8to16[*data++]&0x7bde) + ((i<<9|j<<4)&0x7bde))>>1;

                //Z_ChangeTag (data, PU_CACHE);

                return (byte*) wput;
*/
}

//
// Empty the texture cache (used for load wad at runtime)
//
void R_FlushTextureCache (void)
{
    int i;

    if (numtextures>0)
    {
        for (i=0; i<numtextures; i++)
        {
            if (texturecache[i])
                Z_Free (texturecache[i]);
        }
    }
}

//
// R_LoadTextures
// Initializes the texture list with the textures from the world map.
//
// [WDJ] Original Doom bug: conflict between texture[0] and 0=no-texture.
// Their solution was to not use the first texture.
// In Doom1 == AASTINKY, FreeDoom == AASHITTY, Heretic = BADPATCH.
// Because any wad compatible with other doom engines, like prboom,
// will not be using texture[0], there is little incentive to fix this bug.
// It is likely that texture[0] will be a duplicate of some other texture.
#define BUGFIX_TEXTURE0
// 
//
void R_LoadTextures (void)
{
    maptexture_t*       mtexture;
    texture_t*          texture;
    mappatch_t*         mpatch;
    texpatch_t*         texpatch;
    char*               pnames;

    int                 i,j;

    uint32_t	      * maptex, * maptex1, * maptex2;  // 4 bytes, in wad
    uint32_t          * directory;	// in wad

    char                name[9];
    char*               name_p;

    int*                patchlookup;

    int                 nummappatches;
    int                 offset;
    int                 maxoff;
    int                 maxoff2;
    int                 numtextures1;
    int                 numtextures2;



    // free previous memory before numtextures change

    if (numtextures>0)
    {
        for (i=0; i<numtextures; i++)
        {
            if (textures[i])
                Z_Free (textures[i]);
            if (texturecache[i])
                Z_Free (texturecache[i]);
        }
    }

    // Load the patch names from pnames.lmp.
    name[8] = 0;
    pnames = W_CacheLumpName ("PNAMES", PU_LUMP);  // temp
    // [WDJ] Do endian as use pnames temp
    nummappatches = LE_SWAP32 ( *((uint32_t *)pnames) );  // pnames lump [0..3]
    name_p = pnames+4;  // in lump, after number (uint32_t) is list of patch names
    patchlookup = alloca (nummappatches*sizeof(*patchlookup));

    for (i=0 ; i<nummappatches ; i++)
    {
        strncpy (name,name_p+i*8, 8);
        patchlookup[i] = W_CheckNumForName (name);
    }
    Z_Free (pnames);

    // Load the map texture definitions from textures.lmp.
    // The data is contained in one or two lumps,
    //  TEXTURE1 for shareware, plus TEXTURE2 for commercial.
    maptex = maptex1 = W_CacheLumpName ("TEXTURE1", PU_LUMP); // will be freed
    numtextures1 = LE_SWAP32(*maptex);  // number of textures, lump[0..3]
    maxoff = W_LumpLength (W_GetNumForName ("TEXTURE1"));
    directory = maptex+1;  // after number of textures, at lump[4]

    if (W_CheckNumForName ("TEXTURE2") != -1)
    {
        maptex2 = W_CacheLumpName ("TEXTURE2", PU_LUMP); // will be freed
        numtextures2 = LE_SWAP32(*maptex2); // number of textures, lump[0..3]
        maxoff2 = W_LumpLength (W_GetNumForName ("TEXTURE2"));
    }
    else
    {
        maptex2 = NULL;
        numtextures2 = 0;
        maxoff2 = 0;
    }
#ifdef BUGFIX_TEXTURE0
#define FIRST_TEXTURE  1
    // [WDJ] make room for using 0 as no-texture and keeping first texture
    numtextures = numtextures1 + numtextures2 + 1;
#else   
    numtextures = numtextures1 + numtextures2;
#define FIRST_TEXTURE  0
#endif


    // [smite] separate allocations, fewer horrible bugs
    if (textures)
    {
	Z_Free(textures);
	Z_Free(texturecolumnofs);
	Z_Free(texturecache);
	Z_Free(texturewidthmask);
	Z_Free(textureheight);
    }

    textures         = Z_Malloc(numtextures * sizeof(*textures),         PU_STATIC, 0);
    texturecolumnofs = Z_Malloc(numtextures * sizeof(*texturecolumnofs), PU_STATIC, 0);
    texturecache     = Z_Malloc(numtextures * sizeof(*texturecache),     PU_STATIC, 0);
    texturewidthmask = Z_Malloc(numtextures * sizeof(*texturewidthmask), PU_STATIC, 0);
    textureheight    = Z_Malloc(numtextures * sizeof(*textureheight),    PU_STATIC, 0);

#ifdef BUGFIX_TEXTURE0
    for (i=0 ; i<numtextures-1 ; i++, directory++)
#else
    for (i=0 ; i<numtextures ; i++, directory++)
#endif
    {
        //only during game startup
        //if (!(i&63))
        //    CONS_Printf (".");

        if (i == numtextures1)
        {
            // Start looking in second texture file.
            maptex = maptex2;
            maxoff = maxoff2;
            directory = maptex+1;  // after number of textures, at lump[4]
        }

        // offset to the current texture in TEXTURESn lump
        offset = LE_SWAP32(*directory);  // next uint32_t

        if (offset > maxoff)
            I_Error ("R_LoadTextures: bad texture directory\n");

        // maptexture describes texture name, size, and
        // used patches in z order from bottom to top
	// Ptr to texture header in lump
        mtexture = (maptexture_t *) ( (byte *)maptex + offset);

        texture = textures[i] =
            Z_Malloc (sizeof(texture_t)
                      + sizeof(texpatch_t)*(LE_SWAP16(mtexture->patchcount)-1),
                      PU_STATIC, 0);

        // get texture info from texture lump
        texture->width  = LE_SWAP16(mtexture->width);
        texture->height = LE_SWAP16(mtexture->height);
        texture->patchcount = LE_SWAP16(mtexture->patchcount);
        texture->texture_model = (mtexture->masked)? TM_masked : TM_none; // hint

	// Sparc requires memmove, becuz gcc doesn't know mtexture is not aligned.
	// gcc will replace memcpy with two 4-byte read/writes, which will bus error.
        memmove(texture->name, mtexture->name, sizeof(texture->name));	
#if 0
        // [WDJ] DEBUG TRACE, watch where the textures go
        fprintf( stderr, "Texture[%i] = %8.8s\n", i, mtexture->name);
#endif
        mpatch = &mtexture->patches[0]; // first patch ind in texture lump
        texpatch = &texture->patches[0];

        for (j=0 ; j<texture->patchcount ; j++, mpatch++, texpatch++)
        {
	    // get texture patch info from texture lump
            texpatch->originx = LE_SWAP16(mpatch->originx);
            texpatch->originy = LE_SWAP16(mpatch->originy);
            texpatch->patchnum = patchlookup[LE_SWAP16(mpatch->patchnum)];
            if (texpatch->patchnum == -1)
            {
                I_Error ("R_LoadTextures: Missing patch in texture %s\n",
                         texture->name);
            }
        }

        // determine width power of 2
        j = 1;
        while (j*2 <= texture->width)
            j<<=1;

        texturewidthmask[i] = j-1;
        textureheight[i] = texture->height<<FRACBITS;
    }

    Z_Free (maptex1);
    if (maptex2)
        Z_Free (maptex2);

#ifdef BUGFIX_TEXTURE0
    // Move texture[0] to texture[numtextures-1]
    textures[numtextures-1] = textures[0];
    texturewidthmask[numtextures-1] = texturewidthmask[0];
    textureheight[numtextures-1] = textureheight[0];
    // cannot have ptr to texture in two places, will deallocate twice
    textures[0] = NULL;	// force segfault on any access to textures[0]
#endif   

    //added:01-04-98: this takes 90% of texture loading time..
    // Precalculate whatever possible.
    for (i=0 ; i<numtextures ; i++)
        texturecache[i] = NULL;

    // Create translation table for global animation.
    if (texturetranslation)
        Z_Free (texturetranslation);

    // texturetranslation is 1 larger than texture tables, for some unknown reason
    texturetranslation = Z_Malloc ((numtextures+1)*sizeof(*texturetranslation),
				   PU_STATIC, 0);

    for (i=0 ; i<numtextures ; i++)
        texturetranslation[i] = i;
}


int R_CheckNumForNameList(char *name, lumplist_t* list, int listsize)
{
  int   i;
  int   lump;
  for(i = listsize - 1; i > -1; i--)
  {
    lump = W_CheckNumForNamePwad(name, list[i].wadfile, list[i].firstlump);
    if((lump & 0xffff) > (list[i].firstlump + list[i].numlumps) || lump == -1)
      continue;
    else
      return lump;
  }
  return -1;
}


lumplist_t*  colormaplumps;
int          numcolormaplumps;

// called by R_InitColormaps
void R_InitExtraColormaps()
{
    int       startnum;
    int       endnum;
    int       cfile;
    int       clump;

    numcolormaplumps = 0;
    colormaplumps = NULL;
    cfile = clump = 0;

    for(;cfile < numwadfiles;cfile ++, clump = 0)
    {
        startnum = W_CheckNumForNamePwad("C_START", cfile, clump);
        if(startnum == -1)
            continue;

        endnum = W_CheckNumForNamePwad("C_END", cfile, clump);

        if(endnum == -1)
            I_Error("R_InitColormaps: C_START without C_END\n");

        if((startnum >> 16) != (endnum >> 16))
            I_Error("R_InitColormaps: C_START and C_END in different wad files!\n");

        colormaplumps = (lumplist_t *)realloc(colormaplumps, sizeof(lumplist_t) * (numcolormaplumps + 1));
        colormaplumps[numcolormaplumps].wadfile = startnum >> 16;
        colormaplumps[numcolormaplumps].firstlump = (startnum&0xFFFF) + 1;
        colormaplumps[numcolormaplumps].numlumps = endnum - (startnum + 1);
        numcolormaplumps++;
    }
}



lumplist_t*  flats;
int          numflatlists;

extern int   numwadfiles;


void R_InitFlats ()
{
  int       startnum;
  int       endnum;
  int       cfile;
  int       clump;

  numflatlists = 0;
  flats = NULL;
  cfile = clump = 0;

  for(;cfile < numwadfiles;cfile ++, clump = 0)
  {
#ifdef DEBUG_FLAT
    fprintf( stderr, "Flats in file %i\n", cfile );
#endif	 
    startnum = W_CheckNumForNamePwad("F_START", cfile, clump);
    if(startnum == -1)
    {
#ifdef DEBUG_FLAT
      fprintf( stderr, "F_START not found, file %i\n", cfile );
#endif	 
      clump = 0;
      startnum = W_CheckNumForNamePwad("FF_START", cfile, clump);

      if(startnum == -1) //If STILL -1, search the whole file!
      {
#ifdef DEBUG_FLAT
	fprintf( stderr, "FF_START not found, file %i\n", cfile );
#endif	 
        flats = (lumplist_t *)realloc(flats, sizeof(lumplist_t) * (numflatlists + 1));
        flats[numflatlists].wadfile = cfile;
        flats[numflatlists].firstlump = 0;
        flats[numflatlists].numlumps = 0xffff; //Search the entire file!
        numflatlists ++;
        continue;
      }
    }

    endnum = W_CheckNumForNamePwad("F_END", cfile, clump);
    if(endnum == -1) {
#ifdef DEBUG_FLAT
      fprintf( stderr, "F_END not found, file %i\n", cfile );
#endif	 
      endnum = W_CheckNumForNamePwad("FF_END", cfile, clump);
#ifdef DEBUG_FLAT
      if( endnum == -1 ) {
	 fprintf( stderr, "FF_END not found, file %i\n", cfile );
      }
#endif	 
    }

    if(endnum == -1 || (startnum &0xFFFF) > (endnum & 0xFFFF))
    {
      flats = (lumplist_t *)realloc(flats, sizeof(lumplist_t) * (numflatlists + 1));
      flats[numflatlists].wadfile = cfile;
      flats[numflatlists].firstlump = 0;
      flats[numflatlists].numlumps = 0xffff; //Search the entire file!
      numflatlists ++;
      continue;
    }

    flats = (lumplist_t *)realloc(flats, sizeof(lumplist_t) * (numflatlists + 1));
    flats[numflatlists].wadfile = startnum >> 16;
    flats[numflatlists].firstlump = (startnum&0xFFFF) + 1;
    flats[numflatlists].numlumps = endnum - (startnum + 1);
    numflatlists++;
    continue;
  }

  if(!numflatlists)
    I_Error("R_InitFlats: No flats found!\n");
}



int R_GetFlatNumForName(char *name)
{
  // [WDJ] No use in saving F_START if are not going to use them.
  // FreeDoom, where a flat and sprite both had same name,
  // would display sprite as a flat, badly.
  // Use F_START and F_END first, to find flats without getting a non-flat,
  // and only if not found then try whole file.
  
  int lump = R_CheckNumForNameList(name, flats, numflatlists);

  if(lump == -1) {
     // BP:([WDJ] R_CheckNumForNameList) don't work with gothic2.wad
     // [WDJ] Some wads are reported to use a flat as a patch, but that would
     // have to be handled in the patch display code.
     // If this search finds a sprite, sound, etc., it will display
     // multi-colored confetti.
     lump = W_CheckNumForName(name);
  }
  
  if(lump == -1) {
     // [WDJ] When not found, dont quit, use first flat by default.
     I_SoftError("R_GetFlatNumForName: Could not find flat %.8s\n", name);
     lump = flats[0].firstlump;	// default to first flat
  }

  return lump;
}

// [WDJ] Manage the spritelump_t allocations
// Still use this array so that rot=0 sprite can share one entry for all 8 rotations.

void expand_spritelump( void )
{
    // [WDJ] Expand array and copy
    num_free_spritelump = 256;
    int request = num_spritelump + num_free_spritelump;
    // new array of spritelumps
    spritelump_t * sl_new = Z_Malloc (request*sizeof(spritelump_t), PU_STATIC, 0);
    if( spritelumps ) // existing
    {
        // move existing data
        memcpy( sl_new, spritelumps, sizeof(spritelump_t)*num_spritelump);
        Z_Free( spritelumps ); // old
    }
    spritelumps = sl_new;
}

// Next free spritelump
int  R_Get_spritelump( void )
{
    if( num_free_spritelump == 0 )
       expand_spritelump();
    num_free_spritelump--;
    return  num_spritelump ++;
}

//
// R_InitSpriteLumps
// Finds the width and hoffset of all sprites in the wad,
//  so the sprite does not need to be cached completely
//  just for having the header info ready during rendering.
//

//
//   allocate sprite lookup tables
//
void R_InitSpriteLumps (void)
{
    // the original Doom used to set numspritelumps from S_END-S_START+1

    // [WDJ] Initial allocation, will be expanded as needed
    expand_spritelump();
}


// ----------------------------------------
// COLORMAPS
// 
// COLORMAP lump : array[34]  of mapping tables [256].
//   [0]  : brightest, light level = 248..255, maps direct to palette colors
//   [31] : darkest, light level = 0..7,       maps to grey,black
//   [32] : invulnerability map,               maps to whites and grays
//   [33] : black map,			       maps entirely to black
//   
// Specific palette colors:
//  [0] = black 0,0,0
//  [4] = white 255,255,255
//  [176] = red 255,0,0
//  [112] = green 103,223,95
//  [200] = blue 0,0,255

// Boom: Extra colormaps can be included in the wad.  They are the same
// size and format as the Doom colormap.
// WATERMAP is predefined by Boom, but may be overloaded.

void R_InitExtraColormaps();
void R_ClearColormaps();

//
// R_InitColormaps
//
// called by R_InitData
void R_InitColormaps (void)
{
    int lump;

    // Load in the standard colormap lightmap tables (defined by lump),
    // now 64k aligned for smokie...
    lump = W_GetNumForName("COLORMAP");
    reg_colormaps = Z_MallocAlign (W_LumpLength (lump), PU_STATIC, 0, 16);
    W_ReadLump (lump, reg_colormaps);

    //SoM: 3/30/2000: Init Boom colormaps.
    {
      R_ClearColormaps();
      R_InitExtraColormaps();
    }
}


int    fnd_colormap_lump[MAXCOLORMAPS];  // lump number

//SoM: Clears out extra colormaps between levels.
// called by P_SetupLevel after ZFree(PU_LEVEL,..)
// called by R_InitColormaps
void R_ClearColormaps()
{
  int   i;
#if 0   
  if( num_extra_colormaps > 30 )
     fprintf( stderr, "Number of colormaps: %i\n", num_extra_colormaps );
#endif

  num_extra_colormaps = 0;
  for(i = 0; i < MAXCOLORMAPS; i++)
  {
    fnd_colormap_lump[i] = -1;
    // The ZMalloc colormap was PU_LEVEL and thus is already cleared.
  }
  memset(extra_colormaps, 0, sizeof(extra_colormaps));
}


// [WDJ] Enable to print out results of colormap generate.
//#define VIEW_COLORMAP_GEN

// In order: whiteindex, greyindex, redindex, greenindex, blueindex
static byte  doom_analyze_index[5] = {
    4, 97, 176, 112, 200
};
static byte  heretic_analyze_index[5] = {
    35, 17, 160, 217, 196
};

// [WDJ] Analyze an extra colormap to derive some GL parameters
void  R_Colormap_Analyze( int mapnum )
{
    extracolormap_t * colormapp = & extra_colormaps[ mapnum ];
    colormapp->fadestart = 0;
    colormapp->fadeend = 33;
    colormapp->maskamt = 0x0;
    colormapp->fog = 0;

#ifdef HWRENDER
  // Hardware renderer does not use, nor allocate, the colormap.
  // It uses the rgba field instead.
  if(rendermode == render_soft)
#endif     
  {
    // Software renderer defaults.
    // SoM: Added, we set all params of the colormap to normal because there
    // is no real way to tell how GL should handle a colormap lump anyway..
    colormapp->maskcolor = 0x00ffffff; // white
    colormapp->fadecolor = 0x0; // black
    colormapp->rgba[0] = 0x17ffffff;
  }
#ifdef HWRENDER
  else
  {
    // Analyze the Boom colormap for the hardware renderer.
    // The Boom colormap has been loaded already.
    // lighttable_t = byte array
    byte * cm = extra_colormaps[mapnum].colormap; // Boom colormap
    byte * tstcolor = & doom_analyze_index[0];  // colors to test
    RGBA_t work_rgba;
    int i;

    if( gamemode == heretic )
       tstcolor = & heretic_analyze_index[0];  // heretic colors to test

#if 0
    // For new port, analyze the colormap to find white, grey, red, blue, green.
    // This only affects this color analyzer, and is not fatal.
    int whiteness = 0;
    int greyness = 0;
    int redness = 0;
    int greenness = 0;
    int blueness = 0;
    for(i=0; i<256; i++)
    {
        int cr = pLocalPalette[i].s.red;
        int cg = pLocalPalette[i].s.green;
        int cb = pLocalPalette[i].s.blue;
        int n = cr+cg+cb;
        if( n>whiteness )
        {
            whiteness = n;
	    tstcolor[0] = i;
	}
        n = ((cr > 128)? 255 - cr : cr) + ((cg > 128)? 255 - cg : cg) + ((cb > 128)? 255 - cb : cb);
        if( n>greyness )  // med grey
        {
	    greyness = n;
            tstcolor[1] = i;
	}
        n = cr-cg-cb;
        if( n>redness )
        {
	    redness = n;
	    tstcolor[2] = i; // red
	}
        n= cg-cr-cb;
        if( n>greenness )
        {
	    greenness = n;
	    tstcolor[3] = i; // green
	}
        n= cb-cr-cg;
        if( n>blueness )
        {
	    blueness = n;
	    tstcolor[4] = i;  // blue
	}
    }
#ifdef VIEW_COLORMAP_GEN
    fprintf(stderr,"White index=%i, grey index=%i, red index=%i, green index=%i, blue index=%i\n",
	   tstcolor[0], tstcolor[1], tstcolor[2], tstcolor[3], tstcolor[4]);
#endif   
#endif   

    // [WDJ] Analyze the colormap to get some imitative parameters.
    for(i=0; i<NUM_RGBA_LEVELS; i++)
    {
        // rgba[0]=darkest, rgba[NUM_RGBA_LEVELS-1] is the brightest
        // Within a range, analyze a map nearer the middle of the range.
        int mapindex = (((NUM_RGBA_LEVELS-1-i) * 32) + 8) / NUM_RGBA_LEVELS;
        // Estimate an alpha, could set it to max,
        // smaller alpha indicates washed out color.
        // This is completely ad-hoc, and rough.  It purposely favors high alpha.

// Analyze 4
        // the brightest red, green, blue should have the same brightness
        float h4r = 0.0; // alpha = 1
        int cnt =0;
        int try_cnt = 0;
        int dd4, dn4, k1, k2;
        // for all combinations of tstcolor
        for( k1=0; k1<4; k1++ )
        {
	    byte i1 = tstcolor[k1];
	    byte cm1 = cm[ LIGHTTABLE(mapindex) + i1 ];
	    for( k2=k1+1; k2<5; k2++ )
	    {
	        byte i2 = tstcolor[k2];
	        byte cm2 = cm[ LIGHTTABLE(mapindex) + i2 ];
	        // for each color
	        int krgb;
	        for( krgb=0; krgb<3; krgb++ )  // red, green, blue
	        { 
		    //  (1-h) = (cm[b].r - cm[r].r) / (p[b].r -  p[r].r)
		    switch( krgb )
		    {
		     case 0: // red
		        dd4 = pLocalPalette[i1].s.red -  pLocalPalette[i2].s.red;
		        dn4 = pLocalPalette[cm1].s.red - pLocalPalette[cm2].s.red;
		        break;
		     case 1: // green
		        dd4 = pLocalPalette[i1].s.green -  pLocalPalette[i2].s.green;
		        dn4 = pLocalPalette[cm1].s.green - pLocalPalette[cm2].s.green;
		        break;
		     default: // blue
		        dd4 = pLocalPalette[i1].s.blue -  pLocalPalette[i2].s.blue;
		        dn4 = pLocalPalette[cm1].s.blue - pLocalPalette[cm2].s.blue;
		        break;
		    }
		    if( dn4 && (dd4 > 0.01 || dd4 < -0.01))
		    {
		        float h3 = (float)dn4 / (float)dd4;
		        if( h3 > 1.0 )   h3 = 1.0;
		        if( h3 < 0.01 )  h3 = 0.01;
		        h4r += h3;  // total for avg
		        cnt ++;
		    }
		    try_cnt ++;  // for fog
		}
	    }
	}
        h4r /= cnt;
        int m4_fog = 0;
        // h = 0.0 is a useless table
        if( h4r > 0.99 ) {
	    h4r = 0.99;
	    m4_fog = 1;
	}
        if( h4r < 0.00 ) {
	    h4r = 0.00;
	    m4_fog = 1;
	}
        if( cnt * 2 < try_cnt )  m4_fog = 1;
        h4r = h4r * (0.66 / 1.732);  // correction for testing at 1/3 max brightness
        float h4 = 1.0 - h4r;
        int m4_red, m4_blue, m4_green;
#if 0
        // Generate color tint from changes grey.
        byte greyindex = tstcolor[1];
        byte cm_grey = cm[ LIGHTTABLE(mapindex) + greyindex ];  // white
        // cr = (cm[w].r - (1-h) * p[w].r) / h ;
        m4_red = ( pLocalPalette[cm_grey].s.red - (h4r* pLocalPalette[greyindex].s.red)) / h4;
        if( m4_red > 255 ) m4_red = 255;
        if( m4_red < 0 ) m4_red = 0;
        // cg = (cm[w].g - (1-h) * p[w].g) / h ;
        m4_green = ( pLocalPalette[cm_grey].s.green - (h4r* pLocalPalette[greyindex].s.green)) / h4;
        if( m4_green > 255 ) m4_green = 255;
        if( m4_green < 0 ) m4_green = 0;
        // cb = (cm[w].b - (1-h) * p[w].b) / h ;
        m4_blue = ( pLocalPalette[cm_grey].s.blue - (h4r* pLocalPalette[greyindex].s.blue)) / h4;
        if( m4_blue > 255 ) m4_blue = 255;
        if( m4_blue < 0 ) m4_blue = 0;
#else
        // Generate color tint from changes white.
        byte whiteindex = tstcolor[0];
        byte cm_white = cm[ LIGHTTABLE(mapindex) + whiteindex ];  // white
        // cr = (cm[w].r - (1-h) * p[w].r) / h ;
        m4_red = ( pLocalPalette[cm_white].s.red - (h4r* pLocalPalette[whiteindex].s.red)) / h4;
        if( m4_red > 255 ) m4_red = 255;
        if( m4_red < 0 ) m4_red = 0;
        // cg = (cm[w].g - (1-h) * p[w].g) / h ;
        m4_green = ( pLocalPalette[cm_white].s.green - (h4r* pLocalPalette[whiteindex].s.green)) / h4;
        if( m4_green > 255 ) m4_green = 255;
        if( m4_green < 0 ) m4_green = 0;
        // cb = (cm[w].b - (1-h) * p[w].b) / h ;
        m4_blue = ( pLocalPalette[cm_white].s.blue - (h4r* pLocalPalette[whiteindex].s.blue)) / h4;
        if( m4_blue > 255 ) m4_blue = 255;
        if( m4_blue < 0 ) m4_blue = 0;
#endif
#ifdef VIEW_COLORMAP_GEN
        // Enable to see results of analysis.
        fprintf( stderr,
	       "Analyze4: alpha=%i  red=%i  green=%i  blue=%i  fog=%i\n",
	       (int)(255.0*h4), m4_red, m4_green, m4_blue, m4_fog );
#endif      
        // Not great, get some tints wrong, and sometimes too light.
        work_rgba.s.alpha = (int)(h4 * 26.0);
        work_rgba.s.red = m4_red;
        work_rgba.s.green = m4_green;
        work_rgba.s.blue = m4_blue;

        colormapp->rgba[i] = work_rgba.rgba;  // to extra_colormap

#ifdef VIEW_COLORMAP_GEN
        // Enable to view settings of RGBA for hardware renderer.
        fprintf(stderr,"RGBA[%i]: %8x   (alpha=%i, R=%i, G=%i, B=%i)\n",
	      i, colormapp->rgba[i],
	      (int)work_rgba.s.alpha,
	      (int)work_rgba.s.red, (int)work_rgba.s.green, (int)work_rgba.s.blue );
#endif
    }
    // They had plans, but these are still unused in hardware renderer.
    colormapp->maskcolor = colormapp->rgba[NUM_RGBA_LEVELS-1];
    colormapp->fadecolor = colormapp->rgba[0];
//   colormapp->maskcolor = ((cb) >> 3) + (((cg) >> 2) << 5) + (((cr) >> 3) << 11);
  
  }
#endif
}


// [WDJ] The name parameter has trailing garbage, but the name lookup
// only uses the first 8 chars.
// Return the new colormap id number
int R_ColormapNumForName(char *name)
{
  int lump, i;

  // Check for existing colormap of same name
  lump = R_CheckNumForNameList(name, colormaplumps, numcolormaplumps);
  if(lump == -1)
    I_Error("R_ColormapNumForName: Cannot find colormap lump %8s\n", name);

  for(i = 0; i < num_extra_colormaps; i++)
    if(lump == fnd_colormap_lump[i])
      return i;

  // Add another colormap
  if(num_extra_colormaps == MAXCOLORMAPS)
    I_Error("R_ColormapNumForName: Too many colormaps!\n");

  fnd_colormap_lump[num_extra_colormaps] = lump;

  // aligned on 8 bit for asm code
  extra_colormaps[num_extra_colormaps].colormap = Z_MallocAlign (W_LumpLength (lump), PU_LEVEL, 0, 8);
  // read colormap tables [][] of byte, mapping to alternative palette colors
  W_ReadLump (lump, extra_colormaps[num_extra_colormaps].colormap);

#ifdef VIEW_COLORMAP_GEN
  fprintf(stderr, "\nBoom Colormap: num=%i name= %8.8s\n", num_extra_colormaps, name );
#endif
  R_Colormap_Analyze( num_extra_colormaps );
  
  num_extra_colormaps++;
  return num_extra_colormaps - 1;
}



// SoM:
//
// R_CreateColormap
// This is a more GL friendly way of doing colormaps: Specify colormap
// data in a special linedef's texture areas and use that to generate
// custom colormaps at runtime. NOTE: For GL mode, we only need to color
// data and not the colormap data. 
double  deltas[256][3], map[256][3];

byte NearestColor(byte r, byte g, byte b);
int  RoundUp(double number);

#define HEX_TO_INT(x) (x >= '0' && x <= '9' ? x - '0' : x >= 'a' && x <= 'f' ? x - 'a' + 10 : x >= 'A' && x <= 'F' ? x - 'A' + 10 : 0)
#define ALPHA_TO_INT(x) (x >= 'a' && x <= 'z' ? x - 'a' : x >= 'A' && x <= 'Z' ? x - 'A' : 0)
#define CHAR_TO_INT(c)  (c >= '0' && c <= '9' ? c - '0' : 0)
#define ABS2(x) ((x) < 0 ? -(x) : (x))

// [WDJ]
// The colorstr is the toptexture name:   '#' R(2) G(2) B(2) A(1)
// The ctrlstr is the midtexture name.    '#' FOG(1) FADE_BEGIN(2)  FADE_END(2)
// The fadestr is the bottomtexture name: '#' R(2) G(2) B(2).
// Translations:
//   R(2), G(2), B(2):   2-digit HEX => 256 color (8bit)
//   A(1): char a-z or A-Z  => 0 to 25 alpha
//   FOG: '0'= not-fog, '1'= fog-effect
//   FADE_BEGIN: 2-digit decimal (0..32) colormap where fade begins
//   FADE_END:   2-digit decimal (1..33) colormap where fade-to color is reached.
// Return the new colormap id number
int R_CreateColormap(char *colorstr, char *ctrlstr, char *fadestr)
{
  double color_r, color_g, color_b; // color RGB from top-texture
  double cfade_r, cfade_g, cfade_b; // fade-to color from bottom-texture
  double r, g, b;
  double cbrightness;
  double maskamt = 0, othermask = 0;
  int    alphamask = 0;
  int    i, p;
  unsigned int  cr, cg, cb;  // color RGB as INT
  unsigned int  maskcolor, fadecolor;
  unsigned int  fadestart = 0, fadeend = 33, fadedist = 33;
  int    fog = 0;
  int    mapnum = num_extra_colormaps;  // the new colormap id number
  extracolormap_t * extra_colormap_p;
  byte  *colormap_p;

  if(colorstr[0] == '#')  // colormap generate string is recognized
  {
    // color value from top texture string
    cr = color_r = ((HEX_TO_INT(colorstr[1])<<4) + HEX_TO_INT(colorstr[2]));
    cg = color_g = ((HEX_TO_INT(colorstr[3])<<4) + HEX_TO_INT(colorstr[4]));
    cb = color_b = ((HEX_TO_INT(colorstr[5])<<4) + HEX_TO_INT(colorstr[6]));
    // Create a rough approximation of the color (a 16 bit color)
    //  16bit, color 5:6:5
    maskcolor = ((cb) >> 3) + (((cg) >> 2) << 5) + (((cr) >> 3) << 11);
    //  16bit, color 5:5:5
//    maskcolor = ((cb) >> 3) + (((cg) >> 3) << 5) + (((cr) >> 3) << 10);
    if(colorstr[7] >= 'a' && colorstr[7] <= 'z')
      alphamask = (colorstr[7] - 'a');
    else if(colorstr[7] >= 'A' && colorstr[7] <= 'Z')
      alphamask = (colorstr[7] - 'A');
    else
      alphamask = 24;


    maskamt = (double)alphamask / (double)24;

    othermask = 1 - maskamt;
    maskamt /= 0xff;
  }
  else
  {
    // [WDJ] default for missing upper is not in docs, and was inconsistent
    // Cannot be 0xff after multiply by maskamt=0.
    color_r = 0xff;
    color_g = 0xff;
    color_b = 0xff;
    maskamt = 0;
    //  16bit, color 5:6:5
    maskcolor = ((0xff) >> 3) + (((0xff) >> 2) << 5) + (((0xff) >> 3) << 11);
    //  16bit, color 5:5:5
//    maskcolor = ((0xff) >> 3) + (((0xff) >> 3) << 5) + (((0xff) >> 3) << 10);
  }


  if(ctrlstr[0] == '#')
  {
    // SoM: Get parameters like, fadestart, fadeend, and the fogflag...
    fadestart = CHAR_TO_INT(ctrlstr[3]) + (CHAR_TO_INT(ctrlstr[2]) * 10);
    fadeend = CHAR_TO_INT(ctrlstr[5]) + (CHAR_TO_INT(ctrlstr[4]) * 10);
    if(fadestart > 32 || fadestart < 0)
      fadestart = 0;
    if(fadeend > 33 || fadeend < 1)
      fadeend = 33;
    fadedist = fadeend - fadestart;
    fog = CHAR_TO_INT(ctrlstr[1]) ? 1 : 0;
  }


  if(fadestr[0] == '#')
  {
    cfade_r = cr = ((HEX_TO_INT(fadestr[1]) * 16) + HEX_TO_INT(fadestr[2]));
    cfade_g = cg = ((HEX_TO_INT(fadestr[3]) * 16) + HEX_TO_INT(fadestr[4]));
    cfade_b = cb = ((HEX_TO_INT(fadestr[5]) * 16) + HEX_TO_INT(fadestr[6]));
    //  16bit, color 5:6:5
    fadecolor = (((cb) >> 3) + (((cg) >> 2) << 5) + (((cr) >> 3) << 11));
    //  16bit, color 5:5:5
//    fadecolor = (((cb) >> 3) + (((cg) >> 3) << 5) + (((cr) >> 3) << 10));
  }
  else
  {
    cfade_r = 0;
    cfade_g = 0;
    cfade_b = 0;
    fadecolor = 0;
  }

  for(i = 0; i < num_extra_colormaps; i++)
  {
    if(fnd_colormap_lump[i] != -1)
      continue;
    if(maskcolor == extra_colormaps[i].maskcolor &&
       fadecolor == extra_colormaps[i].fadecolor &&
       maskamt == extra_colormaps[i].maskamt &&
       fadestart == extra_colormaps[i].fadestart &&
       fadeend == extra_colormaps[i].fadeend &&
       fog == extra_colormaps[i].fog)
      return i;
  }

#ifdef VIEW_COLORMAP_GEN
  fprintf(stderr, "\nGenerate Colormap: num=%i\n", num_extra_colormaps );
  fprintf(stderr, " alpha=%2x, color=(%2x,%2x,%2x), fade=(%2x,%2x,%2x), fog=%i\n",
	  alphamask, (int)color_r, (int)color_g, (int)color_b,
	  	     (int)cfade_r, (int)cfade_g, (int)cfade_b, fog );
#endif

  if(num_extra_colormaps == MAXCOLORMAPS)
    I_Error("R_CreateColormap: Too many colormaps!\n");
  num_extra_colormaps++;

  fnd_colormap_lump[mapnum] = -1;
  extra_colormap_p = &extra_colormaps[mapnum];

  // aligned on 8 bit for asm code
  extra_colormap_p->colormap = NULL;
  extra_colormap_p->maskcolor = maskcolor;
  extra_colormap_p->fadecolor = fadecolor;
  extra_colormap_p->maskamt = maskamt;
  extra_colormap_p->fadestart = fadestart;
  extra_colormap_p->fadeend = fadeend;
  extra_colormap_p->fog = fog;

#ifdef HWRENDER
  // Hardware renderer does not use, nor allocate, the colormap.
  // It uses the rgba field instead.
  if(rendermode == render_soft)
#endif
  {
    extra_colormap_p->colormap = colormap_p = Z_MallocAlign((256 * 34) + 10, PU_LEVEL, 0, 16); // Aligning on 16 bits, NOT 8, keeps it from crashing! SSNTails 12-13-2002
     
    // premultiply, this messes up rgba calc so it must be done here
    color_r *= maskamt;
    color_g *= maskamt;
    color_b *= maskamt;
    for(i = 0; i < 256; i++)
    {
      r = pLocalPalette[i].s.red;
      g = pLocalPalette[i].s.green;
      b = pLocalPalette[i].s.blue;
      cbrightness = sqrt((r*r) + (g*g) + (b*b));


      map[i][0] = (cbrightness * color_r) + (r * othermask);
      if(map[i][0] > 255.0)
        map[i][0] = 255.0;
      deltas[i][0] = (map[i][0] - cfade_r) / (double)fadedist;

      map[i][1] = (cbrightness * color_g) + (g * othermask);
      if(map[i][1] > 255.0)
        map[i][1] = 255.0;
      deltas[i][1] = (map[i][1] - cfade_g) / (double)fadedist;

      map[i][2] = (cbrightness * color_b) + (b * othermask);
      if(map[i][2] > 255.0)
        map[i][2] = 255.0;
      deltas[i][2] = (map[i][2] - cfade_b) / (double)fadedist;
    }

    for(p = 0; p < 34; p++)
    {
      for(i = 0; i < 256; i++)
      {
        *colormap_p = NearestColor(RoundUp(map[i][0]), RoundUp(map[i][1]), RoundUp(map[i][2]));
        colormap_p++;
  
        if((unsigned int)p < fadestart)
          continue;
  
        if(ABS2(map[i][0] - cfade_r) > ABS2(deltas[i][0]))
          map[i][0] -= deltas[i][0];
        else
          map[i][0] = cfade_r;

        if(ABS2(map[i][1] - cfade_g) > ABS2(deltas[i][1]))
          map[i][1] -= deltas[i][1];
        else
          map[i][1] = cfade_g;

        if(ABS2(map[i][2] - cfade_b) > ABS2(deltas[i][1]))
          map[i][2] -= deltas[i][2];
        else
          map[i][2] = cfade_b;
      }
    }
  }
#ifdef HWRENDER
  else
  {
      // hardware needs color_r, color_g, color_b, before they get *maskamt.
      for( i=0; i<NUM_RGBA_LEVELS; i++ )
      {
	  // rgba[0]=darkest, rgba[NUM_RGBA_LEVELS-1] is the brightest
          int mapindex = ((NUM_RGBA_LEVELS-1-i) * 34) / NUM_RGBA_LEVELS;
          double colorper, fadeper;
          if( mapindex <= fadestart )
          {
	      colorper = 1.0; // rgba = color
              fadeper = 0.0;
          }
          else if( mapindex < fadeend )
          {
              // mapindex != fadeend, fadeend != fadestart
	      // proportional ramp from color to fade color
	      colorper = ((double)(fadeend-mapindex))/(fadeend-fadestart);
	      fadeper = 1.0 - colorper;
	  }
	  else
	  {
	      // fadeend and after
	      colorper = 0.0;
	      fadeper = 1.0; // rgba = fade
	  }
	  cr = (int)( colorper*color_r + fadeper*cfade_r );
	  cg = (int)( colorper*color_g + fadeper*cfade_g );
	  cb = (int)( colorper*color_b + fadeper*cfade_b );
	  extra_colormap_p->rgba[i] = (alphamask<<24)|(cb<<16)|(cg<<8)|(cr);
#ifdef VIEW_COLORMAP_GEN
	  fprintf(stderr,"RGBA[%i]: %x\n", i, extra_colormap_p->rgba[i]);
#endif
      }
#if 0
#ifdef VIEW_COLORMAP_GEN
    // [WDJ] DEBUG, TEST AGAINST OLD HDW CODE
    unsigned rgba_oldhw = 
	  (HEX_TO_INT(colorstr[1]) << 4) + (HEX_TO_INT(colorstr[2]) << 0) +
	  (HEX_TO_INT(colorstr[3]) << 12) + (HEX_TO_INT(colorstr[4]) << 8) +
	  (HEX_TO_INT(colorstr[5]) << 20) + (HEX_TO_INT(colorstr[6]) << 16) + 
	  (ALPHA_TO_INT(colorstr[7]) << 24);
   if( rgba_oldhw != extra_colormap_p->rgba[0] )
      fprintf(stderr,"RGBA: old=%X, new=%x\n", rgba_oldhw, extra_colormap_p->rgba[i-1]);
#endif
#endif
  }
#endif	       

  return mapnum;
}
//  #undef getnum
#undef ALPHA_TO_INT
#undef HEX_TO_INT
#undef ABS2


//Thanks to quake2 source!
//utils3/qdata/images.c
byte NearestColor(byte r, byte g, byte b)
{
  int dr, dg, db;
  int distortion;
  int bestdistortion = 256 * 256 * 4;
  int bestcolor = 0;
  int i;

  for(i = 0; i < 256; i++) {
    dr = r - pLocalPalette[i].s.red;
    dg = g - pLocalPalette[i].s.green;
    db = b - pLocalPalette[i].s.blue;
    distortion = dr*dr + dg*dg + db*db;
    if(distortion < bestdistortion) {

      if(!distortion)
        return i;

      bestdistortion = distortion;
      bestcolor = i;
      }
    }

  return bestcolor;
}


// Rounds off floating numbers and checks for 0 - 255 bounds
int RoundUp(double number)
{
  if(number > 255.0)
    return 255.0;
  if(number < 0)
    return 0;

  if((int)number <= (int)(number -0.5))
    return (int)number + 1;

  return (int)number;
}




char *R_ColormapNameForNum(int num)
{
  if(num == -1)
    return "NONE";

  if(num < 0 || num > MAXCOLORMAPS)
    I_Error("R_ColormapNameForNum: num is invalid!\n");

  if(fnd_colormap_lump[num] == -1)
    return "INLEVEL";

//  return wadfiles[fnd_colormap_lump[num] >> 16]->lumpinfo[fnd_colormap_lump[num] & 0xffff].name;
  return wadfiles[WADFILENUM(fnd_colormap_lump[num])]->lumpinfo[LUMPNUM(fnd_colormap_lump[num])].name;
}


//
//  build a table for quick conversion from 8bpp to 15bpp
//
int makecol15(int r, int g, int b)
{
   return (((r >> 3) << 10) | ((g >> 3) << 5) | ((b >> 3)));
}

void R_Init8to16 (void)
{
    byte*       palette;
    int         i;

    palette = W_CacheLumpName ("PLAYPAL",PU_CACHE);  // temp, used next loop

    for (i=0;i<256;i++)
    {
                // doom PLAYPAL are 8 bit values
        color8to16[i] = makecol15 (palette[0],palette[1],palette[2]);
        palette += 3;
    }
    // end PLAYPAL lump use

    // test a big colormap
    hicolormaps = Z_Malloc (32768 /**34*/, PU_STATIC, 0);
    for (i=0;i<16384;i++)
         hicolormaps[i] = i<<1;
}


//
// R_InitData
// Locates all the lumps
//  that will be used by all views
// Must be called after W_Init.
//
void R_InitData (void)
{
    //fab highcolor
    if (highcolor)
    {
        CONS_Printf ("\nInitHighColor...");
        R_Init8to16 ();
    }

    CONS_Printf ("\nInitTextures...");
    R_LoadTextures ();
    CONS_Printf ("\nInitFlats...");
    R_InitFlats ();

    CONS_Printf ("\nInitSprites...\n");
    R_InitSpriteLumps ();
    R_InitSprites (sprnames);

    CONS_Printf ("\nInitColormaps...\n");
    R_InitColormaps ();
}


//SoM: REmoved R_FlatNumForName


//
// R_CheckTextureNumForName
// Check whether texture is available.
// Filter out NoTexture indicator.
//
// [WDJ] Original Doom bug: conflict between texture[0] and 0=no-texture.
// 
// Parameter name is 8 char without term.
// Return -1 for not found, 0 for no texture
int     R_CheckTextureNumForName (char *name)
{
    int         i;

    // "NoTexture" marker.
    if (name[0] == '-')
        return 0;

    for (i=FIRST_TEXTURE ; i<numtextures ; i++)
        if (!strncasecmp (textures[i]->name, name, 8) )
            return i;
#ifdef RANGECHECK
    if( i == 0 )
        fprintf( stderr, "Texture %8.8s  is texture[0], imitates no-texture.\n", name);
#endif   
   
    return -1;
}



//
// R_TextureNumForName
// Calls R_CheckTextureNumForName,
//
// Return  0 for no texture "-".
// Return default texture when texture not found (would HOM otherwise).
// Parameter name is 8 char without term.
// Is used for side_t texture fields, which are used for array access
// without further error checks, so never returns -1.
int R_TextureNumForName (char* name)
{
    int i;

    i = R_CheckTextureNumForName (name);
#if 0
// [WDJ] DEBUG TRACE, to see where textures have ended up and which are accessed.
#  define trace_SIZE 512
   static char debugtrace_RTNFN[ trace_SIZE ];
   if( i<trace_SIZE && debugtrace_RTNFN[i] != 0x55 ) {
      fprintf( stderr, "Texture %8.8s is texture[%i]\n", name, i);
      debugtrace_RTNFN[i] = 0x55;
   }
#  undef trace_SIZE   
#endif   

    if (i==-1)
    {
        //I_Error ("R_TextureNumForName: %.8s not found", name);
        CONS_Printf("WARNING: R_TextureNumForName: %.8s not found\n", name);
        i=1;	// default to texture[1]
    }
    return i;
}




//
// R_PrecacheLevel
// Preloads all relevant graphics for the level.
//

// BP: rules : no extern in c !!!
//     slution put a new function in p_setup.c or put this in global (not recommended)
// SoM: Ok.. Here it goes. This function is in p_setup.c and caches the flats.
int P_PrecacheLevelFlats();

void R_PrecacheLevel (void)
{
//  char*               flatpresent; //SoM: 4/18/2000: No longer used
    char*               texturepresent;
    char*               spritepresent;

    int                 i;
    int                 j;
    int                 k;
    int                 lump;

    thinker_t*          th;
    spriteframe_t*      sf;

    //int numgenerated;  //faB:debug

    if (demoplayback)
        return;

    // do not flush the memory, Z_Malloc twice with same user
    // will cause error in Z_CheckHeap(), 19991022 by Kin
    if (rendermode != render_soft)
        return;

    // Precache flats.
    /*flatpresent = alloca(numflats);
    memset (flatpresent,0,numflats);

    // Check for used flats
    for (i=0 ; i<numsectors ; i++)
    {
#ifdef PARANOIA
        if( sectors[i].floorpic<0 || sectors[i].floorpic>numflats )
            I_Error("sectors[%d].floorpic=%d out of range [0..%d]\n",i,sectors[i].floorpic,numflats);
        if( sectors[i].ceilingpic<0 || sectors[i].ceilingpic>numflats )
            I_Error("sectors[%d].ceilingpic=%d out of range [0..%d]\n",i,sectors[i].ceilingpic,numflats);
#endif
        flatpresent[sectors[i].floorpic] = 1;
        flatpresent[sectors[i].ceilingpic] = 1;
    }

    flatmemory = 0;

    for (i=0 ; i<numflats ; i++)
    {
        if (flatpresent[i])
        {
            lump = firstflat + i;
            if(devparm)
               flatmemory += W_LumpLength(lump);
            R_GetFlat (lump);
//            W_CacheLumpNum(lump, PU_CACHE);
        }
    }*/
    flatmemory = P_PrecacheLevelFlats();

    //
    // Precache textures.
    //
    // no need to precache all software textures in 3D mode
    // (note they are still used with the reference software view)
    texturepresent = alloca(numtextures);
    memset (texturepresent,0, numtextures);

    for (i=0 ; i<numsides ; i++)
    {
        // for all sides
        // texture num 0=no-texture, otherwise is valid texture
#if 1
        if (sides[i].toptexture)
            texturepresent[sides[i].toptexture] = 1;
        if (sides[i].midtexture)
            texturepresent[sides[i].midtexture] = 1;
        if (sides[i].bottomtexture)
            texturepresent[sides[i].bottomtexture] = 1;
#else 
        //Hurdler: huh, a potential bug here????
        if (sides[i].toptexture < numtextures)
            texturepresent[sides[i].toptexture] = 1;
        if (sides[i].midtexture < numtextures)
            texturepresent[sides[i].midtexture] = 1;
        if (sides[i].bottomtexture < numtextures)
            texturepresent[sides[i].bottomtexture] = 1;
#endif       
    }

    // Sky texture is always present.
    // Note that F_SKY1 is the name used to
    //  indicate a sky floor/ceiling as a flat,
    //  while the sky texture is stored like
    //  a wall texture, with an episode dependend
    //  name.
    texturepresent[skytexture] = 1;

    //if (devparm)
    //    CONS_Printf("Generating textures..\n");

    texturememory = 0;  // global
    for (i=FIRST_TEXTURE ; i<numtextures ; i++)
    {
        if (!texturepresent[i])
            continue;

        //texture = textures[i];
        if( texturecache[i]==NULL )
            R_GenerateTexture (i);
        //numgenerated++;

        // note: pre-caching individual patches that compose textures became
        //       obsolete since we now cache entire composite textures

        //for (j=0 ; j<texture->patchcount ; j++)
        //{
        //    lump = texture->patches[j].patch;
        //    texturememory += W_LumpLength(lump);
        //    W_CacheLumpNum(lump , PU_CACHE);
        //}
    }
    //CONS_Printf ("total mem for %d textures: %d k\n",numgenerated,texturememory>>10);

    //
    // Precache sprites.
    //
    spritepresent = alloca(numsprites);
    memset (spritepresent,0, numsprites);

    for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    {
        if (th->function.acp1 == (actionf_p1)P_MobjThinker)
            spritepresent[((mobj_t *)th)->sprite] = 1;
    }

    spritememory = 0;
    for (i=0 ; i<numsprites ; i++)
    {
        if (!spritepresent[i])
            continue;

        for (j=0 ; j<sprites[i].numframes ; j++)
        {
            sf = &sprites[i].spriteframes[j];
            for (k=0 ; k<8 ; k++)
            {
                //Fab: see R_InitSprites for more about lumppat,lumpid
                lump = sf->lumppat[k];
                if(devparm)
                   spritememory += W_LumpLength(lump);
                W_CachePatchNum(lump , PU_CACHE);
            }
        }
    }

    //FIXME: this is no more correct with glide render mode
    if (devparm)
    {
        CONS_Printf("Precache level done:\n"
                    "flatmemory:    %ld k\n"
                    "texturememory: %ld k\n"
                    "spritememory:  %ld k\n", flatmemory>>10, texturememory>>10, spritememory>>10 );
    }
}
