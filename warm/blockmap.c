/******************************************************************************
	MODULE:		BLOCKMAP.C
	WRITTEN BY:	Robert Fenske, Jr. (rfenske@swri.edu)
	CREATED:	Feb. 1994
	DESCRIPTION:	This module contains routines to generate the BLOCKMAP
			section.  See the generation routine for an explanation
			of the method used to generate the BLOCKMAP.  The
			optimizations for the horizontal LINEDEF, vertical
			LINEDEF, and single block cases came from ideas
			presented in the Unofficial DOOM Specs written by
			Matt Fell.  The BLOCKMAP packing idea came from Jason
			Hoffoss, author of DMapEdit.
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "dmglobal.h"

local short *Blockmap;				/* built BLOCKMAP */


/******************************************************************************
	ROUTINE:	blockmap_add_line(block,lndx,nlines)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	Feb. 1994
	DESCRIPTION:	This routine adds the LINEDEF lndx to the block's
			block LINEDEF list.  If no list exists yet for the
			block, one is created.  Memory is allocated for no more
			than twelve LINEDEFS (to save memory); only if more
			than twelve LINEDEFS are in a single block is more
			memory allocated.  I chose twelve because a statistical
			analysis of many WADs showed that about 99% of the
			BLOCKMAP blocks contained less than twelve LINEDEFS.
******************************************************************************/
#if defined(ANSI_C)
local void blockmap_add_line(short **block, int lndx, long nlines)
#else
local void blockmap_add_line(block,lndx,nlines)
short **block;
int lndx;
long nlines;
#endif
{
  short *newblock;
  int nmax = min(nlines,12);			/* 12 is empirical */
  int k;

  if (*block == NULL) {				/* allocate if no list yet */
    *block = blockmem(short,nmax);
    (*block)[nmax-1] = -1;			/* flag end o' list */
  }
  for (k = 0; 0 < (*block)[k]; k++);		/* seek to end o' list */
  if ((*block)[k] == -1) {			/* hit end o' list--must */
    newblock = blockmem(short,k+1+nmax);	/* allocate more room    */
    blockcopy(newblock,*block,(k+1)*sizeof(**block));
    blockfree(*block);
    *block = newblock;
    (*block)[k+1+nmax-1] = -1;			/* flag end o' list */
  }
  (*block)[k] = lndx+1;				/* add this LINEDEF */
}


/******************************************************************************
	ROUTINE:	blockmap_make(blockmap,lines,nlines,verts)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	Feb. 1994
	DESCRIPTION:	This routine builds the BLOCKMAP section given the
			LINEDEFS and VERTEXES.  The BLOCKMAP section has the
			following information (all are 16-bit words):

			block grid X origin
			block grid Y origin
			# blocks along X axis (total # blocks --> )
			# blocks along Y axis (N = Xn * Yn        )
			block 0 offset (# words)
			block 1 offset (# words)
				.
				.
			block N-1 offset (# words)
			block 0 data (M words: 0,LINEDEFs in block 0,FFFF)
			block 1 data (M words: 0,LINEDEFs in block 1,FFFF)
				.
				.
			block N-1 data (M words: 0,LINEDEFs in block N-1,FFFF)

			Block 0 is at the lower left, block 1 is to the right
			of block 0 along the x-axis, ..., block N-1 is at the
			upper right.  An N-element pointer array is allocated
			to hold pointers to the list of LINEDEFS in each block.
			If no LINEDEFS occur within a block, it's pointer will
			be NULL.  Then the LINEDEFS are scanned to find the
			blocks each line occupies, building the lists of
			LINEDEFS along the way.  Four cases are considered
			for each LINEDEF.  The line is either diagonal,
			horizontal, vertical, or resides in a single block
			(regardless of orientation).  The non-diagonal cases
			can be optimized since the blocks occupied can be
			directly calculated.  The diagonal case basically
			computes the blocks occupied on each row for all the
			rows between the LINEDEF endpoints.  Once this is
			complete the actual blockmap is allocated and filled.
			It returns the number of words in the blockmap.
	MODIFIED:		Robert Fenske, Jr.	Mar. 1994
			Added in the optimizations for the orthogonal line
			cases using the ideas presented in the Unofficial DOOM
			Specs written by Matt Fell.
				Robert Fenske, Jr.	Feb. 1995
			Added in packing of the map.  All the empty blocks
			reference a single block.  This idea came from
			Jason Hoffoss, author of DMapEdit.
******************************************************************************/
#if defined(ANSI_C)
long blockmap_make(short **blockmap, WAD_LINE *lines, long nlines,
                   WAD_VERT *verts)
