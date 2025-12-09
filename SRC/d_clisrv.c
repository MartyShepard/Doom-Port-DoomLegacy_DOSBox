// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: d_clisrv.c 1143 2015-04-03 13:44:42Z wesleyjohnson $
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
// $Log: d_clisrv.c,v $
// Revision 1.47  2004/07/27 08:19:34  exl
// New fmod, fs functions, bugfix or 2, patrol nodes
//
// Revision 1.46  2004/04/20 00:34:26  andyp
// Linux compilation fixes and string cleanups
//
// Revision 1.45  2003/11/22 00:22:08  darkwolf95
// get rid of FS hud pics on level exit and new game, also added exl's fix for clearing hub variables on new game
//
// Revision 1.44  2003/05/04 04:30:30  sburke
// Ensure that big-endian machines encode/decode network packets as little-endian.
//
// Revision 1.43  2003/03/22 22:35:59  hurdler
//
// Revision 1.42  2002/09/27 16:40:08  tonyd
// First commit of acbot
//
// Revision 1.41  2001/08/20 20:40:39  metzgermeister
// Revision 1.40  2001/06/10 21:16:01  bpereira
//
// Revision 1.39  2001/05/16 17:12:52  crashrl
// Added md5-sum support, removed recursiv wad search
//
// Revision 1.38  2001/05/14 19:02:57  metzgermeister
//   * Fixed floor not moving up with player on E3M1
//   * Fixed crash due to oversized string in screen message ... bad bug!
//   * Corrected some typos
//   * fixed sound bug in SDL
//
// Revision 1.37  2001/04/27 13:32:13  bpereira
// Revision 1.36  2001/04/01 17:35:06  bpereira
// Revision 1.35  2001/03/30 17:12:49  bpereira
// Revision 1.34  2001/03/03 06:17:33  bpereira
// Revision 1.33  2001/02/24 13:35:19  bpereira
// Revision 1.32  2001/02/10 12:27:13  bpereira
//
// Revision 1.31  2001/01/25 22:15:41  bpereira
// added heretic support
//
// Revision 1.30  2000/11/11 13:59:45  bpereira
//
// Revision 1.29  2000/11/02 17:50:06  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.28  2000/10/22 00:20:53  hurdler
// Updated for the latest master server code
//
// Revision 1.27  2000/10/21 23:21:56  hurdler
// Revision 1.26  2000/10/21 08:43:28  bpereira
//
// Revision 1.25  2000/10/17 10:09:27  hurdler
// Update master server code for easy connect from menu
//
// Revision 1.24  2000/10/16 20:02:28  bpereira
// Revision 1.23  2000/10/08 13:29:59  bpereira
// Revision 1.22  2000/10/01 10:18:16  bpereira
// Revision 1.21  2000/09/28 20:57:14  bpereira
// Revision 1.20  2000/09/15 19:49:21  bpereira
// Revision 1.19  2000/09/10 10:37:28  metzgermeister
// Revision 1.18  2000/09/01 19:34:37  bpereira
// Revision 1.17  2000/08/31 14:30:55  bpereira
//
// Revision 1.16  2000/08/21 11:06:43  hurdler
// Add ping and some fixes
//
// Revision 1.15  2000/08/16 15:44:18  hurdler
// update master server code
//
// Revision 1.14  2000/08/16 14:10:01  hurdler
// add master server code
//
// Revision 1.13  2000/08/11 19:10:13  metzgermeister
//
// Revision 1.12  2000/08/11 12:25:23  hurdler
// latest changes for v1.30
//
// Revision 1.11  2000/08/03 17:57:41  bpereira
// Revision 1.10  2000/04/30 10:30:10  bpereira
// Revision 1.9  2000/04/24 20:24:38  bpereira
// Revision 1.8  2000/04/16 18:38:06  bpereira
//
// Revision 1.7  2000/04/04 00:32:45  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.6  2000/03/29 19:39:48  bpereira
//
// Revision 1.5  2000/03/08 17:02:42  hurdler
// fix the joiningame problem under Linux
//
// Revision 1.4  2000/03/06 16:51:08  hurdler
// hack for OpenGL / Open Entry problem
//
// Revision 1.3  2000/02/27 16:30:28  hurdler
// dead player bug fix + add allowmlook <yes|no>
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      DOOM Network game communication and protocol,
//      all OS independend parts.
//
//-----------------------------------------------------------------------------


#include <time.h>
#include <unistd.h>

#include "doomincl.h"
#include "d_clisrv.h"
#include "command.h"
#include "i_net.h"
#include "i_system.h"
#include "i_video.h"
#include "d_net.h"
#include "d_netcmd.h"
#include "d_main.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "keys.h"
#include "doomstat.h"
#include "m_argv.h"
#include "m_menu.h"
#include "console.h"
#include "d_netfil.h"
#include "byteptr.h"

#include "p_saveg.h"
#include "p_setup.h"
#include "z_zone.h"
#include "p_tick.h"
#include "p_local.h"
#include "m_misc.h"
#include "am_map.h"
#include "m_random.h"
#include "mserv.h"
#include "t_vari.h"

#include "b_game.h"	//added by AC for acbot

//
// NETWORKING
//
// gametic is the tic about to (or currently being) run
// maketic is the tic that hasn't had control made for it yet
// server:
//   nettics is the tic for each node
//   next_tic_send is the lowest value of nettics
// client:
//   cl_need_tic is the tic needed by the client for run the game
//   next_tic_send is used to optimize a condition
// normaly maketic>=gametic>0,


const int  NETWORK_VERSION = 21; // separate version number for network protocol (obsolete)



#define PREDICTIONQUEUE         BACKUPTICS
#define PREDICTIONMASK          (PREDICTIONQUEUE-1)

// Server state
boolean         server = true;           // true or false but !server=client
boolean         serverrunning = false;
byte            serverplayer;  // 255= no server player (same as -1)

// Server specific vars.
// player=255 when unused
// nodeingame[]=false when net node is unused
static byte     player_to_nnode[MAXPLAYERS];

static tic_t    cl_maketic[MAXNETNODES];  // unused

// Server net node state
static byte     nnode_to_player[MAXNETNODES];  // 255= unused
static byte     nnode_to_player2[MAXNETNODES]; // splitscreen player, 255= unused
static byte     playerpernode[MAXNETNODES]; // used specialy for splitscreen
static byte     nodewaiting[MAXNETNODES];
static boolean  nodeingame[MAXNETNODES];  // set false as nodes leave game
static tic_t    nettics[MAXNETNODES];     // what tic the client have received
static tic_t    nextsend_tic[MAXNETNODES]; // what server sent to client

static tic_t    next_tic_send;     // min of the nettics
static tic_t    next_tic_clear=0;  // clear next_tic_clear to next_tic_send
static tic_t    maketic;
static int16_t  consistency[BACKUPTICS];
#ifdef CLIENTPREDICTION2
tic_t localgametic;
#endif

// Client specific.
boolean         cl_drone; // client displays, no commands
static byte     cl_nnode; // net node for this client, pointofview server
static boolean  cl_packetmissed;
static tic_t    cl_need_tic;

char            servernode;       // the number of the server node

// Index for netcmds and textcmds
#define BTIC_INDEX( tic )  ((tic)%BACKUPTICS)
// Text buffer for textcmds.
typedef struct {
   byte  buff[MAXTEXTCMD];
} textbuf_t;
  // The first byte of buff is the length.

static ticcmd_t localcmds;
static textbuf_t  localtextcmd;
static ticcmd_t localcmds2;  // player 2
static textbuf_t  localtextcmd2; // splitscreen player2

// engine
ticcmd_t        netcmds[BACKUPTICS][MAXPLAYERS];
static textbuf_t  textcmds[BACKUPTICS][MAXPLAYERS];
  // The first byte of textcmd is the length.

consvar_t cv_playdemospeed  = {"playdemospeed","0",0,CV_Unsigned};


// some software don't support largest packet
// (original sersetup, not exactly, but the probability of sending a packet
// of 512 octet is like 0.1)
uint16_t  software_MAXPACKETLENGTH;

// By Client, Server
int ExpandTics (int low)
{
    int delta;

    delta = low - (maketic&0xff);

    if (delta >= -64 && delta <= 64)
        return (maketic&~0xff) + low;
    if (delta > 64)
        return (maketic&~0xff) - 256 + low;
    if (delta < -64)
        return (maketic&~0xff) + 256 + low;
#ifdef PARANOIA
    I_Error ("ExpandTics: strange value %i at maketic %i",low,maketic);
#endif
    return 0;
}

// -----------------------------------------------------------------
//  Some extra data function for handle textcmd buffer
// -----------------------------------------------------------------

static void (*netxcmd_func[MAXNETXCMD])(char **p,int playernum);

void Register_NetXCmd(netxcmd_t cmd_id, void (*cmd_f) (char **p,int playernum))
{
#ifdef PARANOIA
   if(cmd_id >= MAXNETXCMD)
      I_Error("command id %d too big", cmd_id);
   if(netxcmd_func[cmd_id]!=0)
      I_Error("Command id %d already used", cmd_id);
#endif
   netxcmd_func[cmd_id] = cmd_f;
}

void Send_NetXCmd(byte cmd_id, void *param, int nparam)
{
   if(demoplayback)
       return;

   int textlen = localtextcmd.buff[0];
   if( (textlen + 1 + nparam) > MAXTEXTCMD)
   {
#ifdef PARANOIA
       I_Error("Net command exceeds buffer size: netcmd %d\n", cmd_id);
#else
       CONS_Printf("\2Net Command exceeds buffer\n");
#endif
       return;
   }
   // Append
   textlen++;
   localtextcmd.buff[textlen] = cmd_id;
   if(param && nparam)
   {
       memcpy(&localtextcmd.buff[textlen+1], param, nparam);
       textlen += nparam;
   }
   localtextcmd.buff[0] = textlen;
}

// splitscreen player
void Send_NetXCmd2(byte cmd_id, void *param, int nparam)
{
   if(demoplayback)
       return;

   int textlen = localtextcmd2.buff[0];
   if( (textlen + 1 + nparam) > MAXTEXTCMD)
   {
#ifdef PARANOIA
       I_Error("Net command exceeds buffer size: netcmd %d\n", cmd_id);
#else
       CONS_Printf("\2Net Command fail\n");
#endif
       return;
   }
   // Append
   textlen++;
   localtextcmd2.buff[textlen] = cmd_id;
   if(param && nparam)
   {
       memcpy(&localtextcmd2.buff[textlen+1], param, nparam);
       textlen += nparam;
   }
   localtextcmd2.buff[0] = textlen;
}


