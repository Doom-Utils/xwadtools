#!/bin/sh
# dmps2ul.sh: Script to output part of double page DOOM map
# eg. dmps2ul.sh -u map.ps > map_u.ps
#  or dmps2ul.sh -l map.ps | lpr

if [ $# -ne 2 -o \( "$1" != "-u" -a "$1" != "-l" \) ]; then
        echo "usage: `basename $0` -u/-l PS_filename" >&2
        exit 1
fi
if [ ! -r $2 ]; then
	echo "$2: No such file" >&2
	exit 1
fi
if [ "`head -1 $2 | grep '^%! DMPS.*: DooM PostScript Map'`" = "" -o \
     "`grep '^% center double page$' $2`" = "" ]; then
	echo "$2: Not a double page DOOM PostScript map" >&2
	exit 1
fi

case $1 in
-u)	sed '/^%% start upper/,/^%% end upper/s/^%//' < $2 ;;
-l)	sed '/^%% start lower/,/^%% end lower/s/^%//' < $2 ;;
esac

