/************************************************************************/
/*      Copyright (C) 1998, 1999 by Udo Munk (munkudo@aol.com)          */
/*	Copyright (C) 1999 by Justin Fletcher                           */
/*			   (doom@thevillage.ndirect.co.uk)              */
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
 *	Tool to create WAD files from labels, markers and data lumps.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef RISCOS
#include "unixlike.h"
#endif
#include <string.h>

#include <sys/types.h>

#include "sysdep.h"
#include "strfunc.h"
#include "wad.h"
#include "lump_dir.h"
#include "lump_cpy.h"

#define VERSION "1.3"

int newargc = 0;
char **newargv = NULL;

/* Add an argument to the current array */
void AddArg(char *arg)
{
  char *string;
  if (newargv == NULL)
    newargv = malloc(sizeof(char*));
  else
    newargv = realloc(newargv, (newargc+1) * sizeof(char*));
  string = strdup(arg);
  if ((newargv == NULL) || (string == NULL))
  {
    fprintf(stderr, "Memory allocation failure in initialisation\n");
    exit(1);
  }
  newargv[newargc++] = string;
}

/* Add arguments from a response file */
void AddResponses(char *file)
{
  FILE *f=fopen(file,"r");
  if (f == NULL)
  {
    fprintf(stderr, "Failed to open response file '%s'\n",file);
    exit(1);
  }
  while (!feof(f))
  {
    char buf[256];
    char *line = fgets(buf, 256, f);
    if (line == NULL)
      continue; /* if it didn't read correctly, ignore line */
    {
      int len = strlen(line);
      /* I don't know what DOS leaves on the ends of its lines */
      if (line[len-1] == '\n') line[--len] = '\0';
      if (line[len-1] == '\r') line[--len] = '\0';
      if (line[len-1] == '\n') line[--len] = '\0';
    }
    while (line != NULL && line[0] != '\0')
    {
      char *space = strchr(buf,' ');
      char *tab = strchr(buf, '\t'); /* Find the first one */
      if ((space == NULL) || ((space > tab) && (tab != NULL)))
        space = tab;
      if (space != NULL)
        *(space++) = '\0';
      if (line[0] != '\0')
        AddArg(line); /* Add this argument */
      line = space;
    }
  }
  fclose(f);
}

/* Make a new table of arguments, so we can include a response file */
void MakeNewArgs(int *argc,char **argv[])
{
  int i;
  char **av = *argv;
  for (i = 0; i < *argc; i++)
  {
    if (av[i][0] == '@')
      AddResponses(&av[i][1]);
    else
      AddArg(av[i]);
  }
  *argc = newargc;
  *argv = newargv;
}

void usage(char *name, char *option)
{
	if (option)
	    fprintf(stderr, "%s: Unknown option: %s\n", name, option);
	else
	    fprintf(stderr, "%s: Missing arguments\n", name);
	fprintf(stderr, "Usage: %s [-ipv] wadfile <lumpname lumpfile ... | @response-file>\n",
		name);
	fprintf(stderr, "\t-i: create IWAD file\n");
	fprintf(stderr, "\t-p: create PWAD file\n");
	fprintf(stderr, "\t-v: verbose output\n");
	exit(1);
}

