// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: dehacked.c 1417 2019-01-29 08:00:14Z wesleyjohnson $
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
// $Log: dehacked.c,v $
// Revision 1.17  2004/04/20 00:34:26  andyp
// Linux compilation fixes and string cleanups
//
// Revision 1.16  2003/11/21 17:52:05  darkwolf95
// added "Monsters Infight" for Dehacked patches
//
// Revision 1.15  2002/01/12 12:41:05  hurdler
// Revision 1.14  2002/01/12 02:21:36  stroggonmeth
//
// Revision 1.13  2001/07/16 22:35:40  bpereira
// - fixed crash of e3m8 in heretic
// - fixed crosshair not drawed bug
//
// Revision 1.12  2001/06/30 15:06:01  bpereira
// fixed wronf next level name in intermission
//
// Revision 1.11  2001/04/30 17:19:24  stroggonmeth
// HW fix and misc. changes
//
// Revision 1.10  2001/02/10 12:27:13  bpereira
//
// Revision 1.9  2001/01/25 22:15:41  bpereira
// added heretic support
//
// Revision 1.8  2000/11/04 16:23:42  bpereira
//
// Revision 1.7  2000/11/03 13:15:13  hurdler
// Some debug comments, please verify this and change what is needed!
//
// Revision 1.6  2000/11/02 17:50:06  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.5  2000/08/31 14:30:55  bpereira
// Revision 1.4  2000/04/16 18:38:07  bpereira
//
// Revision 1.3  2000/04/05 15:47:46  stroggonmeth
// Added hack for Dehacked lumps. Transparent sprites are now affected by colormaps.
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Load dehacked file and change table and text from the exe
//
//-----------------------------------------------------------------------------


#include "doomincl.h"

#include "command.h"
#include "console.h"

#include "g_game.h"
#include "p_local.h"
  // monster_infight

#include "sounds.h"
#include "info.h"
#include "action.h"

#include "m_cheat.h"
#include "d_think.h"
#include "dstrings.h"
#include "m_argv.h"
#include "p_inter.h"

//SoM: 4/4/2000: DEHACKED LUMPS
#include "z_zone.h"
#include "w_wad.h"

#include "p_fab.h"
  // translucent change

boolean deh_loaded = false;
byte  flags_valid_deh = false;  // flags altered flags (from DEH), boolean
byte  pars_valid_bex = false;  // have valid PAR values (from BEX), boolean

uint16_t helper_MT = 0xFFFF;  // Substitute helper thing (like DOG).

static boolean  bex_include_notext = 0;  // bex include with skip deh text

// Save compare values, to handle multiple DEH files and lumps
static actionf_t  deh_actions[NUMSTATES];
static char       *deh_sprnames[NUMSPRITES];
static char       *deh_sfxnames[NUMSFX];
static char       *deh_musicname[NUMMUSIC];
static char       *deh_text[NUMTEXT];



#define MAXLINELEN  200

// The code was first written for a file
// and converted to use memory with these functions.
typedef struct {
    char *data;
    char *curpos;
    int size;
} myfile_t;

#define myfeof( a )  (a->data+a->size<=a->curpos)

// Get string upto \n, or eof.
char* myfgets(char *buf, int bufsize, myfile_t *f)
{
    int i=0;

    if( myfeof(f) )
        return NULL;

    bufsize--;  // we need a extra byte for null terminated string
    while(i<bufsize && !myfeof(f) )
    {
        char c = *f->curpos++;
        if( c!='\r' )
            buf[i++]=c;
        if( c=='\n' )
            break;
    }
    buf[i] = '\0';
#ifdef DEBUG_DEH
    memset( &buf[i], 0, bufsize-i-1 );
#endif
    //debug_Printf( "fgets [0]=%d [1]=%d '%s'\n",buf[0],buf[1],buf);

    if( (devparm && verbose) || (verbose>1) )
    {
        // List DEH lines, but not blank lines.
        if( i > 1 )
          GenPrintf(EMSG_errlog, "DEH: %s", buf);  // buf has \n
    }
    return buf;
}

// Get line, skipping comments.
char* myfgets_nocom(char *buf, int bufsize, myfile_t *f)
{
    char* ret;

    do {
        ret = myfgets( buf, bufsize, f );
    } while( ret && ret[0] == '#' );   // skip comment
    return ret;
}


// Read multiple lines into buf
// Only used for text.
size_t  myfread( char *buf, size_t reqsize, myfile_t *f )
{
    size_t byteread = f->size - (f->curpos-f->data);  // bytes left
    if( reqsize < byteread )
        byteread = reqsize;
#ifdef DEBUG_DEH
    memset( &buf[0], 0, reqsize-1 );
#endif
    if( byteread>0 )
    {
        uint32_t i;
        // Read lines except for any '\r'.
        // But should only be taking the '\r' off end of line (/cr/lf)
        for(i=0; i<byteread; )
        {
            char c=*f->curpos++;
            if( c!='\r' )
                buf[i++]=c;
        }
    }
    buf[byteread] = '\0';
    return byteread;
}

static int deh_num_error = 0;

static void deh_error(const char * fmt, ...)
{
    va_list   ap;

    // Show the DEH errors for devparm, devgame, or verbose switches.   
    if (devparm || verbose)
    {
       va_start(ap, fmt);
       GenPrintf_va( EMSG_error, fmt, ap );
       va_end(ap);
    }

    deh_num_error++;
}


// Reject src string if greater than maxlen, or has non-alphanumeric
// For filename protection must reject
//  ( . .. ../ / \  & * [] {} space leading-dash  <32  >127 133 254 255 )
boolean  filename_reject( char * src, int maxlen )
{
     int j;
     for( j=0; ; j++ )
     {
         if( j >= maxlen ) goto reject;

         register char ch = src[j];
         // legal values, all else is illegal
         if(! (( ch >= 'A' && ch <= 'Z' )
            || ( ch >= 'a' && ch <= 'z' )
            || ( ch >= '0' && ch <= '9' )
            || ( ch == '_' ) ))
            goto reject;
     }
     return false; // no reject

  reject:
     return true; // rejected
}


// [WDJ] Do not write into const strings, segfaults will occur on Linux.
// Without fixing all sources of strings to be consistent, the best that
// can be done is to abandon the original string (may be some lost memory)
// Dehacked is only run once at startup and the lost memory is not enough
// to dedicate code to recover.

typedef enum { DRS_nocheck, DRS_name, DRS_string, DRS_format } DRS_type_e;

typedef struct {
    uint16_t   text_id;
    byte  num_s;
} PACKED_ATTR  format_ref_t;

format_ref_t  format_ref_table[] =
{
   {QSPROMPT_NUM, 1},
   {QLPROMPT_NUM, 1},
   {DOSY_NUM, 1},
   {DEATHMSG_SUICIDE, 1},
   {DEATHMSG_TELEFRAG, 2},
   {DEATHMSG_FIST, 2},
   {DEATHMSG_GUN, 2},
   {DEATHMSG_SHOTGUN, 2},
   {DEATHMSG_MACHGUN, 2},
   {DEATHMSG_ROCKET, 2},
   {DEATHMSG_GIBROCKET, 2},
   {DEATHMSG_PLASMA, 2},
   {DEATHMSG_BFGBALL, 2},
   {DEATHMSG_CHAINSAW, 2},
   {DEATHMSG_SUPSHOTGUN, 2},
   {DEATHMSG_PLAYUNKNOW, 2},
   {DEATHMSG_HELLSLIME, 1},
   {DEATHMSG_NUKE, 1},
   {DEATHMSG_SUPHELLSLIME, 1},
   {DEATHMSG_SPECUNKNOW, 1},
   {DEATHMSG_BARRELFRAG, 2},
   {DEATHMSG_BARREL, 1},
   {DEATHMSG_POSSESSED, 1},
   {DEATHMSG_SHOTGUY, 1},
   {DEATHMSG_VILE, 1},
   {DEATHMSG_FATSO, 1},
   {DEATHMSG_CHAINGUY, 1},
   {DEATHMSG_TROOP, 1},
   {DEATHMSG_SERGEANT, 1},
   {DEATHMSG_SHADOWS, 1},
   {DEATHMSG_HEAD, 1},
   {DEATHMSG_BRUISER, 1},
   {DEATHMSG_UNDEAD, 1},
   {DEATHMSG_KNIGHT, 1},
   {DEATHMSG_SKULL, 1},
   {DEATHMSG_SPIDER, 1},
   {DEATHMSG_BABY, 1},
   {DEATHMSG_CYBORG, 1},
   {DEATHMSG_PAIN, 1},
   {DEATHMSG_WOLFSS, 1},
   {DEATHMSG_DEAD, 1},
   {0xFFFF, 0}
};

// [WDJ] 8/26/2011  DEH/BEX replace string
// newstring is a temp buffer ptr, it gets modified for backslash literals
// oldstring is ptr to text ptr  ( &text[i] )
void deh_replace_string( char ** oldstring, char * newstring, DRS_type_e drstype )
{
#ifdef DEH_RECOVER_STRINGS
    // Record bounds for replacement strings.
    // If an oldstring is found within these bounds, it can be free().
    // This is depends on const and malloc heap being two separate areas.
    static char * deh_string_ptr_min = NULL;
    static char * deh_string_ptr_max = NULL;
#endif
    // Most text strings are format strings, and % are significant.
    // New string must not have have any %s %d etc. not present in old string.
    // New freedoom.bex has "1%", that is not present in original string
    // Strings have %s, %s %s (old strings also had %c and %d).
    // Music and sound strings may have '-' and '\0'.
    // [WDJ] Newer compiler could not tolerate these as unsigned char.
    char * newp = &newstring[0];
    if( drstype == DRS_string )
    {
        // Test new string against table
        // Fixes when Chex replacement strings have fewer %s in them,
        // so Chex newmaps.wad can replace that string again.
        int text_id = oldstring - (&text[0]);
        byte num_s = 0;
        int i;
        // look up in table
        for(i=0; ; i++)
        {
            if( format_ref_table[i].text_id > NUMTEXT )  break;
            if( format_ref_table[i].text_id == text_id )
            {
                num_s = format_ref_table[i].num_s;
                drstype = DRS_format;
                break;
            }
        }

        for(i=0; ;)
        {
            // new string must have same or fewer %
            newp = strchr( newp, '%' );
            if( newp == NULL ) break;
            if( drstype == DRS_format )
            {
                // must block %n, write to memory
                // Only %s are left in the text strings
                switch( newp[1] )
                {
                 case '%': // literal %
                   break;
                 case 's':
                   i++;
                   if( i > num_s )   goto bad_replacement;
                   break;
                 default:
                   goto bad_replacement;
                }
            }
            else
            {
                // only  %% allowed
                if( newp[1] != '%' )
                    newp[0] = ' '; // rubout the %
            }
            newp +=2;
        }
    }

    // rewrite backslash literals into newstring, because it only gets shorter
    char * chp = &newstring[0];
    for( newp = &newstring[0]; *newp ; newp++ )
    {
        // Backslash in DEH and BEX strings are not interpreted by printf
        // Must convert \n to LF.
        register char ch = *newp;
        if( ch == 0x5C ) // backslash
        {
            char * endvp = NULL;
            unsigned long v;
            ch = *(++newp);
            switch( ch )
            {
             case 'N': // some file are all caps
             case 'n': // newline
               ch = '\n';  // LF char
               goto incl_char;
             case '\0': // safety
               goto term_string;
             case '0': // NUL, should be unnecessary
               goto term_string;
             case 'x':  // hex
               // These do not get interpreted unless we do it here.
               // Need this for foreign language ??
               v = strtoul( &newp[1], &endvp, 16);  // get hex
               goto check_backslash_value;
             default:
               if( ch >= '1' && ch <= '9' )  // octal
               {
                   // These do not get interpreted unless we do it here.
                   // Need this for foreign language ??
                   v = strtoul( newp, &endvp, 8);  // get octal
                   goto check_backslash_value;
               }
            }
            continue; // ignore unrecognized backslash

         check_backslash_value:
            if( v > 255 ) goto bad_char;  // long check
            ch = v;
            newp = endvp - 1; // continue after number
            // check value against tests
        }
        // reject special character attacks
#if defined( FRENCH_INLINE ) || defined( BEX_LANGUAGE )
        // place checks for allowed foreign lang chars here
        // reported dangerous escape chars
        if( (unsigned char)ch == 133 )  goto bad_char;
        if( (unsigned char)ch >= 254 )  goto bad_char;
//          if( ch == 27 ) continue;  // ESCAPE
#else
        if( (unsigned char)ch > 127 )  goto bad_char;
#endif       
        if( ch < 32 )
        {
            if( ch == '\t' )  ch = ' ';  // change to space
            if( ch == '\r' )  continue;  // remove
            if( ch == '\n' )  goto incl_char;
            if( ch == '\0' )  goto term_string;   // end of string
            goto bad_char;
        }
     incl_char:
        // After a backslash, chp < newp
        *chp++ = ch; // rewrite
    }
 term_string:
    *chp = '\0'; // term rewrite

    if( drstype == DRS_name )
    {
        if( strlen(newstring) > 10 )  goto bad_replacement;
    }

    char * nb = strdup( newstring );  // by malloc
    if( nb == NULL )
        I_Error( "Dehacked/BEX string memory allocate failure" );
#ifdef DEH_RECOVER_STRINGS
    // check if was in replacement string bounds
    if( *oldstring && deh_string_ptr_min
        && *oldstring >= deh_string_ptr_min
        && *oldstring <= deh_string_ptr_max )
        free( *oldstring );
    // track replacement string bounds
    if( deh_string_ptr_min == NULL || nb < deh_string_ptr_min )
        deh_string_ptr_min = nb;
    if( nb > deh_string_ptr_max )
        deh_string_ptr_max = nb;
#else
    // Abandon old strings, might be const
    // Linux GCC programs will segfault if try to free a const string (correct behavior).
    // The lost memory is small and this occurs only once in the program.
#endif
    *oldstring = nb;  // replace the string in the tables
    return;

  bad_char:
    if( chp )
        *chp = '\0'; // hide the bad character
  bad_replacement:
    CONS_Printf( "Replacement string illegal : %s\n", newstring );
    return;
}


