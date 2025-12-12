// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: w_wad.c 1194 2015-12-26 19:08:47Z wesleyjohnson $
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2012 by DooM Legacy Team.
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
// $Log: w_wad.c,v $
// Revision 1.34  2004/04/20 00:34:26  andyp
// Linux compilation fixes and string cleanups
//
// Revision 1.33  2002/07/29 21:52:25  hurdler
// Revision 1.32  2002/07/26 15:21:36  hurdler
// Revision 1.31  2002/01/14 18:54:45  hurdler
// Revision 1.30  2001/07/28 16:18:37  bpereira
// Revision 1.29  2001/05/27 13:42:48  bpereira
//
// Revision 1.28  2001/05/21 14:57:05  crashrl
// Readded directory crawling file search function
//
// Revision 1.27  2001/05/16 22:33:34  bock
// Initial FreeBSD support.
//
// Revision 1.26  2001/05/16 17:12:52  crashrl
// Added md5-sum support, removed recursiv wad search
//
// Revision 1.25  2001/04/17 22:26:07  calumr
// Initial Mac add
//
// Revision 1.24  2001/03/03 06:17:34  bpereira
// Revision 1.23  2001/02/28 17:50:55  bpereira
// Revision 1.22  2001/02/24 13:35:21  bpereira
//
// Revision 1.21  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.20  2000/10/04 16:19:24  hurdler
// Change all those "3dfx names" to more appropriate names
//
// Revision 1.19  2000/09/28 20:57:19  bpereira
// Revision 1.18  2000/08/31 14:30:56  bpereira
//
// Revision 1.17  2000/08/11 21:37:17  hurdler
// fix win32 compilation problem
//
// Revision 1.16  2000/08/11 19:10:13  metzgermeister
// Revision 1.15  2000/07/01 09:23:49  bpereira
//
// Revision 1.14  2000/05/09 20:57:58  hurdler
// use my own code for colormap (next time, join with Boris own code)
// (necessary due to a small bug in Boris' code (not found) which shows strange effects under linux)
//
// Revision 1.13  2000/04/30 10:30:10  bpereira
//
// Revision 1.12  2000/04/27 17:43:19  hurdler
// colormap code in hardware mode is now the default
//
// Revision 1.11  2000/04/16 18:38:07  bpereira
//
// Revision 1.10  2000/04/13 23:47:47  stroggonmeth
// See logs
//
// Revision 1.9  2000/04/11 19:07:25  stroggonmeth
// Finished my logs, fixed a crashing bug.
//
// Revision 1.8  2000/04/09 02:30:57  stroggonmeth
// Fixed missing sprite def
//
// Revision 1.7  2000/04/08 11:27:29  hurdler
// fix some boom stuffs
//
// Revision 1.6  2000/04/07 01:39:53  stroggonmeth
// Fixed crashing bug in Linux.
// Made W_ColormapNumForName search in the other direction to find newer colormaps.
//
// Revision 1.5  2000/04/06 20:40:22  hurdler
// Mostly remove warnings under windows
//
// Revision 1.4  2000/04/05 15:47:47  stroggonmeth
// Added hack for Dehacked lumps. Transparent sprites are now affected by colormaps.
//
// Revision 1.3  2000/04/04 00:32:48  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Handles WAD file header, directory, lump I/O.
//
//-----------------------------------------------------------------------------

#include <fcntl.h>
  // open
#include <unistd.h>
  // close, read, lseek

#include "doomincl.h"
#include "w_wad.h"
#include "z_zone.h"

#include "i_video.h"
  // rendermode
#include "d_netfil.h"
#include "dehacked.h"
  // DEH_LoadDehackedLump
#include "r_defs.h"
#include "i_system.h"

#include "md5.h"
#include "m_swap.h"

#ifdef HWRENDER
#include "hardware/hw_main.h"
#endif



//===========================================================================
//                                                                    GLOBALS
//===========================================================================
int          numwadfiles;             // number of active wadfiles
wadfile_t*   wadfiles[MAX_WADFILES];  // 0 to numwadfiles-1 are valid

// W_Shutdown
// Closes all of the WAD files before quitting
// If not done on a Mac then open wad files
// can prevent removable media they are on from
// being ejected
void W_Shutdown(void)
{
    while (numwadfiles--)
    {
        close(wadfiles[numwadfiles]->handle);
    }
}

//===========================================================================
//                                                        LUMP BASED ROUTINES
//===========================================================================

// Lump Markers

typedef struct {
   char      markername[9];
   uint8_t   marker_namespace;  // start marker namespace
} marker_ident_t;

