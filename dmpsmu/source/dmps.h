/*
   DooM PostScript Maps Utility, by Frans P. de Vries.

Derived from:

   Doom Editor Utility, by Brendon Wyber and Raphaël Quinet.

   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README for more information.

   This program comes with absolutely no warranty.

   DMPS.H - Main DMPSMU and DMPSMAP defines.
*/


/* the includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>


/* global definitions */
#define DMPS_VERSION	"2.6"
#define DEU_VERSION	"5.21 GCC" /* DJGPP/GO32 version */
typedef short int		BCINT;
typedef unsigned short int	UBCINT;

#include "dmunix.h"


/*
   syntactic sugar
*/
typedef BCINT Bool;	/* Boolean data: true or false */

/* boolean constants */
#ifndef TRUE
#define TRUE		1
#define FALSE		0
#endif


/*
   The directory structure is the structure used by DOOM to order the
   data in its WAD files.
*/
typedef struct Directory huge *DirPtr;
struct Directory
{
   long start;		/* offset to start of data */
   long size;		/* byte size of data */
   char name[ 8];	/* name of data block */
};


/*
   The Wad file pointer structure is used for holding the information
   on the Wad files in a linked list.
   The first Wad file is the main WAD file. The rest are patches.
*/
typedef struct WadFileInfo huge *WadPtr;
struct WadFileInfo
{
   WadPtr next;		/* next file in linked list */
   char   *filename;	/* name of the WAD file */
   FILE   *fileinfo;	/* C file stream information */
   char   type[ 4];	/* type of WAD file (IWAD or PWAD) */
   long   dirsize;	/* directory size of WAD */
   long   dirstart;	/* offset to start of directory */
   DirPtr directory;	/* array of directory information */
};


/*
   The master directory structure is used to build a complete directory
   of all the data blocks from all the various Wad files.
*/
typedef struct MasterDirectory huge *MDirPtr;
struct MasterDirectory
{
   MDirPtr next;	/* next in list */
   WadPtr  wadfile;	/* file of origin */
   struct Directory dir;/* directory data */
};


/*
   Description of the command line arguments and config file keywords.
*/
typedef struct
{
   char *short_name;	/* abbreviated command line argument */
   char *long_name;	/* command line arg. or keyword */
   enum	{		/* type of this option */
      OPT_BOOLEAN,	/* boolean (toggle) */
      OPT_INTEGER,	/* integer number */
      OPT_STRING,	/* character string */
      OPT_STRINGACC,	/* character string, but store in a list */
      OPT_STRINGLIST,	/* list of character strings */
      OPT_END		/* end of the options description */
      }  opt_type;
   char *msg_if_true;	/* message printed if option is true */
   char *msg_if_false;	/* message printed if option is false */
   void *data_ptr;	/* pointer to the data */
} OptDesc;


/*
   the interfile global variables
*/

/* from dmpsmu.c/dmpsmap.c */
extern BCINT GameVersion;	/* which game and version? */
extern char *MainWad;		/* name of the main WAD file */
extern char *UserLvlNm;		/* user defined level name */

/* from wads.c */
extern WadPtr  WadFileList;	/* linked list of Wad files */
extern MDirPtr MasterDir;	/* the master directory */

/* from ps.c */
extern FILE *PSFile;		/* the PostScript output file */


/*
   the function prototypes
*/

/* from dmpsmu.c/dmpsmap.c */
int main( int, char *[]);
void ParseCommandLineOptions( int, char *[]);
void Usage( FILE *);
void Credits( FILE *);
void ProgError( char *, ...);
void MainLoop( void);

/* from wads.c */
void OpenMainWad( char *);
void OpenPatchWad( char *);
void CloseWadFiles( void);
void CloseUnusedWadFiles( void);
WadPtr BasicWadOpen( char *);
void BasicWadRead( WadPtr, void huge *, long);
void BasicWadSeek( WadPtr, long);
MDirPtr FindMasterDir( MDirPtr, char *);
void ListMasterDirectory( FILE *);
void ListFileDirectory( FILE *, WadPtr);
void CopyBytes( FILE *, FILE *, long);
Bool Exists( char *);
void DumpDirectoryEntry( FILE *, char *);
void SaveEntryToRawFile( FILE *, char *);

/* from things.c */
BCINT GetThingClass( BCINT);
BCINT GetThingRadius( BCINT);

/* from levels.c */
void ReadLevelData( BCINT, BCINT);
void ForgetLevelData( void);

/* from print.c */
void PrintLevel( BCINT, BCINT);
void PrintMap( void);
void ShadeSector( BCINT, char);
void ShadePolygon( BCINT [], BCINT, char, BCINT *);
BCINT FindTeleExit( BCINT);
Bool PointInPolygon( BCINT [], BCINT, BCINT, BCINT, BCINT *);
Bool BuildPolygons( BCINT [], BCINT, BCINT *);
BCINT FindStartLine( BCINT [], BCINT, BCINT);
BCINT SortPolygon( BCINT [], BCINT, BCINT *);
Bool CheckClockwise( BCINT, BCINT, BCINT, BCINT);
void DisplayFlags( void);
void SetFlag( char, char);
void AnalyzeLevel( BCINT, BCINT);

/* from ps.c */
void InitPage( BCINT, BCINT, char *);
void InitScale( BCINT, BCINT, char *);
Bool CheckScale( BCINT, BCINT);
void AdjustScale( BCINT, BCINT);
void InitLines( void);
void InitThings( void);
void InitShades( void);
void InitLinks( void);
void TermPage( void);
void PrintMapLine( BCINT, BCINT, BCINT, BCINT, Bool, Bool);
void PrintMapThing( BCINT, BCINT, BCINT, BCINT, BCINT);
void PrintTeleLink( BCINT, BCINT, BCINT, BCINT);
void PrintShade( char, BCINT, BCINT);

/* end of file */
