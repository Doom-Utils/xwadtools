/*
   DooM PostScript Maps Utility, by Frans P. de Vries.

Derived from:

   Doom Editor Utility, by Brendon Wyber and Raphaël Quinet.

   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README for more information.

   This program comes with absolutely no warranty.

   PRINT.C - Print and Analyze routines.
*/


/* the includes */
#include "dmps.h"
#include "levels.h"
#include "things.h"


/* global variables */
				/* if TRUE ('+');        if FALSE ('-')        */
Bool FlagDoublePg = FALSE;	/* print double page;    print normal page     */
Bool FlagQuadplPg = FALSE;	/* print quadruple page; print normal page     */
Bool FlagA4Letter = TRUE;	/* print A4 paper;       print Letter paper    */
Bool FlagBorder   = TRUE;	/* print border;         print no border       */
Bool FlagDecors   = TRUE;	/* print decors;         print no decors       */
Bool FlagEnemies  = TRUE;	/* print enemies;        print no enemies      */
Bool FlagFadeLins = TRUE;	/* fade secret lines;    don't fade sec.lines  */
Bool FlagGoodies  = TRUE;	/* print goodies;        print no goodies      */
Bool FlagLegend   = TRUE;	/* print thing legend;   print no thing legend */
Bool FlagMultPlay = FALSE;	/* print multi things;   print no multi things */
Bool FlagName     = TRUE;	/* print level name;     print no level name   */
Bool FlagPortrait = TRUE;	/* print portrait;       print landscape       */
Bool FlagShdAreas = TRUE;	/* shade secret areas;   don't shade sec.areas */
Bool FlagTeleprts = TRUE;	/* link teleports;       don't link teleports  */
Bool FlagUltraVlc = TRUE;	/* print UV things;      print non-UV things   */
Bool FlagAutoZoom = TRUE;	/* auto-zoom map scale;  default map scale     */

BCINT MapCenterX;		/* X coord of map center */
BCINT MapCenterY;		/* Y coord of map center */


/* map attribute definitions */
#define L_IMPASS 0x01		/* impassible linedef */
#define L_SECRET 0x20		/* secret linedef */
#define L_TELEP1 39		/* one-time teleport linedef */
#define L_TELEPR 97		/* repeatable teleport linedef */
#define L_TELPM1 125		/* one-time monster-only teleport linedef */
#define L_TELPMR 126		/* repeatable monster-only teleport linedef */
#define S_SECRET 9		/* secret sector */
#define S_DAMAG1 4		/* damage sector -10/20% | lava hurts lots   */
#define S_DAMAG2 5		/* damage sector - 5/10% | lava hurts little */
#define S_DAMAG3 7		/* damage sector - 2/ 5% | sludge hurts      */
#define S_DAMAG4 16		/* damage sector -10/20% | lava hurts medium */
#define S_DAMEND 11		/* damage sector -10/20% & end level (DOOM-only) */
#define T_MULTI  0x10		/* multi-player thing */
#define T_ULTRA  0x04		/* ultra-violence thing */
#define T_NONUV  0x03		/* non ultra-violence thing */


/*
   read level & print its map
*/
void PrintLevel( BCINT episode, BCINT mission)
{
   ReadLevelData( episode, mission);
   /* define map center */
   MapCenterX = (MapMinX + MapMaxX) / 2;
   MapCenterY = (MapMinY + MapMaxY) / 2;

   /* initialize, scale & draw the page */
   InitPage( episode, mission, ( UserLvlNm != NULL ? UserLvlNm : LevelName ));

   InitScale( MapMaxX - MapMinX, MapMaxY - MapMinY,
	      ( UserLvlNm != NULL ? UserLvlNm : LevelName ));
   if (FlagAutoZoom && (FlagName || FlagLegend))
   {
      BCINT n;
      for (n = 0; n < NumVertexes; n++)
	 /* rough & fine check for covering name or legend areas */
	 if (Vertexes[ n].y > MapCenterY)
	    if (CheckScale( Vertexes[ n].x, Vertexes[ n].y))
	       break; /* for */
      if (n < NumVertexes)
	 AdjustScale( MapMaxX - MapMinX, MapMaxY - MapMinY);
   }

   PrintMap();
   TermPage();

   /* clean up & free space */
   ForgetLevelData();
}


