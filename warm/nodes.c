/******************************************************************************
	MODULE:		NODES.C
	WRITTEN BY:	Robert Fenske, Jr. (rfenske@swri.edu)
	CREATED:	Mar. 1994
	DESCRIPTION:	This module contains routines to generate the SEGS,
			SSECTORS, and NODES sections.  It needs only the
			LINEDEFS and VERTEXES as input.  These three sections
			combine to form a binary space partition tree.  This
			tree is built by recursively dividing the space into
			sections that balance (or at least try to) the number
			of segments and produce the least number of segment
			splits.  The nodes are kept in a binary tree.  The
			segments are kept in an ordered doubly-linked list.
			The created vertices are added to the end of the
			vertex list.  Once the divisions are complete, the
			SEGS, SSECTORS, and NODES structures are created from
			the binary tree and the segment and vertex lists.  Any
			memory allocated for the binary tree or the linked
			list is freed when no longer needed.

			This module does not do any error checking on any
			memory allocation.  If any allocation ever fails, this
			module will bomb.

			This module used to do some error checking while
			building the node tree, but now the tree is generated
			regardless of whether the input describes a correct,
			playable map (though it certainly may bomb if the
			input is not a valid map).

			I wrote the code from the description of the binary
			space partition in the Unofficial DOOM Specs written
			by Matt Fell.  I found these references after I did
			the coding:

			1) Computer Graphics Principles and Practice,
			   2nd ed 1990
			   Foley J.D., van Dam A., Feiner S.K., Hughes J.F.
			   ISBN 0-201-12110-7

			2) "On Visible Surface Generation by a Priori Tree
			   Structures"
			   Fuchs H., Kedem Z.M., Naylor B.F.
			   Computer Graphics (SIGGRAPH '80 Proceedings)
			   v14 #3 July 1980 pp 124-133

			3) "Near Real-Time Shaded Display of Rigid Objects"
			   Fuchs H., Abram G.D., Grant E.D.
			   Computer Graphics (SIGGRAPH '83 Proceesings)
			   v17 #3 July 1983 pp 65-72
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "dmglobal.h"

#if !defined(M_SQRT2)				/* not all systems have this */
#define M_SQRT2		1.41421356237309504880	/* defined (why?)            */
#endif
#define MAX_SHORT	((short)~(1<<(8*sizeof(short)-1)))
#define MAX_LONG	((long)~(1L<<(8*sizeof(long)-1)))

#define tree_branch(c)	((c)=blockmem(NODE_TREE,1), \
			 (c)->left=NULL, (c)->right=NULL, (c))
#define two_sided(l)	(lines[l].lsidndx != -1)
#define vdist2(v1,v2)	(((long)(v1).x-(v2).x)*((long)(v1).x-(v2).x)+\
			 ((long)(v1).y-(v2).y)*((long)(v1).y-(v2).y))

#define update_limits(box,vf,vt)		/* update x,y box limits */ \
			if ((vf)->x < (box)->xmin) (box)->xmin = (vf)->x; \
			if ((vf)->y < (box)->ymin) (box)->ymin = (vf)->y; \
			if ((vt)->x < (box)->xmin) (box)->xmin = (vt)->x; \
			if ((vt)->y < (box)->ymin) (box)->ymin = (vt)->y; \
			if ((box)->xmax < (vf)->x) (box)->xmax = (vf)->x; \
			if ((box)->ymax < (vf)->y) (box)->ymax = (vf)->y; \
			if ((box)->xmax < (vt)->x) (box)->xmax = (vt)->x; \
			if ((box)->ymax < (vt)->y) (box)->ymax = (vt)->y;

typedef struct LIMIT_BOX {			/* x,y limits/bounds */
	short xmin, ymin, xmax, ymax;
} LIMIT_BOX;

typedef struct SEGS_INFO {
	WAD_SEGS seg;				/* a SEGment */
	short sector;				/* associated sector number */
	struct SEGS_INFO *prev, *next;		/* to previous, next SEGment */
} SEGS_INFO;

typedef struct NODE_TREE {
	LIMIT_BOX lim;				/* node rectangle limits */
	SEGS_INFO *pseg;			/* partition line SEG */
	SEGS_INFO *segs;			/* node's SEGS */
	short nsegs;				/* # initial SEGS in node */
	struct NODE_TREE *left, *right;		/* left, right children */
} NODE_TREE;

typedef struct NODE_INFO {
	WAD_NODE *nodes;			/* all nodes */
	int nnodes;				/* total # NODES */
	WAD_VERT *sverts;			/* all SEGS vertices */
	int nsverts;				/* total # SEGS vertices */
	WAD_SEGS *segs;				/* all nodes' SEGS */
	int nsegs;				/* total # segs */
	WAD_SSECTOR *ssecs;			/* all nodes' SSECTORs */
	int nssecs;				/* total # SSECTORs */
	SEGS_INFO *sinfo;			/* all SEGS lists */
	int nverts;				/* # original vertices */
	int nlines;				/* # lines */
	boolean limit_io;			/* limit screen I/O flag */
	boolean use_all_segs;			/* use all SEGS flag */
	boolean *sectmult;			/* SSECTORS w/mult sects flg */
	int nsects;				/* # unique sectors */
	LIMIT_BOX *sectbox;			/* sector x,y bounds */
	long *sectuse;				/* sector w.r.t. partition */
} NODE_INFO;

