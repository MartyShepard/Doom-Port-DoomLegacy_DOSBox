// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: d_netfil.h 1147 2015-04-03 13:49:10Z wesleyjohnson $
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
// $Log: d_netfil.h,v $
// Revision 1.9  2001/07/28 16:18:37  bpereira
//
// Revision 1.8  2001/05/21 14:57:05  crashrl
// Readded directory crawling file search function
//
// Revision 1.7  2001/05/16 17:12:52  crashrl
// Added md5-sum support, removed recursiv wad search
//
// Revision 1.6  2001/01/25 22:15:41  bpereira
// added heretic support
//
// Revision 1.5  2000/10/08 13:30:00  bpereira
// Revision 1.4  2000/04/16 18:38:07  bpereira
// Revision 1.3  2000/02/27 00:42:10  hurdler
// Revision 1.2  2000/02/26 00:28:42  hurdler
// Mostly bug fix (see borislog.txt 23-2-2000, 24-2-2000)
//
//
// DESCRIPTION:
//    Network file transfer.
//
//-----------------------------------------------------------------------------

#ifndef D_NETFIL_H
#define D_NETFIL_H

#include "doomdef.h"
  // MAX_WADPATH
#include "w_wad.h"
  // MAX_WADFILES

// Type of storage, after transmission handling
typedef enum {
    TAH_FILE    = 0,  // close file
    TAH_Z_FREE     ,  // free with Z_Free
    TAH_MALLOC_FREE,  // free with free
    TAH_NOTHING       // do nothing
} TAH_e;

typedef enum {
    FS_NOTFOUND,
    FS_FOUND,
    FS_REQUESTED,
    FS_DOWNLOADING,
    FS_OPEN,        // is opened and used in w_wad
    FS_MD5SUMBAD
} filestatus_e;

typedef struct {
    char    filename[MAX_WADPATH];
    unsigned char    md5sum[16];
    // used only for download
    FILE    *phandle;     // open file (owned)
    uint32_t bytes_recv;  // to determine when done and for status
    uint32_t totalsize;
    filestatus_e status;        // the value returned by recsearch
} fileneed_t;

extern int cl_num_fileneed;
extern fileneed_t cl_fileneed[MAX_WADFILES];

void D_NetFileInit(void);

byte * Put_Server_FileNeed(void);
void CL_Got_Fileneed(int num_fileneed_parm, byte *fileneed_str);
void CL_Prepare_Download_SaveGame(const char *tmpsave);
boolean  CL_waiting_on_fileneed( void );

// Check file list in wadfiles return.
typedef enum {
   CFR_no_files,
   CFR_all_found,
   CFR_download_needed,
   CFR_iwad_error,
   CFR_insufficient_space // Does not seem to be used.
} checkfiles_e;

checkfiles_e  CL_CheckFiles(void);
boolean  CL_Load_ServerFiles(void);

//void SendFile(byte to_node, char *filename, char fileid);
void SendData(byte to_node, byte *data, uint32_t size, TAH_e tah, char fileid);

void Filetx_Ticker(void);
void Got_Filetxpak(void);

boolean Send_RequestFile(void);
void Got_RequestFilePak(byte nnode);


void AbortSendFiles(byte nnode);
void CloseNetFile(void);

#if 0
// Unused
boolean fileexist(char *filename, time_t chk_time);
#endif

// search a file in the wadpath, return FS_FOUND when found
filestatus_e findfile(char *filename, unsigned char *wantedmd5sum, boolean completepath);
filestatus_e checkfilemd5(char *filename, unsigned char *wantedmd5sum);

void nameonly(char *s);

#endif // D_NETFIL_H
