#
#  Makefile for Unix (x)ew.  
#  
#  Common Unix defines.  You will probably have DOT_INI defined.
#
#  ok: this makefile will get some additional targets:
#       'ncurses' if ncurses should be used
#       'curses' if curses should be used
#       'xforms' if the xforms library should be used
#      includes are in CCOPT, libs are in LIBOPT

COMMON_DEF = -DUNIX -DDOT_INI 

#COMMON_DEF += -DAUDIO_DEVICE=\"/dev/dsp\"


#  To compile the X version, use these.  You need the XForms
#  library (v. 0.81, see http://bloch.phys.uwm.edu/pub/forms/).
#  If you have a sound device, set -DAUDIO_DEVICE=\"/dev/dsp\",
#  or whatever your devicename is.  If not, don't define it.

XF_VERSION_DEF = -DWITH_XFORMS
XF_LIBS = $(LIBOPT) -lforms -lwad -lm


#  setup curses and ncurses

CS_VERSION_DEF = -DCURSE
CS_LIBS = -lcurses

NC_VERSION_DEF = -DNCURSE
NC_LIBS = -lncurses


XF_CCFLAGS = $(CFLAGS) $(COMMON_DEF) $(XF_VERSION_DEF) $(CCOPT)
XF_LDFLAGS = -s -L../lib $(XF_LIBS)

CS_CCFLAGS = $(CFLAGS) $(COMMON_DEF) $(CS_VERSION_DEF) $(CCOPT)
CS_LDFLAGS = -s -L../lib $(CS_LIBS)

NC_CCFLAGS = $(CFLAGS) $(COMMON_DEF) $(NC_VERSION_DEF) $(CCOPT)
NC_LDFLAGS = -s $(NC_LIBS) -L../lib

# object files
X_OBS = ew.o xew_m.o xew_f.o makexpm.o fixpal.o

xforms:	 xew

curses:  cs_ew

ncurses: nc_ew

xew:	$(X_OBS)
	$(CC) -o xew $(X_OBS) $(XF_LDFLAGS)

cs_ew: cs_ew.o
	$(CC) -o ew cs_ew.o $(CS_LDFLAGS)

nc_ew: nc_ew.o
	$(CC) -o ew nc_ew.o $(NC_LDFLAGS)

cs_ew.o: ew.c
	$(CC) -c $(CS_CCFLAGS) ew.c -o $@

nc_ew.o: ew.c
	$(CC) -c $(NC_CCFLAGS) ew.c -o $@
        
.c.o:
	$(CC) $(XF_CCFLAGS) -c $< -o $@

clean:
	rm -f ew xew *.o

allclean:
	make clean; cd ../lib; make clean