static void D_Clear_ticcmd(int tic)
{
    int i;
    int btic = BTIC_INDEX( tic );

    for(i=0;i<MAXPLAYERS;i++)
    {
        textcmds[btic][i].buff[0]=0;  // textlen
        netcmds[btic][i].angleturn = 0; //&= ~TICCMD_RECEIVED;
    }
    DEBFILE(va("clear tic %5d (%2d)\n", tic, btic));
}


static void ExtraDataTicker(void)
{
    int  i;
    byte *curpos, *bufferend;
    int btic = BTIC_INDEX( gametic );
    textbuf_t * textbuf;
    int textlen;

    for(i=0;i<MAXPLAYERS;i++)
    {
        if((playeringame[i]) || (i==0))
        {
	    textbuf = & textcmds[btic][i];
	    textlen = textbuf->buff[0];
            curpos=(byte *)&(textbuf->buff[1]);  // start of text
	    // [WDJ] need check for buffer overrun here !!
            bufferend = & textbuf->buff[textlen+1];  // end of text
            while(curpos<bufferend)
            {
                if(*curpos < MAXNETXCMD && netxcmd_func[*curpos])
                {
		    // Execute a NetXCmd.
                    byte cmd_id=*curpos;
                    curpos++;
                    DEBFILE(va("executing x_cmd %d ply %d ", cmd_id,i));
                    (netxcmd_func[cmd_id])((char **)&curpos,i);
                    DEBFILE("done\n");
                }
                else
	        {
		    // [WDJ] Why should a bad demo command byte be fatal.
                    I_SoftError("Got unknown net/demo command [%d]=%d (max %d)\n",
                           (curpos - &(textbuf->buff[0])),
                           *curpos, textlen);
		    D_Clear_ticcmd(btic);
		    return;
		}
            }
        }
    }
}


// -----------------------------------------------------------------
//  end of extra data function
// -----------------------------------------------------------------

// -----------------------------------------------------------------
//  extra data function for lmps
// -----------------------------------------------------------------

// desciption of extradate byte of LEGACY 1.12 not the same of the 1.20
// 1.20 don't have the extradata bits fields but a byte for each command
// see XD_xxx in d_netcmd.h
//
// if extradatabit is set, after the ziped tic you find this :
//
//   type   |  description
// ---------+--------------
//   byte   | size of the extradata
//   byte   | the extradata (xd) bits : see XD_...
//            with this byte you know what parameter follow
// if(xd & XDNAMEANDCOLOR)
//   byte   | color
//   char[MAXPLAYERNAME] | name of the player
// endif
// if(xd & XD_WEAPON_PREF)
//   byte   | original weapon switch : boolean, true if use the old
//          | weapon switch methode
//   char[NUMWEAPONS] | the weapon switch priority
//   byte   | autoaim : true if use the old autoaim system
// endif
boolean AddLmpExtradata(byte **demo_point, int playernum)
{
    int  btic = BTIC_INDEX( gametic );
    textbuf_t * textbuf = & textcmds[btic][playernum];
    int  textlen = textbuf->buff[0];

    if(textlen == 0)  // anything in the buffer
        return false;

    memcpy(*demo_point, textbuf->buff, textlen+1);
    *demo_point += textlen+1;
    return true;
}

void ReadLmpExtraData(byte **demo_pointer, int playernum)
{
    unsigned char nextra,ex;
    int btic = BTIC_INDEX( gametic );
    textbuf_t * textbuf = & textcmds[btic][playernum];

    if(!demo_pointer)
    {
        textbuf->buff[0] = 0;  // text len
        return;
    }
    nextra=**demo_pointer;
    if(demoversion==112) // support old demos v1.12
    {
        int  textlen = 0;
        byte  *p = *demo_pointer+1; // skip nextra

        ex=*p++;
        if(ex & 1)
        {
            textbuf->buff[textlen+1] = XD_NAMEANDCOLOR;
            memcpy(&textbuf->buff[textlen+2],
                   p,
                   MAXPLAYERNAME+1);
            p+=MAXPLAYERNAME+1;
	    textlen += MAXPLAYERNAME+1+1;
        }
        if(ex & 2)
        {
            textbuf->buff[textlen+1] = XD_WEAPONPREF;
            memcpy(&textbuf->buff[textlen+2],
                   p,
                   NUMWEAPONS+2);
            p+=NUMWEAPONS+2;
            textlen += NUMWEAPONS+2+1;
        }
        textbuf->buff[0] = textlen;
        nextra--;
    }
    else
        memcpy(textbuf->buff, *demo_pointer, nextra+1);
    // increment demo pointer
    *demo_pointer +=nextra+1;
}

// -----------------------------------------------------------------
//  end extra data function for lmps
// -----------------------------------------------------------------

// ----- Server/Client Responses

// Client state
typedef enum {
   CLM_searching,
   CLM_downloadfiles,
   CLM_askjoin,
   CLM_waitjoinresponse,
   CLM_downloadsavegame,
   CLM_connected
} cl_mode_t;

static cl_mode_t  cl_mode = CLM_searching;

static int16_t  Consistency(void);
static void Net_Packet_Handler(void);
void SV_ResetServer( void );


// By Client.
// Send a request to join game.
// Called by CL_ConnectToServer.
static boolean  CL_Send_Join( void )
{
    CONS_Printf("Send join request...\n");
    netbuffer->packettype=PT_CLIENTJOIN;

    // Declare how many players at this node.
    if (cl_drone)
        netbuffer->u.clientcfg.localplayers=0;
    else
    if (cv_splitscreen.value)
        netbuffer->u.clientcfg.localplayers=2;
    else
        netbuffer->u.clientcfg.localplayers=1;

    netbuffer->u.clientcfg.version = VERSION;
    netbuffer->u.clientcfg.subversion = LE_SWAP32_FAST(NETWORK_VERSION);

    return HSendPacket(servernode,true,0,sizeof(clientconfig_pak));
}


// By Server.
// Reply to request for server info.
//   reqtime : the send time of the request
static void SV_Send_ServerInfo(int to_node, tic_t reqtime)
{
    byte  *p;

    netbuffer->packettype=PT_SERVERINFO;
    netbuffer->u.serverinfo.version = VERSION;
    netbuffer->u.serverinfo.subversion = LE_SWAP32_FAST(NETWORK_VERSION);
    // return back the time value so client can compute there ping
    netbuffer->u.serverinfo.trip_time = LE_SWAP32_FAST(reqtime);
    netbuffer->u.serverinfo.numberofplayer = doomcom->numplayers;
    netbuffer->u.serverinfo.maxplayer = cv_maxplayers.value;
    netbuffer->u.serverinfo.load = 0;        // unused for the moment
    netbuffer->u.serverinfo.deathmatch = cv_deathmatch.value;
    strncpy(netbuffer->u.serverinfo.servername, cv_servername.string, MAXSERVERNAME);
    if(gamemapname[0])
        strcpy(netbuffer->u.serverinfo.mapname,gamemapname);
    else
        strcpy(netbuffer->u.serverinfo.mapname,G_BuildMapName(gameepisode,gamemap));

    p=Put_Server_FileNeed();

    HSendPacket(to_node, false, 0, p-((byte *)&netbuffer->u));
}


// By Server.
// Accept player joining the game.
static boolean SV_Send_ServerConfig(int to_node)
{
    int   i,playermask=0;
    byte  *p;

    netbuffer->packettype=PT_SERVERCFG;
    for(i=0;i<MAXPLAYERS;i++)
    {
         if(playeringame[i])
              playermask|=1<<i;
    }

    netbuffer->u.servercfg.version         = VERSION;
    netbuffer->u.servercfg.subversion      = LE_SWAP32_FAST(NETWORK_VERSION);

    netbuffer->u.servercfg.serverplayer    = serverplayer;
    netbuffer->u.servercfg.totalplayernum  = doomcom->numplayers;
    netbuffer->u.servercfg.playerdetected  = LE_SWAP32_FAST(playermask);
    netbuffer->u.servercfg.gametic         = LE_SWAP32_FAST(gametic);
    netbuffer->u.servercfg.clientnode      = to_node;
    netbuffer->u.servercfg.gamestate       = gamestate;
    p = netbuffer->u.servercfg.netcvarstates;
    CV_SaveNetVars((char**)&p);

    return HSendPacket(to_node, true, 0,
	   sizeof(serverconfig_pak)+(p - netbuffer->u.servercfg.netcvarstates));
}

#define JOININGAME
#ifdef JOININGAME

// By Server.
// Send a save game to the client.
static void SV_Send_SaveGame(int to_node)
{
    size_t  length;

    P_Alloc_savebuffer( 1 );	// large buffer, but no header
    if(! savebuffer)   return;
    // No savegame header
   
    P_SaveGame();  // fill buffer with game data
    // buffer will automatically grow as needed.

    length = P_Savegame_length();
    if( length < 0 )   return;	// overrun buffer
   
    // then send it !
    SendData(to_node, savebuffer, length, TAH_MALLOC_FREE, 0);
    // SendData frees the savebuffer using free() after it is sent.
    // This is the only use of TAH_MALLOC_FREE.
}

static const char *tmpsave="$$$.sav";

// By Client.
// Act upon the received save game from server.
static void CL_Load_Received_Savegame(void)
{
    // Use savebuffer and save_p from p_saveg.c.
    // There cannot be another savegame in progress when this occurs.
    // [WDJ] Changed to use new load savegame file, with smaller buffer.
    if( P_Savegame_Readfile( tmpsave ) < 0 )  goto cannot_read_file;
    // file is open and savebuffer allocated
    // No Header on network sent savegame

    CONS_Printf("loading savegame\n");

    G_Downgrade (VERSION);

    paused        = false;
    demoplayback  = false;
    automapactive = false;

    // load a base level
    playerdeadview = false;

    P_LoadGame(); // read game data in savebuffer, defer error test
    if( P_Savegame_Closefile( 0 ) < 0 )  goto load_failed;
    // savegame buffer deallocated, and file closed

    // done
    unlink(tmpsave);  // delete file
    consistency[ BTIC_INDEX( gametic ) ] = Consistency();
    CON_ToggleOff ();
    return;

cannot_read_file:
    I_SoftError ("Can't read savegame sent");
    goto failed_exit; // must deallocate savebuffer

load_failed:
    CONS_Printf("Can't load the level !!!\n");
failed_exit:
    // needed when there are error tests before Closefile.
    P_Savegame_Error_Closefile();  // deallocate savebuffer
    return;
}


