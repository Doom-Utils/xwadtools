/******************************************************************************
	MODULE:		WADIO.C
	WRITTEN BY:	Robert Fenske, Jr. (rfenske@swri.edu)
	CREATED:	May  1994
	DESCRIPTION:	This module contains routines to read and write DOOM,
			HERETIC, or HEXEN related IWAD and PWAD files, and
			VERDA patch files.

			For big-endian processors like Sparcs (SunOS/Solaris)
			and PA-RISC (HP-UX), byte swapping must be done for
			each of the little-endian integer fields in a WAD file.
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "sysdep.h"
#include "dmglobal.h"

#define is_name(n,t)	(strncmp((n),(t),strlen(t)) == 0)
#define is_type(n,t)	is_name((n),mapresrc[t])

local char mapresrc[][9] = {			/* resource name in dir */
	"--------", "THINGS", "LINEDEFS", "SIDEDEFS", "VERTEXES", "SEGS",
	"SSECTORS", "NODES", "SECTORS", "REJECT", "BLOCKMAP", "BEHAVIOR"
};
local int mapresiz[] = {			/* item size in resource */
	1, sizeof(WAD_THING), sizeof(WAD_LINE),
	sizeof(WAD_SIDE), sizeof(WAD_VERT),
	sizeof(WAD_SEGS), sizeof(WAD_SSECTOR),
	sizeof(WAD_NODE), sizeof(WAD_SECTOR),
	sizeof(WAD_REJECT), sizeof(WAD_BLOCKMAP),
	sizeof(WAD_BEHAVIOR)
};
local int mapdmsiz[] = {			/* DOOM item size in rsrce */
	1, sizeof(DOOM_THING), sizeof(DOOM_LINE),
	sizeof(WAD_SIDE), sizeof(WAD_VERT),
	sizeof(WAD_SEGS), sizeof(WAD_SSECTOR),
	sizeof(WAD_NODE), sizeof(WAD_SECTOR),
	sizeof(WAD_REJECT), sizeof(WAD_BLOCKMAP),
	sizeof(WAD_BEHAVIOR)
};
local int maphxsiz[] = {			/* HEXEN item size in rsrce */
	1, sizeof(HEXEN_THING), sizeof(HEXEN_LINE),
	sizeof(WAD_SIDE), sizeof(WAD_VERT),
	sizeof(WAD_SEGS), sizeof(WAD_SSECTOR),
	sizeof(WAD_NODE), sizeof(WAD_SECTOR),
	sizeof(WAD_REJECT), sizeof(WAD_BLOCKMAP),
	sizeof(WAD_BEHAVIOR)
};


/******************************************************************************
	ROUTINE:	resource_type(entry)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	Nov. 1995
	DESCRIPTION:	This routine returns the resource type of the input
			directory entry.  A -1 is returned if the entry is
			not a map/level data entry.
******************************************************************************/
#if defined(ANSI_C)
local int resource_type(DIR_ENTRY *entry)
#else
int resource_type(entry)
DIR_ENTRY *entry;
#endif
{
  int type = -1;

  if (entry != NULL) {
    for (type = 0; type < numelm(mapresrc); type++)
      if (is_type(entry->name,type)) break;
    if (type >= numelm(mapresrc)) type = -1;
  }
  return type;
}


/******************************************************************************
	ROUTINE:	resource_update(winfo,entry,data,count)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	June 1994
	DESCRIPTION:	This routine assigns the input data to the specified
			entry.  If the entry already has data, it is freed
			first.  The changed flag is set to TRUE.
******************************************************************************/
#if defined(ANSI_C)
void resource_update(WAD_INFO *winfo, int entry, void *data,
                     long count)
#else
void resource_update(winfo,entry,data,count)
WAD_INFO *winfo;
int entry;
void *data;
long count;
#endif
{
  int type;

  if ((char *)data != winfo->data[entry]) blockfree(winfo->data[entry]);
  winfo->data[entry] = (char *)data;
  if (0 <= (type = resource_type(&winfo->dir[entry])))
    winfo->dir[entry].nbytes = count*mapresiz[type];
  else
    winfo->dir[entry].nbytes = count;
  winfo->count[entry] = count;
  winfo->changed[entry] = TRUE;
}