/* ======================================================================== */
// Load a dehacked file format 6 I (BP) don't know other format
/* ======================================================================== */
/* a sample to see
                   Thing 1 (Player)       {           // MT_PLAYER
int doomednum;     ID # = 3232              -1,             // doomednum
int spawnstate;    Initial frame = 32       S_PLAY,         // spawnstate
int spawnhealth;   Hit points = 3232        100,            // spawnhealth
int seestate;      First moving frame = 32  S_PLAY_RUN1,    // seestate
int seesound;      Alert sound = 32         sfx_None,       // seesound
int reactiontime;  Reaction time = 3232     0,              // reactiontime
int attacksound;   Attack sound = 32        sfx_None,       // attacksound
int painstate;     Injury frame = 32        S_PLAY_PAIN,    // painstate
int painchance;    Pain chance = 3232       255,            // painchance
int painsound;     Pain sound = 32          sfx_plpain,     // painsound
int meleestate;    Close attack frame = 32  S_NULL,         // meleestate
int missilestate;  Far attack frame = 32    S_PLAY_ATK1,    // missilestate
int deathstate;    Death frame = 32         S_PLAY_DIE1,    // deathstate
int xdeathstate;   Exploding frame = 32     S_PLAY_XDIE1,   // xdeathstate
int deathsound;    Death sound = 32         sfx_pldeth,     // deathsound
int speed;         Speed = 3232             0,              // speed
int radius;        Width = 211812352        16*FRACUNIT,    // radius
int height;        Height = 211812352       56*FRACUNIT,    // height
int mass;          Mass = 3232              100,            // mass
int damage;        Missile damage = 3232    0,              // damage
int activesound;   Action sound = 32        sfx_None,       // activesound
int flags;         Bits = 3232              MF_SOLID|MF_SHOOTABLE|MF_DROPOFF|MF_PICKUP|MF_NOTDMATCH,
int raisestate;    Respawn frame = 32       S_NULL          // raisestate
                                         }, */
// [WDJ] BEX flags 9/10/2011
typedef enum { BFexit, BF1, BF2, BF2x, BFT } bex_flags_ctrl_e;

typedef struct {
    char *    name;
    bex_flags_ctrl_e  ctrl;
    uint32_t  flagval;
} flag_name_t;

// [WDJ] From boomdeh.txt, and DoomLegacy2.0
flag_name_t  BEX_flag_name_table[] = 
{
  {"SPECIAL",    BF1, MF_SPECIAL }, // Call TouchSpecialThing when touched.
  {"SOLID",      BF1, MF_SOLID }, // Blocks
  {"SHOOTABLE",  BF1, MF_SHOOTABLE }, // Can be hit
  {"NOSECTOR",   BF1, MF_NOSECTOR },  // Don't link to sector (invisible but touchable)
  {"NOBLOCKMAP", BF1, MF_NOBLOCKMAP }, // Don't link to blockmap (inert but visible)
  {"AMBUSH",     BF1, MF_AMBUSH }, // Not to be activated by sound, deaf monster.
  {"JUSTHIT",    BF1, MF_JUSTHIT }, // Will try to attack right back.
  {"JUSTATTACKED", BF1, MF_JUSTATTACKED }, // Will take at least one step before attacking.
  {"SPAWNCEILING", BF1, MF_SPAWNCEILING }, // Spawned hanging from the ceiling
  {"NOGRAVITY",  BF1, MF_NOGRAVITY }, // Does not feel gravity
  {"DROPOFF",    BF1, MF_DROPOFF }, // Can jump/drop from high places
  {"PICKUP",     BF1, MF_PICKUP }, // Can/will pick up items. (players)
  // two clip bits, set them both
  {"NOCLIP",     BF1, MF_NOCLIP }, // Does not clip against lines.
  {"NOCLIP",     BF2, MF2_NOCLIPTHING }, // Does not clip against Actors.
  // two slide bits, set them both
  {"SLIDE",      BF1, MF_SLIDE }, // Player: keep info about sliding along walls.
  {"SLIDE",      BF2, MF2_SLIDE }, // Slides against walls

  {"FLOAT",      BF1, MF_FLOAT }, // Active floater, can move freely in air (cacodemons etc.)
  {"TELEPORT",   BF1, MF_TELEPORT }, // Don't cross lines or look at heights on teleport.
  {"MISSILE",    BF1, MF_MISSILE }, // Missile. Don't hit same species, explode on block.
  {"DROPPED",    BF1, MF_DROPPED }, // Dropped by a monster
  {"SHADOW",     BF1, MF_SHADOW }, // Partial invisibility (spectre). Makes targeting harder.
  {"NOBLOOD",    BF1, MF_NOBLOOD }, // Does not bleed when shot (furniture)
  {"CORPSE",     BF1, MF_CORPSE }, // Acts like a corpse, falls down stairs etc.
  {"INFLOAT",    BF1, MF_INFLOAT }, // Don't auto float to target's height.
  {"COUNTKILL",  BF1, MF_COUNTKILL }, // On kill, count towards intermission kill total.
  {"COUNTITEM",  BF1, MF_COUNTITEM }, // On pickup, count towards intermission item total.
  {"SKULLFLY",   BF1, MF_SKULLFLY }, // Flying skulls, neither a cacodemon nor a missile.
  {"NOTDMATCH",  BF1, MF_NOTDMATCH }, // Not spawned in DM (keycards etc.)
  // 4 bits of player color translation (gray/red/brown)
  // PrBoom, MBF, EternityEngine have only 2 bits of color translation.
  {"TRANSLATION1", BFT, (1<<MFT_TRANSSHIFT) },  // Boom
  {"TRANSLATION2", BFT, (2<<MFT_TRANSSHIFT) },  // Boom
  {"TRANSLATION3", BFT, (4<<MFT_TRANSSHIFT) },
  {"TRANSLATION4", BFT, (8<<MFT_TRANSSHIFT) },
  {"TRANSLATION",  BFT, (1<<MFT_TRANSSHIFT) },  // Boom/prboom compatibility
  {"UNUSED1     ", BFT, (2<<MFT_TRANSSHIFT) },  // Boom/prboom compatibility
  // Boom/BEX
  {"TRANSLUCENT", BF1, MF_TRANSLUCENT },  // Boom translucent
  // MBF/Prboom Extensions
  {"TOUCHY",  BF1, MF_TOUCHY }, // (MBF) Reacts upon contact
  {"BOUNCES", BF1, MF_BOUNCES },  // (MBF) Bounces off walls, etc.
  {"FRIEND",  BF1, MF_FRIEND }, // (MBF) Friend to player (dog, etc.)

  {"MF2CLEAR",       BF2x, 0 }, // clear MF2 bits, no bits set
  // DoomLegacy 1.4x Extensions
  {"FLOORHUGGER",    BF2x, MF2_FLOORHUGGER }, // [WDJ] moved to MF2
  // Heretic
  {"LOWGRAVITY",     BF2x, MF2_LOGRAV }, // Experiences only 1/8 gravity
  {"WINDTHRUST",     BF2x, MF2_WINDTHRUST }, // Is affected by wind
//  {"FLOORBOUNCE",    BF2, MF2_FLOORBOUNCE }, // Bounces off the floor
      // see MBF/Prboom "BOUNCES"
  {"HERETICBOUNCE",  BF2x, MF2_FLOORBOUNCE }, // Bounces off the floor
  {"THRUGHOST",      BF2x, MF2_THRUGHOST }, // Will pass through ghosts (missile)
  {"FLOORCLIP",      BF2x, MF2_FOOTCLIP }, // Feet may be be clipped
  {"SPAWNFLOAT",     BF2x, MF2_SPAWNFLOAT }, // Spawned at random height
  {"NOTELEPORT",     BF2x, MF2_NOTELEPORT }, // Does not teleport
  {"RIPPER",         BF2x, MF2_RIP }, // Rips through solid targets (missile)
  {"PUSHABLE",       BF2x, MF2_PUSHABLE }, // Can be pushed by other moving actors
//  {"SLIDE",          BF2x, MF2_SLIDE }, // Slides against walls
     // see other "SLIDE", and MF_SLIDE
  {"PASSMOBJ",       BF2x, MF2_PASSMOBJ }, // Enable z block checking.
      // If on, this flag will allow the mobj to pass over/under other mobjs.
  {"CANNOTPUSH",     BF2x, MF2_CANNOTPUSH }, // Cannot push other pushable actors
  {"BOSS",           BF2x, MF2_BOSS }, // Is a major boss, not as easy to kill
  {"FIREDAMAGE",     BF2x, MF2_FIREDAMAGE }, // does fire damage
  {"NODAMAGETHRUST", BF2x, MF2_NODMGTHRUST }, // Does not thrust target when damaging
  {"TELESTOMP",      BF2x, MF2_TELESTOMP }, // Can telefrag another Actor
  {"FLOATBOB",       BF2x, MF2_FLOATBOB }, // use float bobbing z movement
  {"DONTDRAW",       BF2x, MF2_DONTDRAW }, // Invisible (does not generate a vissprite)
  // DoomLegacy 1.4x Internal flags, non-standard
  // Exist but have little use being set by a WAD
  {"ONMOBJ",         BF2x, MF2_ONMOBJ }, // mobj is resting on top of another
//  {"FEETARECLIPPED", BF2x, MF2_FEETARECLIPPED }, // a mobj's feet are now being cut
//  {"FLY",            BF2x, MF2_FLY }, // Fly mode

  // DoomLegacy 2.0 Extensions
  // Heretic/Hexen/ZDoom additions
//  {"HEXENBOUNCE",    BF2x, MF2_FULLBOUNCE }, // Bounces off walls and floor
//  {"SLIDESONWALLS",  BF2x, MF2_SLIDE }, // Slides against walls
//  {"FLOORHUGGER",    BF2x, MF2_FLOORHUGGER },
//  {"CEILINGHUGGER",  BF2x, MF2_CEILINGHUGGER },
//  {"DONTBLAST",      BF2x, MF2_NONBLASTABLE },
//  {"QUICKTORETALIATE", BF2x, MF2_QUICKTORETALIATE },
//  {"NOTARGET",       BF2x, MF2_NOTARGET }, // Will not be targeted by other monsters of same team (like Arch-Vile)
//  {"FLOATBOB",       BF2x, MF2_FLOATBOB }, // Bobs up and down in the air (item)
//  {"CANPASS",        BF2x, 0, }, // TODO inverted!  Can move over/under other Actors
//  {"NONSHOOTABLE",   BF2x, MF2_NONSHOOTABLE }, // Transparent to MF_MISSILEs
//  {"INVULNERABLE",   BF2x, MF2_INVULNERABLE }, // Does not take damage
//  {"DORMANT",        BF2x, MF2_DORMANT }, // Cannot be damaged, is not noticed by seekers
//  {"CANTLEAVEFLOORPIC", BF2x, MF2_CANTLEAVEFLOORPIC }, // Stays within a certain floor texture
//  {"SEEKERMISSILE",  BF2x, MF2_SEEKERMISSILE }, // Is a seeker (for reflection)
//  {"REFLECTIVE",     BF2x, MF2_REFLECTIVE }, // Reflects missiles
//  {"ACTIVATEIMPACT", BF2x, MF2_IMPACT }, // Can activate SPAC_IMPACT
//  {"CANPUSHWALLS",   BF2x, MF2_PUSHWALL }, // Can activate SPAC_PUSH
//  {"DONTSPLASH", BFC_x, MF_NOSPLASH }, // Does not cause splashes in liquid.
//  {"ISMONSTER", BFC_x, MF_MONSTER },
//  {"ACTIVATEMCROSS", BFC_x, MF2_MCROSS }, // Can activate SPAC_MCROSS
//  {"ACTIVATEPCROSS", BFC_x, MF2_PCROSS }, // Can activate SPAC_PCROSS
  {NULL, BFexit, 0} // terminator
};

//#define CHECK_FLAGS2_DEFAULT
#ifdef CHECK_FLAGS2_DEFAULT
// Old PWAD do not know of MF2 bits.
// Default for PWAD that do not set MF2 flags
const uint32_t flags2_default_value = 0; // other ports like prboom
  // | MF2_PASSMOBJ // heretic monsters
  // | MF2_FOOTCLIP // heretic only
  // | MF2_WINDTHRUST; // requires heretic wind sectors
#endif



// Standardized state ranges
enum {
// Doom
  STS_TECH2LAMP4 = 966,
// TNT
  STS_TNT1 = 967,
// MBF
  STS_GRENADE = 968,
  STS_DOGS_PR_STND = 972,
  STS_DOGS_RAISE6 = 998,
  STS_MUSHROOM = 1075,
// Doom Beta
  STS_OLDBFG1 = 999,
  STS_BSKUL_DIE8 = 1074,
// HERETIC
  STH_FREETARGMOBJ = 1,
  STH_PODGENERATOR = 114,
  STH_SPLASH1 = 115,  // is S_HSPLASH1
  STH_TFOG1 = 223,  // is S_HTFOG1
  STH_TFOG13 = 235,
  STH_LIGHTDONE = 236,  // is S_HLIGHTDONE, but unused
  STH_STAFFREADY = 237,
  STH_CRBOWFX4_2 = 562,
  STH_BLOOD1 = 563,  // mapped to S_BLOOD1 (S_HBLOOD dis)
  STH_BLOOD3 = 565,
  STH_BLOODSPLATTER1 = 566,
  STH_BLOODSPLATTERX = 569,
  STH_PLAY = 570, // mapped to S_PLAY (Heretic S_PLAY dis)
  STH_PLAY_XDIE9 = 596,
  STH_PLAY_FDTH1 = 597,  // unmapped, TODO
  STH_PLAY_FDTH20 = 616,
  STH_BLOODYSKULL1 = 617,
  STH_REDAXE3 = 986,
  STH_SND_WATERFALL = 1204,
};


