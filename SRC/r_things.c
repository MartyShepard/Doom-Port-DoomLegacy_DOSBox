// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: r_things.c 1419 2019-01-29 08:01:42Z wesleyjohnson $
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
// $Log: r_things.c,v $
// Revision 1.44  2002/11/12 00:06:05  ssntails
// Support for translated translucent columns in software mode.
//
// Revision 1.43  2002/06/30 21:37:48  hurdler
// Ready for 1.32 beta 5 release
//
// Revision 1.42  2001/12/31 16:56:39  metzgermeister
// see Dec 31 log
//
// Revision 1.41  2001/08/12 15:21:04  bpereira
// see my log
//
// Revision 1.40  2001/08/06 23:57:09  stroggonmeth
// Removed portal code, improved 3D floors in hardware mode.
//
// Revision 1.39  2001/06/16 08:07:55  bpereira
// Revision 1.38  2001/06/10 21:16:01  bpereira
//
// Revision 1.37  2001/05/30 18:15:21  stroggonmeth
// Small crashing bug fix...
//
// Revision 1.36  2001/05/30 04:00:52  stroggonmeth
// Fixed crashing bugs in software with 3D floors.
//
// Revision 1.35  2001/05/22 14:22:23  hurdler
// show 3d-floors bug + hack for filesearch with vc++
//
// Revision 1.34  2001/05/07 20:27:16  stroggonmeth
// Revision 1.33  2001/04/27 13:32:14  bpereira
//
// Revision 1.32  2001/04/17 21:12:08  stroggonmeth
// Little commit. Re-enables colormaps for trans columns in C and fixes some sprite bugs.
//
// Revision 1.31  2001/03/30 17:12:51  bpereira
//
// Revision 1.30  2001/03/21 18:24:56  stroggonmeth
// Misc changes and fixes. Code cleanup
//
// Revision 1.29  2001/03/13 22:14:20  stroggonmeth
// Long time no commit. 3D floors, FraggleScript, portals, ect.
//
// Revision 1.28  2001/02/24 13:35:21  bpereira
//
// Revision 1.27  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.26  2000/11/21 21:13:18  stroggonmeth
// Optimised 3D floors and fixed crashing bug in high resolutions.
//
// Revision 1.25  2000/11/12 14:15:46  hurdler
//
// Revision 1.24  2000/11/09 17:56:20  stroggonmeth
// Hopefully fixed a few bugs and did a few optimizations.
//
// Revision 1.23  2000/11/03 02:37:36  stroggonmeth
//
// Revision 1.22  2000/11/02 17:50:10  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.21  2000/10/04 16:19:24  hurdler
// Change all those "3dfx names" to more appropriate names
//
// Revision 1.20  2000/10/02 18:25:45  bpereira
// Revision 1.19  2000/10/01 10:18:19  bpereira
// Revision 1.18  2000/09/30 16:33:08  metzgermeister
// Revision 1.17  2000/09/28 20:57:18  bpereira
// Revision 1.16  2000/09/21 16:45:08  bpereira
// Revision 1.15  2000/08/31 14:30:56  bpereira
//
// Revision 1.14  2000/08/11 21:37:17  hurdler
// fix win32 compilation problem
//
// Revision 1.13  2000/08/11 19:10:13  metzgermeister
// Revision 1.12  2000/04/30 10:30:10  bpereira
// Revision 1.11  2000/04/24 20:24:38  bpereira
// Revision 1.10  2000/04/20 21:47:24  stroggonmeth
// Revision 1.9  2000/04/18 17:39:40  stroggonmeth
//
// Revision 1.8  2000/04/11 19:07:25  stroggonmeth
// Finished my logs, fixed a crashing bug.
//
// Revision 1.7  2000/04/09 02:30:57  stroggonmeth
// Fixed missing sprite def
//
// Revision 1.6  2000/04/08 17:29:25  stroggonmeth
//
// Revision 1.5  2000/04/06 21:06:20  stroggonmeth
// Optimized extra_colormap code...
// Added #ifdefs for older water code.
//
// Revision 1.4  2000/04/04 19:28:43  stroggonmeth
// Global colormaps working. Added a new linedef type 272.
//
// Revision 1.3  2000/04/04 00:32:48  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Refresh of things, i.e. objects represented by sprites.
//
//-----------------------------------------------------------------------------


#include "doomincl.h"
#include "console.h"
#include "g_game.h"
#include "r_local.h"
#include "sounds.h"             //skin sounds
#include "st_stuff.h"
#include "w_wad.h"
#include "z_zone.h"

#include "i_video.h"            //rendermode
#include "m_swap.h"


static void R_Init_Skins (void);

#define MINZ                  (FRACUNIT*4)
#define BASEYCENTER           (BASEVIDHEIGHT/2)

// put this in transmap of visprite to draw a shade
#define VIS_SMOKESHADE        ((void*)-1)       


typedef struct
{
    int         x1;
    int         x2;

    int         column;
    int         topclip;
    int         bottomclip;

} maskdraw_t;


// SoM: A drawnode is something that points to a 3D floor, 3D side or masked
// middle texture. This is used for sorting with sprites.
typedef struct drawnode_s
{
  visplane_t*   plane;
  drawseg_t*    seg;
  drawseg_t*    thickseg;
  ffloor_t*     ffloor;
  vissprite_t*  sprite;

  struct drawnode_s* next;
  struct drawnode_s* prev;
} drawnode_t;


//
// Sprite rotation 0 is facing the viewer,
//  rotation 1 is one angle turn CLOCKWISE around the axis.
// This is not the same as the angle,
//  which increases counter clockwise (protractor).
// There was a lot of stuff grabbed wrong, so I changed it...
//
fixed_t         pspritescale;
fixed_t         pspriteyscale;  //added:02-02-98:aspect ratio for psprites
fixed_t         pspriteiscale;

lighttable_t**  spritelights;	// selected scalelight for the sprite draw

// constant arrays
//  used for psprite clipping and initializing clipping
short           negonearray[MAXVIDWIDTH];
short           screenheightarray[MAXVIDWIDTH];


//
// INITIALIZATION FUNCTIONS
//

// variables used to look up
//  and range check thing_t sprites patches
spritedef_t*    sprites;
int             numsprites;

static char          * spritename;

// spritetmp
#define MAX_FRAMES   29
static spriteframe_t   sprfrm[MAX_FRAMES];
static int             maxframe;

#ifdef ROT16
#define  NUM_SPRITETMP_ROT   16
#else
#define  NUM_SPRITETMP_ROT   8
#endif
static sprite_frot_t   sprfrot[MAX_FRAMES * NUM_SPRITETMP_ROT];


// ==========================================================================
//
//  New sprite loading routines for Legacy : support sprites in pwad,
//  dehacked sprite renaming, replacing not all frames of an existing
//  sprite, add sprites at run-time, add wads at run-time.
//
// ==========================================================================


spriteframe_t *  get_spriteframe( const spritedef_t * spritedef, int frame_num )
{
   return & spritedef->spriteframe[ frame_num ];
}

sprite_frot_t *  get_framerotation( const spritedef_t * spritedef,
                                    int frame_num, byte rotation )
{
   return & spritedef->framerotation[ (frame_num * spritedef->frame_rot) + rotation ];
}

const byte srp_to_num_rot[4] = { 0, 1, 8, 16 };


// Convert sprfrot formats.

// The pattern of named rotations to draw rotations.
// Index rotation_char order.
static const byte rotation_char_to_draw[16] =
{ 0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15 };

static
void  transfer_to_spritetmp( const spritedef_t * spritedef )
{
    int   src_frames =  spritedef->numframes;
    byte  src_frame_rot = spritedef->frame_rot;
    spriteframe_t * fmv, * fmp, * fmp_end;
    sprite_frot_t * rtv, * rtv_nxt, * rtp, * rtp_nxt;
    byte r, srp;

    // From
    fmv = spritedef->spriteframe;
    rtv = spritedef->framerotation;
    // To
    fmp = & sprfrm[0];
    fmp_end = & sprfrm[src_frames];
    rtp = & sprfrot[0];

    // Temp frame size is at the max (8 or 16).
    for( ; fmp < fmp_end; fmp++, fmv++ )
    {
        // Index the next frame first.
        rtv_nxt = rtv + src_frame_rot;
        rtp_nxt = rtp + NUM_SPRITETMP_ROT;
        // Adapt the rotation pattern to fill the temp array.
        srp = fmv->rotation_pattern;
        fmp->rotation_pattern = srp;
        switch( srp )
        {
         case SRP_1:
            // Duplicate into all rotations so later PWAD can alter it properly.
            for( r=0; r<NUM_SPRITETMP_ROT; r++ )
            {
                memcpy( rtp, rtv, sizeof(sprite_frot_t) );
                rtp++;
            }
            break;

         case SRP_8:
            // Copy 8 rotations.
            memcpy( rtp, rtv, sizeof(sprite_frot_t) * 8 );
            break;

#ifdef ROT16
         case SRP_16:
            // Copy 16 rotation draw order into rotation_char order.
            for( r=0; r<16; r++ )
            {
                int rd = rotation_char_to_draw[r];
                memcpy( &rtp[r], &rtv[rd], sizeof(sprite_frot_t) );
            }
            break;
#endif
         default:
            break;
        }
        rtv = rtv_nxt;
        rtp = rtp_nxt;
    }
}

static
void  transfer_from_spritetmp( spritedef_t * spritedef,
                               const byte dst_srp, const byte dst_frame_rot )
{
    int   dst_frames =  spritedef->numframes;
    spriteframe_t * fmv, * fmp, * fmp_end;
    sprite_frot_t * rtv, * rtv_nxt, * rtp, * rtp_nxt;
    byte srp;
#ifdef ROT16
    byte r;
#endif

    // From spritetmp
    fmp = & sprfrm[0];
    fmp_end = & sprfrm[dst_frames];
    rtp = & sprfrot[0];
    // To
    fmv = spritedef->spriteframe;
    rtv = spritedef->framerotation;
   
    for( ; fmp < fmp_end; fmp++, fmv++ )
    {
        rtv_nxt = rtv + dst_frame_rot;
        rtp_nxt = rtp + NUM_SPRITETMP_ROT;
        srp = fmp->rotation_pattern;
        fmv->rotation_pattern = srp;
        switch( srp )
        {
         case SRP_1:
            // copy 1 rotations
            memcpy( rtv, rtp, sizeof(sprite_frot_t) );
            break;

         case SRP_8:
            // copy 8 rotations
            memcpy( rtv, rtp, sizeof(sprite_frot_t) * 8 );
            break;

#ifdef ROT16
         case SRP_16:
            // copy 16 rotation draw order into rotation_char order.
            for( r=0; r<16; r++ )
            {
                int rd = rotation_char_to_draw[r];
                memcpy( &rtv[rd], &rtp[r], sizeof(sprite_frot_t) );
            }
            break;
#endif
         default:
           break;
        }
        rtv = rtv_nxt;
        rtp = rtp_nxt;
    }
}


