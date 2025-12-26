// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: command.c 1373 2017-12-18 17:19:58Z wesleyjohnson $
//
// Copyright (C) 1998-2016 by DooM Legacy Team.
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
// $Log: command.c,v $
// Include: DOS DJGPP Fixes
//
// Revision 1.15  2005/05/21 08:41:23  iori_
// May 19, 2005 - PlayerArmor FS function;  1.43 can be compiled again.
//
// Revision 1.14  2004/08/26 23:15:45  hurdler
// add FS functions in console (+ minor linux fixes)
//
// Revision 1.13  2003/05/30 22:44:08  hurdler
// add checkcvar function to FS
//
// Revision 1.12  2001/12/27 22:50:25  hurdler
// fix a colormap bug, add scrolling floor/ceiling in hw mode
//
// Revision 1.11  2001/02/24 13:35:19  bpereira
//
// Revision 1.10  2001/01/25 22:15:41  bpereira
// added heretic support
//
// Revision 1.9  2000/11/11 13:59:45  bpereira
// Revision 1.8  2000/11/02 19:49:35  bpereira
// Revision 1.7  2000/10/08 13:29:59  bpereira
// Revision 1.6  2000/09/28 20:57:14  bpereira
// Revision 1.5  2000/08/31 14:30:54  bpereira
// Revision 1.4  2000/08/03 17:57:41  bpereira
// Revision 1.3  2000/02/27 00:42:10  hurdler
// Revision 1.2  2000/02/26 00:28:42  hurdler
// Mostly bug fix (see borislog.txt 23-2-2000, 24-2-2000)
//
//
// DESCRIPTION:
//      parse and execute commands from console input/scripts/
//      and remote server.
//
//      handles console variables, which is a simplified version
//      of commands, each consvar can have a function called when
//      it is modified.. thus it acts nearly as commands.
//
//      code shamelessly inspired by the QuakeC sources, thanks Id :)
//
//-----------------------------------------------------------------------------


#include "doomincl.h"
#include "doomstat.h"
#include "command.h"
#include "console.h"
#include "z_zone.h"
#include "d_clisrv.h"
#include "d_netcmd.h"
#include "m_misc.h"
#include "m_fixed.h"
#include "byteptr.h"
#include "p_saveg.h"

// Hurdler: add FS functionnality to console command
#include "t_vari.h"
//void run_string(char *data);

//========
// protos.
//========
static boolean COM_Exists (const char * com_name);
static void    COM_ExecuteString (const char * text, boolean script);

static void    COM_Alias_f (void);
static void    COM_Echo_f (void);
static void    COM_Exec_f (void);
static void    COM_Wait_f (void);
static void    COM_Help_f (void);
static void    COM_Toggle_f (void);

static boolean    CV_Var_Command (void);
static char *  CV_StringValue (const char * var_name);
static consvar_t  *consvar_vars;       // list of registered console variables

#define COM_TOKEN_MAX   1024
static char    com_token[COM_TOKEN_MAX];
static const char *  COM_Parse (const char * data, boolean script);

CV_PossibleValue_t CV_OnOff[] =    {{0,"Off"}, {1,"On"},    {0,NULL}};
CV_PossibleValue_t CV_YesNo[] =     {{0,"No"} , {1,"Yes"},   {0,NULL}};
CV_PossibleValue_t CV_uint16[]=   {{0,"MIN"}, {0xFFFF,"MAX"}, {0,NULL}};
CV_PossibleValue_t CV_Unsigned[]=   {{0,"MIN"}, {999999999,"MAX"}, {0,NULL}};
CV_PossibleValue_t CV_byte[]=   {{0,"MIN"}, {255,"MAX"}, {0,NULL}};

#define COM_BUF_SIZE    8192   // command buffer size

static int com_wait;       // one command per frame (for cmd sequences)


// command aliases
//
typedef struct cmdalias_s
{
    struct cmdalias_s   *next;
    char    *name;
    char    *value;     // the command string to replace the alias
} cmdalias_t;

static cmdalias_t *com_alias; // aliases list


// =========================================================================
//                            COMMAND BUFFER
// =========================================================================


static vsbuf_t com_text;     // variable sized buffer


//  Add text (a NUL-terminated string) in the command buffer (for later execution)
//
void COM_BufAddText (const char *text)
{
  if (!VS_Print(&com_text, text))
    CONS_Printf ("Command buffer full!\n");
}


// Adds command text immediately after the current command
// Adds a \n to the text
//
void COM_BufInsertText (const char *text)
{
    char    *temp;

    // copy off any commands still remaining in the exec buffer
    int templen = com_text.cursize;
    if (templen)
    {
      // add a trailing NUL (TODO why do we even allow non-string data in a vsbuf_t?)
      temp = Z_Malloc (templen + 1, PU_STATIC, NULL);
      temp[templen] = '\0';
      memcpy (temp, com_text.data, templen);
      VS_Clear (&com_text);
    }
    else
        temp = NULL;    // shut up compiler

    // add the entire text of the file (or alias)
    COM_BufAddText (text);

    // add the copied off data
    if (templen)
    {
      if (!VS_Print(&com_text, temp))
        CONS_Printf ("Command buffer full!!\n");

      Z_Free (temp);
    }
}


