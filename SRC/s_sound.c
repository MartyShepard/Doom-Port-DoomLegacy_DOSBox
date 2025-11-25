// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: s_sound.c 1028 2013-08-14 00:15:29Z wesleyjohnson $
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2013 by DooM Legacy Team.
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
// $Log: s_sound.c,v $
// Revision 1.33  2003/07/14 21:22:24  hurdler
// go RC1
//
// Revision 1.32  2003/07/13 13:16:15  hurdler
// go RC1
//
// Revision 1.31  2002/12/13 22:34:27  ssntails
// MP3/OGG support!
//
// Revision 1.30  2002/09/19 21:47:05  judgecutor
// no message
//
// Revision 1.29  2002/09/12 20:10:51  hurdler
// Added some cvars
//
// Revision 1.28  2002/08/16 20:19:36  judgecutor
// Sound pitching coming back
//
// Revision 1.27  2001/08/20 20:40:39  metzgermeister
// *** empty log message ***
//
// Revision 1.26  2001/05/27 13:42:48  bpereira
// no message
//
// Revision 1.25  2001/04/30 17:19:24  stroggonmeth
// HW fix and misc. changes
//
// Revision 1.24  2001/04/18 19:32:26  hurdler
// no message
//
// Revision 1.23  2001/04/17 22:26:07  calumr
// Initial Mac add
//
// Revision 1.22  2001/04/04 20:24:21  judgecutor
// Added support for the 3D Sound
//
// Revision 1.21  2001/04/02 18:54:32  bpereira
// no message
//
// Revision 1.20  2001/04/01 17:35:07  bpereira
// no message
//
// Revision 1.19  2001/03/03 11:11:49  hurdler
// I hate warnigs ;)
//
// Revision 1.18  2001/02/24 13:35:21  bpereira
// no message
//
// Revision 1.17  2001/01/27 11:02:36  bpereira
// no message
//
// Revision 1.16  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.15  2000/11/21 21:13:18  stroggonmeth
// Optimised 3D floors and fixed crashing bug in high resolutions.
//
// Revision 1.14  2000/11/12 21:59:53  hurdler
// Please verify that sound bug
//
// Revision 1.13  2000/11/03 11:48:40  hurdler
// Fix compiling problem under win32 with 3D-Floors and FragglScript (to verify!)
//
// Revision 1.12  2000/11/02 17:50:10  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.11  2000/10/27 20:38:20  judgecutor
// - Added the SurroundSound support
//
// Revision 1.10  2000/09/28 20:57:18  bpereira
// no message
//
// Revision 1.9  2000/05/07 08:27:57  metzgermeister
// no message
//
// Revision 1.8  2000/04/22 16:16:50  emanne
// Correction de l'interface.
// Une erreur s'y était glissé, d'où un segfault si on compilait sans SDL.
//
// Revision 1.7  2000/04/21 08:23:47  emanne
// To have SDL working.
// Makefile: made the hiding by "@" optional. See the CC variable at
// the begining. Sorry, but I like to see what's going on while building
//
// qmus2mid.h: force include of qmus2mid_sdl.h when needed.
// s_sound.c: ??!
// s_sound.h: with it.
// (sorry for s_sound.* : I had problems with cvs...)
//
// Revision 1.6  2000/03/29 19:39:48  bpereira
// no message
//
// Revision 1.5  2000/03/22 18:51:08  metzgermeister
// introduced I_PauseCD() for Linux
//
// Revision 1.4  2000/03/12 23:21:10  linuxcub
// Added consvars which hold the filenames and arguments which will be used
// when running the soundserver and musicserver (under Linux). I hope I
// didn't break anything ... Erling Jacobsen, linuxcub@email.dk
//
// Revision 1.3  2000/03/06 15:13:08  hurdler
// maybe a bug detected
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:  
//
//
//-----------------------------------------------------------------------------

#ifdef MUSSERV
#include <sys/msg.h>
struct musmsg
{
    long msg_type;
    char msg_text[12];
};
extern int msg_id;
#endif

#include "doomdef.h"
#include "doomstat.h"
#include "command.h"
#include "g_game.h"
#include "m_argv.h"
#include "r_main.h"     //R_PointToAngle2() used to calc stereo sep.
#include "r_things.h"   // for skins
#include "p_info.h"

#include "i_sound.h"
#include "s_sound.h"
#include "w_wad.h"
#include "z_zone.h"
#include "d_main.h"

#include "m_random.h"

// 3D Sound Interface
#include "hardware/hw3sound.h"

// commands for music and sound servers
#ifdef MUSSERV
consvar_t musserver_cmd = { "musserver_cmd", "musserver", CV_SAVE };
consvar_t musserver_arg = { "musserver_arg", "-t 20 -f -u 0", CV_SAVE };
#endif
#ifdef SNDSERV
consvar_t sndserver_cmd = { "sndserver_cmd", "llsndserv", CV_SAVE };
consvar_t sndserver_arg = { "sndserver_arg", "-quiet", CV_SAVE };
#endif

