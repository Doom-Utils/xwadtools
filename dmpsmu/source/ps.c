/*
   DooM PostScript Maps Utility, by Frans P. de Vries.

Derived from:

   Doom Editor Utility, by Brendon Wyber and Raphaël Quinet.

   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README for more information.

   This program comes with absolutely no warranty.

   PS.C - PostScript routines.
*/


/* the includes */
#include "dmps.h"
#include "things.h"
#include "print.h"


/* global variable */
FILE *PSFile;		/* the PostScript output file */


/* local variables */
float Scale;		/* the scale value */
BCINT PageMaxX;		/* maximum X page coord */
BCINT PageMaxY;		/* maximum Y page coord */
BCINT PageCenterX;	/* X coord of page center */
BCINT PageCenterY;	/* Y coord of page center */

BCINT clipXn;		/* X coord of name clip area */
BCINT clipXl;		/* X coord of legend clip area */
BCINT clipY;		/* Y coord of name/legend clip area */


/* definitions */
#define PPI	72	/* points per inch */
#define PBM	18	/* page border margin = 0.25 inch */
#define BMO	4	/* border margin offset 4 points */
#define NFS	16	/* name font size */
#define LFS	7	/* legend font size */
#define LBW	42	/* legend box width */
#define LTR	3	/* legend thing radius */

#define DEFSCLA	12.2	/* default map scale A4 */
#define DEFSCLL	11.9	/* default map scale Letter */


/* macros to convert map coordinates to page coordinates */
#define PAGEX(x)	(PageCenterX + (BCINT) (((x) - MapCenterX) * Scale))
#define PAGEY(y)	(PageCenterY + (BCINT) (((y) - MapCenterY) * Scale))


