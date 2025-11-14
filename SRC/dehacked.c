// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: dehacked.c 862 2011-09-17 17:56:28Z wesleyjohnson $
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
// very small commit
//
// Revision 1.14  2002/01/12 02:21:36  stroggonmeth
// Big commit
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
// no message
//
// Revision 1.9  2001/01/25 22:15:41  bpereira
// added heretic support
//
// Revision 1.8  2000/11/04 16:23:42  bpereira
// no message
//
// Revision 1.7  2000/11/03 13:15:13  hurdler
// Some debug comments, please verify this and change what is needed!
//
// Revision 1.6  2000/11/02 17:50:06  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.5  2000/08/31 14:30:55  bpereira
// no message
//
// Revision 1.4  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.3  2000/04/05 15:47:46  stroggonmeth
// Added hack for Dehacked lumps. Transparent sprites are now affected by colormaps.
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Load dehacked file and change table and text from the exe
//
//-----------------------------------------------------------------------------


#include "doomdef.h"

#include "command.h"
#include "console.h"

#include "g_game.h"

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

extern boolean infight; //DarkWolf95:November 21, 2003: Monsters Infight!

boolean deh_loaded = false;
byte  flags_valid_deh = false;  // flags altered flags (from DEH), boolean
byte  pars_valid_bex = false;  // have valid PAR values (from BEX), boolean

static boolean  bex_include_notext = 0;  // bex include with skip deh text

// Save compare values, to handle multiple DEH files and lumps
actionf_t  deh_actions[NUMSTATES];
char       *deh_sprnames[NUMSPRITES];
char       *deh_sfxnames[NUMSFX];
char	   *deh_musicname[NUMMUSIC];
char       *deh_text[NUMTEXT];



#define MAXLINELEN  200

// the code was first written for a file
// and converted to use memory with these functions
typedef struct {
    char *data;
    char *curpos;
    int size;
} MYFILE;

#define myfeof( a )  (a->data+a->size<=a->curpos)

// get string upto \n, or eof
char* myfgets(char *buf, int bufsize, MYFILE *f)
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
    //CONS_Printf("fgets [0]=%d [1]=%d '%s'\n",buf[0],buf[1],buf);

    return buf;
}

// get line, skipping comments
char* myfgets_nocom(char *buf, int bufsize, MYFILE *f)
{
    char* ret;
    do {
        ret = myfgets( buf, bufsize, f );
    } while( ret && ret[0] == '#' );   // skip comment
    return ret;
}


// Read multiple lines into buf
// Only used for text.
size_t  myfread( char *buf, size_t reqsize, MYFILE *f )
{
    size_t byteread = f->size - (f->curpos-f->data);  // bytes left
    if( reqsize < byteread )
        byteread = reqsize;
    if( byteread>0 )
    {
        ULONG i;
        // read lines except for any '\r'
	// But should only be taking the '\r' off end of line (/cr/lf)
        for(i=0; i<byteread; )
        {
            char c=*f->curpos++;
            if( c!='\r' )
                buf[i++]=c;
        }
    }
    return byteread;
}

static int deh_num_error=0;

