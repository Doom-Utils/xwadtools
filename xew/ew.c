/* ew.c *************************************** updated: 1996-12-18 15:44 TT
 *
 * Description : Examine Wad...
 *               Export sound effects (wav), music (mus) and images (pcx)
 *               from Doom, Doom II, Heretic, Hexen, Strife or any other wad.
 *               The DOS version can display the images as well...
 * Author      : Tormod Tjaberg (tormod@sn.no)
 *               UNIX/Forms stuff by Hans Peter Verne (hpv@kjemi.uio.no)
 * Credits     : ID Software for creating DOOM
 *               Matt Fell for the undocumented DOOM specs
 *
 *
 * Copyright (C) 1996 Tormod Tjaberg
 *
 * This is free software, distributed under the terms of the
 * GNU General Public License. For details see the file COPYING
 *
 * Five defines: MS_DOS - actually the default
 *               PHARLAP- use the PharLap dos extender
 *               UNIX   - compile for unix.
 *               WITH_XFORMS - Use the XForms library (UNIX)
 *               NCURSE - Uses ncurses under UNIX, no effect under MS_DOS
 *               CURSE  - Uses curses under UNIX, no effect under MS_DOS
 *               DOT_INI- The ini file name becomes: ".ewini" instead
 *                        of "ew.ini", no effect under MS_DOS
 *
 * I've compiled ew with the DOS Extender in Zortech C/C++ ver 3.10r2:
 *    ztc -mxi -3 ew.c
 *
 * For the UNIX-versions, see the file unix.txt and Makefile.unix
 *
 * Tested with:
 * MS_DOS: Zortech C/C++ (3.02 & 3.10), Symantec C/C++ (7.2), Borland C (3.0)
 *         Watcom 10.0,10.5 & 10.6 (switch -x), DJGPP 2.0
 * UNIX  : gcc on a Linux box, DECstation and DecAlpha
 *
 * Changes:
 * 990120 : 1.09 : Added option -t to keep color 0 from the Doom palette
 *		   unmodified when exporting into XPM image files. (UM)
 * 961218 : 1.08 : Fixed sound sample length bug (Ta HPV). Turns out that
 *                 people are storing a long (4 bytes) in the nSample field
 *                 after all. 
 * 961205 : 1.07 : Forms version for Unix, some extra #defines included.
 * 961024 : 1.06 : ew now closes files and deallocates buffers in a nice way
 * 960830 : 1.05 : Added compilation under PharLap.
 *                 Define PHARLAP on the command line, had to supply my
 *                 own functions...
 * 960704 : 1.04 : Added support for DJGPP & created a header file
 * 960607 : 1.03 : Added support for Watcom (large model, DOS4G and pmode)
 * 960202 : 1.02 : If no palette file is specified and the WAD contains
 *                 other entries all images are ignored but you may
 *                 still export and play sound files.
 * 960129 : 1.01 : Now ew just ignores images that are too large for
 *                 the current memory model. Added the DOT_INI define.
 * 960125 : 1.00 : Tidied up some documentation. Fixed HOME stuff.
 * 960120 : 0.50 : UNIX'ified the code. Escape in the graphics screen
 *                 just returns you to the text screen.
 *                 Added signal support. Note, signal support under real
 *                 mode MS_DOS is a bit dodgy.
 * 960115 : 0.12 : Now displays keys available +++
 * 960114 : 0.11 : Added numerical keypad support
 * 960109 : 0.10 : All export code seems to work...
 * 960108 : 0.09 : Ini file and system spawn complete...
 * 960102 : 0.08 : Tidied up some code made hires images work.
 * 960101 : 0.07 : Redid the wad id code it's now a bitmask. Avoids checking
 *                 on image details twice. Added image type info to the list.
 * 951231 : 0.06 : Ambiguity when flat is the same size of an image, fixed
 *                 Now runs under Windows (3.11) as well. Zero base ptr changed.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "ew.h"

#define PROGRAM_NAME "ew"
#define PROGRAM_VER  "1.09"

short OrgVidMode = 3;                     /* An estimated guess */
char DAC_Reg[256 * 3];                    /* 256 RGB triplets */
char fGrafixPresent = 0;                  /* Is a graphics adapter present */
char fGrafixMode = 0;                     /* Are we in graphics mode */
char TmpNameStr[FILENAME_MAX];            /* Contains the temp file name */
char ExeStr[FILENAME_MAX];                /* Contains the executable + args */
char Ext_WAV_Player[FILENAME_MAX] = "";   /* filespec to external program */
char Ext_MUS_Player[FILENAME_MAX] = "";   /* filespec to external program */
char Ext_PCX_Viewer[FILENAME_MAX] = "";   /* filespec to external program */

int keep_transparent_color;		  /* keep color 0 from Doom pal */

#if defined(UNIX) && defined(DOT_INI)
char IniFileName[] = ".ewini";     /* For people who like a tidy HOME :-) */
#else
char IniFileName[] = "ew.ini";     /* The ini file name */
#endif

unsigned char *pBitMap = NULL;     /* The raw image buffer */
char *pWadFileSpec = NULL;
char *pPalFileSpec = NULL;
PCXHDR pcxHdr;

/* Global variables
 */
#if defined(DOS386) || defined(UNIX) || defined(__386__) || defined(__DJGPP__)
unsigned char ImgBuf[SCR_WIDTH * SCR_HEIGHT];
#else
#ifdef __LARGE__
unsigned char far ImgBuf[SCR_WIDTH * SCR_HEIGHT];
#else
#error Large model, DOS Extender or UNIX required
#endif
#endif

char *Usage[] =
{
#ifdef MS_DOS
   PROGRAM_NAME " (DOS), ver " PROGRAM_VER " (" __DATE__ ") - (C) 1996 Tormod Tjaberg (tormod@sn.no)",
#else
   PROGRAM_NAME " (UNIX), ver " PROGRAM_VER " (" __DATE__ ") - (C) 1996 Tormod Tjaberg (tormod@sn.no)",
#ifdef WITH_XFORMS
   "Forms version (C) 1996 Hans Peter Verne (hpv@kjemi.uio.no)",
#endif
#endif
   "",
   "Usage: " PROGRAM_NAME " [-t] [-p pcx file] <wad/pwad file>",
   NULL
};


/* Abort()
 *
 * Input  : Message to be displayed with the format of standard printf!
 * Returns: Nothing, but instead it exits to operating system
 *
 * Terminate program, return to OS with a suitable return value.
 * exit() will close all open files.
 */
void Abort(char *pMessage, ...)
{
   va_list argp;

   fprintf(stderr, "\n" PROGRAM_NAME ": ");
   va_start(argp, pMessage);
   vfprintf(stderr, pMessage, argp);
   va_end(argp);
   fprintf(stderr, "\n");

#if (defined(CURSE) ^ defined(NCURSE))
   endwin();
#endif

   exit(EXIT_FAILURE);  /* exit & tell operating system that we've failed */
}

/* Load ini file
 */

