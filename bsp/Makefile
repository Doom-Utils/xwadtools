CCFLAGS = $(CFLAGS)
LFLAGS = -s
LIBS = -lm

bsp: bsp.o
	$(CC) $(LFLAGS) -o $@ bsp.o $(LIBS)

bsp.o: bsp.c bsp.h funcs.c makenode.c structs.h

clean:
	rm -f *.o bsp bsp.exe

allclean:
	make clean

.c.o:
	$(CC) $(CCFLAGS) -c $< -o $@
