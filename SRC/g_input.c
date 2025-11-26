// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: g_input.c 1047 2013-09-22 20:57:40Z wesleyjohnson $
//
// Copyright (C) 1998-2010 by DooM Legacy Team.
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
// $Log: g_input.c,v $
// Include: DOS DJGPP Fixes
//
// Revision 1.12  2002/08/24 22:42:02  hurdler
// Apply Robert Hogberg patches
//
// Revision 1.11  2002/07/01 19:59:58  metzgermeister
// *** empty log message ***
//
// Revision 1.10  2001/03/30 17:12:49  bpereira
// Revision 1.9  2001/02/24 13:35:19  bpereira
// Revision 1.8  2001/02/10 12:27:13  bpereira
//
// Revision 1.7  2001/01/25 22:15:42  bpereira
// added heretic support
//
// Revision 1.6  2000/11/26 00:46:31  hurdler
//
// Revision 1.5  2000/10/04 17:03:57  hurdler
// This is the formule I propose for mouse sensitivity
//
// Revision 1.4  2000/04/16 18:38:07  bpereira
//
// Revision 1.3  2000/04/04 00:32:45  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      handle mouse/keyboard/joystick inputs,
//      maps inputs to game controls (forward,use,open...)
//
//-----------------------------------------------------------------------------

#include "doomincl.h"
#include "doomstat.h"
#include "g_input.h"
#include "keys.h"
#include "hu_stuff.h"   //need HUFONT start & end
#include "keys.h"
#include "d_net.h"
#include "console.h"
#include "i_joy.h"
#include "i_system.h"

int num_joybindings = 0;
joybinding_t joybindings[MAX_JOYBINDINGS];


CV_PossibleValue_t mousesens_cons_t[]={{1,"MIN"},{MAXMOUSESENSITIVITY,"MAXCURSOR"},{MAXINT,"MAX"},{0,NULL}};
CV_PossibleValue_t onecontrolperkey_cons_t[]={{1,"One"},{2,"Several"},{0,NULL}};

// mouse values are used once
consvar_t  cv_mouse_sens_x    = {"mousesensx","10",CV_SAVE,mousesens_cons_t};
consvar_t  cv_mouse_sens_y    = {"mousesensy","10",CV_SAVE,mousesens_cons_t};
consvar_t  cv_mouse2_sens_x   = {"mouse2sensx","10",CV_SAVE,mousesens_cons_t};
consvar_t  cv_mouse2_sens_y   = {"mouse2sensy","10",CV_SAVE,mousesens_cons_t};
consvar_t  cv_allowjump    = {"allowjump","1",CV_NETVAR,CV_YesNo};
consvar_t  cv_allowautoaim = {"allowautoaim","1",CV_NETVAR,CV_YesNo};
consvar_t  cv_controlperkey = {"controlperkey","1",CV_SAVE,onecontrolperkey_cons_t};
//SoM: 3/28/2000: Working rocket jumping.
consvar_t  cv_allowrocketjump = {"allowrocketjump","0",CV_NETVAR,CV_YesNo};

#ifdef SDL
CV_PossibleValue_t mouse_motion_cons_t[]={{0,"Absolute"},{1,"Relative"},{0,NULL}};
consvar_t  cv_mouse_motion = {"mousemotion","0", CV_SAVE|CV_CALL, mouse_motion_cons_t, I_StartupMouse };
#endif
consvar_t  cv_grabinput = {"grabinput","1", CV_SAVE|CV_CALL, CV_OnOff, I_StartupMouse };


int  mousex, mousey;
int  mouse2x, mouse2y;

boolean   gamekeydown[NUMINPUTS]; // Current state of the keys: true if the key is currently down.
boolean gamekeytapped[NUMINPUTS]; // True if the key has been pressed since the last G_BuildTiccmd. Useful for impulse-style controls.

#if defined( __DJGPP__ )
// joystick values are repeated
int             joyxmove;
int             joyymove;
#endif

// two key codes (or virtual key) per game control
int  gamecontrol[num_gamecontrols][2];
int  gamecontrol2[num_gamecontrols][2];        // secondary splitscreen player


// FIXME: this can be simplified to two bytes
typedef struct {
    int dtime;
    boolean state;
    int clicks;
} dclick_t;

