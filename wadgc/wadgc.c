/* WADGC.C */

/* A simple program to make sprites, wall patches,
   textures and floortiles and put them in a PWAD file
   for inclusion in custom levels.
   Author: Stefan Gustavson (stefang@isy.liu.se) 1994
   Modified by Udo Munk (munkudo@aol.com) 1998 - 2000
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !defined(__CYGWIN32__)
#include <search.h>
#endif
#include "sysdep.h"

extern unsigned char doom_rgb[];
extern unsigned char heretic_rgb[];
extern unsigned char hexen_rgb[];
extern unsigned char strife_rgb[];
unsigned char *palette = doom_rgb;
unsigned char sortedpal[256][4];

#define T1FILENAME "texture1.raw"
#define T2FILENAME "texture2.raw"
#define PNFILENAME "pnames.raw"

#define square(x) ((x)*(x))
#define TRUE (1)
#define FALSE (0)

int comparecols(const void *col1, const void *col2)
{
  return(memcmp(col1, col2, 3));
}

int lookup(unsigned char pixel[4], unsigned char palette[][4], int *warning)
{
  char *p;
  int i, index = 0, err, minerr;

#if 0
  /* Cyan in the input means transparent like in DMGRAPH */
  if ((pixel[0] == 0) && (pixel[1] == 255) && (pixel[2] == 255))
    return(-1);
#endif

  /* Quick look for a perfect match - this requires a sorted palette */
  p = (char*)bsearch(pixel, palette, 256, 4, comparecols);
  if (p != NULL)
    return(p[3]);

  /* If no perfect match is found, issue a warning and */
  /* perform an extensive search for the closest match */
  if (!*warning) /* Warn only once */
  {
    fprintf(stderr, "WARNING: the PPM file uses colors not present\n%s\n",
	    "in the color palette. The quantization will take some time.");
    *warning = TRUE;
  }
  minerr = 256 * 256 * 3;
  for (i = 0; i < 256; i++)
  {
    err = (square(pixel[0] - palette[i][0])
           + square(pixel[1] - palette[i][1])
           + square(pixel[2] - palette[i][2]));
    if (err < minerr)
    {
      minerr = err;
      index = i;
    }
  }
  return(index);
}

int findpatch(char *name, int nump, char pname[][8])
{
  int i;

  i = 0;
  while (strncmp(name, pname[i], 8) && (i<nump))
    i++;
  if (i == nump)
  {
    fprintf(stderr,"Couldn't find wall patch %s", name);
    exit(1);
  }
  return(i);
}

void readppmfile(char *ppmfilename, short **imagedata, int *imgw, int *imgh)
{
  FILE *ppmfile;
  char magic[2];
  short *imageptr;
  int i, warning;
  unsigned char pixel[3];
  
  printf("Reading PPM file %s... \n", ppmfilename);
  ppmfile = fopen(ppmfilename, "rb");
  if (ppmfile == NULL)
  {
    fprintf(stderr,"Unable to open PPM file %s.\n", ppmfilename);
    exit(1);
  }
  if (*imagedata != NULL)
    free(*imagedata);
  fscanf(ppmfile, "%2c", magic);
  if ((magic[0] != 'P') || (magic[1] != '6'))
  {
    fprintf(stderr,"Image file %s is not a PPM P6 file!\n", ppmfilename);
    exit(1);
  }
  fscanf(ppmfile, " ");
  while (ungetc(getc(ppmfile), ppmfile) == '#') /* While comment */
    while (getc(ppmfile) != '\n'); /* Read to end-of-line */
  fscanf(ppmfile, " %d %d %*d%*1c", imgw, imgh);
  imageptr = *imagedata = (short *)malloc(*imgw * *imgh * sizeof(short));
  
  warning = FALSE;
  for (i = 0; i < (*imgw * *imgh); i++)
  {
    pixel[0] = getc(ppmfile); /* Red */
    pixel[1] = getc(ppmfile); /* Grn */
    pixel[2] = getc(ppmfile); /* Blu */
    *imageptr++ = (short)lookup(pixel, sortedpal, &warning);
  }
  fclose(ppmfile);
}