static void deh_error(char *first, ...)
{
    va_list     argptr;

    if (devparm)
    {
#define BUF_SIZE 1024
       char buf[BUF_SIZE];

       va_start(argptr, first);
       vsnprintf(buf, BUF_SIZE, first, argptr);
       va_end(argptr);

       CONS_Printf("%s\n",buf);
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
	    || ( ch >= '0' && ch <= '9' ) ))
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
    // Mew freedoom.bex has 1%, that is not present in original string
    // Strings have %s, %s %s (old strings also had %c and %d).
    // Music and sound strings may have '-' and '\0'.
    unsigned char * newp = &newstring[0];
    unsigned char * oldp = &(*oldstring)[0];
    if( drstype == DRS_string )
    {
        for(;;)
        {
            // new string must have same or fewer %, so it must reach end first
            newp = strchr( newp, '%' );
            if( newp == NULL ) break;
            // must block %n, write to memory
	    // must block % not in same order as original
            if( oldp )
	    {
	        oldp = strchr( oldp, '%' );
	        if( oldp )
	        {
		    // looks like a format string
		    drstype = DRS_format;
		    if( oldp[1] != newp[1] )  goto bad_replacement;
		    oldp +=2;
		    newp +=2;
		}
	    }
	    else
	    {
	        // Found % in newstring that was not in oldstring
	        if( drstype == DRS_format )  goto bad_replacement;
	        // erase the %, too hard to determine if safe
		*(newp++) = 0x7F; // rubout the %
	    }
	}
    }
   
    // rewrite backslash literals into newstring, because it only gets shorter
    unsigned char * chp = &newstring[0];
    for( newp = &newstring[0]; *newp ; newp++ )
    {
        // Backslash in DEH and BEX strings are not interpreted by printf
        // Must convert \n to LF.
        register unsigned char ch = *newp;
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
	       v = strtoul(&newp[1], &endvp, 16);  // get hex
	       goto check_backslash_value;
	     default:
	       if( ch >= '1' && ch <= '9' )  // octal
	       {
		   // These do not get interpreted unless we do it here.
		   // Need this for foreign language ??
		   v = strtoul(newp, &endvp, 8);  // get octal
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
#ifdef FRENCH
        // place checks for allowed foreign lang chars here
	// reported dangerous escape chars
	if( ch == 133 ) goto bad_char;
        if( ch >= 254 )  goto bad_char;
//	    if( ch == 27 ) continue;  // ESCAPE
#else
        if( ch > 127 )  goto bad_char;
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
typedef enum { BF1, BF2, BF2x, BFmf=0x80 } bex_flags_ctrl_e;

typedef struct {
    char * name;
    byte   ctrl; // bex_flags_ctrl_e
    uint32_t flagval;
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
  {"NOCLIP",     BF1, MF_NOCLIP | MF_NOCLIPTHING }, // Does not clip against lines or Actors.
  // two slide bits, set them both
  {"SLIDE",      BF1|BFmf, MF_SLIDE }, // Player: keep info about sliding along walls.
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
  {"TRANSLATION1", BF1, (1<<MF_TRANSSHIFT) },  // Boom
  {"TRANSLATION2", BF1, (2<<MF_TRANSSHIFT) },  // Boom
  {"TRANSLATION3", BF1, (4<<MF_TRANSSHIFT) },
  {"TRANSLATION4", BF1, (8<<MF_TRANSSHIFT) },
  {"TRANSLATION",  BF1, (1<<MF_TRANSSHIFT) },  // Boom/prboom compatibility
  {"UNUSED1     ", BF1, (2<<MF_TRANSSHIFT) },  // Boom/prboom compatibility
  // Boom/BEX
  {"TRANSLUCENT", BF1, MF_TRANSLUCENT },  // Boom translucent
  // MBF/Prboom Extensions
//  {"TOUCHY",  BFC_x, MF_TOUCHY }, // (MBF) Reacts upon contact
  {"BOUNCES",  BF2, MF2_FLOORBOUNCE }, // (MBF) Bounces off walls, etc.
      //  heretic bounce is approximation
//  {"FRIEND",  BFC_x, MF_FRIEND }, // (MBF) Friend to player (dog, etc.)

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
  {NULL, 0, 0} // terminator
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

static void readthing(MYFILE *f, int deh_thing_id )
{
  // DEH thing 1.. , but mobjinfo array is 0..
  mobjinfo_t *  mip = & mobjinfo[ deh_thing_id - 1 ];
  char s[MAXLINELEN];
  char *word;
  int value;

  do{
    if(myfgets(s,sizeof(s),f)!=NULL)
    {
      if(s[0]=='\n') break;
      value=searchvalue(s);
      word=strtok(s," ");

      if(!strcasecmp(word,"Bits"))
      {
#ifdef CHECK_FLAGS2_DEFAULT
          boolean flags2_default = 1; // default for flags2
#endif
	  flag_name_t * fnp; // BEX flag names ptr
	  // total replacement of all flag bits
	  mip->flags = 0;
	  mip->flags2 = 0;
	  for(;;)
	  {
	      word = strtok(NULL, " +|\t=\n");
	      if( word == NULL ) break;
	      if( word[0] == '\n' ) break;
	      if( isdigit( word[0] ) )
	      {
		  value = atoi(word);  // numeric entry
		  if( value & MF_TRANSLUCENT )
		  {
		      // Boom bit defined in boomdeh.txt
		      // Was MF_FLOORHUGGER bit, and now need to determine which the PWAD means.
		      fprintf(stderr, "Sets flag MF_FLOORHUGGER or MF_TRANSLUCENT by numeric, guessing ");
		      if( value & (MF_NOBLOCKMAP|MF_MISSILE|MF_NOGRAVITY|MF_COUNTITEM))
		      {
			  // assume TRANSLUCENT, check for known exceptions
			  fprintf(stderr, "MF_TRANSLUCENT\n");
		      }
		      else
		      {
			  // assume FLOORHUGGER, check for known exceptions
			  value &= ~MF_TRANSLUCENT;
			  mip->flags2 |= MF2_FLOORHUGGER;
			  fprintf(stderr, "MF_FLOORHUGGER\n");
		      }
		  }
		  mip->flags |= value; // we are still using same flags bit order
		  flags_valid_deh = true;
		  continue;
	      }
	      // handle BEX flag names
#ifdef CHECK_FLAGS2_DEFAULT
	      flags2_default = 1;
#endif
	      for( fnp = &BEX_flag_name_table[0]; fnp; fnp++ )
	      {
		  if(!strcasecmp( word, fnp->name ))  // find name
		  {
		      switch( fnp->ctrl & ~BFmf)
		      {
		       case BF1:
		         mip->flags |= value;
			 break;
		       case BF2x: // DoomLegacy extension BEX name
#ifdef CHECK_FLAGS2_DEFAULT
			 flags2_default = 0;
#endif
		       case BF2: // standard name that happens to be MF2
		         mip->flags2 |= value;
			 break;
		      }
		      flags_valid_deh = true;
		      // unless multiple flag set for a keyword
		      if( ! fnp->ctrl & BFmf )
		         continue; // next word
		  }
	      }
	      deh_error("Bits name unknown: %s\n", word);
	  }
#ifdef CHECK_FLAGS2_DEFAULT
	  // Unless explicitly used BF2x bit, then put in default bits
	  // Avoid by using MF2CLEAR
	  if( flags2_default )
	  {
	      mip->flags2 |= flags2_default_value;
	  }
#endif	 
	  continue; // next line
      }

      // set the value in apropriet field
      else if(!strcasecmp(word,"ID"))           mip->doomednum   =value;
      else if(!strcasecmp(word,"Initial"))      mip->spawnstate  =value;
      else if(!strcasecmp(word,"Hit"))          mip->spawnhealth =value;
      else if(!strcasecmp(word,"First"))        mip->seestate    =value;
      else if(!strcasecmp(word,"Alert"))        mip->seesound    =value;
      else if(!strcasecmp(word,"Reaction"))     mip->reactiontime=value;
      else if(!strcasecmp(word,"Attack"))       mip->attacksound =value;
      else if(!strcasecmp(word,"Injury"))       mip->painstate   =value;
      else if(!strcasecmp(word,"Pain"))
           {
             word=strtok(NULL," ");
             if(!strcasecmp(word,"chance"))     mip->painchance  =value;
             else if(!strcasecmp(word,"sound")) mip->painsound   =value;
           }
      else if(!strcasecmp(word,"Close"))        mip->meleestate  =value;
      else if(!strcasecmp(word,"Far"))          mip->missilestate=value;
      else if(!strcasecmp(word,"Death"))
           {
             word=strtok(NULL," ");
             if(!strcasecmp(word,"frame"))      mip->deathstate  =value;
             else if(!strcasecmp(word,"sound")) mip->deathsound  =value;
           }
      else if(!strcasecmp(word,"Exploding"))    mip->xdeathstate =value;
      else if(!strcasecmp(word,"Speed"))        mip->speed       =value;
      else if(!strcasecmp(word,"Width"))        mip->radius      =value;
      else if(!strcasecmp(word,"Height"))       mip->height      =value;
      else if(!strcasecmp(word,"Mass"))         mip->mass        =value;
      else if(!strcasecmp(word,"Missile"))      mip->damage      =value;
      else if(!strcasecmp(word,"Action"))       mip->activesound =value;
      else if(!strcasecmp(word,"Bits2"))        mip->flags2      =value;
      else if(!strcasecmp(word,"Respawn"))      mip->raisestate  =value;
      else deh_error("Thing %d : unknown word '%s'\n", deh_thing_id,word);
    }
  } while(s[0]!='\n' && !myfeof(f)); //finish when the line is empty
}

/*
Sprite number = 10
Sprite subnumber = 32968
Duration = 200
Next frame = 200
*/
static void readframe(MYFILE* f, int deh_frame_id)
{
  state_t *  fsp = & states[ deh_frame_id ];
  char s[MAXLINELEN];
  char *word1,*word2;
  int value;

  do{
    if(myfgets(s,sizeof(s),f)!=NULL)
    {
      if(s[0]=='\n') break;
      value=searchvalue(s);
      // set the value in apropriet field
      word1=strtok(s," ");
      word2=strtok(NULL," ");

      if(!strcasecmp(word1,"Sprite"))
      {
             if(!strcasecmp(word2,"number"))     fsp->sprite   =value;
        else if(!strcasecmp(word2,"subnumber"))  fsp->frame    =value;
      }
      else if(!strcasecmp(word1,"Duration"))     fsp->tics     =value;
      else if(!strcasecmp(word1,"Next"))         fsp->nextstate=value;
      else deh_error("Frame %d : unknown word '%s'\n", deh_frame_id,word1);
    }
  } while(s[0]!='\n' && !myfeof(f));
}

static void readsound(MYFILE* f, int deh_sound_id)
{
  sfxinfo_t *  ssp = & S_sfx[ deh_sound_id ];
  char s[MAXLINELEN];
  char *word;
  int value;

  do{
    if(myfgets(s,sizeof(s),f)!=NULL)
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
      else if(!strcasecmp(word,"Zero/One")) ssp->singularity=value;
      else if(!strcasecmp(word,"Value"))    ssp->priority   =value;
      else deh_error("Sound %d : unknown word '%s'\n", deh_sound_id,word);
    }
  } while(s[0]!='\n' && !myfeof(f));
}

static void readtext(MYFILE* f, int len1, int len2 )
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

    str2 = &s[len1];
    s[len1+len2]='\0';
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
      char str1[6];
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
static void bex_strings( MYFILE* f, byte bex_permission )
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
	  break;
      // get continuation line to sb, skipping comments.
      // [WDJ] questionable, but boom202 code skips comments between continuation lines.
      if( ! myfgets_nocom(sb, sizeof(sb), f) )
	  break; // no more lines
      cp = &sb[0];
    } while ( *cp );
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
	        if( filename_reject( stxt, 10 )  goto no_text_change;
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
static void bex_pars( MYFILE* f )
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
    if( strcasecmp( s, "par" ) != 0 )  break;  // not a par line
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

   { NULL, {NULL} }  // table term
};



// BEX [CODEPTR] section
static void bex_codeptr( MYFILE* f )
{
  char funcname[BEX_KEYW_LEN];
  char s[MAXLINELEN];
  int  framenum, nn, i;
   
  // format:
  // [CODEPTR]
  // FRAME <framenum> = <funcname>

  for(;;) {
    if( ! myfgets_nocom(s, sizeof(s), f) )
       break; // no more lines
    if( s[0] == '\n' ) continue;  // blank line
    if( s[0] == 0 ) break;
    if( strcasecmp( s, "FRAME" ) != 0 )  break;  // not a FRAME line
    nn = sscanf( &s[5], "%d = %s", &framenum, funcname );
    if( nn != 2 )
    {
	deh_error( "Bad FRAME syntax\n" );
        continue;
    }
    if( framenum < 0 || framenum > NUMSTATES )
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
	    states[framenum].action.acv = bex_action_table[i].action.acv;
	    goto next_keyw;
	}
    }
  no_action_change:
    deh_error("Action not changed : FRAME %d\n", framenum);
     
  next_keyw:
    continue;
  }
}

