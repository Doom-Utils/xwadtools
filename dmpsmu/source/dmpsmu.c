/*
   DooM PostScript Maps Utility, by Frans P. de Vries.

Derived from:

   Doom Editor Utility, by Brendon Wyber and Raphaël Quinet.

   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README for more information.

   This program comes with absolutely no warranty.

   DMPSMU.C - Main program routines for interactive utility.
*/

/*
 * UM 1999-06-08
 *  - Changed case of WAD files to lower case, I don't have them
 *    in upper case on UNIX systems and Dos/Windows won't care.
 *
 * UM 1999-06-28
 *  - removed strupr(), use version from library.
 */


/* the includes */
#include "dmps.h"
#include "strfunc.h"


/* global variables */
BCINT GameVersion = 0;		/* which game and version? */
				/* if you change this, bad things will happen to you... */
char *MainWad   = "doom.wad";	/* name of the main WAD file */
char *UserLvlNm = NULL;		/* user defined level name */


/* local variables */
char **PatchWads = NULL;	/* list of patch WAD files */

OptDesc options[] =		/* description of the command line options */
{
/*   short & long names	type		message if true/changed	message if false	where to store the value */
   { "W",  "MAIN",	OPT_STRING,	"Main WAD file",	NULL,			&MainWad	},
   { "PW", "PWAD",	OPT_STRINGACC,	"Patch WAD file",	NULL,			&PatchWads	},
   { "",   "FILE",	OPT_STRINGLIST,	"Patch WAD file",	NULL,			&PatchWads	},
   { NULL, NULL,	OPT_END,	NULL,			NULL,			NULL		}
};


/*
   the main program
*/
int main( int argc, char *argv[])
{
   Credits( stdout);
   argv++;
   argc--;
   /* read command line options */
   ParseCommandLineOptions( argc, argv);

   /* load the Wad files */
   OpenMainWad( MainWad);
   if (PatchWads)
      while (PatchWads[ 0])
      {
	 OpenPatchWad( PatchWads[ 0]);
	 PatchWads++;
      }
   /* sanity check */
   CloseUnusedWadFiles();

   /* all systems go! */
   MainLoop();
   /* that's all, folks! */
   CloseWadFiles();
   exit( 0);
}


/*
   append a string to a null-terminated string list
*/
void AppendItemToList( char ***list, char *item)
{
   BCINT i = 0;

   if (*list)
   {
      /* count the number of elements in the list (last = null) */
      while ((*list)[ i])
	 i++;
      /* expand the list */
      *list = (char **) ResizeMemory( *list, (i + 2) * sizeof( char **));
   }
   else
      /* create a new list */
      *list = (char **) GetMemory( 2 * sizeof( char **));

   /* append the new element */
   (*list)[ i] = item;
   (*list)[ i + 1] = NULL;
}