#define SURROUND

#ifdef MACOS_DI
// specific to macos directory
consvar_t play_mode = { "play_mode", "0", CV_SAVE, CV_Unsigned };
#endif

    // pause cd music
#if defined( __DJGPP__ )	
    #define I_PauseCD(void) I_StopCD(void);
#endif

// stereo reverse 1=true, 0=false
consvar_t stereoreverse = { "stereoreverse", "0", CV_SAVE, CV_OnOff };

// if true, all sounds are loaded at game startup
consvar_t precachesound = { "precachesound", "0", CV_SAVE, CV_OnOff };

CV_PossibleValue_t soundvolume_cons_t[] = { {0, "MIN"}, {31, "MAX"}, {0, NULL} };

// actual general (maximum) sound & music volume, saved into the config
consvar_t cv_soundvolume = { "soundvolume", "15", CV_SAVE, soundvolume_cons_t };
consvar_t cv_musicvolume = { "musicvolume", "15", CV_SAVE, soundvolume_cons_t };
consvar_t cv_rndsoundpitch = { "rndsoundpitch", "Off", CV_SAVE, CV_OnOff };

// number of channels available
static void SetChannelsNum(void);
consvar_t cv_numChannels = { "snd_channels", "16", CV_SAVE | CV_CALL, CV_Unsigned, SetChannelsNum };

#ifdef SURROUND
consvar_t cv_surround = { "surround", "0", CV_SAVE, CV_OnOff };
#endif

#define S_MAX_VOLUME            127

// when to clip out sounds
// Does not fit the large outdoor areas.
// added 2-2-98 in 8 bit volume control (befort  (1200*0x10000))
#define S_CLIPPING_DIST         (1200*0x10000)

// Distance tp origin when sounds should be maxed out.
// This should relate to movement clipping resolution
// (see BLOCKMAP handling).
// Originally: (200*0x10000).
// added 2-2-98 in 8 bit volume control (befort  (160*0x10000))
#define S_CLOSE_DIST            (160*0x10000)

// added 2-2-98 in 8 bit volume control (befort  remove the +4)
#define S_ATTENUATOR            ((S_CLIPPING_DIST-S_CLOSE_DIST)>>(FRACBITS+4))

// Adjustable by menu.
#define NORM_VOLUME             snd_MaxVolume

#define NORM_PITCH              128
#define NORM_PRIORITY           64
#define NORM_SEP                128

#define S_PITCH_PERTURB         1
#define S_STEREO_SWING          (96*0x10000)

#ifdef SURROUND
#define SURROUND_SEP            -128
#endif

// percent attenuation from front to back
#define S_IFRACVOL              30

typedef struct
{
    // sound information (if null, channel avail.)
    sfxinfo_t *sfxinfo;

    // origin of sound
    void *origin;

    // handle of the sound being played
    int handle;

} channel_t;

// the set of channels available
static channel_t *channels;

// whether songs are mus_paused
static boolean mus_paused;

// music currently being played
static musicinfo_t *mus_playing = NULL;

// [WDJ] unused
#ifdef CLEANUP
static int nextcleanup;
#endif

//
// Internals.
//
int S_AdjustSoundParams(mobj_t * listener, mobj_t * source, int *vol, int *sep, int *pitch);

static void S_StopChannel(int cnum);

void S_RegisterSoundStuff(void)
{
    if (dedicated)
        return;

    //added:11-04-98: stereoreverse
    CV_RegisterVar(&stereoreverse);
    CV_RegisterVar(&precachesound);

#ifdef SNDSERV
    CV_RegisterVar(&sndserver_cmd);
    CV_RegisterVar(&sndserver_arg);
#endif
#ifdef MUSSERV
    CV_RegisterVar(&musserver_cmd);
    CV_RegisterVar(&musserver_arg);
#endif
#ifdef SURROUND
    CV_RegisterVar(&cv_surround);
#endif

#if 0
//[WDJ]  disabled in 143beta_macosx
//[segabor]
#ifdef MACOS_DI        //mp3 playlist stuff
// specific to macos directory
    {
        int i;
        for (i = 0; i < PLAYLIST_LENGTH; i++)
        {
            user_songs[i].name = malloc(7);
            sprintf(user_songs[i].name, "song%i%i", i / 10, i % 10);
            user_songs[i].defaultvalue = malloc(1);
            *user_songs[i].defaultvalue = 0;
            user_songs[i].flags = CV_SAVE;
            user_songs[i].PossibleValue = NULL;
            CV_RegisterVar(&user_songs[i]);
        }
        CV_RegisterVar(&play_mode);
    }
#endif
#endif
}

