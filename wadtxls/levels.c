/*
   WAD TeXtures LiSter, by Frans P. de Vries.

Derived from:

   DooM PostScript Maps Utility, by Frans P. de Vries.

And thus from:

   Doom Editor Utility, by Brendon Wyber and Raphaël Quinet.

   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README for more information.

   This program comes with absolutely no warranty.

   LEVELS.C - Level loading and saving routines.
*/


/* the includes */
#include <stdio.h>
#include "sysdep.h"
#include "wadtxls.h"
#include "wstructs.h"


/* global variables */
MDirPtr  Level = NULL;		/* master dictionary entry for the level */
char    *LevelName = NULL;	/* official name for the level */

BCINT NumThings = 0;		/* number of things */
TPtr     Things;		/* things data */
BCINT NumLineDefs = 0;		/* number of line defs */
LDPtr    LineDefs;		/* line defs data */
BCINT NumSideDefs = 0;		/* number of side defs */
SDPtr    SideDefs;		/* side defs data */
BCINT NumVertexes = 0;		/* number of vertexes */
VPtr     Vertexes;		/* vertex data */
BCINT NumSegs = 0;		/* number of segments */
SEPtr    Segs = NULL,		/* list of segments */
	 LastSeg = NULL;	/* last segment in the list */
BCINT NumSSectors = 0;		/* number of subsectors */
SSPtr    SSectors = NULL,	/* list of subsectors */
	 LastSSector = NULL;	/* last subsector in the list */
BCINT NumSectors = 0;		/* number of sectors */
SPtr     Sectors;		/* sectors data */

BCINT MapMaxX = -32767;		/* maximum X value of map */
BCINT MapMaxY = -32767;		/* maximum Y value of map */
BCINT MapMinX = 32767;		/* minimum X value of map */
BCINT MapMinY = 32767;		/* minimum Y value of map */


/* official names for all Ultimate DOOM levels */
char *LevelNames[ 4][ 9] = {
   { "Hangar", "Nuclear Plant", "Toxic Refinery", "Command Control", "Phobos Lab",
     "Central Processing", "Computer Station", "Phobos Anomaly", "Military Base" },
   { "Deimos Anomaly", "Containment Area", "Refinery", "Deimos Lab", "Command Center",
     "Halls of the Damned", "Spawning Vats", "Tower of Babel", "Fortress of Mystery" },
   { "Hell Keep", "Slough of Despair", "Pandemonium", "House of Pain",
     "Unholy Cathedral", "Mt. Erebus", "Limbo", "Dis", "Warrens" },
   { "Hell Beneath", "Perfect Hatred", "Sever The Wicked", "Unruly Evil", "They Will Repent",
     "Against Thee Wickedly", "And Hell Followed", "Unto The Cruel", "Fear" }
};

/* official names for all DOOM ][ levels */
char *LevelNames2[ 32] = {
   "Entryway", "Underhalls", "The Gantlet", "The Focus", "The Waste Tunnels", "The Crusher",
   "Dead Simple", "Tricks and Traps", "The Pit", "Refueling Base", "'O' of Destruction!",
   "The Factory", "Downtown", "The Inmost Dens", "Industrial Zone", "Suburbs", "Tenements",
   "The Courtyard", "The Citadel", "Gotcha!", "Nirvana", "The Catacombs", "Barrels o' Fun",
   "The Chasm", "Bloodfalls", "The Abandoned Mines", "Monster Condo", "The Spirit World",
   "The Living End", "Icon of Sin", "Wolfenstein", "Grosse"
};

/* official names for all Heretic levels */
char *LevelNamesH[ 3][ 9] = {
   { "The Docks", "The Dungeons", "The Gatehouse", "The Guard Tower", "The Citadel",
     "The Cathedral", "The Crypts", "Hell's Maw", "The Graveyard" },
   { "The Crater", "The Lava Pits", "The River of Fire", "The Ice Grotto", "The Catacombs",
     "The Labyrinth", "The Great Hall", "The Portals of Chaos", "The Glacier" },
   { "The Storehouse", "The Cesspool", "The Confluence", "The Azure Fortress",
     "The Ophidian Lair", "The Halls of Fear", "The Chasm", "D'Sparil's Keep", "The Aquifer" }
};