//  Flush (execute) console commands in buffer
//   does only one if com_wait
//
void COM_BufExecute ( void )
{
  int     i;
  boolean script = 1;
  char line[1024];

  if (com_wait)
  {
        com_wait--;
        return;
  }

  while (com_text.cursize)
  {
      // find a '\n' or ; line break
      char *text = (char *)com_text.data;
      boolean in_quote = false;

      // This is called without a clue as to what commands are present.
      // Scripts have quoted strings,
      // exec have quoted filenames with backslash: exec "c:\doomdir\".
      // The while loop continues into the exec which can have two levels
      // of quoted strings:
      //      alias zoom_in "fov 15; bind \"mouse 3\" zoom_out"
      script =
        ( ( strncmp(text,"exec",4) == 0 )
          ||( strncmp(text,"map",3) == 0 )
          ||( strncmp(text,"playdemo",8) == 0 )
          ||( strncmp(text,"addfile",7) == 0 )
          ||( strncmp(text,"loadconfig",10) == 0 )
          ||( strncmp(text,"saveconfig",10) == 0 )
          ) ? 0 : 1;  // has filename : is script with quoted strings

      for (i=0; i < com_text.cursize; i++)
      {
        register char ch = text[i];
        if (ch == '"') // non-escaped quote 
          in_quote = !in_quote;
        else if( in_quote )
        {
          if (script && (ch == '\\')) // escape sequence
          {
#if 1
              // [WDJ] Only doublequote and backslash really matter
              i += 1;  // skip it, because other parser does too
              continue;
#else
              switch (text[i+1])
              {
                case '\\': // backslash
                case '"':  // double quote
                case 't':  // tab
                case 'n':  // newline
                  i += 1;  // skip it
                  break;

                default:
                  // unknown sequence, parser will give an error later on.
                  break;
              }
              continue;
#endif	     
          }
        }
        else
        { // not in quoted string
          if (ch == ';') // semicolon separates commands
            break;
          if (ch == '\n' || ch == '\r') // always separate commands
            break;
        }
      }


      if( i > 1023 )  i = 1023;  // overrun of line
      memcpy (line, text, i);
      line[i] = 0;

      // flush the command text from the command buffer, _BEFORE_
      // executing, to avoid that 'recursive' aliases overflow the
      // command text buffer, in that case, new commands are inserted
      // at the beginning, in place of the actual, so it doesn't
      // overflow
      if (i == com_text.cursize)
      {
            // the last command was just flushed
            com_text.cursize = 0;
      }
      else
      {
            i++;
            com_text.cursize -= i;
            // Shuffle text, overlap copy.  Bug fix by Ryan bug_0626.
            memmove(text, text+i, com_text.cursize);
      }

      // execute the command line
      COM_ExecuteString (line, script);

      // delay following commands if a wait was encountered
      if (com_wait)
      {
            com_wait--;
            break;
      }
  }
}


// =========================================================================
//                            COMMAND EXECUTION
// =========================================================================

typedef struct xcommand_s
{
    const char       * name;
    struct xcommand_s * next;
    com_func_t         function;
} xcommand_t;

static  xcommand_t  *com_commands = NULL;     // current commands


#define MAX_ARGS        80
static int         com_argc;
static char        *com_argv[MAX_ARGS];
static char        *com_null_string = "";
static const char * com_args = NULL;          // current command args or NULL


//  Initialize command buffer and add basic commands
//
// Called only once
void COM_Init (void)
{
    int i;
    for( i=0; i<MAX_ARGS; i++ )  com_argv[i] = com_null_string;
    com_argc = 0;

    // allocate command buffer
    VS_Alloc (&com_text, COM_BUF_SIZE);

    // add standard commands
    COM_AddCommand ("alias",COM_Alias_f);
    COM_AddCommand ("echo", COM_Echo_f);
    COM_AddCommand ("exec", COM_Exec_f);
    COM_AddCommand ("wait", COM_Wait_f);
    COM_AddCommand ("help", COM_Help_f);
    COM_AddCommand ("toggle", COM_Toggle_f);
    Register_NetXCmd(XD_NETVAR, Got_NetXCmd_NetVar);
}


// Returns how many args for last command
//
int COM_Argc (void)
{
    return com_argc;
}


// Returns string pointer for given argument number
//
char * COM_Argv (int arg)
{
    if ( arg >= com_argc || arg < 0 )
        return com_null_string;
    return com_argv[arg];
}

// get some args
// More efficient, but preserves read only interface
void  COM_Args( COM_args_t * comargs )
{
    int i;
    comargs->num = com_argc;
    for( i=0; i<4; i++ )
    {
        comargs->arg[i] = com_argv[i];
    }
}

#if 0
// [WDJ] Unused
// Returns string pointer of all command args
//
char *COM_Args (void)
{
    return com_args;
}
#endif


int COM_CheckParm (const char *check)
{
    int         i;

    for (i = 1; i<com_argc; i++)
    {
        if ( !strcasecmp(check, com_argv[i]) )
            return i;
    }
    return 0;
}


// Parses the given string into command line tokens.
//
// Takes a null terminated string.  Does not need to be /n terminated.
// breaks the string up into arg tokens.
static void COM_TokenizeString (const char * text, boolean script)
{
    int  i;

// clear the args from the last string
    for (i=0 ; i<com_argc ; i++)
    {
        Z_Free (com_argv[i]);
        com_argv[i] = com_null_string;  // never leave behind ptrs to old mem
    }

    com_argc = 0;
    com_args = NULL;

    while (1)
    {
// skip whitespace up to a /n
        while (*text && *text <= ' ' && *text != '\n')
            text++;

        if (*text == '\n')
        {   // a newline means end of command in buffer,
            // thus end of this command's args too
            text++;
            break;
        }

        if (!*text)
            return;

        if (com_argc == 1)
            com_args = text;

        text = COM_Parse (text, script);
        if (!text)
            return;

        if (com_argc < MAX_ARGS)
        {
            com_argv[com_argc] = Z_Malloc (strlen(com_token)+1, PU_STATIC, NULL);
            strcpy (com_argv[com_argc], com_token);
            com_argc++;
        }
    }

}


