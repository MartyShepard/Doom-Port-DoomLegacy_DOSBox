// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: sounds.c 1060 2013-12-13 20:25:47Z wesleyjohnson $
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
// $Log: sounds.c,v $
// Revision 1.10  2001/03/30 17:12:51  bpereira
//
// Revision 1.9  2001/03/13 22:14:20  stroggonmeth
// Long time no commit. 3D floors, FraggleScript, portals, ect.
//
// Revision 1.8  2001/02/24 13:35:21  bpereira
//
// Revision 1.7  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.6  2000/11/21 21:13:18  stroggonmeth
// Optimised 3D floors and fixed crashing bug in high resolutions.
//
// Revision 1.5  2000/11/03 11:48:40  hurdler
// Fix compiling problem under win32 with 3D-Floors and FragglScript (to verify!)
//
// Revision 1.4  2000/11/03 02:37:36  stroggonmeth
// Fix a few warnings when compiling.
//
// Revision 1.3  2000/11/02 17:50:10  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      music/sound tables, and related sound routines
//
// Note: the tables were originally created by a sound utility at Id,
//       kept as a sample, DOOM2 sounds.
//
//-----------------------------------------------------------------------------


#include "doomincl.h"
#include "sounds.h"
#include "s_sound.h"
#include "r_defs.h"
#include "r_things.h"
#include "z_zone.h"
#include "w_wad.h"

// NOTE: add \0 for stringlen=6, to allow dehacked patching

//
// Information about all the music
//

musicinfo_t S_music[NUMMUSIC] =
{
    { 0 },
    { "e1m1\0\0", 0 },
    { "e1m2\0\0", 0 },
    { "e1m3\0\0", 0 },
    { "e1m4\0\0", 0 },
    { "e1m5\0\0", 0 },
    { "e1m6\0\0", 0 },
    { "e1m7\0\0", 0 },
    { "e1m8\0\0", 0 },
    { "e1m9\0\0", 0 },
    { "e2m1\0\0", 0 },
    { "e2m2\0\0", 0 },
    { "e2m3\0\0", 0 },
    { "e2m4\0\0", 0 },
    { "e2m5\0\0", 0 },
    { "e2m6\0\0", 0 },
    { "e2m7\0\0", 0 },
    { "e2m8\0\0", 0 },
    { "e2m9\0\0", 0 },
    { "e3m1\0\0", 0 },
    { "e3m2\0\0", 0 },
    { "e3m3\0\0", 0 },
    { "e3m4\0\0", 0 },
    { "e3m5\0\0", 0 },
    { "e3m6\0\0", 0 },
    { "e3m7\0\0", 0 },
    { "e3m8\0\0", 0 },
    { "e3m9\0\0", 0 },
    { "inter\0" , 0 },
    { "intro\0" , 0 },
    { "bunny\0" , 0 },
    { "victor"  , 0 },
    { "introa"  , 0 },
    { "runnin"  , 0 },
    { "stalks"  , 0 },
    { "countd"  , 0 },
    { "betwee"  , 0 },
    { "doom\0\0", 0 },
    { "the_da"  , 0 },
    { "shawn\0" , 0 },
    { "ddtblu"  , 0 },
    { "in_cit"  , 0 },
    { "dead\0\0", 0 },
    { "stlks2"  , 0 },
    { "theda2"  , 0 },
    { "doom2\0" , 0 },
    { "ddtbl2"  , 0 },
    { "runni2"  , 0 },
    { "dead2\0" , 0 },
    { "stlks3"  , 0 },
    { "romero"  , 0 },
    { "shawn2"  , 0 },
    { "messag"  , 0 },
    { "count2"  , 0 },
    { "ddtbl3"  , 0 },
    { "ampie\0" , 0 },
    { "theda3"  , 0 },
    { "adrian"  , 0 },
    { "messg2"  , 0 },
    { "romer2"  , 0 },
    { "tense\0" , 0 },
    { "shawn3"  , 0 },
    { "openin"  , 0 },
    { "evil\0\0", 0 },
    { "ultima"  , 0 },
    { "read_m"  , 0 },
    { "dm2ttl"  , 0 },
    { "dm2int"  , 0 },
// heretic stuff
        { "MUS_E1M1", 0 }, // 1-1
        { "MUS_E1M2", 0 },
        { "MUS_E1M3", 0 },
        { "MUS_E1M4", 0 },
        { "MUS_E1M5", 0 },
        { "MUS_E1M6", 0 },
        { "MUS_E1M7", 0 },
        { "MUS_E1M8", 0 },
        { "MUS_E1M9", 0 },

        { "MUS_E2M1", 0 }, // 2-1
        { "MUS_E2M2", 0 },
        { "MUS_E2M3", 0 },
        { "MUS_E2M4", 0 },
        { "MUS_E1M4", 0 },
        { "MUS_E2M6", 0 },
        { "MUS_E2M7", 0 },
        { "MUS_E2M8", 0 },
        { "MUS_E2M9", 0 },

        { "MUS_E1M1", 0 }, // 3-1
        { "MUS_E3M2", 0 },
        { "MUS_E3M3", 0 },
        { "MUS_E1M6", 0 },
        { "MUS_E1M3", 0 },
        { "MUS_E1M2", 0 },
        { "MUS_E1M5", 0 },
        { "MUS_E1M9", 0 },
        { "MUS_E2M6", 0 },

        { "MUS_E1M6", 0 }, // 4-1
        { "MUS_E1M2", 0 },
        { "MUS_E1M3", 0 },
        { "MUS_E1M4", 0 },
        { "MUS_E1M5", 0 },
        { "MUS_E1M1", 0 },
        { "MUS_E1M7", 0 },
        { "MUS_E1M8", 0 },
        { "MUS_E1M9", 0 },

        { "MUS_E2M1", 0 }, // 5-1
        { "MUS_E2M2", 0 },
        { "MUS_E2M3", 0 },
        { "MUS_E2M4", 0 },
        { "MUS_E1M4", 0 },
        { "MUS_E2M6", 0 },
        { "MUS_E2M7", 0 },
        { "MUS_E2M8", 0 },
        { "MUS_E2M9", 0 },

        { "MUS_E3M2", 0 }, // 6-1
        { "MUS_E3M3", 0 }, // 6-2
        { "MUS_E1M6", 0 }, // 6-3

        { "MUS_TITL", 0 },
        { "MUS_CPTD", 0 }
};


