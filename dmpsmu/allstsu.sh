#!/bin/sh
# allstsu.sh: Shell script to list statistics of the E4 Ultimate DOOM maps

dmpsmu -w DOOMU.WAD <<! | sed '/for help/d' > allstsu.lis
a 4 1
a 4 2
a 4 3
a 4 4
a 4 5
a 4 6
a 4 7
a 4 8
a 4 9
q
!
