#############################################################################
#
# $Id: Makefile 1135 2014-09-17 13:50:58Z wesleyjohnson $
# GNU Make makefile for Doom Legacy (legacy@newdoom.com)
#
# Copyright (C) 1998-2014 by Doom Legacy Team.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
#
#  The preferred system media interface is SDL.
#     Compile the generic SDL version with 'make' (default)
#     Add 'DEBUG=1' to generate a debugging executable.
#
#  Other options (possibly outdated):
#     Compile the Linux X11 version with 'make SMIF=LINUX_X11'
#     Compile the FreeBSD X11 version with 'make SMIF=FREEBSD_X11'
#     Compile the OS/2 version with  'make SMIF=OS2_NATIVE'
#     Compile assembly code for some draw functions with  'make USEASM=1'
#     but the assembly code is not often updated with the latest improvements.
#
# On first execution it will create a dummy make_options file and dirs.
# If you create a make_options file first, then "make dirs" also.
# To update dependency then "make depend", otherwise it is automatic.
# To compile, with options on make command:
#   >> make SMIF=SDL HAVE_MIXER=1
# Command line options have precedence over the make_options file.
# To have values in the make_options file override those on the make
# command line, use override:
#   override SMIF=SDL
#
# This uses GNU Make special commands.  It may work with other make programs,
# and has work-arounds in some places.  It has been tested with mingw32-make.
# If you have problems, please use GNU Make, or gmake.
#
# If you have a workaround for another make, then submit it as a bug fix.
# This makefile must work with Linux, BSD, Win, OS2, DOS, Mac, so Linux specific operations
# must be guarded and limited.  Generic solutions are preferred.
#
# commands:
#  all  :  compile executable (default)
#  dirs : create dep, objs, bin directories
#  clean : delete objs and bin
#  distclean : delete objs, bin, and make_options
#  wad  : generate legacy.wad
# developers commands:
#  disasm  : disasm of the exe
#  objdump_draw : dump draw functions to assembly
#  dll  : generate dll for X11
#############################################################################

# Debug enables, save on reinventing this everytime.
#DEBUG=1
#PROFILEMODE=1

# User tunable settings.
# Edit here, or in make_options, or set by command line SMIF=LINUX_X11
# Most can just uncomment an appropriate selection for your system.
# WIN32=1
# WIN98=1
# WINNT=1
# OS2=1
# DOS=1
# CC_MINGW=1
# CC_WATCOM=1
# CC_CLANG=1
# CC_ENVIRONMENT=1
# CC_EXPLICIT=clang
# MAC=1
# Mac compile using Linux SDL includes (MacPorts, Fink)
# MAC_FRAMEWORK uses the native Mac SDL setup.
# MAC_FRAMEWORK=1

# SMIF - System Media Interface (default=SDL)
#SMIF=SDL
#SMIF=LINUX_X11
#SMIF=FREEBSD_X11
#SMIF=WIN32_NATIVE
#SMIF=OS2_NATIVE
#SMIF=DJGPPDOS_NATIVE

# GGI option on X11
#X11_GGI=1

# SDL Mixer, to get music
#HAVE_MIXER=1

# CD-Music enable/disable, if you don't have the special libraries, etc.
#NOCDMUS=1
ifndef NOCDMUS
  CDMUS=1
endif
# Music options
#openserver5
# MUS_OS=SCOOS5
#unixware2
# MUS_OS=SCOUW2
#unixware7
# MUS_OS=SCOUW7
#ESD demon
# HAVE_ESD=1

# WIN_NATIVE enables
# For FMOD sound.  Can compile without it.
# HAVE_FMOD=1
# FMODINC="" alternative directory to find fmod.h and other includes
# FMODLIB="" alternative directory to find fmod libs
# DDINC="" alternative directory to find ddraw.h and other includes
# DDLIB="" alternative directory to find ddraw and other libs

# When SDL 1.3 is used, some stuff will break.  I don't know what yet.
# Reported that SDLMain is no longer required for Mac using SDL 1.3.
# SDL_1_3=1

# Developers with svn can enable this to have svn revision number in executable.
# Causes compile error message otherwise.
# Until can find test for presence of svn, this is best that can be done.
#SVN_ENABLE=1

# This Makefile may be used in the src directory, or outside it.
# During program development, it is much easier to put the Makefile in
# the working directories, cd to the directory, and invoke the make.
#MAKE_IN_SRC=1

# Invoke make with SRC= to compile other development directories.
ifndef SRC
  ifdef MAKE_IN_SRC
    # Default to the current directory.
    SRC=.
  else
    # Default source directory.
    SRC=src
  endif
endif
ifeq ("$(SRC)", "")
  # SRC is the current directory.
  SRC:=.
endif
ifeq ("$(SRC)", ".")
  # SRC is the current directory, src include is "-I.".
  SRC:=.
  SD:=
  SRCINC:=-I.
else
  # SRC in a sub-directory, src include is "-I$(SRC)".
  SD:=$(SRC)/
  SRCINC:=-I$(SRC)
endif

# Invoke make with BUILD= to compile to other build directories.
ifndef BUILD
#  BUILD:=build
  BUILD:=.
endif
ifeq ("$(BUILD)", ".")
  # BUILD== "." is the main directory.
  BUILD:=
