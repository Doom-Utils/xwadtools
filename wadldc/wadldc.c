/************************************************************************/
/*      Copyright (C) 1998, 1999 by Udo Munk (munkudo@aol.com)          */
/*                                                                      */
/*      Permission to use, copy, modify, and distribute this software   */
/*      and its documentation for any purpose and without fee is        */
/*      hereby granted, provided that the above copyright notice        */
/*      appears in all copies and that both that copyright notice and   */
/*      this permission notice appear in supporting documentation.      */
/*      The author and contibutors make no representations about the    */
/*      suitability of this software for any purpose. It is provided    */
/*      "as is" without express or implied warranty.                    */
/************************************************************************/

/*
 *      Tool to decompile a level map in a WAD file
 */

/*
 * 1.1 (UM 1999-06-25)
 * - Added support for Hexen/ZDoom WAD's.
 * 1.2 (UM 1999-09-25)
 * - Merged with bug fix for big endian from Oliver Kraus
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "sysdep.h"
#include "wad.h"
#include "lump_dir.h"
#include "wadfile.h"

#define VERSION "1.2"

int	verbose;
int	has_behavior;

void usage(char *name, char *option)
{
        if (option)
            fprintf(stderr, "%s: Unknown option: %s\n", name, option);
        else
            fprintf(stderr, "%s: Missing arguments\n", name);
        fprintf(stderr, "Usage: %s [-v] [-g game] mapname wadfile mapfile\n", name);
        fprintf(stderr, "\t-v: verbose output\n");
	fprintf(stderr, "\t-g: game name, Doom, Doom2, Hexen...\n");
        exit(1);
}

void decompile_vertices(char *map, wadfile_t *wf, FILE *fp)
{
	void		*vertices;
	vertex_t	*v;
	int		size;
	int		i;

	if (verbose)
		printf("Decompiling vertices...\n");

	/* get the lump with the vertices for the map */
	if ((vertices =
		get_map_lump(wf, map, "VERTEXES", &size)) == (void *)0) {
		fprintf(stderr, "can't find VERTEXES lump for %s\n", map);
		exit(1);
	}
	
	/* write vertices start marker */
	fprintf(fp, " VERTEXES_START\n");

	/* loop over all vertices in the lump and decompile */
	v = (vertex_t *) vertices;
	for (i = 0; i < size / sizeof(vertex_t); i++) {
		swapint(&v->x);
		swapint(&v->y);
		fprintf(fp, "  V%d : %d %d\n", i, v->x, v->y);
		v++;
	}

	/* write vertices end marker */
	fprintf(fp, " VERTEXES_END\n\n");

	/* free memory */
	free(vertices);
}

void decompile_sectors(char *map, wadfile_t *wf, FILE *fp)
{
	void		*sectors;
	sector_t	*s;
	int		size;
	int		i;

	if (verbose)
		printf("Decompiling sectors...\n");

	/* get the lump with the sectors for the map */
	if ((sectors = get_map_lump(wf, map, "SECTORS", &size)) == (void *)0) {
		fprintf(stderr, "can't find SECTORS lump for %s\n", map);
		exit(1);
	}
	
	/* write sectors start marker */
	fprintf(fp, " SECTORS_START\n");

	/* loop over all sectors in the lump and decompile */
	s = (sector_t *) sectors;
	for (i = 0; i < size / sizeof(sector_t); i++) {
		swapint(&s->f_height);
		swapint(&s->c_height);
		swapint(&s->light);
		swapint(&s->flags);
		swapint(&s->tag);
		fprintf(fp, "  S%d : %d %d", i, s->f_height, s->c_height);
		fprintf(fp, " %.8s", s->f_texture);
		fprintf(fp, " %.8s", s->c_texture);
		fprintf(fp, " %d %d %d\n", s->light, s->flags, s->tag);
		s++;
	}

	/* write sectors end marker */
	fprintf(fp, " SECTORS_END\n\n");

	/* free memory */
	free(sectors);
}

