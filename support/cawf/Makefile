# 	Makefile for cawf

#	define UNIX for standard Unix systems
#
#	define STDLIB for systems that have <stdlib.h>

DEFS = -DUNIX -DSTDLIB

CFLAGS = -O ${DEFS}

HDR = cawf.h regexp.h regmagic.h

SRC = cawf.c error.c expand.c expr.c macsup.c output.c pass2.c pass3.c \
      regerror.c regexp.c store.c string.c

OBJ = cawf.o error.o expand.o expr.o macsup.o output.o pass2.o pass3.o \
      regerror.o regexp.o store.o string.o

all:	bsfilt cawf

bsfilt: bsfilt.c
	${CC} ${CFLAGS} bsfilt.c -o bsfilt -lm

cawf:	${OBJ}
	${CC} ${CFLAGS} ${OBJ} -o cawf -lm

clean:
	rm -f *.o a.out core *errs bsfilt bsfilt.exe cawf cawf.exe
${OBJ}:	${HDR}