local NODE_INFO ninfo;				/* node information */


/******************************************************************************
	ROUTINE:	nodes_segs_init(lines,nlines,sides)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	Mar. 1994
	DESCRIPTION:	This routine initializes the SEGS linked list by
			scanning the LINEDEFS list and creating the
			appropriate SEGS entries.  A seg is created for each
			side a LINEDEF has.  It returns the number of SEGS
			created.
	MODIFIED:		Robert Fenske, Jr.	May  1995
			Added the option to ignore LINEDEFS that do not have
			any affect on the display seen by the player.  This
			idea came from a suggestion by Bernd Kreimeier.
******************************************************************************/
#if defined(ANSI_C)
local int nodes_segs_init(WAD_LINE *lines, long nlines, WAD_SIDE *sides)
#else
local int nodes_segs_init(lines,nlines,sides)
WAD_LINE *lines;
long nlines;
WAD_SIDE *sides;
#endif
{
  WAD_VERT *vf, *vt;
  SEGS_INFO *sinfo;
  LIMIT_BOX *sectbox;
  int nsegs;					/* # SEGS created */
  int i, l;

  nsegs = 0;
  ninfo.sinfo = sinfo = blockmem(SEGS_INFO,1);
  sinfo->prev = NULL;
  for (l = 0; l < nlines; l++) {		/* scan all lines */
    if (!ninfo.use_all_segs &&
        two_sided(l) &&				/* two-sided lines whose    */
        sides[lines[l].rsidndx].sectndx ==	/* sides reference the same */
        sides[lines[l].lsidndx].sectndx &&	/* sector and have no       */
        strcmp(sides[lines[l].rsidndx].nwall,"-") == 0 &&/* textures can be */
        strcmp(sides[lines[l].lsidndx].nwall,"-") == 0)/* ignored           */
      continue;
    for (i = 0; i <= two_sided(l); i++) {
      sinfo->seg.fndx = i==0 ? lines[l].fndx : lines[l].tndx;
      sinfo->seg.tndx = i==0 ? lines[l].tndx : lines[l].fndx;
      vf = &ninfo.sverts[sinfo->seg.fndx], vt = &ninfo.sverts[sinfo->seg.tndx];
      if (vf->x == vt->x && vf->y == vt->y)	/* ignore zero-length lines */
        continue;
      sinfo->seg.angle = (bams)(vt->y==vf->y && vt->x<vf->x ? BAMS180 :
                                atan2((double)(vt->y-vf->y),
                                      (double)(vt->x-vf->x))*rad_to_bams+
                                0.5*sgn(vt->y-vf->y));
      sinfo->seg.sndx = i;			/* 0=right side,1=left side */
      sinfo->seg.lndx = l;
      sinfo->seg.loffset = 0;
      if (i == 0) sinfo->sector = sides[lines[l].rsidndx].sectndx;
      else        sinfo->sector = sides[lines[l].lsidndx].sectndx;
      sectbox = &ninfo.sectbox[sinfo->sector];
      update_limits(sectbox,vf,vt);
      nsegs++;
      sinfo->next = blockmem(SEGS_INFO,1);	/* set up for next one */
      sinfo->next->prev = sinfo;
      sinfo = sinfo->next;
    }
  }
  sinfo->prev->next = NULL;
  blockfree(sinfo);				/* don't need this one */
  return nsegs;					/* return # created SEGS */
}


/******************************************************************************
	ROUTINE:	nodes_split_seg(pseg,seg,right,left)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	Mar. 1994
	DESCRIPTION:	This routine splits the input segment into left and
			right segments based on the input partition line.  The
			intersection of the partition line (based on the input
			"from" and "to" coordinates) with the segment is found
			so that a new vertex can be added to the vertex list.
			The offset field of the left and right segments are
			computed from the distance to the new vertex along the
			segment.
******************************************************************************/
#if defined(ANSI_C)
local void nodes_split_seg(SEGS_INFO *pseg, SEGS_INFO *seg,
                           SEGS_INFO **right,
                           SEGS_INFO **left)
