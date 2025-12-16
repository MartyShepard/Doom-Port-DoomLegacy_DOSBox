// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: Vid_vesa.c 1245 2016-08-04 14:21:00Z wesleyjohnson $
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
// $Log: Vid_vesa.c,v $
// Revision 1.5  2001/03/03 06:17:34  bpereira
//
// Revision 1.4  2001/01/25 22:15:45  bpereira
// added heretic support
//
// Revision 1.3  2000/08/31 14:30:57  bpereira
// Revision 1.2  2000/02/27 00:42:11  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      extended vesa VESA2.0 video modes i/o.
//
//-----------------------------------------------------------------------------


#include <stdlib.h>

#include "doomincl.h"
  // MAXVIDWIDTH, MAXVIDHEIGHT
  // I_Error(), CONS_Printf
//#include "i_system.h"
#include "vid_vesa.h"
#include "screen.h"

#include <dpmi.h>
#include <go32.h>
#include <sys/farptr.h>
#include <sys/movedata.h>
#include <sys/segments.h>
#include <sys/nearptr.h>

#include "console.h"
#include "command.h"            //added:21-03-98: vid_xxx commands
#include "i_video.h"


// PROTOS
static vmode_t *VID_GetModePtr (modenum_t modenum);
int  VID_VesaGetModeInfo (int modenum);
void VID_VesaGetExtraModes (void);
int  VID_VesaInitMode (viddef_t *lvid, vmode_t *currentmode_p);

void VID_Command_NumModes_f (void);
void VID_Command_ModeInfo_f (void);
void VID_Command_ModeList_f (void);
void VID_Command_Mode_f (void);


#define VBEVERSION      2       // we need vesa2 or higher

// -----------------------------------------------------
#define MASK_LINEAR(addr)     (addr & 0x000FFFFF)
#define RM_TO_LINEAR(addr)    (((addr & 0xFFFF0000) >> 12) + (addr & 0xFFFF))
#define RM_OFFSET(addr)       (addr & 0xF)
#define RM_SEGMENT(addr)      ((addr >> 4) & 0xFFFF)
// -----------------------------------------------------

static int totalvidmem;

static vmode_t      vesa_modes[MAX_VESA_MODES] = {{NULL, NULL}};
static vesa_extra_t vesa_extra[MAX_VESA_MODES];

//this is the only supported non-vesa mode : standard 320x200x256c.
#define NUMVGAVIDMODES  1
static int VGA_InitMode (viddef_t *lvid, vmode_t *currentmode_p);
static vmode_t      vgavidmodes[NUMVGAVIDMODES+1] = {
  { // 0 mode, HIDDEN
    & vgavidmodes/*specialmodes*/[1],
    "Initial",
    INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT,
    INITIAL_WINDOW_WIDTH, 1,     // rowbytes, bytes per pixel
    MODE_window, 1,  // windowed, numpages
    NULL,
    VGA_InitMode
  },
  {
    NULL,
    "320x200",
    320, 200,  //(200.0/320.0)*(320.0/240.0),
    320, 1,    // rowbytes, bytes per pixel
    MODE_window, 1,
    NULL,
    VGA_InitMode
  }
};

static char         names[MAX_VESA_MODES][10];

//----------------------------i_video.c------------------------------------
// these ones should go to i_video.c, but I prefer keep them away from the
// doom sources until the vesa stuff is ok.
int     num_all_vidmodes = 0;   // total number of all vidmodes
int     num_full_vidmodes = 0;  // total number of video modes, vga, vesa1, vesa2
vmode_t * all_vidmodes = NULL;  // start of videomodes list (window and fullscreen)
vmode_t * full_vidmodes = NULL; // start of fullscreen vidmodes
vmode_t * currentmode_p = NULL; // the current active videomode.
//----------------------------i_video.c------------------------------------


// table des modes videos.
// seul le mode 320x200x256c standard VGA est support‚ sans le VESA.
// ce mode est le mode num‚ro 0 dans la liste.
typedef struct
{
    int modenum;            // vesa vbe2.0 modenum
    int mode_attributes;
    int winasegment;
    int winbsegment;
    int bytes_per_scanline; // bytes per logical scanline (+16)
    int win;                // window number (A=0, B=1)
    int win_size;           // window size (+6)
    int granularity;        // how finely i can set the window in vid mem (+4)
    int width, height;      // displayed width and height (+18, +20)
    int bits_per_pixel;     // er, better be 8, 15, 16, 24, or 32 (+25)
    int bytes_per_pixel;    // er, better be 1, 2, or 4
    int memory_model;       // and better be 4 or 6, packed or direct color (+27)
    int num_pages;          // number of complete frame buffer pages (+29)
    int red_width;          // the # of bits in the red component (+31)
    int red_pos;            // the bit position of the red component (+32)
    int green_width;        // etc.. (+33)
    int green_pos;          // (+34)
    int blue_width;         // (+35)
    int blue_pos;           // (+36)
    int pptr;
    int pagesize;
    int numpages;
} modeinfo_t;

