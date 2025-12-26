// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: d_net.c 1378 2017-12-18 17:31:35Z wesleyjohnson $
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2016 by DooM Legacy Team.
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
// $Log: d_net.c,v $
// Revision 1.18  2003/05/04 02:29:14  sburke
// Fix for big-endian machines.
//
// Revision 1.17  2001/08/26 15:27:29  bpereira
// added fov for glide and fixed newcoronas code
//
// Revision 1.16  2001/08/20 20:40:39  metzgermeister
// Revision 1.15  2001/02/10 12:27:13  bpereira
// Revision 1.14  2000/10/21 08:43:28  bpereira
// Revision 1.13  2000/10/16 20:02:29  bpereira
// Revision 1.12  2000/10/08 13:29:59  bpereira
// Revision 1.11  2000/09/28 20:57:14  bpereira
// Revision 1.10  2000/09/15 19:49:21  bpereira
// Revision 1.9  2000/09/10 10:38:18  metzgermeister
// Revision 1.8  2000/09/01 19:34:37  bpereira
//
// Revision 1.7  2000/09/01 18:23:42  hurdler
// fix some issues with latest network code changes
//
// Revision 1.6  2000/08/31 14:30:55  bpereira
// Revision 1.5  2000/04/16 18:38:07  bpereira
// Revision 1.4  2000/03/29 19:39:48  bpereira
// Revision 1.3  2000/02/27 00:42:10  hurdler
// Revision 1.2  2000/02/26 00:28:42  hurdler
// Mostly bug fix (see borislog.txt 23-2-2000, 24-2-2000)
//
//
// DESCRIPTION:
//      DOOM Network game communication and protocol, Low level functions,
//      all OS independent parts.
//
//      Implements a Sliding window protocol, without receiver window
//      (out of order reception).
//      This protocol use mix of "goback n" and "selective repeat" implementation
//      The NOTHING packet is sent when the connection is idle,
//      to acknowledge packets
//
//-----------------------------------------------------------------------------

#include "doomincl.h"
#include "d_net.h"
#include "g_game.h"
#include "i_net.h"
#include "i_system.h"
#include "m_argv.h"
#include "w_wad.h"
#include "d_netfil.h"
#include "d_clisrv.h"
#include "i_tcp.h"
#include "m_swap.h"
#include "z_zone.h"

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

#define FORCECLOSE         0x8000
#define CONNECTION_TIMEOUT  (15*TICRATE)

doomcom_t*  doomcom = NULL;
netbuffer_t* netbuffer = NULL;  // points inside doomcom

#ifdef DEBUGFILE
// Debug of some net info into a file, during the game.
FILE*       debugfile = NULL;
#endif

// Rebound queue, self server to self client network.
#define     MAXREBOUND 8
static netbuffer_t reboundstore[MAXREBOUND];
static uint16_t    reboundsize[MAXREBOUND];
static int         rebound_head,rebound_tail;

// Network interfaces (i_net.h)
uint32_t    net_bandwidth;
uint16_t    hardware_MAXPACKETLENGTH;
network_error_e  net_error;

boolean (*I_NetGet) (void);
boolean (*I_NetSend) (void);
boolean (*I_NetCanSend) (void);
void    (*I_NetCloseSocket) (void);
void    (*I_NetFreeNode) (int nodenum);
int     (*I_NetMakeNode) (char *address);
boolean (*I_NetOpenSocket) (void);

// ---- Internal network, for single player game.

boolean Internal_Get(void)
{
    doomcom->remotenode = -1;
    // I_Error("Get without netgame\n");
    net_error = NE_not_netgame;
    return  false;
}

// Function for I_NetSend().
boolean Internal_Send(void)
{
    I_SoftError("Send without netgame\n");
    net_error = NE_not_netgame;
    return  false;
}

void Internal_FreeNode(int nodenum)
{
}

// --- Network statistics

int    net_packetheader_length;

// Network stats.
tic_t   stat_starttic;      // tic of start of stat interval
int     stat_getbytes = 0;  // bytes received this interval
uint64_t  stat_sendbytes = 0;  // bytes sent, total
int     stat_retransmits = 0;  // packets resent
int     stat_duppacket = 0;  // received duplicate packets
int     stat_ackpacket_sent = 0;  // sent ack packets
int     stat_ackpacket_recv = 0;  // received ack packets
int     stat_tic_moved = 0;  // tics to move player
int     stat_tic_miss = 0;   // tics moved that are missing TICCMD_RECEIVED

// NetStat globals
int    netstat_recv_bps, netstat_send_bps;
float  netstat_lost_percent, netstat_dup_percent;  // packets
float  netstat_gamelost_percent;  // tics lost

// Return true when stats have been updated.
boolean Net_GetNetStat(void)
{
    static uint64_t  prev_stat_sendbyte = 0;
   
    tic_t ct = I_GetTime();
    if( stat_starttic+STAT_PERIOD <= ct )
    {
        // Stats are old, update them.
        netstat_recv_bps = (stat_getbytes*TICRATE)/(ct - stat_starttic);
        netstat_send_bps = ((stat_sendbytes - prev_stat_sendbyte)*TICRATE)
                         /(ct - stat_starttic);
        
        netstat_lost_percent = (stat_ackpacket_sent)?
         100.0*(float)stat_retransmits/(float)stat_ackpacket_sent
         : 0.0;
        netstat_dup_percent = (stat_ackpacket_recv)?
         100.0*(float)stat_duppacket/(float)stat_ackpacket_recv
         : 0.0;
        netstat_gamelost_percent = ( stat_tic_moved )?
         100.0*(float)stat_tic_miss/(float)stat_tic_moved
         : 0.0;

        prev_stat_sendbyte = stat_sendbytes;
        stat_getbytes=0;
        stat_ackpacket_sent = stat_ackpacket_recv = 0;
        stat_duppacket = stat_retransmits = 0;
        stat_tic_miss = stat_tic_moved = 0;
        stat_starttic = ct;

        return 1;
    }
    return 0;
}

// -----------------------------------------------------------------
//  Some stuct and function of ACK packets
// -----------------------------------------------------------------
// Network Rules:
// When a packet with an ack_req number is received, it is acknowledged
// by returning that acknum in a later message to the same net node.
// If this is not done, the sender will resend, or eventually conclude
// that the connection has been lost.
// A regular packet has room for one ack_return.  There is also a
// special packet that can send the whole ack queue to a node.
// An acknum is 1..255.  An acknum==0 represents no ACK.
// The acknum for each net node are independent.
// Due to the cmpack comparison method, the waiting ack are limited to a
// range of 128.  Any compared ack values must be within +/-128.

