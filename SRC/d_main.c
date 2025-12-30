// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: d_main.c 1423 2019-01-29 08:06:47Z wesleyjohnson $
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2016 by DooM Legacy Team.
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
// $Log: d_main.c,v $
// Include: DOS DJGPP Fixes
//
// Revision 1.66  2005/12/20 14:58:25  darkwolf95
// Monster behavior CVAR - Affects how monsters react when they shoot each other
//
// Revision 1.65  2004/07/27 08:19:34  exl
// New fmod, fs functions, bugfix or 2, patrol nodes
//
// Revision 1.64  2004/04/20 00:34:26  andyp
// Linux compilation fixes and string cleanups
//
// Revision 1.63  2003/11/21 17:52:05  darkwolf95
// added "Monsters Infight" for Dehacked patches
//
// Revision 1.62  2003/07/23 17:20:37  darkwolf95
// Initial Chex Quest 1 Support
//
// Revision 1.61  2003/07/14 21:22:23  hurdler
// Revision 1.60  2003/07/13 13:16:15  hurdler
//
// Revision 1.59  2003/05/04 02:28:34  sburke
// Fix for big-endian machines.
//
// Revision 1.58  2002/12/13 22:34:27  ssntails
// MP3/OGG support!
//
// Revision 1.57  2002/09/27 16:40:08  tonyd
// First commit of acbot
//
// Revision 1.56  2002/09/17 21:20:02  hurdler
// Quick hack for hacx freeze
//
// Revision 1.55  2002/08/24 22:42:02  hurdler
// Apply Robert Hogberg patches
//
// Revision 1.54  2002/07/26 15:21:36  hurdler
//
// Revision 1.53  2001/12/31 16:56:39  metzgermeister
// see Dec 31 log
//
// Revision 1.52  2001/08/20 20:40:39  metzgermeister
//
// Revision 1.51  2001/08/12 15:21:03  bpereira
// see my log
//
// Revision 1.50  2001/07/16 22:35:40  bpereira
// - fixed crash of e3m8 in heretic
// - fixed crosshair not drawed bug
//
// Revision 1.49  2001/05/27 13:42:47  bpereira
// Revision 1.48  2001/05/16 21:21:14  bpereira
//
// Revision 1.47  2001/05/16 17:12:52  crashrl
// Added md5-sum support, removed recursiv wad search
//
// Revision 1.46  2001/04/27 13:32:13  bpereira
//
// Revision 1.45  2001/04/17 22:26:07  calumr
// Initial Mac add
//
// Revision 1.44  2001/04/04 20:24:21  judgecutor
// Added support for the 3D Sound
//
// Revision 1.43  2001/04/02 18:54:32  bpereira
// Revision 1.42  2001/04/01 17:35:06  bpereira
// Revision 1.41  2001/03/30 17:12:49  bpereira
//
// Revision 1.40  2001/03/19 18:25:02  hurdler
// Is there a GOOD reason to check for modified game with shareware version?
//
// Revision 1.39  2001/03/03 19:43:09  ydario
// OS/2 code cleanup
//
// Revision 1.38  2001/02/24 13:35:19  bpereira
// Revision 1.37  2001/02/10 12:27:13  bpereira
//
// Revision 1.36  2001/01/25 22:15:41  bpereira
// added heretic support
//
// Revision 1.35  2000/11/06 20:52:15  bpereira
// Revision 1.34  2000/11/03 03:27:17  stroggonmeth
// Revision 1.33  2000/11/02 19:49:35  bpereira
//
// Revision 1.32  2000/11/02 17:50:06  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.31  2000/10/21 08:43:28  bpereira
// Revision 1.30  2000/10/08 13:29:59  bpereira
// Revision 1.29  2000/10/02 18:25:44  bpereira
// Revision 1.28  2000/10/01 10:18:16  bpereira
// Revision 1.27  2000/09/28 20:57:14  bpereira
// Revision 1.26  2000/08/31 14:30:55  bpereira
//
// Revision 1.25  2000/08/29 15:53:47  hurdler
// Remove master server connect timeout on LAN (not connected to Internet)
//
// Revision 1.24  2000/08/21 21:13:00  metzgermeister
// Implementation of I_GetKey() in Linux
//
// Revision 1.23  2000/08/10 14:50:19  ydario
// OS/2 port
//
// Revision 1.22  2000/05/07 08:27:56  metzgermeister
// Revision 1.21  2000/04/30 10:30:10  bpereira
//
// Revision 1.20  2000/04/25 19:49:46  metzgermeister
// support for automatic wad search
//
// Revision 1.19  2000/04/24 20:24:38  bpereira
// Revision 1.18  2000/04/23 16:19:52  bpereira
//
// Revision 1.17  2000/04/22 20:27:35  metzgermeister
// support for immediate fullscreen switching
//
// Revision 1.16  2000/04/21 20:04:20  hurdler
// fix a problem with my last SDL merge
//
// Revision 1.15  2000/04/19 15:21:02  hurdler
// add SDL midi support
//
// Revision 1.14  2000/04/18 12:55:39  hurdler
// Revision 1.13  2000/04/16 18:38:07  bpereira
//
// Revision 1.12  2000/04/07 23:10:15  metzgermeister
// fullscreen support under X in Linux
//
// Revision 1.11  2000/04/06 20:40:22  hurdler
// Mostly remove warnings under windows
//
// Revision 1.10  2000/04/05 15:47:46  stroggonmeth
// Added hack for Dehacked lumps. Transparent sprites are now affected by colormaps.
//
// Revision 1.9  2000/04/04 00:32:45  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.8  2000/03/29 19:39:48  bpereira
//
// Revision 1.7  2000/03/28 16:18:41  linuxcub
// Added a command to the Linux sound-server which sets a master volume.
// Added code to the main parts of doomlegacy which uses this command to
// implement volume control for sound effects.
//
// Added code so the (really cool) cd music works for me. The volume didn't
// work for me (with a Teac 532E drive): It always started at max (31) no-
// matter what the setting in the config-file was. The added code "jiggles"
// the volume-control, and now it works for me :-)
// If this code is unacceptable, perhaps another solution is to periodically
// compare the cd_volume.value with an actual value _read_ from the drive.
// Ie. not trusting that calling the ioctl with the correct value actually
// sets the hardware-volume to the requested value. Right now, the ioctl
// is assumed to work perfectly, and the value in cd_volume.value is
// compared periodically with cdvolume.
//
// Updated the spec file, so an updated RPM can easily be built, with
// a minimum of editing. Where can I upload my pre-built (S)RPMS to ?
//
// Erling Jacobsen, linuxcub@email.dk
//
// Revision 1.6  2000/03/23 22:54:00  metzgermeister
// added support for HOME/.legacy under Linux
//
// Revision 1.5  2000/03/06 17:33:36  hurdler
// Revision 1.4  2000/03/05 17:10:56  bpereira
// Revision 1.3  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
//
// DESCRIPTION:
//      DOOM main program (D_DoomMain) and game loop (D_DoomLoop),
//      plus functions to determine game mode (shareware, doom_registered),
//      parse command line parameters, configure game parameters (turbo),
//      and call the startup functions.
//
//-----------------------------------------------------------------------------

#include "doomincl.h"
  // MAX_WADPATH

#ifdef __WIN32__
#include <direct.h>
#else
#include <unistd.h>     // for access
#define _MAX_PATH   MAX_WADPATH
#endif

// using MAX_WADPATH as buffer limit, so _MAX_PATH must be as long
#if _MAX_PATH < MAX_WADPATH
#undef _MAX_PATH
#define _MAX_PATH   MAX_WADPATH
#endif


#include "doomstat.h"

#include "command.h"
#include "console.h"

#include "am_map.h"
#include "d_net.h"
#include "d_netcmd.h"
#include "dehacked.h"
#include "dstrings.h"

#include "f_wipe.h"
#include "f_finale.h"

#include "g_game.h"
#include "g_input.h"

#include "hu_stuff.h"

#include "i_sound.h"
#include "i_system.h"
#include "i_video.h"

#include "m_argv.h"
#include "m_menu.h"
#include "m_misc.h"
#include "m_swap.h"

#include "p_setup.h"
#include "p_fab.h"
#include "p_info.h"
#include "p_local.h"

#include "r_main.h"
#include "r_local.h"

#include "s_sound.h"
#include "st_stuff.h"

#include "t_script.h"

#include "v_video.h"

#include "wi_stuff.h"
#include "w_wad.h"

#include "z_zone.h"
#include "d_main.h"
#include "d_netfil.h"
#include "m_cheat.h"
#include "p_chex.h"

#ifdef HWRENDER
#include "hardware/hw_main.h"
  // 3D View Rendering
#endif

#include "hardware/hw3sound.h"

#include "b_game.h"
  //added by AC for acbot

#ifdef FRENCH_INLINE
#include "d_french.h"
#endif

#if defined( __DJGPP__ )
#define I_Sleep(int) usleep(int);

  // From MakeDOS File
  #ifdef PCDOSi386
  #define DOSNAME "386DX/SX"
  #endif
  #ifdef PCDOSi486
  #define DOSNAME "486DX/SX"
  #endif
  #ifdef PCDOSi586
  #define DOSNAME "Pentium"
  #endif

  #if !defined( PCDOSi386 ) && !defined( PCDOSi486 ) && !defined( PCDOSi586 )
    #define DOSNAME ""
  #endif

  static const char DOSSTRING[] = "[" DOSNAME "]";

  // Proto
  void ListSupportetWads(void);
  void ListSupportetWads_Command(void);  

  char *Posix_Deform_Path(char *path)
  {
    char *p = path;
    for (*p = path[0]; p && *p; p++)
         if (*p == '/') *p = '\\';
    
  return path;
  }

#else
  static const char DOSSTRING[] = "";	
#endif

// Versioning
#ifndef SVN_REV
#define SVN_REV "1421"
#endif


// Version number: major.minor.revision
const int  VERSION  = 147; // major*100 + minor
const int  REVISION = 3;   // for bugfix releases, should not affect compatibility. has nothing to do with svn revisions.
static const char VERSIONSTRING[] = "(Rev " SVN_REV ")";
//static const char VERSIONSTRING[] = "Beta (Rev " SVN_REV ")";
char VERSION_BANNER[80];

// [WDJ] change this if legacy.wad is changed
const short cur_wadversion = 145;	// release wadversion
// usually allow one version behind cur_wadversion, for easier testing
// does not have to be full featured
const short min_wadversion = 144;


//
//  DEMO LOOP
//
int demosequence;
int pagetic;
static const char * pagename = "TITLEPIC";

//  PROTOS
static void Help(void);
static void Clear_SoftError(void);
void Heretic_PatchEngine(void);
//void Chex1_PatchEngine(void);

// Null terminated list of files.
char * startupwadfiles[MAX_WADFILES+1];

// command line switches
boolean devparm = false;        // started game with -devparm
    // devparm enables development mode, with CONS messages reporting
    // on memory usage and other significant events.
boolean nomonsters;             // checkparm of -nomonsters

boolean singletics = false;     // timedemo

boolean nomusic;
boolean nosoundfx; // had clash with WATCOM i86.h nosound() function

boolean dedicated = false;  // dedicated server

byte    verbose = 0;

byte    demo_ctrl;
byte    init_sequence = 0;
byte    fatal_error = 0;

// name buffer sizes including directory and everything
#define FILENAME_SIZE  256

#if defined SMIF_PC_DOS || defined __WIN32__ || defined __OS2__
# define  SLASH  "\\"
#else
# define  SLASH  "/"
#endif

// to make savegamename and directories, in m_menu.c
char *legacyhome = NULL;
int   legacyhome_len;

char *dirlist[] =
  { DEFWADS01,
#ifdef DEFWADS02
    DEFWADS02,
#endif
#ifdef DEFWADS03
    DEFWADS03,
#endif
#ifdef DEFWADS04
    DEFWADS04,
#endif
#ifdef DEFWADS05
    DEFWADS05,
#endif
#ifdef DEFWADS06
    DEFWADS06,
#endif
#ifdef DEFWADS07
    DEFWADS07,
#endif
#ifdef DEFWADS08
    DEFWADS08,
#endif
#ifdef DEFWADS09
    DEFWADS09,
#endif
#ifdef DEFWADS10
    DEFWADS10,
#endif
#ifdef DEFWADS11
    DEFWADS11,
#endif
#ifdef DEFWADS12
    DEFWADS12,
#endif
#ifdef DEFWADS13
    DEFWADS13,
#endif
#ifdef DEFWADS14
    DEFWADS14,
#endif
#ifdef DEFWADS15
    DEFWADS15,
#endif
#ifdef DEFWADS16
    DEFWADS16,
#endif
#ifdef DEFWADS17
    DEFWADS17,
#endif
#ifdef DEFWADS18
    DEFWADS18,
#endif
#ifdef DEFWADS19
    DEFWADS19,
#endif
#ifdef DEFWADS20
    DEFWADS20,
#endif
#ifdef DEFWADS21
    DEFWADS21,
#endif
  };
// Doomwaddir allocation:
// [0] = DOOMWADDIR.  (ref)
// [1] = reserved for dynamic use  (ref)
// [2] = reserved for dynamic use  (ref)
#define DOOMWADDIR_DIRLIST   3
// [3.. (MAX_NUM_DOOMWADDIR - 3)] = DEFWADSxx   (ref or malloc)
// [MAX_NUM_DOOMWADDIR-2] = reserved for dynamic use  (ref)
// [MAX_NUM_DOOMWADDIR-1] = reserved for dynamic use  (ref)
char * doomwaddir[MAX_NUM_DOOMWADDIR];

static byte defdir_stat = 0;  // when defdir valid
static byte defdir_search = 0;  // when defdir search is reasonable
static char * defdir = NULL;  // default dir  (malloc)
static char * progdir = NULL;  // program dir  (malloc)
static char * progdir_wads = NULL;  // program wads directory  (malloc)

#ifdef LAUNCHER
consvar_t cv_home = {"home", "", CV_HIDEN, NULL};
consvar_t cv_doomwaddir = {"doomwaddir", "", CV_HIDEN, NULL};
consvar_t cv_iwad = {"iwad", "", CV_HIDEN, NULL};
#endif


#if defined(__APPLE__) && defined(__MACH__)
// [WDJ] This is very likely only for a development setup using an .app folder
#ifdef EXT_MAC_DIR_SPEC
//[segabor]: for Mac specific resources
extern char mac_legacy_wad[FILENAME_SIZE];  // legacy.wad in Resources
extern char mac_md2_wad[FILENAME_SIZE];	    // md2.wad in Resources
extern char mac_user_home[FILENAME_SIZE];   // for config and savegames
#endif
#endif