static vbeinfoblock_t vesainfo;
static vesamodeinfo_t vesamodeinfo;

// ------------------------------------------------------------------------
// DOS stuff
// ------------------------------------------------------------------------
static unsigned long conventional_memory = -1;

void map_in_conventional_memory(void)
{
    if (conventional_memory == -1)
    {
        if (__djgpp_nearptr_enable())
        {
            conventional_memory = __djgpp_conventional_base;
        }
    }
}

// Converts a flat 32 bit ptr to a realmode 0x12345 type ptr (seg<<4 + offs)
unsigned int ptr2real(void *ptr)
{
    map_in_conventional_memory();
    return (int)ptr - conventional_memory;
}

// Converts 0x12345 (seg<<4+offs) realmode ptr to a flat 32bit ptr
void *real2ptr(unsigned int real)
{
    map_in_conventional_memory();
    return (void *) (real + conventional_memory);
}


/* ======================================================================== */
// Add the standard VGA video modes to the video modes list.
/* ======================================================================== */
// mode 0 and 1 are used for windowed console startup.
void VID_InitVGAModes(void)
{
    // do not include Mode 0 (INITIAL) in count
    all_vidmodes = &vgavidmodes[1];
    num_all_vidmodes += NUMVGAVIDMODES;
				
}

static void append_full_vidmodes( vmode_t* newmodes, int nummodes )
{
    full_vidmodes = newmodes;
    vgavidmodes[NUMVGAVIDMODES].next = newmodes;
	  
    num_full_vidmodes += nummodes;
    num_all_vidmodes += nummodes;
}


// modetype is of modetype_e
range_t  VID_ModeRange( byte modetype )
{
    range_t  mrange = { 1, 1 };
    // INITIAL_WINDOW mode 0 is not included
    if(modetype == MODE_fullscreen)
    {   // fullscreen  2..
        mrange.first = NUMVGAVIDMODES;
        mrange.last = mrange.first + num_full_vidmodes-1;
    }
    else
    {   // window   1..,
	      // does not include mode 0
        mrange.last = num_all_vidmodes;
    }
    return mrange;
}


//added:21-03-98: return info on video mode
char *VID_ModeInfo (modenum_t modenum, char **ppheader)
{
    static char *badmodestr = "Bad video mode number\n";
    vmode_t     *pv;

    pv = VID_GetModePtr (modenum);

    if (!pv)
    {
        if (ppheader)
            *ppheader = NULL;
        return badmodestr;
    }
    else
    {
        //if (ppheader)
        //    *ppheader = pv->header;
        return pv->name;
    }
}



//added:03-02-98: return a video mode number from the dimensions
modenum_t  VID_GetModeForSize( int rw, int rh, byte rmodetype )
{
    modenum_t  modenum = { MODE_NOP, 0 };
    int tdist;
    int bestdist = MAXINT;
		int mi = 1;
    vmode_t * pv = all_vidmodes;

    if( rmodetype == MODE_fullscreen )
    {
        if( num_full_vidmodes == 0 )  goto done;
        mi = NUMVGAVIDMODES;  // fullscreen modes start after
        pv = full_vidmodes;
				
    }
		modenum.modetype = rmodetype;
    for ( ; pv!=NULL; pv=pv->next )
    {
        tdist = abs(pv->width - rw) + abs(pv->height - rh);
        // find closest dist
        if( bestdist > tdist )
        {
	    bestdist = tdist;
	    modenum.index = mi;
			
	    if( tdist == 0 )  break;   // found exact match
	}
	mi++;
    }

done:
    return modenum;
}


/* ======================================================================== */
//
/* ======================================================================== */
// only called once
void VID_Init (void)
{
    COM_AddCommand ("vid_nummodes", VID_Command_NumModes_f);
    COM_AddCommand ("vid_modeinfo", VID_Command_ModeInfo_f);
    COM_AddCommand ("vid_modelist", VID_Command_ModeList_f);
    COM_AddCommand ("vid_mode", VID_Command_Mode_f);
}

// Get Fullscreen, VESA modes, append to VGA window modes
void VID_GetModes(void)
{
    // setup the videmodes list,
    // note that mode 0 must always be VGA mode 0x13
    full_vidmodes = NULL;
    currentmode_p = NULL;
    num_full_vidmodes = 0;
    // setup the vesa_modes list
    VID_VesaGetExtraModes ();

    // the game boots in 320x200 standard VGA, but
    // we need a highcolor mode to run the game in highcolor
    if (highcolor && num_full_vidmodes==0)
    {
        I_SoftError ("No highcolor VESA2 video mode found, cannot run in highcolor.\n");
        highcolor = 0;
        VID_VesaGetExtraModes ();
    }
}