// Add a command before existing ones.
//
void COM_AddCommand (const char *name, com_func_t func)
{
    xcommand_t * cmd;

    // fail if the command is a variable name
    if (CV_StringValue(name)[0])
    {
        CONS_Printf ("%s is a variable name\n", name);
        return;
    }

    // fail if the command already exists
    for (cmd=com_commands ; cmd ; cmd=cmd->next)
    {
        if (!strcmp (name, cmd->name))
        {
            CONS_Printf ("Command %s already exists\n", name);
            return;
        }
    }

    cmd = Z_Malloc (sizeof(xcommand_t), PU_STATIC, NULL);
    cmd->name = name;
    cmd->function = func;
    cmd->next = com_commands;
    com_commands = cmd;
}


//  Returns true if a command by the name given exists
//
static boolean COM_Exists (const char * com_name)
{
    xcommand_t * cmd;

    for (cmd=com_commands ; cmd ; cmd=cmd->next)
    {
        if (!strcmp (com_name,cmd->name))
            return true;
    }

    return false;
}


//  Command completion using TAB key like '4dos'
//  Will skip 'skips' commands
//
//  partial : a partial keyword
const char * COM_CompleteCommand (const char *partial, int skips)
{
    xcommand_t  *cmd;
    int        len;

    len = strlen(partial);

    if (!len)
        return NULL;

// check functions
    for (cmd=com_commands ; cmd ; cmd=cmd->next)
    {
        if (!strncmp (partial,cmd->name, len))
        {
            if (!skips--)
                return cmd->name;
        }
    }

    return NULL;
}



// Parses a single line of text into arguments and tries to execute it.
// The text can come from the command buffer, a remote client, or stdin.
//
static void COM_ExecuteString (const char *text, boolean script)
{
    xcommand_t  *cmd;
    cmdalias_t *a;

    COM_TokenizeString (text, script);

// execute the command line
    if (com_argc==0)
        return;     // no tokens

// check functions
    for (cmd=com_commands ; cmd ; cmd=cmd->next)
    {
        if (!strcmp (com_argv[0],cmd->name))
        {
            cmd->function ();
            return;
        }
    }

// check aliases
    for (a=com_alias ; a ; a=a->next)
    {
        if (!strcmp (com_argv[0], a->name))
        {
            COM_BufInsertText (a->value);
            return;
        }
    }

// check FraggleScript functions
    if (find_variable(com_argv[0])) // if this is a potential FS function, try to execute it
    {
//        run_string(text);
        return;
    }

    // check cvars
    // Hurdler: added at Ebola's request ;)
    // (don't flood the console in software mode with bad gr_xxx command)
    if (!CV_Var_Command () && con_destlines)
    {
        CONS_Printf ("Unknown command '%s'\n", com_argv[0]);
    }
}



// =========================================================================
//                            SCRIPT COMMANDS
// =========================================================================


// alias command : a command name that replaces another command
//
static void COM_Alias_f (void)
{
    cmdalias_t  *a;
    char        cmd[1024];
    int         i;
    COM_args_t  carg;
    
    COM_Args( &carg );
   
    if ( carg.num < 3 )
    {
        CONS_Printf("alias <name> <command>\n");
        return;
    }

    a = Z_Malloc (sizeof(cmdalias_t), PU_STATIC, NULL);
    a->next = com_alias;
    com_alias = a;

    a->name = Z_StrDup (carg.arg[1]);

// copy the rest of the command line
    cmd[0] = 0;     // start out with a null string
    for (i=2 ; i<carg.num ; i++)
    {
        register int n = 1020 - strlen( cmd );  // free space, with " " and "\n"
        strncat (cmd, COM_Argv(i), n);
        if (i != carg.num)
            strcat (cmd, " ");
    }
    strcat (cmd, "\n");

    a->value = Z_StrDup (cmd);
}


// Echo a line of text to console
//
static void COM_Echo_f (void)
{
    int     i;
    COM_args_t  carg;
  
    COM_Args( &carg );

    for (i=1 ; i<carg.num ; i++)
        CONS_Printf ("%s ",COM_Argv(i));
    CONS_Printf ("\n");
}


// Execute a script file
//
static void COM_Exec_f (void)
{
    byte*   buf=NULL;
    COM_args_t  carg;
   
    COM_Args( &carg );

    if (carg.num != 2)
    {
        CONS_Printf ("exec <filename> : run a script file\n");
        return;
    }

// load file

#ifdef DEBUG_EXEC_FILE_LENGTH
    int length;
    length = FIL_ReadFile (carg.arg[1], &buf);
    debug_Printf ("EXEC: file length = %d\n", length);
#else   
    FIL_ReadFile (carg.arg[1], &buf);
#endif

    if (!buf)
    {
        CONS_Printf ("couldn't execute file %s\n", carg.arg[1]);
        return;
    }

    CONS_Printf ("executing %s\n", carg.arg[1]);

// insert text file into the command buffer

    COM_BufInsertText((char *)buf);

// free buffer

    Z_Free(buf);
}


