/******************************************************************************
	MODULE:		REJECT.C
	WRITTEN BY:	Robert Fenske, Jr. (rfenske@swri.edu)
	CREATED:	June 1994
	DESCRIPTION:	This module contains routines to generate the REJECT
			resource.  The REJECT resource is a two-dimensional
			square bit matrix.  Its size is determined by the
			number of sectors: (<# sectors>^2 + 7) / 8 bytes.  Each
			bit is a flag for a sector pair.  So REJECT[s1][s2] is
			zero then any creatures in sector s1 can attack
			anything in sector s2.  A one means no attack is
			allowed.  For most purposes this essesentially reflects
			whether sector s1 can see sector s2.  Which means that
			REJECT[s1][s2] will be the same as REJECT[s2][s1]
			and that REJECT[s1][s1] will always be zero.  But note
			that if REJECT[s1][s1] is one, then any creatures in
			sector s1 will mill about never attacking anything
			within the sector.  The philosophy of this module is
			that REJECT[s1][s2] should be zero if the sectors s1
			and s2 can see each other--this implies that creatures
			can attack from s1 into s2 (and vice versa).  If s1
			and s2 can't see each other then REJECT[s1][s2] should
			be once.  This module generates the REJECT array solely
			based on whether the sectors are in line of sight of
			each other.

			This module does not do any error checking on any
			memory allocation.  If any allocation ever fail, this
			module will bomb.
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "dmglobal.h"

#define rside_sect(l)	rinfo.sides[rinfo.lines[l].rsidndx].sectndx
#define lside_sect(l)	rinfo.sides[rinfo.lines[l].lsidndx].sectndx
#define two_sided(l)	(rinfo.lines[l].lsidndx != -1)

#define bit_no(s1,s2)	((s1)*rinfo.nsects+(s2))
#define clr_flag(a,s1,s2) ((a)[bit_no(s1,s2)/8] &= ~(1<<(bit_no(s1,s2)%8)))
#define set_flag(a,s1,s2) ((a)[bit_no(s1,s2)/8] |= (1<<(bit_no(s1,s2)%8)))
#define get_flag(a,s1,s2) ((a)[bit_no(s1,s2)/8] & (1<<(bit_no(s1,s2)%8)))

typedef struct REJECT_INFO {
	int nsects;				/* # sectors */
	WAD_LINE *lines;
	long nlines;
	WAD_SIDE *sides;
	WAD_VERT *verts;
	WAD_BLOCKMAP *blockmap;
	long *A, *B, *C;			/* lines equation constants */
	int ndiv;				/* # divisions tested / line */
} REJECT_INFO;

local REJECT_INFO rinfo;			/* REJECT information block */

local WAD_REJECT *Reject,			/* built REJECT block */
                  *Checked;			/* sector pairs checked blk */


/******************************************************************************
	ROUTINE:	reject_test_point(lndx,d,ndiv,tp)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	June 1994
	DESCRIPTION:	This routine calculates the input line's test point
			given the ratio d/ndiv (i.e., the dth division of ndiv
			divisions).  It calculates the point
				x = Vfx + d * (Vtx-Vfx) / nd
				y = Vfy + d * (Vty-Vfy) / nd
******************************************************************************/
#if defined(ANSI_C)
local void reject_test_point(int lndx, int d, int ndiv, WAD_VERT *tp)
#else
local void reject_test_point(lndx,d,ndiv,tp)
int lndx;
int d, ndiv;
WAD_VERT *tp;
#endif
{
  WAD_VERT *vf = &rinfo.verts[rinfo.lines[lndx].fndx],
           *vt = &rinfo.verts[rinfo.lines[lndx].tndx];

  tp->x = (long)vf->x + d*((long)vt->x-vf->x)/ndiv;
  tp->y = (long)vf->y + d*((long)vt->y-vf->y)/ndiv;
}