// ========================================================================
// Returns a vmode_t from the video modes list, given a video mode number.
// ========================================================================
static
vmode_t *VID_GetModePtr (modenum_t modenum)
{
    // first mode in all_vidmodes is the HIDDEN INITIAL_WINDOW
    vmode_t *pv = all_vidmodes;  // window
    int mi = modenum.index-1;      // 0..
    byte TextStartup=1;
		
    if ( modenum.modetype == 0 )
    {
        pv = full_vidmodes;
        mi = modenum.index - NUMVGAVIDMODES;  // 2..
    }


    if (!pv || mi < 0 ) 
		{
			if (TextStartup=0)
			{
			  I_SoftError("VID_error 1\n");
			  goto fail;
			}
		}

    while (mi--)
    {
        pv = pv->next;			
        if (!pv)
				{
          //I_SoftError("VID_error 2\n");					
          goto fail;					
				}
    }
    return pv;
fail:
    return NULL;
}


//added:30-01-98:return the name of a video mode
char * VID_GetModeName( modenum_t modenum )
{
    // fullscreen and window modes  1..
    vmode_t *pv = VID_GetModePtr(modenum);
    return (pv)? pv->name : NULL;
}


// ========================================================================
// Sets a video mode
// ========================================================================
// Returns FAIL_end, FAIL_create, of status_return_e, 1 on success;
int VID_SetMode (modenum_t modenum)
{
    int     stat;
    vmode_t *newmode_p, *oldmode_p;
    viddef_t oldvid = vid;   // to back out
    boolean  set_fullscreen = (modenum.modetype == MODE_fullscreen);
    range_t  range = VID_ModeRange( modenum.modetype );


    if ((modenum.index > range.last) || (modenum.index < range.first))
    {
        if (currentmode_p == NULL)
        {
	    modenum.index = 0;    // revert to the default base vid mode
        }
        else
        {
	    I_SoftError ("Unknown video mode\n");
	    return  FAIL_end;
        }
    }

    newmode_p = VID_GetModePtr (modenum);
    if( newmode_p == NULL )
       return  FAIL_end;

    // dont switch to the same display mode
    if (newmode_p == currentmode_p)   goto done;

    // initialize the new mode
    oldmode_p = currentmode_p;
    currentmode_p = newmode_p;

    // initialize vidbuffer size for setmode
    vid.width  = currentmode_p->width;
    vid.height = currentmode_p->height;
    vid.bytepp = currentmode_p->bytesperpixel;
    // vid.bitpp = (vid.bytepp==1)? 8:15;

	
    // Using Updated Commandline
    if( req_drawmode == REQ_highcolor)
    {
        vid.bitpp = 15; vid.drawmode = DRAW15;
    }
    else if( req_drawmode == REQ_truecolor)
    {
        vid.bitpp = 32; vid.drawmode = DRAW32;
    }
    else if( req_drawmode == REQ_specific)
    {
        switch(req_bitpp)
        {
          case 15: vid.bitpp = 15; vid.drawmode = DRAW15;  break;						
          case 16: vid.bitpp = 16; vid.drawmode = DRAW16;  break;						
          case 24: vid.bitpp = 24; vid.drawmode = DRAW24;  break;
          case 32: vid.bitpp = 32; vid.drawmode = DRAW32;  break;
          default: vid.bitpp = 8;  vid.drawmode = DRAW8PAL;break;
        }
    }
    else
    {
        vid.bitpp = 8; vid.drawmode = DRAW8PAL;
    }
		
    /* Nicht die 3 weiteren Zeilen entfernen. Sonst stürzt das Bild ab.*/
    vid.direct_rowbytes = currentmode_p->rowbytes;
    vid.widthbytes = vid.width * vid.bytepp;
    vid.direct_size = vid.direct_rowbytes * vid.height;			
		
    stat = (*currentmode_p->setmode_func) (&vid, currentmode_p);
      // sets vid.direct, vid.buffer, vid.display, vid.ybytes, vid.screen_size, vid.screen1
    if (stat < 0)
    {
        if (stat == FAIL_create)
        {
            // hardware could not setup mode
            //if (!VID_SetMode (vid.modenum))
            //    I_Error ("VID_SetMode: couldn't set video mode (hard failure)");
            I_SoftError("Couldn't set video mode %d\n", modenum);
        }
        else
        if (stat == FAIL_memory)
        {
            I_SoftError ("Not enough mem for VID_SetMode...\n");
	}
        // not enough memory; just put things back the way they were
        currentmode_p = oldmode_p;
        // cannot just copy oldvid because of buffer pointers that
        // are no longer valid
        vid.width  = oldvid.width;
        vid.height = oldvid.height;
        vid.bytepp = oldvid.bytepp;
        vid.bitpp = oldvid.bitpp;
        (*currentmode_p->setmode_func) (&vid, currentmode_p);
        return FAIL_create;
    }

    //vid.widthbytes = vid.width * vid.bytepp;
    //vid.drawmode = (vid.bytepp==1)? DRAW8PAL:DRAW15;
    vid.direct_rowbytes = currentmode_p->rowbytes;
    vid.direct_size = vid.direct_rowbytes * vid.height;
    vid.modenum = modenum;
    vid.fullscreen = set_fullscreen;

#ifdef DEBUG
    CONS_Printf("after VID_SetMode\n");
    CONS_Printf("vid.width    %d\n",vid.width);
    CONS_Printf("vid.height   %d\n",vid.height);
    CONS_Printf("vid.buffer   %x\n",vid.buffer);
    CONS_Printf("vid.rowbytes %d\n",vid.rowbytes);
    CONS_Printf("vid.numpages %d\n",vid.numpages);
    CONS_Printf("vid.recalc   %d\n",vid.recalc);
    CONS_Printf("vid.direct   %x\n",vid.direct);
#endif
    //debug
    //if (vid.rowbytes != vid.width)
    //    I_Error("vidrowbytes (%d) <> vidwidth(%d)\n",vid.rowbytes,vid.width);

    //printf ("%s\n", VID_ModeInfo (vid.modenum, NULL));

    //added:20-01-98: recalc all tables and realloc buffers based on
    //                vid values.
    vid.recalc = 1;

 done:
    return 1;
}



