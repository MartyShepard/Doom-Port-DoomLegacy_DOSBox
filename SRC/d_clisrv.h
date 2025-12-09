// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: d_clisrv.h 1144 2015-04-03 13:46:21Z wesleyjohnson $
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
// $Log: d_clisrv.h,v $
// Revision 1.25  2003/07/13 13:16:15  hurdler
// Revision 1.24  2002/07/26 15:21:36  hurdler
//
// Revision 1.23  2001/12/31 12:30:11  metzgermeister
// fixed buffer overflow
//
// Revision 1.22  2001/11/17 22:12:53  hurdler
// Revision 1.21  2001/08/20 20:40:39  metzgermeister
//
// Revision 1.20  2001/05/16 17:12:52  crashrl
// Added md5-sum support, removed recursiv wad search
//
// Revision 1.19  2001/03/30 17:12:49  bpereira
// Revision 1.18  2001/02/19 18:00:49  hurdler
// Revision 1.17  2001/02/10 12:27:13  bpereira
// Revision 1.16  2000/11/11 13:59:45  bpereira
//
// Revision 1.15  2000/11/02 17:50:06  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.14  2000/10/22 00:20:53  hurdler
// Updated for the latest master server code
//
// Revision 1.13  2000/10/21 08:43:28  bpereira
// Revision 1.12  2000/10/16 20:02:29  bpereira
// Revision 1.11  2000/10/08 13:29:59  bpereira
// Revision 1.10  2000/09/28 20:57:14  bpereira
// Revision 1.9  2000/09/10 10:37:28  metzgermeister
// Revision 1.8  2000/08/31 14:30:55  bpereira
// Revision 1.7  2000/04/30 10:30:10  bpereira
// Revision 1.6  2000/04/24 20:24:38  bpereira
//
// Revision 1.5  2000/04/19 10:56:51  hurdler
// commited for exe release and tag only
//
// Revision 1.4  2000/04/16 18:38:06  bpereira
// Revision 1.3  2000/04/06 20:32:26  hurdler
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      high level networking stuff
//
//-----------------------------------------------------------------------------

#ifndef D_CLISRV_H
#define D_CLISRV_H

#include <stddef.h>

#include "doomtype.h"
#include "d_ticcmd.h"
#include "d_netcmd.h"
#include "tables.h"

//
// Network play related stuff.
// There is a data struct that stores network
//  communication related stuff, and another
//  one that defines the actual packets to
//  be transmitted.
//

// Networking and tick handling related.
#define BACKUPTICS            32
#define DRONE               0x80    // bit set in consoleplayer

#define MAXTEXTCMD           256
//
// Packet structure
//
// Index to packettypename[]
typedef enum   {
    PT_NOTHING,       // to send a nop through network :)
 // High priority
    PT_SERVERCFG,     // server config used in start game (stay 1 for backward compatibility issue)
                      // this is positive response to CLIENTJOIN request
    PT_CLIENTCMD,     // ticcmd of the client
    PT_CLIENTMIS,     // same as above but saying resend from
    PT_CLIENT2CMD,    // with player 2, ticcmd of the client
    PT_CLIENT2MIS,    // with player 2, same as above but saying resend from
    PT_NODEKEEPALIVE, // same but without ticcmd and consistancy
    PT_NODEKEEPALIVEMIS,
    PT_SERVERTICS,    // all cmd for the tic
    PT_SERVERREFUSE,  // server refuse joiner (reason inside)
    PT_SERVERSHUTDOWN,// server is shutting down
    PT_CLIENTQUIT,    // client close the connection
                      
    PT_ASKINFO,       // to ask info of the server (anyone)
    PT_SERVERINFO,    // send game & server info (gamespy)
    PT_REQUESTFILE,   // client request a file transfer

 // Low Priority
    PT_CANFAIL,       // A priority boundary
                      // This packet can't occupy all slots.
 // with HSendPacket(,true,,) these can return false
    PT_FILEFRAGMENT=PT_CANFAIL, // a part of a file
    PT_TEXTCMD,       // extra text command from the client
    PT_TEXTCMD2,      // extra text command from the client (splitscreen)
    PT_CLIENTJOIN,    // client want to join used in start game
    PT_NODE_TIMEOUT,  // packet is sent to self when connection timeout
 // count for table
    NUMPACKETTYPE
} packettype_t;

//#pragma pack(1)

// client to server packet
typedef struct {
   byte        client_tic;
   byte        resendfrom;
   int16_t     consistency;
   ticcmd_t    cmd;
} clientcmd_pak;

// splitscreen packet
// WARNING : must have the same format of clientcmd_pak, for more easy use
typedef struct {
   byte        client_tic;
   byte        resendfrom;
   int16_t     consistency;
   ticcmd_t    cmd;
   ticcmd_t    cmd2;
} client2cmd_pak;

// Server to client packet
// this packet is too large !!!!!!!!!
typedef struct {
   byte        starttic;
   byte        numtics;
   byte        numplayers;
   ticcmd_t    cmds[45]; // normaly [BACKUPTIC][MAXPLAYERS] but too large
//   char        textcmds[BACKUPTICS][MAXTEXTCMD];
} servertics_pak;