int main(int argc, char** argv)
{
	wad_t		wadtype = PWAD;
	char		*wadfile;
	FILE		*fp;
	char		*program;
	wadinfo_t	wad_header;
	lumpdir_t	*lp;
	int		dir_offset;
	int		verbose = 0;
	int		i;
	char		*s;

	/* save program name for usage() */
	program = *argv;

	/* initialize WAD header */
	memset(&wad_header, 0, (size_t)sizeof(wad_header));

        MakeNewArgs(&argc,&argv);

	/* process options */
        while ((--argc > 0) && (**++argv == '-'))
        {
          for (s = *argv+1; *s != '\0'; s++)
          {
                        switch (*s)
                        {
			case 'i':
				wadtype = IWAD;
				break;
			case 'p':
				wadtype = PWAD;
				break;
			case 'v':
				verbose++;
				break;
			default:
				usage(program, --s);
			}
		}
	}

	if (verbose)
		printf("%s version %s\n\n", program, VERSION);

	/* next argument should be the WAD filename */
	if (!argc)
		usage(program, NULL);

	argc--;
	wadfile = *argv++;

	/* there must be more arguments, one lump at least */
	if (!argc)
		usage(program, NULL);

	/* create the WAD file */
	if (verbose)
		printf("Creating WAD file %s...\n", wadfile);

        if ((fp = fopen(wadfile, "wb+")) == NULL) {
		perror("can't create WAD file");
		exit(1);
	}

	/* seek over the WAD header, we need to write it later */
	fseek(fp, (long)sizeof(wad_header), SEEK_CUR);

	/* initialize lump directory */
	lp = init_lump_dir();

	/* now process the lumps */
	while (argc) {
		/* we need argument pairs */
		if (argc < 2)
			usage(program, NULL);

		if (!strcmp(*argv, "-label")) {
			if (verbose)
				printf("Adding label %s...", *(argv+1));
			if ((i = strlen(*(argv+1))) > 8) {
			    fprintf(stderr, "\nlump %s: name > 8\n", *(argv+1));
			    exit(1);
			}
		} else if (!strcmp(*argv, "-marker")) {
			if (verbose)
				printf("Adding marker %s...", *(argv+1));
			if ((i = strlen(*(argv+1))) > 8) {
			    fprintf(stderr, "\nlump %s: name > 8\n", *(argv+1));
			    exit(1);
			}
		} else {
			if (verbose)
				printf("Adding lump %s from file %s, ",
				*argv, *(argv+1));
			if ((i = strlen(*argv)) > 8) {
			    fprintf(stderr, "\nlump %s: name > 8\n", *argv);
			    exit(1);
			}
		}
		
		/* create new directory entry for this lump */
		new_lump(lp);

		/* a label, marker or a data lump? */
		if (!strcmp(*argv, "-label")) {
			/* no data, size is 0, offset is current offset */
			lastlump(lp)->filepos = (int)ftell(fp);
			swaplong(&(lastlump(lp)->filepos));
			lastlump(lp)->size = 0;
			/* copy the lump name into the directory entry */
			strncpy((char *)lastlump(lp)->name, *(argv+1), i);
			if (verbose)
				printf("\n");
		} else if (!strcmp(*argv, "-marker")) {
			/* no data, size and file offset are 0 */
			lastlump(lp)->filepos = 0;
			lastlump(lp)->size = 0;
			/* copy the lump name into the directory entry */
			strncpy((char *)lastlump(lp)->name, *(argv+1), i);
			if (verbose)
				printf("\n");
		} else {
			/* set file offset */
			lastlump(lp)->filepos = (int)ftell(fp);
			swaplong(&(lastlump(lp)->filepos));
			/* copy the lump name into the directory entry */
			strncpy((char *)lastlump(lp)->name, *argv, i);
			/* copy the lump data into the WAD file */
			i = copy_from_lump_file(fp, *(argv+1));
			/* set lump size in dir entry */
			lastlump(lp)->size = i;
			swaplong(&(lastlump(lp)->size));
			if (verbose)
				printf("%d bytes...\n", i);
		}

		/* next argument pair */
		argv += 2;
		argc -= 2;
	}

	/* ok, all lumps added, save current file position */
	dir_offset = (int)ftell(fp);

	/* now write the lump directory */
	if (verbose)
		printf("Writing lump directory...\n");
	for (i=0; i<lp->num_lumps; i++)
	    if (fwrite((void *)lp->lumps[i], sizeof(filelump_t), 1 ,fp) == 0) {
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

	wad_header.numlumps = lp->num_lumps;
	swaplong(&(wad_header.numlumps));
	wad_header.infotableofs = dir_offset;
	swaplong(&(wad_header.infotableofs));

	rewind(fp);

	if (fwrite(&wad_header, (size_t)sizeof(wad_header), 1, fp) == 0) {
		perror("can't write WAD header");
		exit(1);
	}

	/* all done, close WAD file and terminate */
	fclose(fp);
#ifdef RISCOS
	settype(wadfile,0x16C); /* Set as a WAD file */
#endif
	return 0;
}
