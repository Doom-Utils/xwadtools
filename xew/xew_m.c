/* xew_m.c : main function etc. for the Unix Forms version of ew.
  
   Author : Hans Peter Verne (hpv@kjemi.uio.no) 
            Many parts taken from Tormod's ew.c

   Copyright (C) 1996 Hans Peter Verne
 
   This is free software, distributed under the terms of the
   GNU General Public License. For details see the file COPYING
 
   For compiling and use, see the file unix.txt 
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>

#include <forms.h>
#include "xew_f.h"
#include "ew.h"
#include "xew_logo.xpm"

#ifdef AUDIO_DEVICE
#include <unistd.h> 
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#ifdef SOUND_VERSION
#define IOCTL(a,b,c)		ioctl(a,b,&c)
#else
#define IOCTL(a,b,c)		(c = ioctl(a,b,c) )
#endif
#endif /*  AUDIO_DEVICE */

extern int keep_transparent_color;

/* These were local to Tormod's massive main(), so I just 
   make them static (ie. local to this source file) : */
static Directory *pDirEnt = NULL;
static unsigned char *pEntryTag = NULL;
static Int4 *LinearMap = NULL;
static FILE *WADfp = NULL;

/* the RGB's read from file, if any */
static char *USER_rgb = NULL;

#define EXTERNALLY_STR "Externally"
#define INTERNALLY_STR "Internally"

#define  PAL_USER      "User"
#define  PAL_DOOM      "DOOM"
#define  PAL_HERETIC   "HERETIC" 
#define  PAL_E_HERETIC "Ex. Heretic"
#define  PAL_HEXEN     "HEXEN"
#define  PAL_STRIFE    "STRIFE"

#define XPM_STR "XPM"
#define PCX_STR "PCX"


static const char *TagString( int tag ) 
{
  switch ( tag ) 
    {
    case TAG_NONE : return "(none)";
    case TAG_FLAT : return "(flat)";
    case TAG_FULL : return "(full)";
    case TAG_HIRES: return "(hires)";
    case TAG_IMG  : return "(image)";
    case TAG_MUS  : return "(mus)";
    case TAG_SFX  : return "(sfx)";    
    default       : return "(unknown)";
    }
  
  return ""; /* notreached */
}



/* spawn external program associated with index.
   we assume an external program has been defined.
   */

static void SpawnExternal( Int4 index ) 
{
  int RetVal = -1;
  unsigned char *pImgBuf;
  short Width;
  short Height;

  if ( tmpnam(TmpNameStr) ) {
    switch ( pEntryTag[index] )
      {
      case TAG_MUS  : 
	if ( !(RetVal = Gen_MUS_File(WADfp, pDirEnt[index], TmpNameStr)) )
	  sprintf(ExeStr, Ext_MUS_Player, TmpNameStr);
	break;
      case TAG_SFX  :
	if ( !(RetVal = Gen_WAV_File(WADfp, pDirEnt[index], TmpNameStr)) )
	  sprintf(ExeStr, Ext_WAV_Player, TmpNameStr);
	break;
      case TAG_FLAT :
      case TAG_FULL :
      case TAG_IMG  :
	pImgBuf = ParseImgBlock(pDirEnt[index], pEntryTag[index], 
				&Width, &Height, NULL, NULL, WADfp);
	
	if (pImgBuf == NULL)
	  RetVal = 4;
	else  
	  if ((RetVal = PCX_WriteImage(pDirEnt[index], WADfp, Width, Height,
				       pImgBuf, 1, TmpNameStr)) == 0)
	    sprintf(ExeStr, Ext_PCX_Viewer, TmpNameStr);
	
	break;
      case TAG_HIRES:
	if ( !(RetVal = PCX_WriteImage(pDirEnt[index], WADfp, 640, 480, 
				       pBitMap, 4, TmpNameStr)) )
	  sprintf(ExeStr, Ext_PCX_Viewer, TmpNameStr);
	break;
      default:  RetVal = 5; break;
      }
  }
  
  if ( !RetVal ) {
    Message("Spawning \"%s\" ...\n", ExeStr); fflush(stdout);
    system(ExeStr);
    remove(TmpNameStr);
    Message("Returned from spawn.\n");
  }
  else {
    Beep();
    Message("error generating external file \"%s\" (%d)\n", 
	    TmpNameStr, RetVal);
  }

}