// Setup variable doomwaddir for owner usage.
void  owner_wad_search_order( void )
{
    // Wad search order.
    defdir_search = 0;
    if( defdir_stat )
    {
#if !defined( __DJGPP__ )	
        if( ( access( "Desktop", R_OK) == 0 )
          ||( access( "Pictures", R_OK) == 0 )
          ||( access( "Music", R_OK) == 0 ) )
        {
            if( verbose )
                GenPrintf( EMSG_ver, "Desktop, Pictures, or Music dir detected, default dir not searched.\n");
        }
        else
        if(    (strcmp( defdir, cv_home.string ) != 0) // not home directory
            && (strcmp( defdir, progdir ) != 0)        // not program directory
            && (strcmp( defdir, progdir_wads ) != 0) ) // not wads directory
        {
#endif
            defdir_search = 1;
            // Search current dir near first, for other wad searches.
            doomwaddir[1] = defdir;
#if !defined( __DJGPP__ )	
	}
#endif
    }
    // Search progdir/wads early, for other wad searches.
    doomwaddir[2] = progdir_wads;
    // Search last, for other wad searches.
    doomwaddir[MAX_NUM_DOOMWADDIR-1] = progdir;
}



//
// EVENT HANDLING
//
// Events are asynchronous inputs generally generated by the game user.
// Events can be discarded if no responder claims them
// referenced from i_system.c for I_GetKey()

event_t events[MAXEVENTS];
int eventhead = 0;
int eventtail = 0;

//
// D_PostEvent
// Called by the I/O functions when input is detected
//
void D_PostEvent(const event_t * ev)
{
    events[eventhead] = *ev;
    eventhead = (eventhead + 1) & (MAXEVENTS - 1);
}

// just for lock this function
#ifdef SMIF_PC_DOS
void D_PostEvent_end(void)
{
};
#endif

// Clear the input events before re-enabling play
static
void D_Clear_Events( void )
{
   eventhead = eventtail = 0;
   mousex = mousey = 0;  // clear accumulated motion
}

//
// D_Process_Events
// Send all the events of the given timestamp down the responder chain
//
void D_Process_Events(void)
{
    event_t *ev;

    while (eventtail != eventhead)
    {
        ev = &events[eventtail];

        if (M_Responder(ev)) // Menu input
          ;   // menu ate the event
        else if (CON_Responder(ev)) // console input
          ;
        else
          G_Responder(ev);

        eventtail++;
        eventtail = eventtail & (MAXEVENTS - 1);
    }
}

//
// D_Display
//  draw current display, possibly wiping it from the previous
//


// There is a wipe each change of the gamestate.
// wipegamestate can be set to GS_FORCEWIPE to force a wipe on the next draw.
gamestate_e wipegamestate = GS_DEMOSCREEN;

CV_PossibleValue_t screenslink_cons_t[] = { {0, "None"}, {wipe_ColorXForm + 1, "Crossfade"}, {wipe_Melt + 1, "Melt"}, {0, NULL} };
consvar_t cv_screenslink = { "screenlink", "2", CV_SAVE, screenslink_cons_t };

static
void D_Display(void)
{
    // vid : from video setup
    static boolean menuactivestate = false;
    static boolean draw_refresh = false;
    static gamestate_e oldgamestate = GS_FORCEWIPE; // invalid state
    static int borderdrawcount = 0;

    tic_t nowtime;
    tic_t tics;
    tic_t wipestart;
    int y;
    boolean wipe_done;
    boolean wipe;
    boolean redrawsbar;

    if (dedicated)
        return;

    if (nodrawers)
        return; // for comparative timing / profiling

    wipe = false;
    redrawsbar = false;
    wipe_done = false;

    //added:21-01-98: check for change of screen size (video mode)
    if( setmodeneeded.modetype || drawmode_recalc )
    {
        SCR_SetMode();  // change video mode
        //added:26-01-98: NOTE! setsizeneeded is set by SCR_Recalc()
        SCR_Recalc();
          // setsizeneeded -> redrawsbar
          // con_recalc, stbar_recalc, am_recalc
        drawmode_recalc = 0;
    }

    // change the view size if needed
    if (setsizeneeded)
    {
        R_ExecuteSetViewSize();  // set rdraw, view scale, limits, projection
        oldgamestate = GS_FORCEWIPE;  // force background redraw
        redrawsbar = true;
        draw_refresh = true;
    }

    // save the current screen if about to wipe
    if (gamestate != wipegamestate && rendermode == render_soft)
    {
        wipe = true;
        wipe_StartScreen();
    }

    // draw buffered stuff to screen
    // BP: Used only by linux GGI version
    I_UpdateNoBlit();

    // do buffered drawing
    switch (gamestate)
    {
        case GS_LEVEL:
            if (!gametic)
                break;

            // On each gametic
            HU_Erase();
            if (automapactive)
                AM_Drawer();

            if (wipe || menuactivestate
#ifdef HWRENDER
                || rendermode != render_soft
#endif
                || vid.recalc)
            {
                redrawsbar = true;
            }
            break;

        case GS_INTERMISSION:
            WI_Drawer();
            break;

        case GS_FINALE:
            F_Drawer();
            break;

        case GS_DEDICATEDSERVER:
        case GS_DEMOSCREEN:
            D_PageDrawer(pagename);
            break;

        case GS_WAITINGPLAYERS:
            // [WDJ] Because hardware may double buffer, need to overwrite
            // background. Waiting loop may get alternating backgrounds.
            D_PageDrawer(pagename);  // provide background
            D_WaitPlayer_Drawer();
            break;

        case GS_NULL:
        default:
            break;
    }

    if (gamestate == GS_LEVEL)
    {
        if (oldgamestate != GS_LEVEL)
        {
            // Level map play display initialize
            R_FillBackScreen(); // draw the pattern into the back screen
            // the border needs to be initially drawn
            draw_refresh = true;
        }

        // Level map play display
        // draw the view directly
        if (!automapactive)
        {
            // see if the border needs to be updated to the screen
            if( rdraw_scaledviewwidth != vid.width )
            {
                // the menu may draw over parts out of the view window,
                // which are refreshed only when needed
                if( menuactive || menuactivestate || draw_refresh )
                    borderdrawcount = 3;

                if( borderdrawcount )
                {
                    borderdrawcount--;
                    R_DrawViewBorder();     // erase old menu stuff
                }
            }

            if (displayplayer_ptr->mo)
            {
#ifdef CLIENTPREDICTION2
                displayplayer_ptr->mo->flags2 |= MF2_DONTDRAW;
#endif
#ifdef HWRENDER
                if (rendermode != render_soft)
                    HWR_RenderPlayerView(0, displayplayer_ptr);
                else    //if (rendermode == render_soft)
#endif
                    R_RenderPlayerView(displayplayer_ptr);
#ifdef CLIENTPREDICTION2
                displayplayer_ptr->mo->flags2 &= ~MF2_DONTDRAW;
#endif
            }

            // added 16-6-98: render the second screen
            if ( displayplayer2_ptr && displayplayer2_ptr->mo)
            {
#ifdef CLIENTPREDICTION2
                displayplayer2_ptr->mo->flags2 |= MF2_DONTDRAW;
#endif
#ifdef HWRENDER
                if (rendermode != render_soft)
                    HWR_RenderPlayerView(1, displayplayer2_ptr);
                else
#endif
                {
                    // Alter the draw tables to draw into second player window
                    //faB: Boris hack :P !!
                    viewwindowy = vid.height / 2;
                    memcpy(ylookup, ylookup2, rdraw_viewheight * sizeof(ylookup[0]));

                    R_RenderPlayerView(displayplayer2_ptr);

                    // Restore first player tables
                    viewwindowy = 0;
                    memcpy(ylookup, ylookup1, rdraw_viewheight * sizeof(ylookup[0]));
                }
#ifdef CLIENTPREDICTION2
                displayplayer2_ptr->mo->flags2 &= ~MF2_DONTDRAW;
#endif
            }
        }

        HU_Drawer();

        ST_Drawer(redrawsbar);
       
        draw_refresh = false;
    }
    else
    {
        // not GS_LEVEL
        // change gamma if needed
        if( gamestate != oldgamestate )
            V_SetPalette(0);
    }

    menuactivestate = menuactive;
    oldgamestate = wipegamestate = gamestate;

    // draw pause pic
    if (paused && (!menuactive || netgame))
    {
        patch_t *patch;
        if (automapactive)
            y = 4;
        else
            y = viewwindowy + 4;
        patch = W_CachePatchName("M_PAUSE", PU_CACHE);  // endian fix
        // 0
        V_SetupDraw( 0 | V_SCALEPATCH | V_SCALESTART );
        V_DrawScaledPatch(viewwindowx + (BASEVIDWIDTH - patch->width) / 2, y, patch);
    }

    //added:24-01-98:vid size change is now finished if it was on...
    vid.recalc = 0;
    rendermode_recalc = false;

    // Exl: draw a faded background
    if( fs_fadealpha != 0 )
    {
#ifdef HWRENDER
        if( rendermode != render_soft)
        {
            HWR_FadeScreenMenuBack(fs_fadecolor, fs_fadealpha, vid.height);
        }
        else
#endif
        {
            // Fade for software draw.
            V_FadeRect(0, vid.width, vid.height, (0xFF - fs_fadealpha),
                       (fs_fadealpha * 32 / 0x100), fs_fadecolor );
        }
    }

        //FIXME: draw either console or menu, not the two
    CON_Drawer();

    M_Drawer(); // menu is drawn even on top of everything
    NetUpdate();        // send out any new accumulation

//
// normal update
//
    if (!wipe)
    {
        if (cv_netstat.value)
        {
            char s[50];
            Net_GetNetStat();
            sprintf(s, "recv %d b/s", netstat_recv_bps);
            V_DrawString(BASEVIDWIDTH - V_StringWidth(s), BASEVIDHEIGHT - ST_HEIGHT - 40, V_WHITEMAP, s);
            sprintf(s, "send %d b/s", netstat_send_bps);
            V_DrawString(BASEVIDWIDTH - V_StringWidth(s), BASEVIDHEIGHT - ST_HEIGHT - 30, V_WHITEMAP, s);
            sprintf(s, "GameMiss %.2f%%", netstat_gamelost_percent);
            V_DrawString(BASEVIDWIDTH - V_StringWidth(s), BASEVIDHEIGHT - ST_HEIGHT - 20, V_WHITEMAP, s);
            sprintf(s, "SysMiss %.2f%%", netstat_lost_percent);
            V_DrawString(BASEVIDWIDTH - V_StringWidth(s), BASEVIDHEIGHT - ST_HEIGHT - 10, V_WHITEMAP, s);
        }

#ifdef TILTVIEW
        //added:12-02-98: tilt view when marine dies... just for fun
        if (gamestate == GS_LEVEL && cv_tiltview.value
            && displayplayer_ptr->playerstate == PST_DEAD)
        {
            V_DrawTiltView(screens[0]);
        }
        else
#endif
#ifdef PERSPCORRECT
        if (gamestate == GS_LEVEL && cv_perspcorr.value
           && rendermode == render_soft )
        {
            V_DrawPerspView(screens[0], displayplayer_ptr->aiming);
        }
        else
#endif
        {
            //I_BeginProfile();
            // display a graph of ticrate 
            if (cv_ticrate.value )
                V_Draw_ticrate_graph();
            I_FinishUpdate();   // page flip or blit buffer
            //debug_Printf("last frame update took %d\n", I_EndProfile());
        }
        return;
    }

//
// wipe update
//
    if (!cv_screenslink.value)
        return;

    wipe_EndScreen();

    wipestart = I_GetTime() - 1;
    y = wipestart + 2 * TICRATE;        // init a timeout
    do
    {
        do
        {
            nowtime = I_GetTime();
            tics = nowtime - wipestart;
        } while (!tics);
        wipestart = nowtime;
        wipe_done = wipe_ScreenWipe(cv_screenslink.value - 1, tics);
        I_OsPolling();
        I_UpdateNoBlit();
        M_Drawer();     // menu is drawn even on top of wipes
        I_FinishUpdate();       // page flip or blit buffer
    } while (!wipe_done && I_GetTime() < (unsigned) y);

    ST_Invalidate();
}

// =========================================================================
//   D_DoomLoop
// =========================================================================

tic_t rendergametic, oldentertics;
#ifdef CLIENTPREDICTION2
boolean spirit_update;
#endif

//#define SAVECPU_EXPERIMENTAL

// Called by port main program.
void D_DoomLoop(void)
{
    tic_t oldentertics, entertic, realtics, rendertimeout = -1;

    if (demorecording)
        G_BeginRecording();

    if( access( "autoexec.cfg", R_OK) == 0 )
    {
        // user settings
        COM_BufAddText("exec autoexec.cfg\n");
    }

    // end of loading screen: CONS_Printf() will no more call FinishUpdate()
    con_self_refresh = false;

    oldentertics = I_GetTime();

    // make sure to do a d_display to init mode _before_ load a level
    if( setmodeneeded.modetype || drawmode_recalc )
    {
        SCR_SetMode();      // change video mode
        SCR_Recalc();
        drawmode_recalc = 0;
    }
    if( rendermode_recalc )
    {
        I_Rendermode_setup();
        rendermode_recalc = 0;
    }

    D_Clear_Events();  // clear input events to prevent startup jerks,
                         // motion during screen wipe still gets through

    while (1)
    {
        // get real tics
        entertic = I_GetTime();
        realtics = entertic - oldentertics;
        oldentertics = entertic;

#ifdef SAVECPU_EXPERIMENTAL
        if (realtics == 0)
        {
            I_Sleep(10);
            continue;
        }
#endif

        // frame synchronous IO operations
        // UNUSED for the moment (18/12/98)
        I_StartFrame();

#ifdef HW3SOUND
        HW3S_BeginFrameUpdate();
#endif

        // process tics (but maybe not if realtic==0)
        TryRunTics(realtics);
#ifdef CLIENTPREDICTION2
        if (singletics || spirit_update)
#else
        if (singletics || gametic > rendergametic)
#endif
        {
            rendergametic = gametic;
            rendertimeout = entertic + TICRATE / 17;

            //added:16-01-98:consoleplayer -> displayplayer (hear sounds from viewpoint)
            S_UpdateSounds();   // move positional sounds
            // Update display, next frame, with current state.
            D_Display();
#ifdef CLIENTPREDICTION2
            spirit_update = false;
#endif
        }
        else if (rendertimeout < entertic)      // in case the server hang or netsplit
            D_Display();

        //Other implementations might need to update the sound here.
#ifndef SNDSERV
        // Sound mixing for the buffer is snychronous.
        I_UpdateSound();
#endif
        // Synchronous sound output is explicitly called.
#ifndef SNDINTR
        // Update sound output.
        I_SubmitSound();
#endif

#ifdef CDMUS
        // check for media change, loop music..
        I_UpdateCD();
#endif

#ifdef HW3SOUND
        HW3S_EndFrameUpdate();
#endif
    }
}

