#!/usr/bin/perl

# Option arguments may be intermixed with PWADs.
#
# The resulting output file has one level, which is the merge of all
# the levels in the input PWADs.  It is not suitable for play - you
# must rebuild the engine precalculation data (BSP tree, blockmap and
# reject map).
#
# Options:
# -t[abc][xyz]<number> sets [abc][xyz] to <number>:
#  for horizontal translation/rotation/shearing
#    x'= ax*x + bx*y + cx
#    y'= ax*y + by*y + cy
#  for vertical alteration
#    z'= az*z + cz;
#  (defaults are ax=by=az=1.0, others=0.0, ie no change)
# -r<angle>
#  for rotation of Things - in degrees, positive anticlockwise

# Copyright 1995 Ian Jackson.

# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# It is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with GNU Emacs; see the file COPYING.  If not, write to
# the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

undef $/;

$vertexoff= 0;
$linedefoff= 0;
$triggeroff= 0;
$sidedefoff= 0;
$sectoroff= 0;

$outfn= 'tmpm.wad';

$maxtrigused= -1;
$maxtrigchange= 100;

$tp{'a','x'}= 1.0;
$tp{'b','y'}= 1.0;
$tp{'a','z'}= 1.0;

while (@ARGV) {
    $_= shift(@ARGV);
    if (m/^-t([abc])([xyz])(\S+)$/) {
        $tp{$1,$2}= $3+0.0;
    } elsif (m/^-r(-?\d+)$/) {
        $rotate= $1;
    } elsif (m/^-/) {
        die;
    } else {
        &readwad($_);
    }
}

$xyscale1= sqrt($tp{'a','x'}*$tp{'a','x'} + $tp{'a','y'}*$tp{'a','y'});
$xyscale2= sqrt($tp{'b','x'}*$tp{'b','x'} + $tp{'b','y'}*$tp{'b','y'});

$sdiff= $xyscale1 / $xyscale2;
if ($sdiff < 0.99 || $sdiff > 1.01) { print "WARNING: shear !\n"; }

length($levelname) || die;

$dataoffset= 12;
$totallumps= 0;

&makelump($levelname,'');
&makelump('THINGS',$things);
&makelump('LINEDEFS',$linedefs);
&makelump('SIDEDEFS',$sidedefs);
&makelump('VERTEXES',$vertices);
for $f ('SEGS','SSECTORS','NODES') { &makelump($f,$englump{$f}); }
&makelump('SECTORS',$sectors);
for $f ('REJECT','BLOCKMAP') { &makelump($f,$englump{$f}); }

$output= 'PWAD'.pack('VV',$totallumps,$dataoffset).$data.$directory;

open(O,">$outfn") || die;
print(O $output) || die;
close(O) || die;

print "Wrote $outfn ($totallumps lumps).\n";

sub makelump {
    printf "Output lump %s, length %lx, offset %lx.\n",$_[0],length($_[1]),$dataoffset;
    $directory .= pack('VVa8',$dataoffset,length($_[1]),$_[0]);
    $data .= $_[1];
    $dataoffset += length($_[1]);
    $totallumps++;
}

