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

# tkwadcad passes the following arguments to the engine run scripts:
#
# $1 is the PWAD file to run
# $2 is the episode
# $3 is the map
# $4... generic engine options

cd /usr/local/games
exec lxdoom -width 320 -height 200 -2 -file $1 -warp $2 $3 $4 $5 $6 $7 $8