/******************************************************************************
	ROUTINE:	wad_bswap(winfo,resource)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	May  1994
	DESCRIPTION:	This routine swaps the bytes in the short integer
			fields of the various resources.  This is necessary
			since the data files store integers in Intel little-
			endian order, while all real systems use big-endian
			order.
******************************************************************************/
#if defined(ANSI_C)
local void wad_bswap(WAD_INFO *winfo, int resource)
#else
local void wad_bswap(winfo,resource)
WAD_INFO *winfo;
int resource;
#endif
{
#if BIGEND
  short *data = (short *)winfo->data[resource];	/* where data is */
  WAD_THING *things = (WAD_THING *)data;
  WAD_LINE *lines = (WAD_LINE *)data;
  WAD_SIDE *sides = (WAD_SIDE *)data;
  WAD_SECTOR *sects = (WAD_SECTOR *)data;
  int cnt = winfo->count[resource];
  int type;
  int d, w;

  if (data != NULL) {
    type = resource_type(&winfo->dir[resource]);
    switch (type) {
       case THINGS:				/* only swap integer fields */
        for (d = 0; d < cnt; d++) {
          things[d].id = bswapw(things[d].id);
          things[d].x = bswapw(things[d].x);
          things[d].y = bswapw(things[d].y);
          things[d].z = bswapw(things[d].z);
          things[d].angle = bswapw(things[d].angle);
          things[d].item = bswapw(things[d].item);
          things[d].flag = bswapw(things[d].flag);
        }
      bcase LINES:				/* only swap integer fields */
        for (d = 0; d < cnt; d++) {
          lines[d].fndx = bswapw(lines[d].fndx);
          lines[d].tndx = bswapw(lines[d].tndx);
          lines[d].flag = bswapw(lines[d].flag);
          lines[d].sect_tag = bswapw(lines[d].sect_tag);
          lines[d].rsidndx = bswapw(lines[d].rsidndx);
          lines[d].lsidndx = bswapw(lines[d].lsidndx);
        }
      bcase VERTS:				/* these resources consist */
       case SEGS:				/* of only integer fields  */
       case SSECTS:
       case NODES:
       case BLKMAPS:
        for (d = 0; d < cnt; d++)
          for (w = 0; w < mapresiz[type]/2; w++)
            data[d*mapresiz[type]/2+w] = bswapw(data[d*mapresiz[type]/2+w]);
      bcase SIDES:				/* only swap integer fields */
        for (d = 0; d < cnt; d++) {
          sides[d].image_xoff = bswapw(sides[d].image_xoff);
          sides[d].image_yoff = bswapw(sides[d].image_yoff);
          sides[d].sectndx = bswapw(sides[d].sectndx);
        }
      bcase SECTS:				/* only swap integer fields */
        for (d = 0; d < cnt; d++) {
          sects[d].floor_ht = bswapw(sects[d].floor_ht);
          sects[d].ceil_ht = bswapw(sects[d].ceil_ht);
          sects[d].light_lvl = bswapw(sects[d].light_lvl);
          sects[d].property = bswapw(sects[d].property);
          sects[d].line_tag = bswapw(sects[d].line_tag);
        }
      bcase REJECTS:				/* don't have to swap this */
        ;
      bcase BEHAVS:				/* don't have to swap this */
        ;
      bdefault:					/* something else--no swap */
        ;
    }
  }
#endif
}


