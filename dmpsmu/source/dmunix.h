/*
   DooM PostScript Maps Utility, by Frans P. de Vries.

   DMUNIX.H - Some UNIX specific things.

Derived from:

   Doom Editor Utility, by Brendon Wyber and Raphaël Quinet.

   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README for more information.

   This program comes with absolutely no warranty.

   DEU-GO32.H - Some DJGPP/GO32 specific things.

   Originally written by Per Allansson (c91peral@und.ida.liu.se)
*/

/*
 * UM 1999-06-08
 *  - Define BIG_ENDIAN nowadays doesn't work in every environment anymore,
 *    modified to BIGEND as used consistently in xwadtools.
 *
 * UM 1999-06-28
 *  - removed external declaration for strupr(), is decraled in the library
 *    string functions include.
 *  - also removed declarations for endian support, use those from library.
 *
 */


#ifndef DM_UNIX_H
#define DM_UNIX_H

#define huge	/* No huge things in Unix, either */

/* from dmpsmu.c/dmpsmap.c */
extern char *strupr( char *);

/* global (re)definitions */
#ifndef max
#define max(x,y)	(((x)>(y))?(x):(y))
#define min(x,y)	(((x)<(y))?(x):(y))
#endif

#define exch(a,b)	{ BCINT t=(a); (a)=(b); (b)=t; }

#define GetMemory(x)		malloc(x)
#define FreeMemory(x)		free(x)
#define ResizeMemory(x,y)	realloc(x,y)
#define GetFarMemory(x)		malloc(x)
#define FreeFarMemory(x)	free(x)
#define ResizeFarMemory(x,y)	realloc(x,y)

#endif /* DM_UNIX_H */