//
// Information about all the sfx
//

sfxinfo_t S_sfx[NUMSFX] =
{
  // S_sfx[0] needs to be a dummy for odd reasons.
  //   priority (1..256), 1 is highest priority, 64 is avg
  //               |  link           skinsound  
  //               |  |     volume   |  flags
  //               |  |      | pitch |  |
  { "none"     ,   0, NULL, -1, -1, -1 },

  { "pistol"   ,  64, NULL, -1, -1, -1, SFX_saw },
  { "shotgn"   ,  64, NULL, -1, -1, -1 },
  { "sgcock"   ,  64, NULL, -1, -1, -1 },
  { "dshtgn"   ,  64, NULL, -1, -1, -1 },
  { "dbopn\0"  ,  64, NULL, -1, -1, -1 },
  { "dbcls\0"  ,  64, NULL, -1, -1, -1 },
  { "dbload"   ,  64, NULL, -1, -1, -1 },
  { "plasma"   ,  64, NULL, -1, -1, -1 },
  { "bfg\0\0\0",  64, NULL, -1, -1, -1 },
  { "sawup\0"  ,  64, NULL, -1, -1, -1, SFX_saw|SFX_id_fin|SFX_org_kill },
  { "sawidl"   , 118, NULL, -1, -1, -1, SFX_saw|SFX_id_fin|SFX_org_kill },
  { "sawful"   ,  64, NULL, -1, -1, -1, SFX_saw|SFX_id_fin|SFX_org_kill },
  { "sawhit"   ,  64, NULL, -1, -1, -1, SFX_saw|SFX_id_fin|SFX_org_kill },
  { "rlaunc"   ,  64, NULL, -1, -1, -1 },
  { "rxplod"   ,  70, NULL, -1, -1, -1 },
  { "firsht"   ,  70, NULL, -1, -1, -1 },
  { "firxpl"   ,  70, NULL, -1, -1, -1 },
  { "pstart"   , 100, NULL, -1, -1, -1 },
  { "pstop\0"  , 100, NULL, -1, -1, -1 },
  { "doropn"   , 100, NULL, -1, -1, -1, SFX_org_kill },
  { "dorcls"   , 100, NULL, -1, -1, -1, SFX_org_kill },
  { "stnmov"   , 119, NULL, -1, -1, -1, SFX_saw|SFX_id_fin|SFX_org_kill },
  { "swtchn"   ,  78, NULL, -1, -1, -1 },
  { "swtchx"   ,  78, NULL, -1, -1, -1 },
  { "plpain"   ,  96, NULL, -1, -1, SKSPLPAIN},
  { "dmpain"   ,  96, NULL, -1, -1, -1 },
  { "popain"   ,  96, NULL, -1, -1, -1 },
  { "vipain"   ,  96, NULL, -1, -1, -1 },
  { "mnpain"   ,  96, NULL, -1, -1, -1 },
  { "pepain"   ,  96, NULL, -1, -1, -1 },
  { "slop\0\0" ,  78, NULL, -1, -1, SKSSLOP},
  { "itemup"   ,  78, NULL, -1, -1, -1, SFX_single|SFX_player },
  { "wpnup"    ,  78, NULL, -1, -1, -1, SFX_single|SFX_player },
  { "oof\0\0\0",  96, NULL, -1, -1, SKSOOF, SFX_player },
  { "telept"   ,  32, NULL, -1, -1, -1 },
  { "posit1"   ,  98, NULL, -1, -1, -1, SFX_single},
  { "posit2"   ,  98, NULL, -1, -1, -1, SFX_single},
  { "posit3"   ,  98, NULL, -1, -1, -1, SFX_single},
  { "bgsit1"   ,  98, NULL, -1, -1, -1, SFX_single},
  { "bgsit2"   ,  98, NULL, -1, -1, -1, SFX_single},
  { "sgtsit"   ,  98, NULL, -1, -1, -1, SFX_single},
  { "cacsit"   ,  98, NULL, -1, -1, -1, SFX_single},
  { "brssit"   ,  94, NULL, -1, -1, -1, SFX_single},
  { "cybsit"   ,  92, NULL, -1, -1, -1, SFX_single},
  { "spisit"   ,  90, NULL, -1, -1, -1, SFX_single},
  { "bspsit"   ,  90, NULL, -1, -1, -1, SFX_single},
  { "kntsit"   ,  90, NULL, -1, -1, -1, SFX_single},
  { "vilsit"   ,  90, NULL, -1, -1, -1, SFX_single},
  { "mansit"   ,  90, NULL, -1, -1, -1, SFX_single},
  { "pesit\0"  ,  90, NULL, -1, -1, -1, SFX_single},
  { "sklatk"   ,  70, NULL, -1, -1, -1 },
  { "sgtatk"   ,  70, NULL, -1, -1, -1 },
  { "skepch"   ,  70, NULL, -1, -1, -1 },
  { "vilatk"   ,  70, NULL, -1, -1, -1 },
  { "claw\0\0" ,  70, NULL, -1, -1, -1 },
  { "skeswg"   ,  70, NULL, -1, -1, -1 },
  { "pldeth"   ,  32, NULL, -1, -1, SKSPLDETH},
  { "pdiehi"   ,  32, NULL, -1, -1, SKSPDIEHI},
  { "podth1"   ,  70, NULL, -1, -1, -1 },
  { "podth2"   ,  70, NULL, -1, -1, -1 },
  { "podth3"   ,  70, NULL, -1, -1, -1 },
  { "bgdth1"   ,  70, NULL, -1, -1, -1 },
  { "bgdth2"   ,  70, NULL, -1, -1, -1 },
  { "sgtdth"   ,  70, NULL, -1, -1, -1 },
  { "cacdth"   ,  70, NULL, -1, -1, -1 },
  { "skldth"   ,  70, NULL, -1, -1, -1 },
  { "brsdth"   ,  32, NULL, -1, -1, -1 },
  { "cybdth"   ,  32, NULL, -1, -1, -1 },
  { "spidth"   ,  32, NULL, -1, -1, -1 },
  { "bspdth"   ,  32, NULL, -1, -1, -1 },
  { "vildth"   ,  32, NULL, -1, -1, -1 },
  { "kntdth"   ,  32, NULL, -1, -1, -1 },
  { "pedth\0"  ,  32, NULL, -1, -1, -1 },
  { "skedth"   ,  32, NULL, -1, -1, -1 },
  { "posact"   , 120, NULL, -1, -1, -1, SFX_single},
  { "bgact\0"  , 120, NULL, -1, -1, -1, SFX_single},
  { "dmact\0"  , 120, NULL, -1, -1, -1, SFX_single},
  { "bspact"   , 100, NULL, -1, -1, -1, SFX_single},
  { "bspwlk"   , 100, NULL, -1, -1, -1, SFX_single},
  { "vilact"   , 100, NULL, -1, -1, -1, SFX_single},
  { "noway\0"  ,  78, NULL, -1, -1, SKSNOWAY, SFX_player },
  { "barexp"   ,  60, NULL, -1, -1, -1 },
  { "punch\0"  ,  64, NULL, -1, -1, SKSPUNCH },
  { "hoof\0\0" ,  70, NULL, -1, -1, -1 },
  { "metal\0"  ,  70, NULL, -1, -1, -1 },
  { "chgun\0"  ,  64, &S_sfx[sfx_pistol], 150, 0, -1 },
  { "tink\0\0" ,  60, NULL, -1, -1, -1 },
  { "bdopn\0"  , 100, NULL, -1, -1, -1 },
  { "bdcls\0"  , 100, NULL, -1, -1, -1 },
  { "itmbk\0"  , 100, NULL, -1, -1, -1 },
  { "flame\0"  ,  32, NULL, -1, -1, -1 },
  { "flamst"   ,  32, NULL, -1, -1, -1 },
  { "getpow"   ,  60, NULL, -1, -1, -1, SFX_player },
  { "bospit"   ,  70, NULL, -1, -1, -1 },
  { "boscub"   ,  70, NULL, -1, -1, -1 },
  { "bossit"   ,  70, NULL, -1, -1, -1 },
  { "bospn\0"  ,  70, NULL, -1, -1, -1 },
  { "bosdth"   ,  70, NULL, -1, -1, -1 },
  { "manatk"   ,  70, NULL, -1, -1, -1 },
  { "mandth"   ,  70, NULL, -1, -1, -1 },
  { "sssit\0"  ,  70, NULL, -1, -1, -1 },
  { "ssdth\0"  ,  70, NULL, -1, -1, -1 },
  { "keenpn"   ,  70, NULL, -1, -1, -1 },
  { "keendt"   ,  70, NULL, -1, -1, -1 },
  { "skeact"   ,  70, NULL, -1, -1, -1 },
  { "skesit"   ,  70, NULL, -1, -1, -1 },
  { "skeatk"   ,  70, NULL, -1, -1, -1 },
  { "radio\0"  ,  60, NULL, -1, -1, SKSRADIO },

  //added:22-02-98: sound when the player avatar jumps in air 'hmpf!'
  { "jump\0\0" ,  60, NULL, -1, -1, SKSJUMP, SFX_player },
  { "ouch\0\0" ,  64, NULL, -1, -1, SKSOUCH, SFX_player },

  //added:09-08-98:test water sounds
  { "gloop\0"  ,  60, NULL, -1, -1, -1 },
  { "splash"   ,  64, NULL, -1, -1, -1 },
  { "floush"   ,  64, NULL, -1, -1, -1 },

// heretic sounds

  { "gldhit",  32, NULL, -1, -1, -1 },
  { "gntful",  32, NULL, -1, -1, -1 },
  { "gnthit",  32, NULL, -1, -1, -1 },
  { "gntpow",  32, NULL, -1, -1, -1 },
//  { "gntact",  32, NULL, -1, -1, -1 },
  { "gntuse",  32, NULL, -1, -1, -1 },
  { "phosht",  32, NULL, -1, -1, -1 },
  { "phohit",  32, NULL, -1, -1, -1 },
  { "-phopow", 32, &S_sfx[sfx_hedat1], -1, -1, -1 },
  { "lobsht",  20, NULL, -1, -1, -1 },
  { "lobhit",  20, NULL, -1, -1, -1 },
  { "lobpow",  20, NULL, -1, -1, -1 },
  { "hrnsht",  32, NULL, -1, -1, -1 },
  { "hrnhit",  32, NULL, -1, -1, -1 },
  { "hrnpow",  32, NULL, -1, -1, -1 },
  { "ramphit", 32, NULL, -1, -1, -1 },
  { "ramrain", 10, NULL, -1, -1, -1 },
  { "bowsht",  32, NULL, -1, -1, -1 },
  { "stfhit",  32, NULL, -1, -1, -1 },
  { "stfpow",  32, NULL, -1, -1, -1 },
  { "stfcrk",  32, NULL, -1, -1, -1 },
  { "impsit",  32, NULL, -1, -1, -1 },
  { "impat1",  32, NULL, -1, -1, -1 },
  { "impat2",  32, NULL, -1, -1, -1 },
  { "impdth",  80, NULL, -1, -1, -1 },
  { "-impact", 20, &S_sfx[sfx_impsit], -1, -1, -1 },
  { "imppai",  32, NULL, -1, -1, -1 },
  { "mumsit",  32, NULL, -1, -1, -1 },
  { "mumat1",  32, NULL, -1, -1, -1 },
  { "mumat2",  32, NULL, -1, -1, -1 },
  { "mumdth",  80, NULL, -1, -1, -1 },
  { "-mumact", 20, &S_sfx[sfx_mumsit], -1, -1, -1 },
  { "mumpai",  32, NULL, -1, -1, -1 },
  { "mumhed",  32, NULL, -1, -1, -1 },
  { "bstsit",  32, NULL, -1, -1, -1 },
  { "bstatk",  32, NULL, -1, -1, -1 },
  { "bstdth",  80, NULL, -1, -1, -1 },
  { "bstact",  20, NULL, -1, -1, -1 },
  { "bstpai",  32, NULL, -1, -1, -1 },
  { "clksit",  32, NULL, -1, -1, -1 },
  { "clkatk",  32, NULL, -1, -1, -1 },
  { "clkdth",  80, NULL, -1, -1, -1 },
  { "clkact",  20, NULL, -1, -1, -1 },
  { "clkpai",  32, NULL, -1, -1, -1 },
  { "snksit",  32, NULL, -1, -1, -1 },
  { "snkatk",  32, NULL, -1, -1, -1 },
  { "snkdth",  80, NULL, -1, -1, -1 },
  { "snkact",  20, NULL, -1, -1, -1 },
  { "snkpai",  32, NULL, -1, -1, -1 },
  { "kgtsit",  32, NULL, -1, -1, -1 },
  { "kgtatk",  32, NULL, -1, -1, -1 },
  { "kgtat2",  32, NULL, -1, -1, -1 },
  { "kgtdth",  80, NULL, -1, -1, -1 },
  { "-kgtact", 20, &S_sfx[sfx_kgtsit], -1, -1, -1 },
  { "kgtpai",  32, NULL, -1, -1, -1 },
  { "wizsit",  32, NULL, -1, -1, -1 },
  { "wizatk",  32, NULL, -1, -1, -1 },
  { "wizdth",  80, NULL, -1, -1, -1 },
  { "wizact",  20, NULL, -1, -1, -1 },
  { "wizpai",  32, NULL, -1, -1, -1 },
  { "minsit",  32, NULL, -1, -1, -1 },
  { "minat1",  32, NULL, -1, -1, -1 },
  { "minat2",  32, NULL, -1, -1, -1 },
  { "minat3",  32, NULL, -1, -1, -1 },
  { "mindth",  80, NULL, -1, -1, -1 },
  { "minact",  20, NULL, -1, -1, -1 },
  { "minpai",  32, NULL, -1, -1, -1 },
  { "hedsit",  32, NULL, -1, -1, -1 },
  { "hedat1",  32, NULL, -1, -1, -1 },
  { "hedat2",  32, NULL, -1, -1, -1 },
  { "hedat3",  32, NULL, -1, -1, -1 },
  { "heddth",  80, NULL, -1, -1, -1 },
  { "hedact",  20, NULL, -1, -1, -1 },
  { "hedpai",  32, NULL, -1, -1, -1 },
  { "sorzap",  32, NULL, -1, -1, -1 },
  { "sorrise", 32, NULL, -1, -1, -1 },
  { "sorsit",  200,NULL, -1, -1, -1 },
  { "soratk",  32, NULL, -1, -1, -1 },
  { "soract",  200,NULL, -1, -1, -1 },
  { "sorpai",  200,NULL, -1, -1, -1 },
  { "sordsph", 200,NULL, -1, -1, -1 },
  { "sordexp", 200,NULL, -1, -1, -1 },
  { "sordbon", 200,NULL, -1, -1, -1 },
  { "-sbtsit", 32, &S_sfx[sfx_bstsit], -1, -1, -1 },
  { "-sbtatk", 32, &S_sfx[sfx_bstatk], -1, -1, -1 },
  { "sbtdth",  80, NULL, -1, -1, -1 },
  { "sbtact",  20, NULL, -1, -1, -1 },
  { "sbtpai",  32, NULL, -1, -1, -1 },
//  { "plroof",  32, NULL, -1, -1, -1 },
  { "plrpai",  32, NULL, -1, -1, -1 },
  { "plrdth",  80, NULL, -1, -1, -1 },
  { "gibdth",  100,NULL, -1, -1, -1 },
  { "plrwdth", 80, NULL, -1, -1, -1 },
  { "plrcdth", 100,NULL, -1, -1, -1 },
  { "itemup",  32, NULL, -1, -1, -1, SFX_player },
  { "wpnup",   32, NULL, -1, -1, -1, SFX_player },
//  { "telept",  50, NULL, -1, -1, -1 },
  { "doropn",  40, NULL, -1, -1, -1 },
  { "dorcls",  40, NULL, -1, -1, -1 },
  { "dormov",  40, NULL, -1, -1, -1 },
  { "artiup",  32, NULL, -1, -1, -1, SFX_player },
//  { "switch",  40, NULL, -1, -1, -1 },
  { "pstart",  40, NULL, -1, -1, -1 },
  { "pstop",   40, NULL, -1, -1, -1 },
  { "stnmov",  40, NULL, -1, -1, -1 },
  { "chicpai", 32, NULL, -1, -1, -1 },
  { "chicatk", 32, NULL, -1, -1, -1 },
  { "chicdth", 40, NULL, -1, -1, -1 },
  { "chicact", 32, NULL, -1, -1, -1 },
  { "chicpk1", 32, NULL, -1, -1, -1 },
  { "chicpk2", 32, NULL, -1, -1, -1 },
  { "chicpk3", 32, NULL, -1, -1, -1 },
  { "keyup"  , 50, NULL, -1, -1, -1, SFX_player },
  { "ripslop", 16, NULL, -1, -1, -1 },
  { "newpod" , 16, NULL, -1, -1, -1 },
  { "podexp" , 40, NULL, -1, -1, -1 },
  { "bounce" , 16, NULL, -1, -1, -1 },
  { "-volsht", 16, &S_sfx[sfx_bstatk], -1, -1, -1 },
  { "-volhit", 16, &S_sfx[sfx_lobhit], -1, -1, -1 },
  { "burn"   , 10, NULL, -1, -1, -1 },
  { "splash" , 10, NULL, -1, -1, -1 },
  { "gloop"  , 10, NULL, -1, -1, -1 },
//  { "respawn", 10, NULL, -1, -1, -1 },
  { "blssht" , 32, NULL, -1, -1, -1 },
  { "blshit" , 32, NULL, -1, -1, -1 },
//  { "chat"   , 100,NULL, -1, -1, -1 },
  { "artiuse", 32, NULL, -1, -1, -1, SFX_player },
  { "gfrag"  , 100,NULL, -1, -1, -1 },
  { "waterfl", 16, NULL, -1, -1, -1 },

  // Monophonic sounds

  { "wind"   , 16, NULL, -1, -1, -1 },
  { "amb1"   ,  1, NULL, -1, -1, -1 },
  { "amb2"   ,  1, NULL, -1, -1, -1 },
  { "amb3"   ,  1, NULL, -1, -1, -1 },
  { "amb4"   ,  1, NULL, -1, -1, -1 },
  { "amb5"   ,  1, NULL, -1, -1, -1 },
  { "amb6"   ,  1, NULL, -1, -1, -1 },
  { "amb7"   ,  1, NULL, -1, -1, -1 },
  { "amb8"   ,  1, NULL, -1, -1, -1 },
  { "amb9"   ,  1, NULL, -1, -1, -1 },
  { "amb10"  ,  1, NULL, -1, -1, -1 },
  { "amb11"  ,  1, NULL, -1, -1, -1 },
  { "menuud" ,  2, NULL, -1, -1, -1, SFX_single},
  { "menuva" ,  2, NULL, -1, -1, -1, SFX_single},
  { "menuen" ,  2, NULL, -1, -1, -1, SFX_single},
  { "menuop" ,  2, NULL, -1, -1, -1, SFX_single},
  { "menuac" ,  2, NULL, -1, -1, -1, SFX_single}
  // skin sounds free slots to add sounds at run time (Boris HACK!!!)
  // initialized to NULL
};