/******************************************************************************
	ROUTINE:	patch_read(winfo,entry,resources_needed)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	May  1994
	DESCRIPTION:	This routine reads a VERDA patch file.  It reads the
			file twice; the first time is to obtain the sizes of
			each of the resources.  The input resources_needed is
			ignored--everything in the patch file is read.  Also,
			only one level's data is stored in a patch file and
			the BLOCKMAP and REJECT resources are not present.
******************************************************************************/
#if defined(ANSI_C)
boolean patch_read(WAD_INFO *winfo, int entry, long resources_needed)
#else
boolean patch_read(winfo,entry,resources_needed)
WAD_INFO *winfo;
int entry;
long resources_needed;
#endif
{
  char str[256];
  int type;
  WAD_THING *things;
  WAD_LINE *lines;
  WAD_SIDE *sides;
  WAD_VERT *verts;
  WAD_SEGS *segs;
  WAD_SSECTOR *ssecs;
  WAD_NODE *nodes;
  WAD_SECTOR *sects;
  short action_flag;
  int k;

  if (winfo->head.count <= entry)		/* can't read beyond count */
    return FALSE;
  for (k = 0; k < ALL(winfo->type); k++)
    winfo->count[k] = 0;
  rewind(winfo->fp);
  for (k = 0; fgets(str,sizeof str,winfo->fp) != NULL; ) {/* get counts */
    if (str[0] == ';' || str[0] == '#')		/* skip any comments */
      continue;
    if (k == 0) {
      k = sscanf(str," %d %d %d %lf",&winfo->ep,&winfo->mp,&type,&winfo->ver);
      continue;
    }
    k = str[0] != '%';
    if (k) winfo->count[type]++;
  }
  for (k = 0; k < ALL(winfo->type); k++)
    if ((resources_needed & (1L<<k)) && winfo->count[k] > 0) {
      blockfree(winfo->data[k]);
      winfo->dir[k].nbytes = winfo->count[k]*mapresiz[k];
      winfo->data[k] = blockmem(char,winfo->dir[k].nbytes);
    }
  things = (WAD_THING *)winfo->data[THINGS];
  lines  = (WAD_LINE *)winfo->data[LINES];
  sides  = (WAD_SIDE *)winfo->data[SIDES];
  verts  = (WAD_VERT *)winfo->data[VERTS];
  segs   = (WAD_SEGS *)winfo->data[SEGS];
  ssecs  = (WAD_SSECTOR *)winfo->data[SSECTS];
  nodes  = (WAD_NODE *)winfo->data[NODES];
  sects  = (WAD_SECTOR *)winfo->data[SECTS];
  rewind(winfo->fp);
  for (k = 0; fgets(str,sizeof str,winfo->fp) != NULL; ) {
    if (str[0] == ';' || str[0] == '#')		/* skip any comments */
      continue;
    if (k == 0) {
      k = sscanf(str," %d %d %d %lf",&winfo->ep,&winfo->mp,&type,&winfo->ver);
      continue;
    }
    if (!(resources_needed & (1L << type)))
      continue;
    switch (type) {
      case THINGS:				/* THINGS */
	sscanf(str,"%*d %4hx %4hx %3hd %4hx %4hx",
               &things->x,&things->y,&things->angle,&things->item,
               &things->flag);
        things++;
      bcase LINES:				/* LINES */
	sscanf(str,"%*d %4hx %4hx %4hx %4hx %4hx %4hx %4hx",
               &lines->fndx,&lines->tndx,&lines->flag,&action_flag,
               &lines->sect_tag,&lines->rsidndx,&lines->lsidndx);
        lines->special = action_flag;
        lines++;
      bcase SIDES:				/* SIDES */
	sscanf(str,"%*d %4hx %4hx %8c %8c %8c %4hx",
               &sides->image_xoff,&sides->image_yoff,
               sides->lwall,sides->uwall,sides->nwall,&sides->sectndx);
        while (sides->lwall[min(8,strlen(sides->lwall))-1] == ' ')
          sides->lwall[min(8,strlen(sides->lwall))-1] = '\0';
        while (sides->uwall[min(8,strlen(sides->uwall))-1] == ' ')
          sides->uwall[min(8,strlen(sides->uwall))-1] = '\0';
        while (sides->nwall[min(8,strlen(sides->nwall))-1] == ' ')
          sides->nwall[min(8,strlen(sides->nwall))-1] = '\0';
        sides++;
      bcase VERTS:				/* VERTEXES */
	sscanf(str,"%*d %4hx %4hx",&verts->x,&verts->y);
        verts++;
      bcase SEGS:				/* SEGS */
        sscanf(str,"%*d %4hx %4hx %4hx %4hx %4hx %4hx",
               &segs->fndx,&segs->tndx,&segs->angle,&segs->lndx,&segs->sndx,
               &segs->loffset);
        segs++;
      bcase SSECTS:				/* SSECTORS */
        sscanf(str,"%*d %4hx %4hx",&ssecs->count,&ssecs->sndx);
        ssecs++;
      bcase NODES:				/* NODES */
        sscanf(str,"%*d \
%4hx %4hx %4hx %4hx %4hx %4hx %4hx %4hx %4hx %4hx %4hx %4hx %4hx %4hx",
               &nodes->x,&nodes->y,&nodes->xdel,&nodes->ydel,
               &nodes->rymax,&nodes->rymin,&nodes->rxmin,&nodes->rxmax,
               &nodes->lymax,&nodes->lymin,&nodes->lxmin,&nodes->lxmax,
               &nodes->nndx[0],&nodes->nndx[1]);
        nodes++;
      bcase SECTS:				/* SECTORS */
	sscanf(str,"%*d %4hx %4hx %8c %8c %4hx %4hx %4hx",
               &sects->floor_ht,&sects->ceil_ht,sects->floor_desc,
               sects->ceil_desc,&sects->light_lvl,&sects->property,
               &sects->line_tag);
        while (sects->floor_desc[min(8,strlen(sects->floor_desc))-1] == ' ')
          sects->floor_desc[min(8,strlen(sects->floor_desc))-1] = '\0';
        while (sects->ceil_desc[min(8,strlen(sects->ceil_desc))-1] == ' ')
          sects->ceil_desc[min(8,strlen(sects->ceil_desc))-1] = '\0';
        sects++;
    }
    k = str[0] != '%';
  }
  winfo->ep++; winfo->mp++;
  return TRUE;
}


