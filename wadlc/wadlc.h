/* WADLC.H - some data structures for WADLC.C */

typedef struct
{
  char	name[8];
  short	x, y;
} w_vertex_t;

typedef struct
{
  char	name[8];
  short	floorheight, ceilingheight;
  char	floortexture[8], ceilingtexture[8];
  short	brightness, special, tag;
} w_sector_t;
