#include "idbsp.h"
#include <ctype.h>

typedef struct {
	char	identification[4];
	int	numlumps;
	int	infotableofs;
} wadinfo_t;

typedef struct {
	int	filepos;
	int	size;
	char	name[8];
} lumpinfo_t;

/*
 =============================================================================
 */

/*
   ============
   =
   = initFromFile:
   =
   ============
 */
void initFromFile(void)
{
	wadinfo_t	wad;
	lumpinfo_t	*lumps;
	int		i;

	/* read in the header */
	fread(&wad, sizeof(wad), 1, wad_i.handle);
	if (strncmp(wad.identification, "PWAD", 4))
	{
		Error("initFromFile: specified WAD is not a PWAD");
	}

	/* read in the lumpinfo */
	fseek(wad_i.handle, wad.infotableofs, SEEK_SET);

	wad_i.info = (STORAGE *) SafeMalloc(sizeof(STORAGE));
	wad_i.info->data = (lumpinfo_t *) SafeCalloc(wad.numlumps,
				sizeof(lumpinfo_t));
	wad_i.info->count = wad.numlumps;
	wad_i.info->size = sizeof(lumpinfo_t);
	lumps = wad_i.info->data;

	fread(lumps, sizeof(lumpinfo_t), wad.numlumps, wad_i.handle);

	for (i = 0; i < wad.numlumps; i++, lumps++)
	{
		lumps->filepos = bswapl(lumps->filepos);
		lumps->size = bswapl(lumps->size);
	}

	return;
}

/*
   ============
   =
   = initNew:
   =
   ============
 */
void initNew(void)
{
	wadinfo_t	wad;

	wad_i.info = (STORAGE *) SafeMalloc(sizeof(STORAGE));
	wad_i.info->data = (lumpinfo_t *) SafeMalloc(sizeof(lumpinfo_t));
	wad_i.info->size = sizeof(lumpinfo_t);
	wad_i.info->count = 0;

	wad_i.dirty = true;

	/* leave space for wad header */
	fwrite(&wad, sizeof(wad), 1, wad_i.handle);

	return;
}

void WadfileClose(void)
{
	fclose(wad_i.handle);
	return;
}

void WadfileFree(void)
{
	fclose(wad_i.handle);
	free(wad_i.info->data);
	free(wad_i.info);
	free(wad_i.pathname);
}

/*
   =============================================================================
 */

int numLumps(void)
{
	return wad_i.info->count;
}

int lumpsize(int lump)
{
	lumpinfo_t	*inf;

	inf = wad_i.info->data;
	inf += lump;
	return inf->size;
}

int lumpstart(int lump)
{
	lumpinfo_t	*inf;

	inf = wad_i.info->data;
	inf += lump;
	return inf->filepos;
}

char const *lumpname(int lump)
{
	lumpinfo_t	*inf;

	inf = wad_i.info->data;
	inf += lump;
	return inf->name;
}

/*
   ================
   =
   = lumpNamed:
   =
   ================
 */
int lumpNamed(char const *name)
{
	lumpinfo_t	*inf;
	int		i,
			count;
	char		name8[9];
	int		v1,
			v2;

	/* make the name into two integers for easy compares */
	memset(name8, 0, 9);
	if (strlen(name) < 9)
		strncpy(name8, name, 9);
	for (i = 0; i < 9; i++)
		name8[i] = toupper(name8[i]);  /* case insensitive */

	v1 = *(int *)name8;
	v2 = *(int *)&name8[4];


	/* scan backwards so patch lump files take precedence */
	count = wad_i.info->count;
	for (i = count - 1; i >= 0; i--)
	{
		inf = wad_i.info->data;
		inf += i;
		if (*(int *)inf->name == v1 && *(int *)&inf->name[4] == v2)
			return i;
	}
	return -1;
}

/*
   ================
   =
   = loadLump:
   =
   ================
 */
void *loadLump(int lump)
{
	lumpinfo_t	*inf;
	byte		*buf;

	inf = wad_i.info->data;
	inf += lump;
	buf = (byte *) malloc(inf->size);
	fseek(wad_i.handle, inf->filepos, SEEK_SET);
	fread(buf, inf->size, 1, wad_i.handle);

	return buf;
}

void *loadLumpNamed(char const *name)
{
	return loadLump(lumpNamed(name));
}

/*
   ============================================================================
 */

/*
   ================
   =
   = addName:data:size:
   =
   ================
 */
void addName(char const *name, void *data, int size)
{
	int		i;
	lumpinfo_t	*newlump;

	newlump = (lumpinfo_t *) wad_i.info->data + wad_i.info->count;

	wad_i.dirty = true;
	memset(newlump->name, 0, sizeof(newlump->name));
	strncpy(newlump->name, name, 8);
	for (i = 0; i < 8; i++)
		newlump->name[i] = toupper(newlump->name[i]);
	fseek(wad_i.handle, 0, SEEK_END);
	newlump->filepos = ftell(wad_i.handle);
	newlump->size = size;

	fwrite(data, size, 1, wad_i.handle);

	wad_i.info->count += 1;
	wad_i.info->data = (lumpinfo_t *) realloc(wad_i.info->data,
		(wad_i.info->count + 1) * sizeof(lumpinfo_t));

	return;
}

/*
   ================
   =
   = writeDirectory:
   =
   char            identification[4];              // should be IWAD
   int             numlumps;
   int             infotableofs;
   ================
 */
void writeDirectory(void)
{
	wadinfo_t	wad;
	int		i;
	int		count;
	lumpinfo_t	*inf;

	/* write the directory */
	count = wad_i.info->count;
	inf = wad_i.info->data;

	for (i = 0; i < count; i++)
	{
		inf[i].filepos = bswapl(inf[i].filepos);
		inf[i].size = bswapl(inf[i].size);
	}

	fseek(wad_i.handle, 0, SEEK_END);
	wad.infotableofs = ftell(wad_i.handle);
        wad.infotableofs = bswapl(wad.infotableofs);

	fwrite(inf, sizeof(lumpinfo_t), count, wad_i.handle);

	for (i = 0; i < count; i++)
	{
		inf[i].filepos = bswapl(inf[i].filepos);
		inf[i].size = bswapl(inf[i].size);
	}

	/* write the header */
	strncpy(wad.identification, "PWAD", 4);
	wad.numlumps = bswapl(wad_i.info->count);

	fseek(wad_i.handle, 0, SEEK_SET);
	fwrite(&wad, sizeof(wad), 1, wad_i.handle);

	return;
}
