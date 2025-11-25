#############################################################################
#
# $Id$
# GNU Make makefile for Doom Legacy (legacy@newdoom.com)
#
# Copyright (C) 1998-2013 by Doom Legacy Team.
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
# To have values in the make_options file override those on make command line use override:
#   override SMIF=SDL
#
# This uses GNU Make special commands.  It may work with other make programs, and has
# work-arounds in some places.  It has been tested with mingw32-make.
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
# OS2=1
# DOS=1
# CC_MINGW=1
# CC_WATCOM=1

# SMIF - System Media Interface (default=SDL)
#SMIF=SDL
#SMIF=LINUX_X11
#SMIF=FREEBSD_X11
#SMIF=OS2_NATIVE
#SMIF=DJGPPDOS_NATIVE

# GGI option on X11
#X11_GGI=1

# SDL Mixer, to get music
#HAVE_MIXER=1

# CD-Music enable/disable, if you don't have the special libraries, etc.
#ifndef NOCDMUS
  CDMUS=1
#endif
# Music options
#openserver5
# MUS_OS=SCOOS5
#unixware2
# MUS_OS=SCOUW2
#unixware7
# MUS_OS=SCOUW7
#ESD demon
# HAVE_ESD=1

# Developers with svn can enable this to have svn revision number in executable.
# Causes compile error message otherwise.
# Until can find test for presence of svn, this is best that can be done.
#SVN_ENABLE=1

# Subdirectories for objects and binaries
O=../objs
BIN=../bin

# Warning flags
WFLAGS=-Wall
#WFLAGS=-Wall -Wwrite-strings

# Optimization level, -O0 to -O5
OPTLEV=-O3
#OPTLEV=-O5

#use the x86 asm code (not always up to date)
# only useful if have old compiler with bad optimization
#USEASM=1

# Uncomment if you want to use the POLL_POINTER hack in X11
#POLL_POINTER=-DPOLL_POINTER

# gcc or g++
CC=gcc
ifdef CC_WATCOM
  CC=WATCOMC
endif

# Put user settings in this file, and they will be included with every invocation
include ../make_options

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

OPTS:=
OPTINC:=
LIBS:=
LDFLAGS:=

#=================================
ifeq ($(SMIF), SDL)
# SDL is the default system media interface. This is what all modern systems should use.
  # Requires SDL libs, and optionally SDL_mixer libs

# interface directory (where you put all i_*.c)
  INTERFACE=sdl

  MAINOBJ:=i_main.o
  CDMUSOBJ:=i_cdmus.o
  SMIFOBJS:=i_system.o i_video.o i_sound.o i_net.o filesrch.o dosstr.o endtxt.o \
    hw_bsp.o hw_draw.o hw_light.o hw_main.o hw_md2.o hw_cache.o hw_trick.o \
    r_opengl.o ogl_sdl.o hwsym_sdl.o

# platform
  ifdef WIN32
    # Windows (MinGW)
    EXENAME:=doomlegacy.exe
    OPTS:=-DWIN32
    # if have windows >= WinNT, and want large memory reports
#    OPTS+=-DWIN_LARGE_MEM
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
#      OPTINC+=-"I\SDL\include\SDL"
      OPTINC+=-"I/SDL/include/SDL"
#      OPTINC+=-I/mingw/include/SDL
      LIBS+=-L/SDL/lib
#      LIBS+=-L/mingw/lib
#      LIBS+=-L/windows/system32
      LIBS+=-lsdl -lmingw32 -mwindows
    endif
    ifdef CC_WATCOM
      OPTINC+=-I/SDL/include/SDL
#     OPTINC+=-I/watcom/h/SDL
      LIBS+=-L/SDL/lib
#     LIBS+=-L/windows/system32
      LIBS+=-lsdl -lwatcom -mwindows
    endif
    OPTS+=-D_GNU_SOURCE=1 -Dmain=SDL_main
    LIBS+=-lSDLmain -lSDL
  endif

  OPTS+=-DHWRENDER -DSDL
  OPTINC+=-I.

  ifdef HAVE_MIXER
    OPTS+=-DHAVE_MIXER
    LIBS+=-lSDL_mixer 
  endif



