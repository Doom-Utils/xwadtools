/*
   DooM PostScript Maps Utility, by Frans P. de Vries.

Derived from:

   Doom Editor Utility, by Brendon Wyber and Raphaël Quinet.

   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README for more information.

   This program comes with absolutely no warranty.

   PRINT.H - Print flag definitions.
*/


/* the external variables from print.c */
				/* if TRUE ('+');        if FALSE ('-')        */
extern Bool FlagDoublePg;	/* print double page;    print normal page     */
extern Bool FlagQuadplPg;	/* print quadruple page; print normal page     */
extern Bool FlagA4Letter;	/* print A4 paper;       print Letter paper    */
extern Bool FlagBorder;		/* print border;         print no border       */
extern Bool FlagDecors;		/* print decors;         print no decors       */
extern Bool FlagEnemies;	/* print enemies;        print no enemies      */
extern Bool FlagFadeLins;	/* fade secret lines;    don't fade sec.lines  */
extern Bool FlagGoodies;	/* print goodies;        print no goodies      */
extern Bool FlagLegend;		/* print thing legend;   print no thing legend */
extern Bool FlagMultPlay;	/* print multi things;   print no multi things */
extern Bool FlagName;		/* print level name;     print no level name   */
extern Bool FlagPortrait;	/* print portrait;       print landscape       */
extern Bool FlagShdAreas;	/* shade secret areas;   don't shade sec.areas */
extern Bool FlagTeleprts;	/* link teleports;       don't link teleports  */
extern Bool FlagUltraVlc;	/* print UV things;      print non-UV things   */
extern Bool FlagAutoZoom;	/* auto-zoom map scale;  default map scale     */

extern BCINT MapCenterX;	/* X coord of map center */
extern BCINT MapCenterY;	/* Y coord of map center */

/* end of file */
