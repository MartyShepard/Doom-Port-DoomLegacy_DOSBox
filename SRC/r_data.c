// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: r_data.c 1425 2019-01-29 08:07:59Z wesleyjohnson $
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2016 by DooM Legacy Team.
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
// Revision 1.28  2002/01/12 02:21:36  stroggonmeth
//
// Revision 1.27  2001/12/27 22:50:25  hurdler
// fix a colormap bug, add scrolling floor/ceiling in hw mode
//
// Revision 1.26  2001/08/13 22:53:40  stroggonmeth
// Revision 1.25  2001/03/21 18:24:39  stroggonmeth
// Revision 1.24  2001/03/19 18:52:01  hurdler
//
// Revision 1.23  2001/03/13 22:14:20  stroggonmeth
// Long time no commit. 3D floors, FraggleScript, portals, ect.
//
// Revision 1.22  2000/11/04 16:23:43  bpereira
//
// Revision 1.21  2000/11/02 17:50:09  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.20  2000/10/04 16:19:23  hurdler
// Change all those "3dfx names" to more appropriate names
//
// Revision 1.19  2000/09/28 20:57:17  bpereira
//
// Revision 1.18  2000/08/11 12:25:23  hurdler
// latest changes for v1.30
//
// Revision 1.17  2000/07/01 09:23:49  bpereira
// Revision 1.16  2000/05/03 23:51:01  stroggonmeth
// Revision 1.15  2000/04/23 16:19:52  bpereira
// Revision 1.14  2000/04/18 17:39:39  stroggonmeth
//
// Revision 1.13  2000/04/18 12:54:58  hurdler
// software mode bug fixed
//
// Revision 1.12  2000/04/16 18:38:07  bpereira
// Revision 1.11  2000/04/15 22:12:58  stroggonmeth
//
// Revision 1.10  2000/04/13 23:47:47  stroggonmeth
// See logs
//
// Revision 1.9  2000/04/08 17:45:11  hurdler
// fix some boom stuffs
//
// Revision 1.8  2000/04/08 17:29:25  stroggonmeth
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
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Preparation of data for rendering,
//      generation of lookups, caching, retrieval by name.
//
//-----------------------------------------------------------------------------

#include "doomincl.h"
#include "p_local.h"
#include "p_tick.h"
  // thinker
#include "p_setup.h" //levelflats
#include "g_game.h"
#include "i_video.h"
#include "r_local.h"
#include "r_sky.h"
#include "r_data.h"
#include "w_wad.h"
  // numwadfiles
#include "z_zone.h"
#include "v_video.h" //pLocalPalette
#include "m_swap.h"

// Enable Generate_Texture to realloc a texture when estimate was too small
#define GENTEXT_REALLOC
// Enable Generate_Texture to reuse columns
#define GENTEXT_REUSECOL
//#define DEBUG_REUSECOL  (verbose > 1)
//#define DEBUG_REUSECOL  verbose
// Enable bad patch detection
#define GENTEXT_BADPATCH_DETECT

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
// [WDJ] 2012-02-06 shared for DRAW15, DRAW16, DRAW24, DRAW32
union color8_u   color8;  // remap color index to rgb value
uint16_t*  hicolormaps = NULL;  // test a 32k colormap remaps high -> high

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
    int         position;  // dest
    byte*       source;
//    byte*       dest;

//    dest = (byte *)cache;// + 3;

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
#ifdef GENTEXT_REUSECOL
    uint32_t  usedpatchdata;    // to detect reuse, compaction
#endif
#ifdef GENTEXT_BADPATCH_DETECT
    int   patchsize;
#endif
} compat_t;
   

byte* R_GenerateTexture (int texnum)
{
    byte*               texgen;  // generated texture (return)
    texture_t*          texture; // texture info from wad
    texpatch_t*         texpatch;  // patch info to make texture
    patch_t*            realpatch; // patch lump
    uint32_t*           colofs;  // to match size in wad

    byte     * txcblock; // allocated texture memory
    byte     * texture_end; // end of allocated texture area
    patch_t  * txcpatch; // header of txcblock
    byte     * txcdata;  // posting area
    byte     * destpixels;  // current post pixel in txcdata
    int	txcblocksize;

    // array to hold all patches for combine
# define MAXPATCHNUM 256
    compat_t   compat[MAXPATCHNUM];
    post_t   * srcpost, * destpost;

    unsigned int patchcount;
    unsigned int compostsize;
    int patchsize;
    int	colofs_size;
    int x, x1, x2, i, p;
    int postlength;  // length of current post
    int segnxt_y, segbot_y; // may be negative
    int bottom;		// next y in column


    texture = textures[texnum];
    texture->texture_model = TM_invalid; // default in case this fails
    
    // Column offset table size as determined by wad specs.
    // Column pixel data starts after the table.
    colofs_size = texture->width * sizeof( uint32_t );  // width * 4
    // to allocate texture column offset lookup

#if 0
    // To debug problems with specific textures
    if( strncmp(texture->name, "TEKWALL5", 8 ) == 0 )
       debug_Printf("GenerateTexture - match\n");
#endif

    // single-patch textures can have holes and may be used on
    // 2sided lines so they need to be kept in 'packed' format
    patchcount = texture->patchcount;
    if (patchcount==1)
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

#ifdef GENTEXT_BADPATCH_DETECT
        // [WDJ] W_CachePatchNum should only get lumps from PATCH section,
        // but it will return a colormap of the same name.
        // Validity checks.
        // colormap size = 0x2200 to 0x2248
        {
            int patch_colofs_size = realpatch->width * sizeof( uint32_t );  // width * 4
            uint32_t* pat_colofs = (uint32_t*)&(realpatch->columnofs); // to match size in wad
            if( patch_colofs_size + 8 > patchsize )  // column list exceeds patch size
                goto make_dummy_texture;
            for( i=0; i< realpatch->width; i++)
            {
               if( *(pat_colofs++) > patchsize )
                   goto make_dummy_texture;
            }
        }
#endif
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
            GenPrintf(EMSG_info,"R_GenerateTexture: Texture %8s has PNG patch, using dummy texture.\n", texture->name );
#endif
make_dummy_texture:
          {
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
            destpost = (post_t*) ((byte*)txcblock + head_size);  // posting area;
            destpost->topdelta = 0;
            destpost->length = texture->height;
            destpixels = (byte*)destpost + 3;
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
            goto single_patch_finish;
          }
        }
#endif
#if 1
        if( texpatch->originx != 0 || texpatch->originy != 0 )
        {
            // [WDJ] Cannot copy patch to texture.
            // Fixes tekwall2, tekwall3, tekwall5 in FreeDoom,
            // which use right half of large combined patches.
//	    debug_Printf("GenerateTexture %s: offset forced multipatch\n", texture->name );
            goto multipatch_combine;
        }