// =========================================================================
//  Demo
// =========================================================================

//
// D_PageTicker
// Handles timing for warped projection
//
void D_PageTicker(void)
{
    if (--pagetic < 0)
        D_AdvanceDemo();
}

//
// D_PageDrawer : draw a patch supposed to fill the screen,
//                fill the borders with a background pattern (a flat)
//                if the patch doesn't fit all the screen.
//
void D_PageDrawer(const char *lumpname)
{
    int x, y;
    byte *src;
    byte *dest;  // within screen buffer

    // [WDJ] Draw patch for all bpp, bytepp, and padded lines.
    V_SetupDraw( 0 | V_SCALESTART | V_SCALEPATCH | V_CENTERHORZ );

    // software mode which uses generally lower resolutions doesn't look
    // good when the pic is scaled, so it fills space around with a pattern,
    // and the pic is only scaled to integer multiples (x2, x3...)
    if (rendermode == render_soft)
    {
        if ((vid.width > BASEVIDWIDTH) || (vid.height > BASEVIDHEIGHT))
        {
            src = scr_borderflat;
            dest = screens[0];

            for (y = 0; y < vid.height; y++)
            {
                // repeatly draw a 64 pixel wide flat
                dest = screens[0] + (y * vid.ybytes);  // within screen buffer
                for (x = 0; x < vid.width / 64; x++)
                {
                    V_DrawPixels( dest, 0, 64, &src[(y & 63) << 6]);
                    dest += (64 * vid.bytepp);
                }
                if (vid.width & 63)
                {
                    V_DrawPixels( dest, 0, (vid.width & 63), &src[(y & 63) << 6]);
                }
            }
        }
    }
    // big hack for legacy's credits
    if (EN_heretic_hexen && (demosequence != 2))
    {
        V_DrawRawScreen_Num(0, 0, W_GetNumForName(lumpname), 320, 200);
        if (demosequence == 0 && pagetic <= 140)
            V_DrawScaledPatch_Name(4, 160, "ADVISOR" );
    }
    else
    {
        V_DrawScaledPatch_Name(0, 0, lumpname );
    }
}


//
// D_AdvanceDemo
// Called after each demo or intro demosequence finishes
//
// Called by D_StartTitle, with init of demosequence
// Called by D_PageTicker, when gamestate == GS_DEMOSCREEN
// Called by G_CheckDemoStatus when timing or playing a demo
void D_AdvanceDemo(void)
{
    // [WDJ] do not start a demo when a menu or console is open
    if( !(demo_ctrl & DEMO_seq_disabled) && ! menuactive
        && ! console_open )
        demo_ctrl = DEMO_seq_advance;    // flag to trigger D_DoAdvanceDemo
}

//
// This cycles through the demo sequences.
// FIXME - version dependent demo numbers?
//
// Called by TryRunTics when demo_ctrl == DEMO_seq_advance
void D_DoAdvanceDemo(void)
{
    const char * demo_name;

    demo_ctrl = 0;  // cancel DEMO_seq_advance
    players[consoleplayer].playerstate = PST_LIVE;      // not reborn
    gameaction = ga_nothing;

    if (gamemode == ultdoom_retail)
        demosequence = (demosequence + 1) % 7;
    else
        demosequence = (demosequence + 1) % 6;

    switch (demosequence)
    {
        case 0:
            pagename = "TITLEPIC";
            switch (gamemode)
            {
                case hexen:
                    pagetic = 210 + 140;
                    pagename = "TITLE";
                    S_StartMusic(hexen);
                    break;
                case heretic:
                    pagetic = 210 + 140;
                    pagename = "TITLE";
                    S_StartMusic(mus_htitl);
#if defined CDMUS && defined (__DJGPP__)
                    I_PlayCD (1, true);
#endif
                    break;
                case doom2_commercial:
                    pagetic = TICRATE * 11;
                    S_StartMusic(mus_dm2ttl);
#if defined CDMUS && defined (__DJGPP__)
                    I_PlayCD (1, true);
#endif										
                    break;
                default:
                    pagetic = 170;
                    S_StartMusic(mus_intro);
#if defined CDMUS && defined (__DJGPP__)
                    I_PlayCD (1, true);
#endif
                    break;
            }
            gamestate = GS_DEMOSCREEN;
            break;
        case 1:
            demo_name = "demo1";
            goto playdemo;
        case 2:
            pagetic = 200;
            gamestate = GS_DEMOSCREEN;
            pagename = "CREDIT";
            break;
        case 3:
            demo_name = "demo2";
            goto playdemo;
        case 4:
            gamestate = GS_DEMOSCREEN;
            if (gamemode == doom2_commercial)
            {
                pagetic = TICRATE * 11;
                pagename = "TITLEPIC";
                S_StartMusic(mus_dm2ttl);
            }
            else if (gamemode == heretic)
            {
                pagetic = 200;
                if( ! VALID_LUMP( W_CheckNumForName("e2m1") ) )
                    pagename = "ORDER";
                else
                    pagename = "CREDIT";
            }
            else
            {
                pagetic = 200;

                if (gamemode == ultdoom_retail)
                    pagename = text[CREDIT_NUM];
                else
                    pagename = text[HELP2_NUM];
            }
            break;
        case 5:
            demo_name = "demo3";
            goto playdemo;
            // THE DEFINITIVE DOOM Special Edition demo
        case 6:
            demo_name = "demo4";
            goto playdemo;
    }
    return;

 playdemo:
    G_DeferedPlayDemo( demo_name );
    demo_ctrl = DEMO_seq_playdemo;  // demo started here (not console)
    pagetic = 9999999;
    return;
}

//
// D_StartTitle
//
// Called by D_DoomMain(), when not server and not starting game
// Called by Command_ExitGame_f
// Called by CL_ConnectToServer when cannot join server, or aborting
// Called by Got_KickCmd, when player kicked from game
// Called by Net_Packet_Handler, upon server shutdown, timeout, or refused (NACK)
// Called by G_InitNew, when aborting game because cannot downgrade
// Called by M_Responder and M_Setup_prevMenu, when exiting menu and not playing game
void D_StartTitle(void)
{
    if( command_EV_param )
        CV_Restore_User_Settings();  // remove temp settings     
    gameaction = ga_nothing;
    playerdeadview = false;
    displayplayer = consoleplayer = statusbarplayer = 0;
    displayplayer_ptr = consoleplayer_ptr = &players[0]; // [WDJ]
    paused = false;
    demo_ctrl = 0;  // enable screens and seq demos
    demosequence = -1;
    CON_ToggleOff();
    D_AdvanceDemo();
}


// Disable demos
// Called when load game or init new game
void D_DisableDemo(void)
{
    if( demoplayback )
        G_StopDemo();
    // stop DEMO_seq_advance, but preserve DEMO_seq_playdemo so can abort it
    demo_ctrl = (demo_ctrl & DEMO_seq_playdemo) | DEMO_seq_disabled;
}


// =========================================================================
//   D_DoomMain
// =========================================================================


// Print out the search directories for verbose, and error.
//   emf: EMSG_ver, EMSG_error, EMSG_warn
//   enables: 0x01 legacy.wad order
//            0x02 IWAD order
//            0x0F verbose all
static
void  Print_search_directories( byte emf, byte enables )
{
    int wdi;
    GenPrintf(emf, "Search directories:\n");
    // Extra legacy.wad search, and verbose.
    if( (enables&0x01) && progdir )
        GenPrintf(emf, " progdir: %s\n", progdir );
    // Verbose only. For IWAD or legacy.wad they are in doomwaddir entries.
    if( (enables==0x0F) && progdir_wads )
        GenPrintf(emf, "        : %s\n", progdir_wads );
    if( (enables==0x0F) && defdir && defdir_search )
        GenPrintf(emf, " defdir: %s\n", defdir );
#ifdef LEGACYWADDIR
    GenPrintf(emf, " LEGACYWADDIR: %s\n", LEGACYWADDIR );
#endif
    for( wdi=0; wdi<MAX_NUM_DOOMWADDIR; wdi++ )
    {
        if( doomwaddir[wdi] )
            GenPrintf(emf, " Doomwaddir[%i]: %s\n", wdi, doomwaddir[wdi] );
    }
}



//
// D_AddFile
//
static
void D_AddFile(const char *filename)
{
    int numwadfiles;
    char *newfile;

    if( filename == NULL )
        return;

    // find end of wad files by counting
    for (numwadfiles = 0; startupwadfiles[numwadfiles]; numwadfiles++)
        ;
    if( numwadfiles >= MAX_WADFILES )
        I_Error ( "Too many wadfiles, max=%i.\n", MAX_WADFILES );

    newfile = malloc(strlen(filename) + 1);
    strcpy(newfile, filename);

    startupwadfiles[numwadfiles] = newfile;
}


#ifdef LAUNCHER
static void D_Clear_Files( void )
{
    int i;
    for (i = 0; startupwadfiles[i]; i++)
    {
        free( startupwadfiles[i] );
        startupwadfiles[i] = NULL;
    }
}
#endif


// ==========================================================================
// Identify the Doom version, and IWAD file to use.
// Sets 'gamemode' to determine whether doom_registered/doom2_commercial
// features are available (notable loading PWAD files).
// ==========================================================================

// [WDJ] Title and file names used in GDESC_other table entry, and other uses.
#define DESCNAME_SIZE	 64
char other_iwad_filename[ DESCNAME_SIZE ];
char other_gname[ DESCNAME_SIZE ];
char public_title[] = "Public DOOM";

game_desc_e     gamedesc_id;     // unique game id
game_desc_t     gamedesc;	 // active desc

// [WDJ] List of standard lump names to be checked, that appear in many
// game wads.  The game_desc_table also has a list of names that
// are unique to that game wad.
// This list sets a byte value of which are found.
// Each table entry will have required and reject bit masks.

#define COMMON_LUMP_LIST_SIZE   4
enum lumpname_e {
   LN_MAP01 =0x01, LN_E1M1 =0x02, LN_E2M2 =0x04, LN_TITLE =0x08
};
const char * common_lump_names[ COMMON_LUMP_LIST_SIZE ] = 
{
   "MAP01", "E1M1", "E2M2", "TITLE"
};

