#include "idbsp.h"

STORAGE		*secstore_i;
STORAGE		*mapvertexstore_i;
STORAGE		*subsecstore_i;
STORAGE		*maplinestore_i;
STORAGE		*nodestore_i;
STORAGE		*mapthingstore_i;
STORAGE		*ldefstore_i;
STORAGE		*sdefstore_i;

/*
 ===============================================================================

   the output functions byte swap and write lumps

 ===============================================================================
 */


/*
   ================
   =
   = WriteStorage
   =
   ================
 */
void WriteStorage(char *name, STORAGE *store, int esize)
{
	int                 count,
	                    len;

	count = store->count;
	len = esize * count;
	addName(name, store->data, len);
	if (!qflag)
		printf("%s (%i): %i\n", name, count, len);
}

/*
   =================
   =
   = OutputSectors
   =
   =================
 */
void OutputSectors(void)
{
	int		i;
	int		count;
	mapsector_t	*p;

	count = secstore_i->count;
	p = secstore_i->data;

	for (i = 0 ; i < count ; i++, p++)
	{
		p->floorheight = bswapw(p->floorheight);
		p->ceilingheight = bswapw(p->ceilingheight);
		p->lightlevel = bswapw(p->lightlevel);
		p->special = bswapw(p->special);
		p->tag = bswapw(p->tag);
	}

	WriteStorage("sectors", secstore_i, sizeof(mapsector_t));
}


/*
   =================
   =
   = OutputSegs
   =
   =================
 */
void OutputSegs(void)
{
	int		i;
	int		count;
	mapseg_t	*p;

	count = maplinestore_i->count;
	p = maplinestore_i->data;

	for (i = 0 ; i < count ; i++, p++)
	{
		p->v1 = bswapw(p->v1);
		p->v2 = bswapw(p->v2);
		p->angle = bswapw(p->angle);
		p->linedef = bswapw(p->linedef);
		p->side = bswapw(p->side);
		p->offset = bswapw(p->offset);
	}

	WriteStorage("segs", maplinestore_i, sizeof(mapseg_t));
}

/*
   =================
   =
   = OutputSubsectors
   =
   =================
 */
void OutputSubsectors(void)
{
	int		i;
	int		count;
	mapsubsector_t	*p;

	count = subsecstore_i->count;
	p = subsecstore_i->data;

	for (i = 0 ; i < count ; i++, p++)
	{
		p->numsegs = bswapw(p->numsegs);
		p->firstseg = bswapw(p->firstseg);
	}

	WriteStorage("ssectors", subsecstore_i, sizeof(mapsubsector_t));
}

/*
   =================
   =
   = OutputVertexes
   =
   =================
 */
void OutputVertexes(void)
{
	int		i;
	int		count;
	mapvertex_t	*p;

	count = mapvertexstore_i->count;
	p = mapvertexstore_i->data;

	for (i = 0 ; i < count ; i++, p++)
	{
		p->x = bswapw(p->x);
		p->y = bswapw(p->y);
	}

	WriteStorage("vertexes", mapvertexstore_i, sizeof(mapvertex_t));
}

/*
   =================
   =
   = OutputThings
   =
   =================
 */
void OutputThings(void)
{
	int		i;
	int		count;
	mapthing_t	*p;

	count = mapthingstore_i->count;
	p = mapthingstore_i->data;

	for (i = 0 ; i < count ; i++, p++)
	{
		p->x = bswapw(p->x);
		p->y = bswapw(p->y);
		p->angle = bswapw(p->angle);
		p->type = bswapw(p->type);
		p->options = bswapw(p->options);
	}

	if (!maptype)
		WriteStorage("things", mapthingstore_i, sizeof(mapthing_t));
	else
		WriteStorage("things", mapthingstore_i, sizeof(mapthing_ext_t));
}

/*
   =================
   =
   = OutputLineDefs
   =
   =================
 */