// Translate deh frame number to internal state index.
// return S_NULL when invalid state (deh frame)
statenum_t  deh_frame_to_state( int deh_frame )
{
  // Some old wads had negative frame numbers, which should be S_NULL.
  if( deh_frame <= 0 )  goto null_frame;
 
  // remapping
  if( EN_heretic )
  {
     if( deh_frame <= STH_CRBOWFX4_2 )
       return deh_frame + (S_FREETARGMOBJ - STH_FREETARGMOBJ);
     // STH_BLOOD Mapped to the doom blood
     if( deh_frame >= STH_BLOOD1
	 && deh_frame <= STH_BLOOD3 )
       return deh_frame + (S_BLOOD1 - STH_BLOOD1);
     if( deh_frame >= STH_BLOODSPLATTER1
	 && deh_frame <= STH_BLOODSPLATTERX )
       return deh_frame + (S_BLOODSPLATTER1 - STH_BLOODSPLATTER1);
     // STH_PLAY Mapped to the doom player, except for STH_PLAY_FDTH.
     if( deh_frame >= STH_PLAY
	 && deh_frame <= STH_PLAY_XDIE9 )
       return deh_frame + (S_PLAY - STH_PLAY);
#if 0
     // TODO
     if( deh_frame >= STH_PLAY_FDTH1
	 && deh_frame <= STH_PLAY_FDTH20 )
       return deh_frame + (S_PLAY_FDTH1 - STH_PLAY_FDTH1);
#endif
     if( deh_frame >= STH_BLOODYSKULL1
	 && deh_frame <= STH_SND_WATERFALL )
       return deh_frame + (S_BLOODYSKULL1 - STH_BLOODYSKULL1);
  }
  else
  {
     if( deh_frame <= STS_TECH2LAMP4 )
       return deh_frame;
     if( deh_frame == STS_TNT1 )
       return S_TNT1;
     if( deh_frame >= STS_GRENADE
	 && deh_frame <= STS_DOGS_RAISE6 )
       return deh_frame + (S_GRENADE - STS_GRENADE);
     if( deh_frame == STS_MUSHROOM )
       return S_MUSHROOM;
  }

null_frame:
  return S_NULL;
}


// Implement assigns of a state num, from deh.
static
void  set_state( statenum_t * snp, int deh_frame_id )
{
  statenum_t si = deh_frame_to_state(deh_frame_id);
  *snp = si;  // valid or S_NULL
  if( deh_frame_id > 0 && si == S_NULL )
    GenPrintf(EMSG_errlog, "DEH/BEX set state has bad frame id: %i\n", deh_frame_id );
}


static int searchvalue(char *s)
{
  while(s[0]!='=' && s[0]!='\0') s++;
  if (s[0]=='=')
    return atoi(&s[1]);
  else
  {
    deh_error("No value found\n");
    return 0;
  }
}

// Have read  "Thing <deh_thing_id>"
static void readthing(myfile_t *f, int deh_thing_id )
{
  // DEH thing 1.. , but mobjinfo array is 0..
  mobjinfo_t *  mip = & mobjinfo[ deh_thing_id - 1 ];
  char s[MAXLINELEN];
  char *word;
  int value;
  uint32_t flags1, flags2, tflags;

  do{
    if(myfgets(s,sizeof(s),f)!=NULL)  // get line
    {
      if(s[0]=='\n') break;
      value=searchvalue(s);
      word=strtok(s," ");

      // Wads that use Bits: phobiata, hth2, DRCRYPT
      if(!strcasecmp(word,"Bits"))
      {
          boolean flags2x_hit = 0; // doomlegacy extensions hit
          flag_name_t * fnp; // BEX flag names ptr

          flags1 = flags2 = tflags = 0;
          for(;;)
          {
              word = strtok(NULL, " +|\t=\n");
              if( word == NULL )  goto set_flags;
              if( word[0] == '\n' )   goto set_flags;
              // detect bits by integer value
              // MBF DRCRYPT.WAD sets color and MF_FRIEND using Bits, with neg value.
              if( isdigit( word[0] ) || word[0] == '-' )
              {
                  // old style integer value, flags only (not flags2)
                  flags1 = atoi(word);  // numeric entry
                  if( flags1 & MF_TRANSLUCENT )
                  {
                      // Boom bit defined in boomdeh.txt
                      // Was MF_FLOORHUGGER bit, and now need to determine which the PWAD means.
                      // MBF DRCRYPT.WAD has object with bits =
                      // MF_TRANSLUCENT, MF_COUNTKILL, MF_SHADOW, MF_SOLID, MF_SHOOTABLE.
                      GenPrintf(EMSG_errlog, "Sets flag MF_FLOORHUGGER or MF_TRANSLUCENT by numeric, guessing ");
                      if( flags1 & (MF_NOBLOCKMAP|MF_MISSILE|MF_NOGRAVITY|MF_COUNTITEM|MF_SHADOW))
                      {
                          // assume TRANSLUCENT, check for known exceptions
                          GenPrintf(EMSG_errlog, "MF_TRANSLUCENT\n");
                      }
                      else
                      {
                          // Don't know of any wads setting FLOORHUGGER using
                          // bits.
                          // assume FLOORHUGGER, check for known exceptions
                          flags1 &= ~MF_TRANSLUCENT;
                          flags2 |= MF2_FLOORHUGGER;
                          GenPrintf(EMSG_errlog, "MF_FLOORHUGGER\n");
                      }
                  }
                  if( flags1 & MFO_TRANSLATION4 )
                  {
                      // Color translate, moved to tflags.
                      GenPrintf(EMSG_errlog, "Sets color flag MF_TRANSLATE using Bits\n" );
                      tflags |= (flags1 & MFO_TRANSLATION4) >> (MFO_TRANSSHIFT - MFT_TRANSSHIFT);
                      flags1 &= ~MFO_TRANSLATION4;
                  }
                  // we are still using same flags bit order
                  goto next_line;
              }
              // handle BEX flag names
              for( fnp = &BEX_flag_name_table[0]; ; fnp++ )
              {
                  if(fnp->name == NULL)  goto name_unknown;
                  if(!strcasecmp( word, fnp->name ))  // find name
                  {
                      switch( fnp->ctrl )
                      {
                       case BF1:
                         flags1 |= fnp->flagval;
                         break;
                       case BF2x: // DoomLegacy extension BEX name
                         flags2x_hit = 1;
                       case BF2: // standard name that happens to be MF2
                         flags2 |= fnp->flagval;
                         break;
                       case BFT: // standard name that happens to be MFT
                         tflags |= fnp->flagval;
                         break;
                       default:
                         goto name_unknown;
                      }
                      // if next entry is same keyword then process it too
                      if( (fnp[1].name != fnp[0].name) )
                         goto next_word; // done with this word
                      // next entry is same word, process it too
                  }
              }
            name_unknown:
              deh_error("Bits name unknown: %s\n", word);
              // continue with next keyword
            next_word:
              continue;
          }

        set_flags:
          // clear std flags in flags2
          mip->flags2 &= ~(MF2_SLIDE|MF2_FLOORBOUNCE);
          if( flags2x_hit )
          {
              // clear extension flags2 only if some extension names appeared
              mip->flags2 = 0;
          }
#ifdef CHECK_FLAGS2_DEFAULT
          else
          {
              // Unless explicitly used BF2x bit, then put in default bits
              // Avoid by using MF2CLEAR
              mip->flags2 = flags2_default_value;
          }
#endif
        next_line:
          mip->flags = flags1;
          mip->flags2 |= flags2;
          mip->tflags = tflags;  // info extension has color now
          flags_valid_deh = true;
          continue; // next line
      }

      // set the value in apropriet field
      else if(!strcasecmp(word,"ID"))           mip->doomednum   =value;
      else if(!strcasecmp(word,"Initial"))      set_state( &mip->spawnstate, value );
      else if(!strcasecmp(word,"Hit"))          mip->spawnhealth =value;
      else if(!strcasecmp(word,"First"))        set_state( &mip->seestate, value );
      else if(!strcasecmp(word,"Alert"))        mip->seesound    =value;
      else if(!strcasecmp(word,"Reaction"))     mip->reactiontime=value;
      else if(!strcasecmp(word,"Attack"))       mip->attacksound =value;
      else if(!strcasecmp(word,"Injury"))       set_state( &mip->painstate, value );
      else if(!strcasecmp(word,"Pain"))
           {
             word=strtok(NULL," ");
             if(!strcasecmp(word,"chance"))     mip->painchance  =value;
             else if(!strcasecmp(word,"sound")) mip->painsound   =value;
           }
      else if(!strcasecmp(word,"Close"))        set_state( &mip->meleestate, value );
      else if(!strcasecmp(word,"Far"))          set_state( &mip->missilestate, value );
      else if(!strcasecmp(word,"Death"))
           {
             word=strtok(NULL," ");
             if(!strcasecmp(word,"frame"))      set_state( &mip->deathstate, value );
             else if(!strcasecmp(word,"sound")) mip->deathsound  =value;
           }
      else if(!strcasecmp(word,"Exploding"))    set_state( &mip->xdeathstate, value );
      else if(!strcasecmp(word,"Speed"))        mip->speed       =value;
      else if(!strcasecmp(word,"Width"))        mip->radius      =value;
      else if(!strcasecmp(word,"Height"))       mip->height      =value;
      else if(!strcasecmp(word,"Mass"))         mip->mass        =value;
      else if(!strcasecmp(word,"Missile"))      mip->damage      =value;
      else if(!strcasecmp(word,"Action"))       mip->activesound =value;
      else if(!strcasecmp(word,"Bits2"))        mip->flags2      =value;
      else if(!strcasecmp(word,"Respawn"))      set_state( &mip->raisestate, value );
      else deh_error("Thing %d : unknown word '%s'\n", deh_thing_id,word);
    }
  } while(s[0]!='\n' && !myfeof(f)); //finish when the line is empty
}

// Have read  "Frame <deh_frame_id>"
/*
Sprite number = 10
Sprite subnumber = 32968
Duration = 200
Next frame = 200
// used as param 1 and param2 by MBF functions
Unknown 1 = 5
Unknown 2 = 17
*/
static void readframe(myfile_t* f, int deh_frame_id)
{
  state_t * fsp;
  char s[MAXLINELEN];
  char *word1,*word2;
  int value;
   
  // Syntax: "Frame <num>"
  int si = deh_frame_to_state(deh_frame_id);
  if( si == S_NULL )
  {
    deh_error("Frame %d don't exist\n", deh_frame_id);
    return;
  }

  fsp = & states[ si ];
   
  do{
    if(myfgets_nocom(s,sizeof(s),f)!=NULL)
    {
      if(s[0]=='\n') break;
      value=searchvalue(s);
      // set the value in appropriate field
      word1=strtok(s," ");
      word2=strtok(NULL," ");

      if(!strcasecmp(word1,"Sprite"))
      {
        // Syntax: Sprite number = <num>
             if(!strcasecmp(word2,"number"))     fsp->sprite   =value;
        // Syntax: Sprite subnumber = <num>
        else if(!strcasecmp(word2,"subnumber"))  fsp->frame    =value;
      }
        // Syntax: Duration = <num>
      else if(!strcasecmp(word1,"Duration"))     fsp->tics     =value;
        // Syntax: Next frame = <num>
      else if(!strcasecmp(word1,"Next"))         set_state( &fsp->nextstate, value );
      else if(!strcasecmp(word1,"Unknown"))
      {
        // Syntax: Unknown 2 = <num>
        // MBF uses these for parameters (parm1, parm2)
        state_ext_t * sep = P_create_state_ext( fsp );
        if( word2[0] == '1' ) sep->parm1 = value;
        else if( word2[0] == '2' ) sep->parm2 = value;
      }
      else deh_error("Frame %d : unknown word '%s'\n", deh_frame_id, word1);
    }
  } while(s[0]!='\n' && !myfeof(f));
}

// Have read  "Pointer <xref>"
// The xref is a dehacked cross ref number.
static
void  readpointer( myfile_t* f, int xref )
{
  char s[MAXLINELEN];
  char *word2;
  int  i;
  statenum_t si, sj;

  // Syntax: "Pointer <xref> (Frame  <deh_frame_id>)"
  word2 = strtok(NULL," "); // get keyword "Frame"
  word2 = strtok(NULL,")");
  if( ! word2 )
  {
    deh_error("Pointer %i (Frame ... ) : missing ')'\n", xref );
    return;
  }
   
  i = atoi(word2);
  si = deh_frame_to_state(i);		  
  if( si == S_NULL )
  {
    deh_error("Pointer %i : Frame %d don't exist\n", xref, i);
    return;
  }

  // [WDJ] For some reason PrBoom and EE do this in a loop,
  // and then print out a BEX equivalent line.
  // Syntax: "Codep Frame <deh_frame_id>"
  if( myfgets(s,sizeof(s),f) != NULL )
  {
    sj = deh_frame_to_state( searchvalue(s) );
    states[si].action = deh_actions[sj];
  }
}


static void readsound(myfile_t* f, int deh_sound_id)
{
  sfxinfo_t *  ssp = & S_sfx[ deh_sound_id ];
  char s[MAXLINELEN];
  char *word;
  int value;

  do{
    if(myfgets_nocom(s,sizeof(s),f)!=NULL)
    {
      if(s[0]=='\n') break;
      value=searchvalue(s);
      word=strtok(s," ");
      if(!strcasecmp(word,"Offset"))
      {
          value-=150360;
          if(value<=64) value/=8;
          else if(value<=260) value=(value+4)/8;
          else value=(value+8)/8;
          if(value>=-1 && value<sfx_freeslot0-1)
              ssp->name=deh_sfxnames[value+1];
          else
              deh_error("Sound %d : offset out of bound\n", deh_sound_id);
      }
      else if(!strcasecmp(word,"Zero/One"))
          ssp->flags = ( value? SFX_single : 0 );
      else if(!strcasecmp(word,"Value"))    ssp->priority   =value;
      else deh_error("Sound %d : unknown word '%s'\n", deh_sound_id,word);
    }
  } while(s[0]!='\n' && !myfeof(f));
}

// [WDJ] Some strings have been altered, preventing match to text in DEH file.
// This is hash of the original Doom strings.
typedef struct {
    uint32_t  hash;      // computed hash
    uint16_t  indirect;  // id of actual text
} PACKED_ATTR  hash_text_t;

