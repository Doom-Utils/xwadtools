/************************************************************************/
/*      Copyright (C) 1998, 1999, 2000 by Udo Munk (munkudo@aol.com)    */
/*	Copyright (C) 1999 by Andre Majorel (amajorel@teaser.fr)        */
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
 *	Library functions to deal with WAD files
 */

/*
 * AYM 1999-01-06
 * Wrote specialized function to compare lump names, in place of strncmp().
 * This fixes the bug on STEP1 in wadflatext. The new function is (like
 * Doom) case-insensitive.
 *
 * UM 1999-06-26
 * Added function check_lump_name() to check for presence of a lump.
 *
 * UM 1999-06-28
 * Added Hexen BEHAVIOR lump to get_map_lump().
 *
 * UM 1999-08-23
 * Print file name which can't be opened by open_wad().
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "sysdep.h"
#include "wad.h"
#include "lump_dir.h"
#include "wadfile.h"

/*
 * Open a WAD file for reading and read its header and lump directory
 */
wadfile_t *open_wad(const char *name)
{
	wadfile_t	*wf;
	int		i;

	/* allocate a WAD file structure */
	if ((wf = (wadfile_t *)malloc(sizeof(wadfile_t))) == NULL) {
	    fprintf(stderr, "open_wad(): can't allocate WAD file structure\n");
	    exit(1);
	}

	/* set magic number */
	wf->magic = WFILE_MAGIC;

	/* try to open WAD file */
	if ((wf->fp = fopen(name, "rb")) == NULL) {
		fprintf(stderr, "open_wad(): can't open WAD file %s ", name);
		perror("");
		exit(1);
	}

	/* read WAD file header */
	if (fread((void *)&wf->wh, sizeof(wadinfo_t), 1, wf->fp) != 1) {
		fprintf(stderr, "open_wad(): can't read WAD header\n");
		exit(1);
	}
	swaplong(&(wf->wh.numlumps));
	swaplong(&(wf->wh.infotableofs));

        /* is this a WAD file? */
        if (strncmp(wf->wh.identification, "IWAD", 4) &&
            strncmp(wf->wh.identification, "PWAD", 4)) {
                fprintf(stderr, "open_wad(): %s isn't a WAD file\n", name);
                exit(1);
        }

        /* now seek to the directory */
        if (fseek(wf->fp, (long)wf->wh.infotableofs, SEEK_SET) == -1) {
                fprintf(stderr, "open_wad(): can't seek to directory\n");
                exit(1);
        }

	/* initialize lump directory */
	wf->lp = init_lump_dir();

	/* read the directory entries and save them in memory */
	for (i = 0; i < wf->wh.numlumps; i++) {
		/* create a new directory entry */
		new_lump(wf->lp);
		/* and read data into it */
		if (fread((void *)wf->lp->lumps[wf->lp->num_lumps-1],
		    sizeof(filelump_t), 1, wf->fp) != 1) {
			fprintf(stderr,
			    "open_wad(): can't read lump directory entry\n");
			exit(1);
		}
		swaplong(&(wf->lp->lumps[wf->lp->num_lumps-1]->filepos));
		swaplong(&(wf->lp->lumps[wf->lp->num_lumps-1]->size));
	}

	return wf;
}

/*
 * Close WAD file and free all memory
 */
void close_wad(wadfile_t *wf)
{
	/* is this a wadfile_t structure? */
	if (wf->magic != WFILE_MAGIC) {
		fprintf(stderr, "close_wad(): not a wadfile_t structure\n");
		exit(1);
	}

	/* close file */
	fclose(wf->fp);

	/* free memory for lump directory */
	free_lump_dir(wf->lp);
}

/*
 * Check if a lump exists in the WAD file directory, returns 1 if it
 * does, 0 otherwise.
 */
int check_lump_name(wadfile_t *wf, char *lname)
{
	int i;

	/* is this a wadfile_t structure? */
	if (wf->magic != WFILE_MAGIC) {
	    fprintf(stderr, "check_lump_name(): not a wadfile_t structure\n");
	    exit(1);
	}

	/*
	 * Do a linear search over the lump directory and try to find the lump.
	 */
	for (i = 0; i < wf->lp->num_lumps; i++)
	    if (!lump_name_cmp(wf->lp->lumps[i]->name, lname))
		return 1;
	return 0;
}

/*
 * Find a lump by name in the WAD file directory. If found allocate
 * memory and read it's data.
 */