// converts a segm:offs 32bit pair to a 32bit flat ptr
void *VID_ExtraFarToLinear (void *ptr)
{
    int     temp;

    temp = (int)ptr;
    return real2ptr (((temp & 0xFFFF0000) >> 12) + (temp & 0xFFFF));
}



// ========================================================================
// Helper function for VID_VesaGetExtraModes
// In:  vesa mode number, from the vesa videomodenumbers list
// Out: false, if no info for given modenum
// ========================================================================
int VID_VesaGetModeInfo (int modenum)
{
	
    int     bytes_per_pixel;
    int     i;
    __dpmi_regs regs;

    for (i=0; i<sizeof(vesamodeinfo_t); i++)
        _farpokeb(_dos_ds, MASK_LINEAR(__tb)+i, 0);

    regs.x.ax = 0x4f01;
    regs.x.di = RM_OFFSET(__tb);
    regs.x.es = RM_SEGMENT(__tb);
    regs.x.cx = modenum;
    __dpmi_int(0x10, &regs);
    if (regs.h.ah)
        return false;
    else
    {
        dosmemget (MASK_LINEAR(__tb), sizeof(vesamodeinfo_t), &vesamodeinfo);

        bytes_per_pixel = (vesamodeinfo.BitsPerPixel+1)/8;

       // we add either highcolor or lowcolor video modes, not all
       if (vesamodeinfo.BitsPerPixel != BitsColor)
       return false;						
       /*
        if (highcolor && (vesamodeinfo.BitsPerPixel != 15))
            return false;
        if (!highcolor && (vesamodeinfo.BitsPerPixel != 8))
            return false;
       */
        if ((bytes_per_pixel > 4/*2*/) ||
            (vesamodeinfo.XResolution > MAXVIDWIDTH) ||
            (vesamodeinfo.YResolution > MAXVIDHEIGHT))
        {
            return false;
        }

        // we only want color graphics modes that are supported by the hardware
        if ((vesamodeinfo.ModeAttributes &
            (MODE_SUPPORTED_IN_HW | COLOR_MODE | GRAPHICS_MODE) ) !=
             (MODE_SUPPORTED_IN_HW | COLOR_MODE | GRAPHICS_MODE))
        {
            return false;
        }

        // we only work with linear frame buffers, except for 320x200,
        // which is linear when banked at 0xA000
        if (!(vesamodeinfo.ModeAttributes & LINEAR_FRAME_BUFFER))
        {
            if ((vesamodeinfo.XResolution != 320) ||
                (vesamodeinfo.YResolution != 200))
            {
                return false;
            }
        }

        // pagesize
        if ((vesamodeinfo.BytesPerScanLine * vesamodeinfo.YResolution)
             > totalvidmem)
        {
            return false;
        }

        vesamodeinfo.NumberOfImagePages = 1;


#ifdef DEBUG
        modeinfo_t modeinfo;
        CONS_Printf("VID: (VESA) info for mode 0x%x\n", modeinfo.modenum);
        CONS_Printf("  mode attrib        = 0x%0x\n", modeinfo.mode_attributes);
        /*CONS_Printf("  win a attrib       = 0x%0x\n", *(unsigned char*)(infobuf+2));*/
        /*CONS_Printf("  win b attrib       = 0x%0x\n", *(unsigned char*)(infobuf+3));*/
        CONS_Printf("  win a seg          = 0x%0x\n", (int) modeinfo.winasegment);
        CONS_Printf("  win b seg          = 0x%0x\n", (int) modeinfo.winbsegment);
        CONS_Printf("  bytes per scanline = %d\n",modeinfo.bytes_per_scanline);
        CONS_Printf("  width = %d, height = %d\n", modeinfo.width,modeinfo.height);
        CONS_Printf("  win                = %c\n", 'A' + modeinfo.win);
        CONS_Printf("  win granularity    = %d\n", modeinfo.granularity);
        CONS_Printf("  win size           = %d\n", modeinfo.win_size);
        CONS_Printf("  bits per pixel     = %d\n", modeinfo.bits_per_pixel);
        CONS_Printf("  bytes per pixel    = %d\n", modeinfo.bytes_per_pixel);
        CONS_Printf("  memory model       = 0x%x\n", modeinfo.memory_model);
        CONS_Printf("  num pages          = %d\n", modeinfo.num_pages);
        CONS_Printf("  red width          = %d\n", modeinfo.red_width);
        CONS_Printf("  red pos            = %d\n", modeinfo.red_pos);
        CONS_Printf("  green width        = %d\n", modeinfo.green_width);
        CONS_Printf("  green pos          = %d\n", modeinfo.green_pos);
        CONS_Printf("  blue width         = %d\n", modeinfo.blue_width);
        CONS_Printf("  blue pos           = %d\n", modeinfo.blue_pos);
        CONS_Printf("  phys mem           = %x\n", modeinfo.pptr);				
#endif
    }

    return true;
}