// [WDJ] This is constructed from wads where DEH fails.
// To get hash print use -devparm -v.
// > doomlegacy -devparm -v -game doom2 -file xxx.wad 2> xxx.log
static hash_text_t   hash_text_table[] =
{
    {0x26323511, QUITMSG3_NUM},  // dos -> your os
    {0x33033301, QUITMSG4_NUM},  // dos -> your os
    {0x8e3b425e, QUIT2MSG1_NUM}, // dos -> shell
    {0x0042d2cd, DEATHMSG_TELEFRAG}, // telefraged -> telefragged
    {0x106f76c2, DEATHMSG_ROCKET}, // catched -> caught
  // because Chex1PatchEngine changes text before newmaps DEH makes its changes
    {0xea264ed7, E1TEXT_NUM},
    {0xcee03ff5, QUITMSG_NUM},
    {0x55b48886, QUITMSG1_NUM},
    {0xe980b2e0, QUITMSG2_NUM},
    {0x26323511, QUITMSG3_NUM},
    {0x33033301, QUITMSG4_NUM},
    {0x84311a52, QUITMSG5_NUM},
    {0xb6d256a1, QUITMSG6_NUM},
    {0x303ea545, QUITMSG7_NUM},
    {0x8e3b425e, QUIT2MSG1_NUM},
    {0x507e7be5, QUIT2MSG2_NUM},
    {0xc5e635e9, QUIT2MSG3_NUM},
    {0x70136b3e, QUIT2MSG4_NUM},
    {0xe5efb6a8, QUIT2MSG5_NUM},
    {0x04fc8b21, QUIT2MSG6_NUM},
    {0x68d8f2ce, NIGHTMARE_NUM},
    {0x001bcf94, GOTARMOR_NUM},
    {0x01bcfc54, GOTMEGA_NUM},
    {0x01bc5ffd, GOTHTHBONUS_NUM},
    {0x01bc817d, GOTARMBONUS_NUM},
    {0x00010e33, GOTSUPER_NUM},
    {0x01bc64a4, GOTBLUECARD_NUM},
    {0x06f2bfa4, GOTYELWCARD_NUM},
    {0x00de4424, GOTREDCARD_NUM},
    {0x003795f5, GOTSTIM_NUM},
    {0x8e914c80, GOTMEDINEED_NUM},
    {0x001bc72a, GOTMEDIKIT_NUM},
    {0x00000b0b, GOTBERSERK_NUM},
    {0x00e1e245, GOTINVIS_NUM},
    {0x06b5b172, GOTSUIT_NUM},
    {0x0d3bf614, GOTVISOR_NUM},
    {0x000378aa, GOTCLIP_NUM},
    {0x0378e27f, GOTCLIPBOX_NUM},
    {0x000de4c2, GOTROCKET_NUM},
    {0x0378e527, GOTROCKBOX_NUM},
    {0x01bc81b0, GOTCELL_NUM},
    {0x1bc81c85, GOTCELLBOX_NUM},
    {0x06f0ec53, GOTSHELLS_NUM},
    {0xde396c53, GOTSHELLBOX_NUM},
    {0xde1fc095, GOTBACKPACK_NUM},
    {0x048972a1, GOTBFG9000_NUM},
    {0x004899e4, GOTCHAINGUN_NUM},
    {0x0260d3b2, GOTCHAINSAW_NUM},
    {0x1228dc04, GOTLAUNCHER_NUM},
    {0x009143bc, GOTPLASMA_NUM},
    {0x00245214, GOTSHOTGUN_NUM},
    {0x015aa620, STSTR_DQDON_NUM},
    {0x02b54c46, STSTR_DQDOFF_NUM},
    {0x000b945e, STSTR_FAADDED_NUM},
    {0x00824ede, STSTR_KFAADDED_NUM},
    {0x000188ff, STSTR_CHOPPERS_NUM},
    {0x00007ba0, HUSTR_E1M1_NUM},
    {0x001f7c84, HUSTR_E1M2_NUM},
    {0x003fc441, HUSTR_E1M3_NUM},
    {0x007d8282, HUSTR_E1M4_NUM},
    {0x0003f9ac, HUSTR_E1M5_NUM},
//{0x0000199f, ??}, // "GREEN KEY"
    {0x00004375, DEATHMSG_SUICIDE},
    {0x0042d2cd, DEATHMSG_TELEFRAG},
    {0x020d3f9d, DEATHMSG_FIST},
    {0x0004296d, DEATHMSG_GUN},
    {0x0010c1fd, DEATHMSG_SHOTGUN},
    {0x0211416d, DEATHMSG_MACHGUN},
    {0x106f76c2, DEATHMSG_ROCKET},
    {0x020f86c2, DEATHMSG_GIBROCKET},
    {0x00075264, DEATHMSG_PLASMA},
    {0x07904664, DEATHMSG_BFGBALL},
    {0xe3a3462f, DEATHMSG_CHAINSAW},
    {0x000426ad, DEATHMSG_PLAYUNKNOW},
    {0x001cd8c3, DEATHMSG_HELLSLIME},
    {0x01f47e6f, DEATHMSG_NUKE},
    {0x75e39428, DEATHMSG_SUPHELLSLIME},
    {0x01ce59b8, DEATHMSG_SPECUNKNOW},
    {0x020e19cd, DEATHMSG_BARRELFRAG},
    {0x39b61448, DEATHMSG_BARREL},
    {0x021835d2, DEATHMSG_POSSESSED},
    {0x0861051f, DEATHMSG_SHOTGUY},
    {0x0021255e, DEATHMSG_TROOP},
    {0x109d6e18, DEATHMSG_SERGEANT},
    {0x10b701a8, DEATHMSG_BRUISER},
    {0x00000372, DEATHMSG_DEAD},
    {0, 0xFFFF}  // last has invalid indirect
};

static void readtext(myfile_t* f, int len1, int len2 )
{
  char s[2001];
  char * str2;
  int i;

  // it is hard to change all the text in doom
  // here I implement only vital things
  // yes, Text can change some tables like music, sound and sprite name
  
  if(len1+len2 > 2000)
  {
      deh_error("Text too big\n");
      return;
  }

  if( myfread(s, len1+len2, f) )
  {
    if( bex_include_notext )
       return;  // BEX INCLUDE NOTEXT is active, blocks Text replacements

#if 0
    // Debugging trigger
    if( strncmp(s,"GREATER RUNES", 13) == 0 )
    {
        GenPrintf(EMSG_errlog, "Text:%s\n", s);
    }
#endif

    str2 = &s[len1];
    s[len1+len2]='\0';

    if( devparm && verbose )
    {
        // Readable listing of DEH text replacement.
        // Make copy of str1 so can terminate string.
        int len3 = (len1<999)?len1:999;
        char str3[1000];
        strncpy( str3, s, len3);
        str3[len3] = 0;
        GenPrintf(EMSG_errlog, "FROM:%s\nTO:%s\n", str3, str2);
    }

    if((len1 == 4) && (len2 == 4))  // sprite names are always 4 chars
    {
      // sprite table
      for(i=0;i<NUMSPRITES;i++)
      {
        if(!strncmp(deh_sprnames[i],s,len1))
        {
          // May be const string, which will segfault on write
          deh_replace_string( &sprnames[i], str2, DRS_name );
          return;
        }
      }
    }
    if((len1 <= 6) && (len2 <= 6))  // sound effect names limited to 6 chars
    {
      // these names are strings, so compare them correctly
      char str1[8];  // make long enough for 0 term too
      strncpy( str1, s, len1 ); // copy name to proper string
      str1[len1] = '\0';
      // sound table
      for(i=0;i<sfx_freeslot0;i++)
      {
        if(!strcmp(deh_sfxnames[i],str1))
        {
          // sfx name may be Z_Malloc(7) or a const string
          // May be const string, which will segfault on write
          deh_replace_string( &S_sfx[i].name, str2, DRS_name );
          return;
        }
      }
      // music names limited to 6 chars
      // music table
      for(i=1;i<NUMMUSIC;i++)
      {
        if( deh_musicname[i] && (!strcmp(deh_musicname[i], str1)) )
        {
          // May be const string, which will segfault on write
          deh_replace_string( &S_music[i].name, str2, DRS_name );
          return;
        }
      }
    }
    // Limited by buffer size.
    // text table
    for(i=0;i<SPECIALDEHACKED;i++)
    {
      if(!strncmp(deh_text[i],s,len1) && strlen(deh_text[i])==(unsigned)len1)
      {
        // May be const string, which will segfault on write
        deh_replace_string( &text[i], str2, DRS_string );
        return;
      }
    }

    // special text : text changed in Legacy but with dehacked support
    for(i=SPECIALDEHACKED;i<NUMTEXT;i++)
    {
       int ltxt = strlen(deh_text[i]);

       if(len1>ltxt && strstr(s,deh_text[i]))
       {
           // found text to be replaced
           char *t;

           // remove space for center the text
           t=&s[len1+len2-1];
           while(t[0]==' ') { t[0]='\0'; t--; }
           // skip the space
           while(s[len1]==' ') len1++;

           // remove version string identifier
           t=strstr(&(s[len1]),"v%i.%i");
           if(!t) {
              t=strstr(&(s[len1]),"%i.%i");
              if(!t) {
                 t=strstr(&(s[len1]),"%i");
                 if(!t) {
                      t=&s[len1]+strlen(&(s[len1]));
                 }
              }
           }
           t[0]='\0';
           // May be const string, which will segfault on write
           deh_replace_string( &text[i], &(s[len1]), DRS_string );
           return;
       }
    }
    
    // [WDJ] Lookup by hash
    {
       uint32_t hash = 0;
       for(i=0; i<len1; i++)
       {
           if( s[i] >= '0' )
           {
               if( hash&0x80000000 )
                  hash ++;
               hash <<= 1;
               hash += toupper( s[i] ) - '0';
           }
       }
       for(i=0;;i++)
       {
           if( hash_text_table[i].indirect >= NUMTEXT ) break;  // not found
           if( hash_text_table[i].hash == hash )
           {
               deh_replace_string( &text[hash_text_table[i].indirect], &(s[len1]), DRS_string );
               return;
           }
       }
       if( devparm && verbose )
           GenPrintf(EMSG_errlog, "Text hash= 0x%08x :", hash);
    }

    s[len1]='\0';
    deh_error("Text not changed :%s\n",s);
  }
}

// [WDJ] 8/27/2011 BEX text strings
typedef struct {
    char *    kstr;
    uint16_t  text_num;
} bex_text_t;

// must count entries in bex_string_table
uint16_t  bex_string_start_table[3] = { 46+15, 46, 0 };  // start=