//
//
//
static
void R_InstallSpriteLump ( lumpnum_t     lumppat,     // graphics patch
                           uint16_t      spritelump_id, // spritelump_t
                           byte          frame,
                           char          rotation_char,
                           boolean       flipped )
{
    int    r;
    byte   rotation = 0;
    byte   frame_srp;
    spriteframe_t * fmp;
    sprite_frot_t * rtp;

#ifdef ROT16
    // The rotations are saved in the sprfrot in the rotation_char order.
    // They are converted to draw index order when the sprfrot is saved.
    if( rotation_char == '0' )
    {
        rotation = 0;
    }
    else if((rotation_char >= '1') && (rotation_char <= '9'))
    {
        rotation = rotation_char - '1';  // 0..8
    }
    else if((rotation_char >= 'A') && (rotation_char <= 'G'))
    {
        rotation = rotation_char - 'A' + 10 - 1;  // 9..15
    }
    else if((rotation_char >= 'a') && (rotation_char <= 'g'))
    {
        rotation = rotation_char - 'a' + 10 - 1;  // 9..15
    }
#else
    if( rotation_char == '0' )
    {
        rotation = 0;
    }
    else if((rotation_char >= '1') && (rotation_char <= '8'))
    {
        rotation = rotation_char - '1';  // 0..7
    }
#endif

    if( frame >= MAX_FRAMES || rotation >= NUM_SPRITETMP_ROT )
    {
        I_SoftError("R_InstallSpriteLump: Bad frame characters in lump %i\n",
                    spritelump_id);
        return;
    }

    if ((int)frame > maxframe)
        maxframe = frame;

    fmp = & sprfrm[frame];
    frame_srp = fmp->rotation_pattern;
    rtp = & sprfrot[ (frame * NUM_SPRITETMP_ROT) + rotation ];

    if( rotation_char == '0' )
    {
        // the lump should be used for all rotations
        if( devparm )
        {
            if( frame_srp == SRP_1 )
            {
                GenPrintf(EMSG_dev,
                 "R_Init_Sprites: Sprite %s frame %c has multiple rot=0 lump\n",
                 spritename, 'A'+frame);
            }
            else if( frame_srp >= SRP_8 )
            {
                GenPrintf(EMSG_dev,
                 "R_Init_Sprites: Sprite %s frame %c has rotations and a rot=0 lump\n",
                 spritename, 'A'+frame);
            }
        }
        fmp->rotation_pattern = SRP_1;
#if 0
        // Only rotation 0.
        rtp->pat_lumpnum = lumppat;
        rtp->spritelump_id  = spritelump_id;
        rtp->flip = (byte)flipped;
#else
        // Fill the whole array with the single rotation so any later overwrites with
        // SRP_8 will keep the single rotation as the default.
        for (r=0 ; r<NUM_SPRITETMP_ROT ; r++)
        {
            rtp->pat_lumpnum = lumppat;
            rtp->spritelump_id  = spritelump_id;
            rtp->flip = (byte)flipped;
            rtp++;
        }
#endif
        return;
    }

    // The lump is one rotation in a set.
    if( (frame_srp == SRP_1) && devparm )
    {
        GenPrintf(EMSG_dev,
           "R_Init_Sprites: Sprite %s frame %c has rotations and a rot=0 lump\n",
           spritename, 'A'+frame);
    }
   
#ifdef ROT16
    byte new_frame_srp = ( rotation > 7 )? SRP_16 : SRP_8;
    if( fmp->rotation_pattern < new_frame_srp )
        fmp->rotation_pattern = new_frame_srp;
#else
    fmp->rotation_pattern = SRP_8;
#endif

    if( (rtp->spritelump_id != -1) && devparm )
    {
        GenPrintf(EMSG_dev,
           "R_Init_Sprites: Sprite %s : %c : %c has two lumps mapped to it\n",
           spritename, 'A'+frame, rotation_char );
    }

    // lumppat & spritelump_id are the same for original Doom, but different
    // when using sprites in pwad : the lumppat points the new graphics
    // [WDJ] Nope, lump patch and size data both come from the lump.
    // This is the only func that changes them, and they are always both updated.
    rtp->pat_lumpnum = lumppat;
    rtp->spritelump_id = spritelump_id;
    rtp->flip = (byte)flipped;
}


// Install a single sprite, given its identifying name (4 chars)
//
// (originally part of R_AddSpriteDefs)
//
// Pass: name of sprite : 4 chars
//       spritedef_t
//       wadnum         : wad number, indexes wadfiles[], where patches
//                        for frames are found
//       startlump      : first lump to search for sprite frames
//       endlump        : AFTER the last lump to search
//
// Returns true if the sprite was succesfully added
//
boolean R_AddSingleSpriteDef (char* sprname, spritedef_t* spritedef, int wadnum, int startlump, int endlump)
{
    spriteframe_t * fmp;
    sprite_frot_t * rtp;
    lumpinfo_t *lumpinfo;
    uint32_t    numname;
    lumpnum_t   lumpnum;
    lumpnum_t   fnd_lumpnum = 0;
    int         l;
    int         frame;
    int         spritelump_id;
    patch_t     patch;	// temp for read header
    byte        array_srp = SRP_NULL;
    byte        rotation, frame_rot;
    char        rotation_char;

    numname = *(uint32_t *)sprname;

    memset (sprfrot,-1, sizeof(sprfrot));
    memset (sprfrm, 0, sizeof(sprfrm));
    maxframe = -1;
 
    // are we 'patching' a sprite already loaded ?
    // if so, it might patch only certain frames, not all
    if (spritedef->numframes) // (then spriteframes is not null)
    {
        // copy the already defined sprite frames
        // Extract to sprfrot format.
        transfer_to_spritetmp( spritedef );
        maxframe = spritedef->numframes - 1;
    }

    // scan the lumps,
    //  filling in the frames for whatever is found
    lumpinfo = wadfiles[wadnum]->lumpinfo;
    if( endlump > wadfiles[wadnum]->numlumps )
        endlump = wadfiles[wadnum]->numlumps;

    for (l=startlump ; l<endlump ; l++)
    {
        lumpnum = WADLUMP(wadnum,l);	// as used by read lump routines
        if (*(uint32_t *)lumpinfo[l].name == numname)
        {
            frame = lumpinfo[l].name[4] - 'A';
            rotation_char = lumpinfo[l].name[5];

            // skip NULL sprites from very old dmadds pwads
            if( W_LumpLength( lumpnum ) <= 8 )
                continue;

            // store sprite info in lookup tables
            //FIXME:numspritelumps do not duplicate sprite replacements
            W_ReadLumpHeader (lumpnum, &patch, sizeof(patch_t)); // to temp
            // [WDJ] Do endian while translate temp to internal.
            spritelump_id = R_Get_spritelump();
            spritelump_t * sl = &spritelumps[spritelump_id];
            sl->width = LE_SWAP16(patch.width)<<FRACBITS;
            sl->offset = LE_SWAP16(patch.leftoffset)<<FRACBITS;
            sl->topoffset = LE_SWAP16(patch.topoffset)<<FRACBITS;
            sl->height = LE_SWAP16(patch.height)<<FRACBITS;

#ifdef HWRENDER
            //BP: we cannot use special trick in hardware mode because feet in ground caused by z-buffer
            if( rendermode != render_soft )
            {
                // topoffset may be negative, use signed compare
                int16_t p_topoffset = LE_SWAP16(patch.topoffset);
                int16_t p_height = LE_SWAP16(patch.height);
                if( p_topoffset>0 && p_topoffset<p_height) // not for psprite
                {
                    // perfect is patch.height but sometime it is too high
                    sl->topoffset =
                       min(p_topoffset+4, p_height)<<FRACBITS;
                }
            }
#endif

            //----------------------------------------------------

            fnd_lumpnum = lumpnum;
            R_InstallSpriteLump (lumpnum, spritelump_id, frame, rotation_char, false);

            if (lumpinfo[l].name[6])
            {
                frame = lumpinfo[l].name[6] - 'A';
                rotation_char = lumpinfo[l].name[7];
                R_InstallSpriteLump (lumpnum, spritelump_id, frame, rotation_char, true);
            }
        }
    }

    //
    // if no frames found for this sprite
    //
    if (maxframe == -1)
    {
        // the first time (which is for the original wad),
        // all sprites should have their initial frames
        // and then, patch wads can replace it
        // we will skip non-replaced sprite frames, only if
        // they have already have been initially defined (original wad)

        //check only after all initial pwads added
        //if (spritedef->numframes == 0)
        //    I_SoftError("R_AddSpriteDefs: no initial frames found for sprite %s\n",
        //             namelist[i]);

        // sprite already has frames, and is not replaced by this wad
        return false;
    }

    maxframe++;

    array_srp = SRP_NULL;
    //
    //  some checks to help development
    //
    for (frame = 0 ; frame < maxframe ; frame++)
    {
        fmp = & sprfrm[ frame ];
        rtp = & sprfrot[frame * NUM_SPRITETMP_ROT];
        if( array_srp < fmp->rotation_pattern )
            array_srp = fmp->rotation_pattern;

        switch( fmp->rotation_pattern )
        {
          case SRP_NULL:
            // no rotations were found for that frame at all
#ifdef DEBUG_CHEXQUEST
            // [WDJ] 4/28/2009 Chexquest
            // [WDJ] not fatal, some wads have broken sprite but still play
            debug_Printf( "R_Init_Sprites: No patches found for %s frame %c \n",
                          sprname, frame+'A');
#else
            I_SoftError ("R_Init_Sprites: No patches found for %s frame %c\n",
                         sprname, frame+'A');
#endif
            break;

          case SRP_1:
            // only the first rotation is needed
            break;

          case SRP_8:
            // must have all 8 frames
            for (rotation=0 ; rotation<8 ; rotation++)
            {
                // we test the patch lump, or the id lump whatever
                // if it was not loaded the two are -1
                if( ! VALID_LUMP(rtp->pat_lumpnum) )
                {
                    I_SoftError("R_Init_Sprites: Sprite %s frame %c is missing rotation %i\n",
                             sprname, frame+'A', rotation);
                    // Limp, use the last sprite lump read for this sprite.
                    rtp->pat_lumpnum = fnd_lumpnum;
                }
                rtp++;
            }
            break;

#ifdef ROT16
          case SRP_16:
            // must have all 16 frames
            for (rotation=0 ; rotation<16 ; rotation++)
            {
                // we test the patch lump, or the id lump whatever
                // if it was not loaded the two are -1
                if( ! VALID_LUMP(rtp->pat_lumpnum) )
                {
                    I_SoftError("R_Init_Sprites: Sprite %s frame %c is missing rotation %i\n",
                             sprname, frame+'A', rotation);
                    // Limp, use the last sprite lump read for this sprite.
                    rtp->pat_lumpnum = fnd_lumpnum;
                }
                rtp++;
            }
            break;
#endif
        }
    }

    frame_rot = srp_to_num_rot[ array_srp ];
   
    // allocate space for the frames present and copy spritetmp to it
    if( spritedef->numframes                // has been allocated
        && (spritedef->numframes < maxframe  // more frames are defined
            || spritedef->frame_rot < frame_rot) ) // more rotations are defined
    {
        Z_Free (spritedef->spriteframe);
        Z_Free (spritedef->framerotation);
        spritedef->spriteframe = NULL;
        spritedef->framerotation = NULL;
    }

    // allocate this sprite's frames
    if (spritedef->spriteframe == NULL)
    {
        spritedef->spriteframe =
            Z_Malloc (maxframe * sizeof(spriteframe_t), PU_STATIC, NULL);
        spritedef->framerotation =
            Z_Malloc (maxframe * frame_rot * sizeof(sprite_frot_t), PU_STATIC, NULL);
    }

    spritedef->numframes = maxframe;
    spritedef->frame_rot = frame_rot;
    transfer_from_spritetmp( spritedef, array_srp, frame_rot );

    return true;
}



