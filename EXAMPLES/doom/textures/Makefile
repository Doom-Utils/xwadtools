# Location of your Doom][ IWAD file
DOOM2WAD=/usr/local/games/xdoom/doom2.wad

textures.gwad: textures.wgc umgr1.ppm umsky1.ppm umsky2.ppm
	wadext $(DOOM2WAD) PNAMES pnames.raw
	wadext $(DOOM2WAD) TEXTURE1 texture1.raw
	wadgc textures.wgc textures.gwad
	rm temp.* pnames.raw texture1.raw

umgr1u.ppm: umgr1.pov doom.inc
	x-povray umgr1.ini

umgr1.ppm: umgr1u.ppm doom.map
	ppmquant -map doom.map umgr1u.ppm >umgr1.ppm

umsky1u.ppm: umsky1.pov doom.inc
	x-povray umsky1.ini

umsky1.ppm: umsky1u.ppm doom.map
	ppmquant -map doom.map umsky1u.ppm >umsky1.ppm

umsky2u.ppm: umsky2.pov doom.inc
	x-povray umsky2.ini

umsky2.ppm: umsky2u.ppm doom.map
	ppmquant -map doom.map umsky2u.ppm >umsky2.ppm

doom.inc:
	mkpopal -c doom >doom.inc

doom.map:
	mkqmap -c doom >doom.map

clean:
	rm  -f *.raw temp.* doom.inc doom.map \
	umgr1u.ppm umsky1u.ppm umsky2u.ppm

allclean:
	make clean
	rm -f *.ppm *.gwad
