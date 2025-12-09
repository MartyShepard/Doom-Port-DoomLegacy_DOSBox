// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 2004-2015 by DooM Legacy Team.
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

// WADTOOL for legacy.wad manipulation.
// Will handle things.wad as lump name.

#include <string>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef HAVE_MD5
#include "md5.h"
#endif

#define VERSION "0.1.45"

static void help_info ( void )
{
    printf(
"\nWADtool: wad list, extract, and create tool.\n"
"  Handle things.wad as lump name in legacy.wad.\n"
"Version " VERSION "\n"
"Copyright 2004-2015 Doom Legacy Team.\n\n"
"Usage:\n"
"  wadtool -l <wadfile>\n\tList the contents of <wadfile>.\n"
"  wadtool -c <wadfile> <inventoryfile>\n"
"\tConstruct a new <wadfile> using the given inventory file and lumps\n"
"\tfrom the current directory.\n"
"  wadtool -x <wadfile> [<lumpname> ...]\n"
"\tExtract the given lumps into the current directory.\n"
"\tIf no lumpnames are given, extract the entire contents of <wadfile>.\n"
"Option:\n"
"  wadtool -d <dir> <command>\tto/from the <dir>\n"
	  );
}

// WAD files are always little-endian.
static inline int16_t SWAP_INT16( uint16_t x)
{
    return (int16_t)
     (  (( x & (uint16_t)0x00ffU) << 8)
      | (( x & (uint16_t)0xff00U) >> 8)
     );
}

static inline int32_t SWAP_INT32( uint32_t x)
{
    return (int32_t)
     (  (( x & (uint32_t)0x000000ffUL) << 24)
      | (( x & (uint32_t)0x0000ff00UL) <<  8)
      | (( x & (uint32_t)0x00ff0000UL) >>  8)
      | (( x & (uint32_t)0xff000000UL) >> 24)
     );
}

#ifdef __BIG_ENDIAN__
# define LE_SWAP16(x)  SWAP_INT16(x)
# define LE_SWAP32(x)  SWAP_INT32(x)
# define BE_SWAP16(x)  (x)
# define BE_SWAP32(x)  (x)
#else // little-endian machine
# define LE_SWAP16(x)  (x)
# define LE_SWAP32(x)  (x)
# define BE_SWAP16(x)  SWAP_INT16(x)
# define BE_SWAP32(x)  SWAP_INT32(x)
#endif

using namespace std;

// Extraction directory
string extr_dir;

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
  {
    if (name[j] == 0)
    {
      for (j++; j<len; j++)
      {
	if (name[j] != 0)
	{
	  name[j] = 0; // fix it
	  warn = true;
	}
      }
      break;
    }
  }

  if (warn)
    printf("Warning: Fixed Lumpname %s not padded with NULs!\n", name);

  return warn;
}


// Simplified WAD class for wadtool
class Wad
{
protected:
  string filename; // the name of the associated physical file
  FILE *stream;    // associated stream
  int   diroffset; // offset to file directory
  int   numitems;  // number of data items (lumps)

  struct waddir_t *directory;  // wad directory

public:
#ifdef HAVE_MD5
  unsigned char md5sum[16];    // checksum for data integrity checks
#endif

  // constructor and destructor
  Wad();
  ~Wad();

  // open a new wadfile
  bool Open(const char *fname);

  // query data item properties
  int GetNumItems() { return numitems; }
  const char *GetItemName(int i) { return directory[i].name; }
  int GetItemSize(int i) { return directory[i].size; }
  void ListItems(bool lumps);

  // retrieval
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
#ifdef HAVE_MD5
  int n = GetNumItems();
  printf(" %d lumps, MD5: ", n);
  for (int i=0; i<16; i++)
    printf("%02x:", md5sum[i]);
  printf("\n\n");
#endif

  if (!lumps)
    return;

  printf("    #  lumpname     size (B)\n"
	 "----------------------------\n");
  char name8[9];
  name8[8] = '\0';

