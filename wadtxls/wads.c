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

   WADS.C - WAD files routines.
*/


/* the includes */
#include <stdio.h>
#include "sysdep.h"
#include "wadtxls.h"


/* global variables */
WadPtr  WadFileList = NULL;	/* linked list of Wad files */
MDirPtr MasterDir   = NULL;	/* the master directory */



/*
   open the main WAD file, read in its directory and create the master directory
*/
void OpenMainWad( char *filename)
{
   MDirPtr lastp, newp;
   WadPtr wad;
   long n;

   /* open the WAD file */
   printf( "Loading main WAD file: %s...\n", filename);
   wad = BasicWadOpen( filename);
   if (strncmp( wad->type, "IWAD", 4))
      ProgError( "\"%s\" is not the main WAD file", filename);

   /* create the master directory */
   lastp = NULL;
   for (n = 0; n < wad->dirsize; n++)
   {
      newp = (MDirPtr) GetMemory( sizeof( struct MasterDirectory));
      newp->next = NULL;
      newp->wadfile = wad;
      memcpy( (char *) &(newp->dir), (char *) &(wad->directory[ n]), sizeof( struct Directory));
      if (MasterDir)
	 lastp->next = newp;
      else
	 MasterDir = newp;
      lastp = newp;
   }

   /* check if registered/commercial/ultimate version */
   /* if you change this, bad things will happen to you... */
   if (FindMasterDir( MasterDir, "E4M9") != NULL)
      GameVersion = 4;
   else if (FindMasterDir( MasterDir, "MAP29") != NULL)
      GameVersion = 2;
   else if (FindMasterDir( MasterDir, "E2M9") != NULL)
      GameVersion = 1;

   /* check if Heretic version */
   if (FindMasterDir( MasterDir, "M_HTIC") != NULL)
      GameVersion += 16;
}


/*
   open a patch WAD file, read in its directory and alter the master directory
*/
void OpenPatchWad( char *filename)
{
   MDirPtr mdir = NULL;
   WadPtr wad;
   BCINT n, l;
   char entryname[ 9];

   /* ignore the file if it doesn't exist */
   if (! Exists( filename))
   {
      printf( "Warning: patch WAD file \"%s\" doesn't exist.  Ignored.\n", filename);
      return;
   }

   /* open the WAD file */
   printf( "Loading patch WAD file: %s...\n", filename);
   wad = BasicWadOpen( filename);
   if (strncmp( wad->type, "PWAD", 4))
      ProgError( "\"%s\" is not a patch WAD file", filename);

   /* alter the master directory */
   l = 0;
   for (n = 0; n < wad->dirsize; n++)
   {
      strncpy( entryname, wad->directory[ n].name, 8);
      entryname[ 8] = '\0';
      if (l == 0)
      {
	 mdir = FindMasterDir( MasterDir, wad->directory[ n].name);
	 /* if this entry is not in the master directory, then add it */
	 if (! mdir)
	 {
	    printf( "   [Adding new entry %s]\n", entryname);
	    mdir = MasterDir;
	    while (mdir->next)
	       mdir = mdir->next;
	    mdir->next = (MDirPtr) GetMemory( sizeof( struct MasterDirectory));
	    mdir = mdir->next;
	    mdir->next = NULL;
	 }
	 /* if this is a level, then copy this entry and the next 10 */
	 else if ((GameVersion == 2 && wad->directory[ n].name[ 0] == 'M' &&
				       wad->directory[ n].name[ 1] == 'A' &&
				       wad->directory[ n].name[ 2] == 'P' &&
				       wad->directory[ n].name[ 5] == '\0') ||
		  (GameVersion != 2 && wad->directory[ n].name[ 0] == 'E' &&
				       wad->directory[ n].name[ 2] == 'M' &&
				       wad->directory[ n].name[ 4] == '\0'))
	 {
	    printf( "   [Updating level %s]\n", entryname);
	    l = 10;
	 }
	 else
	    printf( "   [Updating entry %s]\n", entryname);
      }
      else
      {
	 mdir = mdir->next;
	 /* the level data should replace an existing level */
	 if (mdir == NULL || strncmp(mdir->dir.name, wad->directory[ n].name, 8))
	    ProgError( "\\%s\" is not an understandable PWAD file (error with %s)", filename, entryname);
	 l--;
      }
      mdir->wadfile = wad;
      memcpy( (char *) &(mdir->dir), (char *) &(wad->directory[ n]), sizeof( struct Directory));
   }
}