// FIXME: only one mouse, only one joy
static  dclick_t  mousedclicks[MOUSEBUTTONS];
#ifdef DBL_JOY_BUTTONS
static  dclick_t  joydclicks[JOYBUTTONS];
#endif

//
//  General double-click detection routine for any kind of input.
//
static boolean G_CheckDoubleClick(boolean state, dclick_t *dt)
{
    if (state != dt->state && dt->dtime > 1 )
    {
        dt->state = state;
        if (state)
            dt->clicks++;
        if (dt->clicks == 2)
        {
            dt->clicks = 0;
            return true;
        }
        else
            dt->dtime = 0;
    }
    else
    {
        dt->dtime ++;
        if (dt->dtime > 20)
        {
            dt->clicks = 0;
            dt->state = false;
        }
    }
    return false;
}



//
//  Remaps the inputs to game controls.
//
//  A game control can be triggered by one or more keys/buttons.
//
//  Each key/mousebutton/joybutton triggers ONLY ONE game control.
//
//
void  G_MapEventsToControls (event_t *ev)
{
  int i;

    switch (ev->type)
    {
      case ev_keydown:
        if (ev->data1 < NUMINPUTS)
	  {
	    gamekeydown[ev->data1] = true;
	    gamekeytapped[ev->data1] = true; // reset in G_BuildTiccmd
	  }
        break;

      case ev_keyup:
        if (ev->data1 < NUMINPUTS)
	  gamekeydown[ev->data1] = false;
        break;

      case ev_mouse:           // buttons are virtual keys
        // [WDJ] To handle multiple mouse motion events per frame instead
        // of letting the last event be the sole mouse motion,
        // add the mouse events.
        // Necessary for OpenBSD which reports x and y in separate events.
        mousex += ev->data2*((cv_mouse_sens_x.value*cv_mouse_sens_x.value)/110.0f + 0.1);
        mousey += ev->data3*((cv_mouse_sens_y.value*cv_mouse_sens_y.value)/110.0f + 0.1);
        break;

      case ev_mouse2:           // buttons are virtual keys
        // add multiple mouse motions
        mouse2x += ev->data2*((cv_mouse2_sens_x.value*cv_mouse2_sens_x.value)/110.0f + 0.1);
        mouse2y += ev->data3*((cv_mouse2_sens_y.value*cv_mouse2_sens_y.value)/110.0f + 0.1);
        break;

      #if defined( __DJGPP__ )
      case ev_joystick:        // buttons are virtual keys
        joyxmove = ev->data2;
        joyymove = ev->data3;
        break;
      #endif
      default:
        break;
    }

    // ALWAYS check for mouse & joystick double-clicks
    // even if no mouse event
    // FIXME: first MOUSE only
    for (i=0;i<MOUSEBUTTONS;i++)
      gamekeydown[KEY_DBLMOUSE1+i] = G_CheckDoubleClick(gamekeydown[KEY_MOUSE1+i], &mousedclicks[i]);

#ifdef DBL_JOY_BUTTONS     
    // joystick doubleclicks
    // FIXME: JOY0 only
    for (i=0;i<JOYBUTTONS;i++)
      gamekeydown[KEY_DBLJOY0BUT0+i] = G_CheckDoubleClick(gamekeydown[KEY_JOY0BUT0+i], &joydclicks[i]);
#endif
}




typedef struct {
    int  keynum;
    char name[16];
} keyname_t;

