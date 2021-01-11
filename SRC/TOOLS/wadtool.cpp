// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: wadtool.cpp 559 2009-11-15 17:01:03Z smite-meister $
//
// Copyright (C) 2004-2009 by DooM Legacy Team.
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
//-----------------------------------------------------------------------------

/// \file
/// \brief Simple tool for WAD manipulation

#include <string>
#include <vector>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>

#include "m_swap.h"
#include "md5.h"

using namespace std;


// wad header
struct wadheader_t 
{
  union
  {
    char magic[4];   // "IWAD", "PWAD"
    int  imagic;
  };
  int  numentries; // number of entries in WAD
  int  diroffset;  // offset to WAD directory
};


// a WAD directory entry
struct waddir_t
{
  int  offset;  // file offset of the resource
  int  size;    // size of the resource
  union
  {
    char name[8]; // name of the resource (NUL-padded)
    int  iname[2];
  };
};


static bool TestPadding(char *name, int len)
{
  // TEST padding of lumpnames
  bool warn = false;
  for (int j=0; j<len; j++)
    if (name[j] == 0)
      {
	for (j++; j<len; j++)
	  if (name[j] != 0)
	    {
	      name[j] = 0; // fix it
	      warn = true;
	    }
	break;
      }

  if (warn)
    printf("Warning: Lumpname %s not padded with zeros!\n", name);

  return warn;
}


/// \brief Simplified WAD class for wadtool
class Wad
{
protected:
  string filename; ///< the name of the associated physical file
  FILE *stream;    ///< associated stream
  int   diroffset; ///< offset to file directory
  int   numitems;  ///< number of data items (lumps)

  struct waddir_t *directory;  ///< wad directory

public:
  unsigned char md5sum[16];    ///< checksum for data integrity checks

  // constructor and destructor
  Wad();
  ~Wad();

  /// open a new wadfile
  bool Open(const char *fname);

  // query data item properties
  int GetNumItems() { return numitems; }
  const char *GetItemName(int i) { return directory[i].name; }
  int GetItemSize(int i) { return directory[i].size; }
  void ListItems(bool lumps);

  /// retrieval
  int ReadItemHeader(int item, void *dest, int size = 0);
};


// constructor
Wad::Wad()
{
  stream = NULL;
  directory = NULL;
  diroffset = numitems = 0;
}

Wad::~Wad()
{
  if (directory)
    free(directory);

  if (stream)
    fclose(stream);
}


void Wad::ListItems(bool lumps)
{
  int n = GetNumItems();
  printf(" %d lumps, MD5: ", n);
  for (int i=0; i<16; i++)
    printf("%02x:", md5sum[i]);
  printf("\n\n");

  if (!lumps)
    return;

  char name8[9];
  name8[8] = '\0';

  waddir_t *p = directory;
  for (int i = 0; i < numitems; i++, p++)
    {
      strncpy(name8, p->name, 8);
      printf(" %4d  %-8s: %10d bytes\n", i, name8, p->size);
    }
}


int Wad::ReadItemHeader(int lump, void *dest, int size)
{
  waddir_t *l = directory + lump;

  // empty resource (usually markers like S_START, F_END ..)
  if (l->size == 0)
    return 0;
  
  // 0 size means read all the lump
  if (size == 0 || size > l->size)
    size = l->size;

  fseek(stream, l->offset, SEEK_SET);
  return fread(dest, 1, size, stream); 
}