/******************************************************************************
	ROUTINE:	reject_block_los(vf,vt,lndx)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	June 1994
	DESCRIPTION:	This routine returns whether the input line lndx
			blocks the view of the point vt from the point vf.
			This is equivalent to determining if the lndx line
			intersects the segment defined by vf and vt.  The
			intersection point is not needed, only the knowledge
			that such an intersection exists.  For such an
			intersection to exist, the points vf and vt must lie on
			opposite sides of the lndx line, and the vertices of
			the lndx line must lie on opposite sides of the vf-vt
			segment.  Given that the line is Ax + By + C = 0 and
			a vertex is (Vx,Vy), the intersection of the line
			and the shortest-distance line from the vertex to the
			line is given by
				Xi = Vx - b * d
				Yi = Vy - a * d
			where a = B/(A^2+B^2)^.5, b = A/(A^2+B^2)^.5,
			c = C/(A^2+B^2)^.5, and d = a*Vx + b*Vy + c.  Since
			the directional information can be obtained from
			Xi - Vx = Vx - b*d - Vx = -b*d and
			Yi - Vx = Vy - a*d - Vy = -a*d, only d is needed to
			determine which side the vertex lies on.  Thus only
			the sign (-1, 0, or +1) of d = (A*Vx + B*Vx + C) /
			(A^2 + B^2)^.5 needs to be considered.  This simple
			matrix can be used to determine the side:
				"from"       "to" vertex
				vertex	left	on	right

				left	left	left	both
				on	left	?	right
				right	both	right	right
			For the ? case, the line's directional information must
			be used to determine the "sidedness".  Right is denoted
			with 0, left denoted by 1, and both denoted by -1.
			A, B, C, and d are calculated only when required to
			enhance the speed of the routine.  For the line of
			sight to be blocked the "both" case must occur for
			the lndx line with repect to the vf-vt segment and for
			the vf-vt segment with respect to the lndx line.
******************************************************************************/
#if defined(ANSI_C)
local int reject_block_los(WAD_VERT *vf, WAD_VERT *vt, int lndx)
#else
local int reject_block_los(vf,vt,lndx)
WAD_VERT *vf, *vt;
int lndx;
#endif
{
  static int rightleft[3][3] = { {  1,  1, -1 },
                                 {  1, -2,  0 },
                                 { -1,  0,  0 } };
  static WAD_VERT lvf, lvt;			/* last vertices */
  static long A, B, C;				/* describes los line */
  WAD_VERT *lf = &rinfo.verts[rinfo.lines[lndx].fndx],/* test line vertices */
            *lt = &rinfo.verts[rinfo.lines[lndx].tndx];
  long pfd, ptd;
  int fsv, tsv, fsl, tsl;			/* "from"/"to" vertex sides */

  if (lvf.x != vf->x || lvt.x != vt->x ||
      lvf.y != vf->y || lvt.y != vt->y) {	/* update A,B,C if have to */
    A = -((long)vt->y - vf->y);			/* partition line is */
    B = (long)vt->x - vf->x;			/* Ax + By + C = 0   */
    C = (long)vt->y*vf->x - (long)vt->x*vf->y;
    lvf = *vf;					/* save new vertices */
    lvt = *vt;
  }
  pfd = A*lf->x + B*lf->y + C;
  fsv = (pfd>=0)-(pfd<=-0);			/* "from" vertex side */
  ptd = A*lt->x + B*lt->y + C;
  tsv = (ptd>=0)-(ptd<=-0);			/* "to" vertex side */
  pfd = rinfo.A[lndx]*vf->x + rinfo.B[lndx]*vf->y + rinfo.C[lndx];
  fsl = (pfd>=0)-(pfd<=-0);			/* "from" vertex side */
  ptd = rinfo.A[lndx]*vt->x + rinfo.B[lndx]*vt->y + rinfo.C[lndx];
  tsl = (ptd>=0)-(ptd<=-0);			/* "to" vertex side */
  return rightleft[1-fsv][1-tsv]==-1 && rightleft[1-fsl][1-tsl]==-1;
}


