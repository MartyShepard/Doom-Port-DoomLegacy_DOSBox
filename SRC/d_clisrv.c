// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: d_clisrv.c 1389 2018-04-23 02:49:28Z wesleyjohnson $
//
// Copyright (C) 1998-2016 by DooM Legacy Team.
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
//      High Level Client / Server communications and functions.
//
//-----------------------------------------------------------------------------


#include <time.h>
#include <unistd.h>

#include "doomincl.h"
#include "doomstat.h"
#include "d_clisrv.h"
#include "command.h"
#include "i_net.h"
#include "i_tcp.h"
#include "i_system.h"
#include "i_video.h"
#include "v_video.h"
#include "d_net.h"
#include "d_netcmd.h"
#include "d_netfil.h"
#include "d_main.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "keys.h"
#include "m_argv.h"
#include "m_menu.h"
#include "console.h"
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
#include "t_script.h"

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

// The addition of wait messages should be transparent to previous network
// versions.
const int  NETWORK_VERSION = 24; // separate version number for network protocol (obsolete)


#define JOININGAME


#if NUM_SERVERTIC_CMD < BACKUPTICS
# error Not enough NUM_SERVERTIC_CMD
#endif

typedef enum {
   NS_idle,
   NS_searching_server,
   NS_waiting,
   NS_active,
   NS_shutdown
} network_state_e;

static network_state_e  network_state = NS_idle;

#define PREDICTIONQUEUE         BACKUPTICS
#define PREDICTIONMASK          (PREDICTIONQUEUE-1)

// Server state
boolean  server = true; // false when Client connected to other server
boolean  serverrunning = false;
byte     serverplayer;  // 255= no server player (same as -1)

// Server specific vars.
// player=255 when unused
// nodeingame[]=false when net node is unused
static byte     player_to_nnode[MAXPLAYERS];

#if 0
static tic_t    cl_maketic[MAXNETNODES];  // unused
#endif

// Server net node state
static byte     nnode_to_player[MAXNETNODES];  // 255= unused
static byte     nnode_to_player2[MAXNETNODES]; // splitscreen player, 255= unused
static byte     playerpernode[MAXNETNODES]; // used specialy for splitscreen
static byte     nodewaiting[MAXNETNODES];
static byte     consistency_faults[MAXNETNODES];
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

byte            servernode = 255; // server net node, 255=none

// Index for netcmds and textcmds
#define BTIC_INDEX( tic )  ((tic)%BACKUPTICS)
// Text buffer for textcmds.
// One extra byte at end for 0 termination, to protect against malicious use.
// Use textbuf_t from textcmdpak

static ticcmd_t localcmds;
static textbuf_t  localtextcmd;
static ticcmd_t localcmds2;  // player 2
static textbuf_t  localtextcmd2; // splitscreen player2

// engine
ticcmd_t        netcmds[BACKUPTICS][MAXPLAYERS];
static textbuf_t  textcmds[BACKUPTICS][MAXPLAYERS];

consvar_t cv_playdemospeed  = {"playdemospeed","0",CV_VALUE,CV_Unsigned};

consvar_t cv_server1 = { "server1", "192.168.1.255", CV_STRING|CV_SAVE, NULL };
consvar_t cv_server2 = { "server2", "", CV_STRING|CV_SAVE, NULL };
consvar_t cv_server3 = { "server3", "", CV_STRING|CV_SAVE, NULL };

CV_PossibleValue_t downloadfiles_cons_t[] = {{0,"Allowed"}
                                           ,{1,"No Download"}
                                           ,{0,NULL}};

consvar_t cv_downloadfiles = {"downloadfiles"  ,"0", CV_SAVE, downloadfiles_cons_t};


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
    I_SoftError ("ExpandTics: strange value %i at maketic %i\n", low, maketic);
#endif
    return 0;
}

// -----------------------------------------------------------------
//  Some extra data function for handle textcmd buffer
// -----------------------------------------------------------------

// NetXCmd indirection.
static void (*netxcmd_func[MAXNETXCMD]) (xcmd_t * xc);

void Register_NetXCmd(netxcmd_e cmd_id, void (*cmd_f) (xcmd_t * xc))
{
#ifdef PARANOIA
   if(cmd_id >= MAXNETXCMD)
      I_Error("NetXCmd id %d exceeds defined range", cmd_id);
   if(netxcmd_func[cmd_id]!=0)
      I_Error("NetXCmd id %d already registered", cmd_id);
#endif
   netxcmd_func[cmd_id] = cmd_f;
}

//  cmd_id :  X command, XD_
//  param : parameter strings
//  param_len : number of parameter strings
void Send_NetXCmd(byte cmd_id, void *param, int param_len)
{
   if(demoplayback)
       return;

   int textlen = localtextcmd.len;
   if( (textlen + 1 + param_len) > MAXTEXTCMD)  // with XD_ and param
   {
#ifdef PARANOIA
       I_SoftError("Net command [%i] exceeds buffer: netcmd %d\n", cmd_id, 1);
#else
       GenPrintf(EMSG_warn, "\2Net Command [%] exceeds buffer\n", 1);
#endif
       return;
   }

   // Append to player1 text commands.
   // First byte is the cmd, followed by its parameters (binary or string).
   localtextcmd.text[textlen++] = cmd_id; // XD_
   if(param && param_len)
   {
       memcpy(&localtextcmd.text[textlen], param, param_len);
       textlen += param_len;
   }
   localtextcmd.len = textlen;
}

// splitscreen player
void Send_NetXCmd2(byte cmd_id, void *param, int param_len)
{
   if(demoplayback)
       return;

   int textlen = localtextcmd2.len;
   if( (textlen + 1 + param_len) > MAXTEXTCMD) // with XD_ and param
   {
#ifdef PARANOIA
       I_SoftError("Net command [%i] exceeds buffer: netcmd %d\n", cmd_id, 2);
#else
       GenPrintf(EMSG_warn, "\2Net Command [%i] exceeds buffer\n", 2);
#endif
       return;
   }

   // Append
   localtextcmd2.text[textlen++] = cmd_id;  // XD_
   if(param && param_len)
   {
       memcpy(&localtextcmd2.text[textlen], param, param_len);
       textlen += param_len;
   }
   localtextcmd2.len = textlen;
}


// NetXCmd with 2 parameters.
void Send_NetXCmd_p2(byte cmd_id, byte param1, byte param2)
{
    byte buf[3];

    buf[0] = param1;
    buf[1] = param2;
    Send_NetXCmd(cmd_id, &buf, 2);
}


static void D_Clear_ticcmd(int tic)
{
    int i;
    int btic = BTIC_INDEX( tic );

    for(i=0;i<MAXPLAYERS;i++)
    {
        textcmds[btic][i].len = 0;  // textlen
        netcmds[btic][i].angleturn = 0; //&= ~TICCMD_RECEIVED;
    }
    DEBFILE(va("Clear tic %5d [%2d]\n", tic, btic));
}