/* Copy RGB entries.  Arrays are assumed to be of right size, ie. 3*256.
 */
#define CopyRGB( NewRGB, OldRGB )  memcpy(NewRGB, OldRGB, (3*256) )


/*  Play raw data on /dev/dsp or whatever.  This is snatched
    from "vplay.c", by Michael Beck - beck@informatik.hu-berlin.de */
static void PlayRaw( unsigned char *samples, int n, int rate )
{
#ifdef AUDIO_DEVICE
  int audio, tmp;

  /* I guess we could have saved some nanoseconds by opening
     the device and do all the checking & setup *once* at startup,
     but then we would block the device, and we will not do that! */

  audio = open(AUDIO_DEVICE, O_WRONLY, 0);
  if (audio == -1) {
    Message("can't open audio device %s...\n",AUDIO_DEVICE);
    return;
  }
  
  IOCTL(audio, SNDCTL_DSP_GETBLKSIZE, tmp);
  if (tmp < 4096 || tmp > 65536) {
    Message("Invalid audio buffers size %d\n", tmp);
    close(audio);
    return;
  }
  
  if (ioctl(audio, SNDCTL_DSP_SYNC, NULL) < 0) {
    Message("couldn't sync dsp device %s...\n",AUDIO_DEVICE);
    close(audio);
    return;
  }
  
  /* set samplesize 8 */
  tmp = 8;
  IOCTL(audio, SNDCTL_DSP_SAMPLESIZE, tmp);
  if ( tmp != 8 ) {
    Message("unable to set 8 bit sample size!\n");
    close(audio);
    return;
  }

  /* set mono */
  tmp = 0;
  if ( IOCTL(audio, SNDCTL_DSP_STEREO, tmp) < 0) {
    Message("unable to set MONO\n");
    close(audio);
    return;
  }

  /* set speed */
  if (IOCTL(audio, SNDCTL_DSP_SPEED, rate ) < 0) {
    Message("unable to set audio speed to %d\n",rate);
    close(audio);
    return;
  }

  /* go : */
  tmp = write(audio, samples, n);
  if ( tmp != n ) 
    Message("could only write %d of %d samples...\n",tmp,n);

  close(audio);

#else
  Message("Not compiled with audio support!\n");
#endif /* AUDIO_DEVICE */
}


static void ReverseBuffer( unsigned char *buf, int n )
{
  int i=0;
  unsigned char tmp;  /* I don't care about fancy swap operations... */

  while( i < n ) {
    tmp = buf[i];
    buf[i] = buf[n];
    buf[n] = tmp;
    n--;
    i++;
  }
}


void bg_cb( FL_OBJECT *a, long l)
{
  /* this should be the colour set for the pixmap in xew_f.c */
  static int col = FL_BLACK;

  col = fl_show_colormap( col );
  /* set background colour for the pixmap.  Foreground 
     doesn't seem to matter... */
  fl_set_object_color(img_pxm,FL_YELLOW, col); 
  /* fl_redraw_object( img_pxm ); */
}



/* Handle the entry internally. Make an image or play the sound.... */
static void HandleInternal( Int4 index )
{
  unsigned char *sndbuf;
  unsigned char *pImgBuf ;
  char **xpmbuf=NULL;
  static char imagetext[50];
  int nc;
  unsigned short samplerate;

  /* Sound sample size: The specs says a short here, with the next
     short empty.  But some entries are (32 bits) ints, and the game
     engine accepts it.  Examples include entries 164--171 (and many
     more) in strife0.wad (shareware v. 1.1), and #50 in the "Clint
     Eastwood DOOM (II?) sfx" pwad.... */
  Uint4 nsamples;
  short w, h, dx, dy;
  
  switch ( pEntryTag[index] )
    {
    case TAG_SFX  : 
      /* Read sound header */
      fseek( WADfp, pDirEnt[index].start + 2, SEEK_SET );
      fread( &samplerate, sizeof(short), 1, WADfp );
      fread( &nsamples, sizeof(Uint4), 1, WADfp );
      if ( nsamples != pDirEnt[index].size - 8)
	Message("Warning, sample size = %d, while Directory size = %d.\n",
		nsamples, pDirEnt[index].size); 
      sndbuf = malloc( nsamples );
      if ( !sndbuf ) {
	Message("can't malloc data for snd buf (%d bytes)\n",nsamples);
	return;
      }
      /* read the samples */
      fread( sndbuf, 1, nsamples, WADfp );
      if ( fl_get_button(rev_but) )            	/* reverse ! */
	ReverseBuffer( sndbuf, nsamples );
      PlayRaw( sndbuf, nsamples, samplerate );
      free( sndbuf );
      break;
    case TAG_FLAT :
    case TAG_FULL :
    case TAG_IMG  : 
      pImgBuf = ParseImgBlock(pDirEnt[index], pEntryTag[index], 
			      &w, &h, &dx, &dy, WADfp);
      /* I planned to use the offsets (dx,dy) for better placement of
	 the image, but I need to find out some useful heuristics
	 for how to do it..... 
	 printf("image: %dx%d, ofs left %d, top %d\n",w,h,dx,dy);
	 */
      if ( pImgBuf && w <= 320 && h <= 200 && 
	   (xpmbuf = CreateXpmImage( pImgBuf, w, h, 0, &nc)) ) {
	fl_free_pixmap_pixmap( img_pxm );
	fl_set_pixmap_data( img_pxm, xpmbuf );
	sprintf(imagetext,"#%d, %dx%d, %d colours",index,w,h,nc);
	fl_set_object_label(img_txt, imagetext);
      }
      else {
	Message("error creating image. (%p, %hdx%hd, %p)\n", 
		pImgBuf, w, h, xpmbuf);
	return;
      }
      break;
    default : Message("entry %d, Not implemented\n",index); break;
    }
}


