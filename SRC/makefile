#############################################################################
#
# $Id: Makefile 949 2012-07-03 19:19:51Z wesleyjohnson $
# GNU Make makefile for Doom Legacy (legacy@newdoom.com)
#
# Copyright (C) 1998-2011 by Doom Legacy Team.
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
#     Add 'DEBUG=1' for debugging information.
#
#  Other options (possibly outdated):
#     Compile the Linux X11 version with 'make SMIF=LINUX_X11'
#     Compile the FreeBSD X11 version with 'make SMIF=FREEBSD_X11'
#     Compile the OS/2 version with  'make SMIF=OS2_NATIVE'
#
#############################################################################

# User tunable settings.  Edit here or set by command line SMIF=LINUX_X11
# Most can just uncomment an appropriate selection for your system.
# WIN32=1
# CC_MINGW=1
# CC_WATCOM=1

# SMIF - System Media Interface (default=SDL)
#SMIF=SDL
#SMIF=LINUX_X11
#SMIF=FREEBSD_X11
#SMIF=OS2_NATIVE
#SMIF=DJGPPDOS_NATIVE

# SDL Mixer, to get music
HAVE_MIXER=1

# CD-Music enable/disable, if you don't have the special libraries, etc.
#ifndef NOCDMUS
CDMUS=1
#endif

# Developers with svn can enable this to have svn revision number in executable.
# Causes compile error message otherwise.
# Until can find test for presence of svn, this is best that can be done.
#SVN_ENABLE=1

# Subdirectories for objects and binaries
O=../objs
BIN=../bin

# Warning flags
WFLAGS = -Wall # -Wwrite-strings

# Optimization level, -O0 to -O5
OPTLEV = -O3
#OPTLEV = -O5

# Debug enables, save on reinventing this everytime.
#DEBUG=1
#PROFILEMODE=1
# Customize to your debugging environment.
ifdef DEBUG
# Normal debug (-g), MACRO info (-g3), stabs or gdb debug info
#  DEBUGFLAGS:= -g3 -DDEBUG_WINDOWED
  DEBUGFLAGS:= -ggdb -DDEBUG_WINDOWED
  # Debuggers do not cope with optimization well, so DEBUG ignores OPTLEV.
  # To debug with optimization, include it here, (-O0, -O1, or $(OPTLEV)).
#  DEBUGFLAGS+= $(OPTLEV)
endif

# standard
#STD= -std=c99
  # not ready yet
  # fails because of alloca, it is not std under c99
  # std=c89, does not support // commments, no inline, no asm

# gcc or g++
CC=gcc
ifdef CC_WATCOM
  CC=WATCOMC
endif

# End of User tunable settings

# Values changed by system detection
# file delete command, init value
RM=rm
# random stuff
NASMFORMAT=elf -DLINUX

ifdef WIN32
  RM=del
endif

# default system media interface
ifndef SMIF
  SMIF = SDL
endif

#=================================
ifeq ($(SMIF), SDL)
# SDL is the default system media interface. This is what all modern systems should use.
  # Requires SDL libs, and optionally SDL_mixer libs

# name of the executable
  EXENAME := doomlegacy

# interface directory (where you put all i_*.c)
  INTERFACE=sdl

  MAINOBJ := $(O)/i_main.o
  CDMUSOBJ := $(O)/i_cdmus.o
  OBJS := $(O)/i_system.o $(O)/i_video.o $(O)/i_sound.o $(O)/i_net.o \
    $(O)/filesrch.o $(O)/dosstr.o $(O)/endtxt.o \
    $(O)/hw_bsp.o $(O)/hw_draw.o $(O)/hw_light.o $(O)/hw_main.o \
    $(O)/hw_md2.o $(O)/hw_cache.o $(O)/hw_trick.o \
    $(O)/r_opengl.o $(O)/ogl_sdl.o $(O)/hwsym_sdl.o

# platform
  ifdef WIN32
    # Windows (MinGW)
    EXENAME := $(EXENAME).exe
    OPTS := -DWIN32
    # if have windows >= WinNT, and want large memory reports
#    OPTS += -DWIN_LARGE_MEM
    LIBS := -lopengl32 -lglu32 -lwsock32 -lm
  else
    # default is Linux, for all unix SDL
    OPTS := -DLINUX
    LDFLAGS=-L/usr/X11R6/lib
    LIBS := -lGL -lGLU -lm
    # -L/usr/X11R6/lib is needed by Linux 2.4 and others that still have
    # the GLU libraries in an X11 directory.
    # -lm is needed for pow, powf, and other MATH1 functions.
    SDL_CONFIG=1
  endif

  ifdef SDL_CONFIG
    OPTS += $(shell sdl-config --cflags)
    LIBS += $(shell sdl-config --libs)
  else
    # Extracted from sdl-config script
    ifdef CC_MINGW
      OPTS += -I\SDL\include\SDL
#      OPTS += -I/mingw/include/SDL
      LIBS += -L/SDL/lib
#      LIBS += -L/mingw/lib
#      LIBS += -L/windows/system32
      LIBS += -lsdl -lmingw32 -mwindows
    endif
    ifdef CC_WATCOM
      OPTS += -I/SDL/include/SDL
#     OPTS += -I/watcom/h/SDL
      LIBS += -L/SDL/lib
#     LIBS += -L/windows/system32
      LIBS += -lsdl -lwatcom -mwindows
    endif
    OPTS += -D_GNU_SOURCE=1 -Dmain=SDL_main
    LIBS += -lSDLmain -lSDL
  endif

  OPTS += -DHWRENDER -DSDL -I.

  ifdef HAVE_MIXER
    OPTS += -DHAVE_MIXER
    LIBS += -lSDL_mixer 
  endif



#=================================
else
ifeq ($(SMIF), LINUX_X11)
  # Requires X, and xshm or ggi libs
  INTERFACE=linux_x

  MUSSERV=linux_x/musserv
  SNDSERV=linux_x/sndserv

  #use the x86 asm code
  #USEASM=1

  # Uncomment if you want to use the POLL_POINTER hack
  #POLL_POINTER=-DPOLL_POINTER

  OPTS=-DLINUX -DOLD_SOUND_DRIVER -DSNDSERV -DHWRENDER
  ifdef CDMUS
    OPTS += -DMUSSERV
  endif
  # objs common to xshm and ggi, video is added later
  MAINOBJ := $(O)/i_main.o
  CDMUSOBJ := $(O)/i_cdmus.o
  OBJS=$(O)/i_system.o $(O)/i_sound.o $(O)/i_net.o \
    $(O)/searchp.o $(O)/filesrch.o $(O)/dosstr.o $(O)/endtxt.o \
    $(O)/hw_bsp.o $(O)/hw_draw.o $(O)/hw_light.o $(O)/hw_main.o \
    $(O)/hw_md2.o $(O)/hw_cache.o $(O)/hw_trick.o \
    $(O)/r_opengl.o $(O)/ogl_x11.o

  X=1

  ifdef X
    OPTS := $(OPTS) $(POLL_POINTER) -I. -I/usr/X11R6/include
    LDFLAGS=-L/usr/X11R6/lib
    LIBS=-lGL -lGLU -lXext -lX11 -lm -ldl -lXxf86vm

    #WITH_DGA=1
    ifdef WITH_DGA
      OPTS += -DWITH_DGA
      LIBS += -lXxf86dga -lXext
    endif #WITH_DGA

