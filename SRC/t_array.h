// Emacs style mode select -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: t_array.h 619 2010-03-23 21:16:50Z wesleyjohnson $
//
// Copyright(C) 2000 James Haley
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// $Log: t_array.h,v $
// Revision 1.1  2004/07/27 08:22:01  exl
// Add fs arrys files
//
//
//--------------------------------------------------------------------------

#ifndef __T_ARRAY_H__
#define __T_ARRAY_H__

#include "t_parse.h"

void T_AddArray(sfarray_t *);
void T_InitSaveList(void);

//#define SAVELIST_STRUCTHEAD
// The structure head seems to be unused, can just be a ptr.
#ifdef SAVELIST_STRUCTHEAD
extern sfarray_t sfsavelist;
#else
extern sfarray_t * sfsavelist;
#endif

#endif