// Delay execution of the rest of the commands to the next frame,
// allows sequences of commands like "jump; fire; backward"
//
static void COM_Wait_f (void)
{
    COM_args_t  carg;
  
    COM_Args( &carg );
    if (carg.num>1)
        com_wait = atoi( carg.arg[1] );
    else
        com_wait = 1;   // 1 frame
}

static void COM_Help_f (void)
{
    xcommand_t  *cmd;
    consvar_t  *cvar;
    int i=0;
    COM_args_t  carg;
    
    COM_Args( &carg );

    if( carg.num>1 )
    {
        cvar = CV_FindVar (carg.arg[1]);
        if( cvar )
        {
            con_Printf("Variable %s:\n",cvar->name);
            con_Printf("  flags :");
            if( cvar->flags & CV_SAVE )
                con_Printf("AUTOSAVE ");
            if( cvar->flags & CV_FLOAT )
                con_Printf("FLOAT ");
            if( cvar->flags & CV_NETVAR )
                con_Printf("NETVAR ");
            if( cvar->flags & CV_CALL )
                con_Printf("ACTION ");
            con_Printf("\n");
            if( cvar->PossibleValue )
            {
                if(strcasecmp(cvar->PossibleValue[0].strvalue,"MIN")==0)
                {
                    for(i=1; cvar->PossibleValue[i].strvalue!=NULL; i++)
                    {
                        if(!strcasecmp(cvar->PossibleValue[i].strvalue,"MAX"))
                            break;
                    }
                    con_Printf("  range from %d to %d\n",cvar->PossibleValue[0].value,cvar->PossibleValue[i].value);
                }
                else
                {
                    con_Printf("  possible value :\n",cvar->name);
                    while(cvar->PossibleValue[i].strvalue)
                    {
                        con_Printf("    %-2d : %s\n",cvar->PossibleValue[i].value,cvar->PossibleValue[i].strvalue);
                        i++;
                    }
                }
            }
        }
        else
            con_Printf("No Help for this command/variable\n");
    }
    else
    {
        // commands
        con_Printf("\2Commands\n");
        for (cmd=com_commands ; cmd ; cmd=cmd->next)
        {
            con_Printf("%s ",cmd->name);
            i++;
        }

        // variable
        con_Printf("\2\nVariables\n");
        for (cvar=consvar_vars; cvar; cvar = cvar->next)
        {
            con_Printf("%s ",cvar->name);
            i++;
        }

        con_Printf("\2\nRead the console docs for more or type help <command or variable>\n");

        if( devparm )
            con_Printf("\2Total : %d\n",i);
    }
}

static void COM_Toggle_f(void)
{
    consvar_t  *cvar;
    COM_args_t  carg;
    
    COM_Args( &carg );

    if(carg.num!=2 && carg.num!=3)
    {
        CONS_Printf("Toggle <cvar_name> [-1]\n"
                    "Toggle the value of a cvar\n");
        return;
    }
    cvar = CV_FindVar (carg.arg[1]);
    if(!cvar)
    {
        CONS_Printf("%s is not a cvar\n", carg.arg[1]);
        return;
    }

    // netcvar don't change imediately
    cvar->flags |= CV_SHOWMODIF_ONCE;  // show modification, reset flag
    if( carg.num==3 )
        CV_AddValue(cvar, atol( carg.arg[2] ));
    else
        CV_AddValue(cvar,+1);
}

// =========================================================================
//                      VARIABLE SIZE BUFFERS
// =========================================================================

#define VSBUFMINSIZE   256

void VS_Alloc (vsbuf_t *buf, int initsize)
{
    if (initsize < VSBUFMINSIZE)
        initsize = VSBUFMINSIZE;
    buf->data = Z_Malloc (initsize, PU_STATIC, NULL);
    buf->maxsize = initsize;
    buf->cursize = 0;
    buf->allowoverflow = false;
}


void VS_Free (vsbuf_t *buf)
{
//  Z_Free (buf->data);
    buf->cursize = 0;
}


void VS_Clear (vsbuf_t *buf)
{
    buf->cursize = 0;
}


// Add length to the space in use.  Detect overflow.
void *VS_GetSpace (vsbuf_t *buf, int length)
{
    if (buf->cursize + length > buf->maxsize)
    {
        if (!buf->allowoverflow)
          return NULL;

        if (length > buf->maxsize)
          return NULL;

        buf->overflowed = true;
        CONS_Printf ("VS buffer overflow");
        VS_Clear (buf);
    }

    void *data = buf->data + buf->cursize;
    buf->cursize += length;

    return data;
}


#if 0
// [WDJ] Unused
//  Copy data at end of variable sized buffer
//
boolean VS_Write (vsbuf_t *buf, void *data, int length)
{
  void *to = VS_GetSpace(buf, length);
  if (!to)
    return false;

  memcpy(to, data, length);
  return true;
}
#endif


//  Print text in variable size buffer, like VS_Write + trailing 0
//
boolean VS_Print (vsbuf_t *buf, const char *data)
{
  int len = strlen(data) + 1;
  int old_size = buf->cursize;  // VS_GetSpace modifies cursize
   
  // Remove trailing 0 before any consideration.
  // Otherwise the extra accumulates until garbage gets between the appends.
  if( old_size )
  {
      if( buf->data[old_size-1] == 0 )
         buf->cursize --;
  }

  // len-1 would be enough if there already is a trailing zero, but...
  byte *to = (byte *)VS_GetSpace(buf, len);
  if (!to)  goto fail_cleanup;
  
  memcpy(to, data, len); 
  return true;
   
fail_cleanup:
  // Restore
  buf->cursize = old_size;
  return false;
}