/******************************************************************************
	ROUTINE:	reject_test_los(l1,l2)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	June 1994
	DESCRIPTION:	This routine tests the line of sight between line l1
			and line l2.  It does this this by checking if any
			one-sided line blocks the view of l2 from l1.  Two-
			sided lines are regarded as transparent in all cases.
			Each line is checked at a number of points, based on
			the ndiv field of rinfo.  Test points are created for
			l1 and l2 and the line of sight is checked between
			these points.  Sector floor and ceiling height are
			ignored.  The blockmap is used to determine the lines
			that potentially block the line of sight between the
			test points; this greatly reduces the number of lines
			that need to be checked.
******************************************************************************/
#if defined(ANSI_C)
local boolean reject_test_los(int l1, int l2)
#else
local boolean reject_test_los(l1,l2)
int l1, l2;
#endif
{
  boolean los = FALSE;				/* line-o'-sight flag */
  WAD_VERT v1, v2;				/* los test vertices */
  long scl = 81920L/(100+rinfo.blockmap[3]);	/* line following scaling */
  long size = 0x80*scl;				/* block size (map coords) */
  long xcc, xcl;
  long xf,yf, xt,yt;
  long xd, yd;					/* x direction, y direction */
  long m;					/* diagonal line slope */
  int i;
  int p = 0;					/* increment to next block */
  int c = 0;					/* # blocks to consider */
  unsigned short *ub;
  long b;					/* blockmap block # */
  int d1, d2;
  long l;

  for (d1 = 0; !los && d1 <= rinfo.ndiv; d1++) {/* do l1's test points */
    reject_test_point(l1,d1,rinfo.ndiv,&v1);
    xf = scl*((long)v1.x-(long)rinfo.blockmap[0]);
    yf = scl*((long)v1.y-(long)rinfo.blockmap[1]);
    for (d2 = 0; !los && d2 <= rinfo.ndiv; d2++) {/* do l2's test points */
      reject_test_point(l2,d2,rinfo.ndiv,&v2);
      los = TRUE;
      xt = scl*((long)v2.x-(long)rinfo.blockmap[0]);
      yt = scl*((long)v2.y-(long)rinfo.blockmap[1]);
      xd = sgn(xt-xf), yd = sgn(yt-yf);
      switch (2*(xf/size == xt/size) + (yf/size == yt/size)) {
       case  0: c=0,                      p=yd*rinfo.blockmap[2];/* diag ln */
       bcase 1: c=abs((int)(xt/size-xf/size))+1, p=xd;	/* horizontal line */
       bcase 2: c=abs((int)(yt/size-yf/size))+1, p=yd*rinfo.blockmap[2];/* */
       bcase 3: c=0+1,                    p=1;	/* start,end in same block */
      }
      b = xf/size + rinfo.blockmap[2]*(yf/size);/* start @ this block */
      for (i = 0; i < c; i++, b+=p) {		/* check special cases */
        ub = (unsigned short *)&rinfo.blockmap[4+b];
        for (l = *ub+1; rinfo.blockmap[l] != -1; l++)
          if (!two_sided(rinfo.blockmap[l]) &&	/* consider only 1-sided */
              reject_block_los(&v1,&v2,rinfo.blockmap[l])) {/* this one- */
            los = FALSE;			/* sided blocks view, so */
            goto next_point;			/* check no further      */
          }
      }
      if (c == 0) {				/* handle diagonal lines */
        m = scl*(yt-yf)/(xt-xf);		/* spanning > 1 block    */
        if (m == 0) m = sgn(yt-yf)*sgn(xt-xf);	/* force a min non-0 slope */
        xcl = xf;
        if (yd == -1) xcc = xf + scl*((yf/size)*size -    1 - yf)/m;
        else          xcc = xf + scl*((yf/size)*size + size - yf)/m;
        do {
          for (c = 0; c <= abs((int)(xcc/size - xcl/size)); c++, b+=xd) {
            ub = (unsigned short *)&rinfo.blockmap[4+b];
            for (l = *ub+1; rinfo.blockmap[l] != -1; l++)
              if (!two_sided(rinfo.blockmap[l]) &&/* consider only 1-sided */
                  reject_block_los(&v1,&v2,rinfo.blockmap[l])) {/* this one- */
                los = FALSE;			/* sided blocks view, so     */
                goto next_point;		/* check no further          */
              }
          }
          b += p-xd;
          xcl = xcc, xcc += yd*scl*size/m;
          if (xd*xcc > xd*xt) xcc = xt;		/* don't overrun endpoint */
        } while (xd*xcl < xd*xt);
      }
next_point: ;
    }
  }
  return los;
}


