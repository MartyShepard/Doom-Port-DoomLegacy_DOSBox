// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: p_lights.c 1351 2017-07-29 18:28:41Z wesleyjohnson $
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
// $Log: p_lights.c,v $
// Revision 1.5  2000/11/02 17:50:07  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.4  2000/04/11 19:07:24  stroggonmeth
// Finished my logs, fixed a crashing bug.
//
// Revision 1.3  2000/04/04 00:32:46  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Handle Sector base lighting effects.
//      Muzzle flash?
//
//-----------------------------------------------------------------------------


#include "doomincl.h"
#include "doomstat.h"
#include "p_local.h"
#include "p_tick.h"
  // think
#include "r_state.h"
#include "z_zone.h"
#include "m_random.h"


// =========================================================================
//                           FIRELIGHT FLICKER
// =========================================================================

//
// T_FireFlicker
//
void T_FireFlicker (fireflicker_t* flick)
{
    int amount;

    if (--flick->count)
        return;

    amount = (PP_Random(pr_lights)&3)*16;

    if (flick->sector->lightlevel - amount < flick->minlight)
        flick->sector->lightlevel = flick->minlight;
    else
        flick->sector->lightlevel = flick->maxlight - amount;

    flick->count = 4;
}



//
// P_SpawnFireFlicker
//
void P_SpawnFireFlicker (sector_t*  sector)
{
    fireflicker_t*      flick;

    // Note that we are resetting sector attributes.
    // Nothing special about it during gameplay.
    sector->special &= ~31; //SoM: Clear non-generalized sector type

    flick = Z_Malloc ( sizeof(*flick), PU_LEVSPEC, 0);
    flick->thinker.function.acp1 = (actionf_p1) T_FireFlicker;

    flick->sector = sector;
    flick->maxlight = sector->lightlevel;
    flick->minlight = P_FindMinSurroundingLight(sector,sector->lightlevel)+16;
    flick->count = 4;

    P_AddThinker (&flick->thinker);
}



//
// BROKEN LIGHT FLASHING
//


//
// T_LightFlash
// Do flashing lights.
//
void T_LightFlash (lightflash_t* flash)
{
    if (--flash->count)
        return;

    if (flash->sector->lightlevel == flash->maxlight)
    {
        flash-> sector->lightlevel = flash->minlight;
        flash->count = (PP_Random(pr_lights)&flash->mintime)+1;
    }
    else
    {
        flash-> sector->lightlevel = flash->maxlight;
        flash->count = (PP_Random(pr_lights)&flash->maxtime)+1;
    }

}




//
// P_SpawnLightFlash
// After the map has been loaded, scan each sector
// for specials that spawn thinkers
//
void P_SpawnLightFlash (sector_t* sector)
{
    lightflash_t*       flash;

    // nothing special about it during gameplay
    sector->special &= ~31; //SoM: 3/7/2000: Clear non-generalized type

    flash = Z_Malloc ( sizeof(*flash), PU_LEVSPEC, 0);
    flash->thinker.function.acp1 = (actionf_p1) T_LightFlash;

    flash->sector = sector;
    flash->maxlight = sector->lightlevel;

    flash->minlight = P_FindMinSurroundingLight(sector,sector->lightlevel);
    flash->maxtime = 64;
    flash->mintime = 7;
    flash->count = (PP_Random(pr_lights)&flash->maxtime)+1;

    P_AddThinker (&flash->thinker);
}



//
// STROBE LIGHT FLASHING
//


//
// T_StrobeFlash
//
void T_StrobeFlash (strobe_t*  flash)
{
    if (--flash->count)
        return;

    if (flash->sector->lightlevel == flash->minlight)
    {
        flash-> sector->lightlevel = flash->maxlight;
        flash->count = flash->brighttime;
    }
    else
    {
        flash-> sector->lightlevel = flash->minlight;
        flash->count =flash->darktime;
    }

}