endif
ifeq ("$(BUILD)", "")
  # BUILD== "" is the main directory.
  ifdef MAKE_IN_SRC
    BUILD_DIR:=../
  else
    BUILD_DIR:=
  endif
else
  # BUILD in a sub-directory.
  ifdef MAKE_IN_SRC
    BUILD_DIR:=../$(BUILD)/
  else
    BUILD_DIR:=$(BUILD)/
  endif
endif

# Invoke make with MAKE_OPTIONS= to specify another file or location.
ifndef MAKE_OPTIONS
  MAKE_OPTIONS = $(BUILD_DIR)make_options
endif

# Subdirectories for binaries and build intermediates
ifdef MAKE_IN_SRC
  BIN = ../bin
else
  BIN = bin
endif
O  = $(BUILD_DIR)objs
DD = $(BUILD_DIR)dep
WD = $(BUILD_DIR)wad


# Warning flags
WFLAGS=-Wall
#WFLAGS=-Wall -Wwrite-strings

# Optimization level, -O0 to -O5
OPTLEV=-O3
#OPTLEV=-O5

#Enable the x86 asm code (which is not always up to date).
# This is only useful if have an old compiler with bad optimization.
#USEASM=1

# Uncomment if you want to use the POLL_POINTER hack in X11
#POLL_POINTER=-DPOLL_POINTER

# Put user settings in this file, and they will be included with every
# invocation of make.
-include $(MAKE_OPTIONS)

# Doom Legacy is developed with GCC.  MINGW gets used for Win32.
# The behavior of the code under other compilers is uncertain.
# Define CC_EXPLICIT in make_options to override this setting.
#  CC_EXPLICT=my_compiler
# Define CC_ENVIRONMENT to allow an environment CC to specify the compiler.

ifdef CC_EXPLICIT
  CC_SELECT:=$(CC_EXPLICIT)
else
  # gcc or g++
  CC_SELECT:=gcc
  ifdef CC_WATCOM
    CC_SELECT:=WATCOMC
  endif
  ifdef CC_CLANG
    CC_SELECT:=clang
  endif
endif
ifdef CC_ENVIRONMENT
  # Give an environment CC precedence in specifing the compiler.
  CC ?= $(CC_SELECT)
else
  CC:=$(CC_SELECT)
endif

# Customize to your debugging environment.
ifdef DEBUG
ifndef DEBUGFLAGS
# Normal debug (-g), MACRO info (-g3), stabs or gdb debug info
#  DEBUGFLAGS=-g3 -DDEBUG_WINDOWED
  DEBUGFLAGS=-ggdb -DDEBUG_WINDOWED
  # Debuggers do not cope with optimization well, so DEBUG ignores OPTLEV.
  # To debug with optimization, include it here, (-O0, -O1, or $(OPTLEV)).
#  DEBUGFLAGS+=$(OPTLEV)
endif
# Set DEBUGLIBS to special debugging libraries
endif

# standard
#STD= -std=c99
  # not ready yet
  # fails because of alloca, it is not std under c99
  # std=c89, does not support // commments, no inline, no asm

# End of User tunable settings


# MAKECMDGOALS is in GNUMake, MinGW
ifdef MAKECMDGOALS
  ifneq ($(MAKECMDGOALS),distclean)
  ifneq ($(MAKECMDGOALS),clean)
  ifneq ($(MAKECMDGOALS),dirs)
  ifneq ($(MAKECMDGOALS),depend)
    # include deps for all except clean and dirs op
    INCLUDE_DEPS=1
  endif
  endif
  endif
  endif
  ifeq ($(MAKECMDGOALS),depend)
    FORCE_AUTO_DEP=1
  endif
else
  #default
  INCLUDE_DEPS=1
  FORCE_AUTO_DEP=1
endif


# Values changed by system detection
# file delete command, init value
RM=rm
# random stuff
NASMFORMAT=elf -DLINUX

ifdef WIN32
  DOSFILE=1
endif

ifdef WIN98
  WIN32=1
  DOSFILE=1
endif

ifdef WINNT
  WIN32=1
  DOSFILE=1
endif

ifdef OS2
  DOSFILE=1
endif

ifdef DOS
  DOSFILE=1
endif

# default system media interface
ifdef SMIF
  # specifc SMIF
  EXT_SMIF:=$(SMIF)
else
  SMIF=SDL
endif

# all OPTINC need to have -I. because of how includes are written
# however the order of -I may be important, so it is left to each.
OPTS:=
OPTINC:=
LIBS:=
LDFLAGS:=

#=================================
ifeq ($(SMIF), SDL)
# SDL is the default system media interface. This is what all modern systems should use.
# Requires SDL libs, and optionally SDL_mixer libs
# This uses files in sdl directory, and not those linux_x nor macos directories.

# interface directory (where you put all i_*.c)
  INTERFACE=sdl

  MAINOBJ:=i_main.o
  CDMUSOBJ:=i_cdmus.o
  SMIFOBJS:=i_system.o i_video.o i_sound.o i_net.o \
    hw_bsp.o hw_draw.o hw_light.o hw_main.o hw_md2.o hw_cache.o hw_trick.o \
    r_opengl.o ogl_sdl.o hwsym_sdl.o \
    filesrch.o dosstr.o endtxt.o

  DEP_HARD:=1
  DEP_OPENGL:=1