static void SetChannelsNum(void)
{
    int i;

    // Allocating the internal channels for mixing
    // (the maximum number of sounds rendered
    // simultaneously) within zone memory.
    if (channels)
        Z_Free(channels);

#ifdef HW3SOUND
    if (hws_mode != HWS_DEFAULT_MODE)
    {
        HW3S_SetSourcesNum();
        return;
    }
#endif
    channels = (channel_t *) Z_Malloc(cv_numChannels.value * sizeof(channel_t), PU_STATIC, 0);

    // Free all channels for use
    for (i = 0; i < cv_numChannels.value; i++)
        channels[i].sfxinfo = 0;

}

void S_InitRuntimeMusic()
{
    int i;

    for (i = mus_firstfreeslot; i < mus_lastfreeslot; i++)
        S_music[i].name = NULL;
}


// [WDJ] Common routine to handling sfx names and get the sound lump.
// Much easier to maintain here.
// Replace S_GetSfxLumpNum
// Called by I_GetSfx
void S_GetSfxLump( sfxinfo_t * sfx )
{
    char name[20] = "\0\0\0\0\0\0\0\0";  // do not leave this to chance [WDJ]
    int sfxlump;

    if (gamemode == heretic){	// [WDJ] heretic names are different
       sprintf(name, "%s", sfx->name);
    }else{
       sprintf(name, "ds%s", sfx->name);
    }

    // Now, there is a severe problem with the sound handling,
    // in it is not (yet/anymore) gamemode aware. That means, sounds from
    // DOOM II will be requested even with DOOM shareware.
    // The sound list is wired into sounds.c, which sets the external variable.
    // I do not do runtime patches to that variable. Instead, we will use a
    // default sound for replacement.

    if (W_CheckNumForName(name) == -1)
    {
        if( verbose > 1 )
	    fprintf(stderr,"Sound missing: %s, Using default sound\n",name);  // [WDJ] debug
	// Heretic shareware: get many missing sound names at sound init,
	// but not after game starts.  These come from list of sounds
	// in sounds.c, but not all those are in the game.
        if (gamemode == heretic)
            sfxlump = W_GetNumForName("keyup");
        else
            sfxlump = W_GetNumForName("dspistol");
    }
    else
    {
        sfxlump = W_GetNumForName(name);
    }
    // if lump not found, W_GetNumForName would have done I_Error
    sfx->lumpnum = sfxlump;

    // Get the sound data from the WAD, allocate lump
    //  in zone memory.
    sfx->length = W_LumpLength(sfxlump);
    sfx->data = (void *) W_CacheLumpNum(sfxlump, PU_SOUND);
   
    // caller must fix size and data ptr for the mixer
}


// [WDJ] Common routine to Get data for a sfx
static void S_GetSfx( sfxinfo_t * sfx )
{
    if ( sfx->name )
    {
//        CONS_Printf ("cached sound %s\n", sfx->name);
        if (sfx->link)
        {
	    // NOTE: linked sounds use the link data at StartSound time
	    // Example is the chaingun sound linked to pistol.
	    if( ! sfx->link->data )
	        I_GetSfx( sfx->link );
	    // Linked to previously loaded
	    sfx->data = sfx->link->data;
	    sfx->length = sfx->link->length;
	}
        else
        {
	    // Load data from WAD file.
	    I_GetSfx( sfx );
	}
    }
}

// [WDJ] Common routine to Free data for a sfx
void S_FreeSfx( sfxinfo_t * sfx )
{
    if( sfx->link )  // do not free linked data
    {
        sfx->data = NULL;
    }
    else if( sfx->data )
    {
        I_FreeSfx( sfx );  // some must free their own buffers

        if( sfx->data )    // if not already free
        {
	    Z_Free( sfx->data );
	    sfx->data = NULL;
	}
    }
}


//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//
void S_Init(int sfxVolume, int musicVolume)
{
    int i;

    if (dedicated)
        return;

    //CONS_Printf( "S_Init: default sfx volume %d\n", sfxVolume);

    S_SetSfxVolume(sfxVolume);
    S_SetMusicVolume(musicVolume);

    SetChannelsNum();

    // no sounds are playing, and they are not mus_paused
    mus_paused = false;

    // Note that sounds have not been cached (yet).
    for (i = 1; i < NUMSFX; i++)
    {
        sfxinfo_t * sfx = & S_sfx[i];
        sfx->lumpnum = sfx->usefulness = -1;    // for I_GetSfx()
        sfx->data = NULL;
        sfx->length = 0;
#if 1
        // [WDJ] Single Saw sound fix.
        // SFX_saw marks some additional sounds that may need SFX_single.
	// The need for these may be obsolete.
        // Need to know the situation that required single saw sound.
	if( sfx->flags & SFX_saw )
	   sfx->flags |= SFX_single;
#endif
    }

    //
    //  precache sounds if requested by cmdline, or precachesound var true
    //
    if (!nosoundfx && (M_CheckParm("-precachesound") || precachesound.value))
    {
        // Initialize external data (all sounds) at start, keep static.
//        CONS_Printf("Loading sounds... ");
        CONS_Printf("Caching sound data (%d sfx)... ", NUMSFX);

        for (i = 1; i < NUMSFX; i++)
        {
            // NOTE: linked sounds use the link's data at StartSound time
            if (S_sfx[i].name && !S_sfx[i].link)
	        S_GetSfx( & S_sfx[i] );
        }

        CONS_Printf(" pre-cached all sound data\n");
    }

    S_InitRuntimeMusic();
}