/*
   initialise the PostScript page
*/
void InitPage( BCINT episode, BCINT mission, char *name)
{
   if (GameVersion == 0)
      fprintf( PSFile, "%%%% Shareware DOOM  Map for level E%dM%d: %s\n", episode, mission, name);
   else if (GameVersion == 1)
      fprintf( PSFile, "%%%% DOOM  Map for level E%dM%d: %s\n", episode, mission, name);
   else if (GameVersion == 2)
      fprintf( PSFile, "%%%% DOOM ][  Map for level MAP%02d: %s\n", mission, name);
   else if (GameVersion == 4)
      fprintf( PSFile, "%%%% Ultimate DOOM  Map for level E%dM%d: %s\n", episode, mission, name);
   else if (GameVersion == 16)
      fprintf( PSFile, "%%%% Shareware Heretic  Map for level E%dM%d: %s\n", episode, mission, name);
   else if (GameVersion == 17)
      fprintf( PSFile, "%%%% Heretic  Map for level E%dM%d: %s\n", episode, mission, name);

   fprintf( PSFile, "\n%% initial settings\n");
   fprintf( PSFile, "1 setlinecap 1 setlinejoin\n");

   /* define page dimensions & orientation */
   if (FlagA4Letter)
   {
      /* A4 paper is 210 * 297 mm ~= 8.27 * 11.70 inch (rounded up) */
      PageMaxX =  8.27 * PPI; /* = 595 */
      PageMaxY = 11.70 * PPI; /* = 842 */
   }
   else
   {
      /* Letter paper is 8.5 * 11.0 inch */
      PageMaxX =  8.5 * PPI; /* = 612 */
      PageMaxY = 11.0 * PPI; /* = 792 */
   }

   if (FlagDoublePg)
   {
      PageMaxX = 2 * PageMaxX - 2 * PBM;
      exch( PageMaxX, PageMaxY);
      fprintf( PSFile, "\n%% center double page\n");
      fprintf( PSFile, "0 %d translate\n", PBM);

      fprintf( PSFile, "%%%% start upper transformation to normal page\n");
      fprintf( PSFile, "%% -90 rotate %d %d translate\n", - PageMaxX + PBM, - PageMaxY / 2 + PBM);
      fprintf( PSFile, "%% %d %d moveto %d %d lineto %d %d lineto %d %d lineto closepath clip\n",
			PageMaxX, PageMaxY,     PageMaxX, PageMaxY / 2,
			0,        PageMaxY / 2, 0,        PageMaxY);
      fprintf( PSFile, "%%%% end upper transformation to normal page\n");
      fprintf( PSFile, "%%%% start lower transformation to normal page\n");
      fprintf( PSFile, "%% 90 rotate %d %d translate\n", - PBM, - PageMaxY / 2 - PBM);
      fprintf( PSFile, "%% %d %d moveto %d %d lineto %d %d lineto %d %d lineto closepath clip\n",
			PageMaxX, PageMaxY / 2, PageMaxX, 0,
			0,        0,            0,        PageMaxY / 2);
      fprintf( PSFile, "%%%% end lower transformation to normal page\n");
   }

   if (FlagQuadplPg)
   {
      PageMaxX = 2 * PageMaxX - 2 * PBM;
      PageMaxY = 2 * PageMaxY - 2 * PBM;
      fprintf( PSFile, "\n%% center quadruple page\n");
      fprintf( PSFile, "%d %d translate\n", PBM, PBM);

      fprintf( PSFile, "%%%% start upper-left transformation to normal page\n");
      fprintf( PSFile, "%% %d %d translate\n", - PBM, - PageMaxY / 2);
      fprintf( PSFile, "%% %d %d moveto %d %d lineto %d %d lineto %d %d lineto closepath clip\n",
			PageMaxX / 2, PageMaxY,     PageMaxX / 2, PageMaxY / 2,
			0,            PageMaxY / 2, 0,            PageMaxY);
      fprintf( PSFile, "%%%% end upper-left transformation to normal page\n");
      fprintf( PSFile, "%%%% start upper-right transformation to normal page\n");
      fprintf( PSFile, "%% %d %d translate\n", - PageMaxX / 2, - PageMaxY / 2);
      fprintf( PSFile, "%% %d %d moveto %d %d lineto %d %d lineto %d %d lineto closepath clip\n",
			PageMaxX,     PageMaxY,     PageMaxX,     PageMaxY / 2,
			PageMaxX / 2, PageMaxY / 2, PageMaxX / 2, PageMaxY);
      fprintf( PSFile, "%%%% end upper-right transformation to normal page\n");
      fprintf( PSFile, "%%%% start lower-left transformation to normal page\n");
      fprintf( PSFile, "%% %d %d translate\n", - PBM, - PBM);
      fprintf( PSFile, "%% %d %d moveto %d %d lineto %d %d lineto %d %d lineto closepath clip\n",
			PageMaxX / 2, PageMaxY / 2, PageMaxX / 2, 0,
			0,            0,            0,            PageMaxY / 2);
      fprintf( PSFile, "%%%% end lower-left transformation to normal page\n");
      fprintf( PSFile, "%%%% start lower-right transformation to normal page\n");
      fprintf( PSFile, "%% %d %d translate\n", - PageMaxX / 2, - PBM);
      fprintf( PSFile, "%% %d %d moveto %d %d lineto %d %d lineto %d %d lineto closepath clip\n",
			PageMaxX,     PageMaxY / 2, PageMaxX,     0,
			PageMaxX / 2, 0,            PageMaxX / 2, PageMaxY / 2);
      fprintf( PSFile, "%%%% end lower-right transformation to normal page\n");
   }

   if (FlagPortrait)
      fprintf( PSFile, "\n%% default Portrait %s page (X * Y = %d * %d points)\n",
			( FlagA4Letter ? ( FlagQuadplPg ? "A2" :
					   ( FlagDoublePg ? "A3" : "A4" ) )
				       : ( FlagQuadplPg ? "double_Tabloid" :
					   ( FlagDoublePg ? "Tabloid" : "Letter" ) ) ),
			PageMaxX, PageMaxY);
   else
   {
      exch( PageMaxX, PageMaxY);
      fprintf( PSFile, "\n%% set Landscape %s page (X * Y = %d * %d points)\n",
			( FlagA4Letter ? ( FlagQuadplPg ? "A2" :
					   ( FlagDoublePg ? "A3" : "A4" ) )
				       : ( FlagQuadplPg ? "double_Tabloid" :
					   ( FlagDoublePg ? "Tabloid" : "Letter" ) ) ),
			PageMaxX, PageMaxY);
      fprintf( PSFile, "90 rotate 0 %d translate\n", - PageMaxY);
   }

   /* print border & name */
   if (FlagBorder)
   {
      fprintf( PSFile, "\n%% print border (margin = %.2f inch)\n", (float) PBM / PPI);
      fprintf( PSFile, "%d %d moveto %d %d lineto %d %d lineto %d %d lineto closepath\n",
			PageMaxX - PBM, PageMaxY - PBM, PageMaxX - PBM, PBM,
			PBM           , PBM           , PBM           , PageMaxY - PBM);
      fprintf( PSFile, "gsave 2.2 setlinewidth 0 setgray stroke\n");
      fprintf( PSFile, "grestore 0.2 setlinewidth 1 setgray stroke\n");
   }
   if (FlagName)
   {
      fprintf( PSFile, "\n%% print name\n");
      fprintf( PSFile, "/Helvetica-Bold findfont %d scalefont setfont\n", NFS);
      fprintf( PSFile, "0.8 setlinewidth\n");
      fprintf( PSFile, "0.5 setgray %d %d moveto ", PBM + BMO, PageMaxY - PBM - NFS);
      if (GameVersion == 2)
	 fprintf( PSFile, "(MAP%02d: %s) show\n", mission, name);
      else
	 fprintf( PSFile, "(E%dM%d: %s) show\n", episode, mission, name);
      fprintf( PSFile, "  0 setgray %d %d moveto ", PBM + BMO, PageMaxY - PBM - NFS);
      if (GameVersion == 2)
	 fprintf( PSFile, "(MAP%02d: %s) true charpath stroke\n", mission, name);
      else
	 fprintf( PSFile, "(E%dM%d: %s) true charpath stroke\n", episode, mission, name);
   }

   /* print legend bulk */
   if (FlagLegend)
   {
      fprintf( PSFile, "\n%% print legend box & labels\n");
      fprintf( PSFile, "/Helvetica-Bold findfont %d scalefont setfont\n", LFS);
      fprintf( PSFile, "0 setlinewidth 0 setgray\n");
      fprintf( PSFile, "%d %d 2 copy moveto %d %d rlineto %d %d rlineto %d %d rlineto closepath\n",
			PageMaxX - PBM - 1, PageMaxY - PBM - NFS, -5 * LBW, 0, 0, NFS - 1, 5 * LBW, 0);
      fprintf( PSFile, "moveto %d %d rmoveto %d %d rlineto\n", - LBW, 0, 0, NFS - 1);
      fprintf( PSFile, "currentpoint stroke moveto\n");
      if (FlagMultPlay)
	 fprintf( PSFile, "%d %d rmoveto (Scale    M) show\n", 2 * BMO, - LFS);
      else
	 fprintf( PSFile, "%d %d rmoveto (Scale) show\n", 2 * BMO, - LFS);
      fprintf( PSFile, "%d %d moveto %d %d rmoveto currentpoint 2 copy\n",
			PageMaxX - PBM, PageMaxY - PBM, -5 * LBW, 0);
      fprintf( PSFile, "%d %d rmoveto (Start) show moveto\n", BMO + 3 * LTR, - LFS);
      fprintf( PSFile, "%d %d rmoveto (Teleport) show\n", BMO + 3 * LTR, -2 * LFS);
      fprintf( PSFile, "moveto %d %d rmoveto currentpoint 2 copy\n", LBW, 0);
      fprintf( PSFile, "%d %d rmoveto (Bonus) show moveto\n", BMO + 3 * LTR, - LFS);
      fprintf( PSFile, "%d %d rmoveto (Weapon) show\n", BMO + 3 * LTR, -2 * LFS);
      fprintf( PSFile, "moveto %d %d rmoveto currentpoint 2 copy\n", LBW, 0);
      fprintf( PSFile, "%d %d rmoveto (Enhance) show moveto\n", BMO + 3 * LTR, - LFS);
      fprintf( PSFile, "%d %d rmoveto (Enemy) show\n", BMO + 3 * LTR, -2 * LFS);
      fprintf( PSFile, "moveto %d %d rmoveto currentpoint\n", LBW, 0);
      fprintf( PSFile, "%d %d rmoveto (Decor) show moveto\n", BMO + 3 * LTR, - LFS);
      if (GameVersion < 16)
	 fprintf( PSFile, "%d %d rmoveto (Barrel) show\n", BMO + 3 * LTR, -2 * LFS);
      else /* GameVersion >= 16 */
	 fprintf( PSFile, "%d %d rmoveto (Pod/Vlc) show\n", BMO + 3 * LTR, -2 * LFS);
   }
}


