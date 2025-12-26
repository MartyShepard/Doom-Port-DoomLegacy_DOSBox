// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: i_net.h 1377 2017-12-18 17:29:50Z wesleyjohnson $
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
extern uint32_t  net_bandwidth; // in byte/sec

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
    // Index to net_nodes:
    // Send: to node
    // Get:  from node (set to -1 = no packet).
    int16_t             remotenode;

    // Number of bytes in doomdata to be sent
    uint16_t            datalength;

    // Info common to all nodes.
    // Console is always node 0.
    uint16_t            num_player_netnodes;
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

// Report network errors with global because so many callers ignore it,
// and it is difficult to pass back up through so many layers.
// This also allows for simpler error returns in the functions.
typedef enum {
   NE_success = 0,
   NE_empty = -1,
   // some doomatic errors
   NE_congestion = -401,
   NE_network_reset = -402,
   NE_network_unreachable = -403,
   NE_unknown_net_error = -404,
   NE_node_unconnected = -405,
   NE_nodes_exhausted = -406,
   NE_not_netgame = -410,
   NE_fail = -499
} network_error_e;

// This is only set on error, it does not indicate success.
// If a test of success is necessary, clear it before the network call.
extern network_error_e  net_error;

// Indirections, to be instantiated by the network driver
// Return packet into doomcom struct.
// Return true when got packet.  Error in net_error.
extern boolean (*I_NetGet) (void);
// Send packet from within doomcom struct.
// Return true when packet has been sent.  Error in net_error.
extern boolean (*I_NetSend) (void);
// Return true if network is ready to send.
extern boolean (*I_NetCanSend) (void);
// Close the net node connection.
extern void    (*I_NetFreeNode) (int nodenum);
// Open a net node connection with a specified address.
// Return the net node number, or network_error_e.  Error in net_error.
extern int     (*I_NetMakeNode) (char *address);
// Open the network socket.
// Return true if the socket is open.
extern boolean (*I_NetOpenSocket) (void);
// Close the network socket, and all net node connections.
extern void    (*I_NetCloseSocket) (void);

// Set address and port of special nodes.
//  saddr: IP address in network byte order
//  port: port number in host byte order
void UDP_Bind_Node( int nnode, unsigned int saddr, uint16_t port );

// Bind an inet or ipx address string to a net node.
boolean  Bind_Node_str( int nnode, char * addrstr, uint16_t port );


boolean I_InitNetwork (void);

#endif