void OutputLineDefs(void)
{
	int		i;
	int		count;

	if (!maptype)
	{
		maplinedef_t	*p;

		count = ldefstore_i->count;
		p = ldefstore_i->data;

		for (i = 0; i < count; i++, p++)
		{
			p->v1 = bswapw(p->v1);
			p->v2 = bswapw(p->v2);
/* some ancient version of DoomEd left ML_MAPPED flags in some of the levels */
			/* p->flags = bswapw(p->flags & ~ML_MAPPED); */
			p->flags = bswapw(p->flags);
			p->special = bswapw(p->special);
			p->tag = bswapw(p->tag);
			p->sidenum[0] = bswapw(p->sidenum[0]);
			p->sidenum[1] = bswapw(p->sidenum[1]);
		}

		WriteStorage("linedefs", ldefstore_i, sizeof(maplinedef_t));
	}
	else
	{
		maplinedef_ext_t	*p;

		count = ldefstore_i->count;
		p = ldefstore_i->data;

		for (i = 0; i < count; i++, p++)
		{
			p->v1 = bswapw(p->v1);
			p->v2 = bswapw(p->v2);
			p->flags = bswapw(p->flags);
			p->sidenum[0] = bswapw(p->sidenum[0]);
			p->sidenum[1] = bswapw(p->sidenum[1]);
		}

		WriteStorage("linedefs", ldefstore_i, sizeof(maplinedef_ext_t));
	}
}

/*
   =================
   =
   = OutputSideDefs
   =
   =================
 */
void OutputSideDefs(void)
{
	int		i;
	int		count;
	mapsidedef_t	*p;

	count = sdefstore_i->count;
	p = sdefstore_i->data;

	for (i = 0 ; i < count ; i++, p++)
	{
		p->textureoffset = bswapw(p->textureoffset);
		p->rowoffset = bswapw(p->rowoffset);
		p->sector = bswapw(p->sector);
	}

	WriteStorage("sidedefs", sdefstore_i, sizeof(mapsidedef_t));
}

/*
   =================
   =
   = OutputNodes
   =
   =================
 */
void OutputNodes(void)
{
	int		i,
			j;
	int		count;
	mapnode_t	*p;

	count = nodestore_i->count;
	p = nodestore_i->data;

	for (i = 0; i < count; i++, p++)
	{
		for (j = 0; j < sizeof(mapnode_t)/2; j++)
			((short *)p)[j] = bswapw(((short *)p)[j]);
	}

	WriteStorage("nodes", nodestore_i, sizeof(mapnode_t));
}

/*
 ===============================================================================

   PROCESSING

 ===============================================================================
 */

/*
   =================
   =
   = UniqueVertex
   =
   = Returns the vertex number, adding a new vertex if needed
   =================
 */
int UniqueVertex(int x, int y)
{
	int		i,
			count;
	mapvertex_t	mv,
			*mvp;

	mv.x = x;
	mv.y = y;

	/* see if an identical vertex already exists */
	count = mapvertexstore_i->count;
	mvp = mapvertexstore_i->data;
	for (i = 0; i < count; i++, mvp++)
		if (mvp->x == mv.x && mvp->y == mv.y)
			return i;

	memcpy((mapvertex_t *) mapvertexstore_i->data + mapvertexstore_i->count,
		&mv, sizeof(mapvertex_t));
	mapvertexstore_i->count += 1;
	mapvertexstore_i->data = (mapvertex_t *) realloc(mapvertexstore_i->data,
		sizeof(mapvertex_t) * (mapvertexstore_i->count + 1));

	return count;
}

/*
   =============================================================================
 */

float	bbox[4];

/*
   =================
   =
   = AddPointToBBox
   =
   =================
 */
void AddPointToBBox(NXPoint *pt)
{
	if (pt->x < bbox[BOXLEFT])
		bbox[BOXLEFT] = pt->x;
	if (pt->x > bbox[BOXRIGHT])
		bbox[BOXRIGHT] = pt->x;

	if (pt->y > bbox[BOXTOP])
		bbox[BOXTOP] = pt->y;
	if (pt->y < bbox[BOXBOTTOM])
		bbox[BOXBOTTOM] = pt->y;
}