// The net node num (nnode) are internal to the Doom program communications.
// Net node num are 1..(MAX_CON_NETNODE-1), 0=myself,
// limited to 254 (byte), 255=empty.
#if ( MAX_CON_NETNODE > 254 )
# error Required: MAX_CON_NETNODE <= 254
#endif
// Player net nodes num 1..MAXNETNODES,
// limited to 127 by chat message packing.
#if ( MAXNETNODES > 127 )
# error Required: MAXNETNODES <= 127
#endif

// Max ack packets that can be saved.  Must exceed the max number of net nodes.
#define MAXACKPACKETS    64
#if ( MAXACKPACKETS <= MAXNETNODES )
# error Required: MAXACKPACKETS > MAXNETNODES
#endif

// Max number of acks to queue for return.
#define MAXACKTOSEND     64
#define URGENT_ACK_FREE_MIN   6
#define ACK_TIMEOUT  (TICRATE/17)

// An aid to traverse a circular ACK queue.
#define ACKTOSEND_DEC( ats )  (((ats)+(MAXACKTOSEND-1))%MAXACKTOSEND)
#define ACKTOSEND_INC( ats )  (((ats)+1)%MAXACKTOSEND)

// INC the acknum, not using 0 which is not an ACK.
#define ACK_INC_STMT( akn )   if( (++(akn))==0 )  (akn)=1
#define ACK_DEC_STMT( akn )   if( (--(akn))==0 )  --(akn)



typedef struct {
  byte   acknum;
  byte   acknum_at_xmit;  // the current acknum at transmit, and re-transmit.
  byte   destination_node;  // dest of the packet (0..(MAXNETNODES-1))
  byte   resent_cnt;  // num times has been resent (0..10)
  tic_t  senttime;
  uint16_t  length;
  byte   pak[MAXPACKETLENGTH];  // the packet, for retransmission
} ackpak_t;

typedef enum {
    NODE_CLOSE  = 1,    // flag is set when connection is closing
} node_flags_e;

// Table of packets that are waiting for ACK, or to be sent again.
// Can be resent (the sender window).
static ackpak_t ackpak[MAXACKPACKETS];

// Node history and ack status.
typedef struct {
    // The next ack to return (like slinding window protocol)
    // All outstanding ack less than it will also be considered ACK-ed.
    byte  return_ack;  // an acknum from a previous msg (0=none)

    // Queue of ACK that must be returned (tail..(head-1)).
    // Required when packets are not consecutive.
    // Queue is empty when head=tail. Full when (head+1=tail).
    byte  acktosend_head;  // index to acktosend, next insert
    byte  acktosend_tail;  // index to acktosend, oldest acknum
    byte  acktosend[MAXACKTOSEND];
   
    // Send packets with an ack request (reliable).
    byte  next_ackreq_num;  // the next ack_req to use
    // Flow control : to not send too many packets for ack
    byte  remote_prev_ack;  // the last ack return received from the net node

    // Automatically send ack packet when not enough traffic (keep alive).
    tic_t lasttime_ack_returned;
    // Detect connection lost.
    tic_t lasttime_packet_received;
    
    byte   flags;   // from node_flags_e
// jacobson tcp timeout evaluation algorithm (Karn variation)
    fixed_t ping;
    fixed_t varping;
    int     timeout;   // computed with ping and varping
} netnode_t;

// Ack structure for player net nodes.
static netnode_t net_nodes[MAXNETNODES];

// #define  NET_NODE_NUM( nnode )   ( nnode - net_nodes )

#define  PINGDEFAULT     ((200*TICRATE*FRACUNIT)/1000)
#define  VARPINGDEFAULT  ( (50*TICRATE*FRACUNIT)/1000)
#define  PING_TIMEOUT(p,v)    ((p)+(4*(v))+(FRACUNIT/2))>>FRACBITS;

// return <0 if a<b (mod 256)
//         0 if a=n (mod 256)
//        >0 if a>b (mod 256)
// mnemonic: to use it compare to 0 : cmpack(a,b)<0 is "a<b" ...
static int cmpack(byte a,byte b)
{
    register int d=a-b;

    if(d>=127 || d<-128)
        return -d;
    return d;
}

// Save the netbuffer packet in the ackpak table.
//   lowtimer : delayed sending
// Return the assigned acknum.
// Return 0 when fails.
static byte Save_packet_acknum( boolean lowtimer )
{
   boolean  low_priority = (netbuffer->packettype >= PT_CANFAIL); // low priority
   int num_freeslot=0;
   ackpak_t * ackpakp;
   byte rnode;
   netnode_t * np;

   rnode = doomcom->remotenode;
   if( rnode >= MAXNETNODES )
       goto ret_fail;  // not a player packet

   np = & net_nodes[rnode];

   // Flow control to avoid ack limitations.
   if(cmpack((byte)(np->remote_prev_ack+MAXACKTOSEND), np->next_ackreq_num) < 0)
       goto  too_fast;  // outstanding ack approaching limit of queue

   for( ackpakp = &ackpak[0]; ackpakp < &ackpak[MAXACKPACKETS]; ackpakp++ )
   {
       // Visit unused ackpak.
       if(ackpakp->acknum==0)
       {
           // Found an unused ackpak.
           if( low_priority )
           {
              // For low priority packet, make sure to leave some ackpak
              // free so urgent packets can be sent.
              num_freeslot++;
              if( num_freeslot < URGENT_ACK_FREE_MIN )
                continue;  // skip, until have enough free ackpak
           }

           // Save the packet and issue an acknum.
           ackpakp->acknum = np->next_ackreq_num;
           ackpakp->acknum_at_xmit = np->next_ackreq_num;  // first transmit
           ACK_INC_STMT( np->next_ackreq_num ); // skip acknum 0
           ackpakp->destination_node = rnode;
           ackpakp->length = doomcom->datalength;

           if(lowtimer)
           {
               // lowtimer means it can't be sent now so try it soon as possible
               ackpakp->senttime = 0;
               ackpakp->resent_cnt = 1;
           }
           else
           {
               ackpakp->senttime = I_GetTime();
               ackpakp->resent_cnt = 0;
           }
           memcpy(ackpakp->pak, netbuffer, ackpakp->length);

           stat_ackpacket_sent++; // for stat

           return ackpakp->acknum;  // return the acknum
       }
   }
   // Did not find free ackpacket.
#ifdef PARANOIA
   if( devparm )
       CONS_Printf("No more free ackpacket\n");
#endif
   // FIXME: wrong message or wrong place for this message.
   if( netbuffer->packettype < PT_CANFAIL )  // high priority
   {
       I_SoftError("Connection lost\n");
   }
   return 0;

   // Rare errors.
too_fast:   
   DEBFILE(va("Node %: too fast %d %d\n",
              rnode, np->remote_prev_ack, np->next_ackreq_num));
   // Fail to send, to avoid overrunning dest buffers.
   goto ret_fail;

ret_fail:
   return 0;
}

