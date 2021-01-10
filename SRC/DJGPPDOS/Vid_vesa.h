// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: Vid_vesa.h 538 2009-09-23 23:24:07Z smite-meister $
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
// $Log: Vid_vesa.h,v $
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      VESA extra modes.
//      
//-----------------------------------------------------------------------------


#include "../doomdef.h"
#include "../screen.h"



#define MODE_SUPPORTED_IN_HW    0x0001
#define COLOR_MODE              0x0008
#define GRAPHICS_MODE           0x0010
#define VGA_INCOMPATIBLE        0x0020
#define LINEAR_FRAME_BUFFER     0x0080
#define LINEAR_MODE             0x4000

#define MAX_VESA_MODES          30  // we'll just take the first 30 if there


// VESA information block structure
typedef struct vbeinfoblock_s
{
    unsigned char  VESASignature[4];
    unsigned short VESAVersion        __attribute__ ((packed));
    unsigned long  OemStringPtr       __attribute__ ((packed));
    byte    Capabilities[4];
    unsigned long  VideoModePtr       __attribute__ ((packed));
    unsigned short TotalMemory        __attribute__ ((packed));
    byte    OemSoftwareRev[2];
    byte    OemVendorNamePtr[4];
    byte    OemProductNamePtr[4];
    byte    OemProductRevPtr[4];
    byte    Reserved[222];
    byte    OemData[256];
} vbeinfoblock_t;


// VESA information for a specific mode
typedef struct vesamodeinfo_s
{
   unsigned short ModeAttributes       __attribute__ ((packed));
   unsigned char  WinAAttributes;
   unsigned char  WinBAttributes;
   unsigned short WinGranularity       __attribute__ ((packed));
   unsigned short WinSize              __attribute__ ((packed));
   unsigned short WinASegment          __attribute__ ((packed));
   unsigned short WinBSegment          __attribute__ ((packed));
   unsigned long  WinFuncPtr           __attribute__ ((packed));
   unsigned short BytesPerScanLine     __attribute__ ((packed));
   unsigned short XResolution          __attribute__ ((packed));
   unsigned short YResolution          __attribute__ ((packed));
   unsigned char  XCharSize;
   unsigned char  YCharSize;
   unsigned char  NumberOfPlanes;
   unsigned char  BitsPerPixel;
   unsigned char  NumberOfBanks;
   unsigned char  MemoryModel;
   unsigned char  BankSize;
   unsigned char  NumberOfImagePages;
   unsigned char  Reserved_page;
   unsigned char  RedMaskSize;
   unsigned char  RedMaskPos;
   unsigned char  GreenMaskSize;
   unsigned char  GreenMaskPos;
   unsigned char  BlueMaskSize;
   unsigned char  BlueMaskPos;
   unsigned char  ReservedMaskSize;
   unsigned char  ReservedMaskPos;
   unsigned char  DirectColorModeInfo;

   /* VBE 2.0 extensions */
   unsigned long  PhysBasePtr          __attribute__ ((packed));
   unsigned long  OffScreenMemOffset   __attribute__ ((packed));
   unsigned short OffScreenMemSize     __attribute__ ((packed));

   /* VBE 3.0 extensions */
   unsigned short LinBytesPerScanLine;
   unsigned char  BnkNumberOfPages;
   unsigned char  LinNumberOfPages;
   unsigned char  LinRedMaskSize;
   unsigned char  LinRedFieldPos;
   unsigned char  LinGreenMaskSize;
   unsigned char  LinGreenFieldPos;
   unsigned char  LinBlueMaskSize;
   unsigned char  LinBlueFieldPos;
   unsigned char  LinRsvdMaskSize;
   unsigned char  LinRsvdFieldPos;
   unsigned long  MaxPixelClock        __attribute__ ((packed));

   unsigned char  Reserved[190];
} vesamodeinfo_t;


// setup standard VGA + VESA modes list, activate the default video mode.
void VID_Init (void);
// setup a video mode, this is to be called from the menu
int  VID_SetMode (int modenum);
