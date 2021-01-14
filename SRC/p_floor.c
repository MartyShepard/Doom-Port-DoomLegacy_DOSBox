// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: p_floor.c 659 2010-05-21 15:39:28Z wesleyjohnson $
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2010 by DooM Legacy Team.
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
// $Log: p_floor.c,v $
// Revision 1.13  2002/06/14 02:43:43  ssntails
// Instant-lower and instant-raise capability for sectors added.
//
// Revision 1.12  2001/03/30 17:12:50  bpereira
// no message
//
// Revision 1.11  2001/01/25 22:15:43  bpereira
// added heretic support
//
// Revision 1.10  2000/11/02 17:50:07  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.9  2000/10/21 08:43:30  bpereira
// no message
//
// Revision 1.8  2000/09/28 20:57:16  bpereira
// no message
//
// Revision 1.7  2000/07/01 09:23:49  bpereira
// no message
//
// Revision 1.6  2000/05/23 15:22:34  stroggonmeth
// Not much. A graphic bug fixed.
//
// Revision 1.5  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.4  2000/04/08 17:29:24  stroggonmeth
// no message
//
// Revision 1.3  2000/04/04 00:32:46  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Floor animation: raising stairs.
//
//-----------------------------------------------------------------------------


#include "doomdef.h"
#include "doomstat.h"
#include "p_local.h"
#include "r_state.h"
#include "s_sound.h"
#include "z_zone.h"


// ==========================================================================
//                              FLOORS
// ==========================================================================