// At the end of each section, return to LNS_global namespace.
#define NUM_MARKER_IDENT   12
// Fill each string with 0, out to 8 characters.
marker_ident_t  marker_ident[ NUM_MARKER_IDENT ] =
{
   { "S_START\0", LNS_sprite }, // S_START to S_END
   { "S_END\0\0\0",   LNS_global },
   { "F_START\0", LNS_flat },   // F_START to F_END
   { "F_END\0\0\0",   LNS_global },
   { "C_START\0", LNS_colormap },  // C_START to C_END
   { "C_END\0\0\0",   LNS_global },
   { "SS_START",  LNS_sprite }, // SS_START to SS_END
   { "SS_END\0\0",    LNS_global },
   { "FF_START",  LNS_flat },   // FF_START to FF_END
   { "FF_END\0\0",    LNS_global },
   { "PP_START",  LNS_patch },  // PP_START to PP_END
   { "PP_END\0\0",    LNS_global },
};


// Make the name into numerical for easy compares.
static void  numerical_name( const char * name, lump_name_t * numname )
{
    numname->namecode = 0;  // clear
    strncpy (numname->s, name, 8);
    // in case the name was a full 8 chars (has no 0 term)
    numname->s[8] = 0;
    // case insensitive compares
    strupr (numname->s);
    // numname.namecode can now be used for compares as uint64_t
}


// W_AddFile
// All files are optional, but at least one file must be
//  found (PWAD, if all required lumps are present).
// Files with a .wad extension are wadlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.
//
// If filename starts with a tilde, the file is handled
//  specially to allow map reloads.
// But: the reload feature is a fragile hack...

static int    reload_lumpnum;
static char*  reload_filename;


//  Allocate a wadfile, setup the lumpinfo (directory) and
//  lumpcache, add the wadfile to the current active wadfiles
//
//  Now returns index into wadfiles[], you can get wadfile_t*
//  with:
//       wadfiles[<return value>]
//
//  return -1 in case of problem
//
// BP: Can now load dehacked files (ext .deh)
int W_LoadWadFile (const char *filename)
{
    int              filenum = numwadfiles;  // return value
    filestatus_e     fs;
    int              handle;
    FILE             *fhandle;
    int              numlumps;
    lumpinfo_t*      lumpinfo;
    lumpcache_t*     lumpcache;
    wadfile_t*       wadfile;
    int              i, m;
    int              length;
    struct stat      bufstat;
    // findfile requires a buffer of (at least) MAX_WADPATH
    char             filenamebuf[MAX_WADPATH];
#ifdef HWRENDER    
    MipPatch_t*      grPatch;
#endif
    lump_namespace_e within_namespace = LNS_global;

    //
    // check if limit of active wadfiles
    //
    if (filenum>=MAX_WADFILES)
    {
        CONS_Printf ("Maximum wad files reached\n");
        return -1;
    }

    strncpy(filenamebuf, filename, MAX_WADPATH-1);
    filenamebuf[MAX_WADPATH-1] = '\0';

    // open wad file
    handle = open (filenamebuf, O_RDONLY|O_BINARY, 0666);
    if( handle == -1 )
    {
        // not in cur dir, must search
        nameonly(filenamebuf); // only search for the name

        // findfile returns dir+filename
        fs = findfile(filenamebuf, NULL, /*OUT*/ filenamebuf);
	if( fs == FS_NOTFOUND )
        {
            CONS_Printf ("File %s not found.\n", filenamebuf);
            return -1;
        }

        handle = open (filenamebuf, O_RDONLY|O_BINARY, 0666);
        if( handle == -1 )
        {
            CONS_Printf ("Can't open %s\n", filenamebuf);
            return -1;
        }
    }

    // detect dehacked file with the "deh" extension, or bex files
    char * extension = &filenamebuf[strlen(filenamebuf)-3];
    if( strcasecmp( extension,"deh")==0
       || strcasecmp( extension,"bex")==0 )
    {
        // This code emulates a wadfile with one lump name "DEHACKED" 
        // at position 0 and size of the whole file.
        // This allow deh file to be copied by network and loaded at the
        // console, like wad files.
        fstat(handle,&bufstat);
        numlumps = 1; 
        lumpinfo = Z_Malloc (sizeof(lumpinfo_t),PU_STATIC,NULL);
        lumpinfo->position = 0;
        lumpinfo->size = bufstat.st_size;
        lumpinfo->lump_namespace = LNS_dehacked;
        strncpy(lumpinfo->name, "DEHACKED", 8);
    }
    else 
    {   // assume wad file
        wadinfo_t        header;
        lumpinfo_t*      lump_p;
        filelump_t*      fileinfo;
        filelump_t*      flp;

        // read the header
        read (handle, &header, sizeof(header));
        if (strncmp(header.identification,"IWAD",4))
        {
            // Homebrew levels?
            if (strncmp(header.identification,"PWAD",4))
            {
                CONS_Printf ("%s doesn't have IWAD or PWAD id\n", filenamebuf);
                return -1;
            }
            // ???modifiedgame = true;
        }
        header.numlumps = LE_SWAP32(header.numlumps);
        header.infotableofs = LE_SWAP32(header.infotableofs);

        // read wad file directory
        length = header.numlumps * sizeof(filelump_t);
        fileinfo = calloc (length, sizeof(*fileinfo));  // temp alloc, zeroed

        lseek (handle, header.infotableofs, SEEK_SET);
        read (handle, fileinfo, length);
        numlumps = header.numlumps;
        
        within_namespace = LNS_global;  // each wad starts in global namespace

        // fill in lumpinfo array for this wad
	flp = fileinfo;
        lump_p = lumpinfo = Z_Malloc (numlumps*sizeof(lumpinfo_t),PU_STATIC,NULL);
        for (i=0 ; i<numlumps ; i++, lump_p++, flp++)
        {
	    // Make name compatible with compares using numerical_name.
	    *((uint64_t*)&lump_p->name) = 0;  // clear
	    strncpy (lump_p->name, flp->name, 8);
	    // Check for namespace markers using clean lump name.
	    for( m=0; m < NUM_MARKER_IDENT; m++ )
	    {
	       // Fast numerical name compare.
	       if( *((uint64_t*)&lump_p->name) == *((uint64_t*)&marker_ident[m].markername) )
		  within_namespace = marker_ident[m].marker_namespace;
	    }
	    // No uppercase because no other port does it here, and it would
	    // catch lowercase named lumps that otherwise would be ignored.
	    lump_p->lump_namespace = within_namespace;
            lump_p->position = LE_SWAP32(flp->filepos);
            lump_p->size     = LE_SWAP32(flp->size);
        }
        free(fileinfo);
    }
    //
    //  link wad file to search files
    //
    fstat(handle,&bufstat);
    wadfile = Z_Malloc (sizeof (wadfile_t),PU_STATIC,NULL);
    wadfile->filename = Z_StrDup(filenamebuf);
    wadfile->handle = handle;
    wadfile->numlumps = numlumps;
    wadfile->lumpinfo = lumpinfo;
    wadfile->filesize = bufstat.st_size;

    //
    //  generate md5sum
    // 
    fhandle = fopen(filenamebuf, "rb");
    {
        int t=I_GetTime();
        md5_stream (fhandle, wadfile->md5sum);
        if( devparm )
        {
            GenPrintf(EMSG_dev, "md5 calc for %s took %f second\n",
                        wadfile->filename,(float)(I_GetTime()-t)/TICRATE);
	}
    }
    fclose(fhandle);
    
    //
    //  set up caching
    //
    length = numlumps * sizeof(lumpcache_t);
    lumpcache = Z_Malloc (length,PU_STATIC,NULL);

    memset (lumpcache, 0, length);
    wadfile->lumpcache = lumpcache;

#ifdef HWRENDER
    //faB: now allocates MipPatch info structures STATIC from the start,
    //     because these were causing a lot of fragmentation of the heap,
    //     considering they are never freed.
    length = numlumps * sizeof(MipPatch_t);
    grPatch = Z_Malloc (length, PU_HWRPATCHINFO, 0);    //never freed
    // set mipmap.downloaded to false
    memset (grPatch, 0, length);
    for (i=0; i<numlumps; i++)
    {
        //store the software patch lump number for each GlidePatch
        grPatch[i].patchlump = (filenum<<16) + i;  // form file/lump
    }
    wadfile->hwrcache = grPatch;
#endif

    //
    //  add the wadfile
    //
    wadfiles[filenum] = wadfile;
    numwadfiles++;

    CONS_Printf ("Added file %s (%i lumps)\n", filenamebuf, numlumps);
    W_LoadDehackedLumps( filenum );
    return filenum;
}



