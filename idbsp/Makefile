INC = ../include
LIB = ../lib

CCFLAGS = $(CFLAGS) -I$(INC)
LFLAGS = -s
LIBS = -L../lib -lwad -lm

all:	idbsp waddwd

IDBSPO=	idbsp.o buildbsp.o doomload.o drawing.o \
	saveblck.o savebsp.o savecnct.o savesctr.o wadfile.o

idbsp: $(IDBSPO) $(LIB)/libwad.a
	$(CC) $(LFLAGS) -o $@ $(IDBSPO) $(LIBS)

DWDO=	waddwd.o funcs.o

waddwd: $(DWDO) $(LIB)/libwad.a
	$(CC) $(LFLAGS) -o $@ $(DWDO) $(LIBS)

$(LIB)/libwad.a:
	cd ../lib; make

idbsp.o:	idbsp.c cmdlib.h doomdata.h idbsp.h wadfile.h
buildbsp.o:	buildbsp.c cmdlib.h doomdata.h idbsp.h wadfile.h
doomload.o:	doomload.c cmdlib.h doomdata.h idbsp.h wadfile.h
drawing.o:	drawing.c cmdlib.h doomdata.h idbsp.h wadfile.h
saveblck.o:	saveblck.c cmdlib.h doomdata.h idbsp.h wadfile.h
savebsp.o:	savebsp.c cmdlib.h doomdata.h idbsp.h wadfile.h
savecnct.o:	savecnct.c cmdlib.h doomdata.h idbsp.h wadfile.h
savesctr.o:	savesctr.c cmdlib.h doomdata.h idbsp.h wadfile.h
wadfile.o:	wadfile.c cmdlib.h doomdata.h idbsp.h wadfile.h
waddwd.o:	waddwd.c waddwd.h
funcs.o:	funcs.c cmdlib.h doomdata.h idbsp.h wadfile.h

clean:
	rm -f *.o idbsp waddwd idbsp.exe waddwd.exe

allclean:
	make clean; cd ../lib; make clean

.c.o:
	$(CC) $(CCFLAGS) -c $< -o $@