//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//

//SoM: Stop all sounds, load level info, THEN start sounds.
void S_StopSounds()
{
    int cnum;

#ifdef HW3SOUND
    if (hws_mode != HWS_DEFAULT_MODE)
    {
        HW3S_StopSounds();
        return;
    }
#endif

    // kill all playing sounds at start of level
    //  (trust me - a good idea)
    for (cnum = 0; cnum < cv_numChannels.value; cnum++)
    {
        S_StopChannel(cnum);  // has all tests needed
    }
}

void S_StartSounds(void)
{
    int mnum;

    // start new music for the level
    mus_paused = false;

    if (gamemode == doom2_commercial)
        mnum = mus_runnin + gamemap - 1;
    else if (gamemode == heretic)
        mnum = mus_he1m1 + (gameepisode - 1) * 9 + gamemap - 1;
    else
    {
        const int spmus[] = {
            // Song - Who? - Where?

            mus_e3m4,   // American     e4m1
            mus_e3m2,   // Romero       e4m2
            mus_e3m3,   // Shawn        e4m3
            mus_e1m5,   // American     e4m4
            mus_e2m7,   // Tim  e4m5
            mus_e2m4,   // Romero       e4m6
            mus_e2m6,   // J.Anderson   e4m7 CHIRON.WAD
            mus_e2m5,   // Shawn        e4m8
            mus_e1m9    // Tim          e4m9
        };

        if (gameepisode < 4)
            mnum = mus_e1m1 + (gameepisode - 1) * 9 + gamemap - 1;
        else
            mnum = spmus[gamemap - 1];
    }

    // HACK FOR COMMERCIAL
    //  if (gamemode==doom2_commercial && mnum > mus_e3m9)
    //      mnum -= mus_e3m9;

    if (info_music && *info_music)
        S_ChangeMusicName(info_music, true);
    else
        S_ChangeMusic(mnum, true);

#ifdef CLEANUP
    nextcleanup = 15;
#endif
}


//
// S_getChannel :
//   Kill origin sounds, dependent upon sfx flags.
//   Reuse the channel, or find another channel.
//   Return channel number, if none available, return -1.
//
static int S_getChannel(void *origin, sfxinfo_t * sfxinfo)
{
    // [WDJ] Like PrBoom, separate channel for player tagged sfx
    uint32_t kill_flags = (sfxinfo->flags & (SFX_player|SFX_saw)) | SFX_org_kill;
    int16_t priority = -1;  // higher is lower priority
    int pick_cnum = -1;
    int cnum;  // channel number to use
    channel_t * c;

    // Find an open channel
    // Stop previous origin sound, so do not break from loop
    // Done in one loop for efficiency
    for (cnum = cv_numChannels.value-1; cnum >= 0 ; cnum--)
    {
        c = & channels[cnum];
        if (! c->sfxinfo)   // empty
        {
            pick_cnum = cnum;
	    priority = 0x3FFF;  // lowest priority
	    continue;
	}
        // stop previous origin sound
        if (origin && c->origin == origin)
        {
	    // reuse channel with same origin, flags, when SFX_org_kill
	    if((c->sfxinfo->flags & (SFX_player|SFX_saw|SFX_org_kill)) == kill_flags )
	        goto reuse_cnum;
	}
        // find lowest priority ( 256 is lowest )
        if (c->sfxinfo->priority > priority)
        {
	    pick_cnum = cnum;
	    priority = c->sfxinfo->priority;
	}
    }

    cnum = pick_cnum;
    if( pick_cnum >= 0 )
    {
        if( priority > 1024 )  // found empty
	    goto use_cnum;
        if( sfxinfo->priority <= priority )  // can replace this sound
	    goto reuse_cnum;
    }
    // No lower priority.  Sorry, Charlie.
    return -1;

 reuse_cnum:
    S_StopChannel(cnum);
 use_cnum:   
    c = &channels[cnum];

    // channel is decided to be cnum.
    c->sfxinfo = sfxinfo;
    c->origin = origin;

    return cnum;
}