static keyname_t keynames[] =
{
  {KEY_NULL,      "null"},

  {KEY_BACKSPACE, "backspace"},
  {KEY_TAB,       "tab"},
  {KEY_ENTER,     "enter"},
  {KEY_PAUSE,     "pause"},  // irrelevant, since this key cannot be remapped...
  {KEY_ESCAPE,    "escape"}, // likewise
  {KEY_SPACE,     "space"},

  {KEY_CONSOLE,    "console"},

  {KEY_NUMLOCK,    "num lock"},
  {KEY_CAPSLOCK,   "caps lock"},
  {KEY_SCROLLLOCK, "scroll lock"},
  {KEY_RSHIFT,     "right shift"},
  {KEY_LSHIFT,     "left shift"},
  {KEY_RCTRL,      "right ctrl"},
  {KEY_LCTRL,      "left ctrl"},
  {KEY_RALT,       "right alt"},
  {KEY_LALT,       "left alt"},
  {KEY_LWIN,       "left win"},
  {KEY_RWIN,       "right win"},
  {KEY_MODE,       "altgr"},
  {KEY_MENU,       "menu"},

  // keypad keys
  {KEY_KEYPAD0, "keypad 0"},
  {KEY_KEYPAD1, "keypad 1"},
  {KEY_KEYPAD2, "keypad 2"},
  {KEY_KEYPAD3, "keypad 3"},
  {KEY_KEYPAD4, "keypad 4"},
  {KEY_KEYPAD5, "keypad 5"},
  {KEY_KEYPAD6, "keypad 6"},
  {KEY_KEYPAD7, "keypad 7"},
  {KEY_KEYPAD8, "keypad 8"},
  {KEY_KEYPAD9, "keypad 9"},
  {KEY_KPADPERIOD,"keypad ."},
  {KEY_KPADSLASH, "keypad /"},
  {KEY_KPADMULT,  "keypad *"},
  {KEY_MINUSPAD,  "keypad -"},
  {KEY_PLUSPAD,   "keypad +"},

  // extended keys (not keypad)
  {KEY_UPARROW,   "up arrow"},
  {KEY_DOWNARROW, "down arrow"},
  {KEY_RIGHTARROW,"right arrow"},
  {KEY_LEFTARROW, "left arrow"},
  {KEY_INS,       "ins"},
  {KEY_DELETE,    "del"},
  {KEY_HOME,      "home"},
  {KEY_END,       "end"},
  {KEY_PGUP,      "pgup"},
  {KEY_PGDN,      "pgdown"},

  // other keys
  {KEY_F1, "F1"},
  {KEY_F2, "F2"},
  {KEY_F3, "F3"},
  {KEY_F4, "F4"},
  {KEY_F5, "F5"},
  {KEY_F6, "F6"},
  {KEY_F7, "F7"},
  {KEY_F8, "F8"},
  {KEY_F9, "F9"},
  {KEY_F10,"F10"},
  {KEY_F11,"F11"},
  {KEY_F12,"F12"},

  // virtual keys for mouse buttons and joystick buttons
  {KEY_MOUSE1,  "mouse 1"},
  {KEY_MOUSE1+1,"mouse 2"},
  {KEY_MOUSE1+2,"mouse 3"},
  {KEY_MOUSEWHEELUP, "mwheel up"},
  {KEY_MOUSEWHEELDOWN,"mwheel down"},
  {KEY_MOUSE1+5,"mouse 6"},
  {KEY_MOUSE1+6,"mouse 7"},
  {KEY_MOUSE1+7,"mouse 8"},
  {KEY_2MOUSE1,  "2nd mouse 2"},    //BP: sorry my mouse handler swap button 1 and 2
  {KEY_2MOUSE1+1,"2nd mouse 1"},
  {KEY_2MOUSE1+2,"2nd mouse 3"},
  {KEY_2MOUSEWHEELUP,"2nd mwheel up"},
  {KEY_2MOUSEWHEELDOWN,"2nd mwheel down"},
  {KEY_2MOUSE1+5,"2nd mouse 6"},
  {KEY_2MOUSE1+6,"2nd mouse 7"},
  {KEY_2MOUSE1+7,"2nd mouse 8"},

  {KEY_DBLMOUSE1,   "mouse 1 d"},
  {KEY_DBLMOUSE1+1, "mouse 2 d"},
  {KEY_DBLMOUSE1+2, "mouse 3 d"},
  {KEY_DBLMOUSE1+3, "mouse 4 d"},
  {KEY_DBLMOUSE1+4, "mouse 5 d"},
  {KEY_DBLMOUSE1+5, "mouse 6 d"},
  {KEY_DBLMOUSE1+6, "mouse 7 d"},
  {KEY_DBLMOUSE1+7, "mouse 8 d"},
  {KEY_DBL2MOUSE1,  "2nd mouse 2 d"},
  {KEY_DBL2MOUSE1+1,"2nd mouse 1 d"},
  {KEY_DBL2MOUSE1+2,"2nd mouse 3 d"},
  {KEY_DBL2MOUSE1+3,"2nd mouse 4 d"},
  {KEY_DBL2MOUSE1+4,"2nd mouse 5 d"},
  {KEY_DBL2MOUSE1+5,"2nd mouse 6 d"},
  {KEY_DBL2MOUSE1+6,"2nd mouse 7 d"},
  {KEY_DBL2MOUSE1+7,"2nd mouse 8 d"},

  {KEY_JOY0BUT0, "Joy 0 b 0"},
  {KEY_JOY0BUT1, "Joy 0 b 1"},
  {KEY_JOY0BUT2, "Joy 0 b 2"},
  {KEY_JOY0BUT3, "Joy 0 b 3"},
  {KEY_JOY0BUT4, "Joy 0 b 4"},
  {KEY_JOY0BUT5, "Joy 0 b 5"},
  {KEY_JOY0BUT6, "Joy 0 b 6"},
  {KEY_JOY0BUT7, "Joy 0 b 7"},
  {KEY_JOY0BUT8, "Joy 0 b 8"},
  {KEY_JOY0BUT9, "Joy 0 b 9"},
  {KEY_JOY0BUT10, "Joy 0 b 10"},
  {KEY_JOY0BUT11, "Joy 0 b 11"},
  {KEY_JOY0BUT12, "Joy 0 b 12"},
  {KEY_JOY0BUT13, "Joy 0 b 13"},
  {KEY_JOY0BUT14, "Joy 0 b 14"},
  {KEY_JOY0BUT15, "Joy 0 b 15"},

  {KEY_JOY1BUT0, "Joy 1 b 0"},
  {KEY_JOY1BUT1, "Joy 1 b 1"},
  {KEY_JOY1BUT2, "Joy 1 b 2"},
  {KEY_JOY1BUT3, "Joy 1 b 3"},
  {KEY_JOY1BUT4, "Joy 1 b 4"},
  {KEY_JOY1BUT5, "Joy 1 b 5"},
  {KEY_JOY1BUT6, "Joy 1 b 6"},
  {KEY_JOY1BUT7, "Joy 1 b 7"},
  {KEY_JOY1BUT8, "Joy 1 b 8"},
  {KEY_JOY1BUT9, "Joy 1 b 9"},
  {KEY_JOY1BUT10, "Joy 1 b 10"},
  {KEY_JOY1BUT11, "Joy 1 b 11"},
  {KEY_JOY1BUT12, "Joy 1 b 12"},
  {KEY_JOY1BUT13, "Joy 1 b 13"},
  {KEY_JOY1BUT14, "Joy 1 b 14"},
  {KEY_JOY1BUT15, "Joy 1 b 15"},

  {KEY_JOY2BUT0, "Joy 2 b 0"},
  {KEY_JOY2BUT1, "Joy 2 b 1"},
  {KEY_JOY2BUT2, "Joy 2 b 2"},
  {KEY_JOY2BUT3, "Joy 2 b 3"},
  {KEY_JOY2BUT4, "Joy 2 b 4"},
  {KEY_JOY2BUT5, "Joy 2 b 5"},
  {KEY_JOY2BUT6, "Joy 2 b 6"},
  {KEY_JOY2BUT7, "Joy 2 b 7"},
  {KEY_JOY2BUT8, "Joy 2 b 8"},
  {KEY_JOY2BUT9, "Joy 2 b 9"},
  {KEY_JOY2BUT10, "Joy 2 b 10"},
  {KEY_JOY2BUT11, "Joy 2 b 11"},
  {KEY_JOY2BUT12, "Joy 2 b 12"},
  {KEY_JOY2BUT13, "Joy 2 b 13"},
  {KEY_JOY2BUT14, "Joy 2 b 14"},
  {KEY_JOY2BUT15, "Joy 2 b 15"},

  {KEY_JOY3BUT0, "Joy 3 b 0"},
  {KEY_JOY3BUT1, "Joy 3 b 1"},
  {KEY_JOY3BUT2, "Joy 3 b 2"},
  {KEY_JOY3BUT3, "Joy 3 b 3"},
  {KEY_JOY3BUT4, "Joy 3 b 4"},
  {KEY_JOY3BUT5, "Joy 3 b 5"},
  {KEY_JOY3BUT6, "Joy 3 b 6"},
  {KEY_JOY3BUT7, "Joy 3 b 7"},
  {KEY_JOY3BUT8, "Joy 3 b 8"},
  {KEY_JOY3BUT9, "Joy 3 b 9"},
  {KEY_JOY3BUT10, "Joy 3 b 10"},
  {KEY_JOY3BUT11, "Joy 3 b 11"},
  {KEY_JOY3BUT12, "Joy 3 b 12"},
  {KEY_JOY3BUT13, "Joy 3 b 13"},
  {KEY_JOY3BUT14, "Joy 3 b 14"},
  {KEY_JOY3BUT15, "Joy 3 b 15"},
};

