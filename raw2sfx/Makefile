INC = ../include
LIB = ../lib

CCFLAGS = $(CFLAGS) -I$(INC)
LFLAGS = -s -L$(LIB)

raw2sfx: raw2sfx.o
	$(CC) $(LFLAGS) -o $@ raw2sfx.o -lwad

raw2sfx.o: raw2sfx.c $(LIB)/libwad.a

$(LIB)/libwad.a:
	cd ../lib; make

clean:
	rm -f *.o raw2sfx raw2sfx.exe

allclean:
	make clean; cd ../lib; make clean

.c.o:
	$(CC) $(CCFLAGS) -c $< -o $@
