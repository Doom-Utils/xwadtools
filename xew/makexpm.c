/* Routines for creating an xpm image from a "raw" image.  The raw
   image must be maximum 320x200, and must be 8 bits (one unsigned
   char) per colour, ie. max 256 colours.  The colourmap (palette) for
   the xpm's must be set with CreateXpmPalette() before
   CreateXpmImage() is called to make the image. The function
   CreateXpmFile() can be used to write an xpm file.
   
   Author: Hans Peter Verne ( hpv@kjemi.uio.no )  

   Copyright (C) 1996 Hans Peter Verne
 
   This is free software, distributed under the terms of the
   GNU General Public License. For details see the file COPYING */

#include <string.h>
#include <stdio.h>
#include "ew.h"  /* these functions are self-contained, but ew.h
                    declares them.  The compiler should warn 
		    if there are mismatches. */

/* The xpm palette.  We use 2 chars per pixel, 
   that's enough for 256 colours.  */
#define xpm_pal_len  16  /* 16 should be enough for anybody */
static char XpmPal[256][xpm_pal_len];

/* This must be unique duplets */
static const char XpmColourStrings[2*256+1] = "  " /* 0 = <space><space> */
  " 1 2 3 4 5 6 7 8 9 a b c d e f101112131415161718191a1b1c1d1e1f20"
  "2122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f40"
  "4142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f60"
  "6162636465666768696a6b6c6d6e6f707172737475767778797a7b7c7d7e7f80"
  "8182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9fa0"
  "a1a2a3a4a5a6a7a8a9aaabacadaeafb0b1b2b3b4b5b6b7b8b9babbbcbdbebfc0"
  "c1c2c3c4c5c6c7c8c9cacbcccdcecfd0d1d2d3d4d5d6d7d8d9dadbdcdddedfe0"
  "e1e2e3e4e5e6e7e8e9eaebecedeeeff0f1f2f3f4f5f6f7f8f9fafbfcfdfeff" ;

static int XpmPaletteSet = 0 ;

/* Create the palette, from the Red-Green-Blue values in RGB, which
   must be a char[3*256].  We use the entries in XpmColourStrings as
   the xpm colour strings.  The pixel value 0 is set in the xpm to
   be transparent, if so requested */
void CreateXpmPalette( const char *RGB, int zero_is_transparent )
{
  unsigned r,g,b;
  int i;
  char *cp;
  
  for( i=0; i<256; i++) {
    r = (unsigned char) RGB[ i * 3 + 0 ] ;
    g = (unsigned char) RGB[ i * 3 + 1 ] ;
    b = (unsigned char) RGB[ i * 3 + 2 ] ;
#ifdef  MAKE_FIXED_PALETTE
    /* For dumping the RGB to a C array (char[]), as used in fixpal.c */
    if( i % 3 == 0 ) 
      printf("\n  ");
    printf("'\\x%02x', '\\x%02x', '\\x%02x', ",r,g,b);
#endif
    cp = XpmPal[i];
    *cp++ = XpmColourStrings[2*i];  /* write in the colour strings */
    *cp   = XpmColourStrings[2*i+1];
    if ( i == 0 && !zero_is_transparent ) {
      strcpy( &XpmPal[i][2]," c None");
      continue;
    }
    sprintf( &XpmPal[i][2]," c #%02x%02x%02x",r,g,b);
  }

  XpmPaletteSet = 1;
}



/* Make an XPM image, ie. a char *[] of xpm-data, from the picture
   RawImage, width W, heigth H.  The image must be max 320x200, 256
   colours.  If redraw != NULL, the current image is just returned.
   (as the name implies, this is useful for redrawing the image,
   perhaps after changing the palette).  If NumCol != NULL, the
   number of colours actually used is passed back here.

   The palette for the image must be set in XpmPal[].

   Returns NULL on error, otherwise the xpm image */

char **CreateXpmImage( unsigned char *RawImage, int W, int H, 
		       int redraw, int *NumCol )
{
  /* the header of the xpm.  */
  static char xpmheader[32];

  /* Here we store the xpm image: max 320x200, 2 chars per
     pixel, pluss 1 byte for null-termination, that's 320*2+1 = 641
     chars per row, rounded up to 648. */
  static char xpmdata[200][648];

  /* This is the array actually returned. It contains pointers to
     the xpm header (1), the palette (max 256), and the picture rows
     (max 200);
     */
  static char *pict[1+256+200];

  int numcolour;         /* how many colours actually used */
  int colour[256];       /* which colours are actually used */
  int i, row, column, pixv;
  char *cp;

  if ( !XpmPaletteSet )
    return NULL;

  if ( redraw ) { /* we want whatever image was here */
    if ( pict[0] )  /*  pict[0] is NULL unless an image has been loaded */
      return pict;
    else
      return NULL;
  }

  memset(colour,0,256*sizeof(int));

  if ( W > 320 || H > 200 )
    return NULL;       /* Illegal image size */

  /* This loop used to be the bottleneck for creating XPM's.  I called
     sprintf(cp,"%02x",pixv) to make the colour string for each
     pixels. It took about ~10s on my 486dx33 for a 320x200!  I sped
     it up by creating the 2-chars colour strings in advance (that is
     what XpmColourStrings[] is used for), and paste them in by table
     lookup.  Now it's much better, 1-2s on a 320x200.  Any other
     suggestions? */

  for (row = 0; row < H; row++ ) {
    cp = xpmdata[row];
    for (column = 0; column < W; column++ ) {
      pixv = RawImage[row*W + column];
      colour[pixv] = 1;
      pixv *= 2;
      *cp++ = XpmColourStrings[pixv++];
      *cp++ = XpmColourStrings[pixv];
    }
    /* terminate string */
    *cp = '\0';
  }

  /* count colours : */
  numcolour=0;
  for( i=0; i<256; i++ ) 
    if ( colour[i] )      
      pict[++numcolour] = XpmPal[i]; 
  /* fill in header */
  sprintf(xpmheader,"%3d %3d %3d 2",W,H,numcolour);
  pict[0] = xpmheader;

  if ( NumCol )
    *NumCol = numcolour;

  for(row=0; row < H; row++ ) 
    pict[1 + numcolour + row] =  xpmdata[row];

  return pict;
}


/* Dump the XPM image to an xpm file */
int CreateXpmFile( char **XpmImage, const char *FileName )
{
  FILE *f;
  char xpmname[16];
  char *cp;
  int i, w, h, nc, cpp;

  f = fopen(FileName,"w");
  if ( !f ) {
    fprintf(stderr,"Can't open file %s\n",FileName);
    return 0;
  }

  /* Read header, find heigth and number of colours */
  i = sscanf(XpmImage[0],"%d %d %d %d",&w, &h, &nc, &cpp );
  if ( i != 4 )
    return 0;
  
  /* copy filename for the xpm name, extension removed. */
  strncpy(xpmname, FileName, 15);
  if ( (cp = strrchr(xpmname,'.')) )      
    *cp = '\0';

  fprintf(f,"/* XPM */\n");
  fprintf(f,"static char *%s[] = {\n",xpmname);
  fprintf(f,"/* width height num_colors chars_per_pixel */\n");
  for( i=0; i < nc + h; i++)
    fprintf(f,"\"%s\",\n",XpmImage[i]);
  fprintf(f,"\"%s\"\n};\n",XpmImage[i]); /* last line, no ',' */
  fprintf(f," /* Converted by CreateXpmFile(), hpv-1996. */\n"); 
  fclose(f);

  return 1;
}