#endif

// By Client.
// Ask the server for some info.
//   to_node : when BROADCASTADDR then all servers will respond
static void CL_Send_AskInfo( int to_node )
{
    netbuffer->packettype = PT_ASKINFO;
    netbuffer->u.askinfo.version = VERSION;
    netbuffer->u.askinfo.send_time = LE_SWAP32_FAST(I_GetTime());
    HSendPacket(to_node, false, 0, sizeof(askinfo_pak));
}

server_info_t serverlist[MAXSERVERLIST];
int serverlistcount=0;

static void SL_ClearServerList( int connectedserver )
{
    int i;
    for( i=0; i<serverlistcount; i++ )
    {
        if( connectedserver != serverlist[i].server_node )
        {
            Net_CloseConnection(serverlist[i].server_node);
            serverlist[i].server_node = 0;
        }
    }
    serverlistcount = 0;
}

static int SL_SearchServer( byte nnode )
{
    int i;
    for( i=0; i<serverlistcount; i++ )
    {
        if( serverlist[i].server_node == nnode )
            return i;
    }

    return -1;
}

static void SL_InsertServer( serverinfo_pak *info, byte nnode)
{
    tic_t  test_time;
    server_info_t se;
    int i, i2;

    // search if not already on it
    i = SL_SearchServer( nnode );
    if( i==-1 )
    {
        // not found add it
        if( serverlistcount >= MAXSERVERLIST )
            return; // list full
        i=serverlistcount++;  // last entry
    }

    // Update info
    serverlist[i].info = *info;
    serverlist[i].server_node = nnode;

    // List is sorted by trip_time (has been converted to ping time)
    // so move the entry until it is sorted (shortest time to [0]).
    se = serverlist[i];  // this is always the updated entry
    test_time = info->trip_time;
    for(;;) {
        i2 = i;  // prev position of updated entry
        if( i>0
	    && test_time < serverlist[i-1].info.trip_time )
        {
            i--;
        }
        else
        if( (i+1)<serverlistcount
	    && test_time > serverlist[i+1].info.trip_time )
        {
            i++;
        }
        else
	    break;  // done
        serverlist[i2] = serverlist[i];  // move other to prev position
        serverlist[i] = se;  // new position
    }
}

// By user, future Client.
void CL_UpdateServerList( boolean internetsearch )
{
    SL_ClearServerList(0);

    if( !netgame )
    {
        I_NetOpenSocket();
        netgame = true;
        multiplayer = true;
    }
    // Search for local servers.
    CL_Send_AskInfo( BROADCASTADDR );

    if( internetsearch )
    {
        msg_server_t *server_list;
        int  i;

        if( (server_list = GetShortServersList()) )
        {
            for (i=0; server_list[i].header[0]; i++)
            {
                int  node;
                char addr_str[24];

                // insert ip (and optionaly port) in node list
                sprintf(addr_str, "%s:%s", server_list[i].ip, server_list[i].port);
                node = I_NetMakeNode(addr_str);
                if( node < 0 )
                    break; // no more node free
                CL_Send_AskInfo( node );
            }
        }
    }
}

// use adaptive send using net_bandwidth and stat.sendbytes
static void CL_ConnectToServer( void )
{
    int     numnodes,nodewaited=doomcom->numnodes,i;
    boolean waitmore;
    tic_t   asksent;
    tic_t   oldtic;

    cl_mode=CLM_searching;

    CONS_Printf("Press ESC to abort\n");
    if( servernode<0 || servernode>=MAXNETNODES )
        CONS_Printf("Searching the server...\n");
    else
        CONS_Printf("Contacting the server...\n");
    DEBFILE(va("waiting %d nodes\n",doomcom->numnodes));
    gamestate = wipegamestate = GS_WAITINGPLAYERS;

    numnodes=1;
    oldtic=I_GetTime()-1;
    asksent=-TICRATE/2;
    SL_ClearServerList(servernode);
    // loop until connected or user escapes
    do {
        switch(cl_mode) {
            case CLM_searching :
                // serverlist is updated by GetPacket function
                if( serverlistcount>0 )
                {
                    // this can be a response to our broadcast request
                    if( servernode==-1 || servernode>=MAXNETNODES)
                    {
                        i = 0;
                        servernode = serverlist[i].server_node;
                        CONS_Printf("Found, ");
                    }
                    else
                    {
                        i=SL_SearchServer(servernode);
                        if (i<0)
                            break; // the case
                    }
                    CL_Got_Fileneed(serverlist[i].info.num_fileneed,
				    serverlist[i].info.fileneed    );
                    CONS_Printf("Checking files...\n");
                    i = CL_CheckFiles();
                    if( i==2 ) // cannot join for some reason
		        goto reset_to_title_exit;
                    else if( i==1 )
                        cl_mode=CLM_askjoin;
                    else
                    {   // must download something
                        // no problem if can't send packet, we will retry later
                        if( Send_RequestFile() )
                            cl_mode=CLM_downloadfiles;
                    }
                    break;
                }
                // ask the info to the server (askinfo packet)
                if(asksent+TICRATE/2<I_GetTime())
                {
                    CL_Send_AskInfo(servernode);
                    asksent=I_GetTime();
                }
                break;
            case CLM_downloadfiles :
                waitmore=false;
                for(i=0; i<cl_num_fileneed; i++)
	        {
                    if(cl_fileneed[i].status==FS_DOWNLOADING
		       || cl_fileneed[i].status==FS_REQUESTED)
                    {
                        waitmore=true;
                        break;
                    }
		}
                if(waitmore)
                    break; // exit the case
	        cl_mode=CLM_askjoin; //don't break case continue to cljoin request now
	        // continue into next case
            case CLM_askjoin :
                CL_Load_ServerFiles();
#ifdef JOININGAME
                // prepare structures to save the file
                // WARNING: this can be useless in case of server not in GS_LEVEL
                // but since the network layer don't provide ordered packet ...
                CL_Prepare_Download_SaveGame(tmpsave);
#endif
                if( CL_Send_Join() )
                    cl_mode=CLM_waitjoinresponse;
                break;
#ifdef JOININGAME
            case CLM_downloadsavegame :
                if( cl_fileneed[0].status==FS_FOUND )
                {
                    CL_Load_Received_Savegame();
                    gamestate = GS_LEVEL;
                    cl_mode=CLM_connected;
                }           //don't break case continue to CLM_connected
                else
                    break;
#endif
	        // continue into next case
            case CLM_waitjoinresponse :
            case CLM_connected :
                break;
        }

        Net_Packet_Handler();
        // connection closed by cancel or timeout
        if( !server && !netgame )
        {
            cl_mode = CLM_searching;
            return;
        }
        Net_AckTicker();

        // call it only once every tic
        if( oldtic!=I_GetTime() )
        {
	    // user response handler
            int key;

            I_OsPolling();
            key = I_GetKey();
            if (key==KEY_ESCAPE)
            {
                M_SimpleMessage ("Network game synchronization aborted.\n\nPress ESC\n");
	        goto reset_to_title_exit;
            }
            if( key=='s' && server) 
                doomcom->numnodes=numnodes;

            Filetx_Ticker();
            oldtic=I_GetTime();

            CON_Drawer ();
            I_FinishUpdate ();              // page flip or blit buffer
        }

        if(server)
        {
	    // Count the net nodes.
            numnodes=0;
            for(i=0;i<MAXNETNODES;i++)
	    {
                if(nodeingame[i]) numnodes++;
	    }

        }
    }  while (!( (cl_mode == CLM_connected) &&
                 ( (!server) || (server && (nodewaited<=numnodes)) )));

    DEBFILE(va("Synchronization Finished\n"));

    consoleplayer&= ~DRONE;
    displayplayer = consoleplayer;
    consoleplayer_ptr = displayplayer_ptr = &players[consoleplayer];
    return;

reset_to_title_exit:
    CL_Reset();
    D_StartTitle();
    return;
}

// By User, future Client.
void Command_connect(void)
{
    if( COM_Argc()<2 )
    {
        CONS_Printf ("connect <serveraddress> : connect to a server\n"
                     "connect ANY : connect to the first lan server found\n"
                     "connect SELF: connect to self server\n");
        return;
    }
    server = false;

    if( strcasecmp(COM_Argv(1),"self")==0 )
    {
        servernode = 0;
        server = true;
        // should be but...
        //SV_SpawnServer();
    }
    else
    {
        // used in menu to connect to a server in the list
        if( netgame && strcasecmp(COM_Argv(1),"node")==0 )
            servernode = atoi(COM_Argv(2));
        else
        if( netgame )
        {
            CONS_Printf("You cannot connect while in netgame\n"
                        "Leave this game first\n");
            return;
        }
        else
        {
            I_NetOpenSocket();
            netgame = true;
            multiplayer = true;
        
            if( strcasecmp(COM_Argv(1),"any")==0 )
                servernode = BROADCASTADDR;
            else
            if( I_NetMakeNode )
                servernode = I_NetMakeNode(COM_Argv(1));
            else
            {
                CONS_Printf("There is no server identification with this network driver\n");
                D_CloseConnection();
                return;
            }
        }
    }
    CL_ConnectToServer();
}

static void Reset_NetNode(byte node);

// Called by Kick cmd.
static void CL_RemovePlayer(int playernum)
{
    int i;
    if( server && !demoplayback )
    {
        byte nnode = player_to_nnode[playernum];
        playerpernode[nnode]--;
        if( playerpernode[nnode]<=0 )
        {
            nodeingame[player_to_nnode[playernum]] = false;
            Net_CloseConnection(player_to_nnode[playernum]);
            Reset_NetNode(nnode);
        }
    }

    // we should use a reset player but there is not such function
    for(i=0;i<MAXPLAYERS;i++)
    {
        players[i].addfrags += players[i].frags[playernum];
        players[i].frags[playernum] = 0;
        players[playernum].frags[i] = 0;
    }
    players[playernum].addfrags = 0;

    // remove avatar of player
    if( players[playernum].mo )
    {
        players[playernum].mo->player = NULL;
        P_RemoveMobj (players[playernum].mo);
    }
    players[playernum].mo = NULL;
    playeringame[playernum] = false;
    player_to_nnode[playernum] = -1;
    while(playeringame[doomcom->numplayers-1]==0
	  && doomcom->numplayers>1)
    {
        doomcom->numplayers--;
    }
}

