#include "idbsp.h"

typedef struct {
	int	x,
		y;
} bpoint_t;

typedef struct {
	int	xl,
		xh,
		yl,
		yh;
} bbox_t;

typedef struct {
	bpoint_t	p1,
			p2;
} bline_t;

typedef struct {
	int		numpoints;
	bbox_t		bounds;
	bpoint_t	*points;
} bchain_t;

typedef struct {
	int	x,
		y;
	int	dx,
		dy;
} bdivline_t;


/* [numsec][numsec] array */
byte		*connections;

int		numblines;
bline_t		*blines;

int		numsectors;
bbox_t		*secboxes;

int		numbchains;
bchain_t	*bchains;

void ClearBBox(bbox_t *box)
{
	box->xl = box->yl = INT_MAX;
	box->xh = box->yh = INT_MIN;
}

void AddToBBox(bbox_t *box, int x, int y)
{
	if (x < box->xl)
		box->xl = x;
	if (x > box->xh)
		box->xh = x;
	if (y < box->yl)
		box->yl = y;
	if (y > box->yh)
		box->yh = y;
}

/*
   ==================
   =
   = BPointOnSide
   =
   = Returns side 0 (front), 1 (back), or -1 (colinear)
   ==================
 */
int BPointOnSide(bpoint_t *pt, bdivline_t *l)
{
	int	dx,
		dy;
	int	left,
		right;

	if (!l->dx)
	{
		if (pt->x < l->x)
			return l->dy > 0;
		return l->dy < 0;
	}
	if (!l->dy)
	{
		if (pt->y < l->y)
			return l->dx < 0;
		return l->dx > 0;
	}

	dx = pt->x - l->x;
	dy = pt->y - l->y;

	left = l->dy * dx;
	right = dy * l->dx;

	if (right < left)
		return 0;	/* front side */
	return 1;		/* back side */
}

void DrawBBox(bbox_t *box)
{
/*
	PSmoveto(box->xl, box->yl);
	PSlineto(box->xh, box->yl);
	PSlineto(box->xh, box->yh);
	PSlineto(box->xl, box->yh);
	PSlineto(box->xl, box->yl);
	PSstroke();
	NXPing();
 */
}

void DrawDivline(bdivline_t *li)
{
/*
	PSmoveto(li->x, li->y);
	PSrlineto(li->dx, li->dy);
	PSstroke();
	NXPing();
 */
}

void DrawBChain(bchain_t *ch)
{
/*
	int      i;

	PSmoveto(ch->points->x, ch->points->y);
	for (i = 1; i < ch->numpoints; i++)
		PSlineto(ch->points[i].x, ch->points[i].y);
	PSstroke();
	NXPing();
 */
}


bdivline_t	ends[2],
		sides[2];
int		end0out,
		end1out,
		side0out,
		side1out;
bbox_t		sweptarea;

/*
   ====================
   =
   = DoesChainBlock
   =
   ====================
 */
boolean DoesChainBlock(bchain_t *chain)
{
/*
   if a solid line can be walked from one side to the other without going out
   an end, the path is blocked
 */
	bpoint_t	*pt;
	int		side,
			startside;
	int		p;

	/* don't check if bounds don't intersect */
	if (sweptarea.xl > chain->bounds.xh ||
	    sweptarea.xh < chain->bounds.xl ||
	    sweptarea.yl > chain->bounds.yh ||
	    sweptarea.yh < chain->bounds.yl)
		return false;

	startside = -1;	/* not started yet */

	for (p = 0, pt = chain->points; p < chain->numpoints; p++, pt++)
	{
		/* find side for pt */
#if 0
		if (p > 0)
		{
			PSmoveto((pt - 1)->x, (pt - 1)->y);
			PSlineto(pt->x, pt->y);
			PSstroke();
			NXPing();
		}
#endif

		if (BPointOnSide(pt, &ends[0]) == end0out)
		{
			startside = -1;	/* off end */
			continue;
		}
		if (BPointOnSide(pt, &ends[1]) == end1out)
		{
			startside = -1;	/* off end */
			continue;
		}
		if (BPointOnSide(pt, &sides[0]) == side0out)
			side = 0;
		else if (BPointOnSide(pt, &sides[1]) == side1out)
			side = 1;
		else
			continue;	/* in middle */

		/* point is on one side or the other */
		if (startside == -1 || startside == side)
		{
			startside = side;
			continue;
		}

		/* opposite of startside */
		return true;	/* totally crossed area */
	}

	return false;
}