/*
   initialise printing scale
*/
void InitScale( BCINT mapXsize, BCINT mapYsize, char *name)
{
   /* define page center */
   PageCenterX = PageMaxX / 2;
   PageCenterY = PageMaxY / 2;

   if (FlagAutoZoom)
   {
      /* compute auto-zoom scale to fit level on page */
      Scale = (float) (1.0 / max( (float) mapXsize / (PageMaxX - 2 * PBM - BMO),
				  (float) mapYsize / (PageMaxY - 2 * PBM - BMO)));
      /* define clipping areas for name/legend */
      if (FlagName || FlagLegend)
      {
	 clipXn = PBM + ((GameVersion == 2 ? 7 : 6) + strlen( name)) * NFS * 0.55 + BMO;
	 clipXl = PageMaxX - PBM - 5 * LBW - BMO;	      /* NFS * 0.55 = av. */
	 clipY  = PageMaxY - PBM - NFS - BMO;		      /* character width */
      }
   }
   else
      /* default scale such that the largest level (E3M6) fits on Portrait */
      /* for DOOM ][ MAP31, these scales are a bit too small, but who cares... ;) */
      Scale = (float) (1.0 / ( FlagA4Letter ? DEFSCLA : DEFSCLL ));
}


/*
   check printing scale
*/
Bool CheckScale( BCINT mapX, BCINT mapY)
{
   /* check if map would cover name or legend areas */
   return (FlagName   && PAGEX( mapX) < clipXn && PAGEY( mapY) > clipY) ||
	  (FlagLegend && PAGEX( mapX) > clipXl && PAGEY( mapY) > clipY);
}


