// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: i_tcp.c 1154 2015-04-03 13:58:51Z wesleyjohnson $
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
// $Log: i_tcp.c,v $
// Revision 1.40  2004/04/20 00:34:26  andyp
// Linux compilation fixes and string cleanups
//
// Revision 1.39  2003/05/04 02:31:53  sburke
// Added many #ifdef, #ifndef SOLARIS.
//
// Revision 1.38  2003/01/19 21:24:26  bock
// Make sources buildable on FreeBSD 5-CURRENT.
//
// Revision 1.37  2001/08/26 15:27:29  bpereira
// added fov for glide and fixed newcoronas code
//
// Revision 1.36  2001/08/21 21:53:37  judgecutor
// Fixed incorect place of #include "d_main.h"
//
// Revision 1.35  2001/08/20 20:40:39  metzgermeister
//
// Revision 1.34  2001/05/16 22:33:34  bock
// Initial FreeBSD support.
//
// Revision 1.33  2001/02/24 13:35:20  bpereira
// Revision 1.32  2001/02/10 12:27:13  bpereira
//
// Revision 1.31  2001/01/05 18:17:43  hurdler
// fix master server bug
//
// Revision 1.30  2000/11/26 00:46:31  hurdler
// Revision 1.29  2000/10/21 08:43:29  bpereira
// Revision 1.28  2000/10/16 20:02:29  bpereira
// Revision 1.27  2000/10/08 13:30:00  bpereira
//
// Revision 1.26  2000/10/01 15:20:23  hurdler
// Add private server
//
// Revision 1.25  2000/09/28 20:57:15  bpereira
// Revision 1.24  2000/09/15 19:49:22  bpereira
// Revision 1.23  2000/09/10 10:43:21  metzgermeister
//
// Revision 1.22  2000/09/08 22:28:30  hurdler
// merge masterserver_ip/port in one cvar, add -private
//
// Revision 1.21  2000/09/01 18:23:42  hurdler
// fix some issues with latest network code changes
//
// Revision 1.20  2000/08/31 14:30:55  bpereira
//
// Revision 1.19  2000/08/29 15:53:47  hurdler
// Remove master server connect timeout on LAN (not connected to Internet)
//
// Revision 1.18  2000/08/21 11:06:44  hurdler
// Add ping and some fixes
//
// Revision 1.17  2000/08/17 23:18:05  hurdler
// fix bad port sent to master server when using -udpport
//
// Revision 1.16  2000/08/16 23:39:41  hurdler
// fix a bug with windows sockets
//
// Revision 1.15  2000/08/16 17:21:50  hurdler
// update master server code (bis)
//
// Revision 1.14  2000/08/16 15:44:18  hurdler
// update master server code
//
// Revision 1.13  2000/08/16 14:10:01  hurdler
// add master server code
//
// Revision 1.12  2000/08/10 14:55:56  ydario
// OS/2 port
//
// Revision 1.11  2000/08/10 14:08:48  hurdler
// Revision 1.10  2000/08/03 17:57:42  bpereira
//
// Revision 1.9  2000/04/21 13:03:27  hurdler
// apply Robert's patch for SOCK_Get error. Boris, can you verify this?
//
// Revision 1.8  2000/04/21 00:01:45  hurdler
// apply Robert's patch for SOCK_Get error. Boris, can you verify this?
//
// Revision 1.7  2000/04/16 18:38:07  bpereira
// Revision 1.6  2000/03/29 19:39:48  bpereira
//
// Revision 1.5  2000/03/08 14:44:52  hurdler
// fix "select" problem under linux
//
// Revision 1.4  2000/03/07 03:32:24  hurdler
// fix linux compilation
//
// Revision 1.3  2000/03/06 15:46:43  hurdler
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
// NOTE:    This is not realy Os dependant because all Os have the same Socket api
//          Just use '#ifdef' for Os dependant stuffs
//
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef __OS2__
  // sys/types.h is also included unconditionally by doomincl.h
# include <sys/types.h>
# include <sys/time.h>
#endif // __OS2__

#include "doomincl.h"

#ifdef __WIN32__
# include <winsock2.h>
# include <ws2tcpip.h>
# ifdef USE_IPX
# include <wsipx.h>
# endif // USE_IPX
#else
  // Not Windows

# if !defined(SCOUW2) && !defined(SCOUW7) && !defined(__OS2__)
#  include <arpa/inet.h>
# endif

// non-windows includes
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <time.h>


#define STD_STRING_LEN 256 // Just some standard length for a char string

// [WDJ] FIXME: Add some test of IPX headers being present.
// When someone puts an IPX package on their system, this will prevent
// using it.

#ifdef SOLARIS
// Previous code: Solaris did not have IPX.
# ifdef USE_IPX
#   undef USE_IPX
# endif
#endif

// Reported to be __OpenBSD__ , but it should be all caps and I am paranoid.
#if defined( __OpenBSD__ ) || defined( __OPENBSD__ )
// OpenBSD does not have IPX.
# ifdef USE_IPX
#   undef USE_IPX
# endif
#endif

