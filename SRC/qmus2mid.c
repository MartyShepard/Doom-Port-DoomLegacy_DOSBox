// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: qmus2mid.c 1257 2016-09-20 17:14:21Z wesleyjohnson $
//
// Copyright (C) 1995 by Sebastien Bacquet.
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
// $Log: qmus2mid.c,v $
// Revision 1.6  2001/03/09 21:53:56  metzgermeister
//
// Revision 1.5  2001/03/03 19:45:12  ydario
// Do not compile under OS/2
//
// Revision 1.4  2000/10/08 13:30:01  bpereira
// Revision 1.3  2000/09/10 10:46:15  metzgermeister
// merged with SDL version
//
// Revision 1.2  2000/02/26 00:28:42  hurdler
// Mostly bug fix (see borislog.txt 23-2-2000, 24-2-2000)
//
//
// DESCRIPTION:
//      convert Doom MUS data to a MIDI music data
//      used by both DOS/WIN32
//
//-----------------------------------------------------------------------------

//defined OS2_MIDI_FILE_TO_FILE
#if defined __OS2__ && defined OS2_MIDI_FILE_TO_FILE
// OS2 is using a file to file version of qmus, in os2/qmus2mid2.c

#else

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "doomincl.h"
#include "qmus2mid.h"
#include "i_system.h"
#include "byteptr.h"
#include "m_swap.h"

// MUS events
#define MUS_EV_SCOREEND     6

// MIDI
#define MIDIHEADER    "MThd\000\000\000\006\000\001"
  // length 6, format 1
#define MIDICREATEPROG  "\000\377\002\026"
  // 0x00, 0xff, 0x02, 0x16
#define MIDIKEY  "\000\377\131\002\000\000"
  // 0x00, 0xff, 0x59, 0x02, 0x00 0x00    // C major
#define MIDITEMPO  "\000\377\121\003\011\243\032"
  // 0x00, 0xff, 0x51, 0x03, 0x09, 0xa3, 0x1a   // usec/quarter_note
#define MIDIEND  "\000\377\057\000"
  // 0x00, 0xff, 0x2f, 0x00    // end of track, header
#define MIDITRACKSRC  "\000\377\003\035"
  // 0x00, 0xff, 0x03, 0x1d


struct MUSheader_s
{
    char          ID[4];            // identifier "MUS" 0x1A
    uint16_t      scoreLength;      // length of score in bytes
    uint16_t      scoreStart;       // absolute file pos of the score
    uint16_t      channels;         // count of primary channels
    uint16_t      sec_channels;      // count of secondary channels
    uint16_t      instrCnt;
    uint16_t      dummy;
    // variable-length part starts here
    uint16_t      instruments[0];
};
typedef struct MUSheader_s MUSheader;

// internal
struct Track
{
    unsigned long  current;
    char           vel;
    long           DeltaTime;
    unsigned char  LastEvent;
    char           *data;            /* Primary data */
};

static struct Track track[16];
uint32_t  track_buffersize = 65536UL;  /* 64 Ko */

static unsigned char MUS2MIDcontrol[15] =
{
        0,                  /* Program change - not a MIDI control change */
        0x00,               /* Bank select */
        0x01,               /* Modulation pot */
        0x07,               /* Volume */
        0x0A,               /* Pan pot */
        0x0B,               /* Expression pot */
        0x5B,               /* Reverb depth */
        0x5D,               /* Chorus depth */
        0x40,               /* Sustain pedal */
        0x43,               /* Soft pedal */
        0x78,               /* All sounds off */
        0x7B,               /* All notes off */
        0x7E,               /* Mono */
        0x7F,               /* Poly */
        0x79                /* Reset all controllers */
};

static byte    MUSchannel;
static byte    MIDItrack;

#define fwritemem(p,s,n,f)  memcpy(*f,p,n*s);*f+=(s*n)

#define fwrite16(x,f)    BE_write_16(f,x)
#define fwrite32(x,f)    BE_write_32(f,x)
// [WDJ] Proper big-endian midi read/write
static void BE_write_16(byte **p, int16_t val)
{
  *(int16_t *)*p = BE_SWAP16_FAST(val);
  *p += sizeof(int16_t);
}

static void BE_write_32(byte **p, int32_t val)
{
  *(int32_t *)*p = BE_SWAP32_FAST(val);
  *p += sizeof(int32_t);
}

#define last(e)         ((unsigned char)(e & 0x80))
#define event_type(e)   ((unsigned char)((e & 0x7F)>>4))
#define channel(e)      ((unsigned char)(e & 0x0F))


static void FreeTracks ( void )
{
    int i;

    for (i = 0; i < 16; i++ )
        if (track[i].data )
            free( track[i].data );
}


static void TWriteByte (byte MIDItrack, char bbyte)
{
    uint32_t pos;

    pos = track[MIDItrack].current;
    if (pos < track_buffersize )
        track[MIDItrack].data[pos] = bbyte;
    else
        I_Error("Mus Convert Error : Track buffer full.\n");

    track[MIDItrack].current++;
}