// BEX entries from boom202s/boomdeh.txt
bex_text_t  bex_string_table[] =
{
// start=3 language changes
// BEX that language may change, but PWAD should not change
   { "D_DEVSTR", D_DEVSTR_NUM },  // dev mode
   { "D_CDROM", D_CDROM_NUM },  // cdrom version
   { "LOADNET", LOADNET_NUM }, // only server can load netgame
   { "QLOADNET", QLOADNET_NUM }, // cant quickload
   { "QSAVESPOT", QSAVESPOT_NUM },  // no quicksave slot
   { "SAVEDEAD", SAVEDEAD_NUM },  // cannot save when not playing
   { "QSPROMPT", QSPROMPT_NUM }, // quicksave, has %s
   { "QLPROMPT", QLPROMPT_NUM }, // quickload, has %s
   { "NEWGAME", NEWGAME_NUM }, // cant start newgame
   { "SWSTRING", SWSTRING_NUM }, // shareware version
   { "MSGOFF", MSGOFF_NUM },
   { "MSGON", MSGON_NUM },
   { "NETEND", NETEND_NUM }, // cant end netgame
   { "ENDGAME", ENDGAME_NUM }, // want to end game ?
   { "DOSY", DOSY_NUM },  // quit to DOS, has %s
   { "EMPTYSTRING", EMPTYSTRING_NUM }, // savegame empty slot
   { "GGSAVED", GGSAVED_NUM }, // game saved
   { "HUSTR_MSGU", HUSTR_MSGU_NUM }, // message not sent
   { "HUSTR_MESSAGESENT", HUSTR_MESSAGESENT_NUM }, // message sent
   { "AMSTR_FOLLOWON", AMSTR_FOLLOWON_NUM },  // Automap follow
   { "AMSTR_FOLLOWOFF", AMSTR_FOLLOWOFF_NUM },
   { "AMSTR_GRIDON", AMSTR_GRIDON_NUM },  // Automap grid
   { "AMSTR_GRIDOFF", AMSTR_GRIDOFF_NUM },
   { "AMSTR_MARKEDSPOT", AMSTR_MARKEDSPOT_NUM },  // Automap marks
   { "AMSTR_MARKSCLEARED", AMSTR_MARKSCLEARED_NUM },
   { "STSTR_MUS", STSTR_MUS_NUM },  // Music
   { "STSTR_NOMUS", STSTR_NOMUS_NUM },
   { "STSTR_NCON", STSTR_NCON_NUM },  // No Clip
   { "STSTR_NCOFF", STSTR_NCOFF_NUM },
   { "STSTR_CLEV", STSTR_CLEV_NUM },  // change level
// BEX not used in DoomLegacy, but have strings
   { "DETAILHI", DETAILHI_NUM },
   { "DETAILLO", DETAILLO_NUM },
   { "GAMMALVL0", GAMMALVL0_NUM },
   { "GAMMALVL1", GAMMALVL1_NUM },
   { "GAMMALVL2", GAMMALVL2_NUM },
   { "GAMMALVL3", GAMMALVL3_NUM },
   { "GAMMALVL4", GAMMALVL4_NUM },
// BEX not used in DoomLegacy, but have strings, was only used in define of other strings
   { "PRESSKEY", PRESSKEY_NUM },
   { "PRESSYN", PRESSYN_NUM },
// BEX not present in DoomLegacy
   { "RESTARTLEVEL", 9999 },
   { "HUSTR_PLRGREEN", 9999 },
   { "HUSTR_PLRINDIGO", 9999 },
   { "HUSTR_PLRBROWN", 9999 },
   { "HUSTR_PLRRED", 9999 },
   { "STSTR_COMPON", 9999 }, // Doom compatibility mode
   { "STSTR_COMPOFF",9999 },
   
// start=2 personal changes
   { "HUSTR_CHATMACRO0", HUSTR_CHATMACRO0_NUM },
   { "HUSTR_CHATMACRO1", HUSTR_CHATMACRO1_NUM },
   { "HUSTR_CHATMACRO2", HUSTR_CHATMACRO2_NUM },
   { "HUSTR_CHATMACRO3", HUSTR_CHATMACRO3_NUM },
   { "HUSTR_CHATMACRO4", HUSTR_CHATMACRO4_NUM },
   { "HUSTR_CHATMACRO5", HUSTR_CHATMACRO5_NUM },
   { "HUSTR_CHATMACRO6", HUSTR_CHATMACRO6_NUM },
   { "HUSTR_CHATMACRO7", HUSTR_CHATMACRO7_NUM },
   { "HUSTR_CHATMACRO8", HUSTR_CHATMACRO8_NUM },
   { "HUSTR_CHATMACRO9", HUSTR_CHATMACRO9_NUM },
   { "HUSTR_TALKTOSELF1", HUSTR_TALKTOSELF1_NUM },
   { "HUSTR_TALKTOSELF2", HUSTR_TALKTOSELF2_NUM },
   { "HUSTR_TALKTOSELF3", HUSTR_TALKTOSELF3_NUM },
   { "HUSTR_TALKTOSELF4", HUSTR_TALKTOSELF4_NUM },
   { "HUSTR_TALKTOSELF5", HUSTR_TALKTOSELF5_NUM },

// start=0 normal game changes
   { "QUITMSG",  QUITMSG_NUM },
   { "NIGHTMARE",  NIGHTMARE_NUM },
   { "GOTARMOR",  GOTARMOR_NUM },
   { "GOTMEGA",  GOTMEGA_NUM },
   { "GOTHTHBONUS",  GOTHTHBONUS_NUM },
   { "GOTARMBONUS",  GOTARMBONUS_NUM },
   { "GOTSTIM", GOTSTIM_NUM },
   { "GOTMEDINEED", GOTMEDINEED_NUM },
   { "GOTMEDIKIT", GOTMEDIKIT_NUM },
   { "GOTSUPER", GOTSUPER_NUM },
   { "GOTBLUECARD", GOTBLUECARD_NUM },
   { "GOTYELWCARD", GOTYELWCARD_NUM },
   { "GOTREDCARD", GOTREDCARD_NUM },
   { "GOTBLUESKUL", GOTBLUESKUL_NUM },
   { "GOTYELWSKUL", GOTYELWSKUL_NUM },
   { "GOTREDSKULL", GOTREDSKULL_NUM },
   { "GOTINVUL", GOTINVUL_NUM },
   { "GOTBERSERK", GOTBERSERK_NUM },
   { "GOTINVIS", GOTINVIS_NUM },
   { "GOTSUIT", GOTSUIT_NUM },
   { "GOTMAP", GOTMAP_NUM },
   { "GOTVISOR", GOTVISOR_NUM },
   { "GOTMSPHERE", GOTMSPHERE_NUM },
   { "GOTCLIP", GOTCLIP_NUM },
   { "GOTCLIPBOX", GOTCLIPBOX_NUM },
   { "GOTROCKET", GOTROCKET_NUM },
   { "GOTROCKBOX", GOTROCKBOX_NUM },
   { "GOTCELL", GOTCELL_NUM },
   { "GOTCELLBOX", GOTCELLBOX_NUM },
   { "GOTSHELLS", GOTSHELLS_NUM },
   { "GOTSHELLBOX", GOTSHELLBOX_NUM },
   { "GOTBACKPACK", GOTBACKPACK_NUM },
   { "GOTBFG9000", GOTBFG9000_NUM },
   { "GOTCHAINGUN", GOTCHAINGUN_NUM },
   { "GOTCHAINSAW", GOTCHAINSAW_NUM },
   { "GOTLAUNCHER", GOTLAUNCHER_NUM },
   { "GOTPLASMA", GOTPLASMA_NUM },
   { "GOTSHOTGUN", GOTSHOTGUN_NUM },
   { "GOTSHOTGUN2", GOTSHOTGUN2_NUM },
   { "PD_BLUEO", PD_BLUEO_NUM },
   { "PD_REDO", PD_REDO_NUM },
   { "PD_YELLOWO", PD_YELLOWO_NUM },
   { "PD_BLUEK", PD_BLUEK_NUM },
   { "PD_REDK", PD_REDK_NUM },
   { "PD_YELLOWK", PD_YELLOWK_NUM },
   { "PD_BLUEC", PD_BLUEC_NUM },
   { "PD_REDC", PD_REDC_NUM },
   { "PD_YELLOWC", PD_YELLOWC_NUM },
   { "PD_BLUES", PD_BLUES_NUM },
   { "PD_REDS", PD_REDS_NUM },
   { "PD_YELLOWS", PD_YELLOWS_NUM },
   { "PD_ANY", PD_ANY_NUM },
   { "PD_ALL3", PD_ALL3_NUM },
   { "PD_ALL6", PD_ALL6_NUM },
   { "HUSTR_MSGU", HUSTR_MSGU_NUM },
   { "HUSTR_E1M1", HUSTR_E1M1_NUM },
   { "HUSTR_E1M2", HUSTR_E1M2_NUM },
   { "HUSTR_E1M3", HUSTR_E1M3_NUM },
   { "HUSTR_E1M4", HUSTR_E1M4_NUM },
   { "HUSTR_E1M5", HUSTR_E1M5_NUM },
   { "HUSTR_E1M6", HUSTR_E1M6_NUM },
   { "HUSTR_E1M7", HUSTR_E1M7_NUM },
   { "HUSTR_E1M8", HUSTR_E1M8_NUM },
   { "HUSTR_E1M9", HUSTR_E1M9_NUM },
   { "HUSTR_E2M1", HUSTR_E2M1_NUM },
   { "HUSTR_E2M2", HUSTR_E2M2_NUM },
   { "HUSTR_E2M3", HUSTR_E2M3_NUM },
   { "HUSTR_E2M4", HUSTR_E2M4_NUM },
   { "HUSTR_E2M5", HUSTR_E2M5_NUM },
   { "HUSTR_E2M6", HUSTR_E2M6_NUM },
   { "HUSTR_E2M7", HUSTR_E2M7_NUM },
   { "HUSTR_E2M8", HUSTR_E2M8_NUM },
   { "HUSTR_E2M9", HUSTR_E2M9_NUM },
   { "HUSTR_E3M1", HUSTR_E3M1_NUM },
   { "HUSTR_E3M2", HUSTR_E3M2_NUM },
   { "HUSTR_E3M3", HUSTR_E3M3_NUM },
   { "HUSTR_E3M4", HUSTR_E3M4_NUM },
   { "HUSTR_E3M5", HUSTR_E3M5_NUM },
   { "HUSTR_E3M6", HUSTR_E3M6_NUM },
   { "HUSTR_E3M7", HUSTR_E3M7_NUM },
   { "HUSTR_E3M8", HUSTR_E3M8_NUM },
   { "HUSTR_E3M9", HUSTR_E3M9_NUM },
   { "HUSTR_E4M1", HUSTR_E4M1_NUM },
   { "HUSTR_E4M2", HUSTR_E4M2_NUM },
   { "HUSTR_E4M3", HUSTR_E4M3_NUM },
   { "HUSTR_E4M4", HUSTR_E4M4_NUM },
   { "HUSTR_E4M5", HUSTR_E4M5_NUM },
   { "HUSTR_E4M6", HUSTR_E4M6_NUM },
   { "HUSTR_E4M7", HUSTR_E4M7_NUM },
   { "HUSTR_E4M8", HUSTR_E4M8_NUM },
   { "HUSTR_E4M9", HUSTR_E4M9_NUM },
   { "HUSTR_1", HUSTR_1_NUM },
   { "HUSTR_2", HUSTR_2_NUM },
   { "HUSTR_3", HUSTR_3_NUM },
   { "HUSTR_4", HUSTR_4_NUM },
   { "HUSTR_5", HUSTR_5_NUM },
   { "HUSTR_6", HUSTR_6_NUM },
   { "HUSTR_7", HUSTR_7_NUM },
   { "HUSTR_8", HUSTR_8_NUM },
   { "HUSTR_9", HUSTR_9_NUM },
   { "HUSTR_10", HUSTR_10_NUM },
   { "HUSTR_11", HUSTR_11_NUM },
   { "HUSTR_12", HUSTR_12_NUM },
   { "HUSTR_13", HUSTR_13_NUM },
   { "HUSTR_14", HUSTR_14_NUM },
   { "HUSTR_15", HUSTR_15_NUM },
   { "HUSTR_16", HUSTR_16_NUM },
   { "HUSTR_17", HUSTR_17_NUM },
   { "HUSTR_18", HUSTR_18_NUM },
   { "HUSTR_19", HUSTR_19_NUM },
   { "HUSTR_20", HUSTR_20_NUM },
   { "HUSTR_21", HUSTR_21_NUM },
   { "HUSTR_22", HUSTR_22_NUM },
   { "HUSTR_23", HUSTR_23_NUM },
   { "HUSTR_24", HUSTR_24_NUM },
   { "HUSTR_25", HUSTR_25_NUM },
   { "HUSTR_26", HUSTR_26_NUM },
   { "HUSTR_27", HUSTR_27_NUM },
   { "HUSTR_28", HUSTR_28_NUM },
   { "HUSTR_29", HUSTR_29_NUM },
   { "HUSTR_30", HUSTR_30_NUM },
   { "HUSTR_31", HUSTR_31_NUM },
   { "HUSTR_32", HUSTR_32_NUM },
   { "PHUSTR_1", PHUSTR_1_NUM },
   { "PHUSTR_2", PHUSTR_2_NUM },
   { "PHUSTR_3", PHUSTR_3_NUM },
   { "PHUSTR_4", PHUSTR_4_NUM },
   { "PHUSTR_5", PHUSTR_5_NUM },
   { "PHUSTR_6", PHUSTR_6_NUM },
   { "PHUSTR_7", PHUSTR_7_NUM },
   { "PHUSTR_8", PHUSTR_8_NUM },
   { "PHUSTR_9", PHUSTR_9_NUM },
   { "PHUSTR_10", PHUSTR_10_NUM },
   { "PHUSTR_11", PHUSTR_11_NUM },
   { "PHUSTR_12", PHUSTR_12_NUM },
   { "PHUSTR_13", PHUSTR_13_NUM },
   { "PHUSTR_14", PHUSTR_14_NUM },
   { "PHUSTR_15", PHUSTR_15_NUM },
   { "PHUSTR_16", PHUSTR_16_NUM },
   { "PHUSTR_17", PHUSTR_17_NUM },
   { "PHUSTR_18", PHUSTR_18_NUM },
   { "PHUSTR_19", PHUSTR_19_NUM },
   { "PHUSTR_20", PHUSTR_20_NUM },
   { "PHUSTR_21", PHUSTR_21_NUM },
   { "PHUSTR_22", PHUSTR_22_NUM },
   { "PHUSTR_23", PHUSTR_23_NUM },
   { "PHUSTR_24", PHUSTR_24_NUM },
   { "PHUSTR_25", PHUSTR_25_NUM },
   { "PHUSTR_26", PHUSTR_26_NUM },
   { "PHUSTR_27", PHUSTR_27_NUM },
   { "PHUSTR_28", PHUSTR_28_NUM },
   { "PHUSTR_29", PHUSTR_29_NUM },
   { "PHUSTR_30", PHUSTR_30_NUM },
   { "PHUSTR_31", PHUSTR_31_NUM },
   { "PHUSTR_32", PHUSTR_32_NUM },
   { "THUSTR_1", THUSTR_1_NUM },
   { "THUSTR_2", THUSTR_2_NUM },
   { "THUSTR_3", THUSTR_3_NUM },
   { "THUSTR_4", THUSTR_4_NUM },
   { "THUSTR_5", THUSTR_5_NUM },
   { "THUSTR_6", THUSTR_6_NUM },
   { "THUSTR_7", THUSTR_7_NUM },
   { "THUSTR_8", THUSTR_8_NUM },
   { "THUSTR_9", THUSTR_9_NUM },
   { "THUSTR_10", THUSTR_10_NUM },
   { "THUSTR_11", THUSTR_11_NUM },
   { "THUSTR_12", THUSTR_12_NUM },
   { "THUSTR_13", THUSTR_13_NUM },
   { "THUSTR_14", THUSTR_14_NUM },
   { "THUSTR_15", THUSTR_15_NUM },
   { "THUSTR_16", THUSTR_16_NUM },
   { "THUSTR_17", THUSTR_17_NUM },
   { "THUSTR_18", THUSTR_18_NUM },
   { "THUSTR_19", THUSTR_19_NUM },
   { "THUSTR_20", THUSTR_20_NUM },
   { "THUSTR_21", THUSTR_21_NUM },
   { "THUSTR_22", THUSTR_22_NUM },
   { "THUSTR_23", THUSTR_23_NUM },
   { "THUSTR_24", THUSTR_24_NUM },
   { "THUSTR_25", THUSTR_25_NUM },
   { "THUSTR_26", THUSTR_26_NUM },
   { "THUSTR_27", THUSTR_27_NUM },
   { "THUSTR_28", THUSTR_28_NUM },
   { "THUSTR_29", THUSTR_29_NUM },
   { "THUSTR_30", THUSTR_30_NUM },
   { "THUSTR_31", THUSTR_31_NUM },
   { "THUSTR_32", THUSTR_32_NUM },

   { "E1TEXT", E1TEXT_NUM },
   { "E2TEXT", E2TEXT_NUM },
   { "E3TEXT", E3TEXT_NUM },
   { "E4TEXT", E4TEXT_NUM },
   { "C1TEXT", C1TEXT_NUM },
   { "C2TEXT", C2TEXT_NUM },
   { "C3TEXT", C3TEXT_NUM },
   { "C4TEXT", C4TEXT_NUM },
   { "C5TEXT", C5TEXT_NUM },
   { "C6TEXT", C6TEXT_NUM },
   { "P1TEXT", P1TEXT_NUM },
   { "P2TEXT", P2TEXT_NUM },
   { "P3TEXT", P3TEXT_NUM },
   { "P4TEXT", P4TEXT_NUM },
   { "P5TEXT", P5TEXT_NUM },
   { "P6TEXT", P6TEXT_NUM },
   { "T1TEXT", T1TEXT_NUM },
   { "T2TEXT", T2TEXT_NUM },
   { "T3TEXT", T3TEXT_NUM },
   { "T4TEXT", T4TEXT_NUM },
   { "T5TEXT", T5TEXT_NUM },
   { "T6TEXT", T6TEXT_NUM },
   { "STSTR_DQDON", STSTR_DQDON_NUM },  // Invincible
   { "STSTR_DQDOFF", STSTR_DQDOFF_NUM },
   { "STSTR_FAADDED", STSTR_FAADDED_NUM },  // Full Ammo
   { "STSTR_KFAADDED", STSTR_KFAADDED_NUM },  // Full Ammo Keys
   { "STSTR_BEHOLD", STSTR_BEHOLD_NUM },  // Power-up
   { "STSTR_BEHOLDX", STSTR_BEHOLDX_NUM }, // Power-up toggle
   { "STSTR_CHOPPERS", STSTR_CHOPPERS_NUM },  // Chainsaw

   { "CC_ZOMBIE", CC_ZOMBIE_NUM },
   { "CC_SHOTGUN", CC_SHOTGUN_NUM },
   { "CC_HEAVY", CC_HEAVY_NUM },
   { "CC_IMP", CC_IMP_NUM },
   { "CC_DEMON", CC_DEMON_NUM },
   { "CC_LOST", CC_LOST_NUM },
   { "CC_CACO", CC_CACO_NUM },
   { "CC_HELL", CC_HELL_NUM },
   { "CC_BARON", CC_BARON_NUM },
   { "CC_ARACH", CC_ARACH_NUM },
   { "CC_PAIN", CC_PAIN_NUM },
   { "CC_REVEN", CC_REVEN_NUM },
   { "CC_MANCU", CC_MANCU_NUM },
   { "CC_ARCH", CC_ARCH_NUM },
   { "CC_SPIDER", CC_SPIDER_NUM },
   { "CC_CYBER", CC_CYBER_NUM },
   { "CC_HERO", CC_HERO_NUM },

   { "BGFLATE1", BGFLATE1_NUM },
   { "BGFLATE2", BGFLATE2_NUM },
   { "BGFLATE3", BGFLATE3_NUM },
   { "BGFLATE4", BGFLATE4_NUM },
   { "BGFLAT06", BGFLAT06_NUM },
   { "BGFLAT11", BGFLAT11_NUM },
   { "BGFLAT20", BGFLAT20_NUM },
   { "BGFLAT30", BGFLAT30_NUM },
   { "BGFLAT15", BGFLAT15_NUM },
   { "BGFLAT31", BGFLAT31_NUM },
#ifdef BEX_SAVEGAMENAME     
   { "SAVEGAMENAME", SAVEGAMENAME_NUM },  // [WDJ] Added 9/5/2011
#else
   { "SAVEGAMENAME", 9998 },  // [WDJ] Do not allow, because of security risk
#endif

// BEX not present in DoomLegacy
   { "BGCASTCALL", 9998 },
   { "STARTUP1", 9998 },
   { "STARTUP2", 9998 },
   { "STARTUP3", 9998 },
   { "STARTUP4", 9998 },
   { "STARTUP5", 9998 },

   { NULL, 0 }  // table term
};