/*
   =================
   =
   = ProcessLines
   =
   = Adds the lines in a subsector to the mapline storage
   =================
 */
void ProcessLines(STORAGE *store_i)
{
	int		i,
			count;
	line_t		*wline;
	mapseg_t	line;
	short		angle;
	float		fangle;

	bbox[BOXLEFT] = INT_MAX;
	bbox[BOXRIGHT] = INT_MIN;
	bbox[BOXTOP] = INT_MIN;
	bbox[BOXBOTTOM] = INT_MAX;

	count = store_i->count;
	for (i = 0; i < count; i++)
	{
		wline = (line_t *) store_i->data + i;
		if (wline->grouped)
			printf("ERROR: line regrouped\n");
		wline->grouped = true;

		memset(&line, 0, sizeof(line));
		AddPointToBBox(&wline->p1);
		AddPointToBBox(&wline->p2);
		line.v1 = UniqueVertex(wline->p1.x, wline->p1.y);
		line.v2 = UniqueVertex(wline->p2.x, wline->p2.y);
		line.linedef = wline->linedef;
		line.side = wline->side;
		line.offset = wline->offset;
		fangle = atan2(wline->p2.y - wline->p1.y, wline->p2.x -
			 wline->p1.x);
		angle = (short)(fangle / (PI * 2) * 0x10000);
		line.angle = angle;
		memcpy((mapseg_t *) maplinestore_i->data +
			maplinestore_i->count, &line, sizeof(line));
		maplinestore_i->count += 1;
		maplinestore_i->data = (mapseg_t *)
			realloc(maplinestore_i->data,
			sizeof(mapseg_t) * (maplinestore_i->count + 1));
	}
}


/*
   =================
   =
   = ProcessSubsector
   =
   =================
 */
int ProcessSubsector(STORAGE *wmaplinestore_i)
{
	int		count;
	worldline_t	*linedef;
	line_t		*wline;
	mapsubsector_t	sub;

	memset(&sub, 0, sizeof(sub));

	count = wmaplinestore_i->count;
	if (count < 1)
		Error("ProcessSubsector: count = %i", count);

	wline = wmaplinestore_i->data;

	linedef = (worldline_t *) linestore_i->data + wline->linedef;
	sub.numsegs = count;
	sub.firstseg = maplinestore_i->count;
	ProcessLines(wmaplinestore_i);

	/* add the new subsector */
	memcpy((mapsubsector_t *) subsecstore_i->data + subsecstore_i->count,
		&sub, sizeof(mapsubsector_t));
	subsecstore_i->count += 1;
	subsecstore_i->data = (mapsubsector_t *) realloc(subsecstore_i->data,
		sizeof(mapsubsector_t) * (subsecstore_i->count + 1));

	return subsecstore_i->count - 1;
}

/*
   =================
   =
   = ProcessNode
   =
   =================
 */