#ifdef __DJGPP__
#include <lsck/lsck.h>
#ifdef USE_IPX
//#define strerror  lsck_strerror
// ipx not yet supported in libsocket (cut and pasted from wsipx.h (winsock)
// [WDJ] Updated to stdint
typedef struct sockaddr_ipx {
    int16_t   sa_family;
    char  sa_netnum[4];
    char  sa_nodenum[6];
    uint16_t  sa_socket;
} SOCKADDR_IPX, *PSOCKADDR_IPX;
#define NSPROTO_IPX      1000
#endif // USE_IPX
#endif // djgpp


#ifdef __OS2__
#ifdef USE_IPX
// ipx not yet supported in libsocket (cut and pasted from wsipx.h (winsock)
#define AF_IPX          23              /* Novell Internet Protocol */
// [WDJ] Updated to stdint.
typedef struct sockaddr_ipx {
    int16_t   sa_family;
    char  sa_netnum[4];
    char  sa_nodenum[6];
    uint16_t  sa_socket;
} SOCKADDR_IPX, *PSOCKADDR_IPX;
#define NSPROTO_IPX      1000
#endif // USE_IPX
#endif // os2


#ifdef LINUX
# include <sys/time.h>
# ifdef USE_IPX
#  ifdef __GLIBC__
#   include <netipx/ipx.h>
#  else
#   ifdef FREEBSD
#    include <netipx/ipx.h>
#   else
#    include <linux/ipx.h>
#   endif
#  endif // glibc
    typedef struct sockaddr_ipx SOCKADDR_IPX, *PSOCKADDR_IPX;
# define NSPROTO_IPX      PF_IPX
# endif // USE_IPX
#endif // linux

  // END of Not Windows
#endif // win32

#include "i_system.h"
#include "i_net.h"
#include "d_net.h"
#include "m_argv.h"
#include "command.h"
#include "d_main.h"

#include "doomstat.h"
#include "mserv.h" //Hurdler: support master server

#ifdef __WIN32__
    // some undefined under win32
#  define IPPORT_USERRESERVED 5000
#ifdef errno
#  undef errno
#endif
   // some very strange things happen when not use h_error ?!?
#  define errno         h_errno
#else
   // linux, djgpp, os2, non-windows
#  define  SOCKET int
#  define  INVALID_SOCKET -1
# ifdef FREEBSD
// From edwin: somewhere on the track between 4.5 and -current this one has disappered.
#  ifndef IPPORT_USERRESERVED
#    define IPPORT_USERRESERVED 5000
#  endif
# endif
#endif

#if defined( WIN32) || defined( __DJGPP__ ) 
   // win32 or djgpp
    // winsock stuff (in winsock a socket is not a file)
#  define ioctl ioctlsocket
#  define close closesocket
#endif


// A network address, kept in network byte order.
typedef union {
        struct sockaddr_in  ip;
#ifdef USE_IPX
        struct sockaddr_ipx ipx;
#endif
}  mysockaddr_t;

// Player and additional net nodes.
static mysockaddr_t clientaddress[MAX_CON_NETNODE];

#define NODE_ADDR_HASHING
#ifdef NODE_ADDR_HASHING
// To receive must set to hash of clientaddress.
// To send, any value > 0 will enable.
#else
// Use node_hash as node connected flag.
#endif
// Contains address hash, is 0 when unused.  Hash is not allowed to be 0.
static byte     node_hash[MAX_CON_NETNODE];
        
static SOCKET   mysocket = -1;


#ifdef USE_IPX
static boolean  ipx;
#endif
int sock_port = (IPPORT_USERRESERVED +0x1d );  // 5029

// Network is big-endian, 386,486,586 PC are little-endian.
// htons: host to net byte order
// ntohs: net to host byte order

#if defined( WIN32) || defined( __OS2__) || defined( SOLARIS)
// [WDJ] Also defined in mserv.c, but too small, will be inlined anyway.
static inline
int inet_aton(const char *hostname,
	      /* OUT */ struct in_addr *addr)
{
    // [WDJ] This cannot handle 255.255.255.255, which == INADDR_NONE.
    addr->s_addr = inet_addr(hostname);
    return ( addr->s_addr != INADDR_NONE );
}   
#endif

// To print error messages
char *SOCK_AddrToStr(mysockaddr_t *sk)
{
    static char s[50];

    if( sk->ip.sin_family==AF_INET)
    {
        // Internet address
        sprintf(s,"%d.%d.%d.%d:%d",((byte *)(&(sk->ip.sin_addr.s_addr)))[0],
                                   ((byte *)(&(sk->ip.sin_addr.s_addr)))[1],
                                   ((byte *)(&(sk->ip.sin_addr.s_addr)))[2],
                                   ((byte *)(&(sk->ip.sin_addr.s_addr)))[3],
                                   ntohs(sk->ip.sin_port));
    }
#ifdef USE_IPX
    else
#ifdef LINUX
    if( sk->ipx.sipx_family==AF_IPX )
    {
# ifdef FREEBSD
        // FreeBSD IPX
        sprintf(s,"%s", ipx_ntoa(sk->ipx.sipx_addr));
# else
        // Linux IPX, but Not FreeBSD
        sprintf(s,"%08x.%02x%02x%02x%02x%02x%02x:%d",
                  sk->ipx.sipx_network,
                  (byte)sk->ipx.sipx_node[0],
                  (byte)sk->ipx.sipx_node[1],
                  (byte)sk->ipx.sipx_node[2],
                  (byte)sk->ipx.sipx_node[3],
                  (byte)sk->ipx.sipx_node[4],
                  (byte)sk->ipx.sipx_node[5],
                  sk->ipx.sipx_port);
# endif
    }
#else
    // IPX Windows, OS2, DJGPP
    if( sk->ipx.sa_family==AF_IPX )
    {
        // IPX address
        sprintf(s,"%02x%02x%02x%02x.%02x%02x%02x%02x%02x%02x:%d",
                  (byte)sk->ipx.sa_netnum[0],
                  (byte)sk->ipx.sa_netnum[1],
                  (byte)sk->ipx.sa_netnum[2],
                  (byte)sk->ipx.sa_netnum[3],
                  (byte)sk->ipx.sa_nodenum[0],
                  (byte)sk->ipx.sa_nodenum[1],
                  (byte)sk->ipx.sa_nodenum[2],
                  (byte)sk->ipx.sa_nodenum[3],
                  (byte)sk->ipx.sa_nodenum[4],
                  (byte)sk->ipx.sa_nodenum[5],
                  sk->ipx.sa_socket);
    }
#endif // linux
#endif // USE_IPX
    else
        sprintf(s,"Unknown type");
    return s;
}

