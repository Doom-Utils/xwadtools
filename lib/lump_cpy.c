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
 *	Library functions to copy lump data to/from WAD files
 */

#ifdef RISCOS

#include <stdio.h>
#include <stdlib.h>
#include "unixlike.h"

#else

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#endif

#include "sysdep.h"
#include "lump_cpy.h"

#define BUFSIZE		1024*5	/* size of buffer for lump data */

static char buf[BUFSIZE];	/* buffer for lump data */

/*
 * Copy raw lump data from a file into a WAD file and return the number of
 * bytes copied
 */
int copy_from_lump_file(FILE *wad, char *lname)
{
	int	i, num, fd;

	/* try to open the lump file for reading */
	if ((fd = open(lname, O_RDONLY | O_BINARY)) == -1) {
		fprintf(stderr, "copy_from_lump_file(): can't open lump file %s\n", lname);
		exit(1);
	}

	num = 0;

	/* read lump data and write into WAD file */
	while ((i = read(fd, buf, BUFSIZE))!=0) {
		if (fwrite((void *)buf, i, 1, wad) == 0) {
			perror("copy_from_lump_file(): can't write lump data");
			exit(1);
		}
		num += i;
	}

	/* close lump file */
	close(fd);

	return num;
}
