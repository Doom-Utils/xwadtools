INC = ../include
LIB = ../lib

CCFLAGS = $(CFLAGS) -I$(INC)
LFLAGS = -s -L$(LIB)

wadgc: wadgc.o
	$(CC) $(LFLAGS) -o $@ wadgc.o -lwad

wadgc.o: wadgc.c $(LIB)/libwad.a $(INC)/sysdep.h

$(LIB)/libwad.a:
	cd ../lib; make

clean:
	rm -f *.o wadgc wadgc.exe

allclean:
	make clean; cd ../lib; make clean

.c.o:
	$(CC) $(CCFLAGS) -c $< -o $@