/* Macro to determine if char is a path or drive delimiter */
#define IS_PATH_DELIM(c)   ((c) == '\\' || (c) == ':' || (c) == '/')

#ifdef MS_DOS
#define PATH_DELIM   ';'
#define SEP_CHARS    ";"
#else  /* It's UNIX */
#define PATH_DELIM   ':'
#define SEP_CHARS    ":"
#endif


/* This function is based on the nifty pfopen function by David Engel
 * from Bob Stout's snippets archive.
 *
 * I've modified it slightly so that it doesn't open a file but instead
 * returns the complete filespec or NULL if it can't be found
 */
char *FindFile(char *name, char *dirs)
{
   char *ptr;
   char *tdirs;
   FILE *file = NULL;
   size_t len;
   static char work[FILENAME_MAX];

   if (dirs == NULL || dirs[0] == '\0')
      return NULL;

   if ((tdirs = malloc(strlen(dirs) + 1)) == NULL)
      return NULL;

   strcpy(tdirs, dirs);

   for (ptr = strtok(tdirs, SEP_CHARS); file == NULL && ptr != NULL;
       ptr = strtok(NULL, SEP_CHARS))
   {
      strcpy(work, ptr);
      len = strlen(work);
      if (len && work[len - 1] != '/' && work[len - 1] != '\\')
         strcat(work, "/");
      strcat(work, name);

      file = fopen(work, "r");
   }
   free(tdirs);

   if (file == NULL)
      return NULL;

   fclose(file);
   return work;
}


/* Create a list of directories separated by PATH_DELIM suitable
 * for use with FindFile above...
 *
 * Order of search under MS_DOS is:
 * . Current dir
 * . Execution path
 * . PATH variable
 *
 * Under UNIX we just look in HOME
 *
 * Returns NULL if we can't find HOME under UNIX
 *
 * Note: Very little buffer overflow detection is done here...
 */
char *Get_pSearchList(char *pFirstArg)
{
   char *pEnv;
   static char SearchBuf[FILENAME_MAX * 2];
#ifdef MS_DOS
   char *p;
   char *pSearchBuf;
#endif


#ifdef UNIX
   /* Under UNIX we must not allow system to be called from any other
    * directory than that of the users. Only search for ew.ini in HOME (hpv)
    */
   if ((pEnv = getenv("HOME")) != NULL)
   {
      strcpy(SearchBuf, pEnv);
      return SearchBuf;
   }
   else
      return NULL;
#else
   /* Start in the current directory */
   SearchBuf[0] = '.';
   SearchBuf[1] = PATH_DELIM;
   SearchBuf[2] = '\0';
   pSearchBuf = &SearchBuf[2];

   if (pFirstArg != NULL)
   {
      strcpy(pSearchBuf, pFirstArg);
      /* Start at end of string and back up till we find the beginning
       * of the filename or a path.
       */
      for (p = pSearchBuf + strlen(pSearchBuf);
           p != pSearchBuf && !IS_PATH_DELIM(*(p - 1));
           p--)
         ;

      *p = PATH_DELIM;
      *(p + 1) = '\0';
   }

   if ((pEnv = getenv("PATH")) != NULL)
      strcat(pSearchBuf, pEnv);

   return SearchBuf;
#endif
}


/* VerifyBuffer()
 *
 * Input    : pointer to buffer in question
 * Returns  : TRUE if this is a valid line to parse, FALSE if not
 *
 * Verify that this is a valid line to parse and not just
 * whitespace or a comment.
 */
char VerifyBuffer(char *pBuffer)
{
   for (; isspace(*pBuffer) && *pBuffer != '\0'; pBuffer++)
      ;

   /* if we have only found spaces and then a ';' ignore this line
    * but if we're at the end of the string we've found nothing
    */
   return (*pBuffer == ';' || *pBuffer == '\0') ? 0 : 1;
}


/* Read external programs into our strings
 *
 * The INI file layout is very simple:
 * ';' in the first column means a comment
 * WAV external command %s
 * MUS external command %s
 * PCX external command %s
 *
 * The %s is where we insert our filename
 */
void ReadIniFile(char *pFileName)
{
   FILE *fp;
   static char InputBuf[200];
   static char TypeStr[6];
   static char FileSpec[200];       /* A calculated guess.. */

   if ((fp = fopen(pFileName, "r")) == NULL)
      Abort("error: unable to open input file \"%s\" in 'r' mode", pFileName);

   while (fgets(InputBuf, 200, fp) != NULL)
   {
      /* verify that the line is not a comment or just whitespace
       */
      if (!(VerifyBuffer(InputBuf)))
         continue;

      if (sscanf(InputBuf, "%5s %200[^\n]", TypeStr, FileSpec) != 2)
         Abort("lexical error: illegal number of tokens in entry string:\n%s", InputBuf);

      /* To avoid embarrassment */
      FileSpec[sizeof(FileSpec) - 1] = '\0';

      if (memcmp(TypeStr, "PCX", 3) == 0)
         strcpy(Ext_PCX_Viewer, FileSpec);
      else
      {
         if (memcmp(TypeStr, "WAV", 3) == 0)
            strcpy(Ext_WAV_Player, FileSpec);
         else
         {
            if (memcmp(TypeStr, "MUS", 3) == 0)
               strcpy(Ext_MUS_Player, FileSpec);
            else
               Abort("invalid type \"%s\"", TypeStr);
         }
      }
   }
   fclose(fp);
}


/* Pretty obvious really */
void Beep(void)
{
   fputc('\07', stderr);
   fflush(stderr);
}


#ifndef WITH_XFORMS
int GetKey(void)
{
#ifdef MS_DOS
   short Key;
   Key = getch();
   /* Shift it up so we can distinguish cursor keys... */
   if (Key == 0)
      Key = (getch() << 8);
   return Key;
#else
   return getch();
#endif
}
#endif


/* Video mode functions
 */

/* To determine whether we have a VGA or NOT
 * Try to call VGA Identify Adapter Function
 */
char fGrafixAdapter(void)
{
#ifdef MS_DOS
   union REGS r;

   r.h.ah = 0x1a;
   r.h.al = 0;

   int86(0x10, &r, &r);

   return (r.h.al == 0x1a) ? 1 : 0;
#else
   return 0;
#endif
}


short GetVideoMode(void)
{
#ifdef MS_DOS
   union REGS r;

   WREGS(r, ax) = 0x0f00;
   int86(0x10, &r, &r);
   return r.h.al;
#else
   return 0;
#endif
}


void VideoMode_19(void)
{
#ifdef MS_DOS
   union REGS r;

   if (GetVideoMode() == 19)
      return;

   WREGS(r, ax) = 19;
   int86(0x10, &r, &r);
#endif
}


void VideoMode_18(void)
{
#ifdef MS_DOS
   union REGS r;

   if (GetVideoMode() == 18)
      return;

   WREGS(r, ax) = 18;
   int86(0x10, &r, &r);
#endif
}


void SaveVideoMode(void)
{
#ifdef MS_DOS
   OrgVidMode = GetVideoMode();
#endif
}


