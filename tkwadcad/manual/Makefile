tkwadcad.ps: tkwadcad.me le-doom.eps tp.eps te-doom.eps
	gpic tkwadcad.me 2>pic.err | gtbl 2>tbl.err | \
	groff -Tps -me >tkwadcad.ps 2>roff.err

clean:
	rm -f pic.err tbl.err roff.err

allclean:
	make clean
	rm -f tkwadcad.ps
