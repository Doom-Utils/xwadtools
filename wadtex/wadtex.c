/************************************************************************/
/*      Copyright (C) 1998, 1999 by Udo Munk (um@compuserve.com)        */
/*      Copyright (C) 1998, 1999 by Andre Majorel (amajorel@teaser.fr)  */
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
 *      Tool to extract wall textures from a WAD file into ppm files
 */

/*
 * 1.3 (AYM 1998-12-22)
 * - Fixed bug that made the first row and the first column appear pink.
 * - Fixed bug in the handling of patches with negative Y offsets.
 * - The names of the output files are now forced to lower case
 *   (rationale: since case is not significant in wad directories, why
 *   not use the most convenient case)
 * - Added the "-p" option to inhibit the above feature.
 *
 * 1.4 (AYM 1999-01-14)
 * - A missing patch is not a fatal error anymore. wadtexext can now be
 *   used on texture pwads.
 * - Added options "-m" and "-M" to tweak the behaviour in face of missing
 *   patches.
 *
 * 1.5 (UM 1999-02-15)
 * - Added support for Heretic and Hexen.
 *
 * 1.6 (AYM 1999-03-07)
 * - Fixed segfault with large textures. Textures that are too large to
 *   fit in the buffer are now cleanly clipped with a warning.
 * - Increased the size of the buffer from 256x200 to 1024x256. I
 *   suspect that 256 might turn out to be insufficient, though... I
 *   must have a bad case of the C programmer's disease... And I should
 *   clip by (width x height) instead of dumbly clipping by width and
 *   height separately.
 *     (UM 1999-03-25)
 * - Added support for Strife.
 *
 * 1.7 (UM 1999-08-23)
 * - Added option -i iwad, so that patches missing in a PWAD can be
 *   used from the IWAD, to complete a texture.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#include "sysdep.h"
#include "strfunc.h"
#include "wad.h"
#include "lump_dir.h"
#include "wadfile.h"

#define VERSION "1.7"

#define TEX_BUF_WIDTH  1024
#define TEX_BUF_HEIGHT 256

extern unsigned char doom_rgb[];
extern unsigned char heretic_rgb[];
extern unsigned char hexen_rgb[];
extern unsigned char strife_rgb[];
static unsigned char *palette = doom_rgb;

static unsigned char img_buf[TEX_BUF_WIDTH * TEX_BUF_HEIGHT];

static int preserve_case = 0;
static int tflag;

static enum {
	OIT_IF_ALL,	/* Output the texture iff no patch is missing */
	OIT_IF_ONE,	/* Output the texture iff at least one patch found */
	OIT_ALWAYS	/* Always output the texture */
} mpatch_policy = OIT_IF_ONE;

static wadfile_t *iwad;
static void *ipnames;

void usage(char *name, char *option)
{
	if (option)
		fprintf(stderr, "%s: Unkown option: %s\n", name, option);

	fprintf(stderr, "Usage: %s [-tp] [-m|-M] [-c pal] [-i iwad] wadfile\n", name);
	fprintf(stderr, "\t-t: also write text description of textures for wadgc\n");
	fprintf(stderr, "\t-p: preserve case of generated files\n");
	fprintf(stderr, "\t-m: don't output texture if patches are missing\n");
	fprintf(stderr, "\t-M: output texture even if all patches are missing\n");
	fprintf(stderr, "\t-c pal: use color palette pal, where pal can be\n");
	fprintf(stderr, "\t        doom, heretic, hexen or strife\n");
	fprintf(stderr, "\t-i iwad: use iwad to find patches not found in wadfile\n");
	exit(1);
}