void ExtraDataTicker(void)
{
    int btic = BTIC_INDEX( gametic );
    int pn;
    textbuf_t * textbuf;
    byte * endbuf;
    byte * endtxt;  // independent of changes by called xfunc
    unsigned int textlen;
    xcmd_t  xcmd;

    // There is a textcmd buffer [MAXTEXTCMD+1] for each active player.
    for(pn=0; pn<MAXPLAYERS; pn++)
    {
        // Execute commands of any player in the game, and always for pn=0.
        if( ! playeringame[pn] && (pn>0) )  continue;

        xcmd.playernum = pn;
        textbuf = & textcmds[btic][pn];
        textlen = textbuf->len;  // 0..255
        if( textlen == 0 )  continue;  // empty text
#if MAXTEXTCMD < 255
        if( textlen > MAXTEXTCMD )   textlen = MAXTEXTCMD;  // bad length
#endif       
        // set extra termination byte at end of buffer (text[MAXTEXTCMD+1])
        endbuf = & textbuf->text[MAXTEXTCMD]; // last char
        endbuf[0] = 0;  // Protect against malicious strings.
        // Commands can have 0 strings, 1 string, or 2 strings.
        // Inventory has just a byte number.
        xcmd.curpos = (byte*)&(textbuf->text[0]);  // start of command
        endtxt = &(textbuf->text[textlen]);  // after last char of text
        if( endtxt >= endbuf )  endtxt = endbuf;
        endtxt[0] = 0;  // Protect against malicious strings.
        xcmd.endpos = endtxt;  // end of text + 1
        // One or more commands are within curpos..endpos-1
        while(xcmd.curpos < endtxt)
        {
            xcmd.cmd = *(xcmd.curpos++);  // XD_ command
            if(xcmd.cmd < MAXNETXCMD && netxcmd_func[xcmd.cmd])
            {
                // Execute a NetXCmd.
                // The NetXCmd must update xcmd.curpos.
                DEBFILE(va("Executing xcmd %d player %d ", xcmd.cmd, pn));
                (netxcmd_func[xcmd.cmd])(&xcmd);
                // nextcmd_func updates curpos, without knowing textlen
                DEBFILE("Execute done\n");
            }
            else
            {
                // [WDJ] Why should a bad demo command byte be fatal.
                I_SoftError("Got unknown net/demo command [%d]=%d len=%d\n",
                           (xcmd.curpos - &(textbuf->text[0])),
                           xcmd.cmd, textlen);
                D_Clear_ticcmd(btic);
                return;
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
//   byte   | LEGACY 1.13:  XDNAMEANDCOLOR, XD_WEAPON_PREF bits
//            LEGACY 1.20:  XD_ codes
//            Determines what parameters follow.
//
// LEGACY 1.12 XD bits
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

// Save textcmd to demo using textbuf_t format.
boolean AddLmpExtradata(byte **demo_point, int playernum)
{
    int  btic = BTIC_INDEX( gametic );
    textbuf_t * textbuf = & textcmds[btic][playernum];
    int  textlen = textbuf->len;

    if(textlen == 0)  // anything in the buffer
        return false;

    // Demo format matches textbuf_t, length limited
    memcpy(*demo_point, textbuf, textlen+1);  // len,text
    *demo_point += textlen+1;
    return true;
}

void ReadLmpExtraData(byte **demo_pointer, int playernum)
{
    unsigned int extra_len;
    int btic = BTIC_INDEX( gametic );
    textbuf_t * textbuf = & textcmds[btic][playernum];
    byte  * dp;

    if(!demo_pointer)  goto no_text_cmd;

    dp = *demo_pointer;
    if( dp == NULL )  goto no_text_cmd;

    extra_len = dp[0];   // textbuf->len
    // [WDJ] Clean separation of old and new formating.
    if(demoversion==112) // support old demos v1.12
    {
        // Different, limited, XCmd format.
        byte  * p = dp;
        int  textlen = 0;
        byte  ex;

        // extra_len is length of extra data, incl length.
        ex = p[1];  // XCmd bits
        p += 2;  // skip extra_len and XCmd
        if(ex & 1)
        {
            textbuf->text[textlen++] = XD_NAMEANDCOLOR;
            memcpy(&textbuf->text[textlen],
                   p,
                   MAXPLAYERNAME+1);
            p+=MAXPLAYERNAME+1;
            textlen += MAXPLAYERNAME+1;
        }
        if(ex & 2)
        {
            textbuf->text[textlen++] = XD_WEAPONPREF;
            memcpy(&textbuf->text[textlen],
                   p,
                   NUMWEAPONS+2);
            p+=NUMWEAPONS+2;
            textlen += NUMWEAPONS+2;
        }
        textbuf->len = textlen;
    }
    else
    {
        // Demo format matches textbuf_t, length limited.
        // extra_len is textbuf->len, excluding len field.
        extra_len ++;
        memcpy(textbuf, dp, extra_len);  // len,text
    }
    // update demo pointer
    *demo_pointer = dp + extra_len;
    return;

no_text_cmd:
    textbuf->len = 0;  // empty text cmd
    return;
}


// -----------------------------------------------------------------
//  end extra data function for lmps
// -----------------------------------------------------------------

// ----- Server/Client Responses

// Client state
typedef enum {
   CLM_searching,
   CLM_server_files,
   CLM_download_req,
   CLM_download_files,
   CLM_askjoin,
   CLM_wait_join_response,
   CLM_download_savegame,
   CLM_connected
} cl_mode_t;

static cl_mode_t  cl_mode = CLM_searching;

static void CL_ConnectToServer(void);
static int16_t  Consistency(void);
static void Net_Packet_Handler(void);


// By Client.
// Send a request to join game.
// Called by CL_ConnectToServer.
static boolean  CL_Send_Join( void )
{
    GenPrintf(EMSG_hud, "Send join request...\n");
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
    netbuffer->u.clientcfg.subversion = LE_SWAP32(NETWORK_VERSION);

    return HSendPacket(servernode,true,0,sizeof(clientconfig_pak_t));
}


// By Server.
// Reply to request for server info.
//   reqtime : the send time of the request
static void SV_Send_ServerInfo(int to_node, tic_t reqtime)
{
    byte  *p;

    netbuffer->packettype=PT_SERVERINFO;
    netbuffer->u.serverinfo.version = VERSION;
    netbuffer->u.serverinfo.subversion = LE_SWAP32(NETWORK_VERSION);
    // return back the time value so client can compute their ping
    netbuffer->u.serverinfo.trip_time = LE_SWAP32(reqtime);
    netbuffer->u.serverinfo.numberofplayer = doomcom->numplayers;
    netbuffer->u.serverinfo.maxplayer = cv_maxplayers.value;
    netbuffer->u.serverinfo.load = 0;        // unused for the moment
    netbuffer->u.serverinfo.deathmatch = cv_deathmatch.EV;  // and command line
    strncpy(netbuffer->u.serverinfo.servername, cv_servername.string, MAXSERVERNAME);
    if(game_map_filename[0])
    {
        // Map command external wad file.
        strcpy(netbuffer->u.serverinfo.mapname,game_map_filename);
    }
    else
    {
        // existing map       
        strcpy(netbuffer->u.serverinfo.mapname,G_BuildMapName(gameepisode,gamemap));
    }

    p=Put_Server_FileNeed();

    HSendPacket(to_node, false, 0, p-((byte *)&netbuffer->u));
}


// By Server.
// Accept player joining the game.
static boolean SV_Send_ServerConfig(int to_node)
{
    int   i,playermask=0;
    xcmd_t xc;

    netbuffer->packettype=PT_SERVERCFG;
    for(i=0;i<MAXPLAYERS;i++)
    {
         if(playeringame[i])
              playermask|=1<<i;
    }

    netbuffer->u.servercfg.version         = VERSION;
    netbuffer->u.servercfg.subversion      = LE_SWAP32(NETWORK_VERSION);

    netbuffer->u.servercfg.serverplayer    = serverplayer;
    netbuffer->u.servercfg.totalplayernum  = doomcom->numplayers;
    netbuffer->u.servercfg.playerdetected  = LE_SWAP32(playermask);
    netbuffer->u.servercfg.gametic         = LE_SWAP32(gametic);
    netbuffer->u.servercfg.clientnode      = to_node;
    netbuffer->u.servercfg.gamestate       = gamestate;

    xc.playernum = 0;
    xc.curpos = netbuffer->u.servercfg.netcvarstates;
    xc.endpos = xc.curpos + NETCVAR_BUFF_LEN - 1;
    CV_SaveNetVars( &xc );
    // curpos is 1 past last cvar (if none then is at netcvarstates)

    return HSendPacket(to_node, true, 0, xc.curpos - ((byte *)&netbuffer->u));
}


// Broadcast to all connected nodes
static void SV_SendPacket_All( boolean reliable, size_t size_packet, const char * msg )
{
    int nn;
    for(nn=1; nn<MAXNETNODES; nn++)
    {
        if(nodeingame[nn])
        {
            HSendPacket(nn, reliable, 0, size_packet);
            if( msg )	   
                debug_Printf( "%s[ %d ]\n", msg, nn );
        }
    }
}

// [WDJ] Update state by sever.
// By Server
static void CL_Send_State( byte server_pause )
{
    netbuffer->packettype=PT_STATE;
    netbuffer->u.state.gametic = LE_SWAP32_FAST(gametic);
    netbuffer->u.state.p_rand_index = P_GetRandIndex(); // to sync P_Random
    netbuffer->u.state.server_pause = server_pause;

    SV_SendPacket_All( true, sizeof(state_pak_t), NULL );
}

// By Client
static void state_handler( void )
{
    // Message is PT_STATE
    tic_t serv_gametic = LE_SWAP32_FAST(netbuffer->u.state.gametic);
    if( serv_gametic != gametic )
    {
        if( verbose > 1 )
            GenPrintf( EMSG_ver, "PT_STATE: gametic %i, server gametic %i\n", gametic, serv_gametic );
        // Gametic cannot be fixed directly, need game commands.        
        // It may just be behind by a tic or two.
    }
    else if( P_GetRandIndex() != netbuffer->u.state.p_rand_index )
    {
        // Same gametic, but different P_Random index.
        GenPrintf( EMSG_warn, "PT_STATE: gametic %i, update P_random index %i to %i\n",
		 gametic, P_GetRandIndex(), netbuffer->u.state.p_rand_index );
        P_SetRandIndex( netbuffer->u.state.p_rand_index ); // to sync P_Random
    }
    paused = netbuffer->u.state.server_pause;

#if 0
    debug_Printf( "STATE: gametic %i, P_Random [%i], paused %i\n",
          gametic, netbuffer->u.state.p_rand_index, netbuffer->u.state.server_pause );
#endif
}


#ifdef JOININGAME

// By Server.
// Send a save game to the client.
static void SV_Send_SaveGame(int to_node)
{
    size_t  length;
    byte    was_paused = paused;

    CL_Send_State( 1 );  // pause game during download
     
    P_Alloc_savebuffer( 1 );	// large buffer, but no header
    if(! savebuffer)   return;

    P_Write_Savegame_Header( NULL, 1 );  // Netgame header
    P_SaveGame();  // fill buffer with game data
    // buffer will automatically grow as needed.

    length = P_Savegame_length();
    if( length < 0 )   return;	// overrun buffer
   
    // then send it !
    SV_SendData(to_node, savebuffer, length, TAH_MALLOC_FREE, 0);
    // SendData frees the savebuffer using free() after it is sent.
    // This is the only use of TAH_MALLOC_FREE.
    
    CL_Send_State( was_paused );  // unpause maybe
    paused = was_paused;
}

static const char *tmpsave="$$$.sav";

// By Client.
// Act upon the received save game from server.
static void CL_Load_Received_Savegame(void)
{
    savegame_info_t   sginfo;  // read header info

    // Use savebuffer and save_p from p_saveg.c.
    // There cannot be another savegame in progress when this occurs.
    // [WDJ] Changed to use new load savegame file, with smaller buffer.
    if( P_Savegame_Readfile( tmpsave ) < 0 )  goto cannot_read_file;
    // file is open and savebuffer allocated

    // Read netgame header.
    sginfo.msg[0] = 0;
    if( ! P_Read_Savegame_Header( & sginfo, 1 ) )  goto load_failed;

    GenPrintf(EMSG_hud, "Loading savegame\n");

    G_setup_VERSION();

    // Sever will control pause during download.
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
    I_SoftError ("Can't read savegame sent\n");
    goto failed_exit; // must deallocate savebuffer

load_failed:
    GenPrintf(EMSG_error, "Can't load the level !!!\n%s", sginfo.msg);
failed_exit:
    // needed when there are error tests before Closefile.
    P_Savegame_Error_Closefile();  // deallocate savebuffer
    return;
}

#endif



// ----- Consistency fail, repair position.
static tic_t prev_tic = 0;


// By Server.
// Send a player repair message.
//  pn : player number
//  to_node : the net node
static void SV_Send_player_repair( int pn, byte to_node )
{
    mobj_t * mo;

    netbuffer->u.repair.repair_type = RQ_PLAYER;
    netbuffer->u.repair.pos.id_num = pn;
    if( ! playeringame[pn] )  return;
    mo = players[pn].mo;
    if( ! mo )  return;
    netbuffer->u.repair.pos.angle = mo->angle;
    netbuffer->u.repair.pos.x = mo->x;
    netbuffer->u.repair.pos.y = mo->y;
    netbuffer->u.repair.pos.z = mo->z;
    netbuffer->u.repair.pos.momx = mo->momx;
    netbuffer->u.repair.pos.momy = mo->momy;
    netbuffer->u.repair.pos.momz = mo->momz;
    HSendPacket(to_node, false, 0, sizeof(repair_pak_t));
}


// By Client.
// Repair the player from the repair message in the netbuffer.
static void CL_player_repair( void )
{
    int pn;
    mobj_t * mo;

    pn = netbuffer->u.repair.pos.id_num;
    if( ! playeringame[pn] )  return;
    mo = players[pn].mo;
    if( ! mo )  return;
    mo->angle = netbuffer->u.repair.pos.angle;
    mo->x = netbuffer->u.repair.pos.x;
    mo->y = netbuffer->u.repair.pos.y;
    mo->z = netbuffer->u.repair.pos.z;
    mo->momx = netbuffer->u.repair.pos.momx;
    mo->momy = netbuffer->u.repair.pos.momy;
    mo->momz = netbuffer->u.repair.pos.momz;
}

// By Server.
// Send a position repair to the client.
//  to_node : to the player node
//  repair_type : RQ_PLAYER, RQ_SUG_SAVEGAME
static void SV_Send_Pos_repair( byte repair_type, byte to_node )
{
    netbuffer->packettype = PT_REPAIR;
    netbuffer->u.repair.gametic = LE_SWAP32_FAST(gametic);
    netbuffer->u.repair.p_rand_index = P_GetRandIndex(); // to sync P_Random
       
#ifdef JOININGAME
    if( repair_type == RQ_SUG_SAVEGAME )
    {
        netbuffer->u.repair.repair_type = RQ_SUG_SAVEGAME;
        HSendPacket(to_node, false, 0, sizeof(repair_pak_t));
        return;
    }
#endif

    SV_Send_player_repair( nnode_to_player[ to_node ], to_node );
   
    if( nnode_to_player2[ to_node ] < 255 )
    {
        SV_Send_player_repair( nnode_to_player2[ to_node ], to_node );
    }
}


#ifdef JOININGAME
// By Client.
// Send a request for a savegame.
//  msg_type : RQ_REQ_SAVEGAME, RQ_REQ_PLAYER
static void CL_Send_Req_repair( repair_type_e msg_type )
{
    netbuffer->packettype=PT_REPAIR;
    netbuffer->u.repair.repair_type = msg_type;

    HSendPacket(servernode, true, 0, sizeof(repair_pak_t));
}
#endif

// [WDJ] Attempt to fix consistency errors.
// By Client and Server
// Act upon the received position repair from server.
static void repair_handler( byte nnode )
{
    // Message is PT_REPAIR
    int msg_type = netbuffer->u.repair.repair_type;
    if( (msg_type < RQ_REQ_TO_SERVER) && ! server )
    {
        // Server repairs client.
        gametic = LE_SWAP32_FAST(netbuffer->u.repair.gametic);
        P_SetRandIndex( netbuffer->u.repair.p_rand_index ); // to sync P_Random
    }
   
    switch( msg_type )
    {
     case RQ_PLAYER:
        // Server repairs player position.
        CL_player_repair();
        break;

#ifdef JOININGAME
     case RQ_SUG_SAVEGAME:
        // Server suggests downloading a savegame from the server.
        CL_Prepare_Download_SaveGame(tmpsave);
        CL_Send_Req_repair( RQ_REQ_SAVEGAME ); // to server
        // Loop here while savegame is downloaded.
        while( cl_fileneed[0].status != FS_FOUND )
        {
            Net_Packet_Handler();
            if( !server && !netgame )
                goto reset_to_title_exit;  // connection closed by cancel or timeout

            Net_AckTicker();

            // Operations performed only once every tic.
            if( prev_tic != I_GetTime() )
            {
                prev_tic = I_GetTime();

                // User response handler
                I_OsPolling();
                switch( I_GetKey() )
                {
                  case 'q':
                  case KEY_ESCAPE:
                     goto reset_to_title_exit;
                }

                // Server upkeep
                if( Filetx_file_cnt )  // File download in progress.
                    Filetx_Ticker();
            }
        }

        // Have received the savegame from the server.
        CL_Load_Received_Savegame();
        CL_Send_Req_repair( RQ_REQ_PLAYER ); // to server
        break;

     case RQ_REQ_SAVEGAME:
        // Client has requested a savegame repair
        if( ! server ) break;   // only handled by server
        if(gamestate == GS_LEVEL)
        {
            SV_Send_SaveGame( nnode ); // send game data
            GenPrintf(EMSG_info, "Send savegame\n");
        }
        break;
       
     case RQ_REQ_PLAYER:
        // Client has requested a player repair
        if( ! server ) break;   // only handled by server
        SV_Send_Pos_repair( RQ_PLAYER, nnode );
#endif
     default:
        break;
    }
    return;
   
reset_to_title_exit:
    CL_Reset();
    D_StartTitle();
    return;
}


// ----- Wait for Server to start net game.
//#define WAITPLAYER_DEBUG

static byte  num_netnodes;
static byte  num_netplayer;  // wait for netplayer, some nodes are 2 players
static byte  wait_netplayer = 0;
static tic_t wait_tics  = 0;

static void SV_Send_NetWait( void )
{
    netbuffer->packettype = PT_NETWAIT;
    netbuffer->u.netwait.num_netplayer = num_netplayer;
    netbuffer->u.netwait.wait_netplayer = wait_netplayer;
    netbuffer->u.netwait.wait_tics = LE_SWAP16( wait_tics );
    netbuffer->u.netwait.p_rand_index = P_GetRandIndex(); // to sync P_Random
#ifdef WAITPLAYER_DEBUG
    debug_Printf( "WaitPlayer update: num_netnodes=%d num_netplayer=%d  wait_netplayer=%d  wait_tics=%d\n",
               num_netnodes, num_netplayer, wait_netplayer, wait_tics );
    SV_SendPacket_All( false, sizeof(netwait_pak_t), "  sent to player" );
#else
    SV_SendPacket_All( false, sizeof(netwait_pak_t), NULL );
#endif
}

void D_WaitPlayer_Drawer( void )
{
    WI_Draw_wait( num_netnodes, num_netplayer, wait_netplayer, wait_tics );
}

void D_WaitPlayer_Setup( void )
{
    if( netgame )
    {
        if( server )
        {
            // Wait for all player nodes, during netgame.
            wait_netplayer = cv_wait_players.value;
            wait_tics = cv_wait_timeout.value * TICRATE;
        }
        else
        {
            // Wait indefinite, until server updates the wait.
            wait_netplayer = 99;
            wait_tics = 0;
        }
    }
    else
    {
        // Single player and local games.
        wait_netplayer = 0;
        wait_tics = 0;
    }
    gamestate = wipegamestate = GS_WAITINGPLAYERS;
}

// Return true when start game.
static boolean  D_WaitPlayer_Ticker()
{
    int  nn;

    if( server )
    {
        // Count the net nodes.
        num_netnodes=0;
        num_netplayer=0;
        for(nn=0; nn<MAXNETNODES; nn++)
        {
            // Only counting nodes with players.
            if(nodeingame[nn])
            {
                if( playerpernode[nn] > 0 )
                {
                    num_netnodes ++;
                    num_netplayer += playerpernode[nn];
                }
            }
        }

        if( wait_tics > 0 || wait_netplayer > 0 )
        {
            // Service the wait tics.
            if( wait_tics > 1 )
            {
                wait_tics--;  // count down to 1
            }

            static  byte net_update_cnt = 0;
            if( ++net_update_cnt > 4 )
            {
                net_update_cnt = 0;
                // Update all net nodes
                SV_Send_NetWait();
            }
        }
    }

    // Clients and Server do same tests.
    if( wait_netplayer > 0 )
    {
        // Waiting on player net nodes, with or without timeout.
        if( num_netplayer < wait_netplayer )
        {
            // Waiting for player net nodes.
            if( wait_tics != 1 )  // timeout at 1
               goto wait_ret;  // waiting only for number of players
        }
    }
    else if( wait_tics > 1 )
        goto wait_ret;  // waiting for players by timeout

    if( server )
    {
        // All nodes need to get info to stop waiting.
        SV_Send_NetWait();
#ifdef WAITPLAYER_DEBUG
        debug_Printf( "Start game sent to players at tic=%d\n", gametic   );
#endif
    }
    return true;  // start game
    

wait_ret:
    return false;  // keep waiting
}

boolean  D_WaitPlayer_Response( int key )
{
    // User response handler
    switch( key )
    {
     case 'q':
     case KEY_ESCAPE:
        if( ! dedicated )
        {
            D_Quit_NetGame();
            SV_StopServer();
            SV_ResetServer();
            D_StartTitle();
            netgame = multiplayer = false;
            return true;
        }
        break;
     case 's':
        if( server )
        {
            // Start game, stop waiting for player nodes.
            wait_netplayer = 0;
            wait_tics = 0;
            SV_Send_NetWait();
#ifdef WAITPLAYER_DEBUG
            debug_Printf( "Start game (key) sent at tic=%d\n", gametic );
#endif
            return true;
        }
        break;
    }
    return false;
}


// ----- Connect to Server

// By Client.
// Ask the server for some info.
//   to_node : when BROADCAST_NODE then all servers will respond
// Called by: CL_Broadcast_AskInfo, CL_Update_ServerList, CL_ConnectToServer
static void CL_Send_AskInfo( int to_node )
{
    netbuffer->packettype = PT_ASKINFO;
    netbuffer->u.askinfo.version = VERSION;
    netbuffer->u.askinfo.send_time = LE_SWAP32(I_GetTime());
    HSendPacket(to_node, false, 0, sizeof(askinfo_pak_t));
}


// By Client.
// Broadcast to find some servers.
//   addrstr: broadcast addr string
// Called by: CL_Update_ServerList
static void CL_Broadcast_AskInfo( char * addrstr )
{
    // Modifies the broadcast address.
    if( addrstr
        && Bind_Node_str( BROADCAST_NODE, addrstr, server_sock_port ) )
    {
//        debugPrintf( "CL_Broadcast_AskInfo  server_sock_port = %d\n", server_sock_port );       
       
        CL_Send_AskInfo( BROADCAST_NODE );
    }
}


// --- ServerList

server_info_t serverlist[MAXSERVERLIST];
int serverlistcount=0;

// Clear the serverlist, closing connections.
//  keep_node: except this server node
static void SL_Clear_ServerList( int keep_node )
{
    int i;
    for( i=0; i<serverlistcount; i++ )
    {
        if( serverlist[i].server_node != keep_node )
        {
            Net_CloseConnection(serverlist[i].server_node);
            serverlist[i].server_node = 0;
        }
    }
    serverlistcount = 0;
}

// Find the server in the serverlist.
// Called by: SL_InsertServer, CL_ConnectToServer
static int SL_Find_Server( byte nnode )
{
    int i;
    for( i=0; i<serverlistcount; i++ )
    {
        if( serverlist[i].server_node == nnode )
            return i;
    }

    return -1;
}

// Insert the server into the serverlist.
static void SL_InsertServer( serverinfo_pak_t * info, byte nnode)
{
    tic_t  test_time;
    server_info_t se;
    int i, i2;

    // search if not already on it
    i = SL_Find_Server( nnode );
    if( i < 0 )
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
// Called by M_Connect.
void CL_Update_ServerList( boolean internetsearch )
{
    int  i;

    SL_Clear_ServerList(0);

    if( !netgame )
    {
        server = false;  // To get correct port
        if( ! I_NetOpenSocket() )  return;  // failed to get socket
        netgame = true;
        multiplayer = true;
        network_state = NS_searching_server;
    }

    // Search for local servers.
    CL_Broadcast_AskInfo( cv_server1.string );
    CL_Broadcast_AskInfo( cv_server2.string );
    CL_Broadcast_AskInfo( cv_server3.string );

    if( internetsearch )
    {
        msg_server_t *server_list;

        server_list = MS_Get_ShortServersList();
        if( server_list )
        {
            // Poll the servers on the list to get ping time.
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



// ----- Connect to Server

// By User, future Client, and by server not dedicated.
// Use adaptive send using net_bandwidth and stat.sendbytes.
// Called by Command_connect, SV_SpawnServer
//  servernode: if set then reconnect, else search
static void CL_ConnectToServer( void )
{
    int  i;
    tic_t   askinfo_tic = 0;  // to repeat askinfo

    cl_mode = CLM_searching;
    D_WaitPlayer_Setup();

    if( servernode >= MAXNETNODES )
    {
        // init value and BROADCAST_NODE
        GenPrintf(EMSG_hud, "Searching for a DoomLegacy server ...\n");
    }
    else
        GenPrintf(EMSG_hud, "Contacting the DoomLegacy server ...\n");

    DEBFILE(va("Waiting %d players\n", wait_netplayer));

    SL_Clear_ServerList(servernode);  // clear all except the current server

    GenPrintf(EMSG_hud, "Press Q or ESC to abort\n");
    // Every player goes through here to connect to game, including a
    // single player on the server.
    // Loop until connected or user escapes.
    // Because of the combination above, this loop must include code for
    // server responding.
    do {
        switch(cl_mode) {
            case CLM_searching :
                // serverlist is updated by GetPacket function
                if( serverlistcount > 0 )
                {
                    cl_mode = CLM_server_files;
                    break;
                }
                // Don't have a serverlist.
                // Poll the server (askinfo packet).
                if( askinfo_tic <= I_GetTime() )
                {
                    // Don't be noxious on the network.
                    // Every 2 seconds is often enough.
                    askinfo_tic = I_GetTime() + (TICRATE*2);
                    if( servernode < MAXNETNODES )
                    {
                        // Specific server.
                        CL_Send_AskInfo(servernode);
                    }
                    else
                    {
                        // Any
                        CL_Update_ServerList( false );
                    }
                }
                break;
            case CLM_server_files :
                // Have a serverlist, serverlistcount > 0.
                // This can be a response to our broadcast request
                if( servernode < MAXNETNODES )
                {
                    // Have a current server.  Find it in the serverlist.
                    i = SL_Find_Server(servernode);
                    // Check if it shutdown, or is missing for some reason.
                    if (i<0)
                        return;  // go back to user
                }
                else
                {
                    // Invalid servernode, get best server from serverlist.
                    i = 0;
                    servernode = serverlist[i].server_node;
                    GenPrintf(EMSG_hud, " Found, ");
                }
                // Check server for files needed.
                CL_Got_Fileneed(serverlist[i].info.num_fileneed,
                                serverlist[i].info.fileneed    );
                GenPrintf(EMSG_hud, " Checking files ...\n");
                switch( CL_CheckFiles() )
                {
                 case CFR_no_files:
                 case CFR_all_found:
                    cl_mode = CLM_askjoin;
                    break;
                 case CFR_download_needed:
                    cl_mode = CLM_download_req;
                    break;
                 case CFR_iwad_error: // cannot join for some reason
                 case CFR_insufficient_space:
                 default:
                    goto reset_to_title_exit;
                }
                break;
            case CLM_download_req:
                // Must download something.
                // Check -nodownload switch, or request downloads.
                switch( Send_RequestFile() )
                {
                 case RFR_success:
                    Net_GetNetStat();  // init for later display
                    cl_mode = CLM_download_files;
                    break;
                 case RFR_send_fail:
                    break;  // retry later
                 case RFR_insufficient_space: // Does not seem to be used.
                 case RFR_nodownload:
                 default:
                    // Due to -nodownload switch, or other fatal error.
                    goto  reset_to_title_exit;
                }
                break;
            case CLM_download_files :
                // Wait test, and display loading files.
                if( CL_waiting_on_fileneed() )
                    break; // continue looping
                // Have all downloaded files.
                cl_mode = CLM_askjoin;
                // continue into next case
            case CLM_askjoin :
                if( ! CL_Load_ServerFiles() )
                {
                    // Cannot load some file.
                    goto  reset_to_title_exit;
                }
#ifdef JOININGAME
                // prepare structures to save the file
                // WARNING: this can be useless in case of server not in GS_LEVEL
                // but since the network layer don't provide ordered packet ...
                // This can be repeated, if CL_Send_Join fails.
                CL_Prepare_Download_SaveGame(tmpsave);
#endif
                if( CL_Send_Join() )  // join game
                    cl_mode = CLM_wait_join_response;
                break;
            case CLM_wait_join_response :
                // see server_cfg_handler()
                break;
#ifdef JOININGAME
            case CLM_download_savegame :
                M_DrawTextBox( 2, NETFILE_BOX_Y, 38, 6);
                V_DrawString (30, NETFILE_BOX_Y+8, 0, "Download Savegame");
                if( cl_fileneed[0].status != FS_FOUND )
                    break; // continue loop

                // Have received the savegame from the server.
                CL_Load_Received_Savegame();
                gamestate = GS_LEVEL;  // game loaded
                cl_mode = CLM_connected;
                // don't break case continue to CLM_connected
#endif
            case CLM_connected :
                break;
        }

        Net_Packet_Handler();
        if( !server && !netgame )
            goto reset_to_searching;  // connection closed by cancel or timeout

        Net_AckTicker();

        // Operations performed only once every tic.
        if( prev_tic != I_GetTime() )
        {
            prev_tic = I_GetTime();

            // User response handler
            I_OsPolling();
            switch( I_GetKey() )
            {
              case 'q':
              case KEY_ESCAPE:
                 goto quit_ret;
            }

            if( Filetx_file_cnt )  // File download in progress.
                Filetx_Ticker();

#if 0
            // Supporting the wait during connect, like it was in the previous
            // code, has marginal value.  Seems to cause more problems.
            D_WaitPlayer_Ticker( 0 );
            if( wait_tics > 0 || wait_netplayer > 0 )
                D_WaitPlayer_Drawer();
#endif
            CON_Drawer ();
            I_FinishUpdate ();              // page flip or blit buffer
        }
    } while ( cl_mode != CLM_connected );

    DEBFILE(va("Synchronization Finished\n"));

    consoleplayer&= ~DRONE;
    displayplayer = consoleplayer;
    consoleplayer_ptr = displayplayer_ptr = &players[consoleplayer];
    return;

reset_to_searching:
    cl_mode = CLM_searching;
    return;

quit_ret:
    M_SimpleMessage ("Network game synchronization aborted.\n\nPress ESC\n");
    goto reset_to_title_exit;

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
        servernode = 0;  // server is self
        server = true;
        // should be but...
        //SV_SpawnServer();
    }
    else
    {
        // used in menu to connect to a server in the list
        if( netgame && strcasecmp(COM_Argv(1),"node")==0 )
        {
            // example: "connect node 4"
            servernode = atoi(COM_Argv(2));
        }
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
            {
                // Connect to first lan server found.
                servernode = BROADCAST_NODE;
            }
            else
            if( I_NetMakeNode )
            {
                // Connect to server at IP addr.
                servernode = I_NetMakeNode(COM_Argv(1));
            }
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

static void Reset_NetNode(byte nnode);

// Called by Kick cmd.
static void CL_RemovePlayer(int playernum)
{
    int i;
    if( server && !demoplayback )
    {
        byte nnode = player_to_nnode[playernum];
        if( playerpernode[nnode] )
            playerpernode[nnode]--;
        if( playerpernode[nnode] == 0 )
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

// By Client and non-specific code, to reset client connect.
void CL_Reset (void)
{
    if (demorecording)
        G_CheckDemoStatus ();

    // reset client/server code
    DEBFILE(va("==== Client reset ====\n"));

    if( servernode < MAXNETNODES )
    {
        // Close connection to server
        nodeingame[servernode]=false;
        Net_CloseConnection(servernode);
    }
    D_CloseConnection();         // netgame=false
    multiplayer = false;
    servernode=0;  // server to self
    server=true;
    doomcom->num_player_netnodes=1;
    doomcom->numplayers=1;
    SV_StopServer();
    SV_ResetServer();

    T_Clear_HubScript(); //DarkWolf95: Originally implemented by Exl
    fs_fadealpha = 0;
    HU_Clear_FSPics();

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

// Name can be player number, or player name.
// Players 0..(MAXPLAYERS-1) are known as Player 1 .. to the user.
// Return player number, 0..(MAXPLAYERS-1).
// Return 255, and put msg to console, when name not found.
byte player_name_to_num(char *name)
{
    // Player num can be 0..250 (limited to MAXPLAYERS).
    int playernum, i;

    playernum=atoi(name);   // test as player number 1..MAXPLAYERS
    if((playernum > 0) && (playernum <= MAXPLAYERS))
    {
        playernum --;  // convert to 0..MAXPLAYERS
        if(playeringame[playernum])
            return playernum;
        goto no_player;
    }

    // Search for player by name.
    for(i=0;i<MAXPLAYERS;i++)
    {
        if(playeringame[i] && strcasecmp(player_names[i],name)==0)
            return i;
    }
    
no_player:   
    CONS_Printf("There is no player named\"%s\"\n",name);
    return 255;
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
    if (COM_Argc() != 2)
    {
        CONS_Printf ("kick <playername> or <playernum> : kick a player\n");
        return;
    }

    if(server)
    {
        int pn = player_name_to_num(COM_Argv(1));
        if(pn < MAXPLAYERS)
           Send_NetXCmd_p2(XD_KICK, pn, KICK_MSG_GO_AWAY);
    }
    else
    {
        CONS_Printf("You are not the server\n");
    }
}

void Got_NetXCmd_KickCmd(xcmd_t * xc)
{
    int pnum=READBYTE(xc->curpos);  // unsigned player num
    int msg =READBYTE(xc->curpos);  // unsigned kick message

    GenPrintf(EMSG_hud, "\2%s ", player_names[pnum]);

    switch(msg)
    {
       case KICK_MSG_GO_AWAY:
               GenPrintf(EMSG_hud, "has been kicked (Go away)\n");
               break;
       case KICK_MSG_CON_FAIL:
               GenPrintf(EMSG_hud, "has been kicked (Consistency failure)\n");
               break;
       case KICK_MSG_TIMEOUT:
               GenPrintf(EMSG_hud, "left the game (Connection timeout)\n");
               break;
       case KICK_MSG_PLAYER_QUIT:
               GenPrintf(EMSG_hud, "left the game\n");
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
consvar_t cv_maxplayers     =
  {"sv_maxplayers","32",CV_NETVAR,maxplayers_cons_t,NULL,32};

void Got_NetXCmd_AddPlayer(xcmd_t * xc);
void Got_NetXCmd_AddBot(xcmd_t * xc);	//added by AC for acbot

// Called one time at init, by D_Startup_NetGame.
void D_Init_ClientServer (void)
{
  DEBFILE(va("==== %s debugfile ====\n", VERSION_BANNER));

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
//    debug_Printf( "viewangleoffset=%i\n", viewangleoffset );

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
    SV_StopServer();  // as an Init
}

// nnode: 0..(MAXNETNODES-1)
static void Reset_NetNode(byte nnode)
{
    nodeingame[nnode] = false;
    nnode_to_player[nnode] = 255;
    nnode_to_player2[nnode] = 255;
    nettics[nnode]=gametic;
    nextsend_tic[nnode]=gametic;
#if 0
    cl_maketic[nnode]=maketic;
#endif
    nodewaiting[nnode]=0;
    playerpernode[nnode]=0;
}

// Called by D_Init_ClientServer, SV_SpawnServer, CL_Reset, D_WaitPlayer_Response
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
        servernode=0;  // server to self

    doomcom->numplayers=0;

    DEBFILE(va("==== Server Reset ====\n"));
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

    // [WDJ] This can tight loop when the network fails.
    if( network_state == NS_shutdown )
        return;
    network_state = NS_shutdown;
     
    DEBFILE("==== Quiting Game, closing connection ====\n" );

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
        // Close registration with the Master Server.
        if ( serverrunning && cv_internetserver.value )
             MS_UnregisterServer(); 
    }
    else
    if( (servernode < MAXNETNODES)
       && nodeingame[servernode] )
    {
        // Client sends quit to server.
        netbuffer->packettype=PT_CLIENTQUIT;
        HSendPacket(servernode,true,0,0);
    }

    D_CloseConnection();

    DEBFILE("==== Log finish ====\n" );
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
#if 0
    cl_maketic[nnode]    = maketic;
#endif
    // Because the server connects to itself and sets nodeingame[0],
    // do not interfere with nodeingame[0] here.
    if(nnode)
       nodeingame[nnode]=true;
}

// Xcmd XD_ADDPLAYER
void Got_NetXCmd_AddPlayer(xcmd_t * xc)
{
    static uint32_t sendconfigtic = 0xffffffff;

    // [WDJ] Having error due to sign extension of byte read (signed char).
    byte nnode = READBYTE(xc->curpos);  // unsigned
    unsigned int newplayernum = READBYTE(xc->curpos);  // unsigned
    boolean splitscreenplayer = newplayernum&0x80;

    newplayernum&=0x7F;  // remove flag bit, and any sign extension

    playeringame[newplayernum]=true;
    G_AddPlayer(newplayernum);
    if( newplayernum+1 > doomcom->numplayers )
        doomcom->numplayers=newplayernum+1;
    // [WDJ] Players are 1..MAXPLAYERS to the user.
    GenPrintf(EMSG_hud, "Player %d is in the game (node %d)\n", (newplayernum+1), nnode);

    if(nnode==cl_nnode)
    {
        // The server is creating my player.
        player_to_nnode[newplayernum]=0;  // for information only
        if(!splitscreenplayer)
        {
            consoleplayer=newplayernum;
            displayplayer=newplayernum;
            displayplayer_ptr = consoleplayer_ptr = &players[newplayernum];
            DEBFILE("Spawning me\n");
        }
        else
        {
            displayplayer2=newplayernum;
            displayplayer2_ptr = &players[displayplayer2];
            DEBFILE("Spawning my brother\n");
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
void Got_NetXCmd_AddBot(xcmd_t * xc)  //added by AC for acbot
{
    // [WDJ] Having error due to sign extension of byte read (signed char).
    unsigned int newplayernum=READBYTE(xc->curpos);  // unsigned
    //int node = 0;
    //int i = 0;
    newplayernum&=0x7F;  // remove flag bit, and any sign extension
    playeringame[newplayernum]=true;
    strcpy(player_names[newplayernum], botinfo[newplayernum].name);
    players[newplayernum].skincolor = botinfo[newplayernum].colour;
    G_AddPlayer(newplayernum);
    players[newplayernum].bot = B_Create_Bot();
    if( newplayernum+1>doomcom->numplayers )
        doomcom->numplayers=newplayernum+1;

    multiplayer=1;

    GenPrintf(EMSG_hud, "Bot %s has entered the game\n", player_names[newplayernum]);
}

// By Server.
// Called by SV_SpawnServer, client_join_handler.
// Return true when a new player is added.
boolean SV_AddWaitingPlayers(void)
{
    boolean  newplayer_added = false;  // return
    byte nnode, nn;
    byte newplayernum, addplayer_param;

    newplayernum=0;
    for(nnode=0; nnode<MAXNETNODES; nnode++)
    {
        // splitscreen can allow 2 player in one node
        for(; nodewaiting[nnode]>0; nodewaiting[nnode]--)
        {
            newplayer_added = true;

            // Search for a free playernum.
            // We can't use playeringame since it is not updated here.
            //while(newplayernum<MAXPLAYERS && playeringame[newplayernum])
            //    newplayernum++;
            for( ;newplayernum<MAXPLAYERS; newplayernum++)
            {
                for(nn=0; nn<MAXNETNODES; nn++)
                {
                    if( nnode_to_player[nn]  == newplayernum
                        || nnode_to_player2[nn] == newplayernum )
                        break;
                }
                if( nn == MAXNETNODES )
                    break;  // found an unused player number
            }
            
#ifdef PARANOIA
            // Should never happen because we check the number of players
            // before accepting the join.
            if(newplayernum==MAXPLAYERS)
                I_Error("SV_AddWaitingPlayers: Reached MAXPLAYERS\n");
#endif
            player_to_nnode[newplayernum] = nnode;

            if( playerpernode[nnode]<1 )
            {
                nnode_to_player[nnode] = newplayernum;
                addplayer_param = newplayernum;
            }
            else
            {
                nnode_to_player2[nnode] = newplayernum;
                addplayer_param = newplayernum | 0x80;  // player 2 flag
            }
            playerpernode[nnode]++;

            Send_NetXCmd_p2(XD_ADDPLAYER, nnode, addplayer_param);
            if( doomcom->numplayers==0 )
                doomcom->numplayers++;  //we must send the change to other players
            
            DEBFILE(va("Server added player %d net node %d\n",
                       newplayernum, nnode));
            // use the next free slot (we can't put playeringame[j]=true here)
            newplayernum++; 
        }
    }

    return newplayer_added;
}

void CL_AddSplitscreenPlayer( void )
{
    if( cl_mode == CLM_connected )
        CL_Send_Join();  // join game
}

void CL_RemoveSplitscreenPlayer( void )
{
    if( cl_mode != CLM_connected )
        return;

    Send_NetXCmd_p2(XD_KICK, displayplayer2, KICK_MSG_PLAYER_QUIT);  // player 2
}

// Is there a game running.
boolean Game_Playing( void )
{
    return (server && serverrunning) || (!server && cl_mode==CLM_connected);
}

// By Server and Server-only commands.
// Called by D_Startup_NetGame (dedicated server).
// Called by Command_Map_f, Command_Load_f (server).
// Return true when a new player is added.
boolean SV_SpawnServer( void )
{
    D_DisableDemo();

    if( serverrunning == false )
    {
        GenPrintf(EMSG_hud, "Starting Server ...\n");
        serverrunning = true;
        SV_ResetServer();
        if( netgame )
        {
            I_NetOpenSocket();
            // Register with the Master Server.
            if( cv_internetserver.value )
            {
                // MasterServer address is in cv_masterserver.
                MS_RegisterServer();
            }
        }

        D_WaitPlayer_Setup();

        // server just connect to itself
        if( !dedicated )
            CL_ConnectToServer();
    }

    return SV_AddWaitingPlayers();
}

// Called by D_Init_ClientServer, G_StopDemo, CL_Reset, SV_StartSinglePlayerServer,
// D_WaitPlayer_Response.
void SV_StopServer( void )
{
    int i;

    gamestate = wipegamestate = GS_NULL;

    localtextcmd.len = 0;  // text len
    localtextcmd2.len = 0; // text len

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
    strncpy(netbuffer->u.stringpak.str, reason, MAX_STRINGPAK_LEN-1);
    netbuffer->u.stringpak.str[MAX_STRINGPAK_LEN-1] = 0;

    netbuffer->packettype = PT_SERVERREFUSE;
    HSendPacket(to_node, true, 0, strlen(netbuffer->u.stringpak.str)+1);
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
                           LE_SWAP32(netbuffer->u.askinfo.send_time));
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
        || LE_SWAP32(netbuffer->u.clientcfg.subversion) != NETWORK_VERSION)
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
            GenPrintf(EMSG_error, "Client Join Comm Failure: client lost\n");
            return;
        }
        DEBFILE("New node joined\n");
#ifdef JOININGAME
        newnode = true;
#endif
    }
#ifdef JOININGAME
    if( nodewaiting[nnode] )
    {
        if( (gamestate == GS_LEVEL) && newnode)
        {
            SV_Send_SaveGame(nnode); // send game data
            GenPrintf(EMSG_info, "Send savegame\n");
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
        byte reason = (netbuffer->packettype == PT_NODE_TIMEOUT) ?
           KICK_MSG_TIMEOUT : KICK_MSG_PLAYER_QUIT;
        // Update other players by kicking nnode.
        Send_NetXCmd_p2(XD_KICK, netconsole, reason);  // kick player
        nnode_to_player[nnode] = 255;

        if( nnode_to_player2[nnode] < MAXPLAYERS )
        {
            if( playeringame[nnode_to_player2[nnode]] )
            {
               // kick player2
               Send_NetXCmd_p2(XD_KICK, nnode_to_player2[nnode], reason);
            }
            nnode_to_player2[nnode] = 255;
        }
    }
    Net_CloseConnection(nnode);
    nodeingame[nnode]=false;
}



// By Client
// PT_SERVERINFO from Server.
//  nnode : remote node
static void server_info_handler( byte nnode )
{
    // Compute ping in ms.
    netbuffer->u.serverinfo.trip_time =
     (I_GetTime() - LE_SWAP32(netbuffer->u.serverinfo.trip_time))*1000/TICRATE; 
    netbuffer->u.serverinfo.servername[MAXSERVERNAME-1]=0;

    SL_InsertServer( &netbuffer->u.serverinfo, nnode);
}


// By Client
// PT_SERVERREFUSE from Server.
static void server_refuse_handler( byte nnode )
{
    if( cl_mode == CLM_wait_join_response )
    {
        M_SimpleMessage(va("Server %i refuses connection\n\nReason :\n%s",
                           nnode,
                           netbuffer->u.stringpak.str));
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
    xcmd_t xc;

    if( cl_mode != CLM_wait_join_response )
        return;

    if(!server)
    {
        // Clients not on the server, update to server time.
        maketic = gametic = cl_need_tic = LE_SWAP32(netbuffer->u.servercfg.gametic);
    }

#ifdef CLIENTPREDICTION2
    localgametic = gametic;
#endif
    nodeingame[servernode]=true;

    // Handle a player on the server.
    serverplayer = netbuffer->u.servercfg.serverplayer;
    if (serverplayer < MAXPLAYERS)  // 255 = no player
        player_to_nnode[serverplayer] = servernode;

    doomcom->numplayers = netbuffer->u.servercfg.totalplayernum;
    cl_nnode = netbuffer->u.servercfg.clientnode;

    GenPrintf(EMSG_hud, "Join accepted, wait next map change ...\n");
    DEBFILE(va("Server accept join gametic=%d, client net node=%d\n",
               gametic, cl_nnode));

    uint32_t  playerdet = LE_SWAP32(netbuffer->u.servercfg.playerdetected);
    for(j=0;j<MAXPLAYERS;j++)
    {
        playeringame[j]=( playerdet & (1<<j) ) != 0;
    }

    xc.playernum = 0;
    xc.curpos = netbuffer->u.servercfg.netcvarstates;
    xc.endpos = xc.curpos + NETCVAR_BUFF_LEN - 1;
    CV_LoadNetVars( &xc );

#ifdef JOININGAME
    cl_mode = ( netbuffer->u.servercfg.gamestate == GS_LEVEL ) ?
       CLM_download_savegame : CLM_connected;
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
        DEBFILE(va("Unknown packet received (%d) from unknown host !\n",
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
            int textlen = textcmds[btic][i].len;
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
    int nbtc_len = netbuffer->u.textcmdpak.len;  // incoming length
    int tc_limit;  // Max size of existing textcmd that can be included with this textcmd.
    int textlen, btic;
    tic_t tic;
    textbuf_t *  textbuf;

    // Move textcmd from netbuffer to a textbuf.

    // Check if tic that we are making isn't too large,
    // else we cannot send it :(
    // Note: doomcom->numplayers+1 is "+1" because doomcom->numplayers
    // can change within this time and sent time.
    tc_limit = software_MAXPACKETLENGTH
         - ( nbtc_len + 2 + SERVER_TIC_BASE_SIZE
            + ((doomcom->numplayers+1) * sizeof(ticcmd_t)) );

    // Search for a tic that has enough space in the ticcmd.
    tic = maketic;
    while( TotalTextCmdPerTic(tic) > tc_limit )
    {
        textbuf = & textcmds[ BTIC_INDEX( tic ) ][netconsole];
        if( (nbtc_len + textbuf->len) < MAXTEXTCMD )
          break; // found one
        tic++;
        if( tic >= (next_tic_send+BACKUPTICS) )  goto drop_packet;
    }

    btic = BTIC_INDEX( tic );
    textbuf = & textcmds[btic][netconsole];
    textlen = textbuf->len;
    DEBFILE(va("Textcmd: btic %d text[%d] player %d nxttic %d maketic %d\n",
               btic, textlen, netconsole, next_tic_send, maketic));
    // Append to the selected buffer.
    memcpy(&textbuf->text[ textlen ],
           &netbuffer->u.textcmdpak.text[0],  // the text
           nbtc_len);
    textbuf->len += nbtc_len;  // text len
    return;
   
drop_packet:
    // Drop the packet, let the node resend it.
    DEBFILE(va("Textcmd too long: max %d used %d maketic %d nxttic %d node %d player %d\n",
               tc_limit, TotalTextCmdPerTic(maketic), maketic, next_tic_send,
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
        DEBFILE(va("Out of order ticcmd discarded: nettics %d\n",
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
    if((start_tic <= gametic)
       && (start_tic > (gametic - BACKUPTICS + 1)) )
    {
        // within previous tics
        btic = BTIC_INDEX(start_tic);
        if(consistency[btic] != LE_SWAP16_FAST(netbuffer->u.clientpak.consistency))
        {
            // Failed the consistency check.
            byte confault = ++consistency_faults[nnode];  // failure count
            if( confault > 7 )
            {
                // Failed the consistency check too many times
#if 1
                Send_NetXCmd_p2(XD_KICK, netconsole, KICK_MSG_CON_FAIL);
#else
                // Debug message instead.
                GenPrintf(EMSG_warn, "Kick player %d at tic %d, consistency failure\n",
                    netconsole, start_tic);
#endif
                DEBFILE(va("Kick player %d at tic %d, consistency %d != %d\n",
                    netconsole, start_tic, consistency[btic],
                    LE_SWAP16_FAST(netbuffer->u.clientpak.consistency)));

            }
#ifdef JOININGAME
            else if( (confault & 0x3) == 3 )
            {
                // try to use savegame to fix consistency
                SV_Send_Pos_repair(RQ_SUG_SAVEGAME, nnode);
            }
#endif
            else
            {
                // try to fix consistency
                SV_Send_Pos_repair(RQ_PLAYER, nnode);
            }
        }
        else if( consistency_faults[nnode] > 0 )
        {
            consistency_faults[nnode] -- ;
        }
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
    byte * endbuffer;  // for buffer overrun tests
    byte   num_txt;
    int    btic, j, k;

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
    endbuffer = (byte*)&ticp[NUM_SERVERTIC_CMD];  // after last content

    // After the nettics are the net textcmds
    k = netbuffer->u.serverpak.numplayers * netbuffer->u.serverpak.numtics;
    if( k >= NUM_SERVERTIC_CMD )  goto exceed_buffer;
    txtp = (byte *)&netbuffer->u.serverpak.cmds[k];
    // txtp uses cmd space for text

    for(ti = start_tic; ti<end_tic; ti++)
    {
        // Clear first
        D_Clear_ticcmd(ti);

        // Copy the tics
        btic = BTIC_INDEX( ti );
        // btic limited to BACKUPTICS-1
        TicCmdCopy(netcmds[btic], ticp, netbuffer->u.serverpak.numplayers);
        ticp += netbuffer->u.serverpak.numplayers;

        // Copy the incoming textcmds.
        num_txt = *(txtp++);  // num_textcmd field, number of txtcmd
        for(j=0; j<num_txt; j++)
        {
            // Format:
            //  byte: playernum
            //  textbuf_t: textcmd
            int pn = *(txtp++); // playernum
            textbuf_t * tc = & textcmds[btic][pn];
            int tc_len = txtp[0]+1;  // max len of 256
              // length of whole textbuf_t 
            if( txtp + tc_len > endbuffer )  goto exceed_buffer;
#if MAXTEXTCMD < 255
            if( tc_len > MAXTEXTCMD+1 )  goto exceed_buffer;  // prevent dest overrun
#endif
            memcpy( tc, txtp, tc_len);
            // force string termination to defend against malicious packets
            tc->text[tc->len] = 0;
            tc->text[MAXTEXTCMD] = 0;
            txtp += tc_len;
        }
    }

    cl_need_tic = end_tic;
    return;

   
 exceed_buffer:
    I_SoftError("Nettics textcmd exceed buffer\n");
    return;
 not_in_packet:
    DEBFILE(va("Needed tic not in packet tic bounds: tic %u\n", cl_need_tic));
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

        // Known nodes only.
        netconsole = nnode_to_player[nnode];  // the player
#ifdef PARANOIA
        if(!(netconsole & DRONE) && netconsole>=MAXPLAYERS)
        {
            I_Error("bad table nnode_to_player : node %d player %d",
                     doomcom->remotenode,netconsole);
        }
#endif


        switch(netbuffer->packettype)
        {
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
            case PT_REPAIR:
                repair_handler( nnode );  // server and client
                break;
            case PT_STATE:
                if( !server )
                    state_handler();  // to client
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
            case PT_NETWAIT:
                if( !server )
                {
                    // Updates of wait for players, from the server.
                    num_netplayer = netbuffer->u.netwait.num_netplayer;
                    wait_netplayer = netbuffer->u.netwait.wait_netplayer;
                    wait_tics = LE_SWAP16( netbuffer->u.netwait.wait_tics );
                    P_SetRandIndex( netbuffer->u.netwait.p_rand_index ); // to sync P_Random
                }
                break;
            default:
                DEBFILE(va("Unknown packet type: type %d node %d\n",
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
    byte  cmd_options = 0;  // easier to understand and maintain
    int packetsize=0;

    if (cl_packetmissed)
        cmd_options |= 1;  // MIS bit
   
    netbuffer->packettype = PT_CLIENTCMD + cmd_options;
    netbuffer->u.clientpak.resendfrom = cl_need_tic;
    netbuffer->u.clientpak.client_tic = gametic;

    if( gamestate == GS_WAITINGPLAYERS )
    {
        // Server is waiting for network players before starting the game.
        // send NODEKEEPALIVE, or NODEKEEPALIVEMIS packet
        netbuffer->packettype = PT_NODEKEEPALIVE + cmd_options;
//        packetsize = sizeof(clientcmd_pak_t)-sizeof(ticcmd_t)-sizeof(int16_t);
        packetsize = offsetof(clientcmd_pak_t, consistency);
        HSendPacket (servernode,false,0,packetsize);
    }
    else
    if( gamestate != GS_NULL )
    {
        int btic = BTIC_INDEX( gametic );
        TicCmdCopy(&netbuffer->u.clientpak.cmd, &localcmds, 1);
        netbuffer->u.clientpak.consistency = LE_SWAP16_FAST(consistency[btic]);

        // send a special packet with 2 cmd for splitscreen
        if (cv_splitscreen.value)
        {
            // send PT_CLIENT2CMD, or PT_CLIENT2CMDMIS packet
            netbuffer->packettype = PT_CLIENT2CMD + cmd_options;
            TicCmdCopy(&netbuffer->u.client2pak.cmd2, &localcmds2, 1);
            packetsize = sizeof(client2cmd_pak_t);
        }
        else
            packetsize = sizeof(clientcmd_pak_t);
        
        HSendPacket (servernode,false,0,packetsize);
    }

    if( cl_mode == CLM_connected )
    {
        // send extra data if needed
        if (localtextcmd.len) // text len
        {
            int tc_len = localtextcmd.len+1;  // text len + cmd
            netbuffer->packettype = PT_TEXTCMD;
            memcpy(&netbuffer->u.textcmdpak, &localtextcmd, tc_len);
            // all extra data have been sended
            if( HSendPacket(servernode, true, 0, tc_len)) // send can fail for some reasons...
                localtextcmd.len = 0;  // text len
        }
        
        // send extra data if needed for player 2 (splitscreen)
        if (localtextcmd2.len) // text len
        {
            int tc_len = localtextcmd2.len+1;  // text len + cmd
            netbuffer->packettype = PT_TEXTCMD2;
            memcpy(&netbuffer->u.textcmdpak, &localtextcmd2, tc_len);
            // all extra data have been sended
            if( HSendPacket(servernode, true, 0, tc_len)) // send can fail for some reasons...
                localtextcmd2.len = 0; // text len
        }
    }
    else
    {
        // Clear XCmds that would overflow the buffers.
        localtextcmd.len = 0;
        localtextcmd2.len = 0;
    }
}


// By Server.
// Send PT_SERVERTICS, the server packet.
// Send tic from next_tic_send to maketic-1.
static void SV_Send_Tics (void)
{
    static byte resend_cnt = 0;  // spread resends at less cost than Get_Time

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
            DEBFILE(va("Send Tics none: node %d maketic %u nxttic %u nettic %u\n",
                       nnode, maketic, nextsend_tic[nnode], nettics[nnode]));
            start_tic = nettics[nnode];
            if( start_tic >= maketic )
                continue;  // all tic are ok, do next node
            if( (nnode + resend_cnt++)&3 )  // some kind of randomness
                continue;  // skip it
            DEBFILE(va("Resend from tic %d\n", start_tic));
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
                    // No tics made it into the packet.
                    if( packsize > MAXPACKETLENGTH )
                    {
                        I_Error("Too many players: cannot send %d data for %d players to net node %d\n"
                                "Well sorry nobody is perfect....\n",
                                packsize, doomcom->numplayers, nnode);
                    }
                    else
                    {
                        end_tic++;  // send it anyway !
                        DEBFILE("Sending empty tic anyway\n");
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
            // There must be a num_txtcmd field for every tic, even if 0.
            num_txt_p = bufpos++; // the num_textcmd field
            num_txt = 0;
            for(pn=0; pn<MAXPLAYERS; pn++)
            {
                // Format:
                //  byte: playernum
                //  textbuf_t: textcmd
                if((pn==0) || playeringame[pn])
                {
                    int textlen = textcmds[btic][pn].len;
                    if(textlen)
                    {
                        *(bufpos++) = pn;  // playernum
                        // Send the textbuf_t, length limited.
                        memcpy(bufpos, &textcmds[btic][pn], textlen+1);
                        bufpos += textlen+1;
                        num_txt++; // inc the number of textcmd sent
                    }
                }
            }
            // Update the num_txtcmd field, even if it is 0.
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
            // Catch startup glitch where playerpernode gets set
            // before the player is actually setup.
            if( ! playeringame[player] )  continue;

            DEBFILE(va("MISS tic %4u for node %d\n", maketic, nnode));
#ifdef PARANOIA
            if( devparm )
                GenPrintf(EMSG_dev, "\2Client %d Miss tic %d\n", nnode, maketic);
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

//  realtics: 0..5
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

    if( realtics > 0 )
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

    if( gamestate == GS_WAITINGPLAYERS )
    {
        // Server is waiting for network players.
        // To wait, execution must not reach G_Ticker.
        if( !server && !netgame )
            goto error_ret;  // connection closed by cancel or timeout
       
        if( realtics <= 0 )
            return;
       
        // Once per tic
        // Wait for players before netgame starts.
        if( ! D_WaitPlayer_Ticker() )
            return;  // Waiting

        // Start game
        if( dedicated )
            gamestate = GS_DEDICATEDSERVER;
        // Others must wait for load game to set gamestate to GS_LEVEL.
    }

    if (cl_need_tic > gametic)
    {
        if (demo_ctrl == DEMO_seq_advance)  // and not disabled
        {
            D_DoAdvanceDemo ();
            return;
        }

        // Run the count * tics
        while (cl_need_tic > gametic)
        {
            DEBFILE(va("==== Runing tic %u (local %d)\n",gametic, localgametic));

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
    return;

error_ret:
    D_StartTitle();
    return;
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
        // Max of gametic and nettics[].
        if(nodeingame[nn]
           && nettics[nn]<next_tic_send )
        {
           next_tic_send = nettics[nn];
        }
    }

    // Don't erase tics not acknowledged
    if( (maketic+count) >= (next_tic_send+BACKUPTICS) )
        count = (next_tic_send+BACKUPTICS) - maketic - 1;

    while( count-- > 0 )
        SV_Maketic();  // create missed tics and increment maketic

    // clear only when acknowledged
    for( ; next_tic_clear<next_tic_send; next_tic_clear++)
        D_Clear_ticcmd(next_tic_clear);  // clear the maketic the new tic

    SV_Send_Tics();

    cl_need_tic=maketic; // the server is a client too
}


void NetUpdate(void)
{
    static tic_t prev_netupdate_time=0;
    tic_t        nowtime;
    int          realtics;	// time is actually long [WDJ]

    nowtime  = I_GetTime();
    realtics = nowtime - prev_netupdate_time;

    if( realtics <= 0 )
    {
        if( realtics > -100000 )  // [WDJ] 1/16/2009  validity check
            return;     // same tic as previous

        // [WDJ] 1/16/2009 something is wrong, like time has wrapped.
        // Program gets stuck waiting for this, so force it out.
        realtics = 1;
    }
    if( realtics > 5 )
    {
        realtics = ( server )? 1 : 5;
    }

    // Executed once per tic, with realtics = 1..5.
    prev_netupdate_time = nowtime;

    if( !server )
        maketic = cl_need_tic;

    if( ! dedicated )
    {
        // Local Client
        I_OsPolling();       // i_getevent
        D_Process_Events ();
          // menu responder ???!!!
          // Cons responder
          // game responder call :
          //    HU_responder,St_responder, Am_responder
          //    F_responder (final)
          //    and G_MapEventsToControls

        Local_Maketic (realtics);  // make local tic

        if( server && !demoplayback )
            CL_Send_ClientCmd();     // send server tic
    }

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
            MS_SendPing_MasterServer( nowtime );

        if(!demoplayback)
            SV_Send_Tic_Update( realtics );  // realtics > 0

        if( Filetx_file_cnt )  // Rare to have file download in progress.
            Filetx_Ticker();
    }

    Net_AckTicker();
    if( ! dedicated )
    {
        M_Ticker ();
        CON_Ticker();
    }
}