/*
   adjust printing scale
*/
void AdjustScale( BCINT mapXsize, BCINT mapYsize)
{
   /* recompute page center & auto-zoom scale */
   PageCenterY = (PageMaxY - NFS - BMO) / 2;
   Scale = (float) (1.0 / max( (float) mapXsize / (PageMaxX - 2 * PBM - BMO),
			       (float) mapYsize / (PageMaxY - 2 * PBM - BMO - NFS - BMO)));
}


/*
   initialise Lines printing
*/
void InitLines()
{
   /* define line macro */
   fprintf( PSFile, "\n%% print map lines\n");
   fprintf( PSFile, "/Line { setgray setlinewidth moveto lineto stroke } bind def\n");
   fprintf( PSFile, "\n");
}


/*
   initialise Things printing
*/
void InitThings()
{
   /* define thing macros */
   fprintf( PSFile, "\n%% print map things\n");
   fprintf( PSFile, "0 setlinewidth 0 setgray\n");
   fprintf( PSFile, "/Start    { 2 copy 6 2 roll moveto dup 360 add arc stroke } bind def\n");
   fprintf( PSFile, "/Enhance  { 3 copy 0 360 arc stroke 2 idiv 0 360 arc stroke } bind def\n");
   fprintf( PSFile, "/Bonus    { moveto 2 idiv\n");
   fprintf( PSFile, "            dup dup rmoveto dup add dup neg 0 rlineto\n");
   fprintf( PSFile, "            dup neg 0 exch rlineto 0 rlineto closepath stroke } bind def\n");
   fprintf( PSFile, "/Weapon   { moveto dup 2 idiv gsave\n");
   fprintf( PSFile, "            dup dup rmoveto dup add dup neg 0 rlineto\n");
   fprintf( PSFile, "            dup neg 0 exch rlineto 0 rlineto closepath stroke\n");
   fprintf( PSFile, "            grestore\n");
   fprintf( PSFile, "            dup dup rmoveto dup add dup neg 0 rlineto\n");
   fprintf( PSFile, "            dup neg 0 exch rlineto 0 rlineto closepath stroke } bind def\n");
   fprintf( PSFile, "/Enemy    { moveto gsave rotate dup 0 rlineto\n");
   fprintf( PSFile, "            dup 1.5 mul neg exch 0.866 mul 2 copy rlineto\n");
   fprintf( PSFile, "            dup dup add neg 0 exch rlineto exch neg exch rlineto\n");
   fprintf( PSFile, "            stroke grestore newpath } bind def\n");
   fprintf( PSFile, "/Decor    { moveto 2 idiv dup 0 rmoveto dup dup add neg 0 rlineto\n");
   fprintf( PSFile, "            dup dup rmoveto dup add neg 0 exch rlineto stroke } bind def\n");
   fprintf( PSFile, "/Barrel   { 0 360 arc stroke } bind def\n");
   fprintf( PSFile, "/Teleport { moveto 2 copy gsave rotate 0 rlineto stroke\n");
   fprintf( PSFile, "            grestore gsave rotate\n");
   fprintf( PSFile, "            dup dup rmoveto dup add dup neg 0 rlineto\n");
   fprintf( PSFile, "            dup neg 0 exch rlineto 0 rlineto closepath stroke\n");
   fprintf( PSFile, "            grestore newpath } bind def\n");
   fprintf( PSFile, "\n");
}