// !!!NOT CHECKED WITH NEW WAD SYSTEM
//
// W_Reload
// Flushes any of the reloadable lumps in memory
//  and reloads the lump directory.
//
void W_Reload (void)
{
    wadinfo_t           header;
    int                 lumpcount;
    lumpinfo_t*         lump_p;
    int                 i;
    int                 handle;
    int                 length;
    int                 filenum, lumpnum;
    filelump_t*         fileinfo;
    filelump_t*         flp;
    lumpcache_t*        lumpcache;

    if (!reload_filename)
        return;

    if ( (handle = open (reload_filename,O_RDONLY | O_BINARY)) == -1)
        I_Error ("W_Reload: couldn't open %s",reload_filename);

    read (handle, &header, sizeof(header));
    lumpcount = LE_SWAP32(header.numlumps);
    header.infotableofs = LE_SWAP32(header.infotableofs);
    length = lumpcount*sizeof(filelump_t);
    fileinfo = calloc (length, sizeof(*fileinfo));  // temp alloc, zeroed
    lseek (handle, header.infotableofs, SEEK_SET);
    read (handle, fileinfo, length);

    // Fill in lumpinfo
    filenum = WADFILENUM(reload_lumpnum);
    lumpnum = LUMPNUM(reload_lumpnum);
    lump_p = & wadfiles[filenum]->lumpinfo[ lumpnum ];
    lumpcache = wadfiles[filenum]->lumpcache;

    flp = fileinfo;
    // Only index lumpcache by lumpnum.
    for (i=lumpnum ; i<lumpnum+lumpcount ; i++)
    {
        if (lumpcache[i])
            Z_Free (lumpcache[i]);

        lump_p->position = LE_SWAP32(flp->filepos);
        lump_p->size = LE_SWAP32(flp->size);
        lump_p++;
        flp++;
    }

    close (handle);
    free(fileinfo);
}