// [WDJ] The gname (first) is used to recognize save games, so don't change it.
// Some of the lump check information was obtained from ZDoom docs.
// The startup_title will be centered on the Title page.
// Switch names (idstr) need to be kept to 8 chars, all lowercase, so they
// can be used in file names on all systems.
// This table is the game search order.
// The first entry matching all characteristics will be used !
#define  NUM_GDESC   (GDESC_other+1)	// number of entries in game_desc_table
game_desc_t  game_desc_table[ NUM_GDESC ] =
{
// Free wads should get their own gamemode identity
// GDESC_freedoom: FreeDoom project, DoomII replacement
   { "FreeDoom", NULL, "freedoom",
        {"freedoom2.wad", "freedoom.wad","doom2.wad"}, NULL,
        {"FREEDOOM", NULL}, LN_MAP01, 0,
        0, GDESC_freedoom, doom2_commercial },
// GDESC_freedm: FreeDM project, DoomII deathmatch
   { "FreeDM", NULL, "freedm",
        {"freedm.wad","doom.wad",NULL}, NULL,
        {"FREEDOOM", "FREEDM"}, LN_MAP01, 0,
        0, GDESC_freedm, doom2_commercial },
// GDESC_doom2: doom2wad
   { "Doom2", "DOOM 2: Hell on Earth", "doom2",
        {"doom2.wad",NULL,NULL}, NULL,
        {NULL, NULL}, LN_MAP01, LN_TITLE,
        GD_idwad, GDESC_doom2, doom2_commercial },
// GDESC_freedoom_ultimate: FreeDoom project, Ultimate Doom replacement
   { "Ultimate FreeDoom", NULL, "freedu",
        {"freedoom1.wad", "freedu.wad","doomu.wad"}, NULL,
        {"FREEDOOM", "E4M1"}, LN_E1M1+LN_E2M2, 0,
        0, GDESC_freedoom_ultimate, ultdoom_retail },
// GDESC_ultimate: Doom1 1995, doomuwad
//                 Doom1 1995 on floppy (doom_se.wad)
   { "Ultimate Doom", "The Ultimate DOOM", "doomu",
        {"doomu.wad","doom_se.wad","doom.wad"}, NULL,
        {"E4M1", NULL}, LN_E1M1+LN_E2M2, LN_TITLE,
        GD_idwad, GDESC_ultimate, ultdoom_retail },
// GDESC_doom: DoomI 1994, doomwad
   { "Doom", "DOOM Registered", "doom",
        {"doom.wad",NULL,NULL}, NULL,
        {"E3M9", NULL}, LN_E1M1+LN_E2M2, LN_TITLE,
        GD_idwad, GDESC_doom, doom_registered },
// GDESC_doom_shareware: DoomI shareware, doom1wad
   { "Doom shareware", "DOOM Shareware", "doom1",
        {"doom1.wad","doom.wad",NULL}, NULL,
        {NULL, NULL}, LN_E1M1, LN_TITLE,
        GD_idwad, GDESC_doom_shareware, doom_shareware },
// GDESC_plutonia: FinalDoom : Plutonia, DoomII engine
   { "Plutonia", "DOOM 2: Plutonia Experiment", "plutonia",
        {"plutonia.wad",NULL,NULL}, NULL,
        {"CAMO1", NULL}, LN_MAP01, LN_TITLE,
        GD_idwad, GDESC_plutonia, doom2_commercial },
// GDESC_tnt: FinalDoom : Tnt Evilution, DoomII engine
   { "Tnt Evilution", "DOOM 2: TNT - Evilution", "tnt",
        {"tnt.wad",NULL,NULL}, NULL,
        {"REDTNT2", NULL}, LN_MAP01, LN_TITLE,
        GD_idwad, GDESC_tnt, doom2_commercial },
// GDESC_blasphemer: FreeDoom project, DoomII replacement
   { "Blasphemer", NULL, "blasphem",
        {"BLASPHEM.WAD","blasphem.wad","heretic.wad"}, NULL,
        {"BLASPHEM", NULL}, LN_E1M1+LN_TITLE, 0,
        0, GDESC_blasphemer, heretic },
// GDESC_heretic: Heretic
   { "Heretic", NULL, "heretic",
        {"heretic.wad",NULL,NULL}, NULL,
        {NULL, NULL}, LN_E1M1+LN_E2M2+LN_TITLE, 0,
        GD_idwad, GDESC_heretic, heretic },
// GDESC_heretic_shareware: Heretic shareware
   { "Heretic shareware", NULL, "heretic1",
        {"heretic1.wad","heretic.wad",NULL}, NULL,
        {NULL, NULL}, LN_E1M1+LN_TITLE, LN_E2M2,
        GD_idwad, GDESC_heretic_shareware, heretic },
// GDESC_hexen: Hexen
   { "Hexen", NULL, "hexen",
        {"hexen.wad",NULL,NULL}, NULL,
        {"MAP40", NULL}, LN_MAP01+LN_TITLE, 0,
        GD_idwad/*|GD_unsupported*/, GDESC_hexen, hexen },
// GDESC_hexen_demo: Hexen
   { "Hexen Demo", NULL, "hexen1",
        {"hexen1.wad","hexen.wad",NULL}, NULL,
        {NULL, NULL}, LN_MAP01+LN_TITLE, 0,
        GD_idwad|GD_unsupported, GDESC_hexen_demo, hexen },
// GDESC_strife: Strife
   { "Strife", NULL, "strife",
        {"strife.wad","STRIFE1.WAD","VOICES.WAD"}, NULL,
        {"ENDSTRF", "MAP20"}, LN_MAP01, 0,
        GD_idwad/*|GD_unsupported*/, GDESC_strife, strife },
// GDESC_strife_shareware: Strife shareware
   { "Strife shareware", NULL, "strife0",
        {"strife0.wad","strife.wad",NULL}, NULL,
        {"ENDSTRF", NULL}, 0, LN_MAP01,
        GD_idwad/*|GD_unsupported*/, GDESC_strife_shareware, strife },
// GDESC_chex1: Chex Quest
   { "Chex Quest", NULL, "chex1",
        {"chex1.wad","chex.wad",NULL}, NULL,
        {"W94_1", "POSSH0M0"}, LN_E1M1, LN_TITLE,
        GD_iwad_pref, GDESC_chex1, chexquest1 },
#if defined (__DJGPP__)
// GDESC_doom2: HacX
   { "HacX", "HacX: Twitch 'n' Kill", "HACX",
        {"HACX.wad",NULL,NULL}, NULL,
        {NULL, NULL}, LN_MAP01, LN_TITLE,
        GD_idwad, GDESC_doom2, doom2_commercial },
#endif				
// GDESC_ultimate_mode: Ultimate Doom replacement
   { "Ultimate mode", NULL, "ultimode",
        {"doomu.wad","doom.wad",NULL}, NULL,
        { NULL, NULL}, LN_E1M1, 0,
        0, GDESC_ultimate, ultdoom_retail },
// GDESC_doom_mode: DoomI replacement
   { "Doom mode", NULL, "doommode",
        {"doom1.wad","doom.wad",NULL}, NULL,
        { NULL, NULL}, LN_E1M1, 0,
        0, GDESC_doom_mode, doom_registered },
// GDESC_heretic_mode: Heretic replacement
   { "Heretic mode", NULL, "heremode",
        {"heretic.wad",NULL,NULL}, NULL,
        { NULL, NULL}, LN_E1M1, 0,
        0, GDESC_heretic_mode, heretic },
// GDESC_hexen_mode: Hexen replacement
   { "Hexen mode", NULL, "hexemode",
        {"hexen.wad",NULL,NULL}, NULL,
        { NULL, NULL}, LN_MAP01, 0,
        GD_idwad/*GD_unsupported*/, GDESC_hexen_mode, hexen },
// GDESC_other: Other iwads, all DoomII features enabled,
// strings are ptrs to buffers
   { other_gname, public_title, "",
        {other_iwad_filename,NULL,NULL}, NULL,
        { NULL, NULL}, LN_MAP01, 0,
        GD_iwad_pref, GDESC_other, doom2_commercial }
};

#ifdef LAUNCHER
// Lookup game by table index
game_desc_t *  D_GameDesc( int i )
{
    if ( i<0 || i > NUM_GDESC-1 )   return NULL;
    return  &game_desc_table[i];
}
#endif


// Check all lump names in lumpnames list, count is limited to 8
// Return byte has a bit set for each lumpname found.
static
byte  Check_lumps( const char * wadname, const char * lumpnames[], int count )
{
    wadinfo_t   header;
    filelump_t  lumpx;
    FILE * 	wadfile;
    const char * reason;
    int         hli, lc;
    byte        result = 0;

    // This routine checks the directory, using the system file cache
    // instead of making an internal lumps directory.
    // Speed is not required here, so no extra speedup checks.

    // List may be fixed length, with NULL entries.
    while( count>0 && (lumpnames[count-1] == NULL))
    {
       count--;    // Reduce count by the number of NULL entries on end.
       // It is easier to consider a NULL as always found.
       result |= 1<<count;
    }
    if( count == 0 )  goto ret_result;  // escape NULL list
   
    // Read the wad file header and get directory
    wadfile = fopen( wadname, "rb" );
    if( wadfile == NULL )  goto open_err;
    fread( &header, sizeof(header), 1, wadfile);
    // check for IWAD or PWAD
    if( strncmp(header.identification+1,"WAD",3) != 0 ) goto not_a_wad;
    // find directory
    header.numlumps = LE_SWAP32(header.numlumps);
    header.infotableofs = LE_SWAP32(header.infotableofs);
    if( fseek( wadfile, header.infotableofs, SEEK_SET ))   goto read_err;
    // Check the directory as it is read out of the system file cache.
    for( hli=0; hli<header.numlumps; hli++ )
    {
        if( fread( &lumpx, sizeof(lumpx), 1, wadfile ) < 1 )   goto read_err;
#ifdef DEBUG       
        int cmp = strncasecmp( lumpx.name, lumpname, 8 );
        if( strncasecmp( lumpx.name, "TITLE", 5 ) == 0 )
         printf( "%8s %c %8s \n", lumpx.name,
                 ( (cmp<0)?'<': (cmp>0)? '>' :'='),
                  lumpname);
#endif       
        for( lc=0; lc<count; lc++ )
        {
            if( strncasecmp( lumpx.name, lumpnames[lc], 8 ) == 0 )
                result |= 1<<lc;  // found it, record it
        }
    }
    fclose( wadfile );

ret_result:   
    return result;	// default is not found

    // Should only be called with known WAD files
not_a_wad:
    reason = "Not a WAD file";
    goto err_ret0;
   
read_err:  // read and seek errors
    reason = "Wad file err";
    goto err_ret0;

open_err:
    reason = "Wad file open err";

err_ret0:
    GenPrintf( EMSG_error, "%s: %s\n", reason, wadname );
    if( wadfile )  fclose( wadfile );
    return 0;
}


static
boolean Check_keylumps ( game_desc_t * gmtp, const char * wadname )
{
    byte lumpbits;
    if( gmtp->require_lump | gmtp->reject_lump )
    {
        lumpbits = Check_lumps( wadname,
                               common_lump_names, COMMON_LUMP_LIST_SIZE );
        if((gmtp->require_lump & lumpbits) != gmtp->require_lump )  goto fail;
        if( gmtp->reject_lump & lumpbits )  goto fail;
    }
    // Check 2 unique lump names, NULLS are treated as found.
    lumpbits = Check_lumps( wadname, &(gmtp->keylump[0]), 2 );
    if( lumpbits != 0x03 )   goto fail;
    return 1;  // lump checks successful
fail:       
    return 0;  // does not match
}


// Checks the possible wad filenames in GDESC_ entry.
// Return true when found and keylumps verified
// Leaves name in pathbuf_p, which must be of MAX_PATH length.
static
boolean  Check_wad_filenames( int gmi, char * pathbuf_p )
{
    game_desc_t * gmtp = &game_desc_table[gmi];
    int w;
    // check each possible filename listed
    for( w=0; w<3; w++ )
    {
        if( gmtp->iwad_filename[w] == NULL ) break;

        if( Search_doomwaddir( gmtp->iwad_filename[w], GAME_SEARCH_DEPTH,
                               /*OUT*/ pathbuf_p ) )
        {
            // File exists.
            if( Check_keylumps( gmtp, pathbuf_p ) )
                return true;
        }
    }
    return false;
}


// May be called again after command restart
static
void IdentifyVersion()
{
    char pathiwad[_MAX_PATH + 16];
    // debug_Printf("MAX_PATH: %i\n", _MAX_PATH);

    boolean  other_names = 0;	// indicates -iwad other names
    boolean  devgame = false;   // indicates -devgame <game>    

    int gamedesc_index = NUM_GDESC; // nothing
    int gmi;

    // find legacy.wad, IWADs
    // and... Doom LEGACY !!! :)
    char *legacywad = NULL;

    if( verbose )
    {
        Print_search_directories( EMSG_ver, 0x0F );
    }


#if defined(__APPLE__) && defined(__MACH__) && defined( EXT_MAC_DIR_SPEC )
    //[segabor]: on Mac OS X legacy.wad is within .app folder
    // for uniformity, use the strdup at found_legacy_wad
    cat_filename(pathiwad, "", mac_legacy_wad );
    if( access( mac_legacy_wad, R_OK) == 0 )    goto found_legacy_wad;
    // check other locations
#endif

    // [WDJ]: find legacy.wad .
    // pathiwad must be MAX_WADPATH to be used by cat_filename.
    // Look in program directory first, because executable may have
    // its own version of legacy.wad.
    if( progdir && ( access( progdir, R_OK) == 0 ) )
    {
        // [WDJ] look for legacy.wad with doomlegacy
        cat_filename(pathiwad, progdir, "legacy.wad");
        if( access( pathiwad, R_OK) == 0 )   goto found_legacy_wad;
    }

#ifdef LEGACYWADDIR
    // [WDJ] Try LEGACYWADDIR as the first wad dir.
    if( access( LEGACYWADDIR , R_OK) == 0 )
    {
        // [WDJ] legacy.wad is in shared directory
        cat_filename(pathiwad, LEGACYWADDIR, "legacy.wad");
        if( access( pathiwad, R_OK) == 0 )   goto found_legacy_wad;
    }
#endif

    // Search wad directories.
    doomwaddir[1] = progdir_wads;
    if( Search_doomwaddir( "legacy.wad", 0, /*OUT*/ pathiwad ) )
         goto found_legacy_wad;

    I_SoftError( "legacy.wad not found\n" );  // fatal exit
    GenPrintf(EMSG_error, "Looked for legacy.wad in:\n" );
    Print_search_directories( EMSG_error, 0x01 );
    goto fatal_err;
   
   
 found_legacy_wad:
    legacywad = strdup( pathiwad );  // malloc
    doomwaddir[1] = NULL;

    if( verbose )
    {
        GenPrintf(EMSG_ver, "Legacy.wad: %s\n", legacywad );
    }

    owner_wad_search_order();

    /*
       French stuff.
       doom2fwad = malloc(strlen(doomwaddir)+1+10+1);
       sprintf(doom2fwad, "%s/doom2f.wad", doomwaddir);
     */

    // [WDJ] were too many chained ELSE. Figured it out once and used direct goto.

    // [WDJ] Old switches -shdev, -regdev, -comdev are now -devgame <game>
    // Earlier did direct test of -devparm, do not overwrite it.
    devgame = M_CheckParm("-devgame");
    // [WDJ] search for one of the listed GDESC_ forcing switches
    if ( devgame || M_CheckParm("-game") )
    {
        char *temp = M_GetNextParm();
        if( temp == NULL )
        {
            I_SoftError( "Switch  -game <name> or -devgame <name>\n" );
            goto fatal_err;
        }
        for( gmi=0; gmi<GDESC_other; gmi++ )
        {
            // compare to recognized game mode names
            if (!strcmp(temp, game_desc_table[gmi].idstr))
                goto game_switch_found;
        }
        I_SoftError( "Switch  -game %s  not recognized\n", temp );
        goto fatal_err;
       
       game_switch_found:
        // switch forces the GDESC_ selection
        gamedesc_index = gmi;
        gamedesc = game_desc_table[gamedesc_index]; // copy the game descriptor
        if (gamedesc.gameflags & GD_unsupported)  goto unsupported_wad;

        // handle the recognized special -devgame switch
        if( devgame )
        {
            devparm = true;
#if 0
            strcpy(configfile, DEVDATA CONFIGFILENAME); // moved
            // [WDJ] Old, irrelevant, and it was interfering with new
            // GDESC changes.
            // Better to just use -file so I am disabling it.
            switch( gamedesc_index )
            {
             case GDESC_doom_shareware:
               // instead use:
               //  doomlegacy -devgame doom1 -file data_se/texture1.lmp data_se/pnames.lmp
               D_AddFile(DEVDATA "doom1.wad");
               D_AddFile(DEVMAPS "data_se/texture1.lmp");
               D_AddFile(DEVMAPS "data_se/pnames.lmp");
               goto got_iwad;
             case GDESC_doom:
               // instead use:
               //   doomlegacy -devgame doom1 -file data_se/texture1.lmp data_se/texture2.lmp data_se/pnames.lmp
               D_AddFile(DEVDATA "doom.wad");
               D_AddFile(DEVMAPS "data_se/texture1.lmp");
               D_AddFile(DEVMAPS "data_se/texture2.lmp");
               D_AddFile(DEVMAPS "data_se/pnames.lmp");
               goto got_iwad;
             case GDESC_doom2:
               // instead use:
               //   doomlegacy -devgame doom2 -file cdata/texture1.lmp cdata/pnames.lmp
               D_AddFile(DEVDATA "doom2.wad");
               D_AddFile(DEVMAPS "cdata/texture1.lmp");
               D_AddFile(DEVMAPS "cdata/pnames.lmp");
               goto got_iwad;
            }
#endif
        }
    }
   

    // Specify the name of the IWAD file to use, so we can have several IWAD's
    // in the same directory, and/or have legacy.exe only once in a different location
    if (M_CheckParm("-iwad"))
    {
        // BP: big hack for fullpath wad, we should use wadpath instead in d_addfile
        char *s = M_GetNextParm();
        if ( s == NULL )
        {
            I_SoftError("Switch -iwad <filename>.\n");
            goto fatal_err;
        }

        const char * ipath = file_searchpath( s );
        if( ipath )
        {
            // Absolute or relative path, no search.
            cat_filename( pathiwad, ipath, s );
        }
        else
        {
            // Simple filename.
            // Find the IWAD in the doomwaddir.
            if( ! Search_doomwaddir( s, IWAD_SEARCH_DEPTH, /*OUT*/ pathiwad ) )
            {
                // Not found in doomwaddir.
                cat_filename( pathiwad, "", s );
            }
        }

#ifdef LAUNCHER
        CV_Set( & cv_iwad, pathiwad );  // for launcher
        cv_iwad.flags &= ~CV_MODIFIED;
#endif

        if ( access(pathiwad, R_OK) < 0 )
        {
            I_SoftError("IWAD %s not found\n", s);
            Print_search_directories( EMSG_error, 0x02 );
            goto fatal_err;
        }

        char *filename = FIL_Filename_of( pathiwad );
        if ( gamedesc_index == NUM_GDESC ) // check forcing switch
        {
            // No forcing switch
            // [WDJ] search game table for matching iwad name
            for( gmi=0; gmi<GDESC_other; gmi++ )
            {
                game_desc_t * gmtp = &game_desc_table[gmi];
                int w;
                // check each possible filename listed
                for( w=0; w<3; w++ )
                {
                    if( gmtp->iwad_filename[w] == NULL ) break;
                    if( strcasecmp(gmtp->iwad_filename[w], filename) == 0 )
                    {
                        if( Check_keylumps( gmtp, pathiwad ) )
                            goto got_gmi_iwad;
                    }
                }
            }
            // unknown IWAD is GDESC_other
            gamedesc_index = GDESC_other;
        }

        other_names = 1;        // preserve other names when forcing switch
        // for save game header
        strncpy( other_iwad_filename, filename, DESCNAME_SIZE );
        other_iwad_filename[ DESCNAME_SIZE-1 ] = 0; // safe
        // create game name from the wad name, used in save game
        strncpy( other_gname, other_iwad_filename, DESCNAME_SIZE );
        other_gname[ DESCNAME_SIZE-1 ] = 0;	// safe
               // use the wad name, without the ".wad" as the gname
        {
            char * dp = strchr( other_gname, '.' );
            if( dp )  *dp = 0;
        }
        goto got_iwad;
    }
    // No -iwad switch:
    // [WDJ] Select IWAD by game switch
    if(gamedesc_index < GDESC_other)  // selected by switch, and no -iwad
    {
        // make iwad name by switch
        // use pathiwad to output wad path from Check_wad_filenames
        if( Check_wad_filenames( gamedesc_index, pathiwad ) )
            goto got_iwad;
        I_SoftError("IWAD %s not found in:\n",
                     game_desc_table[gamedesc_index].iwad_filename[0]);
        Print_search_directories( EMSG_error, 0x02 );
        goto fatal_err;
    }
    // No -iwad switch, and no mode select switch:
    // [WDJ] search the table for the first iwad filename found
    for( gmi=0; gmi<GDESC_other; gmi++ )
    {
        // use pathiwad to output wad path from Check_wad_filenames
        if( Check_wad_filenames( gmi, pathiwad ) )
            goto got_gmi_iwad;
    }

    I_SoftError("Main WAD file not found\n"
            "You need doom.wad, doom2.wad, heretic.wad or some other IWAD file\n"
            "from any shareware, commercial or free version of Doom or Heretic!\n"
#if !defined(__WIN32__) && !(defined __DJGPP__)
            "If you have one of those files, be sure its name is lowercase\n"
            "or use the -iwad command line switch.\n"
#endif
            );
    goto fatal_err;

 got_gmi_iwad:
    gamedesc_index = gmi;  // a search loop found it
 got_iwad:
    gamedesc = game_desc_table[gamedesc_index]; // copy the game descriptor

    if( other_names )  // keep names from -iwad
    {
        gamedesc.gname = other_gname;
        gamedesc.iwad_filename[0] = other_iwad_filename;
    }
    gamedesc_id = gamedesc.gamedesc_id;
    G_set_gamemode( gamedesc.gamemode );
    GenPrintf( EMSG_info, "IWAD recognized: %s\n", gamedesc.gname);

    if (gamedesc.gameflags & GD_unsupported)  goto unsupported_wad;

    D_AddFile(pathiwad);
    D_AddFile(legacywad);  // So can replace some graphics with Legacy ones.
    if( gamedesc.gameflags & GD_iwad_pref )
    {
       // Because legacy.wad replaced some things it shouldn't, give the iwad
       // preference from both search directions.
       // Chexquest1: legacy.wad was replacing the green splats, with bloody ones.
       D_AddFile(pathiwad);
    }
    if( gamedesc.support_wad )
       D_AddFile( gamedesc.support_wad );

cleanup_ret:
    free(legacywad);  // from strdup, free local copy of name
    return;

unsupported_wad:
    I_SoftError("Doom Legacy currently does not support this game.\n");
    goto fatal_err;
   
fatal_err:
    if( legacywad )
    {
        // [WDJ] Load legacywad if possible, because it contains parts of the
        // user interface, and there will misleading errors.
        D_AddFile(legacywad);  // To prevent additional errors.
    }
    fatal_error = 1;
    goto cleanup_ret;
}

