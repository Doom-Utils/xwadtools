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
 * Include file for OS dependend string handling functions
 */

#ifndef __STRFUNC__
#define __STRFUNC__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(UNIX)
#define strcmpi strcasecmp
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

#if defined(RISCOS)
#define strcmpi strcasecmp
#define stricmp strcasecmp
char *strdup (char *);
#endif

#if defined(MSDOS)
#define strcmpi stricmp
#endif

/* some environments have those already */
#if !defined(linux) && !defined(__CYGWIN32__) && !defined(MSDOS) && !defined(__sun)
int strcasecmp (const char *, const char *);
int strncasecmp (const char *, const char *, int);
#endif

/* and those probably only exist under DOS */
#if !defined(MSDOS)
char *strupr (char *);
char *strlwr (char *);
#endif

/* those are for all OS's probably */
void strlcat (char *, const char *);

#ifdef __cplusplus
}
#endif

#endif