//
// W_InitMultipleFiles
// Pass a null terminated list of files to use.
// All files are optional, but at least one file must be found.
// Any load failure is indicated to the caller, for error handling.
// Files with a .wad extension are idlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.
// Lump names can appear multiple times.
// The name searcher looks backwards, so a later file
//  does override all earlier ones.
//
// Return 0 when any file load does not succeed
int W_InitMultipleFiles (char** filenames)
{
    int  rc = 1;

    // open all the files, load headers, and count lumps
    numwadfiles = 0;

    // will be realloced as lumps are added
    for ( ; *filenames ; filenames++)
    {
        if( W_LoadWadFile (*filenames) == -1 )
        {
	    rc = 0;
	    GenPrintf( EMSG_warn, "File not found: %s\n", *filenames );
	}
    }

    if (!numwadfiles)
    {
        I_SoftError ("W_InitMultipleFiles: no files found\n");
        fatal_error = 1;
    }

    return rc;
}


//
//  W_CheckNumForName
//  Lists are currently used by flats and colormaps.
//  These are faster as they skip checking some names.
//  Patches within textures use namespace to protect against trying to use
//  another lump of the same name (colormap) as a patch.

//  Return lump id, or -1 if name not found.
int W_Check_Namespace (const char* name, lump_namespace_e within_namespace)
{
    int  alternate = -1;
    int  i,j;
    lump_name_t name8;
    lumpinfo_t* lump_p;

    numerical_name( name, & name8 );

    // Scan wad files backwards so PWAD files take precedence.
    for (i = numwadfiles-1 ; i>=0; i--)
    {
        // Scan forward within a wad file.
        lump_p = wadfiles[i]->lumpinfo;
        for (j = 0; j<wadfiles[i]->numlumps; j++,lump_p++)
        { 
	    // Fast numerical name compare.
	    if ( *(uint64_t *)lump_p->name == name8.namecode )
	    {
	        // Name matches.
		// Check if lump is from the wanted namespace.
	        if( within_namespace == LNS_any
		    || lump_p->lump_namespace == within_namespace )
	        {
		    // Return wad/lump identifier
		    return ((i<<16) + j);
		}
	        // Wrong namespace.
	        if( alternate == -1 )  // remember first lump found
		   alternate = ((i<<16) + j);  // wad/lump identifier
	    }
	}
    }
    // Not found.
    // Return first matching lump found in any namespace.
    if( alternate != -1 )
      return alternate;
    return -1;
}

//  Return lump id, or -1 if name not found.
int W_CheckNumForName (const char* name)
{
    return W_Check_Namespace( name, LNS_any );
}



//
//  Same as the original, but checks in one pwad only
//  (Used for sprites loading)
//    wadid :  the wad number
//    startlump : the lump number to start the search
//  Return lump number.
int W_CheckNumForNamePwad (char* name, int wadid, int startlump)
{
    int         i;
    lump_name_t name8;
    lumpinfo_t* lump_p;

    // make the name into numerical for easy compares
    numerical_name( name, & name8 );

    //
    // scan forward
    // start at 'startlump', useful parameter when there are multiple
    //                       resources with the same name
    //
    if (startlump < wadfiles[wadid]->numlumps)
    {
        lump_p = wadfiles[wadid]->lumpinfo + startlump;
        for (i = startlump; i<wadfiles[wadid]->numlumps; i++,lump_p++)
        {
	    // Fast numerical name compare.
            if ( *(uint64_t *)lump_p->name == name8.namecode )
            {
                return ((wadid<<16)+i);
            }
        }
    }

    // not found.
    return -1;
}



//
// W_GetNumForName
//   Calls W_CheckNumForName, but bombs out if not found.
//
int W_GetNumForName (char* name)
{
    int i;

    i = W_CheckNumForName (name);

    if (i == -1)
    {
#ifdef DEBUG_CHEXQUEST
        I_SoftError ("W_GetNumForName: %s not found!\n", name);	// [WDJ] 4/28/2009 Chexquest
#else
        I_Error ("W_GetNumForName: %s not found!\n", name);
#endif
    }

    return i;
}

// Scan wads files forward, IWAD precedence.
int W_CheckNumForNameFirst (char* name)
{
    int i, j;
    lump_name_t name8;
    lumpinfo_t* lump_p;

    numerical_name( name, & name8 );

    // scan wad files forward, when original wad resources
    //  must take precedence
    for (i = 0; i<numwadfiles; i++)
    {
        lump_p = wadfiles[i]->lumpinfo;
        for (j = 0; j<wadfiles[i]->numlumps; j++,lump_p++)
        {
            if ( *(uint64_t *)lump_p->name == name8.namecode )
            {
                return ((i<<16) + j);
            }
        }
    }

    return -1;
}