void S_StartSoundAtVolume(void *origin_p, int sfx_id, int volume)
{
    int sep = 0;  // compiler
    int pitch;
    int priority;
    sfxinfo_t *sfx;
    int cnum;

    mobj_t *origin = (mobj_t *) origin_p;

    if (nosoundfx || (origin && origin->type == MT_SPIRIT))
        goto done;

#ifdef HW3SOUND
    if (hws_mode != HWS_DEFAULT_MODE)
    {
        HW3S_StartSound(origin, sfx_id);
        goto done;
    };
#endif

    // Debug.
    /*
       fprintf( stderr,
       "S_StartSoundAtVolume: playing sound %d (%s)\n",
       sfx_id, S_sfx[sfx_id].name );
     */

#ifdef PARANOIA
    // check for bogus sound #
    if (sfx_id < 1 || sfx_id > NUMSFX)
    {
        I_SoftError("Bad sfx #: %d\n", sfx_id);
        goto done;
    }
#endif

    sfx = &S_sfx[sfx_id];

    if ((sfx->skinsound >= 0) && origin && origin->skin)
    {
        // it redirect player sound to the sound in the skin table
        sfx_id = ((skin_t *) origin->skin)->soundsid[sfx->skinsound];
        sfx = &S_sfx[sfx_id];
    }

    // Initialize sound parameters
    if (sfx->link)
    {
        pitch = sfx->pitch;
        priority = sfx->priority;
        volume += sfx->volume;

        if (volume < 1)
            goto done;

        // added 2-2-98 SfxVolume is now the hardware volume, don't mix up
        //    if (volume > SfxVolume)
        //      volume = SfxVolume;

        // update reference from link, it may have been purged
        sfx->data = sfx->link->data;
        sfx->length = sfx->link->length;
    }
    else
    {
        pitch = NORM_PITCH;
        priority = NORM_PRIORITY;
    }

    // Check to see if it is audible,
    //  and if not, modify the params

    //added:16-01-98:changed consoleplayer to displayplayer
    //[WDJ] added displayplayer2_ptr tests, stop segfaults
    if (origin && origin != displayplayer_ptr->mo
	&& !(cv_splitscreen.value && displayplayer2_ptr && origin == displayplayer2_ptr->mo))
    {
        int rc, rc2;
        int volume2 = volume, sep2 /*=sep*/ , pitch2 = pitch;
        rc = S_AdjustSoundParams(displayplayer_ptr->mo, origin, &volume, &sep, &pitch);
        if (cv_splitscreen.value && displayplayer2_ptr)
        {
 	    // splitscreen sound for player2
            rc2 = S_AdjustSoundParams(displayplayer2_ptr->mo, origin, &volume2, &sep2, &pitch2);
            if (!rc2)
            {
                if (!rc)
                    goto done;
            }
            else if (!rc || (rc && volume2 > volume))
            {
                volume = volume2;
                sep = sep2;
                pitch = pitch2;
                if (origin->x == displayplayer2_ptr->mo->x
		    && origin->y == displayplayer2_ptr->mo->y)
                {
                    sep = NORM_SEP;
                }
            }
        }
        else if (!rc)
	    goto done;

        if (origin->x == displayplayer_ptr->mo->x
	    && origin->y == displayplayer_ptr->mo->y)
        {
            sep = NORM_SEP;
        }
    }
    else
    {
        sep = NORM_SEP;
    }

    // hacks to vary the sfx pitches

    //added:16-02-98: removed by Fab, because it used M_Random() and it
    //                was a big bug, and then it doesnt change anything
    //                dont hear any diff. maybe I'll put it back later
    //                but of course not using M_Random().
    //added 16-08-02: added back by Judgecutor
    //Sound pitching for both Doom and Heretic
    if (cv_rndsoundpitch.value)
    {
        if (gamemode == heretic)
        {
            pitch = 128 + (M_Random() & 7);
	    pitch -= (M_Random() & 7);
	}
        else
        {
            if (sfx_id >= sfx_sawup && sfx_id <= sfx_sawhit)
                pitch += 8 - (M_Random() & 15);
            else if (sfx_id != sfx_itemup && sfx_id != sfx_tink)
                pitch += 16 - (M_Random() & 31);
        }
    }

    if (pitch < 0)
        pitch = NORM_PITCH;
    if (pitch > 255)
        pitch = 255;

    // Kill origin sound, reuse channel, or find a channel
    // Dependent upon sfx flags
    cnum = S_getChannel(origin, sfx);
    if (cnum < 0)
        goto done;

    // cache data if necessary
    // NOTE : set sfx->data NULL sfx->lump -1 to force a reload
    if (!sfx->data)
        S_GetSfx( sfx );
   
#ifdef CLEANUP
    // increase the usefulness
    if (sfx->usefulness++ < 0)
        sfx->usefulness = -1;
#endif

    // [WDJ] From PrBoom, wad dakills has zero length sounds
    // (DSBSPWLK, DSBSPACT, DSSWTCHN, DSSWTCHX)
    if (sfx->length <= 0)
       goto done;

#ifdef SURROUND
    // judgecutor:
    // Avoid channel reverse if surround
    if (stereoreverse.value && sep != SURROUND_SEP)
        sep = (~sep) & 255;
#else
    //added:11-04-98:
    if (stereoreverse.value)
        sep = (~sep) & 255;
#endif

    //CONS_Printf("stereo %d reverse %d\n", sep, stereoreverse.value);

    // Returns a handle to a mixer/output channel.
    channels[cnum].handle =
      I_StartSound(sfx_id, volume, sep, pitch, priority);
done:
    return;
}

