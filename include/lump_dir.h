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
 * Include file for the lump directory functions
 */

#ifndef __LUMP_DIR_H__
#define __LUMP_DIR_H__

#define LDIR_MAGIC	(('L'<<24)+('D'<<16)+('I'<<8)+'R')

/* structure of a lump directory */
typedef struct
{
	unsigned int	magic;		/* magic number (LDIR_MAGIC) */
	filelump_t	**lumps;	/* directory entries */
	int		num_lumps;	/* number of lumps in directory */
	int		alloc_lumps;	/* number of alloced lump dir entries */
} lumpdir_t;

lumpdir_t *init_lump_dir(void);		/* initialize lump dir */
void free_lump_dir(lumpdir_t *);	/* free lump dir memory */
void new_lump(lumpdir_t *);		/* create new empty entry in lump dir */

/* access macro for the last lump created with new_lump() */
#define lastlump(x) (x->lumps[x->num_lumps - 1])

#endif