#define BEX_MAX_STRING_LEN   2000
#define BEX_KEYW_LEN  20

// BEX [STRINGS] section
// permission: 0=game, 1=adv, 2=language
static void bex_strings( myfile_t* f, byte bex_permission )
{
  char stxt[BEX_MAX_STRING_LEN+1];
  char keyw[BEX_KEYW_LEN];
  char sb[MAXLINELEN];
  char * stp;
  char * word;
  char * cp;
  int perm_min = bex_string_start_table[bex_permission];
  int i;

  // string format, no quotes:
  // [STRINGS]
  // #comment, ** Maybe ** comment within replacement string
  // <keyw> = <text>
  // <keyw> = <text> backslash
  //   <text> backslash
  //   <text>

  for(;;) {
    if( ! myfgets_nocom(sb, sizeof(sb), f) )  // get line, skipping comments
       break; // no more lines
    if( sb[0] == '\n' ) continue;  // blank line
    if( sb[0] == 0 ) break;
    cp = strchr(sb,'=');  // find after =
    word=strtok(sb," ");
    if( ! word ) break;
    strncpy( keyw, word, BEX_KEYW_LEN-1 );  // because continuation lines use sb
    keyw[BEX_KEYW_LEN-1] = '\0';
    // Limited by buffer size.
    if( cp == NULL ) goto no_text_change;
    cp++; // skip '='
    stxt[BEX_MAX_STRING_LEN] = '\0'; // protection
    stp = &stxt[0];
    // Get the new text
    do {
      while( *cp == ' ' || *cp == '\t' )  cp++; // skip leading space
      if( *cp == '\n' ) break;  // blank line
      if( *cp == 0 ) break;
      if( *cp == '\"' )  cp++;  // skip leading double quote
      while( *cp )
      {   // copy text upto CR
          if( *cp == '\n' ) break;
          *stp++ = *cp++;
          if( stp >= &stxt[BEX_MAX_STRING_LEN] ) break;
      }
      // remove trailing space
      while( stp > stxt && stp[-1] == ' ')
          stp --;
      // test for continuation line
      if( ! (stp > stxt && stp[-1] == '\\') )
          break;  // no backslash continuation
      stp--;  // remove backslash
      if( stp > stxt && stp[-1] == '\"' )
          stp --;  // remove trailing doublequote
      // get continuation line to sb, skipping comments.
      // [WDJ] questionable, but boom202 code skips comments between continuation lines.
      if( ! myfgets_nocom(sb, sizeof(sb), f) )
          break; // no more lines
      cp = &sb[0];
    } while ( *cp );
    if( stp > stxt && stp[-1] == '\"' )
        stp --;  // remove trailing doublequote
    *stp++ = '\0';  // term BEX replacement string in stxt
     
    // search text table for keyw
    for(i=0;  ;i++)
    {
        if( bex_string_table[i].kstr == NULL )  goto no_text_change;
        if(!strcmp(bex_string_table[i].kstr, keyw))  // BEX keyword search
        {
            int text_index = bex_string_table[i].text_num;
#ifdef BEX_SAVEGAMENAME
            // protect file names against attack
            if( i == SAVEGAMENAME_NUM )
            {
                if( filename_reject( stxt, 10 ) )  goto no_text_change;
            }
#endif
            if( i >= perm_min && text_index < NUMTEXT)
            {
                // May be const string, which will segfault on write
                deh_replace_string( &text[text_index], stxt, DRS_string );
            }
            else
            {
                // change blocked, but not an error
            }
            goto next_keyw;
        }
    }
  no_text_change:
    deh_error("Text not changed :%s\n", keyw);
     
  next_keyw:
    continue;
  }
}

// BEX [PARS] section
static void bex_pars( myfile_t* f )
{
  char s[MAXLINELEN];
  int  episode, level, partime;
  int  nn;
   
  // format:
  // [PARS]
  // par <episode> <level> <seconds>
  // par <map_level> <seconds>

  for(;;) {
    if( ! myfgets_nocom(s, sizeof(s), f) )
       break; // no more lines
    if( s[0] == '\n' ) continue;  // blank line
    if( s[0] == 0 ) break;
    if( strncasecmp( s, "par", 3 ) != 0 )  break;  // not a par line
    nn = sscanf( &s[3], " %i %i %i", &episode, &level, &partime );
    if( nn == 3 )
    { // Doom1 Episode, level, time format
      if( (episode < 1) || (episode > 3) || (level < 1) || (level > 9) )
        deh_error( "Bad par E%dM%d\n", episode, level );
      else
      {
        pars[episode][level] = partime;
        pars_valid_bex = true;
      }
    }
    else if( nn == 2 )
    { // Doom2 map, time format
      partime = level;
      level = episode;
      if( (level < 1) || (level > 32))
        deh_error( "Bad PAR MAP%d\n", level );
      else
      {
        cpars[level-1] = partime;
        pars_valid_bex = true;
      }
    }
    else
      deh_error( "Invalid par format\n" );
  }
}


// [WDJ] BEX codeptr strings and function
typedef struct {
    const char *    kstr;
    actionf_t       action;  // union of action ptrs
} PACKED_ATTR  bex_codeptr_t;

// BEX entries from boom202s/boomdeh.txt
bex_codeptr_t  bex_action_table[] = {
   {"NULL", {NULL}},  // to clear a ptr
   {"Light0", {A_Light0}},
   {"WeaponReady", {A_WeaponReady}},
   {"Lower", {A_Lower}},
   {"Raise", {A_Raise}},
   {"Punch", {A_Punch}},
   {"ReFire", {A_ReFire}},
   {"FirePistol", {A_FirePistol}},
   {"Light1", {A_Light1}},
   {"FireShotgun", {A_FireShotgun}},
   {"Light2", {A_Light2}},
   {"FireShotgun2", {A_FireShotgun2}},
   {"CheckReload", {A_CheckReload}},
   {"OpenShotgun2", {A_OpenShotgun2}},
   {"LoadShotgun2", {A_LoadShotgun2}},
   {"CloseShotgun2", {A_CloseShotgun2}},
   {"FireCGun", {A_FireCGun}},
   {"GunFlash", {A_GunFlash}},
   {"FireMissile", {A_FireMissile}},
   {"Saw", {A_Saw}},
   {"FirePlasma", {A_FirePlasma}},
   {"BFGsound", {A_BFGsound}},
   {"FireBFG", {A_FireBFG}},
   {"BFGSpray", {A_BFGSpray}},
   {"Explode", {A_Explode}},
   {"Pain", {A_Pain}},
   {"PlayerScream", {A_PlayerScream}},
   {"Fall", {A_Fall}},
   {"XScream", {A_XScream}},
   {"Look", {A_Look}},
   {"Chase", {A_Chase}},
   {"FaceTarget", {A_FaceTarget}},
   {"PosAttack", {A_PosAttack}},
   {"Scream", {A_Scream}},
   {"SPosAttack", {A_SPosAttack}},
   {"VileChase", {A_VileChase}},
   {"VileStart", {A_VileStart}},
   {"VileTarget", {A_VileTarget}},
   {"VileAttack", {A_VileAttack}},
   {"StartFire", {A_StartFire}},
   {"Fire", {A_Fire}},
   {"FireCrackle", {A_FireCrackle}},
   {"Tracer", {A_Tracer}},
   {"SkelWhoosh", {A_SkelWhoosh}},
   {"SkelFist", {A_SkelFist}},
   {"SkelMissile", {A_SkelMissile}},
   {"FatRaise", {A_FatRaise}},
   {"FatAttack1", {A_FatAttack1}},
   {"FatAttack2", {A_FatAttack2}},
   {"FatAttack3", {A_FatAttack3}},
   {"BossDeath", {A_BossDeath}},
   {"CPosAttack", {A_CPosAttack}},
   {"CPosRefire", {A_CPosRefire}},
   {"TroopAttack", {A_TroopAttack}},
   {"SargAttack", {A_SargAttack}},
   {"HeadAttack", {A_HeadAttack}},
   {"BruisAttack", {A_BruisAttack}},
   {"SkullAttack", {A_SkullAttack}},
   {"Metal", {A_Metal}},
   {"SpidRefire", {A_SpidRefire}},
   {"BabyMetal", {A_BabyMetal}},
   {"BspiAttack", {A_BspiAttack}},
   {"Hoof", {A_Hoof}},
   {"CyberAttack", {A_CyberAttack}},
   {"PainAttack", {A_PainAttack}},
   {"PainDie", {A_PainDie}},
   {"KeenDie", {A_KeenDie}},
   {"BrainPain", {A_BrainPain}},
   {"BrainScream", {A_BrainScream}},
   {"BrainDie", {A_BrainDie}},
   {"BrainAwake", {A_BrainAwake}},
   {"BrainSpit", {A_BrainSpit}},
   {"SpawnSound", {A_SpawnSound}},
   {"SpawnFly", {A_SpawnFly}},
   {"BrainExplode", {A_BrainExplode}},

   // [WDJ] MBF function ptrs, from MBF, EternityEngine.
   {"Detonate", {A_Detonate}},  // Radius damage, variable damage
   {"Mushroom", {A_Mushroom}},  // Mushroom explosion
   {"Die", {A_Die}},  // MBF, kill an object
   {"Spawn", {A_Spawn}},  // SpawnMobj(x,y, parm2, parm1-1)
   {"Turn", {A_Turn}},  // Turn by parm1 degrees
   {"Face", {A_Face}},  // Turn to face parm1 degrees
   {"Scratch", {A_Scratch}},  // Melee attack
   {"PlaySound", {A_PlaySound}},  // Play Sound parm1
      // if parm2 = 0 then mobj sound, else unassociated sound.
   {"RandomJump", {A_RandomJump}},  // Random transition to mobj state parm1
      // probability parm2
   {"LineEffect", {A_LineEffect}},  // Trigger line type parm1, tag = parm2

   {"KeepChasing", {A_KeepChasing}},  // MBF, from EnternityEngine
   
#if 0   
   // [WDJ] Code pointers that are BEX available in EternityEngine.
   {"Nailbomb", {A_Nailbomb}},
#endif

#if 0   
   // haleyjd: start new eternity codeptrs
   {"StartScript", {A_StartScript}},
   {"PlayerStartScript", {A_PlayerStartScript}},
   {"SetFlags", {A_SetFlags}},
   {"UnSetFlags", {A_UnSetFlags}},
   {"BetaSkullAttack", {A_BetaSkullAttack}},
   {"GenRefire", {A_GenRefire}},
   {"FireGrenade", {A_FireGrenade}},
   {"FireCustomBullets", {A_FireCustomBullets}},
   {"FirePlayerMissile", {A_FirePlayerMissile}},
   {"CustomPlayerMelee", {A_CustomPlayerMelee}},
   {"GenTracer", {A_GenTracer}},
   {"BFG11KHit", {A_BFG11KHit}},
   {"BouncingBFG", {A_BouncingBFG}},
   {"BFGBurst", {A_BFGBurst}},          
   {"FireOldBFG", {A_FireOldBFG}},        
   {"Stop", {A_Stop}},              
   {"PlayerThunk", {A_PlayerThunk}},
   {"MissileAttack", {A_MissileAttack}},
   {"MissileSpread", {A_MissileSpread}},
   {"BulletAttack", {A_BulletAttack}},
   {"HealthJump", {A_HealthJump}},
   {"CounterJump", {A_CounterJump}},
   {"CounterSwitch", {A_CounterSwitch}},
   {"SetCounter", {A_SetCounter}},
   {"CopyCounter", {A_CopyCounter}},
   {"CounterOp", {A_CounterOp}},
   {"SetTics", {A_SetTics}},
   {"AproxDistance", {A_AproxDistance}},
   {"ShowMessage", {A_ShowMessage}},
   {"RandomWalk", {A_RandomWalk}},
   {"TargetJump", {A_TargetJump}},
   {"ThingSummon", {A_ThingSummon}},
   {"KillChildren", {A_KillChildren}},
   {"WeaponCtrJump", {A_WeaponCtrJump}},
   {"WeaponCtrSwitch", {A_WeaponCtrSwitch}},
   {"WeaponSetCtr", {A_WeaponSetCtr}},
   {"WeaponCopyCtr", {A_WeaponCopyCtr}},
   {"WeaponCtrOp", {A_WeaponCtrOp}},
   {"AmbientThinker", {A_AmbientThinker}},
   {"SteamSpawn", {A_SteamSpawn}},
   {"EjectCasing", {A_EjectCasing}},
   {"CasingThrust", {A_CasingThrust}},
   {"JumpIfNoAmmo", {A_JumpIfNoAmmo}},
   {"CheckReloadEx", {A_CheckReloadEx}},
#endif

#if 0
   // haleyjd 07/13/03: nuke specials
   {"PainNukeSpec", {A_PainNukeSpec}},
   {"SorcNukeSpec", {A_SorcNukeSpec}},
#endif

   // haleyjd: Heretic pointers
   // Unique to EternityEngine, parameterized.
//   {"SpawnGlitter", {A_SpawnGlitter}},
//   {"AccelGlitter", {A_AccelGlitter}},
//   {"SpawnAbove", {A_SpawnAbove}},
//   {"HticDrop", {A_HticDrop}},  // Deprecated
//   {"HticTracer", {A_HticTracer}},  // parameterized
//   {"HticExplode", {A_HticExplode}},
//   {"HticBossDeath", {A_HticBossDeath}},
   // Heretic normal
   {"MummyAttack", {A_MummyAttack}},
   {"MummyAttack2", {A_MummyAttack2}},
   {"MummySoul", {A_MummySoul}},
   {"ClinkAttack", {A_ClinkAttack}},
   {"BlueSpark", {A_BlueSpark}},
   {"GenWizard", {A_GenWizard}},
   {"WizardAtk1", {A_WizAtk1}},
   {"WizardAtk2", {A_WizAtk2}},
   {"WizardAtk3", {A_WizAtk3}},
   {"SorcererRise", {A_SorcererRise}},
   {"Srcr1Attack", {A_Srcr1Attack}},
   {"Srcr2Decide", {A_Srcr2Decide}},
   {"Srcr2Attack", {A_Srcr2Attack}},
   {"Sor1Chase", {A_Sor1Chase}},
   {"Sor1Pain", {A_Sor1Pain}},
   {"Sor2DthInit", {A_Sor2DthInit}},
   {"Sor2DthLoop", {A_Sor2DthLoop}},
   {"PodPain", {A_PodPain}},
   {"RemovePod", {A_RemovePod}},
   {"MakePod", {A_MakePod}},
   {"KnightAttack", {A_KnightAttack}},
   {"DripBlood", {A_DripBlood}},
   {"BeastAttack", {A_BeastAttack}},
   {"BeastPuff", {A_BeastPuff}},
   {"SnakeAttack", {A_SnakeAttack}},
   {"SnakeAttack2", {A_SnakeAttack2}},
   {"VolcanoBlast", {A_VolcanoBlast}},
   {"VolcBallImpact", {A_VolcBallImpact}},
   {"MinotaurDecide", {A_MinotaurDecide}},
   {"MinotaurAtk1", {A_MinotaurAtk1}},
   {"MinotaurAtk2", {A_MinotaurAtk2}},
   {"MinotaurAtk3", {A_MinotaurAtk3}},
   {"MinotaurCharge", {A_MinotaurCharge}},
   {"MntrFloorFire", {A_MntrFloorFire}},
//   {"LichFire", {A_LichFire}},
//   {"LichWhirlwind", {A_LichWhirlwind}},
   {"LichAttack", {A_HHeadAttack}},
   {"WhirlwindSeek", {A_WhirlwindSeek}},
   {"LichIceImpact", {A_HeadIceImpact}},
   {"LichFireGrow", {A_HeadFireGrow}},
//   {"ImpChargeAtk", {A_ImpChargeAtk}},
   {"ImpMeleeAtk", {A_ImpMeAttack}},
   {"ImpMissileAtk", {A_ImpMsAttack}},  // Boss Imp
   {"ImpDeath", {A_ImpDeath}},
   {"ImpXDeath1", {A_ImpXDeath1}},
   {"ImpXDeath2", {A_ImpXDeath2}},
   {"ImpExplode", {A_ImpExplode}},
   {"PhoenixPuff", {A_PhoenixPuff}},
//   {"PlayerSkull", {A_PlayerSkull}},
//   {"ClearSkin", {A_ClearSkin}},
   
#if 0
   // haleyjd 10/04/08: Hexen pointers
   {"SetInvulnerable", {A_SetInvulnerable}},
   {"UnSetInvulnerable", {A_UnSetInvulnerable}},
   {"SetReflective", {A_SetReflective}},
   {"UnSetReflective", {A_UnSetReflective}},
   {"PigLook", {A_PigLook}},
   {"PigChase", {A_PigChase}},
   {"PigAttack", {A_PigAttack}},
   {"PigPain", {A_PigPain}},
   {"HexenExplode", {A_HexenExplode}},
   {"SerpentUnHide", {A_SerpentUnHide}},
   {"SerpentHide", {A_SerpentHide}},
   {"SerpentChase", {A_SerpentChase}},
   {"RaiseFloorClip", {A_RaiseFloorClip}},
   {"LowerFloorClip", {A_LowerFloorClip}},
   {"SerpentHumpDecide", {A_SerpentHumpDecide}},
   {"SerpentWalk", {A_SerpentWalk}},
   {"SerpentCheckForAttack", {A_SerpentCheckForAttack}},
   {"SerpentChooseAttack", {A_SerpentChooseAttack}},
   {"SerpentMeleeAttack", {A_SerpentMeleeAttack}},
   {"SerpentMissileAttack", {A_SerpentMissileAttack}},
   {"SerpentSpawnGibs", {A_SerpentSpawnGibs}},
   {"SubTics", {A_SubTics}},
   {"SerpentHeadCheck", {A_SerpentHeadCheck}},
   {"CentaurAttack", {A_CentaurAttack}},
   {"CentaurAttack2", {A_CentaurAttack2}},
   {"DropEquipment", {A_DropEquipment}},
   {"CentaurDefend", {A_CentaurDefend}},
   {"BishopAttack", {A_BishopAttack}},
   {"BishopAttack2", {A_BishopAttack2}},
   {"BishopMissileWeave", {A_BishopMissileWeave}},
   {"BishopDoBlur", {A_BishopDoBlur}},
   {"SpawnBlur", {A_SpawnBlur}},
   {"BishopChase", {A_BishopChase}},
   {"BishopPainBlur", {A_BishopPainBlur}},
   {"DragonInitFlight", {A_DragonInitFlight}},
   {"DragonFlight", {A_DragonFlight}},
   {"DragonFlap", {A_DragonFlap}},
   {"DragonFX2", {A_DragonFX2}},
   {"PainCounterBEQ", {A_PainCounterBEQ}},
   {"DemonAttack1", {A_DemonAttack1}},
   {"DemonAttack2", {A_DemonAttack2}},
   {"DemonDeath", {A_DemonDeath}},
   {"Demon2Death", {A_Demon2Death}},
   {"WraithInit", {A_WraithInit}},
   {"WraithRaiseInit", {A_WraithRaiseInit}},
   {"WraithRaise", {A_WraithRaise}},
   {"WraithMelee", {A_WraithMelee}},
   {"WraithMissile", {A_WraithMissile}},
   {"WraithFX2", {A_WraithFX2}},
   {"WraithFX3", {A_WraithFX3}},
   {"WraithFX4", {A_WraithFX4}},
   {"WraithLook", {A_WraithLook}},
   {"WraithChase", {A_WraithChase}},
   {"EttinAttack", {A_EttinAttack}},
   {"DropMace", {A_DropMace}},
   {"AffritSpawnRock", {A_AffritSpawnRock}},
   {"AffritRocks", {A_AffritRocks}},
   {"SmBounce", {A_SmBounce}},
   {"AffritChase", {A_AffritChase}},
   {"AffritSplotch", {A_AffritSplotch}},
   {"IceGuyLook", {A_IceGuyLook}},
   {"IceGuyChase", {A_IceGuyChase}},
   {"IceGuyAttack", {A_IceGuyAttack}},
   {"IceGuyDie", {A_IceGuyDie}},
   {"IceGuyMissileExplode", {A_IceGuyMissileExplode}},
   {"CheckFloor", {A_CheckFloor}},
   {"FreezeDeath", {A_FreezeDeath}},
   {"IceSetTics", {A_IceSetTics}},
   {"IceCheckHeadDone", {A_IceCheckHeadDone}},
   {"FreezeDeathChunks", {A_FreezeDeathChunks}},
#endif

   { NULL, {NULL} }  // table term
};