//
//  W_GetNumForNameFirst : like W_GetNumForName, but scans FORWARD
//                         so it gets resources from the original wad first
//  (this is used only to get S_START for now, in r_data.c)
int W_GetNumForNameFirst (char* name)
{
    int i;

    i = W_CheckNumForNameFirst (name);
    if (i == -1)
        I_Error ("W_GetNumForNameFirst: %s not found!", name);

    return i;
}


//
//  W_LumpLength
//   Returns the buffer size needed to load the given lump.
//
int W_LumpLength (int lump)
{
#ifdef PARANOIA
    if (lump<0) I_Error("W_LumpLength: lump not exist\n");

    if (LUMPNUM(lump) >= wadfiles[WADFILENUM(lump)]->numlumps)
        I_Error ("W_LumpLength: %i >= numlumps",lump);
#endif
    return wadfiles[WADFILENUM(lump)]->lumpinfo[LUMPNUM(lump)].size;
}



//
// W_ReadLumpHeader : read 'size' bytes of lump
//                    sometimes just the header is needed
//
//Fab:02-08-98: now returns the number of bytes read (should == size)
int  W_ReadLumpHeader ( int           lump,
                        void*         dest,
                        int           size )
{
    int         bytesread;
    lumpinfo_t* lif;
    int         handle;
#ifdef PARANOIA
    if (lump<0) I_Error("W_ReadLumpHeader: lump not exist\n");

    if (LUMPNUM(lump) >= wadfiles[WADFILENUM(lump)]->numlumps)
        I_Error ("W_ReadLumpHeader: %i >= numlumps",lump);
#endif
    lif = wadfiles[WADFILENUM(lump)]->lumpinfo + LUMPNUM(lump);

    // the good ole 'loading' disc icon TODO: restore it :)
    // ??? I_BeginRead ();

    // empty resource (usually markers like S_START, F_END ..)
    if (lif->size==0)
        return 0;

/*    if (lif->handle == -1)
    {
        // reloadable file, so use open / read / close
        if ( (handle = open (reload_filename,O_RDONLY|O_BINARY,0666)) == -1)
            I_Error ("W_ReadLumpHeader: couldn't open %s",reload_filename);
    }
    else
*/
        handle = wadfiles[WADFILENUM(lump)]->handle; //lif->handle;

    // 0 size means read all the lump
    if (!size || size>lif->size)
        size = lif->size;
    
    lseek (handle, lif->position, SEEK_SET);
    bytesread = read (handle, dest, size);

    /*if (lif->handle == -1)
        close (handle);*/

    // ??? I_EndRead ();
    return bytesread;
}


//
//  W_ReadLump
//  Loads the lump into the given buffer,
//   which must be >= W_LumpLength().
//
//added:06-02-98: now calls W_ReadLumpHeader() with full lump size.
//                0 size means the size of the lump, see W_ReadLumpHeader
void W_ReadLump ( int           lump,
                  void*         dest )
{
    W_ReadLumpHeader (lump, dest, 0);
}


// ==========================================================================
// W_CacheLumpNum
// ==========================================================================
// [WDJ] Indicates cache miss, new lump read requires endian fixing.
boolean lump_read;	// set by W_CacheLumpNum

void* W_CacheLumpNum ( int lump, int tag )
{
    lumpcache_t*  lumpcache;

    //SoM: 4/8/2000: Don't keep doing operations to the lump variable!
    int  llump = LUMPNUM(lump);
    int  lfile = WADFILENUM(lump);

#ifdef DEBUG_CHEXQUEST
   // [WDJ] Crashes in chexquest with black screen, cannot debug
   if(lump == -1) {
      lump_read = 0;  // no data
       // [WDJ] prevent SIGSEGV in chexquest
      I_SoftError ("W_CacheLumpNum: -1 passed!\n");
      return NULL;
    }
#endif   
#ifdef PARANOIA
    // check return value of a previous W_CheckNumForName()
    //SoM: 4/8/2000: Do better checking. No more SIGSEGV's!
    if(lump == -1)	// [WDJ] must be first to protect use as index
      I_Error ("W_CacheLumpNum: -1 passed!\n");
    if (lfile >= numwadfiles)
      I_Error("W_CacheLumpNum: %i >= numwadfiles(%i)\n", lfile, numwadfiles);
    if (llump >= wadfiles[lfile]->numlumps)
      I_Error ("W_CacheLumpNum: %i >= numlumps", llump);
    if(llump < 0)
      I_Error ("W_CacheLumpNum: %i < 0!\n", llump);
#endif

    lumpcache = wadfiles[lfile]->lumpcache;
    if (!lumpcache[llump])
    {
        // read the lump in

        //CONS_Printf ("cache miss on lump %i\n",lump);
        byte* ptr = Z_Malloc (W_LumpLength (lump), tag, &lumpcache[llump]);
        W_ReadLumpHeader (lump, ptr, 0);   // read whole lump
//        W_ReadLumpHeader (lump, lumpcache[llump], 0);   // read whole lump
        lump_read = 1; // cache miss, read lump, caller must apply endian fix
    }
    else
    {
        //CONS_Printf ("cache hit on lump %i\n",lump);
        // [WDJ] Do not degrade lump to PU_CACHE while it is in use.
        if( tag == PU_CACHE )   tag = PU_CACHE_DEFAULT;
        Z_ChangeTag (lumpcache[llump], tag);
        lump_read = 0;  // cache hit, cache already has endian fixes
    }

    return lumpcache[llump];
}


