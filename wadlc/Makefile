INC = ../include
LIB = ../lib

CCFLAGS = $(CFLAGS) -I$(INC)
LFLAGS = -s -L$(LIB)

wadlc: wadlc.o
	$(CC) $(LFLAGS) -o $@ wadlc.o -lwad

wadlc.o: wadlc.c wadlc.h $(LIB)/libwad.a $(INC)/sysdep.h $(INC)/wad.h

$(LIB)/libwad.a:
	cd ../lib; make

clean:
	rm -f *.o wadlc wadlc.exe

allclean:
	make clean; cd ../lib; make clean

.c.o:
	$(CC) $(CCFLAGS) -c $< -o $@
