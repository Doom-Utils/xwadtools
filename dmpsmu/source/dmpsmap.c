/*
   DooM PostScript Map sidekick, by Frans P. de Vries.

Derived from:

   Doom Editor Utility, by Brendon Wyber and Raphaël Quinet.

   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README for more information.

   This program comes with absolutely no warranty.

   DMPSMAP.C - Main program routines for command-line sidekick.
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
#include "print.h"
#include "strfunc.h"


/* global variables */
BCINT GameVersion = 0;		/* which game and version? */
				/* if you change this, bad things will happen to you... */
char *MainWad   = "doom.wad";	/* name of the main WAD file */
char *UserLvlNm = NULL;		/* user defined level name */


/* local variables */
char **PatchWads = NULL;	/* list of patch WAD files */
char *PSFlName   = NULL;	/* name of the output PostScript file */
BCINT Episode    = 0;		/* the episode number */
BCINT Mission    = 0;		/* the mission number */

OptDesc options[] =		/* description of the command line options */
{
/*   short & long names	type		message if true/changed	message if false	where to store the value */
   { "W",  "MAIN",	OPT_STRING,	"Main WAD file",	NULL,			&MainWad	},
   { "PW", "PWAD",	OPT_STRINGACC,	"Patch WAD file",	NULL,			&PatchWads	},
   { "",   "FILE",	OPT_STRINGLIST,	"Patch WAD file",	NULL,			&PatchWads	},
   { "F2", "",		OPT_BOOLEAN,	"Use double page",	"Use normal page",	&FlagDoublePg	},
   { "F4", "",		OPT_BOOLEAN,	"Use quadruple page",	"Use normal page",	&FlagQuadplPg	},
   { "FA", "",		OPT_BOOLEAN,	"Use A4 paper",		"Use Letter paper",	&FlagA4Letter	},
   { "FB", "",		OPT_BOOLEAN,	"Print border",		"Print no border",	&FlagBorder	},
   { "FD", "",		OPT_BOOLEAN,	"Print decorations",	"Print no decorations",	&FlagDecors	},
   { "FE", "",		OPT_BOOLEAN,	"Print enemies",	"Print no enemies",	&FlagEnemies	},
   { "FF", "",		OPT_BOOLEAN,	"Fade secret lines",	"Don't fade lines",	&FlagFadeLins	},
   { "FG", "",		OPT_BOOLEAN,	"Print goodies",	"Print no goodies",	&FlagGoodies	},
   { "FL", "",		OPT_BOOLEAN,	"Print legend",		"Print no legend",	&FlagLegend	},
   { "FM", "",		OPT_BOOLEAN,	"Print multi things",	"Print no multi things",&FlagMultPlay	},
   { "FN", "",		OPT_BOOLEAN,	"Print name",		"Print no name",	&FlagName	},
   { "FP", "",		OPT_BOOLEAN,	"Use Portrait",		"Use Landscape",	&FlagPortrait	},
   { "FS", "",		OPT_BOOLEAN,	"Shade secret areas",	"Don't shade areas",	&FlagShdAreas	},
   { "FT", "",		OPT_BOOLEAN,	"Link teleports",	"Don't link teleports",	&FlagTeleprts	},
   { "FU", "",		OPT_BOOLEAN,	"Print UV things",	"Print non-UV things",	&FlagUltraVlc	},
   { "FZ", "",		OPT_BOOLEAN,	"Auto-zoom scale",	"Default scale",	&FlagAutoZoom	},
   { "N",  "NAME",	OPT_STRING,	"User level name",	NULL,			&UserLvlNm	},
   { "E",  "EPISODE",	OPT_INTEGER,	"Episode number",	NULL,			&Episode	},
   { "M",  "MISSION",	OPT_INTEGER,	"Mission number",	NULL,			&Mission	},
   { "P",  "PSFILE",	OPT_STRING,	"PostScript file",	NULL,			&PSFlName	},
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
	       if (argv[ 0][ 0] == '+') /* '+/-' reversed for print flags */
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
   fprintf( where, "dmpsmap [-w <main_wad_file>] [-pw <pwad_file>] [-file <pwad_files>...] [+-f<>]\n");
   fprintf( where, "        [-n <name>] -e <episode> -m <mission> -p <PostScript_file>\n");
   fprintf( where, "   -w    Gives the name of the main WAD file (also -main)  Default is doom.wad\n");
   fprintf( where, "   -pw   To add one patch WAD file to be loaded; may be repeated (also -pwad)\n");
   fprintf( where, "   -file To add a list of patch WAD files to be loaded\n");
   fprintf( where, "   +-f<> Sets (+) or resets (-) a print flag <24ABDEFGLMNPSTUZ>; may be repeated\n");
   fprintf( where, "   -n    Defines the user level name (also -name)\n");
   fprintf( where, "   -e    Gives the episode number (also -episode); required\n");
   fprintf( where, "   -m    Gives the mission number (also -mission); required\n");
   fprintf( where, "   -p    Gives the name of the output PostScript file (also -psfile); required\n");
/* fprintf( where, "Put a '+' instead of a '-' before boolean options to reverse their effect\n"); */
}