// =========================================================================
//
//                           CONSOLE VARIABLES
//
//   console variables are a simple way of changing variables of the game
//   through the console or code, at run time.
//
//   console vars acts like simplified commands, because a function can be
//   attached to them, and called whenever a console var is modified
//
// =========================================================================

static char       *cv_null_string = "";
byte    command_EV_param = 0;

//  Search if a variable has been registered
//  returns true if given variable has been registered
//
consvar_t * CV_FindVar (const char * name)
{
    consvar_t  *cvar;

    for (cvar=consvar_vars; cvar; cvar = cvar->next)
    {
        if ( !strcmp(name,cvar->name) )
            return cvar;
    }

    return NULL;
}


//  Build a unique Net Variable identifier number, that is used
//  in network packets instead of the fullname
//
uint16_t  CV_ComputeNetid (const char * s)
{
    uint16_t ret;
    static byte premiers[16] = {2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53};
    int i;

    ret=0;
    i=0;
    while(*s)
    {
        ret += ((byte)(*s)) * ((unsigned int) premiers[i]);
        s++;
        i = (i+1)%16;
    }
    return ret;
}


//  Return the Net Variable, from it's identifier number
//
static consvar_t * CV_FindNetVar (uint16_t netid)
{
    consvar_t  *cvar;

    for (cvar=consvar_vars; cvar; cvar = cvar->next)
    {
        if (cvar->netid==netid)
            return cvar;
    }

    return NULL;
}

static byte OnChange_user_enable = 0;
static void set_cv_str_value (consvar_t *var, const char * valstr, byte call_enable, byte user_enable );

//  Register a variable, that can be used later at the console
//
void CV_RegisterVar (consvar_t *cvar)
{
    // first check to see if it has already been defined
    if (CV_FindVar (cvar->name))
    {
        CONS_Printf ("Variable %s is already defined\n", cvar->name);
        return;
    }

    // check for overlap with a command
    if (COM_Exists (cvar->name))
    {
        CONS_Printf ("%s is a command name\n", cvar->name);
        return;
    }

    cvar->string = NULL;

    // check net cvars
    if (cvar->flags & CV_NETVAR)
    {
        cvar->netid = CV_ComputeNetid (cvar->name);
        if (CV_FindNetVar(cvar->netid))
            I_Error("Variable %s has duplicate netid\n",cvar->name);
    }

    // link the cvar in
    if( !(cvar->flags & CV_HIDEN) )
    {
        cvar->next = consvar_vars;
        consvar_vars = cvar;
    }

#ifdef PARANOIA
    if ((cvar->flags & CV_NOINIT) && !(cvar->flags & CV_CALL))
        I_Error("variable %s have CV_NOINIT without CV_CALL\n");
    if ((cvar->flags & CV_CALL) && !cvar->func)
        I_Error("variable %s have cv_call flags without func");
#endif
    set_cv_str_value(cvar, cvar->defaultvalue,
                     ((cvar->flags & CV_NOINIT) == 0), // call_enable
                     1 );  // user_enable, default is a user setting		     

    // set_cv_str_value will have set this bit
    cvar->flags &= ~CV_MODIFIED;
}


//  Returns the string value of a console var
//
static char * CV_StringValue (const char * var_name)
{
    consvar_t *cvar;

    cvar = CV_FindVar (var_name);
    if (!cvar)
        return cv_null_string;
    return cvar->string;
}


//  Completes the name of a console var
//
const char * CV_CompleteVar (const char * partial, int skips)
{
    consvar_t  *cvar;
    int         len;

    len = strlen(partial);

    if (!len)
        return NULL;

    // check functions
    for (cvar=consvar_vars ; cvar ; cvar=cvar->next)
    {
        if (!strncmp (partial,cvar->name, len))
        {
            if (!skips--)
                return cvar->name;
        }
    }

    return NULL;
}