int ProcessNode(bspnode_t *node, short *totalbox)
{
	short		subbox[2][4];
	int		i,
			r;
	mapnode_t	mnode;

	memset(&mnode, 0, sizeof(mnode));

	if (node->lines_i)	/* NF_SUBSECTOR flags a subsector */
	{
		r = ProcessSubsector(node->lines_i);
		for (i = 0; i < 4; i++)
			totalbox[i] = bbox[i];
		return r | NF_SUBSECTOR;
	}

	mnode.x = node->divline.pt.x;
	mnode.y = node->divline.pt.y;
	mnode.dx = node->divline.dx;
	mnode.dy = node->divline.dy;

	r = ProcessNode(node->side[0], subbox[0]);
	mnode.children[0] = r;
	for (i = 0; i < 4; i++)
		mnode.bbox[0][i] = subbox[0][i];

	r = ProcessNode(node->side[1], subbox[1]);
	mnode.children[1] = r;
	for (i = 0; i < 4; i++)
		mnode.bbox[1][i] = subbox[1][i];

	totalbox[BOXLEFT] = MIN(subbox[0][BOXLEFT], subbox[1][BOXLEFT]);
	totalbox[BOXTOP] = MAX(subbox[0][BOXTOP], subbox[1][BOXTOP]);
	totalbox[BOXRIGHT] = MAX(subbox[0][BOXRIGHT], subbox[1][BOXRIGHT]);
	totalbox[BOXBOTTOM] = MIN(subbox[0][BOXBOTTOM], subbox[1][BOXBOTTOM]);

	memcpy((mapnode_t *) nodestore_i->data + nodestore_i->count, &mnode,
		sizeof(mapnode_t));
	nodestore_i->count += 1;
	nodestore_i->data = (mapnode_t *) realloc(nodestore_i->data,
		sizeof(mapnode_t) * (nodestore_i->count + 1));

	return nodestore_i->count - 1;
}

/*
   =================
   =
   = ProcessNodes
   =
   = Recursively builds the nodes, subsectors, and line lists,
   = then writes the lumps
   =================
 */
void ProcessNodes(void)
{
	short		worldbounds[4];

	subsecstore_i = (STORAGE *) SafeMalloc(sizeof(STORAGE));
	subsecstore_i->data = (mapsubsector_t *)
		SafeMalloc(sizeof(mapsubsector_t));
	subsecstore_i->count = 0;
	subsecstore_i->size = sizeof(mapsubsector_t);

	maplinestore_i = (STORAGE *) SafeMalloc(sizeof(STORAGE));
	maplinestore_i->data = (mapseg_t *) SafeMalloc(sizeof(mapseg_t));
	maplinestore_i->count = 0;
	maplinestore_i->size = sizeof(mapseg_t);

	nodestore_i = (STORAGE *) SafeMalloc(sizeof(STORAGE));
	nodestore_i->data = (mapnode_t *) SafeMalloc(sizeof(mapnode_t));
	nodestore_i->count = 0;
	nodestore_i->size = sizeof(mapnode_t);

	ProcessNode(startnode, worldbounds);
}

/*
   =================
   =
   = ProcessThings
   =
   =================
 */
void ProcessThings(void)
{
	worldthing_t	*wt;
	int		count;

	if (!maptype)
	{
		/* process things for Doom map */
		mapthing_t	mt;

		mapthingstore_i = (STORAGE *) SafeMalloc(sizeof(STORAGE));
		mapthingstore_i->data = (mapthing_t *)
			SafeMalloc(sizeof(mapthing_t));
		mapthingstore_i->count = 0;
		mapthingstore_i->size = sizeof(mapthing_t);

		count = thingstore_i->count;
		wt = thingstore_i->data;
		while (count--)
		{
			memset(&mt, 0, sizeof(mt));
			mt.x = wt->origin.x;
			mt.y = wt->origin.y;
			mt.angle = wt->angle;
			mt.type = wt->type;
			mt.options = wt->options;
			memcpy((mapthing_t *) mapthingstore_i->data +
				mapthingstore_i->count, &mt,
				sizeof(mapthing_t));
			mapthingstore_i->count += 1;
			mapthingstore_i->data = (mapthing_t *)
				realloc(mapthingstore_i->data,
				sizeof(mapthing_t) *
				(mapthingstore_i->count + 1));
			wt++;
		}
	}
	else
	{
		/* process things for Doom map */
		mapthing_ext_t	mt;

		mapthingstore_i = (STORAGE *) SafeMalloc(sizeof(STORAGE));
		mapthingstore_i->data = (mapthing_ext_t *)
			SafeMalloc(sizeof(mapthing_ext_t));
		mapthingstore_i->count = 0;
		mapthingstore_i->size = sizeof(mapthing_ext_t);

		count = thingstore_i->count;
		wt = thingstore_i->data;
		while (count--)
		{
			memset(&mt, 0, sizeof(mt));
			mt.tid = wt->tid;
			mt.x = wt->origin.x;
			mt.y = wt->origin.y;
			mt.z = wt->z;
			mt.angle = wt->angle;
			mt.type = wt->type;
			mt.options = wt->options;
			mt.special = wt->special;
			mt.args[0] = wt->arg1;
			mt.args[1] = wt->arg2;
			mt.args[2] = wt->arg3;
			mt.args[3] = wt->arg4;
			mt.args[4] = wt->arg5;
			memcpy((mapthing_ext_t *) mapthingstore_i->data +
				mapthingstore_i->count, &mt,
				sizeof(mapthing_ext_t));
			mapthingstore_i->count += 1;
			mapthingstore_i->data = (mapthing_ext_t *)
				realloc(mapthingstore_i->data,
				sizeof(mapthing_ext_t) *
				(mapthingstore_i->count + 1));
			wt++;
		}
	}
}

