INC = ../include

CCFLAGS = $(CFLAGS) -I$(INC) -DLIBDIR=\"$(LIBDIR)\"
LFLAGS = -s
LIBS = -lm

OBJS = acc.o error.o misc.o parse.o pcode.o strlist.o symbol.o token.o

acc: $(OBJS)
	$(CC) $(LFLAGS) -o $@ $(OBJS) $(LIBS)

acc.o:		acc.c common.h token.h error.h symbol.h misc.h pcode.h \
		parse.h strlist.h
error.o:	error.c common.h error.h token.h misc.h
parse.o:	parse.c common.h parse.h symbol.h pcode.h token.h error.h \
		misc.h strlist.h
pcode.o:	pcode.c pcode.h common.h error.h misc.h strlist.h
strlist.o:	strlist.c common.h strlist.h error.h misc.h pcode.h
symbol.o:	symbol.c common.h symbol.h misc.h
token.o:	token.c common.h token.h error.h misc.h symbol.h

clean:
	rm -f *.o acc acc.exe

allclean:
	make clean

.c.o:
	$(CC) $(CCFLAGS) -c $< -o $@