// write midi format, big endian
// value: all valid bytes have bit8 set except the last byte (LSB)
static void TWriteVarLen (byte tracknum, uint32_t value)
{
    uint32_t buffer;

    // Shift into buffer last (LSB), to first
    buffer = value & 0x7f;  // last (LSB) has bit8 clear
    while( (value >>= 7) )  // detect valid bytes
    {
        buffer <<= 8;
        buffer |= 0x80;  // not last byte
        buffer |= (value & 0x7f);
    }
    while( 1 )
    {
        TWriteByte( tracknum, (byte)buffer);
        if (buffer & 0x80 )  // detect last byte
            buffer >>= 8;
        else
            break;
    }
}

static int WriteMIDheader( uint16_t ntrks, uint16_t division, byte **file )
{
    fwritemem( MIDIHEADER, 10, 1, file );
    fwrite16( ntrks, file);
    fwrite16( division, file );
    return 0;
}



static void WriteTrack (int tracknum, byte **file)
{
    uint16_t trksize;
    size_t quot, rem;

    /* Do we risk overflow here ? */
    trksize = (uint16_t)track[tracknum].current + 4;
    fwritemem( "MTrk", 4, 1, file );  // track header
    if (!tracknum )
        trksize += 33;

    fwrite32( trksize, file );
    if (!tracknum)
    {
        memset(*file,'\0',33);
        *file+=33;
    }
    quot = (size_t) (track[tracknum].current / 4096);
    rem = (size_t) (track[tracknum].current - quot*4096);
    fwritemem (track[tracknum].data, 4096, quot, file );
    fwritemem (((byte *) track[tracknum].data)+4096*quot, rem, 1, file );
    fwritemem (MIDIEND, 4, 1, file );
}


static void WriteFirstTrack (byte **file)
{
    uint16_t trksize = 43;
    fwritemem( "MTrk", 4, 1, file );  // track header
    fwrite32( trksize, file );
    fwritemem( MIDICREATEPROG , 4, 1, file );
    memset(*file,'\0',22);  // create prog string
    *file+=22;
    fwritemem( MIDIKEY, 6, 1, file );
    fwritemem( MIDITEMPO, 7, 1, file );
    fwritemem( MIDIEND, 4, 1, file );
}

// Read a MUS time from file
static uint32_t ReadTime( byte **musfp )
{
    uint32_t timev = 0;
    int   bbyte;

    // MUS time is variable length, last byte has bit8=0
    do
    {
        bbyte = *(*musfp)++;
        if (bbyte != EOF )
            timev = (timev << 7) + (bbyte & 0x7F);
    } while( (bbyte != EOF) && (bbyte & 0x80) );

    return timev;
}

// return first MIDI channel available, except percussion channel 9
// fixed ch assign for percussion: MUS ch15 -> midi ch9
static char FirstChannelAvailable (char MUS2MIDchannel[])
{
    int i;
    signed char max = -1;  // so first is 0

    // note: skip channel 15 which is percussions
    for (i = 0; i < 15; i++ )
        if (MUS2MIDchannel[i] > max )  // find max of assigned ch
            max = MUS2MIDchannel[i];

    // MIDI channel 9 is used for percussions
    return (max == 8 ? 10 : max+1);
}

// write MIDI event code, with compression
static void MidiEvent(byte newevent)
{
    // compression does not repeat same event
    if ((newevent != track[MIDItrack].LastEvent) /*|| nocomp*/ )
    {
        TWriteByte( MIDItrack, newevent );
        track[MIDItrack].LastEvent = newevent;
    }
}