# name of the exefile
    EXENAME=llxdoom
    SFLAGS=-g $(OPTS)
    OBJS += $(O)/i_video_xshm.o
  else
    OPTS := $(OPTS) -DVID_GGI -I.
    LDFLAGS=
    LIBS=-lggi -lm
# name of the exefile
    EXENAME=llggidoom
    SFLAGS=-g $(OPTS)
    OBJS += $(O)/i_video_ggi.o
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
  #USEASM=1

  # Uncomment if you want to use the POLL_POINTER hack
  #POLL_POINTER=-DPOLL_POINTER

  OPTS=-DLINUX -DFREEBSD -DOLD_SOUND_DRIVER -DSNDSERV -DHWRENDER
  # objs common to xshm and ggi, video is added later
  # Note: free-bsd version of cdmus
  MAINOBJ := $(O)/i_main.o
  CDMUSOBJ := $(O)/i_cdmus_fbsd.o
  OBJS=$(O)/i_system.o $(O)/i_sound.o $(O)/i_net.o \
    $(O)/searchp.o $(O)/filesrch.o $(O)/dosstr.o $(O)/endtxt.o \
    $(O)/hw_bsp.o $(O)/hw_draw.o $(O)/hw_light.o $(O)/hw_main.o \
    $(O)/hw_md2.o $(O)/hw_cache.o $(O)/hw_trick.o

  ifndef FBSD_GGI
    X=1
  endif

  ifdef X
    OPTS := $(OPTS) $(POLL_POINTER) -I. -I/usr/X11R6/include
    LDFLAGS=-L/usr/X11R6/lib
    LIBS=-lXext -lX11 -lm -lXxf86vm -lipx -lkvm -pthread
    # name of the exefile
    EXENAME=llxdoom
    SFLAGS=-g $(OPTS)
    OBJS += $(O)/i_video_xshm.o
  else #ifdef X
    OPTS := $(OPTS) -DVID_GGI -I. -I/usr/local/include
    LDFLAGS=
    LIBS=-lggi -lm -lipx
    # name of the exefile
    EXENAME=llggidoom
    SFLAGS=-g $(OPTS)
    OBJS += $(O)/i_video_ggi.o
  endif #ifdef X


#=================================
else
ifeq ($(SMIF), OS2_NATIVE)
  #Requires OS2 libs, os2.h, os2me.h, fourcc.h, DIVE libs, and mmpm2 libs
  #Does not have cdmusic support
  INTERFACE=os2
  NASMFORMAT=coff

#use the x86 asm code
  NASM=nasm
#USEASM=1
  PROFILEMODE=1
# options
  OPTS=-Zmt -D__OS2__ -I.. -I. -Ios2

  DEBUGLIBS=-lmmpm2 -lsocket -g
  LIBS= $(INTERFACE)/I_pm.def -lmmpm2 -lsocket -g

  SFLAGS=-g $(OPTS)

  MAINOBJ := $(O)/main.o
  CDMUSOBJ := $(O)/i_cdmus.o
  OBJS=$(O)/i_system.o $(O)/i_video.o $(O)/i_sound.o $(O)/i_net.o \
    $(O)/i_dart.o $(O)/i_dive.o $(O)/I_pm.o \
    $(O)/printf.o $(O)/I_pm.res $(O)/filesrch.o

# name of the exefile
  EXENAME=legacy.exe


#=================================
else
ifeq ($(SMIF), WIN32_NATIVE)
  # Requires windows.h, windowsx.h, ddraw.h and ddraw libs, fmod libs
  INTERFACE=win32
  NASMFORMAT=coff

#use the x86 asm code
  #USEASM=1

# options
  OPTS= -DWIN32 -g

  DEBUGLIBS=-lfmod -lwinmm -lsocket
  LIBS=-lfmod -lwinmm -lsocket

  SFLAGS=

  MAINOBJ := $(O)/win_main.o
  OBJS=$(O)/win_sys.o $(O)/win_vid.o $(O)/win_snd.o \
    $(O)/win_net.o $(O)/win_cd.o $(O)/filesrch.o

# name of the exefile
  EXENAME=legacy.exe

#=================================
else
ifeq ($(SMIF), DJGPPDOS_NATIVE)
  # Requires allegro libs, bcd libs (cd-music)
  RM=del
  INTERFACE=djgppdos
  NASMFORMAT=coff

#use the x86 asm code
  #USEASM=1

# options
  OPTS=-DPC_DOS -g

  DEBUGLIBS=-lalleg -lbcd -lsocket
  LIBS=-lalleg -lsocket
  ifdef CDMUS
     LIBS+=-lbcd
  endif

  SFLAGS=

  MAINOBJ := $(O)/i_main.o
  CDMUSOBJ := $(O)/i_cdmus.o
  OBJS=$(O)/i_system.o $(O)/i_video.o $(O)/vid_vesa.o \
    $(O)/i_sound.o $(O)/i_net.o \
    $(O)/filesrch.o

# name of the exefile
  EXENAME=legacy.exe
endif
endif
endif
endif
endif
endif
#=================================




# use assembly routines?
ifdef USEASM
   OBJS:= $(OBJS) $(O)/tmap.o $(O)/vid_copy.o
   OPTS:= $(OPTS) -DUSEASM
endif

# CD-Music
ifdef CDMUS
   OBJS += $(CDMUSOBJ)
   OPTS += -DCDMUS
endif

# compiler and linker flags
CFLAGS = $(WFLAGS)

ifdef PROFILEMODE
# build with gprof profiling information
  CFLAGS += -g -pg -fomit-frame-pointer
else
ifdef DEBUG
# No -fomit-frame-pointer because debugging on some machines depends upon it.
  #CFLAGS += -g3 -O0
  CFLAGS += $(DEBUGFLAGS) -fno-guess-branch-probability
else
# build a normal optimized version
  #CFLAGS += -O3
  CFLAGS += $(OPTLEV) -fomit-frame-pointer
endif
endif

CFLAGS += -ffast-math -fno-strict-aliasing $(OPTS)

ifdef MAC
else
  LDFLAGS += -Xlinker --warn-common
endif

SNDSERV_EXTRAFLAGS:=
ifdef HAVE_ESD
  SNDSERV_EXTRAFLAGS:= HAVE_ESD=1
endif