void CL_Reset (void)
{
    if (demorecording)
        G_CheckDemoStatus ();

    // reset client/server code
    DEBFILE(va("\n-=-=-=-=-=-=-= Client reset =-=-=-=-=-=-=-\n\n"));

    if( servernode>0 && servernode<MAXNETNODES)
    {
        nodeingame[(byte)servernode]=false;
        Net_CloseConnection(servernode);
    }
    D_CloseConnection();         // netgame=false
    multiplayer = false;
    servernode=0;
    server=true;
    doomcom->numnodes=1;
    doomcom->numplayers=1;
    SV_StopServer();
    SV_ResetServer();

    T_ClearHubScript();	//DarkWolf95: Originally implemented by Exl
    fadealpha = 0;
    HU_ClearFSPics();

    // reset game engine
    //D_StartTitle ();
}

void Command_PlayerInfo(void)
{
    int i;

    for(i=0;i<MAXPLAYERS;i++)
    {
        if(playeringame[i])
        {
            if(serverplayer==i)
	    {
                CONS_Printf("\2num:%2d  node:%2d  %s\n",
			    i, player_to_nnode[i], player_names[i]);
	    }
            else
	    {
                CONS_Printf("num:%2d  node:%2d  %s\n",
			    i, player_to_nnode[i], player_names[i]);
	    }
        }
    }
}

int nametonum(char *name)
{
    int playernum,i;

    if( strcmp(name,"0")==0 )
        return 0;

    playernum=atoi(name);

    if(playernum)
    {
        if(playeringame[playernum])
            return playernum;
        else 
            return -1;
    }

    for(i=0;i<MAXPLAYERS;i++)
    {
        if(playeringame[i] && strcasecmp(player_names[i],name)==0)
            return i;
    }
    
    CONS_Printf("there is no player named\"%s\"\n",name);

    return -1;
}

// network kick message codes
typedef enum {
  KICK_MSG_GO_AWAY     = 1,
  KICK_MSG_CON_FAIL    = 2,
  KICK_MSG_PLAYER_QUIT = 3,
  KICK_MSG_TIMEOUT     = 4,
} kick_msg_e;

void Command_Kick(void)
{
    char  buf[3];

    if (COM_Argc() != 2)
    {
        CONS_Printf ("kick <playername> or <playernum> : kick a player\n");
        return;
    }

    if(server)
    {
        buf[0]=nametonum(COM_Argv(1));
        if(buf[0]==-1)
           return;
        buf[1]=KICK_MSG_GO_AWAY;
        Send_NetXCmd(XD_KICK, &buf, 2);
    }
    else
    {
        CONS_Printf("You are not the server\n");
    }

}

void Got_NetXCmd_KickCmd(char **p, int playernum)
{
    int pnum=READBYTE(*p);  // unsigned player num
    int msg =READBYTE(*p);  // unsigned kick message

    CONS_Printf("\2%s ",player_names[pnum]);

    switch(msg)
    {
       case KICK_MSG_GO_AWAY:
               CONS_Printf("has been kicked (Go away)\n");
               break;
       case KICK_MSG_CON_FAIL:
               CONS_Printf("has been kicked (Consistency failure)\n");
               break;
       case KICK_MSG_TIMEOUT:
               CONS_Printf("left the game (Connection timeout)\n");
               break;
       case KICK_MSG_PLAYER_QUIT:
               CONS_Printf("left the game\n");
               break;
    }
    if( pnum==consoleplayer )
    {
        CL_Reset();
        D_StartTitle();
        M_SimpleMessage("You have been kicked by the server\n\nPress ESC\n");
    }
    else
    {
        CL_RemovePlayer(pnum);
    }
}

CV_PossibleValue_t maxplayers_cons_t[]={{1,"MIN"},{32,"MAX"},{0,NULL}};

consvar_t cv_allownewplayer = {"sv_allownewplayers","1",0,CV_OnOff};
consvar_t cv_maxplayers     = {"sv_maxplayers","32",CV_NETVAR,maxplayers_cons_t,NULL,32};

void Got_NetXCmd_AddPlayer(char **p, int playernum);
void Got_NetXCmd_AddBot(char **p,int playernum);	//added by AC for acbot

// called one time at init
void D_ClientServerInit (void)
{
  DEBFILE(va("- - -== %s debugfile ==- - -\n", VERSION_BANNER));

    cl_drone = false;

    // drone server generating the left view of three screen view
    if(M_CheckParm("-left"))
    {
        cl_drone = true;
        viewangleoffset = ANG90;
    }
    // drone server generating the right view of three screen view
    if(M_CheckParm("-right"))
    {
        cl_drone = true;
        viewangleoffset = -ANG90;
    }
    // [WDJ] specify secondary screen angle in degrees (general case of left/right)
    if(M_CheckParm("-screendeg"))
    {
        cl_drone = true;
        if( M_IsNextParm() )
        {
	    // does not accept negative numbers, use 270, 315, etc
	    viewangleoffset = atoi(M_GetNextParm()) * (ANG90 / 90);
	    // it is cheating to have screen looking behind player
	    if( viewangleoffset < -ANG90 )  viewangleoffset = -ANG90;
	    if( viewangleoffset > ANG90 )  viewangleoffset = ANG90;
	}
    }
//    GenPrintf(EMSG_debug, "viewangleoffset=%i\n", viewangleoffset );

    // for dedicated server, without player
    dedicated=M_CheckParm("-dedicated")!=0;
    
    COM_AddCommand("playerinfo",Command_PlayerInfo);
    COM_AddCommand("kick",Command_Kick);
    COM_AddCommand("connect",Command_connect);

    Register_NetXCmd(XD_KICK, Got_NetXCmd_KickCmd);
    Register_NetXCmd(XD_ADDPLAYER, Got_NetXCmd_AddPlayer);
    Register_NetXCmd(XD_ADDBOT, Got_NetXCmd_AddBot);	//added by AC for acbot
    CV_RegisterVar (&cv_allownewplayer);
    CV_RegisterVar (&cv_maxplayers);

    gametic = 0;
    localgametic = 0;

    // do not send anything before the real begin
    SV_StopServer();

    SV_ResetServer();

    if(dedicated)
	SV_SpawnServer();
}

static void Reset_NetNode(byte nnode)
{
    nodeingame[nnode] = false;
    nnode_to_player[nnode] = 255;
    nnode_to_player2[nnode] = 255;
    nettics[nnode]=gametic;
    nextsend_tic[nnode]=gametic;
    cl_maketic[nnode]=maketic;
    nodewaiting[nnode]=0;
    playerpernode[nnode]=0;
}

// Called by D_ClientServerInit, SV_SpawnServer, CL_Reset,
void SV_ResetServer( void )
{
    int    i;

    // +1 because this command will be executed in com_executebuffer in
    // tryruntic so gametic will be incremented, anyway maketic > gametic 
    // is not a issue

    maketic=gametic+1;
    cl_need_tic=maketic;
#ifdef CLIENTPREDICTION2
    localgametic = gametic;
#endif
    next_tic_clear=maketic;

    for (i=0 ; i<MAXNETNODES ; i++)
        Reset_NetNode(i);

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        playeringame[i]=false;
        player_to_nnode[i]=-1;
    }

    cl_nnode=0;
    cl_packetmissed=false;
    viewangleoffset=0;

    if( dedicated )
    {
        nodeingame[0]=true;
        serverplayer = 255;  // no server player
    }
    else
        serverplayer=consoleplayer;

    if(server)
        servernode=0;

    doomcom->numplayers=0;

    DEBFILE(va("\n-=-=-=-=-=-=-= Server Reset =-=-=-=-=-=-=-\n\n"));
}

//
// D_Quit_NetGame
// Called before quitting to leave a net game
// without hanging the other players
//
void D_Quit_NetGame (void)
{
    byte nn; // net node num

    if (!netgame)
        return;

    DEBFILE("===========================================================================\n"
            "                  Quiting Game, closing connection\n"
            "===========================================================================\n");

    // abort send/receive of files
    CloseNetFile();

    if( server )
    {
        // Server sends shutdown to all clients.
        netbuffer->packettype=PT_SERVERSHUTDOWN;
        for(nn=0; nn<MAXNETNODES; nn++)
        {
            if( nodeingame[nn] )
                HSendPacket(nn,true,0,0);
	}
        if ( serverrunning && cv_internetserver.value )
             UnregisterServer(); 
    }
    else
    if(servernode>0
       && servernode<MAXNETNODES
       && nodeingame[(byte)servernode]!=0)
    {
        // Client sends quit to server.
        netbuffer->packettype=PT_CLIENTQUIT;
        HSendPacket(servernode,true,0,0);
    }

    D_CloseConnection();

    DEBFILE("===========================================================================\n"
            "                         Log finish\n"
            "===========================================================================\n");
#ifdef DEBUGFILE
    if (debugfile)
    {
        fclose (debugfile);
        debugfile = NULL;
    }
#endif
}

// Add a node to the game (player will follow at map change or at savegame....)
void SV_AddNode(byte nnode)
{
    nettics[nnode]       = gametic;
    nextsend_tic[nnode]  = gametic;
    cl_maketic[nnode]    = maketic;
    // Because the server connects to itself and sets nodeingame[0],
    // do not interfere with nodeingame[0] here.
    if(nnode)
       nodeingame[nnode]=true;
}

