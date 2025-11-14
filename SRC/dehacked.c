// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: dehacked.c 860 2011-09-17 17:50:44Z wesleyjohnson $
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


void DEH_LoadDehackedFile(MYFILE* f)
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
                   deh_error("Sound %d don't exist\n");
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
        //SoM: Support for Boom Extras (BEX)
/*        else if(!strcasecmp(word, "[STRINGS]"))
             {
             }
        else if(!strcasecmp(word, "[PARS]"))
             {
             }
        else if(!strcasecmp(word, "[CODEPTR]"))
             {
             }*/
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

    DEH_LoadDehackedFile(&f);
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
