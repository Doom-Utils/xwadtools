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

   LIST.C - List routines.
*/


/* the includes */
#include "wadtxls.h"
#include "levels.h"


/* textures uses */
#define	Upper	0x0001
#define	Lower	0x0002
#define	Normal	0x0004
#define	Floor	0x0010
#define	Ceil	0x0020

/* local list structures */
typedef struct NumList huge *NmPtr;
struct NumList
{
   BCINT num;		/* SideDef/Sector number */
   BCINT use;		/* use of this texture   */
   NmPtr next;
};
typedef struct TexList huge *TxPtr;
struct TexList
{
   char  tex[ 8];	/* texture name          */
   BCINT cnt;		/* count of this texture */
   TxPtr next;
   NmPtr first,		/* verbose numbers list  */
	 last;
};
TxPtr List = NULL;	/* textures list */


/*
   read level & list its wall textures and/or flats
*/
void ListLevel( BCINT episode, BCINT mission)
{
   ReadLevelData( episode, mission);

   if (GameVersion == 0)
      fprintf( TxFile, "# Shareware DOOM  Textures list for level E%dM%d: %s\n\n", episode, mission,
			( UserLvlNm != NULL ? UserLvlNm : LevelName ));
   else if (GameVersion == 1)
      fprintf( TxFile, "# DOOM  Textures list for level E%dM%d: %s\n\n", episode, mission,
			( UserLvlNm != NULL ? UserLvlNm : LevelName ));
   else if (GameVersion == 2)
      fprintf( TxFile, "# DOOM ][  Textures list for level MAP%02d: %s\n\n", mission,
			( UserLvlNm != NULL ? UserLvlNm : LevelName ));
   else if (GameVersion == 4)
      fprintf( TxFile, "# Ultimate DOOM  Textures list for level E%dM%d: %s\n\n", episode, mission,
			( UserLvlNm != NULL ? UserLvlNm : LevelName ));
   else if (GameVersion == 16)
      fprintf( TxFile, "# Shareware Heretic  Textures list for level E%dM%d: %s\n\n", episode, mission,
			( UserLvlNm != NULL ? UserLvlNm : LevelName ));
   else if (GameVersion == 17)
      fprintf( TxFile, "# Heretic  Textures list for level E%dM%d: %s\n\n", episode, mission,
			( UserLvlNm != NULL ? UserLvlNm : LevelName ));

   /* list desired textures */
   if (InclTexts)
   {
      BuildTexts();

      fprintf( TxFile, "Wall Textures\n");
      fprintf( TxFile, "=============\n\n");

      PrintList();
      fprintf( TxFile, "\n");
      ForgetList();
   }
   if (InclFlats)
   {
      BuildFlats();

      fprintf( TxFile, "Flats\n");
      fprintf( TxFile, "=====\n\n");

      VerbLine = FALSE; /* no LineDefs */
      PrintList();
      fprintf( TxFile, "\n");
      ForgetList();
   }

   /* clean up & free space */
   ForgetLevelData();
}


/*
   build list of wall textures in SideDefs
*/
void BuildTexts( void)
{
   BCINT n;

   for (n = 0; n < NumSideDefs; n++)
   {
      if (SideDefs[ n].tex1[ 0] != '-')
	 AddToTexList( n, SideDefs[ n].tex1, Upper);
      if (SideDefs[ n].tex2[ 0] != '-')
	 AddToTexList( n, SideDefs[ n].tex2, Lower);
      if (SideDefs[ n].tex3[ 0] != '-')
	 AddToTexList( n, SideDefs[ n].tex3, Normal);
   }
}

/*
   build list of flats in Sectors
*/
void BuildFlats( void)
{
   BCINT n;

   for (n = 0; n < NumSectors; n++)
   {
      AddToTexList( n, Sectors[ n].floort, Floor);
      AddToTexList( n, Sectors[ n].ceilt,  Ceil);
   }
}