void RestoreVideoMode(void)
{
#ifdef MS_DOS
   union REGS r;

   if (OrgVidMode == GetVideoMode())
      return;

   WREGS(r, ax) = OrgVidMode;
   int86(0x10, &r, &r);
#endif
}


/* PCX stuff
 */

/* Input  : byt - byte to analyse
 *          cnt - current count (RL)
 *          fp - file to write to
 * Returns: count of bytes written, 0 if error
 *
 * Subroutine for writing an encoded byte pair
 * (or single byte  if it doesn't encode) to a file.
 *
 * Taken directly from ZSoft documentation (renamed 'this' to 'current' C++)
 */
short encput(unsigned char byt, unsigned char cnt, FILE *fid)
{
   if (cnt)
   {
      if ((cnt == 1) && (0xc0 != (0xc0 & byt)))
      {
         if (EOF == putc((int) byt, fid))
            return (0);/* disk write error (probably full) */
         return (1);
      }
      else
      {
         if (EOF == putc((int) 0xC0 | cnt, fid))
            return (0); /* disk write error */
         if (EOF == putc((int) byt, fid))
            return (0); /* disk write error */
         return (2);
      }
   }
   return 0;
}


/* Input  : inBuff - pointer to scanline data
 *          inLen - length of raw scanline in bytes
 * Returns: number of bytes written into outBuff, 0 if failed
 *
 * This subroutine encodes one scanline and writes it to a file
 *
 * Taken directly from ZSoft documentation
 */
short encLine(unsigned char *inBuff, short inLen, FILE *fp)
{
   unsigned char current;
   unsigned char last;
   unsigned char runCount; /* max single runlength is 63 */
   short srcIndex;
   short i;
   short total;

   total = 0;
   last = *(inBuff);
   runCount = 1;

   for (srcIndex = 1; srcIndex < inLen; srcIndex++)
   {
      current = *(++inBuff);
      if (current == last)
      {
         runCount++;                /* it encodes */
         if (runCount == 63)
         {
            if ((i = encput(last, runCount, fp)) == 0)
               return (0);
            total += i;
            runCount = 0;
         }
      }
      else
      {   /* current != last */
         if (runCount)
         {
            if ((i = encput(last, runCount, fp)) == 0)
               return (0);
            total += i;
         }
         last = current;
         runCount = 1;
      }
   } /* endloop */

   if (runCount)
   {  /* finish up */
      if ((i = encput(last, runCount, fp)) == 0)
         return (0);
      return (total + i);
   }

   return (total);
}


/* Input  : directory entry
 *          file pointer in wad
 *          width of image
 *          height of image
 *          pointer to an image buffer, the use varies see below
 *          number of planes in the image
 *
 * Returns: 0 - OK
 *          else error
 *
 * Loosly based on the pcxdump code in the RTA840
 */
short PCX_WriteImage(Directory DirEntry, FILE *fp, short hres, short vres,
                     unsigned char *pImgBuf, short nplanes, char *pFileName)
{
   short nrows;
   short row;
   short plane;
   short i;
   Int4 imgsiz;
   FILE *fpOut;
   unsigned char *pPlane;

   if ((fpOut = fopen(pFileName, "wb")) == NULL)
      return 2;

   memset(&pcxHdr, 0, sizeof(PCXHDR));

   /* x1 and y1 are already 0 */

   pcxHdr.x2 = hres - 1;
   pcxHdr.y2 = vres - 1;

   pcxHdr.maker = 10;
   pcxHdr.version = 5;
   pcxHdr.code = 1;
   pcxHdr.nplanes = nplanes;
   pcxHdr.palinfo = 1;
   pcxHdr.hres = hres;
   pcxHdr.vres = vres;

   /* The following is plane dependant */
   switch (pcxHdr.nplanes)
   {
      case 1:   /* 256 colour image */
         pcxHdr.bpp = 8;
         pcxHdr.bpl = hres;
         break;
      case 4:   /* 16 colour image */
         pcxHdr.bpp = 1;
         pcxHdr.bpl = hres / 8;

         fseek(fp, DirEntry.start, SEEK_SET);

         /* Get the palette from file */
         fread(pcxHdr.palette, sizeof(char), 3 * 16, fp);

         /* Boost the palette */
         for (i = 0; i < 16 * 3; i++)
            pcxHdr.palette[i] = pcxHdr.palette[i] * 4;
         break;
      default:
         return 3;
         /* NOTREACHED */
         break;
   }

   if (fwrite(&pcxHdr, sizeof(PCXHDR), 1, fpOut) != 1)
      return 1;

   nrows = pcxHdr.y2 - pcxHdr.y1 + 1;

   switch (pcxHdr.nplanes)
   {
      case 1:   /* 256 colour image */
         /* Here we assume the data is in the pImgBuf */
         for (row = 0; row < nrows; row++)
         {
            pPlane = pImgBuf + (row * pcxHdr.bpl);
            if (!encLine(pPlane, pcxHdr.bpl, fpOut))
               return 1;
         }

         fputc(12, fpOut);
         if (fwrite(DAC_Reg, sizeof(char), 3 * 256, fpOut) != 3 * 256)
            return 1;

         break;
      case 4:   /* 16 colour image */
         /* Here we assume that pImgBuf points to a large buffer */
         imgsiz = (Int4) pcxHdr.bpl * (Int4) vres * (Int4) pcxHdr.nplanes;

         if (fread(pImgBuf, sizeof(char), imgsiz, fp) != imgsiz)
            return 1;

         pPlane = pImgBuf;
         for (row = 0; row < nrows; row++)
         {
            for (plane = 0; plane < pcxHdr.nplanes; plane++)
            {
               pPlane = (pImgBuf + (plane * pcxHdr.bpl * vres)) + (row * pcxHdr.bpl);
               if (!encLine(pPlane, pcxHdr.bpl, fpOut))
                  return 1;
            }
         }
         break;
      default:
         break;
   }

   fclose(fpOut);
   return 0;
}


void ReadPal(char *pFileSpec, char *RGB)
{
   int i;
   FILE *fp;

   if ((fp = fopen(pFileSpec, "rb")) == NULL)
      Abort("error, opening file \"%s\"", pFileSpec);

   if (fread(&pcxHdr, sizeof(PCXHDR), 1, fp) != 1)
      Abort("error, reading file \"%s\"", pFileSpec);

   if (pcxHdr.maker != 10)
      Abort("error, the file \"%s\" is not a pcx file", pFileSpec);

   if (pcxHdr.version < 5)
      Abort("error, incorrect PCX version need 5 or higher");

   if (pcxHdr.bpp != 8)
      Abort("error, need a 256 colour image");

   fseek(fp, -769, SEEK_END);  /* wind to start of pallette */

   i = fgetc(fp);

   if (i != 12)
      Abort("error, unable to find the start of the palette information");

   fread(RGB, sizeof(char), 3 * 256, fp);
   fclose(fp);
}