//
// Search for sprites replacements in a wad whose names are in namelist
//
void R_AddSpriteDefs (char** namelist, int wadnum)
{
    lumpnum_t  start_ln, end_ln;
    int         i, ln1, ln2;
    int         addsprites;

    // find the sprites section in this pwad
    // we need at least the S_END
    // (not really, but for speedup)

    start_ln = W_CheckNumForNamePwad ("S_START",wadnum,0);
    if( ! VALID_LUMP(start_ln) )
        start_ln = W_CheckNumForNamePwad ("SS_START",wadnum,0); //deutex compatib.
    if( ! VALID_LUMP(start_ln) )
    {
        // search frames from start of wad
        ln1 = 0;
    }
    else
    {
        // just after S_START
        ln1 = LUMPNUM( start_ln ) + 1;
    }


    end_ln = W_CheckNumForNamePwad ("S_END",wadnum,0);
    if( ! VALID_LUMP(end_ln) )
        end_ln = W_CheckNumForNamePwad ("SS_END",wadnum,0);     //deutex compatib.
    if( ! VALID_LUMP(end_ln) )
    {
        if (devparm)
            GenPrintf(EMSG_dev, "no sprites in pwad %d\n", wadnum);
        return;
        //I_Error ("R_AddSpriteDefs: S_END, or SS_END missing for sprites "
        //         "in pwad %d\n",wadnum);
    }
    ln2 = LUMPNUM( end_ln );

    //
    // scan through lumps, for each sprite, find all the sprite frames
    //
    addsprites = 0;
    for (i=0 ; i<numsprites ; i++)
    {
        spritename = namelist[i];

        if (R_AddSingleSpriteDef (spritename, &sprites[i], wadnum, ln1, ln2) )
        {
            // if a new sprite was added (not just replaced)
            addsprites++;
            if (devparm)
                GenPrintf(EMSG_dev, "sprite %s set in pwad %d\n", namelist[i], wadnum);//Fab
        }
    }

    GenPrintf(EMSG_info, "%d sprites added from file %s\n", addsprites, wadfiles[wadnum]->filename);//Fab
    //CONS_Error ("press enter\n");
}



//
// GAME FUNCTIONS
//

// [WDJ] Remove sprite limits. This is the soft limit, the hard limit is twice this value.
CV_PossibleValue_t spritelim_cons_t[] = {
   {128, "128"}, {192,"192"}, {256, "256"}, {384,"384"},
   {512,"512"}, {768,"768"}, {1024,"1024"}, {1536,"1536"},
   {2048,"2048"}, {3072, "3072"}, {4096,"4096"}, {6144, "6144"},
   {8192,"8192"}, {12288, "12288"}, {16384,"16384"},
   {0, NULL} };
consvar_t  cv_spritelim = { "sprites_limit", "512", CV_SAVE, spritelim_cons_t, NULL };

// [WDJ] Remove sprite limits.
static int  vspr_change_delay = 128;  // quick first allocate
static unsigned int  vspr_random = 0x7f43916;
static int  vspr_halfcnt; // count to halfway
  
static int  vspr_count = 0;	// count of sprites in the frame
static int  vspr_needed = 64;     // max over several frames
static int  vspr_max = 0;	// size of array - 1
static vissprite_t*    vissprites = NULL;  // [0 .. vspr_max]
static vissprite_t*    vissprite_last;	   // last vissprite in array
static vissprite_t*    vissprite_p;    // next free vissprite
static vissprite_t*    vissprite_far;  // a far vissprite, can be replaced

static vissprite_t     vsprsortedhead;  // sorted list head (circular linked)

// Call between frames, it does not copy contents, and does not init.
void vissprites_tablesize ( void )
{
    int request;
    // sprite needed over several frames
    if ( vspr_count > vspr_needed )
        vspr_needed = vspr_count;  // max
    else
        vspr_needed -= (vspr_needed - vspr_count) >> 8;  // slow decay
   
    request = ( vspr_needed > cv_spritelim.value )?
              (cv_spritelim.value + vspr_needed)/2  // soft limit
            : vspr_needed;  // under limit
        
    // round-up to avoid trivial adjustments
    request = (request < (256*6))?
              (request + 0x003F) & ~0x003F   // 64
            : (request + 0x00FF) & ~0x00FF;  // 256
    // hard limit
    if ( request > (cv_spritelim.value * 2) )
        request = cv_spritelim.value * 2;
    
    if ( request == (vspr_max+1) )
        return;		// same as existing allocation

    if( vspr_change_delay < MAXINT )
    {
        vspr_change_delay ++;
    }
    if ( request < vspr_max )
    {
        // decrease allocation
        if ( ( request < cv_spritelim.value )  // once up to limit, stay there
             || ( request > (vspr_max / 4))  // avoid vacillation
             || ( vspr_change_delay < 8192 )  )  // delay decreases
        {
            if (vspr_max <= cv_spritelim.value * 2)  // unless user setting was reduced
                return;
        }
        if ( request < 64 )
             request = 64;  // absolute minimum
    }
    else
    {
        // delay to get max sprites needed for new scene
        // but not too much or is very visible
        if( vspr_change_delay < 16 )  return;
    } 
    vspr_change_delay = 0;
    // allocate
    if ( vissprites )
    {
        free( vissprites );
    }
    do {  // repeat allocation attempt until success
        vissprites = (vissprite_t*) malloc ( sizeof(vissprite_t) * request );
        if( vissprites )
        {
            vspr_max = request-1;
            return;	// normal successful allocation
        }
        // allocation failed
        request /= 2;  // halve the request
    }while( request > 63 );
       
    I_Error ("Cannot allocate vissprites\n");
}


//
// R_Init_Sprites
// Called at program start.
//
void R_Init_Sprites (char** namelist)
{
    int         i;
    char**      check;

    for (i=0 ; i<MAXVIDWIDTH ; i++)
    {
        negonearray[i] = -1;
    }

    vissprites_tablesize();  // initial allocation

    //
    // count the number of sprite names, and allocate sprites table
    //
    check = namelist;
    while (*check != NULL)
        check++;
    numsprites = check - namelist;

    if (!numsprites)
        I_Error ("R_AddSpriteDefs: no sprites in namelist\n");

    sprites = Z_Malloc(numsprites * sizeof(*sprites), PU_STATIC, NULL);
    memset (sprites, 0, numsprites * sizeof(*sprites));

    // find sprites in each -file added pwad
    for (i=0; i<numwadfiles; i++)
        R_AddSpriteDefs (namelist, i);

    //
    // now check for skins
    //

    // all that can be before loading config is to load possible skins
    R_Init_Skins ();
    for (i=0; i<numwadfiles; i++)
        R_AddSkins (i);


    //
    // check if all sprites have frames
    //
    /*
    for (i=0; i<numsprites; i++)
         if (sprites[i].numframes<1)
             I_SoftError("R_Init_Sprites: sprite %s has no frames at all\n", sprnames[i]);
    */
}



//
// R_Clear_Sprites
// Called at frame start.
//
void R_Clear_Sprites (void)
{
    vissprites_tablesize();  // re-allocation
    vspr_random += (vspr_count & 0xFFFF0) + 0x010001;  // just keep it changing
    vissprite_last = &vissprites[vspr_max];
    vissprite_p = vissprites;  // first free vissprite
    vsprsortedhead.next = vsprsortedhead.prev = &vsprsortedhead;  // init sorted
    vsprsortedhead.scale = FIXED_MAX;  // very near, so it is rejected in farthest search
    vissprite_far = & vsprsortedhead;
    vspr_halfcnt = 0; // force vissprite_far init
    vspr_count = 0;  // stat for allocation
}


//
// R_NewVisSprite
//
static vissprite_t     overflowsprite;

// [WDJ] New vissprite sorted by scale
// Closer sprites get preference in the vissprite list when too many.
// Sorted list is farthest to nearest (circular).
//   scale : the draw scale, representing distance from viewer
//   dist_pri : distance priority, 0..255, dead are low, monsters are high
static
vissprite_t* R_NewVisSprite ( fixed_t scale, byte dist_pri,
                             /*OUT*/ vissprite_t * oldsprite )
{
    vissprite_t * vs;
    register vissprite_t * ns;

    vspr_count ++;	// allocation stat
    if (vissprite_p == vissprite_last)  // array full ?
    { 
        unsigned int rn, cnt;
        // array is full
        vspr_random += 0x021019; // semi-random stirring (prime)
        if ( vsprsortedhead.next->scale > scale )
        { 
            // New sprite is farther than farthest sprite in array,
            // even far sprites have random chance of being seen (flicker)
            // Avg (pri=128) monster has 1/16 chance.
            if (((vspr_random >> 8) & 0x7FF) > dist_pri)
                return &overflowsprite;
        }
        // Must remove a sprite to make room.
        // Sacrifice a random sprite from farthest half.
        // Skip a random number of sprites, at least 1.
        // Try for a tapering distance effect.
        rn = (vspr_random & 0x000F) + ((vspr_max - vspr_halfcnt) >> 7);
        for( cnt = 2; ; cnt-- )  // tries to find lower priority
        {
            if( vspr_halfcnt <= 0 ) // halfway count trigger
            {
                // init, or re-init
                vspr_halfcnt = vspr_max / 2;
                vissprite_far = vsprsortedhead.next; // farthest
            }
            vs = vissprite_far;
            rn ++;  // at least 1
            // Move vissprite_far off the sprite that will be removed.
            vspr_halfcnt -= rn;  // count down to halfway
            for( ; rn > 0 ; rn -- )
            {
                vissprite_far = vissprite_far->next; // to nearer sprites
            }
            // Compare priority, but only a few times.
            if( cnt == 0 )  break;
            if( vs->dist_priority <= dist_pri )   break;
        }

        // unlink it so it can be re-linked by distance
        vs->next->prev = vs->prev;
        vs->prev->next = vs->next;
    }
    else
    {
        // still filling up array
        vs = vissprite_p ++;
    }

    // Set links so order is farthest to nearest.
    // Check the degenerate case first and avoid this test in the loop below.
    // Empty list looks to have head as max nearest sprite, so first is farthest.
    if (vsprsortedhead.next->scale > scale)
    {
        // New is farthest, this will happen often because of close preference.
        ns = &vsprsortedhead; // farthest is linked after head
    }
    else
    {
        // Search nearest to farthest.
        // The above farthest check ensures that search will hit something farther.
        ns = vsprsortedhead.prev; // nearest
        while( ns->scale > scale )  // while new is farther
        {
            ns = ns->prev;
        }
    }
    // ns is farther than new
    // Copy before linking, is easier.
    if( oldsprite )
        memcpy( vs, oldsprite, sizeof(vissprite_t));
    // link new vs after ns (nearer than ns)
    vs->next = ns->next;
    vs->next->prev = vs;
    ns->next = vs;
    vs->prev = ns;
    vs->dist_priority = dist_pri;

    return vs;
}



//
// R_DrawMaskedColumn
// Used for sprites and masked mid textures.
// Masked means: partly transparent, i.e. stored
//  in posts/runs of opaque pixels.
// The colfunc_2s function for TM_patch and TM_combine_patch
//
// draw masked global parameters
// clipping array[x], in int screen coord.
short*          dm_floorclip;
short*          dm_ceilingclip;

fixed_t         dm_yscale;  // world to fixed_t screen coord
// draw masked column top and bottom, in fixed_t screen coord.
fixed_t         dm_top_patch, dm_bottom_patch;
// window clipping in fixed_t screen coord., set to FIXED_MAX to disable
// to draw, require dm_windowtop < dm_windowbottom
fixed_t         dm_windowtop, dm_windowbottom;


