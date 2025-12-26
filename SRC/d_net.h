// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: d_net.h 1377 2017-12-18 17:29:50Z wesleyjohnson $
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
// $Log: d_net.h,v $
// Revision 1.5  2001/02/10 12:27:13  bpereira
// Revision 1.4  2000/09/15 19:49:22  bpereira
// Revision 1.3  2000/08/31 14:30:55  bpereira
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Networking stuff.
//      Part of layer 4 (transport) (tp4) of the osi model.
//      Assure the reception of packet and process checksums.
//
//-----------------------------------------------------------------------------

#ifndef D_NET_H
#define D_NET_H

#include "doomtype.h"

//
// Network play related stuff.
// There is a data struct that stores network
//  communication related stuff, and another
//  one that defines the actual packets to
//  be transmitted.
//

// Max player computers in a game. Limited to 127 by chat.
#define MAXNETNODES     32
// Broadcast added xx/5/99: can use broadcast now
#define BROADCAST_NODE   MAXNETNODES
// MasterServer Ping
#define MS_PINGNODE     (MAXNETNODES+1)
// Node connection storage, and send. Limited to 254 (byte).
#define MAX_CON_NETNODE (MAXNETNODES+2)

// Maximum number of players on a single computer.
#define MAXSPLITSCREENPLAYERS   2

// The periodic updating of net stats.
#define STAT_PERIOD  (TICRATE*2)

// NetStat globals
extern int    netstat_recv_bps, netstat_send_bps;
extern float  netstat_lost_percent, netstat_dup_percent;
extern float  netstat_gamelost_percent;

boolean Net_GetNetStat(void);

extern int    net_packetheader_length;
extern int       stat_getbytes;
extern uint64_t  stat_sendbytes;        // realtime updated 

void    Net_AckTicker(void);
boolean Net_AllAckReceived(void);

// if reliable return true if packet sent, 0 else
boolean HSendPacket(int to_node, boolean reliable, byte acknum, int packetlength);
boolean HGetPacket (void);
// Returns true when a network connection is made.
boolean D_Startup_NetGame(void);
void    D_CloseConnection( void );
void    Net_Cancel_Packet_Ack(int nnode);
void    Net_CloseConnection(int nnode);
void    Net_AbortPacketType(byte packettype);
void    Net_Send_AcksPacket(int to_node);
void    Net_Wait_AllAckReceived( uint32_t timeout );
#endif
