/******************************************************************************
	INCLUDE:	DMGLOBAL.I
	WRITTEN BY:	Robert Fenske, Jr. (rfenske@swri.edu)
	CREATED:	Mar. 1994
	DESCRIPTION:	This include file contains all the necessary constant,
			macro, syntax, and type definitions as well as all
			external and global variables needed for the various
			DOOM and HEXEN-related utility programs.
******************************************************************************/

#include <sys/types.h>

#if !defined(DM_Globals)

#undef ANSI_C
#if defined(__STDC__) || defined(__STRICT_ANSI__) || defined(__OS2__)
#define ANSI_C		1
#endif

#if !defined(__OS2__)
#define _Optlink
#endif

/*===========================================================================*/

#define bcase		break;case		/* for a more reasonabble */
#define bdefault	break;default		/* switch statement       */
#define global					/* global declarations */
#define forward		extern			/* forward references */
#define local		static			/* local declarations */
#define otherwise	break;default		/* alternate default case */
#define repeat		do
#define until(expr)	while (!(expr))		/* for do ... until loops */

/*===========================================================================*/

#if defined(MSDOS)	/* DJGPP doesn't define it */
typedef unsigned short	ushort;
#endif
typedef unsigned int	boolean;		/* integer used as boolean */
typedef short		word;			/* 16-bit word */
typedef word		bams;			/* 16-bit bams */

/*===========================================================================*/

#undef PI
#define PI		(double)3.14159265358979323846	/* ubiquitous PI */
#define TWOPI		(double)6.28318530717958647693
#define bams_to_deg	(double)0.00549316406250000000	/* angle conv consts */
#define deg_to_bams   (double)182.04444444444444444444
#define bams_to_rad	(double)0.00009587379924285258
#define rad_to_bams (double)10430.37835047045272494957
#define deg_to_rad	(double)0.01745329251994329577
#define rad_to_deg     (double)57.29577951308232087680
#define BAMS180		((bams)0x8000)		/* BAMS: (-)180 degrees */
#define BAMS090		((bams)0x4000)		/* BAMS: 90 degrees */

#define FALSE		0
#define TRUE		(!FALSE)

/*===========================================================================*/
#if !defined(__OS2__)
#define abs(a)		((a) < 0 ? -(a) : (a))
#define min(a,b)	((a) < (b) ? (a) : (b))
#define max(a,b)	((a) < (b) ? (b) : (a))
#endif
#define numbits(v)	(8*numbytes(v))
#define numbytes(v)	((unsigned)sizeof(v))
#define numelm(a)	(sizeof a / sizeof a[0])
#define sgn(a)		((0<(a))-((a)<0))

	/* the blockmem() routine must allocate and MUST zero memory */
#define blockmem(t,n)	(t *)_blockmem((n),sizeof(t))
#if ANSI_C
local char *_blockmem(long n,long t)		/* this is to provide a     */
#else						/* cleaner exit than simply */
local char *_blockmem(n,t)			/* crashing if memory       */
long n, t;					/* allocation fails         */
#endif
{
  char *block = 0 < n ? calloc(n,t) : NULL;
  if (n > 0 && block == NULL) {
    printf("\nERROR: allocation of %ld bytes failed\n",(long)n*t);
    exit(1);
  }
  return block;
}
#if defined(__OS2__)
#include <string.h>
#define blockfree(b)	(free((char *)(b)))
#elif defined(sun)
#define blockfree(b)	((b)!=NULL?free((char *)(b)):0)
#else
#define blockfree(b)	((b)!=NULL?free((char *)(b)):(void)0)
#endif
#define blockcopy(bt,bf,n) (void)memcpy((char *)(bt),(char *)(bf),(int)(n))

#if defined(VAXC)
#define file_open(file,input,rewrite) \
			fopen((file),(input)?"rb":(rewrite)?"r+b":"wb","\
			       shr=upd")
#else
#define file_open(file,input,rewrite) \
			fopen((file),(input)?"rb":(rewrite)?"r+b":"wb")
#endif

/*===========================================================================*/

typedef struct DOOM_THING {			/* DOOM thing information */
	short x, y;
	short angle;
	short item;
	short flag;
} DOOM_THING;

typedef struct HEXEN_THING {			/* HEXEN thing information */
	short id;
	short x, y;
	short z;
	short angle;
	short item;
	short flag;
	unsigned char special;
	char specarg[5];
} HEXEN_THING;

typedef HEXEN_THING WAD_THING;			/* thing information */

typedef struct DOOM_LINE {			/* DOOM line information */
	short fndx, tndx;
	short flag, action_flag;
	short sect_tag;
	short rsidndx, lsidndx;
} DOOM_LINE;

typedef struct HEXEN_LINE {			/* HEXEN line information */
	short fndx, tndx;
	short flag;
	unsigned char special;
	char specarg[5];
	short rsidndx, lsidndx;
} HEXEN_LINE;

typedef struct WAD_LINE {			/* line information */
	short fndx, tndx;
	short flag, action_flag;
	short sect_tag;
	unsigned char special;
	char specarg[5];
	short rsidndx, lsidndx;
} WAD_LINE;