/******************************************************************************
	ROUTINE:	reject_mark_los(l1,l2)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	June 1994
	DESCRIPTION:	This routine checks the line of sight between lines l1
			and l2.  Since the Reject array is sector-based, the
			decision to test the line of sight is based on the
			state of the Reject and Checked arrays for the sectors
			asociated with the side(s) of the lines.  If any pair
			of sides are in the same sector, the Checked array
			is set and the Reject array is untouched since the
			sides are trivially in sight of each other.  Otherwise,
			if the pair is still flagged as not in line of sight
			or the pair has not been checked yet, the line of
			sight test is performed.  If the line of sight test
			fails, the Reject array is flagged as such.
			Reject[s1][s2] and Reject[s2][s1] are flagged at the
			same time since the same line of sight condition will
			exist for either sector.
******************************************************************************/
#if defined(ANSI_C)
local void reject_mark_los(int l1, int l2)
#else
local void reject_mark_los(l1,l2)
int l1, l2;
#endif
{
  boolean tested = FALSE;			/* whether los already tsted */
  boolean los = FALSE;				/* line o' sight flag */
  int s1, s2;					/* tested sectors */
  int i1, i2;

  for (i1 = 0; i1 <= 1; i1++) {			/* check both sides of l1 */
    s1 = i1==0 ? rside_sect(l1) : lside_sect(l1);
    for (i2 = 0; i2 <= 1; i2++) {		/* check both sides of l2 */
      s2 = i2==0 ? rside_sect(l2) : lside_sect(l2);
      if (s1 == s2)				/* just mark as checked */
        set_flag(Checked,s1,s2);		/* since an obvious los */
      else if (get_flag(Reject,s1,s2) != 0 ||	/* if not in line o' sight */
               get_flag(Checked,s1,s2) == 0) {	/* or unchecked, check it  */
        if (los || (!tested && (los = reject_test_los(l1,l2))!=FALSE)) {
          clr_flag(Reject,s1,s2),		/* in line o' sight */
          clr_flag(Reject,s2,s1);
        }else
          set_flag(Reject,s1,s2),		/* not in line o' sight */
          set_flag(Reject,s2,s1);
        set_flag(Checked,s1,s2);		/* now has been checked */
        set_flag(Checked,s2,s1);
        tested = TRUE;				/* los been tested for l1,l2 */
      }
    }
  }
}


/******************************************************************************
	ROUTINE:	reject_init(lines,nlines,sides,verts,blockmap,ndiv)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	June 1994
	DESCRIPTION:	This routine initializes the reject information block.
			It computes the lines equation constants (where a line
			is defined by Ax + By + C = 0) and finds the number of
			sectors.
******************************************************************************/
#if defined(ANSI_C)
local void reject_init(WAD_LINE *lines, long nlines, WAD_SIDE *sides,
                       WAD_VERT *verts, WAD_BLOCKMAP *blockmap, int ndiv)
#else
local void reject_init(lines,nlines,sides,verts,blockmap,ndiv)
WAD_LINE *lines;
long nlines;
WAD_SIDE *sides;
WAD_VERT *verts;
WAD_BLOCKMAP *blockmap;
int ndiv;
#endif
{
  WAD_VERT *vf, *vt;
  int l;

  rinfo.lines = lines;				/* assign these */
  rinfo.nlines = nlines;
  rinfo.sides = sides;
  rinfo.verts = verts;
  rinfo.blockmap = blockmap;
  rinfo.ndiv = ndiv;
  rinfo.A = blockmem(long,rinfo.nlines);
  rinfo.B = blockmem(long,rinfo.nlines);
  rinfo.C = blockmem(long,rinfo.nlines);
  for (l = 0; l < nlines; l++) {		/* compute lines equations */
    vf = &rinfo.verts[rinfo.lines[l].fndx];
    vt = &rinfo.verts[rinfo.lines[l].tndx];
    rinfo.A[l] = -((long)vt->y-vf->y);
    rinfo.B[l] = (long)vt->x-vf->x;
    rinfo.C[l] = (long)vt->y*vf->x - (long)vt->x*vf->y;
  }
  rinfo.nsects = 0;
  for (l = 0; l < nlines; l++)			/* find highest sector # */
    if (rinfo.nsects < rside_sect(l))
      rinfo.nsects = rside_sect(l);
    else if (two_sided(l) && rinfo.nsects < lside_sect(l))
      rinfo.nsects = lside_sect(l);
  rinfo.nsects++;				/* highest+1 --> # sectors */
}


/******************************************************************************
	ROUTINE:	reject_make(reject,genflags,except,lines,nlines,sides,
			            verts,blockmap)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	June 1994
	DESCRIPTION:	This routine generates the REJECT block.  If first
			intializes the information structure needed for the
			calculation routines.  The REJECT resource is allocated
			based on the computed number of sectors.  The
			allocation initializes the Reject array to all zeroes.
			A companion resource is allocated for flags to indicate
			sector pairs that have been checked at least once and
			is initialized as all unchecked.  If the depth in the
			genflags is zero, then no line-of-sight processing is
			done--the routine skips directly to the exception list.
			Then each sector pair that shares a line is marked as
			checked since the pair can trivially see each other.
			Then all possible unique two-sided line pairs  are
			checked for line of sight (i.e., after checking pair
			l1,l2 then pair l2,l1 doesn't need checking since the
			result will be the same).  Since a line-o'-sight line
			must pass through or start from a two-sided line for
			a given sector, only two-sided lines need to be
			checked.  Any sectors sharing a two-sided line are
			immediately marked as being in line of sight.  Note
			that any sectors with no two-sided lines will be
			implicitly flagged as in sight of every other sector
			since they won't be checked.  Once all the line are
			scanned, the exception list is processed.  This list
			is of the form "[!]<S>-<S>,..." where <S> is either
			a sector number, or an * meaning all sectors.  All
			sector pairs in the exception lists are marked with
			a one, or with a zero if the pair is marked with a
			leading exclamation point.
******************************************************************************/
#if defined(ANSI_C)
long reject_make(WAD_REJECT **reject, int genflags, char *except,
                 WAD_LINE *lines, long nlines, WAD_SIDE *sides,
                 WAD_VERT *verts, WAD_BLOCKMAP *blockmap)
