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
 *      Tool to extract flats from a WAD file into ppm files
 */

/*
 * 1.1 (AYM 1999-01-02)
 * - The names of the output files are now forced to lower case
 *   (rationale: since case is not significant in wad directories, why
 *   not use the most convenient case)
 * - Fixed bug on STEP1.
 * - Added the "-p" option to inhibit the above feature.
 *
 * 1.2 (UM 1999-02-15)
 * - Added support for Heretic and Hexen.
 *
 * 1.3 (UM 1999-03-25)
 * - Added support for Strife.
 * - Accept FF_START as start label too, to be able to extract flats from
 *   PWADS, where FF_START is used by convention.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#include "sysdep.h"
#include "strfunc.h"
#include "wad.h"
#include "lump_dir.h"
#include "wadfile.h"

#define VERSION "1.3"

extern unsigned char doom_rgb[];
extern unsigned char heretic_rgb[];
extern unsigned char hexen_rgb[];
extern unsigned char strife_rgb[];
unsigned char *palette = doom_rgb;
static int preserve_case = 0;

void usage(char *name, char *option)
{
	if (option)
		fprintf(stderr, "%s: Unkown option: %s\n", name, option);

        fprintf(stderr, "Usage: %s [-p] [-c pal] wadfile\n", name);
	fprintf(stderr, "\t-p: preserve case of generated files\n");
	fprintf(stderr, "\t-c pal: use color palette pal, where pal can be\n");
	fprintf(stderr, "\t        doom, heretic, hexen or strife\n");
        exit(1);
}

void decompile(wadfile_t *wf, char *name)
{
	int		i;
	FILE		*fp;
	char		fn[50];
	unsigned char	*flat;

	printf("  %s\n", name);

	/* get flat lump */
	if ((flat = (unsigned char *)get_lump_by_name(wf, name)) == NULL) {
		fprintf(stderr, "can't get lump %s??\n", name);
		exit(1);
	}

	strcpy(&fn[0], "flats/");
        if (preserve_case)
          strcat(&fn[0], name);
        else
          strlcat(&fn[0], name);
	strcat(&fn[0], ".ppm");
	if ((fp = fopen(&fn[0], "wb")) == NULL) {
		fprintf(stderr, "can't open %s for writing\n", &fn[0]);
		exit(1);
	}

	fprintf(fp, "P6\n");
	fprintf(fp, "# CREATOR: wadflat Release %s\n", VERSION);
	fprintf(fp, "64 64\n");
	fprintf(fp, "255\n");

	for (i = 0; i < 4096; i++) {
		fputc((int) palette[*(flat+i) * 3],     fp);
		fputc((int) palette[*(flat+i) * 3 + 1], fp);
		fputc((int) palette[*(flat+i) * 3 + 2], fp);
	}

	fclose(fp);
	free(flat);
}

int main(int argc, char **argv)
{
	int		i;
	int		start_flag = 0;
	wadfile_t	*wf;
	char		*s;
	char		*program;
	char		name[9];

	/* initialize */
	memset(&name[0], 0, 9);

        /* save program name for usage() */
        program = *argv;

	printf("%s version %s\n\n", program, VERSION);

	/* process options */
	while ((--argc > 0) && (**++argv == '-')) {
	  for (s = *argv+1; *s != '\0'; s++) {
		switch (*s) {
		case 'p':
			preserve_case++;
			break;
		case 'c':
			argc--;
			argv++;
			if (!strcmp(*argv, "doom"))
				palette = doom_rgb;
			else if (!strcmp(*argv, "heretic"))
				palette = heretic_rgb;
			else if (!strcmp(*argv, "hexen"))
				palette = hexen_rgb;
			else if (!strcmp(*argv, "strife"))
				palette = strife_rgb;
			else
				usage(program, NULL);
			break;
		default:
			usage(program, --s);
		}
	  }
	}

	/* have one argument left? */
	if (argc != 1)
		usage(program, NULL);

	/* open WAD file */
	wf = open_wad(*argv);

	/* just make flats directory, ignore errors, deal with it later */
	mkdir("flats", 0755);

	/* loop over all lumps and look for the flats */
	for (i = 0; i < wf->wh.numlumps; i++) {
		/* start processing after F_START */
		if (!strncmp(wf->lp->lumps[i]->name, "F_START", 7)) {
			if (start_flag)
				continue;
			start_flag = 1;
			printf("found F_START, start decompiling flats\n");
			continue;
		/* also accept FF_START */
		} else if (!strncmp(wf->lp->lumps[i]->name, "FF_START", 8)) {
			if (start_flag)
				continue;
			start_flag = 1;
			printf("found FF_START, start decompiling flats\n");
			continue;
		} else if (!strncmp(wf->lp->lumps[i]->name, "F_END", 5)) {
			start_flag = 0;
			printf("found F_END, done\n");
			break;
		} else {
			if (start_flag && (wf->lp->lumps[i]->size == 4096)) {
			  strncpy(&name[0], wf->lp->lumps[i]->name, 8);
			  decompile(wf, &name[0]);
			}
		}
	}

	/* done */
	close_wad(wf);
	return 0;
}
