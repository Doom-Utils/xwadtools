LIB = ../lib
INC = ../include

CCFLAGS = $(CFLAGS) -I$(INC)
LFLAGS = -s -L$(LIB)

mkgipal: mkgipal.o
	$(CC) $(LFLAGS) -o $@ mkgipal.o -lwad

mkgipal.o: mkgipal.c

$(LIB)/libwad.a:
	cd ../lib; make

clean:
	rm -f *.o mkgipal

allclean:
	make clean; cd ../lib; make clean

.c.o:
	$(CC) $(CCFLAGS) -c $< -o $@
