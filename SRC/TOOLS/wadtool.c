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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <io.h>

#include <conio.h>
#include <dpmi.h>
#include <go32.h>

#include "md5.h"
unsigned char md5sum[16];    // checksum for data integrity checks

#define VERSION "0.1.45"
#define MAX_LUMPS 16384

char extr_dir[256] = "";

/* Little-Endian Swap (für DOS) */
static inline uint32_t LE_SWAP32(uint32_t x) {
    return (int32_t)
          ( (( x & (uint32_t)0x000000ffUL) << 24)
          | (( x & (uint32_t)0x0000ff00UL) <<  8)
          | (( x & (uint32_t)0x00ff0000UL) >>  8)
          | (( x & (uint32_t)0xff000000UL) >> 24)
					 );
}
static inline int16_t LE_SWAP16(uint16_t x) {
    return (int16_t)
      ( (( x & (uint16_t)0x00ffU) << 8)
      | (( x & (uint16_t)0xff00U) >> 8)
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

typedef struct
{
  union
  {
    char magic[4];   // "IWAD", "PWAD"
    int  imagic;
  };
  int  numentries; // number of entries in WAD
  int  diroffset;  // offset to WAD directory
} wadheader_t;


// a WAD directory entry
typedef struct
{
  int  offset;  // file offset of the resource
  int  size;    // size of the resource
  union
  {
    char name[8]; // name of the resource (NUL-padded)
    int  iname[2];
  };
}waddir_t;

/* Dynamische Listen */
typedef struct {
    waddir_t *entries;
    char    **filenames;
    int      count;
    int      capacity;
} lump_list_t;

/* Prototypes */
void help_info(void);
int  ListWad(const char *wadname, int Pause);
int  CreateWad(const char *wadname, const char *inv_name);
int  ExtractWad(const char *wadname, int num, char **lumpnames);

static int TestPadding(char *name, int len)
{
  // TEST padding of lumpnames
  int warn = 0;
	int j;
  for (j=0; j<len; j++)
  {
    if (name[j] == 0)
    {
      for (j++; j<len; j++)
      {
	if (name[j] != 0)
	{
	  name[j] = 0; // fix it
	  warn = 1;
	}
      }
      break;
    }
  }

  if (warn)
    printf("Warning: Fixed Lumpname %s not padded with NULs!\n", name);

  return warn;
}


/* Hilfsfunktionen */
void str_upper(char *s) {
    while (*s) { *s = toupper(*s); s++; }
}

/* Hauptprogramm */
int main(int argc, char *argv[])
{
    int ac = 1;
		int Pause = 0;
		
    extr_dir[0] = '\0';

    if (argc < 3 || argv[1][0] != '-') {
        help_info();
        return -1;
    }

    if (argv[ac][1] == 'd') {
        if (argv[ac][2] == 0)
				{
					ac++; strcpy(extr_dir, argv[ac]);
					ac++;
				}
        else
				{
					strcpy(extr_dir, &argv[ac][2]);
					ac++;
				}
				
        if (extr_dir[strlen(extr_dir)-1] != '/' && extr_dir[strlen(extr_dir)-1] != '\\')
            strcat(extr_dir, "/");
    }

    switch (argv[ac][1]) {
        case 'l': 
				        {
									if (argc > ac+2)
									{
								      char *NamePause;
											NamePause = argv[ac+2];
										  if ((strcmp(NamePause,"-P")!=0) || (strcmp(NamePause,"-p")!=0))											
                         Pause = 1;
									}							
					        return ListWad(argv[ac+1],Pause);
				        }
        case 'c':
				        {
				          if (argc == ac+3)
										return CreateWad(argv[ac+1], argv[ac+2]);
									
                    printf("Usage: wadtool -c wadfile.wad inventory.txt\n");
										return -1;
								}
        case 'x':
				        {
				          if (argc >= ac+3)
										return ExtractWad(argv[ac+1], argc-ac-2, &argv[ac+2]);
                  else
										return ExtractWad(argv[ac+1], 0, NULL);
								}
        default:  printf("Unknown option '%s'\n", argv[ac]); return -1;
    }
}

void help_info(void)
{
    printf("\nWADtool (List, Extract, Create tool) DOS Version " VERSION "\n"
           "Usage:\n"
           "  wadtool -l <wadfile>.wad \n"
           "  wadtool -c <wadfile>.wad <inventoryfile>.txt\n"
           "  wadtool -x file.wad [lump...]\n"
           "  wadtool -d dir <command>\n\n"
					 " Help/Info:\n"
					 " l:\tList the contents of <wadfile>.wad\n"
					 " c:\tConstruct a new wadfile using the given inventory file and lumps\n"
					 "   \tfrom the current directory.\n"
					 " x:\tExtract the given lumps into the current directory.\n"
           "   \tIf no lumpnames are given, extract the entire contents of wadfile\n"
	         "Option:\n"
           "  wadtool -d <dir> <command> to/from the <dir>\n\n"
           "Copyright 2004-2015 Doom Legacy Team. [- C Version - Marty2Doom 2025 -]\n"					 
					 );
}

/* WAD öffnen und lesen */
int ListWad(const char *wadname, int Pause)
{
    FILE *f;
    wadheader_t h;
    waddir_t *dir;
    int i;
						
    f = fopen(wadname, "rb");
    if (!f) {
        printf("File '%s' could not be opened!\n", wadname);
        return -1;
    }

    fread(&h, sizeof(h), 1, f);
    h.numentries = LE_SWAP32(h.numentries);
    h.diroffset  = LE_SWAP32(h.diroffset);

    if (strncmp(h.magic, "IWAD", 4) && strncmp(h.magic, "PWAD", 4)) {
        printf("Not a valid WAD file!\n");
        fclose(f);
        return -1;
    }

    fseek(f, h.diroffset, SEEK_SET);
    dir = malloc(h.numentries * sizeof(waddir_t));
    fread(dir, sizeof(waddir_t), h.numentries, f);

    printf("\n----------------------------\n");
    
		// endianness conversion for directory
    char name8[9];
    name8[8] = '\0';
		
    for (i = 0; i < h.numentries; i++)
		{
        h.diroffset = LE_SWAP32(h.diroffset);
        dir[i].size   = LE_SWAP32(dir[i].size);
        strncpy(name8, dir[i].name, 8);
        TestPadding(name8, 8);
    }
			
/* HAVE_MD5 */
    rewind(f);
    md5_stream(f, md5sum);
    printf(" %d lumps, MD5: ", h.numentries);
    for (i=0; i<16; i++)
        printf("%02x:", md5sum[i]);
        printf("\n\n");
/* HAVE_MD5 END */

    printf("WAD file '%s': %d lumps\n\n", wadname, h.numentries);
    printf("    #  lumpname     size\n");
    printf("----------------------------\n");
    
    name8[8] = '\0';
		
    for (i = 0; i < h.numentries; i++)
    {
        strncpy(name8, dir[i].name, 8);
        printf(" %4d  %-8s %12d\n", i, name8, LE_SWAP32(dir[i].size));

        /* Alle 20 Zeilen eine Pause */
        if (((i + 1) % 20 == 0) && (Pause == 1))
        {
            printf("\n--- Druecke LEERTASTE zum Fortfahren oder ESC zum abbrechen ---");
            fflush(stdout);

            int c;
            do {
                c = getch();            // wartet auf Tastendruck (kein Enter nötig)
            } while (c != ' ' && c != 13 && c != 27);  // Space, Enter oder ESC

            if (c == 27) {              // ESC = Abbruch
                printf("\nAbgebrochen.\n");
                break;
            }
            printf("\r                                      \r"); // Zeile löschen
        }
    }

    free(dir);
    fclose(f);
    return 0;
}

/* WAD erstellen */
int CreateWad(const char *wadname, const char *inv_name)
{
	  int c;
    FILE *invfile, *out;
    char line[512], lumpname[16], filename[256];
    int i, lump_count = 0;
    waddir_t *dir;
    wadheader_t header = {{'P','W','A','D'}, 0, 0};
    long data_start, dir_start;

    invfile = fopen(inv_name, "r");
    if (!invfile) { printf("Inventory file not found!\n"); return -1; }

    /* Zähle Einträge */
    while (fgets(line, sizeof(line), invfile)) lump_count++;
    rewind(invfile);

    dir = calloc(lump_count, sizeof(waddir_t));
    out = fopen(wadname, "wb");
    if (!out) { printf("Cannot create WAD!\n"); return -1; }

    /* Header-Platzhalter */
    fwrite(&header, sizeof(header), 1, out);
    data_start = ftell(out);

    i = 0;
    while (fgets(line, sizeof(line), invfile))
		{
        if (sscanf(line, "%s %s", filename, lumpname) != 2)
					continue;						
				
        int n = strlen(lumpname);
        if (n < 1 || n > 8)
        {
	        printf("Warning: Lumpname '%s' is not acceptable (Length Name %d).\n", lumpname, n);
	        continue;
        }
			
        strncpy(dir[i].name, lumpname, sizeof(lumpname));
        dir[i].offset = ftell(out);

        char lfilename[256] = "";
	      strcat(lfilename, extr_dir);
	      strcat(lfilename, filename);
        FILE *l = fopen(lfilename, "rb");							
        if (l)
				{
            fseek(l, 0, SEEK_END);
            dir[i].size = ftell(l);
            fseek(l, 0, SEEK_SET);
            char *buf = malloc(dir[i].size);
            fread(buf, 1, dir[i].size, l);
            fwrite(buf, 1, dir[i].size, out);
            free(buf);
            fclose(l);
        }
				else
				{
          dir[i].size = 0;
					printf("- Empty Lumpname in inventory File: %s [%s]\n", lumpname,filename);
        }
        i++;
    }

    dir_start = ftell(out);
    header.numentries = LE_SWAP32(i);
    header.diroffset  = LE_SWAP32(dir_start);

    int j;
    for (j = 0; j < i; j++) {
        dir[j].offset = LE_SWAP32(dir[j].offset);
        dir[j].size   = LE_SWAP32(dir[j].size);
        fwrite(&dir[j], sizeof(waddir_t), 1, out);
    }

    fseek(out, 0, SEEK_SET);
    fwrite(&header, sizeof(header), 1, out);

    fclose(out);
    fclose(invfile);
    free(dir);
    printf("WAD %s created with %d lumps.\n", wadname, i);
    return 0;
}

/* WAD extrahieren */
int ExtractWad(const char *wadname, int num, char **lumpnames)
{
    FILE *f;
    wadheader_t h;
    waddir_t *dir;
    int i, total = 0;

    f = fopen(wadname, "rb");
    if (!f) { printf("File not found!\n"); return -1; }

    fread(&h, sizeof(h), 1, f);
    h.numentries = LE_SWAP32(h.numentries);
    h.diroffset  = LE_SWAP32(h.diroffset);

    dir = malloc(h.numentries * sizeof(waddir_t));
    fseek(f, h.diroffset, SEEK_SET);
    fread(dir, sizeof(waddir_t), h.numentries, f);

    for (i = 0; i < h.numentries; i++) {
        char name[9] = {0};
        strncpy(name, dir[i].name, 8);

        if (num && !strstr(lumpnames[0], name) && num != 1) continue;

        char outname[256];
				sprintf(outname, "%s%s.lmp", extr_dir, name);

        FILE *out = fopen(outname, "wb");
        if (out && dir[i].size > 0) {
            void *buf = malloc(dir[i].size);
            fseek(f, LE_SWAP32(dir[i].offset), SEEK_SET);
            fread(buf, 1, dir[i].size, f);
            fwrite(buf, 1, dir[i].size, out);
            free(buf);
            total++;
        }
        if (out) fclose(out);
        printf("Extracted: %s (%d bytes)\n", name, LE_SWAP32(dir[i].size));
    }

    free(dir);
    fclose(f);
    printf("Done. Extracted %d lumps.\n", total);
    return 0;
}