/* wb_cb : WAD browser callback.  This function is called 
   whenever a user presses an entry in the browser.  */
void wb_cb(FL_OBJECT *a, long l)
{
  int line;
  Int4 index;
  int RetVal;
  const char *ch;

  line = fl_get_browser(a);  /* get line number in browser */
  if ( line == 1 )           /* first line is the legend */
    return;

  index = LinearMap[line];

  /* check appropriate choice-box to see if we should handle 
     this ourself, or spawn an external program */
  switch ( pEntryTag[index] )
    {
    case TAG_SFX  :  ch = fl_get_choice_text( snd_cho ); break;
    case TAG_FLAT :
    case TAG_FULL :
    case TAG_IMG  :  ch = fl_get_choice_text( img_cho ); break;
    case TAG_HIRES:  
      ch = fl_get_choice_text( img_cho );
      if ( !strcmp( ch, INTERNALLY_STR ) )
	Message("Entry type %s cannot be handled internally, "
		"spawning external... \n", TagString(pEntryTag[index]) );
      /* fallthrough */
    case TAG_MUS  :  ch = EXTERNALLY_STR; break;
    default       :  ch = NULL; break;
    }
 
  if ( !strcmp( ch, EXTERNALLY_STR ) ) {  
    /* handle externally, but we must determine if we have an external
       program */
    switch (pEntryTag[index])
      {
      case TAG_MUS  :  RetVal = (Ext_MUS_Player[0] != '\0') ? 0 : 1;  break;
      case TAG_SFX  :  RetVal = (Ext_WAV_Player[0] != '\0') ? 0 : 1;  break;
      case TAG_FLAT :
      case TAG_FULL :
      case TAG_IMG  :
      case TAG_HIRES:  RetVal = (Ext_PCX_Viewer[0] != '\0') ? 0 : 1;  break;
      default       :  RetVal = 1;  break;
      }
    if (RetVal) {
      Message("No external program defined for type %s\n",
	     TagString(pEntryTag[index]) );
      return;
    }
    SpawnExternal( index );
  }
  else    /* handle internally */
    HandleInternal( index );
  
  /* Dump (flush) all X-events before we return, so e.g. if a user
     keeps the "down"-key pressed, the effect will be as expected.  */

  XSync( fl_get_display(), True );
}


/* export_cb : Export Callback
   Export to file the entry highlighted in the browser. */
