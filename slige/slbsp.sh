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
# nodes with bsp. The output PWAD is ready for playing, reject map might
# be wrong, in this case you need to play with the threshold.
#

trap 'rm -f tmp*-$$.wad' 0

if [ $# -ne 2 ]
then
	echo "usage: $0 config-file wad-file"
	exit 1
fi

slige -doom2 -config $1 tmp1-$$.wad || {
	echo "ops, slige problem"
	exit 1
}

bsp tmp1-$$.wad -o $2 || {
	echo "ops, bsp problem"
	exit 1
}

wreject $2 MAP01 1000 || {
	echo "ops, wreject problem"
	exit 1
}

exit 0