// Set variable value, for user settings, save games, and network settings.
// When enabled, it also changes the user saved value in string.
// Also updates value and EV.
//  call_enable : when 0, blocks CV_CALL
//  user_enable : enable setting the string value (user save value)
static void set_cv_str_value (consvar_t *cvar, const char * valstr, byte call_enable, byte user_enable )
{
    char  value_str[64];  // print %d cannot exceed 64
    int i, ival;

#ifdef PARANOIA
    if( valstr == NULL )
    {
        I_SoftError( "set_cv_str_value passed NULL string: %s\n", cvar->name );
        return;
    }
#endif

    ival = atoi(valstr);  // enum and integer values

    if(cvar->PossibleValue)
    {
        if( strcasecmp(cvar->PossibleValue[0].strvalue,"MIN") == 0)
        {   // bounded cvar
            // search for MAX
            for(i=1; cvar->PossibleValue[i].strvalue!=NULL; i++)
            {
                if(!strcasecmp(cvar->PossibleValue[i].strvalue,"MAX"))
                    break;
            }

#ifdef PARANOIA
            if(cvar->PossibleValue[i].strvalue==NULL)
                I_Error("Bounded cvar \"%s\" without MAX !", cvar->name);
#endif
            // [WDJ] Cannot print into const string.
            if(ival < cvar->PossibleValue[0].value)
            {
                ival = cvar->PossibleValue[0].value;
                sprintf(value_str,"%d", ival);
                valstr = value_str;
            }
            if(ival > cvar->PossibleValue[i].value)
            {
                ival = cvar->PossibleValue[i].value;
                sprintf(value_str,"%d", ival);
                valstr = value_str;
            }
        }
        else
        {
            // waw spaghetti programming ! :)

            // check for string match
            for(i=0; cvar->PossibleValue[i].strvalue!=NULL; i++)
            {
                if(!strcasecmp(cvar->PossibleValue[i].strvalue, valstr))
                    goto found;
            }
            // If valstr is not a number, then it cannot be used as an index.
            if( ival == 0 )
            {
               if(strcmp(valstr,"0")!=0) // !=0 if valstr!="0"
                    goto error;
            }
            // check as enum index
            for(i=0; cvar->PossibleValue[i].strvalue!=NULL; i++)
            {
                if(ival == cvar->PossibleValue[i].value)
                    goto found;
            }
            goto error;

found:
            ival = cvar->PossibleValue[i].value;
            if( user_enable )
            {
                cvar->value = ival;
                // When value is from PossibleValue, string is a const char *.
                cvar->string = (char*) cvar->PossibleValue[i].strvalue;
            }
            goto finish;
        }
    }

    // CV_STRING has no temp values, and is used for network addresses.
    // Block it for security reasons, to prevent redirecting.
    // Only change the cvar string when user is making the change.
    if( user_enable )
    {
        // free the old value string
        if(cvar->string)
            Z_Free (cvar->string);

        cvar->string = Z_StrDup (valstr);
    }

    // Update value when set by user, or if flagged as numeric value.
    // CV_uint16, CV_Unsigned may not fit into EV.
    if( user_enable
        || ( cvar->flags & (CV_FLOAT | CV_VALUE) ) )
    {
        if( cvar->flags & CV_FLOAT )
        {
            double d = atof( valstr );
            cvar->value = d * FRACUNIT;
        }
        else
            cvar->value = ival;
    }

finish:
    if( cvar->flags & (CV_SHOWMODIF | CV_SHOWMODIF_ONCE) )
    {
        CONS_Printf("%s set to %s\n", cvar->name, valstr );
        cvar->flags &= ~CV_SHOWMODIF_ONCE;
    }
    DEBFILE(va("%s set to %s\n", cvar->name, cvar->string));
    cvar->flags |= CV_MODIFIED;
    cvar->EV = ival;  // user setting of active value
    // raise 'on change' code
    if( call_enable
        && (cvar->flags & CV_CALL) && (cvar->func) )
    {
        OnChange_user_enable = user_enable;
        cvar->func ();
    }
    return;

error: // not found
    CONS_Printf("\"%s\" is not a possible value for \"%s\"\n", valstr, cvar->name);
    if( strcasecmp(cvar->defaultvalue, valstr) == 0 )
    { 
        I_SoftError("Variable %s default value \"%s\" is not a possible value\n",
                    cvar->name, cvar->defaultvalue);
    }
    return;
}

// Called after demo to restore the user settings.
// Copies value to EV.
void CV_Restore_User_Settings( void )
{
    consvar_t * cvar;

    // Check for modified cvar
    for (cvar=consvar_vars; cvar; cvar = cvar->next)
    {
        if( cvar->flags & CV_VALUE )
        {
            cvar->value = atoi( cvar->string );
        }
        if( (cvar->EV != (byte)cvar->value)
            || (cvar->value >> 8)
            || (cvar->flags & CV_EV_PARAM) )
        {
            cvar->EV = cvar->value;  // user setting of active value
            // Use func to restore state dependent upon this setting.
            // raise 'on change' code
            if( cvar->flags & CV_CALL )
                cvar->func();

            cvar->flags &= ~CV_EV_PARAM;
        }
    }
    command_EV_param = 0;
}


//
// Use XD_NETVAR argument :
//      2 byte for variable identification
//      then the value of the variable followed with a 0 byte (like str)
//
// Receive network game settings, or restore save game.
void Got_NetXCmd_NetVar(xcmd_t * xc)
{
    byte * bp = xc->curpos;	// macros READ,SKIP want byte*

    consvar_t *cvar = CV_FindNetVar(READU16(bp));
    char *svalue = (char *)bp;

    while( *(bp++) ) {  // find 0 term
       if( bp > xc->endpos )  goto buff_overrun;  // bad string
    }
    xc->curpos = bp;	// return updated ptr only once
    if(cvar==NULL)
    {
        CONS_Printf("\2Netvar not found\n");
        return;
    }

    set_cv_str_value(cvar, svalue, 1, 0);  // CALL, temp
    return;

buff_overrun:
    xc->curpos = xc->endpos+2;  // indicate overrun
    return;
}


