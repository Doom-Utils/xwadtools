/************************************************************************/
/*      Copyright (C) 1998, 1999 by Udo Munk (munkudo@aol.com)          */
/*	Copyright (C) 1999, 2000 by Andre Majorel (amajorel@teaser.fr)	*/
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
 *      Tool to list the lump directory of WAD files
 */

/*
 * 1.6 (AYM 1999-05-23)
 * - Added options -h, -H, -k and -s.
 *
 * 1.7 (UM 1999-07-06)
 * - Fixed assumption made that flats always have a size of 4096, when
 *   option -F used. Hexen.wad uses larger sized flats for water. Now
 *   anything between F_START and F_END with a size > 0 is assumed to
 *   be a flat.
 *
 * 1.8 (AYM 2000-01-06)
 * - Can now take more than one argument.
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "sysdep.h"
#include "wad.h"

#define VERSION "1.8"

static int list_wad(const char *file);
static void err(const char *file, const char *fmt, ...);

static char *program = NULL;
static int verbose = 0;
static int hflag = 0;
static int Hflag = 0;
static int kflag = 0;
static int lstflag = 0;
static int sflag = 0;
static int pflag = 0;
static int tflag = 0;
static int fflag = 0;

void usage(char *option)
{
	if (option)
            err(NULL, "Unknown option: %s", option);
        else
            err(NULL, "Missing arguments");
	fprintf(stderr, "Usage: %s [-hHklsv] [-F|-P|-T] wadfile ...\n", program);
	fprintf(stderr, "\t-h: if offsets are listed, print them in hex\n");
	fprintf(stderr, "\t-H: if sizes are listed, print them in hex\n");
	fprintf(stderr, "\t-k: if sizes are listed, print them in kB\n");
	fprintf(stderr, "\t-l: list position and size of the lumps\n");
	fprintf(stderr, "\t-s: list the size of the lumps (in bytes)\n");
	fprintf(stderr, "\t-v: verbose output\n");
	fprintf(stderr, "\t-P: list contents of the PNAMES lump\n");
	fprintf(stderr, "\t-T: list contents of the TEXTUREx lumps\n");
	fprintf(stderr, "\t-F: list all flats between labels F_START/F_END\n");
	exit(1);
}

void print_name(char *n)
{
	printf("%c%c%c%c%c%c%c%c", n[0] ? n[0] : ' ',
				   n[1] ? n[1] : ' ',
				   n[2] ? n[2] : ' ',
				   n[3] ? n[3] : ' ',
				   n[4] ? n[4] : ' ',
				   n[5] ? n[5] : ' ',
				   n[6] ? n[6] : ' ',
				   n[7] ? n[7] : ' ');
}

int main(int argc, char **argv)
{
	int		exit_code = 0;
	int		show_file_name = 0;
	char		*s;

        /* save program name for usage() */
        program = *argv;

        /* process options */
        while ((--argc > 0) && (**++argv == '-')) {
                for (s = *argv + 1; *s != '\0'; s++) {
                        switch (*s) {
			case 'h':
			        hflag++;
				break;
			case 'H':
			        Hflag++;
				break;
			case 'k':
			        kflag++;
				break;
                        case 'l':
                                lstflag++;
                                break;
			case 's':
				sflag++;
				break;
                        case 'v':
                                verbose++;
                                break;
			case 'P':
				pflag++;
				break;
			case 'T':
				tflag++;
				break;
			case 'F':
				fflag++;
				break;
                        default:
                                usage(--s);
                        }
                }
        }

        if (verbose)
                printf("%s version %s\n\n", program, VERSION);

        /* next argument should be the WAD filename */
        if (!argc)
                usage(NULL);

	/* list the contents of all the wads */
	if (argc > 1)
		show_file_name = 1;
	else
		show_file_name = 0;
	for (; argc > 0; argc--, argv++) {
		int r;
		if (show_file_name)
			printf("%.128s:\n", *argv);
		r = list_wad(*argv);
		if (r)
			exit_code = 2;
		if (show_file_name && argc > 1)
			putchar('\n');
	}

	return exit_code;
}

/*
 *	list_wad - list the contents of one wad file
 */