/* ======================================================================== */
// Just print the nice red titlebar like the original DOOM2 for DOS.
/* ======================================================================== */
#ifdef SMIF_PC_DOS
void D_Titlebar(const char *title1, const char *title2)
{
    // DOOM LEGACY banner
    clrscr();
    textattr((BLUE << 4) + WHITE);
    clreol();
    cputs(title1);

    // standard doom/doom2 banner
    textattr((RED << 4) + WHITE);
    clreol();
    gotoxy((80 - strlen(title2)) / 2, 2);
    cputs(title2);
    normvideo();
    gotoxy(1, 3);
}
#endif

#define MAX_TITLE_LEN   80
static char legacytitle[MAX_TITLE_LEN+1];  // length of line

//added:11-01-98:
//
//  Center the title string, then add the date and time of compilation.
//
static void D_Make_legacytitle(void)
{
  const char *s = VERSION_BANNER;
  int i;

  memset(legacytitle, ' ', sizeof(legacytitle));

  for (i = (MAX_TITLE_LEN - strlen(s)) / 2; *s; )  // center
    legacytitle[i++] = *s++;

  const char *u = __DATE__;
  for (i = 0; i < 11; i++)
    legacytitle[i + 1] = u[i]; 

  u = __TIME__;
  for (i = 0; i < 8; i++)
    legacytitle[i + 71] = u[i];

  legacytitle[MAX_TITLE_LEN] = '\0';
}


// Check Legacy.wad
void D_CheckWadVersion()
{
    int wadversion = 0;
    char hs[128];
    int wv2, hlen;
    lumpnum_t ver_lumpnum;
/* BP: disabled since this should work fine now...
    // check main iwad using demo1 version 
    ver_lumpnum = W_CheckNumForNameFirst("demo1");
    // well no demo1, this is not a main wad file
    if( ! VALID_LUMP(ver_lumpnum) )
        I_Error("%s is not a Main wad file (IWAD)\n"
                "try with Doom.wad or Doom2.wad\n"
                "\n"
                "Use -nocheckwadversion to remove this check,\n"
                "but this can cause Legacy to hang\n",wadfiles[0]->filename);
    W_ReadLumpHeader (lump,&wadversion,1);
    if( wadversion<109 )
        I_Error("Your %s file is version %d.%d\n"
                "Doom Legacy need version 1.9\n"
                "Upgrade your version to 1.9 using IdSofware patch\n"
                "\n"
                "Use -nocheckwadversion to remove this check,\n"
                "but this can cause Legacy to hang\n",wadfiles[0]->filename,wadversion/100,wadversion%100);
*/
    // check version, of legacy.wad using version lump
    ver_lumpnum = W_CheckNumForName("version");
    if( ! VALID_LUMP(ver_lumpnum) )
    {
        I_SoftError("No legacy.wad file.\n");
        fatal_error = 1;
        return;
    }
    hlen = W_ReadLumpHeader(ver_lumpnum, &hs, 128);
    if (hlen < 128)
    {
        hs[hlen] = '\0';
        if (sscanf(hs, "Doom Legacy WAD V%d.%d", &wv2, &wadversion) == 2)
          wadversion += wv2 * 100;
    }
    if (wadversion != cur_wadversion)
    {
        I_SoftError("Your legacy.wad file is version %d.%d, you need version %d.%d\n"
                "Use the legacy.wad that came in the same archive as this executable.\n"
                "\n"
                "Use -nocheckwadversion to remove this check,\n"
                "but this can cause Legacy to crash.\n",
                (wadversion / 100), (wadversion % 100),
                (int)(cur_wadversion / 100), (int)(cur_wadversion % 100) );
        if( wadversion < min_wadversion )
            fatal_error = 1;
    }
}

