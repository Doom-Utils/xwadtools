/*
   DooM TeXtures LiSter, by Frans P. de Vries.

Derived from:

   DooM PostScript Maps Utility, by Frans P. de Vries.

And thus from:

   Doom Editor Utility, by Brendon Wyber and Raphaël Quinet.

   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README for more information.

   This program comes with absolutely no warranty.

   WADTXLS.H - Main WADTXLS defines.
*/


/* the includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>


/* global definitions */
#define WADTXLS_VERSION	"1.1"
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

/* from dmtxls.c */
extern BCINT GameVersion;	/* which game and version? */
extern char *MainWad;		/* name of the main WAD file */
extern char *UserLvlNm;		/* user defined level name */
extern FILE *TxFile;		/* the textures output file */
extern Bool InclTexts;		/* include wall textures? */
extern Bool InclFlats;		/* include flats? */
extern Bool Verbose;		/* list all SideDef/Sector numbers? */
extern Bool VerbLine;		/* list all LineDef numbers? */

/* from wads.c */
extern WadPtr  WadFileList;	/* linked list of Wad files */
extern MDirPtr MasterDir;	/* the master directory */


/*
   the function prototypes
*/

/* from dmtxls.c */
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
Bool Exists( char *);

/* from levels.c */
void ReadLevelData( BCINT, BCINT);
void ForgetLevelData( void);

/* from list.c */
void ListLevel( BCINT, BCINT);
void BuildTexts( void);
void BuildFlats( void);
void AddToTexList( BCINT, char *, BCINT);
void AddToNumList( char *, BCINT, BCINT);
void PrintList( void);
BCINT FindLineDef( BCINT);
void ForgetList( void);

/* end of file */