// Get the ack to send, from the ack queue of this node.
//  nnode: 0..(MAXNETNODES-1)
static byte Get_return_ack(byte nnode)
{
    net_nodes[nnode].lasttime_ack_returned = I_GetTime();
    return net_nodes[nnode].return_ack;
}

// Dispense with any packet that has been waiting for this ACK.
//  ackpakp : the ackpak to be removed
// Called when have received an ack from the net node.
static void Remove_ackpak( ackpak_t * ackpakp )
{
    byte dnode = ackpakp->destination_node;  // 0..(MAXNETNODES-1) by caller
    netnode_t * dnp = & net_nodes[dnode];  // dest node of the ackpak

    // Stats
    fixed_t trueping = (I_GetTime() - ackpakp->senttime) << FRACBITS;

    if( ackpakp->resent_cnt )
    {
        // +FRACUNIT/2 for round
        dnp->ping = (dnp->ping*7 + trueping)/8;
        dnp->varping = (dnp->varping*7 + abs(dnp->ping - trueping))/8;
        dnp->timeout = PING_TIMEOUT(dnp->ping, dnp->varping);
    }
    DEBFILE(va("Remove ack %d  trueping %d  ping %f  var %f  timeout %d\n",
               ackpakp->acknum, trueping>>FRACBITS,
               FIXED_TO_FLOAT(dnp->ping), FIXED_TO_FLOAT(dnp->varping),
               dnp->timeout));

    ackpakp->acknum=0;  // ackpak is now idle

    if( dnp->flags & NODE_CLOSE )
    {
        // Marked to close when all ACK have been settled.
        Net_CloseConnection( dnode );
    }
}

// We have got a packet, proceed with the ack request and ack return.
// Return false when acknum indicates it is a duplicate packet.
static boolean Process_packet_ack()
{
    int ati;  // acktosend index
    ackpak_t * ackpakp;
    byte acknum;
    byte nxtack;  // ACK are 1..255, circular
    byte rnode;
    netnode_t * np;
   
    rnode = doomcom->remotenode;  // net node num
    if( rnode >= MAXNETNODES )
        goto ret_fail;  // not a player net node

    np = & net_nodes[rnode];

    // Only process an ack return if it is greater than the previous ack.
    if(netbuffer->ack_return
       && cmpack(np->remote_prev_ack, netbuffer->ack_return) < 0)
    {
        // Received an ack_return, remove the ack from the ackpak.
        np->remote_prev_ack = netbuffer->ack_return;
        // Search the ackbuffer for net node and free it.
        for( ackpakp = &ackpak[0]; ackpakp < &ackpak[MAXACKPACKETS]; ackpakp++ )
        {
            // Remove all ackpak waiting for acknum that are <= ack_return.
            if( ackpakp->acknum
                && ackpakp->destination_node == rnode
                && cmpack(ackpakp->acknum, netbuffer->ack_return) <= 0 )
            {
                Remove_ackpak( ackpakp );
            }
        }
    }

    if( netbuffer->ack_req )
    {
        // Received a packet with ack_req, put it in queue to send the ack back.
        acknum = netbuffer->ack_req;
        stat_ackpacket_recv++;
        if( cmpack(acknum, np->return_ack) <= 0 )
            goto dup_by_range;

        // Check if the acknum is already in the queue.
        // Check active queue, tail..(head-1).
        for(ati = np->acktosend_tail;
            ati != np->acktosend_head;
            ati = ACKTOSEND_INC(ati)   )
        {
            if(np->acktosend[ati] == acknum)   goto dup_ackpak_found;
        }
      
        // Is a good packet so update the queue and return_ack.
        // Must search for holes in the queue.
        // Can only return ack when all previous ack have been received.

        // A missing acknum in the queue indicates a missing packet.
        // Higher acknum will be blocked waiting for them.
        // Because of this, received ack will not remove ack queue
        // entries until return_ack is greater.

        // Find next return_ack.
        nxtack = np->return_ack;
        ACK_INC_STMT( nxtack ); // skip acknum 0

        if(acknum == nxtack)
        {
            // This packet is next in order, which allows for ack cleanup.
            byte hm1; // head-1
            boolean change=true;

            np->return_ack = nxtack;
            ACK_INC_STMT( nxtack ); // skip acknum 0

            hm1 = ACKTOSEND_DEC( np->acktosend_head );  // latest queue entry
            // Find which ackpak can be ACK by return_ack.
            while(change)
            {
                change=false;
                // Search queue tail..(head-1)
                // Find each acknum in seq.  Must deal with missing ack.
                for( ati = np->acktosend_tail;
                     ati != np->acktosend_head;
                     ati= ACKTOSEND_INC(ati)   )
                {
                    if( cmpack(np->acktosend[ati],nxtack) <= 0 )
                    {
                        // Older acknum that is covered by this ack.
                        if( np->acktosend[ati]==nxtack )
                        {
                            // Found the next acknum.
                            // ACK this packet, and all older acknum.
                            np->return_ack = nxtack;
                            ACK_INC_STMT( nxtack ); // skip acknum 0
                            change=true;
                            // The ack will only be released if it is at the
                            // tail or head of the queue.
                        }
                        // The queue is in order of arrival of ack_req,
                        // which will not always be acknum ordered.
                        // All acknum less than nxtack will be considered
                        // ACK-ed by the current return_ack.  This also
                        // catches out-of-order ack_req.
                        if( ati == np->acktosend_tail )
                        {
                            // Release the covered acknum at the tail.
                            np->acktosend[np->acktosend_tail] = 0;
                            np->acktosend_tail = ACKTOSEND_INC(ati);
                        }
                        else if( ati == hm1 )
                        {
                            // Release the latest queue entry.
                            np->acktosend[hm1] = 0;
                            np->acktosend_head = hm1;  // move queue head
                            hm1 = ACKTOSEND_DEC( hm1 );
                            break;  // this was last checkable queue entry
                        }
                        // Putting holes in the queue would interfere with
                        // the cmpack test, which cannot handle 0.  It would
                        // not save any queue space unless entries were moved.
                    }
                }
            }
        }
        else
        {   // Out of order packet ( acknum != nxtack ).
            // Do not update the return_ack.
            // Put this ack in asktosend queue.
            // return_ack will be incremented when the next ack comes.
            byte newhead = ACKTOSEND_INC(np->acktosend_head);
            DEBFILE(va("Out of order packet: expected %d\n", nxtack));
            if(newhead == np->acktosend_tail)  goto queue_full;

            np->acktosend[np->acktosend_head] = acknum;
            np->acktosend_head = newhead;
        }
    }
    return true;
   
queue_full:
    // The ack queue is full. Discard the packet, sender will resend it.
    // If we admit the packet we will not detect the duplication after :(
    DEBFILE("Ack Queue full.\n");
    return false;

dup_by_range:
   // Any acknum < return_ack have already been considered ACK-ed.
   // Duplicate packet (per ack_req).
   DEBFILE(va("Discard (by range) ack %d (duplicated)\n", acknum));
   goto dup_reject;

dup_ackpak_found:	      
   // Already received, ack is in the queue.
   DEBFILE(va("Discard ack %d (duplicated)\n", acknum));
   goto dup_reject;

dup_reject:
   stat_duppacket++;
   // Discard the packet (duplicate)
ret_fail:
   return false;
}

