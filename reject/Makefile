INC = ../include
LIB = ../lib

CCFLAGS = $(CPPFLAGS) -I$(INC)
LFLAGS =

OBJ=memhandl.o xstring.o list.o ilist.o sector.o main.o reject.o \
    pwad.o fileio.o debug.o version.o

LOBJ=$(LIB)/strfunc.o $(LIB)/sysdep.o

wreject: $(OBJ) $(LIB)/libwad.a
	$(CPP) -o $@ $(LFLAGS) $(OBJ) $(LOBJ) $(CPPLIB)

$(LIB)/libwad.a:
	cd ../lib; make

clean:
	rm -f *.o wreject wreject.exe

allclean:
	make clean; cd ../lib; make clean

.C.o:
	$(CPP) $(CCFLAGS) -c $< -o $@