char *gamecontrolname[num_gamecontrols] =
{
    "nothing",        //a key/button mapped to gc_null has no effect
    "forward",
    "backward",
    "strafe",
    "straferight",
    "strafeleft",
    "speed",
    "turnleft",
    "turnright",
    "fire",
    "use",
    "lookup",
    "lookdown",
    "centerview",
    "mouseaiming",
    "weapon1",
    "weapon2",
    "weapon3",
    "weapon4",
    "weapon5",
    "weapon6",
    "weapon7",
    "weapon8",
    "talkkey",
    "scores",
    "jump",
    "console",
    "nextweapon",
    "prevweapon",
    "bestweapon",
    "inventorynext",
    "inventoryprev",
    "inventoryuse",
    "down"
};

#define NUMKEYNAMES (sizeof(keynames)/sizeof(keyname_t))

//
//  Detach any keys associated to the given game control
//  - pass the pointer to the gamecontrol table for the player being edited
void  G_ClearControlKeys (int (*setupcontrols)[2], int control)
{
    setupcontrols[control][0] = KEY_NULL;
    setupcontrols[control][1] = KEY_NULL;
}

//
//  Returns the name of a key (or virtual key for mouse and joy)
//  the input value being an keynum
//
char* G_KeynumToString (int keynum)
{
static char keynamestr[8];

    int    j;

    // return a string with the ascii char if displayable
    if (keynum>' ' && keynum<='z' && keynum!=KEY_CONSOLE)
    {
        keynamestr[0] = keynum;
        keynamestr[1] = '\0';
        return keynamestr;
    }

    // find a description for special keys
    for (j=0;j<NUMKEYNAMES;j++)
    {
        if (keynames[j].keynum==keynum)
            return keynames[j].name;
    }

    // create a name for Unknown key
    sprintf(keynamestr,"key%d",keynum);
    return keynamestr;
}