// Called by SV_Send_ServerConfig, P_SaveGame.
void CV_SaveNetVars(xcmd_t * xc)
{
    char buf[32];
    char * vp;
    consvar_t  *cvar;
    byte * bp = xc->curpos;	// macros want byte*
    

    // We must send all NETVAR cvar, because on another machine,
    // some of them may have a different value.
    for (cvar=consvar_vars; cvar; cvar = cvar->next)
    {
        if( ! (cvar->flags & CV_NETVAR) )  continue;

        // Command line settings goto network games and savegames.
        // CV_STRING do not have temp values.
        if( cvar->flags & (CV_EV_PARAM | CV_VALUE)
//            || ! (cvar->flags & CV_STRING)
          )
        {
            if( cvar->flags & CV_VALUE )
            {
                // May be too large for EV, send the value.
                sprintf (buf, "%d", cvar->value);
            }
            else
            {
                // Send the EV param value instead.
                sprintf (buf, "%d", cvar->EV);
            }
            vp = buf;
        }
        else
        {
            vp = cvar->string;	       
        }
        // potential buffer overrun test
        if((bp + 2 + strlen(vp)) > xc->endpos )  goto buff_overrun;
        // Format:  netid uint16, var_string str0.
        WRITE16(bp,cvar->netid);
        bp = write_string(bp, vp);
    }
    xc->curpos = bp;	// return updated ptr only once
    return;

buff_overrun:
    I_SoftError( "Net Vars overrun available packet space\n" );
    return;
}

// Only used for server state.
void CV_LoadNetVars(xcmd_t * xc)
{
    consvar_t  *cvar;

    // Read netvar from byte stream, identified by netid.
    for (cvar=consvar_vars; cvar; cvar = cvar->next)
    {
        if (cvar->flags & CV_NETVAR)
            Got_NetXCmd_NetVar( xc );
        // curpos on last read can go to endpos+1
        if(xc->curpos > xc->endpos+1)  goto buff_overrun;
    }
    return;

buff_overrun:
    I_SoftError( "Load Net Vars overran packet buffer\n" );
    return;
}

#define SET_BUFSIZE 128

// Sets a var to a string value.
// called by CV_Var_Command to handle "<varname> <value>" entered at the console
void CV_Set (consvar_t *cvar, const char *str_value)
{
    //changed = strcmp(var->string, value);
#ifdef PARANOIA
    if(!cvar)
        I_Error("CV_Set : no variable\n");
    if(!cvar->string)
        I_Error("CV_Set : %s no string set ?!\n", cvar->name);
#endif
    if(strcasecmp(cvar->string, str_value) == 0)
        return; // no changes

    if (netgame)
    {
      // in a netgame, certain cvars are handled differently
      if (cvar->flags & CV_NET_LOCK)
      {
        CONS_Printf("This variable cannot be changed during a netgame.\n");
        return;
      }

      if( cvar->flags & CV_NETVAR )
      {
        if (!server)
        {
            CONS_Printf("Only the server can change this variable.\n");
            return;
        }

        // Change user settings too, but want only one CV_CALL.
        set_cv_str_value(cvar, str_value, 0, 1); // no CALL, user

        // send the value of the variable
        byte buf[SET_BUFSIZE], *p; // macros want byte*
        p = buf;
        // Format:  netid uint16, var_string str0.
        WRITEU16(p, cvar->netid);
        p = write_stringn(p, str_value, SET_BUFSIZE-2-1);
        Send_NetXCmd(XD_NETVAR, buf, (p - buf));
        return;
      }
    }

    set_cv_str_value(cvar, str_value, 1, 1);  // CALL, user
}


//  Expands value to string before calling CV_Set ()
//
void CV_SetValue (consvar_t *cvar, int value)
{
    char    val[32];

    sprintf (val, "%d", value);
    CV_Set (cvar, val);
}

// Set a command line parameter value (temporary).
// This should not affect owner saved values.
void CV_SetParam (consvar_t *cvar, int value)
{
    command_EV_param = 1;  // flag to undo these later
    cvar->EV = value;   // temp setting, during game play
    cvar->flags |= CV_EV_PARAM;
    if( cvar->flags & CV_CALL )
        cvar->func();
}

// If a OnChange func tries to change other values,
// this function should be used.
// It will determine the same user_enable.
void CV_Set_by_OnChange (consvar_t *cvar, int value)
{
    byte saved_user_enable = OnChange_user_enable;
    if( OnChange_user_enable )
    {
        CV_SetValue( cvar, value );
    }
    else
    {
        CV_SetParam( cvar, value );
    }
    OnChange_user_enable = saved_user_enable;
}


#define MINpv 0

void CV_AddValue (consvar_t *cvar, int increment)
{
    int   newvalue = cvar->value + increment;

    if( cvar->PossibleValue )
    {
        if( strcmp(cvar->PossibleValue[MINpv].strvalue,"MIN")==0 )
        {
            // MIN .. MAX
            int min_value = cvar->PossibleValue[MINpv].value; 
            int max_value = MAXINT;
            int max;

            // Search the list.
            for(max=0; max<99 ; max++)
            {
                if( cvar->PossibleValue[max].strvalue == NULL )
                   break;
                if( strcmp(cvar->PossibleValue[max].strvalue,"INC")==0 )
                {
                    // Has an INC
                    newvalue = cvar->value
                     + (increment * cvar->PossibleValue[max].value);
                }
                else
                {
                   max_value = cvar->PossibleValue[max].value;
                }
            }

            if( newvalue < min_value )
            {
                newvalue += max_value - min_value + 1;   // add the max+1
            }
            newvalue = min_value
             + ((newvalue - min_value) % (max_value - min_value + 1));

            CV_SetValue(cvar,newvalue);
        }
        else
        {
            // List of Values
            int max,currentindice=-1,newindice;

            // this code do not support more than same value for differant PossibleValue
            for(max=0; cvar->PossibleValue[max].strvalue!=NULL; max++)
            {
                if( cvar->PossibleValue[max].value == cvar->value )
                    currentindice=max;
            }
            max--;
#ifdef PARANOIA
            if( currentindice==-1 )
            {
                I_SoftError("CV_AddValue : current value %d not found in possible value\n", cvar->value);
                return;
            }
#endif
            newindice=(currentindice+increment+max+1) % (max+1);
            CV_Set(cvar, cvar->PossibleValue[newindice].strvalue);
        }
    }
    else
    {
        CV_SetValue(cvar,newvalue);
    }
}