#if MAXACKTOSEND > MAX_NETBYTE_LEN
# error  Netbuffer: MAXACKTOSEND > MAX_NETBYTE_LEN
#endif

// Send a special packet with only the ack queue.
//  to_node: 0..(MAXNETNODES-1)
void Net_Send_AcksPacket(int to_node)
{
    // Send an packet with the ack queue.
    netbuffer->packettype = PT_ACKS;
    memcpy(netbuffer->u.bytepak.b, net_nodes[to_node].acktosend, MAXACKTOSEND);
    HSendPacket(to_node,false,0,MAXACKTOSEND);
}

// Receive the special packet with only acks.
static void Got_AcksPacket(void)
{
    int j;
    byte rnode;
    byte recv_acknum;  // acknum from the packet
    ackpak_t * ackpakp;

    rnode = doomcom->remotenode;
    if( rnode >= MAXNETNODES )
       return;  // not a player packet

    // The body of the packet bytepak is the ack queue.
    for(j=0;j<MAXACKTOSEND;j++)
    {
        recv_acknum = netbuffer->u.bytepak.b[j];  // from the queue in the packet
        if( recv_acknum == 0 )  continue;  // 0=invalid

        // Find recv_acknum in our ack packets queue.
        // Without regards to queue head or tail.
        for( ackpakp = &ackpak[0]; ackpakp < &ackpak[MAXACKPACKETS]; ackpakp++ )
        {
            if( ackpakp->acknum
                && ackpakp->destination_node == rnode)
            {
                if( ackpakp->acknum == recv_acknum )
                {
                    // Dispense with any packets that have been ACK'ed.
                    Remove_ackpak( ackpakp );
                }
                else
                {
                    // As later acknum are received there is bigger
                    // chance a packet is lost.
                    // When the packet is first transmitted, or resent,
                    // acknum_at_xmit is updated to the
                    // net_nodes[rnode].next_ackreq_num.
                    if( cmpack(ackpakp->acknum_at_xmit, recv_acknum) <= 0 )
                    {
                        // Will cause sooner retransmits of older packets
                        // as the packet's acknum age.
                        if( ackpakp->senttime > 0 )
                            ackpakp->senttime--; // hurry up
                    }
                }
            }
        }
    }
}


// nnode:  1..(MAXNETNODES-1)
void Net_ConnectionTimeout( byte nnode )
{
    netbuffer_t * rebp = & reboundstore[rebound_head];
    // Send a very special packet to self (hack the reboundstore queu).
    // Main code will handle it.
    rebp->packettype = PT_NODE_TIMEOUT;
    rebp->ack_req = 0;
    rebp->ack_return = 0;
    rebp->u.bytepak.b[0] = nnode;

    reboundsize[rebound_head] = PACKET_BASE_SIZE+1;
    rebound_head=(rebound_head+1)%MAXREBOUND;

    // do not redo it quickly (if we do not close connection is for a good reason !)
    net_nodes[nnode].lasttime_packet_received = I_GetTime();
}

// Resend some packets, if needed.
void Net_AckTicker(void)
{
    int nn;  // net node num
    // Get the time once.  It does not change that fast, and this
    // keeps the record times consistent in this function. 
    tic_t  curtime = I_GetTime();
    ackpak_t * ackpakp;
    netnode_t * np;

    // Check all ackpak for old packets.
    for( ackpakp = &ackpak[0]; ackpakp < &ackpak[MAXACKPACKETS]; ackpakp++ )
    {
        if(ackpakp->acknum == 0)  continue;  // 0=inactive

        // An active ackpak.
        np = & net_nodes[ackpakp->destination_node];
        if((ackpakp->senttime + np->timeout) >= curtime)  continue;

        // Need to retransmit
        nn = ackpakp->destination_node;
        if( (ackpakp->resent_cnt > 10) && (np->flags & NODE_CLOSE) )
        {
            DEBFILE(va("Node %d: ack %d sent 10 times so connection is supposed lost\n",
                       nn, ackpakp->acknum));
            Net_CloseConnection( nn | FORCECLOSE );

            ackpakp->acknum = 0;  // inactive
            continue;
        }

        // Retransmit
        DEBFILE(va("Node %d: Resend ack %d, %d+%d<%d\n", nn,
                   ackpakp->acknum, ackpakp->senttime, np->timeout, curtime));
        memcpy(netbuffer, ackpakp->pak, ackpakp->length);
        ackpakp->senttime = curtime;
        ackpakp->resent_cnt++;
        ackpakp->acknum_at_xmit = np->next_ackreq_num;
        HSendPacket( nn, false, ackpakp->acknum,
                     ackpakp->length - PACKET_BASE_SIZE );
        stat_retransmits++; // for stat
    }

    for( nn=1; nn<MAXNETNODES; nn++)
    {
        np = & net_nodes[nn];
        // Using this something like a node open flag.
        if( np->return_ack )
        {
            // If we haven't sent a packet for a long time,
            // send acknowledge packets if needed.
            if( (np->lasttime_ack_returned + ACK_TIMEOUT) < curtime )
                Net_Send_AcksPacket(nn);

            if( (np->lasttime_packet_received + CONNECTION_TIMEOUT) < curtime
                && (np->flags & NODE_CLOSE) == 0 )
            {
                Net_ConnectionTimeout( nn );
            }
        }
    }
}