int G_KeyStringtoNum(char *keystr)
{
    int j;
    int len = strlen(keystr);
    byte stat = 0;


    if(keystr[1]==0 && keystr[0]>' ' && keystr[0]<='z')
        return keystr[0];

  retry_search:
    // caseless comparison, uppercase or lowercase input will work
    for (j=0; j<NUMKEYNAMES; j++)
    {
        if (strcasecmp(keynames[j].name,keystr)==0)
            return keynames[j].keynum;
    }
   
    // failed, try to change _ to space
    if( stat == 0 )
    {
        // [WDJ] Got tired trying to fix bugs about a quoted button name
        // within a the quoted script string like alias.
	// Let them specify the button name with underlines and we will
	// equate it to the name with spaces, and it does not require those
        // escaped quotes.
	//   mouse_3  --> "mouse 3"
        for (j=0; j<len; j++ )
        {
	    if ( keystr[j] == '_' )
	    {
	        keystr[j] = ' ';
	        stat = 1;
	    }
        }
        if ( stat )   goto retry_search;  // found some '_', try it again
    }
    // not found, try to interpret it as a number

    if(strlen(keystr)>3)
        return atoi(&keystr[3]);

    return 0;
}


void G_Controldefault(void)
{
    gamecontrol[gc_forward    ][0]=KEY_UPARROW;
    gamecontrol[gc_forward    ][1]=KEY_MOUSE1+2;
    gamecontrol[gc_backward   ][0]=KEY_DOWNARROW;
    gamecontrol[gc_strafe     ][0]=KEY_LALT;
    gamecontrol[gc_strafe     ][1]=KEY_MOUSE1+1;
    gamecontrol[gc_straferight][0]='.';
    gamecontrol[gc_strafeleft ][0]=',';
    gamecontrol[gc_speed      ][0]=KEY_LSHIFT;
    gamecontrol[gc_turnleft   ][0]=KEY_LEFTARROW;
    gamecontrol[gc_turnright  ][0]=KEY_RIGHTARROW;
    gamecontrol[gc_fire       ][0]=KEY_RCTRL;
    gamecontrol[gc_fire       ][1]=KEY_MOUSE1;
    gamecontrol[gc_use        ][0]=KEY_SPACE;
    gamecontrol[gc_lookup     ][0]=KEY_PGUP;
    gamecontrol[gc_lookdown   ][0]=KEY_PGDN;
    gamecontrol[gc_centerview ][0]=KEY_END;
    gamecontrol[gc_mouseaiming][0]='s';
    gamecontrol[gc_weapon1    ][0]='1';
    gamecontrol[gc_weapon2    ][0]='2';
    gamecontrol[gc_weapon3    ][0]='3';
    gamecontrol[gc_weapon4    ][0]='4';
    gamecontrol[gc_weapon5    ][0]='5';
    gamecontrol[gc_weapon6    ][0]='6';
    gamecontrol[gc_weapon7    ][0]='7';
    gamecontrol[gc_weapon8    ][0]='8';
    gamecontrol[gc_talkkey    ][0]='t';
    gamecontrol[gc_scores     ][0]='f';
    gamecontrol[gc_jump       ][0]='/';
    gamecontrol[gc_console    ][0]=KEY_CONSOLE;
    //gamecontrol[gc_nextweapon ][1]=KEY_JOY0BUT4;
    //gamecontrol[gc_prevweapon ][1]=KEY_JOY0BUT5;

    if( gamemode == heretic )
    {
        gamecontrol[gc_invnext    ][0] = ']';
        gamecontrol[gc_invprev    ][0] = '[';
        gamecontrol[gc_invuse     ][0] = KEY_ENTER;
        gamecontrol[gc_jump       ][0] = KEY_INS;
        gamecontrol[gc_flydown    ][0] = KEY_DELETE;
    }
    else
    {
        gamecontrol[gc_nextweapon ][0]=']';
        gamecontrol[gc_prevweapon ][0]='[';
    }
}

