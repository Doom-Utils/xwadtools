LIB = ../lib
INC = ../include

CCFLAGS = $(CFLAGS) -I$(INC)
LFLAGS = -s -L$(LIB)

mkqmap: mkqmap.o
	$(CC) $(LFLAGS) -o $@ mkqmap.o -lwad

mkqmap.o: mkqmap.c

$(LIB)/libwad.a:
	cd ../lib; make

clean:
	rm -f *.o mkqmap

allclean:
	make clean; cd ../lib; make clean

.c.o:
	$(CC) $(CCFLAGS) -c $< -o $@