#endif
#if 1
        if( realpatch->width > texture->width )
        {
            // [WDJ] Texture is a portion of a large patch.
            // To prevent duplicate large copies.
            // Unfortunately this also catches the large RSKY that legacy
            // has in legacy.wad.
            if( strncmp(texture->name, "SKY", 3 ) == 0 )
            {
                // let sky match patch
                texture->height = realpatch->height;
            }
            else
            {
//	        debug_Printf("GenerateTexture %s: width forced multipatch\n", texture->name );
                goto multipatch_combine;
            }
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
            // Single-sided textures need to be kept packed to preserve holes.
            // The texture may be rebuilt several times due to cache.
            int patch_colofs_size = realpatch->width * sizeof( uint32_t );  // width * 4
            int ofsdiff = colofs_size - patch_colofs_size;
            // reserve 4 bytes at end for empty post handling
            txcblocksize = patchsize + ofsdiff + 4;
#if 0
            // Enable when want to know which textures are triggering this.
            GenPrintf(EMSG_info,"R_GenerateTexture: single patch width does not match texture width %8s\n",
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
            goto single_patch_finish;
        }
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

  single_patch_finish:
        // Interface for texture picture format
        // use the single patch's, single column lookup
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
        //debug_Printf ("R_GenTex SINGLE %.8s size: %d\n",texture->name,patchsize);
        texgen = txcblock;
        goto done;
    }
    // End of Single-patch texture

 multipatch_combine:
 // TGC_combine_patch vrs TGC_picture: Multiple patch textures.
    if( patchcount >= MAXPATCHNUM ) {
       I_SoftError("R_GenerateTexture: Combine patch count %i exceeds %i, ignoring rest\n",
                   patchcount, MAXPATCHNUM);
       patchcount = MAXPATCHNUM - 1;
    }
   
    // First examination of the source patches
    texpatch = texture->patches;
    compostsize = 0;
    for (p=0; p<patchcount; p++, texpatch++)
    {
        compat_t * cp = &compat[p];
        cp->postptr = NULL;	// disable until reach starting column
        cp->nxt_y = MAXINT;	// disable
#ifdef GENTEXT_REUSECOL
        cp->usedpatchdata = 0;
#endif
        cp->originx = texpatch->originx;
        cp->originy = texpatch->originy;
        realpatch = W_CachePatchNum(texpatch->patchnum, PU_IN_USE);  // patch temp
        cp->patch = realpatch;
        cp->width = realpatch->width;
        int patch_colofs_size = realpatch->width * sizeof( uint32_t );  // width * 4
        // add posts, without columnofs table and 8 byte patch header
#ifdef GENTEXT_BADPATCH_DETECT
        patchsize = W_LumpLength(texpatch->patchnum);
        cp->patchsize = patchsize;
        compostsize += patchsize - patch_colofs_size - 8;
#else
        compostsize += W_LumpLength(texpatch->patchnum) - patch_colofs_size - 8;
#endif
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
    // Will usually fit into compostsize because it does so as separate patches.
    // Overlap of normal posts will only reduce the size.
    // Worst case is (width * (height/2) * (1 pixel + 4 byte overhead))
    //   worstsize = colofs_size + 8 + (width * height * 5/2)
    // Usually the size will be much smaller, but it is not predictable.
    // all posts + new columnofs table + patch header
    // Compacted patches, like SW1COMM in Requiem MAP08, usually get expanded,
    // and they expand in the texture when they combine with other patches.

#ifndef GENTEXT_REALLOC
    // Combined patches + table + header + 2 byte per empty column
    txcblocksize = compostsize + (2 * texture->width);
     // this stops failure in caesar.wad
     // No longer needed with expanding texture size
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
#ifdef GENTEXT_REUSECOL
        int livepatchcount = 0;
        int reuse_column = -1;
#endif
       
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
#ifdef GENTEXT_BADPATCH_DETECT
                if( pat_colofs[patch_x] > cp->patchsize )  // detect bad patch
                    goto patch_off;  // post is not within patch memory
#endif
                cp->postptr = (post_t*)( (byte*)realpatch + pat_colofs[patch_x] );  // patch column
                if ( cp->postptr->topdelta == 0xFF )
                    goto patch_off;
#ifdef GENTEXT_REUSECOL
                // Empty columns may be shared too, but they are very small,
                // and it really adds to logic complexity, so only look at non-empty.
                if ( pat_colofs[patch_x] > cp->usedpatchdata )
                    cp->usedpatchdata = pat_colofs[patch_x];  // track normal usage pattern
                else
                {
                    // compaction detected, look for reused column
                    int rpx = (cp->originx < 0) ? -cp->originx : 0;  // x in patch
#ifdef DEBUG_REUSECOL
                    if( DEBUG_REUSECOL )
                       debug_Printf("GenerateTexture: %8s detected reuse of column %i in patch %i\n", texture->name, patch_x, p );
#endif
                    for( ; rpx<patch_x; rpx++ )  // find reused column
                    {
                        if( pat_colofs[rpx] == pat_colofs[patch_x] )  // reused in patch
                        {
                            int rtx = rpx + cp->originx; // x in texture
#if 1
                            // Test for reused column is from single patch
                            int p2;
                            int livepatchcount2 = 0;
#ifdef DEBUG_REUSECOL
                            if( DEBUG_REUSECOL )
                               debug_Printf("GenerateTexture: testing reuse of %i, as %i\n", rpx, patch_x);
#endif
                            // [WDJ] If reused column is reused alone in both cases,
                            // then assume they will be identical in final texture too.
                            for (p2=0; p2<patchcount; p2++ )
                            {
                                compat_t * cp2 = &compat[p2];
                                int p2_x = rtx - cp2->originx; // within patch
                                if( p2_x >= 0 && p2_x < cp2->width )
                                {
                                   livepatchcount2 ++;
                                }
                            }
                            if ( livepatchcount2 == 1 )
                            {
                                reuse_column = rpx + cp->originx; // column in generated texture
                                break;
                            }
#else
                            // Compare with texture column, reuse when identical
                            post_t * rpp = (post_t*)( txcblock + colofs[rtx] - 3 );  // texture column
                            post_t * ppp = cp->postptr;
#ifdef DEBUG_REUSECOL
                            if( DEBUG_REUSECOL )
                               debug_Printf("GenerateTexture: testing reuse of %i, as %i\n", rpx, patch_x);
#endif
                            // [WDJ] Comparision is made difficult by the pad bytes,
                            // which we set to 0, and Requiem.wad does not.
                            while (rpp->topdelta != 0xff)  // find column end
                            {
                                int len = rpp->length;
                                if( rpp->topdelta != ppp->topdelta )  goto reject_1;
                                if( rpp->length != ppp->length )  goto reject_1;
                                // skip leading pad, and trailing pad
                                if( memcmp( ((byte*)rpp)+3, ((byte*)ppp)+3, len ) )  goto reject_1;
                                // next post
                                len += 4;  // sizeof post_t + lead pad + trail pad
                                rpp = (post_t *) (((byte *)rpp) + len);
                                ppp = (post_t *) (((byte *)ppp) + len);
                            }
                            if ( ppp->topdelta == 0xff )
                            {
                                // columns match
                                reuse_column = rtx;
                                break;
                            }
                           reject_1:
                            continue;
#endif
                        }
                    }
                }
                livepatchcount++;
#endif	       
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

#ifdef GENTEXT_REUSECOL
        if( livepatchcount == 1 )
        {
            if( reuse_column >= 0 )
            {
                 // reuse the column
                 colofs[x] = colofs[reuse_column];
#ifdef DEBUG_REUSECOL
                 // DEBUG: enable to see column reuse
                 if( DEBUG_REUSECOL )
                   debug_Printf("GenerateTexture: reuse %i\n", reuse_column);
#endif
                 continue;  // next x
            }
        }
#endif
       
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
        continue; // next x
#ifdef GENTEXT_REALLOC
 exceed_alloc_error:
       {
        // Re-alloc the texture
        byte* old_txcblock = txcblock;
        int old_txcblocksize = txcblocksize;
        // inc alloc by a proportional amount, plus one column
        txcblocksize += (texture->width - x + 1) * txcblocksize / texture->width;
#if 1
        // enable to print re-alloc events
        if( verbose )
            GenPrintf(EMSG_ver, "R_GenerateTexture: %8s re-alloc from %i to %i\n",
                    texture->name, old_txcblocksize, txcblocksize );
#endif
        txcblock = Z_Malloc (txcblocksize, PU_IN_USE,
                      (void**)&texturecache[texnum]);
        memcpy( txcblock, old_txcblock, old_txcblocksize );
        texture_end = txcblock + txcblocksize - 2; // do not exceed
        txcdata = (byte*)txcblock + colofs_size + 8;  // posting area
        destpixels = (byte*)txcblock + colofs[x] - 3;  // this column
        {
            patch_t * txcpatch = (patch_t*) txcblock; // header of txcblock
            colofs = (uint32_t*)&(txcpatch->columnofs);  // has patch header
        }
        x--;  // redo this column
        Z_Free(old_txcblock);  // also nulls texturecache ptr
        texturecache[texnum] = txcblock; // replace ptr undone by Z_Free
       }
#endif
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
   
#ifndef GENTEXT_REALLOC
 exceed_alloc_error:   
    I_SoftError("R_GenerateTexture: %8s exceeds allocated block, make picture\n", texture->name );
    goto error_redo_as_picture;
#endif
   
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
    //debug_Printf ("R_GenTex MULTI  %.8s size: %d\n",texture->name,txcblocksize);

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
byte* R_GetColumn ( int texnum, int col )
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
// R_Load_Textures
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
void R_Load_Textures (void)
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

    int*                patch_to_num;  // patch name to num lookup

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

    // create name index to patch index lookup table, will free before return
    patch_to_num = calloc (nummappatches, sizeof(*patch_to_num));
    for (i=0 ; i<nummappatches ; i++)
    {
        strncpy (name,name_p+i*8, 8);
        patch_to_num[i] = W_Check_Namespace( name, LNS_patch );
    }
    Z_Free (pnames);

    // Load the map texture definitions from textures.lmp.
    // The data is contained in one or two lumps,
    //  TEXTURE1 for shareware, plus TEXTURE2 for commercial.
    maptex = maptex1 = W_CacheLumpName ("TEXTURE1", PU_LUMP); // will be freed
    numtextures1 = LE_SWAP32(*maptex);  // number of textures, lump[0..3]
    maxoff = W_LumpLength (W_GetNumForName ("TEXTURE1"));
    directory = maptex+1;  // after number of textures, at lump[4]

    if( VALID_LUMP( W_CheckNumForName ("TEXTURE2") ) )
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
            I_Error ("R_Load_Textures: bad texture directory\n");

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
        debug_Printf( "Texture[%i] = %8.8s\n", i, mtexture->name);
#endif
        mpatch = &mtexture->patches[0]; // first patch ind in texture lump
        texpatch = &texture->patches[0];

        for (j=0 ; j<texture->patchcount ; j++, mpatch++, texpatch++)
        {
            // get texture patch info from texture lump
            texpatch->originx = LE_SWAP16(mpatch->originx);
            texpatch->originy = LE_SWAP16(mpatch->originy);
            texpatch->patchnum = patch_to_num[LE_SWAP16(mpatch->patchnum)];
            if (texpatch->patchnum == -1)
            {
                I_Error ("R_Load_Textures: Missing patch in texture %s\n",
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
    free(patch_to_num);

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


lumpnum_t  R_CheckNumForNameList(const char *name, lumplist_t* list, int listsize)
{
  int   i;
  lumpnum_t  ln;
  // from last entry to first entry
  for(i = listsize - 1; i > -1; i--)
  {
    ln = W_CheckNumForNamePwad(name, list[i].wadfile, list[i].firstlump);
    if( ! VALID_LUMP(ln) )
      continue;  // not found
    if(LUMPNUM(ln) > (list[i].firstlump + list[i].numlumps) )
      continue;  // not within START END

    return ln;
  }
  return NO_LUMP;
}


// Extra colormaps
lumplist_t * colormaplumps = NULL;  // malloc
int          numcolormaplumps = 0;

// called by R_Load_Colormaps
static void R_Load_ExtraColormaps()
{
    lumpnum_t  start_ln, end_ln;
    int       cfile;
    int  ln1;

    numcolormaplumps = 0;
    colormaplumps = NULL;
    cfile = 0;

    for(;cfile < numwadfiles; cfile ++)
    {
        start_ln = W_CheckNumForNamePwad("C_START", cfile, 0);
        if( ! VALID_LUMP(start_ln) )
            continue;
       
        ln1 = LUMPNUM( start_ln );
        end_ln = W_CheckNumForNamePwad("C_END", cfile, ln1);

        if( ! VALID_LUMP(end_ln) )
        {
            I_SoftError("R_Load_Colormaps: C_START without C_END\n");
            continue;
        }

        if(WADFILENUM(start_ln) != WADFILENUM(end_ln))
        {
            I_SoftError("R_Load_Colormaps: C_START and C_END in different wad files!\n");
            continue;
        }

        colormaplumps = (lumplist_t *)realloc(colormaplumps, sizeof(lumplist_t) * (numcolormaplumps + 1));
        colormaplumps[numcolormaplumps].wadfile = WADFILENUM(start_ln);
        ln1++;
        colormaplumps[numcolormaplumps].firstlump = ln1;
        colormaplumps[numcolormaplumps].numlumps = LUMPNUM(end_ln) - ln1;
        numcolormaplumps++;
    }
}



lumplist_t*  flats;
int          numflatlists;


static
void R_Load_Flats ()
{
  lumpnum_t  start_ln, end_ln;
  int       cfile, ln1, ln2;

  numflatlists = 0;
  flats = NULL;
  cfile = 0;

  for(;cfile < numwadfiles; cfile ++)
  {
#ifdef DEBUG_FLAT
    debug_Printf( "Flats in file %i\n", cfile );
#endif
    start_ln = W_CheckNumForNamePwad("F_START", cfile, 0);
    if( ! VALID_LUMP(start_ln) )
    {
#ifdef DEBUG_FLAT
      debug_Printf( "F_START not found, file %i\n", cfile );
#endif
      start_ln = W_CheckNumForNamePwad("FF_START", cfile, 0);

      if( ! VALID_LUMP(start_ln) )  //If STILL NO_LUMP, search the whole file!
      {
#ifdef DEBUG_FLAT
        debug_Printf( "FF_START not found, file %i\n", cfile );
#endif
        end_ln = NO_LUMP;
        goto save_flat_list;
      }
    }

    // Search for END after START.
    ln1 = LUMPNUM( start_ln );
    end_ln = W_CheckNumForNamePwad("F_END", cfile, ln1);
    if( ! VALID_LUMP(end_ln) )
    {
#ifdef DEBUG_FLAT
      debug_Printf( "F_END not found, file %i\n", cfile );
#endif	 
      end_ln = W_CheckNumForNamePwad("FF_END", cfile, ln1);
#ifdef DEBUG_FLAT
      if( ! VALID_LUMP(end_ln) ) {
         debug_Printf( "FF_END not found, file %i\n", cfile );
      }
#endif
    }

save_flat_list:
    flats = (lumplist_t *)realloc(flats, sizeof(lumplist_t) * (numflatlists + 1));
    flats[numflatlists].wadfile = cfile;
    if(end_ln == NO_LUMP)
    {
      flats[numflatlists].firstlump = 0;
      flats[numflatlists].numlumps = 0xffff; //Search the entire file!
    }
    else
    {
      // delimiting markers were found
      ln2 = LUMPNUM( end_ln );
      if( ln2 <= ln1 )  // should not be able to happen
        ln2 = 0xffff;  // search entire wad
      flats[numflatlists].firstlump = ln1 + 1;
      flats[numflatlists].numlumps = ln2 - (ln1 + 1);
    }
    numflatlists++;
    continue;
  }

  if(!numflatlists)
    I_Error("R_Load_Flats: No flats found!\n");
}



// [WDJ] was R_GetFlatNumForName, but it does not cache like GetFlat
lumpnum_t  R_FlatNumForName(const char *name)
{
  // [WDJ] No use in saving F_START if are not going to use them.
  // FreeDoom, where a flat and sprite both had same name,
  // would display sprite as a flat, badly.
  // Use F_START and F_END first, to find flats without getting a non-flat,
  // and only if not found then try whole file.
  
  lumpnum_t f_lumpnum = R_CheckNumForNameList(name, flats, numflatlists);

  if( ! VALID_LUMP(f_lumpnum) ) {
     // BP:([WDJ] R_CheckNumForNameList) don't work with gothic2.wad
     // [WDJ] Some wads are reported to use a flat as a patch, but that would
     // have to be handled in the patch display code.
     // If this search finds a sprite, sound, etc., it will display
     // multi-colored confetti.
     f_lumpnum = W_CheckNumForName(name);
  }
  
  if( ! VALID_LUMP(f_lumpnum) ) {
     // [WDJ] When not found, dont quit, use first flat by default.
     I_SoftError("R_FlatNumForName: Could not find flat %.8s\n", name);
     f_lumpnum = WADLUMP( flats[0].wadfile, flats[0].firstlump );  // default to first flat
  }

  return f_lumpnum;
}

// [WDJ] Manage the spritelump_t allocations
// Still use this array so that rot=0 sprite can share one entry for all 8 rotations.

void expand_spritelump_alloc( void )
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
       expand_spritelump_alloc();
    num_free_spritelump--;
    return  num_spritelump ++;
}

//
// R_Init_SpriteLumps
// Finds the width and hoffset of all sprites in the wad,
//  so the sprite does not need to be cached completely
//  just for having the header info ready during rendering.
//

//
//   allocate sprite lookup tables
//
void R_Init_SpriteLumps (void)
{
    // the original Doom used to set numspritelumps from S_END-S_START+1

    // [WDJ] Initial allocation, will be expanded as needed
    expand_spritelump_alloc();
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


// called by R_Load_Data
static
void R_Load_Colormaps (void)
{
    lumpnum_t ln;

    // Load in the standard colormap lightmap tables (defined by lump),
    // now 64k aligned for smokie...
    ln = W_GetNumForName("COLORMAP");
    reg_colormaps = Z_MallocAlign ( W_LumpLength(ln), PU_STATIC, NULL, 16);
    W_ReadLump( ln, reg_colormaps );

    //SoM: 3/30/2000: Init Boom colormaps.
    {
      R_Clear_Colormaps();
      R_Load_ExtraColormaps();
    }
}


static lumpnum_t  extra_colormap_lumpnum[MAXCOLORMAPS];  // lump number

//SoM: Clears out extra colormaps between levels.
// called by P_SetupLevel after ZFree(PU_LEVEL,..)
// called by R_Load_Colormaps
void R_Clear_Colormaps()
{
  int   i;
#if 0   
  if( num_extra_colormaps > 30 )
     GenPrintf(EMSG_warn, "Number of colormaps: %i\n", num_extra_colormaps );
#endif

  // extra_colormaps are always present [MAXCOLORMAPS]
  num_extra_colormaps = 0;
  for(i = 0; i < MAXCOLORMAPS; i++)
  {
    extra_colormap_lumpnum[i] = NO_LUMP;
    // [WDJ] The ZMalloc colormap used to be PU_LEVEL, releasing memory without setting the ptrs NULL.
    // It is now PU_COLORMAP, and it is released only by this function.
    if( extra_colormaps[i].colormap )
    {
        Z_Free( extra_colormaps[i].colormap );
        extra_colormaps[i].colormap = NULL;
    }
  }
//  memset(extra_colormaps, 0, sizeof(extra_colormaps));
}


// [WDJ] Enable to print out results of colormap generate.
//#define VIEW_COLORMAP_GEN

// In order: whiteindex, lightgreyindex, medgreyindex, darkgreyindex, grayblackindex
// redindex, greenindex, blueindex
#define NUM_ANALYZE_COLORS  8
static byte  doom_analyze_index[NUM_ANALYZE_COLORS] = {
    4, 87, 96, 106, 7, 188, 123, 206
};
static byte  heretic_analyze_index[NUM_ANALYZE_COLORS] = {
    35, 26, 18, 10, 253, 148, 213, 192
};

// [WDJ] Analyze an extra colormap to derive some GL parameters
void  R_Colormap_Analyze( int mapnum )
{
    extracolormap_t * colormapp = & extra_colormaps[ mapnum ];
    colormapp->fadestart = 0;
    colormapp->fadeend = 33;
//    colormapp->maskalpha = 0x0;  // now in maskcolor
    colormapp->fog = 0;

#ifdef HWRENDER
  // Hardware renderer does not use, nor allocate, the colormap.
  // It uses the rgba field instead.
  if( rendermode == render_soft )
#endif     
  {
    // Software renderer defaults.
    // SoM: Added, we set all params of the colormap to normal because there
    // is no real way to tell how GL should handle a colormap lump anyway..
    colormapp->maskcolor.rgba = 0xffffffff; // white
    colormapp->maskcolor.s.alpha = 0;
    colormapp->fadecolor.rgba = 0x0; // black
    colormapp->rgba[0].rgba = 0xffffffff;
    colormapp->rgba[0].s.alpha = 0xe1;
  }
#ifdef HWRENDER
  else
  {
    // Analyze the Boom colormap for the hardware renderer.
    // The Boom colormap has been loaded already.
    // lighttable_t = byte array
    byte * cm = colormapp->colormap; // Boom colormap
    byte * tstcolor = & doom_analyze_index[0];  // colors to test
    RGBA_t work_rgba;
    int i;

    if( cm == NULL )  // no colormap to analyze
    {
        GenPrintf(EMSG_warn, "R_Colormap_Analyze: map %i, has no colormap\n", mapnum );
        return;
    }

    if( EN_heretic )
       tstcolor = & heretic_analyze_index[0];  // heretic colors to test

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
        for( k1=0; k1<NUM_ANALYZE_COLORS-1; k1++ )
        {
            byte i1 = tstcolor[k1];
            byte cm1 = cm[ LIGHTTABLE(mapindex) + i1 ];
            for( k2=k1+1; k2<NUM_ANALYZE_COLORS; k2++ )
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
                    if( dn4 && (dd4 > 0.01f || dd4 < -0.01f))
                    {
                        float h3 = (float)dn4 / (float)dd4;
                        if( h3 > 1.0f )   h3 = 1.0;
                        if( h3 < 0.01f )  h3 = 0.01f;
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
            h4r = 0.99f;
            m4_fog = 1;
        }
        if( h4r < 0.00 ) {
            h4r = 0.00;
            m4_fog = 1;
        }
        if( cnt * 2 < try_cnt )
            m4_fog = 1;
        h4r = h4r * (0.66f / 1.732f);  // correction for testing at 1/3 max brightness
        float h4 = 1.0 - h4r;
        int m4_red, m4_blue, m4_green;
#if 0
        // Generate color tint from changes grey.
        byte greyindex = tstcolor[2];
        byte cm_grey = cm[ LIGHTTABLE(mapindex) + greyindex ];  // grey
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
        GenPrintf(EMSG_info,
               "Analyze4: alpha=%i  red=%i  green=%i  blue=%i  fog=%i\n",
               (int)(255.0f*h4), m4_red, m4_green, m4_blue, m4_fog );
#endif      
        // Not great, get some tints wrong, and sometimes too light.
        // Give m4_fog some treatment, to stop compiler messages.
        work_rgba.s.alpha = (int)(h4 * 255.0f) - (m4_fog * 2.0f);
        work_rgba.s.red = m4_red;
        work_rgba.s.green = m4_green;
        work_rgba.s.blue = m4_blue;

        colormapp->rgba[i].rgba = work_rgba.rgba;  // to extra_colormap

#ifdef VIEW_COLORMAP_GEN
        // Enable to view settings of RGBA for hardware renderer.
        GenPrintf(EMSG_info,"RGBA[%i]: %8x   (alpha=%i, R=%i, G=%i, B=%i)\n",
              i, colormapp->rgba[i].rgba,
              (int)work_rgba.s.alpha,
              (int)work_rgba.s.red, (int)work_rgba.s.green, (int)work_rgba.s.blue );
#endif
    }
    // They had plans, but these are still unused in hardware renderer.
    colormapp->maskcolor.rgba = colormapp->rgba[NUM_RGBA_LEVELS-1].rgba;
    colormapp->fadecolor.rgba = colormapp->rgba[0].rgba;
  
  }
#endif
}




// [WDJ] The name parameter has trailing garbage, but the name lookup
// only uses the first 8 chars.
// Return the new colormap id number
int R_ColormapNumForName(const char *name)
{
  lumpnum_t lumpnum;
  int i;

  // Check for existing colormap of same name
  lumpnum = R_CheckNumForNameList(name, colormaplumps, numcolormaplumps);
  if( ! VALID_LUMP(lumpnum) )
  {
    I_SoftError("R_ColormapNumForName: Cannot find colormap lump %8s\n", name);
    return 0;
  }

  for(i = 0; i < num_extra_colormaps; i++)
  {
    if(lumpnum == extra_colormap_lumpnum[i])
      return i;
  }

  // Add another colormap
  if(num_extra_colormaps == MAXCOLORMAPS)
  {
    I_SoftError("R_ColormapNumForName: Too many colormaps!\n");
    return 0;
  }

  extra_colormap_lumpnum[num_extra_colormaps] = lumpnum;
  extracolormap_t * ecmp = & extra_colormaps[num_extra_colormaps];

  // The extra_colormap structure is static and allocated ptrs in it must be set NULL upon release.
  // Align on 16 bits, like other colormap allocations.
  // ecmp->colormap =
      Z_MallocAlign (W_LumpLength(lumpnum), PU_COLORMAP, (void**)&(ecmp->colormap), 16);
  // read colormap tables [][] of byte, mapping to alternative palette colors
  W_ReadLump( lumpnum, ecmp->colormap );

#ifdef VIEW_COLORMAP_GEN
  GenPrintf(EMSG_info, "\nBoom Colormap: num=%i name= %8.8s\n", num_extra_colormaps, name );
#endif
  R_Colormap_Analyze( num_extra_colormaps );
  
  num_extra_colormaps++;
  return num_extra_colormaps - 1;
}



// SoM:
//
// R_Create_Colormap
// This is a more GL friendly way of doing colormaps: Specify colormap
// data in a special linedef's texture areas and use that to generate
// custom colormaps at runtime. NOTE: For GL mode, we only need to color
// data and not the colormap data. 

byte NearestColor(byte r, byte g, byte b);
int  RoundUp(double number);
void R_Create_Colormap_ex( extracolormap_t * extra_colormap_p );

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
int R_Create_Colormap_str(char *colorstr, char *ctrlstr, char *fadestr)
{
  extracolormap_t * extra_colormap_p;
  RGBA_t  maskcolor, fadecolor;
  unsigned int  fadestart = 0, fadeend = 33;
  int    c_alpha = 0;
  int    i;
  int    fog = 0;
  int    mapnum = num_extra_colormaps;  // the new colormap id number

  if(colorstr[0] == '#')  // colormap generate string is recognized
  {
    // color value from top texture string
    maskcolor.s.red = ((HEX_TO_INT(colorstr[1])<<4) + HEX_TO_INT(colorstr[2]));
    maskcolor.s.green = ((HEX_TO_INT(colorstr[3])<<4) + HEX_TO_INT(colorstr[4]));
    maskcolor.s.blue = ((HEX_TO_INT(colorstr[5])<<4) + HEX_TO_INT(colorstr[6]));
    if(colorstr[7] >= 'a' && colorstr[7] <= 'z')
      c_alpha = (colorstr[7] - 'a');
    else if(colorstr[7] >= 'A' && colorstr[7] <= 'Z')
      c_alpha = (colorstr[7] - 'A');
    else
      c_alpha = 25;
    maskcolor.s.alpha = (c_alpha * 255) / 25;  // convert from 0..25 to 0..255
  }
  else
  {
    // [WDJ] default for missing upper is not in docs, and was inconsistent
    // Cannot be 0xff after multiply by maskalpha=0.
    maskcolor.s.red = 0xff;
    maskcolor.s.green = 0xff;
    maskcolor.s.blue = 0xff;
    maskcolor.s.alpha = 0;
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
    fog = CHAR_TO_INT(ctrlstr[1]) ? 1 : 0;
  }

  fadecolor.rgba = 0;
  if(fadestr[0] == '#')
  {
    fadecolor.s.red = ((HEX_TO_INT(fadestr[1]) * 16) + HEX_TO_INT(fadestr[2]));
    fadecolor.s.green = ((HEX_TO_INT(fadestr[3]) * 16) + HEX_TO_INT(fadestr[4]));
    fadecolor.s.blue = ((HEX_TO_INT(fadestr[5]) * 16) + HEX_TO_INT(fadestr[6]));
  }

  // find any identical existing colormap
  for(i = 0; i < num_extra_colormaps; i++)
  {
    // created colormaps only
    if( VALID_LUMP(extra_colormap_lumpnum[i]) )
      continue;
    if(   maskcolor.rgba == extra_colormaps[i].maskcolor.rgba
       && fadecolor.rgba == extra_colormaps[i].fadecolor.rgba
// [WDJ] maskalpha is now in maskcolor.alpha
//       && maskalpha == extra_colormaps[i].maskalpha
       && fadestart == extra_colormaps[i].fadestart
       && fadeend == extra_colormaps[i].fadeend
       && fog == extra_colormaps[i].fog )
      return i;
  }

#ifdef VIEW_COLORMAP_GEN
  GenPrintf(EMSG_info, "\nGenerate Colormap: num=%i\n", num_extra_colormaps );
  GenPrintf(EMSG_info, " alpha=%2x, color=(%2x,%2x,%2x), fade=(%2x,%2x,%2x), fog=%i\n",
          maskcolor.s.alpha, maskcolor.s.red, maskcolor.s.green, maskcolor.s.blue,
          fadecolor.s.red, fadecolor.s.green, fadecolor.s.blue,
          fog );
#endif

  if(num_extra_colormaps == MAXCOLORMAPS)
  {
    I_SoftError("R_Create_Colormap: Too many colormaps!\n");
    return 0;
  }

  num_extra_colormaps++;

  // Created colormaps do not have lumpnum
  extra_colormap_lumpnum[mapnum] = NO_LUMP;
  extra_colormap_p = &extra_colormaps[mapnum];

  extra_colormap_p->colormap = NULL;
  extra_colormap_p->maskcolor.rgba = maskcolor.rgba;
  extra_colormap_p->fadecolor.rgba = fadecolor.rgba;
//  extra_colormap_p->maskalpha = maskalpha;  // 0.0 .. 1.0  // now in maskcolor
  extra_colormap_p->fadestart = fadestart;
  extra_colormap_p->fadeend = fadeend;
  extra_colormap_p->fog = fog;

  R_Create_Colormap_ex( extra_colormap_p );

#if 0
#ifdef VIEW_COLORMAP_GEN
  if(rendermode != render_soft)
  {
    // [WDJ] DEBUG, TEST AGAINST OLD HDW CODE
    uint32_t rgba_oldhw =
          (HEX_TO_INT(colorstr[1]) << 4) + (HEX_TO_INT(colorstr[2]) << 0) +
          (HEX_TO_INT(colorstr[3]) << 12) + (HEX_TO_INT(colorstr[4]) << 8) +
          (HEX_TO_INT(colorstr[5]) << 20) + (HEX_TO_INT(colorstr[6]) << 16) +
          (ALPHA_TO_INT(colorstr[7]) << 24);
   if( rgba_oldhw != extra_colormap_p->rgba[0].rgba )
      GenPrintf(EMSG_info,"RGBA: old=%X, new=%x\n", rgba_oldhw, extra_colormap_p->rgba[i-1].rgba);
  }
#endif
#endif
   
  return mapnum;
}
   

// Analyze param and create the colormap data for the rendermode.
// Called when rendermode change.
void R_Create_Colormap_ex( extracolormap_t * extra_colormap_p )
{
  RGBA_t maskcolor, fadecolor;
  double color_r, color_g, color_b; // color RGB from top-texture
  double cfade_r, cfade_g, cfade_b; // fade-to color from bottom-texture
  double maskalpha;
  unsigned int  fadestart, fadeend, c_alpha;
  int  i;
  unsigned int p;
  
  maskcolor.rgba = extra_colormap_p->maskcolor.rgba;
  fadecolor.rgba = extra_colormap_p->fadecolor.rgba;
  fadestart = extra_colormap_p->fadestart;
  fadeend = extra_colormap_p->fadeend;

  c_alpha = maskcolor.s.alpha;
  maskalpha = (double)maskcolor.s.alpha / (double)255;  // 0.0 .. 1.0
  color_r = (double)maskcolor.s.red;
  color_g = (double)maskcolor.s.green;
  color_b = (double)maskcolor.s.blue;

  cfade_r = (double)fadecolor.s.red;
  cfade_g = (double)fadecolor.s.green;
  cfade_b = (double)fadecolor.s.blue;


#ifdef HWRENDER
  // Hardware renderer does not use, nor allocate, the colormap.
  // It uses the rgba field instead.
  if(rendermode == render_soft)
#endif
  {
    byte * colormap_p;
    double othermask = 1.0 - maskalpha;
    double by_alpha = maskalpha / 255.0;
    double fadedist = fadeend - fadestart;
    double  deltas[256][3], map[256][3];
     
    // The extra_colormap structure is static and allocated ptrs in it must be set NULL upon release.
    // Aligning on 16 bits, NOT 8, keeps it from crashing! SSNTails 12-13-2002
    if( extra_colormap_p->colormap == NULL )
    {
        // extra_colormap_p->colormap = 
        Z_MallocAlign((256 * 34) + 10, PU_COLORMAP, (void**)&(extra_colormap_p->colormap), 16);
    }
    colormap_p = extra_colormap_p->colormap;
     
    // premultiply, this messes up rgba calc so it must be done here
    color_r *= by_alpha;  // to 0.0 .. 1.0 range
    color_g *= by_alpha;
    color_b *= by_alpha;
    for(i = 0; i < 256; i++)
    {
      double r = pLocalPalette[i].s.red;
      double g = pLocalPalette[i].s.green;
      double b = pLocalPalette[i].s.blue;
      double cbrightness = sqrt((r*r) + (g*g) + (b*b));

      map[i][0] = (cbrightness * color_r) + (r * othermask);
      if(map[i][0] > 255.0)
        map[i][0] = 255.0;
      deltas[i][0] = (map[i][0] - cfade_r) / fadedist;

      map[i][1] = (cbrightness * color_g) + (g * othermask);
      if(map[i][1] > 255.0)
        map[i][1] = 255.0;
      deltas[i][1] = (map[i][1] - cfade_g) / fadedist;

      map[i][2] = (cbrightness * color_b) + (b * othermask);
      if(map[i][2] > 255.0)
        map[i][2] = 255.0;
      deltas[i][2] = (map[i][2] - cfade_b) / fadedist;
    }

    for(p = 0; p < 34; p++)
    {
      for(i = 0; i < 256; i++)
      {
        *colormap_p = NearestColor(RoundUp(map[i][0]), RoundUp(map[i][1]), RoundUp(map[i][2]));
        colormap_p++;
  
        if( p < fadestart )
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
      if( extra_colormap_p->colormap )
      {
          // Does not use colormap
          Z_Free( extra_colormap_p->colormap );
          extra_colormap_p->colormap = NULL;
      }

      // hardware needs color_r, color_g, color_b, before they get *maskalpha.
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

          RGBA_t * cp = & extra_colormap_p->rgba[i];
          cp->s.red = (int)( colorper*color_r + fadeper*cfade_r );
          cp->s.green = (int)( colorper*color_g + fadeper*cfade_g );
          cp->s.blue = (int)( colorper*color_b + fadeper*cfade_b );
          cp->s.alpha = c_alpha;
#ifdef VIEW_COLORMAP_GEN
          GenPrintf(EMSG_info,"RGBA[%i]: %x\n", i, extra_colormap_p->rgba[i].rgba);
#endif
      }
  }
#endif	       
}
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
    return 255;
  if(number < 0)
    return 0;

  if((int)number <= (int)(number -0.5))
    return (int)number + 1;

  return (int)number;
}




char * R_ColormapNameForNum(int num)
{
  if(num == -1)
    return "NONE";

  if(num < 0 || num > MAXCOLORMAPS)
  {
    I_SoftError("R_ColormapNameForNum: num is invalid!\n");
    return "NONE";
  }

  if( ! VALID_LUMP(extra_colormap_lumpnum[num]) )
    return "INLEVEL";  // created colormap

  return wadfiles[WADFILENUM(extra_colormap_lumpnum[num])]->lumpinfo[LUMPNUM(extra_colormap_lumpnum[num])].name;
}


#if defined( ENABLE_DRAW15 ) || defined( ENABLE_DRAW16 ) || defined( ENABLE_DRAW24 ) || defined( ENABLE_DRAW32 )
//
//  build a table for quick conversion from 8bpp to 15bpp, 16bpp, 24bpp, 32bpp
//
// covert 8 bit colors
void R_Init_color8_translate ( RGBA_t * palette )
{
    int  i;

    // [WDJ] unconditional now, palette flashes etc.
    {
        for (i=0;i<256;i++)
        {
            // use palette after gamma modified
            register byte r = palette[i].s.red;
            register byte g = palette[i].s.green;
            register byte b = palette[i].s.blue;
            switch( vid.drawmode )
            {
#ifdef ENABLE_DRAW15	       
             case DRAW15: // 15 bpp (5,5,5)
               color8.to16[i] = (((r >> 3) << 10) | ((g >> 3) << 5) | ((b >> 3)));
               break;
#endif
#ifdef ENABLE_DRAW16
             case DRAW16: // 16 bpp (5,6,5)
               color8.to16[i] = (((r >> 3) << 11) | ((g >> 2) << 5) | ((b >> 3)));
               break;
#endif
#ifdef ENABLE_DRAW24
             case DRAW24:
               {
                   // different alignment from DRAW32, for speed
                   pixelunion32_t c32;
                   c32.pix24.r = r;
                   c32.pix24.g = g;
                   c32.pix24.b = b;
                   color8.to32[i] = c32.ui32;
               }
               break;
#endif
#ifdef ENABLE_DRAW32
             case DRAW32:
               {
                   pixelunion32_t c32;
                   c32.pix32.r = r;
                   c32.pix32.g = g;
                   c32.pix32.b = b;
                   c32.pix32.alpha = 0xFF;
                   color8.to32[i] = c32.ui32;
               }
               break;
#endif
             default:
               break;  // should not be calling
            }
        }
    }

#ifdef HIGHCOLORMAPS
#if defined( ENABLE_DRAW15 ) || defined( ENABLE_DRAW16 )
    if( hicolormaps == NULL )
    {
        // test a big colormap
        hicolormaps = Z_Malloc (32768 /**34*/, PU_STATIC, 0);
        for (i=0;i<16384;i++)
            hicolormaps[i] = color8.to16[ dc_colormap[ i ] ];
     }
#endif
#endif
}
#endif

typedef struct
{
    byte  alpha;  // 0..255
    byte  opaque, clear;  // 0..100
} translucent_subst_info_t;

// From analyze of std translucent maps (with adjustments)
static  translucent_subst_info_t   translucent_subst[] =
{
    {131, 2, 0}, // TRANSLU_med
    {62, 0, 4}, // TRANSLU_more
    {60, 1, 22}, // TRANSLU_hi
    {140, 11, 0}, // TRANSLU_fire
    {200, 71, 3},  // TRANSLU_fx1
    {182, 0, 0}  // TRANSLU_75
};


//#define VIEW_TRANSLUMAP_GEN
//#define VIEW_TRANSLUMAP_GEN_DETAIL

// [WDJ] Analyze a Boom Translucent Map to derive some GL parameters
void  R_TranslucentMap_Analyze( translucent_map_t * transp, byte * tmap )
{
    // Analyze the Boom translucent map for rgb drawing.
    byte * tstcolor = & doom_analyze_index[0];  // colors to test
    int ti;

    if( EN_heretic )
       tstcolor = & heretic_analyze_index[0];  // heretic colors to test

    float h4 = 0.0; // alpha = 0
    int alpha;
    int tot_cnt = 0;
    int opaque_cnt = 0;
    int clear_cnt = 0;
    int dd4, dn4, k1, k2;
    // tm3 = i1 * (1-alpha) + i2 * alpha
    // alpha = (i1-tm3) / (i1-i2)
    for( k1=0; k1<NUM_ANALYZE_COLORS; k1++ )
    {
        byte i1 = tstcolor[k1];  // background
        for( k2=0; k2<NUM_ANALYZE_COLORS; k2++ )
        {
            if( k1 == k2 ) continue;
            byte i2 = tstcolor[k2];  // translucent
            byte tm3 = tmap[ (i2<<8) + i1 ];
            // for each color
            int krgb;
#ifdef  VIEW_TRANSLUMAP_GEN_DETAIL
GenPrintf(EMSG_info, "bg(%i,%i,%i):fb(%i,%i,%i):>(%i,%i,%i) ",
  pLocalPalette[i1].s.red,pLocalPalette[i1].s.green,pLocalPalette[i1].s.blue,
  pLocalPalette[i2].s.red,pLocalPalette[i2].s.green,pLocalPalette[i2].s.blue,
  pLocalPalette[tm3].s.red,pLocalPalette[tm3].s.green,pLocalPalette[tm3].s.blue
);
#endif	       
            for( krgb=0; krgb<3; krgb++ )  // red, green, blue
            {
                // alpha = (i1-tm3) / (i1-i2)
                switch( krgb )
                {
                 case 0: // red
                   dd4 = (int)pLocalPalette[i1].s.red -  (int)pLocalPalette[i2].s.red;
                   dn4 = (int)pLocalPalette[i1].s.red - (int)pLocalPalette[tm3].s.red;
                   break;
                 case 1: // green
                   dd4 = (int)pLocalPalette[i1].s.green -  (int)pLocalPalette[i2].s.green;
                   dn4 = (int)pLocalPalette[i1].s.green - (int)pLocalPalette[tm3].s.green;
                   break;
                 default: // blue
                   dd4 = (int)pLocalPalette[i1].s.blue -  (int)pLocalPalette[i2].s.blue;
                   dn4 = (int)pLocalPalette[i1].s.blue - (int)pLocalPalette[tm3].s.blue;
                   break;
                }
                // eliminate cases where equation is least accurate
                if( dn4 && (dd4 > 10.0 || dd4 < -10.0))
                {
                    float h3 = (float)dn4 / (float)dd4;
#ifdef VIEW_TRANSLUMAP_GEN_DETAIL
                    GenPrintf(EMSG_info, " %i", (int)(h3*255) );
#endif
                    // eliminate wierd alpha (due to color quantization in making the transmap)
                    if( h3 > 0.0 && h3 < 1.1 )
                    {
                        h4 += h3;  // total for avg
                        tot_cnt ++;
                        if( h3 > 0.9 )   opaque_cnt++;
                        if( h3 < 0.1 )   clear_cnt++;
                    }
                }
#ifdef VIEW_TRANSLUMAP_GEN_DETAIL
                else
                {
                    GenPrintf(EMSG_info, " -" );
                }
#endif	       
            }
#ifdef VIEW_TRANSLUMAP_GEN_DETAIL
            GenPrintf(EMSG_info,"\n");
#endif	       
        }
    }
    alpha = (int) (255.0 * h4 / tot_cnt);
    if( alpha > 255 )  alpha = 255;
    if( alpha < 0 )    alpha = 0;
    transp->alpha = alpha;
    transp->opaque = (opaque_cnt*100)/tot_cnt;
    transp->clear = (clear_cnt*100/tot_cnt);
    transp->substitute_std_translucent = TRANSLU_med;  // default
    transp->substitute_error = 255;
    {
        // find closest standard transparency
        float bestdist = 1E20;
        for(ti=1; ti<TRANSLU_75; ti++)
        {
            translucent_subst_info_t * tinfop = & translucent_subst[ti-1];
            float dista = tinfop->alpha - alpha;  // 0..255
            float distop = tinfop->opaque - transp->opaque;  // 0..100
            float distcl = tinfop->clear - transp->clear;  // 0..100
            float dist = (dista*dista) + (distop*distop) + (distcl*distcl);
            if( dist < bestdist )
            {
                bestdist = dist;
                dist *= (255.0/(40.0*40.0 + 20.0*20.0  + 10.0*10.0));
                if( dist > 255 )  dist = 255;
                transp->substitute_error = dist;
                transp->substitute_std_translucent = ti;
            }
        }
    }
#ifdef VIEW_TRANSLUMAP_GEN
    // Enable to see results of analysis.
    GenPrintf(EMSG_info,
             "Analyze Trans: alpha=%i  opaque=%i%%  clear=%i%%  subst=%i  subst_err=%i\n",
              alpha, transp->opaque, transp->clear,
              transp->substitute_std_translucent, transp->substitute_error);
#endif      
}

#define TRANSLU_STORE_INC  8
translucent_map_t *  translu_store = NULL;
int translu_store_num = 0;
int translu_store_len = 0;

// Return translu_store index
int  R_setup_translu_store( int lump_num )
{
   int ti;
   translucent_map_t * tranlu_map;
   // check if already in store
   for(ti=0; ti< translu_store_num; ti++ )
   {
       if( translu_store[ti].translu_lump_num == lump_num )
          goto done;
   }
   // check for expand store
   if( translu_store_num >= translu_store_len )
   {
       translu_store_len += TRANSLU_STORE_INC;
       translu_store = realloc( translu_store, translu_store_len );
       if( translu_store == NULL )
          I_Error( "Translucent Store: cannot alloc\n" );
   }
   // create new store and fill it in
   ti = translu_store_num++;
   tranlu_map = & translu_store[ti];
   tranlu_map->translu_lump_num = lump_num;
   R_TranslucentMap_Analyze( tranlu_map, W_CacheLumpNum( lump_num, PU_CACHE ) );
   if( verbose>1 )
      GenPrintf(EMSG_ver, "TRANSLU STORE lump=%i, subst=%i\n", lump_num, tranlu_map->substitute_std_translucent );
done:
   return ti;
}

#if 0
// [WDJ] Only enable to setup colormap analyze tables
#define ANALYZE_GAMEMAP
void Analyze_gamemap( void )
{
#define NUMTESTCOLOR  8   
    // For new port, analyze the colormap to find white, grey, red, blue, green.
    // This only affects this color analyzer, and is not fatal.
    static const char * name[NUMTESTCOLOR]=
     {"white", "light grey", "med grey", "dark grey", "grayblack", "red", "green", "blue" };
    typedef struct { byte red, green, blue; } test_rgb_t;
    static const test_rgb_t ideal[NUMTESTCOLOR]=
     {{255,255,255}, {192,192,192}, {128,128,128}, {64,64,64}, {10,10,10}, {100,0,0}, {0,100,0}, {0,0,100} };
    float bestdist[NUMTESTCOLOR];
    byte bestcolor[NUMTESTCOLOR];
    int i, k;

    GenPrintf(EMSG_info, "Game colormap analyze:\n" );
    for(k=0; k<8; k++ )   bestdist[k] = 1E20;
    
    for(i=0; i<256; i++)
    {
        int cr = pLocalPalette[i].s.red;
        int cg = pLocalPalette[i].s.green;
        int cb = pLocalPalette[i].s.blue;
        for(k=0; k<NUMTESTCOLOR; k++ )
        {
            float dr = cr - ideal[k].red;
            float dg = cg - ideal[k].green;
            float db = cb - ideal[k].blue;
            float d = dr*dr + dg*dg + db*db;
            if( d < bestdist[k] )
            {
                bestdist[k] = d;
                bestcolor[k] = i;
            }
        }
    }
    for(k=0; k<NUMTESTCOLOR; k++ )
    {
        GenPrintf(EMSG_info, "Analyze Index %s = %i\n", name[k], bestcolor[k] );
    }
}
#endif

#if 0
// [WDJ] Only enable to setup translucent analyze tables
#define ANALYZE_TRANSLUCENT_MAPS
void Analyze_translucent_maps( void )
{
    // translucent maps
    translucent_map_t trans;
    int k;
    GenPrintf(EMSG_info, "Game translucent maps analyze:\n" );
    if( translucenttables == NULL )   return;
    for(k=1; k<=TRANSLU_fx1; k++ )  // Doom2 maps
    { 
        byte * tmap = & translucenttables[ TRANSLU_TABLE_INDEX(k) ];
        if( tmap )
        {
            GenPrintf(EMSG_info, "  Analyze translucent map %i:\n", k );
            R_TranslucentMap_Analyze( &trans, tmap);
        }
    }
}
#endif


// Table of alpha = 0..255 to translucent tables to be used for r_draw8
// For use by DRAW8PAL, Not for draw modes that can use dr_alpha.
// index by alpha >> 4
const unsigned int  translucent_alpha_table[16] =
{
   // hi=10..15%, more=20..25%, med=50%
   TRANSLU_TABLE_hi,   // 0..15    ( 0.. 6%)
   TRANSLU_TABLE_hi,   // 16..31   ( 6..12%)
   TRANSLU_TABLE_hi,   // 32..47   (12..18%)
   TRANSLU_TABLE_more, // 48..63   (18..25%)
   TRANSLU_TABLE_more, // 64..79   (25..31%)
   TRANSLU_TABLE_more, // 80..95   (31..37%)
   TRANSLU_TABLE_med,  // 96..111  (37..43%)
   TRANSLU_TABLE_med,  // 112..127 (44..50%)
   TRANSLU_TABLE_med,  // 128..143 (50..56%)
 // reversed table usage, per TRANSLU_REV_ALPHA
   TRANSLU_TABLE_med,  // 144..159 (56..62%)
   TRANSLU_TABLE_more, // 160..177 (63..69%)
   TRANSLU_TABLE_more, // 176..191 (69..75%)
   TRANSLU_TABLE_more, // 192..207 (75..81%)
   TRANSLU_TABLE_hi,   // 208..223 (81..87%)
   TRANSLU_TABLE_hi,   // 224..239 (88..93%)
   TRANSLU_TABLE_hi,   // 240..255 (94..100%)
};



// Fake floor fog and water effect structure
#define FWE_STORE_INC  16
fogwater_t *  fweff = NULL;  // array
// 0 of the array is not used
static int fweff_num = 0;
static int fweff_len = 0;  // num allocated

int  R_get_fweff( void )
{
   // check for expand store
   if( fweff_num >= fweff_len )
   {
       fweff_len += FWE_STORE_INC;
       fweff = (fogwater_t*) realloc( fweff, fweff_len * sizeof(fogwater_t) );
       if( fweff == NULL )
          I_Error( "Fog Store: cannot alloc\n" );
   }
   memset( &fweff[fweff_num], 0, sizeof( fogwater_t ) );
   return fweff_num++;
}


// update config settings
void R_FW_config_update( void )
{
   int i;
   // If not init yet, then init will call this again
   if( ! fweff )
       return;

   // defaults affected by controls
   fweff[1].effect = (fogwater_effect_e) cv_water_effect.value;
   fweff[2].effect = (fogwater_effect_e) cv_fog_effect.value;
   fweff[3].effect = fweff[1].effect;
   // update all fweff settings that were defaulted
   for( i=5; i<fweff_num; i++ )
   {
       byte fwf = fweff[i].flags;
       fogwater_t * def = & fweff[ fwf & FWF_index ];
       if( fwf & FWF_default_effect )
       {
           fweff[i].effect = def->effect;
       }
       if( fwf & FWF_default_alpha )
       {
           fweff[i].alpha = def->alpha;
           fweff[i].fsh_alpha = def->fsh_alpha;
       }
   }
   // Must also change the affected FakeFloor flags
   P_Config_FW_Specials ();
}

// start of level
void R_Clear_FW_effect( void )
{
    if( ! fweff )  // no memory yet
    { 
        // init
        R_get_fweff();  // [0] unused
        R_get_fweff();  // [1] water
        R_get_fweff();  // [2] fog
        R_get_fweff();  // [3] opaque water
        R_get_fweff();  // [4] solid floor
        fweff[0].effect = FW_clear; // unused
        fweff[0].alpha = 0;
        fweff[0].fsh_alpha = 0;
        fweff[0].flags = 0;
        fweff[1].effect = FW_clear;
        fweff[1].alpha = 128;  // default water alpha
        fweff[1].fsh_alpha = 128;
        fweff[1].flags = FWF_water | FWF_default_alpha | FWF_default_effect;
        fweff[2].effect = FW_fogdust;
        fweff[2].alpha = 110;  // default fog alpha
        fweff[2].fsh_alpha = 110 * 0.90;
        fweff[2].flags = FWF_fog | FWF_default_alpha | FWF_default_effect;
        fweff[3].effect = FW_clear;
        fweff[3].alpha = 190;  // default opaque water
        fweff[3].fsh_alpha = 190;
        fweff[3].flags = FWF_opaque_water | FWF_default_alpha | FWF_default_effect;
        fweff[4].effect = FW_clear;  // does not matter
        fweff[4].alpha = 128;  // default solid translucent floor
        fweff[4].fsh_alpha = 128;
        fweff[4].flags = FWF_solid_floor;  // not settable defaults
        R_FW_config_update();  // config settings
    }
    fweff_num = 5;  // only save the defaults
}


// Create Fog Effect from texture name string
// Syntax: #aaaN
// where aaa is alpha, in decimal, 0..255
// where N is fog_effect, from this table
fogwater_effect_e  fwe_code_table[FW_num] =
{
   FW_clear,     // 'A' no fog  (old WATER default)
   FW_cast,      // 'B' paint all surfaces with textures
   FW_colormap,  // 'C' use colormap fog (which only colors all sectors)
   FW_inside,    // 'D' render inside side, plane views (old FOG)
   FW_foglite,   // 'E' outside side, plane views, low alpha overall fog sheet
   FW_fogdust,   // 'F' outside, when in fog apply overall fog sheet (FOG default)
   FW_fogsheet,  // 'G' outside, overall fog sheet, sector join fog sheet
   FW_fogfluid,  // 'H' outside, inside fluid, fogsheet
};


// return index into fweff
int R_Create_FW_effect( int special_linedef, char * tstr )
{
    int def_index =  // of fweff[]
       ( special_linedef == 304 )? FWF_opaque_water  // opaque water
     : ( special_linedef == 302 )? FWF_fog           // fog
     : ( special_linedef == 301 )? FWF_water         // water
     : ( special_linedef == 300 )? FWF_solid_floor   // solid floor
     : 0;
    int fwe_index;
    byte wflags = 0;
    fogwater_t * fwp;

    if( tstr[0] != '#' )
    {
        return def_index;  // can use a default
    }
    // make a unique entry for this situation
    fwe_index = R_get_fweff();
    fwp = &fweff[fwe_index];
    fwp->effect = fweff[def_index].effect;  // get default settings
    fwp->alpha = fweff[def_index].alpha;
    wflags = fweff[def_index].flags;

    if( tstr[0] == '#')  // decode upper texture string
    {
        // alpha
        fwp->alpha = CHAR_TO_INT(tstr[1])*100 + CHAR_TO_INT(tstr[2])*10 + CHAR_TO_INT(tstr[3]);
        wflags &= ~ FWF_default_alpha;
        // fog type
        char chf = tstr[4];
        if( chf >= 'A' && chf <= ('A'+FW_num) )
        {
            fwp->effect = fwe_code_table[ chf - 'A' ];
            wflags &= ~ FWF_default_effect;
        }
    }
    if( special_linedef == 300 )  // FWF_solid_floor
    {
        fwp->effect = FW_clear;  // no internal fog effect
    }
    switch( fwp->effect )
    {
     case FW_foglite:
        fwp->fsh_alpha = fwp->alpha * 0.60;
        break;
     case FW_fogdust:
        fwp->fsh_alpha = fwp->alpha * 0.90;
        break;
     case FW_fogsheet: 
        fwp->fsh_alpha = fwp->alpha * 0.80;
        break;
     case FW_fogfluid:
     default:
        fwp->fsh_alpha = fwp->alpha;
        break;
    }
    fwp->flags = wflags;
    return fwe_index;
}


// R_Load_Data
// Locates all the lumps that will be used by all views
// Must be called after W_Init.
//
void R_Load_Data (void)
{
    CONS_Printf ("\nLoad Textures...");
    R_Load_Textures ();
    CONS_Printf ("\nLoad Flats...");
    R_Load_Flats ();

    CONS_Printf ("\nInitSprites...\n");
    R_Init_SpriteLumps ();
    R_Init_Sprites (sprnames);

    CONS_Printf ("\nLoad Colormaps...\n");
    R_Load_Colormaps ();
#ifdef ANALYZE_GAMEMAP
    Analyze_gamemap();
#endif
}


//
// R_CheckTextureNumForName
// Check whether texture is available.
// Filter out NoTexture indicator.
//
// [WDJ] Original Doom bug: conflict between texture[0] and 0=no-texture.
// 
// Parameter name is 8 char without term.
// Return -1 for not found, 0 for no texture
int  R_CheckTextureNumForName (const char *name)
{
    int  i;

    // "NoTexture" marker.
    if (name[0] == '-')
        return 0;

    for (i=FIRST_TEXTURE ; i<numtextures ; i++)
        if (!strncasecmp (textures[i]->name, name, 8) )
            return i;
#ifdef RANGECHECK
    if( i == 0 )
        GenPrintf(EMSG_warn, "Texture %8.8s  is texture[0], imitates no-texture.\n", name);
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
int R_TextureNumForName (const char* name)
{
    int i;

    i = R_CheckTextureNumForName (name);
#if 0
// [WDJ] DEBUG TRACE, to see where textures have ended up and which are accessed.
#  define trace_SIZE 512
   static char debugtrace_RTNFN[ trace_SIZE ];
   if( i<trace_SIZE && debugtrace_RTNFN[i] != 0x55 ) {
      debug_Printf( "Texture %8.8s is texture[%i]\n", name, i);
      debugtrace_RTNFN[i] = 0x55;
   }
#  undef trace_SIZE   
#endif   

    if(i==-1)
    {
        // Exclude # parameters from "not found" message.
        if( isalpha(name[0]) )
            I_SoftError("R_TextureNumForName: %.8s not found\n", name);

        i=1;	// default to texture[1]
    }
    return i;
}




//
// R_PrecacheLevel
// Preloads all relevant graphics for the level.
//

void R_PrecacheLevel (void)
{
    char*  texturepresent;
    char*  spritepresent;

    int i,j,k,n;
    lumpnum_t  lumpnum;

    thinker_t*          th;
    spriteframe_t *     sf;
    sprite_frot_t *     sv;

    //int numgenerated;  //faB:debug

    if (demoplayback)
        return;

    // do not flush the memory, Z_Malloc twice with same user
    // will cause error in Z_CheckHeap(), 19991022 by Kin
    if (rendermode != render_soft)
        return;

    // Precache flats.
    flatmemory = P_PrecacheLevelFlats();

    //
    // Precache textures.
    //
    // no need to precache all software textures in 3D mode
    // (note they are still used with the reference software view)
    texturepresent = calloc(numtextures,sizeof(char));  // temp alloc, zeroed

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
    //  a wall texture, with an episode dependent name.
    texturepresent[skytexture] = 1;

    //if (devparm)
    //    GenPrintf(EMSG_dev, "Generating textures..\n");

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
    }
    //debug_Printf("total mem for %d textures: %d k\n",numgenerated,texturememory>>10);
    free(texturepresent);

    //
    // Precache sprites.
    //
    spritepresent = calloc(numsprites,sizeof(char));  // temp alloc, zeroed

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
            sf = get_spriteframe( &sprites[i], j );
            n = srp_to_num_rot[ sf->rotation_pattern ];
            for (k=0 ; k<n ; k++)
            {
                sv = get_framerotation( &sprites[i], j, k );
                //Fab: see R_Init_Sprites for more about pat_lumpnum,lumpid
                lumpnum = sv->pat_lumpnum;
                if(devparm)
                   spritememory += W_LumpLength(lumpnum);
                W_CachePatchNum( lumpnum, PU_CACHE );
            }
        }
    }
    free(spritepresent);

    //FIXME: this is no more correct with glide render mode
    if (devparm)
    {
        GenPrintf(EMSG_dev,
                    "Precache level done:\n"
                    "flatmemory:    %ld k\n"
                    "texturememory: %ld k\n"
                    "spritememory:  %ld k\n", flatmemory>>10, texturememory>>10, spritememory>>10 );
    }
#ifdef ANALYZE_TRANSLUCENT_MAPS
    // maps are loaded by now
    Analyze_translucent_maps();
#endif
}


void R_Init_rdata(void)
{
    int i;

    // clear for easier debugging
    memset(extra_colormaps, 0, sizeof(extra_colormaps));

    for(i = 0; i < MAXCOLORMAPS; i++)
    {
        extra_colormap_lumpnum[i] = NO_LUMP;
        extra_colormaps[i].colormap = NULL;
    }
}

// Upon change in rendermode.
void R_rdata_setup_rendermode( void )
{
    int i;
   
    // Colormaps are depedent upon rendermode.
    for( i=0; i<num_extra_colormaps; i++ )
    {
        if( VALID_LUMP( extra_colormap_lumpnum[i] ) )
        {
            // Analyze the lump
            R_Colormap_Analyze( i );
        }
        else
        {
            // Create from parameters
            R_Create_Colormap_ex( & extra_colormaps[i] );
        }
    }
}