#ifdef USE_IPX
boolean IPX_cmpaddr(mysockaddr_t *a, mysockaddr_t *b)
{
#ifdef LINUX
#ifdef FREEBSD
    // FreeBSD: IPX address compare
    return ipx_neteq( a->ipx.sipx_addr, b->ipx.sipx_addr) &&
           ipx_hosteq( a->ipx.sipx_addr, b->ipx.sipx_addr );
#else
    // Linux (except FreeBSD): IPX address compare
    return ((memcmp(&(a->ipx.sipx_network) ,&(b->ipx.sipx_network) ,4)==0) &&
            (memcmp(&(a->ipx.sipx_node),&(b->ipx.sipx_node),6)==0));
#endif
#else
    // Windows, OS2, DJGPP: IPX address compare
    return ((memcmp(&(a->ipx.sa_netnum) ,&(b->ipx.sa_netnum) ,4)==0) &&
            (memcmp(&(a->ipx.sa_nodenum),&(b->ipx.sa_nodenum),6)==0));
#endif // linux
}

#ifdef NODE_ADDR_HASHING
byte  IPX_hashaddr(mysockaddr_t *a)
{
    // Not allowed to be 0.
    // Big endian, want final addr byte.
#ifdef LINUX
    // Linux: IPX address hash
    return ((byte)(a->ipx.sipx_node[5])) | 0x80;
#else
    // Windows, OS2, DJGPP: IPX address hash
    return ((byte)(a->ipx.sa_nodenum[5])) | 0x80;
#endif // linux
}
#endif

#endif // USE_IPX

boolean UDP_cmpaddr(mysockaddr_t *a, mysockaddr_t *b)
{
    return (a->ip.sin_addr.s_addr == b->ip.sin_addr.s_addr
	    && a->ip.sin_port == b->ip.sin_port);
}

#ifdef NODE_ADDR_HASHING
byte  UDP_hashaddr(mysockaddr_t *a)
{
    // Not allowed to be 0.
    // Big endian, want final addr byte.
    return ((byte*)(&(a->ip.sin_addr.s_addr)))[3] | 0x80;
}
#endif

// Indirect function for net address compare.
boolean (*SOCK_cmpaddr) (mysockaddr_t *a, mysockaddr_t *b);
#ifdef NODE_ADDR_HASHING
byte    (*SOCK_hashaddr) (mysockaddr_t *a);
#endif


// Set address and port of utility net nodes.
//  saddr: IP address in network byte order
//  port: port number in host byte order
void UDP_Bind_Node( int nnode, unsigned int saddr, unsigned int port )
{
    clientaddress[nnode].ip.sin_family      = AF_INET;
    clientaddress[nnode].ip.sin_port        = htons(port);
    clientaddress[nnode].ip.sin_addr.s_addr = saddr;
#ifdef NODE_ADDR_HASHING
    node_hash[nnode] = UDP_hashaddr( &clientaddress[nnode] );
#else
    node_hash[nnode] = 1;
#endif
}


  // Return net node.  When nodes full, return 255.
static byte get_freenode( void )
{
    byte nn;

    // Only this range is dynamically allocated, the others are preallocated.
    for( nn=1; nn<MAXNETNODES; nn++)  // self is not free
    {
        if( node_hash[nn] == 0 )
        {
#ifdef NODE_ADDR_HASHING
            node_hash[nn]=1;  // enable send, but hash is needed to receive
#else
            node_hash[nn]=1;  // used as node_connection flag
#endif
            return nn;
        }
    }
    return 255;
}

// Function for I_NetFreeNode().
void SOCK_FreeNode(int nnode)
{
    // can't disconnect to self :)
    if( nnode == 0 )
        return;

#ifdef DEBUGFILE
    if( debugfile )
    {
        fprintf(debugfile,"Free node %d (%s)\n",
		nnode, SOCK_AddrToStr(&clientaddress[nnode]));
    }
#endif

    // Disconnect and invalid address.
    node_hash[nnode] = 0;
    memset(&clientaddress[nnode], 0, sizeof(clientaddress[nnode]));
}


