/*
   IDBSP.C
   (c) 1994 Ron Rossbach (ej070@cleveland.freenet.edu)

   This program is a mostly direct port of id's BSP compiler to MS-DOS.  In the
   words of John Carmack - "This code was written and evolved ... so it's
   probably not the cleanest thing in the world."

   Inquiries, comments, etc. are always welcome at the above address.

   Use and distribution of this code permitted according to the terms of the
   GNU General Public License.  Please give credit where credit is due (namely
   to id for releasing the code in the first place!)

   Please do not use this code to develop modified levels for the shareware
   version of DOOM.

   version 1.0.1 by Antony Suter <antony@werple.apana.org.au>
      these fixes and modifications (c) 1994 by Antony Suter.

   version 2.0.0 by Udo Munk <munkudo@aol.com>, see README for details.
   version 2.0.1 fixes some big endian bugs
 */

#include "idbsp.h"

WADFILE		wad_i;
static char	*behfn;
int		maptype;
map_format	mapformat;
int		qflag;
int		sflag;

void BSPMain(FILE *pDWD, boolean overwrite);
void BSPMain2(FILE *pDWD, boolean overwrite);

void usage(void)
{
	printf("Usage: idbsp [-qs] [-b behavior.aco] infile [wadfile]\n"
		"Default is tmp.wad if wadfile not specified\n"
		"\t-q: quit - suppress informational output\n"
		"\t-s: don't split lines with tag >= 900\n"
		"\t-b: use file behavior.aco as BEHAVIOR lump for\n"
		"\t    a Hexen WAD\n");
	Error("Exiting....");
}

int main(int argc, char **argv)
{
	char		szDWDName[81],
			szWADName[81];
	FILE		*pDWD;
	boolean		overwrite;
	char		*p;

	/* process options */
	while ((--argc > 0) && (**++argv == '-'))
	{
		for (p = *argv+1; *p != '\0'; p++)
		{
			switch (*p)
			{
				case 'q':
					qflag++;
					break;

				case 's':
					sflag++;
					break;

				case 'b':
					behfn = argv[1];
					argv++;
					argc--;
					break;

				default:
					usage();
			}
		}
	}

	if (!qflag)
	    printf("** IDBSP - id's node builder for DOOM, version 2.0.1 **\n"
		"DOS port by Ron Rossbach (ej070@cleveland.freenet.edu)\n"
		"Version 1.0.1 by Antony Suter <antony@werple.apana.org.au>\n"
		"Version 2.0.x by Udo Munk <munkudo@aol.com>\n\n"
		"DOOM is a registered trademark of id Software, Inc.\n"
		"ID will not provide technical support for this program.\n"
		"See the accompanying README for terms of use, distribution, etc.\n");

	if (argc < 1 || argc > 2)
		usage();

	strcpy(szDWDName, argv[0]);

	if (argc == 2)
		strcpy(szWADName, argv[1]);
	else
		strcpy(szWADName, "tmp.wad");

	if (!qflag)
		printf("In File: %s\nWAD File: %s\n", szDWDName, szWADName);

	if ((pDWD = fopen(szDWDName, "r")) == NULL)
	{
		printf("Cannot open infile\n");
		return 1;
	}

#if 0
	if ((wad_i.handle = fopen(szWADName, "rb+")) == NULL)
	{
		wad_i.handle = fopen(szWADName, "wb");
		if (wad_i.handle == NULL)
			Error("Cannot open WAD file %s", szWADName);
		overwrite = true;
	}
	else
	{
		printf("WAD file %s already exists.  Overwrite? (y/n): ",
		       szWADName);
		if (toupper(getchar()) == 'Y')
		{
			overwrite = true;
			fclose(wad_i.handle);
			wad_i.handle = fopen(szWADName, "wb");
		}
		else
			overwrite = false;
	}
#endif

	wad_i.handle = fopen(szWADName, "wb");
	if (wad_i.handle == NULL)
		Error("Cannot open WAD file %s", szWADName);
	overwrite = true;

	wad_i.pathname = (char *)SafeMalloc(strlen(szWADName) + 1);

	if (wad_i.pathname == NULL)
		Error("wad_i.pathname: malloc");
	strcpy(wad_i.pathname, szWADName);

	/* get file extension */
	p = strrchr(szDWDName, '.') + 1;
	/* process dependend on extension, either dwd format or map format */
	if (!strcmp(p, "dwd"))
	{
		mapformat = format_dwd;
		BSPMain(pDWD, overwrite);
	}
	else if (!strcmp(p, "map"))
	{
		mapformat = format_map;
		BSPMain2(pDWD, overwrite);
	}
	else
		Error("unknown input file type");

	fclose(pDWD);
	fclose(wad_i.handle);

	return 0;
}

