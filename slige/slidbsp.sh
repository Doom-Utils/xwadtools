#!/bin/sh

# Copyright (C) 1999-2000 by Udo Munk (munkudo@aol.com)
#
# Permission to use, copy, modify, and distribute this software
# and its documentation for any purpose and without fee is
# hereby granted, provided that the above copyright notice
# appears in all copies and that both that copyright notice and
# this permission notice appear in supporting documentation.
# The author and contibutors make no representations about the
# suitability of this software for any purpose. It is provided
# "as is" without express or implied warranty.
#
# Example shell script to generate a Doom ][ level with slige and build
# nodes with idbsp. The output PWAD is ready for playing.
#

trap 'rm -f slcust.wad tmp*-$$.wad tmp*-$$.map' 0

if [ $# -ne 2 ]
then
	echo "usage: $0 config-file wad-file"
	exit 1
fi

slige -doom2 -cwad slcust.wad || {
	echo "ops, slige problem generating custom PWAD"
	exit 1
}

slige -doom2 -nocustom -noslinfo -config $1 tmp1-$$.wad || {
	echo "ops, slige problem generating level map"
	exit 1
}

wadldc MAP01 tmp1-$$.wad tmp1-$$.map || {
	echo "ops, wadldc problem decompiling level map"
	exit 1
}

idbsp -q tmp1-$$.map tmp1-$$.wad || {
	echo "ops, idbsp problem"
	exit 1
}

wadcat -o $2 tmp1-$$.wad slcust.wad || {
	echo "ops, wadcat problem concatenating PWADS"
	exit 1
}

exit 0
