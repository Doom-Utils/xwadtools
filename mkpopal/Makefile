LIB = ../lib
INC = ../include

CCFLAGS = $(CFLAGS) -I$(INC)
LFLAGS = -s -L$(LIB)

mkpopal: mkpopal.o
	$(CC) $(LFLAGS) -o $@ mkpopal.o -lwad

mkpopal.o: mkpopal.c

$(LIB)/libwad.a:
	cd ../lib; make

clean:
	rm -f *.o mkpopal

allclean:
	make clean; cd ../lib; make clean

.c.o:
	$(CC) $(CCFLAGS) -c $< -o $@