void SetPal(void)
{
#ifdef MS_DOS
   short i;

   for (i = 0; i < 256; i++)
   {
      outp(0x3c8, i);
      outp(0x3c9, DAC_Reg[i * 3 + 0] / 4);
      outp(0x3c9, DAC_Reg[i * 3 + 1] / 4);
      outp(0x3c9, DAC_Reg[i * 3 + 2] / 4);
   }
#endif
}


/* Take a possible wad image entry and try to parse it into
 * a display buffer. Each pixel occupies one byte in the buffer
 * This code is only for 256 colour images
 *
 * For TAG_IMG images, the offset is returned in *dpx, *dpy, if !NULL.
 */
unsigned char *ParseImgBlock(Directory DirEntry, unsigned char Tag, short *pWidth, short *pHeight, short *pdx, short *pdy, FILE *fp)
{
   short Width;
   short Height;
   short i;
   short x;
   short PostInx;
   unsigned char RowStart;
   unsigned char NumColoured;
   unsigned char *pPost;
   Int4 *pColumn;

   fseek(fp, DirEntry.start, SEEK_SET);
   fread(pBitMap, sizeof(unsigned char), DirEntry.size, fp);

   switch (Tag)
   {
      case TAG_FLAT:
         *pWidth = 64;
         *pHeight = 64;
         return pBitMap;
         /* NOTREACHED */
      case TAG_FULL:
         *pWidth = SCR_WIDTH;
         *pHeight = SCR_HEIGHT;
         return pBitMap;
         /* NOTREACHED */
      case TAG_HIRES:
         return NULL;
         /* NOTREACHED */
      case TAG_IMG:
         *pWidth = Width = *((short *) &pBitMap[0]);
         *pHeight = Height = *((short *) &pBitMap[2]);

         /* offset : */
         if (pdx) 
            *pdx = *((short *) &pBitMap[4]);
         if (pdy)
            *pdy = *((short *) &pBitMap[6]);

         /* Since we don't fill the entire area make sure the area of the new
          * image contains zero
          */
         memset(ImgBuf, 0, Width * Height);

         /* pColumn points to the entire column index
          * pPost is the pointer to the 'posts' within a column...
          */
         pColumn = (Int4 *) &pBitMap[8];

         for (x = 0; x < Width; x++)
         {
            pPost = (unsigned char *) pBitMap + pColumn[x];
            RowStart = pPost[0];
            PostInx = 1;
            while (RowStart != 255 && PostInx < 512)
            {
               NumColoured = pPost[PostInx];
               PostInx += 2;    /* for some weird reason the first pixel is not drawn */
               for (i = 0; i < NumColoured; i++)
                  ImgBuf[ (RowStart + i) * Width + x] = pPost[i + PostInx];

               PostInx += NumColoured + 1;   /* neither is the last... */
               RowStart = pPost[PostInx++];
            }
            if (RowStart != 255)
               return NULL;
         }

         return ImgBuf;
         /* NOTREACHED */
      default:
         return NULL;
   }
   /* NOTREACHED */
}


#if defined(PHARLAP)
size_t p_fread(unsigned char far *pi, size_t bytesper, size_t numitems, FILE *fp)
{
   int c;
   char far *p;
   char far *pend;
   unsigned int bytesleft;
   unsigned int u;

   p = (char far *) pi;
   bytesleft = bytesper * numitems;
   pend = p + bytesleft;

   while (p < pend)
   {
      c = fgetc(fp);
      if (c != EOF)
      {
         *p = c;
         p++;
         continue;
      }
      numitems = (p - (char *) pi) / bytesper;
      break;
   }
   return numitems;
}
#endif

#if defined(PHARLAP)
void DisplayHiRes(Directory DirEntry, unsigned char far *pVid, FILE *fp)
#else
void DisplayHiRes(Directory DirEntry, unsigned char *pVid, FILE *fp)
#endif
{
#ifdef MS_DOS
   short i;
   union REGS r;
   char  DAC_16[3 * 16];
   short PlaneMask[] = { 0x102, 0x202, 0x402, 0x802 };

   VideoMode_18();

   fseek(fp, DirEntry.start, SEEK_SET);

   /* Get the palette */
   fread(DAC_16, sizeof(char), 3 * 16, fp);

   /* Set the correct palette for VGA a bit more complicated that it seems
    * The pixel value on the screen is used as an index into a 16 bit table,
    * the register table. Each entry in this table points to a RGB entry in
    * the DAC register.
    */
   for (i = 0; i < 16; i++)
   {
      /* Determine actual DAC entry used */
      r.h.ah = 0x10;
      r.h.al = 0x07;
      r.h.bl = i;

      int86(0x10, &r, &r);

      outp(0x3c8, r.h.bh);
      outp(0x3c9, DAC_16[i * 3 + 0]);
      outp(0x3c9, DAC_16[i * 3 + 1]);
      outp(0x3c9, DAC_16[i * 3 + 2]);
   }

   for (i = 0; i < 4; i++)
   {
      /* set up for plane masking */
      outp(0x3ce, 5);
      outpw(0x3c4, PlaneMask[i]);
#if defined(PHARLAP)
      if (p_fread(pVid, sizeof(char), 38400, fp) != 38400)
#else
      if (fread(pVid, sizeof(char), 38400, fp) != 38400)
#endif
      {
         outpw(0x3c4, 0xf02);
         Beep();
         return;
      }
      /* restore plane mask */
      outpw(0x3c4, 0xf02);
   }
#endif
}


/* Create a valid file name from the wad entry. Some wads contain illegal
 * file names so we have to legalise them a bit. This is all pretty
 * DOS based. No test is made to see if the resulting filename will
 * overwrite an existing file...
 */
void GenFileName(char *pEntryName, char *pSuffix, char *pFileName)
{
   static char BadFileNameChar[] = " /,;^+[]\"=*?:.\\|<>";
   static char MapFileNameChar[] = "_1'%!@()#-$&1204{}";
   char *pBadChar;
   short i;

   memcpy(pFileName, pEntryName, 8);
   pFileName[8] = '\0';

   for (i = 0; pFileName[i] != '\0'; i++)
   {
      if ((pBadChar = strchr(BadFileNameChar, pFileName[i])) != NULL)
         pFileName[i] = MapFileNameChar[pBadChar - BadFileNameChar];
   }

   if (pSuffix != NULL)
      strcat(pFileName, pSuffix);
}


#ifdef UNIX
/* Determine the byte ordering */
char fLowEndianMachine(void)
{
   short i = 0x0001;
   char *p = (char *) &i;

   return *p;
}
#endif


/* Parse the command line and extract a pcx file name (to get a pallette)
 * and a wad name
 */