void R_DrawMaskedColumn (column_t* column)
{
    fixed_t     top_post_sc, bottom_post_sc;  // fixed_t screen coord.
    fixed_t     basetexturemid = dc_texturemid; // save to restore after

    // over all column posts for this column
    for ( ; column->topdelta != 0xff ; )
    {
        // calculate unclipped screen coordinates
        //  for post
        top_post_sc = dm_top_patch + dm_yscale*column->topdelta;
        bottom_post_sc = (dm_bottom_patch == FIXED_MAX) ?
            top_post_sc + dm_yscale*column->length
            : dm_bottom_patch + dm_yscale*column->length;

        // fixed_t to int screen coord.
        dc_yl = (top_post_sc+FRACUNIT-1)>>FRACBITS;
        dc_yh = (bottom_post_sc-1)>>FRACBITS;

        if(dm_windowtop != FIXED_MAX && dm_windowbottom != FIXED_MAX)
        {
          // screen coord. where +y is down screen
          if(dm_windowtop > top_post_sc)
            dc_yl = (dm_windowtop + FRACUNIT - 1) >> FRACBITS;
          if(dm_windowbottom < bottom_post_sc)
            dc_yh = (dm_windowbottom - 1) >> FRACBITS;
        }

        if (dc_yh >= dm_floorclip[dc_x])
            dc_yh = dm_floorclip[dc_x]-1;
        if (dc_yl <= dm_ceilingclip[dc_x])
            dc_yl = dm_ceilingclip[dc_x]+1;

        // [WDJ] limit to split screen area above status bar,
        // instead of whole screen,
        if (dc_yl <= dc_yh && dc_yl < rdraw_viewheight && dc_yh > 0)  // [WDJ] exclude status bar
        {
            //[WDJ] phobiata.wad has many views that need clipping
            if ( dc_yl < 0 )   dc_yl = 0;
            if ( dc_yh >= rdraw_viewheight )   dc_yh = rdraw_viewheight - 1;

            dc_source = (byte *)column + 3;
            dc_texturemid = basetexturemid - (column->topdelta<<FRACBITS);
            // dc_source = (byte *)column + 3 - column->topdelta;
            fog_col_length = column->length;

            // Drawn by either R_DrawColumn
            //  or (SHADOW) R_DrawFuzzColumn.
#ifdef PARANOIA 
            //Hurdler: quick fix... something more proper should be done!!!
            // [WDJ] Fixed by using rdraw_viewheight instead of vid.height
            // in limit test above.
            if (!ylookup[dc_yl] && colfunc==basecolfunc) // R_DrawColumn_8
            {
                I_SoftError("WARNING: avoiding a crash in %s %d\n", __FILE__, __LINE__);
            }
            else
            {
                colfunc ();
            }
#else
            colfunc ();
#endif
        }
        column = (column_t *)(  (byte *)column + column->length + 4);
    }

    dc_texturemid = basetexturemid;
}



//
// R_DrawVisSprite
//  dm_floorclip and dm_ceilingclip should also be set.
//
static void R_DrawVisSprite ( vissprite_t*          vis,
                              int                   x1,
                              int                   x2 )
{
    column_t*           column;
    int                 texturecolumn;
    fixed_t             texcol_frac;
    patch_t*            patch;


    //Fab:R_Init_Sprites now sets a wad lump number
    // Use common patch read so do not have patch in cache without endian fixed.
    patch = W_CachePatchNum (vis->patch, PU_CACHE);

    dc_colormap = vis->colormap;

    // Support for translated and translucent sprites. SSNTails 11-11-2002
    dr_alpha = 0;  // ensure use of translucent normally for all drawers
    if((vis->mobjflags & MFT_TRANSLATION6) && vis->translucentmap)
    {
        colfunc = skintranscolfunc;
        dc_translucent_index = vis->translucent_index;
        dc_translucentmap = vis->translucentmap;
        dc_skintran = MFT_TO_SKINMAP( vis->mobjflags ); // skins 1..
    }
    else if (vis->translucentmap==VIS_SMOKESHADE)
    {
        // Draw smoke
        // shadecolfunc uses 'reg_colormaps'
        colfunc = shadecolfunc;
    }
    else if (vis->translucentmap)
    {
//        colfunc = fuzzcolfunc;
        colfunc = (vis->mobjflags & MF_SHADOW)? fuzzcolfunc : transcolfunc;
        dc_translucent_index = vis->translucent_index;
        dc_translucentmap = vis->translucentmap;    //Fab:29-04-98: translucency table
    }
    else if (vis->mobjflags & MFT_TRANSLATION6)
    {
        // translate green skin to another color
        colfunc = skincolfunc;
        dc_skintran = MFT_TO_SKINMAP( vis->mobjflags ); // skins 1..
    }

    if((vis->extra_colormap || view_colormap) && !fixedcolormap)
    {
       // reverse indexing, and change to extra_colormap, default 0
       int lightindex = dc_colormap? (dc_colormap - reg_colormaps) : 0;
       lighttable_t* cm = (view_colormap? view_colormap : vis->extra_colormap->colormap);
       dc_colormap = & cm[ lightindex ];
    }
    if(!dc_colormap)
      dc_colormap = & reg_colormaps[0];

    //dc_iscale = abs(vis->xiscale)>>detailshift;  ???
    dc_iscale = FixedDiv (FRACUNIT, vis->scale);
    dc_texturemid = vis->texturemid;
    dc_texheight = 0;

    texcol_frac = vis->startfrac;
    dm_yscale = vis->scale;
    dm_top_patch = centeryfrac - FixedMul(dc_texturemid,dm_yscale);
    dm_windowtop = dm_windowbottom = dm_bottom_patch = FIXED_MAX; // disable

    for (dc_x=vis->x1 ; dc_x<=vis->x2 ; dc_x++, texcol_frac += vis->xiscale)
    {
        texturecolumn = texcol_frac>>FRACBITS;
#ifdef RANGECHECK
        if (texturecolumn < 0 || texturecolumn >= patch->width) {
            // [WDJ] Give msg and don't draw it
            I_SoftError ("R_DrawVisSprite: bad texturecolumn\n");
            return;
        }
#endif
        column = (column_t *) ((byte *)patch + patch->columnofs[texturecolumn]);
        R_DrawMaskedColumn (column);
    }

    colfunc = basecolfunc;
}




//
// R_SplitSprite
// runs through a sector's lightlist and
static void R_SplitSprite (vissprite_t* sprite, mobj_t* thing)
{
  int           i;
  int		sz_cut;		// where lightheight cuts on screen
  fixed_t	lightheight;
  sector_t*     sector;
  ff_light_t*   ff_light; // lightlist item
  vissprite_t*  newsprite;

  sector = sprite->sector;

  for(i = 1; i < sector->numlights; i++)	// from top to bottom
  {
    ff_light = &frontsector->lightlist[i];
    lightheight = ff_light->height;
     
    // must be a caster
    if(lightheight >= sprite->gz_top || !(ff_light->caster->flags & FF_CUTSPRITES))
      continue;
    if(lightheight <= sprite->gz_bot)
      return;

    // where on screen the lightheight cut appears
    sz_cut = (centeryfrac - FixedMul(lightheight - viewz, sprite->scale)) >> FRACBITS;
    if(sz_cut < 0)
            continue;
    if(sz_cut > rdraw_viewheight)	// [WDJ] 11/14/2009
            return;
        
    // Found a split! Make a new sprite, copy the old sprite to it, and
    // adjust the heights.
    newsprite = R_NewVisSprite( sprite->scale, sprite->dist_priority, sprite );

    sprite->cut |= SC_BOTTOM;
    sprite->gz_bot = lightheight;

    newsprite->gz_top = sprite->gz_bot;

    // [WDJ] 11/14/2009 clip at window again, fix split sprites corrupt status bar
    sprite->sz_bot = (sz_cut < rdraw_viewheight)? sz_cut : rdraw_viewheight;
    newsprite->sz_top = sz_cut - 1;

    if(lightheight < sprite->pz_top
           && lightheight > sprite->pz_bot)
    {
        sprite->pz_bot = newsprite->pz_top = lightheight;
    }
    else
    {
        newsprite->pz_bot = newsprite->gz_bot; 
        newsprite->pz_top = newsprite->gz_top;
    }

    newsprite->cut |= SC_TOP;
    if(!(ff_light->caster->flags & FF_NOSHADE))
    {
      lightlev_t  vlight = *ff_light->lightlevel  // visible light 0..255
          + ((ff_light->caster->flags & FF_FOG)? extralight_fog : extralight);

      spritelights =
          (vlight < 0) ? scalelight[0]
        : (vlight >= 255) ? scalelight[LIGHTLEVELS-1]
        : scalelight[vlight>>LIGHTSEGSHIFT];

      newsprite->extra_colormap = ff_light->extra_colormap;

      if (thing->frame & FF_SMOKESHADE)
        ;
      else
      {
/*        if (thing->frame & FF_TRANSMASK)
          ;
        else if (thing->flags & MF_SHADOW)
          ;*/

        if (fixedcolormap )
          ;
        else if ((thing->frame & (FF_FULLBRIGHT|FF_TRANSMASK)
                  || thing->flags & MF_SHADOW)
                 && !(newsprite->extra_colormap && newsprite->extra_colormap->fog))
          ;
        else
        {
          int dlit = sprite->xscale>>(LIGHTSCALESHIFT-detailshift);
          if (dlit >= MAXLIGHTSCALE)
            dlit = MAXLIGHTSCALE-1;
          newsprite->colormap = spritelights[dlit];
        }
      }
    }
    sprite = newsprite;
  }
}


