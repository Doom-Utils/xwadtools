/************************************************************************/
/*      Copyright (C) 1998, 1999 by Udo Munk (munkudo@aol.com)          */
/*      Copyright (C) 1998, 2000 by Andre Majorel (amajorel@teaser.fr)  */
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
 *
 * 1.8 (AYM 2000-10-26)
 * - Added option "-n" to just list the composition of textures without
 *   saving them to file.
 * - Now accepts any number (including zero) of pwads.
 * - Changed the mode of textures/ from 0755 to 0777 on the theory that
 *   limiting permissions is the job of umask, not of the application.
 * - Fixed most of the counterintuitive quirks w.r.t. to the handling of
 *   wads, notably the fact that the pwad had to have a PNAMES and
 *   TEXTURE1. It's now possible to extract textures from a pwad that
 *   replaces just patches like dmspac.zip.
 * - Program name is now hard-coded because I felt it didn't make much
 *   sense to have wadtex print "stupid version 1.8" on startup just
 *   because someone renamed the binary to "stupid".
 * - Removed the arbitrary limit of 1024x256 on image size by making the
 *   texture buffer dynamically allocated. It's now possible to extract
 *   all textures from the shareware Hacx wad, including "AD" that is
 *   2048 wide. I just knew I had the C programmer's disease.
 * - Error messages now printed through new err() function so that they
 *   don't cut stdout messages in the middle when redirected.
 * - Added option "-w" to extract only some of the textures.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fnmatch.h>

#include "sysdep.h"
#include "strfunc.h"
#include "wad.h"
#include "lump_dir.h"
#include "wadfile.h"

#define VERSION "1.8"

/* item in a linked list of wads */
typedef struct listitem_s {
	struct listitem_s *next;
	char *name;
	wadfile_t *wf;
} listitem_t;

const char program[] = "wadtex";

extern unsigned char doom_rgb[];
extern unsigned char heretic_rgb[];
extern unsigned char hexen_rgb[];
extern unsigned char strife_rgb[];
static unsigned char *palette = doom_rgb;

static unsigned char *img_buf;
static size_t        img_buf_sz;

static int preserve_case = 0;
static int tflag;
static int extract = 1;
static char *wildcard;

static enum {
	OIT_IF_ALL,	/* Output the texture iff no patch is missing */
	OIT_IF_ONE,	/* Output the texture iff at least one patch found */
	OIT_ALWAYS	/* Always output the texture */
} mpatch_policy = OIT_IF_ONE;

void err(const char *fmt, ...)
{
	va_list list;

	fflush(stdout);
	fprintf(stderr, "%s: ", program);
	va_start(list, fmt);
	vfprintf(stderr, fmt, list);
	va_end(list);
	fputc('\n', stderr);
}

void usage(const char *option)
{
	if (option)
		err("Unkown option: %s", option);

	printf("Usage: %s [-c pal] [-i iwad] [-m|-M] [-npt] [-w expr] [wadfile ...]\n",
	    program);
	printf("\t-c pal: use color palette pal, where pal can be\n");
	printf("\t        doom, heretic, hexen or strife\n");
	printf("\t-i iwad: use iwad to find patches not found in wadfile\n");
	printf("\t-m: don't output texture if patches are missing\n");
	printf("\t-M: output texture even if all patches are missing\n");
	printf("\t-n: list the composition without creating any PPM files\n");
	printf("\t-p: preserve case of generated files\n");
	printf("\t-t: also write text description of textures for wadgc\n");
	printf("\t-w expr: only extract textures matching wildcard expr\n");
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
	  err("texture uses patch %d, but PNAMES has %d patches only", pnum, p_num);
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
		err("can't open %s for writing", &fn[0]);
		exit(1);
	}

	fprintf(fp, "P6\n");
	fprintf(fp, "# CREATOR: %s Release %s\n", program, VERSION);
	fprintf(fp, "%hd %hd\n", width, height);
	fprintf(fp, "255\n");

	for (j = 0; j < height; j++) {
		for (i = 0; i < width; i++) {
			pixel = img_buf[j * width + i];
			fputc((int) palette[pixel * 3],     fp);
			fputc((int) palette[pixel * 3 + 1], fp);
			fputc((int) palette[pixel * 3 + 2], fp);
		}
	}

	if (fclose(fp)) {
		err("%s: %s", fn, strerror(errno));
		exit(1);
	};
}