// Xcmd XD_ADDPLAYER
void Got_NetXCmd_AddPlayer(char **p,int playernum)
{
    static uint32_t sendconfigtic = 0xffffffff;

    // [WDJ] Having error due to sign extension of byte read (signed char).
    unsigned int node=READBYTE(*p);  // unsigned
    unsigned int newplayernum=READBYTE(*p);  // unsigned
    boolean splitscreenplayer = newplayernum&0x80;

    newplayernum&=0x7F;  // remove flag bit, and any sign extension

    playeringame[newplayernum]=true;
    G_AddPlayer(newplayernum);
    if( newplayernum+1 > doomcom->numplayers )
        doomcom->numplayers=newplayernum+1;
    CONS_Printf("Player %d is in the game (node %d)\n",newplayernum,node);

    if(node==cl_nnode)
    {
        // The server is creating my player.
        player_to_nnode[newplayernum]=0;  // for information only
        if(!splitscreenplayer)
        {
            consoleplayer=newplayernum;
            displayplayer=newplayernum;
	    displayplayer_ptr = consoleplayer_ptr = &players[newplayernum];
            DEBFILE("spawning me\n");
        }
        else
        {
            displayplayer2=newplayernum;
	    displayplayer2_ptr = &players[displayplayer2];
            DEBFILE("spawning my brother\n");
        }
    }

    // the new player send there config
    // and the old player send there config to the new one
    // WARNING : this can cause a bottleneck in the txtcmd
    //           this can also produce consistency failure if packet get lost
    //           because everybody knows the actual config except the joiner
    //    TODO : fixthis

    //  Don't send config more than once per tic (more than one player join)
    if( sendconfigtic!=gametic )
    {
        sendconfigtic=gametic;
        D_Send_PlayerConfig();
    }
}

// Xcmd XD_ADDBOT
void Got_NetXCmd_AddBot(char **p, int playernum)  //added by AC for acbot
{
    // [WDJ] Having error due to sign extension of byte read (signed char).
    unsigned int newplayernum=READBYTE(*p);  // unsigned
    //int node = 0;
    //int i = 0;
    newplayernum&=0x7F;  // remove flag bit, and any sign extension
    playeringame[newplayernum]=true;
    strcpy(player_names[newplayernum], botinfo[newplayernum].name);
    players[newplayernum].skincolor = botinfo[newplayernum].colour;
    G_AddPlayer(newplayernum);
    players[newplayernum].bot = B_CreateBot();
    if( newplayernum+1>doomcom->numplayers )
        doomcom->numplayers=newplayernum+1;

    multiplayer=1;

    CONS_Printf ("Bot %s has entered the game\n", player_names[newplayernum]);
}

// By Server.
// Called by SV_SpawnServer, client_join_handler.
boolean SV_AddWaitingPlayers(void)
{
    int  node,n;
    int  newplayer=false;
    byte buf[2],newplayernum;

    newplayernum=0;
    for(node=0;node<MAXNETNODES;node++)
    {
        // splitscreen can allow 2 player in one node
        for(;nodewaiting[node]>0;nodewaiting[node]--)
        {
            newplayer=true;

            // search for a free playernum
            // we can't use playeringame since it is not updated here
            //while(newplayernum<MAXPLAYERS && playeringame[newplayernum])
            //    newplayernum++;
            for ( ;newplayernum<MAXPLAYERS;newplayernum++)
            {
                for(n=0;n<MAXNETNODES;n++)
	        {
                    if( nnode_to_player[n]  == newplayernum
                        || nnode_to_player2[n] == newplayernum )
                        break;
		}
                if( n == MAXNETNODES )
                    break;
            }
            
#ifdef PARANOIA
            // should never happens since we check the playernum before accept 
            // the join
            if(newplayernum==MAXPLAYERS)
                I_Error("SV_AddWaitingPlayers : j=MAXPLAYERS\n");
#endif
            player_to_nnode[newplayernum] = node;

            buf[0]=node;
            buf[1]=newplayernum;
            if( playerpernode[node]<1 )
                nnode_to_player[node] = newplayernum;
            else
            {
                nnode_to_player2[node] = newplayernum;
                buf[1]|=0x80;
            }
            playerpernode[node]++;

            Send_NetXCmd(XD_ADDPLAYER,&buf,2);
            if( doomcom->numplayers==0 )
                doomcom->numplayers++;  //we must send the change to other players
            
            DEBFILE(va("Server added player %d node %d\n", newplayernum, node));
            // use the next free slot (we can't put playeringame[j]=true here)
            newplayernum++; 
        }
    }

    return newplayer;
}

void CL_AddSplitscreenPlayer( void )
{
    if( cl_mode == CLM_connected )
        CL_Send_Join();  // join game
}

void CL_RemoveSplitscreenPlayer( void )
{
    char  buf[2];
    
    if( cl_mode != CLM_connected )
        return;

    buf[0]=displayplayer2;  // player 2
    buf[1]=KICK_MSG_PLAYER_QUIT;
    Send_NetXCmd(XD_KICK, &buf, 2);
}

// Is there a game running.
boolean Game_Playing( void )
{
    return (server && serverrunning) || (!server && cl_mode==CLM_connected);
}

// By Server.
// Called by D_ClientServerInit, Command_Map_f, Command_Load_f, 
boolean SV_SpawnServer( void )
{
    D_DisableDemo();

    if( serverrunning == false )
    {
        CONS_Printf("Starting Server....\n");
        serverrunning = true;
        SV_ResetServer();
        if( netgame )
        {
            I_NetOpenSocket();
            if( cv_internetserver.value )
                RegisterServer(0, 0);
        }

        // server just connect to itself
        if( !dedicated )
            CL_ConnectToServer();
    }

    return SV_AddWaitingPlayers();
}

// Called by D_ClientServerInit, G_StopDemo, CL_Reset, SV_StartSinglePlayerServer.
void SV_StopServer( void )
{
    int i;

    gamestate = wipegamestate = GS_NULL;

    localtextcmd.buff[0]=0;  // text len
    localtextcmd2.buff[0]=0; // text len

    for(i=0; i<BACKUPTICS; i++)
        D_Clear_ticcmd(i);

    consoleplayer=0;
    cl_mode = CLM_searching;
    maketic=gametic+1;
    cl_need_tic=maketic;
    serverrunning = false;
}

// called at singleplayer start and stopdemo
void SV_StartSinglePlayerServer(void)
{
    server        = true;
    netgame       = false;
    multiplayer   = false;

    // no more tic the game with this settings !
    SV_StopServer();

    if( cv_splitscreen.value )
        multiplayer    = true;
}

// By Server.
// Called by client_join_handler.
static void SV_Send_Refuse(int to_node, char *reason)
{
    strcpy(netbuffer->u.serverrefuse.reason, reason);

    netbuffer->packettype=PT_SERVERREFUSE;
    HSendPacket(to_node, true, 0, strlen(netbuffer->u.serverrefuse.reason)+1);
    Net_CloseConnection(to_node);
}

// By Server.
// PT_ASKINFO from Client.
// Handle a client request for server info.
static void server_askinfo_handler( byte nnode )
{
    if(serverrunning)
    {
        // Make the send_time the round trip ping time.
        SV_Send_ServerInfo(nnode,
			   LE_SWAP32_FAST(netbuffer->u.askinfo.send_time));
        Net_CloseConnection(nnode);  // a temp connection
    }
}

// By Server.
// PT_CLIENTJOIN from future client.
//   nnode: net node that is joining
static void client_join_handler( byte nnode )
{
#ifdef JOININGAME
    boolean newnode=false;
#endif
    if( netbuffer->u.clientcfg.version != VERSION
	|| LE_SWAP32_FAST(netbuffer->u.clientcfg.subversion) != NETWORK_VERSION)
    {
        SV_Send_Refuse(nnode,
	   va("Different DOOM versions cannot play a net game! (server version %s)",
	       VERSION_BANNER));
        return;
    }
    // nnode==0 is self, which is always accepted.
    if(!cv_allownewplayer.value && nnode!=0 )
    {
        SV_Send_Refuse(nnode,
	  "The server is not accepting people for the moment");
        return;
    }
   
    // TODO; compute it using nodewaiting and playeringame
    if( (doomcom->numplayers + 1) > cv_maxplayers.value)
    {
        SV_Send_Refuse(nnode,
	   va("Maximum of player reached (max:%d)", cv_maxplayers.value));
        return;
    }
   
    // Client authorized to join.
    nodewaiting[nnode] = netbuffer->u.clientcfg.localplayers - playerpernode[nnode];
    if(!nodeingame[nnode])
    {
        SV_AddNode(nnode);
        if(!SV_Send_ServerConfig(nnode))
        {
	    // TODO : fix this !!!
	    CONS_Printf("Internal Error 5 : client lost\n");
	    return;
	}
        DEBFILE("new node joined\n");
#ifdef JOININGAME
        newnode = true;
#endif
    }
#ifdef JOININGAME
    if( nodewaiting[nnode] )
    {
        if( gamestate == GS_LEVEL && newnode)
        {
	    SV_Send_SaveGame(nnode); // send game data
	    CONS_Printf("send savegame\n");
	}
        SV_AddWaitingPlayers();
    }
#endif
}

// BY Server.
// PT_NODE_TIMEOUT, PT_CLIENTQUIT
//   nnode : the network client node quitting
//   netconsole : the player
static void client_quit_handler( byte nnode, int netconsole )
{
    // nodeingame will made false during the execution of kick command.
    // This allows the sending of some packets to the quiting client
    // and to have them ack back.
    nodewaiting[nnode]= 0;
    if(netconsole >= 0 && playeringame[netconsole])
    {
        byte  buf[2];
        buf[0]=netconsole;  // kick player
        buf[1]=(netbuffer->packettype == PT_NODE_TIMEOUT) ?
	   KICK_MSG_TIMEOUT : KICK_MSG_PLAYER_QUIT;
        // Update other players by kicking nnode.
        Send_NetXCmd(XD_KICK,&buf,2);
        nnode_to_player[nnode] = 255;

        if( nnode_to_player2[nnode] < MAXPLAYERS )
        {
	    if( playeringame[nnode_to_player2[nnode]] )
	    {
	       buf[0]=nnode_to_player2[nnode]; // kick player2
	       Send_NetXCmd(XD_KICK, &buf, 2);
	    }
	    nnode_to_player2[nnode] = 255;
	}
    }
    Net_CloseConnection(nnode);
    nodeingame[nnode]=false;
}



// By Client
// PT_SERVERINFO from Server.
static void server_info_handler( byte nnode )
{
    // Compute ping in ms.
    netbuffer->u.serverinfo.trip_time =
     (I_GetTime() - LE_SWAP32_FAST(netbuffer->u.serverinfo.trip_time))*1000/TICRATE; 
    netbuffer->u.serverinfo.servername[MAXSERVERNAME-1]=0;

    SL_InsertServer( &netbuffer->u.serverinfo, nnode);
}