// read a WAD file from disk
bool Wad::Open(const char *fname)
{
  stream = fopen(fname, "rb");
  if (!stream)
    return false;

  filename = fname;

  // read header
  wadheader_t h;
  fread(&h, sizeof(wadheader_t), 1, stream);

  if (h.imagic != *reinterpret_cast<const int *>("IWAD") &&
      h.imagic != *reinterpret_cast<const int *>("PWAD"))
    {
      printf("Bad WAD magic number!\n");
      fclose(stream);
      stream = NULL;
      return false;
    }

  // endianness swapping
  numitems = LONG(h.numentries);
  diroffset = LONG(h.diroffset);

  // read wad file directory
  fseek(stream, diroffset, SEEK_SET);
  waddir_t *p = directory = (waddir_t *)malloc(numitems * sizeof(waddir_t)); 
  fread(directory, sizeof(waddir_t), numitems, stream);  

  // endianness conversion for directory
  for (int i = 0; i < numitems; i++, p++)
    {
      p->offset = LONG(p->offset);
      p->size   = LONG(p->size);
      TestPadding(p->name, 8);
    }

  // generate md5sum 
  rewind(stream);
  md5_stream(stream, md5sum);

  return true;
}



//=============================================================================

// prints wad contents
int ListWad(const char *wadname)
{
  Wad w;

  if (!w.Open(wadname))
    {
      printf("File '%s' could not be opened!\n", wadname);
      return -1;
    }

  printf("WAD file '%s':\n", wadname);
  w.ListItems(true);

  return 0;
}



// creates a new wad file from the lumps listed in a special "inventory" file
int CreateWad(const char *wadname, const char *inv_name)
{
  // read the inventory file
  FILE *invfile = fopen(inv_name, "rb");
  if (!invfile)
    {
      printf("Could not open the inventory file.\n");
      return -1;
    }

  int   len, i;
  vector<waddir_t>  dir;
  vector<string> fnames;

  //char *p = NULL;
  //while ((len = getline(&p, &dummy, invfile)) > 0) {}
  //free(p);

  char p[256];
  while (fgets(p, 256, invfile))
    {
      len = strlen(p);

      for (i=0; i<len && !isspace(p[i]); i++)
	; // pass the lump filename
      if (i == 0)
	{
	  printf("warning: you must give a filename for each lump.\n");
	  continue;
	}
      p[i++] = '\0'; // pass the first ws char

      for ( ; i<len && isspace(p[i]); i++)
	; // pass the ws

      char *lumpname = &p[i];
      for ( ; i<len && !isspace(p[i]); i++) // we're looking for a newline, but windows users will have crap like \r before it
	; // pass the lumpname
      p[i] = '\0';

      int n = strlen(lumpname);
      if (n < 1 || n > 8)
	{
	  printf("warning: lumpname '%s' is not acceptable.\n", lumpname);
	  continue;
	}

      if (p[0] == '-')
	{
	  printf("empty lump: %s\n", lumpname);
	}
      else if (access(p, R_OK))
	{
	  printf("warning: filename '%s' cannot be accessed.\n", p);
	  continue;
	}

      waddir_t temp;
      strncpy(temp.name, lumpname, 8);
      dir.push_back(temp);
      fnames.push_back(p);
    }

  fclose(invfile);

  len = dir.size(); // number of lumps
  if (len < 1)
    return 0;

  // construct the WAD
  FILE *outfile = fopen(wadname, "wb");
  if (!outfile)
    {
      printf("Could not create file '%s'.\n", wadname);
      return -1;
    }

  // file layout: header, lumps, directory
  wadheader_t h;
  h.imagic = *reinterpret_cast<const int *>("PWAD");
  h.numentries = LONG(len);
  h.diroffset = 0; // temporary, will be fixed later

  // write header
  fwrite(&h, sizeof(wadheader_t), 1, outfile);

  // write the lumps
  for (i=0; i<len; i++)
    {
      dir[i].offset = ftell(outfile);

      if (fnames[i] == "-")
	{
	  // separator lump
	  dir[i].size = 0;
	  continue;
	}

      FILE *lumpfile = fopen(fnames[i].c_str(), "rb");

      // get file system info about the lumpfile
      struct stat tempstat;
      fstat(fileno(lumpfile), &tempstat);
      int size = dir[i].size = tempstat.st_size;
 
      // insert the lump
      void *buf = malloc(size);
      fread(buf, size, 1, lumpfile);
      fclose(lumpfile);
      fwrite(buf, size, 1, outfile);
      free(buf);
   }

  h.diroffset = LONG(ftell(outfile)); // actual directory offset

  // write the directory
  for (i=0; i<len; i++)
    {
      dir[i].offset = LONG(dir[i].offset);
      dir[i].size   = LONG(dir[i].size);
      fwrite(&dir[i], sizeof(waddir_t), 1, outfile);
    }

  // re-write the header with the correct diroffset
  rewind(outfile);
  fwrite(&h, sizeof(wadheader_t), 1, outfile);

  fclose(outfile);
  return ListWad(wadname); // see if it opens OK
}