/*
   read in the level data
*/
void ReadLevelData( BCINT episode, BCINT mission)
{
   MDirPtr dir;
   char name[ 9];
   BCINT n, m, val;
   BCINT OldNumVertexes;
   BCINT *VertexUsed = (BCINT *)0;

   /* find the various level information from the master directory */
   if (GameVersion == 2) /* check for DOOM ][ */
      sprintf( name, "MAP%02d", mission);
   else
      sprintf( name, "E%dM%d", episode, mission);
   Level = FindMasterDir( MasterDir, name);
   if (! Level)
      ProgError( "level data not found");

   if (GameVersion == 2)
      LevelName = LevelNames2[ mission - 1];
   else
      if (GameVersion >= 16)
         if (GameVersion == 17 && episode == 4 && mission == 1) /* Heretic E4M1 */
	    LevelName = "IDMAP01";
         else
	    LevelName = LevelNamesH[ episode - 1][ mission - 1];
      else /* GameVersion == 0, 1, or 4 */
	 LevelName = LevelNames[ episode - 1][ mission - 1];

   /* read in the Things data */
   dir = FindMasterDir( Level, "THINGS");
   if (dir)
      NumThings = (BCINT) (dir->dir.size / sizeof( struct Thing));
   else
      NumThings = 0;
   if (NumThings > 0)
   {
      Things = (TPtr) GetFarMemory( (unsigned long) NumThings * sizeof( struct Thing));
      BasicWadSeek( dir->wadfile, dir->dir.start);
      for (n = 0; n < NumThings; n++)
      {
	 BasicWadRead( dir->wadfile, &(Things[ n].xpos), 2);
	 swapint( &(Things[ n].xpos));
	 BasicWadRead( dir->wadfile, &(Things[ n].ypos), 2);
	 swapint( &(Things[ n].ypos));
	 BasicWadRead( dir->wadfile, &(Things[ n].angle), 2);
	 swapint( &(Things[ n].angle));
	 BasicWadRead( dir->wadfile, &(Things[ n].type), 2);
	 swapint( &(Things[ n].type));
	 BasicWadRead( dir->wadfile, &(Things[ n].when), 2);
	 swapint( &(Things[ n].when));
      }
   }

   /* get the number of Vertices */
   dir = FindMasterDir( Level, "VERTEXES");
   if (dir)
      OldNumVertexes = (BCINT) (dir->dir.size / sizeof( struct Vertex));
   else
      OldNumVertexes = 0;
   if (OldNumVertexes > 0)
   {
      VertexUsed = (BCINT *) GetMemory( OldNumVertexes * sizeof( BCINT));
      for (n = 0; n < OldNumVertexes; n++)
	 VertexUsed[ n] = FALSE;
   }

   /* read in the LineDef information */
   dir = FindMasterDir( Level, "LINEDEFS");
   if (dir)
      NumLineDefs = (BCINT) (dir->dir.size / sizeof( struct LineDef));
   else
      NumLineDefs = 0;
   if (NumLineDefs > 0)
   {
      LineDefs = (LDPtr) GetFarMemory( (unsigned long) NumLineDefs * sizeof( struct LineDef));
      BasicWadSeek( dir->wadfile, dir->dir.start);
      for (n = 0; n < NumLineDefs; n++)
      {
	 BasicWadRead( dir->wadfile, &(LineDefs[ n].start), 2);
	 swapint( &(LineDefs[ n].start));
	 VertexUsed[ LineDefs[ n].start] = TRUE;
	 BasicWadRead( dir->wadfile, &(LineDefs[ n].end), 2);
	 swapint( &(LineDefs[ n].end));
	 VertexUsed[ LineDefs[ n].end] = TRUE;
	 BasicWadRead( dir->wadfile, &(LineDefs[ n].flags), 2);
	 swapint( &(LineDefs[ n].flags));
	 BasicWadRead( dir->wadfile, &(LineDefs[ n].type), 2);
	 swapint( &(LineDefs[ n].type));
	 BasicWadRead( dir->wadfile, &(LineDefs[ n].tag), 2);
	 swapint( &(LineDefs[ n].tag));
	 BasicWadRead( dir->wadfile, &(LineDefs[ n].sidedef1), 2);
	 swapint( &(LineDefs[ n].sidedef1));
	 BasicWadRead( dir->wadfile, &(LineDefs[ n].sidedef2), 2);
	 swapint( &(LineDefs[ n].sidedef2));
      }
   }

   /* read in the SideDef information */
   dir = FindMasterDir( Level, "SIDEDEFS");
   if (dir)
      NumSideDefs = (BCINT) (dir->dir.size / sizeof( struct SideDef));
   else
      NumSideDefs = 0;
   if (NumSideDefs > 0)
   {
      SideDefs = (SDPtr) GetFarMemory( (unsigned long) NumSideDefs * sizeof( struct SideDef));
      BasicWadSeek( dir->wadfile, dir->dir.start);
      for (n = 0; n < NumSideDefs; n++)
      {
	 BasicWadRead( dir->wadfile, &(SideDefs[ n].xoff), 2);
	 swapint( &(SideDefs[ n].xoff));
	 BasicWadRead( dir->wadfile, &(SideDefs[ n].yoff), 2);
	 swapint( &(SideDefs[ n].yoff));
	 BasicWadRead( dir->wadfile, &(SideDefs[ n].tex1), 8);
	 BasicWadRead( dir->wadfile, &(SideDefs[ n].tex2), 8);
	 BasicWadRead( dir->wadfile, &(SideDefs[ n].tex3), 8);
	 BasicWadRead( dir->wadfile, &(SideDefs[ n].sector), 2);
	 swapint( &(SideDefs[ n].sector));
      }
   }

   /* read in the Vertices which are all the corners of the level, but ignore the */
   /* Vertices not used in any LineDef (they usually are at the end of the list). */
   NumVertexes = 0;
   for (n = 0; n < OldNumVertexes; n++)
      if (VertexUsed[ n])
	 NumVertexes++;
   if (NumVertexes > 0)
   {
      Vertexes = (VPtr) GetFarMemory( (unsigned long) NumVertexes * sizeof( struct Vertex));
      dir = FindMasterDir( Level, "VERTEXES");
      BasicWadSeek( dir->wadfile, dir->dir.start);
      MapMaxX = -32767;
      MapMaxY = -32767;
      MapMinX = 32767;
      MapMinY = 32767;
      m = 0;
      for (n = 0; n < OldNumVertexes; n++)
      {
	 BasicWadRead( dir->wadfile, &val, 2);
	 swapint( &val);
	 if (VertexUsed[ n])
	 {
	    if (val < MapMinX)
	       MapMinX = val;
	    if (val > MapMaxX)
	       MapMaxX = val;
	    Vertexes[ m].x = val;
	 }
	 BasicWadRead( dir->wadfile, &val, 2);
	 swapint( &val);
	 if (VertexUsed[ n])
	 {
	    if (val < MapMinY)
	       MapMinY = val;
	    if (val > MapMaxY)
	       MapMaxY = val;
	    Vertexes[ m].y = val;
	    m++;
	 }
      }
      if (m != NumVertexes)
	 ProgError("inconsistency in the Vertexes data\n");
   }

   if (OldNumVertexes > 0)
   {
      /* update the Vertex numbers in the LineDefs (not really necessary, but...) */
      m = 0;
      for (n = 0; n < OldNumVertexes; n++)
	 if (VertexUsed[ n])
	    VertexUsed[ n] = m++;
      for (n = 0; n < NumLineDefs; n++)
      {
	 LineDefs[ n].start = VertexUsed[ LineDefs[ n].start];
	 LineDefs[ n].end = VertexUsed[ LineDefs[ n].end];
      }
      FreeMemory( VertexUsed);
   }

   /* ignore the Segs, SSectors and Nodes */

   /* read in the Sectors information */
   dir = FindMasterDir( Level, "SECTORS");
   if (dir)
      NumSectors = (BCINT) (dir->dir.size / sizeof( struct Sector));
   else
      NumSectors = 0;
   if (NumSectors > 0)
   {
      Sectors = (SPtr) GetFarMemory( (unsigned long) NumSectors * sizeof( struct Sector));
      BasicWadSeek( dir->wadfile, dir->dir.start);
      for (n = 0; n < NumSectors; n++)
      {
	 BasicWadRead( dir->wadfile, &(Sectors[ n].floorh), 2);
	 swapint( &(Sectors[ n].floorh));
	 BasicWadRead( dir->wadfile, &(Sectors[ n].ceilh), 2);
	 swapint( &(Sectors[ n].ceilh));
	 BasicWadRead( dir->wadfile, &(Sectors[ n].floort), 8);
	 BasicWadRead( dir->wadfile, &(Sectors[ n].ceilt), 8);
	 BasicWadRead( dir->wadfile, &(Sectors[ n].light), 2);
	 swapint( &(Sectors[ n].light));
	 BasicWadRead( dir->wadfile, &(Sectors[ n].special), 2);
	 swapint( &(Sectors[ n].special));
	 BasicWadRead( dir->wadfile, &(Sectors[ n].tag), 2);
	 swapint( &(Sectors[ n].tag));
      }
   }

   /* ignore the Reject and BlockMap */
}


/*
   forget the level data
*/
void ForgetLevelData()
{
   /* forget the Things */
   NumThings = 0;
   if (Things)
      FreeFarMemory( Things);
   Things = NULL;

   /* forget the Vertices */
   NumVertexes = 0;
   if (Vertexes)
      FreeFarMemory( Vertexes);
   Vertexes = NULL;

   /* forget the LineDefs */
   NumLineDefs = 0;
   if (LineDefs)
      FreeFarMemory( LineDefs);
   LineDefs = NULL;

   /* forget the SideDefs */
   NumSideDefs = 0;
   if (SideDefs)
      FreeFarMemory( SideDefs);
   SideDefs = NULL;

   /* forget the Sectors */
   NumSectors = 0;
   if (Sectors)
      FreeFarMemory( Sectors);
   Sectors = NULL;

   /* forget the level pointers */
   Level = NULL;
   LevelName = NULL;
}

/* end of file */