// By Client
// PT_SERVERREFUSE from Server.
static void server_refuse_handler( byte nnode )
{
    if( cl_mode == CLM_waitjoinresponse )
    {
        M_SimpleMessage(va("Server %i refuses connection\n\nReason :\n%s",
			   nnode,
			   netbuffer->u.serverrefuse.reason));
        CL_Reset();
        D_StartTitle();
    }
}


// By Client
// PT_SERVERCFG from Server.
// Received acceptance of the player node joining the game.
static void server_cfg_handler( byte nnode )
{
    int j;
    byte *p;

    if( cl_mode != CLM_waitjoinresponse )
        return;

    if(!server)
    {
        // Clients not on the server, update to server time.
        maketic = gametic = cl_need_tic = LE_SWAP32_FAST(netbuffer->u.servercfg.gametic);
    }

#ifdef CLIENTPREDICTION2
    localgametic = gametic;
#endif
    nodeingame[(byte)servernode]=true;

    // Handle a player on the server.
    serverplayer = netbuffer->u.servercfg.serverplayer;
    if (serverplayer < MAXPLAYERS)  // 255 = no player
        player_to_nnode[serverplayer] = servernode;

    doomcom->numplayers = netbuffer->u.servercfg.totalplayernum;
    cl_nnode = netbuffer->u.servercfg.clientnode;

    CONS_Printf("Join accepted, wait next map change...\n");
    DEBFILE(va("Server accept join gametic=%d, client net node=%d\n",
	       gametic, cl_nnode));

    uint32_t  playerdet = LE_SWAP32_FAST(netbuffer->u.servercfg.playerdetected);
    for(j=0;j<MAXPLAYERS;j++)
    {
        playeringame[j]=( playerdet & (1<<j) ) != 0;
    }

    p = netbuffer->u.servercfg.netcvarstates;
    CV_LoadNetVars( (char**)&p );
#ifdef JOININGAME
    cl_mode = ( netbuffer->u.servercfg.gamestate == GS_LEVEL ) ?
       CLM_downloadsavegame : CLM_connected;
#else
    cl_mode = CLM_connected;
#endif
}

// By Client
// PT_SERVERSHUTDOWN from Server.
static void server_shutdown_handler()
{
    if( cl_mode != CLM_searching )
    {
        M_SimpleMessage("Server has Shutdown\n\nPress Esc");
        CL_Reset();
        D_StartTitle();
    }
}

// By Client
// PT_NODE_TIMEOUT
static void server_timeout_handler()
{
    if( cl_mode != CLM_searching )
    {
        M_SimpleMessage("Server Timeout\n\nPress Esc");
        CL_Reset();
        D_StartTitle();
    }
}


// By Client, Server.
// Invoked by anyone trying to join !
static void unknown_host_handler( byte nnode )
{
    // Packet can be from client trying to join server,
    // or from a server responding to a join request.
    if( nnode != servernode )
    {
        // Client trying to Join.
        DEBFILE(va("Received packet from unknown host %d\n",nnode));
    }

    // The commands that are allowd by an unknown host.
    switch(netbuffer->packettype)
    {
     case PT_ASKINFO:  // client has asked server for info
        if( server )
            server_askinfo_handler( nnode );
        break;
     case PT_SERVERREFUSE : // negative response of client join request
        server_refuse_handler( nnode );
        break;
     case PT_SERVERCFG :    // positive response of client join request
        server_cfg_handler( nnode );
        break;
     // handled in d_netfil.c
     case PT_FILEFRAGMENT :
        if( !server )
	    Got_Filetxpak();
        break;
     case PT_REQUESTFILE :
        if( server )
	    Got_RequestFilePak(nnode);
        break;
     case PT_NODE_TIMEOUT:
     case PT_CLIENTQUIT:
        if( server )
	    Net_CloseConnection(nnode);
        break;
     case PT_SERVERTICS:
        if( nnode == servernode )
        {
	    // do not remove my own server
	    // (we have just to get a out of order packet)
	    break;
	}
        // Server tic from unknown source.
        // Fall through to default.
     default:
        DEBFILE(va("unknown packet received (%d) from unknown host !\n",
		   netbuffer->packettype));
        Net_CloseConnection(nnode);  // a temp connection
        break; // ignore it
    } // switch
}


// used at txtcmds received to check packetsize bound
static int TotalTextCmdPerTic(int tic)
{
    int i,total=1; // num of textcmds in the tic (ntextcmd byte)

    int btic = BTIC_INDEX( tic );

    for(i=0;i<MAXPLAYERS;i++)
    {
        if( (i==0) || playeringame[i] )
	{
	    int textlen = textcmds[btic][i].buff[0];
	    if( textlen )
	       total += textlen + 2; // "+2" for size and playernum
	}
    }

    return total;
}

// Copy an array of ticcmd_t, swapping between host and network byte order.
//
static void TicCmdCopy(ticcmd_t * dst, ticcmd_t * src, int n)
{
    int i;
    for (i = 0; i < n; src++, dst++, i++)
    {
#ifdef CLIENTPREDICTION2
	dst->x = LE_SWAP32_FAST(src->x);
	dst->y = LE_SWAP32_FAST(src->y);
#endif
	dst->forwardmove = src->forwardmove;
	dst->sidemove    = src->sidemove;
	dst->angleturn   = LE_SWAP16_FAST(src->angleturn);
	dst->aiming      = LE_SWAP16_FAST(src->aiming);
	dst->buttons     = src->buttons;
    }
}

// By Server
// PT_TEXTCMD, PT_TEXTCMD2
//   nnode : the network client
//   netconsole : playernum
static void net_textcmd_handler( byte nnode, int netconsole )
{
    int ntextsize = netbuffer->u.textcmd[0];
    int mpsize;  // Max size of packet than can be sent with the text
    int textlen, btic;
    tic_t tic;
    textbuf_t *  textbuf;  // first byte of this buffer is the cur usage

    // Check if tic that we are making isn't too large,
    // else we cannot send it :(
    // Note: doomcom->numplayers+1 is "+1" because doomcom->numplayers
    // can change within this time and sent time.
    mpsize = software_MAXPACKETLENGTH
	 - ( ntextsize + 2 + SERVER_TIC_BASE_SIZE
	    + ((doomcom->numplayers+1) * sizeof(ticcmd_t)) );

    // Search for a tic that has enough space in the ticcmd.
    tic = maketic;
    while( TotalTextCmdPerTic(tic) > mpsize )
    {
        textbuf = & textcmds[ BTIC_INDEX( tic ) ][netconsole];
        if( (ntextsize + textbuf->buff[0]) <= MAXTEXTCMD )
	  break; // found one
        tic++;
        if( tic >= (next_tic_send+BACKUPTICS) )  goto drop_packet;
    }

    btic = BTIC_INDEX( tic );
    textbuf = & textcmds[btic][netconsole];
    textlen = textbuf->buff[0];
    DEBFILE(va("Textcmd: btic %d buff[%d] player %d nxttic %d maketic %d\n",
	       btic, textlen+1, netconsole, next_tic_send, maketic));
    // Append to the selected buffer.
    memcpy(&textbuf->buff[ textlen+1 ],
	   netbuffer->u.textcmd+1,  // the text
	   ntextsize);
    textbuf->buff[0] += ntextsize;  // text len
    return;
   
drop_packet:
    // Drop the packet, let the node resend it.
    DEBFILE(va("Textcmd too long: max %d used %d maketic %d nxttic %d node %d player %d\n",
	       mpsize, TotalTextCmdPerTic(maketic), maketic, next_tic_send,
	       nnode, netconsole));
    Net_Cancel_Packet_Ack(nnode);
    return;
}

// By Server
// PT_CLIENTCMD, PT_CLIENT2CMD, PT_CLIENTMIS, PT_CLIENT2MIS,
// PT_NODEKEEPALIVE, PT_NODEKEEPALIVEMIS from Client.
//  netconsole: a player
static void client_cmd_handler( byte netcmd, byte nnode, int netconsole )
{
    tic_t  start_tic, end_tic;
    int  btic;

    // To save bytes, only the low byte of tic numbers are sent
    // Figure out what the rest of the bytes are
    start_tic  = ExpandTics (netbuffer->u.clientpak.client_tic);
    end_tic = ExpandTics (netbuffer->u.clientpak.resendfrom);

    if(  netcmd == PT_CLIENTMIS
	 || netcmd == PT_CLIENT2MIS
	 || netcmd == PT_NODEKEEPALIVEMIS 
	 || nextsend_tic[nnode] < end_tic )
    {
        nextsend_tic[nnode] = end_tic;
    }

    // Discard out of order packet
    if( nettics[nnode] > end_tic )
    {
        DEBFILE(va("Out of order ticcmd discarded, nettics = %d\n",
		   nettics[nnode]));
        return;
    }

    // Update the nettics.
    nettics[nnode] = end_tic;

    // Don't do any tic cmds for drones, just update their nettics.
    if((netconsole & DRONE) || netconsole==-1
       || netcmd==PT_NODEKEEPALIVE || netcmd==PT_NODEKEEPALIVEMIS)
       return;

    // Check consistency
    btic = BTIC_INDEX(start_tic);
    if((start_tic <= gametic)
       && (start_tic > (gametic - BACKUPTICS + 1))
       && (consistency[btic] != LE_SWAP16_FAST(netbuffer->u.clientpak.consistency))) 
    {
        // Failed the consistency check.
#if 1
        byte buf[3];

        buf[0]=netconsole;
        buf[1]=KICK_MSG_CON_FAIL;
        Send_NetXCmd(XD_KICK, &buf, 2);
#else
        CONS_Printf("\2player %d kicked [%d] consistency failure\n",netconsole,start_tic);
#endif
        DEBFILE(va("player %d kicked [%d] %d!=%d\n",
		    netconsole,start_tic,consistency[btic],
		    LE_SWAP16_FAST(netbuffer->u.clientpak.consistency)));

    }

    // Copy the ticcmd
    btic = BTIC_INDEX( maketic );
    TicCmdCopy(&netcmds[btic][netconsole],
	       &netbuffer->u.clientpak.cmd, 1);

    if( netcmd==PT_CLIENT2CMD
	&& (nnode_to_player2[nnode] < MAXPLAYERS))
    {
        // Copy the ticcmd for player2.
        TicCmdCopy(&netcmds[btic][nnode_to_player2[nnode]], 
		   &netbuffer->u.client2pak.cmd2, 1);
    }
    return;
}