//
// Move a plane (floor or ceiling) and check for crushing
//
//SoM: I had to copy the entire function from Boom because it was causing errors.
result_e T_MovePlane ( sector_t*     sector,
		       fixed_t       speed,
		       fixed_t       dest,
		       boolean       crush, // enables crushing damage
		       int           floorOrCeiling,
		       int           direction )
{
  boolean       flag;
  fixed_t       lastpos;     
  fixed_t       destheight; //jff 02/04/98 used to keep floors/ceilings
                            // from moving thru each other

  switch(floorOrCeiling)
  {
    case 0:
      // Moving a floor
      switch(direction)
      {
        case -1:
          //SoM: 3/20/2000: Make splash when platform floor hits water
//          if(boomsupport && sector->modelsec != -1 && sector->model == SM_Legacy_water)
          if((sector->model == SM_Legacy_water) && boomsupport)
          {
            if(((sector->floorheight - speed) < sectors[sector->modelsec].floorheight )
               && (sector->floorheight > sectors[sector->modelsec].floorheight))
              S_StartSound((mobj_t *)&sector->soundorg, sfx_gloop);
          }
          // Moving a floor down
          if (sector->floorheight - speed < dest)
          {
            lastpos = sector->floorheight;
            sector->floorheight = dest;
            flag = P_CheckSector(sector,crush);
            if (flag == true && sector->numattached)                   
            {
              sector->floorheight =lastpos;
              P_CheckSector(sector,crush);
            }
            return pastdest;
          }
          else
          {
            lastpos = sector->floorheight;
            sector->floorheight -= speed;
            flag = P_CheckSector(sector,crush);
            if(flag == true && sector->numattached)
            {
              sector->floorheight = lastpos;
              P_CheckSector(sector, crush);
              return crushed;
            }
          }
          break;
                                                
        case 1:
          // Moving a floor up
          // keep floor from moving thru ceilings
          //SoM: 3/20/2000: Make splash when platform floor hits water
//          if(boomsupport && sector->modelsec != -1 && sector->model == SM_Legacy_water)
          if((sector->model == SM_Legacy_water) && boomsupport)
          {
            if(((sector->floorheight + speed) > sectors[sector->modelsec].floorheight)
               && (sector->floorheight < sectors[sector->modelsec].floorheight))
              S_StartSound((mobj_t *)&sector->soundorg, sfx_gloop);
          }
//          destheight = (!boomsupport || dest<sector->ceilingheight)?
//                          dest : sector->ceilingheight;
          destheight = (boomsupport && (dest > sector->ceilingheight))?
		sector->ceilingheight : dest;
          if (sector->floorheight + speed > destheight)
          {
            lastpos = sector->floorheight;
            sector->floorheight = destheight;
            flag = P_CheckSector(sector,crush);
            if (flag == true)
            {
              sector->floorheight = lastpos;
              P_CheckSector(sector,crush);
            }
            return pastdest;
          }
          else
          {
            // crushing is possible
            lastpos = sector->floorheight;
            sector->floorheight += speed;
            flag = P_CheckSector(sector,crush);
            if (flag == true)
            {
              if (!boomsupport)
              {
                if (crush == true)
                  return crushed;
              }
              sector->floorheight = lastpos;
              P_CheckSector(sector,crush);
              return crushed;
            }
          }
          break;
      }
      break;
                                                                        
    case 1:
      // moving a ceiling
      switch(direction)
      {
        case -1:
          if((sector->model == SM_Legacy_water) && boomsupport)
          {
	    // make sound when ceiling hits water
            if(((sector->ceilingheight - speed) < sectors[sector->modelsec].floorheight)
               && (sector->ceilingheight > sectors[sector->modelsec].floorheight))
              S_StartSound((mobj_t *)&sector->soundorg, sfx_gloop);
          }
          // moving a ceiling down
          // keep ceiling from moving thru floors
//          destheight = (!boomsupport || dest>sector->floorheight)?
//                          dest : sector->floorheight;
          destheight = (boomsupport && (dest<sector->floorheight))?
		sector->floorheight : dest;
          if (sector->ceilingheight - speed < destheight)
          {
            lastpos = sector->ceilingheight;
            sector->ceilingheight = destheight;
            flag = P_CheckSector(sector,crush);

            if (flag == true)
            {
              sector->ceilingheight = lastpos;
              P_CheckSector(sector,crush);
            }
            return pastdest;
          }
          else
          {
            // crushing is possible
            lastpos = sector->ceilingheight;
            sector->ceilingheight -= speed;
            flag = P_CheckSector(sector,crush);

            if (flag == true)
            {
              if (crush == true)
                return crushed;
              sector->ceilingheight = lastpos;
              P_CheckSector(sector,crush);
              return crushed;
            }
          }
          break;
                                                
        case 1:
//          if(boomsupport && sector->modelsec != -1 && sector->model == SM_Legacy_water)
          if((sector->model == SM_Legacy_water) && boomsupport)
          {
	    // make sound when ceiling hits water
            if(((sector->ceilingheight + speed) > sectors[sector->modelsec].floorheight)
               && (sector->ceilingheight < sectors[sector->modelsec].floorheight))
              S_StartSound((mobj_t *)&sector->soundorg, sfx_gloop);
          }
          // moving a ceiling up
          if (sector->ceilingheight + speed > dest)
          {
            lastpos = sector->ceilingheight;
            sector->ceilingheight = dest;
            flag = P_CheckSector(sector,crush);
            if (flag == true && sector->numattached)
            {
              sector->ceilingheight = lastpos;
              P_CheckSector(sector,crush);
            }
            return pastdest;
          }
          else
          {
            lastpos = sector->ceilingheight;
            sector->ceilingheight += speed;
            flag = P_CheckSector(sector,crush);
            if (flag == true && sector->numattached)
            {
              sector->ceilingheight = lastpos;
              P_CheckSector(sector,crush);
              return crushed;
            }
          }
          break;
      }
      break;
    }
    return ok;
}


