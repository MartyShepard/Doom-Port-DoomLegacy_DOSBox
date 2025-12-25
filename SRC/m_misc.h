// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: m_misc.h 1368 2017-11-01 01:17:48Z wesleyjohnson $
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
// $Log: m_misc.h,v $
// Revision 1.3  2000/04/16 18:38:07  bpereira
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Default Config File.
//      PCX Screenshots.
//      File i/o
//      Common used routines
//    
//-----------------------------------------------------------------------------

#ifndef M_MISC_H
#define M_MISC_H

#include "doomdef.h"
  // MAX_WADPATH
#include "doomtype.h"

// the file where all game vars and settings are saved
#define CONFIGFILENAME   "config.cfg"


//
// MISC
//
//===========================================================================

boolean FIL_WriteFile ( char const*   name,
                        void*         source,
                        int           length );

int  FIL_ReadFile ( char const*   name,
                    byte**        buffer );


// Extended Read and Write of buffers.
typedef enum {
   ERR_RW= -8,   // errors are negative
   STAT_EOF=16, STAT_CLOSED, STAT_OPEN  // state
} stat_err_e;

typedef struct
{
    void*    buffer;
    uint32_t bufcnt;	// bytes in buffer (last read)
    int      handle;
    int      stat_error;     // error and status
} ExtFIL_t;

int FIL_ExtFile_Open ( ExtFIL_t * ft, char const* name, boolean write_flag );
int FIL_ExtWriteFile ( ExtFIL_t * ft, size_t length );
int FIL_ExtReadFile ( ExtFIL_t * ft, size_t length );
void FIL_ExtFile_Close ( ExtFIL_t * ft );


//  path : extension is added to path parameter
void FIL_DefaultExtension (char *path, const char *extension);

// Point to start of the filename in longer string
char * FIL_Filename_of( char * nstr );

#if 0
//added:11-01-98:now declared here for use by G_DoPlayDemo(), see there...
void FIL_ExtractFileBase (char* path, char* dest);
#endif

boolean FIL_CheckExtension (const char * in);

//===========================================================================

void M_ScreenShot (void);

//===========================================================================

extern char   configfile[MAX_WADPATH];

void Command_SaveConfig_f (void);
void Command_LoadConfig_f (void);
void Command_ChangeConfig_f (void);

void M_FirstLoadConfig(void);
//Fab:26-04-98: save game config : cvars, aliases..
void M_SaveConfig (const char *filename);

//===========================================================================


// dest must be filename buffer of MAX_WADPATH
// If directory dn does not end in '/', then a separator will be included.
void cat_filename( char * dest, const char * dn, const char * fn );

#if 0
// s1=s2+s3+s1 (1024 lenghtmax)
void strcatbf(char *s1, const char *s2, const char *s3);
#endif

#endif