// Convert MUS to MIDI
// Return QMUS_error_code_e, 0 on success
int qmus2mid (byte *mus,  // input mus
              byte *mid,  // output buffer in memory
              uint16_t division, // ticks per quarter note
              int buffersize, // using track buffersize
              int nocomp,     // no compression, is ignored
              int muslength,  // input mus length
              int midbuffersize, // output buffer length
              unsigned long* midilength)    //faB: returns midi file length in here
{
  static   MUSheader* MUSh;

    byte*    file_mus = mus;
    byte*    file_mid = mid;     // pointer into output buffer
    byte*    musend = mus + muslength;

    byte     MIDIchannel;
    byte     et;
    uint16_t TrackCnt=0;
    int      i, event, data;
    uint32_t    DeltaTime;
    uint32_t    TotalTime=0;

    byte     MIDIchan2track[16];
    char     MUS2MIDchannel[16];

//    char ouch = 0;  // unused

    //r = ReadMUSheader (&MUSh, file_mus);
    MUSh = (MUSheader *)mus;
    if (strncmp (MUSh->ID, MUSHEADER, 4))
        return QM_NOTMUSFILE;

    //fseek( file_mus, MUSh.scoreStart, SEEK_SET );
    file_mus = mus + MUSh->scoreStart;

    if (MUSh->channels > 15)      /* <=> MUSchannels+drums > 16 */
        return QM_TOOMCHAN;

    for (i=0; i<16; i++)
    {
        MUS2MIDchannel[i] = -1;  // channel not used (yet)
        track[i].current = 0;
        track[i].vel = 64;
        track[i].DeltaTime = 0;
        track[i].LastEvent = 0;
        track[i].data = NULL;
    }

    if (buffersize)
        track_buffersize = ((uint32_t) buffersize) << 10;

    event = *(file_mus++);
    et    = event_type (event);
    MUSchannel = channel (event);

    while ( (et != MUS_EV_SCOREEND)
	    && file_mus < musend
             && (event != EOF) )
    {
        if (MUS2MIDchannel[MUSchannel] == -1)
        {
            MIDIchannel = MUS2MIDchannel[MUSchannel] =
                // if percussion use channel 9
                ((MUSchannel == 15) ? 9 : FirstChannelAvailable (MUS2MIDchannel) );
            MIDItrack = MIDIchan2track[MIDIchannel] = (byte)TrackCnt++;
            if (!(track[MIDItrack].data = (char *) malloc(track_buffersize) ))
            {
                FreeTracks();
                return QM_MEMALLOC;
            }
        }
        else
        {
            MIDIchannel = MUS2MIDchannel[MUSchannel];
            MIDItrack   = MIDIchan2track [MIDIchannel];
        }
        TWriteVarLen( MIDItrack, track[MIDItrack].DeltaTime );
        track[MIDItrack].DeltaTime = 0;
        switch (et)
        {
	 case 0 :  // release note
            MidiEvent((byte)0x90 | MIDIchannel);
            
            data = *(file_mus++);
            TWriteByte( MIDItrack, (byte)(data & 0x7F));
            TWriteByte( MIDItrack, 0 );
            break;
            
	 case 1 :  // play note
            MidiEvent((byte)0x90 | MIDIchannel);
            
            data = *(file_mus++);
            TWriteByte( MIDItrack, (byte)(data & 0x7F));
            if (data & 0x80)
                track[MIDItrack].vel = (*file_mus++) & 0x7F;
            TWriteByte( MIDItrack, track[MIDItrack].vel );
            break;
            
	 case 2 :  // bend note
            MidiEvent((byte)0xE0 | MIDIchannel);
            
            data = *(file_mus++);
            TWriteByte( MIDItrack, (data & 1) << 6 );
            TWriteByte( MIDItrack, data >> 1 );
            break;
	 case 3 :  // sys event
            MidiEvent((byte)0xB0 | MIDIchannel);
            
            data = *(file_mus++);
            TWriteByte( MIDItrack, MUS2MIDcontrol[data] );
            if (data == 12 )
                TWriteByte( MIDItrack, MUSh->channels+1 );
            else
                TWriteByte( MIDItrack, 0 );
            break;
	 case 4 :  // control change
            data = *(file_mus++);
            if (data )
            {
                MidiEvent((byte)0xB0 | MIDIchannel);
                
                TWriteByte( MIDItrack, MUS2MIDcontrol[data] );
            }
            else  /* program change */
                MidiEvent((byte)0xC0 | MIDIchannel);
            
            data = *(file_mus++);
            TWriteByte( MIDItrack, data & 0x7F );
            break;
	 case 5 :  // unknown
	 case 7 :  // unknown
            FreeTracks();
            return QM_MUSFILECOR;
	 case 6 :  // score end
	 default : break;
        }
        if (last( event ) )
        {
            DeltaTime = ReadTime( &file_mus );
            TotalTime += DeltaTime;
            for (i = 0; i < (int) TrackCnt; i++ )
                track[i].DeltaTime += DeltaTime;
        }
        event = *(file_mus++);
        if (event != EOF )
        {
            et = event_type( event );
            MUSchannel = channel( event );
        }
//        else
//            ouch = 1;
    }

    if (!division)
        division = 89;
        // prboom defaults to 70, all callers pass 64

#ifndef SMIF_SDL
    for (i = 0; i < 16; i++)
    {
        if (track[i].current && track[i].data)
        {
            TWriteByte(i, (byte)0x00); // midi end of track code
            TWriteByte(i, (byte)0xFF);
            TWriteByte(i, (byte)0x2F);
            TWriteByte(i, (byte)0x00);
        }
    }
#endif

#if 0
    if (et != MUS_EV_SCOREEND)
      GenPrintf(EMSG_warn,"QMUS end without score end\n");
#endif

    WriteMIDheader( TrackCnt+1, division, &file_mid );
    WriteFirstTrack( &file_mid );
    for (i = 0; i < (int) TrackCnt; i++ )
        WriteTrack( i, &file_mid );

    if (file_mid>mid+midbuffersize)
        return QM_MIDTOOLARGE;

    FreeTracks();

    //faB: return length of Midi data
    *midilength = (file_mid - mid);

    return 0;
}


#endif // __OS2__