# platform
  ifdef WIN32
    # Windows (MinGW)
    EXENAME:=doomlegacy.exe
    OPTS:=-DWIN32
    ifdef WINNT
      # if have windows >= WinNT, and want large memory reports
      OPTS+=-DWIN_LARGE_MEM
    endif
    LIBS:=-lopengl32 -lglu32 -lwsock32 -lm
  else
    # default is Linux, for all unix SDL
    EXENAME:=doomlegacy
    OPTS:=-DLINUX
    LDFLAGS=-L/usr/X11R6/lib
    LIBS:=-lGL -lGLU -lm
    # -L/usr/X11R6/lib is needed by Linux 2.4 and others that still have
    # the GLU libraries in an X11 directory.
    # -lm is needed for pow, powf, and other MATH1 functions.
    SDL_CONFIG:=1
  endif

  ifdef SDL_CONFIG
    OPTINC+=$(shell sdl-config --cflags)
    LIBS+=$(shell sdl-config --libs)
  else
    # Extracted from sdl-config script
    ifdef CC_MINGW
      OPTS+=-D_GNU_SOURCE=1 -Dmain=SDL_main
      OPTINC+=-I/SDL/include/SDL
#      OPTINC+=-I/mingw/include/SDL
#      OPTINC+=-I/mingw/include
      LIBS+=-L/SDL/lib
#      LIBS+=-L/mingw/lib
#      LIBS+=-L/windows/system32
      LIBS+=-lsdl -lmingw32 -mwindows
      LIBS+=-lSDLmain -lSDL
    endif
    ifdef CC_WATCOM
      OPTS+=-D_GNU_SOURCE=1 -Dmain=SDL_main
      OPTINC+=-I/SDL/include/SDL
#     OPTINC+=-I/watcom/h/SDL
      LIBS+=-L/SDL/lib
#     LIBS+=-L/windows/system32
      LIBS+=-lsdl -lwatcom -mwindows
      LIBS+=-lSDLmain -lSDL
    endif
    ifdef MAC
    # Some Mac SDL downloads do not have sdl-config.
    # Because I do not know of a way for MAKE to test this, comment
    # out the SDL_CONFIG=1 above if you do not have sdl-config.
    # Put correct mac sdl-config info here (somebody).
      OPTS+=-D_GNU_SOURCE=1 -D_REENTRANT
      OPTINC+=-I/usr/include/SDL
      LIBS+=-L/usr/lib -Wl,-rpath,/usr/lib -lSDL -lpthread
    endif
  endif

  OPTS+=-DHWRENDER -DSDL
  OPTINC+=$(SRCINC)

  ifdef HAVE_MIXER
    OPTS+=-DHAVE_MIXER
    LIBS+=-lSDL_mixer 
  endif

  ifdef MAC
    # Objective-C main program is required for SDL 1.2.x
    # DoomLegacy has its own SDLmain source, but others may work,
    # though they would change the default directory according to .app.
    SMIFOBJS+=SDLmain.o
    LIBS+=-lobjc
    # Optional places for include and libraries, unique to Mac.
    # Enable MAC_FRAMEWORK if you really want to debug frameworks (I don't).
    ifdef MAC_FRAMEWORK
      CFLAGS += -DMAC_FRAMEWORK
      FRAMEWORK = -framework Cocoa -framework OpenGL -framework SDL
      # Do not remember which example had -F./osx/Frameworks      
      #      LDFLAGS += -F./osx/Frameworks $(FRAMEWORK) -framework Foundation
      LDFLAGS +=  $(FRAMEWORK) -framework Foundation
    endif
  endif


#=================================
else
ifeq ($(SMIF), LINUX_X11)
  # Requires X, and xshm or ggi libs
  INTERFACE=linux_x

  MUSSERV:=linux_x/musserv
  SNDSERV:=linux_x/sndserv
  
  DEP_HARD:=1
  DEP_X11:=1
  DEP_OPENGL:=1

  #use the x86 asm code
  #USEASM=1

  # Uncomment if you want to use the POLL_POINTER hack
  #POLL_POINTER:=-DPOLL_POINTER

  OPTS:=-DLINUX -DVID_X11 -DOLD_SOUND_DRIVER -DSNDSERV -DHWRENDER
  ifdef CDMUS
    OPTS+=-DMUSSERV
  endif
  
  # objs common to xshm and ggi, video is added later
  MAINOBJ:=i_main.o
  CDMUSOBJ:=i_cdmus.o
  SMIFOBJS:=i_system.o i_sound.o i_net.o \
    hw_bsp.o hw_draw.o hw_light.o hw_main.o hw_md2.o hw_cache.o hw_trick.o \
    r_opengl.o ogl_x11.o \
    searchp.o filesrch.o dosstr.o endtxt.o

  ifdef X11_GGI
    # GGI
    OPTS+=-DVID_GGI
    OPTINC:=$(SRCINC)
    LDFLAGS:=
    LIBS:=-lggi -lm
    # name of the exefile
    EXENAME:=llggidoom
    SFLAGS:=-g $(OPTS)
    SMIFOBJS+=i_video_ggi.o
  else
    # standard X11
    OPTS+=$(POLL_POINTER)
    OPTINC+=$(SRCINC) -I/usr/X11R6/include
    LDFLAGS:=-L/usr/X11R6/lib
    LIBS:=-lGL -lGLU -lXext -lX11 -lm -ldl -lXxf86vm

    #WITH_DGA=1
    ifdef WITH_DGA
      OPTS+=-DWITH_DGA
      LIBS+=-lXxf86dga -lXext
    endif #WITH_DGA

    # name of the exefile
    EXENAME:=llxdoom
    SFLAGS:=-g $(OPTS)
    SMIFOBJS+=i_video_xshm.o
  endif