// BEX [CODEPTR] section
static void bex_codeptr( myfile_t* f )
{
  char funcname[BEX_KEYW_LEN];
  char s[MAXLINELEN];
  int  framenum, nn, i, si;
   
  // format:
  // [CODEPTR]
  // FRAME <framenum> = <funcname>

  for(;;) {
    if( ! myfgets_nocom(s, sizeof(s), f) )
       break; // no more lines
    if( s[0] == '\n' ) continue;  // blank line
    if( s[0] == 0 ) break;
    if( strncasecmp( s, "FRAME", 5 ) != 0 )  break;  // not a FRAME line
    nn = sscanf( &s[5], "%d = %s", &framenum, funcname );
    if( nn != 2 )
    {
        deh_error( "Bad FRAME syntax\n" );
        continue;
    }
    si = deh_frame_to_state(framenum);
    if( si == S_NULL )
    {
        deh_error( "Bad BEX FRAME number %d\n", framenum );
        continue;
    }
    // search action table
    for(i=0;  ;i++)
    {
        if( bex_action_table[i].kstr == NULL )  goto no_action_change;
        if(!strcasecmp(bex_action_table[i].kstr, funcname))  // BEX action search
        {
            // change the sprite behavior at the framenum
            states[si].action.acv = bex_action_table[i].action.acv;
            goto next_keyw;
        }
    }
  no_action_change:
    deh_error("Action not changed : FRAME %d\n", framenum);
     
  next_keyw:
    continue;
  }
}


// [WDJ] MBF helper, From PrBoom (not in MBF)
// haleyjd 9/22/99
//
// Allows handy substitution of any thing for helper dogs.  DEH patches
// are being made frequently for this purpose and it requires a complete
// rewiring of the DOG thing.  I feel this is a waste of effort, and so
// have added this new [HELPER] BEX block

// BEX [HELPER] section
static void bex_helper( myfile_t* f )
{
  char s[MAXLINELEN];
  int  tn, nn;
   
  // format:
  // [HELPER]
  // TYPE = <num>

  for(;;)
  {
    if( ! myfgets_nocom(s, sizeof(s), f) )
       break; // no more lines

    if( s[0] == '\n' ) continue;  // blank line
    if( s[0] == 0 ) break;
    if( strncasecmp( s, "TYPE", 4 ) != 0 )  break;  // not a TYPE line
    nn = sscanf( &s[4], " = %d", &tn );

    if( nn != 1 )
    {
        deh_error( "Bad TYPE syntax\n" );
        continue;
    }

    // In BEX things are 1..
    if( tn <= 0 || tn > ENDDOOM_MT )
    {
        deh_error( "Bad BEX thing number %d\n", tn );
        continue;
    }

    helper_MT = tn - 1;  // mobj MT  (in BEX things are 1.. )
  }
}
   

// include another DEH or BEX file
void bex_include( char * inclfilename )
{
  static boolean include_nested = 0;
  
  // myfile_t is local to DEH_LoadDehackedLump
  
  if( include_nested )
  {
    deh_error( "BEX INCLUDE, only one level allowed\n" );
    return;
  }
  // save state
  
  include_nested = 1;
//  DEH_LoadDehackedFile( inclfile );  // do the include file
  W_Load_WadFile (inclfilename);
  include_nested = 0;
   
  // restore state
}



/*
Ammo type = 2
Deselect frame = 11
Select frame = 12
Bobbing frame = 13
Shooting frame = 17
Firing frame = 10
*/
static void readweapon(myfile_t *f, int deh_weapon_id)
{
  weaponinfo_t * wip = & doomweaponinfo[ deh_weapon_id ];
  char s[MAXLINELEN];
  char *word;
  int value;

  do{
    if(myfgets(s,sizeof(s),f)!=NULL)
    {
      if(s[0]=='\n') break;
      value=searchvalue(s);
      word=strtok(s," ");

           if(!strcasecmp(word,"Ammo"))       wip->ammo      =value;
      else if(!strcasecmp(word,"Deselect"))   set_state( &wip->upstate, value );
      else if(!strcasecmp(word,"Select"))     set_state( &wip->downstate, value );
      else if(!strcasecmp(word,"Bobbing"))    set_state( &wip->readystate, value );
      else if(!strcasecmp(word,"Shooting")) { set_state( &wip->atkstate, value );
					      wip->holdatkstate = wip->atkstate; }
      else if(!strcasecmp(word,"Firing"))     set_state( &wip->flashstate, value );
      else deh_error("Weapon %d : unknown word '%s'\n", deh_weapon_id,word);
    }
  } while(s[0]!='\n' && !myfeof(f));
}

/*
Ammo 1
Max ammo = 400
Per ammo = 40

where:
  Ammo 0 = Bullets
  Ammo 1 = Shells (shotgun)
  Ammo 2 = Cells
  Ammo 3 = Rockets
*/

extern int clipammo[];
extern int GetWeaponAmmo[];

static void readammo(myfile_t *f,int num)
{
  char s[MAXLINELEN];
  char *word;
  int value;

  do{
    if(myfgets(s,sizeof(s),f)!=NULL)
    {
      if(s[0]=='\n') break;
      value=searchvalue(s);
      word=strtok(s," ");

           if(!strcasecmp(word,"Max"))  maxammo[num] =value;
      else if(!strcasecmp(word,"Per"))
      {
          clipammo[num]=value;	// amount of ammo for small item
          GetWeaponAmmo[num] = 2*value;  // per weapon
      }
      else if(!strcasecmp(word,"Perweapon")) GetWeaponAmmo[num] = 2*value; 
      else deh_error("Ammo %d : unknown word '%s'\n",num,word);
    }
  } while(s[0]!='\n' && !myfeof(f));
}

// i don't like that but do you see a other way ?
extern int idfa_armor;
extern int idfa_armor_class;
extern int idkfa_armor;
extern int idkfa_armor_class;
extern int god_health;
extern int initial_health;
extern int initial_bullets;
extern int MAXHEALTH;
extern int max_armor;
extern int green_armor_class;
extern int blue_armor_class;
extern int maxsoul;
extern int soul_health;
extern int mega_health;


