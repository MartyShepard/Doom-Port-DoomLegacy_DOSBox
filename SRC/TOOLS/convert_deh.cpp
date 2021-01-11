// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: convert_deh.cpp 559 2009-11-15 17:01:03Z smite-meister $
//
// Copyright (C) 1998-2009 by DooM Legacy Team.
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
//-----------------------------------------------------------------------------

/// \file
/// \brief DeHackEd distiller and converter from Legacy 1.43 to Legacy 2.0.

#include <stdarg.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "mnemonics.h"
#include "parser.h"

#include "g_actor.h"
#include "info.h"
#include "dstrings.h"


class dehacked_t
{
private:
  Parser p;
  int  num_errors;

  int  FindValue();
  int  FindState();
  bool ReadFlags(struct mobjinfo_t *m);

  void Read_Thing(const char *str);
  void Read_Frame(const char *str);
  void Read_Sound(int num);
  void Read_Text(int len1, int len2);
  void Read_Weapon(int num);
  void Read_Ammo(int num);
  void Read_Misc();
  void Read_Cheat();
  void Read_CODEPTR();
  void Read_STRINGS();

public:
  bool loaded;

  dehacked_t();
  bool LoadDehackedLump(const char *buf, int len);
  void error(const char *first, ...);

  int   idfa_armor;
  float idfa_armorfactor;
  int   idkfa_armor;
  float idkfa_armorfactor;
  int   god_health;

  int max_health;
  int max_soul_health;

  int initial_bullets;
};




//===========================================
// Partial Parser class implementation
//===========================================

Parser::Parser()
{
  length = 0;
  ms = me = s = e = NULL;
}


Parser::~Parser()
{
  if (ms)
    free(ms);
}



// prepares a buffer for parsing
int Parser::Open(const char *buf, int len)
{
  if (len <= 0)
    return 0;

  length = len;

  ms = (char *)malloc(length + 1);
  memcpy(ms, buf, length);
  ms[length] = '\0'; // to make searching easy

  me = ms + length; // past-the-end pointer
  s = e = ms;

  return length;
}


// Removes all chars 'c' from the buffer and compactifies it.
void Parser::DeleteChars(char c)
{
  char *q = ms;
  for (char *p = ms; p < me; p++)
    if (*p != c)
      *q++ = *p;

  // and then fix the new limits
  me = q;
  length = me - ms;
  ms[length] = '\0'; // to make searching easy
}


// Clears the parser
void Parser::Clear()
{
  if (ms)
    free(ms);

  length = 0;
  ms = me = s = e = NULL;
}


// Replace all comments after 's' with whitespace.
// anything between the symbol // or ; and the next newline is a comment.
// TODO right now there is no way to escape these symbols!
void Parser::RemoveComments(char c, bool linestart)
{
  if (linestart)
    {
      // only interpret it as a comment if it is in the beginning of a line
      // (for DeHackEd and the stupid ID # thing!)
      for (char *p = s; p < me; p++)
	{
	  if (p[0] == '\n' && p[1] == c)
	    {
	      for (p++ ; p < me && *p != '\n'; p++)
		*p = ' ';
	      p--;
	    }
	}   
      return;
    }

  if (c == '/')
    for (char *p = s; p+1 < me; p++)
      {
	if (p[0] == '/' && p[1] == '/')
	  for ( ; p < me && *p != '\n'; p++)
	    *p = ' ';
      }
  else
    for (char *p = s; p < me; p++)
      {
	if (p[0] == c)
	  for ( ; p < me && *p != '\n'; p++)
	    *p = ' ';
      }    
}


// Reads at most n chars starting from the next line. Updates e.
// Returns the number of chars actually read.
int Parser::ReadChars(char *to, int n)
{
  int i;
  for (i = 0; e < me && i < n; e++, i++)
    to[i] = *e;

  to[i] = '\0';
  return i; 
}


// NOTE you must use this before using the line-oriented Parser methods.
// Seeks the next row ending with a newline.
// Returns false if the lump ends.
bool Parser::NewLine(bool pass_ws)
{
  // end passes any whitespace
  if (pass_ws)
    while (e < me && isspace(*e))
      e++;

  s = e; // this is where the next line starts

  // seek the next newline
  while (e < me && (*e != '\n'))
    e++;

  if (e < me)
    {
      *e = '\0';  // mark the line end
      e++; // past-the-end
      return true;
    }

  return false; // lump ends
}


