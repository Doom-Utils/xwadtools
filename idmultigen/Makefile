INC = ../include
LIB = ../lib

CCFLAGS = $(CFLAGS) -I$(INC)
LFLAGS = -s -L$(LIB)

idmultigen: idmultigen.o
	$(CC) $(LFLAGS) -o $@ idmultigen.o -lwad

idmultigen.o: idmultigen.c $(INC)/strfunc.h $(INC)/cmdlib.h $(INC)/scriplib.h \
	$(LIB)/libwad.a

$(LIB)/libwad.a:
	cd ../lib; make

clean:
	rm -f *.o idmultigen idmultigen.exe info.c info.h

allclean:
	make clean; cd ../lib; make clean

.c.o:
	$(CC) $(CCFLAGS) -c $< -o $@