#=================================
else
ifeq ($(SMIF), FREEBSD_X11)
  # Requires X11/Xlib, and xshm or ggi libs
  INTERFACE=linux_x

  MUSSERV=linux_x/musserv
  SNDSERV=linux_x/sndserv
  OSVERSION:=$(shell /sbin/sysctl -n kern.osreldate)
  OSMAJOR:=$(shell echo $(OSVERSION) | sed 's/.....$$//')

  DEP_HARD:=1
  DEP_X11:=1
  DEP_OPENGL:=1

  #use the x86 asm code
  #USEASM:=1

  # Uncomment if you want to use the POLL_POINTER hack
  #POLL_POINTER:=-DPOLL_POINTER

  OPTS:=-DLINUX -DVID_X11 -DFREEBSD -DOLD_SOUND_DRIVER -DSNDSERV -DHWRENDER
  # objs common to xshm and ggi, video is added later
  # Note: free-bsd version of cdmus
  MAINOBJ:=i_main.o
  CDMUSOBJ:=i_cdmus_fbsd.o
  SMIFOBJS:=i_system.o i_sound.o i_net.o \
    hw_bsp.o hw_draw.o hw_light.o hw_main.o hw_md2.o hw_cache.o hw_trick.o \
    searchp.o filesrch.o dosstr.o endtxt.o
 
  ifdef X11_GGI
    # GGI
    OPTS+=-DVID_GGI
    OPTINC+=$(SRCINC) -I/usr/local/include
    LDFLAGS:=
    LIBS:=-lggi -lm -lipx
    # name of the exefile
    EXENAME:=llggidoom
    SFLAGS:=-g $(OPTS)
    SMIFOBJS+=i_video_ggi.o
  else
    # standard X11
    OPTS+=$(POLL_POINTER)
    OPTINC+=$(SRCINC) -I/usr/X11R6/include
    LDFLAGS:=-L/usr/X11R6/lib
    LIBS:=-lXext -lX11 -lm -lXxf86vm -lipx -lkvm -pthread
    # name of the exefile
    EXENAME:=llxdoom
    SFLAGS:=-g $(OPTS)
    SMIFOBJS+=i_video_xshm.o
  endif

#=================================
else
ifeq ($(SMIF), OS2_NATIVE)
  #Requires OS2 libs, os2.h, os2me.h, fourcc.h, DIVE libs, and mmpm2 libs
  #Does not have cdmusic support
  DOSFILE=1
  INTERFACE=os2
  NASMFORMAT:=coff

#use the x86 asm code
  NASM:=nasm
#USEASM=1
  PROFILEMODE=1
# options (separate OS2 library and OS2_NATIVE video)
  OPTS:=-D__OS2__ -DOS2_NATIVE
  OPTINC+=-Zmt $(SRCINC) -I$(SD)os2

#  DEBUGLIBS:=
  LIBS:=-g $(INTERFACE)/I_pm.def -lmmpm2 -lsocket

  SFLAGS:=-g $(OPTS)

  MAINOBJ:=main.o
  CDMUSOBJ:=i_cdmus.o
  SMIFOBJS:=i_system.o i_video.o i_sound.o i_net.o i_dart.o i_dive.o I_pm.o \
    printf.o I_pm.res filesrch.o

# name of the exefile
  EXENAME:=legacy.exe


#=================================
else
ifeq ($(SMIF), WIN32_NATIVE)
  # Requires windows.h, windowsx.h, ddraw.h and ddraw libs, and
  # optionally fmod libs
  DOSFILE=1
  INTERFACE=win32
  NASMFORMAT:=coff

  DEP_HARD:=1
  DEP_DD:=1

  # DDINC="" alternative directory to find ddraw.h and other includes
  # DDLIB="" alternative directory to find ddraw and other libs
  ifndef DDINC
#    DDINC=-I\MINGW\MX80\include
  endif
  ifndef DDLIB
#    DDLIB=-L\MINGW\MX80\lib
  endif
  # FMODINC="" alternative directory to find fmod.h and other includes
  # FMODLIB="" alternative directory to find fmod libs
  ifdef HAVE_FMOD
    ifndef FMODINC
      FMODINC=-I\MINGW\fmod\include
    endif
    ifndef FMODLIB
      FMODLIB=-L\MINGW\fmod\lib
    endif
  endif

#use the x86 asm code
  #USEASM=1

# options (separate WIN32 library, and WIN_NATIVE video)
  OPTS:=-DWIN32 -DWIN_NATIVE
  OPTINC+=-g $(SRCINC)
  ifdef DDINC
    OPTINC+=$(DDINC)
  endif