int get_patch(wadfile_t *wf, FILE *fp, void *pnames, short width, short height,
	       short xoff, short yoff, short pnum, const char **patchname)
{
	int		i;
	int		x, y;
	int		n;
	int		*p1;
	char		*p2;
	int		p_num;
	char		name[9];
	unsigned char	*picture;
	short		pwidth;
	int		*columns;
	unsigned char	*post;

	/* initialization */
	memset(&name[0], 0, 9);

	/* number of patches in PNAMES */
	p1 = (int *) pnames;
	p_num = *p1++;
	swaplong(&p_num);
	if (pnum > p_num) {
	  fprintf(stderr, "texture uses patch %d, but PNAMES has %d patches only\n", pnum, p_num);
	  exit(1);
	}

	/* lookup name for that patch */
	p2 = ((char *) p1) + 8 * pnum;
	*patchname = (const char *) p2;
	strncpy(&name[0], p2, 8);
	for (i = 0; i < 8; i++) {
		name[i] = toupper(name[i]);
	}

	/* and get that picture lump */
	if ((picture=(unsigned char *)get_lump_by_name(wf, &name[0])) == NULL) {
		return 1;
	}

	/* copy picture data into pixel buffer */
	pwidth = *((short *) picture);
	swapint(&pwidth);
	printf("    Got patch %s, width %d, x=%d y=%d\n", &name[0], pwidth,
			xoff, yoff);
	columns = (int *) (picture + 8);
	for (x = 0; x < pwidth; x++) {
	  swaplong(columns);
	  post = picture + *columns++;
	  y = (int) *post++;
	  while (y != 255) {
	    n = (int) *post++;
	    post++;	/* first byte unused */
	    for (i = 0; i < n; i++) {
		if ((y+yoff+i >= 0) && (x+xoff >= 0) &&
		    (y+yoff+i < height) && (x+xoff < width))
		  img_buf[(y+yoff+i) * width + (x+xoff)] = *post;
		post++;
	    }
	    post++;	/* last byte unused */
	    y = (int) *post++;
	  }
	}

	/* text file too? */
	if (tflag) {
	  fprintf(fp, "    %s %d %d\n", &name[0], xoff, yoff);
	}

	/* free picture lump memory */
	free(picture);
	return 0;
}