/*
   output the credits of the program to the specified file
*/
void Credits( FILE *where)
{
   fprintf( where, "DMPSMAP: DooM PostScript Map sidekick, ver %s\n", DMPS_VERSION);
   fprintf( where, " By Frans P. de Vries (fpv@xymph.iaf.nl)\n");
   fprintf( where, "[Derived from DEU v%s by Brendon Wyber and Raphaël Quinet]\n\n", DEU_VERSION);
/* fprintf( where, "Derived from DEU: Doom Editor Utility, ver %s\n\n", DEU_VERSION);
   fprintf( where, " By Raphaël Quinet (quinet@montefiore.ulg.ac.be)\n"
		   "and Brendon J Wyber (b.wyber@csc.canterbury.ac.nz)\n"
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
   the main program "loop" (inherited from 'dmpsmu.c' to enable a single 'dmps.h')
*/
void MainLoop()
{
   if (GameVersion == 2)
   {
      if (Episode != 0)
	 ProgError( "invalid or missing game episode number (%d)", Episode);
      if (Mission < 1 || Mission > 32)
	 ProgError( "invalid or missing game mission number (%d)", Mission);
   }
   else /* GameVersion != 2 */
      if (!(GameVersion == 17 && Episode == 4 && Mission == 1)) /* Heretic E4M1 */
      {
	 if (Episode < 1 || Episode > (GameVersion == 4 ? 4 : (GameVersion & 1 ? 3 : 1)))
	    ProgError( "invalid or missing game episode number (%d)", Episode);
	 if (Mission < 1 || Mission > 9)
	    ProgError( "invalid or missing game mission number (%d)", Mission);
      }

   if (PSFlName == NULL)
      ProgError( "PostScript file name argument missing");

   if (GameVersion == 2)
      printf( "\nOutputting PostScript map of level MAP%02d to \"%s\".\n",
	       Mission, PSFlName);
   else
      printf( "\nOutputting PostScript map of level E%dM%d to \"%s\".\n",
	       Episode, Mission, PSFlName);
   if ((PSFile = fopen( PSFlName, "wt")) == NULL)
      ProgError( "error opening output file \"%s\"", PSFlName);
   fprintf( PSFile, "%%! DMPSMAP: DooM PostScript Map sidekick, ver %s\n", DMPS_VERSION);
   PrintLevel( Episode, Mission);
   fprintf( PSFile, "\n%%%% End of file.\n");
   fclose( PSFile);

   if (GameVersion == 0)
      printf("Remember to register your copy of DOOM!\n");
   else if (GameVersion == 16)
      printf("Remember to register your copy of Heretic!\n");
}

/* end of file */
