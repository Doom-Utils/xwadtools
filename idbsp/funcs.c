/*
   FUNCS.C - functions for reading the WAD file and writing the DWD file
 */

#include "waddwd.h"

void 	*ReadWAD(FILE *f, long num, size_t size)
{
	int	numread;
	void	*p;

	p = (void *)SafeCalloc(num, size);
	if ((numread = fread(p, size, num, f)) != num)
	{
		free(p);
		Error("ReadWAD: read only %d of intended %d items",
		      numread, num);
	}

	return p;
}

void *ReadStorage(FILE *f, lumpinfo_t *dir, long *num, size_t size)
{
	fseek(f, dir->filepos, SEEK_SET);
	*num = dir->size / size;

	return ReadWAD(f, *num, size);
}

void WriteSideAndSector(FILE *f, mapsidedef_t *s, mapsector_t *sec)
{
	char	topt[9],
		bott[9],
		midt[9],
		fpic[9],
		cpic[9];

	strncpy(topt, s->toptexture, 8);
	topt[8] = '\0';
	strncpy(bott, s->bottomtexture, 8);
	bott[8] = '\0';
	strncpy(midt, s->midtexture, 8);
	midt[8] = '\0';
	strncpy(fpic, sec->floorpic, 8);
	fpic[8] = '\0';
	strncpy(cpic, sec->ceilingpic, 8);
	cpic[8] = '\0';

	fprintf(f, "    %d (%d : %s / %s / %s )\n",
		s->rowoffset, s->textureoffset, topt, bott, midt);

	fprintf(f, "    %d : %s %d : %s %d %d %d\n",
		sec->floorheight, fpic, sec->ceilingheight,
		cpic, sec->lightlevel, sec->special, sec->tag);
}

void WriteLines(FILE *f, long num, maplinedef_t *lines, mapvertex_t *vertexes,
		mapsidedef_t *sides, mapsector_t *sectors)
{
	int		i;
	mapsidedef_t	*s;
	maplinedef_t	*l;

	l = lines;
	fprintf(f, "lines:%ld\n", num);

	for (i = 0; i < num; i++, l++)
	{
		fprintf(f, "(%d,%d) to (%d,%d) : %d : %d : %d\n",
			vertexes[l->v1].x, vertexes[l->v1].y,
			vertexes[l->v2].x, vertexes[l->v2].y,
			l->flags, l->special, l->tag);
		s = &sides[l->sidenum[0]];
		WriteSideAndSector(f, s, &sectors[s->sector]);
		if (l->flags & ML_TWOSIDED)
		{
			s = &sides[l->sidenum[1]];
			WriteSideAndSector(f, s, &sectors[s->sector]);
		}
	}
}

void WriteThings(FILE *f, long num, mapthing_t *t)
{
	int	i;

	fprintf(f, "\nthings:%ld\n", num);
	for (i = 0; i < num; i++, t++)
		fprintf(f, "(%i,%i, %d) :%d, %d\n", t->x, t->y, t->angle,
			t->type, t->options);
}

void WriteHeader(FILE *f)
{
	fprintf(f, "WorldServer version 4\n");
}

void ExtractResource(FILE *f, lumpinfo_t *l)
{
	FILE	*d;
	char	fname[13];
	void	*p;
	long	num;

	memset(fname, 0, sizeof(fname));
	strncpy(fname, l->name, 8);
	strcat(fname, ".lmp");

	if ((d = fopen(fname, "wb")) == NULL)
		Error("Could not extract %s", fname);

	p = (void *)ReadStorage(f, l, &num, l->size);

	fwrite(p, l->size, 1, d);
	fclose(d);
	free(p);
}