void G_SaveKeySetting(FILE *f)
{
    int i;

    for(i=1;i<num_gamecontrols;i++)
    {
        fprintf(f,"setcontrol \"%s\" \"%s\"",
		gamecontrolname[i],
		G_KeynumToString(gamecontrol[i][0]));

        if(gamecontrol[i][1])
	    fprintf(f," \"%s\"\n",
                        G_KeynumToString(gamecontrol[i][1]));
        else
            fprintf(f,"\n");
    }

    for(i=1;i<num_gamecontrols;i++)
    {
        fprintf(f, "setcontrol2 \"%s\" \"%s\"",
		gamecontrolname[i],
		G_KeynumToString(gamecontrol2[i][0]));

        if(gamecontrol2[i][1])
	    fprintf(f, " \"%s\"\n",
		    G_KeynumToString(gamecontrol2[i][1]));
        else
	    fprintf(f,"\n");
    }

#if !defined( __DJGPP__ )
    // Writes the joystick axis binding commands to the config file.
    for (i=0; i<num_joybindings; i++)
    {
        joybinding_t j = joybindings[i];
        fprintf(f, "bindjoyaxis %d %d %d %d %f\n",
		j.joynum, j.axisnum, j.playnum, (int)(j.action), j.scale);
    }
#endif
}

void G_CheckDoubleUsage(int keynum)
{
    if( cv_controlperkey.value==1 )
    {
        int i;
        for(i=0;i<num_gamecontrols;i++)
        {
            if( gamecontrol[i][0]==keynum )
                gamecontrol[i][0]= KEY_NULL;
            if( gamecontrol[i][1]==keynum )
                gamecontrol[i][1]= KEY_NULL;
            if( gamecontrol2[i][0]==keynum )
                gamecontrol2[i][0]= KEY_NULL;
            if( gamecontrol2[i][1]==keynum )
                gamecontrol2[i][1]= KEY_NULL;
        }
    }
}

void setcontrol(int (*gc)[2], char * cstr)
{
    int numctrl;
    char *namectrl;
    int keynum;
    COM_args_t  carg;
    
    COM_Args( &carg );
   
    if ( carg.num!= 3 && carg.num!=4 )
    {
        CONS_Printf ("setcontrol%s <controlname> <keyname> [<2nd keyname>]\n", cstr);
        return;
    }

    namectrl=carg.arg[1];
    for(numctrl=0;numctrl<num_gamecontrols
                  && strcasecmp(namectrl,gamecontrolname[numctrl])
                 ;numctrl++);
    if(numctrl==num_gamecontrols)
    {
        CONS_Printf("Control '%s' unknown\n",namectrl);
        return;
    }
    keynum=G_KeyStringtoNum(carg.arg[2]);
    G_CheckDoubleUsage(keynum);
    gc[numctrl][0]=keynum;

    if(carg.num==4)
        gc[numctrl][1]=G_KeyStringtoNum(carg.arg[3]);
    else
        gc[numctrl][1]=0;
}