#=================================
else
ifeq ($(SMIF), LINUX_X11)
  # Requires X, and xshm or ggi libs
  INTERFACE=linux_x

  MUSSERV:=linux_x/musserv
  SNDSERV:=linux_x/sndserv

  #use the x86 asm code
  #USEASM=1

  # Uncomment if you want to use the POLL_POINTER hack
  #POLL_POINTER:=-DPOLL_POINTER

  OPTS:=-DLINUX -DOLD_SOUND_DRIVER -DSNDSERV -DHWRENDER
  ifdef CDMUS
    OPTS+=-DMUSSERV
  endif
  # objs common to xshm and ggi, video is added later
  MAINOBJ:=i_main.o
  CDMUSOBJ:=i_cdmus.o
  SMIFOBJS:=i_system.o i_sound.o i_net.o searchp.o filesrch.o dosstr.o endtxt.o \
    hw_bsp.o hw_draw.o hw_light.o hw_main.o hw_md2.o hw_cache.o hw_trick.o \
    r_opengl.o ogl_x11.o

  ifdef X11_GGI
    # GGI
    OPTS+=-DVID_GGI
    OPTINC:=-I.
    LDFLAGS:=
    LIBS:=-lggi -lm
    # name of the exefile
    EXENAME:=llggidoom
    SFLAGS:=-g $(OPTS)
    SMIFOBJS+=i_video_ggi.o
  else
    # standard X11
    OPTS+=$(POLL_POINTER)
    OPTINC+=-I. -I/usr/X11R6/include
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

  #use the x86 asm code
  #USEASM:=1

  # Uncomment if you want to use the POLL_POINTER hack
  #POLL_POINTER:=-DPOLL_POINTER

  OPTS:=-DLINUX -DFREEBSD -DOLD_SOUND_DRIVER -DSNDSERV -DHWRENDER
  # objs common to xshm and ggi, video is added later
  # Note: free-bsd version of cdmus
  MAINOBJ:=i_main.o
  CDMUSOBJ:=i_cdmus_fbsd.o
  SMIFOBJS:=i_system.o i_sound.o i_net.o searchp.o filesrch.o dosstr.o endtxt.o \
    hw_bsp.o hw_draw.o hw_light.o hw_main.o hw_md2.o hw_cache.o hw_trick.o

  ifdef X11_GGI
    # GGI
    OPTS+=-DVID_GGI
    OPTINC+=-I. -I/usr/local/include
    LDFLAGS:=
    LIBS:=-lggi -lm -lipx
    # name of the exefile
    EXENAME:=llggidoom
    SFLAGS:=-g $(OPTS)
    SMIFOBJS+=i_video_ggi.o
  else
    # standard X11
    OPTS+=$(POLL_POINTER)
    OPTINC+=-I. -I/usr/X11R6/include
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
# options
  OPTS:=-D__OS2__
  OPTINC:=-Zmt -I.. -I. -Ios2

#  DEBUGLIBS:=-lmmpm2 -lsocket -g
  LIBS:=$(INTERFACE)/I_pm.def -lmmpm2 -lsocket -g

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
  # Requires windows.h, windowsx.h, ddraw.h and ddraw libs, fmod libs
  DOSFILE=1
  INTERFACE=win32
  NASMFORMAT:=coff

#use the x86 asm code
  #USEASM=1

# options
  OPTS:=-DWIN32
  OPTINC:=-g

#  DEBUGLIBS:=-lfmod -lwinmm -lsocket
  LIBS:=-lfmod -lwinmm -lsocket

  SFLAGS:=

  MAINOBJ:=win_main.o
  SMIFOBJS:=win_sys.o win_vid.o win_snd.o win_net.o win_cd.o filesrch.o

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

#use the x86 asm code
  #USEASM=1

# options
  OPTS:=-DPC_DOS
  OPTINC:=-g

#  DEBUGLIBS:=-lalleg -lsocket
  LIBS:=-lalleg -lsocket
  ifdef CDMUS
     LIBS+=-lbcd
#     DEBUGLIBS+=-lbcd
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
tables.o info.o dstrings.o \
screen.o v_video.o \
z_zone.o \
r_draw.o r_plane.o r_segs.o r_sky.o r_things.o r_splats.o r_bsp.o r_data.o r_main.o \
p_ceilng.o p_doors.o p_enemy.o p_floor.o p_genlin.o p_info.o p_inter.o p_lights.o \
p_map.o p_maputl.o p_plats.o p_pspr.o p_setup.o p_sight.o p_spec.o \
p_switch.o p_mobj.o p_telept.o p_tick.o p_user.o p_fab.o \
p_hsight.o \
p_heretic.o \
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
d_netcmd.o d_clisrv.o d_net.o d_netfil.o i_tcp.o \
m_menu.o m_misc.o m_argv.o m_bbox.o m_fixed.o m_swap.o m_cheat.o m_random.o \
console.o command.o \
p_saveg.o \
dehacked.o \
w_wad.o \
d_items.o d_main.o

