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
 *      Tool to convert a palette from a WAD file into a C array
 */

#include <stdio.h>
#include <stdlib.h>

#include "wad.h"
#include "lump_dir.h"
#include "wadfile.h"

void usage(char *name)
{
	fprintf(stderr, "Usage: %s wadfile num prefix\n", name);
	fprintf(stderr, "\twadfile: IWAD with the PLAYPAL resource\n");
	fprintf(stderr, "\tnum: number of the colormap entry\n");
	fprintf(stderr, "\tprefix: prefix used for the C array\n");
	exit(1);
}

int main(int argc, char **argv)
{
	wadfile_t	*wf;
	unsigned char	*palette;
	unsigned char	*p;
	int		num;
	char		*prefix;
	char		*program;
	int		i;

	/* save program name for usage() */
	program = argv[0];

	/* arguments */
	if (argc != 4)
		usage(program);
	num = atoi(argv[2]);
	prefix = argv[3];

	/* try to open wadfile and get PLAYPAL */
	wf = open_wad(argv[1]);
	if ((palette = (unsigned char *)get_lump_by_name(wf, "PLAYPAL")) == NULL) {
		fprintf(stderr, "%s: can't find PLAYPAL lump in %s\n",
			program, argv[1]);
		exit(1);
	}
	close_wad(wf);

	/* set pointer to the wanted palette */
	p = palette + num * 3;

	/* print prefix */
	printf("const unsigned char %s_rgb[3 * 256] = {\n", prefix);

	/* loop through the palette and print its values */
	for (i = 0; i < 256 * 3; i++) {
	    printf(" '\\x%02X',", *p++);
	    if (!((i + 1) % 9))
		printf("\n");
	}
	printf(" };\n");

	exit(0);
}