#  DEBUGLIBS:=
  LIBS:=
  ifdef DDLIB
    LIBS+=$(DDLIB)
  endif
  ifdef CC_MINGW
    LIBS+=-lwinmm -lwsock32 -lgdi32 -lddraw -ldsound -ldinput -ldxguid
  else
    LIBS+=-lwinmm -lsocket -lddraw -ldsound -ldinput
  endif

  ifdef WINNT
    # if have windows >= WinNT, and want large memory reports
    OPTS+=-DWIN_LARGE_MEM
  endif

  ifdef HAVE_FMOD
    OPTS+=-DFMOD_SOUND
    ifdef FMODLIB
      LIBS+=$(FMODLIB) -lfmod
    endif
  endif

  SFLAGS:=

  MAINOBJ:=win_main.o
  SMIFOBJS:=win_sys.o win_vid.o win_snd.o win_net.o win_cd.o \
    filesrch.o fabdxlib.o mid2strm.o dx_error.o

# name of the exefile
  EXENAME:=legacy.exe

#=================================
else
ifeq ($(SMIF), DJGPPDOS_NATIVE)
  # Requires allegro libs, bcd libs (cd-music)
  RM:=del
  DOSFILE=1
  INTERFACE=djgppdos
  NASMFORMAT:=coff

#  DEP_HARD:=1

#use the x86 asm code
  #USEASM=1

# options
  OPTS:=-DPC_DOS
  OPTINC+=-g $(SRCINC)

#  DEBUGLIBS:=
  LIBS:=-lalleg -lsocket
  ifdef CDMUS
     LIBS+=-lbcd
  endif

  SFLAGS:=

  MAINOBJ:=i_main.o
  CDMUSOBJ:=i_cdmus.o
  SMIFOBJS:=i_system.o i_video.o vid_vesa.o i_sound.o i_net.o filesrch.o

# name of the exefile
  EXENAME:=legacy.exe
endif
endif
endif
endif
endif
endif
#=================================

# DOS/OS2/WIN32 file differences
ifdef DOSFILE
  RM:=del
  O_WIN:=$(subst /,\,$(O))
endif

OPTOBJS:=

# use assembly routines?
ifdef USEASM
   OPTOBJS+=tmap.o vid_copy.o
   OPTS+=-DUSEASM
endif


# CD-Music
ifdef CDMUS
   OPTOBJS+=$(CDMUSOBJ)
   OPTS+=-DCDMUS
endif

# compiler and linker flags
CFLAGS:=$(WFLAGS)

ifdef PROFILEMODE
# build with gprof profiling information
  CFLAGS+=-g -pg -fomit-frame-pointer
else
ifdef DEBUG
# No -fomit-frame-pointer because debugging on some machines depends upon it.
  #CFLAGS+=-g3 -O0
  CFLAGS+=$(DEBUGFLAGS) -fno-guess-branch-probability
  ifdef DEBUGLIBS
# Search debug libraries first, otherwise normal library
    LIBS:=$(DEBUGLIBS) $(LIBS)
  endif
else
# build a normal optimized version
  #CFLAGS+=-O3
  CFLAGS+=$(OPTLEV) -fomit-frame-pointer
endif
endif

CFLAGS+=-ffast-math -fno-strict-aliasing
# WIN98 linking needs LCFLAGS without -D flags and does not need includes
LCFLAGS:=$(CFLAGS)
CFLAGS+=$(OPTS) $(OPTINC)

ifdef MAC
else
  LDFLAGS+=-Xlinker --warn-common
endif

SNDSERV_EXTRAFLAGS:=
ifdef HAVE_ESD
  SNDSERV_EXTRAFLAGS:=HAVE_ESD=1
endif


# objs that are in main directories
MOBJS:=\
screen.o v_video.o \
r_draw.o r_plane.o r_segs.o r_sky.o r_things.o r_splats.o \
r_bsp.o r_data.o r_main.o \
z_zone.o \
p_sight.o p_mobj.o p_enemy.o p_user.o p_inter.o p_pspr.o \
p_lights.o p_ceilng.o p_doors.o p_plats.o p_floor.o p_spec.o \
p_switch.o p_genlin.o p_telept.o p_tick.o p_fab.o p_info.o p_setup.o \
p_map.o p_maputl.o \
p_heretic.o p_hsight.o \
p_chex.o \
sb_bar.o hu_stuff.o \
st_lib.o st_stuff.o \
t_array.o t_func.o t_oper.o t_parse.o t_prepro.o t_script.o t_spec.o t_vari.o \
sounds.o qmus2mid.o s_sound.o s_amb.o mserv.o \
b_game.o b_look.o b_node.o b_search.o \
g_state.o g_input.o g_game.o \
f_finale.o f_wipe.o \
wi_stuff.o \
am_map.o \
md5.o \
m_menu.o m_misc.o m_argv.o m_bbox.o m_fixed.o m_swap.o m_cheat.o m_random.o \
console.o command.o \
p_saveg.o \
tables.o info.o dstrings.o dehacked.o w_wad.o \
d_netcmd.o d_clisrv.o d_net.o d_netfil.o i_tcp.o \
d_items.o d_main.o