//
// MOVE A FLOOR TO IT'S DESTINATION (UP OR DOWN)
//
void T_MoveFloor(floormove_t* mfloor)
{
    result_e    res = 0;

    res = T_MovePlane(mfloor->sector,
                      mfloor->speed,
                      mfloor->floordestheight,
                      mfloor->crush,0,mfloor->direction);

    if (!(leveltime % (8*NEWTICRATERATIO)))
        S_StartSound((mobj_t *)&mfloor->sector->soundorg,
                     ceilmovesound);

    if (res == pastdest)
    {
        //mfloor->sector->specialdata = NULL;
        if (mfloor->direction == 1)
        {
            switch(mfloor->type)
            {
              case donutRaise:
                mfloor->sector->special = mfloor->newspecial;
                mfloor->sector->floorpic = mfloor->texture;
                break;
              case genFloorChgT: //SoM: 3/6/2000: Add support for General types
              case genFloorChg0:
                mfloor->sector->special = mfloor->newspecial;
                //SoM: 3/6/2000: this records the old special of the sector
                mfloor->sector->oldspecial = mfloor->oldspecial;
                // Don't break.
              case genFloorChg:
                mfloor->sector->floorpic = mfloor->texture;
                break;
              default:
                break;
            }
        }
        else if (mfloor->direction == -1)
        {
            switch(mfloor->type)
            {
              case lowerAndChange:
                mfloor->sector->special = mfloor->newspecial;
                // SoM: 3/6/2000: Store old special type
                mfloor->sector->oldspecial = mfloor->oldspecial;
                mfloor->sector->floorpic = mfloor->texture;
                break;
              case genFloorChgT:
              case genFloorChg0:
                mfloor->sector->special = mfloor->newspecial;
                mfloor->sector->oldspecial = mfloor->oldspecial;
                // Don't break
              case genFloorChg:
                mfloor->sector->floorpic = mfloor->texture;
                break;
              default:
                break;
            }
        }

        mfloor->sector->floordata = NULL; // Clear up the thinker so others can use it
        P_RemoveThinker(&mfloor->thinker);

        // SoM: This code locks out stair steps while generic, retriggerable generic stairs
        // are building.
      
        if (mfloor->sector->stairlock==-2) // if this sector is stairlocked
        {
          sector_t *sec = mfloor->sector;
          sec->stairlock=-1;              // thinker done, promote lock to -1

          while (sec->prevsec>=0 && sectors[sec->prevsec].stairlock!=-2)
            sec = &sectors[sec->prevsec]; // search for a non-done thinker
          if (sec->prevsec==-1)           // if all thinkers previous are done
          {
            sec = mfloor->sector;          // search forward
            while (sec->nextsec>=0 && sectors[sec->nextsec].stairlock!=-2) 
              sec = &sectors[sec->nextsec];
            if (sec->nextsec==-1)         // if all thinkers ahead are done too
            {
              while (sec->prevsec>=0)    // clear all locks
              {
                sec->stairlock = 0;
                sec = &sectors[sec->prevsec];
              }
              sec->stairlock = 0;
            }
          }
        }

        if ((mfloor->type == buildStair && gamemode == heretic) || 
            gamemode != heretic)
            S_StartSound((mobj_t *)&mfloor->sector->soundorg, sfx_pstop);
    }

}


// SoM: 3/6/2000: Lots'o'copied code here.. Elevators.
//
// T_MoveElevator()
//
// Move an elevator to it's destination (up or down)
// Called once per tick for each moving floor.
//
// Passed an elevator_t structure that contains all pertinent info about the
// move. See P_SPEC.H for fields.
// No return.
//
// SoM: 3/6/2000: The function moves the plane differently based on direction, so if it's 
// traveling really fast, the floor and ceiling won't hit each other and stop the lift.
void T_MoveElevator(elevator_t* elevator)
{
  result_e      res = 0;

  if (elevator->direction<0)      // moving down
  {
    res = T_MovePlane             //jff 4/7/98 reverse order of ceiling/floor
    (
      elevator->sector,
      elevator->speed,
      elevator->ceilingdestheight,
      0,
      1,                          // move floor
      elevator->direction
    );
    if (res==ok || res==pastdest) // jff 4/7/98 don't move ceil if blocked
      T_MovePlane
      (
        elevator->sector,
        elevator->speed,
        elevator->floordestheight,
        0,
        0,                        // move ceiling
        elevator->direction
      );
  }
  else // up
  {
    res = T_MovePlane             //jff 4/7/98 reverse order of ceiling/floor
    (
      elevator->sector,
      elevator->speed,
      elevator->floordestheight,
      0,
      0,                          // move ceiling
      elevator->direction
    );
    if (res==ok || res==pastdest) // jff 4/7/98 don't move floor if blocked
      T_MovePlane
      (
        elevator->sector,
        elevator->speed,
        elevator->ceilingdestheight,
        0,
        1,                        // move floor
        elevator->direction
      );
  }

  // make floor move sound
  if (!(leveltime % (8*NEWTICRATERATIO)))
    S_StartSound((mobj_t *)&elevator->sector->soundorg, sfx_stnmov);
    
  if (res == pastdest)            // if destination height acheived
  {
    elevator->sector->floordata = NULL;     //jff 2/22/98
    elevator->sector->ceilingdata = NULL;   //jff 2/22/98
    P_RemoveThinker(&elevator->thinker);    // remove elevator from actives

    // make floor stop sound
    S_StartSound((mobj_t *)&elevator->sector->soundorg, sfx_pstop);
  }
}




