// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: hw_data.h 1422 2019-01-29 08:05:39Z wesleyjohnson $
//
// Copyright (C) 1998-2012 by DooM Legacy Team.
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
// $Log: hw_data.h,v $
// Revision 1.9  2003/06/05 20:51:36  hurdler
// remove the BOOL define
//
// Revision 1.8  2000/10/04 16:21:57  hurdler
// Revision 1.7  2000/04/30 10:30:10  bpereira
//
// Revision 1.6  2000/04/27 17:48:47  hurdler
// colormap code in hardware mode is now the default
//
// Revision 1.5  2000/04/24 15:22:47  hurdler
// Support colormap for text
//
// Revision 1.4  2000/04/22 16:09:14  hurdler
// support skin color in hardware mode
//
// Revision 1.3  2000/04/07 23:10:15  metzgermeister
// fullscreen support under X in Linux
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      defines structures and exports for the standard 3D driver DLL used by Doom Legacy
//
//-----------------------------------------------------------------------------

#ifndef HW_DATA_H
#define HW_DATA_H

#include "doomtype.h"

//THIS MUST DISAPPEAR!!!
#include "hw_glide.h"


// ==========================================================================
//                                                               TEXTURE INFO
// ==========================================================================

// grInfo.data holds the address of the graphics data cached in heap memory
//                NULL if the texture is not in Doom heap cache.
struct Mipmap_s {
    GrTexInfo       grInfo;         //for TexDownloadMipMap
    uint32_t        tfflags;	 // TF_ texture flags
    uint16_t        height;
    uint16_t        width;
    unsigned int    downloaded;  // dll driver has it in cache
                                 // opengl : texture num, must match GLuint
                                 //   typedef unsigned int GLuint

    // multiple texture renderings, by colormap and TF_Opaquetrans
    struct Mipmap_s   *nextcolormap;  // next for this texture
    byte              *colormap;

    // opengl/glide
    struct Mipmap_s*  nextmipmap;// glide  : the FIFO list of texture in the memory
                                 //          _DO NOT TOUCH IT_
                                 // opengl : list of all texture in opengl driver
    // glide only
    FxU32           cachepos;        //offset in hardware cache
    FxU32           mipmapSize;      //size of mipmap
};
typedef struct Mipmap_s Mipmap_t;


//
// Doom texture info, as cached for hardware rendering
//
typedef struct {
    Mipmap_t  mipmap;
    float     scaleX;             //used for scaling textures on walls
    float     scaleY;
} MipTexture_t;


// A cached patch as converted to hardware format, holding the original patch_t
// header so that the existing code can retrieve ->width, ->height as usual
// This is returned by W_CachePatchNum()/W_CachePatchName(), when rendermode
// is 'render_glide'. Else it returns the normal patch_t data.

// Derived from:   pat_hdr_t;
// [WDJ] This is used for reading patches from wad.
typedef struct {
    // the 4 first fields come right away from the original patch_t
    // pat_hdr_t
    uint16_t   width;          // bounding box size
    uint16_t   height;
    int16_t    leftoffset;     // pixels to the left of origin
    int16_t    topoffset;      // pixels below the origin
    // hardware patch fields
    lumpnum_t  patch_lumpnum;  // the software patch lump num for when the hardware patch
                               // was flushed, and we need to re-create it
    float      max_s, max_t;
    Mipmap_t   mipmap;
} MipPatch_t;

// [WDJ] Fixed size fog textures
#define FOG_WIDTH  256
#define FOG_HEIGHT  64

#endif // HW_DATA_H
