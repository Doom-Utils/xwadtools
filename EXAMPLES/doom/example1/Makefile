wadex1.wad: wadex1d1.wad wadex1d2.wad
	rm wadex1.wad
	wadcat -o wadex1.wad wadex1d1.wad wadex1d2.wad
	rm wadex1d1.wad wadex1d2.wad

wadex1d1.wad: wadex1.map
	sed -e 's/LEVEL_START 0 1/LEVEL_START 1 1/' wadex1.map >tmp.map
	idbsp -q tmp.map wadex1d1.wad
	rm tmp.map

wadex1d2.wad: wadex1.map
	idbsp -q wadex1.map wadex1d2.wad

clean:
