#!/bin/sh
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# Usage : $1 -> source feeds, space separated
# 	  $2 -> other options (not used yet)
#
# Note : we do not yet resolve package name conflicts
#

# Directories
FEEDS_DIR=$TOPDIR/feeds
PACKAGE_DIR=$TOPDIR/package

# We work in the TOPDIR as defined in the caller Makefile
cd $TOPDIR
# This directory will be structured this way : feeds/feed-name
[ -d $FEEDS_DIR ] || mkdir -p $FEEDS_DIR


# Some functions we might call several times a run
delete_symlinks() {
	find $1 -type l | xargs -r rm -f
}

setup_symlinks() {
	# We assume that feeds do reproduce the hierarchy : section/package
	# so that we can make this structure be flat in $PACKAGE_DIR
	for dir in $(ls $1/)
	do
		ln -s $1/$dir/*/* $2/
	done
}

extract_feed_name() {
	# We extract the last name of the URL, maybe we should rename this as domain.tld.repository.name
	echo "$(echo $1 | sed -e "s/[^A-Za-z\.]\+/_/g")"
}

# We can delete symlinks every time we start this script, since modifications have been made in the $FEEDS_DIR anyway
delete_symlinks "$PACKAGE_DIR"
# Finally setup symlinks
setup_symlinks "$FEEDS_DIR" "$PACKAGE_DIR"
