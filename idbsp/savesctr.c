#include "idbsp.h"

STORAGE		*secdefstore_i;
int		vertexsubcount[MAXVERTEX];
short		vertexsublist[MAXVERTEX][MAXTOUCHSECS];
int		subsectordef[MAXSUBSECTORS];
int		subsectornum[MAXSUBSECTORS];
int		buildsector;

/*
   ==========================
   =
   = RecursiveGroupSubsector
   =
   ==========================
 */
void RecursiveGroupSubsector(int ssnum)
{
	int		i,
			l,
			count;
	int		vertex;
	int		checkss;
	short		*vertsub;
	int		vt;
	mapseg_t	*seg;
	mapsubsector_t	*ss;
	mapsidedef_t	*sd;

	ss = (mapsubsector_t *) subsecstore_i->data + ssnum;
	subsectornum[ssnum] = buildsector;

	for (l = 0; l < ss->numsegs; l++)
	{
		if (!maptype)
		{
		    maplinedef_t	*ld;

		    seg = (mapseg_t *) maplinestore_i->data + ss->firstseg + l;
		    ld = (maplinedef_t *) ldefstore_i->data + seg->linedef;
		    DrawLineDef(ld);
		    sd = (mapsidedef_t *) sdefstore_i->data +
			ld->sidenum[seg->side];
		}
		else
		{
		    maplinedef_ext_t	*ld;
		    seg = (mapseg_t *) maplinestore_i->data + ss->firstseg + l;
		    ld = (maplinedef_ext_t *) ldefstore_i->data + seg->linedef;
		    sd = (mapsidedef_t *) sdefstore_i->data +
			ld->sidenum[seg->side];
		}
		sd->sector = buildsector;

		for (vt = 0; vt < 2; vt++)
		{
			if (vt)
				vertex = seg->v1;
			else
				vertex = seg->v2;

			vertsub = vertexsublist[vertex];
			count = vertexsubcount[vertex];
			for (i = 0; i < count; i++)
			{
				checkss = vertsub[i];
				if (subsectordef[checkss]==subsectordef[ssnum])
				{
					if (subsectornum[checkss] == -1)
					{
					    RecursiveGroupSubsector(checkss);
					    continue;
					}
					if (subsectornum[checkss]!=buildsector)
					    Error("RecursiveGroupSubsector: "
					    "regrouped (%d (subsectornum[%d]) "
					    "!= %d",
					    subsectornum[checkss], checkss,
					    buildsector);
				}
			}
		}
	}
}

/*
   =================
   =
   = UniqueSector
   =
   = Returns the sector number, adding a new sector if needed
   =================
 */
int UniqueSector(sectordef_t *def)
{
	int		i,
			count;
	mapsector_t	ms,
			*msp;

	ms.floorheight = def->floorheight;
	ms.ceilingheight = def->ceilingheight;
	memcpy(ms.floorpic, def->floorflat, 8);
	memcpy(ms.ceilingpic, def->ceilingflat, 8);
	ms.lightlevel = def->lightlevel;
	ms.special = def->special;
	ms.tag = def->tag;

	/* see if an identical sector already exists */
	count = secdefstore_i->count;
	msp = (mapsector_t *) secdefstore_i->data;
	for (i = 0; i < count; i++, msp++)
		if (!memcmp(msp, &ms, sizeof(ms)))
			return i;

	memcpy((mapsector_t *) secdefstore_i->data + secdefstore_i->count,
		&ms, sizeof(mapsector_t));
	secdefstore_i->count += 1;
	secdefstore_i->data = (mapsector_t *) realloc(secdefstore_i->data,
		sizeof(mapsector_t) * (secdefstore_i->count + 1));

	return count;
}

void AddSubsectorToVertex(int subnum, int vertex)
{
	int		j;

	for (j = 0; j < vertexsubcount[vertex]; j++)
		if (vertexsublist[vertex][j] == subnum)
			return;
	vertexsublist[vertex][j] = subnum;
	vertexsubcount[vertex]++;
}


/*
   ================
   =
   = BuildSectordefs
   =
   = Call before ProcessNodes
   ================
 */