// passes any contiguous whitespace
void Parser::PassWS()
{
  while (s < me && isspace(*s))
    s++;
}



// Tokenizer. Advances s.
char *Parser::GetToken(const char *delim)
{
  // Damnation! If strtok_r() only was part of ISO C!
  //return strtok_r(s, delim, &s);

  // pass initial delimiters
  int n = strlen(delim);
  for (; s < me && *s; s++)
    {
      int i;
      for (i=0; i<n; i++)
	if (*s == delim[i])
	  break;
      if (i >= n)
	break;
    }

  if (s == me || *s == '\0')
    return NULL;

  char *temp = strtok(s, delim);
  s += strlen(temp) + 1; // because strtok marks the token end with a NUL
  if (s >= e)
    s = e-1; // do not leave the line
  return temp;
}


// Gets one char, ignoring starting whitespace.
bool Parser::GetChar(char *to)
{
  PassWS();

  if (*s)
    {
      *to = *s;
      s++;
      return true;
    }

  return false;
}


// Get an integer, advance the 's' pointer
int Parser::GetInt()
{
  char *tail = NULL;
  int val = strtol(s, &tail, 0);
  if (tail == s)
    {
      printf("Expected an integer, got '%s'.\n", s);
      return 0;
    }

  s = tail;
  return val;
}


bool Parser::MustGetInt(int *to)
{
  char *tail = NULL;
  int val = strtol(s, &tail, 0);
  if (tail == s)
    return false;

  s = tail;
  *to = val;
  return true;
}



// Tries to match a string 'p' to a NULL-terminated array of strings.
// Returns the index of the first matching string, or -1 if there is no match.
int P_MatchString(const char *p, const char *strings[])
{
  for (int i=0; strings[i]; i++)
    if (!strcasecmp(p, strings[i]))
      return i;

  return -1;
}



//===========================================
//   The DeHackEd part
//===========================================


dehacked_t DEH; // one global instance

FILE *out = NULL;

enum
{
  NUM_DOOM_STATES = 967, // all states in Doom (with null)
  NUM_LEGACY_STATES = 9,
  s_gap1 = 563, // hblood * 3
  s_gap2 = 570, // s_play * 47
  NUM_HERETIC_STATES = 1205, // with null

  NUM_DOOM_THINGS = 137,
  NUM_LEGACY_THINGS = 11,
  t_gap1 = 55, // tfog, teleportman * 2
  t_gap2 = 94, // hblood * 1
  t_gap3 = 96, // hplayer * 1
  NUM_HERETIC_THINGS = 160,

  NUM_DOOM_WSTATES = 87, // weapon states in Doom
};


bool ExpandThingNum(int num)
{
  int t = num-1; // begin at 0

  // t is zero-based
  if (t < NUM_DOOM_THINGS)
    fprintf(out, "Thing %d", t+1);
  else if ((t -= NUM_DOOM_THINGS) < NUM_LEGACY_THINGS)
    fprintf(out, "Thing L%d", t+1);
  else if ((t -= NUM_LEGACY_THINGS) < t_gap1)
    fprintf(out, "Thing H%d", t+1);
  else if ((t += 2) < t_gap2)
    fprintf(out, "Thing H%d", t+1);
  else if ((t += 1) < t_gap3)
    fprintf(out, "Thing H%d", t+1);
  else if ((t += 1) < NUM_HERETIC_THINGS)
    fprintf(out, "Thing H%d", t+1);
  else
    {
      DEH.error("Thing %d doesn't exist!\n", num);
      fprintf(out, "Thing -1");
      return false;
    }

  return true;
}


const char *ExpandStateNum(int s)
{
  static char text[100];

  // s is zero-based
  if (s < NUM_DOOM_STATES)
    sprintf(text, "%d", s);
  else if ((s -= NUM_DOOM_STATES) < NUM_LEGACY_STATES)
    sprintf(text, "L%d", s);
  else if ((s -= NUM_LEGACY_STATES - 1) < s_gap1) // HNULL
    sprintf(text, "H%d", s);
  else if ((s += 3) < s_gap2) // HBLOOD
    sprintf(text, "H%d", s);
  else if ((s += 47) < NUM_HERETIC_STATES)// HPLAY
    sprintf(text, "H%d", s);
  else
    {
      DEH.error("State %d doesn't exist!\n", s);
      return "-1";
    }

  return text;
}