/*
   draw the actual game map
*/
void PrintMap( void)
{
   BCINT n, m;

   /* draw the shaded secret & damage Sectors if desired */
   if (FlagShdAreas)
   {
      InitShades();

      /* find all secret & damage Sectors */
      for (n = 0; n < NumSectors; n++)
	 if (Sectors[ n].special == S_DAMAG1 || Sectors[ n].special == S_DAMAG2 ||
	     Sectors[ n].special == S_DAMAG3 || Sectors[ n].special == S_DAMAG4 ||
	     (Sectors[ n].special == S_DAMEND && GameVersion < 16))
	    ShadeSector( n, 'D');
	 else if (Sectors[ n].special == S_SECRET)
	    ShadeSector( n, 'S');
   }

   /* draw the LineDefs to form the map */
   InitLines();

   for (n = 0; n < NumLineDefs; n++)
      if (FlagFadeLins)
      {
	 /* check if Sector on either side of LineDef is secret */
	 m = Sectors[ SideDefs[ LineDefs[ n].sidedef1].sector].special;
	 if (m != S_SECRET && LineDefs[ n].sidedef2 != -1)
	    m = Sectors[ SideDefs[ LineDefs[ n].sidedef2].sector].special;

	 PrintMapLine( Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
		       Vertexes[ LineDefs[ n].end].x,   Vertexes[ LineDefs[ n].end].y,
		       (LineDefs[ n].flags & L_IMPASS),
		       (LineDefs[ n].flags & L_SECRET) || m == S_SECRET);
      }
      else
	 PrintMapLine( Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
		       Vertexes[ LineDefs[ n].end].x,   Vertexes[ LineDefs[ n].end].y,
		       (LineDefs[ n].flags & L_IMPASS), FALSE);

   /* draw all desired Things */
   InitThings();

   for (n = 0; n < NumThings; n++)
      /* check whether multi-player Thing should be printed */
      if (FlagMultPlay || ! (Things[ n].when & T_MULTI))
	 /* check whether UV or non-UV Thing should be printed */
	 if ((FlagUltraVlc && (Things[ n].when & T_ULTRA)) ||
	     (! FlagUltraVlc && (Things[ n].when & T_NONUV)) ||
	     Things[ n].when == 0) /* print always if no difficulty bits */
	 {
	    m = Things[ n].type;
	    switch( GetThingClass( m))
	    {
	    case CLASS_START:
	       /* always print Player 1 Start, others only if multi-player Things */
	       if (FlagMultPlay || m == THING_PLAYER1)
		  break;
	       else
		  continue;
	    case CLASS_ENHANCE:
	       if (FlagGoodies)
		  break;
	       else
		  continue;
	    case CLASS_BONUS:
	       if (FlagGoodies)
		  break;
	       else
		  continue;
	    case CLASS_WEAPON:
	       if (FlagGoodies)
		  break;
	       else
		  continue;
	    case CLASS_ENEMY:
	       if (FlagEnemies)
		  break;
	       else
		  continue;
	    case CLASS_DECOR:
	       if (FlagDecors)
		  break;
	       else
		  continue;
	    case CLASS_BARREL:
	       if (FlagDecors)
		  break;
	       else
		  continue;
	    case CLASS_TELEPORT:
	       /* always print Teleport Exits */
	       break;
	    default:
	       break;
	    }

	    PrintMapThing( GetThingClass( m), Things[ n].xpos, Things[ n].ypos,
			   GetThingRadius( m), Things[ n].angle);
	 }

   /* draw the Teleport links if desired */
   if (FlagTeleprts)
   {
      BCINT t;

      InitLinks();

      /* find all (incl. monster-only) teleporting LineDefs */
      for (n = 0; n < NumLineDefs; n++)
	 if (LineDefs[ n].type == L_TELEP1 || LineDefs[ n].type == L_TELEPR ||
	     LineDefs[ n].type == L_TELPM1 || LineDefs[ n].type == L_TELPMR)
	 {
	    /* find associated Sector */
	    for (m = 0; m < NumSectors; m++)
	       if (Sectors[ m].tag == LineDefs[ n].tag)
		  if ((t = FindTeleExit( m)) != NumThings)
		  {
		     /* draw link from LineDef midpoint to teleport exit center */
		     PrintTeleLink( (Vertexes[ LineDefs[ n].start].x + Vertexes[ LineDefs[ n].end].x) / 2,
				    (Vertexes[ LineDefs[ n].start].y + Vertexes[ LineDefs[ n].end].y) / 2,
				    Things[ t].xpos, Things[ t].ypos);
		     break; /* for */
		  }

	    if (m == NumSectors) /* safety check */
	       printf( "[No sector tagged %d found with teleport exit.]\n",
			LineDefs[ n].tag);
	 }
   }
}


#define	MAXLNS	250  /* MAP29 sector 0 has 162 lines; RRWARD02 sector 117 has 206 lines */

/*
   shade a secret (S) or damage (D) Sector w/o their inner Sectors
*/
void ShadeSector( BCINT sector, char type)
{
   BCINT lines[ MAXLNS], end = 0;
   BCINT l, outer, next;

   if (! BuildPolygons( lines, sector, &end))
   {
      printf( "[%s sector %d has too many lines for shading.]\n",
	       ( type == 'S' ? "Secret" : "Damage" ), sector);
      return;
   }

   /* shade the secret/damage Sector */
   outer = l = 0;
   while (l < end)
   {
      /* shade the outer polygon */
      ShadePolygon( lines, outer, type, &next);
      l = next + 1;
      while (l < end)
      {
	 next = (lines[l] > 0) ? LineDefs[ lines[l]-1].start
			       : LineDefs[-lines[l]-1].end;
	 /* check for disjoint polygon */
	 if (! PointInPolygon( lines, outer, Vertexes[ next].x,
					     Vertexes[ next].y, &next))
	 {
	    outer = l;
	    break; /* while */
	 }
	 /* un-shade inner polygon(s) */
	 ShadePolygon( lines, l, 'W', &next);
	 l = next + 1;
      }
   }
}


/*
   shade polygon lines[start] with 'type'
*/
void ShadePolygon( BCINT lines[], BCINT start, char type, BCINT *end)
{
   BCINT n = start;

   /* type-shade inside LineDefs of closed polygon */
   start = (lines[n] > 0) ? LineDefs[ lines[n]-1].start
			  : LineDefs[-lines[n]-1].end;
   PrintShade( 'B', Vertexes[ start].x, Vertexes[ start].y);
   while (TRUE)
   {
      if (lines[n] > 0)
	 if (start == LineDefs[ lines[n]-1].end)
	    break; /* while */
	 else
	    PrintShade( 'N', Vertexes[ LineDefs[ lines[n]-1].end].x,
			     Vertexes[ LineDefs[ lines[n]-1].end].y);
      else /* lines[n] < 0 */
	 if (start == LineDefs[-lines[n]-1].start)
	    break; /* while */
	 else
	    PrintShade( 'N', Vertexes[ LineDefs[-lines[n]-1].start].x,
			     Vertexes[ LineDefs[-lines[n]-1].start].y);
      n++;
   }
   PrintShade (type, 0, 0);
   *end = n;
}


