CCFLAGS = $(CFLAGS)
LFLAGS = -s

trigcalc: trigcalc.o
	$(CC) $(LFLAGS) -o $@ trigcalc.o

trigcalc.o: trigcalc.c

clean:
	rm -f *.o trigcalc trigcalc.exe

allclean:
	make clean

.c.o:
	$(CC) $(CCFLAGS) -c $< -o $@