//========================================================================
//  The DeHackEd class
//========================================================================

dehacked_t::dehacked_t()
{
  loaded = false;
  num_errors = 0;

  idfa_armor = 200;
  idfa_armorfactor = 0.5;
  idkfa_armor = 200;
  idkfa_armorfactor = 0.5;
  god_health = 100;

  // TODO these are not yet used...
  max_health = 200;
  max_soul_health = 200;
}


void dehacked_t::error(const char *first, ...)
{
  va_list argptr;

  char buf[1000];

  va_start(argptr, first);
  vsprintf(buf, first, argptr);
  va_end(argptr);

  printf("DEH: %s", buf);
  num_errors++;
}


// A small hack for retrieving values following a '=' sign.
// Does not change the parser state.
int dehacked_t::FindValue()
{
  char *temp = p.Pointer(); // save the current location

  // find the first occurrence of str
  char *res = strstr(temp, "=");
  if (res)
    p.SetPointer(++res); // pass the = sign
  else
    {
      error("Missing equality sign!\n");
      return 0;
    }

  int value;
  if (!p.MustGetInt(&value))
    {
      error("No value found\n");
      value = 0;
    }

  p.SetPointer(temp); // and go back
  return value;
}



bool dehacked_t::ReadFlags(mobjinfo_t *m)
{
  int v1 = 0, v2 = 0;
  char *word = p.GetToken("=+| \t");
  flag_mnemonic_t *b;

  // we allow bitwise-ORed combinations of BEX mnemonics and numeric values
  while (word)
    {
      if (isdigit(word[0]))
	{
	  // old-style numeric entry, just do the conversion
	  int temp = atoi(word);
	  for (int i=0; i<26; i++)
	    if (temp & OriginalFlags[i].old_flag)
	      {
		int f = OriginalFlags[i].new_flag;
		if (f == 0)
		  error("NOTE: Flag %d has no in-game effect.\n", OriginalFlags[i].old_flag);
		v1 |= f;
	      }

	  word = p.GetToken("+| \t"); // next token
	  continue;
	}
      
      // must be a mnemonic
      for (b = BEX_FlagMnemonics; b->name; b++)
	if (!strcasecmp(word, b->name))
	  {
	    switch (b->flagword)
	      {
	      case 1:
		v1 |= b->flag;
		break;

	      case 2:
	      default:
		v2 |= b->flag;
		break;
	      }

	    break;
	  }

      if (!b->name)
	{
	  error("Unknown bit mnemonic '%s'\n", word);
	}
		
      word = p.GetToken("+| \t"); // next token
    }


  if (v1 == 0 && v2 == 0)
    fputs("0", out);
  else for (b = BEX_FlagMnemonics; b->name; b++)
    if (((b->flagword == 1) ? v1 : v2) & b->flag)
      fprintf(out, "%s ", b->name);

  fputs("\n", out);

  return true;
}



//========================================================================
// Load a dehacked file format 6. I (BP) don't know other format
//========================================================================

/*
  Thing sample:
Thing 1 (Player)
ID # = 3232              -1,             // doomednum
Initial frame = 32       S_PLAY,         // spawnstate
Hit points = 3232        100,            // spawnhealth
First moving frame = 32  S_PLAY_RUN1,    // seestate
Alert sound = 32         sfx_None,       // seesound
Reaction time = 3232     0,              // reactiontime
Attack sound = 32        sfx_None,       // attacksound
Injury frame = 32        S_PLAY_PAIN,    // painstate
Pain chance = 3232       255,            // painchance
Pain sound = 32          sfx_plpain,     // painsound
Close attack frame = 32  S_NULL,         // meleestate
Far attack frame = 32    S_PLAY_ATK1,    // missilestate
Death frame = 32         S_PLAY_DIE1,    // deathstate
Exploding frame = 32     S_PLAY_XDIE1,   // xdeathstate
Death sound = 32         sfx_pldeth,     // deathsound
Speed = 3232             0,              // speed
Width = 211812352        16*FRACUNIT,    // radius
Height = 211812352       56*FRACUNIT,    // height
Mass = 3232              100,            // mass
Missile damage = 3232    0,              // damage
Action sound = 32        sfx_None,       // activesound
Bits = 3232              MF_SOLID|MF_SHOOTABLE|MF_DROPOFF|MF_PICKUP|MF_NOTDMATCH,
Respawn frame = 32       S_NULL          // raisestate
*/