/*
 * returns 0 on success, non-zero if not all textures were properly extracted.
 */
int decompile(listitem_t *wadlist, char *section, void *texture, void *pnames)
{
	int	rc = 0;
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

	/* initialize */
	memset(&tname[0], 0, sizeof tname);

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
		{
		  int n;
		  for (n = 0; n < sizeof tname; n++)
			  tname[n] = toupper(((unsigned char *) tname)[n]);
		}
		if (wildcard != NULL && fnmatch(wildcard, tname, 0) != 0)
			continue;

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

		/* grow the texture buffer if necessary */
		{
		  unsigned long newsize = width * height;
		  if ((size_t) newsize != newsize) {
		    err("Texture too big, size_t overflow (%hd x %hd). Skipping.",
			width, height);
		    rc = 1;
		    continue;
		  }
		  if (newsize > img_buf_sz) {
		    unsigned char *newbuf = realloc(img_buf, newsize);
		    if (newbuf == NULL) {
		      err("Texture too big, not enough memory (%d x %d). Skipping.",
			  width, height);
		      rc = 1;
		      continue;
		    }
		    img_buf_sz = newsize;
		    img_buf    = newbuf;
		  }
		}

		/* want text file too? */
		if (tflag) {
		  strcpy(&fn[0], "textures/");
		  if (preserve_case)
		    strcat(&fn[0], &tname[0]);
		  else
		    strlcat(&fn[0], &tname[0]);
		  strcat(&fn[0], ".wgc");
		  if ((fp = fopen(&fn[0], "w")) == NULL) {
		    err("can't open %s for writing", &fn[0]);
		    exit(1);
		  }
		  fprintf(fp, "%s_START\n", section);
		  fprintf(fp, "  %s %d %d %d\n", &tname[0], width, height,
		      num_pat);
		}

		/* clear pixel buffer */
		memset(img_buf, 255, img_buf_sz);

		/* loop over all patches for the texture */
		patches_found = 0;
		for (j = 0; j < num_pat; j++) {
		  const char *patchname = NULL;
		  listitem_t *wad;

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
		  for (wad = wadlist; wad != NULL; wad = wad->next) {
		    if (get_patch(wad->wf, fp, pnames, width, height,
			  xoff, yoff, pnum, &patchname) == 0) {
		      patches_found++;
		      break;
		    }
		  }
		  if (wad == NULL)
		    printf("    Warning: patch \"%.8s\" is missing\n",
			patchname);
		}

		if (patches_found == 0 && mpatch_policy != OIT_ALWAYS) {
		  printf("    Warning: skipping void texture \"%s\".\n", tname);
		  rc = 1;
		}
		else if (patches_found < num_pat && mpatch_policy == OIT_IF_ALL) {
		  printf("    Warning: skipping incomplete texture \"%s\".\n", tname);
		  rc = 1;
		}
		else
		  if (extract)
		    write_ppm(&tname[0], width, height);

		/* terminate and close the text file too */
		if (tflag) {
		  fprintf(fp, "%s_END\n", section);
		  if (fclose(fp)) {
		    err("%s: %s", fn, strerror(errno));
		    exit(1);
		  }
		}
	}
	return rc;
}