/*
   =============================================================================
 */

/*
   ==================
   =
   = ProcessSidedef
   =
   ==================
 */
int ProcessSidedef(worldside_t *ws)
{
	mapsidedef_t	ms;

	ms.textureoffset = ws->firstcollumn;
	ms.rowoffset = ws->firstrow;
	memcpy(ms.toptexture, ws->toptexture, 8);
	memcpy(ms.bottomtexture, ws->bottomtexture, 8);
	memcpy(ms.midtexture, ws->midtexture, 8);
	ms.sector = ws->sector;

	memcpy((mapsidedef_t *) sdefstore_i->data + sdefstore_i->count,
		&ms, sizeof(mapsidedef_t));
	sdefstore_i->count += 1;
	sdefstore_i->data = (mapsidedef_t *) realloc(sdefstore_i->data,
		sizeof(mapsidedef_t) * (sdefstore_i->count + 1));

	return sdefstore_i->count - 1;
}

/*
   ==================
   =
   = ProcessLineSideDefs
   =
   = Must be called after BuildSectors
   ==================
 */
void ProcessLineSideDefs(void)
{
	int		i,
			count;
	worldline_t	*wl;

	if (!maptype)
	{
		/* process Doom linedefs */
		maplinedef_t	ld;

		mapvertexstore_i = (STORAGE *) SafeMalloc(sizeof(STORAGE));
		mapvertexstore_i->data = (mapvertex_t *)
			SafeMalloc(sizeof(mapvertex_t));
		mapvertexstore_i->count = 0;
		mapvertexstore_i->size = sizeof(mapvertex_t);

		ldefstore_i = (STORAGE *) SafeMalloc(sizeof(STORAGE));
		ldefstore_i->data = (maplinedef_t *)
			SafeMalloc(sizeof(maplinedef_t));
		ldefstore_i->count = 0;
		ldefstore_i->size = sizeof(maplinedef_t);

		sdefstore_i = (STORAGE *) SafeMalloc(sizeof(STORAGE));
		sdefstore_i->data = (mapsidedef_t *)
			SafeMalloc(sizeof(mapsidedef_t));
		sdefstore_i->count = 0;
		sdefstore_i->size = sizeof(mapsidedef_t);

		count = linestore_i->count;
		wl = linestore_i->data;
		for (i = 0; i < count; i++, wl++)
		{
			ld.v1 = UniqueVertex(wl->p1.x, wl->p1.y);
			ld.v2 = UniqueVertex(wl->p2.x, wl->p2.y);
			ld.flags = wl->flags;
			ld.special = wl->special;
			ld.tag = wl->tag;
			ld.sidenum[0] = ProcessSidedef(&wl->side[0]);
			if (wl->flags & ML_TWOSIDED)
				ld.sidenum[1] = ProcessSidedef(&wl->side[1]);
			else
				ld.sidenum[1] = -1;

			memcpy((maplinedef_t *) ldefstore_i->data +
				ldefstore_i->count, &ld, sizeof(maplinedef_t));
			ldefstore_i->count += 1;
			ldefstore_i->data = (maplinedef_t *)
				realloc(ldefstore_i->data,
				sizeof(maplinedef_t) *
				(ldefstore_i->count + 1));
		}
	}
	else
	{
		/* process Hexen linedefs */
		maplinedef_ext_t	ld;

		mapvertexstore_i = (STORAGE *) SafeMalloc(sizeof(STORAGE));
		mapvertexstore_i->data = (mapvertex_t *)
			SafeMalloc(sizeof(mapvertex_t));
		mapvertexstore_i->count = 0;
		mapvertexstore_i->size = sizeof(mapvertex_t);

		ldefstore_i = (STORAGE *) SafeMalloc(sizeof(STORAGE));
		ldefstore_i->data = (maplinedef_ext_t *)
			SafeMalloc(sizeof(maplinedef_ext_t));
		ldefstore_i->count = 0;
		ldefstore_i->size = sizeof(maplinedef_ext_t);

		sdefstore_i = (STORAGE *) SafeMalloc(sizeof(STORAGE));
		sdefstore_i->data = (mapsidedef_t *)
			SafeMalloc(sizeof(mapsidedef_t));
		sdefstore_i->count = 0;
		sdefstore_i->size = sizeof(mapsidedef_t);

		count = linestore_i->count;
		wl = linestore_i->data;
		for (i = 0; i < count; i++, wl++)
		{
			ld.v1 = UniqueVertex(wl->p1.x, wl->p1.y);
			ld.v2 = UniqueVertex(wl->p2.x, wl->p2.y);
			ld.flags = wl->flags;
			ld.special = wl->special;
			ld.args[0] = wl->arg1;
			ld.args[1] = wl->arg2;
			ld.args[2] = wl->arg3;
			ld.args[3] = wl->arg4;
			ld.args[4] = wl->arg5;
			ld.sidenum[0] = ProcessSidedef(&wl->side[0]);
			if (wl->flags & ML_TWOSIDED)
				ld.sidenum[1] = ProcessSidedef(&wl->side[1]);
			else
				ld.sidenum[1] = -1;

			memcpy((maplinedef_ext_t *) ldefstore_i->data +
				ldefstore_i->count, &ld,
				sizeof(maplinedef_ext_t));
			ldefstore_i->count += 1;
			ldefstore_i->data = (maplinedef_ext_t *)
				realloc(ldefstore_i->data,
				sizeof(maplinedef_ext_t) *
				(ldefstore_i->count + 1));
		}
	}
}

