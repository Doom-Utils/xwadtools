/* WADLC.C */
/* A simple program to make custom Doom levels.
   Temporary beta version only - no support guaranteed!
   Author: Stefan Gustavson (stefang@isy.liu.se) 1994
   The entries SEGS, SSECTORS, NODES, REJECT and BLOCKMAP
   are left empty, but they are created in order not to
   confuse level editors and the program WAD_DWD used as
   a preprocessor to IDBSP.
*/

/*
 * AYM 1999-05-23 : Made wadlc pipeable (input file "-" means
 * stdin and output file "-" means stdout).
 *
 * UM 1999-06-26
 * - raised size of the thing, linedef, ... arrays to max possible size
 *
 * UM 1999-06-28
 * - fixed warning from GNU C/EGCS
 *
 * UM 1999-06-29
 * - reformated a bit, to be able to work better with the source
 * - use wad structures from wad.h in the include directory, removed
 *   the unused ones from wadlc.h
 * - replaced all magic number voodoo with sizeof(some structure) logic
 * - then added support for Hexen map type and if so, write Hexen wad
 *   with empty BEHAVIOR lump
 *
 * UM 1999-07-02
 * - read and ignore the game type in LEVEL_START
 *
 * UM 1999-07-08
 * - a BEHAVIOR lump with a size of 0 bytes crashes Hexen engines. Write
 *   one which has the memory footprint of an empty compiled script source.
 */

#include <stdio.h>
#include <string.h>
#include "sysdep.h"
#include "wad.h"
#include "wadlc.h"

static int quiet = 0;  /* No messages to stdout AT ALL */

void addentry(int *pos, int length, const char *name, FILE *wadfile)
{
  static char entryname8[8];

  putlong(*pos, wadfile);
  putlong(length, wadfile);
  *pos += length;
  strncpy(entryname8, name, 8);
  fwrite(entryname8, 8, 1, wadfile);
  if (!quiet)
    printf("Adding entry %s (%d bytes)\n", name, length);
}

int findmatch(const char *name, const char *table, int numentries, int length)
{
  int i = 0;

  while (strncmp(name, table, 8) && (i < numentries))
  {
    i++;
    table += length;
  }
  if (i == numentries)
  {
    fprintf(stderr,"Couldn't match name: %s", name);
    exit(-1);
  }
  return(i);
}

void allowcomment(FILE *infile)
{
  while (ungetc(getc(infile), infile) == '#')
    while (getc(infile) != '\n');
}