/*
   initialise shaded secret Sectors printing
*/
void InitShades()
{
   /* define shading macros */
   fprintf( PSFile, "\n%% print shaded sectors\n");
   fprintf( PSFile, "0 setlinewidth\n");
   fprintf( PSFile, "/BeginShade { moveto } bind def\n");
   fprintf( PSFile, "/NextShade { lineto } bind def\n");
   fprintf( PSFile, "/EndSecret { closepath 0.88 setgray fill } bind def\n");
   fprintf( PSFile, "/EndDamage { closepath 0.98 setgray fill } bind def\n");
   fprintf( PSFile, "/EndWhite { closepath 1 setgray fill } bind def\n");
   fprintf( PSFile, "\n");
}


/*
   initialise Teleport links printing
*/
void InitLinks()
{
   /* define teleport link macro */
   fprintf( PSFile, "\n%% print teleport links\n");
   fprintf( PSFile, "0.2 setlinewidth 0.75 setgray\n");
   fprintf( PSFile, "/TeleLink { moveto lineto stroke } bind def\n");
   fprintf( PSFile, "\n");
}


/*
   terminate the PostScript page
*/
void TermPage()
{
   /* finish legend */
   if (FlagLegend)
   {
      BCINT legendX = PageMaxX - PBM + BMO + LTR,
	    legendY = PageMaxY - PBM + LTR - 1;

      fprintf( PSFile, "\n%% print legend things & scale\n");
      fprintf( PSFile, "0 setlinewidth 0 setgray\n");
      fprintf( PSFile, "\n");
      fprintf( PSFile, "%d %d %d %d Start\n", LTR, 90, legendX - 5 * LBW, legendY - LFS);
      fprintf( PSFile, "%d %d %d %d Teleport\n", LTR, 90, legendX - 5 * LBW, legendY - 2 * LFS);
      fprintf( PSFile, "%d %d %d Bonus\n", LTR, legendX - 4 * LBW, legendY - LFS);
      fprintf( PSFile, "%d %d %d Weapon\n", LTR, legendX - 4 * LBW, legendY - 2 * LFS);
      fprintf( PSFile, "%d %d %d Enhance\n", legendX - 3 * LBW, legendY - LFS, LTR);
      fprintf( PSFile, "%d %d %d %d Enemy\n", LTR, 90, legendX - 3 * LBW, legendY - 2 * LFS - 1);
      fprintf( PSFile, "%d %d %d Decor\n", LTR, legendX - 2 * LBW, legendY - LFS);
      fprintf( PSFile, "%d %d %d Barrel\n", legendX - 2 * LBW, legendY - 2 * LFS, LTR / 2);

      fprintf( PSFile, "%d %d moveto (1 : %.1f) show\n",
			PageMaxX - PBM - LBW + 2 * BMO, PageMaxY - PBM - 2 * LFS,
			(float) (1.0 / Scale));
   }

   fprintf( PSFile, "\nshowpage\n");
}