int main(int argc, char **argv)
{
	int        rc = 0;		/* Exit code */
	char       *s;
	listitem_t *wadlist;		/* Linked list of open wads */
	char       *iwad = NULL;	/* Filename of the iwad (or NULL) */
	void       *pnames;		/* The contents of the PNAMES lump */
	void       *texture1;		/* The contents of the TEXTURE1 lump */
	void       *texture2;		/* The contents of the TEXTURE2 lump */

	printf("%s version %s\n\n", program, VERSION);

	/* process options */
	while ((--argc > 0) && (**++argv == '-')) {
	  for (s = *argv+1; *s != '\0'; s++) {
		switch (*s) {
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
				usage(NULL);
			break;

		case 'i':
			argc--;
			argv++;
			iwad = *argv;
			break;

		case 'm':
			mpatch_policy = OIT_IF_ALL;
			break;

		case 'M':
			mpatch_policy = OIT_ALWAYS;
			break;

		case 'n':
			extract = 0;
			break;

		case 'p':
			preserve_case++;
			break;

		case 't':
			tflag++;
			break;

		case 'w':
			argc--;
			argv++;
			wildcard = *argv;
			{
			  char *p;
			  for (p = wildcard; *p != '\0'; p++)
				  *p = toupper(*((unsigned char *) p));
			}
			break;

		default:
			usage(--s);
		}
	  }
	}

	if (argc == 0 && iwad == NULL) {
	  err("No wad given");
	  exit(1);
	}

	/* build list of wads, last pwad first, iwad last */
	wadlist = NULL;
	if (iwad != NULL) {
	  wadlist = malloc(sizeof *wadlist);
	  if (wadlist == NULL) {
	    err("Not enough memory");
	    exit(1);
	  }
	  wadlist->name = iwad;
	  wadlist->wf   = open_wad(iwad);
	  wadlist->next = NULL;
	}
	for (; argc > 0; argv++, argc--) {
	  listitem_t *newitem = malloc(sizeof *newitem);
	  if (newitem == NULL) {
	    err("Not enough memory");
	    exit(1);
	  }
	  /* insert at the beginning of the list */
	  newitem->name = *argv;
	  newitem->wf   = open_wad(*argv);
	  newitem->next = wadlist;
	  wadlist = newitem;
	}

	/* locate PNAMES, TEXTURE1 and TEXTURE2 */
	pnames   = NULL;
	texture1 = NULL;
	texture2 = NULL;
	{
	  listitem_t *wad;

	  for (wad = wadlist; wad != NULL; wad = wad->next) {
	    if (pnames == NULL)
	      pnames = get_lump_by_name(wad->wf, "PNAMES");
	    if (texture1 == NULL)
	      texture1 = get_lump_by_name(wad->wf, "TEXTURE1");
	    if (texture2 == NULL)
	      texture2 = get_lump_by_name(wad->wf, "TEXTURE2");
	  }
	}
	if (pnames == NULL) {
		err("No PNAMES lump found");
		exit(1);
	}
	if (texture1 == NULL) {
		err("No TEXTURE1 lump found");
		exit(1);
	}
	    
	/*
	 * make textures directory, ignore errors, we'll handle that later
	 * when we try to write the graphics files into it
	 */
	if (extract || tflag)
		mkdir("textures", 0777);

	/* decompile the textures */
	printf("Got TEXTURE1 lump, decompiling...\n");
	{
	  int r = decompile(wadlist, "TEXTURE1", texture1, pnames);
	  if (r > rc)
		  rc = r;
	  if (texture2 != NULL) {
		  printf("Got TEXTURE2 lump, decompiling...\n");
		  r = decompile(wadlist, "TEXTURE2", texture2, pnames);
		  if (r > rc)
			  rc = r;
	  }
	}

	/* clean up and done */
	if (pnames != NULL)
	  free(pnames);
	if (texture1 != NULL)
	  free(texture1);
	if (texture2 != NULL)
	  free(texture2);
	{
	  listitem_t *wad, *next;

	  for (wad = wadlist; wad != NULL;) {
	    next = wad->next;
	    if (wad->wf != NULL)
	      close_wad(wad->wf);
	    free(wad);
	    wad = next;
	  }
	}
	if (img_buf != NULL)
	  free(img_buf);
	return rc;
}
