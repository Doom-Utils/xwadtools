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
 *      Tool to make a color palette for xpaint
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

	/* empty the xpaint palette */
	printf("reset\n\n");

	/* loop over the palette */
	for (i = 0; i < 256; i++) {
		printf("solid #%02x%02x%02x\n", palette[i * 3],
			palette[i * 3 + 1], palette[i * 3 + 2]);
	}

	return(0);
}