# not too sophisticated dependency
# objs that are in main directories, INTERFACE specific obj files are in
# listed in the interface #if above
OBJS :=  $(OBJS) \
	$(O)/i_tcp.o	\
	$(O)/sounds.o  $(O)/qmus2mid.o  $(O)/s_sound.o  $(O)/s_amb.o	 \
	$(O)/v_video.o  $(O)/screen.o	\
	$(O)/z_zone.o	\
	$(O)/dstrings.o  $(O)/tables.o  $(O)/info.o	\
		$(O)/f_finale.o  \
		$(O)/f_wipe.o	 \
		$(O)/d_main.o	 \
		$(O)/d_net.o	 \
		$(O)/d_items.o	 \
		$(O)/g_game.o	 \
		$(O)/m_menu.o	 \
		$(O)/m_misc.o	 \
		$(O)/m_argv.o	 \
		$(O)/m_bbox.o	 \
		$(O)/m_fixed.o	 \
		$(O)/m_swap.o	 \
		$(O)/m_cheat.o	 \
		$(O)/m_random.o  \
		$(O)/am_map.o	 \
		$(O)/p_ceilng.o  \
		$(O)/p_doors.o	 \
		$(O)/p_enemy.o	 \
		$(O)/p_floor.o	 \
		$(O)/p_genlin.o  \
		$(O)/p_info.o	 \
		$(O)/p_inter.o	 \
		$(O)/p_lights.o  \
		$(O)/p_map.o	 \
		$(O)/p_maputl.o  \
		$(O)/p_plats.o	 \
		$(O)/p_pspr.o	 \
		$(O)/p_setup.o	 \
		$(O)/p_sight.o	 \
		$(O)/p_spec.o	 \
		$(O)/p_switch.o  \
		$(O)/p_mobj.o	 \
		$(O)/p_telept.o  \
		$(O)/p_tick.o	 \
		$(O)/p_saveg.o	 \
		$(O)/p_user.o	 \
		$(O)/r_bsp.o	 \
		$(O)/r_data.o	 \
		$(O)/r_draw.o	 \
		$(O)/r_main.o	 \
		$(O)/r_plane.o	 \
		$(O)/r_segs.o	 \
		$(O)/r_sky.o	 \
		$(O)/r_things.o  \
		$(O)/w_wad.o	 \
		$(O)/wi_stuff.o  \
		$(O)/st_lib.o	 \
		$(O)/st_stuff.o  \
		$(O)/hu_stuff.o  \
		$(O)/p_fab.o	 \
		$(O)/dehacked.o  \
		$(O)/g_input.o	 \
		$(O)/console.o	 \
		$(O)/command.o	 \
		$(O)/d_netcmd.o  \
		$(O)/d_clisrv.o  \
		$(O)/r_splats.o  \
		$(O)/g_state.o	 \
		$(O)/t_array.o   \
		$(O)/t_func.o	 \
		$(O)/t_oper.o	 \
		$(O)/t_parse.o	 \
		$(O)/t_prepro.o  \
		$(O)/t_script.o  \
		$(O)/t_spec.o	 \
		$(O)/t_vari.o	 \
		$(O)/d_netfil.o  \
		$(O)/mserv.o	 \
		$(O)/p_hsight.o  \
		$(O)/sb_bar.o	 \
		$(O)/p_heretic.o \
		$(O)/md5.o	 \
		$(O)/p_chex.o	 \
		$(O)/b_game.o	\
		$(O)/b_look.o	\
		$(O)/b_node.o	\
		$(O)/b_search.o



#=======================================================
# rules
#=======================================================
.PHONY : all clean asm dll wad


ifeq ($(SMIF), LINUX_X11)
all:	 $(BIN)/$(EXENAME) dll musserv sndserv

else
ifeq ($(SMIF), FREEBSD_X11)
all:	 $(BIN)/$(EXENAME) dll sndserv

else
all:	 $(BIN)/$(EXENAME)

endif
endif