sub readwad {
    $fn= $_[0];
    open(W,$fn) || die; $w=<W>;  close(W);
    $c= substr($w,0,4);  $c eq 'PWAD' || die ">$c<";
    $wlen= length($w);
    $nlumps= &i($w,4,4,'V');
    $diroff= &i($w,8,4,'V');
    printf "%s: %d directory entries.\n",$fn,$nlumps;
    for ($i=0; $i<$nlumps; $i++) {
        $destart= $diroff+$i*16;
        $thisoff= &i($w,$destart,4,'V');
        $thissize= &i($w,$destart+4,4,'V');
        $thisname= &i($w,$destart+8,8,'A8');
        $thislump= substr($w,$thisoff,$thissize);
#        printf "%s: %-8s offset=%lx length=%lx\n", $fn,$thisname,$thisoff,$thissize;
        length($thislump) == $thissize || die "$thissize, ".length($thislump);
        if ($thisname =~ m/^E\dM\d$/ || $thisname =~ m/^MAP\d\d$/) {
            print "$fn: Merging data from level $thisname.\n";
            $levelname= $thisname;
            $vertexoff += $vertexcnt; $vertexcnt= 0;
            $linedefoff += $linedefcnt; $linedefcnt= 0;
            $sidedefoff += $sidedefcnt; $sidedefcnt= 0;
            $sectoroff += $sectorcnt; $sectorcnt= 0;
            $thingoff += $thingcnt; $thingcnt= 0;
            $triggeroff = $maxtrigused+1;
        } elsif ($thisname eq 'THINGS') {
            ($thissize % 10) && die $thissize;
            for ($j=0; $j<$thissize; $j+=10) {
                $x= &i($thislump,0,2,'v');
                $y= &i($thislump,2,2,'v');
                $things.= &transco($x,$y,'x');
                $things.= &transco($x,$y,'y');
                $things.= pack('v', (&i($thislump,4,2,'v') + $rotate) % 360);
                $things.= substr($thislump,6,4);
                $thingcnt++;
#&paranoia(10,$things);
                $thislump= substr($thislump,10);
            }
            print "$fn: Copied THINGS ($thingcnt).\n";
        } elsif ($thisname eq 'LINEDEFS') {
            ($thissize % 14) && die $thissize;
            for ($j=0; $j<$thissize; $j+=14) {
                $linedefs.= pack('v',$vertexoff+&i($thislump,0,2,'v'));
                $linedefs.= pack('v',$vertexoff+&i($thislump,2,2,'v'));
                $linedefs.= substr($thislump,4,4);
#                print "$linedefcnt\n";
                $linedefs.= &transtrigger(&i($thislump,8,2,'v'));
                $linedefs.= &trans(&i($thislump,10,2,'v'),$sidedefoff);
                $linedefs.= &trans(&i($thislump,12,2,'v'),$sidedefoff);
#&paranoia(14,$linedefs);
                $linedefcnt++;
                $thislump= substr($thislump,14);
            }
            print "$fn: Copied LINEDEFS ($linedefcnt).\n";
        } elsif ($thisname eq 'SIDEDEFS') {
            ($thissize % 30) && die $thissize;
            for ($j=0; $j<$thissize; $j+=30) {
                $sidedefs.= pack('v',$xyscale*&i($thislump,0,2,'v'));
                $sidedefs.= pack('v',$tp{'a','z'}*&i($thislump,2,2,'v'));
                $sidedefs.= substr($thislump,4,24);
                $sidedefs.= &trans(&i($thislump,28,2,'v'),$sectoroff);
#&paranoia(30,$sidedefs);
                $sidedefcnt++;
                $thislump= substr($thislump,30);
            }
            print "$fn: Copied SIDEDEFS ($sidedefcnt).\n";
        } elsif ($thisname eq 'VERTEXES') {
            ($thissize % 4) && die $thissize;
            for ($j=0; $j<$thissize; $j+=4) {
                $x= &i($thislump,0,2,'v');
                $y= &i($thislump,2,2,'v');
                $vertices.= &transco($x,$y,'x');
                $vertices.= &transco($x,$y,'y');
#&paranoia(4,$vertices);
                $vertexcnt++;
                $thislump= substr($thislump,4);
            }
            print "$fn: Copied VERTicES ($vertexcnt).\n";
        } elsif ($thisname eq 'SECTORS') {
#$orglump= $thislump;
            ($thissize % 26) && die $thissize;
            for ($j=0; $j<$thissize; $j+=26) {
                $sectors.= &transco(&i($thislump,0,2,'v'),0,'z');
                $sectors.= &transco(&i($thislump,2,2,'v'),0,'z');
                $sectors.= substr($thislump,4,20);
                $sectors.= &transtrigger(&i($thislump,24,2,'v'));
#&paranoia(26,$sectors);
                $sectorcnt++;
                $thislump= substr($thislump,26);
            }
#$sectors eq $orglump || die;
#            print "$fn: Copied SECTORS ($sectorcnt).\n";
        } elsif ($thisname =~ m/^(SEGS|SSECTORS|NODES|REJECT|BLOCKMAP)$/) {
            print "$fn: Discarding engine data ($thisname).\n";
#            $englump{$thisname}= $thislump;
        } else {
            print "$fn: WARNING: Ignoring unknown lump $thisname, $thissize bytes.\n";
        }
    }
}

sub transco {
    # in-times-a in-times-b usecoord
    $_[0] -= 65536 if $_[0] >= 32768;
    $_[1] -= 65536 if $_[1] >= 32768;
    $newco= $_[0]*$tp{'a',$_[2]} + $_[1]*$tp{'b',$_[2]} + $tp{'c',$_[2]};
#    printf "%s: trans (%f,%f) =$_[2]=> %f\n", $fn, $_[0],$_[1], $newco;
    pack('v',$newco);
}

sub paranoia {
    ($xx=substr($thislump,0,$_[0])) eq
    ($yy=substr($_[1],$j)) ||
        die unpack("h*",$xx)." --\n".unpack("h*",$yy)." $_[0]";
}

sub transtrigger {
    # value => string
    $thistrigger= $_[0];
    if ($thistrigger != 0) {
        if ($thistrigger < $maxtrigchange) {
#        printf "changed trigger %d by %d\n", $thistrigger, $triggeroff;
            $thistrigger += $triggeroff;
        }
        if ($thistrigger < $maxtrigchange &&
            $thistrigger > $maxtrigused) { $maxtrigused = $thistrigger; }
    }
    pack('v',$thistrigger);
}
    
sub trans {
    # value, offset => string
    if (($_[0] & 0xffff) != 0xffff) { $_[0] += $_[1]; }
    pack('v',$_[0]);
}
            
sub i {
    # string, offset, length, unpack => value
    return unpack($_[3],substr($_[0],$_[1],$_[2]));
}