//Hurdler: something is wrong with Robert's patch and win2k
// Function for I_NetGet().
// Return packet into doomcom struct.
// Return true when got packet.  Error in net_error.
boolean  SOCK_Get(void)
{
    byte  nnode;
#ifdef NODE_ADDR_HASHING
    byte  hashaddr;
#endif
    int   rcnt;  // data bytes received
    socklen_t     fromlen;
    mysockaddr_t  fromaddress;

    fromlen = sizeof(fromaddress);  // num bytes of addr for OUT
    // fromaddress: OUT the actual address.
    // fromlen: IN sizeof fromaddress, OUT the actual length of the address.
#ifdef LINUX
    rcnt = recvfrom(mysocket,
		    &doomcom->data,  // packet
		    MAXPACKETLENGTH,  // packet length
		    0,  // flags
                    /*OUT*/ (struct sockaddr *)&fromaddress,  // net address
		    /*IN,OUT*/ &fromlen );  // net address length
#else
    // winsock.h  recvfrom(SOCKET, char*, int, int, struct sockaddr*, int*)
    rcnt = recvfrom(mysocket,
		    // Some other port requires (char*), undocumented.
		    (char *)&doomcom->data,
		    MAXPACKETLENGTH,  // packet length
		    0,  // flags
                    /*OUT*/ (struct sockaddr *)&fromaddress,  // net address
		    /*IN,OUT*/ &fromlen );  // net address length
#endif
    if(rcnt < 0)  goto recv_err;
    
//    DEBFILE(va("Get from %s\n",SOCK_AddrToStr(&fromaddress)));

    // Find remote node number, player nodes only.
#ifdef NODE_ADDR_HASHING
    hashaddr = SOCK_hashaddr( &fromaddress );  // hash != 0
    // GenPrintf(EMSG_debug, "hashaddr=%d\n", hashaddr );
#endif
    for (nnode=0; nnode<MAXNETNODES; nnode++)
    {
#ifdef NODE_ADDR_HASHING
        // [WDJ] avoid testing null addresses.
        if( node_hash[nnode] != hashaddr )  continue;
#endif
        if( SOCK_cmpaddr(&fromaddress, &(clientaddress[nnode])) )
	     goto return_node;  // found match
    }

    // Net node not found.
    nnode = get_freenode();  // Find a free node.
    if(nnode >= MAXNETNODES)  goto no_nodes;

#ifdef NODE_ADDR_HASHING
    // Set node_hash[nnode] to enable receive.
    node_hash[nnode] = hashaddr;
#endif
    // Save the addr of the net node.
    memcpy(&clientaddress[nnode], &fromaddress, fromlen);
#ifdef DEBUGFILE
    if( debugfile )
    {
        fprintf(debugfile,"New node detected: node:%d address:%s\n",
		nnode, SOCK_AddrToStr(&clientaddress[nnode]));
    }
#endif

return_node:
    doomcom->remotenode = nnode; // good packet from a game player
    doomcom->datalength = rcnt;
    return true;

    // Rare errors
recv_err:
    // Send failed, determine the error.
#ifdef __WIN32__
    if(errno == WSAEWOULDBLOCK || errno == WSATRY_AGAIN )  // no message
#else
    if(errno == EWOULDBLOCK || errno == EAGAIN)   // no message
#endif
    {
        net_error = NE_empty;
        goto no_packet;
    }

#ifdef __WIN32__
    if( (errno == WSAEMSGSIZE)   // message too large
	|| (errno == WSAECONNREFUSED) )  // connection refused
#else
    if( (errno == EMSGSIZE)   // message too large
	|| (errno == ECONNREFUSED) )  // connection refused
#endif
    {
        net_error = NE_fail;
        goto no_packet;
    }
   
    I_SoftError("SOCK_Get: %s\n", strerror(errno));

#ifdef __WIN32__
    if( errno == WSAENETUNREACH || errno == WSAEFAULT || errno == WSAEBADF )
#else
    if( errno == ENETUNREACH || errno == EFAULT || errno == EBADF )
#endif   
    {
        // network unreachable
        net_error = NE_network_unreachable; // allows test net without crashing
        goto no_packet;
    }
    // Many other errors.
    I_Error("SOCK_Get\n");

no_nodes:
#ifdef DEBUGFILE
    // node table full
    if( debugfile )
        fprintf(debugfile,"SOCK_Get: Free nodes all used.\n");
#endif
    net_error = NE_nodes_exhausted;
    goto no_packet;

no_packet:
    doomcom->remotenode = -1;  // no packet
    return false;
}


static fd_set  write_set;  // Linux: modified by select

// Function for I_NetCanSend().
// Check if we can send (to save a buffer transfer).
boolean SOCK_CanSend(void)
{
    // [WDJ] Linux: select modifies timeval, so it must be init with each call.
    struct timeval timeval_0 = {0,0};  // immediate
    int stat;

    // [WDJ] Linux: write_set is modified by the select call, so it must
    // be init each call.  Smaller code with write_set static global.
    FD_ZERO(&write_set);
    FD_SET(mysocket, &write_set);
    // huh Boris, are you sure about the 1th argument:
    // it is the highest-numbered descriptor in any of the three
    // sets, plus 1 (I suppose mysocket + 1).
    // BP:ok, no prob since it is ignored in windows :)
    // Linux: select man page specifies (highest file descriptor + 1).
    // winsock.h: select(int, fd_set*, fd_set*, fd_set*, const struct timeval*)
    stat = select(mysocket + 1,
		  NULL,  // read fd
		  /*IN,OUT*/ &write_set,  // write fd to watch
		  NULL,  // exceptions
		  /*IN,OUT*/ &timeval_0   // timeout
		 );
    return ( stat > 0 );
}