// ==========================================================================
// W_CacheLumpName
// ==========================================================================
void* W_CacheLumpName ( char* name, int tag )
{
    return W_CacheLumpNum (W_GetNumForName(name), tag);
}



// ==========================================================================
//                                         CACHING OF GRAPHIC PATCH RESOURCES
// ==========================================================================

// Graphic 'patches' are loaded, and if necessary, converted into the format
// the most useful for the current rendermode. For software renderer, the
// graphic patches are kept as is. For the hardware renderer, graphic patches
// are 'unpacked', and are kept into the cache in that unpacked format, the
// heap memory cache then act as a 'level 2' cache just after the graphics
// card memory.

//
// Cache a patch into heap memory, convert the patch format as necessary
//

// [WDJ] When there is another lump with the same name as a patch, this will
// sometimes get that lump and convert the header as if it was a patch.
// This should only get lumps that are patches.

// Cache the patch with endian conversion
// [WDJ] Only read patches using this function, hardware render too.
// [WDJ] Removed inline because is also called from in hardware/hw_cache.c.
void* W_CachePatchNum_Endian ( int lump, int tag )
{
// __BIG_ENDIAN__ is defined on MAC compilers, not on WIN, nor LINUX
#ifdef __BIG_ENDIAN__
    patch_t * patch = W_CacheLumpNum(lump,tag);
    // [WDJ] If newly read patch then fix endian.
    if( lump_read )
    {
        patch->height = LE_SWAP16(patch->height);
        patch->width = LE_SWAP16(patch->width);
	patch->topoffset = LE_SWAP16(patch->topoffset);
	patch->leftoffset = LE_SWAP16(patch->leftoffset);
        {
	    // [WDJ] columnofs[ 0 .. width-1 ]
	    // The patch structure only shows 8, but there can be many more.
            int i = patch->width - 1;
            for( ; i>=0; i-- )
	       patch->columnofs[i] = LE_SWAP32( patch->columnofs[i] );
	}
    }
    return patch;
#else
    // [WDJ] Optimized version for little-endian, much faster
    return W_CacheLumpNum(lump,tag);
#endif
}


#ifdef HWRENDER // not win32 only 19990829 by Kin

// Called from many draw functions
void* W_CachePatchNum ( int lump, int tag )
{
    MipPatch_t*   grPatch;

    if( rendermode == render_soft ) {
        return W_CachePatchNum_Endian ( lump, tag );
    }
   
    // hardware render

#ifdef PARANOIA
    // check the return value of a previous W_CheckNumForName()
    if ( ( lump==-1 ) ||
         (LUMPNUM(lump) >= wadfiles[WADFILENUM(lump)]->numlumps) )
        I_Error ("W_CachePatchNum: %i >= numlumps", LUMPNUM(lump));
#endif

    grPatch = &(wadfiles[WADFILENUM(lump)]->hwrcache[LUMPNUM(lump)]);

    if( ! grPatch->mipmap.grInfo.data ) 
    {   // first time init grPatch fields
        // we need patch w,h,offset,...
        patch_t *tmp_patch = W_CachePatchNum_Endian(grPatch->patchlump, PU_LUMP); // temp use
        // default no TF_Opaquetrans
        HWR_MakePatch ( tmp_patch, grPatch, &grPatch->mipmap, 0);
        Z_Free (tmp_patch);
        // HWR_MakePatch makes grInfo.data as PU_HWRCACHE
    }

    // return MipPatch_t, which can be casted to (patch_t) with valid patch header info
    return (void*)grPatch;
}