void export_cb(FL_OBJECT *a, long l)
{
  Int4 index;
  int RetVal;
  unsigned char *pImgBuf;
  short Width;
  short Height;
  const char *format;

  RetVal = fl_get_browser(wad_brow);
  if ( RetVal == 1 )           /* first line is the legend */
    return;
  index = LinearMap[RetVal];
  
  switch (pEntryTag[index])
    {
    case TAG_MUS  : 
      GenFileName(pDirEnt[index].name, ".mus", TmpNameStr);
      RetVal = Gen_MUS_File(WADfp, pDirEnt[index], TmpNameStr);
      break;
    case TAG_SFX  :
      GenFileName(pDirEnt[index].name, ".wav", TmpNameStr);
      RetVal = Gen_WAV_File(WADfp, pDirEnt[index], TmpNameStr);
      break;
    case TAG_FLAT :
    case TAG_FULL :
    case TAG_IMG  :
      pImgBuf = ParseImgBlock(pDirEnt[index], pEntryTag[index], 
			      &Width, &Height, NULL, NULL, WADfp);
      if (pImgBuf == NULL) {
	RetVal = 4;
	break;
      }
      format = fl_get_choice_text( exp_img_fmt_cho );
      if ( !strcmp(format, PCX_STR ) ) {           /* pcx */
	GenFileName(pDirEnt[index].name, ".pcx", TmpNameStr);
	RetVal = PCX_WriteImage(pDirEnt[index], WADfp, Width, Height, 
				pImgBuf, 1, TmpNameStr);
      }
      else {  /* xpm */
	char **xpm;
	GenFileName(pDirEnt[index].name, ".xpm", TmpNameStr);
	/* don't just redraw, we don't know what's in the xpm: */
	xpm = CreateXpmImage( pImgBuf, Width, Height, 0, NULL ); 
	if ( xpm && CreateXpmFile( xpm, TmpNameStr ) ) 
	  RetVal = 0;
	else
	  RetVal = 666; /* why not? */
      }
      break;
    case TAG_HIRES:
      GenFileName(pDirEnt[index].name, ".pcx", TmpNameStr);
      RetVal = PCX_WriteImage(pDirEnt[index], WADfp, 640, 480, 
			      pBitMap, 4, TmpNameStr);
      break;
    default:
      RetVal = 1;
      break;
    }
  
  if (RetVal != 0) {
    Beep();
    Message("error generating external file \"%s\" (%d)\n", 
	   TmpNameStr, RetVal);
  }
  else
    Message("External file \"%s\" created OK\n", TmpNameStr); 
}


/* null_callback for various objects.  Does nothing, as the state 
   of the object is queried when needed */
void null_cb(FL_OBJECT *a, long l) {}


/* Callback for the sound-choice.  Only used to hide/show the
   "reverse" button, and only if we have sound */
void snd_cho_cb(FL_OBJECT *a, long l)
{
#ifdef AUDIO_DEVICE
  const char *ch;

  ch = fl_get_choice_text( a );
  if ( !strcmp(ch, EXTERNALLY_STR ) ) 
    fl_hide_object( rev_but );
  else 
    fl_show_object( rev_but );
  
#endif
}



/* callback for palette-change choice */
void pal_ch_cb(FL_OBJECT *a, long l)
{
  const char *ch;
  char **current_xpm;

  ch = fl_get_choice_text( a );
  
  /* copy the RGB selected by the user to the "current" RGB. */

  if( !strcmp(ch,PAL_DOOM) )
    CopyRGB( DAC_Reg, doom_rgb);
  
  else if( !strcmp(ch,PAL_HERETIC) )
    CopyRGB( DAC_Reg, heretic_rgb);

  else if( !strcmp(ch,PAL_E_HERETIC) )
    CopyRGB( DAC_Reg, E_HERETIC_rgb);

  else if( !strcmp(ch,PAL_HEXEN) )
    CopyRGB( DAC_Reg, hexen_rgb);
  
  else if( !strcmp(ch,PAL_STRIFE) )
    CopyRGB( DAC_Reg, strife_rgb);
  
  else if( !strcmp(ch, PAL_USER) && USER_rgb )
    CopyRGB( DAC_Reg, USER_rgb);

  else  /* this should not happen */
    Abort("unknown palette type!\n");

  CreateXpmPalette( DAC_Reg, keep_transparent_color );

  /* redraw current image */
  if ( ( current_xpm = CreateXpmImage( 0, 0, 0, 1, NULL)) ) {
    fl_free_pixmap_pixmap( img_pxm );
    fl_set_pixmap_data( img_pxm, current_xpm );
  } 
}


void quit_cb(FL_OBJECT *a, long l)
{
  /* We free up whatever has been set (ie. malloc'ed): */

  if ( pDirEnt ) 
    free( pDirEnt );
  if ( pEntryTag )
    free( pEntryTag );
  if ( pBitMap )
    free( pBitMap );
  if ( LinearMap )
    free( LinearMap );
  if ( USER_rgb )
    free( USER_rgb );
  if ( WADfp )
    fclose( WADfp );
  
  exit(EXIT_SUCCESS);
}