// By Client.
// PT_SERVERTICS from Server.
static void servertic_handler( byte nnode )
{
    tic_t  start_tic, end_tic, ti;
    ticcmd_t * ticp;  // net tics
    byte * txtp;  // net txtcmd text
    byte   num_txt;
    int    btic, j;

    start_tic = ExpandTics (netbuffer->u.serverpak.starttic);
    end_tic   = start_tic + netbuffer->u.serverpak.numtics;

    if( end_tic > (gametic + BACKUPTICS))
        end_tic = (gametic + BACKUPTICS);  // limit to backup capability

    // Check if missed any packets.
    cl_packetmissed = (start_tic > cl_need_tic);

    if((start_tic > cl_need_tic) || (end_tic <= cl_need_tic))
       goto not_in_packet;
   
    // The needed tic is within this packet.
    // Nettics
    ticp = netbuffer->u.serverpak.cmds;
    // After the nettics are the net textcmds
    txtp = (byte *)&netbuffer->u.serverpak.cmds[
	   netbuffer->u.serverpak.numplayers*netbuffer->u.serverpak.numtics];

    for(ti = start_tic; ti<end_tic; ti++)
    {
        // Clear first
        D_Clear_ticcmd(ti);

        // Copy the tics
        btic = BTIC_INDEX( ti );
        TicCmdCopy(netcmds[btic], ticp, netbuffer->u.serverpak.numplayers);
        ticp += netbuffer->u.serverpak.numplayers;

        // Copy the textcmds
        num_txt = *(txtp++);  // number of txtcmd
        for(j=0; j<num_txt; j++)
        {
	    int pn = *txtp++; // playernum
	    memcpy(textcmds[btic][pn].buff, txtp, txtp[0]+1);
	    txtp += txtp[0]+1;
	}
    }

    cl_need_tic = end_tic;
    return;

 not_in_packet:
    DEBFILE(va("Needed tic not in packet tic bounds : %u\n", cl_need_tic));
    return;
}


//
// Get Packets and invoke their Server and Client handlers.
//
static void Net_Packet_Handler(void)
{
    int  netconsole;
    byte nnode;

    while ( HGetPacket() )
    {
        nnode = doomcom->remotenode;
        // ---- Universal Handling ----------
        switch(netbuffer->packettype) {
	 case PT_CLIENTJOIN:
	    if( server )
	        client_join_handler( nnode );
            continue;
	 case PT_SERVERSHUTDOWN:
	    if( !server && (nnode == servernode) )
	        server_shutdown_handler();
            continue;
         case PT_NODE_TIMEOUT:
	    if( !server && (nnode == servernode) )
	    {
	        server_timeout_handler();
	        continue;
	    }
	    break; // There are other PT_NODE_TIMEOUT handler.
	 case PT_SERVERINFO:
	    server_info_handler( nnode );
            continue;
	}

        if(!nodeingame[nnode])
        {
	    unknown_host_handler( nnode );
            continue; //while
        }

        netconsole = nnode_to_player[nnode];  // the player
#ifdef PARANOIA
        if(!(netconsole & DRONE) && netconsole>=MAXPLAYERS)
        {
            I_Error("bad table nnode_to_player : node %d player %d",
                     doomcom->remotenode,netconsole);
	}
#endif


        switch(netbuffer->packettype) {
	    // ---- Server Handling Client packets ----------
            case PT_CLIENTCMD  :
            case PT_CLIENT2CMD :
            case PT_CLIENTMIS  :
            case PT_CLIENT2MIS :
            case PT_NODEKEEPALIVE :
            case PT_NODEKEEPALIVEMIS :
                if(server)
	             client_cmd_handler(netbuffer->packettype, nnode, netconsole );
                break;
            case PT_TEXTCMD2 : // splitscreen special
                netconsole=nnode_to_player2[nnode];
	        // fall through
            case PT_TEXTCMD :
                if(server)
	        {
		    if( netconsole<0 || netconsole>=MAXPLAYERS )
		    {
		        // Do not ACK the packet from a strange netconsole.
		        Net_Cancel_Packet_Ack(nnode);
		        break;
		    }
	            net_textcmd_handler( nnode, netconsole );
		}
                break;
            case PT_NODE_TIMEOUT:
            case PT_CLIENTQUIT:
                if(server)
	            client_quit_handler( nnode, netconsole );
                break;

            // ---- CLIENT Handling Server packets ----------
            case PT_SERVERTICS :
	        servertic_handler( nnode );
                break;
            case PT_SERVERCFG :
	        break;
            case PT_FILEFRAGMENT :
                if( !server )
                    Got_Filetxpak();
                break;
            default:
                DEBFILE(va("UNKNOWN PACKET TYPE RECEIVED %f from host %d\n",
			   netbuffer->packettype, nnode));
	        break;
        } // end switch
    } // end while
}


// ----- NetUpdate
// Builds ticcmds for console player,
// sends out a packet

// no more use random generator, because at very first tic isn't yet synchronized
static int16_t Consistency(void)
{
    int16_t ret=0;
    int   pn;

    DEBFILE(va("TIC %d ",gametic));
    for(pn=0; pn<MAXPLAYERS; pn++)
    {
        if( playeringame[pn] && players[pn].mo )
        {
            DEBFILE(va("p[%d].x = %f ", pn, FIXED_TO_FLOAT(players[pn].mo->x)));
            ret += players[pn].mo->x;
        }
    }
    DEBFILE(va("pos = %d, rnd %d\n",ret,P_GetRandIndex()));
    ret+=P_GetRandIndex();

    return ret;
}

// By Server, Client.
// send the client packet to the server
// Called by NetUpdate, 
static void CL_Send_ClientCmd (void)
{
/* oops can do that until i have implemented a real dead reckoning
    static ticcmd_t lastcmdssent;
    static int      lastsenttime=-TICRATE;

    if( memcmp(&localcmds,&lastcmdssent,sizeof(ticcmd_t))!=0 || lastsenttime+TICRATE/3<I_GetTime())
    {
        lastsenttime=I_GetTime();
*/
    int packetsize=0;

    netbuffer->packettype=PT_CLIENTCMD;

    if (cl_packetmissed)
        netbuffer->packettype++;
    netbuffer->u.clientpak.resendfrom = cl_need_tic;
    netbuffer->u.clientpak.client_tic = gametic;

    if(gamestate==GS_WAITINGPLAYERS)
    {
        // send NODEKEEPALIVE packet
        netbuffer->packettype+=4;
        packetsize = sizeof(clientcmd_pak)-sizeof(ticcmd_t)-sizeof(short);
        HSendPacket (servernode,false,0,packetsize);
    }
    else
    if( gamestate!=GS_NULL )
    {
        int btic = BTIC_INDEX( gametic );
        TicCmdCopy(&netbuffer->u.clientpak.cmd, &localcmds, 1);
        netbuffer->u.clientpak.consistency = LE_SWAP16_FAST(consistency[btic]);

        // send a special packet with 2 cmd for splitscreen
        if (cv_splitscreen.value)
        {
            netbuffer->packettype+=2;
            TicCmdCopy(&netbuffer->u.client2pak.cmd2, &localcmds2, 1);
            packetsize = sizeof(client2cmd_pak);
        }
        else
            packetsize = sizeof(clientcmd_pak);
        
        HSendPacket (servernode,false,0,packetsize);
    }

    if( cl_mode == CLM_connected )
    {
        // send extra data if needed
        if (localtextcmd.buff[0]) // text len
        {
	    int len = localtextcmd.buff[0]+1;  // text len + cmd
            netbuffer->packettype=PT_TEXTCMD;
            memcpy(netbuffer->u.textcmd, localtextcmd.buff, len);
            // all extra data have been sended
            if( HSendPacket(servernode, true, 0, len)) // send can fail for some reasons...
                localtextcmd.buff[0] = 0;  // text len
        }
        
        // send extra data if needed for player 2 (splitscreen)
        if (localtextcmd2.buff[0]) // text len
        {
	    int len = localtextcmd2.buff[0]+1;  // text len + cmd
            netbuffer->packettype=PT_TEXTCMD2;
            memcpy(netbuffer->u.textcmd, localtextcmd2.buff, len);
            // all extra data have been sended
            if( HSendPacket(servernode, true, 0, len)) // send can fail for some reasons...
                localtextcmd2.buff[0] = 0; // text len
        }
    }

}