/*
   add texture to list
*/
void AddToTexList( BCINT num, char *tex, BCINT use)
{
   TxPtr curr = List, temp = NULL, new;
   char  texname[ 9];

   /* map name to uppercase */
   texname[ 8] = '\0';
   strncpy( texname, tex, 8);
   strupr( texname);
   strncpy( tex, texname, 8);

   /* find insertion point */
   while (curr != NULL && strncmp( curr->tex, tex, 8) < 0)
   {
      temp = curr;
      curr = curr->next;
   }

   /* check for existing entry */
   if (curr != NULL && strncmp( curr->tex, tex, 8) == 0)
   {
      curr->cnt++;
      AddToNumList( (char *) curr, num, use);
   }
   else /* curr == NULL || strncmp( curr->tex, tex, 8) > 0 */
   {
      /* make new entry */
      new = (TxPtr) GetFarMemory( sizeof( struct TexList));
      strncpy( new->tex, tex, 8);
      new->cnt = 1;
      new->next = NULL;
      new->first = new->last = NULL;
      AddToNumList( (char *) new, num, use);

      /* check for head of list */
      if (temp == NULL)
      {
	 new->next = List;
	 List = new;
      }
      else
	 /* check for tail of list */
	 if (curr == NULL)
	    temp->next = new;
	 else /* insert before curr */
	 {
	    new->next = curr;
	    temp->next = new;
	 }
   }
}

/*
   add SideDef to list
*/
void AddToNumList( char *txp, BCINT num, BCINT use)
{
   TxPtr texp = (TxPtr) txp;
   NmPtr curr = texp->last, new;

   /* check for existing entry */
   if (curr != NULL && curr->num == num)
      curr->use |= use;
   else /* curr == NULL || curr->num > num */
   {
      /* make new entry */
      new = (NmPtr) GetFarMemory( sizeof( struct NumList));
      new->num = num;
      new->use = use;
      new->next = NULL;

      /* check for empty list */
      if (curr == NULL)
	 texp->first = texp->last = new;
      else /* append to tail */
      {
	 curr->next = new;
	 texp->last = new;
      }
   }
}


/*
   print textures list
*/
void PrintList( void)
{
   TxPtr curt = List;
   NmPtr curn;
   char  texname[ 9];
   BCINT total = 0, cols, line;
   Bool  tab;

   texname[ 8] = '\0';

   /* print all TexList entries */
   while (curt != NULL)
   {
      total += curt->cnt;
      strncpy( texname, curt->tex, 8);
      fprintf( TxFile, "%5d %s", curt->cnt, texname);

      /* list all SideDef/Sector numbers? */
      if (Verbose)
      {
	 cols = 0;
	 tab = TRUE;
	 curn = curt->first;

	 /* print all NumList entries */
	 while (curn != NULL)
	 {
	    if (tab)
	       fprintf( TxFile, "\t");
	    fprintf( TxFile, " %4d", curn->num);
	    if (curn->use & Upper)
	       fprintf( TxFile, "U");
	    if (curn->use & Lower)
	       fprintf( TxFile, "L");
	    if (curn->use & Normal)
	       fprintf( TxFile, "N");
	    if (curn->use & Floor)
	       fprintf( TxFile, "F");
	    if (curn->use & Ceil)
	       fprintf( TxFile, "C");
	    /* check for column tab */
	    tab = (curn->use != Upper + Lower + Normal);

	    /* list all LineDef numbers? */
	    if (VerbLine)
	    {
	       if (tab)
		  fprintf( TxFile, "\t");
	       if ((line = FindLineDef( curn->num)) > 0)
		  fprintf( TxFile, "=%4d-1", line-1);
	       else /* line < 0 */
		  fprintf( TxFile, "=%4d-2", -line-1);
	       ++cols;
	       tab = TRUE;
	    }

	    /* check for last column */
	    if ((++cols % 8) == 0 && curn->next != NULL)
	    {
	       fprintf( TxFile, "\n\t");
	       tab = TRUE;
	    }

	    curn = curn->next;
	 }
      }
      fprintf( TxFile, "\n");

      curt = curt->next;
   }

   fprintf( TxFile, "-----\n%5d\n", total);
}

/*
   find LineDef(+1) corresponding to SideDef
*/
BCINT FindLineDef( BCINT side)
{
   BCINT n;

   /* check all LineDefs */
   for (n = 0; n < NumLineDefs; n++)
      if (LineDefs[ n].sidedef1 == side)
	 return (n+1);
      else
	 if (LineDefs[ n].sidedef2 == side)
	    return (-(n+1));

   /* never get here, just to prevent compiler warnings */
   return (BCINT)0;
}

/*
   forget textures list
*/
void ForgetList( void)
{
   TxPtr curt = List;

   /* free all TexList entries */
   while (curt != NULL)
   {
      TxPtr tmpt = curt;
      NmPtr curn = curt->first;

      /* free all NumList entries */
      while (curn != NULL)
      {
	 NmPtr tmpn = curn;

	 curn = curn->next;
	 FreeFarMemory( tmpn);
      }

      curt = curt->next;
      FreeFarMemory( tmpt);
   }
   List = NULL;
}

/* end of file */