/*  This function should work like printf to the Message Browser 
    (mesg_brow). */

void Message( char *pMessage, ... )
{
#define  MESSAGE_LEN  67      /* max length of text line in browser */

  static char s[1024];        /* enough ? */
  char line[MESSAGE_LEN+1]; 
  char *sp;
  va_list argp;
  int nl, i;

  va_start(argp, pMessage);
  vsprintf(s, pMessage, argp);
  va_end(argp);

  /* break s up into smaller chunks, if needed.  */
  nl = strlen(s)/(MESSAGE_LEN+1) + 1;

  sp = s;
  for( i = 0; i < nl; i++) {
    strncpy(line,sp,MESSAGE_LEN);
    line[MESSAGE_LEN] = '\0';  /* Am I paranoid? */
    sp +=MESSAGE_LEN;
    fl_addto_browser(mesg_brow, line );
  }

  XFlush( fl_get_display() );   /* flush the X output buffer */
}


void SpinnWheel( int stop )
{
#include "wheel.xpm"  /* here we find the static char *wheel?[]  */
  static int state;

  fl_free_pixmap_pixmap( wheel_pxm );
  fl_hide_object( wheel_pxm );

  if ( stop ) 
    return;

  switch ( state )
    { 
    default :
    case 0 : 
      fl_set_pixmap_data( wheel_pxm, wheel0 );
      state = 1;
      break;
    case 1 :
      fl_set_pixmap_data( wheel_pxm, wheel1 );
      state = 2;
      break;
    case 2 :
      fl_set_pixmap_data( wheel_pxm, wheel2 );
      state = 3;
      break;
    case 3 :
      fl_set_pixmap_data( wheel_pxm, wheel3 );
      state = 0;
      break;
    }  

  /* the hide/show and XFlush seems to be necessary to get 
     the right effect */
  fl_show_object( wheel_pxm );
  XFlush( fl_get_display() ); 

}




/* Make a linear map of useful entries.  */
Int4 *BulidLinearMap(Directory *pDir, unsigned char *pTag, 
		     Int4 nol, Int4 *useful)
{
  Int4 l, n, v;
  Int4 *linmap;
  /* first, count useful Directory entries : */
  for( n=l=0; l < nol; l++)
    if ( pTag[l] != TAG_NONE ) 
      n++;
  *useful = n;

  /* get mem for map */
  if ( !(linmap = malloc( (2 + n) * sizeof(Int4) ) ) )
    return NULL;
  
  /* fill in map,  waste the two first elements  */
  for( v=2, l=0; l < nol; l++) 
    if ( pTag[l] != TAG_NONE )
      linmap[v++] = l;

  return linmap;
}



/* fill the browser */
void FillBrowser(Int4  entries )
{
  Int4 index;
  Int4 real_index;
  char bl[100];
  
  fl_freeze_form(xew_mainf);
  fl_clear_browser(wad_brow);

  /* make a legend */
  sprintf(bl,"@_@fName     Type    Index");
  fl_add_browser_line(wad_brow, bl);

  for ( index = 0; index < entries; index++ ) {
    real_index = LinearMap[index+2];
    sprintf(bl,"%-8.8s %-7.7s %5d", pDirEnt[real_index].name,
	    TagString( pEntryTag[real_index] ),
	    real_index );
    fl_add_browser_line(wad_brow, bl);
  }
  fl_select_browser_line(wad_brow,1);
  fl_unfreeze_form(xew_mainf);
}   


/* Reads files and initializes things.  This is mostly from 
   Tormod's main() ...  */