// [WDJ] Called from hardware render for special mapped sprites
void* W_CacheMappedPatchNum ( int lump, uint32_t drawflags )
{
    MipPatch_t*   grPatch;

#ifdef PARANOIA
    // check the return value of a previous W_CheckNumForName()
    if ( ( lump==-1 ) ||
         (LUMPNUM(lump) >= wadfiles[WADFILENUM(lump)]->numlumps) )
        I_Error ("W_CachePatchNum: %i >= numlumps", LUMPNUM(lump));
#endif

    grPatch = &(wadfiles[WADFILENUM(lump)]->hwrcache[LUMPNUM(lump)]);

    if( ! grPatch->mipmap.grInfo.data )
    {   // first time init grPatch fields
        // we need patch w,h,offset,...
        patch_t *tmp_patch = W_CachePatchNum_Endian(grPatch->patchlump, PU_LUMP); // temp use
        // pass TF_Opaquetrans
        HWR_MakePatch ( tmp_patch, grPatch, &grPatch->mipmap, drawflags);
        Z_Free (tmp_patch);
        // HWR_MakePatch makes grInfo.data as PU_HWRCACHE
    }

    // return MipPatch_t, which can be casted to (patch_t) with valid patch header info
    return (void*)grPatch;
}

#else // HWRENDER version
// Software renderer
void* W_CachePatchNum ( int lump, int tag )
{
    return W_CachePatchNum_Endian( lump, tag );
}
#endif // HWRENDER version


// Find patch in LNS_patch namespace
void* W_CachePatchName ( char* name, int tag )
{
    int lumpid;
    // substitute known name for name not found
    lumpid = W_Check_Namespace( name, LNS_patch );
    if( lumpid == -1 )
    {
        name = "BRDR_MM";
        lumpid = W_Check_Namespace( name, LNS_patch );
        if( lumpid == -1 )
        I_Error ("W_CachePatchName: %s not found!\n", name);
    }
    return W_CachePatchNum( lumpid, tag);
}


// convert raw heretic picture to legacy pic_t format
// Used for heretic: TITLE, HELP1, HELP2, ORDER, CREDIT, FINAL1, FINAL2, E2END
// Used for raven demo screen
void* W_CacheRawAsPic( int lump, int width, int height, int tag)
{
    // [WDJ] copy of CacheLumpNum with larger lump allocation,
    // read into pic, and no endian fixes
    lumpcache_t*  lumpcache;
    //SoM: 4/8/2000: Don't keep doing operations to the lump variable!
    int  llump = LUMPNUM(lump);
    int  lfile = WADFILENUM(lump);

#ifdef PARANOIA
    // check return value of a previous W_CheckNumForName()
    //SoM: 4/8/2000: Do better checking. No more SIGSEGV's!
    if(lump == -1)
      I_Error ("W_CacheRawAsPic: -1 passed!\n");
    if (lfile >= numwadfiles)
      I_Error("W_CacheRawAsPic: %i >= numwadfiles(%i)\n", lfile, numwadfiles);
    if (llump >= wadfiles[lfile]->numlumps)
      I_Error ("W_CacheRawAsPic: %i >= numlumps", llump);
    if(llump < 0)
      I_Error ("W_CacheRawAsPic: %i < 0!\n", llump);
#endif

    lumpcache = wadfiles[lfile]->lumpcache;
    if (!lumpcache[llump]) 	// cache miss
    {
        // read the lump in

        // Allocation is larger than what W_CacheLumpNum does
        pic_t* pic = Z_Malloc (W_LumpLength (lump)+sizeof(pic_t),
			       tag, &lumpcache[llump]);
        // read lump + pic into pic->data (instead of lumpcache)
        W_ReadLumpHeader (lump, pic->data, 0);
        // set pic info from caller parameters, (which are literals)
        pic->width = width;
        pic->height = height;
        pic->mode = PALETTE;
    }
    else
    {
        // [WDJ] Do not degrade lump to PU_CACHE while it is in use.
        if( tag == PU_CACHE )
	   tag = PU_CACHE_DEFAULT;
        Z_ChangeTag (lumpcache[llump], tag);
    }

    return lumpcache[llump];
}


// Cache and endian convert a pic_t
void* W_CachePicNum( int lumpnum, int tag )
{
// __BIG_ENDIAN__ is defined on MAC compilers, not on WIN, nor LINUX
#ifdef __BIG_ENDIAN__
    pic_t * pt = W_CacheLumpNum ( lumpnum, tag );
    // [WDJ] If newly read pic then fix endian.
    if( lump_read )
    {
        pt->height = LE_SWAP16(pt->height);
        pt->width = LE_SWAP16(pt->width);
//        pt->reserved = LE_SWAP16(pt->reserved);
    }
    return pt;
#else
    // [WDJ] Optimized version for little-endian, much faster
    return W_CacheLumpNum(lumpnum,tag);
#endif
}

// Cache and endian convert a pic_t
void* W_CachePicName( char* name, int tag )
{
    return W_CachePicNum( W_GetNumForName(name), tag);
}


// Search for all DEHACKED lump in all wads and load it.
void W_LoadDehackedLumps( int wadnum )
{
    int clump = 0;
    
    while (1)
    { 
        clump = W_CheckNumForNamePwad("DEHACKED", wadnum, clump);
        if(clump == -1)
            break;
        CONS_Printf("Loading dehacked from %s\n",wadfiles[wadnum]->filename);
        DEH_LoadDehackedLump(clump);
        clump++;
    }
}