# combine all objs ( main, INTERFACE specific, asm, CDMUS )
OBJS:=$(addprefix $(O)/, $(SMIFOBJS) $(OPTOBJS) $(MOBJS))
MAINOBJ:=$(addprefix $(O)/, $(MAINOBJ))

#=======================================================
# rules
#=======================================================
# Do not have directories as prerequisites because their date is updated whenever
# a file is output, which fouls up the rules.

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


# File ../make_options does not exist on first make
# create dummy make_options file and dirs
# create dirs without triggering a remake of make_options (a PHONY always triggers)
../make_options :
	@ echo "-------------------------------------------------------------------------------" \
	&& echo "Initializing DoomLegacy make" \
	&& echo "-------------------------------------------------------------------------------"
ifdef DOSFILE
	@echo # DoomLegacy Make options  > "../make_options"
else
	@echo \# DoomLegacy Make options  > "../make_options"
endif
	@echo # >> "../make_options"
	@echo MAKE_OPTIONS_PRESENT=1  >> "../make_options"
ifdef WIN98
	@echo WIN98=1  >> "../make_options"
endif
ifdef WIN32
	@echo WIN32=1  >> "../make_options"
endif
ifdef DOS
	@echo DOS=1  >> "../make_options"
endif
ifdef OS2
	@echo OS2=1  >> "../make_options"
endif
ifdef EXT_SMIF
	@echo SMIF=$(EXT_SMIF)  >> "../make_options"
endif
	$(MAKE) dirs
	@ echo "-------------------------------------------------------------------------------" \
	&& echo "Edit ..\make_options file with OS selection, before running make again," \
	&& echo "or use make command line options every time." \
	&& echo "-------------------------------------------------------------------------------"
	stop  # stop make, by error if necessary


# Win32/OS2/DOS sees "/" as a switch, "\" is an escape to make
dirs :
ifdef DOSFILE
	@echo "Making dirs (WIN32/OS2/DOS)"
	mkdir "$(BIN)"
	mkdir "../objs"
	mkdir "../dep"
else
	@echo "Making dirs (Linux/BSD)"
	mkdir -p $(BIN)
	mkdir -p ../objs
	mkdir -p ../dep
endif


# Win32/OS2/DOS sees "/" as a switch, "\" is an escape to make
clean:
ifdef DOSFILE
	@echo "Removing dep"
	- $(RM) ..\dep\*.dep
	@echo "Removing obj  $(O_WIN)"
	- $(RM) $(O_WIN)\*.o
