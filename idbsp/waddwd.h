/*
   WAD_DWD.H
 */

#ifndef __WAD_DWD__
#define __WAD_DWD__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>
#include <string.h>
#include "doomdata.h"

typedef struct {
	char	identification[4];	/* IWAD or PWAD */
	long	numlumps;
	long	infotableofs;
} wadinfo_t;

typedef struct {
	long	filepos;
	long	size;
	char	name[8];
} lumpinfo_t;

void Error(char *, ...);
void *SafeCalloc(unsigned, size_t);
void *ReadWAD(FILE *, long, size_t);
void *ReadStorage(FILE *, lumpinfo_t *, long *, size_t);
void WriteHeader(FILE *);
void WriteLines(FILE *, long, maplinedef_t *, mapvertex_t *, mapsidedef_t *,
		mapsector_t *);
void WriteThings(FILE *, long, mapthing_t *);
void WriteSideAndSector(FILE *, mapsidedef_t *, mapsector_t *);
void ExtractResource(FILE *, lumpinfo_t *);

#endif