void GetArgs(int argc, char *argv[])
{
   short i;           /* counter */
   pWadFileSpec = NULL;
   pPalFileSpec = NULL;

   if (argc > 1)
   {
      for (i = 1; i < argc; i++)
      {
         if (argv[i][0] != '-')
         {
            pWadFileSpec = argv[i];
            continue;
         }

         switch (argv[i][1])
         {
	    case 't':
		keep_transparent_color++;
		break;
            case 'p':
               if (argv[i][2] != '\0')
                  pPalFileSpec = &argv[i][2];   /* filespec follows p */
               else
               {
                  if (argv[i + 1] != NULL)
                  {
                     pPalFileSpec = argv[i + 1];
                     i++;
                  }
               }
               break;
            default:
               Abort("command error, invalid option '%c'", argv[i][1]);
               break;
         }
      }
   }
   else
   {
      i = 0;
      while (Usage[i] != NULL)
         puts(Usage[i++]);
      exit(EXIT_FAILURE);
   }

   if (pWadFileSpec == NULL)
      Abort("command error, missing wad file name");
}


/* Write an unsigned long to file
 */
void wulong(FILE *fp, Uint4 ul)
{
   if (fwrite(&ul, sizeof(Uint4), 1, fp) != 1)
      Abort("error writing to output file");
}


/* Write an unsigned short to file
 */
void wushort(FILE *fp, unsigned short us)
{
   if (fwrite(&us, sizeof(unsigned short), 1, fp) != 1)
      Abort("error writing to output file");
}

/* Write a WAV file
 *
 * Returns
 * 0 - OK
 * else error
 */
char Gen_WAV_File(FILE *fp, Directory DirEntry, char *pFileName)
{
   Uint4 nSamples;
   Uint4 j;
   FILE *fpOut;

   if ((fpOut = fopen(pFileName, "wb")) == NULL)
   {
#ifdef WITH_XFORMS
      Message("unable to open output file '%s'\n", pFileName);
#else
      printf("\nunable to open output file '%s'", pFileName);
      return 1;
#endif
   }

   /* All WAV header stuff is a bit hardwired see the RIFF format
    * for details on the WAV file
    */

   /* Add 4 to skip the header */
   fseek(fp, DirEntry.start + 4, SEEK_SET);

   /* Next long (Uint4) is the samples */
   fread(&nSamples, sizeof(Uint4), 1, fp);

   fputs("RIFF", fpOut);
   wulong(fpOut, (Uint4) nSamples + 8 + 16 + 12); /* + header size */
   fputs("WAVE", fpOut);
   fputs("fmt ", fpOut);
   wulong(fpOut,  16);            /* fmt chunk size */
   wushort(fpOut, 1);             /* WAVE_FORMAT_PCM */
   wushort(fpOut, 1);             /* 1 channel only */
   wulong(fpOut,  11025);         /* SamplesPerSec */
   wulong(fpOut,  (Uint4) (11025L * 1 * CHAR_BIT + 7) / CHAR_BIT); /* nBlockAlign */
   wushort(fpOut, (1 * CHAR_BIT + 7) / CHAR_BIT);
   wushort(fpOut, CHAR_BIT);      /* BitsPerSample 8 of course */
   fputs("data", fpOut);
   wulong(fpOut, (Uint4) nSamples);

   if (nSamples != (DirEntry.size - 8))
#ifdef WITH_XFORMS
   Message("Warning, wad error, mismatch in "
           "directory size and sample size\n");
#else
   printf("\nWarning, wad error, mismatch in directory size and data size");
#endif

   for (j = 0; j < nSamples; j++)
      fputc(fgetc(fp), fpOut);

   /* Align on short */
   if (nSamples & 1)
      fputc(0, fpOut);

   if (ferror(fp) || ferror(fpOut))
   {
      fclose(fpOut);
      perror("\nerror, ");
      remove(pFileName);
      return 2;
   }

   fclose(fpOut);
   return 0;
}

/* Write a MUS file
 *
 * Returns
 * 0 - OK
 * else error
 */
char Gen_MUS_File(FILE *fp, Directory DirEntry, char *pFileName)
{
   unsigned short j;
   FILE *fpOut;

   if ((fpOut = fopen(pFileName, "wb")) == NULL)
   {
#ifdef WITH_XFORMS
      Message("unable to open output file '%s'\n", pFileName);
#else
      printf("\nunable to open output file '%s'", pFileName);
#endif
      return 1;
   }

   fseek(fp, DirEntry.start, SEEK_SET);

   for (j = 0; j < DirEntry.size; j++)
      fputc(fgetc(fp), fpOut);

   if (ferror(fp) || ferror(fpOut))
   {
      fclose(fpOut);
      perror("\nerror, ");
      remove(pFileName);
      return 2;
   }

   fclose(fpOut);

   return 0;
}


/* This function maps the TAG bitmask to a number suitable for array
 * indexing. To make this work the bitmasks must be mutully exclusive.
 * If anyone has a better way of doing this "normalization" please mail
 * me. We learn as long as we live...
 */
unsigned char MapBitMaskToInx(unsigned char BitMask)
{
   switch (BitMask)
   {
      case TAG_FLAT: return 0;
      case TAG_FULL: return 1;
      case TAG_IMG: return 2;
      case TAG_HIRES: return 3;
      case TAG_MUS: return 4;
      case TAG_SFX: return 5;
      default:
         break;
   }
   return 0;
}


/* Traverse the wad directory and set the appropriate tag for each entry
 */
void BuildWadDir(FILE *fp, Directory *pDirEnt, unsigned char *pEntryTag, Int4 NumOfLumps)
{
   short x;
   Int4 j;
   Int4 LumpInx;
   char Buf[10];

   /* These are used when analysing a possible image */
   short Width;
   short Height;
   short LeftOfs;
   short TopOfs;
   Int4  Col1;
   Int4  Col2;

#ifdef WITH_XFORMS
   Message("Building wad directory, please wait...\n");
#else
   printf("Building wad directory ");
#endif

   /* Make some sort of pacifier */
   x = NumOfLumps > 40 ? 40 : NumOfLumps;
   j = (NumOfLumps / x) + 1;

   for (LumpInx = 0; LumpInx < NumOfLumps; LumpInx++)
   {
      if (LumpInx % j == 0)
      {
#ifdef WITH_XFORMS
         /* show nice spinning wheel */
         SpinnWheel(0);
#else
         putchar('.');
         fflush(stdout);
#endif
      }

      if (pDirEnt[LumpInx].size == 0)
         continue;

      fseek(fp, pDirEnt[LumpInx].start, SEEK_SET);
      fread(Buf, sizeof(char), 4, fp);

      switch (Buf[0])
      {
         case 'M':
            if (memcmp(Buf, "MUS\032", 4) == 0)
               pEntryTag[LumpInx] |= TAG_MUS;
            break;
         case '\03':
            if (memcmp(Buf, "\03\00\21\53", 4) == 0)
               pEntryTag[LumpInx] |= TAG_SFX;
            break;
         default:
            Width = *((short *) &Buf[0]);
            Height = *((short *) &Buf[2]);
            fread(&LeftOfs, sizeof(short), 1, fp);
            fread(&TopOfs, sizeof(short), 1, fp);

            if (Width > 0 && Width <= 320
               && Height > 0 && Height <= 200
               && (LeftOfs <= 320 && LeftOfs >= -320)
               && (TopOfs <= 200 && TopOfs >= -200))
            {
               /* See if the two first columns are OK */
               fread(&Col1, sizeof(Int4), 1, fp);
               fread(&Col2, sizeof(Int4), 1, fp);

               if (!((Col1 < 16 || Col1 > pDirEnt[LumpInx].size) ||
                   (Col2 < 16 || Col2 > pDirEnt[LumpInx].size)))
               {
                  /* Looks like it's an image */
                  pEntryTag[LumpInx] |= TAG_IMG;
                  break;
               }
            }

            if (pDirEnt[LumpInx].size == 64000)
            {
               pEntryTag[LumpInx] |= TAG_FULL;
               break;
            }

            if (pDirEnt[LumpInx].size == 4096)
            {
               pEntryTag[LumpInx] |= TAG_FLAT;
               break;
            }

            /* Hexen startup screen 640*480 16 bit VGA with palette */
            if (pDirEnt[LumpInx].size == 153648)
            {
               pEntryTag[LumpInx] |= TAG_HIRES;
               break;
            }

            break;
      }
   }
#ifdef WITH_XFORMS
   /* stop the spinning wheel */
   SpinnWheel(1);
#endif
}