void Command_Setcontrol_f(void)
{
    setcontrol(gamecontrol, "");
}

void Command_Setcontrol2_f(void)
{
    setcontrol(gamecontrol2, "2");
}



//! Magically converts a console command to a joystick axis binding. Also releases bindings.
void Command_BindJoyaxis_f()
{
  joybinding_t j;
  unsigned int i;
  COM_args_t  carg;
    
  COM_Args( &carg );

  if(carg.num == 1)
  { // Print bindings.
    CONS_Printf("%d joysticks found.\n", num_joysticks);
    if(num_joybindings == 0) {
      CONS_Printf("No joystick axis bindings defined.\n");
      return;
    }
    CONS_Printf("Current axis bindings.\n");
    for(i=0; i<num_joybindings; i++) {
      j = joybindings[i];
      CONS_Printf("%d %d %d %d %f\n", j.joynum, j.axisnum,
		  j.playnum, (int)j.action, j.scale);
    }
    return;
  }

  if (carg.num == 4 || carg.num > 6)
  {
    CONS_Printf("bindjoyaxis [joynum] [axisnum] [playnum] [action] [scale]  to bind\n"
		"bindjoyaxis [joynum] [axisnum]  to unbind\n");
    return;
  }

  j.joynum  = atoi( carg.arg[1] );
  if(j.joynum < 0 || j.joynum >= num_joysticks) {
    CONS_Printf("Attempting to bind/release non-existent joystick %d.\n", j.joynum);
    return;
  }

  j.axisnum = (carg.num >= 3) ? atoi( carg.arg[2] ) : -1;
#if !defined( __DJGPP__ )
  if(j.axisnum < -1 || j.axisnum >= I_JoystickNumAxes(j.joynum)) {
    CONS_Printf("Attempting to bind/release non-existent axis %d.\n", j.axisnum);
    return;
  }
#endif
  if (carg.num == 3)
  { // release binding(s)
    /* Takes one or two parameters. The first one is the joystick number
       and the second is the axis number. If either is not specified, all
       values are assumed to match.
    */

    int num_keep_bindings = 0;
    joybinding_t keep_bindings[MAX_JOYBINDINGS];

    if(num_joybindings == 0) {
      CONS_Printf("No bindings to unset.\n");
      return;
    }

    unsigned int i;
    for(i=0; i<num_joybindings; i++) {
      joybinding_t temp = joybindings[i];
      if((j.joynum == temp.joynum) &&
	 (j.axisnum == -1 || j.axisnum == temp.axisnum))
	continue; // We have a binding to prune.

      keep_bindings[num_keep_bindings++] = temp; // keep it
    }

    // We have the new bindings.
    if (num_keep_bindings == num_joybindings) {
      CONS_Printf("No bindings matched the parameters.\n");
      return;
    }

    // replace the bindings
    num_joybindings = num_keep_bindings;
    for(i=0; i<num_keep_bindings; i++)
      joybindings[i] = keep_bindings[i];
  }
  else
  { // create a binding
    j.playnum = atoi( carg.arg[3] );
    // carg.arg[0..3] only, use COM_Argv for others
    j.action  = (joyactions_e)(atoi(COM_Argv(4)));
    if (carg.num == 6)
      j.scale = atof(COM_Argv(5));
    else
      j.scale = 1.0f;

    if(j.action < 0 || j.action >= num_joyactions) {
      CONS_Printf("Attempting to bind non-existent action %d.\n", (int)(j.action));
      return;
    }

    // Overwrite existing binding, if any. Otherwise just append.
    for(i=0; i<num_joybindings; i++) {
      joybinding_t j2 = joybindings[i];
      if(j2.joynum == j.joynum && j2.axisnum == j.axisnum) {
	joybindings[i] = j;
	CONS_Printf("Joystick binding modified.\n");
	return;
      }
    }
    // new binding
    if (num_joybindings < MAX_JOYBINDINGS)
    {
	joybindings[num_joybindings++] = j;
	CONS_Printf("Joystick binding added.\n");
    }
    else
      CONS_Printf("Maximum number of joystick bindings reached.\n");
  }
}