// Cancel the ack of the last packet received.  This forces the net node
// to resend it.  This is an escape when the higher layer don't have room,
// or something else ....)
//   nnode: 0..(MAXNETNODES-1)
void Net_Cancel_Packet_Ack(int nnode)
{
    netnode_t * np;
    int hm1;
    byte nxtack;
    byte cancel_acknum = netbuffer->ack_req;  // acknum to cancel

    DEBFILE(va("UnAck node %d\n",nnode));

    if(!nnode)
        return;  // net node 0 is self
    if(cancel_acknum==0)
        return;  // cannot cancel acknum==0

    np = & net_nodes[nnode];
    nxtack = np->return_ack;
    hm1 = ACKTOSEND_DEC( np->acktosend_head );  // last entry
    if( np->acktosend[hm1] == cancel_acknum )
    {
        // Remove canceled ack from the queue head.
        np->acktosend[hm1] = 0;
        np->acktosend_head = hm1;
    }
    else
#if 0
    // This is unneeded as it is a subset of the next case.
    if( nxtack == cancel_acknum )
    {
        // Only one ack needs to be undone.
        ACK_DEC_STMT( nxtack );
        np->return_ack = nxtack;
    }
    else
#endif   
    {
        // Put at the queue tail, all the intervening ack num.
        // If nxtack==cancel_acknum already, then only the return_ack needs
        // to updated.
        // Leaving a hole in the queue would indicate missing packets, and it
        // would wait for them, never sending the later ACK.
        // This assumes they all fit, since they were just all removed.
        // The canceled acknum must NOT be put in the queue.
        while (nxtack != cancel_acknum)
        {
            // Put nxtack back into the queue.
            np->acktosend_tail = ACKTOSEND_DEC( np->acktosend_tail );
            np->acktosend[np->acktosend_tail] = nxtack;

            ACK_DEC_STMT( nxtack );
        }
        // nxtack==cancel_acknum, and the canceled acknum is NOT in the queue.
#if 1
        // The return_ack must be set just before the canceled acknum.
        nxtack = cancel_acknum;
        ACK_DEC_STMT( nxtack );
#else
        // [WDJ] Bug: For the nxtack!=cancel_acknum case, it undid the DEC of
        // return_ack, which would have left return_ack greater than the
        // canceled acknum.  This would be the same as not canceling the ack.
        ACK_INC_STMT( nxtack );
        // This has now been combined with the nxtack==acknum case, because
        // the return_ack must be set just less than the canceled acknum in
        // both cases.
#endif
        np->return_ack = nxtack;
    }
}

// Return true if all ackpak have been received.
boolean Net_Test_AllAckReceived(void)
{
   ackpak_t * ackpakp;

   for( ackpakp = &ackpak[0]; ackpakp < &ackpak[MAXACKPACKETS]; ackpakp++ )
   {
      if(ackpakp->acknum)
          return false;
   }

   return true;
}

// Wait for all ack_return.
//  timeout : in seconds
void Net_Wait_AllAckReceived( uint32_t timeout )
{
    tic_t ct = I_GetTime();  // current time

    timeout = ct + timeout*TICRATE;  // convert timeout to absolute time

    HGetPacket();
    while( !Net_Test_AllAckReceived() )
    {
        while(ct == I_GetTime()) { }  // wait for next tic
        HGetPacket();
        Net_AckTicker();
        ct = I_GetTime();
        if( ct > timeout )  break;  // Timeout
    }
}

// Init Ack for player net nodes.
//  nnode: 0..(MAXNETNODES-1)
static void InitNode( int nnode )
{
    netnode_t * np = & net_nodes[nnode];
    np->acktosend_head  = 0;
    np->acktosend_tail  = 0;
    np->ping            = PINGDEFAULT;
    np->varping         = VARPINGDEFAULT;
    np->timeout         = PING_TIMEOUT(np->ping,np->varping);
    np->return_ack      = 0;
    np->next_ackreq_num = 1;
    np->remote_prev_ack = 0;
    np->flags           = 0;
}

static void InitAck()
{
   int i;

   for(i=0;i<MAXACKPACKETS;i++)
      ackpak[i].acknum=0;

   for(i=0;i<MAXNETNODES;i++)
       InitNode( i );  // Init Ack for player net nodes.
}

// Remove from retransmit any packets of the indicated type.
void Net_AbortPacketType(byte packettype)
{
    ackpak_t * ackpakp;

    // Check the retransmit ackpak for these type packets.
    for( ackpakp = &ackpak[0]; ackpakp < &ackpak[MAXACKPACKETS]; ackpakp++ )
    {
        if( ackpakp->acknum == 0 )  continue;
#if 1
        if( ((netbuffer_t *)ackpakp->pak)->packettype==packettype )
#else
        // [WDJ] UNUSED: and not likely to be used.
        // packettype=255 is ANY packet
        if( packettype==255
            || (((netbuffer_t *)ackpakp->pak)->packettype==packettype) )
#endif
        {
            // Free the ackpak.
            ackpakp->acknum=0;
        }
    }
}

// -----------------------------------------------------------------
//  end of acknowledge function
// -----------------------------------------------------------------


// Remove a node, clear all ack from this node and reset askret
//   nnode : the net node number, 0..(MAX_CON_NETNODE-1)
//      may be OR with flag  FORCECLOSE.
void Net_CloseConnection(int nnode)
{
    boolean forceclose = ((nnode & FORCECLOSE)!=0);
    ackpak_t * ackpakp;

    nnode &= ~FORCECLOSE;
    if( nnode >= MAXNETNODES )
    {
#if 0
        // [WDJ] GCC 4.5.2  Compiler bug.
        goto free_done;
#else
        // [WDJ] Smaller code by 3K.
        I_NetFreeNode(nnode);
        return;
#endif
    }
    if( nnode == 0 )
        return;  // Cannot close self connection.

    // Shutdown player net node.
    net_nodes[nnode].flags |= NODE_CLOSE;

    // Try to Send ack back (two army problem).
    if( net_nodes[nnode].return_ack )
    {
        // Send some empty packets with acks.
        Net_Send_AcksPacket( nnode );
        Net_Send_AcksPacket( nnode );
    }
    // Check if we have to wait for ack from this node.
    for( ackpakp = &ackpak[0]; ackpakp < &ackpak[MAXACKPACKETS]; ackpakp++ )
    {
        if( ackpakp->acknum
            && ackpakp->destination_node == nnode )
        {
            if( !forceclose )
                return;     // connection will be closed when ack is returned

            // Net Timeout, ack not likely. Force close.
            ackpakp->acknum = 0;  // inactive
        }
    }

    // No waiting for ack from this net node.
    InitNode(nnode);
    AbortSendFiles(nnode);
#if 0
// [WDJ] GCC 4.5.2 Using this label triggers a compiler bug that costs 3K size.
// An if {}, also costs 3K in program size ! All variations.
free_done:
#endif
    I_NetFreeNode(nnode);
    return;
}