//
// D_DoomMain
//
// Called from port main program to processes setup.
// Returns before game starts.
void D_DoomMain()
{
    int p, wdi;
    char fbuf[FILENAME_SIZE];
    char dirbuf[_MAX_PATH ];

    int startepisode;
    int startmap;
    boolean autostart;

#ifdef FRENCH_INLINE
    french_early_text();
#endif
#if defined( __DJGPP__ )
    char dosroot[MAX_WADPATH]; 
#endif
    // print version banner just once here, use it anywhere
    sprintf(VERSION_BANNER, "Doom Legacy %d.%d.%d %s %s", VERSION/100, VERSION%100, REVISION, VERSIONSTRING, DOSSTRING);
    demoversion = VERSION;

    D_Make_legacytitle();

    memset( startupwadfiles, 0, sizeof(startupwadfiles) );
   
    CON_Init_Setup();  // vid, zone independent
    EOUT_flags |= EOUT_con;  // all msgs to CON buffer
    use_font1 = 1;  // until PLAYPAL and fonts loaded
    vid.draw_ready = 0;  // disable print reaching console

    //added:18-02-98:keep error messages until the final flush(stderr)
    if (setvbuf(stderr, NULL, _IOFBF, 1000))
        GenPrintf(EMSG_warn,"setvbuf didnt work\n");
    setbuf(stdout, NULL);       // non-buffered output

    // get parameters from a response file (eg: doom3 @parms.txt)
    M_FindResponseFile();

    // some basic commandline options
    if (M_CheckParm("--version"))
    {
      printf("%s\n", legacytitle);
      exit(0);
    }

    if (M_CheckParm("-v"))
    {
      verbose = 1;
    }
    if (M_CheckParm("-v2"))
    {
      verbose = 2;
    }
   
    if (M_CheckParm("--help") || M_CheckParm("-h"))
    {
      printf("%s\n", legacytitle);
      Help();
      exit(0);
    }

#if defined( __DJGPP__ )
    if (M_CheckParm("-listwads") || M_CheckParm("-lw")) ListSupportetWads_Command();
#endif       
    GenPrintf( EMSG_info|EMSG_all, "%s\n", legacytitle);

    // Find or make a default dir that is not root dir
    // get the current directory (possible problem on NT with "." as current dir)
    if (getcwd(dirbuf, _MAX_PATH) != NULL)
    {
        #if defined (__DJGPP__)
         Posix_Deform_Path(dirbuf);
        #endif			
        // Need a working default dir, to prevent "" leading to root files.
        if( (strlen(dirbuf) > 4)
            || (strcmp( dirbuf, "." ) == 0) )   // systems that pass "."
        {
            defdir = strdup( dirbuf );
            if( verbose )
                GenPrintf(EMSG_ver, "Current directory: %s\n", defdir);

            if( access( defdir, X_OK ) == 0 )
                defdir_stat = 1;
        }
    }

    if( I_Get_Prog_Dir( defdir, /*OUT*/ dirbuf ) )
    {

        progdir = strdup( dirbuf );
        #if defined (__DJGPP__)
         Posix_Deform_Path(progdir);
        #endif				
        if( verbose )
          GenPrintf(EMSG_ver, "Program directory: %s\n", progdir);

        // Set the directories that are relative to the program directory.
        if( access( progdir, X_OK ) == 0 )
        {
            cat_filename(dirbuf, progdir, "wads");
            progdir_wads = strdup(dirbuf);
        }
    }

    memset( doomwaddir, 0, sizeof(doomwaddir) );
    doomwaddir[0] = getenv("DOOMWADDIR");  // ptr to environment string

    // CDROM overrides doomwaddir (when valid)
    if (M_CheckParm("-cdrom"))
    {
        GenPrintf(EMSG_hud, D_CDROM);
        // [WDJ] Execute DoomLegacy off CDROM ??
        // DoomLegacy already has separate doomwaddir and legacyhome.
        // Legacy is not compatible with other port config and savegames,
        // so do not put such in old doom "c:\\doomdata".
        // Substitute CDROM for doomwaddir, but not legacyhome.
        if( defdir )
            doomwaddir[0] = ""; // wads from cur dir
        defdir_stat = 0; // do not let legacyhome use current dir
    }

#if 0
//[WDJ] disabled in 143beta_macosx
// was test on MACOS_DI but could exclude or include __MACH__ ??
//[segabor]
#if defined( __APPLE__ ) && ! defined( __MACH__ )
    // cwd is always "/" when app is dbl-clicked
    if (!strcasecmp(doomwaddir, "/"))
    {
        // doomwaddir maybe malloc string, maybe not
        doomwaddir[0] = I_GetWadDir();
    }
#endif
#endif

    EOUT_flags = EOUT_text | EOUT_log | EOUT_con;

    CONS_Printf(text[Z_INIT_NUM]);
    // Cannot Init nor register cv_ vars until after Z_Init and some
    // other systems are init first.
    // -mb cannot be changed by Launcher, use Response File instead
    Z_Init();

    // Init once
    COM_Init(); // command buffer
    // Can now call CV_RegisterVar, and COM_AddCommand

    // may have some command line dependent init, like joystick
    I_SysInit();

    dedicated = M_CheckParm("-dedicated") != 0;

    //--- Display Error Messages
    CONS_Printf("StartupGraphics...\n");
    // setup loading screen with dedicated=0 and vid=800,600
    V_Init_VideoControl();  // before I_StartupGraphics

    if( ! dedicated )
    {
#if defined( __DJGPP__ )
        // Diese Arguemente (für den Screen) werden für die DOS Version zu spät aufgerufen
        set_drawmode = cv_drawmode.EV;
        req_bitpp = 0;  // because of launcher looping
        req_alt_bitpp = 0;
        if( M_CheckParm("-highcolor") )
        {
            set_drawmode = DRM_explicit_bpp;  // 15 or 16 bpp
            req_bitpp = 16;
            req_alt_bitpp = 15;
        }
        if( M_CheckParm("-truecolor") )
        {
            set_drawmode = DRM_explicit_bpp;  // 24 or 32 bpp
            req_bitpp = 32;
            req_alt_bitpp = 24;
        }
        if( M_CheckParm("-native") )
        {
            set_drawmode = DRM_native;  // bpp of the default screen
        }
        p = M_CheckParm("-bpp");  // specific bit per pixel color
        if( p )
        {
            // binding, should fail if cannot find a mode
            req_bitpp = atoi(myargv[p + 1]);
            if( ! V_CanDraw( req_bitpp ) )            
              I_Error( "-bpp invalid\n");
            
            set_drawmode = DRM_explicit_bpp;
        }
#endif		
        I_StartupGraphics();    // window
        SCR_Startup();
    }

    if( verbose > 1 )
        CONS_Printf("Init DEH, cht, menu\n");
    P_clear_state_ext();  // init state_ext
    // save Doom, Heretic, Chex strings for DEH
    DEH_Init();  // Init DEH before files and lumps loaded
    cht_Init();	 // init iwad independent cheats info, needed by Responder

    M_Init();    // init menu
    R_Init_rdata();

    if( verbose > 1 )
        CONS_Printf( "Register\n" );

    // Any cv_ with CV_SAVE need to be registered here, before reading the config.
    // Some of these are dependent upon the dedicated command line switch.
    CON_Register();
    D_Register_ClientCommands(); //Hurdler: be sure that this is called before D_Setup_NetGame
    D_Register_MiscCommands();	//[WDJ] more than just DeathMatch

    M_Register_Menu_Controls();
    HU_Register_Commands();
    ST_Register_Commands();

    T_Register_Commands();    // fragglescript
    B_Register_Commands();    //added by AC for acbot
    R_Register_EngineStuff();
    S_Register_SoundStuff();

    P_Register_Info_Commands();

#ifdef LAUNCHER
    CV_RegisterVar(&cv_home);
    CV_RegisterVar(&cv_doomwaddir);
    CV_RegisterVar(&cv_iwad);
    CV_Set( &cv_doomwaddir, doomwaddir[0] ? doomwaddir[0] : "" );
    cv_doomwaddir.flags &= ~CV_MODIFIED;
#endif

    //Fab:29-04-98: do some dirty chatmacros strings initialisation
    HU_Init_Chatmacros();

    // load default control
    G_Controldefault();
   

    // Before this line are initializations that are run only one time.
    //---------------------------------------------------- 
    // After this line is code that deals with configuration,
    // game and wad selection, and finding files and directories.
    // It may retry some actions and may execute functions multiple times.

#ifdef LAUNCHER   
// ----------- launcher restarts here 
restart_command:

    fatal_error = 0;
    verbose = 0;  // verbose may be changed by launcher
    if (M_CheckParm("-v"))
    {
      verbose = 1;
    }
    if (M_CheckParm("-v2"))
    {
      verbose = 2;
    }
    dedicated = M_CheckParm("-dedicated") != 0;

    if( legacyhome )
       free( legacyhome );  // from previous
#endif

    V_SetupFont( 1, NULL, 0 );  // Startup font size
    EOUT_flags = EOUT_text | EOUT_log | EOUT_con;

    devparm |= M_CheckParm("-devparm");  // -devparm or -devgame
    if (devparm)
      CONS_Printf(D_DEVSTR);

    if( verbose > 1 )
        CONS_Printf("Find HOME\n");
    // userhome section
    {
        const char * userhome = NULL;
#ifdef LAUNCHER
        byte   userhome_parm = 0;
#endif
        if (M_CheckParm("-home"))
        {
            userhome = M_GetNextParm();
            if( userhome == NULL )
            {
                I_SoftError( "Switch  -home <directory>\n" );
                userhome = "";
                fatal_error = 1;
            }
#ifdef LAUNCHER
            userhome_parm = 1;
#endif
        }
        else
        {
            userhome = getenv("HOME");
            if( userhome && verbose > 1 )
                CONS_Printf("HOME = %s\n", userhome);
#ifdef WIN32
            if( userhome && strstr( userhome, "MSYS" ) )
            {
                // Ignore MSYS HOME, it is not the one wanted.
                if( verbose > 1 )
                    CONS_Printf("Ignore MYS HOME = %s\n", userhome);
                userhome = NULL;
            }
            // Windows XP,
            if( !userhome )
            {
                 userhome = getenv("UserProfile");
                 if( userhome && verbose > 1 )
                     CONS_Printf("UserProfile = %s\n", userhome);
            }
#endif
        }

        if (!userhome)
        {
#if !defined( __DJGPP__ )					
            if(verbose)
                GenPrintf(EMSG_ver, "Please set $HOME to your home directory, or use -home switch\n");

#if 0
            // [WDJ] Using current directory just lead to the user losing
            // the config and savegames.

            // Try to use current directory and defaults
            // Make an absolute default directory, not root.
            if( defdir_stat )
            {
                // have working default dir
                // userhome cannot be "", because save games can end up in root directory
                cat_filename( dirbuf, defdir, DEFHOME );
                userhome = strdup(dirbuf);  // malloc
                if(verbose)
                    GenPrintf(EMSG_ver, " Using userhome= %s\n", userhome );
            }
            else
            {
                GenPrintf(EMSG_warn, " No home dir, and no defdir.\n" );
            }
#endif
#endif
        }

#ifdef LAUNCHER       
        if( (! userhome_parm || init_sequence == 0)
          && userhome )
        {
            // Save the input userhome for the Launcher, unless it came from -home.
            CV_Set( &cv_home, userhome );
            cv_home.flags &= ~CV_MODIFIED;
        }
#endif

#if defined(__APPLE__) && defined(__MACH__) && defined( EXT_MAC_DIR_SPEC )
        //[segabor] ... ([WDJ] MAC port has vars handy)
//        sprintf(configfile, "%s/DooMLegacy.cfg", mac_user_home);
        cat_filename( configfile, mac_user_home, "DooMLegacy.cfg" );
        sprintf(savegamename, "%s/Saved games/Game %%d.doomSaveGame", mac_user_home);
        if ( ! userhome)
            userhome = mac_user_home;
        // legacyhome = strdup( mac_user_home );
        // Needs slash
        legacyhome = (char*) malloc( strlen(userhome) + 3 );
        // example: "/home/user/"
        sprintf(legacyhome, "%s/", userhome);
#else
        // Find the legacyhome directory
        if (userhome)
        {
            // [WDJ] find directory, .doomlegacy, or .legacy
            char dirpath[ MAX_WADPATH ];

            // form directory filename, with slash (for savegamename)
            cat_filename( dirpath, userhome, DEFAULTDIR1 SLASH );
            // if it exists then use it
            if( access(dirpath, R_OK) < 0 )  // not found
            {
                // not there, try 2nd choice
                cat_filename( dirpath, userhome, DEFAULTDIR2 SLASH );
                if( access(dirpath, R_OK) < 0 )  // not found
                {
                    // not there either, then make primary default dir
                    cat_filename( dirpath, userhome, DEFAULTDIR1 SLASH );
                }
            }
            // make subdirectory in userhome
            // example: "/home/user/.doomlegacy/"
            legacyhome = strdup( dirpath );  // malloc
        }
        else
        {
            // Check for an existing DEFHOME in current directory.
            // Only if user has made it.
            if( access(DEFHOME, R_OK) == 0 )  // legacy home found
            {
                legacyhome = strdup( DEFHOME );  // malloc, will be free.
            }
        }

        if( ! legacyhome )
        {
            // Make a default legacy home in the program directory.
            // default absolute path, do not set to ""
        #if defined( __DJGPP__ )		
             getcwd(dosroot, MAX_WADPATH-1);
             Posix_Deform_Path(dosroot);
             progdir = dosroot;
             sprintf(dirbuf, "%s\\", dosroot);
             legacyhome = strdup( dirbuf );
        #else						
            cat_filename( dirbuf, progdir, DEFHOME );
            legacyhome = strdup( dirbuf );  // malloc, will be free.
        #endif
            if( verbose )
                GenPrintf(EMSG_ver, "Default legacyhome= %s\n", legacyhome );
        }

        // Make the legacyhome directory
        if( access(legacyhome, R_OK) < 0 )  // not found
        {
            if( verbose )
                GenPrintf(EMSG_ver, "MKDIR legacyhome= %s\n", legacyhome );
            I_mkdir( legacyhome, 0700);
        }
        legacyhome_len = strlen(legacyhome);
       
        // [WDJ] configfile must be set whereever legacyhome is on DOS or WIN32
        {
            const char * cfgstr;
            // user specific config file
            // little hack to allow a different config file for opengl
            // may be a problem if opengl cannot really be started
            if (M_CheckParm("-opengl"))
            {
                // example: /home/user/.legacy/glconfig.cfg
                cfgstr = "gl" CONFIGFILENAME;
            }
            else if( devparm )
            {
                // example: /home/user/.legacy/devdataconfig.cfg
                cfgstr = DEVDATA CONFIGFILENAME;
            }
            else
            {
                // example: /home/user/.legacy/config.cfg
                cfgstr = CONFIGFILENAME;
            }
            cat_filename( configfile, legacyhome, cfgstr );
        }

#ifdef SAVEGAMEDIR
        // default savegame file name, example: "/home/user/.legacy/%s/doomsav%i.dsg"
//        sprintf(savegamename, "%s%%s" SLASH "%s", legacyhome, text[NORM_SAVEI_NUM]);
        snprintf(savegamename, MAX_WADPATH-1, "%s%%s" SLASH "%s%s", legacyhome, SAVEGAMENAME, "%d.dsg");
        // so can extract legacyhome from savegamename later
#else    
        // default savegame file name, example: "/home/user/.legacy/doomsav%i.dsg"
//        sprintf(savegamename, "%s%s", legacyhome, text[NORM_SAVEI_NUM]);
        snprintf(savegamename, MAX_WADPATH-1, "%s%s%s", legacyhome, SAVEGAMENAME, "%d.dsg");
#endif
        savegamename[MAX_WADPATH-1] = '\0';
#endif

        // [WDJ] Would have a doomwaddir in the config file too, but LoadConfig
        // is done way too late for that.
        for( wdi=0; wdi<(sizeof(dirlist)/sizeof(char*)); wdi++ )
        {
            char ** dwp = & doomwaddir[wdi+DOOMWADDIR_DIRLIST]; // where it goes in doomwaddir
            if( *dwp && (*dwp != dirlist[wdi]) )
                free( *dwp );  // was malloc
            // Default searches
            if( dirlist[wdi] && dirlist[wdi][0] == '~' )
            {
                // Relative to user home.
                cat_filename( dirbuf, (userhome? userhome : "" ), &dirlist[wdi][2]);
                *dwp = strdup( dirbuf );  // (malloc)
            }
            else
            {
                *dwp = dirlist[wdi];  // (ref)
            }
        }
    }
#if 0 || defined( __DJGPP__ )
    if (M_CheckParm("-listdirs") || M_CheckParm("-ld"))    
    Print_search_directories( EMSG_debug, 0x0F );  
#endif

    if( verbose )
    {
        GenPrintf(EMSG_ver, "Config: %s\n", configfile );
        GenPrintf(EMSG_ver, "Savegames: %s\n", savegamename );
    }

    // identify the main IWAD file to use
    IdentifyVersion();  // game, iwad
    modifiedgame = false;

    // Title page
    const char *gametitle = gamedesc.startup_title;  // set by IdentifyVersion
    if( gametitle == NULL )   gametitle = gamedesc.gname;
    if( gametitle )
      GenPrintf(EMSG_info, "%s\n", gametitle);

    // add any files specified on the command line with -file wadfile
    // to the wad list
    //
    // convenience hack to allow -wart e m to add a wad file
    // prepend a tilde to the filename so wadfile will be reloadable
    p = M_CheckParm("-wart");
    if (p)
    {
        // big hack, change to -warp so a later CheckParm does the warp.
        myargv[p][4] = 'p';

        // Map name handling.  Form wad name from map/episode numbers.
        switch (gamemode)
        {
            case doom_shareware:
            case ultdoom_retail:
            case doom_registered:
                sprintf(fbuf, "~" DEVMAPS "E%cM%c.wad", myargv[p + 1][0], myargv[p + 2][0]);
                GenPrintf(EMSG_info, "Warping to Episode %s, Map %s.\n", myargv[p + 1], myargv[p + 2]);
                break;

            case doom2_commercial:
            default:
                p = atoi(myargv[p + 1]);
                if (p < 10)
                    sprintf(fbuf, "~" DEVMAPS "cdata/map0%i.wad", p);
                else
                    sprintf(fbuf, "~" DEVMAPS "cdata/map%i.wad", p);
                break;
        }
        D_AddFile(fbuf);
        // continue and execute -warp
    }

    if (M_CheckParm("-file"))
    {
        // the parms after p are wadfile/lump names,
        // until end of parms or another - preceded parm
        modifiedgame = true;    // homebrew levels
        while (M_IsNextParm())
            D_AddFile( M_GetNextParm() );
    }

    // load dehacked file
    p = M_CheckParm("-dehacked");
    if (!p)
        p = M_CheckParm("-deh");        //Fab:02-08-98:like Boom & DosDoom
    if (p != 0)
    {
        while (M_IsNextParm())
            D_AddFile( M_GetNextParm() );
    }

#ifdef FRENCH_INLINE
    french_text();
#endif

#ifdef BEX_LANGUAGE
    if ( M_CheckParm("-lang") )
    {
        // will check for NULL parameter
        BEX_load_language( M_GetNextParm(), 2 );  // language name
    }
#ifdef BEX_LANG_AUTO_LOAD
    else
    {
        BEX_load_language( NULL, 2 );  // default language name
    }
#endif
#endif

    // Load wad, including the main wad file.
    // This will read DEH and BEX files.  Need devparm and verbose.
    if( W_Init_MultipleFiles(startupwadfiles) == 0 )
    {
       // Some wad failed to load.
       if( !M_CheckParm( "-noloadfail" ) )
         fatal_error = true;
    }
    
    if ( !M_CheckParm("-nocheckwadversion") )
        D_CheckWadVersion();


    //Hurdler: someone wants to keep those lines?
    //BP: i agree with you why should be registered to play someone wads ?
    //    unfortunately most additional wad have more texture and monsters
    //    that shareware wad do, so there will miss resource :(

    if ( gamedesc.gameflags & GD_idwad )
    {
      // [WDJ] These warnings only apply to id iwad files, and should not
      // appear when only using FreeDoom or third party iwads.

      // Check for -file in shareware
      if (modifiedgame)
      {
        // These are the lumps that will be checked in IWAD,
        // if any one is not present, execution will be aborted.
        char name[23][8] = {
            "e2m1", "e2m2", "e2m3", "e2m4", "e2m5", "e2m6", "e2m7", "e2m8", "e2m9",
            "e3m1", "e3m3", "e3m3", "e3m4", "e3m5", "e3m6", "e3m7", "e3m8", "e3m9",
            "dphoof", "bfgga0", "heada1", "cybra1", "spida1d1"
        };

        if (gamemode == doom_shareware)
            CONS_Printf("\nYou shouldn't use -file with the shareware version. Register!\n");

        // Check for fake IWAD with right name,
        // but w/o all the lumps of the registered version.
        if (gamemode == doom_registered)
        {
            int i;
            for (i = 0; i < 23; i++)
            {
                if( ! VALID_LUMP( W_CheckNumForName(name[i]) ) )
                    CONS_Printf("\nThis is not the registered version.");
            }
        }
      }
   
      // If additonal PWAD files are used, print modified banner
      if (modifiedgame)
          CONS_Printf(text[MODIFIED_NUM]);

      // Check and print which version is executed.
      switch (gamemode)
      {
        case doom_shareware:
        case indetermined:
            CONS_Printf(text[SHAREWARE_NUM]);
            break;
        case doom_registered:
        case ultdoom_retail:
        case doom2_commercial:
            CONS_Printf(text[COMERCIAL_NUM]);
            break;
        default:
            // Ouch.
            break;
      }
    }
   
    EOUT_flags = EOUT_text | EOUT_log | EOUT_con;

#if defined( __DJGPP__ )   
    V_Clear_Display();		
    I_RequestConGraphics();
#endif  
#ifdef LAUNCHER   
    if ( fatal_error || init_sequence == 1 || (init_sequence == 0 && myargc < 2 ))
    {
        // [WDJ] Invoke built-in launcher command line
#if 0
# if 0
        setmodeneeded = VID_GetModeForSize(800,600);
# endif
        V_SetPalette(0);
        SCR_SetMode();      // change video mode
        SCR_Recalc();
#endif
        if ( fatal_error )
        {
            CONS_Printf("Fatal error display: (press ESC to continue).\n");
            con_destlines = BASEVIDHEIGHT;
            do
            {
                CON_Draw_Console();
                I_OsPolling();
                D_Process_Events ();  // menu and console responder
                CON_Ticker ();
                I_UpdateNoBlit();
                I_FinishUpdate();       // page flip or blit buffer
            } while( con_destlines>0 );
        }
        init_sequence = 1;
        M_LaunchMenu();  // changes init_sequence > 1 to exit restart loop

        // restart
        Clear_SoftError();
        D_Clear_Files();
        con_Printf( "Launcher restart:\n" );
        goto restart_command;
    }
#endif
    
    //--------------------------------------------------------- 
    // After this line, commit to the initial game and video port selected.
    // Use I_Error.

    if( ! VALID_LUMP( W_CheckNumForName ( "PLAYPAL" ) ) )
    {
        //Hurdler: I'm tired of that question ;)
        I_Error (
        "The main IWAD file is not found, or does not have PLAYPAL lump.\n"
        "The IWAD can be either doom.wad, doom1.wad, doom2.wad, tnt.wad\n"
        "plutonia.wad, heretic.wad, or heretic1.wad from any shareware\n"
        "or commercial version of Doom or Heretic, or some other IWAD!\n"
        "Cannot use legacy.wad, nor a PWAD, for an IWAD.\n" );
    }

    if( fatal_error )
    {
        I_Error ( "Shutdown due to fatal error.\n" );
    }

    //----------------------------------------------------
   
    // Load the config before the full graphics.
   
    M_FirstLoadConfig();        // WARNING : this do a "COM_BufExecute()"

    //---------------------------------------------------- READY SCREEN
    // we need to check for dedicated before initialization of some subsystems
    dedicated = M_CheckParm("-dedicated") != 0;
    if( dedicated )
    {
        nodrawers = true;
        vid.draw_ready = 0;
        drawmode_recalc = false;
        I_ShutdownGraphics();
        EOUT_flags = EOUT_log;
    }
    else
    {
        set_drawmode = cv_drawmode.EV;
        req_bitpp = 0;  // because of launcher looping
        req_alt_bitpp = 0;

        if( M_CheckParm("-highcolor") )
        {
            set_drawmode = DRM_explicit_bpp;  // 15 or 16 bpp
            req_bitpp = 16;
            req_alt_bitpp = 15;
        }
        if( M_CheckParm("-truecolor") )
        {
            set_drawmode = DRM_explicit_bpp;  // 24 or 32 bpp
            req_bitpp = 32;
            req_alt_bitpp = 24;
        }
        if( M_CheckParm("-native") )
        {
            set_drawmode = DRM_native;  // bpp of the default screen
        }
        p = M_CheckParm("-bpp");  // specific bit per pixel color
        if( p )
        {
            // binding, should fail if cannot find a mode
            req_bitpp = atoi(myargv[p + 1]);
            if( ! V_CanDraw( req_bitpp ) )
            {
#ifdef LAUNCHER   
              I_SoftError( "-bpp invalid\n");
              goto restart_command;
#else
              I_Error( "-bpp invalid\n");
#endif
            }
            set_drawmode = DRM_explicit_bpp;
        }

        // Allow a config file for opengl to overload the config settings.
        // It may be edited to set only what settings should be specific to opengl.
        // May be a problem if opengl cannot really be started.

        if( M_CheckParm("-opengl") )
        {
            set_drawmode = DRM_opengl; // opengl temporary
        }
#ifdef SMIF_WIN_NATIVE
#ifdef HWRENDER   
        else if( M_CheckParm ("-3dfx") || M_CheckParm ("-glide") )
        {
            set_drawmode = DRM_glide; // glide temporary
        }
        else if( M_CheckParm ("-minigl") ) // MiniGL is considered to be opengl
        {
            set_drawmode = DRM_minigl; // opengl temporary
        }
        else if( M_CheckParm ("-d3d") )
        {
            set_drawmode = DRM_d3d; // D3D temporary
        }
#endif
#endif
        //--------------------------------------------------------- CONSOLE
        // setup loading screen
        CONS_Printf("RequestFullGraphics...\n");
        V_switch_drawmode( set_drawmode );  // command line, do not change config files
#ifdef DEBUG_WINDOWED
        I_RequestFullGraphics( false );
#else
        I_RequestFullGraphics( cv_fullscreen.EV );
#endif
        // text only, incomplete for rendering
        I_Rendermode_setup();  // need HWR_SetPalette
        SCR_Recalc();
        V_SetPalette (0);  // on new screen
        V_Clear_Display();

#ifdef HWRENDER
        // Set the rendermode patch storage.
        HWR_patchstore = (rendermode > render_soft);
        EN_HWR_flashpalette = 0;  // software and default
        if( rendermode != render_soft )
            HWR_Startup_Render();  // hardware render init
#endif
        // we need the font of the console
        CONS_Printf(text[HU_INIT_NUM]);
        // switch off use_font1 when hu_font is loaded
        HU_Load_Graphics();  // dependent upon dedicated and game

        CON_Init_Video();  // dependent upon vid, hu_font
        EOUT_flags = EOUT_log | EOUT_con;
    }

    // get skill / episode / map from parms
    gameskill = sk_medium;
    startepisode = 1;
    startmap = 1;
    autostart = false;

    p = M_CheckParm("-skill");
    if (p && p < myargc - 1)
    {
        gameskill = myargv[p + 1][0] - '1';
        autostart = true;
    }

    p = M_CheckParm("-episode");
    if (p && p < myargc - 1)
    {
        startepisode = myargv[p + 1][0] - '0';
        startmap = 1;
        autostart = true;
    }

    p = M_CheckParm("-warp");
    if (p && p < myargc - 1)
    {
        if (gamemode == doom2_commercial)
            startmap = atoi(myargv[p + 1]);
        else
        {
            startepisode = myargv[p + 1][0] - '0';
            if (p < myargc - 2 && myargv[p + 2][0] >= '0' && myargv[p + 2][0] <= '9')
                startmap = myargv[p + 2][0] - '0';
            else
                startmap = 1;
        }
        autostart = true;
    }

    // [WDJ] This triggers the first draw to the screen,
    // debug it here instead of waiting for CONS_Printf in BloodTime_OnChange
    CONS_Printf( "Init after ...\n" );

    CONS_Printf(text[W_INIT_NUM]);
    // adapt tables to legacy needs
    P_PatchInfoTables();

    if (gamemode == heretic)
    {
        Heretic_PatchEngine();
#ifdef FRENCH_INLINE
        french_heretic();
#endif
    }

    if(gamemode == chexquest1)
    {
        Chex1_PatchEngine();
#ifdef FRENCH_INLINE
        french_chexquest();
#endif
    }

    B_Init_Bots();       //added by AC for acbot

    //--------------------------------------------------------- CONFIG.CFG
//    M_FirstLoadConfig();        // WARNING : this do a "COM_BufExecute()"

    // set user default mode or mode set at cmdline
    SCR_CheckDefaultMode();

    wipegamestate = gamestate;
    //------------------------------------------------ COMMAND LINE PARAMS

#ifdef CDMUS
    // Initialize CD-Audio, no music on a dedicated server
    if (!M_CheckParm("-nocd") && ! dedicated )
      I_InitCD();
#endif

    if (M_CheckParm("-splitscreen"))
        CV_SetParam(&cv_splitscreen, 1);
   
    // Affects only the game started at the command line.
    // CV_NETVAR are sent by string, CV_SAVE are saved by string.
    nomonsters = M_CheckParm("-nomonsters");

    if (M_CheckParm("-respawn"))
      CV_SetParam( &cv_respawnmonsters, 1 );  // NETVAR
//      COM_BufAddText("respawnmonsters 1\n");
    if (M_CheckParm("-coopmonsters"))
      CV_SetParam( &cv_monbehavior, 1 );  // NETVAR, SAVE
//      COM_BufAddText("monsterbehavior 1\n");
    if (M_CheckParm("-infight"))
      CV_SetParam( &cv_monbehavior, 2 );  // NETVAR, SAVE
//      COM_BufAddText("monsterbehavior 2\n");
    if (M_CheckParm("-teamplay"))
      CV_SetParam( &cv_teamplay, 1 );  // NETVAR
//        COM_BufAddText("teamplay 1\n");
    if (M_CheckParm("-teamskin"))
      CV_SetParam( &cv_teamplay, 2 );  // NETVAR
//        COM_BufAddText("teamplay 2\n");
    // Setting deathmatch also sets cv_itemrespawn (d_netcmd).
    if (M_CheckParm("-altdeath"))
      CV_SetParam( &cv_deathmatch, 2 );  // NETVAR
//      COM_BufAddText("deathmatch 2\n");
    else if (M_CheckParm("-deathmatch"))
      CV_SetParam( &cv_deathmatch, 1 );  // NETVAR
//        COM_BufAddText("deathmatch 1\n");
    if (M_CheckParm("-fast"))
      CV_SetParam( &cv_fastmonsters, 1 );  // NETVAR
//        COM_BufAddText("fastmonsters 1\n");
    //added by AC
    if (M_CheckParm("-predicting"))
      CV_SetParam( &cv_predictingmonsters, 1 );  // NETVAR
//        COM_BufAddText("predictingmonsters 1\n");

    if (M_CheckParm("-timer"))
    {
        char *s = M_GetNextParm();
        if( s == NULL )
        {
            I_SoftError( "Switch  -timer <seconds>\n" );
        }
        else
        {
            // May be larger than EV, so cannot use CV_SetParam.
            CV_Set( &cv_timelimit, s );  // NETVAR
//            COM_BufAddText(va("timelimit %s\n", s));
        }
    }

    if (M_CheckParm("-avg"))
    {
        // May be larger than EV, so cannot use CV_SetParam.
        CV_SetValue( &cv_timelimit, 20 );  // NETVAR
//        COM_BufAddText("timelimit 20\n");
        CONS_Printf(text[AUSTIN_NUM]);
    }

    // turbo option, is not meant to be saved in config, still
    // supported at cmd-line for compatibility
    if (M_CheckParm("-turbo"))
    {
        if( M_IsNextParm() )
        {
            COM_BufAddText(va("turbo %s\n", M_GetNextParm()));
        }
        else
        {
            I_SoftError( "Switch  -turbo <10-255>\n" );
        }
    }

    // push all "+" parameter at the command buffer
    M_PushSpecialParameters();

    CONS_Printf(text[M_INIT_NUM]);
    M_Configure();

    CONS_Printf(text[R_INIT_NUM]);
    R_Init();

    //
    // setting up sound
    //
    CONS_Printf(text[S_SETSOUND_NUM]);
    nosoundfx = M_CheckParm("-nosound");
    nomusic = M_CheckParm("-nomusic");
    // Music init is in I_StartupSound
    I_StartupSound();
#if defined( __DJGPP__ )		
    I_InitMusic();      // setup music buffer for quick mus2mid  
#endif		
    S_Init(cv_soundvolume.value, cv_musicvolume.value);

    CONS_Printf(text[ST_INIT_NUM]);
    ST_Init();

    // SoM: Init FraggleScript
    T_Init_FS();

    // init all NETWORK
    CONS_Printf(text[D_CHECKNET_NUM]);
    if (D_Startup_NetGame())
        autostart = true;

    // check for a driver that wants intermission stats
    p = M_CheckParm("-statcopy");
    if (p && p < myargc - 1)
    {
        I_SoftError("Sorry but statcopy isn't supported at this time\n");
        /*
           // for statistics driver
           extern  void*   statcopy;

           statcopy = (void*)atoi(myargv[p+1]);
           CONS_Printf (text[STATREG_NUM]);
         */
    }

    // start the apropriate game based on parms
    p = M_CheckParm("-record");
    if (p && p < myargc - 1)
    {
        G_RecordDemo(myargv[p + 1]);
        autostart = true;
    }

    // demo doesn't need anymore to be added with D_AddFile()
    p = M_CheckParm("-playdemo");
    if( !p && M_CheckParm("-timedemo") )
      p = 2500;  // indicate timedemo
    if (p)
    {
      if( ! M_IsNextParm() )
      {
        I_SoftError( "Switch  -playdemo <name>  or  -timedemo <name> \n" );
      }
      else
      {
        char demo_name[MAX_WADPATH];  // filename
        // add .lmp to identify the EXTERNAL demo file
        // it is NOT possible to play an internal demo using -playdemo,
        // rather push a playdemo command.. to do.

        strncpy(demo_name, M_GetNextParm(), MAX_WADPATH-1);
        demo_name[MAX_WADPATH-1] = '\0';
        // get spaced filename or directory
        while (M_IsNextParm())
        {
            // [WDJ] Protect against long demo name on command line
            int dn_free = MAX_WADPATH - 2 - strlen(demo_name);
            if( dn_free > 1 )
            {
                strcat(demo_name, " ");
                strncat(demo_name, M_GetNextParm(), dn_free );
                demo_name[MAX_WADPATH-1] = '\0';
            }
        }
        FIL_DefaultExtension(demo_name, ".lmp");


        if( p == 2500 )
        {  // timedemo
            CONS_Printf("Timing Benchmark Demo %s.\n", demo_name);					
            G_TimeDemo(demo_name);
        }
        else
        {  // playdemo
            CONS_Printf("Playing demo %s.\n", demo_name);				
            singledemo = true;  // quit after one demo
            G_DeferedPlayDemo(demo_name);
        }

        gamestate = wipegamestate = GS_NULL;

        return;
      }
    }

    p = M_CheckParm("-loadgame");
    if (p && p < myargc - 1)
    {
        G_Load_Game(atoi(myargv[p + 1]));
    }
    else
    {
        if (dedicated && server)
        {
            pagename = "TITLEPIC";
            gamestate = GS_WAITINGPLAYERS;
        }
        else if (autostart || netgame
                 || M_CheckParm("+connect") || M_CheckParm("-connect"))
        {
            //added:27-02-98: reset the current version number
            G_setup_VERSION();
            gameaction = ga_nothing;
            if (server && !M_CheckParm("+map"))
                COM_BufAddText(va("map \"%s\"\n", G_BuildMapName(startepisode, startmap)));
        }
        else
            D_StartTitle();     // start up intro loop

    }
    Clear_SoftError();
}


