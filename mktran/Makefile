INC = ../include
LIB = ../lib

CCFLAGS = $(CFLAGS) -I$(INC)
LFLAGS = -s -L$(LIB)

mktran: mktran.o
	$(CC) $(LFLAGS) -o $@ mktran.o -lwad

mktran.o: mktran.c $(INC)/sysdep.h $(INC)/wad.h $(INC)/lump_dir.h \
	$(INC)/wadfile.h $(LIB)/libwad.a

$(LIB)/libwad.a:
	cd ../lib; make

clean:
	rm -f *.o mktran mktran.exe

allclean:
	make clean; cd ../lib; make clean

.c.o:
	$(CC) $(CCFLAGS) -c $< -o $@