void rawtodoom(short *imagedata, int imgw, int imgh, int ox, int oy,
	       int w, int h, int x, int y, unsigned char *doomimage,
	       int *reslength)
{
  int i, j;
  int dataptr, tempptr;
  short pxl;

  doomimage[0] = w & 0xff;
  doomimage[1] = (w & 0xff00) >> 8;
  doomimage[2] = h & 0xff;
  doomimage[3] = (h & 0xff00) >> 8;
  doomimage[4] = x & 0xff;
  doomimage[5] = (x & 0xff00) >> 8;
  doomimage[6] = y & 0xff;
  doomimage[7] = (y & 0xff00) >> 8;
  /* doomimage[8] to doomimage[8 + w * 4 - 1] are column indices */
  dataptr = 8 + w * 4;
  for (i = 0; i < w; i++)
  {
    doomimage[8 + 4 *i]      = dataptr & 0xff;
    doomimage[8 + 4 * i + 1] = (dataptr & 0xff00) >> 8;
    doomimage[8 + 4 * i + 2] = (dataptr & 0xff0000) >> 16;
    doomimage[8 + 4 * i + 3] = (dataptr & 0xff000000) >> 24;

    j = 0;
    while (j < h)
    {
      while ((imagedata[(oy + j) * imgw + ox + i] == -1) && (j < h))
	j++;
      if (j >= h)
	break;
      /* Write out one post */
      doomimage[dataptr++] = j & 0xff;
      tempptr = dataptr++;
      doomimage[dataptr++] = 0;	/* One dummy pixel for some reason */
      while ((j < h) && ((pxl = imagedata[(oy + j) * imgw + ox + i]) != -1))
      {
	doomimage[dataptr++] = pxl;
	j++;
      }
      doomimage[dataptr++] = 0;	/* One dummy pixel for some reason */
      doomimage[tempptr] = dataptr - tempptr - 3;
    }
    /* Write out the end-of-column marker */
    doomimage[dataptr++] = 255;
  }
  *reslength = dataptr;
}

void appenddata(FILE *wadfile, char *filename)
{
  FILE *datafile;
  int buf;

  datafile = fopen(filename, "rb");
  if (datafile != NULL)
  {
    while (!feof(datafile))
    {
      buf = getc(datafile);
      if (!feof(datafile))
	putc(buf, wadfile);
    }
    fclose(datafile);
  }
}

void allowcomment(FILE *infile)
{
  while (ungetc(getc(infile), infile) == '#')
    while (getc(infile) != '\n');
}

int initpnames(char *filename, char pname[][8])
{
  FILE *pnfile;
  int n;

  pnfile = fopen(filename, "rb");
  if (pnfile == NULL)
  {
    fprintf(stderr, "Unable to open data file %s.\n", filename);
    exit(1);
  }
  n = getlong(pnfile);
  fread(pname, 8, n, pnfile);
  fclose(pnfile);
  return(n);
}

int inittexture(char *filename, long tsize[], int *reslength, FILE *datafile)
{
  FILE *tfile;
  int numtextures, i, j, n;
  char tname[8];

  tfile = fopen(filename, "rb");
  if (tfile == NULL)
  {
    fprintf(stderr, "Unable to open data file %s.\n", filename);
    exit(1);
  }
  numtextures = getlong(tfile);
  for (i = 0; i < numtextures; i++)
    getlong(tfile); /* Skip the pointer table - it is rebuilt later */
  for (i = 0; i < numtextures; i++)
  {
    fread(tname, 8, 1, tfile);
    fwrite(tname, 8, 1, datafile);
    putshort(getshort(tfile), datafile);
    putshort(getshort(tfile), datafile);
    putshort(getshort(tfile), datafile); /* Width */
    putshort(getshort(tfile), datafile); /* Height */
    putlong(getlong(tfile), datafile);
    putshort(n = getshort(tfile), datafile); /* Numpatches */
    for (j = 0; j < n; j++)
    {
      putshort(getshort(tfile), datafile); /* X offset */
      putshort(getshort(tfile), datafile); /* Y offset */
      putshort(getshort(tfile), datafile); /* Patch index */
      putshort(getshort(tfile), datafile);
      putshort(getshort(tfile), datafile);
    }
    tsize[i] = 22 + 10 * n;
    *reslength += tsize[i] + 4;
  }
  return numtextures;
}