int ExtractWad(const char *wadname, int num, char *lumpnames[])
{
  Wad w;

  if (!w.Open(wadname))
    {
      printf("File '%s' could not be opened!\n", wadname);
      return -1;
    }

  int n = w.GetNumItems();
  printf("Extracting the lumps from WAD file %s\n", wadname);
  w.ListItems(false);

  string logname = wadname;
  logname += ".log";
  FILE *log = stdout;

  if (!num)
    {
      // create a log file (which can be used as an inventory file when recreating the wad!)
      log = fopen(logname.c_str(), "wb");
    }

  char name8[9];
  name8[8] = '\0';    
  int count = 0;
  int ln = 0;

  do {

  int i;
  // extract the lumps into files
  for (i = 0; i < n; i++)
    {
      const char *name = w.GetItemName(i);
      strncpy(name8, name, 8);

      if (num && strcasecmp(name8, lumpnames[ln]))
	continue; // not the correct one

      string lfilename = name8;
      lfilename += ".lmp";

      int size = w.GetItemSize(i);

      printf(" %-12s: %10d bytes\n", name8, size);
      if (size == 0)
	{
	  fprintf(log, "-\t\t%s\n", name8);
	  continue; // do not extract separator lumps...
	}
      else
	fprintf(log, "%-12s\t\t%s\n", lfilename.c_str(), name8);

      void *dest = malloc(size);
      w.ReadItemHeader(i, dest, 0);

      FILE *output = fopen(lfilename.c_str(), "wb");
      fwrite(dest, size, 1, output);
      fclose(output);
      count++;
      free(dest);

      if (num)
	break; // extract only first instance
    }

  if (num && i == n)
    printf("Lump '%s' not found.\n", lumpnames[ln]);

  } while (++ln < num);

  if (!num)
    fclose(log);

  printf("\nDone. Wrote %d lumps.\n", count);
  return 0;
}





int main(int argc, char *argv[])
{
  if (argc < 3 || argv[1][0] != '-')
    {
      printf("\nWADtool: Simple commandline tool for manipulating WAD files.\n"
	     "Copyright 2004-2009 Doom Legacy Team.\n\n"
	     "Usage:\n"
	     "  wadtool -l <wadfile>\t\t\tLists the contents of the WAD.\n"
	     "  wadtool -c <wadfile> <inventoryfile>\tConstructs a new WAD using the given inventory file.\n"
	     "  wadtool -x <wadfile> [<lumpname> ...]\tExtracts the given lumps into current directory.\n"
	     "    If no lumpnames are given, extracts the entire contents of the WAD.\n");
      return -1;
    }


  int ret = -1;
  switch (argv[1][1])
    {
    case 'l':
      ret = ListWad(argv[2]);
      break;
    case 'c':
      if (argc == 4)
	ret = CreateWad(argv[2], argv[3]);
      else
	printf("Usage: wadtool -c wadfile.wad <inventoryfile>\n");
      break;
    case 'x':
      if (argc >= 4)
	ret = ExtractWad(argv[2], argc-3, &argv[3]);
      else
	ret = ExtractWad(argv[2], 0, NULL);
      break;
    default:
      printf("Unknown option '%s'", argv[1]);
    }

  return ret;
}
