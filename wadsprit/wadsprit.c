/************************************************************************/
/*      Copyright (C) 1999-2000 by Udo Munk (munkudo@aol.com)           */
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
 *      Tool to extract sprites from a WAD file into ppm files
 */

/*
 * 1.2 (UM 1999-02-15)
 * - Added support for Heretic and Hexen.
 *
 * 1.3 (UM 1999-03-25)
 * - Added support for Strife.
 *
 * 1.4 (UM 2000-01-09)
 * - Stupid Windows can't create files with \ in filename. So don't
 *   abort on write errors immediately, just count the errors and
 *   go on.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
static unsigned char img_buf[320 * 200];
static int preserve_case = 0;
static int errors = 0;

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

void write_ppm(char *name, short width, short height)
{
	FILE		*fp;
	char		fn[50];
	int		i, j;
	unsigned char	pixel;

	strcpy(&fn[0], "sprites/");
	if (preserve_case)
		strcat(&fn[0], name);
	else
		strlcat(&fn[0], name);
	strcat(&fn[0], ".ppm");
	if ((fp = fopen(&fn[0], "wb")) == NULL) {
		fprintf(stderr, "can't open %s for writing\n", &fn[0]);
		errors++;
		return;
	}

	fprintf(fp, "P6\n");
	fprintf(fp, "# CREATOR: wadsprit Release %s\n", VERSION);
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
	unsigned char	*sprite;
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

	/* get sprite lump */
	if ((sprite = (unsigned char *)get_lump_by_name(wf, name)) == NULL) {
		fprintf(stderr, "can't find sprite lump %s\n", name);
		exit(1);
	}

	/* clear the pixel buffer */
	memset(img_buf, 255, 320 * 200);

	/* get width, height and offsets from the lump header */
	p1 = (short *) sprite;
	width = *p1++;
	swapint(&width);
	height = *p1++;
	swapint(&height);
	xoff = *p1++;
	swapint(&xoff);
	yoff = *p1++;
	swapint(&yoff);
	
	printf("  got sprite %s, width=%d, height=%d\n", name,
		width, height);

	/* copy picture data into pixel buffer */
	columns = (int *) (sprite + 8);
	for (x = 0; x < width; x++) {
	  swaplong(&columns);
	  post = sprite + *columns++;
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

	/* free sprite lump memory */
	free(sprite);

	/* write image buffer into ppm file */
	write_ppm(name, width, height);
}

int main(int argc, char **argv)
{
	char		*program;
	char		*s;
	wadfile_t	*wf;
	int		i;
	int		start_flag = 0;
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

	/*
	 * make sprites directory, ignore errors, we'll handle that later
	 * when we try to write the graphics files into it
	 */
	mkdir("sprites", 0755);

	/* loop over all lumps and look for the sprites */
	for (i = 0; i < wf->wh.numlumps; i++) {
		/* start processing after S_START */
		if (!strncmp(wf->lp->lumps[i]->name, "S_START", 7)) {
			start_flag = 1;
			printf("found S_START, start decompiling sprites\n");
			continue;
		} else if (!strncmp(wf->lp->lumps[i]->name, "S_END", 5)) {
			start_flag = 0;
			printf("found S_END, done\n");
			break;
		} else {
			if (start_flag) {
			  strncpy(&name[0], wf->lp->lumps[i]->name, 8);
			  decompile(wf, &name[0]);
			}
		}
	}

	/* print number of errors (for Windows) */
	printf("\n\nNumber of write errors: %d\n", errors);

	/* clean up and done */
	close_wad(wf);
	return 0;
}