void dehacked_t::Read_Thing(const char *str)
{
  int num = atoi(str);

  ExpandThingNum(num);
  // preserve the comment
  fprintf(out, " %s\n", p.Pointer());

  while (p.NewLine(false))
    {
      p.PassWS();
      if (!p.LineLen())
	break; // a whitespace-only line ends the record

      char *word = p.GetToken(" "); // get first word

      // special handling for mnemonics
      if (!strcasecmp(word, "Bits"))
	{
	  fprintf(out, "Bits = ");
	  ReadFlags(NULL);
	  continue;
	}

      int value = FindValue();

      // set the value in appropriate field
      if (!strcasecmp(word, "ID"))            fprintf(out, "ID # = %d\n", value);
      else if (!strcasecmp(word,"Hit"))       fprintf(out, "Hit points = %d\n", value);
      else if (!strcasecmp(word,"Alert"))     fprintf(out, "Alert sound = %d\n", value);
      else if (!strcasecmp(word,"Reaction"))  fprintf(out, "Reaction time = %d\n", value);
      else if (!strcasecmp(word,"Attack"))    fprintf(out, "Attack sound = %d\n", value);
      else if (!strcasecmp(word,"Pain"))
	{
	  word = p.GetToken(" ");
	  if (!strcasecmp(word,"chance"))     fprintf(out, "Pain chance = %d\n", value);
	  else if (!strcasecmp(word,"sound")) fprintf(out, "Pain sound = %d\n", value);
	}
      else if (!strcasecmp(word,"Death"))
	{
	  word = p.GetToken(" ");
	  if (!strcasecmp(word,"frame"))
	    {
	      fprintf(out, "Death frame = %s\n", ExpandStateNum(value));
	    }
	  else if (!strcasecmp(word,"sound")) fprintf(out, "Death sound = %d\n", value);
	}
      else if (!strcasecmp(word,"Speed"))     fprintf(out, "Speed = %d\n", value);
      else if (!strcasecmp(word,"Width"))     fprintf(out, "Width = %d\n", value);
      else if (!strcasecmp(word,"Height"))    fprintf(out, "Height = %d\n", value);
      else if (!strcasecmp(word,"Mass"))      fprintf(out, "Mass = %d\n", value);
      else if (!strcasecmp(word,"Missile"))   fprintf(out, "Missile damage = %d\n", value);
      else if (!strcasecmp(word,"Action"))    fprintf(out, "Action sound = %d\n", value);
      else
	{
	  const char *s = ExpandStateNum(value);

	  if (!strcasecmp(word,"Initial"))        fprintf(out, "Initial frame = %s\n", s);
	  else if (!strcasecmp(word,"First"))     fprintf(out, "First moving frame = %s\n", s);
	  else if (!strcasecmp(word,"Injury"))    fprintf(out, "Injury frame = %s\n", s);
	  else if (!strcasecmp(word,"Close"))     fprintf(out, "Close attack frame = %s\n", s);
	  else if (!strcasecmp(word,"Far"))       fprintf(out, "Far attack frame = %s\n", s);
	  else if (!strcasecmp(word,"Exploding")) fprintf(out, "Exploding frame = %s\n", s);
	  else if (!strcasecmp(word,"Respawn"))   fprintf(out, "Respawn frame = %s\n", s);
	  else error("Thing %d : Unknown field '%s'\n", num, word);
	}
    }

  fputs("\n", out);
}


/*
  Frame sample:
Sprite number = 10
Sprite subnumber = 32968
Duration = 200
Next frame = 200
Codep = 111 // Legacy addition
*/

