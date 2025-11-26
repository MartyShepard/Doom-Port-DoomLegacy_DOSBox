// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: d_netfil.h 1035 2013-08-14 00:38:40Z wesleyjohnson $
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
    FILE    *phandle;  
    ULONG   currentsize;
    ULONG   totalsize;
    filestatus_e status;        // the value returned by recsearch
} fileneeded_t;

extern int fileneedednum;
extern fileneeded_t fileneeded[MAX_WADFILES];

void D_NetFileInit(void);

byte *PutFileNeeded(void);
void D_ParseFileneeded(int fileneedednum_parm, byte *fileneededstr);
void CL_PrepareDownloadSaveGame(const char *tmpsave);

// check file list in wadfiles return 0 when a file is not found 
//                                    1 if all file are found 
//                                    2 if you cannot connect (different wad version or 
//                                                   no enought space to download files)
int CL_CheckFiles(void);
void CL_LoadServerFiles(void);
void SendFile(int node,char *filename, char fileid);
void SendData(int node, byte *data, ULONG size, TAH_e tah, char fileid);

void FiletxTicker(void);
void Got_Filetxpak(void);

boolean SendRequestFile(void);
void Got_RequestFilePak(int node);


void AbortSendFiles(int node);
void CloseNetFile(void);

boolean fileexist(char *filename,time_t time);

// search a file in the wadpath, return FS_FOUND when found
filestatus_e findfile(char *filename, unsigned char *wantedmd5sum, boolean completepath);
filestatus_e checkfilemd5(char *filename, unsigned char *wantedmd5sum);

void nameonly(char *s);

#endif // D_NETFIL_H