static void readmisc(myfile_t *f)
{
  char s[MAXLINELEN];
  char *word,*word2;
  int value;
  do{
    if(myfgets(s,sizeof(s),f)!=NULL)
    {
      if(s[0]=='\n') break;
      value=searchvalue(s);  // none -> 0
      word=strtok(s," ");
      word2=strtok(NULL," ");

      if(!strcasecmp(word,"Initial"))
      {
         if(!strcasecmp(word2,"Health"))          initial_health=value;
         else if(!strcasecmp(word2,"Bullets"))    initial_bullets=value;
      }
      else if(!strcasecmp(word,"Max"))
      {
         if(!strcasecmp(word2,"Health"))          MAXHEALTH=value;
         else if(!strcasecmp(word2,"Armor"))      max_armor=value;
         else if(!strcasecmp(word2,"Soulsphere")) maxsoul=value;
      }
      else if(!strcasecmp(word,"Green"))         green_armor_class=value;
      else if(!strcasecmp(word,"Blue"))          blue_armor_class=value;
      else if(!strcasecmp(word,"Soulsphere"))    soul_health=value;
      else if(!strcasecmp(word,"Megasphere"))    mega_health=value;
      else if(!strcasecmp(word,"God"))           god_health=value;
      else if(!strcasecmp(word,"IDFA"))
      {
         word2=strtok(NULL," ");
         if(!strcmp(word2,"="))               idfa_armor=value;
         else if(!strcasecmp(word2,"Class"))  idfa_armor_class=value;
      }
      else if(!strcasecmp(word,"IDKFA"))
      {
         word2=strtok(NULL," ");
         if(!strcmp(word2,"="))               idkfa_armor=value;
         else if(!strcasecmp(word2,"Class"))  idkfa_armor_class=value;
      }
      else if(!strcasecmp(word,"BFG"))        doomweaponinfo[wp_bfg].ammopershoot = value;
      else if(!strcasecmp(word,"Monsters Ignore"))
      {
          // ZDoom at least
          // Looks like a coop setting
          switch( value )
          {
           case 0:
             monster_infight_deh = INFT_infight_off; // infight off
             break;
           default:
             monster_infight_deh = INFT_coop; // coop
             break;
          }
      }
      else if(!strcasecmp(word,"Monsters"))
      {
          //DarkWolf95:November 21, 2003: Monsters Infight!
          //[WDJ] from prboom the string is "Monsters Infight"
          // cannot confirm any specific valid numbers
          switch( value )
          {
           case 221: // value=221 -> on (prboom)
           case 0: // previous behavior: default to on
           case 1: // if user tries to set it on
              monster_infight_deh = INFT_infight; // infight on
              break;
           case 202: // value=202 -> off (prboom)
           default:  // off
              monster_infight_deh = INFT_infight_off; // infight off
              break;
           case 3: // extended behavior, coop monsters
              monster_infight_deh = INFT_coop;
              break;
          }
      }
      else deh_error("Misc : unknown word '%s'\n",word);
    }
  } while(s[0]!='\n' && !myfeof(f));
}

extern byte cheat_mus_seq[];
extern byte cheat_choppers_seq[];
extern byte cheat_god_seq[];
extern byte cheat_ammo_seq[];
extern byte cheat_ammonokey_seq[];
extern byte cheat_noclip_seq[];
extern byte cheat_commercial_noclip_seq[];
extern byte cheat_powerup_seq[7][10];
extern byte cheat_clev_seq[];
extern byte cheat_mypos_seq[];
extern byte cheat_amap_seq[];

static void change_cheat_code(byte *cheatseq, byte *newcheat)
{
  byte *i,*j;

  // encrypt data
  for(i=newcheat;i[0]!='\0';i++)
      i[0]=SCRAMBLE(i[0]);

  for(i=cheatseq,j=newcheat;j[0]!='\0' && j[0]!=0xff;i++,j++)
  {
      if(i[0]==1 || i[0]==0xff) // no more place in the cheat
      {
         deh_error("Cheat too long\n");
         return;
      }
      else
         i[0]=j[0];
  }

  // newcheatseq < oldcheat
  j=i;
  // search special cheat with 100
  for(;i[0]!=0xff;i++)
  {
      if(i[0]==1)
      {
         *j++=1;
         *j++=0;
         *j++=0;
         break;
      }
  }
  *j=0xff;

  return;
}

// Read cheat section
static void readcheat(myfile_t *f)
{
  char s[MAXLINELEN];
  char *word,*word2;
  byte *value;

  do{
    if(myfgets(s,sizeof(s),f)!=NULL)
    {
      // for each line "<word> = <value>"
      if(s[0]=='\n') break;
      strtok(s,"=");  // after '='
      value = (byte *)strtok(NULL," \n");         // skip the space
      strtok(NULL," \n");              // finish the string
      word=strtok(s," ");

      if(!strcasecmp(word     ,"Change"))        change_cheat_code(cheat_mus_seq,value);
      else if(!strcasecmp(word,"Chainsaw"))      change_cheat_code(cheat_choppers_seq,value);
      else if(!strcasecmp(word,"God"))           change_cheat_code(cheat_god_seq,value);
      else if(!strcasecmp(word,"Ammo"))
           {
             word2=strtok(NULL," ");

             if(word2 && !strcmp(word2,"&")) change_cheat_code(cheat_ammo_seq,value);
             else                            change_cheat_code(cheat_ammonokey_seq,value);
           }
      else if(!strcasecmp(word,"No"))
           {
             word2=strtok(NULL," ");
             if(word2)
                word2=strtok(NULL," ");

             if(word2 && !strcmp(word2,"1")) change_cheat_code(cheat_noclip_seq,value);
             else                            change_cheat_code(cheat_commercial_noclip_seq,value);

           }
      else if(!strcasecmp(word,"Invincibility")) change_cheat_code(cheat_powerup_seq[0],value);
      else if(!strcasecmp(word,"Berserk"))       change_cheat_code(cheat_powerup_seq[1],value);
      else if(!strcasecmp(word,"Invisibility"))  change_cheat_code(cheat_powerup_seq[2],value);
      else if(!strcasecmp(word,"Radiation"))     change_cheat_code(cheat_powerup_seq[3],value);
      else if(!strcasecmp(word,"Auto-map"))      change_cheat_code(cheat_powerup_seq[4],value);
      else if(!strcasecmp(word,"Lite-Amp"))      change_cheat_code(cheat_powerup_seq[5],value);
      else if(!strcasecmp(word,"BEHOLD"))        change_cheat_code(cheat_powerup_seq[6],value);
      else if(!strcasecmp(word,"Level"))         change_cheat_code(cheat_clev_seq,value);
      else if(!strcasecmp(word,"Player"))        change_cheat_code(cheat_mypos_seq,value);
      else if(!strcasecmp(word,"Map"))           change_cheat_code(cheat_amap_seq,value);
      else deh_error("Cheat : unknown word '%s'\n",word);
    }
  } while(s[0]!='\n' && !myfeof(f));
}


// permission: 0=game, 1=adv, 2=language
void DEH_LoadDehackedFile(myfile_t* f, byte bex_permission)
{
  
  char       s[1000];
  char       *word,*word2;
  int        i;

  deh_num_error=0;

  // it don't test the version of doom
  // and version of dehacked file
  while(!myfeof(f))
  {
    myfgets(s,sizeof(s),f);
    if(s[0]=='\n' || s[0]=='#')  // skip blank lines and comments
      continue;

    word=strtok(s," ");  // first keyword
    if(word!=NULL)
    {
      if( word[0] == '\n' )  // ignore blank line
        continue;
      if(!strncmp(word, "[STRINGS]", 9))
      {
        bex_strings(f, bex_permission);
        continue;
      }
      else if(!strncmp(word, "[PARS]", 6))
      {
        bex_pars(f);
        continue;
      }
      else if(!strncmp(word, "[CODEPTR]", 9))
      {
        bex_codeptr(f);
        continue;
      }
      else if(!strncmp(word, "[HELPER]", 8))
      {
        bex_helper(f);
        continue;
      }
      else if(!strncmp(word, "INCLUDE", 7))
      {
        word=strtok(NULL," ");
        if(!strcasecmp( word, "NOTEXT" ))
        {
          bex_include_notext = 1;
          word=strtok(NULL," "); // filename
        }
        bex_include( word ); // include file
        bex_include_notext = 0;
        continue;
      }
      else if(!strncmp(word, "Engine", 6)
              || !strncmp(word, "Data", 4)
              || !strncmp(word, "IWAD", 4) )
         continue; // WhackEd2 data, ignore it

      word2=strtok(NULL," ");  // id number
      if(word2!=NULL)
      {
        i=atoi(word2);

        if(!strcasecmp(word,"Thing"))
        {
          // "Thing <num>"
          if(i<=NUMMOBJTYPES && i>0)
            readthing(f,i);
          else
            deh_error("Thing %d don't exist\n",i);
        }
        else if(!strcasecmp(word,"Frame"))
             {
               // "Frame <num>"
               readframe(f,i);
             }
        else if(!strcasecmp(word,"Pointer"))
             {
               // "Pointer <xref>"
               readpointer(f,i);
             }
        else if(!strcasecmp(word,"Sound"))
             {
               // "Sound <num>"
               if(i<NUMSFX && i>=0)
                   readsound(f,i);
               else
                   deh_error("Sound %d don't exist\n", i);
             }
        else if(!strcasecmp(word,"Sprite"))
             {
               // "Sprite <num>"
               // PrBoom does not support the DEH sprite block.
               if(i<NUMSPRITES && i>=0)
               {
                 if(myfgets(s,sizeof(s),f)!=NULL)
                 {
                   int k;
                   k=(searchvalue(s)-151328)/8;
                   if(k>=0 && k<NUMSPRITES)
                       sprnames[i]=deh_sprnames[k];
                   else
                       deh_error("Sprite %i : offset out of bound\n",i);
                 }
               }
               else
                  deh_error("Sprite %d don't exist\n",i);
             }
        else if(!strcasecmp(word,"Text"))
             {
               // "Text <num>"
               int j;

               if((word=strtok(NULL," "))!=NULL)
               {
                 j=atoi(word);
                 readtext(f,i,j);
               }
               else
                   deh_error("Text : missing second number\n");

             }
        else if(!strcasecmp(word,"Weapon"))
             {
               // "Weapon <num>"
               if(i<NUMWEAPONS && i>=0)
                   readweapon(f,i);
               else
                   deh_error("Weapon %d don't exist\n",i);
             }
        else if(!strcasecmp(word,"Ammo"))
             {
               // "Ammo <num>"
               if(i<NUMAMMO && i>=0)
                   readammo(f,i);
               else
                   deh_error("Ammo %d don't exist\n",i);
             }
        else if(!strcasecmp(word,"Misc"))
               // "Misc <num>"
               readmisc(f);
        else if(!strcasecmp(word,"Cheat"))
               // "Cheat <num>"
               readcheat(f);
        else if(!strcasecmp(word,"Doom"))
             {
               // "Doom <num>"
               int ver = searchvalue(strtok(NULL,"\n"));
               if( ver!=19)
                  deh_error("Warning : patch from a different doom version (%d), only version 1.9 is supported\n",ver);
             }
        else if(!strcasecmp(word,"Patch"))
             {
               // "Patch <num>"
               word=strtok(NULL," ");
               if(word && !strcasecmp(word,"format"))
               {
                  if(searchvalue(strtok(NULL,"\n"))!=6)
                     deh_error("Warning : Patch format not supported");
               }
             }
        else deh_error("Unknown word : %s\n",word);
      }
      else
          deh_error("missing argument for '%s'\n",word);
    }
    else
        deh_error("No word in this line:\n%s\n",s);

  } // end while

  if (deh_num_error>0)
  {
      CONS_Printf("%d warning(s) in the dehacked file\n",deh_num_error);
      if (devparm)
          getchar();
  }

  deh_loaded = true;
  if( flags_valid_deh )
      Translucency_OnChange();  // ensure translucent updates
}

// read dehacked lump in a wad (there is special trick for for deh 
// file that are converted to wad in w_wad.c)
void DEH_LoadDehackedLump( lumpnum_t lumpnum )
{
    myfile_t f;
    
    f.size = W_LumpLength(lumpnum);
    f.data = Z_Malloc(f.size + 1, PU_IN_USE, 0);  // temp
    W_ReadLump(lumpnum, f.data);
    f.curpos = f.data;
    f.data[f.size] = 0;

    DEH_LoadDehackedFile(&f, 0);
    Z_Free(f.data);
}


// [WDJ] Before any changes, save all comparison info, so that multiple
// DEH files and lumps can be handled without interfering with each other.
// Called once
void DEH_Init(void)
{
  int i;
  // save value for cross reference
  for(i=0;i<NUMSTATES;i++)
      deh_actions[i]=states[i].action;
  for(i=0;i<NUMSPRITES;i++)
      deh_sprnames[i]=sprnames[i];
  for(i=0;i<NUMSFX;i++)
      deh_sfxnames[i]=S_sfx[i].name;
  for(i=1;i<NUMMUSIC;i++)
      deh_musicname[i]=S_music[i].name;
  for(i=0;i<NUMTEXT;i++)
      deh_text[i]=text[i];
  monster_infight_deh = INFT_none; // not set
#if defined DEBUG_WINDOWED || defined PARANOIA
  // [WDJ] Checks of constants that I could not do at compile-time (enums).
  if( (int)STS_TECH2LAMP4 != (int)S_TECH2LAMP4 )
    I_Error( "DEH: state S_TECH2LAMP4 error\n" );
  if( STH_PODGENERATOR != (S_PODGENERATOR - S_FREETARGMOBJ + STH_FREETARGMOBJ ))
    I_Error( "DEH: state S_PODGENERATOR error" );
  if( STH_SND_WATERFALL != (S_SND_WATERFALL - S_BLOODYSKULL1 + STH_BLOODYSKULL1 ))
    I_Error( "DEH: state S_SND_WATERFALL error\n" );
#endif
}

#ifdef BEX_LANGUAGE
#include <fcntl.h>
#include <unistd.h>

// Load a language BEX file, by name (french, german, etc.)
void BEX_load_language( char * langname, byte bex_permission )
{
    int  handle;
    int  bytesread;
    struct stat  bufstat;
    char filename[MAX_WADPATH];
    myfile_t f;

    f.data = NULL;
    if( langname == NULL )
       langname = "lang";  // default name
    strncpy(filename, langname, MAX_WADPATH);
    filename[ MAX_WADPATH - 6 ] = '\0';
    strcat( filename, ".bex" );
    handle = open (filename,O_RDONLY|O_BINARY,0666);
    if( handle == -1)
    {
        CONS_Printf( "Lang file not found: %s\n", filename );
        goto errexit;
    }
    fstat(handle,&bufstat);
    f.size = bufstat.st_size;
    f.data = malloc( f.size );
    if( f.data == NULL )
    {
        CONS_Printf("Lang file too large for memory\n" );
        goto errexit;
    }
    bytesread = read (handle, f.data, f.size);
    if( bytesread < f.size )
    {
        CONS_Printf( "Lang file read failed\n" );
        goto errexit;
    }
    f.curpos = f.data;
    f.data[f.size] = 0;
    DEH_LoadDehackedFile(&f, bex_permission);

   errexit:
    free( f.data );
}
#endif