void S_StartSound(void *origin, int sfx_id)
{
    // the volume is handled 8 bits

#ifdef HW3SOUND
    if (hws_mode != HWS_DEFAULT_MODE)
        HW3S_StartSound(origin, sfx_id);
    else
#endif
        S_StartSoundAtVolume(origin, sfx_id, 255);
}

void S_StopSound(void *origin)
{
    int cnum;

    // SoM: Sounds without origin can have multiple sources, they shouldn't
    // be stopped by new sounds.
    if (!origin)
        return;

#ifdef HW3SOUND
    if (hws_mode != HWS_DEFAULT_MODE)
    {
        HW3S_StopSound(origin);
        return;
    }
#endif
    for (cnum = 0; cnum < cv_numChannels.value; cnum++)
    {
        if (channels[cnum].sfxinfo && channels[cnum].origin == origin)
        {
	    if( (channels[cnum].sfxinfo->flags & SFX_org_kill) )
	    {
	        S_StopChannel(cnum);
	    }
	}
    }
}

//
// Stop and resume music, during game PAUSE.
//
void S_PauseSound(void)
{
    if (mus_playing && !mus_paused)
    {
        I_PauseSong(mus_playing->handle);
        mus_paused = true;
    }

#ifdef CDMUS
    // pause cd music
    I_PauseCD();
#endif
}

void S_ResumeSound(void)
{
    if (mus_playing && mus_paused)
    {
        I_ResumeSong(mus_playing->handle);
        mus_paused = false;
    }

#ifdef CDMUS
    // resume cd music
    I_ResumeCD();
#endif
}

//
// Updates music & sounds
//
static int actualsfxvolume;     //check for change through console
static int actualmusicvolume;

void S_UpdateSounds(void)
{
    int audible;
    int cnum;
    int volume;
    int sep;
    int pitch;
    sfxinfo_t *sfx;
    channel_t *c;

    mobj_t *listener = displayplayer_ptr->mo;

    if (dedicated)
        return;

    // Update sound/music volumes, if changed manually at console
    if (actualsfxvolume != cv_soundvolume.value)
        S_SetSfxVolume(cv_soundvolume.value);
    if (actualmusicvolume != cv_musicvolume.value)
        S_SetMusicVolume(cv_musicvolume.value);

#ifdef HW3SOUND
    if (hws_mode != HWS_DEFAULT_MODE)
    {
        HW3S_UpdateSources();
        return;
    }
#endif

#ifdef CLEANUP
       Clean up unused data.
       if (gametic > nextcleanup)
       {
       for (i=1 ; i<NUMSFX ; i++)
       {
       if (S_sfx[i].usefulness==0)
       {
       //S_sfx[i].usefulness--;

       // don't forget to unlock it !!!
       // __dmpi_unlock_....
       //Z_ChangeTag(S_sfx[i].data, PU_CACHE);
       //S_sfx[i].data = 0;

       CONS_Printf ("\2flushed sfx %.6s\n", S_sfx[i].name);
       }
       }
       nextcleanup = gametic + 15;
       }
#endif

    for (cnum = 0; cnum < cv_numChannels.value; cnum++)
    {
        c = &channels[cnum];
        sfx = c->sfxinfo;

        if (c->sfxinfo)
        {
            if (I_SoundIsPlaying(c->handle))
            {
                // initialize parameters
                volume = 255;   //8 bits internal volume precision
                pitch = NORM_PITCH;
                sep = NORM_SEP;

                if (sfx->link)  // strange (BP)
                {
                    pitch = sfx->pitch;
                    volume += sfx->volume;
                    if (volume < 1)
                    {
                        S_StopChannel(cnum);
                        continue;
                    }
                }

                // check non-local sounds for distance clipping
                //  or modify their params
                if (c->origin && listener != c->origin
		    && !(cv_splitscreen.value && displayplayer2_ptr && c->origin == displayplayer2_ptr->mo))
                {
                    int audible2;
                    int volume2 = volume, sep2 = sep, pitch2 = pitch;
                    audible = S_AdjustSoundParams(listener, c->origin, &volume, &sep, &pitch);

                    if (cv_splitscreen.value && displayplayer2_ptr)
                    {
		        // splitscreen sound for player2
                        audible2 = S_AdjustSoundParams(displayplayer2_ptr->mo, c->origin, &volume2, &sep2, &pitch2);
                        if (audible2 && (!audible || (audible && volume2 > volume)))
                        {
                            audible = true;
                            volume = volume2;
                            sep = sep2;
                            pitch = pitch2;
                        }
                    }

                    if (!audible)
                    {
                        S_StopChannel(cnum);
                    }
                    else
                        I_UpdateSoundParams(c->handle, volume, sep, pitch);
                }
            }
            else
            {
                // if channel is allocated but sound has stopped,
                //  free it
                S_StopChannel(cnum);
            }
        }
    }
    // kill music if it is a single-play && finished
    // if (     mus_playing
    //      && !I_QrySongPlaying(mus_playing->handle)
    //      && !mus_paused )
    // S_StopMusic();

}

