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
 * Include file for system dependend functions
 */

#ifndef __SYSDEP_H__
#define __SYSDEP_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSDOS
#include <io.h>
#endif

/*
 * ------------------------------------------------------------------------
 * Byte order
 * ------------------------------------------------------------------------
 */
#ifdef BIGEND

/* macros usable for assigments */
#define bswapw(v)	((unsigned short)((((v)>>8)&0xFF) | ((v)<<8)))
#define bswapl(v)	((unsigned int)((((v)>>24)&0x000000FFL) | \
			                 (((v)>>8)&0x0000FF00L) | \
			                 (((v)<<8)&0x00FF0000L) | \
			                 ( (v)<<24)))

/* and functions swapping in place */
void swapint(short int *i);
void swaplong(int *l);

#else

#define bswapw(v)	(v)
#define bswapl(v)	(v)

#define swapint(i)
#define swaplong(l)

#endif

void putshort(short int, FILE *);
void putlong(int, FILE *);
short int getshort(FILE *);
int getlong(FILE *);

/*
 * ------------------------------------------------------------------------
 * File handling
 * ------------------------------------------------------------------------
 */
#if defined(UNIX) && !defined(__CYGWIN32__)
#define O_BINARY 0
#endif

#if defined(UNIX)
#define PATHSEPERATOR   	'/'
#define PATHSEPERATOR_STR	"/"
#define EXTSEPERATOR		'.'
#define EXTSEPERATOR_STR	"."
#elif defined(RISCOS)
#define PATHSEPERATOR   	'.'
#define PATHSEPERATOR_STR	"."
#define EXTSEPERATOR		'/'
#define EXTSEPERATOR_STR	"/"
#elif defined(MSDOS)
#define PATHSEPERATOR   	'\\'
#define PATHSEPERATOR_STR	"\\"
#define EXTSEPERATOR		'.'
#define EXTSEPERATOR_STR	"."
#else
#error unsupported OS, cannot figure out path seperator
#endif

/* on solaris, the function tell() is available, only need filelength */
#if defined(UNIX)
int filelength (int);
#if !defined(__sun)
int tell (int);
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