// Function for I_NetSend().
// Send packet from within doomcom struct.
// Return true when packet has been sent.  Error in net_error.
boolean  SOCK_Send(void)
{
    byte  nnode = doomcom->remotenode;
    int  cnt;  // chars sent
                         
    if( node_hash[nnode] == 0 )   goto node_unconnected;

    // sockaddr is defined in sys/socket.h
#ifdef LINUX
    cnt = sendto(mysocket,
		&doomcom->data, doomcom->datalength,  // packet
                0,  // flags
		(struct sockaddr *)&clientaddress[nnode],  // net address
                sizeof(struct sockaddr));  // net address length
#else
    // winsock.h: sendto(SOCKET, char*, int, int, struct sockaddr*, int)
    cnt = sendto(mysocket,
		// Some other port requires (char*), undocumented.
		(char *)&doomcom->data, doomcom->datalength,  // packet
                0,  // flags
		(struct sockaddr *)&clientaddress[nnode],  // net address
                sizeof(struct sockaddr));  // net address length
#endif

//    DEBFILE(va("send to %s\n",SOCK_AddrToStr(&clientaddress[doomcom->remotenode])));
    if( cnt < 0 )  goto send_err;
    return true;
   
    // Rare error.
send_err:
//  if( errno == ENOBUFS )  // out of buffer space
#ifdef __WIN32__
    if( errno == WSAEWOULDBLOCK || errno == WSATRY_AGAIN )
#else
    // Linux
    // ECONNREFUSED can be got in linux port.
    if( errno == ECONNREFUSED || errno == EWOULDBLOCK || errno == EAGAIN )
#endif
    {
        net_error = NE_congestion;  // silent
        goto err_return;
    }

//        printf( "errno= %d  ", errno );
    I_SoftError("SOCK_Send to node %d (%s): %s\n",
		 nnode,
		 SOCK_AddrToStr(&clientaddress[nnode]),
		 strerror(errno));

//#if defined(WIN32) && defined(__MINGW32__)
#ifdef __WIN32__
    if( errno == WSAENETUNREACH || errno == WSAEFAULT || errno == WSAEBADF )
#else
    // Linux
    if( errno == ENETUNREACH || errno == EFAULT || errno == EBADF )
#endif
    {
        // network unreachable
        net_error = NE_network_unreachable; // allows test net without crashing
        goto err_return;
    }
    // Many other errors.
    I_Error("SOCK_Send\n");

node_unconnected:
    net_error = NE_node_unconnected;
    goto err_return;

err_return:
    return false;
}


//
// UDPsocket
//
static SOCKET  UDP_Socket (void)
{
    SOCKET s;
    struct sockaddr_in  address;
#if defined(WIN32) && defined(__MINGW32__)
    u_long trueval = true;
#else
    int    trueval = true;
#endif
    int optval;
    socklen_t optlen;
    int stat;

    // allocate a socket
    s = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s<0 || s==INVALID_SOCKET)
    {
        I_SoftError("UDP_socket: Create socket failed: %s\n", strerror(errno));
        goto no_socket;
    }

    memset (&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    
    //Hurdler: I'd like to put a server and a client on the same computer
    //BP: in fact for client we can use any free port we want i have read 
    //    in some doc that connect in udp can do it for us...
    if( M_CheckParm ("-clientport") )
    {
        if( !M_IsNextParm() )
            I_Error("syntax : -clientport <portnum>");
        address.sin_port = htons(atoi(M_GetNextParm()));
    }
    else
        address.sin_port = htons(sock_port);

    stat = bind (s, (struct sockaddr *)&address, sizeof(address) );
    if (stat == -1)
    {
        I_SoftError("UDP_Socket: Bind failed: %s\n", strerror(errno));
        goto no_socket;
    }

    // make it non blocking
    ioctl (s, FIONBIO, &trueval);

    // make it broadcastable
#ifdef LINUX
    stat = setsockopt(s, SOL_SOCKET, SO_BROADCAST,
		      &trueval,  // option value
		      sizeof(trueval));  // length of value
#else
    // winsock.h:  getsockopt(SOCKET, int, int, char*, int*)
    stat = setsockopt(s, SOL_SOCKET, SO_BROADCAST,
		      // Some other port requires (char*), undocumented.
		      (char *)&trueval,  // option value
		      sizeof(trueval));  // length of value
#endif

    // Set Network receive buffer size.
    optlen=sizeof(optval);  // Linux: gets modified
    // optval: gets the value of the option
    // optlen: gets the actual length of the option
#ifdef LINUX
    stat = getsockopt(s, SOL_SOCKET, SO_RCVBUF,
		      /* OUT */ &optval,  // option value
		      /* IN,OUT */ &optlen);  // available length
#else
    // FIXME: so an int value is written to a (char *); portability!!!!!!!
    // winsock.h:  getsockopt(SOCKET, int, int, char*, int*)
    stat = getsockopt(s, SOL_SOCKET, SO_RCVBUF,
		      // Some other port requires (char*), undocumented.
		      /* OUT */ (char *)&optval,  // option value
		      /* IN,OUT */ &optlen);  // available length
#endif
    CONS_Printf("Network receive buffer: %dKb\n", optval>>10);

    if(optval < (64<<10)) // 64k
    {
        optval = (64<<10);
#ifdef LINUX
        stat = setsockopt(s, SOL_SOCKET, SO_RCVBUF,
			  &optval,
			  sizeof(optval));
#else
        // winsock.h  setsockopt(SOCKET, int, int, const char*, int)
        stat = setsockopt(s, SOL_SOCKET, SO_RCVBUF,
			  // Some other port requires (char*), undocumented.
			  (char *)&optval,
			  sizeof(optval));
#endif
        if( stat < 0 )
            CONS_Printf("Network receive buffer: Failed to set buffer to 64k.\n");
        else
            CONS_Printf("Network receive buffer: set to %dKb\n", optval>>10);
    }

    // ip + udp
    net_packetheader_length = 20 + 8; // for stats

    // should not receive from self, but will set it up anyway.
    clientaddress[0].ip.sin_family      = AF_INET;
    clientaddress[0].ip.sin_port        = htons(sock_port);
    clientaddress[0].ip.sin_addr.s_addr = INADDR_LOOPBACK;
                                  // inet_addr("127.0.0.1");
#ifdef NODE_ADDR_HASHING
    node_hash[0] = UDP_hashaddr( &clientaddress[0] );
#endif

    // Setup broadcast adress to BROADCASTADDR entry
    // To send broadcasts, PT_ASKINFO
    clientaddress[BROADCASTADDR].ip.sin_family      = AF_INET;
    clientaddress[BROADCASTADDR].ip.sin_port        = htons(sock_port);
    clientaddress[BROADCASTADDR].ip.sin_addr.s_addr = INADDR_BROADCAST;
#ifdef NODE_ADDR_HASHING
//    node_hash[BROADCASTADDR] = UDP_hashaddr( &clientaddress[BROADCASTADDR] );
    node_hash[BROADCASTADDR] = 1;  // send only
#endif

    doomcom->extratics=1; // internet is very high ping

    SOCK_cmpaddr=UDP_cmpaddr;
#ifdef NODE_ADDR_HASHING
    SOCK_hashaddr=UDP_hashaddr;
#endif
    return s;
   
no_socket:
    return -1;
}


