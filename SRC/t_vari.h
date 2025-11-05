// Emacs style mode select -*- C++ -*-
//----------------------------------------------------------------------------
//
// $Id: t_vari.h 801 2011-03-06 18:55:50Z wesleyjohnson $
//
// Copyright(C) 2000 Simon Howard
// Copyright (C) 2001-2011 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// $Log: t_vari.h,v $
// Revision 1.3  2004/07/27 08:19:37  exl
// New fmod, fs functions, bugfix or 2, patrol nodes
//
// Revision 1.2  2003/05/30 22:44:07  hurdler
// add checkcvar function to FS
//
// Revision 1.1  2000/11/02 17:57:28  stroggonmeth
// FraggleScript files...
//
//
//--------------------------------------------------------------------------


#ifndef __VARIABLE_H__
#define __VARIABLE_H__

typedef struct svariable_s svariable_t;
#define VARIABLESLOTS 16

#include "t_parse.h"
#include "p_mobj.h"
#include "m_fixed.h"

// hash the variables for speed: this is the hashkey

#define variable_hash(n)                \
              (   ( (n)[0] + (n)[1] +   \
                   ((n)[1] ? (n)[2] +   \
                   ((n)[2] ? (n)[3]  : 0) : 0) ) % VARIABLESLOTS )

     // svariable_t
struct svariable_s
{
  char *name;
  int type;       // vt_string or vt_int: same as in svalue_t
  union
  {
    int32_t    i;
    fixed_t    fixed;
    char      *s;
    mobj_t    *mobj;
    sfarray_t *a;           // arrays
    
    char **pS;              // pointer to game string
    int *pI;                // pointer to game int
    fixed_t *pFixed;
    mobj_t **pMobj;         // pointer to game obj
    double *pf;
    sfarray_t **pA;         // arrays
    
    void (*handler)();      // for functions
    char *labelptr;         // for labels
  } value;
  svariable_t *next;       // for hashing
};

// variable types

enum
{
  FSVT_string,
  FSVT_int,
  FSVT_fixed,
  FSVT_mobj,         // a map object
  FSVT_function,     // functions are stored as variables
  FSVT_label,        // labels for goto calls are variables
  FSVT_const,        // const
  FSVT_array,        // array
  FSVT_pInt,         // pointer to game int
  FSVT_pFixed,
  FSVT_pString,      // pointer to game string
  FSVT_pMobj,        // pointer to game mobj
  FSVT_pArray,       // haleyjd: 05/27: pointer to game array
};

// variables

void T_ClearHubScript();

void init_variables();
svariable_t *new_variable(script_t *script, char *name, int vtype);
svariable_t *find_variable(char *name);
svariable_t *variableforname(script_t *script, char *name);
svalue_t getvariablevalue(svariable_t *v);
void setvariablevalue(svariable_t *v, svalue_t newvalue);
void clear_variables(script_t *script);

svariable_t *add_game_int(char *name, int *var);
svariable_t *add_game_string(char *name, char **var);
svariable_t *add_game_mobj(char *name, mobj_t **mo);

// functions

svalue_t evaluate_function(int start, int stop);   // actually run a function
svariable_t *new_function(char *name, void (*handler)() );

// arguments to handler functions

#define MAXARGS 128
extern int t_argc;
extern svalue_t *t_argv;
extern svalue_t t_return;

#endif
