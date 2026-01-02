// Emacs style mode select -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: b_game.h 1462 2019-09-27 06:58:56Z wesleyjohnson $
//
// Copyright (C) 2002 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
//
// $Log: b_game.h,v $
// Revision 1.3  2002/09/28 06:53:11  tonyd
// fixed CR problem, fixed game options crash
//
// Revision 1.2  2002/09/27 16:40:08  tonyd
// First commit of acbot
//
//-----------------------------------------------------------------------------

#ifndef BOTGAME_H
#define BOTGAME_H

#include "b_bot.h"
// bot_game.h
#include "d_ticcmd.h"
#include "d_player.h"

typedef struct
{
    char* name;    // botnames
    byte  colour;  // 0..10
    uint16_t  skinrand;
} bot_info_t;

extern bot_info_t  botinfo[MAXPLAYERS];

void B_Register_Commands(void);
void B_BuildTiccmd(player_t* p, ticcmd_t* cmd);
void B_Init_Bots(void);
void B_Init_Nodes(void);
void Command_AddBot(void);
void B_Regulate_Bots( int req_numbots );

bot_t* B_Create_Bot();
void B_SpawnBot(bot_t* p);

////////// CTF ///////////

extern boolean ctf;
//////////////////////////

#endif