/*
   find a Teleport Exit inside the given Sector
*/
BCINT FindTeleExit( BCINT sector)
{
   BCINT lines[ MAXLNS], end = 0;
   BCINT m, l, outer, next;
   Bool  outside;

   if (! BuildPolygons( lines, sector, &end))
   {
      printf( "[Sector %d has too many lines to find teleport exit in.]\n", sector);
      return NumThings;
   }

   /* find all teleport exits */
   for (m = 0; m < NumThings; m++)
      if (Things[ m].type == THING_TELEPORT)
      {
	 /* check all disjoint outer polygons */
	 outer = l = 0;
	 while (l < end)
	    /* check if Thing inside this outer polygon */
	    if (PointInPolygon( lines, outer, Things[ m].xpos,
					      Things[ m].ypos, &next))
	    {
	       outside = TRUE;
	       /* check if Thing outside all inner polygons */
	       l = next + 1;
	       while (l < end)
	       {
		  next = (lines[l] > 0) ? LineDefs[ lines[l]-1].start
					: LineDefs[-lines[l]-1].end;
		  /* check for disjoint polygon */
		  if (! PointInPolygon( lines, outer, Vertexes[ next].x,
						      Vertexes[ next].y, &next))
		  {
		     outer = l;
		     break; /* while */
		  }
		  if (PointInPolygon( lines, l, Things[ m].xpos,
						Things[ m].ypos, &next))
		     outside = FALSE;
		  l = next + 1;
	       }
	       if (outside)
		  return m;
	    }
	    else /* skip inner polygons */
	    {
	       l = next + 1;
	       while (l < end)
	       {
		  next = (lines[l] > 0) ? LineDefs[ lines[l]-1].start
					: LineDefs[-lines[l]-1].end;
		  /* check for disjoint polygon */
		  if (! PointInPolygon( lines, outer, Vertexes[ next].x,
						      Vertexes[ next].y, &next))
		  {
		     outer = l;
		     break; /* while */
		  }
		  outside = PointInPolygon( lines, l, Things[ m].xpos,
						      Things[ m].ypos, &next);
		  l = next + 1;
	       }
	    }
      }
   return NumThings;
}


/*
   check if Vertex (xp,yp) is inside polygon lines[start] (using crossings test)
*/
Bool PointInPolygon( BCINT lines[], BCINT start, BCINT xp, BCINT yp, BCINT *end)
{
   BCINT vi, vj, n = start;
   Bool  inside = FALSE;

   /* crosstest all LineDefs of closed polygon */
   start = (lines[n] > 0) ? LineDefs[ lines[n]-1].start
			  : LineDefs[-lines[n]-1].end;
   while (TRUE)
   {
      if (lines[n] > 0)
      {
	 vi = LineDefs[ lines[n]-1].start;
	 vj = LineDefs[ lines[n]-1].end;
      }
      else /* lines[n] < 0 */
      {
	 vi = LineDefs[-lines[n]-1].end;
	 vj = LineDefs[-lines[n]-1].start;
      }
      if (( ((Vertexes[vi].y <= yp) && (yp < Vertexes[vj].y)) ||
	    ((Vertexes[vj].y <= yp) && (yp < Vertexes[vi].y)) ) &&
	  ( xp < (Vertexes[vj].x - Vertexes[vi].x) * (yp - Vertexes[vi].y) /
		 (Vertexes[vj].y - Vertexes[vi].y) + Vertexes[vi].x))
	 inside = ! inside;
      if (start == vj)
	 break; /* while */
      n++;
   }

   *end = n;
   return inside;
}


/*
   build Sector's LineDefs into sorted polygon(s)
*/
Bool BuildPolygons( BCINT lines[], BCINT sector, BCINT *end)
{
   BCINT m, l;

   /* build list of all LineDefs that make up the Sector */
   for (m = 0; m < NumLineDefs; m++)
   {
      l = SideDefs[ LineDefs[ m].sidedef1].sector;
      if (l != sector && LineDefs[ m].sidedef2 != -1)
	 l = SideDefs[ LineDefs[ m].sidedef2].sector;
      if (l == sector)
      {
	 if (*end < MAXLNS) /* safety check */
	    lines[(*end)++] = m + 1; /* allow LineDef 0's orientation to be */
	 else                        /* reversed too (eg. E3M6 sector 4) */
	    return FALSE;
      }
   }

   /* sort list for consecutive LineDefs on outer and any inner polygons
      (eg. E2M1 sector 3 has 4 inner polygons); negating "reverse" LineDefs */
   l = 0;
   while (l < *end)
   {
      m = FindStartLine( lines, l, *end);
      /* put it at front */
      if (l != m)
	 exch( lines[l], lines[m]);

      m = SortPolygon( lines, l, end);
      l = m + 1;
   }
   return TRUE;
}