// Prepare free sfx slots to add sfx at run time
void S_InitRuntimeSounds (void)
{
    int  i;

    for (i=sfx_freeslot0; i<=sfx_lastfreeslot; i++)
        S_sfx[i].name = NULL;
}

// Add a new sound fx into a free sfx slot.
int S_AddSoundFx (char *name, uint32_t flags)
{
    uint16_t least_usefulness = 8000;
    int leastid = 0;
    int sfxid;

    // quick search for free slot
    for(sfxid=sfx_freeslot0; sfxid<NUMSFX; sfxid++)
    {
        // find empty sfx slot
        if(!S_sfx[sfxid].name)
	   goto addsfx;
    }
    // no free slots, try to remove least useful
    // separate loop because the SoundPlaying test is a search
    for(sfxid=sfx_freeslot0; sfxid<NUMSFX; sfxid++)
    {
        // degrade all equally
        if ( S_sfx[sfxid].usefulness > -8000 )
	   S_sfx[sfxid].usefulness --;
        // find least useful
        // all entries must be valid because empty slot search failed
        if ( S_sfx[sfxid].skinsound < 0  // not a skin sound
	     && S_sfx[sfxid].usefulness < least_usefulness)
        {
	    if (!S_SoundPlaying(NULL, sfxid))
	    {
	        least_usefulness = S_sfx[sfxid].usefulness;
	        leastid = sfxid;
	    }
	}
    }
    if ( leastid )
    {
        // clear the leastid slot
        S_RemoveSoundFx( leastid );
        sfxid = leastid;
        goto addsfx;
    }
    CONS_Printf("\2No more free sound slots\n");
    return 0;

 addsfx:
    S_sfx[sfxid].name=(char *)Z_Malloc(7,PU_STATIC,NULL);
    strncpy(S_sfx[sfxid].name,name,6);
    S_sfx[sfxid].name[6]='\0';
    S_sfx[sfxid].flags= flags;
    S_sfx[sfxid].priority=60;
    S_sfx[sfxid].link=0;
    S_sfx[sfxid].pitch=-1;
    S_sfx[sfxid].volume=-1;
    S_sfx[sfxid].lumpnum=-1;
    S_sfx[sfxid].skinsound=-1;
    S_sfx[sfxid].usefulness=-1;

    // if precache load it here ! todo !
    S_sfx[sfxid].data=NULL;
    return sfxid;
}


