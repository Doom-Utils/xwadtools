/************************************************************************/
/*      Copyright (C) 1998, 1999 by Udo Munk (munkudo@aol.com)	        */
/*      Copyright (C) 1999-2001 by Andre Majorel (amajorel@teaser.fr)   */
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
 *   Tool to convert raw flats data extracted from a WAD file into ppm files
 */

/*
   AYM 1999-10-06
     New options -c, -h, -I, -n, -o, -s, -v, -w.
     Can now process more than one file at a time.
     Name of output file is now implied.

   UM 1999-11-17
     Use Andre's my_getopt instead depending on libc getopt.

   AYM 2001-01-24
     Added ROTT palette.
*/

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>

#include "mygetopt.h"

#define MAXLEN 256

static int process_file(const char *ifile, const char *ofilename, int width,
		int height, long pixels);
static void err(const char *fmt, ...);

extern const unsigned char doom_rgb[];
extern const unsigned char doom04_rgb[];
extern const unsigned char doom04t_rgb[];
extern const unsigned char doom05_rgb[];
extern const unsigned char *doom05t_rgb;
extern const unsigned char heretic_rgb[];
extern const unsigned char hexen_rgb[];
extern const unsigned char rott_rgb[];
extern const unsigned char strife_rgb[];

static long offset     = 0;
static long pixels     = -1;
static int  width      = -1;
static int  height     = -1;
static int  verbose    = 0;
static int  interleave = 0;
static const unsigned char *pal = doom_rgb;

void usage(char *name)
{
	fprintf(stderr,
	    "Usage: %s [options] file ...\n",
	    name);
	fprintf (stderr,
"Options:\n"
"  -c game  Palette (doom|doom04|..04t|..05|..05t|heretic|hexen|rott|strife)\n"
"  -h n     Set image height to n pixels. Default 64.\n"
"  -I       Use Doom alpha interleaving scheme.\n"
"  -n n     Read only n bytes of file. Default up to EOF.\n"
"  -o file  Set output name. Default is input name with .ppm extension.\n"
"  -s n     Skip first n bytes of file. Default 0.\n"
"  -v       Verbose mode.\n"
"  -w n     Set image width to n pixels. Default 64.\n");
	exit(1);
}

int main(int argc, char **argv)
{
	char		*program;
	const char	*output_file_name = NULL;  /* -o */
	int		rc = 0;

	/* save program name for usage() */
	program = *argv;

	/* parse options */
	{
	int r;
	while ((r = my_getopt(argc, argv, "c:h:In:o:s:vw:")) != EOF) {
		if (r == '?')
			usage(program);
		else if (r == 'c') {
			if (! strcmp(my_optarg, "doom"))
				pal = doom_rgb;
			else if (! strcmp(my_optarg, "doom04"))
				pal = doom04_rgb;
			else if (! strcmp(my_optarg, "doom04t"))
				pal = doom04t_rgb;
			else if (! strcmp(my_optarg, "doom05"))
				pal = doom05_rgb;
			else if (! strcmp(my_optarg, "doom05t"))
				pal = doom05t_rgb;
			else if (! strcmp(my_optarg, "heretic"))
				pal = heretic_rgb;
			else if (! strcmp(my_optarg, "hexen"))
				pal = hexen_rgb;
			else if (! strcmp(my_optarg, "rott"))
				pal = rott_rgb;
			else if (! strcmp(my_optarg, "strife"))
				pal = strife_rgb;
			else
				usage(program);
		}
		else if (r == 'h') {
			char *p;
			height = strtol(my_optarg, &p, 0);
			if (*my_optarg == '\0' || *p != '\0' || height < 1) {
				err("-h wants a strictly positive integer");
				exit(1);
			}
		}
		else if (r == 'I') {
			interleave = 1;
		}
		else if (r == 'n') {
			char *p;
			pixels = strtol(my_optarg, &p, 0);
			if (*my_optarg == '\0' || *p != '\0' || pixels < 0) {
				err("-n wants a positive integer");
				exit(1);
			}
		}
		else if (r == 'o') {
			if (strlen(my_optarg) > MAXLEN) {
				err("-o: name too long");
				exit(1);
			}
			output_file_name = my_optarg;
		}
		else if (r == 's') {
			char *p;
			offset = strtol(my_optarg, &p, 0);
			if (*my_optarg == '\0' || *p != '\0' || offset < 0) {
				err("-s wants a positive integer");
				exit(1);
			}
		}
		else if (r == 'v') {
			verbose = 1;
		}
		else if (r == 'w') {
			char *p;
			width = strtol(my_optarg, &p, 0);
			if (*my_optarg == '\0' || *p != '\0' || width < 1) {
				err("-w wants a strictly positive integer");
				exit(1);
			}
		}
		else
			usage(program);  /* can't happen */
	}
	}

	/* must have at least one file name */
	if (argc - my_optind < 1)
		usage(program);

	/* -o not allowed if more than one file name */
	if (output_file_name != NULL && argc - my_optind > 1) {
		err("-o not allowed if more than one file name");
		exit(1);
	}

	for (; my_optind < argc; my_optind++) {
		const char *ifilename = argv[my_optind];
		char ofilename[MAXLEN + 1];

		if (strlen(ifilename) > MAXLEN) {
			err("%.256s: name too long", ifilename);
			err("%.256s: fatal error--skipping file", ifilename);
			rc = 2;
			continue;
		}
		/* if the -o option was given, use that. Else build output
		   file name by appending ".ppm" to input file name after
		   having stripped it of any ".lmp" extension */
		if (output_file_name != NULL) {
			strcpy(ofilename, output_file_name);
		}
		else {
			const char *const ext1 = ".lmp";
			const char *const ext2 = ".ppm";
			char *p;

			strcpy(ofilename, ifilename);
			p = ofilename + strlen(ofilename);
			if (strlen(ofilename) >= strlen(ext1)
				&& !strcmp(p - strlen(ext1), ext1)) {
				p -= strlen(ext1);
				*p = '\0';
			}
			if (strlen(ofilename) + strlen(ext2) > MAXLEN) {
				err("%.256s: name too long", ifilename);
				err("%.256s: fatal error--skipping file");
				rc = 2;
				continue;
			}
			strcpy(p, ext2);
		}
		if (verbose)
			printf("%.256s -> %.256s\n", ifilename, ofilename);
		if (process_file(ifilename, ofilename, width, height, pixels))
			rc = 2;
	}
	return rc;
}