void dehacked_t::Read_Frame(const char *str)
{
  int num = atoi(str);

  fprintf(out, "Frame %s %s\n", ExpandStateNum(num), p.Pointer());

  while (p.NewLine(false))
    {
      p.PassWS();
      if (!p.LineLen())
	break; // a whitespace-only line ends the record

      int value = FindValue();

      // set the value in appropriate field
      char *word = p.GetToken(" ");

	  if (!strcasecmp(word,"Sprite"))
	    {
	      word = p.GetToken(" ");
	      if (!strcasecmp(word,"number"))         fprintf(out, "Sprite number = %d\n", value);
	      else if (!strcasecmp(word,"subnumber")) fprintf(out, "Sprite subnumber = %d\n", value);
	    }
	  else if (!strcasecmp(word,"Duration"))      fprintf(out, "Duration = %d\n", value);
	  else if (!strcasecmp(word,"Next"))
	    {
	      fprintf(out, "Next frame = %s\n", ExpandStateNum(value));
	    }
	  else if (!strcasecmp(word,"Codep"))
	    {
	      fprintf(out, "Codep = %s\n", ExpandStateNum(value));
	    }
	  else error("Frame %d : Unknown field '%s'\n", num, word);
    }

  fputs("\n", out);
}


// deprecated
void dehacked_t::Read_Sound(int num)
{
  error("Sound command currently unsupported.\n");
  return;
}


// this part of dehacked really sucks, but it still partly supported
void dehacked_t::Read_Text(int len1, int len2)
{
  char s[2001];

  // FIXME dehacked text
  // it is hard to change all the text in doom
  // here i implement only vital things
  // yes, "text" can change some tables like music, sound and sprite names
  if (len1+len2 > 2000)
    {
      error("Text too long\n");
      return;
    }

  if (p.ReadChars(s, len1 + len2) != len1 + len2)
    {
      error("Read failed\n");
      return;
    }

  fprintf(out, "Text %d %d\n%s\n\n", len1, len2, s);
}


/*
  Weapon sample:
Ammo type = 2
TODO ammopershoot
Deselect frame = 11
Select frame = 12
Bobbing frame = 13
Shooting frame = 17
Firing frame = 10
*/
void dehacked_t::Read_Weapon(int num)
{
  fprintf(out, "Weapon %d %s\n", num, p.Pointer());

  while (p.NewLine(false))
    {
      p.PassWS();
      if (!p.LineLen())
	break; // a whitespace-only line ends the record

      int value = FindValue();
      char *word = p.GetToken(" ");

      if (!strcasecmp(word,"Ammo"))
	fprintf(out, "Ammo type = %d\n", value);
      else
	{
	  const char *p = ExpandStateNum(value);

	  if (!strcasecmp(word,"Deselect"))      fprintf(out, "Deselect frame = %s\n", p);
	  else if (!strcasecmp(word,"Select"))   fprintf(out, "Select frame = %s\n", p);
	  else if (!strcasecmp(word,"Bobbing"))  fprintf(out, "Bobbing frame = %s\n", p);
	  else if (!strcasecmp(word,"Shooting")) fprintf(out, "Shooting frame = %s\n", p);
	  else if (!strcasecmp(word,"Firing"))   fprintf(out, "Firing frame = %s\n", p);
	  else error("Weapon %d : unknown word '%s'\n", num, word);
	}
    }
  fputs("\n", out);
}


/*
  Ammo sample:
Max ammo = 400
Per ammo = 40
*/
void dehacked_t::Read_Ammo(int num)
{
  fprintf(out, "Ammo %d %s\n", num, p.Pointer());

  while (p.NewLine(false))
    {
      p.PassWS();
      if (!p.LineLen())
	break; // a whitespace-only line ends the record

      int value = FindValue();
      char *word = p.GetToken(" ");

      if (!strcasecmp(word,"Max"))
	fprintf(out, "Max ammo = %d\n", value);
      else if (!strcasecmp(word,"Per"))
	fprintf(out, "Per ammo = %d\n", value);
      /*
      else if (!strcasecmp(word,"Perweapon"))
	{
	  mobjinfo[clips[2][num]].spawnhealth = value;
	  if (clips[3][num] > MT_NONE)
	    mobjinfo[clips[3][num]].spawnhealth = value;
	}
      */
      else
	error("Ammo %d : unknown word '%s'\n", num, word);
    }
  fputs("\n", out);
}


