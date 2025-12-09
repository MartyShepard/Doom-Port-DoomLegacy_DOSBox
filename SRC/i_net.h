// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: i_net.h 1145 2015-04-03 13:47:13Z wesleyjohnson $
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
// $Log: i_net.h,v $
// Revision 1.9  2001/02/10 12:27:13  bpereira
// Revision 1.8  2000/10/16 20:02:29  bpereira
// Revision 1.7  2000/09/10 10:40:06  metzgermeister
// Revision 1.6  2000/09/01 19:34:37  bpereira
//
// Revision 1.5  2000/09/01 18:23:42  hurdler
// fix some issues with latest network code changes
//
// Revision 1.4  2000/08/31 14:30:55  bpereira
// Revision 1.3  2000/04/16 18:38:07  bpereira
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      System specific network interface stuff.
//
//-----------------------------------------------------------------------------

#ifndef I_NET_H
#define I_NET_H

#include <stdint.h>

#ifdef __GNUG__
#pragma interface
#endif

#define DOOMCOM_ID       0x12345678l

#define MAXPACKETLENGTH  1450 // For use in a LAN
#define INETPACKETLENGTH 512 // For use on the internet

extern uint16_t  hardware_MAXPACKETLENGTH;
extern uint32_t  net_bandwidth; // in byte/s

// [WDJ] Can simplify doomcom when drop support for DOS net.
// Fixed as stdint sizes.
typedef struct
{
    // Supposed to be DOOMCOM_ID
    uint32_t            id;

    // (DOSNET) DOOM executes an int to execute commands.
    int16_t             intnum;
    // (DOSNET) Communication between DOOM and the driver.
    // Is CMD_SEND or CMD_GET.
    uint16_t            command;
    // Is dest for send, set by get (-1 = no packet).
    int16_t             remotenode;

    // Number of bytes in doomdata to be sent
    uint16_t            datalength;

    // Info common to all nodes.
    // Console is always node 0.
    uint16_t            numnodes;
    // Flag: 1 = no duplication, 2-5 = dup for slow nets.
    uint16_t            unused_ticdup;
    // Number of extratics in each packet.
    uint16_t            extratics;
    // deathmatch type 0=coop, 1=deathmatch 1 ,2 = deathmatch 2.
    uint16_t            unused_deathmatch;
    // Flag: -1 = new game, 0-5 = load savegame
    int16_t             unused_savegame;
    int16_t             unused_episode;        // 1-3
    int16_t             unused_map;            // 1-9
    int16_t             unused_skill;          // 1-5

    // Info specific to this node.
    int16_t             consoleplayer;
    // Number total of players
    uint16_t            numplayers;

    // These are related to the 3-display mode,
    //  in which two drones looking left and right
    //  were used to render two additional views
    //  on two additional computers.
    // Probably not operational anymore. (maybe a day in Legacy)
    // 1 = left, 0 = center, -1 = right
    int16_t             unused_angleoffset;
    // 1 = drone
    uint16_t            unused_drone;

    // The packet data to be sent.
    char                data[MAXPACKETLENGTH];

} doomcom_t;

extern doomcom_t *doomcom;
// Called by D_DoomMain.

// to be defined by the network driver
extern void    (*I_NetGet) (void);                // return packet in doomcom struct
extern void    (*I_NetSend) (void);               // send packet within doomcom struct
extern boolean (*I_NetCanSend) (void);            // ask to driver if all is ok to send data now
extern void    (*I_NetFreeNodenum) (int nodenum); // close a connection 
extern int     (*I_NetMakeNode) (char *address);  // open a connection with specified address
extern boolean (*I_NetOpenSocket) (void);         // opend all connections
extern void    (*I_NetCloseSocket) (void);        // close all connections no more allow geting any packet 

boolean I_InitNetwork (void);

#endif
