/************************************************************************/
/*      Copyright (C) 1998, 1999, 2000 by Udo Munk (munkudo@aol.com)    */
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
 *	Library functions to build a lump directory for a WAD file
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wad.h"
#include "lump_dir.h"

#define INCREMENT	10	/* increment for allocating dir entries */

/*
 * Initialize the lump directory
 */
lumpdir_t *init_lump_dir(void)
{
	lumpdir_t *lp;

	/* allocate lump directory */
	if ((lp = (lumpdir_t *)malloc(sizeof(lumpdir_t))) == NULL) {
		fprintf(stderr,
			"init_lump_dir(): can't allocate lumpdir structure\n");
		exit(1);
	}

	/* set magic number */
	lp->magic = LDIR_MAGIC;

	/* no lumps in dir yet */
	lp->num_lumps = 0;

	/* allocate INCREMENT pointers in the lump dir */
	if ((lp->lumps = (filelump_t **)malloc(sizeof(filelump_t *)*INCREMENT))
		== NULL) {
		fprintf(stderr,
			"init_lump_dir(): can't allocate memory for lump directory\n");
		exit(1);
	}

	/* remember how much we have allocated so far */
	lp->alloc_lumps = INCREMENT;

	return lp;
}

/*
 * Free all lump directory memory
 */
void free_lump_dir(lumpdir_t *lp)
{
	int	i;

	/* is argument a lump directory structure? */
	if (lp->magic != LDIR_MAGIC) {
		fprintf(stderr, "free_lump_dir(): not a lumpdir_t structure\n");
		exit(1);
	}

	/* free all allocated lump structures */
	for (i = 0; i < lp->num_lumps; i++)
		free((void *)lp->lumps[i]);

	/* now free the lump directory */
	free((void *)lp->lumps);

	/* free the lump directory structure it self */
	free((void *)lp);
}

/*
 * Create a new empty lump in the lump directory
 */
void new_lump(lumpdir_t *lp)
{
	/* is argument a lump directory structure? */
	if (lp->magic != LDIR_MAGIC) {
		fprintf(stderr, "new_lump(): not a lumpdir_t structure\n");
		exit(1);
	}

	/* if all allocated dir entries are used add INCREMENT new ones */
	if (lp->num_lumps == lp->alloc_lumps) {
		if ((lp->lumps = (filelump_t **)realloc((void *)lp->lumps,
			lp->alloc_lumps * sizeof(filelump_t *) +
			sizeof(filelump_t *) * INCREMENT)) == NULL) {
			fprintf(stderr,
				"new_lump(): can't reallocate memory for lumps\n");
			exit(1);
		}
		lp->alloc_lumps += INCREMENT;
	}

	/* allocate memory for the dir entry and add it to the directory */
	if ((lp->lumps[lp->num_lumps] = (filelump_t *)
		    malloc(sizeof(filelump_t))) == NULL) {
		fprintf(stderr,
			"new_lump(): can't allocate memory for lump dir entry\n");
		exit(1);
	}

	/* clear the new lump entry */
	memset((void *)lp->lumps[lp->num_lumps], 0, sizeof(filelump_t));

	/* one more lump in the dir now */
	lp->num_lumps++;
}
