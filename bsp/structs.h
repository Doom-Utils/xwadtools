/*- Doom Structures .. Colin Reed 1994 -------------------------------------*/

struct wad_header 					/* Linked wad files list.*/
{
	char type[4];
	long int num_entries;
	long int dir_start;
};

struct directory 						/* The directory entry header*/
{
	long int start;
	long int length;
	char name[8];
};

struct Block
{
	short minx;
	short miny;
	short xblocks;
	short yblocks;
};

/*- The level structures ---------------------------------------------------*/

struct Thing
{
   short xpos;      /* x position */
   short ypos;      /* y position */
   short angle;     /* facing angle */
   short type;      /* thing type */
   short when;      /* appears when? */
};

struct Vertex
{
   short int x;         /* X coordinate */
   short int y;         /* Y coordinate */
};

struct LineDef
{
   short int start;     /* from this vertex ... */
   short int end;       /* ... to this vertex */
   short int flags;     /* see NAMES.C for more info */
   short int type;      /* see NAMES.C for more info */
   short int tag;       /* crossing this linedef activates the sector with the same tag */
   short int sidedef1;  /* sidedef */
   short int sidedef2;  /* only if this line adjoins 2 sectors */
};

struct SideDef
{
   short int xoff;      /* X offset for texture */
   short int yoff;      /* Y offset for texture */
   char tex1[8];  /* texture name for the part above */
   char tex2[8];  /* texture name for the part below */
   char tex3[8];  /* texture name for the regular part */
   short int sector;    /* adjacent sector */
};

struct Sector
{
   short int floorh;    /* floor height */
   short int ceilh;     /* ceiling height */
   char floort[8];/* floor texture */
   char ceilt[8]; /* ceiling texture */
   short int light;     /* light level (0-255) */
   short int special;   /* special behaviour (0 = normal, 9 = secret, ...) */
   short int tag;       /* sector activated by a linedef with the same tag */
};

/*--------------------------------------------------------------------------*/
/* These are the structure used for creating the NODE bsp tree.             */
/*--------------------------------------------------------------------------*/

struct Seg
{
   short int start;     /* from this vertex ... */
   short int end;       /* ... to this vertex */
   unsigned short angle;/* angle (0 = east, 16384 = north, ...) */
   short int linedef;   /* linedef that this seg goes along*/
   short int flip;      /* true if not the same direction as linedef */
   unsigned short dist; /* distance from starting point */
	struct Seg *next;
        short psx,psy,pex,pey;  /* Start, end coordinates */
        long pdx,pdy,ptmp;      /* Used in intersection calculations */
        long len;
        short sector;
};

struct Pseg
{
   short int start;     /* from this vertex ... */
   short int end;       /* ... to this vertex */
   unsigned short angle;/* angle (0 = east, 16384 = north, ...) */
   short int linedef;   /* linedef that this seg goes along*/
   short int flip;      /* true if not the same direction as linedef */
   unsigned short dist; /* distance from starting point */
};

struct Node
{
   short int x, y;									/* starting point*/
   short int dx, dy;									/* offset to ending point*/
   short int maxy1, miny1, minx1, maxx1;		/* bounding rectangle 1*/
   short int maxy2, miny2, minx2, maxx2;		/* bounding rectangle 2*/
   short int chright, chleft;				/* Node or SSector (if high bit is set)*/
	struct Node *nextr,*nextl;
	short int node_num;	        		/* starting at 0 (but reversed when done)*/
        long ptmp;
};

struct Pnode
{
   short int x, y;									/* starting point*/
   short int dx, dy;									/* offset to ending point*/
   short int maxy1, miny1, minx1, maxx1;		/* bounding rectangle 1*/
   short int maxy2, miny2, minx2, maxx2;		/* bounding rectangle 2*/
   short int chright, chleft;				/* Node or SSector (if high bit is set)*/
};

struct SSector
{
   short int num;       /* number of Segs in this Sub-Sector */
   short int first;     /* first Seg */
};

/*--------------------------------------------------------------------------*/
