// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: keys.h 714 2010-07-27 22:48:37Z smite-meister $
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
//-----------------------------------------------------------------------------


#ifndef __KEYS_H__
#define __KEYS_H__

/// These are the key event codes posted by the keyboard handler, and closely match the SDLKey symbols.
/// 0-127 are ASCII codes. The codes KEY_NUMKB- are reserved for virtual keys.
enum key_input_e
{
  KEY_NULL = 0, // null key, triggers nothing

  KEY_BACKSPACE  = 8,
  KEY_TAB        = 9,
  KEY_ENTER      = 13,
  KEY_PAUSE      = 19,
  KEY_ESCAPE     = 27,
  KEY_SPACE      = 32,

  // numbers
  // big letters
  KEY_BACKQUOTE  = 96,
  KEY_CONSOLE    = KEY_BACKQUOTE,
  // small letters
  KEY_DELETE     = 127, // ascii ends here

  // SDL international keys 160-255

  // keypad
  KEY_KEYPAD0 = 256,
  KEY_KEYPAD1,
  KEY_KEYPAD2,
  KEY_KEYPAD3,
  KEY_KEYPAD4,
  KEY_KEYPAD5,
  KEY_KEYPAD6,
  KEY_KEYPAD7,
  KEY_KEYPAD8,
  KEY_KEYPAD9,
  KEY_KPADPERIOD,
  KEY_KPADSLASH,
  KEY_KPADMULT,
  KEY_MINUSPAD,
  KEY_PLUSPAD,
  KEY_KPADENTER,
  KEY_KPADEQUALS,
  // arrows + home/end pad
#if !defined (__DJGPP__)
  KEY_UPARROW,
  KEY_DOWNARROW,
  KEY_RIGHTARROW,
  KEY_LEFTARROW,
#endif
  KEY_INS,
  KEY_HOME,
  KEY_END,
  KEY_PGUP,
  KEY_PGDN,
  // function keys
  KEY_F1,
  KEY_F2,
  KEY_F3,
  KEY_F4,
  KEY_F5,
  KEY_F6,
  KEY_F7,
  KEY_F8,
  KEY_F9,
  KEY_F10,
  KEY_F11,
  KEY_F12,

  // modifier keys
  KEY_NUMLOCK = 300,
  KEY_CAPSLOCK,
  KEY_SCROLLLOCK,
  KEY_RSHIFT,
  KEY_LSHIFT,
  KEY_RCTRL,
  KEY_LCTRL,
  KEY_RALT,
  KEY_LALT,
  KEY_unused1,
  KEY_unused2,
  KEY_LWIN,
  KEY_RWIN,
  KEY_MODE, // altgr
  KEY_unused3,
  // other function keys
  KEY_HELP = 315,
  KEY_PRINT,
  KEY_SYSREQ,
  KEY_BREAK,
  KEY_MENU,

  KEY_NUMKB, // all real keyboard codes are under this value

  // mouse and joystick buttons are handled as 'virtual' keys
  MOUSEBUTTONS =  8,
  MAXJOYSTICKS = 4,   // "Only" 4 joysticks per machine.
  JOYBUTTONS   = 16,  // Max number of buttons for a joystick.
  JOYHATBUTTONS = 4,  // Four hat directions.

  KEY_MOUSE1          = KEY_NUMKB, // mouse buttons, including the wheel
  KEY_MOUSEWHEELUP    = KEY_MOUSE1 + 3, // usually
  KEY_MOUSEWHEELDOWN,
  KEY_DBLMOUSE1       = KEY_MOUSE1     + MOUSEBUTTONS, // double clicks

  KEY_2MOUSE1         = KEY_DBLMOUSE1  + MOUSEBUTTONS, // second mouse buttons
  KEY_2MOUSEWHEELUP   = KEY_2MOUSE1 + 3,
  KEY_2MOUSEWHEELDOWN,
  KEY_DBL2MOUSE1      = KEY_2MOUSE1    + MOUSEBUTTONS,

  //KEY_JOYSTICKSTART   = KEY_DBL2MOUSE1 + MOUSEBUTTONS, // joystick buttons
  KEY_JOY0BUT0 = KEY_DBL2MOUSE1 + MOUSEBUTTONS, // joystick buttons
  KEY_JOY0BUT1,
  KEY_JOY0BUT2,
  KEY_JOY0BUT3,
  KEY_JOY0BUT4,
  KEY_JOY0BUT5,
  KEY_JOY0BUT6,
  KEY_JOY0BUT7,
  KEY_JOY0BUT8,
  KEY_JOY0BUT9,
  KEY_JOY0BUT10,
  KEY_JOY0BUT11,
  KEY_JOY0BUT12,
  KEY_JOY0BUT13,
  KEY_JOY0BUT14,
  KEY_JOY0BUT15,

  KEY_JOY1BUT0,
  KEY_JOY1BUT1,
  KEY_JOY1BUT2,
  KEY_JOY1BUT3,
  KEY_JOY1BUT4,
  KEY_JOY1BUT5,
  KEY_JOY1BUT6,
  KEY_JOY1BUT7,
  KEY_JOY1BUT8,
  KEY_JOY1BUT9,
  KEY_JOY1BUT10,
  KEY_JOY1BUT11,
  KEY_JOY1BUT12,
  KEY_JOY1BUT13,
  KEY_JOY1BUT14,
  KEY_JOY1BUT15,

  KEY_JOY2BUT0,
  KEY_JOY2BUT1,
  KEY_JOY2BUT2,
  KEY_JOY2BUT3,
  KEY_JOY2BUT4,
  KEY_JOY2BUT5,
  KEY_JOY2BUT6,
  KEY_JOY2BUT7,
  KEY_JOY2BUT8,
  KEY_JOY2BUT9,
  KEY_JOY2BUT10,
  KEY_JOY2BUT11,
  KEY_JOY2BUT12,
  KEY_JOY2BUT13,
  KEY_JOY2BUT14,
  KEY_JOY2BUT15,

  KEY_JOY3BUT0,
  KEY_JOY3BUT1,
  KEY_JOY3BUT2,
  KEY_JOY3BUT3,
  KEY_JOY3BUT4,
  KEY_JOY3BUT5,
  KEY_JOY3BUT6,
  KEY_JOY3BUT7,
  KEY_JOY3BUT8,
  KEY_JOY3BUT9,
  KEY_JOY3BUT10,
  KEY_JOY3BUT11,
  KEY_JOY3BUT12,
  KEY_JOY3BUT13,
  KEY_JOY3BUT14,
  KEY_JOY3BUT15,

  //KEY_JOYSTICKEND,
  /*
  KEY_DBLJOY1       = KEY_JOY1 + JOYBUTTONS,
  KEY_DBLJOY14      = KEY_DBLJOY1 + JOYBUTTONS - 1,
  */

  // number of total 'button' inputs, includes keyboard keys, plus virtual
  // keys (mousebuttons and joybuttons become keys)
  NUMINPUTS
};

#if defined (__DJGPP__)
#define KEY_UPARROW     (0x80+102)
#define KEY_LEFTARROW   (0x80+105)
#define KEY_RIGHTARROW  (0x80+107)
#define KEY_DOWNARROW   (0x80+110)
#define KEY_SHIFT       (0x80+54)
#endif
#endif
