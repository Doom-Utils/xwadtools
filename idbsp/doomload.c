#include "idbsp.h"

int		linenum = 0;
STORAGE		*linestore_i,
		*thingstore_i;
int		secnum = 0;
w_sector_t	sectors[8192];

/*
   =================
   =
   = ReadLine, DWD format
   =
   =================
 */
worldline_t *ReadLine(FILE *file, worldline_t *line)
{
	NXPoint		*p1,
			*p2;
	worldside_t	*s;
	sectordef_t	*e;
	int		i;

	memset(line, 0, sizeof(worldline_t));

	p1 = &line->p1;
	p2 = &line->p2;

	if (fscanf(file, "(%f,%f) to (%f,%f) : %d : %d : %d\n",
			&p1->x, &p1->y, &p2->x, &p2->y, &line->flags,
			&line->special, &line->tag) != 7)
		Error("Failed ReadLine");

	for (i = 0; i <= ((line->flags & ML_TWOSIDED) != 0); i++)
	{
		s = &line->side[i];
		if (fscanf(file, "    %d (%d : %s / %s / %s )\n",
				&s->firstrow, &s->firstcollumn, s->toptexture,
				s->bottomtexture, s->midtexture) != 5)
			Error("Failed ReadLine (side)");
		e = &s->sectordef;
		if (fscanf(file, "    %d : %s %d : %s %d %d %d\n",
				&e->floorheight, e->floorflat,
				&e->ceilingheight, e->ceilingflat,
				&e->lightlevel, &e->special, &e->tag) != 7)
			Error("Failed ReadLine (sector)");
		if (!strcmp(e->floorflat, "-"))
			printf("WARNING: line %i has no sectordef\n", linenum);
	}

	linenum++;

	return line;
}

/*
   =================
   =
   = ReadThing, DWD format
   =
   =================
 */
worldthing_t *ReadThing(FILE *file, worldthing_t *thing)
{
	int                 x,
	                    y;

	memset(thing, 0, sizeof(*thing));

	if (fscanf(file, "(%i,%i, %d) :%d, %d\n",
			&x, &y, &thing->angle, &thing->type,
			&thing->options) != 5)
		Error("Failed ReadThing");

	thing->origin.x = x;
	thing->origin.y = y;

	return thing;
}

/*
   ==================
   =
   = LineOverlaid
   =
   = Check to see if the line is colinear and overlapping any previous lines
   ==================
 */
typedef struct
{
	float	left,
		right,
		top,
		bottom;
} bbox_t;

void BBoxFromPoints(bbox_t *box, NXPoint *p1, NXPoint *p2)
{
	if (p1->x < p2->x)
	{
		box->left = p1->x;
		box->right = p2->x;
	}
	else
	{
		box->left = p2->x;
		box->right = p1->x;
	}
	if (p1->y < p2->y)
	{
		box->bottom = p1->y;
		box->top = p2->y;
	}
	else
	{
		box->bottom = p2->y;
		box->top = p1->y;
	}
}

boolean LineOverlaid(worldline_t *line)
{
	int		j,
			count;
	worldline_t	*scan;
	divline_t	wl;
	bbox_t		linebox,
			scanbox;

	wl.pt = line->p1;
	wl.dx = line->p2.x - line->p1.x;
	wl.dy = line->p2.y - line->p1.y;

	count = linestore_i->count;
	scan = linestore_i->data;

	for (j = 0; j < count; j++, scan++)
	{
		if (PointOnSide(&scan->p1, &wl) != -1)
			continue;
		if (PointOnSide(&scan->p2, &wl) != -1)
			continue;
		/* line is colinear, see if it overlapps */
		BBoxFromPoints(&linebox, &line->p1, &line->p2);
		BBoxFromPoints(&scanbox, &scan->p1, &scan->p2);

		if (linebox.right > scanbox.left &&
		    linebox.left < scanbox.right)
			return true;
		if (linebox.bottom < scanbox.top &&
		    linebox.top > scanbox.bottom)
			return true;
	}
	return false;
}

/*
   ===================
   =
   = LoadDoomMap, DWD format
   =
   ===================
 */