//
// Checksum
//
// Return in net endian
static uint32_t  Netbuffer_Checksum (void)
{
    int       i, len;
    uint32_t  cs;
    unsigned char   *buf;

    cs = 0x1234567;

    len = doomcom->datalength - 4;
    buf = (unsigned char*)netbuffer+4;
    for (i=0 ; i<len ; i++,buf++)
        cs += (*buf) * (i+1);

    return LE_SWAP32_FAST(cs);
}

#ifdef DEBUGFILE

static void fprintfstring(byte *s,byte len)
{
    int i;
    int mode=0;

    for (i=0 ; i<len ; i++)
    {
       if(s[i]<32)
       {
           if(mode==0) {
               fprintf (debugfile,"[%d", s[i]);
               mode = 1;
           } else
               fprintf (debugfile,",%d", s[i]);
       }
       else
       {
           if(mode==1) {
              fprintf (debugfile,"]");
              mode=0;
           }
           fprintf (debugfile,"%c", s[i]);
       }
    }
    if(mode==1) fprintf (debugfile,"]");
    fprintf(debugfile,"\n");
}

static char *packettypename[NUMPACKETTYPE]={
    "NOTHING",
    "SERVERCFG",
    "CLIENTCMD",
    "CLIENTMIS",
    "CLIENT2CMD",
    "CLIENT2MIS",
    "NODEKEEPALIVE",
    "NODEKEEPALIVEMIS",
    "SERVERTICS",
    "SERVERREFUSE",
    "SERVERSHUTDOWN",
    "CLIENTQUIT",

    "ASKINFO",
    "SERVERINFO",
    "REQUESTFILE",
    "REPAIR",
    "ACKS",
    "STATE",
    "DUMMY18",
    "DUMMY19",

    "FILEFRAGMENT",
    "TEXTCMD",
    "TEXTCMD2",
    "CLIENTJOIN",
    "NODETIMEOUT",
    "WAITINFO",

};

static void DebugPrintpacket(char *header)
{
    fprintf (debugfile,"%-12s (node %d,ackreq %d,ackret %d,size %d) type(%d) : %s\n"
                      ,header
                      ,doomcom->remotenode
                      ,netbuffer->ack_req, netbuffer->ack_return
                      ,doomcom->datalength
                      ,netbuffer->packettype,packettypename[netbuffer->packettype]);

    switch(netbuffer->packettype)
    {
       case PT_ASKINFO:
           fprintf(debugfile
                  ,"    send_time %u\n"
                  ,(unsigned int)netbuffer->u.askinfo.send_time);
           break;
       case PT_CLIENTJOIN:
           fprintf(debugfile
                  ,"    number %d mode %d\n"
                  ,netbuffer->u.clientcfg.localplayers
                  ,netbuffer->u.clientcfg.mode);
           break;
       case PT_SERVERTICS:
           fprintf(debugfile
                  ,"    firsttic %d ply %d tics %d ntxtcmd %d\n    "
                  ,ExpandTics (netbuffer->u.serverpak.starttic)
                  ,netbuffer->u.serverpak.numplayers
                  ,netbuffer->u.serverpak.numtics
                  ,(int)(&((char *)netbuffer)[doomcom->datalength] - (char *)&netbuffer->u.serverpak.cmds[netbuffer->u.serverpak.numplayers*netbuffer->u.serverpak.numtics]));
           fprintfstring(                                            (byte *)&netbuffer->u.serverpak.cmds[netbuffer->u.serverpak.numplayers*netbuffer->u.serverpak.numtics]
                        ,&((char *)netbuffer)[doomcom->datalength] - (char *)&netbuffer->u.serverpak.cmds[netbuffer->u.serverpak.numplayers*netbuffer->u.serverpak.numtics]);
           break;
       case PT_CLIENTCMD:
       case PT_CLIENT2CMD:
       case PT_CLIENTMIS:
       case PT_CLIENT2MIS:
       case PT_NODEKEEPALIVE:
       case PT_NODEKEEPALIVEMIS:
           fprintf(debugfile
                  ,"    tic %4d resendfrom %d localtic %d\n"
                  ,ExpandTics (netbuffer->u.clientpak.client_tic)
                  ,ExpandTics (netbuffer->u.clientpak.resendfrom)
                  ,0 /*netbuffer->u.clientpak.cmd.localtic*/);
           break;
       case PT_TEXTCMD:
       case PT_TEXTCMD2:
           fprintf(debugfile
                  ,"    length %d\n    "
                  ,*(unsigned char*)netbuffer->u.textcmdpak.len);
           fprintfstring(netbuffer->u.textcmdpak.text,netbuffer->u.textcmdpak.len);
           break;
       case PT_SERVERCFG:
           fprintf(debugfile
                  ,"    playermask %x players %d clientnode %d serverplayer %d gametic %lu gamestate %d\n"
                  ,(unsigned int)netbuffer->u.servercfg.playerdetected
                  ,netbuffer->u.servercfg.totalplayernum
                  ,netbuffer->u.servercfg.clientnode
                  ,netbuffer->u.servercfg.serverplayer
                  ,(unsigned long)netbuffer->u.servercfg.gametic
                  ,netbuffer->u.servercfg.gamestate);
           break;
       case PT_SERVERINFO :
           fprintf(debugfile
                  ,"    '%s' player %i/%i, map %s, filenum %d, time %u \n"
                  ,netbuffer->u.serverinfo.servername
                  ,netbuffer->u.serverinfo.numberofplayer
                  ,netbuffer->u.serverinfo.maxplayer
                  ,netbuffer->u.serverinfo.mapname
                  ,netbuffer->u.serverinfo.num_fileneed
                  ,(unsigned int)netbuffer->u.serverinfo.trip_time);
           fprintfstring(netbuffer->u.serverinfo.fileneed,(char *)netbuffer+doomcom->datalength-(char *)netbuffer->u.serverinfo.fileneed);
           break;
       case PT_SERVERREFUSE :
           fprintf(debugfile
                  ,"    reason %s\n"
                  ,netbuffer->u.serverrefuse.reason);
           break;
       case PT_REPAIR :
           fprintf(debugfile,
                   "    repairtype %d, P_Random index %d, tic %i,\n (id_num %d, angle %x, x %i.%i, y %i.%i, z %i.%i, momx %i.%i, momy %i.%i, momz %i,%i\n",
                   u.repair.repair_type, u.repair.p_rand_index, u.repair.gametic,
                   u.repair.pos.id_num, u.repair.pos.angle,
                   u.repair.pos.x >> 16, u.repair.pos.x & 0xFFFF, u.repair.pos.y >> 16, u.repair.pos.y & 0xFFFF, u.repair.pos.z >> 16, u.repair.pos.z & 0xFFFF,
                   u.repair.pos.momx >> 16, u.repair.pos.momx & 0xFFFF, u.repair.pos.momy >> 16, u.repair.pos.momy & 0xFFFF, u.repair.pos.momz >> 16, u.repair.pos.momz & 0xFFFF );
           break;
       case PT_STATE :
           fprintf(debugfile,
                   "    tic %i, P_Random index %d, sever_pause %d\n",
                    u.state.gametic, u.state.p_rand_index, u.state.server_pause );
           break;
       case PT_FILEFRAGMENT :
           fprintf(debugfile
                  ,"    fileid %d datasize %d position %lu\n"
                  ,netbuffer->u.filetxpak.fileid
                  ,netbuffer->u.filetxpak.size
                  ,(unsigned long)netbuffer->u.filetxpak.position);
           break;
       case PT_REQUESTFILE :
       case PT_NODE_TIMEOUT :
       case PT_ACKS :
       default : // write as a raw packet
         fprintfstring(netbuffer->u.bytepak.b, (char *)netbuffer + doomcom->datalength - (char *)netbuffer->u.bytepak.b);
           break;

    }
}
#endif

