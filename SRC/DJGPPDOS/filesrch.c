#include <stdio.h>
#include <direct.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "..\filesrch.h"
#include "..\d_netfil.h"
#include "../m_misc.h"

//
// filesearch:
//
// ATTENTION : make sure there is enouth space in filename to put a full path (255 or 512)
//    filename must be buffer of MAX_WADPATH
// if needmd5check==0 there is no md5 check
// if changestring then filename will be change with the full path and name
// maxsearchdepth==0 only search given directory, no subdirs
// return FS_NOTFOUND
//        FS_MD5SUMBAD;
//        FS_FOUND
#define MAX_SRCHPATH (MAX_WADPATH * 2)

filestatus_e filesearch(char *filename, char *startpath, unsigned char *wantedmd5sum, boolean completepath, int maxsearchdepth)
{
    DIR **dirhandle;
    struct dirent *dent;
    struct stat fstat;
    int found=0;
    char *searchname = strdup( filename);
    filestatus_e retval = FS_NOTFOUND;
    int remspace;
    int depthleft=maxsearchdepth;
    int *searchpathindex;  // each directory in the searchpath
    char searchpath[MAX_SRCHPATH];

    dirhandle = (DIR**) malloc( maxsearchdepth * sizeof( DIR*));
    searchpathindex = (int*) malloc( maxsearchdepth * sizeof( int));
    
    strncpy( searchpath, startpath, MAX_SRCHPATH-1);
    searchpath[MAX_SRCHPATH-1] = '\0';
    searchpathindex[--depthleft] = strlen( searchpath) + 1;

    dirhandle[depthleft] = opendir( searchpath);

    if(searchpath[searchpathindex[depthleft]-2] != '\\')
    {
        searchpath[searchpathindex[depthleft]-1] = '\\';
        searchpath[searchpathindex[depthleft]] = 0;
    }
    else
    {
        searchpathindex[depthleft]--;
    }

    while( (!found) && (depthleft < maxsearchdepth))
    {
        searchpath[searchpathindex[depthleft]]=0;
        dent = readdir( dirhandle[depthleft]);  // next dir entry
        if( dent)
        {
	    // append dir name
	    remspace = (MAX_SRCHPATH - 1) - searchpathindex[depthleft];
            strncpy(&searchpath[searchpathindex[depthleft]], dent->d_name, remspace);
        }

        if( !dent)  // done with dir
        {
            closedir( dirhandle[depthleft++]);
        } 
        else if( dent->d_name[0]=='.' &&
             (dent->d_name[1]=='\0' ||
              (dent->d_name[1]=='.' &&
               dent->d_name[2]=='\0')))
        {
            // ignore the "." and ".." entries, we don't want to scan uptree
        }
        else if( stat(searchpath,&fstat) < 0) // do we want to follow symlinks? if not: change it to lstat
        {
            // was the file (re)moved? can't stat it
        } 
        else if( S_ISDIR(fstat.st_mode) && depthleft)
        {
	    remspace = (MAX_SRCHPATH - 1) - searchpathindex[depthleft];
            strncpy(&searchpath[searchpathindex[depthleft]], dent->d_name, remspace);
            searchpathindex[--depthleft] = strlen(searchpath) + 1;

            if( !(dirhandle[depthleft] = opendir(searchpath)))
            {
                // can't open it... maybe no read-permissions
                // go back to previous dir
                depthleft++;
            }

            searchpath[searchpathindex[depthleft]-1]='\\';
            searchpath[searchpathindex[depthleft]]=0;
        }
        else if (!strcasecmp(searchname, dent->d_name))
        {
            switch( checkfilemd5(searchpath, wantedmd5sum))
            {
                case FS_FOUND:
                    if(completepath)
                    {
                        strncpy(filename, searchpath, MAX_WADPATH-1);
		        filename[MAX_WADPATH-1] = '\0';
                    }
#if 0
// [WDJ] This is used for "find if file exists",
// which is not choice to return the dir name instead.
// If this is ever needed, it requires a separate enable flag.	       
                    else
                    {
                        strncpy(filename, dent->d_name, MAX_WADPATH-1);
		        filename[MAX_WADPATH-1] = '\0';
                    }
#endif	       
                    retval=FS_FOUND;
                    found=1;
                    break;
                case FS_MD5SUMBAD:
                    retval = FS_MD5SUMBAD;
                    break;
                default:
                    // prevent some compiler warnings
                    break;
            }
        }
    }

    for(; depthleft<maxsearchdepth; closedir(dirhandle[depthleft++]));

    free(searchname);
    free(searchpathindex);
    free(dirhandle);

    return retval;
}