//
// P_SpawnStrobeFlash
// After the map has been loaded, scan each sector
// for specials that spawn thinkers
//
void
P_SpawnStrobeFlash( sector_t* sector,
                    int fastOrSlow, int inSync )
{
    strobe_t*   flash;

    flash = Z_Malloc ( sizeof(*flash), PU_LEVSPEC, 0);
    flash->thinker.function.acp1 = (actionf_p1) T_StrobeFlash;

    flash->sector = sector;
    flash->darktime = fastOrSlow;
    flash->brighttime = STROBEBRIGHT;
    flash->maxlight = sector->lightlevel;
    flash->minlight = P_FindMinSurroundingLight(sector, sector->lightlevel);

    if (flash->minlight == flash->maxlight)
        flash->minlight = 0;

    // nothing special about it during gameplay
    sector->special &= ~31; //SoM: 3/7/2000: Clear non-generalized sector type

    if (!inSync)
        flash->count = (PP_Random(pr_lights)&7)+1;
    else
        flash->count = 1;

    P_AddThinker (&flash->thinker);
}


//
// Start strobing lights (usually from a trigger)
//
int EV_StartLightStrobing(line_t* line)
{
    sector_t*   sec;

    int secnum = -1; // init search FindSector
    while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
    {
        sec = &sectors[secnum];
        if (P_SectorActive( S_lighting_special, sec)) //SoM: 3/7/2000: New way to check thinker
          continue;

        P_SpawnStrobeFlash (sec,SLOWDARK, 0);
    }
    return 1;
}



//
// TURN LINE'S TAG LIGHTS OFF
//
int EV_TurnTagLightsOff(line_t* line)
{
    int                 i;
    int                 j;
    int                 min;
    sector_t*           sector;
    sector_t*           tsec;
    line_t*             templine;

    sector = sectors;

    for (j = 0; j < numsectors; j++, sector++)
    {
        // for each sector
        if (sector->tag == line->tag)
        {
            // for each sector with matching tag
            min = sector->lightlevel;
            for (i = 0; i < sector->linecount; i++)
            {
                // for all lines in sector linelist
                templine = sector->linelist[i];
                tsec = getNextSector(templine,sector);
                if (!tsec)
                    continue;
                // find any lower light level
                if (tsec->lightlevel < min)
                    min = tsec->lightlevel;
            }
            sector->lightlevel = min;
        }
    }
    return 1;
}


//
// TURN LINE'S TAG LIGHTS ON
// Turn tagged sectors to specified or max neighbor level.
//
//  bright: light level,  0= use max neighbor light level
//  return 1
int EV_LightTurnOn ( line_t* line, lightlev_t bright )
{
    int         fsecn, j;
    lightlev_t  sll;  // set or max light level
    sector_t*   sector;
    sector_t*   adjsec;
    line_t*     adjline;

    fsecn = -1;
    while ((fsecn = P_FindSectorFromLineTag(line, fsecn)) >= 0)
    {
        // For each sector with matching Tag
        sll = bright; //SoM: 3/7/2000: Search for maximum per sector
        sector = &sectors[fsecn];
       
        // For each sector with matching tag
        if( bright == 0 )
        {
            // Find max adjacent light.
            for (j = 0; j < sector->linecount; j++)
            {
                // for each line in sector linelist
                adjline = sector->linelist[j];
                adjsec = getNextSector(adjline,sector);

                if( !adjsec )
                    continue;

                // find any brighter light level
                if( sll < adjsec->lightlevel ) //SoM: 3/7/2000
                    sll = adjsec->lightlevel;
            }
        }
        sector->lightlevel = sll;

        if( !EN_boom_physics )  // old behavior
            bright = sll;  // maximums are not independent
    }
    return 1;
}


//
// Spawn glowing light
//