void *get_lump_by_name(wadfile_t *wf, char *lname)
{
	int i;
	int found = 0;
	void *lump = (void *)0;

	/* is this a wadfile_t structure? */
	if (wf->magic != WFILE_MAGIC) {
	    fprintf(stderr, "get_lump_by_name(): not a wadfile_t structure\n");
	    exit(1);
	}

	/* check lump name length */
	if (strlen(lname) > 8) {
	    fprintf(stderr, "get_lump_by_name(): lump name longer 8 chars\n");
	    exit(1);
	}

	/*
	 * Do a linear search over the lump directory and find the lump.
	 * This would be faster with a hashtable build by open_wad() and left
	 * in the wadfile_t structure, but this just are some tools and we
	 * don't need to deal with the data in realtime (yet). Anyway, there
	 * is room for improvements...
	 */
	for (i = 0; i < wf->lp->num_lumps; i++) {
		if (!lump_name_cmp(wf->lp->lumps[i]->name, lname)) {
			found++;
			break;
		}
	}

	/* found it? */
	if (!found)
		return lump;

	/* allocate memory for the lump data */
	if ((lump = malloc(wf->lp->lumps[i]->size)) == NULL) {
	    fprintf(stderr,
		"get_lump_by_name(): can't allocate memory for lump data\n");
	    exit(1);
	}

	/* seek to the lump in WAD file */
        if (fseek(wf->fp, (long)wf->lp->lumps[i]->filepos, SEEK_SET) == -1) {
	    fprintf(stderr, "get_lump_by_name(): can't seek to lump data\n");
	    exit(1);
        }

	/* read the lump data */
	if (fread(lump, wf->lp->lumps[i]->size, 1, wf->fp) != 1) {
	    fprintf(stderr, "get_lump_by_name(): can't read lump data\n");
	    exit(1);
	}

	return lump;
}

/*
 * Find a map lump by name in the WAD file directory. If found allocate
 * memory and read it's data.
 */
void *get_map_lump(wadfile_t *wf, char *map, char *lname, int *size)
{
	int mentry;
	int found = 0;
	void *lump = (void *)0;
	int i;

        /* is this a wadfile_t structure? */
        if (wf->magic != WFILE_MAGIC) {
                fprintf(stderr, "get_map_lump(): not a wadfile_t structure\n");
                exit(1);
        }

	/* now try find the map in the lump directory for the wad */
	for (mentry = 0; mentry < wf->lp->num_lumps; mentry++) {
		if (!strcmp(wf->lp->lumps[mentry]->name, map)) {
			found++;
			break;
		}
	}

	/* found it? */
	if (!found)
		return lump;

	/* set mentry to the first data lump for the map */
	mentry++;

	/* now search for the data lump we are looking for in this map */
	for (i = 0; i < 11; i++) {
		/* check if the lump belongs to the map */
		if ((strncmp(wf->lp->lumps[mentry+i]->name,"THINGS",6)!=0) &&
		    (strncmp(wf->lp->lumps[mentry+i]->name,"LINEDEFS",8)!=0) &&
		    (strncmp(wf->lp->lumps[mentry+i]->name,"SIDEDEFS",8)!=0) &&
		    (strncmp(wf->lp->lumps[mentry+i]->name,"VERTEXES",8)!=0) &&
		    (strncmp(wf->lp->lumps[mentry+i]->name,"SEGS",4)!=0) &&
		    (strncmp(wf->lp->lumps[mentry+i]->name,"SSECTORS",8)!=0) &&
		    (strncmp(wf->lp->lumps[mentry+i]->name,"NODES",5)!=0) &&
		    (strncmp(wf->lp->lumps[mentry+i]->name,"SECTORS",7)!=0) &&
		    (strncmp(wf->lp->lumps[mentry+i]->name,"REJECT",6)!=0) &&
		    (strncmp(wf->lp->lumps[mentry+i]->name,"BLOCKMAP",8)!= 0) &&
		    (strncmp(wf->lp->lumps[mentry+i]->name,"BEHAVIOR",8)!= 0))
			break;	/* not a lump for this map anymore */
		if (!strncmp(wf->lp->lumps[mentry+i]->name, lname,
			     strlen(lname))) {
			/* found it, get the lump */
			*size = wf->lp->lumps[mentry+i]->size;
			if ((lump = malloc(*size)) == NULL) {
			  fprintf(stderr,
			      "get_map_lump(): can't allocate memory\n");
			  exit(1);
			}
			if (fseek(wf->fp,
			    (long)wf->lp->lumps[mentry+i]->filepos,
			     SEEK_SET) == -1) {
			  fprintf(stderr,
			      "get_map_lump(): can't seek to lump\n");
			  exit(1);
			}
			if (fread(lump, *size, 1, wf->fp) != 1) {
			  fprintf(stderr, "get_map_lump(): can't read lump\n");
			  exit(1);
			}
			break;
		}
	}

	return lump;
}

/*
 * Compare two lump names.
 * Return value is similar to strcmp's.
 */
int lump_name_cmp(const char *name1, const char *name2)
{
	int n;

	for (n = 0; n < 8; n++)	{
		if (toupper(name1[n]) != toupper(name2[n]))
		  return toupper(name1[n]) - toupper(name2[n]);
		if (name1[n] == '\0')
		  return 0;
	}
	return 0;
}