//
// HANDLE FLOOR TYPES
//
int EV_DoFloor ( line_t* line, floor_e floortype )
{
    sector_t*           sec;
    floormove_t*        mfloor;
    int                 rtn = 0;
    int                 i;

    int secnum = -1;  // init search FindSector
    while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
    {
        sec = &sectors[secnum];
        
        // SoM: 3/6/2000: Boom has multiple thinkers per sector.
        // Don't start a second thinker on the same floor
        if (P_SectorActive(floor_special,sec)) //jff 2/23/98
          continue;

        // new floor thinker
        rtn = 1;
        mfloor = Z_Malloc (sizeof(*mfloor), PU_LEVSPEC, 0);
        P_AddThinker (&mfloor->thinker);
        sec->floordata = mfloor; //SoM: 2/5/2000
        mfloor->thinker.function.acp1 = (actionf_p1) T_MoveFloor;
        mfloor->type = floortype;
        mfloor->crush = false;

        switch(floortype)
        {
          case lowerFloor:
            mfloor->direction = -1;
            mfloor->sector = sec;
            mfloor->speed = FLOORSPEED;
            mfloor->floordestheight = P_FindHighestFloorSurrounding(sec);
            break;

            //jff 02/03/30 support lowering floor by 24 absolute
          case lowerFloor24:
            mfloor->direction = -1;
            mfloor->sector = sec;
            mfloor->speed = FLOORSPEED;
            mfloor->floordestheight = mfloor->sector->floorheight + 24 * FRACUNIT;
            break;

            //jff 02/03/30 support lowering floor by 32 absolute (fast)
          case lowerFloor32Turbo:
            mfloor->direction = -1;
            mfloor->sector = sec;
            mfloor->speed = FLOORSPEED*4;
            mfloor->floordestheight = mfloor->sector->floorheight + 32 * FRACUNIT;
            break;

          case lowerFloorToLowest:
            mfloor->direction = -1;
            mfloor->sector = sec;
            mfloor->speed = FLOORSPEED;
            mfloor->floordestheight = P_FindLowestFloorSurrounding(sec);
            break;

            //jff 02/03/30 support lowering floor to next lowest floor
          case lowerFloorToNearest:
            mfloor->direction = -1;
            mfloor->sector = sec;
            mfloor->speed = FLOORSPEED;
            mfloor->floordestheight =
              P_FindNextLowestFloor(sec,mfloor->sector->floorheight);
            break;

          case turboLower:
            mfloor->direction = -1;
            mfloor->sector = sec;
            mfloor->speed = FLOORSPEED * 4;
            mfloor->floordestheight = P_FindHighestFloorSurrounding(sec);
            if (mfloor->floordestheight != sec->floorheight || gamemode == heretic )
                mfloor->floordestheight += 8*FRACUNIT;
            break;

          case raiseFloorCrush:
            mfloor->crush = true;
          case raiseFloor:
            mfloor->direction = 1;
            mfloor->sector = sec;
            mfloor->speed = FLOORSPEED;
            mfloor->floordestheight = P_FindLowestCeilingSurrounding(sec);
            if (mfloor->floordestheight > sec->ceilingheight)
                mfloor->floordestheight = sec->ceilingheight;
            mfloor->floordestheight -= (8*FRACUNIT)* (floortype == raiseFloorCrush);
            break;

          case raiseFloorTurbo:
            mfloor->direction = 1;
            mfloor->sector = sec;
            mfloor->speed = FLOORSPEED*4;
            mfloor->floordestheight = P_FindNextHighestFloor(sec,sec->floorheight);
            break;

          case raiseFloorToNearest:
            mfloor->direction = 1;
            mfloor->sector = sec;
            mfloor->speed = FLOORSPEED;
            mfloor->floordestheight = P_FindNextHighestFloor(sec,sec->floorheight);
            break;

          case raiseFloor24:
            mfloor->direction = 1;
            mfloor->sector = sec;
            mfloor->speed = FLOORSPEED;
            mfloor->floordestheight = mfloor->sector->floorheight + 24 * FRACUNIT;
            break;

          // SoM: 3/6/2000: support straight raise by 32 (fast)
          case raiseFloor32Turbo:
            mfloor->direction = 1;
            mfloor->sector = sec;
            mfloor->speed = FLOORSPEED*4;
            mfloor->floordestheight = mfloor->sector->floorheight + 32 * FRACUNIT;
            break;

          case raiseFloor512:
            mfloor->direction = 1;
            mfloor->sector = sec;
            mfloor->speed = FLOORSPEED;
            mfloor->floordestheight = mfloor->sector->floorheight + 512 * FRACUNIT;
            break;

          case raiseFloor24AndChange:
            mfloor->direction = 1;
            mfloor->sector = sec;
            mfloor->speed = FLOORSPEED;
            mfloor->floordestheight = mfloor->sector->floorheight + 24 * FRACUNIT;
            sec->floorpic = line->frontsector->floorpic;
            sec->special = line->frontsector->special;
            sec->oldspecial = line->frontsector->oldspecial;
            break;

          case raiseToTexture:
          {
              int       minsize = MAXINT;
              side_t*   side;

              if (boomsupport) minsize = 32000<<FRACBITS; //SoM: 3/6/2000: ???
              mfloor->direction = 1;
              mfloor->sector = sec;
              mfloor->speed = FLOORSPEED;
              for (i = 0; i < sec->linecount; i++)
              {
                if (twoSided (secnum, i) )
                {
                  side = getSide(secnum,i,0);
                  // jff 8/14/98 don't scan texture 0, its not real
                  if (side->bottomtexture > 0 ||
                      (!boomsupport && !side->bottomtexture))
		  {
                    if (textureheight[side->bottomtexture] < minsize)
                      minsize = textureheight[side->bottomtexture];
		  }
                  side = getSide(secnum,i,1);
                  // jff 8/14/98 don't scan texture 0, its not real
                  if (side->bottomtexture > 0 ||
                      (!boomsupport && !side->bottomtexture))
		  {
                    if (textureheight[side->bottomtexture] < minsize)
                      minsize = textureheight[side->bottomtexture];
		  }
                }
              }
              if (!boomsupport)
                mfloor->floordestheight = mfloor->sector->floorheight + minsize;
              else
              {
                mfloor->floordestheight =
                  (mfloor->sector->floorheight>>FRACBITS) + (minsize>>FRACBITS);
                if (mfloor->floordestheight>32000)
                  mfloor->floordestheight = 32000;        //jff 3/13/98 do not
                mfloor->floordestheight<<=FRACBITS;       // allow height overflow
              }
            break;
          }
          //SoM: 3/6/2000: Boom changed allot of stuff I guess, and this was one of 'em 
          case lowerAndChange:
            mfloor->direction = -1;
            mfloor->sector = sec;
            mfloor->speed = FLOORSPEED;
            mfloor->floordestheight = P_FindLowestFloorSurrounding(sec);
            mfloor->texture = sec->floorpic;

            // jff 1/24/98 make sure floor->newspecial gets initialized
            // in case no surrounding sector is at floordestheight
            // --> should not affect compatibility <--
            mfloor->newspecial = sec->special; 
            //jff 3/14/98 transfer both old and new special
            mfloor->oldspecial = sec->oldspecial;
    
            //jff 5/23/98 use model subroutine to unify fixes and handling
            // BP: heretic have change something here
            sec = P_FindModelFloorSector(mfloor->floordestheight,sec-sectors);
            if (sec)
            {
              mfloor->texture = sec->floorpic;
              mfloor->newspecial = sec->special;
              //jff 3/14/98 transfer both old and new special
              mfloor->oldspecial = sec->oldspecial;
            }
            break;
		  // Instant Lower SSNTails 06-13-2002
          case instantLower:
            mfloor->direction = -1;
            mfloor->sector = sec;
            mfloor->speed = MAXINT/2; // Go too fast and you'll cause problems...
            mfloor->floordestheight =
            P_FindLowestFloorSurrounding(sec);
            break;
          default:
            break;
        }
    }
    return rtn;
}


