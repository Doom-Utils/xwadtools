/*
   DooM PostScript Maps Utility, by Frans P. de Vries.

Derived from:

   Doom Editor Utility, by Brendon Wyber and Raphaël Quinet.

   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README for more information.

   This program comes with absolutely no warranty.

   LEVELS.H - Level data definitions.
*/


/* the includes */
#include "wstructs.h"


/* the external variables from levels.c */
extern MDirPtr  Level;		/* master dictionary entry for the level */
extern char    *LevelName;	/* official name for the level */

extern BCINT NumThings;		/* number of things */
extern TPtr     Things;		/* things data */
extern BCINT NumLineDefs;	/* number of line defs */
extern LDPtr    LineDefs;	/* line defs data */
extern BCINT NumSideDefs;	/* number of side defs */
extern SDPtr    SideDefs;	/* side defs data */
extern BCINT NumVertexes;	/* number of vertexes */
extern VPtr     Vertexes;	/* vertex data */
extern BCINT NumSegs;		/* number of segments */
extern SEPtr    Segs,		/* list of segments */
		LastSeg;	/* last segment in the list */
extern BCINT NumSSectors;	/* number of subsectors */
extern SSPtr    SSectors,	/* list of subsectors */
		LastSSector;	/* last subsector in the list */
extern BCINT NumSectors;	/* number of sectors */
extern SPtr     Sectors;	/* sectors data */

extern BCINT MapMaxX;		/* maximum X value of map */
extern BCINT MapMaxY;		/* maximum Y value of map */
extern BCINT MapMinX;		/* minimum X value of map */
extern BCINT MapMinY;		/* minimum Y value of map */

/* end of file */