void S_RemoveSoundFx (int id)
{
    if (id>=sfx_freeslot0 &&
        id<=sfx_lastfreeslot &&
        S_sfx[id].name)
    {
        fprintf(stderr, "RemoveSoundFx: %s\n", S_sfx[id].name );
        S_FreeSfx(&S_sfx[id]);
        Z_Free(S_sfx[id].name);
        S_sfx[id].lumpnum=-1;
        S_sfx[id].name=NULL;  // free sfx slot to use again
    }
}


//
// S_AddMusic
// Adds a single song to the runtime songs.
int S_AddMusic(char *name)
{
  int    i;
  char   lumpname[9];

  sprintf(lumpname, "d_%.6s", name);

  for(i = mus_firstfreeslot; i < mus_lastfreeslot; i++)
  {
    if(S_music[i].name == NULL)
    {
      S_music[i].name = Z_Strdup(name, PU_STATIC, 0);
      S_music[i].lumpnum = W_GetNumForName(lumpname);
      S_music[i].data = 0;
      return i;
    }
  }

  CONS_Printf("All music slots are full!\n");
  return 0;
}


int S_FindMusic(char *name)
{
  int   i;

  for(i = 0; i < NUMMUSIC; i++)
  {
    if(!S_music[i].name)
      continue;
    if(!strcasecmp(name, S_music[i].name)) return i;
  }

  return S_AddMusic(name);
}
