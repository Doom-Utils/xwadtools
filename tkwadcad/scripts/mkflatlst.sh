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

exec 1> flats.lst
printf "# Flat list for %s generated with mkflatlst.sh\n\n" `basename $1`

lswad -F $1 | while read name; do
  file=`echo $name | tr '[A-Z]' '[a-z]'`
  file=$file.ppm
  printf "%s %s\n" $name $file
done