// include another DEH or BEX file
void bex_include( char * inclfilename )
{
  static boolean include_nested = 0;
  
  // MYFILE is local to DEH_LoadDehackedLump
  
  if( include_nested )
  {
    deh_error( "BEX INCLUDE, only one level allowed\n" );
    return;
  }
  // save state
  
  include_nested = 1;
//  DEH_LoadDehackedFile( inclfile );  // do the include file
  W_LoadWadFile (inclfilename);
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
static void readweapon(MYFILE *f, int deh_weapon_id)
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
      else if(!strcasecmp(word,"Deselect"))   wip->upstate   =value;
      else if(!strcasecmp(word,"Select"))     wip->downstate =value;
      else if(!strcasecmp(word,"Bobbing"))    wip->readystate=value;
      else if(!strcasecmp(word,"Shooting"))   wip->atkstate  = wip->holdatkstate = value;
      else if(!strcasecmp(word,"Firing"))     wip->flashstate=value;
      else deh_error("Weapon %d : unknown word '%s'\n", deh_weapon_id,word);
    }
  } while(s[0]!='\n' && !myfeof(f));
}

/*
Max ammo = 400
Per ammo = 40
*/

extern int clipammo[];
extern int GetWeaponAmmo[];

static void readammo(MYFILE *f,int num)
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
      else if(!strcasecmp(word,"Per")) { clipammo[num]=value;GetWeaponAmmo[num] = 2*value; }
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