typedef struct {
   byte        version;    // exe from differant version don't work
   uint32_t    subversion; // contain build version and maybe crc

   // server lunch stuffs
   byte        serverplayer;
   byte        totalplayernum;
   tic_t       gametic;
   byte        clientnode;
   byte        gamestate;
   
   uint32_t    playerdetected; // playeringame vector in bit field
   byte        netcvarstates[0];
} serverconfig_pak;

typedef struct {
   byte        version;    // exe from differant version don't work
   uint32_t    subversion; // contain build version and maybe crc
   byte        localplayers;
   byte        mode;
} clientconfig_pak;

typedef struct {
   char        fileid;
   uint32_t    position;
   uint16_t    size;
   byte        data[100];  // size is variable using hardare_MAXPACKETLENGTH
} filetx_pak;

#define MAXSERVERNAME 32
typedef struct {
    byte       version;
    ULONG      subversion;
    byte       numberofplayer;
    byte       maxplayer;
    byte       deathmatch;
    tic_t      trip_time;   // askinfo time in packet, ping time in list
    float      load;        // unused for the moment
    char       mapname[8];
    char       servername[MAXSERVERNAME];
    byte       num_fileneed;
    byte       fileneed[4096];   // is filled with writexxx (byteptr.h)
} serverinfo_pak;

#define MAXSERVERLIST 32  // limited by the display
typedef struct { 
    serverinfo_pak info;
    byte  server_node;  // network node this server is on
} server_info_t;

extern server_info_t  serverlist[MAXSERVERLIST];
extern int serverlistcount;


typedef struct {
   byte        version;
   tic_t       send_time;      // used for ping evaluation
} askinfo_pak;

typedef struct {
    char       reason[255];
} serverrefuse_pak;


//
// Network packet data.
//
typedef struct
{                
    uint32_t   checksum;
    byte       ack_req;       // Ask for an acknowlegement with this ack num.
   			      // 0= no ack
    byte       ack_return;    // Return the ack number of a packet.
                              // 0= no ack

    byte       packettype;
    byte       reserved;      // padding
    union  {   clientcmd_pak     clientpak;
               client2cmd_pak    client2pak;
               servertics_pak    serverpak;
               serverconfig_pak  servercfg;
               byte              textcmd[MAXTEXTCMD+1];
               filetx_pak        filetxpak;
               clientconfig_pak  clientcfg;
               serverinfo_pak    serverinfo;
               serverrefuse_pak  serverrefuse;
               askinfo_pak       askinfo;
           } u;

} netbuffer_t;

//#pragma pack()

// points inside doomcom
extern  netbuffer_t*   netbuffer;        

extern consvar_t cv_playdemospeed;

//#define PACKET_BASE_SIZE     ((int)&( ((netbuffer_t *)0)->u))
#define PACKET_BASE_SIZE     offsetof(netbuffer_t, u)
//#define FILETX_HEADER_SIZE       ((int)   ((filetx_pak *)0)->data)
#define FILETX_HEADER_SIZE   offsetof(filetx_pak, data)
//#define SERVER_TIC_BASE_SIZE  ((int)&( ((netbuffer_t *)0)->u.serverpak.cmds[0]))
#define SERVER_TIC_BASE_SIZE offsetof(netbuffer_t, u.serverpak.cmds[0])

extern boolean   server;
extern uint16_t  software_MAXPACKETLENGTH;
extern boolean   acceptnewnode;
extern byte      servernode;  // the server net node, 255=none

extern boolean   cl_drone;  // is a drone client

extern consvar_t cv_allownewplayer;
extern consvar_t cv_maxplayers;

// Used in d_net, the only dependence.
int     ExpandTics (int low);
void    D_ClientServerInit (void);

// initialise the other field
void    Register_NetXCmd(netxcmd_t cmd_id,
			void (*cmd_f) (char **p, int playernum));
void    Send_NetXCmd(byte cmd_id, void *param, int nparam);
void    Send_NetXCmd2(byte cmd_id, void *param, int nparam); // splitsreen player

// Create any new ticcmds and broadcast to other players.
void    NetUpdate (void);
void    D_PredictPlayerPosition(void);

boolean SV_AddWaitingPlayers(void);
void    SV_StartSinglePlayerServer(void);
boolean SV_SpawnServer( void );
void    SV_SpawnPlayer(int playernum, int x, int y, angle_t angle);
void    SV_StopServer( void );
void    SV_ResetServer( void );

void    CL_AddSplitscreenPlayer( void );
void    CL_RemoveSplitscreenPlayer( void );
void    CL_Reset (void);
void    CL_UpdateServerList( boolean internetsearch );
// is there a game running
boolean Game_Playing( void );


// Broadcasts special packets to other players
// to notify of game exit.
void    D_Quit_NetGame (void);

// How many ticks to run.
void    TryRunTics (tic_t realtic);

// extra data for lmps
boolean AddLmpExtradata(byte **demo_p,int playernum);
void    ReadLmpExtraData(byte **demo_pointer,int playernum);

// translate a playername in a player number return -1 if not found and
// print a error message in the console
int     nametonum(char *name);

#endif