void decompile_linedefs(char *map, wadfile_t *wf, FILE *fp)
{
	void		*lines;
	void		*sdefs;
	linedef_t	*l1;
	linedef2_t	*l2;
	sidedef_t	*s;
	int		size;
	int		size_sd;
	int		i;

	if (verbose)
		printf("Decompiling linedefs...\n");

	/* get the lump with the linedefs for the map */
	if ((lines = get_map_lump(wf, map, "LINEDEFS", &size)) == (void *)0) {
		fprintf(stderr, "can't find LINEDEFS lump for %s\n", map);
		exit(1);
	}

	/* and we need the sidedefs too... */
	if ((sdefs = get_map_lump(wf, map, "SIDEDEFS", &size_sd)) == (void *)0) {
		fprintf(stderr, "can't find SIDEDEFS lump for %s\n", map);
		exit(1);
	}
        
        /* 14 sep 99 Oliver Kraus */
        /* convert sidedefs before the linedef loop */
        /* this will fix a bug in the sparc port */

#ifdef BIGEND        
        s = (sidedef_t *)sdefs;
        for (i = 0; i < size_sd/sizeof(sidedef_t); i++, s++ )
        {
		swapint(&s->x);
		swapint(&s->y);
		swapint(&s->sector);
	}
#endif 
	
	/* write linedefs start marker */
	fprintf(fp, " LINEDEFS_START\n");

	if (!has_behavior)
	{
	    /* Doom or Doom2 map */

	    /* loop over all linedefs in the lump and decompile */
	    l1 = (linedef_t *) lines;
	    for (i = 0; i < size / sizeof(linedef_t); i++, l1++) {

		/* the linedef self */
		swapint(&l1->v_from);
		swapint(&l1->v_to);
		swapint(&l1->flags);
		swapint(&l1->type);
		swapint(&l1->tag);
		swapint(&l1->r_side);
		swapint(&l1->l_side);
		fprintf(fp, "  V%d V%d : %d %d %d\n", l1->v_from, l1->v_to,
			l1->flags, l1->type, l1->tag);

		/* right sidedef */
		s = (sidedef_t *)sdefs + l1->r_side;
                /* do NOT swap sidedef values here
		swapint(&s->x);
		swapint(&s->y);
		swapint(&s->sector);
                */
		fprintf(fp, "   S%d %d %d", s->sector, s->x, s->y);
		fprintf(fp, " %.8s", s->u_texture);
		fprintf(fp, " %.8s", s->l_texture);
		fprintf(fp, " %.8s\n", s->m_texture);

		/* left sidedef */
		if (l1->l_side == -1)
		  fprintf(fp, "   -\n");
		else {
		  s = (sidedef_t *)sdefs + l1->l_side;
                  /* do NOT swap sidedef values here
		  swapint(&s->x);
		  swapint(&s->y);
		  swapint(&s->sector);
                  */
		  fprintf(fp, "   S%d %d %d", s->sector, s->x, s->y);
		  fprintf(fp, " %.8s", s->u_texture);
		  fprintf(fp, " %.8s", s->l_texture);
		  fprintf(fp, " %.8s\n", s->m_texture);
		}
	    }

	}  else {

	    /* Hexen or ZDoom map */

	    /* loop over all linedefs in the lump and decompile */
	    l2 = (linedef2_t *) lines;
	    for (i = 0; i < size / sizeof(linedef2_t); i++, l2++) {

		/* the linedef self */
		swapint(&l2->v_from);
		swapint(&l2->v_to);
		swapint(&l2->flags);
		swapint(&l2->r_side);
		swapint(&l2->l_side);
		fprintf(fp, "  V%d V%d : %d %d %d %d %d %d %d\n",
			l2->v_from, l2->v_to, l2->flags, l2->type,
			l2->arg1, l2->arg2, l2->arg3, l2->arg4,
			l2->arg5);

		/* right sidedef */
		s = (sidedef_t *)sdefs + l2->r_side;
                /* do NOT swap sidedef values here
		swapint(&s->x);
		swapint(&s->y);
		swapint(&s->sector);
                */
		fprintf(fp, "   S%d %d %d", s->sector, s->x, s->y);
		fprintf(fp, " %.8s", s->u_texture);
		fprintf(fp, " %.8s", s->l_texture);
		fprintf(fp, " %.8s\n", s->m_texture);

		/* left sidedef */
		if (l2->l_side == -1)
		  fprintf(fp, "   -\n");
		else {
		  s = (sidedef_t *)sdefs + l2->l_side;
                  /* do NOT swap sidedef values here
		  swapint(&s->x);
		  swapint(&s->y);
		  swapint(&s->sector);
                  */
		  fprintf(fp, "   S%d %d %d", s->sector, s->x, s->y);
		  fprintf(fp, " %.8s", s->u_texture);
		  fprintf(fp, " %.8s", s->l_texture);
		  fprintf(fp, " %.8s\n", s->m_texture);
		}
	    }
	}

	/* write linedefs end marker */
	fprintf(fp, " LINEDEFS_END\n\n");

	/* free memory */
	free(lines);
	free(sdefs);
}

