CCFLAGS = $(CFLAGS)
LFLAGS = -s

qmus2mid: qmus2mid.o
	$(CC) $(LFLAGS) -o $@ qmus2mid.o

qmus2mid.o: qmus2mid.c qmus2mid.h

clean:
	rm -f *.o qmus2mid qmus2mid.exe

allclean:
	make clean

.c.o:
	$(CC) $(CCFLAGS) -c $< -o $@