//
// HSendPacket
//
//  packetlength: number of bytes in u part of packet
//  acknum: retransmit of a packet with this acknum
// Return true when packet is sent.
boolean HSendPacket(int to_node, boolean reliable, byte acknum,
                    int packetlength)
{

    doomcom->datalength = packetlength + PACKET_BASE_SIZE;
    if(to_node == 0)
    {
        // Send packet to self.
        if((rebound_head+1)%MAXREBOUND==rebound_tail)
        {
#ifdef PARANOIA
            CONS_Printf("Empty rebound buf\n");
#endif
            goto fail_ret;
        }
        memcpy(&reboundstore[rebound_head],netbuffer,doomcom->datalength);
        reboundsize[rebound_head]=doomcom->datalength;
        rebound_head=(rebound_head+1)%MAXREBOUND;
#ifdef DEBUGFILE
        if (debugfile)
        {
            doomcom->remotenode = to_node;
            DebugPrintpacket("SENDLOCAL");
        }
#endif
        return true;
    }

//    if (demoplayback)
//        return true;

    if (!netgame)   goto not_netgame;

    // Do this before Save_packet_acknum() because that function will
    // backup the current packet.
    doomcom->remotenode = to_node;
    if(doomcom->datalength <= 0)   goto empty_packet;

    // Include any pending return_ack, player nodes only.
    netbuffer->ack_return = (to_node<MAXNETNODES)?
       Get_return_ack(to_node)  // ack a previous packet
       : 0;  // broadcast, no ack

    if(reliable)
    {
        // Packet sent with an ack_req, and retransmitted if necessary.
        if( I_NetCanSend && !I_NetCanSend() )
        {
            // Network cannot transmit right now.
            // Enhancement for slow networks, not required.
            if( netbuffer->packettype < PT_CANFAIL )
            {
                // High priority, deferred transmit.
                netbuffer->ack_req = Save_packet_acknum(true);
            }

            DEBFILE("HSendPacket : Out of bandwidth\n");
            goto fail_ret;
        }
        // Save packet, issue an acknum.
        netbuffer->ack_req = Save_packet_acknum(false);
        if( netbuffer->ack_req == 0 )  goto fail_ret;
    }
    else
    {
        // Either acknum==0, or this is a retransmit using existing acknum.
        netbuffer->ack_req = acknum;
    }

    netbuffer->checksum = Netbuffer_Checksum();
    stat_sendbytes += (net_packetheader_length + doomcom->datalength); // for stat

#if 1
    return I_NetSend();
#else
    // DEBUG
    // simulate internet :)
    if( rand()<RAND_MAX/5 )
    {
#ifdef DEBUGFILE
        if (debugfile)
            DebugPrintpacket("SEND");
#endif
        return I_NetSend();
    }
#ifdef DEBUGFILE
    else
    {
        if (debugfile)
            DebugPrintpacket("NOTSEND");
    }
#endif
    return true;  // indicates sent, but gets lost
#endif

// Rare errors
not_netgame:
    I_SoftError ("HSendPacket: not in netgame\n");
    goto fail_ret;

empty_packet:
    DEBFILE("HSendPacket: abort send of empty packet\n");
#ifdef DEBUGFILE
    if (debugfile)
        DebugPrintpacket("TRISEND");
#endif
    goto fail_ret;

fail_ret:
    return false;
}

//
// HGetPacket
// Returns false if no packet is waiting
// Check Datalength and checksum
//
boolean HGetPacket (void)
{
    if (rebound_tail!=rebound_head)
    {
        // Receive from self.  Get a packet from my local net queue.
        memcpy(netbuffer,&reboundstore[rebound_tail],reboundsize[rebound_tail]);
        doomcom->datalength = reboundsize[rebound_tail];
        if( netbuffer->packettype == PT_NODE_TIMEOUT )
            doomcom->remotenode = netbuffer->u.bytepak.b[0];  // to node timeout
        else
            doomcom->remotenode = 0;  // from self

        rebound_tail=(rebound_tail+1)%MAXREBOUND;
#ifdef DEBUGFILE
        if (debugfile)
           DebugPrintpacket("GETLOCAL");
#endif
        return true;
    }

    if (!netgame)   goto fail_ret;

    if( ! I_NetGet() )   goto fail_ret;  // no packet

    stat_getbytes += (net_packetheader_length + doomcom->datalength); // for stat

    if( doomcom->remotenode < MAXNETNODES )
    {
        // Player netnode
        net_nodes[doomcom->remotenode].lasttime_packet_received = I_GetTime();
    }
    else if (doomcom->remotenode >= MAX_CON_NETNODE)  goto bad_node_num;

    if (netbuffer->checksum != Netbuffer_Checksum())  goto bad_checksum;

#ifdef DEBUGFILE
    if (debugfile)
        DebugPrintpacket("GET");
#endif

    // Process the ack_num and ack_return fields.
    if(!Process_packet_ack())    goto fail_ret;    // discated (duplicated)

    // a packet with just ack_return
    if( netbuffer->packettype == PT_ACKS)
    {
        // Detect the special acks packet.
        Got_AcksPacket();
        return false;
    }

    return true;

// Rare errors
bad_node_num:
    DEBFILE(va("HGetPacket: receive packet from node %d !\n", doomcom->remotenode));
    goto fail_ret;

bad_checksum:
    DEBFILE("HGetPacket: Bad packet checksum\n");
    goto fail_ret;

fail_ret:
    return false;
}


