INC = ../include
LIB = ../lib

CCFLAGS = $(CFLAGS) -I$(INC)
LFLAGS = -s -L$(LIB)

swantbls: swantbls.o
	$(CC) $(LFLAGS) -o $@ swantbls.o -lwad

swantbls.o: swantbls.c $(INC)/sysdep.h $(LIB)/libwad.a

$(LIB)/libwad.a:
	cd ../lib; make

clean:
	rm -f *.o swantbls swantbls.exe

allclean:
	make clean; cd ../lib; make clean

.c.o:
	$(CC) $(CCFLAGS) -c $< -o $@
