#!/bin/sh
# allstsh.sh: Shell script to list statistics of all Heretic maps

dmpsmu -w HERETIC.WAD <<! | sed '/for help/d' > allstsh.lis
a 1 1
a 1 2
a 1 3
a 1 4
a 1 5
a 1 6
a 1 7
a 1 8
a 1 9
a 2 1
a 2 2
a 2 3
a 2 4
a 2 5
a 2 6
a 2 7
a 2 8
a 2 9
a 3 1
a 3 2
a 3 3
a 3 4
a 3 5
a 3 6
a 3 7
a 3 8
a 3 9
a 4 1
q
!
