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
 *      Tool to concatenate multiple WAD files into a single one
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "sysdep.h"
#include "wad.h"
#include "lump_dir.h"
#include "wadfile.h"

#define VERSION "1.0"

void usage(char *name, char *option)
{
	if (option)
		fprintf(stderr, "%s: Unknown option: %s\n", name, option);

	fprintf(stderr, "Usage: %s [-ivp] -o outfile infile1 infile2 ...\n",
		name);

	fprintf(stderr, "\t-i: create IWAD file\n");
	fprintf(stderr, "\t-p: create PWAD file\n");
	fprintf(stderr, "\t-v: verbose output\n");

	exit(1);
}

int main(int argc, char **argv)
{
	char		*program;
	wad_t		wadtype = PWAD;
	wadfile_t	*inwad;
	wadinfo_t	wad_header;
	lumpdir_t	*olp;
	char		lname[9];
	unsigned char	*lump;
	int		dir_offset;
	int		verbose = 0;
	char		*outfile = NULL;
	FILE		*ofp;
	char		*s;
	int		i;
	struct stat	blah;

	/* save program name for usage() */
	program = *argv;

	/* initialize data structures */
	memset(&wad_header, 0, (size_t)sizeof(wad_header));
	memset(&lname[0], 0, 9);

	/* process options */
	while ((--argc > 0) && (**++argv == '-')) {
	    for (s = *argv+1; *s != '\0'; s++) {
		switch (*s) {
		case 'i':
			wadtype = IWAD;
			break;
		case 'p':
			wadtype = PWAD;
			break;
		case 'v':
			verbose++;
			break;
		case 'o':
			outfile = *++argv;
			break;
		default:
			usage(program, --s);
		}
	    }
	}

	if (verbose)
		printf("%s version %s\n\n", program, VERSION);

	/* did we get a filename for the output file? */
	if (!outfile)
		usage(program, NULL);

	/* we need 2 input files at least, else it won't be cat */
	if (--argc < 2)
		usage(program, NULL);

	/* output file may not exist already, could be one of the input files */
	if (stat(outfile, &blah) != -1) {
		fprintf(stderr, "file %s already exists, aborting\n", outfile);
		exit(1);
	}

	/* ok, try to create the output file */
	if (verbose)
		printf("Creating WAD file %s...\n", outfile);
	if ((ofp = fopen(outfile, "wb+")) == NULL) {
		perror("can't create WAD file");
		exit(1);
	}

	/* seek over the WAD header, we need to write it later */
	fseek(ofp, (long)sizeof(wad_header), SEEK_CUR);
	
	/* initialize lump directory for the output file */
	olp = init_lump_dir();

	/* loop over all input file */
	while (argc--) {

	  if (verbose)
	    printf("Processing WAD file %s\n", *argv);

	  /* open next input WAD file */
	  inwad = open_wad(*argv);

	  /* loop over all lumps in the input file */
	  for (i = 0; i < inwad->wh.numlumps; i++) {

	    /* get lump name */
	    strncpy(&lname[0], inwad->lp->lumps[i]->name, 8);
	    if (verbose)
	      printf("\tcopying lump %s..., ", &lname[0]);

	    /* create new directory entry for the lump */
	    new_lump(olp);

	    /* is this a label lump? */
	    if ((inwad->lp->lumps[i]->size == 0) &&
		(inwad->lp->lumps[i]->filepos != 0)) {
		lastlump(olp)->filepos = (int)ftell(ofp);
		swaplong(&(lastlump(olp)->filepos));
		lastlump(olp)->size = 0;
		strncpy((char *)lastlump(olp)->name, &lname[0], 8);
		if (verbose)
		  printf("a label");
	    }

	    /* or is this a marker lump? */
	    else if (inwad->lp->lumps[i]->size == 0) {
		lastlump(olp)->filepos = 0;
		lastlump(olp)->size = 0;
		strncpy((char *)lastlump(olp)->name, &lname[0], 8);
		if (verbose)
		  printf("a marker");
	    }

	    /* ok, must be real data lump then */
	    else {
	      /* get lumpdata from input file */
	      if ((lump = (unsigned char *)get_lump_by_name(inwad, &lname[0]))
		== NULL) {
		fprintf(stderr, "can't get lump %s??\n", &lname[0]);
		exit(1);
	      }

	      lastlump(olp)->filepos = (int)ftell(ofp);
	      swaplong(&(lastlump(olp)->filepos));
	      strncpy((char *)lastlump(olp)->name, &lname[0], 8);
	      if (fwrite(lump, inwad->lp->lumps[i]->size, 1, ofp) == 0) {
		perror("can't write data lump into outfile");
		exit(1);
	      }
	      lastlump(olp)->size = inwad->lp->lumps[i]->size;
	      swaplong(&(lastlump(olp)->size));
	      if (verbose)
		printf("a data lump %d bytes", inwad->lp->lumps[i]->size);

	      /* free allocated lump memory */
	      free(lump);
	    }

	    if (verbose)
		printf("\n");

	  }

	  /* close input WAD file */
	  close_wad(inwad);

	  /* process next input file */
	  argv++;

	}

	/* ok, all lumps from all WAD files added, save current file pointer */
	dir_offset = (int)ftell(ofp);

	/* now write the lump directory */
	if (verbose)
		printf("Writing lump directory...\n");
	for (i = 0; i < olp->num_lumps; i++)
	  if (fwrite((void *)olp->lumps[i], sizeof(filelump_t), 1 ,ofp) == 0) {
	    perror("can't write lump directory");
	    exit(1);
	  }

	/* fill in the WAD header and write it */
	if (verbose)
		printf("Writing WAD header...\n");

	if (wadtype == IWAD)
		strncpy(wad_header.identification, "IWAD", 4);
	else
		strncpy(wad_header.identification, "PWAD", 4);

        wad_header.numlumps = olp->num_lumps;
        swaplong(&(wad_header.numlumps));
        wad_header.infotableofs = dir_offset;
        swaplong(&(wad_header.infotableofs));

        rewind(ofp);

        if (fwrite(&wad_header, (size_t)sizeof(wad_header), 1, ofp) == 0) {
                perror("can't write WAD header");
                exit(1);
        }

        /* all done, close output WAD file and terminate */
        fclose(ofp);

	return 0;
}
