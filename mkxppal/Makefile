INC = ../include
LIB = ../lib

CCFLAGS = $(CFLAGS) -I$(INC)
LFLAGS = -s -L$(LIB)

mkxppal: mkxppal.o
	$(CC) $(LFLAGS) -o $@ mkxppal.o -lwad

mkxppal.o: mkxppal.c $(LIB)/libwad.a

$(LIB)/libwad.a:
	cd ../lib; make

clean:
	rm -f *.o mkxppal

allclean:
	make clean; cd ../lib; make clean

.c.o:
	$(CC) $(CCFLAGS) -c $< -o $@
