/************************************************************************/
/*      Copyright (C) 1998, 1999 by Udo Munk (munkudo@aol.com)          */
/*                                                                      */
/*      Permission to use, copy, modify, and distribute this software   */
/*      and its documentation for any purpose and without fee is        */
/*      hereby granted, provided that the above copyright notice        */
/*      appears in all copies and that both that copyright notice and   */
/*      this permission notice appear in supporting documentation.      */
/*      The author and contibutors make no representations about the    */
/*      suitability of this software for any purpose. It is provided    */
/*      "as is" without express or implied warranty.                    */
/************************************************************************/

/*
 *	Library functions to deal with system dependencies
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
 * ------------------------------------------------------------------------
 * handling for different byte orders
 * ------------------------------------------------------------------------
 */
#ifdef BIGEND

/* write short integer into a file in little endian order */
void putshort(short int d, FILE *filep)
{
  putc(d & 0xff, filep);
  putc((d & 0xff00) >> 8, filep);  
}

/* write integer into a file in little endian order */
void putlong(int d, FILE *filep)
{
  putc(d & 0xff, filep);
  putc((d & 0xff00) >> 8, filep);
  putc((d & 0xff0000) >> 16, filep);
  putc((d & 0xff000000) >> 24, filep);
}

/* get short integer from a file in little endian order */
short int getshort(FILE *filep)
{
  short int s;

  s = getc(filep) & 0xff;
  s += (getc(filep) & 0xff)<<8;
  return(s);
}

/* get integer from a file in little endian order */
int getlong(FILE *filep)
{
  int l;
  l = getc(filep) & 0xff;
  l += (getc(filep) & 0xff) <<8;
  l += (getc(filep) & 0xff) <<16;
  l += (getc(filep) & 0xff) <<24;
  return(l);
}

/* swap short integer from little endian to big endian order */
void swapint(short int *i)
{
  short int t;

  ((char *) &t)[ 0] = ((char *) i)[ 1];
  ((char *) &t)[ 1] = ((char *) i)[ 0];
  *i = t;
}

/* swap long integer from little endian to big endian order */
void swaplong(int *l)
{
  int t;

  ((char *) &t)[ 0] = ((char *) l)[ 3];
  ((char *) &t)[ 1] = ((char *) l)[ 2];
  ((char *) &t)[ 2] = ((char *) l)[ 1];
  ((char *) &t)[ 3] = ((char *) l)[ 0];
  *l = t;
}

#else	/* LITTLE ENDIAN */

/* write short integer into a file in little endian order */
void putshort(short int d, FILE *filep)
{
  fwrite(&d, sizeof(short int), 1, filep);
}

/* write long integer into a file in little endian order */
void putlong(int d, FILE *filep)
{
  fwrite(&d, sizeof(int), 1, filep);
}

/* get short integer from a file in little endian order */
short int getshort(FILE *filep)
{
  short d;

  fread(&d, sizeof(short int), 1, filep);
  return(d);
}

/* get integer from a file in little endian order */
int getlong(FILE *filep)
{
  int d;

  fread(&d, sizeof(int), 1, filep);
  return(d);
}

#endif

/*
 * ------------------------------------------------------------------------
 * file handling
 * ------------------------------------------------------------------------
 */
#if defined(UNIX)
int filelength(int handle)
{
	struct stat fileinfo;
    
	if (fstat(handle, &fileinfo) == -1)
	{
		fprintf(stderr,"Error fstating");
		exit(1);
	}

	return fileinfo.st_size;
}

#if !defined(__sun)
int tell(int handle)
{
	return lseek(handle, 0, 1);
}
#endif

#endif