void write_ppm(const char *name, short width, short height)
{
	FILE		*fp;
	char		fn[50];
	int		i, j;
	unsigned char	pixel;

	strcpy(&fn[0], "textures/");
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
	fprintf(fp, "# CREATOR: wadtex Release %s\n", VERSION);
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

void decompile(wadfile_t *wf, char *section, void *texture, void *pnames)
{
	int	i, j;
	int	patches_found;
	int	*p1;
	char	*p2;
	short	*p3;
	int	num_tex;
	int	off_tex;
	short	dummy;
	short	width, height, num_pat;
	short	xoff, yoff, pnum;
	char	tname[9];
	FILE	*fp = NULL;
	char	fn[50];
	int	actual_width;	/* dimensions after clipping to fit buffer */
	int	actual_height;

	/* initialize */
	memset(&tname[0], 0, 9);

	/* get number of textures in texture lump */
	p1 = (int *) texture;
	num_tex = *p1;
	swaplong(&num_tex);
	printf("  found %d textures\n", num_tex);

	/* loop over all textures */
	for (i = 0; i < num_tex; i++) {

		/* offset for texture in lump */
		p1 = (int *) texture + i + 1;
		off_tex = *p1;
		swaplong(&off_tex);
		p2 = (char *) texture + off_tex;

		/* get name of texture */
		strncpy(&tname[0], p2, 8);

		/* get texture composition */
		p3 = (short *) (p2 + 8);
		dummy = *p3++;		/* always 0 */
		dummy = *p3++;		/* always 0 */
		width = *p3++;
		swapint(&width);
		height = *p3++;
		swapint(&height);
		dummy = *p3++;		/* always 0 */
		dummy = *p3++;		/* always 0 */
		num_pat = *p3++;
		swapint(&num_pat);

		printf("  decompiling texture %s, %d patches, %dx%d\n",
		  &tname[0], num_pat, width, height);

		/* clip to fit in texture buffer */
		actual_width  = width;
		actual_height = height;
		if (actual_width > TEX_BUF_WIDTH)
		  actual_width = TEX_BUF_WIDTH;
		if (actual_height > TEX_BUF_HEIGHT)
		  actual_height = TEX_BUF_HEIGHT;
		if (actual_width != width || actual_height != height)
	  	  printf("    Warning: texture too large, clipped to %dx%d\n",
		    actual_width, actual_height);

		/* want text file too? */
		if (tflag) {
		  strcpy(&fn[0], "textures/");
		  if (preserve_case)
		    strcat(&fn[0], &tname[0]);
		  else
		    strlcat(&fn[0], &tname[0]);
		  strcat(&fn[0], ".wgc");
		  if ((fp = fopen(&fn[0], "w")) == NULL) {
		    fprintf(stderr, "can't open %s for writing\n", &fn[0]);
			exit(1);
		  }
		  fprintf(fp, "%s_START\n", section);
		  fprintf(fp, "  %s %d %d %d\n", &tname[0], actual_width,
		  	actual_height, num_pat);
		}

		/* clear pixel buffer */
		memset(img_buf, 255, sizeof(img_buf));

		/* loop over all patches for the texture */
		patches_found = 0;
		for (j = 0; j < num_pat; j++) {
		  const char *patchname = NULL;
		  xoff = *p3++;
		  swapint(&xoff);
		  yoff = *p3++;
		  swapint(&yoff);
		  if (yoff < 0)
                    yoff = 0;
		  pnum = *p3++;
		  swapint(&pnum);
		  dummy = *p3++;	/* always 1, stepdir? what is that?? */
		  dummy = *p3++;	/* always 0 for colormap 0 */
		  if (!get_patch(wf, fp, pnames, actual_width, actual_height,
		  		xoff, yoff, pnum, &patchname))
		    goto foundit;
		  if (iwad) {
			if (get_patch(iwad, fp, ipnames, actual_width,
				actual_height, xoff, yoff, pnum, &patchname))
		  	  printf("    Warning: patch \"%.8s\" is missing\n",
					patchname);
		  else
foundit:
		    patches_found++;
		  }
		}

		if (patches_found == 0 && mpatch_policy != OIT_ALWAYS)
		  printf("    Warning: skipping void texture \"%s\".\n",
			&tname[0]);
		else if (patches_found < num_pat && mpatch_policy == OIT_IF_ALL)
		  printf("    Warning: skipping incomplete texture \"%s\".\n",
			&tname[0]);
		else
		  write_ppm(&tname[0], actual_width, actual_height);

		/* terminate and close the text file too */
		if (tflag) {
		  fprintf(fp, "%s_END\n", section);
		  fclose(fp);
		}
	}
}

int main(int argc, char **argv)
{
	wadfile_t	*wf;
	char		*program;
	void		*pnames;
	void		*texture;
	char		*s;

	/* save program name for usage() */
	program = *argv;

	printf("%s version %s\n\n", program, VERSION);

	/* process options */
	while ((--argc > 0) && (**++argv == '-')) {
	  for (s = *argv+1; *s != '\0'; s++) {
		switch (*s) {
		case 't':
			tflag++;
			break;

		case 'p':
			preserve_case++;
			break;

		case 'M':
			mpatch_policy = OIT_ALWAYS;
			break;

		case 'm':
			mpatch_policy = OIT_IF_ALL;
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

		case 'i':
			argc--;
			argv++;
			iwad = open_wad(*argv);
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

	/* we need lumps PNAMES and TEXTURE1 at least */
	if ((pnames = get_lump_by_name(wf, "PNAMES")) == (void *)0) {
		fprintf(stderr, "can't find lump PNAMES\n");
		exit(1);
	}
	printf("Got PNAMES lump...\n");
	if ((texture = get_lump_by_name(wf, "TEXTURE1")) == (void *)0) {
		fprintf(stderr, "can't find lump TEXTURE1\n");
		exit(1);
	}

	/* if we got an IWAD, we want PNAMES from that one too */
	if (iwad) {
	  if ((ipnames = get_lump_by_name(iwad, "PNAMES")) == (void *)0) {
		fprintf(stderr, "can't find lump PNAMES in IWAD\n");
		exit(1);
	  }
	}

	/*
	 * make textures directory, ignore errors, we'll handle that later
	 * when we try to write the graphics files into it
	 */
	mkdir("textures", 0755);

	/* decompile the textures */
	printf("Got TEXTURE1 lump, decompiling...\n");
	decompile(wf, "TEXTURE1", texture, pnames);

	/* free TEXTURE1 and lets see if we can get TEXTURE2 too */
	free(texture);
	if ((texture = get_lump_by_name(wf, "TEXTURE2")) != (void *)0) {
		printf("Got TEXTURE2 lump, decompiling...\n");
		decompile(wf, "TEXTURE2", texture, pnames);
		free(texture);
	}

	/* clean up and done */
	free(pnames);
	close_wad(wf);
	return 0;
}