/*
   close all the Wad files, deallocating the WAD file structures
*/
void CloseWadFiles()
{
   MDirPtr curd, nextd;
   WadPtr curw, nextw;

   /* close the Wad files */
   curw = WadFileList;
   WadFileList = NULL;
   while (curw)
   {
      nextw = curw->next;
      fclose( curw->fileinfo);
      FreeMemory( curw->directory);
      FreeMemory( curw);
      curw = nextw;
   }

   /* delete the master directory */
   curd = MasterDir;
   MasterDir = NULL;
   while (curd)
   {
      nextd = curd->next;
      FreeMemory( curd);
      curd = nextd;
   }
}


/*
   forget unused patch Wad files
*/
void CloseUnusedWadFiles()
{
   MDirPtr mdir;
   WadPtr curw, prevw;

   prevw = NULL;
   curw = WadFileList;
   while (curw)
   {
      /* check if the Wad file is used by a directory entry */
      mdir = MasterDir;
      while (mdir && mdir->wadfile != curw)
	 mdir = mdir->next;
      if (mdir)
	 prevw = curw;
      else
      {
	 /* if this Wad file is never used, close it */
	 if (prevw)
	    prevw->next = curw->next;
	 else
	    WadFileList = curw->next;
	 fclose( curw->fileinfo);
	 FreeMemory( curw->directory);
	 FreeMemory( curw);
      }
      curw = prevw->next;
   }
}


/*
   basic opening of WAD file and creation of node in Wad linked list
*/
WadPtr BasicWadOpen( char *filename)
{
   WadPtr curw, prevw;

   /* find the WAD file in the Wad file list */
   prevw = WadFileList;
   if (prevw)
   {
      curw = prevw->next;
      while (curw && strcmp( filename, curw->filename))
      {
	 prevw = curw;
	 curw = prevw->next;
      }
   }
   else
      curw = NULL;

   /* if this entry doesn't exist, add it to the WadFileList */
   if (! curw)
   {
      curw = (WadPtr) GetMemory( sizeof( struct WadFileInfo));
      if (! prevw)
	 WadFileList = curw;
      else
	 prevw->next = curw;
      curw->next = NULL;
      curw->filename = filename;
   }

   /* open the file */
   if ((curw->fileinfo = fopen( filename, "rb")) == NULL)
   {
      if (! prevw)
	 WadFileList = NULL;
      else
	 prevw->next = curw->next;
      FreeMemory( curw);
      ProgError( "error opening \"%s\"", filename);
   }

   /* read in the WAD directory info */
   BasicWadRead( curw, curw->type, 4);
   if (strncmp( curw->type, "IWAD", 4) && strncmp( curw->type, "PWAD", 4))
      ProgError( "\"%s\" is not a valid WAD file", filename);
   BasicWadRead( curw, &curw->dirsize, sizeof( curw->dirsize));
   swaplong( &(curw->dirsize));
   BasicWadRead( curw, &curw->dirstart, sizeof( curw->dirstart));
   swaplong( &(curw->dirstart));

   /* read in the WAD directory itself */
   curw->directory = (DirPtr) GetMemory( sizeof( struct Directory) * curw->dirsize);
   BasicWadSeek( curw, curw->dirstart);
   BasicWadRead( curw, curw->directory, sizeof( struct Directory) * curw->dirsize);
#ifdef BIGEND
   {
     int i;
     for (i = 0; i < curw->dirsize; i++)
     {
        DirPtr d = &(curw->directory[ i]);
        swaplong( &(d->start));
        swaplong( &(d->size));
     }
   }
#endif

   /* all done */
   return curw;
}


/*
   read bytes from a file and store it into an address with error checking
*/
void BasicWadRead( WadPtr wadfile, void huge *addr, long size)
{
   if (fread( addr, 1, size, wadfile->fileinfo) != size)
      ProgError( "error reading from \"%s\"", wadfile->filename);
}


/*
   go to offset of a file with error checking
*/
void BasicWadSeek( WadPtr wadfile, long offset)
{
   if (fseek( wadfile->fileinfo, offset, 0))
      ProgError( "seek error on \"%s\" to pos %ld", wadfile->filename, offset);
}


/*
   find an entry in the master directory
*/
MDirPtr FindMasterDir( MDirPtr from, char *name)
{
   while (from)
   {
      if (! strncmp( from->dir.name, name, 8))
	 break;
      from = from->next;
   }
   return from;
}


/*
   check if a file exists and is readable
*/
Bool Exists( char *filename)
{
   FILE *test;

   if (! (test = fopen( filename, "rb")))
      return FALSE;
   fclose( test);
   return TRUE;
}

/* end of file */