// ========================================================================
// Get extended VESA modes information, keep the ones that we support,
// so they'll be available in the game Video menu.
// ========================================================================
#define MAXVESADESC 100
static char vesadesc[MAXVESADESC] = "";

void VID_VesaGetExtraModes (void)
{
    int             i;
    unsigned long   addr;
    int             nummodes;
    __dpmi_meminfo  phys_mem_info;
    unsigned long   mode_ptr;
    __dpmi_regs     regs;
    char VesaResolution[12];

    // make a copy of the video modes list! else trash in __tb
    unsigned short  vmode[MAX_VESA_MODES+1];
    int             numvmodes;
    unsigned short  vesamode;

    // new ugly stuff...
    for (i=0; i<sizeof(vbeinfoblock_t); i++)
       _farpokeb(_dos_ds, MASK_LINEAR(__tb)+i, 0);

    dosmemput("VBE2", 4, MASK_LINEAR(__tb));

    // see if VESA support is available
    regs.x.ax = 0x4f00;
    regs.x.di = RM_OFFSET(__tb);
    regs.x.es = RM_SEGMENT(__tb);
    __dpmi_int(0x10, &regs);
    if (regs.h.ah)
        goto no_vesa;

    dosmemget(MASK_LINEAR(__tb), sizeof(vbeinfoblock_t), &vesainfo);

    if (strncmp(vesainfo.VESASignature, "VESA", 4) != 0)
    {
no_vesa:
        CONS_Printf ("No VESA driver\n");
        return;
    }

    if (vesainfo.VESAVersion < (VBEVERSION<<8))
    {
        CONS_Printf ("VESA VBE %d.0 not available\n", VBEVERSION);
        return;
    }

    //
    // vesa version number
    //
    CONS_Printf ("%4.4s %d.%d (", vesainfo.VESASignature,
                                  vesainfo.VESAVersion>>8,
                                  vesainfo.VESAVersion&0xFF);

    //
    // vesa description string
    //
    i = 0;
    addr = RM_TO_LINEAR(vesainfo.OemStringPtr);
    _farsetsel(_dos_ds);
    while (_farnspeekb(addr) != 0)
    {
        vesadesc[i++] = _farnspeekb(addr++);
        if (i == MAXVESADESC-1)
            break;
    }
    vesadesc[i]=0;
    CONS_Printf ("%s)\n",vesadesc);

    totalvidmem = vesainfo.TotalMemory << 16;
    nummodes  = 0;
    numvmodes = 0;		
    mode_ptr = RM_TO_LINEAR(vesainfo.VideoModePtr);

    while (nummodes < MAX_VESA_MODES)
    {
       vesamode = _farpeekw(_dos_ds, mode_ptr);
    
       // Sicherheitschecks – das ist der Trick!
       if (vesamode == 0xFFFF)        // normales Ende
           break;
       if (vesamode < 0x100)           // ungültig (unter 0x100 = Textmodi)
           break;
       if (mode_ptr >= RM_TO_LINEAR(vesainfo.VideoModePtr) + 512)
           break;                      // Liste zu lang ? Abbruch (Schutz!)

       vmode[numvmodes++] = vesamode;
       mode_ptr += 2;
    }
    vmode[numvmodes] = 0xFFFF;  // sauber abschließen

    nummodes  = 0;  // number of video modes accepted for the game	
    numvmodes = 0;  // go again through vmodes table
    while ( ((vesamode=vmode[numvmodes++]) != 0xFFFF) && (nummodes < MAX_VESA_MODES) )
    {		
  
        //fill the modeinfo struct.
        if (VID_VesaGetModeInfo (vesamode))
        {

           // ================================================================
           // 2. Doppelte Auflösungen entfernen (egal welches BPP – einfach den ersten behalten)
           // ================================================================
           if (nummodes > 1)
           {
             int write_idx = 1;  // wo wir hinschreiben

             for (i = 1; i < nummodes; i++)
             {
               // Wenn Auflösung anders ist als die vorherige ? behalten
               if (vesa_modes[i].width  != vesa_modes[write_idx-1].width || vesa_modes[i].height != vesa_modes[write_idx-1].height)
               {
                   vesa_modes[write_idx++] = vesa_modes[i];			      			
               }
               // sonst: gleiche Auflösung ? verwerfen (egal welches BPP)
              }
              nummodes = write_idx;  // neue Anzahl
            }

            vesa_modes[nummodes].next = &vesa_modes[nummodes+1];
            if (vesamodeinfo.XResolution > 999)
            {
                if (vesamodeinfo.YResolution > 999)
                {
                    sprintf (&names[nummodes][0], "%4dx%4d", vesamodeinfo.XResolution,
                                                             vesamodeinfo.YResolution);
                    names[nummodes][9] = 0;
                }
                else
                {
                    sprintf (&names[nummodes][0], "%4dx%3d", vesamodeinfo.XResolution,
                                                             vesamodeinfo.YResolution);
                    names[nummodes][8] = 0;
                }
            }
            else
            {
                if (vesamodeinfo.YResolution > 999)
                {
                    sprintf (&names[nummodes][0], "%3dx%4d", vesamodeinfo.XResolution,
                                                             vesamodeinfo.YResolution);
                    names[nummodes][8] = 0;
                }
                else
                {
                    sprintf (&names[nummodes][0], "%3dx%3d", vesamodeinfo.XResolution,
                                                             vesamodeinfo.YResolution);
                    names[nummodes][7] = 0;
                }
            }

            vesa_modes[nummodes].name = &names[nummodes][0];
            vesa_modes[nummodes].width = vesamodeinfo.XResolution;
            vesa_modes[nummodes].height = vesamodeinfo.YResolution;

            //added:20-01-98:aspect ratio to be implemented...
            vesa_modes[nummodes].rowbytes = vesamodeinfo.BytesPerScanLine;
            vesa_modes[nummodes].modetype = MODE_fullscreen;
            vesa_modes[nummodes].extradata = &vesa_extra[nummodes];
            vesa_modes[nummodes].setmode_func = VID_VesaInitMode; /* Nicht entfernen. Crasht */

            if (vesamodeinfo.ModeAttributes & LINEAR_FRAME_BUFFER)
            {
            // add linear bit to mode for linear modes
                vesa_extra[nummodes].vesamode = vesamode | LINEAR_MODE;
                vesa_modes[nummodes].numpages = 1; //vesamodeinfo.NumberOfImagePages;

                phys_mem_info.address = (int)vesamodeinfo.PhysBasePtr;
                phys_mem_info.size = 0x400000;

                // returns -1 on error
                if (__dpmi_physical_address_mapping(&phys_mem_info))
                {
                    //skip this mode, it doesnt work
                    continue;
                }

                // if physical mapping was ok... convert the selector:offset
                vesa_extra[nummodes].linearmem =
                           real2ptr (phys_mem_info.address);

                // lock the region
                __dpmi_lock_linear_region (&phys_mem_info);
            }
            else
            {
            // banked at 0xA0000
                vesa_extra[nummodes].vesamode = vesamode;
                //vesa_extra[nummodes].pages[0] = 0;
                vesa_extra[nummodes].linearmem =
                           real2ptr (vesamodeinfo.WinASegment<<4);

                vesa_modes[nummodes].numpages = 1; //modeinfo.numpages;
            }

            vesa_modes[nummodes].bytesperpixel = (vesamodeinfo.BitsPerPixel+1)/8;
            
#ifdef DEBUG
            sprintf (VesaResolution, "%dx%dx%d", vesamodeinfo.XResolution, vesamodeinfo.YResolution, vesamodeinfo.BitsPerPixel);						
            GenPrintf( EMSG_info,  " - Added Vesa Mode: %-2d - %s\n",nummodes, VesaResolution);
#endif											
            nummodes++;
        }
    }

    if (nummodes)
    {
        // append the vesa modes to the modelist
        vesa_modes[nummodes-1].next = NULL; //full_vidmodes;
        append_full_vidmodes( &vesa_modes[0], nummodes );
    }

}