//
// R_ProjectSprite
// Generates a vissprite for a thing, if it might be visible.
//
static void R_ProjectSprite (mobj_t* thing)
{
    fixed_t             tr_x, tr_y;
    fixed_t             gxt, gyt;
    fixed_t             tx, tz;

    fixed_t             xscale;
    fixed_t             yscale; //added:02-02-98:aaargll..if I were a math-guy!!!

    int                 x1, x2, fr;

    sector_t*		thingsector;	 // [WDJ] 11/14/2009
   
    spritedef_t*        sprdef;
    spriteframe_t *     sprframe;
    sprite_frot_t *     sprfrot;
    spritelump_t *      sprlump;

    unsigned int        rot;
    byte                flip;

    byte                dist_pri;  // distance priority

    vissprite_t*        vis;
    ff_light_t *        ff_light = NULL;  // lightlist light

    angle_t             ang;
    fixed_t             iscale;

    //SoM: 3/17/2000
    fixed_t             gz_top;
    int                 thingmodelsec;
    boolean	        thing_has_model;  // has a model, such as water


    // transform the origin point
    tr_x = thing->x - viewx;
    tr_y = thing->y - viewy;

    gxt = FixedMul(tr_x,viewcos);
    gyt = -FixedMul(tr_y,viewsin);

    tz = gxt-gyt;

    // thing is behind view plane?
    if (tz < MINZ)
        return;

    // aspect ratio stuff :
    xscale = FixedDiv(projection, tz);
    yscale = FixedDiv(projectiony, tz);

    gxt = -FixedMul(tr_x,viewsin);
    gyt = FixedMul(tr_y,viewcos);
    tx = -(gyt+gxt);

    // too far off the side?
    if (abs(tx)>(tz<<2))
        return;

    // decide which patch to use for sprite relative to player
#ifdef RANGECHECK
    if ((unsigned)thing->sprite >= numsprites) {
        // [WDJ] Give msg and don't draw it
        I_SoftError ("R_ProjectSprite: invalid sprite number %i\n",
                 thing->sprite);
        return;
    }
#endif

    //Fab:02-08-98: 'skin' override spritedef currently used for skin
    if (thing->skin)
        sprdef = &((skin_t *)thing->skin)->spritedef;
    else
        sprdef = &sprites[thing->sprite];

#ifdef RANGECHECK
    if ( (thing->frame&FF_FRAMEMASK) >= sprdef->numframes ) {
        // [WDJ] Give msg and don't draw it
        I_SoftError ("R_ProjectSprite: invalid sprite frame %i : %i for %s\n",
                 thing->sprite, thing->frame, sprnames[thing->sprite]);
        return;
    }
#endif

    // [WDJ] segfault control in heretic shareware, not all sprites present
    if( (byte*)sprdef->spriteframe < (byte*)0x1000 )
    {
        I_SoftError("R_ProjectSprite: sprframe ptr NULL for sprite %d\n", thing->sprite );
        return;
    }

    fr = thing->frame & FF_FRAMEMASK;
    sprframe = get_spriteframe( sprdef, fr );

    if( sprframe->rotation_pattern == SRP_1 )
    {
        // use single rotation for all views
        rot = 0;  //Fab: for vis->patch below
    }
    else
    {
        // choose a different rotation based on player view
        ang = R_PointToAngle(thing->x, thing->y);       // uses viewx,viewy

        if( sprframe->rotation_pattern == SRP_8)
        {
            // 8 direction rotation pattern
            rot = (ang - thing->angle + (unsigned) (ANG45/2) * 9) >> 29;
        }
#ifdef ROT16
        else if( sprframe->rotation_pattern == SRP_16)
        {
            // 16 direction rotation pattern
            rot = (ang - thing->angle + (unsigned) (ANG45/4) * 17) >> 28;
        }
#endif
        else return;
    }
   
    sprfrot = get_framerotation( sprdef, fr, rot );
    //Fab: [WDJ] spritelump_id is the index
    sprlump = &spritelumps[sprfrot->spritelump_id];
    flip = sprfrot->flip;

    // calculate edges of the shape
    if( flip )
    {
        // [WDJ] Flip offset, as suggested by Fraggle (seen in prboom 2003)
        tx -= sprlump->width - sprlump->offset;
    }
    else
    {
        // apply offset from sprite lump normally
        tx -= sprlump->offset;
    }
    x1 = (centerxfrac + FixedMul (tx,xscale) ) >>FRACBITS;

    // off the right side?
    if (x1 > rdraw_viewwidth)
        return;

    tx += sprlump->width;
    x2 = ((centerxfrac + FixedMul (tx,xscale) ) >>FRACBITS) - 1;

    // off the left side
    if (x2 < 0)
        return;

    //SoM: 3/17/2000: Disregard sprites that are out of view..
    gz_top = thing->z + sprlump->topoffset;

    thingsector = thing->subsector->sector;	 // [WDJ] 11/14/2009
    if(thingsector->numlights)
    {
      lightlev_t  vlight;
      ff_light = R_GetPlaneLight(thingsector, gz_top);
      vlight = *ff_light->lightlevel;
      if(!( ff_light->caster && (ff_light->caster->flags & FF_FOG) ))
        vlight += extralight;

      spritelights =
          (vlight < 0) ? scalelight[0]
        : (vlight >= 255) ? scalelight[LIGHTLEVELS-1]
        : scalelight[vlight>>LIGHTSEGSHIFT];
    }

    thingmodelsec = thingsector->modelsec;
    thing_has_model = thingsector->model > SM_fluid; // water

    if (thing_has_model)   // only clip things which are in special sectors
    {
      sector_t * thingmodsecp = & sectors[thingmodelsec];

      // [WDJ] 4/20/2010  Added some structure and ()
      // [WDJ] Could use viewer_at_water to force view of objects above and
      // below to be seen simultaneously.
      // Instead have choosen to have objects underwater not be seen until
      // viewer_underwater.
      // When at viewer_at_water, will not see objects above nor below the water.
      // As this has some validity in reality, and does not generate HOM,
      // will live with it.  It is transient, and most players will not notice.
      if (viewer_has_model)
      {
          // [WDJ] FakeFlat uses viewz<=floor, and thing used viewz<floor,
          // They both should be the same or else things do not
          // appear when just underwater.
          if( viewer_underwater ?
              (thing->z >= thingmodsecp->floorheight)
              : (gz_top < thingmodsecp->floorheight)
              )
              return;
          // [WDJ] FakeFlat uses viewz>=ceiling, and thing used viewz>ceiling,
          // They both should be the same or else things do not
          // appear when just over ceiling.
          if( viewer_overceiling ?
              ((gz_top < thingmodsecp->ceilingheight) && (viewz > thingmodsecp->ceilingheight))
              : (thing->z >= thingmodsecp->ceilingheight)
              )
              return;
      }
    }

    // Store information in a vissprite.
    dist_pri = thing->height >> 16;  // height (fixed_t), 0..120, 56=norm.
    if( thing->flags & MF_MISSILE )
        dist_pri += 60;  // missiles are important
    else
    {
        // CORPSE may not be MF_SHOOTABLE.
        if( thing->flags & MF_CORPSE )
            dist_pri >>= 2;  // corpse has much less priority
        else if( thing->flags & (MF_SHOOTABLE|MF_COUNTKILL) )
            dist_pri += 20;  // monsters are important too
    }

    vis = R_NewVisSprite ( yscale, dist_pri, NULL );
    // do not waste time on the massive number of sprites in the distance
    if( vis == &overflowsprite )  // test for rejected, or too far
        return;

    // [WDJ] Only pass water models, not colormap model sectors
    vis->heightsec = thing_has_model ? thingmodelsec : -1 ; //SoM: 3/17/2000
    vis->mobjflags = (thing->flags & MF_SHADOW) | (thing->tflags & MFT_TRANSLATION6);
    vis->scale = yscale;           //<<detailshift;
    vis->gx = thing->x;
    vis->gy = thing->y;
    vis->gz_bot = gz_top - sprlump->height;
    vis->gz_top = gz_top;
    vis->thingheight = thing->height;
    vis->pz_bot = thing->z;
    vis->pz_top = vis->pz_bot + vis->thingheight;
    vis->texturemid = vis->gz_top - viewz;
    // foot clipping
    if(thing->flags2&MF2_FEETARECLIPPED
       && thing->z <= thingsector->floorheight)
    { 
         vis->texturemid -= 10*FRACUNIT;
    }

    vis->x1 = (x1 < 0) ? 0 : x1;
    vis->x2 = (x2 >= rdraw_viewwidth) ? rdraw_viewwidth-1 : x2;
    vis->xscale = xscale; //SoM: 4/17/2000
    vis->sector = thingsector;
    vis->sz_top = (centeryfrac - FixedMul(vis->gz_top - viewz, yscale)) >> FRACBITS;
    vis->sz_bot = (centeryfrac - FixedMul(vis->gz_bot - viewz, yscale)) >> FRACBITS;
    vis->cut = SC_NONE;	// none, false
    vis->extra_colormap = (ff_light)?
        ff_light->extra_colormap
        : thingsector->extra_colormap;

    iscale = FixedDiv (FRACUNIT, xscale);

    if (flip)
    {
        vis->startfrac = sprlump->width - 1;
        vis->xiscale = -iscale;
    }
    else
    {
        vis->startfrac = 0;
        vis->xiscale = iscale;
    }

    if (vis->x1 > x1)
        vis->startfrac += vis->xiscale*(vis->x1-x1);

    //Fab: lumppat is the lump number of the patch to use, this is different
    //     than spritelump_id for sprites-in-pwad : the graphics are patched
    // [WDJ] Nope, both are updated from the lump together.
    vis->patch = sprfrot->pat_lumpnum;


//
// determine the colormap (lightlevel & special effects)
//
    vis->translucentmap = NULL;
    vis->translucent_index = 0;
    
    // specific translucency
    if (thing->frame & FF_SMOKESHADE)
    {
        // Draw smoke
        // not really a colormap ... see R_DrawVisSprite
//        vis->colormap = VIS_SMOKESHADE; 
        vis->colormap = NULL;
        vis->translucentmap = VIS_SMOKESHADE; 
    }
    else
    {
        if (thing->frame & FF_TRANSMASK)
        {
            vis->translucent_index = (thing->frame&FF_TRANSMASK)>>FF_TRANSSHIFT;
            vis->translucentmap = & translucenttables[ FF_TRANSLU_TABLE_INDEX(thing->frame) ];
        }
        else if (thing->flags & MF_SHADOW)
        {
            // actually only the player should use this (temporary invisibility)
            // because now the translucency is set through FF_TRANSMASK
            vis->translucent_index = TRANSLU_hi;
            vis->translucentmap = & translucenttables[ TRANSLU_TABLE_hi ];
        }

    
        if (fixedcolormap )
        {
            // fixed map : all the screen has the same colormap
            //  eg: negative effect of invulnerability
            vis->colormap = fixedcolormap;
        }
        else if( ( (thing->frame & (FF_FULLBRIGHT|FF_TRANSMASK))
                   || (thing->flags & MF_SHADOW) )
                 && (!vis->extra_colormap || !vis->extra_colormap->fog)  )
        {
            // full bright : goggles
            vis->colormap = & reg_colormaps[0];
        }
        else
        {

            // diminished light
            int index = xscale>>(LIGHTSCALESHIFT-detailshift);

            if (index >= MAXLIGHTSCALE)
                index = MAXLIGHTSCALE-1;

            vis->colormap = spritelights[index];
        }
    }

    if(thingsector->numlights)
        R_SplitSprite(vis, thing);
}




//
// R_AddSprites
// During BSP traversal, this adds sprites by sector.
//
void R_AddSprites (sector_t* sec, int lightlevel)
{
    mobj_t*   thing;

    if (rendermode != render_soft)
        return;

    // BSP is traversed by subsector.
    // A sector might have been split into several
    //  subsectors during BSP building.
    // Thus we check whether its already added.
    if (sec->validcount == validcount)
        return;

    // Well, now it will be done.
    sec->validcount = validcount;

    if(!sec->numlights)  // otherwise see ProjectSprite
    {
      if(sec->model < SM_fluid)   lightlevel = sec->lightlevel;

      lightlev_t  vlight = lightlevel + extralight;

      spritelights =
          (vlight < 0) ? scalelight[0]
        : (vlight >= 255) ? scalelight[LIGHTLEVELS-1]
        : scalelight[vlight>>LIGHTSEGSHIFT];
    }

    // Handle all things in sector.
    for (thing = sec->thinglist ; thing ; thing = thing->snext)
    {
        if((thing->flags2 & MF2_DONTDRAW)==0)
            R_ProjectSprite (thing);
    }
}


const int PSpriteSY[NUMWEAPONS] =
{
     0,             // staff
     5*FRACUNIT,    // goldwand
    15*FRACUNIT,    // crossbow
    15*FRACUNIT,    // blaster
    15*FRACUNIT,    // skullrod
    15*FRACUNIT,    // phoenix rod
    15*FRACUNIT,    // mace
    15*FRACUNIT,    // gauntlets
    15*FRACUNIT     // beak
};

