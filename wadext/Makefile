INC = ../include
LIB = ../lib

CCFLAGS = $(CFLAGS) -I$(INC)
LFLAGS = -s -L$(LIB)

wadext: wadext.o
	$(CC) $(LFLAGS) -o $@ wadext.o -lwad

wadext.o: wadext.c $(LIB)/libwad.a $(INC)/sysdep.h

$(LIB)/libwad.a:
	cd ../lib; make

clean:
	rm -f *.o wadext wadext.exe

allclean:
	make clean; cd ../lib; make clean

.c.o:
	$(CC) $(CCFLAGS) -c $< -o $@