int main(int argc, char *argv[])
{
  FILE		*infile, *wadfile;
  char		keyword[20];
  char		vertexname[20], sectorname[20];
  char		fromvertexname[20], tovertexname[20];
  int		ox, oy, x, y, z;
  int		numentries, reslength;
  int		i;
  int		episode, level, maptype = 0;
  char		gametype[20];
  int		lastitem;
  int		floorh, ceilingh, brightness, special, tag, type, attrs, angle;
  int		arg1, arg2, arg3, arg4, arg5;
  char		floort[20], ceilingt[20];
  char		uppert[20], lowert[20], normalt[20];
  char		thingname[20];
  int		tid;
  int		fromvertexindex, tovertexindex;
  int		rightsidedefindex, leftsidedefindex;
  int		sectorindex;
  char		behavior[16] = { 'A', 'C', 'S', '\0', 0x08, '\0', '\0', '\0',
				 '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'
			       };
  
  w_vertex_t	vertices[32766];
  w_sector_t	sectors[8192];
  sidedef_t	sidedefs[32766];
  linedef_t	linedefs[16383];
  linedef2_t	linedefs2[16383];
  things_t	things[32767];
  things2_t	things2[32767];
  int		numvertices = 0, numsectors = 0, numsidedefs = 0,
		numlinedefs = 0, numthings = 0;

  char		entryname[9];
  int		entrypos;

  if (argc != 3)
  {
    fprintf(stderr,"Usage: %s infile outfile\n", argv[0]);
    exit(-1);
  }
  if (!strcmp(argv[1], "-"))
    infile = stdin;
  else
    infile = fopen(argv[1], "r");
  if (infile == NULL)
  {
    fprintf(stderr,"Unable to open input file %s\n", argv[1]);
    exit(-1);
  }
  if (!strcmp(argv[2], "-"))
    wadfile = stdout;
  else
    wadfile = fopen(argv[2], "wb");
  if (wadfile == NULL)
  {
    fprintf(stderr,"Unable to create output file %s\n", argv[2]);
    exit(-1);
  }
  if (wadfile == stdout)
    quiet = 1;
  allowcomment(infile);
  fscanf(infile, " %s ", keyword);

  if (!strcmp(keyword, "LEVEL_START"))
  {
    fscanf(infile, " %d %d %d %s ", &episode, &level, &maptype, &gametype[0]);
    allowcomment(infile);
    fscanf(infile, " %s ", keyword);
  }
  else
  {
    fprintf(stderr,"LEVEL_START marker missing!\n");
    exit(-1);
  }

  if (!strcmp(keyword, "VERTEXES_START") |
      !strcmp(keyword, "VERTICES_START"))
  {
    /* Process vertices until VERTEXES_END is found */
    lastitem = 0;
    numvertices = 0;
    while (!lastitem)
    {
      allowcomment(infile);
      fscanf(infile, " %s ", vertexname);
      if (!strcmp(vertexname, "VERTEXES_END") |
	  !strcmp(vertexname, "VERTICES_END"))
		lastitem = 1;
      else
      {
	fscanf(infile, " : %d %d ", &x, &y);
	allowcomment(infile);
	vertices[numvertices].x = x;
	vertices[numvertices].y = y;
	strncpy(vertices[numvertices].name, vertexname, 8);
	numvertices++;
      }
    }
    fscanf(infile, " %s ", keyword);
  }
  else
  {
    fprintf(stderr,"VERTEXES_START marker missing!\n");
    exit(-1);
  }
  
  if (!strcmp(keyword, "SECTORS_START"))
  {
    /* Process sectors until SECTORS_END is found */
    lastitem = 0;
    numsectors = 0;
    while (!lastitem)
    {
      allowcomment(infile);
      fscanf(infile, " %s ", sectorname);
      if (!strcmp(sectorname, "SECTORS_END"))
	lastitem = 1;
      else
      {
	fscanf(infile, " : %d %d %s %s %d %d %d ", &floorh, &ceilingh,
	       floort, ceilingt, &brightness, &special, &tag);
	allowcomment(infile);
	sectors[numsectors].floorheight = floorh;
	sectors[numsectors].ceilingheight = ceilingh;
	sectors[numsectors].brightness = brightness;
	sectors[numsectors].special = special;
	sectors[numsectors].tag = tag;
	strncpy(sectors[numsectors].name, sectorname, 8);
	strncpy(sectors[numsectors].floortexture, floort, 8);
	strncpy(sectors[numsectors].ceilingtexture, ceilingt, 8);
	numsectors++;
      }
    }
    fscanf(infile, " %s ", keyword);
  }
  else
  {
    fprintf(stderr,"SECTORS_START marker missing!\n");
    exit(-1);
  }

  if (!strcmp(keyword, "LINEDEFS_START"))
  {
    /* Process lines until LINEDEFS_END is found */
    lastitem = 0;
    numlinedefs = 0;
    while (!lastitem)
    {
      allowcomment(infile);
      fscanf(infile, " %s ", fromvertexname);
      if (!strcmp(fromvertexname, "LINEDEFS_END"))
	lastitem = 1;
      else
      {
	if (!maptype)
	{
	  /* Doom map typ */
	  fscanf(infile, " %s : %d %d %d ", tovertexname, &attrs, &type, &tag);
	  allowcomment(infile);
	  fromvertexindex = findmatch(fromvertexname, (char*)vertices,
			              numvertices, sizeof(w_vertex_t));
	  tovertexindex = findmatch(tovertexname, (char*)vertices,
				    numvertices, sizeof(w_vertex_t));
	}
	else
	{
	  /* Hexen map type */
	  fscanf(infile, " %s : %d %d %d %d %d %d %d ",
			  tovertexname, &attrs, &type, &arg1, &arg2, &arg3,
			  &arg4, &arg5);
	  allowcomment(infile);
	  fromvertexindex = findmatch(fromvertexname, (char*)vertices,
				      numvertices, sizeof(w_vertex_t));
	  tovertexindex = findmatch(tovertexname, (char*)vertices,
				    numvertices, sizeof(w_vertex_t));
	}
	/* Read one or two SIDEDEFS */
	fscanf(infile, " %s %d %d %s %s %s ", sectorname, &ox, &oy,
	       uppert, lowert, normalt);
	allowcomment(infile);
	sectorindex = findmatch(sectorname, (char*)sectors,
				numsectors, sizeof(w_sector_t));
	sidedefs[numsidedefs].sector = sectorindex;
	sidedefs[numsidedefs].x = ox;
	sidedefs[numsidedefs].y = oy;
	strncpy(sidedefs[numsidedefs].u_texture, uppert, 8);
	strncpy(sidedefs[numsidedefs].l_texture, lowert, 8);
	strncpy(sidedefs[numsidedefs].m_texture, normalt, 8);
	rightsidedefindex = numsidedefs;
	numsidedefs++;
	fscanf(infile, "%s", sectorname);
	if (!strcmp(sectorname, "-"))
	{
	  allowcomment(infile);
	  leftsidedefindex = -1;
	}
	else
	{
	  fscanf(infile, " %d %d %s %s %s ", &ox, &oy, uppert, lowert, normalt);
	  allowcomment(infile);
	  sectorindex = findmatch(sectorname, (char*)sectors,
				  numsectors, sizeof(w_sector_t));
	  sidedefs[numsidedefs].sector = sectorindex;
	  sidedefs[numsidedefs].x = ox;
	  sidedefs[numsidedefs].y = oy;
	  strncpy(sidedefs[numsidedefs].u_texture, uppert, 8);
	  strncpy(sidedefs[numsidedefs].l_texture, lowert, 8);
	  strncpy(sidedefs[numsidedefs].m_texture, normalt, 8);
	  leftsidedefindex = numsidedefs;
	  numsidedefs++;
	}
	if (!maptype)
	{
	  /* Doom map type */
	  linedefs[numlinedefs].v_from = fromvertexindex;
	  linedefs[numlinedefs].v_to = tovertexindex;
	  linedefs[numlinedefs].flags = attrs;
	  linedefs[numlinedefs].type = type;
	  linedefs[numlinedefs].tag = tag;
	  linedefs[numlinedefs].r_side = rightsidedefindex;
	  linedefs[numlinedefs].l_side = leftsidedefindex;
	}
	else
	{
	  /* Hexen map type */
	  linedefs2[numlinedefs].v_from = fromvertexindex;
	  linedefs2[numlinedefs].v_to = tovertexindex;
	  linedefs2[numlinedefs].flags = attrs;
	  linedefs2[numlinedefs].type = type;
	  linedefs2[numlinedefs].arg1 = arg1;
	  linedefs2[numlinedefs].arg2 = arg2;
	  linedefs2[numlinedefs].arg3 = arg3;
	  linedefs2[numlinedefs].arg4 = arg4;
	  linedefs2[numlinedefs].arg5 = arg5;
	  linedefs2[numlinedefs].r_side = rightsidedefindex;
	  linedefs2[numlinedefs].l_side = leftsidedefindex;
	}
	numlinedefs++;
      }
    }
    fscanf(infile, " %s ", keyword);
  }
  else
  {
    fprintf(stderr,"LINEDEFS_START marker missing!\n");
    exit(-1);
  }

  if (!strcmp(keyword, "THINGS_START"))
  {
    /* Process things until THINGS_END is found */
    lastitem = 0;
    numthings = 0;
    while (!lastitem)
    {
      allowcomment(infile);
      fscanf(infile, " %s ", thingname);
      if (!strcmp(thingname, "THINGS_END"))
	lastitem = 1;
      else
      {
	sscanf(thingname, "%d", &type);
	if (!maptype)
	{
	  /* Doom map type */
	  fscanf(infile, " : %d %d %d %d ", &x, &y, &angle, &attrs);
	  allowcomment(infile);
	  things[numthings].type = type;
	  things[numthings].x = x;
	  things[numthings].y = y;
	  things[numthings].angle = angle;
	  things[numthings].flags = attrs;
	}
	else
	{
	  /* Hexen map type */
	  fscanf(infile, " : %d %d %d %d %d %d %d %d %d %d %d %d ", &tid,
			  &x, &y, &z, &angle, &attrs, &special, &arg1, &arg2,
			  &arg3, &arg4, &arg5);
	  allowcomment(infile);
	  things2[numthings].thingid = tid;
	  things2[numthings].x = x;
	  things2[numthings].y = y;
	  things2[numthings].z = z;
	  things2[numthings].angle = angle;
	  things2[numthings].type = type;
	  things2[numthings].flags = attrs;
	  things2[numthings].special = special;
	  things2[numthings].arg1 = arg1;
	  things2[numthings].arg2 = arg2;
	  things2[numthings].arg3 = arg3;
	  things2[numthings].arg4 = arg4;
	  things2[numthings].arg5 = arg5;
	}
	numthings++;
      }
    }
    fscanf(infile, " %s ", keyword);
  }
  else
  {
    fprintf(stderr,"THINGS_START marker missing!\n");
    exit(-1);
  }

  if (infile != stdin)
    fclose(infile);

  /* Concatenate all the data into the final PWAD file */

  /* Header */
  fprintf(wadfile,"PWAD");
  if (!maptype)
  {
    /* Doom map type */
    numentries = 11;
    putlong(numentries, wadfile);
    reslength = numthings * sizeof(things_t) +
		numlinedefs * sizeof(linedef_t) +
		numsidedefs * sizeof(sidedef_t) +
		numvertices * sizeof(vertex_t) +
		numsectors * sizeof(sector_t) +
		sizeof(wadinfo_t);
  }
  else
  {
    /* Hexen map type */
    numentries = 12;
    putlong(numentries, wadfile);
    reslength = numthings * sizeof(things2_t) +
		numlinedefs * sizeof(linedef2_t) +
		numsidedefs * sizeof(sidedef_t) +
		numvertices * sizeof(vertex_t) +
		numsectors * sizeof(sector_t) +
		sizeof(behavior) +
		sizeof(wadinfo_t);
  }
  putlong(reslength, wadfile);

  /* ExMy - start at position 12 (zero length entry) */

  /* THINGS - start at position 12 */
  for (i = 0; i < numthings; i++)
  {
    if (!maptype)
    {
      /* Doom map type */
      putshort(things[i].x, wadfile);
      putshort(things[i].y, wadfile);
      putshort(things[i].angle, wadfile);
      putshort(things[i].type, wadfile);
      putshort(things[i].flags, wadfile);
    }
    else
    {
      /* Hexen map type */
      putshort(things2[i].thingid, wadfile);
      putshort(things2[i].x, wadfile);
      putshort(things2[i].y, wadfile);
      putshort(things2[i].z, wadfile);
      putshort(things2[i].angle, wadfile);
      putshort(things2[i].type, wadfile);
      putshort(things2[i].flags, wadfile);
      fwrite(&things2[i].special, 1, 1, wadfile);
      fwrite(&things2[i].arg1, 1, 1, wadfile);
      fwrite(&things2[i].arg2, 1, 1, wadfile);
      fwrite(&things2[i].arg3, 1, 1, wadfile);
      fwrite(&things2[i].arg4, 1, 1, wadfile);
      fwrite(&things2[i].arg5, 1, 1, wadfile);
    }
  }
  
  /* LINEDEFS - start at position 12 + numthings * sizeof(things_t) */
  for (i = 0; i < numlinedefs; i++)
  {
    if (!maptype)
    {
      /* Doom map type */
      putshort(linedefs[i].v_from, wadfile);
      putshort(linedefs[i].v_to, wadfile);
      putshort(linedefs[i].flags, wadfile);
      putshort(linedefs[i].type, wadfile);
      putshort(linedefs[i].tag, wadfile);
      putshort(linedefs[i].r_side, wadfile);
      putshort(linedefs[i].l_side, wadfile);
    }
    else
    {
      /* Hexen map type */
      putshort(linedefs2[i].v_from, wadfile);
      putshort(linedefs2[i].v_to, wadfile);
      putshort(linedefs2[i].flags, wadfile);
      fwrite(&linedefs2[i].type, 1, 1, wadfile);
      fwrite(&linedefs2[i].arg1, 1, 1, wadfile);
      fwrite(&linedefs2[i].arg2, 1, 1, wadfile);
      fwrite(&linedefs2[i].arg3, 1, 1, wadfile);
      fwrite(&linedefs2[i].arg4, 1, 1, wadfile);
      fwrite(&linedefs2[i].arg5, 1, 1, wadfile);
      putshort(linedefs2[i].r_side, wadfile);
      putshort(linedefs2[i].l_side, wadfile);
    }
  }

  /* SIDEDEFS - start at position 12 + numthings * sizeof(things_t) +
   * numlinedefs * sizeof(linedef_t)
   */
  for (i = 0; i < numsidedefs; i++)
  {
    putshort(sidedefs[i].x, wadfile);
    putshort(sidedefs[i].y, wadfile);
    fwrite(sidedefs[i].u_texture, 8, 1, wadfile);
    fwrite(sidedefs[i].l_texture, 8, 1, wadfile);
    fwrite(sidedefs[i].m_texture, 8, 1, wadfile);	
    putshort(sidedefs[i].sector, wadfile);
  }
  
  /* VERTEXES - start at position 12 + numthings * sizeof(things_t) +
   * numlinedefs * sizeof(linedef_t) + numsidedefs * sizeof(sidedef_t)
   */
  for (i = 0; i < numvertices; i++)
  {
    putshort(vertices[i].x, wadfile);
    putshort(vertices[i].y, wadfile);
  }
  
  /* SECTORS - start at position 12 + numthings * sizeof(things_t) +
   * numlinedefs * sizeof(linedef_t) + numsidedefs * sizeof(sidedef_t) +
   * numvertices * sizeof(vertex_t)
   */
  for (i = 0; i < numsectors; i++)
  {
    putshort(sectors[i].floorheight, wadfile);
    putshort(sectors[i].ceilingheight, wadfile);
    fwrite(sectors[i].floortexture, 8, 1, wadfile);
    fwrite(sectors[i].ceilingtexture, 8, 1, wadfile);
    putshort(sectors[i].brightness, wadfile);
    putshort(sectors[i].special, wadfile);
    putshort(sectors[i].tag, wadfile);
  }
  
  /* Hexen BEHAVIOR lump */
  if (maptype)
  {
    fwrite(&behavior[0], sizeof(behavior), 1, wadfile);
  }

  /* Now, write the main directory */
  entrypos = 12;
  if (episode > 0)
      sprintf(entryname, "E%1dM%1d", episode, level);
  else
      sprintf(entryname, "MAP%02d", level);
  addentry(&entrypos, 0,				entryname,  wadfile);
  if (!maptype)
  {
    addentry(&entrypos, numthings*sizeof(things_t),	"THINGS",   wadfile);
    addentry(&entrypos, numlinedefs*sizeof(linedef_t),	"LINEDEFS", wadfile);
  }
  else
  {
    addentry(&entrypos, numthings*sizeof(things2_t),	"THINGS",   wadfile);
    addentry(&entrypos, numlinedefs*sizeof(linedef2_t),	"LINEDEFS", wadfile);
  }
  addentry(&entrypos, numsidedefs*sizeof(sidedef_t),	"SIDEDEFS", wadfile);
  addentry(&entrypos, numvertices*sizeof(vertex_t),	"VERTEXES", wadfile);
  addentry(&entrypos, 0,              			"SEGS",     wadfile);
  addentry(&entrypos, 0,              			"SSECTORS", wadfile);
  addentry(&entrypos, 0,              			"NODES",    wadfile);
  addentry(&entrypos, numsectors*sizeof(sector_t),	"SECTORS",  wadfile);
  addentry(&entrypos, 0,              			"REJECT",   wadfile);
  addentry(&entrypos, 0,              			"BLOCKMAP", wadfile);

  /* add empty BEHAVIOR for Hexen map type */
  if (maptype)
	addentry(&entrypos, sizeof(behavior),		"BEHAVIOR", wadfile);
  
  /* DONE! */
  if (wadfile != stdout)
    fclose(wadfile);
  return 0;
}