// Print error and continue game [WDJ] 1/19/2009
#define SOFTERROR_LISTSIZE   8
static const char *  SE_msg[SOFTERROR_LISTSIZE];
static uint32_t      SE_val[SOFTERROR_LISTSIZE]; // we only want to compare
static int  SE_msgcnt = 0;
static int  SE_next_msg_slot = 0;

byte  EOUT_flags = EOUT_text | EOUT_log;  // EOUT_e

static void Clear_SoftError(void)
{
   SE_msgcnt = 0;
}


// Print out error and continue program.  Maintains list of errors and
// does not repeat error messages in recent history.
void I_SoftError (const char *errmsg, ...)
{
    va_list     argptr;
    int         index;
    uint32_t    errval;

    // Message first.
    va_start (argptr,errmsg);
    errval = va_arg( argptr, uint32_t ) ; // sample it as an int, no matter what
    va_end (argptr);
//  debug_Printf("errval=%d\n", errval );   // debug
    for( index = 0; index < SE_msgcnt; index ++ ){
       if( errmsg == SE_msg[index] ){
          if( errval == SE_val[index] ) goto done;	// it is a repeat msg
       }
    }
    // save comparison info
    SE_msg[SE_next_msg_slot] = errmsg;
    SE_val[SE_next_msg_slot] = errval;
    SE_next_msg_slot++;
    if( SE_next_msg_slot > SE_msgcnt )
       SE_msgcnt = SE_next_msg_slot;  // max
    if( SE_next_msg_slot >= SOFTERROR_LISTSIZE )
       SE_next_msg_slot = 0;  // wrap
    // Error, always prints EMSG_text
    fprintf (stderr, "Warn: ");
    va_start (argptr,errmsg);
    GenPrintf_va( EMSG_error, errmsg, argptr );  // handles EOUT_con
    va_end (argptr);

done:   
    fflush( stderr );
}


