#!/bin/sh
# allmaph.sh: Shell script to generate all Heretic maps

dmpsmap -w HERETIC.WAD -fp -e 1 -m 1 -p e1m1.ps > /dev/null
dmpsmap -w HERETIC.WAD -fp -e 1 -m 2 -p e1m2.ps > /dev/null
dmpsmap -w HERETIC.WAD +fp -e 1 -m 3 -p e1m3.ps > /dev/null
dmpsmap -w HERETIC.WAD -fp -e 1 -m 4 -p e1m4.ps > /dev/null
dmpsmap -w HERETIC.WAD +fp -e 1 -m 5 -p e1m5.ps > /dev/null
dmpsmap -w HERETIC.WAD -fp -e 1 -m 6 -p e1m6.ps > /dev/null
dmpsmap -w HERETIC.WAD +fp -e 1 -m 7 -p e1m7.ps > /dev/null
dmpsmap -w HERETIC.WAD +fp -e 1 -m 8 -p e1m8.ps > /dev/null
dmpsmap -w HERETIC.WAD +fp -e 1 -m 9 -p e1m9.ps > /dev/null

dmpsmap -w HERETIC.WAD -fp -e 2 -m 1 -p e2m1.ps > /dev/null
dmpsmap -w HERETIC.WAD -fp -e 2 -m 2 -p e2m2.ps > /dev/null
dmpsmap -w HERETIC.WAD +fp -e 2 -m 3 -p e2m3.ps > /dev/null
dmpsmap -w HERETIC.WAD -fp -e 2 -m 4 -p e2m4.ps > /dev/null
dmpsmap -w HERETIC.WAD +fp -e 2 -m 5 -p e2m5.ps > /dev/null
dmpsmap -w HERETIC.WAD +fp -e 2 -m 6 -p e2m6.ps > /dev/null
dmpsmap -w HERETIC.WAD +fp -e 2 -m 7 -p e2m7.ps > /dev/null
dmpsmap -w HERETIC.WAD -fp -e 2 -m 8 -p e2m8.ps > /dev/null
dmpsmap -w HERETIC.WAD +fp -e 2 -m 9 -p e2m9.ps > /dev/null

dmpsmap -w HERETIC.WAD +fp -e 3 -m 1 -p e3m1.ps > /dev/null
dmpsmap -w HERETIC.WAD +fp -e 3 -m 2 -p e3m2.ps > /dev/null
dmpsmap -w HERETIC.WAD +fp -e 3 -m 3 -p e3m3.ps > /dev/null
dmpsmap -w HERETIC.WAD -fp -e 3 -m 4 -p e3m4.ps > /dev/null
dmpsmap -w HERETIC.WAD +fp -e 3 -m 5 -p e3m5.ps > /dev/null
dmpsmap -w HERETIC.WAD +fp -e 3 -m 6 -p e3m6.ps > /dev/null
dmpsmap -w HERETIC.WAD +fp -e 3 -m 7 -p e3m7.ps > /dev/null
dmpsmap -w HERETIC.WAD +fp -e 3 -m 8 -p e3m8.ps > /dev/null
dmpsmap -w HERETIC.WAD +fp -e 3 -m 9 -p e3m9.ps > /dev/null

dmpsmap -w HERETIC.WAD -fp -e 4 -m 1 -p e4m1.ps > /dev/null