/*
 *	process_file
 *	Read one input file and write one output file
 *	Return 0 on success, non-zero on failure
 */
static int process_file(const char *ifile, const char *ofilename, int width,
		int height, long pixels)
{
	FILE *ifp = NULL;
	FILE *ofp = NULL;
	int rc = 0;

	if ((ifp = fopen(ifile, "rb")) == NULL) {
		err("can't open \"%.256s\" for reading (%s)",
				ifile, strerror(errno));
		rc = 1;
		goto byebye;
	}

	/* number of bytes (pixels) to read from the file */
	if (pixels < 0) {
		int r = fseek(ifp, 0, SEEK_END);
		if (r) {
			err("%.256s: can't seek to EOF (%s)",
					ifile, strerror(errno));
			rc = 1;
			goto byebye;
		}
		pixels = ftell(ifp) - offset;
		rewind(ifp);
	}

	/* if neither width nor height is given, assume width is 64 */
	if (width < 0 && height < 0) {
		width = 64;
		height = pixels / width;
	}
	/* if only width is given, deduce height from width */
	else if (height < 0 && width > 0) {
		height = pixels / width;
	}
	/* and conversely */
	else if (width < 0 && height > 0) {
		width = pixels / height;
	}
	/* if both are specified, just do that (adjust pixels later) */
	else if (width > 0 && height > 0) {
		;
	}
	/* can't happen */
	else {
		err("report this bug to the maintainer ! (w=%d, h=%d)",
				width, height);
		exit(1);
	}

	if ((long) width * height < pixels) {
		err("%.256s: dropping %ld pixels",
			ifile, pixels - (long) width * height);
		pixels = (long) width * height;
	}
	else if ((long) width * height > pixels) {
		err("%.256s: not enough data for %d x %d = %ld pixels",
				width, height, (long) width * height);
		rc = 1;
		goto byebye;
	}

	/* write the PPM header */
	if ((ofp = fopen(ofilename, "wb")) == NULL) {
		err("can't open \"%.256s\" for writing (%s)",
				ofilename, strerror(errno));
		rc = 1;
		goto byebye;
	}
	fprintf(ofp, "P6\n");
	fprintf(ofp, "# CREATOR: raw2ppm\n");
	fprintf(ofp, "%d %d\n", width, height);
	fprintf(ofp, "255\n");

	/* honour the -s (skip) option */
	if (offset != 0) {
		int r = fseek(ifp, offset, SEEK_SET);
		if (r) {
			err("%.256s: can't seek to %08lXh (%s)",
					ifile, offset, strerror(errno));
			rc = 1;
			goto byebye;
		}
	}

	/* write the PPM pixel data */
	{
		/* Freaky Doom alpha interleaving algorithm. The input file
		   contains 4 frames, each as high as the final image but only
		   1/4th as wide. The first frame contains columns 0, 4, 8...
		   of the final image. The second frame contains columns 1,
		   5... The third frame contains columns 2, 6... The fourth
		   frame contains columns 3, 7... VGA framebuffer layout ?? */
		if (interleave) {
			unsigned char *buf = malloc(pixels);
			unsigned char *p   = buf;
			long n;
			if (buf == NULL) {
				err("%.256s: not enough memory", ifile);
				rc = 1;
				goto byebye;
			}
			for (n = 0; n < pixels; n++) {
				int c = getc(ifp);
				if (c == EOF) {
					err("%.256s: unexpected EOF", ifile);
					rc = 1;
					goto byebye;
				}
				*p = c;
				p += 4;
				if (p >= buf + pixels)  /* new frame */
					p -= pixels - 1;
			}
			for (n = 0; n < pixels; n++) {
				int o = 3 * buf[n];
				putc(pal[o],     ofp);
				putc(pal[o + 1], ofp);
				putc(pal[o + 2], ofp);
			}
			free(buf);
		}
		/* normal, non-interleaved data */
		else {
			long n;
			for (n = 0; n < pixels; n++) {
				int c = getc(ifp);
				if (c == EOF) {
					err("%.256s: unexpected EOF", ifile);
					rc = 1;
					goto byebye;
				}
				putc(pal[c * 3],     ofp);
				putc(pal[c * 3 + 1], ofp);
				putc(pal[c * 3 + 2], ofp);
			}
		}
	}

byebye:
	if (ifp != NULL)
		fclose(ifp);
	if (ofp != NULL)
		fclose(ofp);
	if (rc)
		err("%.256s: fatal error--skipping file");
	return rc;
}

/*
 *	err
 *	printf() a message to stderr, prefixed by "raw2ppm: "
 *	and followed by a newline.
 */
static void err(const char *fmt, ...)
{
	va_list args;
	fflush(stdout);
	fputs("raw2ppm: ", stderr);
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	putc('\n', stderr);
	fflush(stderr);  /* not normally needed */
}