// The system independent quit and save config.
void D_Quit_Save ( quit_severity_e severity )
{
    // Prevent recursive I_Quit(), mainly due to situation problems.
    static byte quitseq = 0;
    uint16_t *  endtext = NULL;

    // If this gets called twice, it cannot just return.
    // Some shutdown routine called I_Quit again due to an error and we
    // cannot get back to the previous invocation to finish the shutdown.
    if( quitseq == 0 )
    {
        quitseq = 1;
        //added:16-02-98: when recording a demo, should exit using 'q' key,
        //   but sometimes we forget and use 'F10'.. so save here too.
        if (demorecording)
           G_CheckDemoStatus();
    }
    if( quitseq < 2 )
    {
        quitseq = 2;
        D_Quit_NetGame ();
    }
    if( quitseq < 5 )
    {
        quitseq = 5;
        I_ShutdownSound();
    }
#ifdef CDMUS
    if( quitseq < 6 )
    {
        quitseq = 6;
        I_ShutdownCD();
    }
#endif
    if( quitseq < 8 )
    {
        quitseq = 8;
        if( severity == QUIT_normal )
            M_SaveConfig (NULL);
    }
    if( quitseq < 10 )
    {
        quitseq = 10;
        if( (severity == QUIT_normal)
             && ! M_CheckParm("-noendtxt") )
        {
            // [WDJ] Check on errors during I_Error shutdown.
            // Avoid repeat errors during bad environment shutdown.
            lumpnum_t endtxt_num = W_CheckNumForName("ENDOOM");
            if( VALID_LUMP(endtxt_num) )
                endtext = W_CacheLumpNum( endtxt_num, PU_STATIC );
            // If there are any more errors, then do not show the end text.
        }
    }
    if( quitseq < 11 )
    {
        quitseq = 11;
        // Close open wad files.
        // Neccesity for a Mac.  Open files hang devices.
        W_Shutdown();
    }
    if( quitseq < 15 )
    {
        quitseq = 15;
#if !defined (__DJGPP__)				
        I_Shutdown_IO();
#endif
    }
    if( quitseq < 20 )
    {
        quitseq = 20;
        if( severity != QUIT_normal )
            I_Sleep( 3000 );  // to see some messages
        vid.draw_ready = 0;        
        I_ShutdownGraphics();
        HU_Release_Graphics();
        if( HWR_patchstore )
        {
#ifdef HWRENDER
            HWR_Shutdown_Render();
#endif       
        }
    }
    if( quitseq < 22 )
    {
        quitseq = 22;
        I_ShutdownSystem();
    }
    if( quitseq < 29 )
    {
        quitseq = 29;
        if( (severity == QUIT_normal)
          && endtext )
        {
            // Show the ENDOOM text
            printf("\r");
            I_Show_EndText( endtext );
        }
    }
#if defined (__DJGPP__) && defined LOGMESSAGES
    if (logstream) fclose(logstream);
#endif		
}

// I_Quit exits with (exit 0).
// No return
void I_Quit (void)
{
    D_Quit_Save( QUIT_normal );
    I_Quit_System();  // No Return
}


static void Help( void )
{
  char * np = M_GetNextParm();
   
  if( np == NULL )
  {
    printf
       ("Usage: doomlegacy"
#ifdef HWRENDER	
        " [-opengl]"
#endif
        " [-iwad xxx.wad] [-file pwad.wad ...]\n"
        "--version   Print Doom Legacy version\n"
        "-h     Help\n"
        "-h g   Help game and wads\n"
        "-h m   Help multiplayer\n"
        "-h c   Help config\n"
        "-h s   Help server\n"
        "-h d   Help demo\n"
        "-h D   Help Devmode\n"
        );
     return;
  }
  switch( np[0] )
  {
   case 'g': // game
     printf
       (
        "-game name      doomu, doom2, tnt, plutonia, freedoom, heretic, chex1, etc.\n"
        "-iwad file      The game wad\n"
        "-file file      Load DEH and PWAD files (one or more)\n"
        "-deh  file      Load DEH files (one or more)\n"
        "-loadgame num   Load savegame num\n"  
        "-episode 2      Goto episode 2, level 1\n"
        "-skill 3        Skill 1 to 5\n"
        "-warp 13        Goto map13\n"
        "-warp 1 3       Goto episode 1 level 3\n"
        "-nomonsters     No monsters\n"
        "-respawn        Monsters respawn after killed\n"
        "-coopmonsters   Monsters cooperate\n"
        "-infight        Monsters fight each other\n"
        "-fast           Monsters are fast\n"
        "-predicting     Monsters aim better\n"
        "-turbo num      Player speed %%, 10 to 255\n"   
        );
     break;
   case 'm': // multiplayer
     printf
       (
        "-teamplay       Play with teams by color\n"
        "-teamskin       Play with teams using skins\n"
        "-splitscreen    Two players on this screen\n"
        "-deathmatch     Deathmatch, weapons respawn\n"
        "-altdeath       Deathmatch, items respawn\n"
        "-timer num      Timelimit in minutes\n"
        "-avg            Austin 20 min rounds\n"
        );
     break;
   case 'c': // config
     printf
       (
#if defined (__DJGPP__)			 
        "-v   -v2        Verbose (activte Log \"LEGACY.LOG\")\n"
#else
        "-v   -v2        Verbose\n"
#endif			
        "-home name      Config and savegame directory\n"
        "-config file    Config file\n"
#ifdef HWRENDER		
        "-opengl         OpenGL hardware renderer\n"
#endif
        "-nosound        No sound effects\n"
#ifdef CDMUS
        "-nocd           No CD music\n"
#endif
        "-nomusic        No music\n"
        "-precachesound  Preload sound effects\n"
        "-mb num         Pre-allocate num MiB of memory\n"
        "-width num      Video mode width\n"
        "-height num     Video mode height\n"
        "-highcolor      Request 15bpp or 16bpp\n"
        "-truecolor      Request 24bpp or 32bpp\n"
        "-native         Video mode in native bpp\n"
        "-bpp num        Video mode in (8,15,16,24,32) bpp\n"
        "-nocheckwadversion   Ignore legacy.wad version\n"
#ifdef BEX_LANGUAGE
        "-lang name      Load BEX language file name.bex\n"
#endif
#if defined (__DJGPP__)
        "-listdirs/-ld   Lists supportet search directorys\n"
        "-listwads/-lw   Lists supportet iWads\n"
#endif
        );
     break;
   case 's': // server
     printf
       (
        "-server         Start as game server\n"
        "-dedicated      Dedicated server, no player\n"
        "-connect name   Connect to server name\n"
        "-bandwidth bps  Net bandwidth in bytes/sec\n"
        "-packetsize num Net packetsize\n"
        "-nodownload     No download from server\n"
        "-nofiles        Download all from server\n"
        "-clientport x   Use port x for client\n"
        "-udpport x      Use udp port x for server (and client)\n"
#ifdef USE_IPX
        "-ipx            Use IPX\n"
#endif
        "-extratic x     Send redundant player movement\n"
        "-debugfile file Log to debug file\n"
        "-left           Left slaved view\n"
        "-right          Right slaved view\n"
        "-screendeg x    Slaved view at x degrees\n"
        );
     break;
   case 'd': // demo
     printf
       (
        "-record file    Record demo to file\n"
        "-maxdemo num    Limit record demo size, in KiB\n"
        "-playdemo file  Play demo from file\n"
        );
     break;
   case 'D': // devmode
     printf
       (
        "-devparm        Develop mode\n"
        "-devgame gamename  Develop mode, and specify game\n"
        "-wart 3 1       Load file devmaps/E3M1.wad, then warp to it\n"
        "-wart 13        Load file devmaps/cdata/map13.wad, then warp to it\n"
        "-timedemo file  Timedemo from file\n"
        "-nodraw         Timedemo without draw\n"
        "-noblit         Timedemo without blit\n"
        );
     break;
  }
}
#if defined (__DJGPP__)
void ListSupportetWads(void)
{
  
 int iWad_index = NUM_GDESC; // nothing
 int gmi;
 int wad;
 game_desc_t     iWadSupport;	 // active desc 

 printf("DoomLegacy Supportet: %d Games\n",iWad_index);

 for( gmi=0; gmi<iWad_index-1; gmi++ )
 {
   iWadSupport = game_desc_table[gmi];

   if ((gmi + 1) % 4 == 0)
   {
    printf("\n--- Press Space, Enter to continue or ESC to abort ---");
    fflush(stdout);

    int c;
    do {
       c = getch();            // wartet auf Tastendruck (kein Enter nötig)
       } while (c != ' ' && c != 13 && c != 27);  // Space, Enter oder ESC

       if (c == 27) {              // ESC = Abbruch
         printf("\nAbgebrochen.\n");
         break;
       }
       printf("\r                                      \r"); // Zeile löschen
   }
   else
     printf("\n");

    if (iWadSupport.startup_title == NULL)
    {
        if (iWadSupport.gameflags & GD_unsupported)
            printf("Supportet: %-15s [Not Supportet]\n",iWadSupport.gname);
        else
            printf("Supportet: %-15s\n",iWadSupport.gname);
    }
    else
    {
       if (iWadSupport.gameflags & GD_unsupported)
           printf("Supportet: %-15s [Not Supportet]\n",iWadSupport.startup_title);
       else
           printf("Supportet: %-15s\n",iWadSupport.startup_title);
    }

    printf("Directory: C:\\%s\\\n",iWadSupport.idstr);

    for( wad=0; wad<3-1; wad++ )
    {
      if (iWadSupport.iwad_filename[wad] == NULL)
          continue;			
      
      printf("     iWad: %s\n",iWadSupport.iwad_filename[wad]);
    }

    if (iWadSupport.support_wad != NULL)
        printf("Support : %s\n",iWadSupport.support_wad);			
    }
}

void ListSupportetWads_Command(void)
{
    printf("%s\n", legacytitle);
    ListSupportetWads();
    exit(0);    
}
  
#endif

