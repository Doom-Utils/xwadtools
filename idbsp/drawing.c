#include "idbsp.h"

STORAGE		*window_i,
		*view_i;
float		scale = 0.125;
NXRect		worldbounds;

/*
   ================
   =
   = IDRectFromPoints
   =
   = Makes the rectangle just touch the two points
   =
   ================
 */
void IDRectFromPoints(NXRect *rect, NXPoint const *p1, NXPoint const *p2)
{
/* return a rectangle that encloses the two points */
	if (p1->x < p2->x)
	{
		rect->origin.x = p1->x;
		rect->size.width = p2->x - p1->x + 1;
	}
	else
	{
		rect->origin.x = p2->x;
		rect->size.width = p1->x - p2->x + 1;
	}

	if (p1->y < p2->y)
	{
		rect->origin.y = p1->y;
		rect->size.height = p2->y - p1->y + 1;
	}
	else
	{
		rect->origin.y = p2->y;
		rect->size.height = p1->y - p2->y + 1;
	}
}

/*
   ==================
   =
   = IDEnclosePoint
   =
   = Make the rect enclose the point if it doesn't allready
   =
   ==================
 */
void IDEnclosePoint(NXRect *rect, NXPoint const *point)
{
	float	right,
		top;

	right = rect->origin.x + rect->size.width - 1;
	top = rect->origin.y + rect->size.height - 1;

	if (point->x < rect->origin.x)
		rect->origin.x = point->x;
	if (point->y < rect->origin.y)
		rect->origin.y = point->y;
	if (point->x > right)
		right = point->x;
	if (point->y > top)
		top = point->y;

	rect->size.width = right - rect->origin.x + 1;
	rect->size.height = top - rect->origin.y + 1;
}

/*
   ===========
   =
   = BoundLineStore
   =
   ===========
 */
void BoundLineStore(STORAGE *lines_i, NXRect *r)
{
	int		i,
			c;
	worldline_t	*line_p;

	c = lines_i->count;
	if (!c)
		Error("BoundLineStore: empty list");

	line_p = lines_i->data;
	IDRectFromPoints(r, &line_p->p1, &line_p->p2);

	for (i = 1; i < c; i++)
	{
		line_p = (worldline_t *) lines_i->data + i;
		IDEnclosePoint(r, &line_p->p1);
		IDEnclosePoint(r, &line_p->p2);
	}
}

/*
   ===========
   =
   = DrawLineStore
   =
   = Draws all of the lines in the given storage object
   =
   ===========
 */
void DrawLineStore(STORAGE *lines_i)
{
#if 0
	int		i,
			c;
	worldline_t	*line_p;

	if (!draw)
		return;

	c = lines_i->count;

	for (i = 0; i < c; i++)
	{
		line_p = (worldline_t *) lines_i->data + i;
		PSmoveto(line_p->p1.x, line_p->p1.y);
		PSlineto(line_p->p2.x, line_p->p2.y);
		PSstroke();
	}
	NXPing();
#endif
}

/*
   ===========
   =
   = DrawLine
   =
   = Draws all of the lines in the given storage object
   =
   ===========
 */
void DrawLineDef(maplinedef_t *ld)
{
#if 0
	mapvertex_t	*v1,
			*v2;

	if (!draw)
		return;

	v1 = mapvertexstore_i->data + ld->v1;
	v2 = mapvertexstore_i->data + ld->v2;
	PSmoveto(v1->x, v1->y);
	PSlineto(v2->x, v2->y);
	PSstroke();
	NXPing();
#endif
}

/*
   ===========
   =
   = DrawMap
   =
   ===========
 */
void DrawMap(void)
{
#if 0
	NXRect	scaled;
#endif

	BoundLineStore(linestore_i, &worldbounds);
	worldbounds.origin.x -= 8;
	worldbounds.origin.y -= 8;
	worldbounds.size.width += 16;
	worldbounds.size.height += 16;

#if 0
	if (!draw)
		return;

	scaled.origin.x = 300;
	scaled.origin.y = 80;
	scaled.size.width = worldbounds.size.width * scale;
	scaled.size.height = worldbounds.size.height * scale;
#endif
}

/*
   ===========
   =
   = EraseWindow
   =
   ===========
 */
void EraseWindow(void)
{
#if 0
	if (!draw)
		return;

	NXEraseRect(&worldbounds);
	NXPing();
#endif
}

/*
   ============================
   =
   = DrawDivLine
   =
   ============================
 */
void DrawDivLine(divline_t *div)
{
#if 0
	float	vx,
		vy,
		dist;

	if (!draw)
		return;

	PSsetgray(NX_BLACK);

	dist = sqrt(pow(div->dx, 2) + pow(div->dy, 2));
	vx = div->dx / dist;
	vy = div->dy / dist;

	dist = MAX(worldbounds.size.width, worldbounds.size.height);

	PSmoveto(div->pt.x - vx * dist, div->pt.y - vy * dist);
	PSlineto(div->pt.x + vx * dist, div->pt.y + vy * dist);
	PSstroke();
	NXPing();
#endif
}
