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
 * Include file for the WAD file functions
 */

#ifndef __WADFILE_H__
#define __WADFILE_H__

/* magic number for WAD file structure */
#define WFILE_MAGIC (('W'<<24)+('F'<<16)+('I'<<8)+'L')

typedef struct
{
	unsigned int	magic;		/* magic number */
	FILE		*fp;		/* filepointer for WAD file */
	wadinfo_t	wh;		/* WAD header */
	lumpdir_t	*lp;		/* lump directory */
} wadfile_t;

wadfile_t *open_wad(const char *);
void close_wad(wadfile_t *);
int check_lump_name(wadfile_t *, char *);
void *get_lump_by_name(wadfile_t *, char *);
void *get_map_lump(wadfile_t *, char *, char *, int *);
int lump_name_cmp(const char *, const char *);

#endif
