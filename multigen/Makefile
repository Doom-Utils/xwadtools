INC = ../include
LIB = ../lib

CCFLAGS = $(CFLAGS) -I$(INC)
LFLAGS = -s -L$(LIB)

multigen: multigen.o
	$(CC) $(LFLAGS) -o $@ multigen.o -lwad

multigen.o: multigen.c $(INC)/strfunc.h $(INC)/cmdlib.h $(INC)/scriplib.h \
	$(LIB)/libwad.a

$(LIB)/libwad.a:
	cd ../lib; make

clean:
	rm -f *.o multigen multigen.exe info.c info.h

allclean:
	make clean; cd ../lib; make clean

.c.o:
	$(CC) $(CCFLAGS) -c $< -o $@