#else
local void nodes_split_seg(pseg,seg,right,left)
SEGS_INFO *pseg, *seg;
SEGS_INFO **right, **left;
#endif
{
  WAD_VERT *pf = &ninfo.sverts[pseg->seg.fndx],
           *pt = &ninfo.sverts[pseg->seg.tndx],
           *vf = &ninfo.sverts[seg->seg.fndx],
           *vt = &ninfo.sverts[seg->seg.tndx];
  long Ap = -((long)pt->y - pf->y),		/* partition line is */
       Bp = (long)pt->x - pf->x,		/* Ax + By + C = 0   */
       Cp = (long)pt->y*pf->x - (long)pt->x*pf->y;
  long As = -((long)vt->y - vf->y),		/* SEG line is     */
       Bs = (long)vt->x - vf->x,		/* Ax + By + C = 0 */
       Cs = (long)vt->y*vf->x - (long)vt->x*vf->y;
  double x, y;					/* intersection coords */
  WAD_VERT iv;					/* intersection vertex */
  int v;					/* intersection vertex index */

  *right = blockmem(SEGS_INFO,1);		/* new right side SEG */
  (*right)->seg = seg->seg;
  (*right)->sector = seg->sector;
  (*right)->next = NULL;
  *left = blockmem(SEGS_INFO,1);		/* new left side SEG */
  (*left)->seg = seg->seg;
  (*left)->sector = seg->sector;
  (*left)->next = NULL;
  x =  ((double)Bs*Cp - (double)Bp*Cs)/((double)Bp*As - (double)Bs*Ap);
  y = -((double)As*Cp - (double)Ap*Cs)/((double)Bp*As - (double)Bs*Ap);
  iv.x = x + sgn(x)*0.5;
  iv.y = y + sgn(y)*0.5;
  for (v = ninfo.nsverts-1; v >= ninfo.nverts; v--)/* check if vert already */
    if (ninfo.sverts[v].x == iv.x &&		/* used (will happen if 2-  */
        ninfo.sverts[v].y == iv.y) break;	/* sided LINEDEF split)     */
  if (v < ninfo.nverts)
    ninfo.sverts[v = ninfo.nsverts++] = iv;	/* add new vertex to list */
  if (seg->seg.sndx == 0) {			/* this is a right side SEG */
    if (Ap*vf->x + Bp*vf->y + Cp < 0) {
      (*right)->seg.tndx = v;
      (*left)->seg.fndx = v;
      (*left)->seg.loffset = seg->seg.loffset + sqrt((double)vdist2(*vf,iv));
    }else {
      (*right)->seg.fndx = v;
      (*right)->seg.loffset = seg->seg.loffset + sqrt((double)vdist2(*vf,iv));
      (*left)->seg.tndx = v;
    }
  }else {					/* this is a left side SEG */
    if (Ap*vt->x + Bp*vt->y + Cp < 0) {
      (*right)->seg.fndx = v;
      (*right)->seg.loffset = seg->seg.loffset + sqrt((double)vdist2(*vt,iv));
      (*left)->seg.tndx = v;
    }else {
      (*right)->seg.tndx = v;
      (*left)->seg.fndx = v;
      (*left)->seg.loffset = seg->seg.loffset + sqrt((double)vdist2(*vt,iv));
    }
  }
}


/******************************************************************************
	ROUTINE:	nodes_insert_seg(seglist,seg,preinsert)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	Mar. 1994
	DESCRIPTION:	This routine inserts the input SEG into the SEGS list
			either before or after the SEG that seglist references,
			depending on the preinsert flag.
******************************************************************************/
#if defined(ANSI_C)
local void nodes_insert_seg(SEGS_INFO *seglist, SEGS_INFO *seg, int preinsert)
#else
local void nodes_insert_seg(seglist,seg,preinsert)
SEGS_INFO *seglist, *seg;
int preinsert;
#endif
{
  if (preinsert) {				/* insert before */
    seg->prev = seglist->prev;
    seg->next = seglist;
  }else {					/* insert after */
    seg->prev = seglist;
    seg->next = seglist->next;
  }
  if (seg->prev != NULL) seg->prev->next = seg;
  if (seg->next != NULL) seg->next->prev = seg;
}


/******************************************************************************
	ROUTINE:	nodes_segs_bounds(node)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	Mar. 1994
	DESCRIPTION:	This routine scans all the SEGS contained in the input
			node to find the minimum and maximum coordinates for
			the node boundaries.
******************************************************************************/
#if defined(ANSI_C)
local void nodes_segs_bounds(NODE_TREE *node)
#else
local void nodes_segs_bounds(node)
NODE_TREE *node;
#endif
{
  SEGS_INFO *sinfo;
  WAD_VERT *vf, *vt;
  int s;

  node->lim.xmin = node->lim.ymin = MAX_SHORT;
  node->lim.xmax = node->lim.ymax = -node->lim.xmin;
  for (sinfo = node->segs, s = 0; s < node->nsegs; s++) {
    vf = &ninfo.sverts[sinfo->seg.fndx], vt = &ninfo.sverts[sinfo->seg.tndx];
    update_limits(&node->lim,vf,vt);
    sinfo = sinfo->next;
  }
}