void BuildSectordefs(void)
{
	int		i;
	worldline_t	*wl;
	mapsector_t	*msp;
	int		count;

	/* build sectordef list */
	secdefstore_i = (STORAGE *) SafeMalloc(sizeof(STORAGE));
	secdefstore_i->data = (mapsector_t *) SafeMalloc(sizeof(mapsector_t));
	secdefstore_i->count = 0;
	secdefstore_i->size = sizeof(mapsector_t);

	if (mapformat == format_dwd)
	{
		count = linestore_i->count;
		wl = linestore_i->data;
		for (i = 0; i < count; i++, wl++)
		{
			wl->side[0].sector =
				UniqueSector(&wl->side[0].sectordef);
			if (wl->flags & ML_TWOSIDED)
			{
				wl->side[1].sector =
					UniqueSector(&wl->side[1].sectordef);
			}
		}
	}
	else if (mapformat == format_map)
	{
		for (i = 0; i < secnum; i++)
		{
			secdefstore_i->data = (mapsector_t *)
				realloc(secdefstore_i->data,
					sizeof(mapsector_t) * (i + 1));
			secdefstore_i->count += 1;
			msp = (mapsector_t *) secdefstore_i->data + i;
			msp->floorheight = sectors[i].floorheight;
			msp->ceilingheight = sectors[i].ceilingheight;
			memcpy(msp->floorpic, sectors[i].floortexture, 8);
			memcpy(msp->ceilingpic, sectors[i].ceilingtexture, 8);
			msp->lightlevel = sectors[i].brightness;
			msp->special = sectors[i].special;
			msp->tag = sectors[i].tag;
		}
	}
}

/*
   ================
   =
   = ProcessSectors
   =
   = Must be called after ProcessNodes, because it references the subsector list
   ================
 */
void ProcessSectors(void)
{
	int		i,
			l;
	int		numss;
	mapsubsector_t	*ss;
	mapsector_t	sec;
	mapseg_t	*seg = (mapseg_t *)0;
	mapsidedef_t	*ms;

	/* build a connection matrix that lists all the subsectors that touch
	   each vertex */
	memset(vertexsubcount, 0, sizeof(vertexsubcount));
	memset(vertexsublist, 0, sizeof(vertexsublist));
	numss = subsecstore_i->count;

	for (i = 0; i < numss; i++)
	{
		if (!qflag)
			printf("Processing subsector #%d of %d\r", i, numss);
		ss = (mapsubsector_t *) subsecstore_i->data + i;
		for (l = 0; l < ss -> numsegs; l++)
		{
			seg = (mapseg_t *) maplinestore_i->data +
				ss->firstseg + l;
			AddSubsectorToVertex(i, seg->v1);
			AddSubsectorToVertex(i, seg->v2);
		}
		subsectornum[i] = -1;		   /* ungrouped */
		if (!maptype)
		{
			maplinedef_t	*ml;

			ml = (maplinedef_t *) ldefstore_i->data + seg->linedef;
			ms = (mapsidedef_t *) sdefstore_i->data +
				ml->sidenum[seg->side];
		}
		else
		{
			maplinedef_ext_t	*ml;

			ml = (maplinedef_ext_t *) ldefstore_i->data +
				seg->linedef;
			ms = (mapsidedef_t *) sdefstore_i->data +
				ml->sidenum[seg->side];
		}
		subsectordef[i] = ms->sector;
	}
	if (!qflag)
		printf("\n");

	/* recursively build final sectors */
	secstore_i = (STORAGE *) SafeMalloc(sizeof(STORAGE));
	secstore_i->data = (mapsector_t *) SafeMalloc(sizeof(mapsector_t));
	secstore_i->count = 0;
	secstore_i->size = sizeof(mapsector_t);

	buildsector = 0;
/*
	if (draw)
		PSsetgray(0);
 */
	for (i = 0; i < numss; i++)
	{
		if (subsectornum[i] == -1)
		{
			EraseWindow();
			RecursiveGroupSubsector(i);
			sec = *((mapsector_t *) secdefstore_i->data +
				subsectordef[i]);
			memcpy((mapsector_t *) secstore_i->data +
				secstore_i->count, &sec, sizeof(mapsector_t));
			secstore_i->count += 1;
			secstore_i->data = (mapsector_t *)
				realloc(secstore_i->data,
				sizeof(mapsector_t) * (secstore_i->count + 1));

			buildsector++;
		}
	}
}