Int4 ReadFiles(void)
{
  char *pStr;
  char Buf[10];
  Int4 NumOfLumps;
  Int4 DirOfs;
  Int4 LumpInx;
  Int4 NumLargeImg; 
  Int4 UsefulEntries;
  Int4 NumWadType[NUM_TAG];            
  Int4 LargestImgEntry;
  
  pStr = Get_pSearchList(NULL);
  if (pStr != NULL)
    pStr = FindFile(IniFileName, pStr);

  if (pStr != NULL) {
    Message("Ini file \"%s\" found\n", pStr);
    ReadIniFile(pStr);
    if (Ext_WAV_Player[0] != '\0')
      Message("WAV : \"%s\"\n", Ext_WAV_Player);
    if (Ext_MUS_Player[0] != '\0')
      Message("MUS : \"%s\"\n", Ext_MUS_Player);
    if (Ext_PCX_Viewer[0] != '\0')
      Message("PCX : \"%s\"\n", Ext_PCX_Viewer);
  }

  else
    Message("Ini file \"%s\" NOT found\n", IniFileName);

  if ( pPalFileSpec ) {
    USER_rgb = malloc( 3*256 );
    if ( !USER_rgb )
      Abort("Can't get mem for palette...\n");
    ReadPal(pPalFileSpec, USER_rgb);
    CopyRGB( DAC_Reg, USER_rgb );
    Message("Palette file '%s' loaded successfully\n", pPalFileSpec);
  }
  else 
    CopyRGB( DAC_Reg, doom_rgb); /* set doom palette as default */

  CreateXpmPalette( DAC_Reg, keep_transparent_color); 

  if ((WADfp = fopen(pWadFileSpec, "rb")) == NULL) 
    Abort("unable to open input file \"%s\"", pWadFileSpec);

  fgetc(WADfp); /* Waste the I or P before WAD */
  fread(Buf, sizeof(char), 3, WADfp);
  if (memcmp(Buf, "WAD", 3) != 0) 
    Abort("the file \"%s\" is not a proper wad file", pWadFileSpec);

  fread(&NumOfLumps, sizeof(Int4), 1, WADfp);
  fread(&DirOfs, sizeof(Int4), 1, WADfp);

  Message("Number of wad entries: %d\nWad directory offset: 0x%x\n", 
	 NumOfLumps, DirOfs);
  fseek(WADfp, DirOfs, SEEK_SET);
  
  /* Let's be real careful... */
  if ((NumOfLumps * sizeof(Directory)) > UINT_MAX)
    Abort("error, directory table too large,"
	  " cannot malloc more than (%u) bytes", UINT_MAX);

  if ((pDirEnt = malloc(NumOfLumps * sizeof(Directory))) == NULL)
    Abort("error, no memory for wad directory table");

  if ((pEntryTag = calloc(NumOfLumps, sizeof(char))) == NULL)
    Abort("error, no memory for wad directory tag table");

  if (fread(pDirEnt, sizeof(Directory), NumOfLumps, WADfp) != NumOfLumps)
    Abort("error reading wad directory");

  BuildWadDir(WADfp, pDirEnt, pEntryTag, NumOfLumps);

  /* The WAD Directory contains many entries of no value to us, these
     entries forms "holes" in pDirEnt.  We need a one-to-one
     correspondance between the line in the browser and the directory
     entry, so we build a "linear map" : The entries in the linear map
     are indices to pDirEnt (and pEntryTag), omitting the holes. */ 

  LinearMap = BulidLinearMap(pDirEnt, pEntryTag, NumOfLumps, &UsefulEntries);
  if ( !LinearMap )
    Abort("\nerror building wad directory linear map");

  Message("\nUseful WAD Entries : %d\n", UsefulEntries);
   
  /* To make sure we find something */
  memset(NumWadType, 0, sizeof(NumWadType));
  LargestImgEntry = 0;

  /* The number of large images discarded because malloc can't handle
   * them 
    */
  NumLargeImg = 0;

  /* I want to count the number of each type in the wad file. Can't
   * use the TAG_FLAT and such directly since they are bitmasks
   */
  for (LumpInx = 0; LumpInx < NumOfLumps; LumpInx++) {
    /* Is this image to large for malloc ? */
    if ((pEntryTag[LumpInx] & TAG_GFX) && pDirEnt[LumpInx].size > MAX_IMG_SIZ)
      {
	pEntryTag[LumpInx] = TAG_NONE;
	NumLargeImg++;
	continue;
      }
    
    if (pEntryTag[LumpInx] != 0)
      NumWadType[MapBitMaskToInx(pEntryTag[LumpInx])]++;
    
    if ( (pEntryTag[LumpInx] & TAG_GFX) && 
	 pDirEnt[LumpInx].size > LargestImgEntry)
      LargestImgEntry = pDirEnt[LumpInx].size;
  }

  Message("\nMUS   entries: %d\n", NumWadType[MapBitMaskToInx(TAG_MUS)]);
  Message("SFX   entries: %d\n", NumWadType[MapBitMaskToInx(TAG_SFX)]);
  Message("FLAT  entries: %d\n", NumWadType[MapBitMaskToInx(TAG_FLAT)]);
  Message("FULL  entries: %d\n", NumWadType[MapBitMaskToInx(TAG_FULL)]);
  Message("IMAGE entries: %d\n", NumWadType[MapBitMaskToInx(TAG_IMG)]);
  Message("HIRES entries: %d\n", NumWadType[MapBitMaskToInx(TAG_HIRES)]);

  if ( !UsefulEntries )
    Abort("There are no MUS, SFX, FLAT, FULL, IMAGE or HIRES" 
	  "entries in this wad");


  if (LargestImgEntry != 0) {
    if ((pBitMap = calloc(LargestImgEntry, sizeof(char))) == NULL)
      Abort("error, no memory for image buffer");
  }
  return UsefulEntries;
}