/******************************************************************************
	ROUTINE:	nodes_decide_side(pseg,seg)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	Mar. 1994
	DESCRIPTION:	This routine returns whether the input segment is
			on the right side, left side, or both sides of the
			input partition line.  This is done by determining
			on which side of the partition line each vertex of
			the seg lies.  Given that the partition line is
			Ax + By + C = 0 and a vertex is (Vx,Vy), the
			intersection of the partition line and the shortest-
			distance line from the vertex to the partition line
			is given by
					Xi = Vx - b * d
					Yi = Vy - a * d
			where a = B/(A^2+B^2)^.5, b = A/(A^2+B^2)^.5,
			c = C/(A^2+B^2)^.5, and d = a*Vx + b*Vy + c.  Since
			the directional information can be obtained from
			Xi - Vx = Vx - b*d - Vx = -b*d and
			Yi - Vx = Vy - a*d - Vy = -a*d, only d is needed to
			determine which side the vertex lies on.  Thus only
			the sign (-1, 0, or +1) of d = (A*Vx + B*Vy + C) /
			(A^2 + B^2)^.5 needs to be considered.  This simple
			matrix can be used to determine the side:
				"from"       "to" vertex
				vertex	left	on	right

				left	left	left	both
				on	left	?	right
				right	both	right	right
			For the ? case, the side information in the seg must
			be used to determine the "sidedness".  Right is denoted
			with 0, left denoted by 1, and both denoted by -1.
			A, B, C, and d are calculated only when required to
			enhance the speed of the routine.
******************************************************************************/
#if defined(ANSI_C)
local int nodes_decide_side(WAD_SEGS *pseg, WAD_SEGS *seg)
#else
local int nodes_decide_side(pseg,seg)
WAD_SEGS *pseg, *seg;
#endif
{
  static int rightleft[3][3] = { {  1,  1, -1 },
                                 {  1, -2,  0 },
                                 { -1,  0,  0 } };
  static WAD_VERT *lpf = NULL, *lpt = NULL;	/* last partition line verts */
  static long A, B, C;				/* describes partition line */
  static long pd;
  WAD_VERT *pf = &ninfo.sverts[pseg->fndx],	/* partition line vertices */
           *pt = &ninfo.sverts[pseg->tndx],
           *vf = &ninfo.sverts[seg->fndx],	/* segment vertices */
           *vt = &ninfo.sverts[seg->tndx];
  long pfd, ptd;
  int sideflag;					/* which side SEG is on */
  int fside, tside;				/* "from"/"to" vertex side */

  if (lpf != pf || lpt != pt) {			/* update A,B,C if have to */
    A = -((long)pt->y - pf->y);			/* partition line is */
    B = (long)pt->x - pf->x;			/* Ax + By + C = 0   */
    C = (long)pt->y*pf->x - (long)pt->x*pf->y;
    pd = (long)(sqrt((double)A*A+(double)B*B)/M_SQRT2+0.5);
    lpf = pf;					/* save new vertices */
    lpt = pt;
  }
  pfd = A*vf->x + B*vf->y + C;
  fside = (pfd>=0)-(pfd<=-0);			/* "from" vertex side */
  ptd = A*vt->x + B*vt->y + C;
  tside = (ptd>=0)-(ptd<=-0);			/* "to" vertex side */
  sideflag = rightleft[1-fside][1-tside];
  if (sideflag == -1) {				/* test again for "both" */
    fside = (pfd>pd)-(pfd<-pd);			/* "from" vertex side */
    tside = (ptd>pd)-(ptd<-pd);			/* "to" vertex side */
    sideflag = rightleft[1-fside][1-tside];
  }
  if (sideflag == -2)				/* need to determine based */
    sideflag = pseg->angle != seg->angle;	/* on direction            */
  return sideflag;
}


/******************************************************************************
	ROUTINE:	nodes_sector_side(pseg,sector)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	May  1995
	DESCRIPTION:	This routine returns whether the input sector is
			strictly on the right side, left side, or both sides
			of the input partition line.  This is done by
			determining on which side of the partition line each
			vertex of the rectangle formed by the sector's limits
			lies.  See nodes_decide_side() for the method used to
			determine the "sidedness" of a vertex.  All four
			vertices must be on the same side and not on the
			partition line (hence my term "strictly" on the same
			side) for the sector to be considered on the right or
			on the left side.  Otherwise it is considered to be
			on both sides.
******************************************************************************/
#if defined(ANSI_C)
local int nodes_sector_side(WAD_SEGS *pseg, int sector)
#else
local int nodes_sector_side(pseg,sector)
WAD_SEGS *pseg;
int sector;
#endif
{
  static WAD_VERT *lpf = NULL, *lpt = NULL;	/* last partition line verts */
  static long A, B, C;				/* describes partition line */
  WAD_VERT *pf = &ninfo.sverts[pseg->fndx],	/* partition line vertices */
           *pt = &ninfo.sverts[pseg->tndx];
  long pfd, ptd;
  int sideflag;
  int fsidel, tsidel, fsideh = 0, tsideh = 0;	/* "from"/"to" vertex side */
  LIMIT_BOX *limits = &ninfo.sectbox[sector];

  if (lpf != pf || lpt != pt) {			/* update A,B,C if have to */
    A = -((long)pt->y - pf->y);			/* partition line is */
    B = (long)pt->x - pf->x;			/* Ax + By + C = 0   */
    C = (long)pt->y*pf->x - (long)pt->x*pf->y;
    lpf = pf;					/* save new vertices */
    lpt = pt;
  }
  pfd = A*limits->xmin + B*limits->ymin + C;
  fsidel = (pfd>=0)-(pfd<=-0);			/* "from" vertex side */
  ptd = A*limits->xmax + B*limits->ymin + C;
  tsidel = (ptd>=0)-(ptd<=-0);			/* "to" vertex side */
  if (fsidel == tsidel) {
    pfd = A*limits->xmin + B*limits->ymax + C;
    fsideh = (pfd>=0)-(pfd<=-0);		/* "from" vertex side */
    if (tsidel == fsideh) {
      ptd = A*limits->xmax + B*limits->ymax + C;
      tsideh = (ptd>=0)-(ptd<=-0);		/* "to" vertex side */
    }
  }
  sideflag = fsidel+tsidel+fsideh+tsideh;
  return sideflag == -4 ? 0 :			/* it's on the right */
         sideflag ==  4 ? 1 : -1;		/* it's on left, or both */
}


