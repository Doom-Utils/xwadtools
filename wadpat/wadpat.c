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
 *      Tool to extract texture patches from a WAD file into ppm files
 */

/*
 * 1.2 (UM 1999-02-15)
 * - Added support for Heretic and Hexen.
 *
 * 1.3 (UM 1999-03-25)
 * - Added support for Strife.
 * - The palette was used uninitialized, fixed to use Doom palette by
 *   default. Thanks to Jamie Guinan for the bug report.
 *
 * 1.4 (AYM 1999-06-10)
 * - Added option -l.
 * - Allow extraction of patches larger than 64k pixels.
 * - Fixed segfault on patches larger than the image buffer.
 * - Fixed segfault on options with missing parameters.
 * - Assorted minor tweakings here and there.
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

#define VERSION "1.4"

extern unsigned char doom_rgb[];
extern unsigned char heretic_rgb[];
extern unsigned char hexen_rgb[];
extern unsigned char strife_rgb[];
unsigned char *palette = doom_rgb;
static unsigned char img_buf[512 * 512];
static int preserve_case = 0;

void usage(char *name, char *option, char error)
{
	if (option && error == 'u')
		fprintf(stderr, "%s: Unkown option: %s\n", name, option);
	if (option && error == 'm')
		fprintf(stderr, "%s: Option %s requires an argument\n", name,
			option);
	if (option && error == 'i')
		fprintf(stderr, "%s: Invalid argument to option %s\n", name,
			option);

	fprintf(stderr, "Usage: %s [-c pal] [-l lump] [-p] wadfile\n", name);
	fprintf(stderr, "\t-c pal: use color palette pal, where pal can be\n");
	fprintf(stderr, "\t        doom, heretic, hexen or strife\n");
	fprintf(stderr, "\t-l: extract only patch in lump <lump>\n");
	fprintf(stderr, "\t-p: preserve case of generated files\n");
	exit(1);
}

void write_ppm(char *name, short width, short height)
{
	FILE		*fp;
	char		fn[50];
	int		i, j;
	unsigned char	pixel;

	strcpy(&fn[0], "patches/");
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
	fprintf(fp, "# CREATOR: wadpatch Release %s\n", VERSION);
	fprintf(fp, "%d %d\n", width, height);
	fprintf(fp, "255\n");

	for (j = 0; j < height; j++) {
		for (i = 0; i < width; i++) {
			pixel = img_buf[j * width + i];
			fputc((int) palette[pixel * 3],     fp);
			fputc((int) palette[pixel * 3 + 1], fp);
			fputc((int) palette[pixel * 3 + 2], fp);
		}
	}

	fclose(fp);
}

void decompile(wadfile_t *wf, char *name)
{
	unsigned char	*patch;
	short		width;
	short		height;
	short		xoff;
	short		yoff;
	short		*p1;
	int		*columns;
	unsigned char	*post;
	int		x;
	int		y;
	int		n;
	int		i;

	/* get patch lump */
	if ((patch = (unsigned char *)get_lump_by_name(wf, name)) == NULL) {
		fprintf(stderr, "can't find patch lump %s\n", name);
		exit(1);
	}

	/* clear the pixel buffer */
	memset(img_buf, 255, sizeof img_buf);

	/* get width, height and offsets from the lump header */
	p1 = (short *) patch;
	width = *p1++;
	swapint(&width);
	height = *p1++;
	swapint(&height);
	xoff = *p1++;
	swapint(&xoff);
	yoff = *p1++;
	swapint(&yoff);
	
	printf("  got patch %s, width=%d, height=%d\n", name,
		width, height);
	if (width == 0 || height == 0) {
		printf("  Warning: patch has null width or height, skipped.\n");
		free (patch);
		return;
	}
	if (width * height > sizeof img_buf) {
		printf("  Warning: patch has more than %d pixels, skipped.\n",
		    (int) sizeof img_buf);
		free(patch);
		return;
	}

	/* copy picture data into pixel buffer */
	columns = (int *) (patch + 8);
	for (x = 0; x < width; x++) {
	  swaplong(&columns);
	  post = patch + *columns++;
	  y = (int) *post++;
	  while (y != 255) {
	    n = (int) *post++;
	    post++;	/* first byte unused */
	    for (i = 0; i < n; i++) {
		img_buf[(y+i) * width + x] = *post;
		post++;
	    }
	    post++;	/* last byte unused */
	    y = (int) *post++;
	  }
	}

	/* free patch lump memory */
	free(patch);

	/* write image buffer into ppm file */
	write_ppm(name, width, height);
}

int main(int argc, char **argv)
{
	char		*program;
	char		*s;
	wadfile_t	*wf;
	void		*pnames = 0;
	int		i;
	int		j;
	int		n;
	char		name[9];
	int		*p1;
	char		*p2;
	char		lump_name[9];

	/* initialize */
	memset(&name[0], 0, sizeof name);
	memset(&lump_name[0], 0, sizeof lump_name);

	/* save program name for usage() */
	program = *argv;

	printf("%s version %s\n\n", program, VERSION);

	/* process options */
	while ((--argc > 0) && (**++argv == '-')) {
	  for (s = *argv+1; *s != '\0'; s++) {
		switch (*s) {
		case 'c':
			argc--;
			argv++;
			if (argc < 1)
				usage (program, argv[-1], 'm');
			if (!strcmp(*argv, "doom"))
				palette = doom_rgb;
			else if (!strcmp(*argv, "heretic"))
				palette = heretic_rgb;
			else if (!strcmp(*argv, "hexen"))
				palette = hexen_rgb;
			else if (!strcmp(*argv, "strife"))
				palette = strife_rgb;
			else
				usage(program, argv[-1], 'i');
			break;
		case 'l':
			argc--;
			argv++;
			if (argc < 1)
				usage (program, argv[-1], 'm');
			*lump_name = '\0';
			strncat(lump_name, *argv, sizeof lump_name - 1);
			break;
		case 'p':
			preserve_case++;
			break;
		default:
			usage(program, --s, 'u');
		}
	  }
	}
	  

	/* have one argument left? */
	if (argc != 1)
		usage(program, NULL, 0);

	/* open WAD file */
	wf = open_wad(*argv);
	
	/* if not -l, we need the PNAMES lump from the WAD file */
	if (! *lump_name) {
		if ((pnames=get_lump_by_name(wf, "PNAMES")) == (void *)0) {
			fprintf(stderr, "can't find lump PNAMES\n");
			exit(1);
		}
		printf("Found PNAMES, decompiling patches...\n");
	}

	/*
	 * make patches directory, ignore errors, we'll handle that later
	 * when we try to write the graphics files into it
	 */
	mkdir("patches", 0755);

	/* if -l, decompile only the patch in lump <lump_name> */
	if (*lump_name) {
		decompile(wf, lump_name);
	}
	/* else, loop over all patch names in PNAMES and decompile them */
        else {
		p1 = (int *) pnames;
		n = *p1;
		swaplong(&n);
		p2 = (char *) (p1 + 1);
		for (i = 0; i < n; i++) {
			strncpy(&name[0], p2, 8);
			for (j = 0; j < 8; j++)
				name[j] = toupper(name[j]);
			decompile(wf, &name[0]);
			p2 += 8;
		}
	}

	/* clean up and done */
	if (pnames)
		free(pnames);
	close_wad(wf);
	return 0;
}