/*
   =============================================================================
 */

/*
   ==================
   =
   = SaveDoomMap
   =
   ==================
 */
void SaveDoomMap(void)
{
	if (!qflag)
		printf("BuildSectordefs\n");
	BuildSectordefs();

	if (!qflag)
		printf("ProcessThings\n");
	ProcessThings();

	if (!qflag)
		printf("ProcessLineSideDefs\n");
	ProcessLineSideDefs();

	if (!qflag)
		printf("ProcessNodes\n");
	ProcessNodes();

	if (!qflag)
		printf("ProcessSectors\n");
	ProcessSectors();

	if (!qflag)
		printf("ProcessConnections\n");
	ProcessConnections();

	/* all processing is complete, write everything out */
	OutputThings();
	OutputLineDefs();
	OutputSideDefs();
	OutputVertexes();
	OutputSegs();
	OutputSubsectors();
	OutputNodes();
	OutputSectors();
	OutputConnections();
}

/*
   ==================
   =
   = SaveBehavior, for Hexen style maps
   =
   ==================
 */
void SaveBehavior(char *fn)
{
	char	data[16] = { 'A', 'C', 'S', '\0', 0x08, '\0', '\0', '\0',
			     '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0' };

	if (maptype)
	{
		if (!fn)
			addName("behavior", data, sizeof(data));
		else
			AddFromFile("behavior", 0, fn);
	}
}