#ifdef USE_IPX
static SOCKET  IPX_Socket (void)
{
    SOCKET s;
    SOCKADDR_IPX  address;
#if defined(WIN32) && defined(__MINGW32__)
    u_long trueval = true;
#else
    int    trueval = true;
#endif
    int    optval;
    socklen_t optlen;
    int  stat, i;

    // allocate a socket
    s = socket (AF_IPX, SOCK_DGRAM, NSPROTO_IPX);
    if (s<0 || s==INVALID_SOCKET)
    {
        I_SoftError("IPX_socket: Create socket failed: %s\n", strerror(errno));
        goto no_ipx;
    }

    memset (&address, 0, sizeof(address));
#ifdef LINUX
    address.sipx_family = AF_IPX;
    address.sipx_port = htons(sock_port);
#else
    address.sa_family = AF_IPX;
    address.sa_socket = htons(sock_port);
#endif // linux
    stat = bind (s, (struct sockaddr *)&address, sizeof(address));
    if( stat == -1)
    {
        I_SoftError("IPX_Socket: Bind failed: %s\n", strerror(errno));
        goto no_ipx;
    }

    // make it non blocking
    ioctl (s, FIONBIO, &trueval);

    // make it broadcastable
#ifdef LINUX
    stat = setsockopt(s, SOL_SOCKET, SO_BROADCAST,
		      &trueval,  // option value
		      sizeof(trueval));
#else
    // winsock.h:  setsockopt(SOCKET, int, int, const char*, int)
    stat = setsockopt(s, SOL_SOCKET, SO_BROADCAST,
		      // Some other port requires (char*), undocumented.
		      (char *)&trueval,  // option value
		      sizeof(trueval));
#endif

    // Set Network receive buffer size.
    optlen=sizeof(optval);  // gets modified
    // optval: gets the value of the option
    // optlen: gets the actual length of the option
#ifdef LINUX
    stat = getsockopt(s, SOL_SOCKET, SO_RCVBUF,
		      /* OUT */ &optval,  // option value
		      /* IN,OUT */ &optlen);  // available length
#else
    // FIXME: so an int value is written to a (char *); portability!!!!!!!
    // winsock.h  getsockopt(SOCKET, int, int, char*, int*)
    stat = getsockopt(s, SOL_SOCKET, SO_RCVBUF,
		      // Some other port requires (char*), undocumented.
		      /* OUT */ (char *)&optval,  // option value
		      /* IN,OUT */ &optlen);  // available length
#endif
    // [WDJ] Had remnants of 64K.  Set to 128K.
    CONS_Printf("Network receive buffer: %dKb\n", optval>>10);
    if(optval < (128<<10)) // 128K
    {
        optval = (128<<10);
        stat = setsockopt(s, SOL_SOCKET, SO_RCVBUF,
			  (char *)&optval,
			  sizeof(optval));
        if( stat < 0 )
            CONS_Printf("Network receive buffer: Failed to set buffer to 128k.\n");
        else
            CONS_Printf("Network receive buffer: set to %dKb\n", optval>>10);
    }

    // ipx header
    net_packetheader_length=30; // for stats

    // setup broadcast adress to BROADCASTADDR entry
#ifdef LINUX
    clientaddress[BROADCASTADDR].ipx.sipx_family = AF_IPX;
    clientaddress[BROADCASTADDR].ipx.sipx_port = htons(sock_port);
#ifdef FREEBSD
    // FreeBSD
    clientaddress[BROADCASTADDR].ipx.sipx_addr.x_net.s_net[0] = 0;
    clientaddress[BROADCASTADDR].ipx.sipx_addr.x_net.s_net[1] = 0;
    for(i=0;i<6;i++)
       clientaddress[BROADCASTADDR].ipx.sipx_addr.x_host.c_host[i] = (byte)0xFF;
#else
    // Linux, but Not FreeBSD
    clientaddress[BROADCASTADDR].ipx.sipx_network = 0;
    for(i=0;i<6;i++)
       clientaddress[BROADCASTADDR].ipx.sipx_node[i] = (byte)0xFF;
#endif
#else
    // Windows, etc.
    clientaddress[BROADCASTADDR].ipx.sa_family = AF_IPX;
    clientaddress[BROADCASTADDR].ipx.sa_socket = htons(sock_port);
    for(i=0;i<4;i++)
       clientaddress[BROADCASTADDR].ipx.sa_netnum[i] = 0;
    for(i=0;i<6;i++)
       clientaddress[BROADCASTADDR].ipx.sa_nodenum[i] = (byte)0xFF;
#endif // linux
#ifdef NODE_ADDR_HASHING
//    node_hash[BROADCASTADDR] = IPX_hashaddr( &clientaddress[BROADCASTADDR] );
    node_hash[BROADCASTADDR] = 1;  // send only
#endif

    SOCK_cmpaddr=IPX_cmpaddr;
#ifdef NODE_ADDR_HASHING
    SOCK_hashaddr=IPX_hashaddr;
#endif
    return s;

no_ipx:
    return -1;
}
#endif // USE_IPX

