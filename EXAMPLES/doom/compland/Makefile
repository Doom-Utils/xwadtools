# Location of your Doom][ IWAD file
DOOM2WAD=/usr/local/games/xdoom/doom2.wad

compland.wad: compland.lwad compland.gwad compland.mwad 
	rm -f compland.wad
	wadcat -o compland.wad compland.lwad compland.mwad compland.gwad

compland.mwad:
	mkwad compland.mwad D_RUNNIN sdreams.mus

compland.lwad: compland.map
	idbsp -q compland.map compland.lwad

compland.gwad: compland.wgc rocksb.ppm rocksg.ppm palmtree.ppm smaltree.ppm \
	tree1.ppm tree2.ppm k_a1.ppm k_a2.ppm k_a3.ppm k_a4.ppm
	wadext $(DOOM2WAD) PNAMES pnames.raw
	wadext $(DOOM2WAD) TEXTURE1 texture1.raw
	wadgc compland.wgc compland.gwad
	rm temp.* pnames.raw texture1.raw

ummarb1u.ppm: ummarb1.pov doom.inc
	x-povray ummarb1.ini

ummarb1.ppm: ummarb1u.ppm doom.map
	ppmquant -map doom.map ummarb1u.ppm >ummarb1.ppm

ummarbf1u.ppm: ummarb1.pov doom.inc
	x-povray ummarbf1.ini

ummarbf1.ppm: ummarbf1u.ppm doom.map
	ppmquant -map doom.map ummarbf1u.ppm >ummarbf1.ppm

doom.inc:
	mkpopal -c doom >doom.inc

doom.map:
	mkqmap -c doom >doom.map

clean:
	rm -f *.bak compland.lwad compland.gwad compland.mwad \
	doom.inc doom.map ummarb1u.ppm ummarbf1u.ppm