// ========================================================================
// Free the video buffer of the last video mode,
// allocate a new buffer for the video mode to set.
// ========================================================================
boolean VID_FreeAndAllocVidbuffer (viddef_t *lvid)
{
    int  vidbuffersize;

    // Must agree with FinishUpdate, which uses VID_BlitLinearScreen
#if 1 
    // screen size same as video buffer, simple copy
    lvid->ybytes = lvid->direct_rowbytes;
    lvid->screen_size = lvid->direct_size;
#else
    // minimal screen buffer, must copy by line (VID_BlitLinearScreen)
    lvid->ybytes = lvid->widthbytes;
    lvid->screen_size = lvid->ybytes * lvid->height;
#endif
    vidbuffersize = (lvid->screen_size * NUMSCREENS);

    // free allocated buffer for previous video mode
    if (lvid->buffer!=NULL)
    {
        free(lvid->buffer);
    }

    // allocate the new screen buffer
    lvid->buffer = (byte *) malloc(vidbuffersize);
    lvid->display = lvid->buffer;  // display = buffer, screen[0]
    if( lvid->buffer == NULL )
    {
        lvid->screen1 = NULL;
        return false;
    }
    lvid->screen1 = lvid->buffer + lvid->screen_size;

    // initially clear the video buffer
    memset (lvid->buffer, 0, vidbuffersize);

#ifdef DEBUG
 CONS_Printf("VID_FreeAndAllocVidbuffer done, vidbuffersize: %x\n",vidbuffersize);
#endif
    return true;
}