void decompile_things(char *map, wadfile_t *wf, FILE *fp)
{
	void		*things;
	things_t	*t1;
	things2_t	*t2;
	int		size;
	int		i;

	if (verbose)
		printf("Decompiling things...\n");

	/* get the lump with the things for the map */
	if ((things = get_map_lump(wf, map, "THINGS", &size)) == (void *)0) {
		fprintf(stderr, "can't find THINGS lump for %s\n", map);
		exit(1);
	}

	/* write things start marker */
	fprintf(fp, " THINGS_START\n");

	if (!has_behavior)
	{
	    /* Doom or Doom2 map */

	    /* loop over all things in the lump and decompile */
	    t1 = (things_t *) things;
	    for (i = 0; i < size / sizeof(things_t); i++, t1++) {
		swapint(&t1->x);
		swapint(&t1->y);
		swapint(&t1->angle);
		swapint(&t1->type);
		swapint(&t1->flags);
		fprintf(fp, "  %d : %d %d %d %d\n", t1->type, t1->x, t1->y,
				t1->angle, t1->flags);
	    }
	} else {
	    /* Hexen or ZDoom map */

	    /* loop over all things in the lump and decompile */
	    t2 = (things2_t *) things;
	    for (i = 0; i < size / sizeof(things2_t); i++, t2++) {
		swapint(&t2->thingid);
		swapint(&t2->x);
		swapint(&t2->y);
		swapint(&t2->z);
		swapint(&t2->angle);
		swapint(&t2->type);
		swapint(&t2->flags);
		fprintf(fp, "  %d : %d %d %d %d %d %d %d %d %d %d %d %d\n",
				t2->type, t2->thingid, t2->x, t2->y, t2->z,
				t2->angle, t2->flags, t2->special, t2->arg1,
				t2->arg2, t2->arg3, t2->arg4, t2->arg5);
	    }
	}

	/* write things end marker */
	fprintf(fp, " THINGS_END\n\n");

	/* free memory */
	free(things);
}

int main(int argc, char** argv)
{
	char		*s;
	char		*program;
	wadfile_t	*wf;
	FILE		*fp;
	char		*game = "Doom2";

	/* save program name for usage() */
	program = *argv;

	/* process options */
	while ((--argc > 0) && (**++argv == '-')) {
		for (s = *argv+1; *s != '\0'; s++) {
			switch (*s) {
			case 'v':
				verbose++;
				break;
			case 'g':
				argc--;
				argv++;
				game = *argv;
				break;
			default:
				usage(program, --s);
			}
		}
	}

	if (verbose)
		printf("%s version %s\n\n", program, VERSION);

	/* we need exactely 3 arguments */
	if (argc != 3)
		usage(program, NULL);

	/* try to open WAD file */
	if (verbose)
		printf("Opening WAD file %s...\n", *(argv+1));
	wf = open_wad(*(argv+1));

	/* try to open MAP file */
	if (verbose)
		printf("Opening map file %s...\n", *(argv+2));
	if ((fp = fopen(*(argv+2), "w")) == NULL) {
		perror("can't open map file for writing");
		exit(1);
	}

	/* check if this is a Hexen map, indicated by presence of BEHAVIOR */
	has_behavior = check_lump_name(wf, "BEHAVIOR");

	/* write comment and level start marker into map file */
	fprintf(fp, "# This map was decompiled with wadldc\n\n");
	if (!strncmp(*argv, "MAP", 3)) {
		/* Doom2, ZDoom  or Hexen map */
		if (*(*argv+3) != '0')
			fprintf(fp, "LEVEL_START 0 %s %d %s\n\n", *argv+3,
				has_behavior, game);
		else
			fprintf(fp, "LEVEL_START 0 %s %d %s\n\n", *argv+4,
				has_behavior, game);
	} else {
		/* Doom map */
		fprintf(fp, "LEVEL_START %c %c %d %s\n\n", *(*argv+1),
			*(*argv+3), has_behavior, game);
	}

	/* decompile the sections we need */
	decompile_vertices(*argv, wf, fp);
	decompile_sectors(*argv, wf, fp);
	decompile_linedefs(*argv, wf, fp);
	decompile_things(*argv, wf, fp);

	/* write level end marker into map file */
	fprintf(fp, "LEVEL_END\n");

	/* close files */
	close_wad(wf);
	fclose(fp);

	/* and done */
	return 0;
}