void LoadDoomMap(FILE *file)
{
	int		i;
	int		linecount,
			thingcount;
	worldline_t	*line;
	worldthing_t	*thing;

	/* read lines */
	if (fscanf(file, "\nlines:%d\n", &linecount) != 1)
		Error("LoadDoomMap: can't read linecount");
	printf("%i lines\n", linecount);

	linestore_i = (STORAGE *) SafeMalloc(sizeof(STORAGE));
	linestore_i->data = (worldline_t *)
		SafeCalloc(linecount, sizeof(worldline_t));
	linestore_i->size = sizeof(worldline_t);
	linestore_i->count = 0;
	line = linestore_i->data;

	for (i = 0; i < linecount; i++, linestore_i->count++, line++)
	{
		if (!qflag)
			printf("Reading line #%d\r", i);
		ReadLine(file, line);
		if (line->p1.x == line->p2.x && line->p1.y == line->p2.y)
		{
			printf("WARNING: line %i is length 0\n", i);
			continue;
		}

		if (LineOverlaid(line))
		{
			printf("WARNING: line %i is overlaid\n", i);
			continue;
		}

	}
	if (!qflag)
		printf("\n");

	/* read things */
	if (fscanf(file, "\nthings:%d\n", &thingcount) != 1)
		Error("LoadDoomMap: can't read thingcount");
	if (!qflag)
		printf("%i things\n", thingcount);

	thingstore_i = (STORAGE *) SafeMalloc(sizeof(STORAGE));
	thingstore_i->data = (worldthing_t *)
		SafeCalloc(thingcount, sizeof(worldthing_t));
	thingstore_i->size = sizeof(worldthing_t);
	thingstore_i->count = thingcount;
	thing = thingstore_i->data;

	for (i = 0; i < thingcount; i++)
	{
		if (!qflag)
			printf("Reading thing #%d\r", i);
		ReadThing(file, thing);
		thing++;
	}
	if (!qflag)
		printf("\n");
}

/*
   =================
   =
   = Overread comments and comment lines in MAP files
   =
   =================
 */
void allowcomment(FILE *file)
{
	while (ungetc(getc(file), file) == '#')
		while (getc(file) != '\n')
			;
}

/*
   =================
   =
   = Find matching vertex or sector
   =
   =================
 */
int findmatch(const char *name, const char *table, int numentries, int length)
{
	int	i = 0;

	while (strncmp(name, table, 8) && (i < numentries))
	{
		i++;
		table += length;
	}
	if (i == numentries)
	{
		fprintf(stderr, "Couldn't match name: %s", name);
		exit(-1);
	}
	return i;
}

/*
   ===================
   =
   = LoadMap, MAP format - Doom style or Hexen style
   =
   ===================
 */
