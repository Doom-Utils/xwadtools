INC = ../include

CCFLAGS = $(CFLAGS) -I$(INC)
LFLAGS = -s
LIBS = -lm

OBJ = warm.o nodes.o blockmap.o reject.o wadio.o

warm: $(OBJ)
	$(CC) $(LFLAGS) -o $@ $(OBJ) $(LIBS)

warm.o: warm.c dmglobal.h
nodes.o: nodes.c dmglobal.h
blockmap.o: blockmap.c dmglobal.h
reject.o: reject.c dmglobal.h
wadio.o: wadio.c dmglobal.h

clean:
	rm -f *.o warm warm.exe

allclean:
	make clean

.c.o:
	$(CC) $(CCFLAGS) -c $< -o $@