static void readmisc(MYFILE *f)
{
  char s[MAXLINELEN];
  char *word,*word2;
  int value;
  do{
    if(myfgets(s,sizeof(s),f)!=NULL)
    {
      if(s[0]=='\n') break;
      value=searchvalue(s);
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
      else if(!strcasecmp(word,"BFG"))            doomweaponinfo[wp_bfg].ammopershoot = value;
      else if(!strcasecmp(word,"Monsters"))       infight = true; //DarkWolf95:November 21, 2003: Monsters Infight!
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
static void readcheat(MYFILE *f)
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
void DEH_LoadDehackedFile(MYFILE* f, byte bex_permission)
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
               if(i<NUMSTATES && i>=0)
                  readframe(f,i);
               else
                  deh_error("Frame %d don't exist\n",i);
             }
        else if(!strcasecmp(word,"Pointer"))
             {
	       // "Pointer <num>"
               word=strtok(NULL," "); // get frame
               if((word=strtok(NULL,")"))!=NULL)
               {
                 i=atoi(word);
                 if(i<NUMSTATES && i>=0)
                 {
                   if(myfgets(s,sizeof(s),f)!=NULL)
                     states[i].action=deh_actions[searchvalue(s)];
                 }
                 else
                    deh_error("Pointer : Frame %d don't exist\n",i);
               }
               else
                   deh_error("pointer (Frame %d) : missing ')'\n",i);
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
void DEH_LoadDehackedLump(int lump)
{
    MYFILE f;
    
    f.size = W_LumpLength(lump);
    f.data = Z_Malloc(f.size + 1, PU_IN_USE, 0);  // temp
    W_ReadLump(lump, f.data);
    f.curpos = f.data;
    f.data[f.size] = 0;

    DEH_LoadDehackedFile(&f, 0);
    Z_Free(f.data);
}


// [WDJ] Before any changes, save all comparison info, so that multiple
// DEH files and lumps can be handled without interfering with each other.
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
}
