/************************************************************************/
/*      Copyright (C) 1998, 1999 by Udo Munk (munkudo@aol.com)          */
/*	Copyright (C) 1998 by TeamTNT (www.teamtnt.com)                 */
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
 *      Tool to build translucency colormap from WAD colormap
 *
 * The algorithm to compute the translucency table was taken from the
 * Boom phase I source release from 5/27/98. Boom also is a modified
 * Doom engine based on id Software's public source release. Boom is
 * copyrighted by TeamTNT and the id Software license (GPL nowadays).
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>

#include "sysdep.h"
#include "wad.h"
#include "lump_dir.h"
#include "wadfile.h"

#define VERSION "1.2"

#define TSC 12		    /* number of fixed point digits in filter percent */
int tran_filter_pct = 50;   /* default filter percent */
float brightness = 1.0;	    /* default brightness */

void usage(char *name, char *option)
{
        if (option)
            fprintf(stderr, "%s: Unknown option: %s\n", name, option);
        else
            fprintf(stderr, "%s: Missing arguments\n", name);
        fprintf(stderr, "Usage: %s [-f num] [-v] wadfile tranmapfile\n", name);
	fprintf(stderr, "\t-f num: filter percent (0 - 100)\n");
	fprintf(stderr, "\t-b num: brightness (1.0 - 1.5\n");
	fprintf(stderr, "\t-v: verbose output\n");
        exit(1);
}

int main(int argc, char **argv)
{
	wadfile_t	*wf;
	char		*program;
	unsigned char	*playpal;
	char		*s;
	int		verbose = 0;
	int		fd;
	unsigned char	*tranmap;
	long		pal[3][256], tot[256], pal_w1[3][256];
	long		w1 = ((unsigned long) tran_filter_pct << TSC)/100;
	long		w2 = (1L << TSC) - w1;

	/* save program name for usage() */
	program = *argv;

	/* options */
        while ((--argc > 0) && (**++argv == '-')) {
                for (s = *argv+1; *s != '\0'; s++) {
                        switch (*s) {
			case 'b':
				brightness = atof(*++argv);
				argc--;
				break;
			case 'f':
				tran_filter_pct = atoi(*++argv);
				argc--;
				break;
                        case 'v':
                                verbose++;
                                break;
                        default:
                                usage(program, --s);
                        }
                }
        }

	if (verbose)
                printf("%s version %s\n\n", program, VERSION);

	/* we need 2 args, WAD file and a filename for the trans table */
	if (argc != 2)
		usage(program, NULL);

	if (verbose)
		printf("Opening WAD file %s...\n", *argv);

	/* open WAD file and get PLAYPAL lump with the color tables */
	wf = open_wad(*argv);
	if (verbose)
		printf("Reading color tables\n");
	if ((playpal = (unsigned char *)get_lump_by_name(wf, "PLAYPAL")) == NULL) {
		fprintf(stderr, "%s: can't find PLAYPAL lump in %s\n",
			program, *argv);
		exit(1);
	}
	close_wad(wf);

	if (verbose)
	  printf("Calculating translucency table with %d%% filter and %2.2f brigthness...\n",
			tran_filter_pct, brightness);

	/* now build the tranlucency table, first allocate memory */
	if ((tranmap = (unsigned char *)malloc(256 * 256)) == NULL) {
		fprintf(stderr, "%s: can't allocate memory for tranmap\n", program);
		exit(1);
	}

	/*
	 * First convert PLAYPAL into long int type and transpose array,
	 * for fast inner-loop calculations. Precompile tot array.
	 */
	{
	  int i = 255;
	  const unsigned char *p = playpal + 255 * 3;

	  do {
	    long t, d;

	    pal_w1[0][i] = (pal[0][i] = t = p[0]) * w1;
	    d = t * t;
	    pal_w1[1][i] = (pal[1][i] = t = p[1]) * w1;
	    d += t * t;
	    pal_w1[2][i] = (pal[2][i] = t = p[2]) * w1;
	    d += t * t;
	    p -= 3;
	    tot[i] = d << (TSC-1);
	  } while (--i >= 0);
	}

	/* next compute all entries using minimum arithmetic */
	{
	  int i, j;
	  unsigned char *tp = tranmap;

	  for (i = 0; i < 256; i++) {
	    long r1 = pal[0][i] * w2;
	    long g1 = pal[1][i] * w2;
	    long b1 = pal[2][i] * w2;

	    for (j = 0; j < 256; j++, tp++) {
	      int color = 255;
	      long err;
	      long r = (pal_w1[0][j] + r1) * brightness;
	      long g = (pal_w1[1][j] + g1) * brightness;
	      long b = (pal_w1[2][j] + b1) * brightness;
	      long best = LONG_MAX;

	      do {
		if ((err = tot[color] - pal[0][color] * r
			- pal[1][color] * g - pal[2][color] * b) < best)
			best = err, *tp = color;
	      } while (--color >= 0);
	    }
	  }
	}

	if (verbose)
		printf("Writing translucency table into file %s...\n", *(argv+1));

	if ((fd = open(*(argv+1), O_WRONLY|O_CREAT|O_BINARY, 0644)) == -1) {
		perror("can't open data file for writing");
		exit(1);
	}

	if (write(fd, tranmap, 256 * 256) == -1) {
		perror("can't write translucency data into file");
		exit(1);
	}

	close(fd);
	free(playpal);
	free(tranmap);

	return 0;
}