// ========================================================================
// Set video mode routine for STANDARD VGA MODES (NO HIGHCOLOR)
// Out: 1 ok, negative FAIL codes
// ========================================================================
static
int VGA_InitMode (viddef_t *lvid, vmode_t *currentmode_p)
{
    __dpmi_regs   regs;

    if (!VID_FreeAndAllocVidbuffer (lvid))
       return FAIL_memory;  //no mem

    //added:26-01-98: should clear video mem here

    //set mode 0x13
    regs.h.ah = 0;
    regs.h.al = 0x108/*0x13*/; // Benutze 0x108 (640x480 Textmodus als Start)
    __dpmi_int(0x10, &regs);

    // here it is the standard VGA 64k window, not an LFB
    // (you could have 320x200x256c with LFB in the vesa modes)
    lvid->direct = (byte *) real2ptr (0xa0000);
    lvid->numpages = 1;
    lvid->bytepp = currentmode_p->bytesperpixel+1;

    return 1;
}

// ========================================================================
// Set video mode routine for VESA video modes, see VID_SetMode()
// Out: 1 ok, FAIL errors (negative)
// ========================================================================
int VID_VesaInitMode (viddef_t *lvid, vmode_t *currentmode_p)
{
    vesa_extra_t    *extra;
    __dpmi_regs     regs;

    extra = currentmode_p->extradata;

#ifdef DEBUG
 CONS_Printf("\n\n=== VID_VesaInitMode DEBUG Mode: ===\n");
 CONS_Printf("currentmode_p->Name %s\n",currentmode_p->name);
 CONS_Printf(" Width   : %d\n",currentmode_p->width);
 CONS_Printf(" Height  : %d\n",currentmode_p->height);
 CONS_Printf(" Rowbytes: %d\n",currentmode_p->rowbytes);
 CONS_Printf(" Numpages: %d\n",currentmode_p->numpages); 
 switch(currentmode_p->modetype)
 {
   case 0:CONS_Printf(" ModeType: %d [MODE_NOP]\n",currentmode_p->modetype);break;
   case 1:CONS_Printf(" ModeType: %d [MODE_window]\n",currentmode_p->modetype);break;
   case 2:CONS_Printf(" ModeType: %d [MODE_fullscreen]\n",currentmode_p->modetype);break;
   case 3:CONS_Printf(" ModeType: %d [MODE_voodoo]\n",currentmode_p->modetype);break;
   case 4:CONS_Printf(" ModeType: %d [MODE_either]\n",currentmode_p->modetype);break;
   case 5:CONS_Printf(" ModeType: %d [MODE_other]\n",currentmode_p->modetype);break;		
 } 
 switch(currentmode_p->bytesperpixel)
 {
	 case 1:CONS_Printf(" BytesPerPixel %d [8bit]\n",currentmode_p->bytesperpixel);break;
   case 2:CONS_Printf(" BytesPerPixel %d [15/16bit]\n",currentmode_p->bytesperpixel);break;
   case 3:CONS_Printf(" BytesPerPixel %d [24bit]\n",currentmode_p->bytesperpixel);break;
   case 4:CONS_Printf(" BytesPerPixel %d [32bit]\n",currentmode_p->bytesperpixel);break;		
 } 

 CONS_Printf("currentmode_p->Extradata :\n");
 CONS_Printf(" ->VesaMode  = %x\n",extra->vesamode);
 CONS_Printf(" ->LinearMEM = %x\n\n",extra->linearmem);
#endif

    //added:20-01-98:no page flipping now... TO DO!!!
    lvid->numpages = 1;

    // clean up any old vid buffer lying around, alloc new if needed
    if (!VID_FreeAndAllocVidbuffer (lvid))
       return FAIL_memory;                  //no mem


    //added:20-01-98: should clear video mem here


    // set the mode
    regs.x.ax = 0x4f02;
    regs.x.bx = extra->vesamode;
    __dpmi_int (0x10, &regs);

    if (regs.x.ax != 0x4f)
        return FAIL_create;               // could not set mode

//added:20-01-98: should setup wait_vsync flag, currentpage here...
//                plus check for display_enable bit

//added:20-01-98: here we should set the page if page flipping...

    // points to LFB, or the start of VGA mem.
    lvid->direct = extra->linearmem;
    lvid->bytepp = currentmode_p->bytesperpixel;

    return 1;
}

// ========================================================================
//                     VIDEO MODE CONSOLE COMMANDS
// ========================================================================