/*
   handle command line options
*/
void ParseCommandLineOptions( int argc, char *argv[])
{
   BCINT optnum;

   while (argc > 0)
   {
      if (argv[ 0][ 0] != '-' && argv[ 0][ 0] != '+')
	 ProgError( "options must start with '-' or '+'");
      strupr( argv[ 0]);
      if (!strcmp( argv[ 0], "-?") || !strcmp( argv[ 0], "-H") ||
	  !strcmp( argv[ 0], "-HELP"))
      {
	 Usage( stdout);
	 exit( 0);
      }
      for (optnum = 0; options[ optnum].opt_type != OPT_END; optnum++)
      {
	 if (!strcmp( &(argv[ 0][ 1]), options[ optnum].short_name) ||
	     !strcmp( &(argv[ 0][ 1]), options[ optnum].long_name))
	 {
	    switch (options[ optnum].opt_type)
	    {
	    case OPT_BOOLEAN:
	       if (argv[ 0][ 0] == '-')
	       {
		  *((Bool *) (options[ optnum].data_ptr)) = TRUE;
		  if (options[ optnum].msg_if_true)
		     printf( "%s.\n", options[ optnum].msg_if_true);
	       }
	       else
	       {
		  *((Bool *) (options[ optnum].data_ptr)) = FALSE;
		  if (options[ optnum].msg_if_false)
		     printf( "%s.\n", options[ optnum].msg_if_false);
	       }
	       break;
	    case OPT_INTEGER:
	       if (argc <= 1)
		  ProgError( "missing argument after \"%s\"", argv[ 0]);
	       argv++;
	       argc--;
	       *((BCINT *) (options[ optnum].data_ptr)) = atoi( argv[ 0]);
	       if (options[ optnum].msg_if_true)
		  printf( "%s: %d.\n", options[ optnum].msg_if_true, atoi( argv[ 0]));
	       break;
	    case OPT_STRING:
	       if (argc <= 1)
		  ProgError( "missing argument after \"%s\"", argv[ 0]);
	       argv++;
	       argc--;
	       *((char **) (options[ optnum].data_ptr)) = argv[ 0];
	       if (options[ optnum].msg_if_true)
		  printf( "%s: %s.\n", options[ optnum].msg_if_true, argv[ 0]);
	       break;
	    case OPT_STRINGACC:
	       if (argc <= 1)
		  ProgError( "missing argument after \"%s\"", argv[ 0]);
	       argv++;
	       argc--;
	       AppendItemToList( (char ***) options[ optnum].data_ptr, argv[ 0]);
	       if (options[ optnum].msg_if_true)
		  printf( "%s: %s.\n", options[ optnum].msg_if_true, argv[ 0]);
	       break;
	    case OPT_STRINGLIST:
	       if (argc <= 1)
		  ProgError( "missing argument after \"%s\"", argv[ 0]);
	       while (argc > 1 && argv[ 1][ 0] != '-' && argv[ 1][ 0] != '+')
	       {
		  argv++;
		  argc--;
		  AppendItemToList( (char ***) options[ optnum].data_ptr, argv[ 0]);
		  if (options[ optnum].msg_if_true)
		     printf( "%s: %s.\n", options[ optnum].msg_if_true, argv[ 0]);
	       }
	       break;
	    default:
	       ProgError( "unknown option type (BUG!)");
	    }
	    break;
	 }
      }
      if (options[ optnum].opt_type == OPT_END)
	 ProgError( "invalid argument: \"%s\"", argv[ 0]);
      argv++;
      argc--;
   }
}


/*
   output the program usage to the specified file
*/
void Usage( FILE *where)
{
   fprintf( where, "Usage:\n");
   fprintf( where, "dmpsmu  [-w <main_wad_file>] [-pw <pwad_file>] [-file <pwad_files>...]\n");
   fprintf( where, "   -w    Gives the name of the main WAD file (also -main)  Default is doom.wad\n");
   fprintf( where, "   -pw   To add one patch WAD file to be loaded; may be repeated (also -pwad)\n");
   fprintf( where, "   -file To add a list of patch WAD files to be loaded\n");
/* fprintf( where, "Put a '+' instead of a '-' before boolean options to reverse their effect\n"); */
}


/*
   output the credits of the program to the specified file
*/
void Credits( FILE *where)
{
   fprintf( where, "DMPSMU: DooM PostScript Maps Utility, ver %s\n", DMPS_VERSION);
   fprintf( where, " By Frans P. de Vries (fpv@xymph.iaf.nl)\n");
   fprintf( where, "[Derived from DEU v%s by Brendon Wyber and Raphaël Quinet]\n\n", DEU_VERSION);
/* fprintf( where, "Derived from DEU: Doom Editor Utility, ver %s\n\n", DEU_VERSION);
   fprintf( where, " By Raphaël Quinet (quinet@montefiore.ulg.ac.be)\n"
		   "and Brendon J. Wyber (b.wyber@csc.canterbury.ac.nz)\n"
		   " Ported to DJGPP/GO32 by Per Allansson (c91peral@und.ida.liu.se)\n"
		   "and Per Kofod (per@ollie.dnk.hp.com)\n\n"); */
}


/*
   terminate the program reporting an error
*/
void ProgError( char *errstr, ...)
{
   va_list args;

   va_start( args, errstr);
   printf( "\nProgram Error: *** ");
   vprintf( errstr, args);
   printf( " ***\n");
   va_end( args);
   /* clean up & free space */
   ForgetLevelData();
   CloseWadFiles();
   exit( 1);
}