clean:
	$(RM) $(O)/*
ifeq ($(SMIF), LINUX_X11)
	$(RM) $(SNDSERV)/linux/*
	$(RM) $(MUSSERV)/linux/*
else
ifeq ($(SMIF), FREEBSD_X11)
	$(RM) $(SNDSERV)/linux/*
	$(RM) $(MUSSERV)/linux/*
endif
endif



$(O):
	@mkdir $(O)

.PHONY : versionstring
# This may fail because, (a) svn not installed, (b) not a svn directory.
# This compiles d_main a second time, with SVN_REV set.
versionstring:
ifdef SVN_ENABLE
	$(CC) -c $(CFLAGS) -DSVN_REV=\"`svn info | grep Revision | sed -e 's/Revision: //'`\" d_main.c -o $(O)/d_main.o
endif


wad	:
ifdef WAD
	@echo "Building a new legacy.wad using an old version $(WAD)..."
	if test ! -d ../wad; then mkdir ../wad; fi
	cd ../wad ; \
	../../common/bin/wadtool -x $(WAD) ; \
	cp ../trunk/resources/* . ; \
	../../common/bin/wadtool -c legacy.wad legacy.wad.inventory ; \
	mv legacy.wad $(BIN) ; \
	rm *.txt *.h *.lmp
	@echo "Finished building legacy.wad."
else
	@echo "Usage: make wad WAD=/path/to/old/legacy.wad"
endif




#make a disasm of the exe
asm: $(BIN)/$(EXENAME)
	objdump -d $(BIN)/$(EXENAME) --no-show-raw-insn > $(BIN)/doomlegacy.s


# executable
$(BIN)/$(EXENAME): $(O) $(OBJS) $(MAINOBJ) versionstring
#	 @mkdir $(BIN)
	@echo Linking...
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(MAINOBJ) \
	-o $(BIN)/$(EXENAME) $(LIBS)



#dll
dll : $(O)/r_opengl.o $(O)/ogl_x11.o
	$(CC) -O3 -o $(BIN)/r_opengl.so -shared -nostartfiles $(O)/r_opengl.o $(O)/ogl_x11.o -L/usr/X11R6/lib -L/usr/local/lib -lX11 -lXext -lGL -lGLU -lm

musserv:
	@mkdir -p $(MUSSERV)/linux
	cd $(MUSSERV) \
	&& make -f Makefile.linux

sndserv:
	@mkdir -p $(SNDSERV)/linux
	cd $(SNDSERV) \
	&& $(MAKE) $(SNDSERV_EXTRAFLAGS)



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


#=======================================================

$(O)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/%.o: $(INTERFACE)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/%.o: hardware/%.c
	$(CC) $(CFLAGS) -c $< -o $@

ifeq ($(SMIF), OS2_NATIVE)
$(O)/%.res: os2/%.rc
	rc.exe -i .\os2 -r $< $@
endif

#=======================================================
#[WDJ] Dependencies, replaces makedep.mak, which was unmaintained.

# This PHONY disables automatic dep file generation when a file changes.
# Comment it out for automatic dep file generation.
.PHONY : ../dep/*.dep

.PHONY : depend
depend: ../dep/main1.dep ../dep/$(INTERFACE).dep

../dep/main1.dep : a*.c b*.c d*.c f*.c g*.c h*.c
../dep/main2.dep : i*.c m*.c p*.c
../dep/main3.dep : q*.c r*.c s*.c t*.c v*.c z*.c
# none: e*.c j*.c k*.c l*.c n*.c o*.c
../dep/hardware.dep : hardware/*.c hardware/*/*.c
# $(INTERFACE).dep : $(INTERFACE)/*.c
../dep/djgppdos.dep : djgppdos/*.c
../dep/linux_x.dep : linux_x/*.c linux_x/musserv/*.c linux_x/sndserv/*.c
../dep/os_2.dep : os2/*.c
../dep/macos.dep : macos/*.c
../dep/sdl.dep : sdl/*.c
../dep/win32.dep : win32/*.c

vpath %.o $(O)

ifdef WIN32
../dep :
	@mkdir ..\dep;

%.dep :
	@echo "Making dependencies $(@F)"
	$(CC) $(CFLAGS) -MM $^ > ..\dep\$(@F)
else
../dep :
	@mkdir ../dep;

%.dep :
	@echo "Making dependencies $(@F)"
	if test ! -d ../dep; then mkdir ../dep; fi
	$(CC) $(CFLAGS) -MM $^ > ../dep/$(@F)
	sed --separate -e "s@^[a-zA-Z0-9_]*.o:@\$$(O)/&@" ../dep/$(@F) > sed.dep
	mv sed.dep ../dep/$(@F)
endif

$(O)/*.o : doomdef.h

$(O)/r_draw.o : r_draw.c r_draw8.c r_draw16.c r_draw24.c r_draw32.c

include ../dep/main1.dep
include ../dep/main2.dep
include ../dep/main3.dep
include ../dep/hardware.dep
include ../dep/$(INTERFACE).dep

#=======================================================
# If you have a really old make or want these old dependencies.
ifdef OLD_DEPENDENCIES

#dependecy made by gcc itself ! (see makedep.mak for more)
$(O)/dstrings.o: dstrings.c dstrings.h d_englsh.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/i_sound.o: $(INTERFACE)/i_sound.c doomdef.h \
 doomtype.h g_state.h m_swap.h doomstat.h doomdata.h d_player.h \
 d_items.h p_pspr.h m_fixed.h tables.h info.h d_think.h \
 p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h command.h i_system.h \
 d_event.h i_sound.h sounds.h z_zone.h m_argv.h m_misc.h \
 w_wad.h s_sound.h qmus2mid.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/i_cdmus.o: $(INTERFACE)/i_cdmus.c doomdef.h \
 doomtype.h g_state.h m_swap.h	i_sound.h sounds.h command.h \
 i_system.h d_ticcmd.h d_event.h  s_sound.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/i_video.o: $(INTERFACE)/i_video.c doomdef.h \
 doomtype.h g_state.h m_swap.h i_system.h d_ticcmd.h d_event.h \
 v_video.h r_defs.h m_fixed.h d_think.h p_mobj.h tables.h \
 doomdata.h info.h screen.h command.h m_argv.h \
 i_video.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/i_net.o: $(INTERFACE)/i_net.c doomdef.h doomtype.h \
 g_state.h m_swap.h i_system.h d_ticcmd.h d_event.h d_net.h \
 m_argv.h doomstat.h doomdata.h d_player.h d_items.h p_pspr.h \
 m_fixed.h tables.h info.h d_think.h p_mobj.h d_clisrv.h \
 d_netcmd.h command.h z_zone.h i_net.h i_tcp.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/i_tcp.o: i_tcp.c doomdef.h doomtype.h g_state.h m_swap.h i_system.h \
 d_ticcmd.h d_event.h i_net.h d_net.h m_argv.h doomstat.h doomdata.h \
 d_player.h d_items.h p_pspr.h m_fixed.h tables.h info.h d_think.h \
 p_mobj.h d_clisrv.h d_netcmd.h command.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/i_system.o: $(INTERFACE)/i_system.c doomdef.h \
 doomtype.h g_state.h m_swap.h \
 m_misc.h w_wad.h i_video.h \
 i_sound.h sounds.h d_net.h \
 g_game.h doomstat.h doomdata.h \
 d_player.h d_items.h p_pspr.h \
 m_fixed.h tables.h info.h \
 d_think.h p_mobj.h d_ticcmd.h \
 d_clisrv.h d_netcmd.h command.h \
 d_event.h d_main.h m_argv.h \
 z_zone.h g_input.h \
 console.h i_system.h \
 i_joy.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/tables.o: tables.c tables.h m_fixed.h doomtype.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/f_finale.o: f_finale.c doomdef.h doomtype.h g_state.h m_swap.h \
 am_map.h d_event.h dstrings.h d_englsh.h d_main.h w_wad.h f_finale.h \
 g_game.h doomstat.h doomdata.h d_player.h d_items.h p_pspr.h \
 m_fixed.h tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h \
 d_netcmd.h command.h hu_stuff.h wi_stuff.h r_defs.h screen.h \
 r_local.h m_bbox.h r_main.h r_data.h r_state.h r_bsp.h r_segs.h \
 r_plane.h r_sky.h r_things.h sounds.h r_draw.h s_sound.h i_video.h \
 v_video.h z_zone.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/f_wipe.o: f_wipe.c doomdef.h doomtype.h g_state.h m_swap.h m_random.h \
 f_wipe.h i_system.h d_ticcmd.h d_event.h i_video.h v_video.h r_defs.h \
 m_fixed.h d_think.h p_mobj.h tables.h doomdata.h info.h screen.h \
 command.h z_zone.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/d_main.o: d_main.c doomdef.h doomtype.h g_state.h m_swap.h command.h \
 console.h d_event.h doomstat.h doomdata.h d_player.h d_items.h \
 p_pspr.h m_fixed.h tables.h info.h d_think.h p_mobj.h d_ticcmd.h \
 d_clisrv.h d_netcmd.h am_map.h d_net.h dehacked.h dstrings.h \
 d_englsh.h f_wipe.h f_finale.h g_game.h g_input.h keys.h hu_stuff.h \
 w_wad.h wi_stuff.h r_defs.h screen.h i_sound.h sounds.h i_system.h \
 i_video.h m_argv.h m_menu.h m_misc.h p_setup.h p_fab.h r_main.h \
 r_data.h r_state.h r_local.h m_bbox.h r_bsp.h r_segs.h r_plane.h \
 r_sky.h r_things.h r_draw.h s_sound.h st_stuff.h v_video.h z_zone.h \
 d_main.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/d_net.o: d_net.c doomdef.h doomtype.h g_state.h m_swap.h d_clisrv.h \
 d_ticcmd.h d_netcmd.h command.h g_game.h doomstat.h doomdata.h \
 d_player.h d_items.h p_pspr.h m_fixed.h tables.h info.h d_think.h \
 p_mobj.h d_event.h i_net.h i_system.h m_argv.h d_net.h
	$(CC) $(CFLAGS) -c $< -o $@
$(O)/d_items.o: d_items.c info.h d_think.h d_items.h doomdef.h doomtype.h \
 g_state.h m_swap.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/g_game.o: g_game.c doomdef.h doomtype.h g_state.h m_swap.h command.h \
 console.h d_event.h dstrings.h d_englsh.h d_main.h w_wad.h d_net.h \
 d_netcmd.h f_finale.h p_setup.h doomdata.h p_saveg.h i_system.h \
 d_ticcmd.h wi_stuff.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h am_map.h m_random.h p_local.h \
 m_bbox.h p_tick.h r_defs.h screen.h p_maputl.h p_spec.h r_data.h \
 r_state.h r_draw.h r_main.h r_sky.h s_sound.h sounds.h g_game.h \
 doomstat.h d_clisrv.h g_input.h keys.h p_fab.h m_cheat.h m_misc.h \
 m_menu.h m_argv.h hu_stuff.h st_stuff.h z_zone.h i_video.h p_inter.h \
 byteptr.h i_joy.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/m_menu.o: m_menu.c am_map.h d_event.h doomtype.h g_state.h doomdef.h \
 m_swap.h dstrings.h d_englsh.h d_main.h w_wad.h console.h r_local.h \
 tables.h m_fixed.h screen.h command.h m_bbox.h r_main.h d_player.h \
 d_items.h p_pspr.h info.h d_think.h p_mobj.h doomdata.h d_ticcmd.h \
 r_data.h r_defs.h r_state.h r_bsp.h r_segs.h r_plane.h r_sky.h \
 r_things.h sounds.h r_draw.h hu_stuff.h wi_stuff.h g_game.h \
 doomstat.h d_clisrv.h d_netcmd.h g_input.h keys.h m_argv.h s_sound.h \
 i_system.h m_menu.h v_video.h i_video.h z_zone.h p_local.h p_tick.h \
 p_maputl.h p_spec.h p_fab.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/m_misc.o: m_misc.c doomdef.h doomtype.h g_state.h m_swap.h g_game.h \
 doomstat.h doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h \
 command.h d_event.h m_misc.h w_wad.h hu_stuff.h wi_stuff.h r_defs.h \
 screen.h v_video.h z_zone.h g_input.h keys.h i_video.h d_main.h \
 m_argv.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/m_argv.o: m_argv.c doomdef.h doomtype.h g_state.h m_swap.h command.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/m_bbox.o: m_bbox.c doomtype.h m_bbox.h m_fixed.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/m_fixed.o: m_fixed.c i_system.h d_ticcmd.h doomtype.h d_event.h \
 g_state.h m_fixed.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/m_swap.o: m_swap.c
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/m_cheat.o: m_cheat.c doomdef.h doomtype.h g_state.h m_swap.h \
 dstrings.h d_englsh.h am_map.h d_event.h m_cheat.h g_game.h \
 doomstat.h doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h \
 command.h r_local.h screen.h m_bbox.h r_main.h r_data.h r_defs.h \
 r_state.h r_bsp.h r_segs.h r_plane.h r_sky.h r_things.h sounds.h \
 r_draw.h p_local.h p_tick.h p_maputl.h p_spec.h p_inter.h i_sound.h \
 s_sound.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/m_random.o: m_random.c doomdef.h doomtype.h g_state.h m_swap.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/am_map.o: am_map.c doomdef.h doomtype.h g_state.h m_swap.h doomstat.h \
 doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h tables.h info.h \
 d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h command.h \
 g_game.h d_event.h am_map.h g_input.h keys.h m_cheat.h p_local.h \
 m_bbox.h p_tick.h r_defs.h screen.h p_maputl.h p_spec.h v_video.h \
 st_stuff.h i_system.h i_video.h r_state.h r_data.h dstrings.h \
 d_englsh.h w_wad.h z_zone.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/p_ceilng.o: p_ceilng.c doomdef.h doomtype.h g_state.h m_swap.h \
 p_local.h command.h d_player.h d_items.h p_pspr.h m_fixed.h tables.h \
 info.h d_think.h p_mobj.h doomdata.h d_ticcmd.h m_bbox.h p_tick.h \
 r_defs.h screen.h p_maputl.h p_spec.h r_state.h r_data.h s_sound.h \
 sounds.h z_zone.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/p_doors.o: p_doors.c doomdef.h doomtype.h g_state.h m_swap.h \
 dstrings.h d_englsh.h p_local.h command.h d_player.h d_items.h \
 p_pspr.h m_fixed.h tables.h info.h d_think.h p_mobj.h doomdata.h \
 d_ticcmd.h m_bbox.h p_tick.h r_defs.h screen.h p_maputl.h p_spec.h \
 r_state.h r_data.h s_sound.h sounds.h z_zone.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/p_enemy.o: p_enemy.c doomdef.h doomtype.h g_state.h m_swap.h g_game.h \
 doomstat.h doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h \
 command.h d_event.h p_local.h m_bbox.h p_tick.h r_defs.h screen.h \
 p_maputl.h p_spec.h r_main.h r_data.h r_state.h s_sound.h sounds.h \
 m_random.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/p_floor.o: p_floor.c doomdef.h doomtype.h g_state.h m_swap.h p_local.h \
 command.h d_player.h d_items.h p_pspr.h m_fixed.h tables.h info.h \
 d_think.h p_mobj.h doomdata.h d_ticcmd.h m_bbox.h p_tick.h r_defs.h \
 screen.h p_maputl.h p_spec.h r_state.h r_data.h s_sound.h sounds.h \
 z_zone.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/p_genlin.o: p_genlin.c doomdef.h g_game.h p_local.h r_data.h m_random.h \
 m_random.h s_sound.h z_zone.h command.h d_player.h d_think.h m_fixed.h \
 m_bbox.h p_tick.h r_defs.h p_maputl.h p_mobj.h screen.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/p_info.o: p_info.c doomdef.h dehacked.h dstrings.h p_setup.h p_info.h \
 p_mobj.h t_script.h w_wad.h z_zone.h command.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/p_inter.o: p_inter.c doomdef.h doomtype.h g_state.h m_swap.h \
 i_system.h d_ticcmd.h d_event.h am_map.h dstrings.h d_englsh.h \
 g_game.h doomstat.h doomdata.h d_player.h d_items.h p_pspr.h \
 m_fixed.h tables.h info.h d_think.h p_mobj.h d_clisrv.h d_netcmd.h \
 command.h m_random.h p_local.h m_bbox.h p_tick.h r_defs.h screen.h \
 p_maputl.h p_spec.h p_inter.h s_sound.h sounds.h r_main.h r_data.h \
 r_state.h st_stuff.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/p_lights.o: p_lights.c doomdef.h doomtype.h g_state.h m_swap.h \
 p_local.h command.h d_player.h d_items.h p_pspr.h m_fixed.h tables.h \
 info.h d_think.h p_mobj.h doomdata.h d_ticcmd.h m_bbox.h p_tick.h \
 r_defs.h screen.h p_maputl.h p_spec.h r_state.h r_data.h z_zone.h \
 m_random.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/p_map.o: p_map.c doomdef.h doomtype.h g_state.h m_swap.h g_game.h \
 doomstat.h doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h \
 command.h d_event.h m_bbox.h m_random.h p_local.h p_tick.h r_defs.h \
 screen.h p_maputl.h p_spec.h p_inter.h r_state.h r_data.h r_main.h \
 r_sky.h s_sound.h sounds.h r_splats.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/p_maputl.o: p_maputl.c p_local.h command.h doomtype.h d_player.h \
 d_items.h doomdef.h g_state.h m_swap.h p_pspr.h m_fixed.h tables.h \
 info.h d_think.h p_mobj.h doomdata.h d_ticcmd.h m_bbox.h p_tick.h \
 r_defs.h screen.h p_maputl.h p_spec.h r_main.h r_data.h r_state.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/p_plats.o: p_plats.c doomdef.h doomtype.h g_state.h m_swap.h p_local.h \
 command.h d_player.h d_items.h p_pspr.h m_fixed.h tables.h info.h \
 d_think.h p_mobj.h doomdata.h d_ticcmd.h m_bbox.h p_tick.h r_defs.h \
 screen.h p_maputl.h p_spec.h s_sound.h sounds.h r_state.h r_data.h \
 z_zone.h m_random.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/p_pspr.o: p_pspr.c doomdef.h doomtype.h g_state.h m_swap.h d_event.h \
 p_local.h command.h d_player.h d_items.h p_pspr.h m_fixed.h tables.h \
 info.h d_think.h p_mobj.h doomdata.h d_ticcmd.h m_bbox.h p_tick.h \
 r_defs.h screen.h p_maputl.h p_spec.h p_inter.h s_sound.h sounds.h \
 g_game.h doomstat.h d_clisrv.h d_netcmd.h g_input.h keys.h r_main.h \
 r_data.h r_state.h m_random.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/p_setup.o: p_setup.c doomdef.h doomtype.h g_state.h m_swap.h d_main.h \
 d_event.h w_wad.h g_game.h doomstat.h doomdata.h d_player.h d_items.h \
 p_pspr.h m_fixed.h tables.h info.h d_think.h p_mobj.h d_ticcmd.h \
 d_clisrv.h d_netcmd.h command.h p_local.h m_bbox.h p_tick.h r_defs.h \
 screen.h p_maputl.h p_spec.h p_setup.h i_sound.h sounds.h r_sky.h \
 r_data.h r_state.h r_things.h s_sound.h z_zone.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/p_sight.o: p_sight.c doomdef.h doomtype.h g_state.h m_swap.h p_local.h \
 command.h d_player.h d_items.h p_pspr.h m_fixed.h tables.h info.h \
 d_think.h p_mobj.h doomdata.h d_ticcmd.h m_bbox.h p_tick.h r_defs.h \
 screen.h p_maputl.h p_spec.h r_main.h r_data.h r_state.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/p_spec.o: p_spec.c doomdef.h doomtype.h g_state.h m_swap.h g_game.h \
 doomstat.h doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h \
 command.h d_event.h p_local.h m_bbox.h p_tick.h r_defs.h screen.h \
 p_maputl.h p_spec.h p_setup.h r_data.h r_state.h m_random.h s_sound.h \
 sounds.h w_wad.h z_zone.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/p_switch.o: p_switch.c doomdef.h doomtype.h g_state.h m_swap.h \
 g_game.h doomstat.h doomdata.h d_player.h d_items.h p_pspr.h \
 m_fixed.h tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h \
 d_netcmd.h command.h d_event.h p_local.h m_bbox.h p_tick.h r_defs.h \
 screen.h p_maputl.h p_spec.h s_sound.h sounds.h r_main.h r_data.h \
 r_state.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/p_mobj.o: p_mobj.c doomdef.h doomtype.h g_state.h m_swap.h g_game.h \
 doomstat.h doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h \
 command.h d_event.h g_input.h keys.h st_stuff.h hu_stuff.h w_wad.h \
 wi_stuff.h r_defs.h screen.h p_local.h m_bbox.h p_tick.h p_maputl.h \
 p_spec.h p_inter.h p_setup.h r_main.h r_data.h r_state.h r_things.h \
 sounds.h r_sky.h s_sound.h z_zone.h m_random.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/p_telept.o: p_telept.c doomdef.h doomtype.h g_state.h m_swap.h \
 g_game.h doomstat.h doomdata.h d_player.h d_items.h p_pspr.h \
 m_fixed.h tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h \
 d_netcmd.h command.h d_event.h p_local.h m_bbox.h p_tick.h r_defs.h \
 screen.h p_maputl.h p_spec.h r_state.h r_data.h s_sound.h sounds.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/p_tick.o: p_tick.c doomstat.h doomdata.h doomtype.h doomdef.h \
 g_state.h m_swap.h d_player.h d_items.h p_pspr.h m_fixed.h tables.h \
 info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h command.h \
 g_game.h d_event.h p_local.h m_bbox.h p_tick.h r_defs.h screen.h \
 p_maputl.h p_spec.h z_zone.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/p_saveg.o: p_saveg.c doomdef.h doomtype.h g_state.h m_swap.h g_game.h \
 doomstat.h doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h \
 command.h d_event.h p_local.h m_bbox.h p_tick.h r_defs.h screen.h \
 p_maputl.h p_spec.h r_state.h r_data.h z_zone.h w_wad.h p_setup.h \
 byteptr.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/p_user.o: p_user.c doomdef.h doomtype.h g_state.h m_swap.h d_event.h \
 g_game.h doomstat.h doomdata.h d_player.h d_items.h p_pspr.h \
 m_fixed.h tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h \
 d_netcmd.h command.h p_local.h m_bbox.h p_tick.h r_defs.h screen.h \
 p_maputl.h p_spec.h r_main.h r_data.h r_state.h s_sound.h sounds.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/r_bsp.o: r_bsp.c doomdef.h doomtype.h g_state.h m_swap.h g_game.h \
 doomstat.h doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h \
 command.h d_event.h r_local.h screen.h m_bbox.h r_main.h r_data.h \
 r_defs.h r_state.h r_bsp.h r_segs.h r_plane.h r_sky.h r_things.h \
 sounds.h r_draw.h r_splats.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/r_data.o: r_data.c doomdef.h doomtype.h g_state.h m_swap.h g_game.h \
 doomstat.h doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h \
 command.h d_event.h i_video.h r_local.h screen.h m_bbox.h r_main.h \
 r_data.h r_defs.h r_state.h r_bsp.h r_segs.h r_plane.h r_sky.h \
 r_things.h sounds.h r_draw.h p_local.h p_tick.h p_maputl.h p_spec.h \
 w_wad.h z_zone.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/r_draw.o: r_draw.c doomdef.h doomtype.h g_state.h m_swap.h r_local.h \
 tables.h m_fixed.h screen.h command.h m_bbox.h r_main.h d_player.h \
 d_items.h p_pspr.h info.h d_think.h p_mobj.h doomdata.h d_ticcmd.h \
 r_data.h r_defs.h r_state.h r_bsp.h r_segs.h r_plane.h r_sky.h \
 r_things.h sounds.h r_draw.h st_stuff.h d_event.h i_video.h \
 v_video.h w_wad.h z_zone.h r_draw8.c r_draw16.c
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/r_main.o: r_main.c doomdef.h doomtype.h g_state.h m_swap.h g_game.h \
 doomstat.h doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h \
 command.h d_event.h g_input.h keys.h r_local.h screen.h m_bbox.h \
 r_main.h r_data.h r_defs.h r_state.h r_bsp.h r_segs.h r_plane.h \
 r_sky.h r_things.h sounds.h r_draw.h r_splats.h st_stuff.h p_local.h \
 p_tick.h p_maputl.h p_spec.h i_video.h m_menu.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/r_plane.o: r_plane.c doomdef.h doomtype.h g_state.h m_swap.h console.h \
 d_event.h g_game.h doomstat.h doomdata.h d_player.h d_items.h \
 p_pspr.h m_fixed.h tables.h info.h d_think.h p_mobj.h d_ticcmd.h \
 d_clisrv.h d_netcmd.h command.h r_data.h r_defs.h screen.h r_state.h \
 r_local.h m_bbox.h r_main.h r_bsp.h r_segs.h r_plane.h r_sky.h \
 r_things.h sounds.h r_draw.h r_splats.h v_video.h w_wad.h \
 z_zone.h p_setup.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/r_segs.o: r_segs.c doomdef.h doomtype.h g_state.h m_swap.h r_local.h \
 tables.h m_fixed.h screen.h command.h m_bbox.h r_main.h d_player.h \
 d_items.h p_pspr.h info.h d_think.h p_mobj.h doomdata.h d_ticcmd.h \
 r_data.h r_defs.h r_state.h r_bsp.h r_segs.h r_plane.h r_sky.h \
 r_things.h sounds.h r_draw.h r_splats.h w_wad.h z_zone.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/r_sky.o: r_sky.c doomdef.h doomtype.h g_state.h m_swap.h r_sky.h \
 m_fixed.h r_local.h tables.h screen.h command.h m_bbox.h r_main.h \
 d_player.h d_items.h p_pspr.h info.h d_think.h p_mobj.h doomdata.h \
 d_ticcmd.h r_data.h r_defs.h r_state.h r_bsp.h r_segs.h r_plane.h \
 r_things.h sounds.h r_draw.h w_wad.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/r_things.o: r_things.c doomdef.h doomtype.h g_state.h m_swap.h \
 console.h d_event.h g_game.h doomstat.h doomdata.h d_player.h \
 d_items.h p_pspr.h m_fixed.h tables.h info.h d_think.h p_mobj.h \
 d_ticcmd.h d_clisrv.h d_netcmd.h command.h r_local.h screen.h \
 m_bbox.h r_main.h r_data.h r_defs.h r_state.h r_bsp.h r_segs.h \
 r_plane.h r_sky.h r_things.h sounds.h r_draw.h st_stuff.h w_wad.h \
 z_zone.h i_video.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/w_wad.o: w_wad.c doomdef.h doomtype.h g_state.h m_swap.h w_wad.h \
 z_zone.h i_video.h d_netfil.h md5.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/wi_stuff.o: wi_stuff.c doomdef.h doomtype.h g_state.h m_swap.h \
 wi_stuff.h d_player.h d_items.h p_pspr.h m_fixed.h tables.h info.h \
 d_think.h p_mobj.h doomdata.h d_ticcmd.h g_game.h doomstat.h \
 d_clisrv.h d_netcmd.h command.h d_event.h hu_stuff.h w_wad.h r_defs.h \
 screen.h m_random.h r_local.h m_bbox.h r_main.h r_data.h r_state.h \
 r_bsp.h r_segs.h r_plane.h r_sky.h r_things.h sounds.h r_draw.h \
 s_sound.h st_stuff.h i_video.h v_video.h z_zone.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/v_video.o: v_video.c doomdef.h doomtype.h g_state.h m_swap.h r_local.h \
 tables.h m_fixed.h screen.h command.h m_bbox.h r_main.h d_player.h \
 d_items.h p_pspr.h info.h d_think.h p_mobj.h doomdata.h d_ticcmd.h \
 r_data.h r_defs.h r_state.h r_bsp.h r_segs.h r_plane.h r_sky.h \
 r_things.h sounds.h r_draw.h v_video.h hu_stuff.h d_event.h w_wad.h \
 wi_stuff.h i_video.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/st_lib.o: st_lib.c doomdef.h doomtype.h g_state.h m_swap.h st_lib.h \
 r_defs.h m_fixed.h d_think.h p_mobj.h tables.h doomdata.h info.h \
 screen.h command.h w_wad.h st_stuff.h d_event.h d_player.h d_items.h \
 p_pspr.h d_ticcmd.h v_video.h z_zone.h i_video.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/st_stuff.o: st_stuff.c doomdef.h doomtype.h g_state.h m_swap.h \
 am_map.h d_event.h g_game.h doomstat.h doomdata.h d_player.h \
 d_items.h p_pspr.h m_fixed.h tables.h info.h d_think.h p_mobj.h \
 d_ticcmd.h d_clisrv.h d_netcmd.h command.h m_cheat.h r_local.h \
 screen.h m_bbox.h r_main.h r_data.h r_defs.h r_state.h r_bsp.h \
 r_segs.h r_plane.h r_sky.h r_things.h sounds.h r_draw.h p_local.h \
 p_tick.h p_maputl.h p_spec.h p_inter.h m_random.h st_stuff.h st_lib.h \
 w_wad.h i_video.h v_video.h keys.h z_zone.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/hu_stuff.o: hu_stuff.c doomdef.h doomtype.h g_state.h m_swap.h \
 hu_stuff.h d_event.h w_wad.h wi_stuff.h d_player.h d_items.h p_pspr.h \
 m_fixed.h tables.h info.h d_think.h p_mobj.h doomdata.h d_ticcmd.h \
 r_defs.h screen.h command.h d_netcmd.h d_clisrv.h g_game.h doomstat.h \
 g_input.h keys.h i_video.h dstrings.h d_englsh.h st_stuff.h r_local.h \
 m_bbox.h r_main.h r_data.h r_state.h r_bsp.h r_segs.h r_plane.h \
 r_sky.h r_things.h sounds.h r_draw.h v_video.h z_zone.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/s_sound.o: s_sound.c doomdef.h doomtype.h g_state.h m_swap.h command.h \
 g_game.h doomstat.h doomdata.h d_player.h d_items.h p_pspr.h \
 m_fixed.h tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h \
 d_netcmd.h d_event.h m_argv.h r_main.h r_data.h r_defs.h screen.h \
 r_state.h r_things.h sounds.h i_sound.h s_sound.h w_wad.h z_zone.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/z_zone.o: z_zone.c doomdef.h doomtype.h g_state.h m_swap.h z_zone.h \
 i_system.h d_ticcmd.h d_event.h command.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/info.o: info.c doomdef.h doomtype.h g_state.h m_swap.h sounds.h \
 m_fixed.h p_mobj.h tables.h d_think.h doomdata.h info.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/sounds.o: sounds.c doomtype.h sounds.h r_defs.h m_fixed.h d_think.h \
 p_mobj.h tables.h doomdata.h doomdef.h g_state.h m_swap.h info.h \
 screen.h command.h r_things.h z_zone.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/tmap.o: tmap.nas
	nasm -o $@ -f $(NASMFORMAT) $<

$(O)/p_fab.o: p_fab.c doomdef.h doomtype.h g_state.h m_swap.h g_game.h \
 doomstat.h doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h \
 command.h d_event.h p_local.h m_bbox.h p_tick.h r_defs.h screen.h \
 p_maputl.h p_spec.h r_state.h r_data.h p_fab.h m_random.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/dehacked.o: dehacked.c doomdef.h doomtype.h g_state.h m_swap.h \
 command.h console.h d_event.h g_game.h doomstat.h doomdata.h \
 d_player.h d_items.h p_pspr.h m_fixed.h tables.h info.h d_think.h \
 p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h sounds.h m_cheat.h \
 dstrings.h d_englsh.h m_argv.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/qmus2mid.o: qmus2mid.c doomdef.h doomtype.h g_state.h m_swap.h \
 i_system.h d_ticcmd.h d_event.h byteptr.h m_swap.h $(qmus2mid_h)
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/qmus2mid_sdl.o: qmus2mid_sdl.c doomdef.h doomtype.h g_state.h m_swap.h \
 i_system.h d_ticcmd.h d_event.h byteptr.h m_swap.h $(qmus2mid_h)
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/vid_copy.o: vid_copy.s
	$(CC) $(OPTS) $(SFLAGS) -x assembler-with-cpp -c $< -o $@

$(O)/vid_vesa.o: $(INTERFACE)/vid_vesa.c i_system.h \
 d_ticcmd.h doomtype.h d_event.h \
 g_state.h $(INTERFACE)/vid_vesa.h doomdef.h \
 m_swap.h screen.h command.h \
 st_stuff.h d_player.h d_items.h \
 p_pspr.h m_fixed.h tables.h \
 info.h d_think.h p_mobj.h \
 doomdata.h console.h i_video.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/g_input.o: g_input.c doomdef.h doomtype.h g_state.h m_swap.h g_input.h \
 d_event.h keys.h command.h hu_stuff.h w_wad.h wi_stuff.h d_player.h \
 d_items.h p_pspr.h m_fixed.h tables.h info.h d_think.h p_mobj.h \
 doomdata.h d_ticcmd.h r_defs.h screen.h d_net.h console.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/screen.o: screen.c doomdef.h doomtype.h g_state.h m_swap.h screen.h \
 command.h console.h d_event.h am_map.h i_system.h d_ticcmd.h \
 i_video.h r_local.h tables.h m_fixed.h m_bbox.h r_main.h d_player.h \
 d_items.h p_pspr.h info.h d_think.h p_mobj.h doomdata.h r_data.h \
 r_defs.h r_state.h r_bsp.h r_segs.h r_plane.h r_sky.h r_things.h \
 sounds.h r_draw.h m_argv.h v_video.h st_stuff.h hu_stuff.h w_wad.h \
 wi_stuff.h z_zone.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/console.o: console.c doomdef.h doomtype.h g_state.h m_swap.h console.h \
 d_event.h g_game.h doomstat.h doomdata.h d_player.h d_items.h \
 p_pspr.h m_fixed.h tables.h info.h d_think.h p_mobj.h d_ticcmd.h \
 d_clisrv.h d_netcmd.h command.h g_input.h keys.h hu_stuff.h w_wad.h \
 wi_stuff.h r_defs.h screen.h sounds.h st_stuff.h s_sound.h v_video.h \
 i_video.h z_zone.h i_system.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/command.o: command.c doomdef.h doomtype.h g_state.h m_swap.h \
 doomstat.h doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h \
 command.h console.h d_event.h z_zone.h m_misc.h w_wad.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/d_netcmd.o: d_netcmd.c doomdef.h doomtype.h g_state.h m_swap.h \
 console.h d_event.h command.h d_netcmd.h d_clisrv.h d_ticcmd.h \
 i_system.h dstrings.h d_englsh.h g_game.h doomstat.h doomdata.h \
 d_player.h d_items.h p_pspr.h m_fixed.h tables.h info.h d_think.h \
 p_mobj.h hu_stuff.h w_wad.h wi_stuff.h r_defs.h screen.h g_input.h \
 keys.h m_menu.h r_local.h m_bbox.h r_main.h r_data.h r_state.h \
 r_bsp.h r_segs.h r_plane.h r_sky.h r_things.h sounds.h r_draw.h \
 p_inter.h p_setup.h s_sound.h m_misc.h am_map.h byteptr.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/d_clisrv.o: d_clisrv.c doomdef.h doomtype.h g_state.h m_swap.h \
 command.h i_net.h i_system.h d_ticcmd.h d_event.h d_net.h d_netcmd.h \
 d_clisrv.h d_main.h w_wad.h g_game.h doomstat.h doomdata.h d_player.h \
 d_items.h p_pspr.h m_fixed.h tables.h info.h d_think.h p_mobj.h \
 hu_stuff.h wi_stuff.h r_defs.h screen.h keys.h m_argv.h m_menu.h \
 console.h d_netfil.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/r_splats.o: r_splats.c r_draw.h r_defs.h m_fixed.h doomtype.h \
 d_think.h p_mobj.h tables.h doomdata.h doomdef.h g_state.h m_swap.h \
 info.h screen.h command.h r_main.h d_player.h d_items.h p_pspr.h \
 d_ticcmd.h r_data.h r_state.h r_plane.h r_splats.h w_wad.h z_zone.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/g_state.o: g_state.c g_state.h doomtype.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/d_netfil.o: d_netfil.c doomdef.h doomtype.h g_state.h m_swap.h \
 doomstat.h doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h \
 command.h g_game.h d_event.h i_net.h i_system.h m_argv.h d_net.h \
 d_netfil.h z_zone.h md5.h filesrch.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/t_func.o: t_func.c command.h doomstat.h doomtype.h d_main.h \
 g_game.h hu_stuff.h info.h m_random.h p_mobj.h p_tick.h \
 p_spec.h p_inter.h r_data.h r_main.h r_segs.h s_sound.h w_wad.h \
 z_zone.h t_parse.h t_spec.h t_script.h t_oper.h t_vari.h t_func.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/t_oper.o: t_oper.c command.h doomstat.h doomtype.h z_zone.h \
 t_parse.h t_vari.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/t_parse.o: t_parse.c command.h doomtype.h doomdef.h s_sound.h \
 w_wad.h z_zone.h t_parse.h t_spec.h t_script.h t_oper.h t_vari.h t_func.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/t_prepro.o: t_prepro.c command.h doomtype.h w_wad.h z_zone.h \
 t_parse.h t_spec.h t_script.h t_oper.h t_vari.h t_func.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/t_script.o: t_script.c command.h g_game.h r_state.h p_info.h \
 p_mobj.h p_spec.h p_setup.h w_wad.h z_zone.h t_parse.h t_script.h \
 t_vari.h t_func.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/t_spec.o: t_spec.c command.h doomtype.h z_zone.h \
 t_parse.h t_spec.h t_vari.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/i_main.o: $(INTERFACE)/i_main.c doomdef.h \
 doomtype.h g_state.h m_swap.h \
 m_argv.h d_main.h d_event.h \
 w_wad.h i_system.h d_ticcmd.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/md5.o: md5.c md5.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/p_chex.o: p_chex.c
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/filesrch.o: $(INTERFACE)/filesrch.c filesrch.h d_netfil.h
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/b_game.o: b_game.c b_bot.h b_game.h b_look.h \
 b_node.h doomdef.h doomstat.h m_random.h p_local.h \
 z_zone.h command.h r_state.h v_video.h m_argv.h \
 p_setup.h r_main.h r_things.h g_game.h d_net.h

$(O)/b_look.o: b_look.c b_bot.h b_game.h b_look.h \
 b_node.h g_game.h r_defs.h p_local.h m_random.h \
 r_main.h z_zone.h

$(O)/b_node.o: b_node.c b_bot.h b_node.h z_zone.h \
 g_game.h r_state.h p_maputl.h r_main.h p_local.h \
 p_setup.h

$(O)/b_search.o: b_search.c g_game.h r_defs.h p_local.h \
 m_random.h r_main.h z_zone.h b_search.h b_node.h

endif

#############################################################
#
#############################################################