/******************************************************************************
	ROUTINE:	nodes_partition_line(node)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	Mar. 1994
	DESCRIPTION:	This routine searches for a suitable SEG to use as a
			partition line (which will divide the input node).
			Suitable is taken to mean the SEG that most equalizes
			the number of SEGS on each side of the partition line
			and mimimizes the number of SEG splits that need to be
			done.  Ideally, the partition line should also do
			this for all the node's children as well, but the
			calculations would be far too time consuming; therefore
			only the input node is considered.  The most suitable
			partition is found by selecting the SEG that minimizes
			the expression max(Rc,Lc) + A*Sc, where Rc, Lc, Sc
			are the right side, left side, and split counts
			respectively and A is an empirical constant.  This
			expression will be minimized when the left and right
			counts are equal and the number of splits is low.
			This routine sets the node's partition line to be the
			SEG that creates the best (minimum) value.  This
			process is an N^2 operation, so a number of short
			cuts are taken to reduce the number of SEGS actually
			checked.  This effectively means reducing the number
			of times nodes_decide_side() and nodes_sector_side()
			are called.
			These short cuts are as follows:
				o The number of candidate partition lines is
				  determined by
					# SEGS		# partition checks
					0 < # < 34		all
					34 <= # < 300		#/21
					300 <= #		#/57
				o Only one of the two SEGS spawned (which
				  are always consecutive in the SEGS list)
				  by a LINEDEF is checked
				o If the SEG is in a sector that is entirely
				  on one side of the current partition line,
				  it does not have to be checked for which
				  side it is on
				o If the current # splits is > 21 or is more
				  than four higher than the # splits associated
				  with the best partition line so far, there
				  is no need to consider the current partition
				  any further since it is not a good candidate
			If bgm is still -1 after all SEGS are processed
			(meaning no suitable partition line was found), then
			a partition line choice is forced by choosing the
			first SEG as the partition line.
******************************************************************************/
#if defined(ANSI_C)
local void nodes_partition_line(NODE_TREE *node)
#else
local void nodes_partition_line(node)
NODE_TREE *node;
#endif
{
  static int ncnt = 0;				/* for use by sectflag */
  long bgm = MAX_LONG-1;			/* max geometric mean count */
  int bsplits = 2*node->nsegs;			/* associated best # splits */
  long gm = bgm;				/* geometric mean count */
  int rcnt, lcnt, splits;			/* right, left, split count */
  int sideflag;					/* which side SEG is on */
  long *sectflag;				/* which side sector is on */
  SEGS_INFO *sinfo, *iseg;
  int s, i;

  sinfo = node->segs;
  for (s = 0; s < node->nsegs; s++) {		/* scan SEGS in node */
    if ((s < 32 &&				/* 32,21 & 300,57 empirical */
        (s == 0 || sinfo->seg.lndx != sinfo->prev->seg.lndx)) ||
        s % (s < 300 ? 21 : 57) == 0) {
      ncnt += 10;
      sideflag = rcnt = lcnt = splits = 0;
      iseg = node->segs;
      for (i = 0; i < node->nsegs; i++) {	/* get SEGS pos w.r.t. pseg */
        sectflag = &ninfo.sectuse[iseg->sector];
        if (i == s)				/* already know this will */
          sideflag = 0;				/* be on right side       */
        else if (sinfo->seg.lndx == iseg->seg.lndx)/* and this will be on */
          sideflag = 1;				/* left side              */
        else if (*sectflag >= ncnt)		/* know which side because */
          sideflag = *sectflag - ncnt;		/* know about sector       */
        else if (i == 0 ||			/* must compute which side */
                 iseg->seg.lndx != iseg->prev->seg.lndx) {
          if (node->nsegs > 23 && *sectflag < ncnt-1) {
            *sectflag = nodes_sector_side(&sinfo->seg,iseg->sector)+ncnt;
            sideflag = *sectflag - ncnt;
          }
          if (*sectflag < ncnt)
            sideflag = nodes_decide_side(&sinfo->seg,&iseg->seg);
        }
        if (sideflag == 0)       rcnt++;	/* count SEGS on both sides */
        else if (sideflag == 1)  lcnt++;	/* of the partition line    */
        else if (sideflag == -1) splits++;	/* count # splits */
        gm = max(rcnt,lcnt)+25*splits/2;
      	if (splits > 25 ||			/* already too many splits */
            splits > bsplits+4) gm = bgm+1;
        if (gm > bgm) break;			/* too poor so can stop */
        iseg = iseg->next;
      }
      if (rcnt != node->nsegs) {
        if (gm < bgm) {
          bgm = gm, bsplits = splits;
          node->pseg = sinfo;			/* best partition so far */
        }
      }
    }
    sinfo = sinfo->next;
  }
  if (bgm == MAX_LONG-1) {			/* need to force a partition */
    node->pseg = node->segs;			/* line: use first SEG and   */
    node->pseg->sector = -node->pseg->sector-1;	/* flag as artificial        */
  }
}


/******************************************************************************
	ROUTINE:	nodes_divide_node(node,left,right)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	Mar. 1994
	DESCRIPTION:	This routine divides the input node into left and
			right nodes based on the partition line of the input
			node.  This essentially means that the list of SEGS
			associated with the input node must be divided into
			left and right collections of SEGS.  This division is
			done by moving all the left side SEGS to the end of
			the SEGS list, leaving all right side SEGS at the front
			of the list.  Those SEGS that need to be split have
			their new right side SEG take the position of the old
			SEG and their new left side SEG put at the end of the
			list.  Once the list is divided, the right and left
			nodes are set the appropriate section of the list and
			their bounds are computed.
******************************************************************************/
#if defined(ANSI_C)
local void nodes_divide_node(NODE_TREE *node, NODE_TREE *right,
                             NODE_TREE *left)