/******************************************************************************
	ROUTINE:	patch_write(oinfo,winfo)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	May  1994
	DESCRIPTION:	This routine writes a VERDA patch file.  Only one
			level's data is stored in a patch file and the
			BLOCKMAP, REJECT, and BEHAVIOR resources are not
			written.
******************************************************************************/
#if defined(ANSI_C)
boolean patch_write(WAD_INFO *oinfo, WAD_INFO *winfo)
#else
boolean patch_write(oinfo,winfo)
WAD_INFO *oinfo, *winfo;
#endif
{
  WAD_THING *things = (WAD_THING *)winfo->data[THINGS];
  WAD_LINE *lines = (WAD_LINE *)winfo->data[LINES];
  WAD_SIDE *sides = (WAD_SIDE *)winfo->data[SIDES];
  WAD_VERT *verts = (WAD_VERT *)winfo->data[VERTS];
  WAD_SEGS *segs = (WAD_SEGS *)winfo->data[SEGS];
  WAD_SSECTOR *ssecs = (WAD_SSECTOR *)winfo->data[SSECTS];
  WAD_NODE *nodes = (WAD_NODE *)winfo->data[NODES];
  WAD_SECTOR *sects = (WAD_SECTOR *)winfo->data[SECTS];
  int k;

  if (is_name(winfo->dir[MAINS].name,"MAP")) winfo->ver = 2;
  else                                       winfo->ver = 1;
  fprintf(oinfo->fp,"%d %d %d %4.2f\r\n",
          --winfo->ep,--winfo->mp,THINGS,winfo->ver);
  for (k = 0; k < winfo->count[THINGS]; k++)
    fprintf(oinfo->fp,"%03d %04x %04x %03d %04x %02x\r\n",k,
            (ushort)things[k].x,(ushort)things[k].y,things[k].angle,
            things[k].item,things[k].flag);
  fprintf(oinfo->fp,"%%\r\n%d %d %d %4.2f\r\n",
          winfo->ep,winfo->mp,LINES,winfo->ver);
  for (k = 0; k < winfo->count[LINES]; k++)
    fprintf(oinfo->fp,"%03d %04x %04x %04x %04x %04x %04x %04x\r\n",k,
            lines[k].fndx,lines[k].tndx,
            (ushort)lines[k].flag,(ushort)lines[k].action_flag,
            (ushort)lines[k].sect_tag,
            (ushort)lines[k].rsidndx,(ushort)lines[k].lsidndx);
  fprintf(oinfo->fp,"%%\r\n%d %d %d %4.2f\r\n",
          winfo->ep,winfo->mp,SIDES,winfo->ver);
  for (k = 0; k < winfo->count[SIDES]; k++)
    fprintf(oinfo->fp,"%03d %04x %04x %-8.8s %-8.8s %-8.8s %03x\r\n",k,
            (ushort)sides[k].image_xoff,(ushort)sides[k].image_yoff,
            sides[k].lwall,sides[k].uwall,sides[k].nwall,sides[k].sectndx);
  fprintf(oinfo->fp,"%%\r\n%d %d %d %4.2f\r\n",
          winfo->ep,winfo->mp,VERTS,winfo->ver);
  for (k = 0; k < winfo->count[VERTS]; k++)
    fprintf(oinfo->fp,"%03d %04x %04x\r\n",k,
            (ushort)verts[k].x,(ushort)verts[k].y);
  fprintf(oinfo->fp,"%%\r\n%d %d %d %4.2f\r\n",
          winfo->ep,winfo->mp,SEGS,winfo->ver);
  for (k = 0; k < winfo->count[SEGS]; k++)
    fprintf(oinfo->fp,"%03d %04x %04x %04x %04x %04x %04x\r\n",k,
            segs[k].fndx,segs[k].tndx,(ushort)segs[k].angle,
            segs[k].lndx,segs[k].sndx,segs[k].loffset);
  fprintf(oinfo->fp,"%%\r\n%d %d %d %4.2f\r\n",
          winfo->ep,winfo->mp,SSECTS,winfo->ver);
  for (k = 0; k < winfo->count[SSECTS]; k++)
    fprintf(oinfo->fp,"%03d %04x %04x\r\n",k,
            ssecs[k].count,ssecs[k].sndx);
  fprintf(oinfo->fp,"%%\r\n%d %d %d %4.2f\r\n",
          winfo->ep,winfo->mp,NODES,winfo->ver);
  for (k = 0; k < winfo->count[NODES]; k++)
    fprintf(oinfo->fp,"%03d \
%04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\r\n",k,
            (ushort)nodes[k].x,(ushort)nodes[k].y,
            (ushort)nodes[k].xdel,(ushort)nodes[k].ydel,
            (ushort)nodes[k].rymax,(ushort)nodes[k].rymin,
            (ushort)nodes[k].rxmin,(ushort)nodes[k].rxmax,
            (ushort)nodes[k].lymax,(ushort)nodes[k].lymin,
            (ushort)nodes[k].lxmin,(ushort)nodes[k].lxmax,
            (ushort)nodes[k].nndx[0],(ushort)nodes[k].nndx[1]);
  fprintf(oinfo->fp,"%%\r\n%d %d %d %4.2f\r\n",
          winfo->ep,winfo->mp,SECTS,winfo->ver);
  for (k = 0; k < winfo->count[SECTS]; k++)
    fprintf(oinfo->fp,"%03d %04x %04x %-8.8s %-8.8s %04x %04x %04x\r\n",k,
 	   (ushort)sects[k].floor_ht,(ushort)sects[k].ceil_ht,
            sects[k].floor_desc,sects[k].ceil_desc,
            sects[k].light_lvl,sects[k].property,(ushort)sects[k].line_tag);
  return TRUE;
}


