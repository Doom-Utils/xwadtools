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
 *      Tool to make a color quantization map for usage with ppmquant
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
			break;
		default:
			usage(program);
		}
	  }
	}

	/* write P3 header */
	printf("P3\n");
	printf("255 1\n");
	printf("255\n");

	/* loop over palette and write the color defines */
	for (i = 0; i < 256; i++) {
		r = palette[i * 3];
		g = palette[i * 3 + 1];
		b = palette[i * 3 + 2];
		printf("%d %d %d\n", r, g, b);
	}

	return(0);
}