/*
   ====================
   =
   = BuildConnections
   =
   ====================
 */
enum {
	si_north, si_east, si_south, si_west
};

void BuildConnections(void)
{
	int		blockcount,
			passcount;
	int		i,
			j,
			k,
			s,
			bn;
	int		x = 0,
			y = 0;
	bbox_t		*bbox[2];
	int		walls[4];
	bpoint_t	points[2][2];

	/* look for obscured sectors */
	blockcount = passcount = 0;
	bbox[0] = secboxes;
	for (i = 0; i < numsectors - 1; i++, bbox[0]++)
	{
		if (!qflag)
			printf("connection #%d of %d\r", i, numsectors - 1);
		bbox[1] = bbox[0] + 1;
		if (bbox[0]->xh - bbox[0]->xl < 64 ||
		    bbox[0]->yh - bbox[0]->yl < 64)
		{ /* don't bother with small sectors (stairs, doorways, etc) */
			continue;
		}

		for (j = i + 1; j < numsectors; j++, bbox[1]++)
		{
			if (bbox[1]->xh - bbox[1]->xl < 64 ||
			    bbox[1]->yh - bbox[1]->yl < 64)
			{ /* don't bother with small sectors (stairs, */
			  /* doorways, etc) */
				continue;
			}
			if (bbox[1]->xl <= bbox[0]->xh &&
			    bbox[1]->xh >= bbox[0]->xl &&
			    bbox[1]->yl <= bbox[0]->yh &&
			    bbox[1]->yh >= bbox[0]->yl)
			{	/* touching sectors are never blocked */
				passcount++;
				continue;
			}

			sweptarea.xl = bbox[0]->xl < bbox[1]->xl ?
				bbox[0]->xl : bbox[1]->xl;
			sweptarea.xh = bbox[0]->xh > bbox[1]->xh ?
				bbox[0]->xh : bbox[1]->xh;
			sweptarea.yl = bbox[0]->yl < bbox[1]->yl ?
				bbox[0]->yl : bbox[1]->yl;
			sweptarea.yh = bbox[0]->yh > bbox[1]->yh ?
				bbox[0]->yh : bbox[1]->yh;

			/* calculate the swept area between the sectors */
			for (bn = 0; bn < 2; bn++)
			{
				memset(walls, 0, sizeof(walls));
				if (bbox[bn]->xl <= bbox[!bn]->xl)
					walls[si_west] = 1;
				if (bbox[bn]->xh >= bbox[!bn]->xh)
					walls[si_east] = 1;
				if (bbox[bn]->yl <= bbox[!bn]->yl)
					walls[si_south] = 1;
				if (bbox[bn]->yh >= bbox[!bn]->yh)
					walls[si_north] = 1;

				for (s = 0; s < 5; s++)
				{
					switch (s & 3)
					{
						case si_north:
						x = bbox[bn]->xl;
						y = bbox[bn]->yh;
						break;

						case si_east:
						x = bbox[bn]->xh;
						y = bbox[bn]->yh;
						break;

						case si_south:
						x = bbox[bn]->xh;
						y = bbox[bn]->yl;
						break;

						case si_west:
						x = bbox[bn]->xl;
						y = bbox[bn]->yl;
						break;
					}
					if (!walls[(s - 1) & 3] && walls[s & 3])
					{
						points[bn][0].x = x;
						points[bn][0].y = y;
					}
					if (walls[(s - 1) & 3] && !walls[s & 3])
					{
						points[bn][1].x = x;
						points[bn][1].y = y;
					}
				}

				ends[bn].x = points[bn][0].x;
				ends[bn].y = points[bn][0].y;
				ends[bn].dx = points[bn][1].x - points[bn][0].x;
				ends[bn].dy = points[bn][1].y - points[bn][0].y;
			}

			sides[0].x = points[0][0].x;
			sides[0].y = points[0][0].y;
			sides[0].dx = points[1][1].x - points[0][0].x;
			sides[0].dy = points[1][1].y - points[0][0].y;

			sides[1].x = points[0][1].x;
			sides[1].y = points[0][1].y;
			sides[1].dx = points[1][0].x - points[0][1].x;
			sides[1].dy = points[1][0].y - points[0][1].y;

			end0out = !BPointOnSide(&points[1][0], &ends[0]);
			end1out = !BPointOnSide(&points[0][0], &ends[1]);
			side0out = !BPointOnSide(&points[0][1], &sides[0]);
			side1out = !BPointOnSide(&points[0][0], &sides[1]);

			/* look for a line change that covers the swept area */
			for (k = 0; k < numbchains; k++)
			{
				if (!DoesChainBlock(&bchains[k]))
					continue;
				blockcount++;
				connections[i * numsectors + j] =
					connections[j * numsectors + i] = 1;
/*
				if (draw)
				{
					EraseWindow();
					DrawBBox(bbox[0]);
					DrawBBox(bbox[1]);
					DrawDivline(&ends[0]);
					DrawDivline(&ends[1]);
					DrawDivline(&sides[0]);
					DrawDivline(&sides[1]);
					DrawBChain(&bchains[k]);
				}
 */
				goto blocked;
			}

			/* nothing definately blocked the path */
			passcount++;
blocked:
			; /* some brain dead compiler needs a statement here */
		}
	}
	if (!qflag)
		printf("\npasscount: %i\nblockcount: %i\n", passcount,
			blockcount);
}

