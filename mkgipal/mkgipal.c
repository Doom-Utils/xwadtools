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
 *      Tool to make a color palette for gimp
 */

/*
 * 1.1 (UM 1999-05-31)
 * - Instead of Untitled write color index number, is more comfortable
 *   to use in the gimp palette dialog.
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

	/* gimp wants this */
	printf("GIMP Palette\n");
	printf("# %s palette for gimp\n", palname);

	/* loop over the palette */
	for (i = 0; i < 256; i++) {
		printf("%3d %3d %3d\tcolor no. %d\n", palette[i * 3],
			palette[i * 3 + 1], palette[i * 3 + 2], i);
	}

	return(0);
}