void T_Glow( glow_t* gp)
{
    switch(gp->direction)
    {
      case -1:
        // DOWN
        gp->sector->lightlevel -= GLOWSPEED;
        if (gp->sector->lightlevel <= gp->minlight)
        {
            gp->sector->lightlevel += GLOWSPEED;
            gp->direction = 1;
        }
        break;

      case 1:
        // UP
        gp->sector->lightlevel += GLOWSPEED;
        if (gp->sector->lightlevel >= gp->maxlight)
        {
            gp->sector->lightlevel -= GLOWSPEED;
            gp->direction = -1;
        }
        break;
    }
}


void P_SpawnGlowingLight( sector_t*  sector)
{
    glow_t* gp;

    gp = Z_Malloc( sizeof(*gp), PU_LEVSPEC, 0);
    gp->thinker.function.acp1 = (actionf_p1) T_Glow;

    gp->sector = sector;
    gp->minlight = P_FindMinSurroundingLight(sector,sector->lightlevel);
    gp->maxlight = sector->lightlevel;
    gp->direction = -1;

    sector->special &= ~0x1F; //SoM: 3/7/2000: Reset only non-generic types.

    P_AddThinker(& gp->thinker);
}



// P_FadeLight()
//
// Fade all the lights in sectors with a particular tag to a new value
//
void P_FadeLight( uint16_t tag, lightlev_t destvalue, lightlev_t speed)
{
  lightfader_t * lf;

  // search all sectors for ones with tag
  int secnum = -1; // init search FindSector
  while ((secnum = P_FindSectorFromTag(tag,secnum)) >= 0)
  {
      sector_t *sector = &sectors[secnum];
      sector->lightingdata = sector;    // just set it to something

      lf = Z_Malloc(sizeof(*lf), PU_LEVSPEC, 0);
      lf->thinker.function.acp1 = (actionf_p1)T_LightFade;

      P_AddThinker(&lf->thinker);       // add thinker

      lf->sector = sector;
      lf->destlight = destvalue;
      lf->speed = speed;
  }
}



// T_LightFade()
//
// Just fade the light level in a sector to a new level
//

void T_LightFade(lightfader_t * lf)
{
  lightlev_t seclight = lf->sector->lightlevel;
   
  if(seclight < lf->destlight)
  {
    // increase the lightlevel
    seclight += lf->speed; // move lightlevel
    if(seclight >= lf->destlight)
      goto achieved_target;
  }
  else
  {
    // decrease lightlevel
    seclight -= lf->speed; // move lightlevel
    if(seclight <= lf->destlight)
      goto achieved_target;
  }
  lf->sector->lightlevel = seclight;
  return;
   
achieved_target:
  // stop changing light level
  lf->sector->lightlevel = lf->destlight;    // set to dest lightlevel

  lf->sector->lightingdata = NULL;          // clear lightingdata
  P_RemoveThinker(&lf->thinker);            // remove thinker       
}


// [WDJ] From MBF, PrBoom
// killough 10/98:
//
// Used for doors with gradual lighting effects.
// Turn light in tagged sectors to specified or max neighbor level.
//
//   line :  has sector TAG
//   level : light level fraction, 0=min, 1=max, else interpolate min..max
// Returns true
int EV_LightTurnOnPartway(line_t *line, fixed_t level)
{
  sector_t * sector;
  int i, j;
  int minll, maxll;

  if (level < 0)          // clip at extremes
    level = 0;
  if (level > FRACUNIT)
    level = FRACUNIT;

  // search all sectors for ones with same tag as activating line
  i = -1;
  while ( (i = P_FindSectorFromLineTag(line,i)) >= 0 )
  {
      sector = &sectors[i];
      maxll = 0;
      minll = sector->lightlevel;

      for (j = 0; j < sector->linecount; j++)
      {
          sector_t * adjsec = getNextSector(sector->linelist[j], sector);
          if(adjsec)
          {
              if( maxll < adjsec->lightlevel )
                  maxll = adjsec->lightlevel;
              if( minll > adjsec->lightlevel )
                  minll = adjsec->lightlevel;
          }
      }

      // Set level in-between extremes
      sector->lightlevel =
        (level * maxll + (FRACUNIT-level) * minll) >> FRACBITS;
  }
  return 1;
}