/*
   ====================
   =
   = BuildBlockingChains
   =
   ====================
 */
void BuildBlockingChains(void)
{
	boolean		*used;
	int		i,
			j;
	bpoint_t	*temppoints,
			*pt_p;
	bline_t		*li1,
			*li2;
	STORAGE		*chains_i;
	bchain_t	bch;
	int		cx,
			cy;

	used = (boolean *) alloca(numblines * sizeof(*used));
	memset(used, 0, numblines * sizeof(*used));
	temppoints = (bpoint_t *) alloca(numblines * sizeof(*temppoints));

	chains_i = (STORAGE *) SafeMalloc(sizeof(STORAGE));
	chains_i->data = (bchain_t *) SafeMalloc(sizeof(bchain_t));
	chains_i->count = 0;
	chains_i->size = sizeof(bchain_t);

	li1 = blines;
	for (i = 0; i < numblines; i++, li1++)
	{
		if (!qflag)
			printf("bline #%d of %d\r", i, numblines);
		if (used[i])
			continue;
		used[i] = true;

		/* start a new chain */
		pt_p = temppoints;
		pt_p->x = li1->p1.x;
		pt_p->y = li1->p1.y;
		pt_p++;
		pt_p->x = cx = li1->p2.x;
		pt_p->y = cy = li1->p2.y;
		pt_p++;

		ClearBBox(&bch.bounds);
		AddToBBox(&bch.bounds, li1->p1.x, li1->p1.y);
		AddToBBox(&bch.bounds, cx, cy);

		/* look for connected lines */
		do
		{
			li2 = li1 + 1;
			for (j = i + 1; j < numblines; j++, li2++)
				if (!used[j] && li2->p1.x == cx &&
				    li2->p1.y == cy)
					break;

			if (j == numblines)
				break;	/* no more lines in chain */

			/* add to chain */
			used[j] = true;
			pt_p->x = cx = li2->p2.x;
			pt_p->y = cy = li2->p2.y;
			pt_p++;
			AddToBBox(&bch.bounds, cx, cy);
		} while (1);

		/* save the block chain */
		bch.numpoints = pt_p - temppoints;
		bch.points = (bpoint_t *)
			malloc(bch.numpoints * sizeof(*bch.points));
		memcpy(bch.points, temppoints,
			bch.numpoints * sizeof(*bch.points));
		memcpy((bchain_t *) chains_i->data + chains_i->count, &bch,
			sizeof(bchain_t));
		chains_i->count += 1;
		chains_i->data = (bchain_t *) realloc(chains_i->data,
			sizeof(bchain_t) * (chains_i->count + 1));
		/* DrawBChain(&bch); */
	}
	if (!qflag)
		printf("\n");

	numbchains = chains_i->count;
	bchains = chains_i->data;
}

/*
   ====================
   =
   = ProcessConnections
   =
   ====================
 */