/* This is for the signal handlers */
void SigHandler(int sig)
{
   signal(SIGINT, SIG_IGN);
   signal(SIGTERM, SIG_IGN);

#if (defined(CURSE) ^ defined(NCURSE))
   endwin();
#else
   if (fGrafixPresent)
      RestoreVideoMode();
#endif

   printf("\n%s: Received signal number %d\n", PROGRAM_NAME, sig);
   exit(EXIT_FAILURE);  /* exit & tell operating system that we've failed */
}


#if defined(PHARLAP)
/* These functions have to be far */
void p_memcpy(unsigned char far *dest, unsigned char *source, size_t len)
{
   if (len == 0)
      return;
   do 
   {
      *dest++ = *source++;
   } while (--len != 0);
}

void p_memset(unsigned char far *dest, int c, size_t len)
{
   if (len == 0) return;
   do 
   {
      *dest++ = c;  /* ??? to be unrolled */
   } while (--len != 0);
}
#endif



#ifdef UNIX      /* No use having these tests under MS_DOS */
void CheckHost()
{
   if (sizeof(Int4) != 4)
      Abort("This code relies on that sizeof(Int4) == 4,\n"
        "change the typedef of Int4 in the source...\n");

   if (!fLowEndianMachine())
      Abort("this code only works on a little endian machine");
}
#endif

/* For the Xforms-version, we have an new and better main :-) */
#ifndef WITH_XFORMS