  waddir_t *p = directory;
  for (int i = 0; i < numitems; i++, p++)
  {
      strncpy(name8, p->name, 8);
      printf(" %4d  %-8s %12d\n", i, name8, p->size);
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
  numitems = LE_SWAP32(h.numentries);
  diroffset = LE_SWAP32(h.diroffset);

  // read wad file directory
  fseek(stream, diroffset, SEEK_SET);
  waddir_t *p = directory = (waddir_t *)malloc(numitems * sizeof(waddir_t)); 
  fread(directory, sizeof(waddir_t), numitems, stream);  

  // endianness conversion for directory
  for (int i = 0; i < numitems; i++, p++)
  {
      p->offset = LE_SWAP32(p->offset);
      p->size   = LE_SWAP32(p->size);
      TestPadding(p->name, 8);
  }

#ifdef HAVE_MD5
  // generate md5sum 
  rewind(stream);
  md5_stream(stream, md5sum);
#endif

  return true;
}



//=============================================================================

// Print wad contents
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



// Create a new wad file from the lumps listed in a special "inventory" file
int CreateWad(const char *wadname, const char *inv_name)
{
  string lfilename;

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

  printf("Creating WAD file %s", wadname);
  if( extr_dir.length() > 0 )
  {
      printf( " from dir %s", extr_dir.c_str() );
  }
  printf(".\n");

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
      else
      {
	  // Inventory already has .lmp, .txt, and .h"
	  lfilename = extr_dir;
	  lfilename += p;
	  if (access(lfilename.c_str(), R_OK))
	  {
	      printf("warning: filename '%s' cannot be accessed.\n", p);
	      // Try name as wadtool would have created it,
	      // as lumpname with .lmp appended.
	      strncpy( p, lumpname, 8 );
	      p[8] = 0;
	      strcat( p, ".lmp" );  // cannot exceed 12 chars
	      lfilename = extr_dir;
	      lfilename += p;
	      if (access(lfilename.c_str(), R_OK))
	      {
		  continue;  // still not found
	      }
	      // Found it as wadtool wrote the name.
	      printf(" found as '%s'.\n", p );
	  }
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
  h.numentries = LE_SWAP32(len);
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

      printf( "lump=%s\n", fnames[i].c_str() );

      // Inventory already has .lmp, .txt, and .h"
      lfilename = extr_dir;
      lfilename += fnames[i].c_str();
      FILE *lumpfile = fopen( lfilename.c_str(), "rb");

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

  h.diroffset = LE_SWAP32(ftell(outfile)); // actual directory offset

  // write the directory
  for (i=0; i<len; i++)
  {
      dir[i].offset = LE_SWAP32(dir[i].offset);
      dir[i].size   = LE_SWAP32(dir[i].size);
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

  printf("Extracting the lumps from WAD file %s", wadname);
  if( extr_dir.length() > 0 )
  {
      printf( " into dir %s", extr_dir.c_str() );
  }
  printf(".\n");

  w.ListItems(false);

  string logname = wadname;
  logname += ".log";
  FILE *logf = stdout;

  if (!num)
  {
      // create a log file (which can be used as an inventory file when recreating the wad!)
      logf = fopen(logname.c_str(), "wb");
  }

  char name8[9];
  name8[8] = '\0';    
  int count = 0;
  int ln = 0;
  int i;

  do {

    // extract the lumps into files
    for (i = 0; i < n; i++)
    {
      const char *name = w.GetItemName(i);
      strncpy(name8, name, 8);

      if (num && strcasecmp(name8, lumpnames[ln]))
	continue; // not the correct one

      string lfilename = extr_dir;
      lfilename += name8;
      lfilename += ".lmp";  // Do not know, so all get .lmp

      int size = w.GetItemSize(i);

      printf(" %-12s: %10d bytes\n", name8, size);
      if (size == 0)
      {
	  fprintf(logf, "-\t\t%s\n", name8);
	  continue; // do not extract separator lumps...
      }
     
      fprintf(logf, "%-12s\t\t%s\n", lfilename.c_str(), name8);

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
    fclose(logf);

  printf("\nDone. Wrote %d lumps.\n", count);
  return 0;
}

int main(int argc, char *argv[])
{
  if (argc < 3 || argv[1][0] != '-')
  {
     help_info();
     return -1;
  }

  int ret = -1;
  int ac = 1;
  if( argv[ac][1] == 'd' )
  {
      // Extraction directory
      if( argv[ac][2] == 0 )
      {
          ac++;
	  extr_dir = argv[ac];
	  ac++;
      }
      else
      {
	  extr_dir = & argv[ac][2];
	  ac++;
      }
      char term = (extr_dir.c_str())[ extr_dir.length() - 1 ];
      if( term != '/' && term != '\\' )
        extr_dir += '/';
  }
  switch (argv[ac][1])
  {
    case 'l':
      ret = ListWad(argv[ac+1]);
      break;
    case 'c':
      if (argc == (ac+3))
	ret = CreateWad(argv[ac+1], argv[ac+2]);
      else
	printf("Usage: wadtool -c wadfile.wad <inventoryfile>\n");
      break;
    case 'x':
      if (argc >= (ac+3))
	ret = ExtractWad(argv[ac+1], (argc-ac-2), &argv[ac+2]);
      else
	ret = ExtractWad(argv[ac+1], 0, NULL);
      break;
    default:
      printf("Unknown option '%s'\n", argv[ac]);
  }

  return ret;
}