void ProcessConnections(void)
{
	int		i,
			s,
			wlcount,
			count;
	bbox_t		*secbox;
	STORAGE		*lines;
	worldline_t	*wl;
	mapvertex_t	*vt;
	mapsidedef_t	*sd;
	bline_t		bline;
	int		sec;
	int		RonsLines = 0;

	numsectors = secstore_i->count;
	wlcount = linestore_i->count;

	connections = (unsigned char *)malloc(numsectors * numsectors + 8);		/* allow rounding to bytes */
	memset(connections, 0, numsectors * numsectors);

	secboxes = secbox = (bbox_t *) malloc(numsectors * sizeof(bbox_t));
	for (i = 0; i < numsectors; i++, secbox++)
		ClearBBox(secbox);

	/* calculate bounding boxes for all sectors */
	if (!maptype)
	{
		maplinedef_t	*p;

		count = ldefstore_i->count;
		p = ldefstore_i->data;
		vt = mapvertexstore_i->data;
		for (i = 0; i < count; i++, p++)
		{
			if (!qflag)
				printf("BBox #%d of %d\r", i, count);
			for (s = 0; s < 1; s++)
			{
				if (p->sidenum[s] == -1)
					continue;	/* no back side */
				/* add both points to sector bounding box */
				sd = (mapsidedef_t *) sdefstore_i->data +
					p->sidenum[s];
				sec = sd->sector;
				AddToBBox(&secboxes[sec], vt[p->v1].x,
					vt[p->v1].y);
				AddToBBox(&secboxes[sec], vt[p->v2].x,
					vt[p->v2].y);
			}
		}
	}
	else
	{
		maplinedef_ext_t	*p;

		count = ldefstore_i->count;
		p = ldefstore_i->data;
		vt = mapvertexstore_i->data;
		for (i = 0; i < count; i++, p++)
		{
			if (!qflag)
				printf("BBox #%d of %d\r", i, count);
			for (s = 0; s < 1; s++)
			{
				if (p->sidenum[s] == -1)
					continue;	/* no back side */
				/* add both points to sector bounding box */
				sd = (mapsidedef_t *) sdefstore_i->data +
					p->sidenum[s];
				sec = sd->sector;
				AddToBBox(&secboxes[sec], vt[p->v1].x,
					vt[p->v1].y);
				AddToBBox(&secboxes[sec], vt[p->v2].x,
					vt[p->v2].y);
			}
		}
	}
	if (!qflag)
		printf("\n");

	/* make a list of only the solid lines */
/*
	lines = SafeMalloc(sizeof(STORAGE));
	lines->data = SafeMalloc(sizeof(bline));
	lines->count = 0;
	lines->size = sizeof(bline);
 */

	wl = linestore_i->data;
	for (i = 0; i < wlcount; wl++, i++)
	{
		if (wl->flags & ML_TWOSIDED)
			continue;
		RonsLines++;
	}

	lines = (STORAGE *) SafeMalloc(sizeof(STORAGE));
	lines->data = (bline_t *) SafeCalloc(RonsLines, sizeof(bline));
	lines->count = 0;
	lines->size = sizeof(bline);

	wl = linestore_i->data;
	for (i = 0; i < wlcount; wl++, i++)
	{
		if (!qflag)
			printf("Line #%d of %d\r", i, wlcount);
		if (wl->flags & ML_TWOSIDED)
			continue;	/* don't add two sided lines */
		bline.p1.x = wl->p1.x;
		bline.p1.y = wl->p1.y;
		bline.p2.x = wl->p2.x;
		bline.p2.y = wl->p2.y;
		memcpy((bline_t *) lines->data + lines->count, &bline,
			sizeof(bline));
		lines->count += 1;
/*
		lines->data = (bline_t *)realloc(lines->data,
		sizeof(bline_t) + (lines->count + 1));
 */
	}
	if (!qflag)
		printf("\n");

	blines = lines->data;
	numblines = lines->count;

	/* build blocking chains */
	if (!qflag)
		printf("BuildBlockingChains\n");
	BuildBlockingChains();

	/* build connection list */
	if (!qflag)
		printf("BuildConnections\n");
	BuildConnections();
}

/*
   =================
   =
   = OutputConnections
   =
   =================
 */
void OutputConnections(void)
{
	int	i;
	int	bytes;
	byte	*cons;
	char	*bits;

	cons = connections;
	bytes = (numsectors * numsectors + 7) / 8;
	bits = (char *)malloc(bytes);

	for (i = 0; i < bytes; i++)
	{
		bits[i] = cons[0] + (cons[1] << 1) + (cons[2] << 2) +
			(cons[3] << 3) + (cons[4] << 4) + (cons[5] << 5) +
			(cons[6] << 6) + (cons[7] << 7);
		cons += 8;
	}

	addName("reject", bits, bytes);

	if (!qflag)
		printf("reject: %i\n", bytes);
}