/*
   find most clockwise LineDef extending from the most lower-left Vertex
   (clockwise as defined by the crossproduct of the two end Vertices
    wrt. to the common lower-left Vertex)
*/
BCINT FindStartLine( BCINT lines[], BCINT start, BCINT end)
{
   BCINT m, l = 0, vm, x0 = 32767, y0 = 32767;

   for (m = start; m < end; m++)
   {
      /* reset LineDef orientation */
      if (lines[m] < 0)
	 lines[m] = - lines[m];

      vm = LineDefs[ lines[m]-1].start;
      if ((Vertexes[ vm].y < y0) ||
	  (Vertexes[ vm].y == y0 && Vertexes[ vm].x < x0))
      {
	 l = m;
	 x0 = Vertexes[ vm].x;
	 y0 = Vertexes[ vm].y;
      }
      else if (Vertexes[ vm].y == y0 && Vertexes[ vm].x == x0)
	   {
	      BCINT x1, y1, x2 = Vertexes[ LineDefs[ lines[m]-1].end].x,
			    y2 = Vertexes[ LineDefs[ lines[m]-1].end].y;
	      if (lines[l] > 0)
	      {
		 x1 = Vertexes[ LineDefs[ lines[l]-1].end].x;
		 y1 = Vertexes[ LineDefs[ lines[l]-1].end].y;
	      }
	      else /* lines[l] < 0 */
	      {
		 x1 = Vertexes[ LineDefs[-lines[l]-1].start].x;
		 y1 = Vertexes[ LineDefs[-lines[l]-1].start].y;
	      }
	      /* check if lines[m]-1 is clockwise from lines[l]-1 */
	      if (((x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0)) < 0)
		 l = m;
	   }

      vm = LineDefs[ lines[m]-1].end;
      if ((Vertexes[ vm].y < y0) ||
	  (Vertexes[ vm].y == y0 && Vertexes[ vm].x < x0))
      {
	 l = m;
	 x0 = Vertexes[ vm].x;
	 y0 = Vertexes[ vm].y;
	 lines[l] = - lines[l];
      }
      else if (Vertexes[ vm].y == y0 && Vertexes[ vm].x == x0)
	   {
	      BCINT x1, y1, x2 = Vertexes[ LineDefs[ lines[m]-1].start].x,
			    y2 = Vertexes[ LineDefs[ lines[m]-1].start].y;
	      if (lines[l] > 0)
	      {
		 x1 = Vertexes[ LineDefs[ lines[l]-1].end].x;
		 y1 = Vertexes[ LineDefs[ lines[l]-1].end].y;
	      }
	      else /* lines[l] < 0 */
	      {
		 x1 = Vertexes[ LineDefs[-lines[l]-1].start].x;
		 y1 = Vertexes[ LineDefs[-lines[l]-1].start].y;
	      }
	      /* check if lines[m]-1 is clockwise from lines[l]-1 */
	      if (((x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0)) < 0)
	      {
		 l = m;
		 lines[l] = - lines[l];
	      }
	   }
   }
   return l;
}


/*
   sort Sector's LineDef list from LineDef 'start' into next polygon
   (derived from algorithms to find the convex hull of a set of points)
*/
BCINT SortPolygon( BCINT lines[], BCINT start, BCINT *end)
{
   BCINT m, l, k, vi, vj;

   for (k = start; k < *end - 1; k++)
   {
      if (lines[k] > 0)
      {
	 vi = LineDefs[ lines[k]-1].start;
	 vj = LineDefs[ lines[k]-1].end;
      }
      else /* lines[k] < 0 */
      {
	 vi = LineDefs[-lines[k]-1].end;
	 vj = LineDefs[-lines[k]-1].start;
      }

      /* find "best" consecutive LineDef */
      l = k - 1;
      for (m = k + 1; m < *end; m++)
      {
	 /* reset LineDef orientation */
	 if (lines[m] < 0)
	    lines[m] = - lines[m];

	 if (vj == LineDefs[ lines[m]-1].start)
	 {
	    if (l < k)
	       l = m;
	    else /* more than LineDef extending from this Vertex */
	       if (CheckClockwise( vj, vi,
				   (lines[l] > 0) ? LineDefs[ lines[l]-1].end
						  : LineDefs[-lines[l]-1].start,
				   LineDefs[ lines[m]-1].end))
		  l = m;
	 }
	 else if (vj == LineDefs[ lines[m]-1].end)
	 {
	    lines[m] = - lines[m];
	    if (l < k)
	       l = m;
	    else /* more than LineDef extending from this Vertex */
	       if (CheckClockwise( vj, vi,
				   (lines[l] > 0) ? LineDefs[ lines[l]-1].end
						  : LineDefs[-lines[l]-1].start,
				   LineDefs[-lines[m]-1].start))
		  l = m;
	 }
      }

      /* check for consecutive LineDef */
      if (l > k)
      {
	 exch( lines[k + 1], lines[l]);
	 /* check for closed polygon */
	 if (((lines[start] > 0) ? LineDefs[ lines[start]-1].start
				 : LineDefs[-lines[start]-1].end) ==
	     ((lines[k + 1] > 0) ? LineDefs[ lines[k + 1]-1].end
				 : LineDefs[-lines[k + 1]-1].start))
	    /* return end LineDef of this polygon */
	    return k+1;
      }
      else /* no consecutive LineDef */
      {
	 /* discard stray LineDef & restart sort */
	 (*end)--;
	 exch( lines[k], lines[*end]);
	 return start-1;
      }
   }

   /* no closed polygon: discard first LineDef & restart sort */
   (*end)--;
   exch( lines[start], lines[*end]);
   return start-1;
}


