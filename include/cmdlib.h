/*
 *	This library functions were written by id Software.
 *
 *	Copyright (C) 1994 by id Software
 *
 *	I have modified this some to get it integrated into the tool
 *	collection. Please don't bother anyone at id Software about
 *	this, if you have a problem you could try to email me.
 *
 *	24 June 1998, Udo Munk	munkudo@aol.com
 */

#ifndef __CMDLIB_H__
#define __CMDLIB_H__

#if defined(UNIX)

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#elif defined(RISCOS)

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#else

#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <io.h>
#include <direct.h>
#include <process.h>
#include <dos.h>
#include <stdarg.h>
#include <conio.h>
#include <bios.h>

#endif

#ifndef __BYTEBOOL__
#define __BYTEBOOL__
typedef enum {false, true} boolean;
typedef unsigned char byte;
#endif

int	GetKey (void);

void	Error (char *error, ...);
int	CheckParm (char *check);

int 	SafeOpenWrite (char *filename);
int 	SafeOpenRead (char *filename);
void 	SafeRead (int handle, void *buffer, long count);
void 	SafeWrite (int handle, void *buffer, long count);
void 	*SafeMalloc (long size);
void    *SaveCalloc (unsigned, unsigned);

long	LoadFile (char *filename, void **bufferptr);
void	SaveFile (char *filename, void *buffer, long count);

void 	DefaultExtension (char *path, char *extension);
void 	DefaultPath (char *path, char *basepath);
void 	StripFilename (char *path);
void 	StripExtension (char *path);
void 	ExtractFileBase (char *path, char *dest);

long 	ParseNum (char *str);

#endif
