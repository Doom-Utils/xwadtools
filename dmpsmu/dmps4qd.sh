#!/bin/sh
# dmps4qd.sh: Script to output part of quadruple page DOOM map
# eg. dmps4qd.sh -ur map.ps > map_ur.ps
#  or dmps4qd.sh -ll map.ps | lpr

if [ $# -ne 2 -o \( "$1" != "-ul" -a "$1" != "-ur" -a "$1" != "-ll" -a "$1" != "-lr" \) ]; then
        echo "usage: `basename $0` -ul/-ur/-ll/-lr PS_filename" >&2
        exit 1
fi
if [ ! -r $2 ]; then
	echo "$2: No such file" >&2
	exit 1
fi
if [ "`head -1 $2 | grep '^%! DMPS.*: DooM PostScript Map'`" = "" -o \
     "`grep '^% center quadruple page$' $2`" = "" ]; then
	echo "$2: Not a quadruple page DOOM PostScript map" >&2
	exit 1
fi

case $1 in
-ul)	sed '/^%% start upper-left/,/^%% end upper-left/s/^%//' < $2 ;;
-ur)	sed '/^%% start upper-right/,/^%% end upper-right/s/^%//' < $2 ;;
-ll)	sed '/^%% start lower-left/,/^%% end lower-left/s/^%//' < $2 ;;
-lr)	sed '/^%% start lower-right/,/^%% end lower-right/s/^%//' < $2 ;;
esac