/* process input file in DWD format */
void BSPMain(FILE *pDWD, boolean overwrite)
{
	int	version,
		episode,
		level;
	long	size;
	char	tmp[81],
		tmpname[81],
		dirname[9];

	printf("overwrite = %d\n", overwrite);
	if (overwrite)
		initNew();
	else
		initFromFile();

	if (!fscanf(pDWD, "WorldServer version %d\n", &version) || version != 4)
		Error("LoadDoomMap: not a version 4 doom map");

	if (!qflag)
		printf("Loading version 4 doom map\n");

	while (fgets(tmp, 81, pDWD) != NULL)
	{
		/* Handle Doom map level */
		if (sscanf(tmp, "level:E%dM%d", &episode, &level) == 2)
		{
			sprintf(tmp, "E%dM%d", episode, level);
			if (!qflag)
				printf("Building Map Level %s\n", tmp);
			addName(tmp, 0, 0);
			if (!qflag)
				printf("Loading DWD file.....\n");
			LoadDoomMap(pDWD);
			if (!qflag)
				printf("Building BSP....\n");
			DrawMap();
			BuildBSP();
			if (!qflag)
				printf("Saving WAD file.....\n");
			SaveDoomMap();
			SaveBlocks();
			/* Free storage */
			FreeGlobalStorage();
		}
		/* Handle Doom][ map level */
		else if (sscanf(tmp, "level:MAP%d", &level) == 1)
		{
			sprintf(tmp, "MAP%02d", level);
			if (!qflag)
				printf("Building Map Level %s\n", tmp);
			addName(tmp, 0, 0);
			if (!qflag)
				printf("Loading DWD file.....\n");
			LoadDoomMap(pDWD);
			if (!qflag)
				printf("Building BSP....\n");
			DrawMap();
			BuildBSP();
			if (!qflag)
				printf("Saving WAD file.....\n");
			SaveDoomMap();
			SaveBlocks();
			/* Free storage */
			FreeGlobalStorage();
		}
		/* Handle other resources */
		if (sscanf(tmp, "%s :%ld", dirname, &size) == 2)
		{
			strcpy(tmp, dirname);
			strcpy(tmpname, tmp);
			strcat(tmpname, ".lmp");
			if (!qflag)
				printf("Adding resource %s\n", tmpname);
			AddFromFile(tmp, size, tmpname);
		}
	}

	if (!qflag)
		printf("Writing directory.....\n");
	writeDirectory();

	return;
}

/* process input file in MAP format */
void BSPMain2(FILE *file, boolean overwrite)
{
	char	keyword[20];
	char	gametype[20];
	int	episode;
	int	level;
	char	mapname[20];

	if (!qflag)
		printf("overwrite = %d\n", overwrite);
	if (overwrite)
		initNew();
	else
		initFromFile();

	allowcomment(file);
	fscanf(file, " %s ", &keyword[0]);

	if (!strcmp(keyword, "LEVEL_START"))
	{
		fscanf(file, " %d %d %d %s ", &episode, &level, &maptype,
			&gametype[0]);
	}
	else
		Error("LEVEL_START marker missing");

	if (episode == 0)
		sprintf(&mapname[0], "MAP%02d", level);
	else
		sprintf(&mapname[0], "E%dM%d", episode, level);

	if (!qflag)
		printf("Building Map Level %s for %s\n", &mapname[0],
			&gametype[0]);
	addName(&mapname[0], 0, 0);
	if (!qflag)
		printf("Loading MAP file...\n");
	LoadMap(file);
	if (!qflag)
		printf("Building BSP...\n");
	DrawMap();
	BuildBSP();
	if (!qflag)
		printf("Saving WAD file...\n");
	SaveDoomMap();
	SaveBlocks();
	SaveBehavior(behfn);
	FreeGlobalStorage();
	if (!qflag)
		printf("Writing directory...\n");
	writeDirectory();
}

void AddFromFile(char *resname, int size, char *fname)
{
	void		*p;
	FILE		*f;
	struct stat	fileinfo;

	if ((f = fopen(fname, "rb")) == NULL)
		Error("AddFromFile: could not open file %s", fname);

	if (!size)
	{
		stat(fname, &fileinfo);
		size = fileinfo.st_size;
	}

	p = (void *)SafeMalloc(size);
	fread(p, size, 1, f);
	addName(resname, p, size);
	fclose(f);
	free(p);
}

void FreeGlobalStorage(void)
{
	free(secstore_i->data);
	free(linestore_i->data);
	free(thingstore_i->data);
	free(secstore_i->data);
	free(mapvertexstore_i->data);
	free(subsecstore_i->data);
	free(maplinestore_i->data);
	free(nodestore_i->data);
	free(mapthingstore_i->data);
	free(ldefstore_i->data);
	free(sdefstore_i->data);

	free(secstore_i);
	free(linestore_i);
	free(thingstore_i);
	free(mapvertexstore_i);
	free(subsecstore_i);
	free(maplinestore_i);
	free(nodestore_i);
	free(mapthingstore_i);
	free(ldefstore_i);
	free(sdefstore_i);
}

void progress(void)
{
	char			*s = "/-\\|/-\\|";
	static unsigned char	pcnt = 0;

	if (!qflag)
		return;

	if ((pcnt & 15) == 0)
	{
		printf("%c\b", s[((pcnt) / 16) & 7]);
		fflush(stdout);
	}
	pcnt++;
}

void Error(char *error, ...)
{
	va_list	argptr;

	va_start(argptr, error);
	vprintf(error, argptr);
	va_end(argptr);
	printf("\n");
	exit(1);
}

void *SafeMalloc(unsigned size)
{
	void	*ret = (void *)malloc(size);

	if (!ret)
		Error("SafeMalloc: Failed to allocate %u bytes", size);

	return ret;
}

void *SafeCalloc(unsigned num, unsigned size)
{
	void	*ret = (void *)calloc(num, size);

	if (!ret)
		Error("SafeCalloc: Failed to allocate %u of %u bytes",
		      num, size);

	return ret;
}

void *SafeRealloc(void *p, unsigned size)
{
	void	*ret = (void *)realloc(p, size);

	if (!ret)
		Error("SafeRealloc: Failed to allocate %u bytes", size);

	return ret;
}