static int list_wad(const char *file)
{
	wadinfo_t	wad_header;
	filelump_t	lump;
	FILE		*fp;
	int		list_flats = 0;
	int		i, j;
	int		n, m;
	int		toff;
	char		name[9];
	long		old_off, tex_off, sav_off;

	/* try to open WAD file */
        if ((fp = fopen(file, "rb")) == NULL) {
                err(file, "can't open (%s)", strerror (errno));
                return 1;
        }

	/* read WAD file header */
	if (fread((void *)&wad_header, sizeof(wadinfo_t), 1, fp) != 1) {
		err(file, "can't read WAD header");
		return 1;
	}
	swaplong(&(wad_header.numlumps));
	swaplong(&(wad_header.infotableofs));

	/* is this a WAD file? */
	if (strncmp(wad_header.identification, "IWAD", 4) &&
	    strncmp(wad_header.identification, "PWAD", 4)) {
		err(file, "not a WAD file");
		return 1;
	}

	if (verbose) {
		printf("WAD type = %c%c%c%c\n", wad_header.identification[0],
			wad_header.identification[1],
			wad_header.identification[2],
			wad_header.identification[3]);
		printf("no. of lumps: %d\n", wad_header.numlumps);
		printf("directory offset: %d\n", wad_header.infotableofs);
	}

	/* now seek to the directory */
	if (fseek(fp, (long)wad_header.infotableofs, SEEK_SET) == -1) {
		err(file, "can't seek to directory");
		return 1;
	}

	/* now read the directory entries */
	for (i = 0; i < wad_header.numlumps; i++) {
		if (fread((void *)&lump, sizeof(filelump_t), 1, fp) != 1) {
			err(file, "can't read lump directory entry");
			return 1;
		}
		swaplong(&(lump.filepos));
		swaplong(&(lump.size));

		/* list contents of PNAMES lump? */
		if (pflag) {
		  if (strncmp(&lump.name[0], "PNAMES", 6)) {
		    continue;
		  } else {
		    if (verbose) {
		      printf("\nContents of PNAMES:\n");
		    }
		    if (fseek(fp, (long)lump.filepos, SEEK_SET) == -1) {
			    err(file, "can't seek to PNAMES");
			    return 1;
		    }
		    n = getlong(fp);
		    memset(&name[0], 0, 9);
		    for (i = 0; i < n; i++) {
		      fread((void *)&name[0], 8, 1, fp);
		      printf("%s\n", &name[0]);
		    }
		    break;
		  }

		/* list contents of TEXTUREx lumps? */
		} else if (tflag) {
		  if (strncmp(&lump.name[0], "TEXTURE1", 8) &&
		      strncmp(&lump.name[0], "TEXTURE2", 8)) {
			continue;
		  } else {
		    old_off = ftell(fp);
		    memset(&name[0], 0, 9);
		    strncpy(&name[0], &lump.name[0], 8);
		    if (verbose) {
		      printf("\nContents of %s:\n", &name[0]);
		    }
		    if (fseek(fp, (long)lump.filepos, SEEK_SET) == -1) {
			    err(file, "can't seek to %s", file, &name[0]);
			    return 1;
		    }
		    tex_off = ftell(fp);
		    n = getlong(fp);
		    for (j = 0; j < n; j++) {
		      toff = getlong(fp);
		      sav_off = ftell(fp);
		      if (fseek(fp, tex_off + toff, SEEK_SET) == -1) {
			err(file, "can't seek to texture");
			return 1;
		      }
		      fread((void *)&name[0], 8, 1, fp);
		      printf("%s", &name[0]);
		      if (lstflag) {
			m = getshort(fp);	/* always 0 */
			m = getshort(fp);	/* always 0 */
			m = getshort(fp);	/* width */
			printf("\t%d", (int)m);
			m = getshort(fp);	/* height */
			printf("\t%d", (int)m);
			m = getshort(fp);	/* always 0 */
			m = getshort(fp);	/* always 0 */
			m = getshort(fp);	/* number of patches */
			printf("\t%d", (int)m);
		      }
		      putchar('\n');
		      fseek(fp, sav_off, SEEK_SET);
		    }
		    fseek(fp, old_off, SEEK_SET);
		  }

		/* list flats between F_START and F_END? */
		} else if (fflag) {
		  if (!strncmp(&lump.name[0], "F_START", 7)) {
			list_flats = 1;
			continue;
		  } else if (!strncmp(&lump.name[0], "F_END", 5)) {
			list_flats = 0;
			break;
		  } else {
			if (list_flats && (lump.size > 0)) {
				print_name(&lump.name[0]);
				putchar('\n');
			}
		  }

		/* list contents of the whole WAD file */
		} else {
		  print_name(&lump.name[0]);
		  if (lstflag || sflag) {
		  	if (kflag) {
				printf("\t%8d", (lump.size + 512) / 1024);
			} else if (Hflag) {
				printf("\t%08Xh", lump.size);
			} else {
				printf("\t%8d", lump.size);
			}
		  }
		  if (lstflag) {
			if (hflag) {
		  		printf("\t%08Xh", lump.filepos);
			} else {
				printf("\t%8d", lump.filepos);
			}
		  }
		  putchar('\n');
		}
	}

	return 0;
}

/*
 *	err - printf an error message to stderr
 */
static void err(const char *file, const char *fmt, ...)
{
	va_list args;

	fflush (stdout);
	if (program != NULL)
		fprintf (stderr, "%.32s: ", program);
	if (file != NULL)
		fprintf (stderr, "%.128s: ", file);
	va_start (args, fmt);
	vfprintf (stderr, fmt, args);
	fputc ('\n', stderr);
	fflush (stderr);  /* Paranoia */
}
