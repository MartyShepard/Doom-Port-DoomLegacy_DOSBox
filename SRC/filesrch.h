// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: filesrch.h 1035 2013-08-14 00:38:40Z wesleyjohnson $
//
// Portions Copyright (C) 2000-2013 by DooM Legacy Team.
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
// DESCRIPTION:
//      Search for a file.
//      
//-----------------------------------------------------------------------------

#ifndef FILESRCH_H
#define FILESRCH_H

#include "d_netfil.h"

#define MAXSEARCHDEPTH 20

filestatus_e filesearch(char *filename, char *startpath, unsigned char *wantedmd5sum, boolean completepath, int maxsearchdepth);

#endif // FILESRCH_H