/* Just a collection of routines to set fontsize, fontstyle, etc. in
    the objects on the form. */

void set_object_defaults(void) 
{
  fl_set_browser_fontsize(wad_brow,FL_NORMAL_SIZE);
  fl_set_browser_fontstyle(wad_brow,FL_FIXEDBOLD_STYLE);

  fl_set_browser_fontsize(mesg_brow,FL_NORMAL_SIZE);
  fl_set_browser_fontstyle(mesg_brow,FL_FIXEDBOLD_STYLE);

#ifdef AUDIO_DEVICE
  fl_addto_choice(snd_cho, INTERNALLY_STR );
#else
  /* just hide the revers-button if we can't play internally */
  fl_hide_object( rev_but );
#endif
  fl_addto_choice(snd_cho, EXTERNALLY_STR );

  fl_set_choice_fontsize(snd_cho,FL_NORMAL_SIZE);
  fl_set_choice_fontstyle(snd_cho,FL_BOLD_STYLE);
  
  /* luckily, ANSI allows compile-time string concatenation: */
  fl_addto_choice(img_cho, INTERNALLY_STR "|" EXTERNALLY_STR );
  fl_set_choice_fontsize(img_cho,FL_NORMAL_SIZE);
  fl_set_choice_fontstyle(img_cho,FL_BOLD_STYLE);

  if ( pPalFileSpec )  /* if the user supplied a palette... */
    fl_addto_choice(pal_type_cho, PAL_USER);
    /* (if the palette can't be loaded from file, the program 
       will die anyway...) */
  fl_addto_choice(pal_type_cho, PAL_DOOM "|" PAL_HERETIC "|" 
		  PAL_E_HERETIC "|" PAL_HEXEN "|" PAL_STRIFE );

  fl_set_choice_fontsize(pal_type_cho,FL_NORMAL_SIZE);
  fl_set_choice_fontstyle(pal_type_cho,FL_BOLD_STYLE);

  fl_addto_choice(exp_img_fmt_cho, XPM_STR "|" PCX_STR );
  fl_set_choice_fontsize(exp_img_fmt_cho,FL_NORMAL_SIZE);
  fl_set_choice_fontstyle(exp_img_fmt_cho,FL_BOLD_STYLE);

  fl_setpup_fontsize(FL_NORMAL_SIZE);
  fl_setpup_fontstyle(FL_BOLD_STYLE);
}


int main(int argc, char *argv[])
{
  Int4 entries;
  FL_IOPT opt;

  CheckHost();

  /* There is a conflict between forms and xew about the meaning of
     the arguments: The "-p" option is taken as "-privat" by
     fl_initialize(), forcing a private colourmap.  This is what we
     want anyway, so ok... :-).  We just call GetArgs() first, because
     fl_initialize() removes the "-p" from argv[] ....  */

  GetArgs(argc, argv);

  /* Force private colourmap (why do I need opt?) */
  fl_set_defaults( FL_PDPrivateMap, &opt );

  fl_initialize(&argc, argv, "XEw", NULL, 0);

  /* Set up our signal handlers */
  signal(SIGINT, SigHandler);
  signal(SIGTERM, SigHandler);

  create_the_forms();
  set_object_defaults();
  /* load logo  */
  fl_set_pixmap_data(img_pxm, xew_logo_xpm);

  fl_show_form(xew_mainf,FL_PLACE_MOUSE, FL_FULLBORDER, "xew");

  entries = ReadFiles();
  FillBrowser( entries );

  fl_do_forms();

  return EXIT_FAILURE ;   /* shouldn't be reached */

}