//
// R_DrawPSprite, Draw one player sprite.
//
// Draw parts of the viewplayer weapon
void R_DrawPSprite (pspdef_t* psp)
{
    fixed_t             tx;
    int                 x1, x2, fr;
    spritedef_t*        sprdef;
//    spriteframe_t*      sprframe;
    sprite_frot_t *     sprfrot;
    spritelump_t*       sprlump;
    vissprite_t*        vis;
    vissprite_t         avis;

    // [WDJ] 11/14/2012 use viewer variables, which will be for viewplayer

    // decide which patch to use
#ifdef RANGECHECK
    if ( (unsigned)psp->state->sprite >= numsprites) {
        // [WDJ] Give msg and don't draw it, (** Heretic **)
        I_SoftError ("R_DrawPSprite: invalid sprite number %i\n",
                 psp->state->sprite);
        return;
    }
#endif

    sprdef = &sprites[psp->state->sprite];

#ifdef RANGECHECK
    if ( (psp->state->frame & FF_FRAMEMASK)  >= sprdef->numframes) {
        // [WDJ] Give msg and don't draw it
        I_SoftError ("R_DrawPSprite: invalid sprite frame %i : %i for %s\n",
                 psp->state->sprite, psp->state->frame, sprnames[psp->state->sprite]);
        return;
    }
#endif
   
    // [WDJ] segfault control in heretic shareware, not all sprites present
    if( (byte*)sprdef->spriteframe < (byte*)0x1000 )
    {
        I_SoftError("R_DrawPSprite: sprframe ptr NULL for state %d\n", psp->state );
        return;
    }

    fr = psp->state->frame & FF_FRAMEMASK;
//    sprframe = get_spriteframe( sprdef, fr );

    // use single rotation for all views
    sprfrot = get_framerotation( sprdef, fr, 0 );
   
    //Fab: see the notes in R_ProjectSprite about spritelump_id,lumppat
    sprlump = &spritelumps[sprfrot->spritelump_id];

    // calculate edges of the shape

    //added:08-01-98:replaced mul by shift
    tx = psp->sx-((BASEVIDWIDTH/2)<<FRACBITS); //*FRACUNITS);

    //added:02-02-98:spriteoffset should be abs coords for psprites, based on
    //               320x200
#if 0
    // [WDJ] I don't think that weapon sprites need flip, but prboom
    // and prboom-plus are still supporting it, so maybe there are some.
    // There being one viewpoint per offset, probably do not need this.
    if( sprfrot->flip )
    {
        // debug_Printf("Player weapon flip detected!\n" );
        tx -= sprlump->width - sprlump->offset;  // Fraggle's flip offset
    }
    else
    {
        // apply offset from sprite lump normally
        tx -= sprlump->offset;
    }
#else
    tx -= sprlump->offset;
#endif
    x1 = (centerxfrac + FixedMul (tx,pspritescale) ) >>FRACBITS;

    // off the right side
    if (x1 > rdraw_viewwidth)
        return;

    tx += sprlump->width;
    x2 = ((centerxfrac + FixedMul (tx, pspritescale) ) >>FRACBITS) - 1;

    // off the left side
    if (x2 < 0)
        return;

    // store information in a vissprite
    vis = &avis;
    vis->mobjflags = 0;
    vis->texturemid = (cv_splitscreen.EV) ?
        (120<<(FRACBITS)) + FRACUNIT/2 - (psp->sy - sprlump->topoffset)
        : (BASEYCENTER<<FRACBITS) + FRACUNIT/2 - (psp->sy - sprlump->topoffset);

    if( EN_heretic_hexen )
    {
        if( rdraw_viewheight == vid.height
            || (!cv_scalestatusbar.EV && vid.dupy>1) )
            vis->texturemid -= PSpriteSY[viewplayer->readyweapon];
    }

    //vis->texturemid += FRACUNIT/2;

    vis->x1 = (x1 < 0) ? 0 : x1;
    vis->x2 = (x2 >= rdraw_viewwidth) ? rdraw_viewwidth-1 : x2;
    vis->scale = pspriteyscale;  //<<detailshift;

    if( sprfrot->flip )
    {
        vis->xiscale = -pspriteiscale;
        vis->startfrac = sprlump->width - 1;
    }
    else
    {
        vis->xiscale = pspriteiscale;
        vis->startfrac = 0;
    }

    if (vis->x1 > x1)
        vis->startfrac += vis->xiscale*(vis->x1-x1);

    //Fab: see above for more about spritelump_id,lumppat
    vis->patch = sprfrot->pat_lumpnum;
    vis->translucentmap = NULL;
    vis->translucent_index = 0;
    if (viewplayer->mo->flags & MF_SHADOW)      // invisibility effect
    {
        vis->colormap = NULL;   // use translucency

        // in Doom2, it used to switch between invis/opaque the last seconds
        // now it switch between invis/less invis the last seconds
        if (viewplayer->powers[pw_invisibility] > 4*TICRATE
                 || viewplayer->powers[pw_invisibility] & 8)
        {
            vis->translucent_index = TRANSLU_hi;
            vis->translucentmap = & translucenttables[ TRANSLU_TABLE_hi ];
        }
        else
        {
            vis->translucent_index = TRANSLU_med;
            vis->translucentmap = & translucenttables[ TRANSLU_TABLE_med ];
        }
    }
    else if (fixedcolormap)
    {
        // fixed color
        vis->colormap = fixedcolormap;
    }
    else if (psp->state->frame & FF_FULLBRIGHT)
    {
        // full bright
        vis->colormap = & reg_colormaps[0]; // [0]
    }
    else
    {
        // local light
        vis->colormap = spritelights[MAXLIGHTSCALE-1];
    }

    if(viewer_sector->numlights)
    {
      lightlev_t  vlight;  // 0..255
      ff_light_t * ff_light =
        R_GetPlaneLight(viewer_sector, viewmobj->z + (41 << FRACBITS));
      vis->extra_colormap = ff_light->extra_colormap;
      vlight = *ff_light->lightlevel + extralight;

      spritelights =
          (vlight < 0) ? scalelight[0]
        : (vlight >= 255) ? scalelight[LIGHTLEVELS-1]
        : scalelight[vlight>>LIGHTSEGSHIFT];

      vis->colormap = spritelights[MAXLIGHTSCALE-1];
    }
    else
      vis->extra_colormap = viewer_sector->extra_colormap;

    R_DrawVisSprite (vis, vis->x1, vis->x2);
}



//
// R_DrawPlayerSprites
//
// Draw the viewplayer weapon, render_soft.
void R_DrawPlayerSprites (void)
{
    int         i = 0;
    lightlev_t  vlight;  // visible light 0..255
    pspdef_t*   psp;

    int kikhak;

    // rendermode == render_soft
    // [WDJ] 11/14/2012 use viewer variables for viewplayer

    // get light level
    if(viewer_sector->numlights)
    {
      ff_light_t * ff_light =
        R_GetPlaneLight(viewer_sector, viewmobj->z + viewmobj->info->height);
      vlight = *ff_light->lightlevel + extralight;
    }
    else
      vlight = viewer_sector->lightlevel + extralight;

    spritelights =
        (vlight < 0) ? scalelight[0]
      : (vlight >= 255) ? scalelight[LIGHTLEVELS-1]
      : scalelight[vlight>>LIGHTSEGSHIFT];

    // clip to screen bounds
    dm_floorclip = screenheightarray;
    dm_ceilingclip = negonearray;

    //added:06-02-98: quickie fix for psprite pos because of freelook
    kikhak = centery;
    centery = centerypsp;             //for R_DrawColumn
    centeryfrac = centery<<FRACBITS;  //for R_DrawVisSprite

    // add all active psprites
    for (i=0, psp=viewplayer->psprites;
         i<NUMPSPRITES;
         i++,psp++)
    {
        if (psp->state)
            R_DrawPSprite (psp);
    }

    //added:06-02-98: oooo dirty boy
    centery = kikhak;
    centeryfrac = centery<<FRACBITS;
}



// R_Create_DrawNodes
// Creates and sorts a list of drawnodes for the scene being rendered.
static void           R_Create_DrawNodes();
static drawnode_t*    R_CreateDrawNode (drawnode_t* link);

static drawnode_t     nodebankhead;
static drawnode_t     nodehead;

// called by R_DrawMasked
static void R_Create_DrawNodes( void )
{
  drawnode_t*   entry;
  drawseg_t*    ds;
  vissprite_t*  vsp;  // rover vissprite
  drawnode_t*   dnp;  // rover drawnode
  visplane_t*   plane;
  int           i, p, x1, x2;
  fixed_t       delta;
  int           sintersect;
//  fixed_t       gzm;
  fixed_t       pz_mid; // mid of sprite
  fixed_t       scale;

    // Add the 3D floors, thicksides, and masked textures...
    for(ds = ds_p; ds-- > drawsegs;)
    {
      if(ds->numthicksides)
      {
        for(i = 0; i < ds->numthicksides; i++)
        {
          entry = R_CreateDrawNode(&nodehead);
          entry->thickseg = ds;
          entry->ffloor = ds->thicksides[i];
        }
      }
      if(ds->maskedtexturecol)
      {
        entry = R_CreateDrawNode(&nodehead);
        entry->seg = ds;
      }
      if(ds->numffloorplanes)
      {
        // create drawnodes for the floorplanes with the closest last
        // [WDJ] Sort as they are put into the list.  This avoids repeating
        // searching through the same entries, PlaneBounds() and tests.
        drawnode_t * first_dnp = nodehead.prev; // previous last drawnode
        for(p = 0; p < ds->numffloorplanes; p++)
        {
            if(!ds->ffloorplanes[p])  // ignore NULL
              continue;
            plane = ds->ffloorplanes[p];
            ds->ffloorplanes[p] = NULL;  // remove from floorplanes
            R_PlaneBounds(plane);  // set highest_top, lowest_bottom
                 // in screen coord, where 0 is top (hi)
            if(plane->lowest_bottom < con_clipviewtop
               || plane->highest_top > rdraw_viewheight  // [WDJ] rdraw window, not vid.height
               || plane->highest_top > plane->lowest_bottom)
            {
              continue;  // not visible, next plane
            }
            delta = abs(plane->height - viewz); // new entry distance
            // merge sort into the drawnode list
            dnp = first_dnp->next; // first plane, or nodehead
            while( dnp != &nodehead )  // until reach end of new entries
            {
                // test for plane closer
                // everything between first_dnp and nodehead must be plane
                if(abs(dnp->plane->height - viewz) < delta)
                    break; // is farther than dnp
                dnp = dnp->next; // towards closer, towards nodehead
            }
            // create new drawnode
            entry = R_CreateDrawNode(dnp); // before closer entry, or nodehead
            entry->plane = plane;
            entry->seg = ds;
        }
      }
    }

    if(vissprite_p == vissprites)  // empty sprite list
      return;

    // sprite list is sorted from vprsortedhead    
    // traverse vissprite sorted list, nearest to farthest
    for(vsp = vsprsortedhead.prev; vsp != &vsprsortedhead; vsp = vsp->prev)
    {
      if(vsp->sz_top > vid.height || vsp->sz_bot < 0)
        continue;

      sintersect = (vsp->x1 + vsp->x2) / 2;
//      gzm = (vsp->gz_bot + vsp->gz_top) / 2;
      pz_mid = (vsp->pz_bot + vsp->pz_top) / 2;

      // search drawnodes
      // drawnodes are in bsp order, partially sorted
      for(dnp = nodehead.next; dnp != &nodehead; dnp = dnp->next)
      {
        if(dnp->plane)
        {
          // sprite vrs floor plane
          if(dnp->plane->minx > vsp->x2 || dnp->plane->maxx < vsp->x1)
            continue;  // next dnp
          if(vsp->sz_top > dnp->plane->lowest_bottom
             || vsp->sz_bot < dnp->plane->highest_top)
            continue;  // next dnp

          // [WDJ] test mid of sprite instead of toe and head
          // to avoid whole sprite affected by a marginal overlap
          if( dnp->plane->height < viewz )
          {
              // floor
              if( dnp->plane->height < pz_mid )  continue;  // sprite over floor
          }
          else
          {
              // ceiling
              if( dnp->plane->height > pz_mid )  continue;  // sprite under ceiling
          }
          {
            // SoM: NOTE: Because a visplane's shape and scale is not directly
            // bound to any single linedef, a simple poll of it's scale is
            // not adequate. We must check the entire scale array for any
            // part that is in front of the sprite.

            x1 = vsp->x1;
            x2 = vsp->x2;
            if(x1 < dnp->plane->minx) x1 = dnp->plane->minx;
            if(x2 > dnp->plane->maxx) x2 = dnp->plane->maxx;

            for(i = x1; i <= x2; i++)
            {
              // keeps sprite from being seen through floors
              if(dnp->seg->backscale[i] > vsp->scale)
              {
                  // this plane needs to be drawn after sprite
                  goto  dnp_closer_break;
              }
            }
            continue;  // next dnp
          }
        }
        else if(dnp->thickseg)
        {
          // sprite vrs 3d thickseg
          if(vsp->x1 > dnp->thickseg->x2 || vsp->x2 < dnp->thickseg->x1)
            continue;  // next dnp

          // max of scale1, scale2 (which is closest)
          scale = (dnp->thickseg->scale1 > dnp->thickseg->scale2) ? dnp->thickseg->scale1 : dnp->thickseg->scale2;
          if(scale <= vsp->scale)
            continue;  // next dnp
          scale = dnp->thickseg->scale1 + (dnp->thickseg->scalestep * (sintersect - dnp->thickseg->x1));
          if(scale <= vsp->scale)
            continue;  // next dnp

          if((*dnp->ffloor->topheight > viewz
                 && *dnp->ffloor->bottomheight < viewz)
             || (*dnp->ffloor->topheight < viewz
                 && vsp->gz_top < *dnp->ffloor->topheight)
             || (*dnp->ffloor->bottomheight > viewz
                 && vsp->gz_bot > *dnp->ffloor->bottomheight))
          {
            // thickseg is closer, must be drawn after sprite
            goto  dnp_closer_break;
          }
        }
        else if(dnp->seg)
        {
          // sprite vrs seg
          if(vsp->x1 > dnp->seg->x2 || vsp->x2 < dnp->seg->x1)
            continue;  // next dnp

          scale = dnp->seg->scale1 > dnp->seg->scale2 ? dnp->seg->scale1 : dnp->seg->scale2;
          if(scale <= vsp->scale)
            continue;  // next dnp
          scale = dnp->seg->scale1 + (dnp->seg->scalestep * (sintersect - dnp->seg->x1));

          if(vsp->scale < scale)
          {
            goto  dnp_closer_break;
          }
        }
        else if(dnp->sprite)
        {
          // sprite vrs sprite
          if(dnp->sprite->x1 > vsp->x2 || dnp->sprite->x2 < vsp->x1)
            continue;  // next dnp
          if(dnp->sprite->sz_top > vsp->sz_bot || dnp->sprite->sz_bot < vsp->sz_top)
            continue;  // next dnp

          if(dnp->sprite->scale > vsp->scale)
          {
            goto  dnp_closer_break;
          }
        }
        continue;  // next dnp
      }
      // end of dnp
      if(dnp == &nodehead)
      {
        // end of list, draw in front of everything else
        entry = R_CreateDrawNode(&nodehead);
        entry->sprite = vsp;
      }
      continue; // next vsp

    dnp_closer_break:
      // enter sprite after dnp
      entry = R_CreateDrawNode(NULL);
      (entry->prev = dnp->prev)->next = entry;
      (entry->next = dnp)->prev = entry;
      entry->sprite = vsp;
    }  // for vsp
}