int createtexture(char *texturename, FILE *infile, FILE *datafile,
		  int numpatches, char pname[][8])
{
  int w, h, texturepatches, x, y;
  int i, j;
  char texturename8[8];
  char patchname[20];

  fscanf(infile, " %d %d %d ", &w, &h, &texturepatches);
  allowcomment(infile);
  strncpy(texturename8, texturename, 8);
  fwrite(texturename8, 8, 1, datafile);
  putshort(0, datafile);
  putshort(0, datafile);
  putshort(w, datafile);
  putshort(h, datafile);
  putlong(0, datafile);
  putshort(texturepatches, datafile);
  for (i = 0; i < texturepatches; i++)
  {
    fscanf(infile, " %s %d %d ", patchname, &x, &y);
    allowcomment(infile);
    j = findpatch(patchname, numpatches, pname);
    putshort(x, datafile);
    putshort(y, datafile);
    putshort(j, datafile);
    putshort(0, datafile);
    putshort(0, datafile);
  }
  return (22 + 10 * texturepatches);
  /* t1size[numt1++] = 22 + 10 * texturepatches; */
  /* reslength += t1size[numt1-1] + 4; */
  /* fscanf(infile, " %s ", keyword); */
}

void usage(char *s)
{
  fprintf(stderr,"Usage: %s [-c palette] infile outfile\n", s);
  fprintf(stderr,"\tpalette can be doom, hexen, heretic or strife\n");
  exit(1);
}