//Hurdler: temporary addition and changes for master server

static int init_tcp_driver = 0;

void I_InitTcpDriver(void)
{
    if (!init_tcp_driver)
    {
#ifdef __WIN32__
        WSADATA winsockdata;
        if( WSAStartup(MAKEWORD(1,1),&winsockdata) )
            I_Error("No Tcp/Ip driver detected");
#endif
#ifdef __DJGPP_
        if( !__lsck_init() )
            I_Error("No Tcp/Ip driver detected");
#endif
        init_tcp_driver = 1;
    }
}


// Function for I_NetCloseSocket().
void SOCK_CloseSocket( void )
{
    if( mysocket>=0 )
    {
        //if( server )
        //    UnregisterServer(); 
#ifdef __DJGPP__
// quick fix bug in libsocket 0.7.4 beta 4 onder winsock 1.1 (win95)
#else
        // Not DJGPP
        close(mysocket);
#endif
        mysocket = -1;
    }
}

void I_ShutdownTcpDriver(void)
{
    if( mysocket!=-1 )
        SOCK_CloseSocket();

    if ( init_tcp_driver )
    {
#ifdef __WIN32__
        WSACleanup();
#endif
#ifdef __DJGPP__
        __lsck_uninit();
#endif
        init_tcp_driver = 0;
    }
}


// Function for I_NetMakeNode().
// Called by CL_UpdateServerList, Command_connect, mserv:open_UDP_Socket
// Return the net node number, or network_error_e.
int SOCK_NetMakeNode (char *hostname)
{
    int newnode;
    mysockaddr_t  newaddr;
    char *localhostname;  // owns string
    char *portchar;
    int portnum = htons(sock_port);

    // [WDJ] From command line can get "192.168.127.34:5234:"
    // From console only get ""192.168.127.34", the port portion is stripped.
    localhostname = strdup(hostname);
    //GenPrintf(EMSG_debug, "Parm localhostname=%s\n", localhostname );
#define PARSE_LOCALHOSTNAME
#ifdef PARSE_LOCALHOSTNAME
    // Split into ip address and port.
    char * st = localhostname;
    strtok(st,":");  // overwrite the colon with a 0.
    portchar = strtok(NULL,":");
    if( portchar )
        portnum = htons(atoi(portchar));
#else
    // OLD code duplicates effort.
    // retrieve portnum from address !
    strtok(localhostname,":");
    portchar = strtok(NULL,":");
    if( portchar )
        portnum = htons(atoi(portchar));
    free(localhostname);
#endif
    //GenPrintf(EMSG_debug, "  hostname=%s  portchar=%s\n", localhostname, portchar );

    // server address only in ip
#ifdef USE_IPX
    if(ipx)
    {
        // ipx only
#ifdef PARSE_LOCALHOSTNAME
        free(localhostname);
#endif
        return BROADCASTADDR;
    }
#endif

    // tcp/ip
#ifdef PARSE_LOCALHOSTNAME
    // Previous operation on localhostname already parsed out the ip addr.
#else
        char            *t;

         // remove the port in the hostname as we've it already
        t = localhostname = strdup(hostname);
        while ((*t != ':') && (*t != '\0'))
            t++;
        *t = '\0';
#endif
    //GenPrintf(EMSG_debug, "  ip hostname=%s\n", localhostname );

    // Too early, but avoids resolving names we cannot use.
    newnode = get_freenode();
    if( newnode >= MAXNETNODES )
        goto no_nodes;  // out of nodes

    // Find the IP of the server.
    // [WDJ] This cannot handle addr 255.255.255.255 which == INADDR_NONE.
    newaddr.ip.sin_addr.s_addr = inet_addr(localhostname);
    if(newaddr.ip.sin_addr.s_addr==INADDR_NONE) // not a ip, ask the dns
    {
        struct hostent * hostentry;      // host information entry
        CONS_Printf("Resolving %s\n",localhostname);
        hostentry = gethostbyname (localhostname);
        if (!hostentry)
        {
	    CONS_Printf ("%s unknown\n", localhostname);
	    I_NetFreeNode(newnode);  // release the newnode
	    goto abort_makenode;
	}
        newaddr.ip.sin_addr.s_addr = *(int *)hostentry->h_addr_list[0];
    }
    CONS_Printf("Resolved %s\n",
		 inet_ntoa(*(struct in_addr *)&newaddr.ip.sin_addr.s_addr));

    // Commit to the new node.
    clientaddress[newnode].ip.sin_family      = AF_INET;
    clientaddress[newnode].ip.sin_port        = portnum;
    clientaddress[newnode].ip.sin_addr.s_addr = newaddr.ip.sin_addr.s_addr;
#ifdef NODE_ADDR_HASHING
    node_hash[newnode] = SOCK_hashaddr( &newaddr );  // hash != 0
#endif

clean_ret:
    free(localhostname);
    return newnode;

    // Rare errors.
abort_makenode:
    newnode = NE_fail;
    goto clean_ret;

no_nodes:
    newnode = NE_nodes_exhausted;
    goto clean_ret;
}