/******************************************************************************
	ROUTINE:	wad_open(file,input,rewrite)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	June 1994
	DESCRIPTION:	This routine opens the specified file.  The file is
			created if input and rewrite are FALSE.  If the open
			fails for any reason a NULL is returned, otherwise an
			information block is allocated for the file.  The file
			is identified as either a (P/I)WAD file or a VERDA
			patch file.  If the file is a (P/I)WAD file, then it
			is further determined whether it is a DOOM/HERETIC
			WAD or a HEXEN WAD.  For a (P/I)WAD file, the resource
			directory is read.  The resource directory is held in
			two locations--one for modification and one to preserve
			the original directory.  The address of the information
			block is returned.
******************************************************************************/
#if defined(ANSI_C)
WAD_INFO *wad_open(char *file, boolean input, boolean rewrite)
#else
WAD_INFO *wad_open(file,input,rewrite)
char *file;
boolean input, rewrite;
#endif
{
  FILE *fp;
  WAD_INFO *winfo;
  int type;
  int e;

  fp = file_open(file,input,rewrite);
  if (fp == NULL)				/* oops */
    return NULL;
  winfo = blockmem(WAD_INFO,1);			/* WAD information block */
  winfo->fp = fp;
  if (input || rewrite) {
    rewind(fp);
    fread((char *)&winfo->head,sizeof winfo->head,1,fp);
    if (is_name(winfo->head.ident,"IWAD") ||
        is_name(winfo->head.ident,"PWAD")) {
      winfo->type = DOOM_FILE;			/* it's a (I/P)WAD */
      winfo->head.count = bswapl(winfo->head.count);
      winfo->head.offset = bswapl(winfo->head.offset);
    }else {
      rewind(fp);
      if (3 == fscanf(fp," %d %d %*d %lf",&winfo->ep,&winfo->mp,&winfo->ver)) {
        winfo->type = PATCH_FILE;		/* it's a patch file */
        winfo->head.count = ALL(winfo->type);
      }else					/* it's illegal */
        memset(&winfo->head,0,sizeof(winfo->head));
    }
    winfo->origdir = blockmem(DIR_ENTRY,winfo->head.count);
    winfo->dir = blockmem(DIR_ENTRY,winfo->head.count);
    winfo->data = blockmem(char *,winfo->head.count);
    winfo->changed = blockmem(boolean,winfo->head.count);
    winfo->count = blockmem(long,winfo->head.count);
    if (winfo->type == DOOM_FILE) {		/* it's a (I/P)WAD file */
      rewind(fp);
      fseek(fp,winfo->head.offset,0);		/* read directory */
      fread((char *)winfo->origdir,sizeof(*winfo->origdir),
            (int)winfo->head.count,fp);
      for (e = 0; e < winfo->head.count; e++)	/* see if it's a HEXEN WAD */
        if (is_name(winfo->origdir[e].name,"MAPINFO") ||
            is_name(winfo->origdir[e].name,"BEHAVIOR"))
          winfo->type = HEXEN_FILE;
    }else if (winfo->type == PATCH_FILE) {	/* it's a patch file */
      patch_read(winfo,0,~(~0L<<ALL(winfo->type)));/* read to get counts */
      for (e = 0; e < ALL(winfo->type); e++) {
        winfo->origdir[e] = winfo->dir[e];
        blockcopy(winfo->origdir[e].name,mapresrc[e],
                  sizeof winfo->origdir[e].name);
      }
      sprintf(winfo->origdir[MAINS].name,
              winfo->ver != 2 ? "E%dM%d" : "MAP%1d%1d",winfo->ep,winfo->mp);
    }
    for (e = 0; e < winfo->head.count; e++) {	/* get resources file  */
      winfo->dir[e] = winfo->origdir[e];	/* offsets and lengths */
      winfo->dir[e].offset =
      winfo->origdir[e].offset = bswapl(winfo->origdir[e].offset);
      winfo->dir[e].nbytes =
      winfo->origdir[e].nbytes = bswapl(winfo->origdir[e].nbytes);
      if ((type = resource_type(&winfo->dir[e])) < 0)
        winfo->count[e] = winfo->dir[e].nbytes;	/* just # bytes in resource */
      else if (winfo->type == DOOM_FILE)
        winfo->count[e] = winfo->dir[e].nbytes/mapdmsiz[type];
      else if (winfo->type == PATCH_FILE)
        winfo->count[e] = winfo->dir[e].nbytes/mapresiz[type];
      else
        winfo->count[e] = winfo->dir[e].nbytes/maphxsiz[type];
    }
  }
  return winfo;
}