//
// D_Startup_NetGame
// Works out player numbers among the net participants
//
// Returns true when a network game is started by command line.
boolean D_Startup_NetGame(void)
{
    boolean client;
    int  num;

    // Bring up low level functions.
    InitAck();
    rebound_tail=0;
    rebound_head=0;

    stat_starttic = I_GetTime();  // start netstat interval

    // Default
    I_NetGet           = Internal_Get;
    I_NetSend          = Internal_Send;
    I_NetCanSend       = NULL;
    I_NetCloseSocket   = NULL;
    I_NetFreeNode      = Internal_FreeNode;
    I_NetMakeNode      = NULL;

    // The default mode is server.
    // Set to Client mode when connect to another server.
    server = true;
    netgame = false;
    multiplayer = false;

    // Need dedicated and server determined, before most other Init.
    num = MAXNETNODES+9;  // invalid, cv_wait_players already has default.
    dedicated = ( M_CheckParm("-dedicated") != 0 );
    if( dedicated )
    {
        server = true;
        netgame = true;
        num = 0;  // dedicated number players
        // -server can set some other wait nodes value
    }

    if ( M_CheckParm ("-server"))
    {
        server = true;
        netgame = true;

        // If a number of clients (i.e. nodes) is specified, the server will
        // wait for the clients to connect before starting.
        // If no number is specified here, the server starts with 1 client,
        // others can join in-game.
        // A value of 0 allows join in-game, and wait_timeout game start,
        // with an unknown number of players.
        if( M_IsNextParm() )
        {
            // Number of players.
            num = atoi(M_GetNextParm());
            if( num < 0 )
               num = 0;
            if( num > MAXNETNODES )
               num = MAXNETNODES;
        }
        else
            num = 1;
        // Wait for player nodes during startup.
    }

    if( num < MAXNETNODES+1 )
    {
        // It is possible to escape to menus, and direct setting only the
        // value interferes with menu setting of the value.
        CV_SetValue( &cv_wait_players, num );
    }

    client = ( M_CheckParm ("-connect") != 0 );
    if( client )
    {
        if( netgame )  // from -dedicated or -server
        {
            I_SoftError( "Ignore -connect: conflict with -dedicated, -server.\n" );
        }
        else
        {
            // Command_connect is invoked by Init_TCP_Network.
            server = false;
	    // Init_TCP_Network tests -connect, and tests netgame,
	    // then issues connect command that sets netgame, and multiplayer.
//            netgame = true;  
        }
    }

    if( netgame )
        multiplayer = true;


    // I_InitNetwork sets port dependent settings in doomcom and netgame.
    // Check and initialize the network driver.

    // [WDJ] Can simplify this and doomcom when drop support for DOS net.
    // Only dos version with external driver will return true.
    // It uses -net to select DOSNET (vrs TCP or IP). Does not set netgame.
    if( ! I_InitNetwork() )   // startup DOSNET
    {
        // InitNetwork did not init doomcom.
        // Init doomcom and netgame.
        doomcom=Z_Malloc(sizeof(doomcom_t),PU_STATIC,NULL);
        memset(doomcom,0,sizeof(doomcom_t));
        doomcom->id = DOOMCOM_ID;        
        doomcom->unused_deathmatch = 0;  // unused
        doomcom->consoleplayer = 0;
        doomcom->extratics = 0;

        I_Init_TCP_Network();
    }
    doomcom->num_player_netnodes = 0;
    doomcom->unused_ticdup = 1;  // unused
    netbuffer = (netbuffer_t *)&doomcom->data;

    if (M_CheckParm ("-extratic"))
    {
        // extratic causes redundant transmission of tics, to prevent
        // retransmission of player movement
        // Combination of the vanilla -extratic and -dup
        num = 1;  // default param on -extratic
        if( M_IsNextParm() )
        {
            num = atoi(M_GetNextParm());
            if( num < 0 )
               num = 0;
            if( num > 20 )
               num = 20;  // reasonable
        }
        CONS_Printf("Set extratic to %d\n", num);
        doomcom->extratics = num;
    }

    // Defaults
    hardware_MAXPACKETLENGTH = MAXPACKETLENGTH;
    net_bandwidth = 3000;

    if(M_CheckParm ("-bandwidth"))
    {
        // set the expected network bandwith, in bytes per second
        // default is 3K
        if(M_IsNextParm())
        {
            net_bandwidth = atoi(M_GetNextParm());
            if( net_bandwidth<1000 ) 
                net_bandwidth=1000;
            if( net_bandwidth>100000 )  
                hardware_MAXPACKETLENGTH = MAXPACKETLENGTH;
            CONS_Printf("Network bandwidth set to %d\n",net_bandwidth);
        }
        else
            I_Error("usage : -bandwidth <byte_per_sec>");
    }

    software_MAXPACKETLENGTH=hardware_MAXPACKETLENGTH;
    if(M_CheckParm ("-packetsize"))
    {
        num = atoi(M_GetNextParm());
        if(num < 75)
           num = 75;
        if(num > hardware_MAXPACKETLENGTH)
           num = hardware_MAXPACKETLENGTH;
        software_MAXPACKETLENGTH = num;
    }

#ifdef PARANOIA
    if (doomcom->id != DOOMCOM_ID)
        I_Error ("Doomcom buffer invalid!");
#endif

#ifdef DEBUGFILE
    if (M_CheckParm ("-debugfile"))
    {
        char    filename[20];
        int     k=doomcom->consoleplayer-1;
        if( M_IsNextParm() )
            k = atoi(M_GetNextParm())-1;
        while (!debugfile && k<MAXPLAYERS)
        {
            k++;
            sprintf (filename,"debug%i.txt",k);
            debugfile = fopen (filename,"w");
        }
        if( debugfile )
            CONS_Printf ("debug output to: %s\n",filename);
        else
            CONS_Printf ("\2cannot debug output to file !\n",filename);
    }
#endif

    // Bring up higher level functions.
    D_Init_ClientServer(); // inits numplayers=0

    // Last because these need dedicated and server flags.
    SV_ResetServer();
    if(dedicated)
        SV_SpawnServer();

    return netgame;
}


extern void D_CloseConnection( void )
{
    int i;

    if( netgame )
    {
        // wait the ack_return with timout of 5 Sec
        Net_Wait_AllAckReceived(5);

        // close all connection
        for( i=0; i<MAX_CON_NETNODE; i++ )
            Net_CloseConnection(i | FORCECLOSE);

        InitAck();

        if( I_NetCloseSocket )
            I_NetCloseSocket();
        
        I_NetGet           = Internal_Get;
        I_NetSend          = Internal_Send;
        I_NetCanSend       = NULL;
        I_NetCloseSocket   = NULL;
        I_NetFreeNode	   = Internal_FreeNode;
        I_NetMakeNode      = NULL;
        netgame = false;
    }
}