// called by R_Create_DrawNodes
static drawnode_t* R_CreateDrawNode (drawnode_t* link)
{
  drawnode_t* node;

  node = nodebankhead.next;
  if(node == &nodebankhead)
  {
    node = malloc(sizeof(drawnode_t));
  }
  else
    (nodebankhead.next = node->next)->prev = &nodebankhead;

  if(link)
  {
    node->next = link;
    node->prev = link->prev;
    link->prev->next = node;
    link->prev = node;
  }

  node->plane = NULL;
  node->seg = NULL;
  node->thickseg = NULL;
  node->ffloor = NULL;
  node->sprite = NULL;
  return node;
}



static void R_DoneWithNode(drawnode_t* node)
{
  (node->next->prev = node->prev)->next = node->next;
  (node->next = nodebankhead.next)->prev = node;
  (node->prev = &nodebankhead)->next = node;
}



static void R_Clear_DrawNodes()
{
  drawnode_t* dnp; // rover drawnode
  drawnode_t* next;

  for(dnp = nodehead.next; dnp != &nodehead; )
  {
    next = dnp->next;
    R_DoneWithNode(dnp);
    dnp = next;
  }

  nodehead.next = nodehead.prev = &nodehead;
}



void R_Init_DrawNodes()
{
  nodebankhead.next = nodebankhead.prev = &nodebankhead;
  nodehead.next = nodehead.prev = &nodehead;
}



//
// R_DrawSprite
//
//Fab:26-04-98:
// NOTE : uses con_clipviewtop, so that when console is on,
//        don't draw the part of sprites hidden under the console
void R_DrawSprite (vissprite_t* spr)
{
    drawseg_t*          ds;
    short               clipbot[MAXVIDWIDTH];
    short               cliptop[MAXVIDWIDTH];
    int                 x;
    int                 r1;
    int                 r2;
    fixed_t             scale;
    fixed_t             lowscale;
    int                 silhouette;

    for (x = spr->x1 ; x<=spr->x2 ; x++)
        clipbot[x] = cliptop[x] = -2;

    // Scan drawsegs from end to start for obscuring segs.
    // The first drawseg that has a greater scale is the clip seg.
    //SoM: 4/8/2000:
    // Pointer check was originally nonportable
    // and buggy, by going past LEFT end of array:

    //    for (ds=ds_p-1 ; ds >= drawsegs ; ds--)    old buggy code
    for (ds=ds_p ; ds-- > drawsegs ; )
    {

        // determine if the drawseg obscures the sprite
        if (ds->x1 > spr->x2
         || ds->x2 < spr->x1
         || (!ds->silhouette
             && !ds->maskedtexturecol) )
        {
            // does not cover sprite
            continue;
        }

        // r1..r2 where drawseg overlaps sprite (intersect)
        r1 = ds->x1 < spr->x1 ? spr->x1 : ds->x1;  // max x1
        r2 = ds->x2 > spr->x2 ? spr->x2 : ds->x2;  // min x2

        // (lowscale,scale) = minmax( ds->scale1, ds->scale2 )
        if (ds->scale1 > ds->scale2)
        {
            lowscale = ds->scale2;
            scale = ds->scale1;
        }
        else
        {
            lowscale = ds->scale1;
            scale = ds->scale2;
        }

        if (scale < spr->scale
            || ( lowscale < spr->scale
                 && !R_PointOnSegSide (spr->gx, spr->gy, ds->curline) ) )
        {
            // masked mid texture?
            /*if (ds->maskedtexturecol)
                R_RenderMaskedSegRange (ds, r1, r2);*/
            // seg is behind sprite
            continue;  // next drawseg
        }

        // clip this piece of the sprite
        silhouette = ds->silhouette;

        // check sprite bottom above clip height
        if (spr->gz_bot >= ds->sil_bottom_height)
            silhouette &= ~SIL_BOTTOM;

        // check sprite top above clip height
        if (spr->gz_top <= ds->sil_top_height)
            silhouette &= ~SIL_TOP;

        if (silhouette == SIL_BOTTOM)
        {
            // bottom sil
            for (x=r1 ; x<=r2 ; x++)
                if (clipbot[x] == -2)
                    clipbot[x] = ds->spr_bottomclip[x];
        }
        else if (silhouette == SIL_TOP)
        {
            // top sil
            for (x=r1 ; x<=r2 ; x++)
                if (cliptop[x] == -2)
                    cliptop[x] = ds->spr_topclip[x];
        }
        else if (silhouette == (SIL_BOTTOM|SIL_TOP))
        {
            // both
            for (x=r1 ; x<=r2 ; x++)
            {
                if (clipbot[x] == -2)
                    clipbot[x] = ds->spr_bottomclip[x];
                if (cliptop[x] == -2)
                    cliptop[x] = ds->spr_topclip[x];
            }
        }
    }
    //SoM: 3/17/2000: Clip sprites in water.
    if (spr->heightsec != -1)  // only things in specially marked sectors
    {
        fixed_t h,mh;
        // model sector for special sector clipping
        sector_t * spr_heightsecp = & sectors[spr->heightsec];

        // beware, this test does two assigns to mh, and an assign to h
        if ((mh = spr_heightsecp->floorheight) > spr->gz_bot
            && (h = centeryfrac - FixedMul(mh-=viewz, spr->scale)) >= 0
            && (h >>= FRACBITS) < rdraw_viewheight)
        {
            if (mh <= 0 || (viewer_has_model && !viewer_underwater))
            {                          // clip bottom
              for (x=spr->x1 ; x<=spr->x2 ; x++)
                if (clipbot[x] == -2 || h < clipbot[x])
                  clipbot[x] = h;
            }
            else                        // clip top
            {
              for (x=spr->x1 ; x<=spr->x2 ; x++)
                if (cliptop[x] == -2 || h > cliptop[x])
                  cliptop[x] = h;
            }
        }

        // beware, this test does an assign to mh, and an assign to h
        if ((mh = spr_heightsecp->ceilingheight) < spr->gz_top
            && (h = centeryfrac - FixedMul(mh-viewz, spr->scale)) >= 0
            && (h >>= FRACBITS) < rdraw_viewheight)
        {
            if (viewer_overceiling)
            {                         // clip bottom
              for (x=spr->x1 ; x<=spr->x2 ; x++)
                if (clipbot[x] == -2 || h < clipbot[x])
                  clipbot[x] = h;
            }
            else                       // clip top
            {
              for (x=spr->x1 ; x<=spr->x2 ; x++)
                if (cliptop[x] == -2 || h > cliptop[x])
                  cliptop[x] = h;
            }
        }
    }
    if(spr->cut & SC_TOP && spr->cut & SC_BOTTOM)
    {
      fixed_t   h;
      for(x = spr->x1; x <= spr->x2; x++)
      {
        h = spr->sz_top;
        if(cliptop[x] == -2 || h > cliptop[x])
          cliptop[x] = h;

        h = spr->sz_bot;
        if(clipbot[x] == -2 || h < clipbot[x])
          clipbot[x] = h;
#if 0
        // brute fix to status bar clipping, until better fix (found R_SplitSprite)
        if ( rdraw_viewheight < clipbot[x] )	// [WDJ] brute temp fix
            clipbot[x] = rdraw_viewheight;
#endif
      }
    }
    else if(spr->cut & SC_TOP)
    {
      fixed_t   h;
      for(x = spr->x1; x <= spr->x2; x++)
      {
        h = spr->sz_top;
        if(cliptop[x] == -2 || h > cliptop[x])
          cliptop[x] = h;
      }
    }
    else if(spr->cut & SC_BOTTOM)
    {
      fixed_t   h;
      for(x = spr->x1; x <= spr->x2; x++)
      {
        h = spr->sz_bot;
        if(clipbot[x] == -2 || h < clipbot[x])
          clipbot[x] = h;
      }
    }
    
    // all clipping has been performed, so draw the sprite

    // check for unclipped columns
    for (x = spr->x1 ; x<=spr->x2 ; x++)
    {
        if (clipbot[x] == -2)
            clipbot[x] = rdraw_viewheight;

        if (cliptop[x] == -2)
            //Fab:26-04-98: was -1, now clips against console bottom
            cliptop[x] = con_clipviewtop;
    }

    dm_floorclip = clipbot;
    dm_ceilingclip = cliptop;
    R_DrawVisSprite (spr, spr->x1, spr->x2);
}