void LoadMap(FILE *file)
{
	char			keyword[20];
	int			lastitem;
	int			numvertices = 0;
	static w_vertex_t	vertices[32766];
	int			x, y;
	int			floorh, ceilingh, brightness, special, tag;
	char			floort[20];
	char			ceilingt[20];
	worldline_t		*line;
	worldside_t		*side;
	char			tovertex[20];
	int			v1, v2;
	char			secname[20];
	int			numthings;
	worldthing_t		*thing;

	/* get next keyword */
	allowcomment(file);
	fscanf(file, " %s ", &keyword[0]);

	/* process vertices */
	if (!strcmp(&keyword[0], "VERTEXES_START") |
	    !strcmp(&keyword[0], "VERTICES_START"))
	{
		lastitem = 0;
		while (!lastitem)
		{
			allowcomment(file);
			fscanf(file, " %s ", &keyword[0]);
			if (!strcmp(&keyword[0], "VERTEXES_END") |
			    !strcmp(&keyword[0], "VERTICES_END"))
				lastitem = 1;
			else
			{
				if (!qflag)
					printf("Reading vertex #%i\r",
						numvertices);
				fscanf(file, " : %d %d ", &x, &y);
				allowcomment(file);
				vertices[numvertices].x = x;
				vertices[numvertices].y = y;
				strncpy(vertices[numvertices].name,
					&keyword[0], 8);
				numvertices++;
			}
		}
		fscanf(file, " %s ", &keyword[0]);
	}
	else
		Error("VERTICES_START marker missing");
	if (!qflag)
		printf("\n");

	/* process sectors */
	if (!strcmp(&keyword[0], "SECTORS_START"))
	{
		lastitem = 0;
		while (!lastitem)
		{
			allowcomment(file);
			fscanf(file, " %s ", &keyword[0]);
			if (!strcmp(&keyword[0], "SECTORS_END"))
				lastitem = 1;
			else
			{
				if (!qflag)
					printf("Reading sector #%i\r",
						secnum);
				fscanf(file, " : %d %d %s %s %d %d %d ",
					&floorh, &ceilingh, &floort[0],
					&ceilingt[0], &brightness, &special,
					&tag);
				allowcomment(file);
				sectors[secnum].floorheight = floorh;
				sectors[secnum].ceilingheight = ceilingh;
				sectors[secnum].brightness = brightness;
				sectors[secnum].special = special;
				sectors[secnum].tag = tag;
				strncpy(sectors[secnum].name,
					&keyword[0], 8);
				strncpy(sectors[secnum].floortexture,
					&floort[0], 8);
				strncpy(sectors[secnum].ceilingtexture,
					&ceilingt[0], 8);
				secnum++;
			}
		}
		fscanf(file, " %s ", &keyword[0]);
	}
	else
		Error("SECTORS_START marker missing");
	if (!qflag)
		printf("\n");

	/* process lines */
	linestore_i = (STORAGE *) SafeMalloc(sizeof(STORAGE));
	linestore_i->size = sizeof(worldline_t);
	linestore_i->count = 0;
	linestore_i->data = (worldline_t *) 0;
	if (!strcmp(&keyword[0], "LINEDEFS_START"))
	{
		lastitem = 0;
		while (!lastitem)
		{
			allowcomment(file);
			fscanf(file, " %s ", &keyword[0]);
			if (!strcmp(&keyword[0], "LINEDEFS_END"))
				lastitem = 1;
			else
			{
			    linestore_i->data = (worldline_t *)
					SafeRealloc(linestore_i->data,
					  (linenum + 1) * sizeof(worldline_t));
			    line = (worldline_t *) linestore_i->data + linenum;
			    memset(line, 0, sizeof(worldline_t));
			    if (!qflag)
				printf("Reading line #%i\r", linenum);
			    if (!maptype)
			    {
				/* Doom map type */
				fscanf(file, " %s : %d %d %d ", &tovertex[0],
					&line->flags, &line->special,
					&line->tag);
				allowcomment(file);
				v1 = findmatch(keyword, (char*)vertices,
					numvertices, sizeof(w_vertex_t));
				v2 = findmatch(tovertex, (char*)vertices,
					numvertices, sizeof(w_vertex_t));
				line->p1.x = vertices[v1].x;
				line->p1.y = vertices[v1].y;
				line->p2.x = vertices[v2].x;
				line->p2.y = vertices[v2].y;
			    }
			    else
			    {
				/* Hexen map type */
				fscanf(file, " %s : %d %d %d %d %d %d %d ",
					&tovertex[0], &line->flags,
					&line->special, &line->arg1,
					&line->arg2, &line->arg3, &line->arg4,
					&line->arg5);
				allowcomment(file);
				v1 = findmatch(keyword, (char*)vertices,
					numvertices, sizeof(w_vertex_t));
				v2 = findmatch(tovertex, (char*)vertices,
					numvertices, sizeof(w_vertex_t));
				line->p1.x = vertices[v1].x;
				line->p1.y = vertices[v1].y;
				line->p2.x = vertices[v2].x;
				line->p2.y = vertices[v2].y;
			    }
			    /* read one or two sidedefs */
			    side = &line->side[0];
			    fscanf(file, " %s %d %d %s %s %s ", &secname[0],
				&side->firstcollumn, &side->firstrow,
				side->toptexture, side->bottomtexture,
				side->midtexture);
			    allowcomment(file);
			    side->sector = findmatch(secname, (char*)sectors,
						secnum, sizeof(w_sector_t));
			    fscanf(file, "%s", &secname[0]);
			    if (!strcmp(&secname[0], "-"))
				allowcomment(file);
			    else
			    {
				side = &line->side[1];
		    		fscanf(file, " %d %d %s %s %s ",
				&side->firstcollumn, &side->firstrow,
				side->toptexture, side->bottomtexture,
				side->midtexture);
				allowcomment(file);
				side->sector = findmatch(secname,
						(char*)sectors, secnum,
						sizeof(w_sector_t));
			    }
			    if (line->p1.x == line->p2.x &&
				line->p1.y == line->p2.y)
					printf("WARNING: line %i is length 0\n",
						linenum);
			    if (LineOverlaid(line))
				printf("WARNING: line %i is overlaid\n",
					linenum);
			    linenum++;
			    linestore_i->count++;
			}
		}
		fscanf(file, " %s ", &keyword[0]);
	}
	else
		Error("LINEDEFS_START marker missing");
	if (!qflag)
		printf("\n");

	/* process things */
	thingstore_i = (STORAGE *) SafeMalloc(sizeof(STORAGE));
	thingstore_i->size = sizeof(worldthing_t);
	thingstore_i->count = 0;
	thingstore_i->data = (worldthing_t *) 0;
	if (!strcmp(&keyword[0], "THINGS_START"))
	{
		lastitem = 0;
		numthings = 0;
		while (!lastitem)
		{
			allowcomment(file);
			fscanf(file, " %s ", &keyword[0]);
			if (!strcmp(&keyword[0], "THINGS_END"))
				lastitem = 1;
			else
			{
			    thingstore_i->data = (worldthing_t *)
				SafeRealloc(thingstore_i->data,
				  (numthings + 1) * sizeof(worldthing_t));
			    thingstore_i->count++;
			    thing = (worldthing_t *)thingstore_i->data +
					numthings;
			    memset(thing, 0, sizeof(worldthing_t));
			    sscanf(keyword, "%d", &thing->type);
			    if (!qflag)
				printf("Reading thing #%i\r", numthings);
			    if (!maptype)
			    {
				/* Doom map type */
				fscanf(file, " : %d %d %d %d ", &x, &y,
				    &thing->angle, &thing->options);
				thing->origin.x = x;
				thing->origin.y = y;
			    }
			    else
			    {
				/* Hexen map type */
				fscanf(file,
				    " : %d %d %d %d %d %d %d %d %d %d %d %d ",
				    &thing->tid, &x, &y, &thing->z,
				    &thing->angle, &thing->options,
				    &thing->special, &thing->arg1, &thing->arg2,
				    &thing->arg3, &thing->arg4, &thing->arg5);
			        thing->origin.x = x;
			        thing->origin.y = y;
			    }
			    numthings++;
			}
		}
	}
	else
		Error("THINGS_START marker missing");
	if (!qflag)
		printf("\n");
}