/*
   the main program menu loop
*/
void MainLoop()
{
   WadPtr wad;
   FILE *file;
   char input[ 120];
   char *com, *out;
   BCINT episode, mission;

   for (;;)
   {
      /* get the input */
      printf( "\n[? for help]> ");
      gets( input);
      printf( "\n");

      /* eat the white space and get the first command word */
      com = strtok( input, " ");
      strupr( com);

      /* user just hit return */
      if (com == NULL)
	 printf( "[Please enter a command or ? for help.]\n");

      /* user inputting for help */
      else if (!strcmp( com, "?") || !strcmp( com, "HELP") || !strcmp( com, "H"))
      {
	 printf( "? or H[elp]                       -- to display this text\n");
	 printf( "A[nalyze] episode mission         -- to analyze a game level's statistics\n");
	 printf( "D[ump] <DirEntry> [outfile]       -- to dump a directory entry in hex\n");
	 printf( "F[lags] { <+-><24ABDEFGLMNPSTUZ>} -- to display/set the print flags\n");
	 printf( "L[ist] <WadFile> [outfile]        -- to list the directory of a WAD file\n");
	 printf( "M[aster] [outfile]                -- to list the master directory\n");
	 printf( "N[ame] [name or '']               -- to display/set the user level name\n");
	 printf( "P[rint] episode mission <PSfile>  -- to print a game level to a PostScript file\n");
	 printf( "Q[uit]                            -- to quit\n");
	 printf( "R[ead] <WadFile>                  -- to read a new WAD patch file\n");
	 printf( "W[ads]                            -- to display the open WAD files\n");
	 printf( "X[tract] <DirEntry> <RawFile>     -- to save (extract) one object to a raw file\n");
      }

      /* user asked for list of open WAD files */
      else if (!strcmp( com, "WADS") || !strcmp( com, "W"))
      {
	 printf( "%-20s  IWAD  (Main ", WadFileList->filename);
	 switch (GameVersion % 16)
	 {
	    case 0: printf( "Shareware"); break;
	    case 1: printf( "Registered"); break;
	    case 2: printf( "Commercial"); break;
	    case 4: printf( "Ultimate"); break;
	 }
	 printf( " WAD file)\n");

	 for (wad = WadFileList->next; wad; wad = wad->next)
	 {
	    if (GameVersion == 2 && wad->directory[ 0].name[ 0] == 'M' &&
		 		    wad->directory[ 0].name[ 1] == 'A' &&
				    wad->directory[ 0].name[ 2] == 'P')
	       printf( "%-20s  PWAD  (Patch WAD file for level %c%c)\n",
			wad->filename, wad->directory[ 0].name[ 3], wad->directory[ 0].name[ 4]);
	    else if (GameVersion != 2 && wad->directory[ 0].name[ 0] == 'E' &&
					 wad->directory[ 0].name[ 2] == 'M')
	       printf( "%-20s  PWAD  (Patch WAD file for episode %c mission %c)\n",
			wad->filename, wad->directory[ 0].name[ 1], wad->directory[ 0].name[ 3]);
	    else
	    {
	       /* kludge */
	       strncpy( input, wad->directory[ 0].name, 8);
	       input[ 8] = '\0';
	       printf( "%-20s  PWAD  (Patch WAD file for %s)\n", wad->filename, input);
	    }
	 }
      }

      /* user asked to quit */
      else if (!strcmp( com, "QUIT") || !strcmp( com, "Q"))
      {
	 if (GameVersion == 0)
	    printf("Remember to register your copy of DOOM!\n");
	 else if (GameVersion == 16)
	    printf("Remember to register your copy of Heretic!\n");
	 printf( "Goodbye...\n");
	 break;
      }

      /* user asked to display/set the print flags */
      else if (!strcmp( com, "FLAGS") || !strcmp( com, "F"))
      {
	 com = strtok( NULL, " ");
	 if (com == NULL)
	    DisplayFlags();
	 else
	    while (com != NULL)
	    {
	       strupr( com);
	       if (com[ 0] != '+' && com[ 0] != '-')
		  printf( "[Flag must start with '+' or '-'.]\n");
	       else if (!strchr( "24ABDEFGLMNPSTUZ", com[ 1]))
		  printf( "[Flag must be one of '24ABDEFGLMNPSTUZ'.]\n");
	       else if (com[ 2] != ' ' && com[ 2] != '\0')
		  printf( "[Flag must be one character.]\n");
	       else
		  SetFlag( com[ 1], com[ 0]);
	       com = strtok( NULL, " ");
	    }
      }

      /* user asked to display/set the user level name */
      else if (!strcmp( com, "NAME") || !strcmp( com, "N"))
      {
	 com = strtok( NULL, " ");
	 if (com == NULL)
	    if (UserLvlNm != NULL)
	       printf( "User level name : %s\n", UserLvlNm);
	    else
	       printf( "[User level name is not set.]\n");
	 else
	    if (! strcmp( com, "''"))
	    {
	       /* reset user level name */
	       FreeMemory( UserLvlNm);
	       UserLvlNm = NULL;
	       printf( "User level name reset\n");
	    }
	    else
	    {
	       /* set user level name */
	       UserLvlNm = (char *) GetMemory( (strlen( com) + 1) * sizeof( char));
	       strcpy( UserLvlNm, com);
	       while ((com = strtok( NULL, " ")) != NULL)
	       {
		  UserLvlNm = (char *) ResizeMemory( UserLvlNm, (strlen( UserLvlNm) + 1 +
								 strlen( com)) * sizeof( char));
		  strcat( UserLvlNm, " ");
		  strcat( UserLvlNm, com);
	       }
	       printf( "User level name set : %s\n", UserLvlNm);
	    }
      }

      /* user asked to print a level */
      else if (!strcmp( com, "PRINT") || !strcmp( com, "P"))
      {
	 com = strtok( NULL, " ");
	 if (com == NULL)
	 {
	    printf( "[Episode number argument missing.]\n");
	    continue;
	 }
	 episode = atoi( com);
	 com = strtok( NULL, " ");
	 if (com == NULL)
	 {
	    printf( "[Mission number argument missing.]\n");
	    continue;
	 }
	 mission = atoi( com);

	 if (GameVersion == 2)
	 {
	    if (episode != 0)
	    {
	       printf( "[Invalid game episode number (%d).]\n", episode);
	       continue;
	    }
	    if (mission < 1 || mission > 32)
	    {
	       printf( "[Invalid game mission number (%d).]\n", mission);
	       continue;
	    }
	 }
	 else /* GameVersion != 2 */
	    if (!(GameVersion == 17 && episode == 4 && mission == 1)) /* Heretic E4M1 */
	    {
	       if (episode < 1 || episode > (GameVersion == 4 ? 4 : (GameVersion & 1 ? 3 : 1)))
	       {
		  printf( "[Invalid game episode number (%d).]\n", episode);
		  continue;
	       }
	       if (mission < 1 || mission > 9)
	       {
		  printf( "[Invalid game mission number (%d).]\n", mission);
		  continue;
	       }
	    }

	 out = strtok( NULL, " ");
	 if (! out)
	 {
	    printf( "[PostScript file name argument missing.]\n");
	    continue;
	 }
	 if (GameVersion == 2)
	    printf( "Outputting PostScript map of level MAP%02d to \"%s\".\n",
		     mission, out);
	 else
	    printf( "Outputting PostScript map of level E%dM%d to \"%s\".\n",
		     episode, mission, out);
	 if ((PSFile = fopen( out, "wt")) == NULL)
	    ProgError( "error opening output file \"%s\"", out);
	 fprintf( PSFile, "%%! DMPSMU: DooM PostScript Maps Utility, ver %s\n",
			   DMPS_VERSION);
	 PrintLevel( episode, mission);
	 fprintf( PSFile, "\n%%%% End of file.\n");
	 fclose( PSFile);
      }

      /* user asked to analyze a level */
      else if (!strcmp( com, "ANALYZE") || !strcmp( com, "A"))
      {
	 com = strtok( NULL, " ");
	 if (com == NULL)
	 {
	    printf( "[Episode number argument missing.]\n");
	    continue;
	 }
	 episode = atoi( com);
	 com = strtok( NULL, " ");
	 if (com == NULL)
	 {
	    printf( "[Mission number argument missing.]\n");
	    continue;
	 }
	 mission = atoi( com);

	 if (GameVersion == 2)
	 {
	    if (episode != 0)
	    {
	       printf( "[Invalid game episode number (%d).]\n", episode);
	       continue;
	    }
	    if (mission < 1 || mission > 32)
	    {
	       printf( "[Invalid game mission number (%d).]\n", mission);
	       continue;
	    }
	 }
	 else /* GameVersion != 2 */
	    if (!(GameVersion == 17 && episode == 4 && mission == 1)) /* Heretic E4M1 */
	    {
	       if (episode < 1 || episode > (GameVersion == 4 ? 4 : (GameVersion & 1 ? 3 : 1)))
	       {
		  printf( "[Invalid game episode number (%d).]\n", episode);
		  continue;
	       }
	       if (mission < 1 || mission > 9)
	       {
		  printf( "[Invalid game mission number (%d).]\n", mission);
		  continue;
	       }
	    }

	 AnalyzeLevel( episode, mission);
      }

      /* user ask for a listing of a WAD file */
      else if (!strcmp( com, "LIST") || !strcmp( com, "L"))
      {
	 com = strtok( NULL, " ");
	 if (com == NULL)
	 {
	    printf( "[Wad file name argument missing.]\n");
	    continue;
	 }
	 for (wad = WadFileList; wad; wad = wad->next)
	    if (!strcmp( com, wad->filename))
	       break;
	 if (! wad)
	 {
	    printf( "[Wad file \"%s\" is not open.]\n", com);
	    continue;
	 }
	 out = strtok( NULL, " ");
	 if (out)
	 {
	    printf( "Outputting directory of \"%s\" to \"%s\".\n", wad->filename, out);
	    if ((file = fopen( out, "wt")) == NULL)
	       ProgError( "error opening output file \"%s\"", out);
	    Credits( file);
	    ListFileDirectory( file, wad);
	    fprintf( file, "\nEnd of file.\n");
	    fclose( file);
	 }
	 else
	    ListFileDirectory( stdout, wad);
      }

      /* user asked for the list of the master directory */
      else if (!strcmp( com, "MASTER") || !strcmp( com, "M"))
      {
	 out = strtok( NULL, " ");
	 if (out)
	 {
	    printf( "Outputting master directory to \"%s\".\n", out);
	    if ((file = fopen( out, "wt")) == NULL)
	       ProgError( "error opening output file \"%s\"", out);
	    Credits( file);
	    ListMasterDirectory( file);
	    fprintf( file, "\nEnd of file.\n");
	    fclose( file);
	 }
	 else
	    ListMasterDirectory( stdout);
      }

      /* user asked to read a new patch WAD file */
      else if (!strcmp( com, "READ") || !strcmp( com, "R"))
      {
	 com = strtok( NULL, " ");
	 if (com == NULL)
	 {
	    printf( "[Wad file name argument missing.]\n");
	    continue;
	 }
	 out = strtok( NULL, " ");
	 if (out)
	    *out = '\0';
	 out = (char *) GetMemory( (strlen( com) + 1) * sizeof( char));
	 strcpy( out, com);
	 OpenPatchWad( out);
	 CloseUnusedWadFiles();
      }

      /* user asked to dump the contents of a WAD file entry */
      else if (!strcmp( com, "DUMP") || !strcmp( com, "D"))
      {
	 com = strtok( NULL, " ");
	 if (com == NULL)
	 {
	    printf( "[Object name argument missing.]\n");
	    continue;
	 }
	 strupr( com);
	 out = strtok( NULL, " ");
	 if (out)
	 {
	    printf( "Outputting directory entry data to \"%s\".\n", out);
	    if ((file = fopen( out, "wt")) == NULL)
	       ProgError( "error opening output file \"%s\"", out);
	    Credits( file);
	    DumpDirectoryEntry( file, com);
	    fprintf( file, "\nEnd of file.\n");
	    fclose( file);
	 }
	 else
	    DumpDirectoryEntry( stdout, com);
      }

      /* user asked to extract an object to a raw binary file */
      else if (!strcmp( com, "XTRACT") || !strcmp( com, "X") || !strcmp( com, "EXTRACT"))
      {
	 com = strtok( NULL, " ");
	 if (com == NULL)
	 {
	    printf( "[Object name argument missing.]\n");
	    continue;
	 }
	 if (strlen( com) > 8 || strchr( com, '.'))
	 {
	    printf( "[Invalid object name.]\n");
	    continue;
	 }
	 strupr( com);
	 out = strtok( NULL, " ");
	 if (! out)
	 {
	    printf( "[Raw file name argument missing.]\n");
	    continue;
	 }
	 for (wad = WadFileList; wad; wad = wad->next)
	    if (!strcmp( out, wad->filename))
	       break;
	 if (wad)
	 {
	    printf( "[You may not overwrite an opened Wad file with raw data.]\n");
	    continue;
	 }
	 printf( "Saving directory entry data to \"%s\".\n", out);
	 if ((file = fopen( out, "wb")) == NULL)
	    ProgError( "error opening output file \"%s\"", out);
	 SaveEntryToRawFile( file, com);
	 fclose( file);
      }

      /* unknown command */
      else
	 printf( "[Unknown command \"%s\"!]\n", com);
   }
}

/* end of file */