// SoM: 3/6/2000: Function for changing just the floor texture and type.
//
// EV_DoChange()
//
// Handle pure change types. These change floor texture and sector type
// by trigger or numeric model without moving the floor.
//
// The linedef causing the change and the type of change is passed
// Returns true if any sector changes
//
//
int EV_DoChange ( line_t* line, change_e changetype )
{
  sector_t*             sec;
  sector_t*             secm;
  int                   rtn = 0;

  int secnum = -1; // init search FindSector
  // change all sectors with the same tag as the linedef
  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    sec = &sectors[secnum];
              
    rtn = 1;

    // handle trigger or numeric change type
    switch(changetype)
    {
      case trigChangeOnly:
        sec->floorpic = line->frontsector->floorpic;
        sec->special = line->frontsector->special;
        sec->oldspecial = line->frontsector->oldspecial;
        break;
      case numChangeOnly:
        secm = P_FindModelFloorSector(sec->floorheight,secnum);
        if (secm) // if no model, no change
        {
          sec->floorpic = secm->floorpic;
          sec->special = secm->special;
          sec->oldspecial = secm->oldspecial;
        }
        break;
      default:
        break;
    }
  }
  return rtn;
}




//
// BUILD A STAIRCASE!
//

// SoM: 3/6/2000: Use the Boom version of this function.
int EV_BuildStairs ( line_t*  line, stair_e type )
{
  int                   height;
  int                   texture;
  int                   ok;
  int                   rtn = 0;
  int                   secnum, new_secnum, old_secnum;
  int                   i;
    
  sector_t*             sec;
  sector_t*             tsec;

  floormove_t*		mfloor;
    
  fixed_t               stairsize;
  fixed_t               speed;

  secnum = -1; // init search FindSector
  // start a stair at each sector tagged the same as the linedef
  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    sec = &sectors[secnum];
              
    // don't start a stair if the first step's floor is already moving
    if (P_SectorActive(floor_special,sec))
      continue;
      
    // create new floor thinker for first step
    rtn = 1;
    mfloor = Z_Malloc (sizeof(*mfloor), PU_LEVSPEC, 0);
    P_AddThinker (&mfloor->thinker);
    sec->floordata = mfloor;
    mfloor->thinker.function.acp1 = (actionf_p1) T_MoveFloor;
    mfloor->direction = 1;
    mfloor->sector = sec;
    mfloor->type = buildStair;   //jff 3/31/98 do not leave uninited

    // set up the speed and stepsize according to the stairs type
    switch(type)
    {
      case build8:
        speed = FLOORSPEED/4;
        stairsize = 8*FRACUNIT;
        if (boomsupport)
          mfloor->crush = false; //jff 2/27/98 fix uninitialized crush field
        break;
      case turbo16:
        speed = FLOORSPEED*4;
        stairsize = 16*FRACUNIT;
        if (boomsupport)
          mfloor->crush = true;  //jff 2/27/98 fix uninitialized crush field
        break;
      // used by heretic
      default:
        speed = FLOORSPEED;
        stairsize = type;
        if (boomsupport)
          mfloor->crush = true;  //jff 2/27/98 fix uninitialized crush field
        break;
    }
    mfloor->speed = speed;
    height = sec->floorheight + stairsize;
    mfloor->floordestheight = height;
              
    texture = sec->floorpic;
    old_secnum = secnum;           //jff 3/4/98 preserve loop index
      
    // Find next sector to raise
    //   1. Find 2-sided line with same sector side[0] (lowest numbered)
    //   2. Other side is the next sector to raise
    //   3. Unless already moving, or different texture, then stop building
    do
    {
      ok = 0;
      for (i = 0; i < sec->linecount; i++)
      {
	// for each line of the sector linelist
        if ( !((sec->linelist[i])->flags & ML_TWOSIDED) )
          continue;
                                  
        tsec = (sec->linelist[i])->frontsector;
        new_secnum = tsec-sectors;
          
        if (secnum != new_secnum)
          continue;

        tsec = (sec->linelist[i])->backsector;
        if (!tsec) continue;     //jff 5/7/98 if no backside, continue
        new_secnum = tsec - sectors;

        // if sector's floor is different texture, look for another
        if (tsec->floorpic != texture)
          continue;

        if (!boomsupport) // jff 6/19/98 prevent double stepsize
          height += stairsize; // jff 6/28/98 change demo compatibility

        // if sector's floor already moving, look for another
        if (P_SectorActive(floor_special,tsec)) //jff 2/22/98
          continue;
                                  
        if (boomsupport) // jff 6/19/98 increase height AFTER continue
          height += stairsize; // jff 6/28/98 change demo compatibility

        sec = tsec;
        secnum = new_secnum;

        // create and initialize a thinker for the next step
        mfloor = Z_Malloc (sizeof(*mfloor), PU_LEVSPEC, 0);
        P_AddThinker (&mfloor->thinker);

        sec->floordata = mfloor; //jff 2/22/98
        mfloor->thinker.function.acp1 = (actionf_p1) T_MoveFloor;
        mfloor->direction = 1;
        mfloor->sector = sec;
        mfloor->speed = speed;
        mfloor->floordestheight = height;
        mfloor->type = buildStair; //jff 3/31/98 do not leave uninited
        //jff 2/27/98 fix uninitialized crush field
        if (boomsupport)
          mfloor->crush = type==build8? false : true;
        ok = 1;
        break;
      }
    } while(ok);      // continue until no next step is found
    secnum = old_secnum; //jff 3/4/98 restore loop index
  }
  return rtn;
}