# combine all objs ( main, INTERFACE specific, asm, CDMUS )
OBJS:=$(addprefix $(O)/, $(SMIFOBJS) $(OPTOBJS) $(MOBJS))
MAINOBJ:=$(addprefix $(O)/, $(MAINOBJ))

#=======================================================
# rules
#=======================================================
# Do not have directories as prerequisites because their date is updated
# whenever a file is output, which fouls up the rules.

.PHONY : all dirs clean distclean asm dll wad

# prevent compile without make_options file
ifdef MAKE_OPTIONS_PRESENT

# ALL, compile and link
ifeq ($(SMIF), LINUX_X11)
all: $(BIN)/$(EXENAME) dll musserv sndserv

else
ifeq ($(SMIF), FREEBSD_X11)
all:	 $(BIN)/$(EXENAME) dll sndserv

else
all:	 $(BIN)/$(EXENAME)

endif
endif

endif


# File make_options does not exist on first make.
# Create dummy make_options file and dirs.
# Create dirs without triggering a remake of make_options
# (a PHONY always triggers).
$(MAKE_OPTIONS):
	@ echo "-------------------------------------------------------------------------------" \
	&& echo "Initializing DoomLegacy make" \
	&& echo "-------------------------------------------------------------------------------"
ifdef DOSFILE
	@echo # DoomLegacy Make options  > "$(MAKE_OPTIONS)"
else
	@echo "# DoomLegacy Make options"  > "$(MAKE_OPTIONS)"
endif
	@echo # >> "$(MAKE_OPTIONS)"
	@echo MAKE_OPTIONS_PRESENT=1  >> "$(MAKE_OPTIONS)"
ifdef WIN98
	@echo WIN98=1  >> "$(MAKE_OPTIONS)"
endif
ifdef WIN32
	@echo WIN32=1  >> "$(MAKE_OPTIONS)"
endif
ifdef WINNT
	@echo WINNT=1  >> "$(MAKE_OPTIONS)"
endif
ifdef DOS
	@echo DOS=1  >> "$(MAKE_OPTIONS)"
endif
ifdef OS2
	@echo OS2=1  >> "$(MAKE_OPTIONS)"
endif
ifdef EXT_SMIF
	@echo SMIF=$(EXT_SMIF)  >> "$(MAKE_OPTIONS)"
endif
	$(MAKE) dirs
	@ echo "-------------------------------------------------------------------------------" \
	&& echo "Edit $(MAKE_OPTIONS) file with OS selection, before running make again," \
	&& echo "or use make command line options every time." \
	&& echo "-------------------------------------------------------------------------------"
	stop  # stop make, by error if necessary


# Win32/OS2/DOS sees "/" as a switch, "\" is an escape to make
dirs :
ifdef DOSFILE
	@echo "Making dirs (WIN32/OS2/DOS)"
	mkdir "$(BIN)"
	mkdir "$(O)"
	mkdir "$(DD)"
else
	@echo "Making dirs (Linux/BSD)"
	mkdir -p $(BIN)
	mkdir -p $(O)
	mkdir -p $(DD)
endif


# Win32/OS2/DOS sees "/" as a switch, "\" is an escape to make
clean:
ifdef DOSFILE
	@echo "Removing dependencies"
	- $(RM) $(DD)\*.dep
	@echo "Removing obj  $(O_WIN)"
	- $(RM) $(O_WIN)\*.o