// miscellaneous one-liners
void dehacked_t::Read_Misc()
{
  error("Misc command currently unsupported.\n");
  return;

  /*
  extern int MaxArmor[];
  const double ac = 1.0/6;

  char *word1, *word2;
  int value;

  while (p.NewLine(false))
    {
      p.PassWS();
      if (!p.LineLen())
	break; // a whitespace-only line ends the record

      value = FindValue();
      word1 = p.GetToken(" ");
      word2 = p.GetToken(" ");

      if (!strcasecmp(word1,"Initial"))
	{
	  if (!strcasecmp(word2,"Health"))       mobjinfo[MT_PLAYER].spawnhealth = value;
	  else if (!strcasecmp(word2,"Bullets")) pawndata[0].bammo = value;
	}
      else if (!strcasecmp(word1,"Max"))
	{
	  if (!strcasecmp(word2,"Health"))          max_health = value;
	  else if (!strcasecmp(word2,"Armor"))      MaxArmor[0] = value;
	  else if (!strcasecmp(word2,"Soulsphere")) max_soul_health = value;
	}
      else if (!strcasecmp(word1,"Green"))
	{
	  mobjinfo[MT_GREENARMOR].spawnhealth = value*100;
	  mobjinfo[MT_GREENARMOR].speed = ac*(value+1);
	}
      else if (!strcasecmp(word1,"Blue"))
	{
	  mobjinfo[MT_BLUEARMOR].spawnhealth = value*100;
	  mobjinfo[MT_BLUEARMOR].speed = ac*(value+1);
	}
      else if (!strcasecmp(word1,"Soulsphere")) mobjinfo[MT_SOULSPHERE].spawnhealth = value;
      else if (!strcasecmp(word1,"Megasphere")) mobjinfo[MT_MEGA].spawnhealth = value;
      else if (!strcasecmp(word1,"God"))        god_health = value;
      else if (!strcasecmp(word1,"IDFA"))
	{
	  word2 = p.GetToken(" ");
	  if (!strcasecmp(word2,"="))          idfa_armor = value;
	  else if (!strcasecmp(word2,"Class")) idfa_armorfactor = ac*(value+1);
	}
      else if (!strcasecmp(word1,"IDKFA"))
	{
	  word2 = p.GetToken(" ");
	  if (!strcasecmp(word2,"="))          idkfa_armor = value;
	  else if (!strcasecmp(word2,"Class")) idkfa_armorfactor = ac*(value+1);
	}
      else if (!strcasecmp(word1,"BFG")) wpnlev1info[wp_bfg].ammopershoot = value;
      else if (!strcasecmp(word1,"Monsters"))      {} // TODO
      else error("Misc : unknown command '%s'\n", word1);
    }
  */
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

/*
static void change_cheat_code(byte *old, byte *n)
{
  for ( ; *n && *n != 0xff; old++, n++)
    if (*old == 1 || *old == 0xff) // no more place in the cheat
      {
	DEH.error("Cheat too long\n");
	return;
      }
    else
      *old = *n;

  // newcheatseq < oldcheat
  n = old;
  // search special cheat with 100
  for ( ; *n != 0xff; n++)
    if (*n == 1)
      {
	*old++ = 1;
	*old++ = 0;
	*old++ = 0;
	break;
      }
  *old = 0xff;
}
*/


// deprecated
void dehacked_t::Read_Cheat()
{
  error("Cheat command currently unsupported.\n");
  return;

  /*
  while (p.NewLine(false))
    {
      p.PassWS();
      if (!p.LineLen())
	break; // a whitespace-only line ends the record

      // FIXME how does this work?
      p.GetToken("=");
      byte *value = (byte *)p.GetToken(" \n"); // skip the space
      p.GetToken(" \n");         // finish the string
      char *word = p.GetToken(" ");

      if (!strcasecmp(word,"Change"))        change_cheat_code(cheat_mus_seq,value);
      else if (!strcasecmp(word,"Chainsaw")) change_cheat_code(cheat_choppers_seq,value);
      else if (!strcasecmp(word,"God"))      change_cheat_code(cheat_god_seq,value);
      else if (!strcasecmp(word,"Ammo"))
	{
	  word = p.GetToken(" ");
	  if (word && !strcasecmp(word,"&")) change_cheat_code(cheat_ammo_seq,value);
	  else                           change_cheat_code(cheat_ammonokey_seq,value);
	}
      else if (!strcasecmp(word,"No"))
	{
	  word = p.GetToken(" ");
	  if (word)
	    word = p.GetToken(" ");

	  if (word && !strcasecmp(word,"1")) change_cheat_code(cheat_noclip_seq,value);
	  else                           change_cheat_code(cheat_commercial_noclip_seq,value);
	}
       // FIXME! this could be replaced with a possibility to create new cheat codes (easy)
      else if (!strcasecmp(word,"Invincibility")) change_cheat_code(cheat_powerup_seq[0],value);
      else if (!strcasecmp(word,"Berserk"))       change_cheat_code(cheat_powerup_seq[1],value);
      else if (!strcasecmp(word,"Invisibility"))  change_cheat_code(cheat_powerup_seq[2],value);
      else if (!strcasecmp(word,"Radiation"))     change_cheat_code(cheat_powerup_seq[3],value);
      else if (!strcasecmp(word,"Auto-map"))      change_cheat_code(cheat_powerup_seq[4],value);
      else if (!strcasecmp(word,"Lite-Amp"))      change_cheat_code(cheat_powerup_seq[5],value);
      else if (!strcasecmp(word,"BEHOLD"))        change_cheat_code(cheat_powerup_seq[6],value);

      else if (!strcasecmp(word,"Level"))         change_cheat_code(cheat_clev_seq,value);
      else if (!strcasecmp(word,"Player"))        change_cheat_code(cheat_mypos_seq,value);
      else if (!strcasecmp(word,"Map"))           change_cheat_code(cheat_amap_seq,value);
      else error("Cheat : unknown word '%s'\n",word);
    }
  */
}


// Parses the BEX [CODEPTR] section
void dehacked_t::Read_CODEPTR()
{
  fprintf(out, "[CODEPTR]\n");

  while (p.NewLine(false))
    {
      p.PassWS();
      if (!p.LineLen())
	break; // a whitespace-only line ends the record

      char *word = p.GetToken(" ");
      if (strcasecmp(word, "Frame"))
	{
	  error("[CODEPTR]: Unknown command '%s'\n", word);
	  continue;
	}

      int s = p.GetInt();
      word = p.GetToken(" =");
     
      fprintf(out, "Frame %s = %s\n", ExpandStateNum(s), word); 
    }

  fputs("\n", out);
}


// Parses the BEX [STRINGS] section
void dehacked_t::Read_STRINGS()
{
  fprintf(out, "[STRINGS]\n");

  while (p.NewLine(false))
    {
      p.PassWS();
      if (!p.LineLen())
	break; // a whitespace-only line ends the record

      char *word = p.GetToken(" ");
      string_mnemonic_t *m;
      for (m = BEX_StringMnemonics; m->name && strcasecmp(word, m->name); m++);
      if (!m->name)
	{	  
	  error("[STRINGS]: Unknown mnemonic '%s'\n", word);
	  continue;
	}

      // FIXME backslash-continued lines...
      char *newtext = p.GetToken("=");
      fprintf(out, "%s =%s\n", word, newtext);
    }

  fputs("\n", out);
}



// dehacked command parser
enum DEH_cmd_t
{
  DEH_Thing,
  DEH_Frame,
  DEH_Pointer,
  DEH_Sound,
  DEH_Sprite,
  DEH_Text,
  DEH_Weapon,
  DEH_Ammo,
  DEH_Misc,
  DEH_Cheat,
  DEH_Doom,
  DEH_Patch,
  DEH_CODEPTR,
  DEH_PARS,
  DEH_STRINGS,
  DEH_NUM
};

static const char *DEH_cmds[DEH_NUM + 1] =
{
  "thing", "frame", "pointer", "sound", "sprite", "text", "weapon", "ammo", "misc", "cheat", "doom", "patch",
  "[CODEPTR]", "[PARS]", "[STRINGS]", NULL
};


// Parse a DeHackEd lump
// (there is special trick for converting .deh files into WAD lumps)
bool dehacked_t::LoadDehackedLump(const char *buf, int len)
{
  if (!p.Open(buf, len))
    return false;

  p.DeleteChars('\r'); // annoying cr's.

  num_errors = 0;

  int i;

  p.RemoveComments('#', true);
  while (p.NewLine())
    {
      char *word1, *word2;

      if ((word1 = p.GetToken(" ")))
	{
	  word2 = p.GetToken(" ");
	  if (word2)
	    i = atoi(word2);
	  else
	    {
	      i = 0;
	      //error("Warning: missing argument for '%s'\n", word1);
	    }

	  switch (P_MatchString(word1, DEH_cmds))
	    {
	    case DEH_Thing:
	      Read_Thing(word2);
	      break;

	    case DEH_Frame:
	      Read_Frame(word2);
	      break;

	    case DEH_Pointer:
	      /*
		Syntax:
		pointer uuu (frame xxx)
		codep frame = yyy
	      */
	      p.GetToken(" "); // get rid of "(frame"
	      if ((word1 = p.GetToken(")")))
		{
		  int s = atoi(word1);
		  fprintf(out, "Pointer %d (frame %d)\n", i, s);
		  if (p.NewLine())
		    {
		      p.GetToken(" ");
		      p.GetToken(" ");
		      word2 = p.GetToken(" =");
		      fprintf(out, "Codep frame = %s\n\n", word2);
		    }
		}
	      else
		error("Pointer %d : (Frame xxx) missing\n", i);
	      break;

	    case DEH_Sound:
	      Read_Sound(i);
	      break;

	    case DEH_Sprite:
	      fprintf(out, "Sprite %d\n", i);
	      error("Sprite command currently unsupported.\n");
	      if (p.NewLine())
		{
		  // FIXME
		  //int k = (FindValue() - 151328) / 8;
		}
	      break;

	    case DEH_Text:
	      if ((word1 = p.GetToken(" ")))
		{
		  int j = atoi(word1);
		  Read_Text(i, j);
		}
	      else
		error("Text : missing second number\n");
	      break;

	    case DEH_Weapon:
	      Read_Weapon(i);
	      break;

	    case DEH_Ammo:
	      Read_Ammo(i);
	      break;

	    case DEH_Misc:
	      Read_Misc();
	      break;

	    case DEH_Cheat:
	      Read_Cheat();
	      break;

	    case DEH_Doom:
	      i = FindValue();
	      if (i != 19)
		error("Warning : Patch from a different Doom version (%d), only version 1.9 is supported\n", i);
	      fprintf(out, "Doom version = %d\n", i);
	      break;

	    case DEH_Patch:
	      if (word2 && !strcasecmp(word2, "format"))
		{
		  if ((i = FindValue()) != 6)
		    error("Warning : Patch format not supported");
		  fprintf(out, "Patch format = %d\n\n", i);
		}
	      break;

	      // BEX stuff
	    case DEH_CODEPTR:
	      Read_CODEPTR();
	      break;
	    case DEH_PARS:
	      // TODO support PARS?
	      error("BEX [PARS] block currently unsupported.\n");
	      break;
	    case DEH_STRINGS:
	      Read_STRINGS();
	      break;
	    default:
	      error("Unknown command : %s\n", word1);
	    }
	}
      else
        error("No word in this line:\n%s\n", p.GetToken("\0"));
    }

  if (num_errors > 0)
    {
      printf("DEH: %d warning(s).\n", num_errors);
    }

  loaded = true;
  p.Clear();

  return true;
}


int FIL_ReadFile(const char *name, byte **buffer)
{
  struct stat fileinfo;

  int handle = open(name, O_RDONLY | O_BINARY, 0666);

  if (handle == -1)
    return 0;

  if (fstat(handle, &fileinfo) == -1)
    return 0;

  int length = fileinfo.st_size;
  byte *buf = (byte *)malloc(length+1);

  int count = read(handle, buf, length);
  close(handle);

  if (count < length)
    return 0;

  //Fab:26-04-98:append 0 byte for script text files
  buf[length] = 0;

  *buffer = buf;
  return length;
}


int main(int argc, char *argv[])
{
  if (argc != 3)
    {
      printf("This program converts DeHackEd files written for Doom Legacy versions 1.43 or earlier\n"
	     "to a format that is compatible with Doom Legacy 2.0.\n"
	     "Only those DeHackEd files need to be converted that use Heretic resources.\n"
	     "Usage: convert_deh old.deh new.deh\n");
      return -1;
    }

  int len = 0;
  byte *infile = NULL;
  if ((len = FIL_ReadFile(argv[1], &infile)) == 0)
    {
      printf("Could not open the input file '%s'.\n", argv[1]);
      return -1;
    }

  out = fopen(argv[2], "wb");
  if (!out)
    {
      printf("Could not open the output file '%s'.\n", argv[2]);
      return -1;
    }

  DEH.LoadDehackedLump((char *)infile, len);
  free(infile);
  fclose(out);

  return 0;
}
