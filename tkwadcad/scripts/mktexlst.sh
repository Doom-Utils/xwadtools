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

exec 1> textures.lst
printf "# Wall texture list for %s generated with mktexlst.sh\n\n" `basename $1`
printf "\055 \055 0 0 0\n"

lswad -Tl $1 | while read name width height patches; do
  file=`echo $name | tr '[A-Z]' '[a-z]'`
  file=$file.ppm
  printf "%s %s %s %s %s\n" $name $file $width $height $patches
done