#else
long blockmap_make(blockmap,lines,nlines,verts)
short **blockmap;				/* built BLOCKMAP */
WAD_LINE *lines;				/* map LINEDEFS */
long nlines;					/* map # lines */
WAD_VERT *verts;				/* map VERTEXES */
#endif
{
  short xmin,ymin, xmax,ymax;			/* map coords min/max */
  long scl;					/* line following scaling */
  long size = 0x80;				/* block size (map coords) */
  short xorig, yorig;				/* blockmap x,y origin */
  short xn, yn;					/* # blocks in x,y dirs */
  long xst, xsf;				/* scaled x from/to */
  long xf,yf, xt,yt;				/* from here to here */
  long xd, yd;					/* x direction, y direction */
  long m;					/* diagonal line slope */
  int o;
  int l, k, i;
  int p = 0;					/* increment to next block */
  int c = 0;					/* # blocks to consider */
  short **boxlist;				/* array of blocks' lists */
  long b;					/* block number */
  long t;

  xmin = ymin = (short)0x7FFF, xmax = ymax = (short)0x8000;
  for (l = 0; l < nlines; l++) {		/* find min/max map coords */
    xf = verts[lines[l].fndx].x, yf = verts[lines[l].fndx].y;
    if (xf < xmin) xmin = (short)xf;		/* check from vertex */
    if (yf < ymin) ymin = (short)yf;
    if (xmax < xf) xmax = (short)xf;
    if (ymax < yf) ymax = (short)yf;
    xt = verts[lines[l].tndx].x, yt = verts[lines[l].tndx].y;
    if (xt < xmin) xmin = (short)xt;		/* check to vertex */
    if (yt < ymin) ymin = (short)yt;
    if (xmax < xt) xmax = (short)xt;
    if (ymax < yt) ymax = (short)yt;
  }
  xorig = xmin-8;				/* get x origin */
  yorig = ymin-8;				/* get y origin */
  xn = (xmax-xorig+size)/size;			/* get # in x direction */
  yn = (ymax-yorig+size)/size;			/* get # in y direction */
  boxlist = blockmem(short *,xn*yn);
  scl = 81920L/(100+yn);			/* so scl*scl*size*yn<2^31-1 */
  size *= scl;
  t = 0;					/* total len of all lists */
  for (l = 0; l < nlines; l++) {		/* scan LINEDEFS here */
    xf = scl*((long)verts[lines[l].fndx].x-(long)xorig);
    yf = scl*((long)verts[lines[l].fndx].y-(long)yorig);
    xt = scl*((long)verts[lines[l].tndx].x-(long)xorig);
    yt = scl*((long)verts[lines[l].tndx].y-(long)yorig);
    xd = sgn(xt-xf), yd = sgn(yt-yf);
    switch (2*(xf/size == xt/size) + (yf/size == yt/size)) {
     case  0: c=0,                             p=yd*xn;/* diagonal line */
     bcase 1: c=abs((int)(xt/size-xf/size))+1, p=xd;/* horizontal line */
     bcase 2: c=abs((int)(yt/size-yf/size))+1, p=yd*xn;/* vertical line */
     bcase 3: c=0+1,                           p=1;/* start,end same block */
    }
    b = xf/size + xn*(yf/size);			/* start @ this block */
    for (i = 0; i < c; i++, b+=p) {		/* add to lists for special */
      blockmap_add_line(&boxlist[b],l,nlines);	/* cases: horizontal,       */
      t++;					/* vertical, & single block */
    }
    if (c == 0) {				/* handle diagonal lines */
      m = scl*(yt-yf)/(xt-xf);			/* spanning > 1 block    */
      if (m == 0) m = sgn(yt-yf)*sgn(xt-xf);	/* force a min non-0 slope */
      xsf = xf;
      if (yd == -1) xst = xf + scl*((yf/size)*size -    1 - yf)/m;
      else          xst = xf + scl*((yf/size)*size + size - yf)/m;
      do {
        for (c = 0; c <= abs((int)(xst/size - xsf/size)); c++, b+=xd) {
          blockmap_add_line(&boxlist[b],l,nlines);
          t++;					/* + to total # line entries */
        }
        b += p-xd;				/* will start @ this block */
        xsf = xst, xst += yd*scl*size/m;
        if (xd*xst > xd*xt) xst = xt;		/* don't overrun endpoint */
      } while (xd*xsf < xd*xt);
    }
  }
  Blockmap = blockmem(short,4+xn*yn+t+2*xn*yn);
  t = 0;
  Blockmap[t++] = xorig;			/* fill in X,Y origin */
  Blockmap[t++] = yorig;
  Blockmap[t++] = xn;				/* fill in # in X and */
  Blockmap[t++] = yn;				/* Y directions       */
  for (k = 0, i = 0; i < xn*yn; i++)		/* count # empty blocks */
    if (boxlist[i] == NULL) k++;
  o = t;					/* offsets start here */
  l = t += xn*yn;
  Blockmap[t++] = 0;				/* all empty blocks will */
  Blockmap[t++] = -1;				/* point to this one     */
  for (i = 0; i < xn*yn; i++)			/* now fill in BLOCKMAP */
    if (boxlist[i] != NULL) {			/* this block is occupied */
      Blockmap[o++] = t;			/* offset in BLOCKMAP */
      Blockmap[t++] = 0;			/* always zero */
      for (k = 0; 0 < boxlist[i][k]; k++)	/* list of lines in this */
        Blockmap[t++] = boxlist[i][k]-1;	/* block                 */
      blockfree(boxlist[i]);
      Blockmap[t++] = -1;			/* always -1 */
      if (t >= 65536L)				/* remaining offsets are bad */
        printf("\nERROR: BLOCKMAP too large; must reduce map size\n");
    }else
      Blockmap[o++] = l;			/* point to empty block */
  blockfree(boxlist);
  if (blockmap != NULL) *blockmap = Blockmap;
  return t;					/* # words in BLOCKMAP */
}