/******************************************************************************
	ROUTINE:	wad_read(winfo,entry,resources_needed)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	May  1994
	DESCRIPTION:	This routine reads a WAD file. resources_needed governs
			which resources from the level are actually read.  The
			resources which differ between DOOM/HERETIC and HEXEN
			are translated appropriately.  This translation
			depends on the fact that the input resource item
			structures are smaller than the working structures so
			that the copying can be done in place (in the same
			array).
******************************************************************************/
#if defined(ANSI_C)
boolean wad_read(WAD_INFO *winfo, int entry, long resources_needed)
#else
boolean wad_read(winfo,entry,resources_needed)
WAD_INFO *winfo;
int entry;
long resources_needed;
#endif
{
  long nbytes;
  int type;
  WAD_THING *things;
  DOOM_THING *dthings;
  WAD_LINE *lines;
  DOOM_LINE *dlines;
  HEXEN_LINE *xlines;
  int d, b;
  int i;

  if (winfo == NULL ||
      winfo->fp == NULL) return FALSE;		/* can't do if invalid */
  if (winfo->type == PATCH_FILE)
    return patch_read(winfo,entry,resources_needed);/* do patch file */
  for (i = 0; i < ALL(winfo->type); i++) {
    if (winfo->head.count <= entry+i)		/* can't read beyond count */
      return FALSE;
    if (resources_needed & (1L<<i)) {		/* get requested resources */
      blockfree(winfo->data[entry+i]);
      nbytes = winfo->origdir[entry+i].nbytes;
      if (0 <= (type = resource_type(&winfo->origdir[entry+i])))
        nbytes = winfo->count[entry+i]*mapresiz[type];
      winfo->data[entry+i] = blockmem(char,nbytes);
      fseek(winfo->fp,winfo->origdir[entry+i].offset,0);
      fread(winfo->data[entry+i],1,(int)winfo->origdir[entry+i].nbytes,
            winfo->fp);
      if (nbytes != winfo->origdir[entry+i].nbytes)
        switch (type) {				/* translate as needed */
           case THINGS:
            things = (WAD_THING *)winfo->data[entry+i];
            dthings = (DOOM_THING *)winfo->data[entry+i];
            for (d = winfo->count[entry+i]-1; d >= 0; d--) {/* must be  */
              things[d].flag =  dthings[d].flag;/* copied in this order */
              things[d].item =  dthings[d].item;
              things[d].angle = dthings[d].angle;
              things[d].y =     dthings[d].y;
              things[d].x =     dthings[d].x;
            }
          bcase LINES:
            lines = (WAD_LINE *)winfo->data[entry+i];
            dlines = (DOOM_LINE *)winfo->data[entry+i];
            xlines = (HEXEN_LINE *)winfo->data[entry+i];
            for (d = winfo->count[entry+i]-1; d >= 0; d--)/* must be    */
              if (winfo->type == DOOM_FILE) {	/* copied in this order */
                lines[d].lsidndx =     dlines[d].lsidndx;
                lines[d].rsidndx =     dlines[d].rsidndx;
                lines[d].sect_tag =    dlines[d].sect_tag;
                lines[d].action_flag = dlines[d].action_flag;
                lines[d].flag =        dlines[d].flag;
                lines[d].tndx =        dlines[d].tndx;
                lines[d].fndx =        dlines[d].fndx;
              }else {
                lines[d].lsidndx = xlines[d].lsidndx;
                lines[d].rsidndx = xlines[d].rsidndx;
                for (b = sizeof(xlines[d].specarg)-1; b >= 0; b--)
                  lines[d].specarg[b] = xlines[d].specarg[b];
                lines[d].special = xlines[d].special;
                lines[d].flag =    xlines[d].flag;
                lines[d].tndx =    xlines[d].tndx;
                lines[d].fndx =    xlines[d].fndx;
              }
        }
      winfo->dir[entry+i].nbytes = nbytes;
      wad_bswap(winfo,entry+i);			/* byte swap if necessary */
    }
  }
  return TRUE;
}