//
// R_DrawMasked
//
void R_DrawMasked (void)
{
    drawnode_t*           r2;
    drawnode_t*           next;

    R_Create_DrawNodes();

    for(r2 = nodehead.next; r2 != &nodehead; r2 = r2->next)
    {
      if(r2->plane)
      {
        next = r2->prev;
        R_DrawSinglePlane(r2->plane);
        R_DoneWithNode(r2);
        r2 = next;
      }
      else if(r2->seg && r2->seg->maskedtexturecol != NULL)
      {
        next = r2->prev;
        R_RenderMaskedSegRange(r2->seg, r2->seg->x1, r2->seg->x2);
        r2->seg->maskedtexturecol = NULL;
        R_DoneWithNode(r2);
        r2 = next;
      }
      else if(r2->thickseg)
      {
        next = r2->prev;
        R_RenderThickSideRange(r2->thickseg, r2->thickseg->x1, r2->thickseg->x2, r2->ffloor);
        R_DoneWithNode(r2);
        r2 = next;
      }
      else if(r2->sprite)
      {
        next = r2->prev;
        R_DrawSprite(r2->sprite);
        R_DoneWithNode(r2);
        r2 = next;
      }
    }
    R_Clear_DrawNodes();
}





// ==========================================================================
//
//                              SKINS CODE
//
// ==========================================================================

// This does not deallocate the skins memory.
#define SKIN_ALLOC   8
int         numskins = 0;
skin_t *    skins[MAXSKINS+1];
skin_t *    skin_free = NULL;
skin_t      marine;


static
int  get_skin_slot(void)
{
    skin_t * sk;
    int si, i;

    // Find unused skin slot, or add one.
    for(si=0; si<numskins; si++ )
    {
        if( skins[si] == NULL )  break;
    }
    if( si >= MAXSKINS )  goto none;

    // Get skin alloc.
    if( skin_free == NULL )
    {
        i = SKIN_ALLOC;
        sk = (skin_t*) malloc( sizeof(skin_t) * SKIN_ALLOC );
        if( sk == NULL )   goto none;
        // Link to free list
        while( i-- )
        {
            *(skin_t**)sk = skin_free;  // link
            skin_free = sk++;
        }
    }

    sk = skin_free;
    skin_free = *(skin_t**)sk;  // unlink
    skins[si] = sk;
    if( si >= numskins )  numskins = si+1;
    return si;

none:
    return 0xFFFF;
}

static
void  free_skin( int skin_num )
{
    skin_t * sk;
    
    if( skin_num >= MAXSKINS )  return;
    sk = skins[skin_num];
    if( sk == NULL )  return;

    skins[skin_num] = NULL;
    *(skin_t**)sk = skin_free;  // Link into free list
    skin_free = sk;

    while( numskins>0 && (skins[numskins-1] == NULL) )
    {
        numskins --;
    }
    // Cannot move existing skins
}

static
void Skin_SetDefaultValue(skin_t *skin)
{
    int   i;

    // setup the 'marine' as default skin
    memset (skin, 0, sizeof(skin_t));
    strcpy (skin->name, DEFAULTSKIN);
    strcpy (skin->faceprefix, "STF");
    for (i=0;i<sfx_freeslot0;i++)
    {
        if (S_sfx[i].skinsound!=-1)
        {
            skin->soundsid[S_sfx[i].skinsound] = i;
        }
    }
//    memcpy(&skin->spritedef, &sprites[SPR_PLAY], sizeof(spritedef_t));
}

//
// Initialize the basic skins
//
void R_Init_Skins (void)
{
    skin_free = NULL;

    memset (skins, 0, sizeof(skins));
   
    // initialize free sfx slots for skin sounds
    S_InitRuntimeSounds ();

    // make the standard Doom2 marine as the default skin
    // skin[0] = marine skin
    skins[0] = & marine;
    Skin_SetDefaultValue( & marine );
    memcpy(&marine.spritedef, &sprites[SPR_PLAY], sizeof(spritedef_t));
    numskins = 1;
}

// Returns the skin index if the skin name is found (loaded from pwad).
// Return 0 (the default skin) if not found.
int R_SkinAvailable (const char* name)
{
    int  i;

    for (i=0;i<numskins;i++)
    {
        if( skins[i] && strcasecmp(skins[i]->name, name)==0)
            return i;
    }
    return 0;
}


void SetPlayerSkin_by_index( player_t * player, int index )
{
    skin_t * sk;

    if( index >= numskins )   goto default_skin;
   
    sk = skins[index];
    if( sk == NULL )   goto default_skin;
    
    // Change the face graphics
    if( player == &players[statusbarplayer]
        // for save time test it there is a real change
        && !( skins[player->skin] && strcmp (skins[player->skin]->faceprefix, sk->faceprefix)==0 )
        )
    {
        ST_Release_FaceGraphics();
        ST_Load_FaceGraphics(sk->faceprefix);
    }

set_skin:
    // Record the player skin.
    player->skin = index;
 
    // A copy of the skin value so that dead body detached from
    // respawning player keeps the skin
    if( player->mo )
        player->mo->skin = sk;
    return;

default_skin:
    index = 0;  // the old marine skin
    sk = &marine;
    goto set_skin;
}


// network code calls this when a 'skin change' is received
void  SetPlayerSkin (int playernum, const char *skinname)
{
    int   i;

    for(i=0;i<numskins;i++)
    {
        // search in the skin list
        if( skins[i] && strcasecmp(skins[i]->name,skinname)==0)
        {
            SetPlayerSkin_by_index( &players[playernum], i );
            return;
        }
    }

    GenPrintf(EMSG_warn, "Skin %s not found\n", skinname);
    // not found put the old marine skin
    SetPlayerSkin_by_index( &players[playernum], 0 );
}


//
// Add skins from a pwad, each skin preceded by 'S_SKIN' marker
//

// Does the same as in w_wad, but check only for
// the first 6 characters (this is so we can have S_SKIN1, S_SKIN2..
// for wad editors that don't like multiple resources of the same name)
//
int W_CheckForSkinMarkerInPwad (int wadid, int startlump)
{
    lump_name_t name8;
    uint64_t mask6;  // big endian, little endian
    int         i;
    lumpinfo_t* lump_p;

    name8.namecode = -1; // make 6 char mask
    name8.s[6] = name8.s[7] = 0;
    mask6 = name8.namecode;

    numerical_name( "S_SKIN", & name8 );  // fast compares

    // scan forward, start at <startlump>
    if (startlump < wadfiles[wadid]->numlumps)
    {
        lump_p = wadfiles[wadid]->lumpinfo + startlump;
        for (i = startlump; i<wadfiles[wadid]->numlumps; i++,lump_p++)
        {
            // Only check first 6 characters.
            if( (*(uint64_t *)lump_p->name & mask6) == name8.namecode )
            {
                return WADLUMP(wadid,i);
            }
        }
    }
    return -1; // not found
}

//
// Find skin sprites, sounds & optional status bar face, & add them
//
void R_AddSkins (int wadnum)
{
    int         lumpnum, lastlump, lumpn;

    lumpinfo_t* lumpinfo;
    char*       sprname;
    uint32_t    numname;

    char*       buf;
    char*       buf2;

    char*       token;
    char*       value;
   
    skin_t *    sk;
    int         skin_index;

    int         i,size;

    //
    // search for all skin markers in pwad
    //

    lastlump = 0;
    for(;;)
    {
        sprname = NULL;

        lumpnum = W_CheckForSkinMarkerInPwad (wadnum, lastlump);
        if( lumpnum == -1 )  break;

        lumpn = LUMPNUM(lumpnum);
        lastlump = lumpn + 1;  // prevent repeating same skin

        skin_index = get_skin_slot();
        if( skin_index > MAXSKINS )
        {
            GenPrintf(EMSG_warn, "ignored skin lump %d (%d skins maximum)\n", lumpn, MAXSKINS);
            continue; //faB:so we know how many skins couldn't be added
        }
        sk = skins[skin_index];

        // set defaults
        Skin_SetDefaultValue(sk);
        sprintf (sk->name,"skin %d", numskins-1);

        buf  = W_CacheLumpNum (lumpnum, PU_CACHE);
        size = W_LumpLength (lumpnum);

        // for strtok
        buf2 = (char *) malloc (size+1);
        if(!buf2)
        {
             I_SoftError("R_AddSkins: No more free memory\n");
             goto skin_error;
        }
        memcpy (buf2,buf,size);
        buf2[size] = '\0';

        // parse
        token = strtok (buf2, "\r\n= ");
        while (token)
        {
            if(token[0]=='/' && token[1]=='/') // skip comments
            {
                token = strtok (NULL, "\r\n"); // skip end of line
                goto next_token;               // find the real next token
            }

            value = strtok (NULL, "\r\n= ");
//            CONS_Printf("token = %s, value = %s",token,value);
//            CONS_Error("ga");

            if (!value)
            {
                I_SoftError("R_AddSkins: syntax error in S_SKIN lump# %d in WAD %s\n", lumpn, wadfiles[wadnum]->filename);
                goto skin_error;
            }

            if (!strcasecmp(token,"name"))
            {
                // the skin name must uniquely identify a single skin
                // I'm lazy so if name is already used I leave the 'skin x'
                // default skin name set above
                if (!R_SkinAvailable (value))
                {
                    strncpy (sk->name, value, SKINNAMESIZE);
                    strlwr (sk->name);
                }
            }
            else
            if (!strcasecmp(token,"face"))
            {
                strncpy (sk->faceprefix, value, 3);
                sk->faceprefix[3] = 0;
                strupr (sk->faceprefix);
            }
            else
            if (!strcasecmp(token,"sprite"))
            {
                sprname = value;
                strupr(sprname);
            }
            else
            {
                int found=false;
                // copy name of sounds that are remapped for this skin
                for (i=0;i<sfx_freeslot0;i++)
                {
                    if (!S_sfx[i].name)
                      continue;
                    if (S_sfx[i].skinsound!=-1 &&
                        !strcasecmp(S_sfx[i].name, token+2) )
                    {
                        sk->soundsid[S_sfx[i].skinsound]=
                            S_AddSoundFx(value+2, S_sfx[i].flags);
                        found=true;
                    }
                }
                if(!found)
                {
                    I_SoftError("R_AddSkins: Unknown keyword '%s' in S_SKIN lump# %d (WAD %s)\n",
                               token, lumpn, wadfiles[wadnum]->filename);
                    goto skin_error;
                }
            }
next_token:
            token = strtok (NULL,"\r\n= ");
        }

        // if no sprite defined use sprite just after this one
        if( !sprname )
        {
            lumpn++;
            lumpinfo = wadfiles[wadnum]->lumpinfo;

            // get the base name of this skin's sprite (4 chars)
            sprname = lumpinfo[lumpn].name;
            numname = *(uint32_t *)sprname;

            // skip to end of this skin's frames
            lastlump = lumpn;
            while (*(uint32_t *)lumpinfo[lastlump].name == numname)
                lastlump++;
            // allocate (or replace) sprite frames, and set spritedef
            R_AddSingleSpriteDef (sprname, &sk->spritedef, wadnum, lumpn, lastlump);
        }
        else
        {
            // search in the normal sprite tables
            char **name;
            boolean found = false;
            for(name = sprnames;*name;name++)
            {
                if( strcmp(*name, sprname) == 0 )
                {
                    found = true;
                    sk->spritedef = sprites[sprnames-name];
                }
            }

            // not found so make a new one
            if( !found )
                R_AddSingleSpriteDef (sprname, &sk->spritedef, wadnum, 0, MAXINT);

        }

        CONS_Printf ("added skin '%s'\n", sk->name);

        free(buf2);
    }
    return;
   
skin_error:
    free_skin(skin_index);	       
    return;
}
