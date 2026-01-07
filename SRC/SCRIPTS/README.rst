README for Doom Legacy [VERSION], svn[SVNREV]
======================================
Released [DATE]

What you will need
------------------

1. Download the Doom Legacy executable for your system (e.g. doomlegacy_[VERSION]_windows_32_sdl.zip)
2. Download the shared files in doomlegacy_[VERSION]_common.zip (contains legacy.wad [WADVERSION] and the docs)
3. Linux only: Install the run-time libraries (SDL 1.2.x, SDL_mixer 1.2.x), unless you already have them.
   These libraries are included with the Windows executable.
4. Obtain one or more IWADs containing the game data (Doom, Final Doom, UltDoom, Plutonia, TNT, Heretic, FreeDoom, etc.)

We provide precompiled executables for Linux and Win32, built with default options.
You can also compile an executable yourself, the source code package contains the compilation instructions.

Doom Legacy SDL version requires the SDL and SDL_mixer run-time libraries.
Using SDL is recommended, but there are some other compile options (like X11 native).


Play setup
----------

Extract the executable package and the common package in the same folder.


Configuration and saves
~~~~~~~~~~~~~~~~~~~~~~~

Doom Legacy will create a hidden directory in your home directory to keep your
config and game save files.  This directory is named ".doomlegacy", or
something similar as details vary by operating system.
If you compile your own Doom Legacy, then this directory name
is set by DEFAULTDIR1 and DEFAULTDIR2 of the doomdef.h file.
The alternative home directory DEFHOME is used if a user specific one cannot be made.

When you use a drawmode, and if that drawmode config file does not
exist, then the settings from the main config file will be used instead.
If the drawmode config file exists, then settings from that drawmode config file
will be displayed, and in force, while that drawmode is selected.

To create a drawmode config file, to hold video settings for that
drawmode, follow these steps.

1. Go to video options menu.
2. Press F3 (changes to drawmode config file editing)
3. Go to drawmode menu.
4. Select the drawmode.
5. Press 'C' to create an initial drawmode config file.
6. Goto video menu.
7. If there is a setting you want in the drawmode config file,
   go to it and press INSERT.
8. Set your default video mode.  Remember to press 'D' to set default.
9. Repeat for other drawmodes.
10. Press F1 to close config editing.


WAD files
~~~~~~~~~

Doom Legacy will search for WADs in predefined directories before it looks in
the current run directory.
To find legacy.wad, it searches LEGACYWADDIR.
For the game IWAD and other WAD files, it searches several directories.
These are defined by DEFWADS01 to DEFWADS20 in the doomdef.h file.
You can keep shared WADs in one of the system-wide directories, and
personal WADs in one of the user relative directories, and Doom Legacy
will find WADs in those directories without having to specify the directory.
Doom Legacy will also search sub-directories, so you can organize your WADs.


Predefined directories
~~~~~~~~~~~~~~~~~~~~~~

In the DEFWADS strings, ~ denotes the user home directory.

::

  Windows:
  DEFHOME   "\legacyhome"
  DEFAULTDIR1 "doomlegacy"
  DEFAULTDIR2 "legacy"
  DEFWADS01  "~\games\doom"
  DEFWADS02  "~\games\doomwads"
  DEFWADS03  "~\games\doomlegacy\wads"
  DEFWADS04  "\doomwads"
  DEFWADS05  "\games\doomwads"
  DEFWADS06  "\games\doom"
  DEFWADS10  "\Program Files\doomlegacy\wads"

  Linux, FreeBSD, and Unix:
  The binary can also be installed in "/usr/local/bin".
  DEFHOME    "legacyhome"
  DEFAULTDIR1 ".doomlegacy"
  DEFAULTDIR2 ".legacy"
  LEGACYWADDIR  "/usr/local/share/games/doomlegacy"
  DEFWADS01  "~/games/doomlegacy/wads"
  DEFWADS02  "~/games/doomwads"
  DEFWADS03  "~/games/doom"
  DEFWADS04  "/usr/local/share/games/doomlegacy/wads"
  DEFWADS05  "/usr/local/share/games/doomwads"
  DEFWADS06  "/usr/local/share/games/doom"
  DEFWADS07  "/usr/local/games/doomlegacy/wads"
  DEFWADS08  "/usr/local/games/doomwads"
  DEFWADS09  "/usr/share/games/doom"
  DEFWADS10  "/usr/share/games/doomlegacy/wads"
  DEFWADS11  "/usr/share/games/doomwads"
  DEFWADS12  "/usr/games/doomlegacy/wads"
  DEFWADS13  "/usr/games/doomwads"
  DEFWADS16  "~/games/doomlegacy"
  DEFWADS17  "/usr/local/share/games/doomlegacy"
  DEFWADS18  "/usr/local/games/doomlegacy"
  DEFWADS19  "/usr/share/games/doomlegacy"
  DEFWADS20  "/usr/games/doomlegacy"


Other versions
--------------

There are some options to compile a version of Doom Legacy for other systems.

Linux X11-windows native (tested, have binaries)
  - requires X11 (such as X11R6), the usual Linux window system that is
    included with every Linux package (only tiny Linux systems running
    standalone would be without this).

FreeBSD X11-windows native (tested by at least one user)
  - similar to Linux X11 but has some slight library differences.

NETBSD (tested by at least one user)
  - a few library differences

Linux GGI (old and not tested lately)
  - requires GGI libraries

Unixware, and Openserver5 versions (untested lately, usability is unknown)
  - has different music servers

Windows Direct-X native (may or may not work depending upon your header files)
  - requires Direct-X 7 (at least).
  - with or without FMOD

Mac SDL (code exists, is not working, needs a tester).

Macos native (old and not tested lately).

OS2 native (old and not tested lately).

DOS native (old and not tested lately).
  - requires Allegro
  - requires dos compiler


Compiling from source
---------------------

See docs/README_Compiling.txt