int main(int argc, char *argv[])
{
   int Key;
   FILE *fp;
   Int4 NumOfLumps;
   Int4 NumLargeImg;
   Int4 LumpInx;
   Int4 DirOfs;
   Int4 PageDown = 12;
   Directory *pDirEnt;
   short i;
   short RetVal;
   short Width;
   short Height;
   char Buf[10];
   char *pStr;
   unsigned char *pImgBuf;
   unsigned char *pEntryTag;
   unsigned char TagType;
#ifdef MS_DOS
#if defined(PHARLAP)
   unsigned char far *pVid = NULL;
#else
   unsigned char *pVid = NULL;
#endif
#endif

   /* Counters */
   Int4 NumWadType[NUM_TAG];
   Int4 LargestImgEntry;

#ifdef UNIX
   CheckHost();
#endif

   GetArgs(argc, argv);

#if (defined(CURSE) ^ defined(NCURSE))
   initscr();
   cbreak();
   noecho();
   keypad(stdscr, TRUE);
#endif

   /* Set up our signal handlers */
   signal(SIGINT, SigHandler);
   signal(SIGTERM, SigHandler);

#ifndef __DJGPP__
#ifdef __386__
#if defined(PHARLAP)
   pVid = (unsigned char far *) MK_FP(0x34, 0xa0000);
#else
   pVid = (unsigned char *) (0xa000 << 4);
#endif
#else
#ifdef MS_DOS
#ifndef DOS386
   pVid = MK_FP(0xa000, 0x0000);
#else
#if defined(PHARLAP)
   pVid = (unsigned char far *) MK_FP(_x386_zero_base_selector, 0xa0000);
#else
   pVid = (unsigned char *) _x386_zero_base_ptr + 0xa0000;
#endif
#endif
#endif
#endif
#else
   if (__djgpp_nearptr_enable())
      pVid = (unsigned char *) (__djgpp_conventional_base + 0xa0000);
   else
      Abort("Near pointers not available (DJGPP)");
#endif

   fGrafixPresent = fGrafixAdapter();
   printf("Graphics adapter%s found...\n", fGrafixPresent ? "" : " NOT");

   pStr = Get_pSearchList(argv[0]);
   if (pStr != NULL)
      pStr = FindFile(IniFileName, pStr);

   if (pStr != NULL)
   {
      printf("Ini file \"%s\" found\n", pStr);
      ReadIniFile(pStr);
      if (Ext_WAV_Player[0] != '\0')
         printf("WAV : \"%s\"\n", Ext_WAV_Player);
      if (Ext_MUS_Player[0] != '\0')
         printf("MUS : \"%s\"\n", Ext_MUS_Player);
      if (Ext_PCX_Viewer[0] != '\0')
         printf("PCX : \"%s\"\n", Ext_PCX_Viewer);
   }
   else
      printf("Ini file \"%s\" NOT found\n", IniFileName);

   if (pPalFileSpec != NULL)
   {
      ReadPal(pPalFileSpec, DAC_Reg);
      printf("Palette file '%s' loaded successfully\n", pPalFileSpec);
   }

   if ((fp = fopen(pWadFileSpec, "rb")) == NULL)
      Abort("unable to open input file \"%s\"", pWadFileSpec);

   fgetc(fp); /* Waste the I or P before WAD */
   fread(Buf, sizeof(char), 3, fp);
   if (memcmp(Buf, "WAD", 3) != 0)
      Abort("the file \"%s\" is not a proper wad file", pWadFileSpec);

   fread(&NumOfLumps, sizeof(Int4), 1, fp);
   fread(&DirOfs, sizeof(Int4), 1, fp);

   printf("Number of wad entries: %ld\nWad directory offset: 0x%lx\n", NumOfLumps, DirOfs);
   fseek(fp, DirOfs, SEEK_SET);

   /* Let's be real careful... */
   if ((NumOfLumps * sizeof(Directory)) > UINT_MAX)
      Abort("error, directory table too large, cannot malloc more than (%u) bytes", UINT_MAX);

   if ((pDirEnt = (Directory *) malloc(NumOfLumps * sizeof(Directory))) == NULL)
      Abort("error, no memory for wad directory table");

   if ((pEntryTag = (unsigned char *) calloc(NumOfLumps, sizeof(char))) == NULL)
      Abort("error, no memory for wad directory tag table");

   if (fread(pDirEnt, sizeof(Directory), NumOfLumps, fp) != NumOfLumps)
      Abort("error reading wad directory");

   BuildWadDir(fp, pDirEnt, pEntryTag, NumOfLumps);

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
   for (LumpInx = 0; LumpInx < NumOfLumps; LumpInx++)
   {
      /* Is this image to large for malloc ? */
      if ((pEntryTag[LumpInx] & TAG_GFX) && pDirEnt[LumpInx].size > MAX_IMG_SIZ)
      {
         pEntryTag[LumpInx] = TAG_NONE;
         NumLargeImg++;
         continue;
      }

      if (pEntryTag[LumpInx] != 0)
         NumWadType[MapBitMaskToInx(pEntryTag[LumpInx])] ++;

      if ((pEntryTag[LumpInx] & TAG_GFX) && pDirEnt[LumpInx].size > LargestImgEntry)
         LargestImgEntry = pDirEnt[LumpInx].size;
   }

   printf("\nMUS   entries: %ld\n", NumWadType[MapBitMaskToInx(TAG_MUS)]);
   printf("SFX   entries: %ld\n", NumWadType[MapBitMaskToInx(TAG_SFX)]);
   printf("FLAT  entries: %ld\n", NumWadType[MapBitMaskToInx(TAG_FLAT)]);
   printf("FULL  entries: %ld\n", NumWadType[MapBitMaskToInx(TAG_FULL)]);
   printf("IMAGE entries: %ld\n", NumWadType[MapBitMaskToInx(TAG_IMG)]);
   printf("HIRES entries: %ld\n", NumWadType[MapBitMaskToInx(TAG_HIRES)]);

   if (NumLargeImg != 0)
   {
      printf("NOTE: %ld image%s could not be handled by the current memory model\n",
              NumLargeImg, (NumLargeImg == 1) ? "" : "s");
   }

   /* There are images in the file but no palette file is available... */
   if (LargestImgEntry != 0 && pPalFileSpec == NULL)
   {
      printf("NOTE: No palette file was specified. All images ignored.\n");

      /* Disable all images in the wad directory */
      for (LumpInx = 0; LumpInx < NumOfLumps; LumpInx++)
      {
         if (pEntryTag[LumpInx] & TAG_GFX)
            pEntryTag[LumpInx] = TAG_NONE;
      }

      NumWadType[MapBitMaskToInx(TAG_FLAT)] = 0;
      NumWadType[MapBitMaskToInx(TAG_FULL)] = 0;
      NumWadType[MapBitMaskToInx(TAG_IMG)] = 0;
      NumWadType[MapBitMaskToInx(TAG_HIRES)] = 0;

      /* Now there are no images */
      LargestImgEntry = 0;
   }

   /* Do a small sanity check... */
   for (i = 0; NumWadType[i] == 0 && i < NUM_TAG; i++)
      ;

   if (i == NUM_TAG)
      Abort("There are no MUS, SFX, FLAT, FULL, IMAGE or HIRES entries in this wad");

   if (LargestImgEntry != 0)
   {
      /* There are images in the file but no palette file is available... */
      if (pPalFileSpec == NULL)
         Abort("error, a palette file is required when the wad contains images");

      if ((pBitMap = (unsigned char *) calloc(LargestImgEntry, sizeof(char))) == NULL)
         Abort("error, no memory for image buffer");
   }
   else
   {
      /* Make sure graphics are disabled */
      fGrafixPresent = 0;
      pBitMap = NULL;
   }

   printf("\nUse the cursor keys or keypad to navigate throught the wad.\n");
   printf("Hit escape to quit, '%c' to export entry", KEY_EXPORT);

   if (!(Ext_WAV_Player[0] == '\0' && Ext_MUS_Player[0] == '\0' && Ext_PCX_Viewer[0] == '\0'))
      printf(", enter to spawn external program.\n");
   else
      printf(".\n");

   if (fGrafixPresent)
   {
      SaveVideoMode();
      printf("Hit space to toggle graphics mode...\n");
   }

   /* Make sure we start in text mode */
   fGrafixMode = 0;

   if (PageDown >= NumOfLumps)
      PageDown = 1;

   /* Set key to down so we're sure we'll find something */
   Key = KEY_DOWN;
   TagType = TAG_ALL;
   pImgBuf = NULL;
   do
   {
      switch (Key)
      {
         case KEY_EXPORT: /* export, or gold hey do any Norwegians understand this ? */
            switch (pEntryTag[LumpInx])
            {
               case TAG_MUS:
                  GenFileName(pDirEnt[LumpInx].name, ".mus", TmpNameStr);
                  RetVal = Gen_MUS_File(fp, pDirEnt[LumpInx], TmpNameStr);
                  break;
               case TAG_SFX:
                  GenFileName(pDirEnt[LumpInx].name, ".wav", TmpNameStr);
                  RetVal = Gen_WAV_File(fp, pDirEnt[LumpInx], TmpNameStr);
                  break;
               case TAG_FLAT:
               case TAG_FULL:
               case TAG_IMG:
                  GenFileName(pDirEnt[LumpInx].name, ".pcx", TmpNameStr);
                  pImgBuf = ParseImgBlock(pDirEnt[LumpInx], pEntryTag[LumpInx], &Width, &Height, NULL, NULL, fp);

                  if (pImgBuf == NULL)
                     RetVal = 4;
                  else
                     RetVal = PCX_WriteImage(pDirEnt[LumpInx], fp, Width, Height, pImgBuf, 1, TmpNameStr);
                  break;
               case TAG_HIRES:
                  GenFileName(pDirEnt[LumpInx].name, ".pcx", TmpNameStr);
                  RetVal = PCX_WriteImage(pDirEnt[LumpInx], fp, 640, 480, pBitMap, 4, TmpNameStr);
                  break;
               default:
                  RetVal = 1;
                  break;
            }

            if (RetVal != 0)
            {
               if (fGrafixMode)
                  RestoreVideoMode();
               Beep();
               printf("\nerror generating external file \"%s\" (%d)", TmpNameStr, RetVal);
            }
            else
            {
               if (fGrafixMode)
                  Key = -1;     /* don't redraw image */
               else
                  printf("\nExternal file \"%s\" created OK", TmpNameStr);
            }
            break;
         case KEY_SPAWN:
            /* Must be done in text mode ... */
            if (fGrafixMode)
            {
               Key = -1;
               break;
            }

            /* Determine if we have an external program */
            switch (pEntryTag[LumpInx])
            {
               case TAG_MUS:
                  RetVal = (Ext_MUS_Player[0] != '\0') ? 0 : 1;
                  break;
               case TAG_SFX:
                  RetVal = (Ext_WAV_Player[0] != '\0') ? 0 : 1;
                  break;
               case TAG_FLAT:
               case TAG_FULL:
               case TAG_IMG:
               case TAG_HIRES:
                  RetVal = (Ext_PCX_Viewer[0] != '\0') ? 0 : 1;
                  break;
               default:
                  RetVal = 1;
                  break;
            }

            if (RetVal)
            {
               printf("\nNo external program defined for this type");
               break;
            }

            /* Assume error */
            RetVal = 1;

            if (tmpnam(TmpNameStr) != NULL)
            {
               switch (pEntryTag[LumpInx])
               {
                  case TAG_MUS:
                     if ((RetVal = Gen_MUS_File(fp, pDirEnt[LumpInx], TmpNameStr)) == 0)
                        sprintf(ExeStr, Ext_MUS_Player, TmpNameStr);
                     break;
                  case TAG_SFX:
                     if ((RetVal = Gen_WAV_File(fp, pDirEnt[LumpInx], TmpNameStr)) == 0)
                        sprintf(ExeStr, Ext_WAV_Player, TmpNameStr);
                     break;
                  case TAG_FLAT:
                  case TAG_FULL:
                  case TAG_IMG:
                     pImgBuf = ParseImgBlock(pDirEnt[LumpInx], pEntryTag[LumpInx], &Width, &Height, NULL, NULL, fp);

                     if (pImgBuf == NULL)
                        RetVal = 4;
                     else
                     {
                        if ((RetVal = PCX_WriteImage(pDirEnt[LumpInx], fp, Width, Height, pImgBuf, 1, TmpNameStr)) == 0)
                           sprintf(ExeStr, Ext_PCX_Viewer, TmpNameStr);
                     }
                     break;
                  case TAG_HIRES:
                     if ((RetVal = PCX_WriteImage(pDirEnt[LumpInx], fp, 640, 480, pBitMap, 4, TmpNameStr)) == 0)
                        sprintf(ExeStr, Ext_PCX_Viewer, TmpNameStr);
                     break;
                  default:
                     break;
               }
            }

            if (RetVal == 0)
            {
               /* Redraw the entry when we return, don't touch Key */
               printf("\nSpawning \"%s\" ...\n", ExeStr); 
               fflush(stdout);
               system(ExeStr);
               remove(TmpNameStr);
               printf("\nReturned from spawn.");
            }
            else
            {
               Beep();
               printf("\nerror generating external file \"%s\" (%d)", TmpNameStr, RetVal);
            }
            break;
         case KEY_X_UP:
         case KEY_X_DOWN:
         case KEY_UP:
         case KEY_DOWN:
            do
               LumpInx = (LumpInx + ((Key == KEY_DOWN) || (Key == KEY_X_DOWN) ? 1 : (NumOfLumps - 1))) % NumOfLumps;
            while (!(pEntryTag[LumpInx] & TagType));
            break;
         case KEY_HOME:
         case KEY_X_HOME:
            LumpInx = 0;
            while (!(pEntryTag[LumpInx] & TagType))
               LumpInx = (LumpInx + 1) % NumOfLumps;
            break;
         case KEY_END:
         case KEY_X_END:
            LumpInx = NumOfLumps - 1;
            while (!(pEntryTag[LumpInx] & TagType))
               LumpInx = (LumpInx + (NumOfLumps - 1)) % NumOfLumps;
            break;
         case KEY_PPAGE:
         case KEY_NPAGE:
         case KEY_X_PPAGE:
         case KEY_X_NPAGE:
            for (i = 0; i < PageDown; i++)
            {
               do
                  LumpInx = (LumpInx + ((Key == KEY_NPAGE) || (Key == KEY_X_NPAGE) ? 1 : (NumOfLumps - 1))) % NumOfLumps;
               while (!(pEntryTag[LumpInx] & TagType));
            }
            break;
         case KEY_SPACE:
            /* Only allowed to enter Graphics mode if we're on an image */
            if (!(pEntryTag[LumpInx] & TAG_GFX))
            {
               Key = -1;
               break;
            }

            if (fGrafixPresent)
            {
               if (fGrafixMode)
               {
                  RestoreVideoMode();
                  TagType = TAG_ALL;
               }
               else
               {
                  TagType = TAG_GFX;
                  if (pEntryTag[LumpInx] & TAG_HIRES)
                     VideoMode_18();
                  else
                     VideoMode_19();
                  SetPal();
               }
               /* toggle state */
               fGrafixMode ^= 1;
            }
            else
               Key = -1;            /* Make sure it's not registered */
            break;
         default:
            Key = -1;
            break;
      }

      if (Key != -1)
      {
         if ((pEntryTag[LumpInx] & TAG_GFX) && fGrafixMode)
         {
            if (pEntryTag[LumpInx] & TAG_HIRES)
            {
#ifdef MS_DOS
               DisplayHiRes(pDirEnt[LumpInx], pVid, fp);
#endif
            }
            else
            {
               if (GetVideoMode() != 19)
               {
                  VideoMode_19();
                  SetPal();
               }

               pImgBuf = ParseImgBlock(pDirEnt[LumpInx], pEntryTag[LumpInx], &Width, &Height, NULL, NULL, fp);

               if (pImgBuf == NULL)
                  Beep();
               else
               {
#ifdef MS_DOS
#if defined(PHARLAP)
                  /* Clear the video buffer */
                  p_memset(pVid, 0, SCR_WIDTH * SCR_HEIGHT);

                  for (i = 0; i < Height; i++)
                     p_memcpy(&pVid[i * SCR_WIDTH], &pImgBuf[i * Width], Width);
#else
                  /* Clear the video buffer */
                  memset(pVid, 0, SCR_WIDTH * SCR_HEIGHT);

                  for (i = 0; i < Height; i++)
                     memcpy(&pVid[i * SCR_WIDTH], &pImgBuf[i * Width], Width);
#endif
#endif
               }
            }
         }
         else
         {
            putchar('\n');
            for (i = 0; i < 8; i++)
            {
               if (pDirEnt[LumpInx].name[i] != 0)
                  putchar(pDirEnt[LumpInx].name[i]);
               else
                  putchar(' ');
            }

            switch (pEntryTag[LumpInx])
            {
               case TAG_FLAT: printf(" (flat)  %ld", LumpInx); 
                  break;
               case TAG_FULL: printf(" (full)  %ld", LumpInx); 
                  break;
               case TAG_HIRES: printf(" (hires) %ld", LumpInx); 
                  break;
               case TAG_IMG: printf(" (image) %ld", LumpInx); 
                  break;
               case TAG_MUS: printf(" (mus)   %ld", LumpInx); 
                  break;
               case TAG_SFX: printf(" (sfx)   %ld", LumpInx); 
                  break;

               default:
                  break;
            }
            fflush(stdout);
         }
      }

      Key = GetKey();

      if (fGrafixMode && Key == KEY_ESC)
         Key = KEY_SPACE;

   } while (Key != KEY_ESC);

   if (fGrafixPresent)
      RestoreVideoMode();

#if (defined(CURSE) ^ defined(NCURSE))
   endwin();
#endif

   /* Cleanup... */
   fclose(fp);
   free(pDirEnt);
   free(pEntryTag);

   if (pBitMap)
      free(pBitMap);

   printf("\n\nReturning to operating system...\n");

   return 0;
}

#endif /* WITH_XFORMS */
