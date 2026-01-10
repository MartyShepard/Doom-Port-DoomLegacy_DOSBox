#! /bin/bash
# Bash script for building Doom Legacy releases and uploading them to SourceForge.net
# Copyright (C) 2014-2020 by Doom Legacy Team.


#========================================================
# Set these variables first.

username=""                                 # sourceforge.net username of the person doing the upload
srcdir="/home/doomlegacy/legacy_one/trunk"  # Doom Legacy source tree location
workdir="."                                 # working directory, where all the packages are built

readmefile="README.rst"                     # name of the README file to be included in every binary release package
legacywadfile="$srcdir/bin/legacy.wad"      # prebuilt legacy.wad location.
wadtool="$srcdir/bin/wadtool"               # prebuilt wadtool executable, for updating legacy.wad
exefile="$srcdir/bin/doomlegacy"            # prebuilt Doom Legacy executable to package
binary_spec="linux2.6_64_sdl"               # binary architecture and platform
#binary_spec="windows_32_sdl"



# read the Doom Legacy version number from the source code
ver=$(sed -n -e "s/^const int  VERSION  = \([0-9]\)\([0-9]*\).*$/\1.\2/p" $srcdir/src/d_main.c)
rev=$(sed -n -e "s/^const int  REVISION = \([0-9]*\).*$/\1/p" $srcdir/src/d_main.c)
version=$ver.$rev

# read legacy.wad version from the VERSION lump
wadversion=$(sed -n -e "s/^Doom Legacy WAD V\([0-9].[0-9]*\).*$/\1/p" $srcdir/resources/VERSION.txt)

# SVN revision
svnrev=$(svn info $srcdir | sed -n -e "s/^Revision: \([0-9]*\)/\1/p")

# today's date
releasedate=$(date --rfc-3339='date')

# prefix for the package names
prefix="doomlegacy_"$version

# temporary packaging directory
tempdir=$prefix

# directory where all the packages are collected for upload
releasedir=$version


#========================================================

echo Doom Legacy $version, svn$svnrev

# make the release files directory 
mkdir -p $workdir
cd $workdir
mkdir -p $releasedir
mkdir -p $tempdir

case "$1" in
    source)
	echo "Building the source package from src tree at "$srcdir

	srcname=$prefix"_source"
	# temporary packaging directory
	tempdir=$tempdir"_source"
	# copy the source tree, remove build cruft
	cp -ar $srcdir $tempdir
	rm $tempdir/bin/* $tempdir/objs/* $tempdir/dep/*
	# into a tar package
	tar -cjf $releasedir/$srcname".tar.bz2" $tempdir
	;;

    common)
	echo "Building the common package, legacy.wad at "$legacywadfile

	# Break legacy.wad into lumps, update the lumps, then rebuild the WAD
	echo "Building updated legacy.wad, version "$wadversion
	waddir=$tempdir"_wad"
	mkdir -p $waddir
	cp -a  $legacywadfile $waddir
	cd $waddir
	$wadtool -x legacy.wad
	cp -a $srcdir/resources/* .
	$wadtool -c ../$tempdir/legacy.wad legacy.wad.inventory
	cd ..

	# add the documentation
	cp -ar $srcdir/docs $tempdir
	# into a zip package
	zip -r $releasedir/$prefix"_common.zip" $tempdir
        ;;

    binary)
	echo "Building a binary package for "$binary_spec

	# prepare the README file
	sed -e "s/\[DATE\]/$releasedate/" -e "s/\[VERSION\]/$version/" -e "s/\[WADVERSION\]/$wadversion/" -e "s/\[SVNREV\]/$svnrev/" <$srcdir/scripts/$readmefile >$tempdir/$readmefile
	cp -a $exefile $tempdir
	tar -cjf $releasedir/$prefix"_"$binary_spec".tar.bz2" $tempdir
	#zip -r $releasedir/$prefix"_"$binary_spec".zip" $tempdir
	# put a copy of the README file in the release directory
	cp -a $tempdir/$readmefile $releasedir
	;;
         
    upload)
	# Upload all the built release packages to Sourceforge.net file release system.
	# note the trailing slash after the source dir, which means "copy the contents of the directory".
	rsync -aiv -e ssh $releasedir/ $username@frs.sourceforge.net:/home/frs/project/doomlegacy/$version
        ;;

    upload_docs)
	# Upload the latest version of the docs to the website.
	# Note the trailing slash after the source dir, which means "copy the contents of the directory".
	rsync -aiv -e ssh $srcdir/docs/ $username,doomlegacy@web.sourceforge.net:htdocs/docs
        ;;

    clean)
	# Clean up (delete) the auxiliary directories.
	rm -r $tempdir
	rm -r $tempdir"_source"
	rm -r $tempdir"_wad"
        ;;

    *)
        echo $"Usage: $0 {source|common|binary|upload|docs|clean}"
        exit 1
esac
echo "Done."
exit
