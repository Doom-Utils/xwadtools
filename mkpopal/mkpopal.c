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
 *      Tool to make a color include file for povray
 */

#include <stdio.h>
#include <string.h>

extern unsigned char doom_rgb[];
extern unsigned char heretic_rgb[];
extern unsigned char hexen_rgb[];
extern unsigned char strife_rgb[];
unsigned char *palette = doom_rgb;

void usage(char *s)
{
    fprintf(stderr, "Usage: %s [-c palette]\n", s);
    fprintf(stderr, "\tpalette can be one of doom, heretic, hexen or strife\n");
    exit(1);
}

int main(int argc, char **argv)
{
	char	*program;
	char	*s;
	int	i;
	int	r, g, b;
	char	*palname = "doom";

	/* process options */
	program = argv[0];
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
				usage(program);
			palname = *argv;
			break;
		default:
			usage(program);
		}
	  }
	}

	/* write some documentation header */
	printf("// Pre-defined colors for %s for use in scene files\n",
		palname);
	printf("// generated with %s\n\n", program);

	/* loop over palette and write the color defines */
	for (i = 0; i < 256; i++) {
		r = palette[i * 3];
		g = palette[i * 3 + 1];
		b = palette[i * 3 + 2];
		printf("#declare %s%d = rgb <%d/255, %d/255, %d/255>\n",
			palname, i, r, g, b);
	}

	return(0);
}