//SoM: 3/6/2000: boom donut function
//
// EV_DoDonut()
//
// Handle donut function: lower pillar, raise surrounding pool, both to height,
// texture and type of the sector surrounding the pool.
//
// Passed the linedef that triggered the donut
// Returns whether a thinker was created
//
int EV_DoDonut(line_t*  line)
{
  sector_t* s1;
  sector_t* s2;
  sector_t* s2model;
  int       secnum;
  int       rtn = 0;
  int       i;

  floormove_t* mfloor;  // new floor

  // do function on all sectors with same tag as linedef
  secnum = -1;  // init search FindSector
  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    s1 = &sectors[secnum];                // s1 is pillar's sector
              
    // do not start the donut if the pillar is already moving
    if (P_SectorActive(floor_special,s1)) //jff 2/22/98
      continue;
                      
    s2 = getNextSector(s1->linelist[0],s1);  // s2 is pool's sector
    if (!s2) continue;                    // note lowest numbered line around
                                          // pillar must be two-sided 

    // do not start the donut if the pool is already moving
    if (boomsupport && P_SectorActive(floor_special,s2)) 
      continue;                           //jff 5/7/98
                      
    // find a two sided line around the pool whose other side isn't the pillar
    for (i = 0; i < s2->linecount; i++)
    {
      // for each line of sector s2 linelist
      // [WDJ] using ptr s = s2->linelist[i] gives larger code (by 32 bytes).
      //jff 3/29/98 use true two-sidedness, not the flag
      // killough 4/5/98: changed demo_compatibility to compatibility
      if (!boomsupport)
      {
        if (!(s2->linelist[i]->flags & ML_TWOSIDED)
	    || (s2->linelist[i]->backsector == s1))
          continue;
      }
      else if (!s2->linelist[i]->backsector
	       || s2->linelist[i]->backsector == s1)
        continue;

      rtn = 1; //jff 1/26/98 no donut action - no switch change on return

      s2model = s2->linelist[i]->backsector;  // s2model is model sector for changes
        
      //  Spawn rising slime
      mfloor = Z_Malloc (sizeof(*mfloor), PU_LEVSPEC, 0);
      P_AddThinker (&mfloor->thinker);
      s2->floordata = mfloor; //jff 2/22/98
      mfloor->thinker.function.acp1 = (actionf_p1) T_MoveFloor;
      mfloor->type = donutRaise;
      mfloor->crush = false;
      mfloor->direction = 1;
      mfloor->sector = s2;
      mfloor->speed = FLOORSPEED / 2;
      mfloor->texture = s2model->floorpic;
      mfloor->newspecial = 0;
      mfloor->floordestheight = s2model->floorheight;
        
      //  Spawn lowering donut-hole pillar
      mfloor = Z_Malloc (sizeof(*mfloor), PU_LEVSPEC, 0);
      P_AddThinker (&mfloor->thinker);
      s1->floordata = mfloor; //jff 2/22/98
      mfloor->thinker.function.acp1 = (actionf_p1) T_MoveFloor;
      mfloor->type = lowerFloor;
      mfloor->crush = false;
      mfloor->direction = -1;
      mfloor->sector = s1;
      mfloor->speed = FLOORSPEED / 2;
      mfloor->floordestheight = s2model->floorheight;
      break;
    }
  }
  return rtn;
}


