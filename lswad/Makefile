INC = ../include
LIB = ../lib

CCFLAGS = $(CFLAGS) -I$(INC)
LFLAGS = -s -L$(LIB)

lswad: lswad.o
	$(CC) $(LFLAGS) -o $@ lswad.o -lwad

lswad.o: lswad.c $(INC)/sysdep.h $(INC)/wad.h $(LIB)/libwad.a


$(LIB)/libwad.a:
	cd ../lib; make

clean:
	rm -f *.o lswad lswad.exe

allclean:
	make clean; cd ../lib; make clean

.c.o:
	$(CC) $(CCFLAGS) -c $< -o $@