// By Server.
// Send PT_SERVERTICS, the server packet.
// Send tic from next_tic_send to maketic-1.
static void SV_Send_Tics (void)
{
    tic_t start_tic, end_tic, ti;
    int  nnode;
    int  pn, packsize;
    int  btic;
    char *num_txt_p, num_txt;
    char *bufpos;

    // Send PT_SERVERTIC to all client, but not to myself.
    // For each node create a packet with x tics and send it.
    // x is computed using nextsend_tic[n], max packet size and maketic.
    for(nnode=1; nnode<MAXNETNODES; nnode++)
    {
        if( ! nodeingame[nnode] )  continue;

        // Send a packet to each client node in this game.
	// They may be different, with individual status.
        end_tic = maketic;
          // The last tic to send is one before maketic.

        // assert nextsend_tic[nnode]>=nettics[nnode]
        start_tic = nextsend_tic[nnode];  // last tic sent to this nnode
       
        if(start_tic >= maketic)
        {
	    // We have sent all tics, so we will use the extrabandwidth
	    // to resend packets that are supposed lost.
	    // This is necessary since lost packet detection works when we
	    // have received packet with (firsttic > cl_need_tic)
	    // (in the getpacket servertics case).
	    DEBFILE(va("Nothing to send node %d mak=%u sup=%u net=%u \n", 
		       nnode, maketic, nextsend_tic[nnode], nettics[nnode]));
	    start_tic = nettics[nnode];
	    if( start_tic >= maketic )
	        continue;  // all tic are ok, do next node
	    if( (I_GetTime()+nnode)&3 )  // some kind of randomness
	        continue;  // skip it
	    DEBFILE(va("Sent %d anyway\n", start_tic));
	}

        // Limit start to first new tic we have.
        if( start_tic < next_tic_send )
	    start_tic = next_tic_send;

        // Compute the length of the packet and cut it if too large.
        packsize = SERVER_TIC_BASE_SIZE;
        for(ti=start_tic; ti<end_tic; ti++)
        {
	    // All of the ticcmd
	    packsize += sizeof(ticcmd_t) * doomcom->numplayers;
	    // All of the textcmd
	    packsize += TotalTextCmdPerTic(ti);

	    if( packsize > software_MAXPACKETLENGTH )
	    {
	        // Exceeds max packet size.
	        DEBFILE(va("Packet too large (%d) at tic %d (should be from %d to %d)\n", 
			    packsize, ti, start_tic, end_tic));
	        end_tic = ti;  // limit this packet due to size

	        // too bad :
	        // Too many players have sent extradata and there is too
	        // much data for one tic packet.
	        // Too avoid it put the data on the next tic (see getpacket 
	        // textcmd case). But when numplayer changes the computation
	        // can be different
	        if(end_tic == start_tic)
	        {
		    if( packsize > MAXPACKETLENGTH )
		    {
		        I_Error("Too many players: cannot send %d data for %d players to net node %d\n"
				"Well sorry nobody is perfect....\n",
				packsize, doomcom->numplayers, nnode);
		    }
		    else
		    {
		        end_tic++;  // send it anyway !
		        DEBFILE("sending it anyway\n");
		    }
		}
	        break;
	    }
	}
            
        // Send the tics, start_tic..(end_tic-1), in one packet.

        netbuffer->packettype = PT_SERVERTICS;
        netbuffer->u.serverpak.starttic = start_tic;
        netbuffer->u.serverpak.numtics = (end_tic - start_tic); // num tics
        netbuffer->u.serverpak.numplayers = LE_SWAP16_FAST(doomcom->numplayers);
        bufpos=(char *)&netbuffer->u.serverpak.cmds;
       
        // All the ticcmd_t, start_tic..(end_tic-1)
        for(ti=start_tic; ti<end_tic; ti++)
        {
	    int btic = BTIC_INDEX( ti );
	    TicCmdCopy((ticcmd_t*) bufpos, netcmds[btic], doomcom->numplayers);
	    bufpos += doomcom->numplayers * sizeof(ticcmd_t);
	}
            
        // All the textcmd, start_tic..(end_tic-1)
        for(ti=start_tic; ti<end_tic; ti++)
        {
	    btic = BTIC_INDEX( ti );
	    num_txt_p = bufpos++; // the num textcmd field
	    num_txt = 0;
	    for(pn=0; pn<MAXPLAYERS; pn++)
	    {
	        if((pn==0) || playeringame[pn])
	        {
		    int textlen = textcmds[btic][pn].buff[0];
		    if(textlen)
                    {
                        *(bufpos++) = pn;  // playernum
		        // The textcmd is textlen, text.
                        memcpy(bufpos, textcmds[btic][pn].buff, textlen+1);
                        bufpos += textlen+1;
                        num_txt++; // inc the number of textcmd
                    }
		}
	    }
	    // There must be a num txtcmd field for every tic, even if 0.
	    *num_txt_p = num_txt; // the number of textcmd
	}
        packsize = bufpos - (char *)&(netbuffer->u);

        HSendPacket(nnode, false, 0, packsize);

	// Record next tic for this net node.
	// Extratic causes redundant transmission of tics.
	ti = (end_tic - doomcom->extratics);  // normal
        // When tic is too large, only one tic is sent so don't go backward !
        if( ti <= start_tic )
	   ti = end_tic;  // num tics is too small for extratics
        if( ti < nettics[nnode] )
	   ti = nettics[nnode];
        nextsend_tic[nnode] = ti;
    }
    // node 0 is me !
    nextsend_tic[0] = maketic;
}

//
// TryRunTics
//
static void Local_Maketic(int realtics)
{
    if(dedicated)
	return;
    
    I_OsPolling();       // i_getevent
    D_ProcessEvents ();  // menu responder ???!!!
                         // Cons responder
                         // game responder call :
                         //    HU_responder,St_responder, Am_responder
                         //    F_responder (final)
                         //    and G_MapEventsToControls

    rendergametic=gametic;
    // translate inputs (keyboard/mouse/joystick) into game controls
    G_BuildTiccmd(&localcmds, realtics, 0);
    // [WDJ] requires splitscreen and player2 present
    if (cv_splitscreen.value && displayplayer2_ptr )
      G_BuildTiccmd(&localcmds2, realtics, 1);

#ifdef CLIENTPREDICTION2
    if( !paused && localgametic<gametic+BACKUPTICS)
    {
        P_MoveSpirit ( &players[consoleplayer], &localcmds, realtics );
        localgametic+=realtics;
    }
#endif
    localcmds.angleturn |= TICCMD_RECEIVED;
}

void SV_SpawnPlayer(int playernum, int x, int y, angle_t angle)
{
    // for futur copytic use the good x,y and angle!
    if( server )
    {
        int btic = BTIC_INDEX( maketic );
#ifdef CLIENTPREDICTION2
        netcmds[btic][playernum].x=x;
        netcmds[btic][playernum].y=y;
#endif
        netcmds[btic][playernum].angleturn=(angle>>16) | TICCMD_RECEIVED;
    }
}

// create missed tic
void SV_Maketic(void)
{
    int btic = BTIC_INDEX( maketic );
    int bticprev, i, player;
    byte nnode;

    for(nnode=0; nnode<MAXNETNODES; nnode++)
    {
        if(playerpernode[nnode] == 0)  continue;
       
        player=nnode_to_player[nnode];
        if((netcmds[btic][player].angleturn & TICCMD_RECEIVED) == 0)
        {
	    // Did not receive this tic.
	    GenPrintf(EMSG_debug, "MISS tic %4u for node %d\n",
		      maketic, nnode);
	    DEBFILE(va("MISS tic %4u for node %d\n", maketic, nnode));
#ifdef PARANOIA
	    if( devparm )
	        GenPrintf(EMSG_dev, "\2Client Miss tic %d\n", maketic);
#endif
	    // Copy the previous tic
	    bticprev = BTIC_INDEX(maketic-1);
	    for(i=0; i<playerpernode[nnode]; i++)
	    {
	        netcmds[btic][player] = netcmds[bticprev][player];
	        netcmds[btic][player].angleturn &= ~TICCMD_RECEIVED;
	        player = nnode_to_player2[nnode];
	    }
	}
    }
    // all tic are now present, make the next
    maketic++;
}

#ifdef DEBUGFILE
static  int     net_load;
#endif

void TryRunTics (tic_t realtics)
{
    // the machine have laged but is not so bad
    if(realtics>TICRATE/7) // FIXME: consistency failure!!
    {
        if(server)
            realtics=1;
        else
            realtics=TICRATE/7;
    }

    if(singletics)
        realtics = 1;

    if( realtics>= 1)
        COM_BufExecute();            

    NetUpdate();

    if(demoplayback)
    {
        cl_need_tic = gametic + realtics + cv_playdemospeed.value;
        // start a game after a demo
        maketic+=realtics;
        next_tic_send=maketic;
        next_tic_clear=next_tic_send;
    }

#ifdef DEBUGFILE
    if(realtics==0)
        if(net_load) net_load--;
#endif
    Net_Packet_Handler();

#ifdef DEBUGFILE
    if (debugfile && (realtics || cl_need_tic>gametic))
    {
        //SoM: 3/30/2000: Need long int in the format string for args 4 & 5.
        fprintf (debugfile,
                 "------------ Tryruntic : REAL:%lu NEED:%lu GAME:%lu LOAD: %i\n",
                 (unsigned long)realtics, (unsigned long)cl_need_tic,
		 (unsigned long)gametic, net_load);
        net_load=100000;
    }
#endif

    if (cl_need_tic > gametic)
    {
        if (demo_ctrl == DEMO_seq_advance)  // and not disabled
            D_DoAdvanceDemo ();
        else
        // run the count * tics
        while (cl_need_tic > gametic)
        {
            DEBFILE(va("============ Runing tic %u (local %d)\n",gametic, localgametic));

            G_Ticker ();
            ExtraDataTicker();
            gametic++;
            // skip paused tic in a demo
            if(demoplayback)
            {
	        if(paused)
		   cl_need_tic++;
	    }
            else
	    {
                consistency[ BTIC_INDEX( gametic ) ] = Consistency();
	    }
        }
    }
}

// By Server
// Send Tic updates
static void SV_Send_Tic_Update( int count )
{
    byte nn;

    // Find lowest tic, over all nodes.
    next_tic_send = gametic;
    for( nn=0; nn<MAXNETNODES; nn++)
    {
        if(nodeingame[nn]
	   && nettics[nn]<next_tic_send )
        {
	   next_tic_send = nettics[nn];
	}
    }

    // Don't erase tics not acknowledged
    if( maketic+count >= (next_tic_send+BACKUPTICS) )
        count = (next_tic_send+BACKUPTICS) - maketic - 1;

    while( count-- )
        SV_Maketic();  // create missed tics and increment maketic

    // clear only when acknowledged
    for( ; next_tic_clear<next_tic_send; next_tic_clear++)
        D_Clear_ticcmd(next_tic_clear);  // clear the maketic the new tic

    SV_Send_Tics();

    cl_need_tic=maketic; // the server is a client too
}


void NetUpdate(void)
{
    static tic_t gametime=0;
    tic_t        nowtime;
    int          realtics;	// time is actually long [WDJ]

    nowtime  = I_GetTime();
    realtics = nowtime - gametime;

    if( realtics <= 0 )
    {
      if( realtics > -100000 )  // [WDJ] 1/16/2009  validity check
	return;     // nothing new to update

      // [WDJ] 1/16/2009 something is wrong, like time has wrapped.
      // Program gets stuck waiting for this, so force it out.
      realtics = 5;
    }
    if( realtics>5 )
    {
        if( server )
            realtics=1;
        else
            realtics=5;
    }

    gametime = nowtime;

    if( !server )
        maketic = cl_need_tic;

    Local_Maketic (realtics);    // make local tic, and call menu ?!

    if( server && !demoplayback && !dedicated)
        CL_Send_ClientCmd();     // send server tic

    Net_Packet_Handler();  // get packet from client or from server

    // Client sends its commands after a receive of the server tic.
    // The server sends before because in single player is better.

    if( !server )
        CL_Send_ClientCmd();   // send tic cmd
    else
    {
        // By Server
        //Hurdler: added for acking the master server
        if( cv_internetserver.value )
            SendPingToMasterServer();

        if(!demoplayback)
	    SV_Send_Tic_Update( realtics );
    }
    Net_AckTicker();
    M_Ticker ();
    CON_Ticker();
    Filetx_Ticker();
}