/*
   check if v2 is clockwise from v1 and v0 wrt. vc
   (at least two of v0-v1, v1-v2, v2-v0 need to be clockwise)
*/
Bool CheckClockwise( BCINT vc, BCINT v0, BCINT v1, BCINT v2)
{
   BCINT cw = 0;

   if (((Vertexes[v0].x - Vertexes[vc].x) * (Vertexes[v1].y - Vertexes[vc].y) -
	(Vertexes[v1].x - Vertexes[vc].x) * (Vertexes[v0].y - Vertexes[vc].y)) < 0)
      cw--;
   else
      cw++;

   if (((Vertexes[v1].x - Vertexes[vc].x) * (Vertexes[v2].y - Vertexes[vc].y) -
	(Vertexes[v2].x - Vertexes[vc].x) * (Vertexes[v1].y - Vertexes[vc].y)) < 0)
      cw--;
   else
      cw++;

   if (((Vertexes[v2].x - Vertexes[vc].x) * (Vertexes[v0].y - Vertexes[vc].y) -
	(Vertexes[v0].x - Vertexes[vc].x) * (Vertexes[v2].y - Vertexes[vc].y)) < 0)
      cw--;
   else
      cw++;

   return (cw < 0);
}


/*
   display all print flags
*/
void DisplayFlags( void)
{
   printf( "Print flags:\n");
   printf( "-----------\n");
   printf( "%s\n", ( FlagDoublePg ? "+2\tuse double page"
				  : "-2\tuse normal page" ));
   printf( "%s\n", ( FlagQuadplPg ? "+4\tuse quadruple page"
				  : "-4\tuse normal page" ));
   printf( "%s\n", ( FlagA4Letter ? "+A\tuse A4 paper size"
				  : "-A\tuse Letter paper size" ));
   printf( "%s\n", ( FlagBorder   ? "+B\tprint border around map"
				  : "-B\tprint no border around map" ));
   printf( "%s\n", ( FlagDecors   ? "+D\tprint all decorations & barrels"
				  : "-D\tprint no decorations & barrels" ));
   printf( "%s\n", ( FlagEnemies  ? "+E\tprint all enemies"
				  : "-E\tprint no enemies" ));
   printf( "%s\n", ( FlagFadeLins ? "+F\tfade secret lines"
				  : "-F\tdon't fade secret lines" ));
   printf( "%s\n", ( FlagGoodies  ? "+G\tprint all goodies (weapons/bonuses/enhancements)"
				  : "-G\tprint no goodies (weapons/bonuses/enhancements)" ));
   printf( "%s\n", ( FlagLegend   ? "+L\tprint legend above map"
				  : "-L\tprint no legend above map" ));
   printf( "%s\n", ( FlagMultPlay ? "+M\tprint multi-player things"
				  : "-M\tprint no multi-player things" ));
   printf( "%s\n", ( FlagName     ? "+N\tprint name above map"
				  : "-N\tprint no name above map" ));
   printf( "%s\n", ( FlagPortrait ? "+P\tuse Portrait orientation"
				  : "-P\tuse Landscape orientation" ));
   printf( "%s\n", ( FlagShdAreas ? "+S\tshade secret & damage areas"
				  : "-S\tdon't shade secret & damage areas" ));
   printf( "%s\n", ( FlagTeleprts ? "+T\tlink teleports"
				  : "-T\tdon't link teleports" ));
   printf( "%s\n", ( FlagUltraVlc ? "+U\tprint things for Ultra-Violence skill"
				  : "-U\tprint things below Ultra-Violence skill" ));
   printf( "%s\n", ( FlagAutoZoom ? "+Z\tauto-zoom map scale"
				  : "-Z\tdefault map scale" ));
}


/*
   set a print flag
*/
void SetFlag( char flag, char val)
{
   switch( flag)
   {
   case '2':
      FlagDoublePg = ( val == '+' ? TRUE : FALSE );
      printf( "%s\n", ( FlagDoublePg ? "+2\tuse double page"
				     : "-2\tuse normal page" ));
      if (FlagDoublePg && FlagQuadplPg)
	 SetFlag( '4', '-');
      break;
   case '4':
      FlagQuadplPg = ( val == '+' ? TRUE : FALSE );
      printf( "%s\n", ( FlagQuadplPg ? "+4\tuse quadruple page"
				     : "-4\tuse normal page" ));
      if (FlagQuadplPg && FlagDoublePg)
	 SetFlag( '2', '-');
      break;
   case 'A':
      FlagA4Letter = ( val == '+' ? TRUE : FALSE );
      printf( "%s\n", ( FlagA4Letter ? "+A\tuse A4 paper size"
				     : "-A\tuse Letter paper size" ));
      break;
   case 'B':
      FlagBorder   = ( val == '+' ? TRUE : FALSE );
      printf( "%s\n", ( FlagBorder   ? "+B\tprint border around map"
				     : "-B\tprint no border around map" ));
      break;
   case 'D':
      FlagDecors   = ( val == '+' ? TRUE : FALSE );
      printf( "%s\n", ( FlagDecors   ? "+D\tprint all decorations & barrels"
				     : "-D\tprint no decorations & barrels" ));
      break;
   case 'E':
      FlagEnemies  = ( val == '+' ? TRUE : FALSE );
      printf( "%s\n", ( FlagEnemies  ? "+E\tprint all enemies"
				     : "-E\tprint no enemies" ));
      break;
   case 'F':
      FlagFadeLins = ( val == '+' ? TRUE : FALSE );
      printf( "%s\n", ( FlagFadeLins ? "+F\tfade secret lines"
				     : "-F\tdon't fade secret lines" ));
      break;
   case 'G':
      FlagGoodies  = ( val == '+' ? TRUE : FALSE );
      printf( "%s\n", ( FlagGoodies  ? "+G\tprint all goodies (weapons/bonuses/enhancements)"
				     : "-G\tprint no goodies (weapons/bonuses/enhancements)" ));
      break;
   case 'L':
      FlagLegend   = ( val == '+' ? TRUE : FALSE );
      printf( "%s\n", ( FlagLegend   ? "+L\tprint legend above map"
				     : "-L\tprint no legend above map" ));
      break;
   case 'M':
      FlagMultPlay = ( val == '+' ? TRUE : FALSE );
      printf( "%s\n", ( FlagMultPlay ? "+M\tprint multi-player things"
				     : "-M\tprint no multi-player things" ));
      break;
   case 'N':
      FlagName     = ( val == '+' ? TRUE : FALSE );
      printf( "%s\n", ( FlagName     ? "+N\tprint name above map"
				     : "-N\tprint no name above map" ));
      break;
   case 'P':
      FlagPortrait = ( val == '+' ? TRUE : FALSE );
      printf( "%s\n", ( FlagPortrait ? "+P\tuse Portrait orientation"
				     : "-P\tuse Landscape orientation" ));
      break;
   case 'S':
      FlagShdAreas = ( val == '+' ? TRUE : FALSE );
      printf( "%s\n", ( FlagShdAreas ? "+S\tshade secret & damage areas"
				     : "-S\tdon't shade secret & damage areas" ));
      break;
   case 'T':
      FlagTeleprts = ( val == '+' ? TRUE : FALSE );
      printf( "%s\n", ( FlagTeleprts ? "+T\tlink teleports"
				     : "-T\tdon't link teleports" ));
      break;
   case 'U':
      FlagUltraVlc = ( val == '+' ? TRUE : FALSE );
      printf( "%s\n", ( FlagUltraVlc ? "+U\tprint things for Ultra-Violence skill"
				     : "-U\tprint things below Ultra-Violence skill" ));
      break;
   case 'Z':
      FlagAutoZoom = ( val == '+' ? TRUE : FALSE );
      printf( "%s\n", ( FlagAutoZoom ? "+Z\tauto-zoom map scale"
				     : "-Z\tdefault map scale" ));
      break;
   }
}