#else
local void nodes_divide_node(node,right,left)
NODE_TREE *node, *right, *left;
#endif
{
  int sideflag;					/* which side SEG is on */
  SEGS_INFO *next, *end;
  SEGS_INFO *lseg, *rseg;			/* for splitting seg in two */
  SEGS_INFO *sinfo;
  int i;

  sinfo = node->segs;
  right->nsegs = left->nsegs = 0;
  for (end = sinfo, i = 0; i < node->nsegs-1; i++) end = end->next;
  for (i = 0; i < node->nsegs; i++) {		/* scan all node's SEGS */
    if (node->pseg == sinfo) {			/* either always on right */
      sideflag = node->pseg->sector >= 0 ? 0 : 1;/* side or forced to be  */
      if (node->pseg->sector < 0)		/* on left side           */
        node->pseg->sector = -node->pseg->sector-1;/* now put sect # back */
    }else					/* have to pick side anyway */
      sideflag = nodes_decide_side(&node->pseg->seg,&sinfo->seg);
    next = sinfo->next;
    switch (sideflag) {
      case   0: right->nsegs++;			/* just add to right's total */
      bcase  1: if (sinfo->prev != NULL)	/* move to end of list */
                  sinfo->prev->next = sinfo->next;
                if (sinfo->next != NULL)
                  sinfo->next->prev = sinfo->prev;
                if (end == sinfo) end = sinfo->prev;
                if (node->segs == sinfo) node->segs = sinfo->next;
                nodes_insert_seg(end,sinfo,FALSE);
                end = sinfo;
                left->nsegs++;
      bcase -1: nodes_split_seg(node->pseg,sinfo,&rseg,&lseg);
                sinfo->seg = rseg->seg;		/* make this the right SEG */
                right->nsegs++;
                blockfree(rseg);		/* don't need this now */
                nodes_insert_seg(end,lseg,FALSE);/* add left SEG to end */
                end = lseg;
                left->nsegs++;
                ninfo.nsegs++;			/* one more for total */
    }
    sinfo = next;
  }
  for (sinfo = node->segs, i = 0; i < right->nsegs; i++)
    sinfo = sinfo->next;
  right->segs = node->segs;			/* SEGS on right side of   */
  nodes_segs_bounds(right);			/* partition line are here */
  left->segs = sinfo;				/* SEGS on left side of    */
  nodes_segs_bounds(left);			/* partition line are here */
}


/******************************************************************************
	ROUTINE:	nodes_subsector_test(node)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	Mar. 1994
	DESCRIPTION:	This routine checks whether the input node can be
			an SSECTOR or not.  To be a subsector, the SEGS in
			the node must describe a convex polygon (no interior
			angles > 180 degrees).  This is equivalent to having
			all the SEGS on the right side of each other.  If the
			unique sector flag is set, then all the SEGS in the
			node must refer to the same SECTOR before it will be
			considered an SSECTOR.
******************************************************************************/
#if defined(ANSI_C)
local int nodes_subsector_test(NODE_TREE *node)
#else
local int nodes_subsector_test(node)
NODE_TREE *node;
#endif
{
  int subsector = TRUE;
  SEGS_INFO *sinfo, *iseg;
  int s, i;

  sinfo = node->segs;
  for (s = 0; s < node->nsegs; s++) {		/* scan all SEGS */
    for (iseg = node->segs, i = 0; i < node->nsegs; i++) {
      if (i != s) {
        if ((!ninfo.sectmult[sinfo->sector] && sinfo->sector!=iseg->sector) ||
            nodes_decide_side(&sinfo->seg,&iseg->seg) != 0) {
          subsector = FALSE;			/* interior angle > 180 degs */
          goto done;				/* so not an SSECTOR         */
        }
      }
      iseg = iseg->next;
    }
    sinfo = sinfo->next;
  }
done:
  return subsector;
}


/******************************************************************************
	ROUTINE:	nodes_partition_node(node)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	Mar. 1994
	DESCRIPTION:	This routine performs the binary space partitioning.
			It recursively divides (partitions) the input node
			until each leaf of the subtree defined by the input
			node is an SSECTOR.  A partition line is obtained and
			the left and right subtrees are allocated.  The left
			subtree is always checked first.  If it is not an
			SSECTOR, a recursive call is made to further divide
			the left subtree.  The same procedure is then performed
			on the right subtree.  The number of left and right
			children plus one for the current node is returned.
******************************************************************************/
#if defined(ANSI_C)
local int nodes_partition_node(NODE_TREE *node)
#else
local int nodes_partition_node(node)
NODE_TREE *node;
#endif
{
  int nl, nr;					/* # left, right nodes */
  NODE_TREE *left, *right;			/* left, right children */

  nodes_partition_line(node);			/* obtain partition line */
  node->right = tree_branch(right);
  node->left = tree_branch(left);
  nodes_divide_node(node,right,left);
  if (nodes_subsector_test(left)) {		/* found an SSECTOR */
    if (!ninfo.limit_io) printf("*\010\010");
    nl = 1;
  }else {					/* need further subdivision */
    if (!ninfo.limit_io) printf("L");
    nl = nodes_partition_node(left);
  }
  if (nodes_subsector_test(right)) {		/* found an SSECTOR */
    if (!ninfo.limit_io) printf("*\010\010");
    nr = 1;
  }else {					/* need further subdivision */
    if (!ninfo.limit_io) printf("R");
    nr = nodes_partition_node(right);
  }
  return nl + nr + 1;				/* # left + # right + this 1 */
}


