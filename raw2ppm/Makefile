INC = ../include
LIB = ../lib

CCFLAGS = $(CFLAGS) -I$(INC)
LFLAGS = -s -L$(LIB)

raw2ppm: raw2ppm.o
	$(CC) $(LFLAGS) -o $@ raw2ppm.o -lwad

raw2ppm.o: raw2ppm.c $(LIB)/libwad.a

$(LIB)/libwad.a:
	cd ../lib; make

clean:
	rm -f *.o raw2ppm raw2ppm.exe

allclean:
	make clean; cd ../lib; make clean

.c.o:
	$(CC) $(CCFLAGS) -c $< -o $@