//  vid_nummodes
//
//added:21-03-98:
void VID_Command_NumModes_f (void)
{
     range_t  mr = VID_ModeRange( MODE_fullscreen );
     GenPrintf( EMSG_info, "Video modes %d to %d available(s)\n", mr.first, mr.last );		 
}


//  vid_modeinfo <modenum>
//
void VID_Command_ModeInfo_f (void)
{
    range_t  mr = VID_ModeRange( MODE_fullscreen );
    modenum_t   mn = {MODE_fullscreen, 0};
    vmode_t     *pv;

    if (COM_Argc()!=2)
        mn = vid.modenum;          // describe the current mode
    else
       mn.index = atoi (COM_Argv(1));   //    .. the given mode number

    if (mn.index > mr.last || mn.index < mr.first ) //faB: dont accept the windowed mode 0
    {
        GenPrintf( EMSG_warn, "No such video mode\n");
        return;
    }

    pv = VID_GetModePtr(mn);
    VID_GetModeInfo_c(pv,mn);

}


//  vid_modelist
//
void VID_Command_ModeList_f (void)
{
    range_t  mr = VID_ModeRange( MODE_fullscreen );
    modenum_t   mn = {MODE_fullscreen, 0};
    int         i;
    char        *modename;
    char        *attr_str;
		vmode_t     *pv;
    boolean     na;
    
		na = false;

    for (i=mr.first ; i<=mr.last ; i++)
    {			  
	      mn.index = i;
				modename = VID_GetModeName (mn);
        pv = VID_GetModePtr (mn);
        if( pv )
				{
			  /*
			  attr_str = (pv->bytesperpixel==1)? " (hicolor)" : "";
				GenPrintf( EMSG_info, "%d: %s%s\n", i, modename, attr_str);
				*/
        if (i==0 || pv->bytesperpixel==1)
            GenPrintf( EMSG_info, "%d: %s\n", i, modename);
        else if (pv->bytesperpixel==2)
            GenPrintf( EMSG_info, "%d: %s (Highcolor 15/16Bit)\n", i, modename);
        else if (pv->bytesperpixel==3)
            GenPrintf( EMSG_info, "%d: %s (Truecolor 24Bit)\n", i, modename);
        else if (pv->bytesperpixel==4)
            GenPrintf( EMSG_info, "%d: %s (Truecolor 32Bit)\n", i, modename);
				}
    }

}


//  vid_mode <modenum>
//
void VID_Command_Mode_f (void)
{	  
    range_t  mr = VID_ModeRange( MODE_fullscreen );
    modenum_t   mn = {MODE_fullscreen, 0};
    vmode_t *pv;
		
    if (COM_Argc()!=2)
    {
        GenPrintf( EMSG_warn, "vid_mode <modenum> : set video mode\n");
        return;
    }

    mn.index = atoi(COM_Argv(1));
    if (mn.index > mr.last || mn.index < mr.first ) //faB: dont accept the windowed mode 0
        GenPrintf( EMSG_warn, "No such video mode\n");
    else
    {
        // request vid mode change
        setmodeneeded = mn;
        pv = VID_GetModePtr(mn);
        VID_GetModeInfo_c(pv,mn);
    }
}

void VID_GetModeInfo_c(vmode_t *pv, modenum_t mn)
{
	
    CONS_Printf ("Current Screen Resolution Mode(%d): %s\n", mn.index,VID_ModeInfo (mn, NULL));
    CONS_Printf ("- Width : %d\n"
                 "- Height: %d\n",
                 pv->width,
                 pv->height);

    switch (pv->bytesperpixel)
    {
       case 0: CONS_Printf ("- Bytes per Pixel %d (%1dBit Mode)\n",pv->bytesperpixel,BitsColor); break;
       case 1: CONS_Printf ("- Bytes per Pixel %d (%1dBit Mode)\n",pv->bytesperpixel,BitsColor); break;
       case 2:
       {
          if (BitsColor==15)
              CONS_Printf ("- Bytes per Pixel %d (%dBit Mode)\n",pv->bytesperpixel,BitsColor);
          else
              CONS_Printf ("- Bytes per Pixel %d (%dBit Mode)\n",pv->bytesperpixel,BitsColor);
          break;
       }
       case 3: CONS_Printf ("- Bytes per Pixel %d (%dBit Mode)\n",pv->bytesperpixel,BitsColor); break;
       case 4: CONS_Printf ("- Bytes per Pixel %d (%dBit Mode)\n",pv->bytesperpixel,BitsColor); break;
    }

    CONS_Printf ("- Bytes per Scanline: %d\n"
                 "- Numpages: %d\n",
                 pv->rowbytes,
                 pv->numpages);
}
/* ========================================================================
   Folgendes wurde mit svn 1069 entfernt und von mir wieder eingefügt damit
 	 der Code funktioniert. Fixed.
   ========================================================================
	 
   added:30-01-98: return number of video modes in pvidmodes list	 
*/
static inline
int VID_NumModes(void)
{
  return ( vid.fullscreen )? num_full_vidmodes : num_all_vidmodes;
}
