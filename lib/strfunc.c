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
 * This module should handle string functions not defined by the ANSI
 * standard for the various OS's. Better handle this in one place than
 * litter all sources with the stuff...
 */

#include <ctype.h>
#include "strfunc.h"

/* some environments have those already */
#if !defined(linux) && !defined(__CYGWIN32__)
int strcasecmp (const char *a, const char *b)
{
    const char *p;
    const char *q;

    for (p = a, q = b; *p && *q; p++, q++)
    {
      int diff = tolower(*p) - tolower(*q);
      if (diff) return diff;
    }
    if (*p) return 1;       /* p was longer than q */
    if (*q) return -1;      /* p was shorter than q */
    return 0;               /* Exact match */
}

int strncasecmp (const char *a, const char *b, int n)
{
    const char *p;
    const char *q;

    for (p = a, q = b; /*NOTHING*/; p++, q++)
    {
      int diff;
      if (p == a + n) return 0;     /*   Match up to n characters */
      if (!(*p && *q)) return *p - *q;
      diff = tolower(*p) - tolower(*q);
      if (diff) return diff;
    }
    /*NOTREACHED*/
}
#endif

char *strupr (char *start)
{
	char    *in;
	in = start;
	while (*in)
	{
		*in = toupper(*in);
		in++;
	}
	return start;
}

char *strlwr (char *start)
{
	char	*in;

	in = start;
	while (*in)
	{
		*in = toupper(*in);
		in++;
	}
	return start;
}

void strlcat (char *to, const char *from)
{
	for (to += strlen(to); *from; to++, from++)
		*to = tolower(*from);
	*to = 0;
}

#if defined(RISCOS)
/* Simple string duplicator */
char *strdup (char *x)
{
	int l = strlen(x);
	char *p = malloc(l+1);

	if (p==NULL)
		return NULL;
	return strcpy(p,x);
}
#endif