/******************************************************************************
	ROUTINE:	wad_write(oinfo,winfo)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	May  1994
	DESCRIPTION:	This routine writes a WAD file.  If oinfo has a
			directory then the input file is being rewritten with
			the new data; otherwise, a new file is written.  The
			resources are written out in reverse order; this easily
			handles the case where the input file is being
			rewritten and some of the resources have grown in size.
			Also in the rewrite case, if a resource is smaller than
			before, it is marked with the new smaller size but the
			following resources are not "shifted down".  Thus there
			will be some parts of the file that will be unused.
			The resources which differ between DOOM/HERETIC and
			HEXEN are translated appropriately.  This translation
			depends on the fact that the input resource item
			structures are smaller than the working structures so
			that the copying can be done in place (in the same
			array).
******************************************************************************/
#if defined(ANSI_C)
boolean wad_write(WAD_INFO *oinfo,WAD_INFO *winfo)
#else
boolean wad_write(oinfo,winfo)
WAD_INFO *oinfo, *winfo;
#endif
{
  WAD_THING *things;
  DOOM_THING *dthings;
  WAD_LINE *lines;
  DOOM_LINE *dlines;
  HEXEN_LINE *xlines;
  long dir_offset;				/* new directory offset */
  int type;					/* level data resource type */
  long maxlen = 0L;				/* max size of orig entries */
  char *buf;					/* temporary data buffer */
  int e, d, b;

  if (oinfo == NULL ||
      oinfo->fp == NULL) return FALSE;		/* can't do if invalid */
  if (oinfo->type == PATCH_FILE)
    return patch_write(oinfo,winfo);		/* do patch file */
  winfo->dir[0].offset = sizeof winfo->head;
  for (e = 0; e < winfo->head.count; e++) {	/* compute new directory */
    if (maxlen < winfo->origdir[e].nbytes)
      maxlen = winfo->origdir[e].nbytes;
    if (0 <= (type = resource_type(&winfo->dir[e])))
    {
      if (oinfo->type == DOOM_FILE)
        winfo->dir[e].nbytes = winfo->count[e]*mapdmsiz[type];
      else
        winfo->dir[e].nbytes = winfo->count[e]*maphxsiz[type];
    }
    if (e == 0) continue;
    if (oinfo->dir == NULL || winfo->origdir[e-1].offset == 0)
      winfo->dir[e].offset = winfo->dir[e-1].offset + winfo->dir[e-1].nbytes;
    else
      winfo->dir[e].offset = winfo->dir[e-1].offset +
                             max(winfo->dir[e-1].nbytes,
                                 winfo->origdir[e].offset-
                                 winfo->origdir[e-1].offset);
  }
  buf = blockmem(char,maxlen);			/* buffer for copying */
  for (e = winfo->head.count-1; e >= 0; e--) {	/* write resources */
    if (winfo->changed[e] || winfo->type == PATCH_FILE) {/* write new data */
      if (winfo->data[e] != NULL) {
      	wad_bswap(winfo,e);			/* byte swap if necessary */
        switch (resource_type(&winfo->dir[e])) {/* translate as needed */
           case THINGS:
            things = (WAD_THING *)winfo->data[e];
            dthings = (DOOM_THING *)winfo->data[e];
            for (d = 0; d < winfo->count[e]; d++)/* must be copied in */
              if (winfo->type == DOOM_FILE) {	/* this order         */
                dthings[d].x =     things[d].x;
                dthings[d].y =     things[d].y;
                dthings[d].angle = things[d].angle;
                dthings[d].item =  things[d].item;
                dthings[d].flag =  things[d].flag;
              }
          bcase LINES:
            lines = (WAD_LINE *)winfo->data[e];
            dlines = (DOOM_LINE *)winfo->data[e];
            xlines = (HEXEN_LINE *)winfo->data[e];
            for (d = 0; d < winfo->count[e]; d++)/* must be copied in */
              if (winfo->type == DOOM_FILE) {	/* this order         */
                dlines[d].fndx =        lines[d].fndx;
                dlines[d].tndx =        lines[d].tndx;
                dlines[d].flag =        lines[d].flag;
                dlines[d].action_flag = lines[d].action_flag;
                dlines[d].sect_tag =    lines[d].sect_tag;
                dlines[d].rsidndx =     lines[d].rsidndx;
                dlines[d].lsidndx =     lines[d].lsidndx;
              }else {
                xlines[d].fndx =    lines[d].fndx;
                xlines[d].tndx =    lines[d].tndx;
                xlines[d].flag =    lines[d].flag;
                xlines[d].special = lines[d].special;
                for (b = 0; b < sizeof(xlines[d].specarg); b++)
                  xlines[d].specarg[b] = lines[d].specarg[b];
                xlines[d].rsidndx = lines[d].rsidndx;
                xlines[d].lsidndx = lines[d].lsidndx;
              }
        }
        fseek(oinfo->fp,winfo->dir[e].offset,0);
        fwrite(winfo->data[e],(int)winfo->dir[e].nbytes,1,oinfo->fp);
        blockfree(winfo->data[e]);
        winfo->data[e] = NULL;
      }
    }else if (oinfo->dir == NULL ||
              winfo->origdir[e].offset != winfo->dir[e].offset) {
      fseek(winfo->fp,winfo->origdir[e].offset,0);
      fread(buf,sizeof(*buf),(int)winfo->origdir[e].nbytes,winfo->fp);
      fseek(oinfo->fp,winfo->dir[e].offset,0);
      fwrite(buf,(int)winfo->origdir[e].nbytes,sizeof(*buf),oinfo->fp);
    }
  }
  blockfree(buf);				/* done with temp buffer */
  dir_offset = winfo->dir[winfo->head.count-1].offset +
               winfo->dir[winfo->head.count-1].nbytes;
  for (e = 0; e < winfo->head.count; e++) {
    winfo->dir[e].offset = bswapl(winfo->dir[e].offset);
    winfo->dir[e].nbytes = bswapl(winfo->dir[e].nbytes);
  }
  fseek(oinfo->fp,dir_offset,0);		/* write new directory */
  fwrite((char *)winfo->dir,(int)winfo->head.count,sizeof(*winfo->dir),
         oinfo->fp);
  for (e = 0; e < winfo->head.count; e++) {
    winfo->dir[e].offset = bswapl(winfo->dir[e].offset);
    winfo->dir[e].nbytes = bswapl(winfo->dir[e].nbytes);
  }
  if (oinfo->dir != NULL)
    blockcopy((char *)winfo->origdir,(char *)winfo->dir,/* this is now       */
              winfo->head.count*sizeof(winfo->dir[0]));/* original directory */
  if (winfo->type == PATCH_FILE) blockcopy(winfo->head.ident,"PWAD",4);
  winfo->head.offset = bswapl(dir_offset);
  winfo->head.count = bswapl(winfo->head.count);
  fseek(oinfo->fp,0L,0);			 /* write new header */
  fwrite((char *)&winfo->head,sizeof(winfo->head),1,oinfo->fp);
  winfo->head.offset = bswapl(dir_offset);
  winfo->head.count = bswapl(winfo->head.count);
  return TRUE;
}


/******************************************************************************
	ROUTINE:	wad_close(winfo)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	June 1994
	DESCRIPTION:	This routine closes the specified file and it frees
			the associated memory.
******************************************************************************/
#if defined(ANSI_C)
void wad_close(WAD_INFO *winfo)
#else
void wad_close(winfo)
WAD_INFO *winfo;
#endif
{
  if (winfo != NULL) {
    if (winfo->fp != NULL) (void)fclose(winfo->fp);
    blockfree(winfo->dir);			/* done with these */
    blockfree(winfo->origdir);
    blockfree(winfo->data);
    blockfree(winfo->changed);
    blockfree(winfo->count);
    blockfree(winfo);
  }
}