else
	@echo "Removing dep"
	- $(RM) ../dep/*
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
# except to remove ../make_options
distclean: clean
ifdef DOSFILE
	- $(RM) ..\make_options
else
	- $(RM) ../make_options
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
	- mkdir "..\wad"
	cd "..\wad" ; \
	&& "..\..\common\bin\wadtool" -x $(WAD) ; \
	&& copy "..\trunk\resources\*" . ; \
	&& "..\..\common\bin\wadtool" -c legacy.wad legacy.wad.inventory ; \
	&& move legacy.wad $(BIN) ; \
	&& $(RM) *.txt *.h *.lmp
else
	- mkdir -p "../wad"
	cd ../wad ; \
	../../common/bin/wadtool -x $(WAD) ; \
	cp ../trunk/resources/* . ; \
	../../common/bin/wadtool -c legacy.wad legacy.wad.inventory ; \
	mv legacy.wad $(BIN) ; \
	rm *.txt *.h *.lmp
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
	$(CC) -O3 -o $(BIN)/r_opengl.so -shared -nostartfiles $(O)/r_opengl.o $(O)/ogl_x11.o -L/usr/X11R6/lib -L/usr/local/lib -lX11 -lXext -lGL -lGLU -lm


# Linux X11 sound and music servers
export BASEOBJ:=$(O)/$(MUSSERV)
export BASEBIN:=$(BIN)
export SMIF
ifdef MUS_OS
  export MUS_OS
endif
ifdef HAVE_ESD
  export HAVE_ESD
endif

musserv:
	cd $(MUSSERV) && $(MAKE) dirs
	cd $(MUSSERV) && $(MAKE)

sndserv:
	cd $(SNDSERV) && $(MAKE) dirs
	cd $(SNDSERV) && $(MAKE) $(SNDSERV_EXTRAFLAGS)


#=======================================================

# executable rule

$(BIN)/$(EXENAME): $(OBJS) $(MAINOBJ) versionstring
	@echo Linking...
ifdef DOSFILE
# MinGW on WIN98: linking cannot handle $(OPTS), use $(LCFLAGS) instead of $(CFLAGS)
	$(CC) $(LCFLAGS) $(LDFLAGS) \
	$(OBJS) $(MAINOBJ) \
	$(LIBS) -o $(BIN)/$(EXENAME)
else
# Linux/BSD
ifdef CC_EXE_FIRST
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(BIN)/$(EXENAME) \
	$(MAINOBJ) $(OBJS) \
	$(LIBS)
else
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(MAINOBJ) \
	$(LIBS) -o $(BIN)/$(EXENAME)
endif
endif


#=======================================================

# obj rules

vpath %.o $(O)

$(O)/*.o : doomdef.h

$(O)/r_draw.o : r_draw.c r_draw8.c r_draw16.c r_draw24.c r_draw32.c


# common obj rules

$(O)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/%.o: $(INTERFACE)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/%.o: hardware/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# OS2 rules
ifeq ($(SMIF), OS2_NATIVE)
$(O)/%.res: os2/%.rc
	rc.exe -i .\\os2 -r $< $@
endif


# OpenGL rules
ifeq ($(SMIF), SDL)
$(O)/r_opengl.o: hardware/r_opengl/r_opengl.c hardware/r_opengl/r_opengl.h
	$(CC) $(CFLAGS) -c $< -o $@

else 
# LINUX_X11, FREEBSD_X11 at least
$(O)/r_opengl.o: hardware/r_opengl/r_opengl.c hardware/r_opengl/r_opengl.h
	$(CC) $(CFLAGS) -shared -nostartfiles -c $< -o $@
endif


$(O)/ogl_x11.o:  hardware/r_opengl/ogl_x11.c hardware/r_opengl/r_opengl.h
	$(CC) $(CFLAGS) -shared -nostartfiles -I/usr/X11R6/include -c $< -o $@ 


# assembly rules
ifdef USEASM
$(O)/tmap.o: tmap.nas
	nasm -o $@ -f $(NASMFORMAT) $<

$(O)/vid_copy.o: vid_copy.s
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
.PHONY : ../dep/*.dep
endif

.PHONY : depend  main_depend  interface_depend
depend:  ../dep/main1.dep ../dep/main3.dep ../dep/main4.dep ../dep/main5.dep \
../dep/main_d.dep ../dep/main_m.dep ../dep/main_p.dep \
../dep/main_r.dep ../dep/main_t.dep \
../dep/hard1.dep ../dep/hard2.dep ../dep/$(INTERFACE).dep

../dep/main1.dep : a*.c b*.c c*.c
../dep/main3.dep : q*.c s*.c
../dep/main4.dep : f*.c g*.c h*.c
../dep/main5.dep : v*.c z*.c i*.c
../dep/main_d.dep : d*.c
../dep/main_m.dep : m*.c
../dep/main_p.dep : p*.c
../dep/main_r.dep : r*.c
../dep/main_t.dep : t*.c
# none: e*.c j*.c k*.c l*.c n*.c o*.c
../dep/hard1.dep : hardware/*.c
../dep/hard2.dep : hardware/*/*.c
# $(INTERFACE).dep : $(INTERFACE)/*.c
../dep/djgppdos.dep : djgppdos/*.c
../dep/linux_x.dep : linux_x/*.c linux_x/musserv/*.c linux_x/sndserv/*.c
../dep/os_2.dep : os2/*.c
../dep/macos.dep : macos/*.c
../dep/sdl.dep : sdl/*.c
../dep/win32.dep : win32/*.c

FIXDEP=1

%.dep :
	@echo "Making dependencies $(@F)"
ifdef DOSFILE
	$(CC) $(CFLAGS) -MM $^ > ..\dep\$(@F)
else
	$(CC) $(CFLAGS) -MM $^ > ../dep/$(@F)
ifdef FIXDEP	
	sed -e "s!^[a-zA-Z0-9_]*\.o:!\$$\(O)/&!" ../dep/$(@F) > ../dep/sed.dep
	mv ../dep/sed.dep ../dep/$(@F)
endif
endif


# Stop update of include files when clean
ifdef MAKE_OPTIONS_PRESENT
ifdef INCLUDE_DEPS
include ../dep/main1.dep
include ../dep/main3.dep
include ../dep/main4.dep
include ../dep/main5.dep
include ../dep/main_d.dep
include ../dep/main_p.dep
include ../dep/main_m.dep
include ../dep/main_r.dep
include ../dep/main_t.dep
include ../dep/hard1.dep
include ../dep/hard2.dep
include ../dep/$(INTERFACE).dep
endif
endif

#############################################################
#
#############################################################