/* DOOM power-ups local array index names */
#define Shotgun  0
#define Cmbatgun 1
#define Chaingun 2
#define RcktLnch 3
#define Plasmagn 4
#define Bfg9000  5
#define Chainsaw 6
#define Berserk  7
#define Backpack 8
#define SecurArm 9
#define CmbatArm 10
#define SoulSphr 11
#define MegaSphr 12
#define BlurArtf 13
#define InvlnArt 14
#define RadSuit  15
#define CompMap  16
#define LiteAmp  17

/* Heretic power-ups local array index names */
#define Crossbow 0
#define DrgnClaw 1
#define HellStaf 2
#define PhnixRod 3
#define Mace     4
#define Gauntlts 5
#define BagHold  6
#define SilvShld 7
#define EnchShld 8
#define MystcUrn 9
#define ShadwSph 10
#define RingInvl 11
#define MapScrol 12
#define TomePowr 13
#define TimeBomb 14
#define ChaosDev 15
#define Torch    16
#define Wings    17
#define MorphOvm 18

/* DOOM enemies local array index names */
#define Human    0
#define Sergeant 1
#define Commando 2
#define Imp      3
#define Demon    4
#define Spectre  5
#define LostSoul 6
#define CacoDmn  7
#define PainElem 8
#define Mancubus 9
#define Revenant 10
#define Knight   11
#define Baron    12
#define Arachno  13
#define ArchVile 14
#define CyberDmn 15
#define SpiderBs 16
#define Wolf3dSS 17

/* Heretic enemies local array index names */
#define Gargoyl  0
#define FGargoyl 1
#define Golem    2
#define GolemGh  3
#define NGolem   4
#define NGolemGh 5
#define Warrior  6
#define WarriorG 7
#define Disciple 8
#define SabrClaw 9
#define WrDragon 10
#define Ophidian 11
#define IronLich 12
#define MauloTr  13
#define DSparil  14