#else
long reject_make(reject,genflags,except,lines,nlines,sides,verts,blockmap)
WAD_REJECT **reject;
int genflags;
char *except;
WAD_LINE *lines;
long nlines;
WAD_SIDE *sides;
WAD_VERT *verts;
WAD_BLOCKMAP *blockmap;
#endif
{
  long size;					/* # bytes in REJECT block */
  int s1, s2;
  int l1, l2;

	/* number of divisions used is 2; there is very little to
	   be gained by using more divisions, expecially as the
	   total computation time is on the order of the square of
	   the number of divisions; this number should be extracted
	   from the RG_REJDEPTH flag, but currently is not          */
  reject_init(lines,nlines,sides,verts,blockmap,2);
  size = ((long)rinfo.nsects*rinfo.nsects+7)/8;	/* # bytes in REJECT block */
  Reject = blockmem(WAD_REJECT,size);		/* this is REJECT block */
  Checked = blockmem(WAD_REJECT,size);
  if ((genflags & RG_REJDEPTH) &&		/* do if have non-0 depth & */
      strstr(except,"*-*") == NULL) {		/* except doesn't cover all */
    for (l1 = 0; l1 < rinfo.nlines; l1++)	/* first mark all sectors    */
      if (two_sided(l1))			/* sharing a line as checked */
        set_flag(Checked,rside_sect(l1),lside_sect(l1)),/* since obvious los */
        set_flag(Checked,lside_sect(l1),rside_sect(l1));
    for (l1 = 0; l1 < rinfo.nlines; l1++)	/* check all line pairs */
      if (two_sided(l1)) {
        if (!(genflags & RG_LIMIT_IO)) // || (rinfo.nlines-l1)%20 == 0)
          printf("%04d\010\010\010\010",(int)(rinfo.nlines-l1));
        for (l2 = l1+1; l2 < rinfo.nlines; l2++)
          if (two_sided(l2)) reject_mark_los(l1,l2);
      }
  }
  while (except != NULL) {			/* if have exception list */
    int s1l, s1h, s2l, s2h;			/* process it; leading !  */
    int not = (*except == '!');			/* inverts operation      */
    if (not) except++;
    if (2 == sscanf(except,"%d-%d",&s1,&s2))	/* specific sector pair */
      s1l = s1h = s1, s2l = s2h = s2;
    else if (2 == sscanf(except,"%d-%1[*]",&s1,(char *)&s2))/* sect against */
      s1l = s1h = s1, s2l = 0, s2h = rinfo.nsects-1;/* all others           */
    else if (1 == sscanf(except,"*-%d",&s2))	/* sector against all others */
      s1l = 0, s1h = rinfo.nsects-1, s2l = s2h = 0;
    else if (0 == strncmp(except,"*-*",3))	/* all sector pairs */
      s1l = 0, s1h = rinfo.nsects-1, s2l = 0, s2h = rinfo.nsects-1;
    else					/* illegal: force skip */
      s1l = s1h = s2l = s2h = -1;
    if (0 <= s1l && s1l < rinfo.nsects &&	/* must be valid range */
        0 <= s1h && s1h < rinfo.nsects &&
        0 <= s2l && s2l < rinfo.nsects &&
        0 <= s2h && s2h < rinfo.nsects)
    {
      for (s1 = s1l; s1 <= s1h; s1++)
        for (s2 = s2l; s2 <= s2h; s2++)
          if (not) clr_flag(Reject,s1,s2), clr_flag(Reject,s2,s1);
          else     set_flag(Reject,s1,s2), set_flag(Reject,s2,s1);
    }
    except = strchr(except,',');
    if (except != NULL) except++;
  }
  blockfree(rinfo.A);				/* done with these */
  blockfree(rinfo.B);
  blockfree(rinfo.C);
  blockfree(Checked);				/* done with this */
  *reject = Reject;
  return size;
}