/*
   draw a Line on the page from map coords
*/
void PrintMapLine( BCINT mapXstart, BCINT mapYstart, BCINT mapXend, BCINT mapYend,
		   Bool solid, Bool secret)
{
   fprintf( PSFile, "%d %d %d %d %.1f %.1f Line\n",
		     PAGEX( mapXstart), PAGEY( mapYstart), PAGEX( mapXend), PAGEY( mapYend),
		     (solid ? 0.7 : 0 ), (secret ? 0.5 : 0));
}


/*
   draw a Thing on the page from map coords
*/
void PrintMapThing( BCINT class, BCINT mapXcenter, BCINT mapYcenter,
				 BCINT mapRadius,  BCINT mapAngle)
{
   switch( class)
   {
   case CLASS_START:
      fprintf( PSFile, "%d %d %d %d Start\n",
			(int) (mapRadius * Scale), mapAngle,
			PAGEX( mapXcenter), PAGEY( mapYcenter));
      break;
   case CLASS_ENHANCE:
      fprintf( PSFile, "%d %d %d Enhance\n",
			PAGEX( mapXcenter), PAGEY( mapYcenter),
			(int) (mapRadius * Scale));
      break;
   case CLASS_BONUS:
      fprintf( PSFile, "%d %d %d Bonus\n",
			(int) (mapRadius * Scale),
			PAGEX( mapXcenter), PAGEY( mapYcenter));
      break;
   case CLASS_WEAPON:
      fprintf( PSFile, "%d %d %d Weapon\n",
			(int) (mapRadius * Scale),
			PAGEX( mapXcenter), PAGEY( mapYcenter));
      break;
   case CLASS_ENEMY:
      fprintf( PSFile, "%d %d %d %d Enemy\n",
			(int) (mapRadius * Scale), mapAngle,
			PAGEX( mapXcenter), PAGEY( mapYcenter));
      break;
   case CLASS_DECOR:
      fprintf( PSFile, "%d %d %d Decor\n",
			(int) (mapRadius * Scale),
			PAGEX( mapXcenter), PAGEY( mapYcenter));
      break;
   case CLASS_BARREL:
      fprintf( PSFile, "%d %d %d Barrel\n",
			PAGEX( mapXcenter), PAGEY( mapYcenter),
			(int) (mapRadius * Scale));
      break;
   case CLASS_TELEPORT:
      fprintf( PSFile, "%d %d %d %d Teleport\n",
			(int) (mapRadius * Scale), mapAngle,
			PAGEX( mapXcenter), PAGEY( mapYcenter));
      break;
   }
}


/*
   draw a shaded secret Sector component on the page
*/
void PrintShade( char type, BCINT mapX, BCINT mapY)
{
   switch( type)
   {
   case 'B':
      fprintf( PSFile, "%d %d BeginShade\n", PAGEX( mapX), PAGEY( mapY));
      break;
   case 'N':
      fprintf( PSFile, "%d %d NextShade\n", PAGEX( mapX), PAGEY( mapY));
      break;
   case 'S':
      fprintf( PSFile, "EndSecret\n");
      break;
   case 'D':
      fprintf( PSFile, "EndDamage\n");
      break;
   case 'W':
      fprintf( PSFile, "EndWhite\n");
      break;
   }
}


/*
   draw a Teleport link on the page from map coords
*/
void PrintTeleLink( BCINT mapXstart, BCINT mapYstart, BCINT mapXend, BCINT mapYend)
{
   fprintf( PSFile, "%d %d %d %d TeleLink\n",
		     PAGEX( mapXstart), PAGEY( mapYstart), PAGEX( mapXend), PAGEY( mapYend));
}

/* end of file */