/******************************************************************************
	ROUTINE:	nodes_place_node(nodes,nndx,sndx,nodetree)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	Mar. 1994
	DESCRIPTION:	This routine builds the NODES structure from the
			input node tree.  It traverses the node tree in a
			post-order fashion, left side first.  As the tree is
			scanned, the NODES, SSECTORS, and SEGS lists are filled
			in as appropriate.  The SSECTORS and SEGS lists are
			referenced through the ninfo block.  The node tree
			entries are deleted after they are used.  The node
			number (or index) is returned, unless an SSECTOR is
			found, then a -(SSECTOR number) is returned.
******************************************************************************/
#if defined(ANSI_C)
local int nodes_place_node(WAD_NODE *nodes, int *nndx, int *sndx,
                           NODE_TREE *nodetree)
#else
local int nodes_place_node(nodes,nndx,sndx,nodetree)
WAD_NODE *nodes;
int *nndx, *sndx;
NODE_TREE *nodetree;
#endif
{
  int nnum;					/* node number to return */
  int lnum, rnum;
  SEGS_INFO *sinfo, *next;
  WAD_NODE *node;
  int s;

  if (nodetree->left != NULL) {			/* traverse node subtree */
    int fndx = nodetree->pseg->seg.fndx;
    int tndx = nodetree->pseg->seg.tndx;
    lnum = nodes_place_node(nodes,nndx,sndx,nodetree->left);
    rnum = nodes_place_node(nodes,nndx,sndx,nodetree->right);
    node = &nodes[nnum = (*nndx)++];
    node->x = ninfo.sverts[fndx].x;		/* these 4 fields describe */
    node->y = ninfo.sverts[fndx].y;		/* the partition line      */
    node->xdel = ninfo.sverts[tndx].x - node->x;
    node->ydel = ninfo.sverts[tndx].y - node->y;
    node->lymax = nodetree->left->lim.ymax;
    node->lymin = nodetree->left->lim.ymin;
    node->lxmin = nodetree->left->lim.xmin;
    node->lxmax = nodetree->left->lim.xmax;
    if (lnum < 0) node->nndx[1] = 0x8000 | (-lnum-1);/* mark as SSECTOR */
    else          node->nndx[1] = lnum;		/* mark as NODE */
    blockfree(nodetree->left);			/* done with left subtree */
    node->rymax = nodetree->right->lim.ymax;
    node->rymin = nodetree->right->lim.ymin;
    node->rxmin = nodetree->right->lim.xmin;
    node->rxmax = nodetree->right->lim.xmax;
    if (rnum < 0) node->nndx[0] = 0x8000 | (-rnum-1);/* mark as SSECTOR */
    else          node->nndx[0] = rnum;		/* mark as NODE */
    blockfree(nodetree->right);			/* done with right subtree */
  }else {					/* SSECTOR (tree leaf) */
    ninfo.ssecs[*sndx].count = nodetree->nsegs;
    if (*sndx == 0) ninfo.ssecs[*sndx].sndx = 0;
    else            ninfo.ssecs[*sndx].sndx = ninfo.ssecs[*sndx-1].sndx+
                                              ninfo.ssecs[*sndx-1].count;
    sinfo = nodetree->segs;
    for (s = 0; s < nodetree->nsegs; s++) {	/* copy node's SEGS to full */
      ninfo.segs[ninfo.nsegs++] = sinfo->seg;	/* SEGS array               */
      next = sinfo->next;
      blockfree(sinfo);
      sinfo = next;
    }
    nnum = -++(*sndx);				/* mark as leaf */
  }
  return nnum;					/* ret node # or <0 if leaf */
}


/******************************************************************************
	ROUTINE:	nodes_make(nodes,nnodes,ssecs,nssecs,segs,nsegs,
			           verts,nverts,lines,nlines,sides,genflags)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	Mar. 1994
	DESCRIPTION:	This routine generates the NODES, SSECTORS, and SEGS
			sections.  It first finds the minimum and maximum x,y
			coordinates of the map to use in the root of the node
			tree.  Then the node tree root is created.  The
			necessary counters in the ninfo block are zeroed and
			the SEGS vertices list is allocated.  Then
			nodes_partition_node() is called to build the node
			tree.  Next, the NODES, SSECTORS, and SEGS lists are
			allocated based on the values calculated during the
			construction of the node tree.  The necessary counters
			are zeroed and nodes_place_node() is called to fill
			the NODES, SSECTORS, and SEGS lists with the correct
			information.  All the appropriate values are placed
			into the return variables and the number of computed
			node entries is returned.
******************************************************************************/
#if defined(ANSI_C)
long nodes_make(WAD_NODE **nodes, long *nnodes, WAD_SSECTOR **ssecs,
               long *nssecs, WAD_SEGS **segs, long *nsegs,
               WAD_VERT **verts, long *nverts, WAD_LINE **lines,
               long *nlines, WAD_SIDE **sides, char *genflags)