/*
   read level & analyze its statistics
*/
void AnalyzeLevel( BCINT episode, BCINT mission)
{
   BCINT classes[ 9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   BCINT powrups[ 19] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   BCINT enemies[ 18] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   BCINT n, m, secret = 0, telept = 0;

   ReadLevelData( episode, mission);

   /* count Thing classes, Power-Ups types & Enemies types */
   for (n = 0; n < NumThings; n++)
      /* check whether multi-player Thing should be counted */
      if (FlagMultPlay || ! (Things[ n].when & T_MULTI))
	 /* check whether UV or non-UV Thing should be counted */
	 if ((FlagUltraVlc && (Things[ n].when & T_ULTRA)) ||
	     (! FlagUltraVlc && (Things[ n].when & T_NONUV)) ||
	     Things[ n].when == 0) /* count always if no difficulty bits */
	 {
	    m = Things[ n].type;
	    classes[ GetThingClass( m)]++;

	    if (GetThingClass( m) == CLASS_WEAPON || GetThingClass( m) == CLASS_ENHANCE)
	    {
	       if (GameVersion < 16)
		  switch( m)
		  {
		     case THING_SHOTGUN:   powrups[ Shotgun ]++; break;
		     case THING_COMBATGUN: powrups[ Cmbatgun]++; break;
		     case THING_CHAINGUN:  powrups[ Chaingun]++; break;
		     case THING_LAUNCHER:  powrups[ RcktLnch]++; break;
		     case THING_PLASMAGUN: powrups[ Plasmagn]++; break;
		     case THING_BFG9000:   powrups[ Bfg9000 ]++; break;
		     case THING_CHAINSAW:  powrups[ Chainsaw]++; break;
		     case THING_BERSERK:   powrups[ Berserk ]++; break;
		     case THING_BACKPACK:  powrups[ Backpack]++; break;
		     case THING_GREENARMOR:powrups[ SecurArm]++; break;
		     case THING_BLUEARMOR: powrups[ CmbatArm]++; break;
		     case THING_SOULSPHERE:powrups[ SoulSphr]++; break;
		     case THING_MEGASPHERE:powrups[ MegaSphr]++; break;
		     case THING_BLURSPHERE:powrups[ BlurArtf]++; break;
		     case THING_INVULN:    powrups[ InvlnArt]++; break;
		     case THING_RADSUIT:   powrups[ RadSuit ]++; break;
		     case THING_COMPMAP:   powrups[ CompMap ]++; break;
		     case THING_LITEAMP:   powrups[ LiteAmp ]++; break;
		     default: break;       /* ignore ammo boxes & keys */
		  }
	       else /* Heretic Things */
		  switch( m)
		  {
		     case THING_CROSSBOW:  powrups[ Crossbow]++; break;
		     case THING_DRAGONCLAW:powrups[ DrgnClaw]++; break;
		     case THING_HELLSTAFF: powrups[ HellStaf]++; break;
		     case THING_PHOENIXROD:powrups[ PhnixRod]++; break;
		     case THING_MACE:      powrups[ Mace    ]++; break;
		     case THING_GAUNTLETS: powrups[ Gauntlts]++; break;
		     case THING_BAGHOLDING:powrups[ BagHold ]++; break;
		     case THING_SILVSHIELD:powrups[ SilvShld]++; break;
		     case THING_ENCHSHIELD:powrups[ EnchShld]++; break;
		     case THING_MYSTICURN: powrups[ MystcUrn]++; break;
		     case THING_SHADOWSPHR:powrups[ ShadwSph]++; break;
		     case THING_RINGINVULN:powrups[ RingInvl]++; break;
		     case THING_MAPSCROLL: powrups[ MapScrol]++; break;
		     case THING_TOMEPOWER: powrups[ TomePowr]++; break;
		     case THING_TIMEBOMB:  powrups[ TimeBomb]++; break;
		     case THING_CHAOSDEV:  powrups[ ChaosDev]++; break;
		     case THING_TORCH:     powrups[ Torch   ]++; break;
		     case THING_WINGS:     powrups[ Wings   ]++; break;
		     case THING_MORPHOVUM: powrups[ MorphOvm]++; break;
		     default: break;      /* ignore ammo boxes & keys */
		  }
	    }

	    if (GetThingClass( m) == CLASS_ENEMY)
	    {
	       if (GameVersion < 16)
		  switch( m)
		  {
		     case THING_HUMAN:     enemies[ Human   ]++; break;
		     case THING_SERGEANT:  enemies[ Sergeant]++; break;
		     case THING_COMMANDO:  enemies[ Commando]++; break;
		     case THING_IMP:       enemies[ Imp     ]++; break;
		     case THING_DEMON:     enemies[ Demon   ]++; break;
		     case THING_SPECTRE:   enemies[ Spectre ]++; break;
		     case THING_LOSTSOUL:  enemies[ LostSoul]++; break;
		     case THING_CACODEMON: enemies[ CacoDmn ]++; break;
		     case THING_PAINELEM:  enemies[ PainElem]++; break;
		     case THING_MANCUBUS:  enemies[ Mancubus]++; break;
		     case THING_REVENANT:  enemies[ Revenant]++; break;
		     case THING_KNIGHT:    enemies[ Knight  ]++; break;
		     case THING_BARON:     enemies[ Baron   ]++; break;
		     case THING_ARACHNO:   enemies[ Arachno ]++; break;
		     case THING_ARCHVILE:  enemies[ ArchVile]++; break;
		     case THING_CYBERDEMON:enemies[ CyberDmn]++; break;
		     case THING_SPIDERBOSS:enemies[ SpiderBs]++; break;
		     case THING_WOLF3DSS:  enemies[ Wolf3dSS]++; break;
		  }
	       else /* Heretic Things */
		  switch( m)
		  {
		     case THING_GARGOYLE:  enemies[ Gargoyl ]++; break;
		     case THING_FRGARGOYLE:enemies[ FGargoyl]++; break;
		     case THING_GOLEM:     enemies[ Golem   ]++; break;
		     case THING_GOLEMGH:   enemies[ GolemGh ]++; break;
		     case THING_NTGOLEM:   enemies[ NGolem  ]++; break;
		     case THING_NTGOLEMGH: enemies[ NGolemGh]++; break;
		     case THING_WARRIOR:   enemies[ Warrior ]++; break;
		     case THING_WARRIORGH: enemies[ WarriorG]++; break;
		     case THING_DISCIPLE:  enemies[ Disciple]++; break;
		     case THING_SABRECLAW: enemies[ SabrClaw]++; break;
		     case THING_WEREDRAGON:enemies[ WrDragon]++; break;
		     case THING_OPHIDIAN:  enemies[ Ophidian]++; break;
		     case THING_IRONLICH:  enemies[ IronLich]++; break;
		     case THING_MAULOTAUR: enemies[ MauloTr ]++; break;
		     case THING_DSPARIL:   enemies[ DSparil ]++; break;
		  }
	    }
	 }

   /* count secret Sectors & teleporting LineDefs */
   for (n = 0; n < NumSectors; n++)
      if (Sectors[ n].special == S_SECRET)
	 secret++;
   for (n = 0; n < NumLineDefs; n++)
      if (LineDefs[ n].type == L_TELEP1 || LineDefs[ n].type == L_TELEPR)
	 telept++;

   /* print all statistics */
   if (GameVersion == 2) {
      printf( "Statistics of level MAP%02d: %s\n", mission,
	       ( UserLvlNm != NULL ? UserLvlNm : LevelName ));
      printf( "=========================\n\n");
   }
   else {
      printf( "Statistics of level E%dM%d: %s\n", episode, mission,
	       ( UserLvlNm != NULL ? UserLvlNm : LevelName ));
      printf( "========================\n\n");
   }

   printf( "MapDim:\t  MaxX\t  MinX\t SizeX\t  MaxY\t  MinY\t SizeY\n");
   printf( "\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\n",
	    MapMaxX, MapMinX, MapMaxX - MapMinX, MapMaxY, MapMinY, MapMaxY - MapMinY);

   printf( "\n");
   printf( "Struct:\tThings\tVertxs\tLinDfs\tTelePt\tSidDfs\tSectrs\tSecret\n");
   printf( "\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\n",
	    NumThings, NumVertexes, NumLineDefs, telept, NumSideDefs, NumSectors, secret);

   printf( "\n");
   if (GameVersion < 16)
      printf( "Things:\t START\tTLPORT\t BONUS\tWEAPON\tENHANC\t ENEMY\t DECOR\tBARREL\n");
   else
      printf( "Things:\t START\tTLPORT\t BONUS\tWEAPON\tENHANC\t ENEMY\t DECOR\tPOD/VC\n");
   printf( "\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\n",
	    classes[ CLASS_START],   classes[ CLASS_TELEPORT], classes[ CLASS_BONUS], classes[ CLASS_WEAPON],
	    classes[ CLASS_ENHANCE], classes[ CLASS_ENEMY],    classes[ CLASS_DECOR], classes[ CLASS_BARREL]);
   if (classes[ CLASS_UNKNOWN] != 0)
      printf( "Things:\tUNKNOWN\t%6d\n", classes[ CLASS_UNKNOWN]);

   if (GameVersion < 16)
   {
      printf( "\n");
      printf( "PowrUps:SHOTGN\tCOMBAT\tCHAING\tRCKTLR\tPLASMA\t BFG9K\tCHNSAW\tBERSRK\tBCKPCK\n");
      printf( "\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\n",
	       powrups[ Shotgun], powrups[ Cmbatgun], powrups[ Chaingun], powrups[ RcktLnch], powrups[ Plasmagn],
	       powrups[ Bfg9000], powrups[ Chainsaw], powrups[ Berserk], powrups[ Backpack]);
      printf( "\tGRNARM\tBLUARM\tSOULSP\tMEGASP\tBLURAF\tINVLAF\tRDSUIT\tCMPMAP\tLITAMP\n");
      printf( "\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\n",
	       powrups[ SecurArm], powrups[ CmbatArm], powrups[ SoulSphr], powrups[ MegaSphr], powrups[ BlurArtf],
	       powrups[ InvlnArt], powrups[ RadSuit], powrups[ CompMap], powrups[ LiteAmp]);

      printf( "\n");
      printf( "Enemies: HUMAN\t SARGE\tCOMNDO\t   IMP\t DEMON\tSPECTR\tLOSTSL\tCACODM\tPNELEM\n");
      printf( "\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\n",
	       enemies[ Human], enemies[ Sergeant], enemies[ Commando], enemies[ Imp], enemies[ Demon],
	       enemies[ Spectre], enemies[ LostSoul], enemies[ CacoDmn], enemies[ PainElem]);
      printf( "\tMANCBS\tREVENT\tKNIGHT\t BARON\tARCHNO\tARVILE\tCYBERD\tSPIDER\tWOLFSS\n");
      printf( "\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\n",
	       enemies[ Mancubus], enemies[ Revenant], enemies[ Knight], enemies[ Baron], enemies[ Arachno],
	       enemies[ ArchVile], enemies[ CyberDmn], enemies[ SpiderBs], enemies[ Wolf3dSS]);
   }
   else
   {
      printf( "\n");
      printf( "PowrUps:CROSSB\tDRCLAW\tHLSTAF\tPHXROD\t  MACE\tGAUNTL\tBAGHLD\tSILVSH\tENCHSH\n");
      printf( "\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\n",
	       powrups[ Crossbow], powrups[ DrgnClaw], powrups[ HellStaf], powrups[ PhnixRod], powrups[ Mace],
	       powrups[ Gauntlts], powrups[ BagHold], powrups[ SilvShld], powrups[ EnchShld]);
      printf( "MYSTCU\tSHADOW\tRNGINV\tMAPSCR\tTOMEPW\tTMBOMB\tCHAOSD\t TORCH\t WINGS\tMROVUM\n");
      printf( "%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\n",
	       powrups[ MystcUrn], powrups[ ShadwSph], powrups[ RingInvl], powrups[ MapScrol], powrups[ TomePowr],
	       powrups[ TimeBomb], powrups[ ChaosDev], powrups[ Torch], powrups[ Wings], powrups[ MorphOvm]);

      printf( "\n");
      printf( "Enemies:GARGOY\tFRGARG\t GOLEM\tGOLEMG\tNTGOLM\tNGOLMG\t WARRR\tWARRRG\n");
      printf( "\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\n",
	       enemies[ Gargoyl], enemies[ FGargoyl], enemies[ Golem], enemies[ GolemGh],
	       enemies[ NGolem], enemies[ NGolemGh], enemies[ Warrior], enemies[ WarriorG]);
      printf( "\tDISCIP\tSBCLAW\tWERDGN\tOPHIDN\tIRONLH\tMAULTR\tDSPARL\n");
      printf( "\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\n",
	       enemies[ Disciple], enemies[ SabrClaw], enemies[ WrDragon], enemies[ Ophidian],
	       enemies[ IronLich], enemies[ MauloTr], enemies[ DSparil]);
   }

   /* clean up & free space */
   ForgetLevelData();
}

/* end of file */