void S_SetMusicVolume(int volume)
{
    if (volume < 0 || volume > 31)
        CONS_Printf("musicvolume should be between 0-31\n");

    volume &= 31;

    CV_SetValue(&cv_musicvolume, volume);
    actualmusicvolume = cv_musicvolume.value;   //check for change of var

    I_SetMusicVolume(volume);

#ifdef __DJGPP__
    I_SetMusicVolume(31);       //faB: this is a trick for buggy dos drivers.. I think.
#endif
}

void S_SetSfxVolume(int volume)
{
    if (volume < 0 || volume > 31)
        CONS_Printf("sfxvolume should be between 0-31\n");

    CV_SetValue(&cv_soundvolume, volume & 31);
    actualsfxvolume = cv_soundvolume.value;     //check for change of var

#ifdef HW3SOUND
    hws_mode == HWS_DEFAULT_MODE ? I_SetSfxVolume(volume & 31) : HW3S_SetSfxVolume(volume & 31);
#else
    // now hardware volume
    I_SetSfxVolume(volume & 31);
#endif

}

//
// Starts some music with the music id found in sounds.h.
//
void S_StartMusic(int m_id)
{
    S_ChangeMusic(m_id, true);
}

//
// S_ChangeMusicName
// Changes music by name
void S_ChangeMusicName(char *name, int looping)
{
    int music;

    if (!strncmp(name, "-", 6))
    {
        S_StopMusic();
        return;
    }

    music = S_FindMusic(name);

    if (music > mus_None && music < NUMMUSIC)
        S_ChangeMusic(music, looping);
    else
    {
        CONS_Printf("music not found: %s\n", name);
        S_StopMusic();  // stop music anyway
    }
}

void S_ChangeMusic(int music_num, int looping)
{
    musicinfo_t *music;

    if (dedicated)
        return;

    if (nomusic)
        return;

    if ((music_num <= mus_None) || (music_num >= NUMMUSIC))
    {
        CONS_Printf("ERROR: Bad music number %d\n", music_num);
        return;
    }
    else
        music = &S_music[music_num];

    if (mus_playing == music)
        return;

    // shutdown old music
    S_StopMusic();

    // get lumpnum if neccessary
    if (!music->lumpnum)
    {
        if (gamemode == heretic)
            music->lumpnum = W_GetNumForName(music->name);
        else
            music->lumpnum = W_GetNumForName(va("d_%s", music->name));
    }
    // load & register it
    music->data = (void *) W_CacheLumpNum(music->lumpnum, PU_MUSIC);
    music->handle = I_RegisterSong(music->data, W_LumpLength(music->lumpnum));

#ifdef MUSSERV

    if (msg_id != -1)
    {
        struct musmsg msg_buffer;

        msg_buffer.msg_type = 6;
        memset(msg_buffer.msg_text, 0, sizeof(msg_buffer.msg_text));
        sprintf(msg_buffer.msg_text, "d_%s", music->name);
        msgsnd(msg_id, (struct msgbuf *) &msg_buffer, sizeof(msg_buffer.msg_text), IPC_NOWAIT);
    }

#endif /* #ifdef MUSSERV */

    // play it
    I_PlaySong(music->handle, looping);

    mus_playing = music;
}


void S_StopMusic()
{
    if (mus_playing)
    {
        if (mus_paused)
            I_ResumeSong(mus_playing->handle);

        I_StopSong(mus_playing->handle);
        I_UnRegisterSong(mus_playing->handle);
	Z_ChangeTag(mus_playing->data, PU_CACHE);

        mus_playing->data = NULL;
        mus_playing = NULL;
    }
}