#else
long nodes_make(nodes,nnodes,ssecs,nssecs,segs,nsegs,verts,nverts,
                lines,nlines,sides,genflags)
WAD_NODE **nodes;
long *nnodes;
WAD_SSECTOR **ssecs;
long *nssecs;
WAD_SEGS **segs;
long *nsegs;
WAD_VERT **verts;
long *nverts;
WAD_LINE **lines;
long *nlines;
WAD_SIDE **sides;
char *genflags;
#endif
{
  NODE_TREE *nodetree;				/* BSP node tree */
  int i;

  ninfo.use_all_segs = strchr(genflags,'a') != NULL;
  ninfo.limit_io = strchr(genflags,'s') != NULL;
  ninfo.nlines = *nlines;			/* this many lines */
  ninfo.nsverts = -1;
  for (i = 0; i < ninfo.nlines; i++) {		/* find maximum used vertex */
    if (ninfo.nsverts < (*lines)[i].fndx) ninfo.nsverts = (*lines)[i].fndx;
    if (ninfo.nsverts < (*lines)[i].tndx) ninfo.nsverts = (*lines)[i].tndx;
  }
  ninfo.nverts = ++ninfo.nsverts;		/* this many lines' vertices */
  ninfo.sverts = blockmem(WAD_VERT,2*ninfo.nsverts);/* assume no more than   */
  for (i = 0; i < ninfo.nsverts; i++)		/* nsverts new verts created */
    ninfo.sverts[i] = (*verts)[i];
  ninfo.nsects = -1;
  for (i = 0; i < ninfo.nlines; i++) {		/* find max sector number */
    int sector = (*sides)[(*lines)[i].rsidndx].sectndx;
    if (ninfo.nsects < sector) ninfo.nsects = sector;
    if ((*lines)[i].lsidndx != -1) {
      sector = (*sides)[(*lines)[i].lsidndx].sectndx;
      if (ninfo.nsects < sector) ninfo.nsects = sector;
    }
  }
  ninfo.nsects++;				/* this is # SECTORS */
  ninfo.sectbox = blockmem(LIMIT_BOX,ninfo.nsects);
  for (i = 0; i < ninfo.nsects; i++) {		/* init sect bounding boxes */
    ninfo.sectbox[i].xmin =
    ninfo.sectbox[i].ymin = MAX_SHORT;
    ninfo.sectbox[i].xmax =
    ninfo.sectbox[i].ymax = -ninfo.sectbox[i].xmin;
  }
  ninfo.sectuse = blockmem(long,ninfo.nsects);
  ninfo.sectmult = blockmem(boolean,ninfo.nsects);
  if (strchr(genflags,'m') != NULL)
  {
    if (strchr(genflags,'(') == NULL)		/* all with multi sectors */
      for (i = 0; i < ninfo.nsects; i++) ninfo.sectmult[i] = TRUE;
    else {
      genflags = strchr(genflags,'(')+1;
      for (;;)					/* get specific sectors */
        if (1 == sscanf(genflags,"%d",&i) &&
            0 <= i && i < ninfo.nsects) {
          ninfo.sectmult[i] = TRUE;
          genflags += strspn(genflags,"0123456789");
          if (genflags != NULL) genflags++;
        }else
          break;
    }
  }
  ninfo.nsegs = nodes_segs_init(*lines,*nlines,*sides);/* init SEGS list */
  nodetree = tree_branch(nodetree);		/* node tree root */
  nodetree->nsegs = ninfo.nsegs;
  nodetree->segs = ninfo.sinfo;
  nodes_segs_bounds(nodetree);
  printf("%d used segs ==>%s",ninfo.nsegs,ninfo.limit_io?"":"  ");
  ninfo.nnodes = nodes_partition_node(nodetree)/2;/* BSP it */
  blockfree(ninfo.sectmult);
  blockfree(ninfo.sectuse);
  blockfree(ninfo.sectbox);
  ninfo.nodes = blockmem(WAD_NODE,ninfo.nnodes);
  ninfo.nssecs = ninfo.nnodes+1;		/* always 1 more SSECTOR */
  ninfo.ssecs = blockmem(WAD_SSECTOR,ninfo.nssecs);
  ninfo.segs = blockmem(WAD_SEGS,ninfo.nsegs);
  ninfo.nsegs = ninfo.nssecs = ninfo.nnodes = 0;
  (void)nodes_place_node(ninfo.nodes,&ninfo.nnodes,&ninfo.nssecs,nodetree);
  if (nodes != NULL)  *nodes = ninfo.nodes;
  if (nnodes != NULL) *nnodes = ninfo.nnodes;
  if (ssecs != NULL)  *ssecs = ninfo.ssecs;
  if (nssecs != NULL) *nssecs = ninfo.nssecs;
  if (segs != NULL)   *segs = ninfo.segs;
  if (nsegs != NULL)  *nsegs = ninfo.nsegs;
  if (verts != NULL)  *verts = ninfo.sverts;
  if (nverts != NULL) *nverts = ninfo.nsverts;
  blockfree(nodetree);				/* done with the node tree */
  return *nnodes;				/* return number of nodes */
}