typedef struct WAD_SIDE {			/* side information */
	short image_xoff, image_yoff;
	char lwall[8], uwall[8], nwall[8];
	short sectndx;
} WAD_SIDE;

typedef struct WAD_VERT {			/* vertex information */
	short x, y;
} WAD_VERT;

typedef struct WAD_SEGS {			/* segment information */
	short fndx, tndx;
	bams angle;
	short lndx;
	short sndx;
	short loffset;
} WAD_SEGS;

typedef struct WAD_SSECTOR {			/* subsector information */
	short count;
	short sndx;
} WAD_SSECTOR;

typedef struct WAD_NODE {			/* node information */
	short x, y;
	short xdel, ydel;
	short rymax, rymin, rxmin, rxmax;
	short lymax, lymin, lxmin, lxmax;
	short nndx[2];
} WAD_NODE;

typedef struct WAD_SECTOR {			/* sector information */
	short floor_ht, ceil_ht;
	char floor_desc[8], ceil_desc[8];
	short light_lvl;
	short property;
	short line_tag;
} WAD_SECTOR;

typedef unsigned char	WAD_REJECT;		/* reject information */

typedef short		WAD_BLOCKMAP;		/* blockmap information */

typedef unsigned char	WAD_BEHAVIOR;		/* behavior information */

/*===========================================================================*/

#define MAINS		0			/* a level's resources */
#define THINGS		1
#define LINES		2
#define SIDES		3
#define VERTS		4
#define SEGS		5
#define SSECTS		6
#define NODES		7
#define SECTS		8
#define REJECTS		9
#define BLKMAPS		10
#define BEHAVS		11			/* Hexen only */
#define ALL(t)		((t)==HEXEN_FILE?12:11)	/* always last */

#define RG_REJDEPTH	(7<<0)			/* reject depth (0 - 7) */
#define RG_MULTSECT	(1<<3)			/* nodes: multiple sect mode */
#define RG_USEALL	(1<<4)			/* nodes: use all segs */
#define RG_LIMIT_IO	(1<<5)			/* limit screen I/O */

#define DOOM_FILE	1			/* input/output file types */
#define HEXEN_FILE	2
#define PATCH_FILE	3

typedef struct WAD_HEAD {			/* WAD file header */
	char ident[4];
	long count;
	long offset;
} WAD_HEAD;

typedef struct DIR_ENTRY {			/* WAD file directory entry */
	long offset;
	long nbytes;
	char name[8];
} DIR_ENTRY;

typedef struct WAD_INFO {			/* WAD file information */
	FILE *fp;					/* file pointer */
	int type;					/* (I/P)WAD,patch */
	WAD_HEAD head;					/* file header */
	DIR_ENTRY *origdir,				/* orig directory */
	          *dir;					/* resources dirctry */
	char **data;					/* resources data */
	boolean *changed;				/* rsrce changed flg */
	long *count;					/* resources size */
	int ep, mp;					/* cur episode/map */
	double ver;					/* patch version */
} WAD_INFO;

/*===========================================================================*/

extern void resource_update(
#if defined(ANSI_C)
	WAD_INFO *winfo,
	int entry,
	void *data,
	long count
#endif
);
extern boolean patch_read(
#if defined(ANSI_C)
	WAD_INFO *winfo,
	int entry,
	long resources_needed
#endif
);
extern boolean patch_write(
#if defined(ANSI_C)
	WAD_INFO *oinfo,
	WAD_INFO *winfo
#endif
);
extern WAD_INFO *wad_open(
#if defined(ANSI_C)
	char *file,
	boolean input,
	boolean rewrite
#endif
);
extern boolean wad_read(
#if defined(ANSI_C)
	WAD_INFO *winfo,
	int entry,
	long resources_needed
#endif
);
extern boolean wad_write(
#if defined(ANSI_C)
	WAD_INFO *oinfo,
	WAD_INFO *winfo
#endif
);
extern void wad_close(
#if defined(ANSI_C)
	WAD_INFO *winfo
#endif
);

extern long nodes_make(				/* makes NODES,SSECTORS,SEGS */
#if defined(ANSI_C)
	WAD_NODE **nodes,
	long *nnodes,
	WAD_SSECTOR **ssecs,
	long *nssecs,
	WAD_SEGS **segs,
	long *nsegs,
	WAD_VERT **verts,
	long *nverts,
	WAD_LINE **lines,
	long *nlines,
	WAD_SIDE **sides,
	char *genflags
#endif
);
extern long blockmap_make(			/* makes the BLOCKMAP */
#if defined(ANSI_C)
	short **blockmap,
	WAD_LINE *lines,
	long nlines,
	WAD_VERT *verts
#endif
);
extern long reject_make(			/* makes REJECT */
#if defined(ANSI_C)
	WAD_REJECT **reject,
	int rej_flags,
	char *except,
	WAD_LINE *lines,
	long nlines,
	WAD_SIDE *sides,
	WAD_VERT *verts,
	WAD_BLOCKMAP *blockmap
#endif
);

#define DM_Globals	1			/* now have defined these */
#endif