static void S_StopChannel(int cnum)
{
    channel_t *c = &channels[cnum];

    if (c->sfxinfo)
    {
        // stop the sound playing
        if (I_SoundIsPlaying(c->handle))
        {
            I_StopSound(c->handle);
        }

#if 0
// [WDJ] Does nothing       
        // check to see
        //  if other channels are playing the sound
        int i;
        for (i = 0; i < cv_numChannels.value; i++)
        {
            if (cnum != i && c->sfxinfo == channels[i].sfxinfo)
            {
                break;
            }
        }
#endif

#ifdef CLEANUP
        // degrade usefulness of sound data
        c->sfxinfo->usefulness--;
#endif

        c->sfxinfo = 0;
    }
}

//
// Changes volume, stereo-separation, and pitch variables
//  from the norm of a sound effect to be played.
// If the sound is not audible, returns a 0.
// Otherwise, modifies parameters and returns 1.
//
int S_AdjustSoundParams(mobj_t * listener, mobj_t * source, int *vol, int *sep, int *pitch)
{
    fixed_t approx_dist;
    fixed_t adx;
    fixed_t ady;
    angle_t angle;
   
    if( ! listener )  return 0;  // [WDJ] Stop splitscreen segfault.

    // calculate the distance to sound origin
    //  and clip it if necessary
    adx = abs(listener->x - source->x);
    ady = abs(listener->y - source->y);

    // From _GG1_ p.428. Appox. eucledian distance fast.
    approx_dist = adx + ady - ((adx < ady ? adx : ady) >> 1);

    // Original has MAP08 without sound clipping by distance
    if (gamemap != 8 && approx_dist > S_CLIPPING_DIST)
    {
        return 0;
    }

    // angle of source to listener
    angle = R_PointToAngle2(listener->x, listener->y, source->x, source->y);

    if (angle > listener->angle)
        angle = angle - listener->angle;
    else
        angle = angle + (0xffffffff - listener->angle);

#ifdef SURROUND

    // Produce a surround sound for angle from 105 till 255
    if (cv_surround.value && (angle > (ANG90 + (ANG45 / 3)) && angle < (ANG270 - (ANG45 / 3))))
        *sep = SURROUND_SEP;
    else
    {
#endif

        angle >>= ANGLETOFINESHIFT;

        // stereo separation
        *sep = 128 - (FixedMul(S_STEREO_SWING, finesine[angle]) >> FRACBITS);

#ifdef SURROUND
    }
#endif

    // volume calculation
    if (approx_dist < S_CLOSE_DIST)
    {
        // added 2-2-98 SfxVolume is now hardware volume
        *vol = 255;     //snd_SfxVolume;
    }
    // Original had MAP08 making distant sound effects louder than near.
    // removed hack here for gamemap==8 (it made far sound still present)
    else
    {
        // distance effect
	// Where old equation had snd_SfxVolume this has 15
        *vol = (15 * ((S_CLIPPING_DIST - approx_dist) >> FRACBITS)) / S_ATTENUATOR;
    }

    return (*vol > 0);
}


// SoM: Searches through the channels and checks for origin or id.
// returns 0 of not found, returns 1 if found.
// if id<0, then don't check it...
int S_SoundPlaying(void *origin, int id)
{
    int cnum;

#ifdef HW3SOUND
    if (hws_mode != HWS_DEFAULT_MODE)
    {
        return HW3S_SoundPlaying(origin, id);
    }
#endif

    for (cnum = 0; cnum < cv_numChannels.value; cnum++)
    {
        if (origin && channels[cnum].origin == origin)
            return 1;
        if (id >= 0 && (channels[cnum].sfxinfo - S_sfx == id))
            return 1;
    }
    return 0;
}

//
// S_StartSoundName
// Starts a sound using the given name.
#define MAXNEWSOUNDS 10
int newsounds[MAXNEWSOUNDS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void S_StartSoundName(void *mo, char *soundname)
{
    int i;
    int soundnum = 0;
    //Search existing sounds...
    for (i = sfx_None + 1; i < NUMSFX; i++)
    {
        if (!S_sfx[i].name)
            continue;
        if (!stricmp(S_sfx[i].name, soundname))
        {
            soundnum = i;
            break;
        }
    }

    if (!soundnum)
    {
        for (i = 0; i < MAXNEWSOUNDS; i++)
        {
            if (newsounds[i] == 0)
                break;
            if (!S_SoundPlaying(NULL, newsounds[i]))
            {
                S_RemoveSoundFx(newsounds[i]);
                break;
            }
        }

        if (i == MAXNEWSOUNDS)
        {
            CONS_Printf("Cannot load another extra sound!\n");
            return;
        }

        soundnum = S_AddSoundFx(soundname, 0);
        newsounds[i] = soundnum;
    }

    S_StartSound(mo, soundnum);
}
