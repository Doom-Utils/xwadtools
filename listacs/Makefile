INC = ../include
LIB = ../lib

CCFLAGS = $(CPPFLAGS) -I$(INC)
LFLAGS = -s -L$(LIB)

OBJ = listacs.o

LOBJ=$(LIB)/strfunc.o

listacs: $(OBJ)
	$(CPP) -o $@ $(LFLAGS) $(OBJ) $(LOBJ) $(CPPLIB)

$(LIB)/libwad.a:
	cd ../lib; make

clean:
	rm -f *.o listacs listacs.exe

allclean:
	make clean; cd ../lib; make clean

.C.o:
	$(CPP) $(CCFLAGS) -c $< -o $@
