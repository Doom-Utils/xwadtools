#!/bin/sh

# Copyright (C) 1998-2000 by Udo Munk (munkudo@aol.com)
#
# Permission to use, copy, modify, and distribute this software
# and its documentation for any purpose and without fee is
# hereby granted, provided that the above copyright notice
# appears in all copies and that both that copyright notice and
# this permission notice appear in supporting documentation.
# The author and contibutors make no representations about the
# suitability of this software for any purpose. It is provided
# "as is" without express or implied warranty.

trap 'rm -f /tmp/tmp$$.wad' 0

echo "Compiling map: $1"
echo "------------------------------------------------------------"
wadlc $1 /tmp/tmp$$.wad || {
	echo "*** ERROR ***"; exit 1
}
echo "------------------------------------------------------------"

echo; echo
echo "Building nodes:"
echo "------------------------------------------------------------"
warm -q /tmp/tmp$$.wad $2 || {
	echo "*** ERROR ***"; exit 1
}
echo "------------------------------------------------------------"

exit 0
