/* WADEXT.C extract a lump in raw format from a WAD file */
/* Stefan Gustavson 1994 */
/* Modified for the Cywin tools by Udo Munk <munkudo@aol.com> 14/12/98 */
/* Made output pipeable, Udo Munk <munkudo@aol.com> 1999-06-11 */

#include <stdio.h>
#include <string.h>
#include "sysdep.h"

int main(argc, argv)
     int argc;
     char *argv[];
{
  FILE *wadfile, *rawfile;
  char identifier[4];
  int i, j, numentries, dirstart, entrypos, entrysize;
  char entryname[8];
  int quiet = 0;	/* no messages to stdout */
  
  if (argc != 4)
    {
      fprintf(stderr, "Usage: %s wadfile entryname datafile\n", argv[0]);
      exit(-1);
    }

  if (!strcmp(argv[3], "-"))
    {
      rawfile = stdout;
      quiet++;
    }
  else
    rawfile = fopen(argv[3], "wb");
  if(rawfile == NULL)
    {
      fprintf(stderr, "Unable to open output file %s.\n", argv[3]);
      exit(-1);
    }
  
  wadfile = fopen(argv[1], "rb");
  if(wadfile == NULL)
    {
      fprintf(stderr, "File %s not found.\n", argv[1]);
      exit(-1);
    }
  fread(identifier, 4, 1, wadfile);
  if(strncmp("PWAD", identifier, 4) & strncmp("IWAD", identifier, 4))
    {
      fprintf(stderr, "File %s is not a WAD file.\n", argv[1]);
      exit(-1);
    }
  numentries = getlong(wadfile);
  dirstart = getlong(wadfile);
  if(fseek(wadfile, dirstart, 0))
    {
      fprintf(stderr, "File seek error.\n");
      exit(-1);
    }
 for(i=0; i<numentries; i++)
   {
     entrypos = getlong(wadfile);
     entrysize = getlong(wadfile);
     fread(entryname, 8, 1, wadfile);
     if(!strncmp(entryname, argv[2], 8))
       {
	 if (!quiet)
	   printf("Extracting entry %.8s (%d bytes)... ", entryname, entrysize);
	 fseek(wadfile, entrypos, 0);
	 for(j=0; j<entrysize; j++)
	   putc(getc(wadfile), rawfile);
	 if (!quiet)
	   printf("Done.\n");
	 fclose(wadfile);
	 fclose(rawfile);
	 exit(0);
       }
   }
  fprintf(stderr, "Entry %s not found!\n", argv[2]);
  fclose(wadfile);
  fclose(rawfile);
  return 1;
}