else
	@echo "Removing dependencies"
	- $(RM) $(DD)/*
	@echo "Removing obj  $(O)"
	- $(RM) $(O)/*.o
endif
ifeq ($(SMIF), LINUX_X11)
	- $(RM) $(O)/$(MUSSERV)/*/*.o
	- $(RM) $(O)/$(SNDSERV)/*/*.o
else
ifeq ($(SMIF), FREEBSD_X11)
	- $(RM) $(O)/$(MUSSERV)/*/*.o
	- $(RM) $(O)/$(SNDSERV)/*/*.o
endif
endif


# As all intermediates are kept outside of dist, there is nothing extra to do
# except to remove make_options
distclean: clean
ifdef DOSFILE
	- $(RM) $(MAKE_OPTIONS)
else
	- $(RM) $(MAKE_OPTIONS)
endif


.PHONY : versionstring
# This may fail because, (a) svn not installed, (b) not a svn directory.
# This compiles d_main a second time, with SVN_REV set.
versionstring:
ifdef SVN_ENABLE
	$(CC) -c $(CFLAGS) -DSVN_REV=\"`svn info | grep Revision | sed -e 's/Revision: //'`\" d_main.c -o $(O)/d_main.o
endif

# Maintenance Functions, mostly Linux only

# Make wad file
wad	:
ifdef WAD
	@echo "Building a new legacy.wad using an old version $(WAD)..."
ifdef DOSFILE
	@echo "Untested"
	@mkdir "$(WD)"
	$(BIN)\wadtool.exe -x $(WAD) -d $(WD)
	copy resources\* $(WD)
	$(BIN)\wadtool.exe -c $(BIN)\legacy.wad -d $(WD) resources\legacy.wad.inventory
else
	@mkdir -p $(WD)
	$(BIN)/wadtool.exe -x $(WAD) -d $(WD)
	cp resources/* $(WD)
	$(BIN)/wadtool.exe -c $(BIN)/legacy.wad -d $(WD) resources/legacy.wad.inventory
endif
	@echo "Finished building legacy.wad."
else
	@echo "Usage: make wad WAD=/path/to/old/legacy.wad"
endif


#make a disasm of the exe
disasm: $(BIN)/$(EXENAME)
	objdump -d $(BIN)/$(EXENAME) --no-show-raw-insn > $(BIN)/doomlegacy.s

# To dump draw functions to assembly
objdump_draw: $(O)/r_draw.o
	objdump -d $(O)/r_draw.o -l --source --no-show-raw-insn > $(BIN)/r_draw.s


# Make dll for X11
dll : $(O)/r_opengl.o $(O)/ogl_x11.o
	$(CC) -O3 $(OPTS) -o $(BIN)/r_opengl.so -shared -nostartfiles \
	  $(O)/r_opengl.o $(O)/ogl_x11.o \
	  -L/usr/X11R6/lib -L/usr/local/lib -lX11 -lXext -lGL -lGLU -lm


# Linux X11 sound and music servers
ifdef MAKE_IN_SRC
  export BASEOBJ:=../../$(O)/$(MUSSERV)
  export BASEBIN:=../../$(BIN)
else
  export BASEOBJ:=../../../$(O)/$(MUSSERV)
  export BASEBIN:=../../../$(BIN)
endif
export CC
export SMIF
ifdef MUS_OS
  export MUS_OS
endif
ifdef HAVE_ESD
  export HAVE_ESD
endif

musserv:
	cd $(SD)$(MUSSERV) && $(MAKE) dirs
	cd $(SD)$(MUSSERV) && $(MAKE)

sndserv:
	cd $(SD)$(SNDSERV) && $(MAKE) dirs
	cd $(SD)$(SNDSERV) && $(MAKE) $(SNDSERV_EXTRAFLAGS)


#=======================================================

# executable rule

$(BIN)/$(EXENAME): $(OBJS) $(MAINOBJ) versionstring
	@echo Linking...
ifdef DOSFILE
# MinGW on WIN98: linking cannot handle $(OPTS), use $(LCFLAGS) instead of $(CFLAGS)
	$(CC) $(LCFLAGS) $(LDFLAGS) -o $(BIN)/$(EXENAME) \
	$(MAINOBJ) $(OBJS) \
	$(LIBS)
else
# Linux/BSD
ifdef CC_EXE_LAST
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(MAINOBJ) \
	$(LIBS) -o $(BIN)/$(EXENAME)
else
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(BIN)/$(EXENAME) \
	$(MAINOBJ) $(OBJS) \
	$(LIBS)
endif
endif


#=======================================================

# obj rules

vpath %.o $(O)

#$(O)/*.o : doomdef.h

$(O)/r_draw.o : $(SD)r_draw.c $(SD)r_draw8.c $(SD)r_draw16.c $(SD)r_draw24.c $(SD)r_draw32.c


# common obj rules

$(O)/%.o: $(SD)%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/%.o: $(SD)$(INTERFACE)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/%.o: $(SD)hardware/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# OS2 rules
ifeq ($(SMIF), OS2_NATIVE)
$(O)/%.res: $(SD)os2/%.rc
	rc.exe -i $(SD)os2 -r $< $@
endif


# OpenGL rules
ifeq ($(SMIF), SDL)
$(O)/r_opengl.o: $(SD)hardware/r_opengl/r_opengl.c $(SD)hardware/r_opengl/r_opengl.h
	$(CC) $(CFLAGS) -c $< -o $@

ifdef MAC
# Mac must have Objective-C main for SDL
$(O)/SDLmain.o : $(SD)sdl/SDLmain.m
	$(CC) -x objective-c $(CFLAGS) $(FRAMEWORK) -c $< -o $@
endif

else 
# LINUX_X11, FREEBSD_X11 at least
$(O)/r_opengl.o: $(SD)hardware/r_opengl/r_opengl.c $(SD)hardware/r_opengl/r_opengl.h
	$(CC) $(CFLAGS) -shared -nostartfiles -c $< -o $@
endif


$(O)/ogl_x11.o:  $(SD)hardware/r_opengl/ogl_x11.c $(SD)hardware/r_opengl/r_opengl.h
	$(CC) $(CFLAGS) -shared -nostartfiles -I/usr/X11R6/include -c $< -o $@ 


# assembly rules
ifdef USEASM
$(O)/tmap.o: $(SD)tmap.nas
	nasm -o $@ -f $(NASMFORMAT) $<

$(O)/vid_copy.o: $(SD)vid_copy.s
	$(CC) $(OPTS) $(OPTINC) $(SFLAGS) -x assembler-with-cpp -c $< -o $@
endif


# stop searches for re-building source files

*.c :
	@:

*/*.c :
	@:

hardware/*/*.c :
	@:

hardware/*/*/*.c :
	@:

*.h :
	@:

*/*.h :
	@:

hardware/*/*.h :
	@:

hardware/*/*/*.h :
	@:

# Do not remake makefile

Makefile:
	@:

#=======================================================
#[WDJ] Dependencies, replaces makedep.mak, which was unmaintained.

ifndef FORCE_AUTO_DEP
# This PHONY disables automatic dep file generation when a file changes.
# Define FORCE_AUTO_DEP to have automatic dep file generation.
.PHONY : $(DD)/*.dep
endif

.PHONY : depend
depend:  $(DD)/main1.dep $(DD)/main3.dep $(DD)/main4.dep $(DD)/main5.dep \
 $(DD)/main_d.dep $(DD)/main_m.dep \
 $(DD)/main_p1.dep $(DD)/main_p2.dep $(DD)/main_p3.dep $(DD)/main_ph.dep \
 $(DD)/main_r.dep $(DD)/main_t.dep

$(DD)/main1.dep : $(SD)a*.c $(SD)b*.c $(SD)c*.c
$(DD)/main3.dep : $(SD)q*.c $(SD)s*.c
$(DD)/main4.dep : $(SD)f*.c $(SD)g*.c $(SD)h*.c
$(DD)/main5.dep : $(SD)v*.c $(SD)z*.c $(SD)i*.c
$(DD)/main_d.dep : $(SD)d*.c
$(DD)/main_m.dep : $(SD)m*.c
$(DD)/main_p1.dep : $(SD)p_c*.c $(SD)p_d*.c $(SD)p_e*.c $(SD)p_f*.c
$(DD)/main_p2.dep : $(SD)p_g*.c $(SD)p_i*.c $(SD)p_l*.c $(SD)p_m*.c
$(DD)/main_p3.dep : $(SD)p_p*.c $(SD)p_s*.c $(SD)p_t*.c $(SD)p_u*.c
$(DD)/main_ph.dep : $(SD)p_h*.c
$(DD)/main_r.dep : $(SD)r*.c
$(DD)/main_t.dep : $(SD)t*.c
# none: e*.c j*.c k*.c l*.c n*.c o*.c

ifdef INTERFACE
depend: $(DD)/$(INTERFACE).dep
endif
# $(INTERFACE).dep : $(SD)$(INTERFACE)/*.c
$(DD)/djgppdos.dep : $(SD)djgppdos/*.c
$(DD)/linux_x.dep : $(SD)linux_x/*.c $(SD)linux_x/musserv/*.c $(SD)linux_x/sndserv/*.c
$(DD)/os_2.dep : $(SD)os2/*.c
$(DD)/macos.dep : $(SD)macos/*.c
$(DD)/sdl.dep : $(SD)sdl/*.c
$(DD)/win32.dep : $(SD)win32/*.c

ifdef DEP_HARD
depend: $(DD)/hard1.dep
$(DD)/hard1.dep : $(SD)hardware/*.c
endif

ifdef DEP_OPENGL
depend: $(DD)/hardgl.dep
$(DD)/hardgl.dep : $(SD)hardware/r_opengl/r_opengl.c
endif

ifdef DEP_DD
# Direct-draw dependent dep.
depend: $(DD)/hardwin.dep
$(DD)/hardwin.dep : $(SD)hardware/r_d3d/*.cpp
$(DD)/hardwin.dep : $(SD)hardware/s_ds3d/*.c
$(DD)/hardwin.dep : $(SD)hardware/r_glide/*.c
endif

ifdef DEP_MINIGL
depend: $(DD)/hardmini.dep
$(DD)/hardmini.dep : $(SD)hardware/r_minigl/*.c
endif

ifdef DEP_X11
depend: $(DD)/hardx11.dep
$(DD)/hardx11.dep : $(SD)hardware/r_opengl/ogl_x11.c
$(DD)/hardx11.dep : $(SD)hardware/r_opengl/r_opengl.c
endif

FIXDEP=1

%.dep :
	@echo "Making dependencies $(@F)"
ifdef DOSFILE
	$(CC) $(CFLAGS) -MM $^ > $(DD)\$(@F)
else
	$(CC) $(CFLAGS) -MM $^ > $(DD)/$(@F)
ifdef FIXDEP	
	sed -e "s!^[a-zA-Z0-9_]*\.o:!\$$\(O)/&!" $(DD)/$(@F) > $(DD)/sed.dep
	mv $(DD)/sed.dep $(DD)/$(@F)
endif
endif


# Stop update of include files when clean
ifdef MAKE_OPTIONS_PRESENT
ifdef INCLUDE_DEPS
include $(DD)/main1.dep
include $(DD)/main3.dep
include $(DD)/main4.dep
include $(DD)/main5.dep
include $(DD)/main_d.dep
include $(DD)/main_p1.dep
include $(DD)/main_p2.dep
include $(DD)/main_p3.dep
include $(DD)/main_ph.dep
include $(DD)/main_m.dep
include $(DD)/main_r.dep
include $(DD)/main_t.dep
include $(DD)/hard1.dep
ifdef INTERFACE
 include $(DD)/$(INTERFACE).dep
endif
ifdef DEP_OPENGL
 include $(DD)/hardgl.dep
endif
ifdef DEP_DD
 include $(DD)/hardwin.dep
endif
ifdef DEP_MINIGL
 include $(DD)/hardmini.dep
endif
ifdef DEP_X11
 include $(DD)/hardx11.dep
endif
endif
endif

#############################################################
#
#############################################################