// [WDJ] Return a sum unique to a lump, to detect replacements.
// The lumpptr must be to a Z_Malloc lump.
uint64_t  W_lump_checksum( void* lumpptr )
{
    // Work only with the lumpptr given, cannot trust that can get stats
    // on the exact same lump, it may be in multiple wads and pwads.
    // Very simple checksum over the size of the Z_Malloc block.
    int lumpsize = Z_Datasize( lumpptr );
    uint64_t  checksum = 0;
    int i;
    for( i=0; i<lumpsize; i++ )
      checksum += ((byte*)lumpptr)[i];
    return checksum;
}


#if 0
// Makes DOS assumptions
// --------------------------------------------------------------------------
// W_Profile
// --------------------------------------------------------------------------
//
/*     --------------------- UNUSED ------------------------
int             info[2500][10];
int             profilecount;

void W_Profile (void)
{
    int         i;
    memblock_t* block;
    void*       ptr;
    char        ch;
    FILE*       f;
    int         j;
    char        name[9];


    for (i=0 ; i<numlumps ; i++)
    {
        ptr = lumpcache[i];
        if (!ptr)
        {
            ch = ' ';
            continue;
        }
        else
        {
            block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t));
            if (block->memtag < PU_PURGELEVEL)
                ch = 'S';
            else
                ch = 'P';
        }
        info[i][profilecount] = ch;
    }
    profilecount++;

    f = fopen ("waddump.txt","w");
    name[8] = 0;

    for (i=0 ; i<numlumps ; i++)
    {
        memcpy (name,lumpinfo[i].name,8);

        for (j=0 ; j<8 ; j++)
            if (!name[j])
                break;

        for ( ; j<8 ; j++)
            name[j] = ' ';

        fprintf (f,"%s ",name);

        for (j=0 ; j<profilecount ; j++)
            fprintf (f,"    %c",info[i][j]);

        fprintf (f,"\n");
    }
    fclose (f);
}

// --------------------------------------------------------------------------
// W_AddFile : the old code kept for reference
// --------------------------------------------------------------------------
// All files are optional, but at least one file must be
//  found (PWAD, if all required lumps are present).
// Files with a .wad extension are wadlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.
//

int filelen (int handle)
{
    struct stat fileinfo;

    if (fstat (handle,&fileinfo) == -1)
        I_Error ("Error fstating");

    return fileinfo.st_size;
}


int W_AddFile (char *filename)
{
    wadinfo_t           header;
    lumpinfo_t*         lump_p;
    unsigned            i;
    int                 handle;
    int                 length;
    int                 startlump;
    filelump_t*         fileinfo;
    filelump_t          singleinfo;
    int                 storehandle;

    // open the file and add to directory

    // handle reload indicator.
    if (filename[0] == '~')
    {
        filename++;
        reload_filename = filename;
        reload_lumpnum = numlumps;
    }

    if ( (handle = open (filename,O_RDONLY | O_BINARY)) == -1)
    {
        CONS_Printf (" couldn't open %s\n",filename);
        return 0;
    }

    CONS_Printf (" adding %s\n",filename);
    startlump = numlumps;

    if (strcasecmp (filename+strlen(filename)-3, "wad") )
    {
        // single lump file
        fileinfo = &singleinfo;
        singleinfo.filepos = 0;
        singleinfo.size = LE_SWAP32(filelen(handle));
        FIL_ExtractFileBase (filename, singleinfo.name);
        numlumps++;
    }
    else
    {
        // WAD file
        read (handle, &header, sizeof(header));
        if (strncmp(header.identification,"IWAD",4))
        {
            // Homebrew levels?
            if (strncmp(header.identification,"PWAD",4))
            {
                I_Error ("Wad file %s doesn't have IWAD "
                         "or PWAD id\n", filename);
            }

            // ???modifiedgame = true;
        }
        header.numlumps = LE_SWAP32(header.numlumps);
        header.infotableofs = LE_SWAP32(header.infotableofs);
        length = header.numlumps*sizeof(filelump_t);
        fileinfo = alloca (length);
        lseek (handle, header.infotableofs, SEEK_SET);
        read (handle, fileinfo, length);
        numlumps += header.numlumps;
    }


    // Fill in lumpinfo
    lumpinfo = realloc (lumpinfo, numlumps*sizeof(lumpinfo_t));

    if (!lumpinfo)
        I_Error ("Couldn't realloc lumpinfo");

    lump_p = &lumpinfo[startlump];

    storehandle = reload_filename ? -1 : handle;

    for (i=startlump ; i<numlumps ; i++,lump_p++, fileinfo++)
    {
        lump_p->handle = storehandle;
        lump_p->position = LE_SWAP32(fileinfo->filepos);
        lump_p->size = LE_SWAP32(fileinfo->size);
        strncpy (lump_p->name, fileinfo->name, 8);
    }

    if (reload_filename)
        close (handle);

    return 1;
}
*/
#endif
