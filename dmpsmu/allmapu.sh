#!/bin/sh
# allmapu.sh: Shell script to generate the E4 Ultimate DOOM maps
#             use DOOMU.WAD to distinguish from regular DOOM.WAD

dmpsmap -w DOOMU.WAD -fp -e 4 -m 1 -p e4m1.ps > /dev/null
dmpsmap -w DOOMU.WAD +fp -e 4 -m 2 -p e4m2.ps > /dev/null
dmpsmap -w DOOMU.WAD -fp -e 4 -m 3 -p e4m3.ps > /dev/null
dmpsmap -w DOOMU.WAD -fp -e 4 -m 4 -p e4m4.ps > /dev/null
dmpsmap -w DOOMU.WAD -fp -e 4 -m 5 -p e4m5.ps > /dev/null
dmpsmap -w DOOMU.WAD -fp -e 4 -m 6 -p e4m6.ps > /dev/null
dmpsmap -w DOOMU.WAD +fp -e 4 -m 7 -p e4m7.ps > /dev/null
dmpsmap -w DOOMU.WAD +fp -e 4 -m 8 -p e4m8.ps > /dev/null
dmpsmap -w DOOMU.WAD +fp -e 4 -m 9 -p e4m9.ps > /dev/null