//  Allow display of variable content or change from the console
//
//  Returns false if the passed command was not recognised as
//  console variable.
//
static boolean CV_Var_Command (void)
{
    consvar_t   *cvar;
    const char * tstr;
    COM_args_t  carg;
    int tval, i;
    
    COM_Args( &carg );

    // check variables
    cvar = CV_FindVar ( carg.arg[0] );
    if(!cvar)
        return false;

    // perform a variable print or set
    if ( carg.num == 1 )  goto show_value;

    CV_Set (cvar, carg.arg[1] );
    return true;

show_value:
    if( cvar->flags & CV_STRING )  goto std_show_str;
    if( cvar->flags & CV_VALUE )
    {
        if( cvar->value == atoi(cvar->string) )  goto std_show_str;
        tval = cvar->value;
    }
    else if( (cvar->flags & CV_EV_PARAM)
        || (cvar->EV != (byte)cvar->value) )
    {
        tval = cvar->EV;
    }
    else goto std_show_str;
   
    if( cvar->PossibleValue )
    {
        for( i = 0;  ; i++ )
        {
	    if( cvar->PossibleValue[i].strvalue == NULL )  break;
            if( cvar->PossibleValue[i].value == tval )
            {
                tstr = cvar->PossibleValue[i].strvalue;
                goto show_by_str;
            }
        }
    }

    CONS_Printf ("\"%s\" is \"%i\" config \"%s\" default is \"%s\"\n",
                 cvar->name, tval, cvar->string, cvar->defaultvalue);
    return true;
	
show_by_str:
    CONS_Printf ("\"%s\" is \"%s\" config \"%s\" default is \"%s\"\n",
                 cvar->name, tstr, cvar->string, cvar->defaultvalue);
    return true;

std_show_str:
    CONS_Printf ("\"%s\" is \"%s\" default is \"%s\"\n",
                 cvar->name, cvar->string, cvar->defaultvalue);
    return true;
}


//  Save console variables that have the CV_SAVE flag set
//
void CV_SaveVariables (FILE *f)
{
    consvar_t *cvar;

    for (cvar = consvar_vars ; cvar ; cvar=cvar->next)
    {
        if (cvar->flags & CV_SAVE)
            fprintf (f, "%s \"%s\"\n", cvar->name, cvar->string);
    }
}


//============================================================================
//                            SCRIPT PARSE
//============================================================================

//  Parse a token out of a string, handles script files too
//  returns the data pointer after the token
//  Do not mangle filenames, set script only where strings might have '\' escapes.
static const char * COM_Parse (const char * data, boolean script)
{
    int c;
    int len = 0;
    com_token[0] = '\0';

    if (!data)
        return NULL;

// skip whitespace
skipwhite:
    while ( (c = *data) <= ' ')
    {
        if (!c)
            return NULL;            // end of file;
        data++;
    }

// skip // comments
    // Also may be Linux filename: //home/user/.legacy
    if ( script && (c == '/' && data[1] == '/'))
    {
        while (*data && *data != '\n')
            data++;
        goto skipwhite;
    }


// handle quoted strings specially
    if (c == '"')
    {
        data++;
        while ( len < COM_TOKEN_MAX-1 )
        {
            c = *data++;
#if !defined( __DJGPP__ )							
	    if (!c)
            {
              // NUL in the middle of a quoted string. Missing closing quote?
              CONS_Printf("Error: Quoted string ended prematurely.\n");
              goto term_done;
            }
#endif						
            if (c == '"') // closing quote
              goto term_done;

            if ( script && (c == '\\')) // c-like escape sequence
            {
              switch (*data)
              {
              case '\\':  // backslash
                com_token[len++] = '\\'; break;

              case '"':  // double quote
                com_token[len++] = '"'; break;

              case 't':  // tab
                com_token[len++] = '\t'; break;

              case 'n':  // newline
                com_token[len++] = '\n'; break;

              default:
                CONS_Printf("Error: Unknown escape sequence '\\%c'\n", *data);
                break;
              }

              data++;
              continue;
            }

            // normal char
            com_token[len++] = c;
        }
#if defined( __DJGPP__ )					
	    if (!c)
            {
	      // Move code because this fixed commandline eq "-timedemo demo2"
	      // NUL in the middle of a quoted string. Missing closing quote?
	      CONS_Printf("Error: Quoted string ended prematurely.\n");
	      goto term_done;
//	      return data;
            }
#endif						
    }

// parse single characters
    // Also ':' can appear in WIN path names
    if (script && (c=='{' || c=='}'|| c==')'|| c=='(' || c=='\'' || c==':'))
    {
      if( len >= COM_TOKEN_MAX-2 )  goto term_done;
      com_token[len++] = c;
      data++;
      goto term_done;
    }

// parse a regular word
    do
    {
      if( len >= COM_TOKEN_MAX-2 )  goto term_done;
      com_token[len++] = c;
      data++;
      c = *data;
      if (script && (c=='{' || c=='}'|| c==')'|| c=='(' || c=='\'' || c==':'))
        break;
    } while (c > ' ');

term_done:   
    com_token[len] = '\0';
    return data;
}
