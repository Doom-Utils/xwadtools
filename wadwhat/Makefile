INC = ../include

CCFLAGS = $(CFLAGS) -I$(INC)
LFLAGS = -s

wadwhat: wadwhat.o
	$(CC) $(LFLAGS) -o $@ wadwhat.o

wadwhat.o: wadwhat.c

clean:
	rm -f *.o wadwhat wadwhat.exe

allclean:
	make clean

.c.o:
	$(CC) $(CCFLAGS) -c $< -o $@