// SoM: Boom elevator support.
//
// EV_DoElevator
//
// Handle elevator linedef types
//
// Passed the linedef that triggered the elevator and the elevator action
//
// jff 2/22/98 new type to move floor and ceiling in parallel
//
int EV_DoElevator ( line_t* line, elevator_e elevtype )
{
  sector_t*             sec;
  elevator_t*           elevator;
  int                   rtn = 0;

  // act on all sectors with the same tag as the triggering linedef
  int secnum = -1; // init search FindSector
  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    sec = &sectors[secnum];
              
    // If either floor or ceiling is already activated, skip it
    if (sec->floordata || sec->ceilingdata) //jff 2/22/98
      continue;
      
    // create and initialize new elevator thinker
    rtn = 1;
    elevator = Z_Malloc (sizeof(*elevator), PU_LEVSPEC, 0);
    P_AddThinker (&elevator->thinker);
    sec->floordata = elevator; //jff 2/22/98
    sec->ceilingdata = elevator; //jff 2/22/98
    elevator->thinker.function.acp1 = (actionf_p1) T_MoveElevator;
    elevator->type = elevtype;

    // set up the fields according to the type of elevator action
    switch(elevtype)
    {
        // elevator down to next floor
      case elevateDown:
        elevator->direction = -1;
        elevator->sector = sec;
        elevator->speed = ELEVATORSPEED;
        elevator->floordestheight =
          P_FindNextLowestFloor(sec,sec->floorheight);
        elevator->ceilingdestheight =
          elevator->floordestheight + sec->ceilingheight - sec->floorheight;
        break;

        // elevator up to next floor
      case elevateUp:
        elevator->direction = 1;
        elevator->sector = sec;
        elevator->speed = ELEVATORSPEED;
        elevator->floordestheight =
          P_FindNextHighestFloor(sec,sec->floorheight);
        elevator->ceilingdestheight =
          elevator->floordestheight + sec->ceilingheight - sec->floorheight;
        break;

        // elevator to floor height of activating switch's front sector
      case elevateCurrent:
        elevator->sector = sec;
        elevator->speed = ELEVATORSPEED;
        elevator->floordestheight = line->frontsector->floorheight;
        elevator->ceilingdestheight =
          elevator->floordestheight + sec->ceilingheight - sec->floorheight;
        elevator->direction =
          elevator->floordestheight>sec->floorheight?  1 : -1;
        break;

      default:
        break;
    }
  }
  return rtn;
}