int main(int argc, char *argv[])
{
  FILE *infile, *wadfile;
  FILE *dirfile1, *dirfile2, *datafile, *origfile;
  char keyword[20];
  char spritename[20];
  char origname[8];
  char ppmfilename[80], lastppmfilename[80];
  short *imagedata;
  int imgw, imgh;
  int ox, oy, w, h, x, y;
  unsigned char *doomimage;
  int reslength, ackreslength, numdirentries;
  int i, j;
  int lastsprite;
  char pname[1024][8]; /* For PNAMES index lookup entry */
  int texturepatches = 0, numpatches = 0;
  long t1size[512]; /* Temp buffer for TEXTURE1 sizes */
  long t2size[512]; /* Temp buffer for TEXTURE2 sizes */
  int numt1, numt2, orignumt1, orignumt2, ackpos;
  char entryname[9], entryname8[8];
  int entrysize, entrypos;
  int spritesection, patchsection, texture1section, texture2section;
  int flat1section, flat2section;
  int imagesection;
  char *s;
  char *program;

  program = argv[0];

  if (argc < 3)
    usage(program);

  while ((--argc > 0) && (**++argv == '-'))
  {
    for (s = *argv+1; *s != '\0'; s++)
    {
      switch (*s)
      {
	case 'c':
	  argc--;
	  argv++;
	  if (!strcmp(*argv, "doom"))
	    palette = doom_rgb;
	  else if (!strcmp(*argv, "heretic"))
	    palette = heretic_rgb;
	  else if (!strcmp(*argv, "hexen"))
	    palette = hexen_rgb;
	  else if (!strcmp(*argv, "strife"))
	    palette = strife_rgb;
	  else
	    usage(program);
	  break;
	default:
	  usage(program);
      }
    }
  }

  if (argc < 2)
    usage(program);

  infile = fopen(argv[0], "r");
  if (infile == NULL)
  {
    fprintf(stderr,"Unable to open input file %s\n", argv[0]);
    exit(1);
  }
  wadfile = fopen(argv[1], "wb");
  if (wadfile == NULL)
  {
    fprintf(stderr,"Unable to create output file %s\n", argv[1]);
    exit(1);
  }
  dirfile1 = fopen("temp.d1", "wb"); /* Directory for TEXTURE1&2 and PNAMES */
  if (dirfile1 == NULL)
  {
    fprintf(stderr,"Unable to create temporary file temp.d1\n");
    exit(1);
  }
  dirfile2 = fopen("temp.d2", "wb"); /* Directory from S_START to F_END */
  if (dirfile2 == NULL)
  {
    fprintf(stderr,"Unable to create temporary file temp.d2\n");
    exit(1);
  }
  lastppmfilename[0] = '\0';
  imagedata = NULL;
  /* No dynamic allocation - just make room for a full 320x200 image */
  doomimage = (unsigned char *)malloc(68168 * sizeof(char));
  ackreslength = 0;
  numdirentries = 0;

  /* Sort the palette to speed up the image input */
  for (i = 0; i < 256; i++)
  {
    sortedpal[i][0] = palette[i * 3];
    sortedpal[i][1] = palette[i * 3 + 1];
    sortedpal[i][2] = palette[i * 3 + 2];
    sortedpal[i][3] = i;
  }      
  qsort((char*)sortedpal, 256, 4, comparecols);

  spritesection = FALSE;
  patchsection = FALSE;
  texture1section = FALSE;
  texture2section = FALSE;
  flat1section = FALSE;
  flat2section = FALSE;
  imagesection = FALSE;

  allowcomment(infile);
  fscanf(infile, " %s ", keyword);

  if (!strcmp(keyword,"S_START"))
  {
    spritesection = TRUE;
    datafile = fopen("temp.s", "wb");
    if (datafile == NULL)
    {
      fprintf(stderr,"Unable to create temporary file temp.s\n");
      exit(1);
    }
    fprintf(dirfile2, "S_START 0\n");
    numdirentries += 1;
    /* Process sprites until S_END is found */
    lastsprite = 0;
    while (!lastsprite)
    {
      allowcomment(infile);
      fscanf(infile, " %s ", spritename);
      if (!strcmp(spritename, "S_END"))
	lastsprite = 1;
      else
      {
	printf("Creating sprite %s... \n", spritename);
	fscanf(infile, " %s @ %d %d W %d H %d X %d Y %d ",
	       ppmfilename, &ox, &oy, &w, &h, &x, &y);
	allowcomment(infile);
	/* If the ppm file is not the same as before: read it in */
	if (strcmp(ppmfilename, lastppmfilename))
	{
	  readppmfile(ppmfilename, &imagedata, &imgw, &imgh);
	  strcpy(lastppmfilename, ppmfilename);
	}
	/* Clip out the specified region and convert it */
	rawtodoom(imagedata, imgw, imgh, ox, oy, w, h, x, y,
		  doomimage, &reslength);
	/* Write data to datafile and some info to dirfile2 */
	for (i = 0; i < reslength; i++)
	  putc(doomimage[i], datafile);
	fprintf(dirfile2, "%s %d\n", spritename, reslength);
	ackreslength += reslength;
	numdirentries += 1;
      }
    }
    fprintf(dirfile2, "S_END 0\n");
    numdirentries += 1;
    fclose(datafile);
    fscanf(infile, " %s ", keyword);
  }

  if (!strcmp(keyword, "P_START"))
  {
    patchsection = TRUE;
    /* Initialize PNAMES with all the original entries */
    numpatches = initpnames(PNFILENAME, pname);
    datafile = fopen("temp.p", "wb");
    if (datafile == NULL)
    {
      fprintf(stderr,"Unable to create temporary file temp.p\n");
      exit(1);
    }
    fprintf(dirfile2, "PP_START 0\n");
    numdirentries += 1;
    /* Read patches one by one, updating pname[] and numpatches */
    lastsprite = 0;
    while (!lastsprite)
    {
      allowcomment(infile);
      fscanf(infile, " %s ", spritename);
      if (!strcmp(spritename, "P_END"))
	lastsprite = 1;
      else
      {
	printf("Creating wall patch %s... \n", spritename);
	strncpy(pname[numpatches++], spritename, 8);
	fscanf(infile, " %s @ %d %d W %d H %d ",
	       ppmfilename, &ox, &oy, &w, &h);
	allowcomment(infile);
	/* If the ppm file is not the same as before: read it in */
	if (strcmp(ppmfilename, lastppmfilename))
	{
	  readppmfile(ppmfilename, &imagedata, &imgw, &imgh);
	  strcpy(lastppmfilename, ppmfilename);
	}
	/* Clip out the specified region and convert it */
	rawtodoom(imagedata, imgw, imgh, ox, oy, w, h, w/2-1, h,
		  doomimage, &reslength);
	/* Write data to datafile and some info to dirfile2 */
	for (i = 0; i < reslength; i++)
	  putc(doomimage[i], datafile);
	fprintf(dirfile2, "%s %d\n", spritename, reslength);
	ackreslength += reslength;
	numdirentries += 1;
      }
    }
    fprintf(dirfile2, "PP_END 0\n");
    numdirentries += 1;
    fclose(datafile);
    allowcomment(infile);
    fscanf(infile, " %s ", keyword);
  }

  numt1 = 0;
  if (!strcmp(keyword, "TEXTURE1_START"))
  {
    texture1section = TRUE;
    datafile = fopen("temp.t1", "wb");
    reslength = 4;
    /* Open the file containing all the original entries */
    origfile = fopen(T1FILENAME, "rb");
    if (origfile == NULL)
    {
      fprintf(stderr, "Unable to open data file %s.\n", T1FILENAME);
      exit(1);
    }
    orignumt1 = getlong(origfile);
    for (i = 0; i < orignumt1; i++)
      getlong(origfile); /* Skip the pointer table - it is rebuilt later */
    fread(origname, 8, 1, origfile);

    /* Read the textures, updating t1size, numt1 and reslength */
    allowcomment(infile);
    fscanf(infile, " %s ", keyword);
    while (strcmp(keyword, "TEXTURE1_END"))
    {
      /* while ((original textures left) && (name does not match)) */
      /*     copy one original texture to output */
      while ((numt1 < orignumt1) && (strncmp(origname, keyword, 8)))
      {
	fwrite(origname, 8, 1, datafile);
	for (i = 0; i < 12; i++)
	  putc(getc(origfile), datafile);
	texturepatches = getshort(origfile);
	putshort(texturepatches, datafile);
	for (i = 0; i < texturepatches * 10; i++)
	  putc(getc(origfile), datafile);
	t1size[numt1] = 22 + texturepatches * 10;
	reslength += t1size[numt1++] + 4;
	if (numt1 < orignumt1)
	  fread(origname, 8, 1, origfile);
      }

      /* if (name matches) */
      /*     replace original texture with the new one */
      /*     read past the original texture data */
      if (!strncmp(origname, keyword, 8))
      {
	printf("Replacing texture %s...\n", keyword);
	t1size[numt1] = createtexture(keyword, infile, datafile,
				      numpatches, pname);
	reslength += t1size[numt1] + 4;
	numt1++;
	for (i = 0; i < 12; i++)
	  getc(origfile);
	texturepatches = getshort(origfile);
	for (i = 0; i < texturepatches * 10; i++)
	  getc(origfile);
	fscanf(infile, " %s ", keyword);
	if (numt1 < orignumt1)
	  fread(origname, 8, 1, origfile);
      }
      else
      {
	/* No original entries remain, start creating new ones */
	printf("Creating texture %s... \n", keyword);
	t1size[numt1] = createtexture(keyword, infile, datafile,
				      numpatches, pname);
	reslength += t1size[numt1] + 4;
	numt1++;
	fscanf(infile, " %s ", keyword);
      }
    }
    /* while (original textures left)          */
    /*     copy one original texture to output */
    while (numt1 < orignumt1)
    {
      fwrite(origname, 8, 1, datafile);
      for (i = 0; i < 12; i++)
	putc(getc(origfile), datafile);
      texturepatches = getshort(origfile);
      putshort(texturepatches, datafile);
      for (i = 0; i < texturepatches * 10; i++)
	putc(getc(origfile), datafile);
      t1size[numt1] = 22 + texturepatches * 10;
      reslength += t1size[numt1++] + 4;
      if (numt1 < orignumt1)
	fread(origname, 8, 1, origfile);
    }
    fprintf(dirfile1, "TEXTURE1 %d\n", reslength);
    numdirentries += 1;
    ackreslength += reslength;
    fclose(datafile);
    allowcomment(infile);
    fscanf(infile, " %s ", keyword);
  }

  numt2 = 0;
  if (!strcmp(keyword, "TEXTURE2_START"))
  {
    texture2section = TRUE;
    datafile = fopen("temp.t2", "wb");
    reslength = 4;
    /* Open the file containing all the original entries */
    origfile = fopen(T2FILENAME, "rb");
    if (origfile == NULL)
    {
      fprintf(stderr, "Unable to open data file %s.\n", T2FILENAME);
      exit(1);
    }
    orignumt2 = getlong(origfile);
    for (i = 0; i < orignumt2; i++)
      getlong(origfile); /* Skip the pointer table - it is rebuilt later */
    fread(origname, 8, 1, origfile);

    /* Read the textures, updating t2size, numt2 and reslength */
    allowcomment(infile);
    fscanf(infile, " %s ", keyword);
    while (strcmp(keyword, "TEXTURE2_END"))
    {
      /* while ((original textures left) && (name does not match)) */
      /*     copy one original texture to output */
      while ((numt2 < orignumt2) && (strncmp(origname, keyword, 8)))
      {
	fwrite(origname, 8, 1, datafile);
	for (i = 0; i < 12; i++)
	  putc(getc(origfile), datafile);
	texturepatches = getshort(origfile);
	putshort(texturepatches, datafile);
	for (i = 0; i < texturepatches * 10; i++)
	  putc(getc(origfile), datafile);
	t2size[numt2] = 22 + texturepatches * 10;
	reslength += t2size[numt2++] + 4;
	if (numt2 < orignumt2)
	  fread(origname, 8, 1, origfile);
      }

      /* if (name matches) */
      /*     replace original texture with the new one */
      /*     read past the original texture data */
      if (!strncmp(origname, keyword, 8))
      {
	printf("Replacing texture %s...\n", keyword);
	t2size[numt2] = createtexture(keyword, infile, datafile,
				      numpatches, pname);
	reslength += t2size[numt2] + 4;
	numt2++;
	for (i = 0; i < 12; i++)
	  getc(origfile);
	texturepatches = getshort(origfile);
	for (i = 0; i < texturepatches * 10; i++)
	  getc(origfile);
	fscanf(infile, " %s ", keyword);
	if (numt2 < orignumt2)
	  fread(origname, 8, 1, origfile);
      }
      else
      {
	/* No original entries remain, start creating new ones */
	printf("Creating texture %s... \n", keyword);
	t2size[numt2] = createtexture(keyword, infile, datafile,
				      numpatches, pname);
	reslength += t2size[numt2] + 4;
	numt2++;
	fscanf(infile, " %s ", keyword);
      }
    }
    /* while (original textures left)          */
    /*     copy one original texture to output */
    while (numt2 < orignumt2)
    {
      fwrite(origname, 8, 1, datafile);
      for (i = 0; i < 12; i++)
	putc(getc(origfile), datafile);
      texturepatches = getshort(origfile);
      putshort(texturepatches, datafile);
      for (i = 0; i < texturepatches * 10; i++)
	putc(getc(origfile), datafile);
      t2size[numt2] = 22 + texturepatches * 10;
      reslength += t2size[numt2++] + 4;
      if (numt2 < orignumt2)
	fread(origname, 8, 1, origfile);
    }
    fprintf(dirfile1, "TEXTURE2 %d\n", reslength);
    numdirentries +=1;
    ackreslength += reslength;
    fclose(datafile);
    allowcomment(infile);
    fscanf(infile, " %s ", keyword);
  }

  if (patchsection)
  {
    fprintf(dirfile1, "PNAMES %d\n", numpatches * 8 + 4);
    numdirentries += 1;
    ackreslength += (numpatches * 8 + 4);
  }
  fclose(dirfile1);
  
  if (!strcmp(keyword, "F_START"))
  {
    fprintf(dirfile2, "F_START 0\n");
    numdirentries += 1;
    allowcomment(infile);
    fscanf(infile, " %s ", keyword);
    if (!strcmp(keyword, "F1_START"))
    {
      allowcomment(infile);
      flat1section = TRUE;
      fprintf(dirfile2, "F1_START 0\n");
      numdirentries += 1;
      datafile = fopen("temp.f1", "wb");
      /* Read the floor tiles one by one, writing names to dirfile2 */
      fscanf(infile, " %s ", keyword);
      while (strcmp(keyword, "F1_END"))
      {
	printf("Creating floor/ceiling %s... \n", keyword);
	fscanf(infile, " %s @ %d %d ", ppmfilename, &ox, &oy);
	allowcomment(infile);
	/* If the ppm file is not the same as before: read it in */
	if (strcmp(ppmfilename, lastppmfilename))
	{
	  readppmfile(ppmfilename, &imagedata, &imgw, &imgh);
	  strcpy(lastppmfilename, ppmfilename);
	}
	/* Write out the specified region in raw format */
	for (i = 0; i < 64; i++)
	  for (j = 0; j < 64; j++)
	    putc(imagedata[(oy + i) * imgw + ox + j], datafile);
	/* Write some info to dirfile2 */
	fprintf(dirfile2, "%s %d\n", keyword, 4096);
	ackreslength += 4096;
	numdirentries += 1;
	fscanf(infile, " %s ", keyword);
      }
      fprintf(dirfile2, "F1_END 0\n");
      numdirentries += 1;
      fclose(datafile);
      allowcomment(infile);
      fscanf(infile, " %s ", keyword);
    }
    if (!strcmp(keyword, "F2_START"))
    {
      allowcomment(infile);
      flat2section = TRUE;
      fprintf(dirfile2, "F2_START 0\n");
      numdirentries += 1;
      datafile = fopen("temp.f2", "wb");
      /* Read the floor tiles one by one, writing names to dirfile2 */
      fscanf(infile, " %s ", keyword);
      while (strcmp(keyword, "F2_END"))
      {
	printf("Creating floor/ceiling %s... \n", keyword);
	fscanf(infile, " %s @ %d %d ", ppmfilename, &ox, &oy);
	allowcomment(infile);
	/* If the ppm file is not the same as before: read it in */
	if (strcmp(ppmfilename, lastppmfilename))
	{
	  readppmfile(ppmfilename, &imagedata, &imgw, &imgh);
	  strcpy(lastppmfilename, ppmfilename);
	}
	/* Write out the specified region in raw format */
	for (i = 0; i < 64; i++)
	  for (j = 0; j < 64; j++)
	    putc(imagedata[(oy + i) * imgw + ox + j], datafile);
	/* Write some info to dirfile2 */
	fprintf(dirfile2, "%s %d\n", keyword, 4096);
	ackreslength += 4096;
	numdirentries += 1;
	allowcomment(infile);
	fscanf(infile, " %s ", keyword);
      }
      fprintf(dirfile2, "F2_END 0\n");
      numdirentries += 1;
      fclose(datafile);
      allowcomment(infile);
      fscanf(infile, " %s ", keyword);
    }
    fprintf(dirfile2, "F_END 0\n");
    numdirentries += 1;
    allowcomment(infile);
    fscanf(infile, " %s ", keyword);
  }

  if (!strcmp(keyword, "I_START"))
  {
    imagesection = TRUE;
    datafile = fopen("temp.i", "wb");
    if (datafile == NULL)
    {
      fprintf(stderr,"Unable to create temporary file temp.i\n");
      exit(1);
    }
    /* Read images one by one, until I_END found */
    lastsprite = 0;
    while (!lastsprite)
    {
      allowcomment(infile);
      fscanf(infile, " %s ", spritename);
      if (!strcmp(spritename, "I_END"))
	lastsprite = 1;
      else
      {
	printf("Creating image %s... \n", spritename);
	fscanf(infile, " %s ", ppmfilename);
	allowcomment(infile);
	/* If the ppm file is not the same as before: read it in */
	if (strcmp(ppmfilename, lastppmfilename))
	{
	  readppmfile(ppmfilename, &imagedata, &imgw, &imgh);
	  strcpy(lastppmfilename, ppmfilename);
	}
	/* Clip out the specified region and convert it */
	rawtodoom(imagedata, imgw, imgh, 0, 0, imgw, imgh, 0, 0,
		  doomimage, &reslength);
	/* Write data to datafile and some info to dirfile2 */
	for (i = 0; i < reslength; i++)
	  putc(doomimage[i], datafile);
	fprintf(dirfile2, "%s %d\n", spritename, reslength);
	ackreslength += reslength;
	numdirentries += 1;
      }
    }
    fclose(datafile);
    allowcomment(infile);
    fscanf(infile, " %s ", keyword);
  }

  fclose(infile);
  fclose(dirfile2);
  free(doomimage);

  /* Concatenate all the data into the final PWAD file */
  printf("Creating WAD file %s... \n", argv[1]);

  /* Header */
  fprintf(wadfile, "PWAD");
  putlong(numdirentries, wadfile);
  ackreslength += 12; /* The header is 12 bytes */
  putlong(ackreslength, wadfile);

  /* TEXTURE1 */
  if (texture1section)
  {
    printf("Adding entry TEXTURE1\n");
    putlong(numt1, wadfile);
    ackpos = 4 + numt1 * 4;
    for (i = 0; i < numt1; i++)
    {
      putlong(ackpos, wadfile);
      ackpos += t1size[i];
    }
    appenddata(wadfile, "temp.t1");
  }

  /* TEXTURE2 */
  if (texture2section)
  {
    printf("Adding entry TEXTURE2\n");
    putlong(numt2, wadfile);
    ackpos = 4 + numt2 * 4;
    for (i = 0; i < numt2; i++)
    {
      putlong(ackpos, wadfile);
      ackpos += t2size[i];
    }
    appenddata(wadfile, "temp.t2");
  }

  /* PNAMES */
  if (patchsection)
  {
    printf("Adding entry PNAMES\n");
    putlong(numpatches, wadfile);
    fwrite(&pname[0][0], 8, numpatches, wadfile);
  }
  
  /* Sprites */
  if (spritesection)
  {
    printf("Adding sprite entries\n");
    appenddata(wadfile, "temp.s");
  }

  /* Wall patches */
  if (patchsection)
  {
    printf("Adding wall patch entries\n");
    appenddata(wadfile, "temp.p");
  }

  /* Floor tiles */
  if (flat1section)
  {
    printf("Adding FLATS1 entries\n");
    appenddata(wadfile, "temp.f1");
  }
  if (flat2section)
  {
    printf("Adding FLATS2 entries\n");
    appenddata(wadfile, "temp.f2");
  }

  /* Images */
  if (imagesection)
  {
    printf("Adding image enties\n");
    appenddata(wadfile, "temp.i");
  }
  
  /* Now, write the directory by using data from the directory files. */
  entrypos = 12;
  dirfile1 = fopen("temp.d1", "rb");
  if (dirfile1 != NULL)
  {
    while (!feof(dirfile1))
    {
      fscanf(dirfile1, " %s %d", entryname, &entrysize);
      if (!feof(dirfile1))
      {
	putlong(entrypos, wadfile);
	putlong(entrysize, wadfile);
	strncpy(entryname8, entryname, 8);
	fwrite(entryname8, 8, 1, wadfile);
	entrypos += entrysize;
      }
    }
    fclose(dirfile1);
  }

  dirfile2 = fopen("temp.d2","rb");
  if (dirfile2 != NULL)
  {
    while (!feof(dirfile2))
    {
      fscanf(dirfile2, " %s %d", entryname, &entrysize);
      if (!feof(dirfile2))
      {
	putlong(entrypos, wadfile);
	putlong(entrysize, wadfile);
	strncpy(entryname8, entryname, 8);
	fwrite(entryname8, 8, 1, wadfile);
	entrypos += entrysize;
      }
    }
    fclose(dirfile2);
  }

  /* DONE! */
  printf("WAD file %s complete.\n", argv[1]);
  fclose(wadfile);
  return 0;
}
