# for SunOS use:
# INCLUDES = /usr/users/sigc/a01/bin/SC1.0/ansi_include
# for Irix, Linux, Ultrix, HP-UX use:
# (none)
INCLUDES = ../../include

LIB = ../../lib

CFLAGS	= $(CFLAGS) -I$(INCLUDES)
LDFLAGS	= -s -L$(LIB)

# Programs & Objects
PROGRAMS= dmpsmu dmpsmap
OBJS	= wads.o things.o levels.o print.o ps.o

# Rules & Dependencies
all::	$(PROGRAMS)

dmpsmu.o:  dmps.h dmunix.h
dmpsmap.o: dmps.h dmunix.h print.h
wads.o:    dmps.h dmunix.h
levels.o:  dmps.h dmunix.h wstructs.h
things.o:  dmps.h dmunix.h things.h
print.o:   dmps.h dmunix.h things.h levels.h wstructs.h
ps.o:      dmps.h dmunix.h things.h print.h


# Commands
dmpsmu: dmpsmu.o $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $@.o -lwad

dmpsmap: dmpsmap.o $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $@.o -lwad

$(LIB)/libwad.a:
	cd ../lib; make

.c.o:
	$(CC) $(CFLAGS) $(DEFINES) -I$(INCLUDES) -c $<
# for HP-UX use:
#	$(CC) $(CFLAGS) $(DEFINES) -I$(INCLUDES) -Aa -c $<


# Remove Objects
clean:
	rm -f *.o dmpsmu dmpsmu.exe dmpsmap dmpsmap.exe

allclean:
	make clean; cd ../lib; make clean
