/************************************************************************/
/*      Copyright (C) 1999 by Udo Munk (munkudo@aol.com)                */
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
 *   Tool to convert raw sound data into id's sfx format
 */

#include <stdio.h>
#include "sysdep.h"

void usage(char *name)
{
	fprintf(stderr, "Usage: %s infile outfile\n", name);
	exit(1);
}

int main(int argc, char **argv)
{
	FILE		*ifp, *ofp;
	int		flen;
	unsigned short	i;
	unsigned char	buf[65535];

	if (argc != 3)
		usage(*argv);

	if ((ifp = fopen(*(argv+1), "rb")) == NULL) {
		fprintf(stderr, "can't open input file for reading\n");
		exit(1);
	}

	flen = filelength(fileno(ifp));
	if (flen > 65535) {
		fprintf(stderr, "input file has more than 65535 samples\n");
		exit(1);
	}

	if ((ofp = fopen(*(argv+2), "wb")) == NULL) {
		fprintf(stderr, "can't open output file for writing\n");
		exit(1);
	}

	/* magic number for id's sfx format probably */
	i = 3;
	putshort(i, ofp);
	/* 11kHz sample rate */
	i = 11025;
	putshort(i, ofp);
	/* number of samples */
	i = (unsigned short) flen;
	putshort(i, ofp);
	/* separator, whatever */
	i = 0;
	putshort(i, ofp);
	
	fread(&buf[0], flen, 1, ifp);
	fwrite(&buf[0], flen, 1, ofp);

	fclose(ifp);
	fclose(ofp);
	return 0;
}
