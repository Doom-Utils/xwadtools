/*
   WAD_DWD.C
   (c) 1994 Ron Rossbach (ej070@cleveland.freenet.edu)

   This program converts an ordinary WAD file into a slightly extended version
   of a DWD file.  The DWD file can then be used as input to IDBSP.

   Use and distribution of this code permitted according to the terms of the GNU
   General Public License.

   version 1.0.1 by Antony Suter <antony@werple.apana.org.au>
      these fixes and modification (c) 1994 by Antony Suter

   version 2.0.0 by Udo Munk <munkudo@aol.com>, see README for details.
 */

#include "waddwd.h"
#include "sysdep.h"
#include "swapfunc.h"

/*
   ===============
   =
   = main
   =
   ===============
 */
int main(int argc, char *argv[])
{
	char		dwd_name[81];
	char		wad_name[81];
	char		tmp[9];
	FILE		*dwd,
			*wad;
	int		i;
	long		numthings,
			numvertexes,
			numlines,
			numsides,
			numsectors;
	int		episode,
			level;
	mapvertex_t	*vertexes = NULL;
	mapsidedef_t	*sidedefs = NULL;
	maplinedef_t	*linedefs = NULL;
	mapsector_t	*sectors = NULL;
	mapthing_t	*things = NULL;
	lumpinfo_t	*lumps = NULL;
	wadinfo_t	wadheader;

	printf("** WAD_DWD - a WAD->DWD converter for DOOM - version 2.0.0 **\n"
		"(c) 1994 Ron Rossbach (ej070@cleveland.freenet.edu)\n"
		"Version 1.0.1 by Antony Suter <antony@werple.apana.org.au>\n"
		"Version 2.0.0 by Udo Munk <munkudo@aol.com>\n\n"
		"DOOM is a registered trademark of id Software, Inc.\n"
		"See the accompanying README for terms of use, distribution, etc.\n");

	if (argc < 2 || argc > 3)
	{
		printf("Usage: \"waddwd wadfile [dwdfile]\"\n"
		       "tmp.dwd is the default if dwdfile is not specified.\n");
		Error("Exiting....");
	}

	strcpy(wad_name, argv[1]);

	if (argc == 3)
		strcpy(dwd_name, argv[2]);
	else
		strcpy(dwd_name, "tmp.dwd");

	if ((wad = fopen(wad_name, "rb")) == NULL)
		Error("Cannot open WAD file");

	if ((dwd = fopen(dwd_name, "w")) == NULL)
	{
		fclose(wad);
		Error("Cannot open DWD file");
	}

	/* Read the WAD header */
	fread(&wadheader, sizeof(wadinfo_t), 1, wad);
        wadheader.infotableofs = bswapl(wadheader.infotableofs);
        wadheader.numlumps = bswapl(wadheader.numlumps);
	fseek(wad, wadheader.infotableofs, SEEK_SET);

	printf("WAD file: %s\nDWD file: %s\n", wad_name, dwd_name);

	/* Read the WAD directory */
	lumps = ReadWAD(wad, wadheader.numlumps, sizeof(lumpinfo_t));
        swap_lump_list(lumps,wadheader.numlumps);

	/* Process directory entries */
	WriteHeader(dwd);
	for (i = 0; i < wadheader.numlumps; i++)
	{
		strncpy(tmp, lumps[i].name, 8);
		tmp[8] = '\0';

		if (strlen(tmp) <= 0)
			continue;

		/* Handle map levels, Doom */
		if (sscanf(tmp, "E%dM%d", &episode, &level) == 2)
		{
			printf("Writing Level: %s\n", tmp);
			things = (mapthing_t *) ReadStorage(wad, &lumps[++i],
				&numthings, sizeof(mapthing_t));
                        swap_thing_list(things, numthings);
			linedefs = (maplinedef_t *) ReadStorage(wad,
				&lumps[++i], &numlines, sizeof(maplinedef_t));
                        swap_linedef_list(linedefs, numlines);
			sidedefs = (mapsidedef_t *) ReadStorage(wad,
				&lumps[++i], &numsides, sizeof(mapsidedef_t));
                        swap_sidedef_list(sidedefs, numsides);
			vertexes = (mapvertex_t *) ReadStorage(wad, &lumps[++i],                        
				&numvertexes, sizeof(mapvertex_t));
                        swap_vertex_list(vertexes, numvertexes);
                        while( strncmp(lumps[i].name, "SECTORS", 7 ) != 0 )
                          i++;
			sectors = (mapsector_t *) ReadStorage(wad, &lumps[i],
				&numsectors, sizeof(mapsector_t));
                        swap_sector_list(sectors, numsectors);
			i += 2;
			fprintf(dwd, "\nlevel:E%dM%d\n", episode, level);
			WriteLines(dwd, numlines, linedefs, vertexes,
				sidedefs, sectors);
			WriteThings(dwd, numthings, things);
			free(vertexes);
			free(sidedefs);
			free(linedefs);
			free(sectors);
		}
		/* Handle map levels, Doom][ */
		else if (sscanf(tmp, "MAP%d", &level) == 1)
		{
			printf("Writing Level: %s\n", tmp);
			things = (mapthing_t *) ReadStorage(wad, &lumps[++i],
				&numthings, sizeof(mapthing_t));
                        swap_thing_list(things, numthings);
			linedefs = (maplinedef_t *) ReadStorage(wad,
				&lumps[++i], &numlines, sizeof(maplinedef_t));
                        swap_linedef_list(linedefs, numlines);
			sidedefs = (mapsidedef_t *) ReadStorage(wad,
				&lumps[++i], &numsides, sizeof(mapsidedef_t));
                        swap_sidedef_list(sidedefs, numsides);
			vertexes = (mapvertex_t *) ReadStorage(wad, &lumps[++i],
				&numvertexes, sizeof(mapvertex_t));
                        swap_vertex_list(vertexes, numvertexes);
                        while( strncmp(lumps[i].name, "SECTORS", 7 ) != 0 )
                          i++;
			sectors = (mapsector_t *) ReadStorage(wad, &lumps[i],
				&numsectors, sizeof(mapsector_t));
                        swap_sector_list(sectors, numsectors);
			i += 2;
			fprintf(dwd, "\nlevel:MAP%02d\n", level);
			WriteLines(dwd, numlines, linedefs, vertexes, sidedefs,
				sectors);
			WriteThings(dwd, numthings, things);
			free(vertexes);
			free(sidedefs);
			free(linedefs);
			free(sectors);
		}
		/* Other Resources */
		else
		{
			printf("Writing Resource: %s\n", tmp);
			fprintf(dwd, "\n%s :%ld\n", tmp, lumps[i].size);
			ExtractResource(wad, &lumps[i]);
		}
	}

	free(lumps);
	fclose(wad);
	fclose(dwd);

	return 0;
}

/*
   ===============
   =
   = progress
   =
   = provides some visual feedback for the user....
   =
   ===============
 */
void progress(void)
{
	char			*s = "/-\\|/-\\|";
	static unsigned char	pcnt = 0;

	if ((pcnt & 15) == 0)
	{
		printf("%c\b", s[((pcnt) / 16) & 7]);
		fflush(stdout);
	}
	pcnt++;
}

/*
   ====================
   =
   = Error
   =
   = Displays an error message and exits program
   =
   ====================
 */
void Error(char *error, ...)
{
	va_list	argptr;

	va_start(argptr, error);
	vprintf(error, argptr);
	va_end(argptr);
	printf("\n");
	exit(1);
}

void *SafeCalloc(unsigned num, size_t size)
{
	void	*ret = (void *)calloc(num, size);

	if (!ret)
		Error("SafeCalloc: Failed to allocate %u of %u bytes", num,
			size);

	return ret;
}