// Function for I_NetOpenSocket().
boolean SOCK_OpenSocket( void )
{
    int i;

    memset(clientaddress,0,sizeof(clientaddress));

    node_hash[0] = 1; // always connected to self
    for(i=1; i<MAX_CON_NETNODE; i++)
        node_hash[i] = 0;
   
    I_NetSend        = SOCK_Send;
    I_NetGet         = SOCK_Get;
    I_NetCloseSocket = SOCK_CloseSocket;
    I_NetFreeNode    = SOCK_FreeNode;
    I_NetMakeNode    = SOCK_NetMakeNode;


#ifdef __WIN32__
    // seem like not work with libsocket nor linux :(
    I_NetCanSend  = SOCK_CanSend;
#else
    // [WDJ] Fixed, Linux can change select time.
    I_NetCanSend  = SOCK_CanSend;
#endif

    // build the socket
    // Setup up Broadcast.
#ifdef USE_IPX
    if(ipx) {
        mysocket = IPX_Socket ();
        net_bandwidth = 800000;
        hardware_MAXPACKETLENGTH = MAXPACKETLENGTH;
    }
    else
#endif // USE_IPX
    {
        // TCP, UDP
        mysocket = UDP_Socket ();
       // if (server && cv_internetserver.value)
       //     RegisterServer(mysocket, sock_port);
    }

    return (mysocket >= 0);
}


boolean I_InitTcpNetwork( void )
{
    char     serverhostname[255];
    boolean  ret=0;
    int      num;

#ifdef USE_IPX
    ipx=M_CheckParm("-ipx");
#endif
    
    // initilize the driver
    I_InitTcpDriver(); 
    I_AddExitFunc (I_ShutdownTcpDriver);

    if ( M_CheckParm ("-udpport") )
        sock_port = atoi(M_GetNextParm());

    // parse network game options,
    if ( M_CheckParm ("-server") || dedicated)
    {
        server=true;

        // if a number of clients (i.e. nodes) is specified, the server will wait for the clients to connect before starting
        // if no number is specified here, the server starts with 1 client, others can join in-game.
        // since Boris has implemented join in-game, there is no actual need for specifying a particular number here
        // FIXME: for dedicated server, numnodes needs to be set to 0 upon start
        if( M_IsNextParm() )
        {
	    // Number of players.
            num = atoi(M_GetNextParm());
	    if( num < 0 )
	       num = 0;
	    if( num > MAXNETNODES)
               num = MAXNETNODES;
	}
        else if (dedicated)
            num = 0;
        else
            num = 1;

        doomcom->num_player_netnodes = num;

        // server
        servernode = 0;  // server set to self
        // FIXME:
        // ??? and now ?
        // server on a big modem ??? 4*isdn
        net_bandwidth = 16000;
        hardware_MAXPACKETLENGTH = INETPACKETLENGTH;

        ret = true;
    }
    else if( M_CheckParm ("-connect") )
    {
        if(M_IsNextParm())
            strcpy(serverhostname,M_GetNextParm());
        else
            serverhostname[0]=0; // assuming server in the LAN, use broadcast to detect it

        // server address only in ip
        if(serverhostname[0]
#ifdef USE_IPX	   
	   && !ipx
#endif
	   )
        {
            COM_BufAddText("connect \"");
            COM_BufAddText(serverhostname);
            COM_BufAddText("\"\n");

            // probably modem
            hardware_MAXPACKETLENGTH = INETPACKETLENGTH;
        }
        else
        {
            // so we're on a LAN
            COM_BufAddText("connect any\n");

            net_bandwidth = 800000;
            hardware_MAXPACKETLENGTH = MAXPACKETLENGTH;
        }
    }

    I_NetOpenSocket = SOCK_OpenSocket;

    return ret;
}
