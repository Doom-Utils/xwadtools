/*
 * WADWHAT.C - print the contents of a DOOM WAD file
 *
 * by Randall R. Spangler
 * Modified for Doom II by Gregory Kwok
 *
 * Version 1.1, released   08/28/94
 * Version 2.0, programmed 11/26/95
 * Version 2.1, updated    09/21/96
 * Version 2.2, updated    09/29/96
 * Version 2.3, updated    02/16/98
 *
 * Ported to UNIX by Udo Munk	06/12/98
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifndef UNIX
#include <dos.h>
#include <dir.h>
#endif

struct s_thing {                /* Thing data */
    short int   x, y;           /* Coordinates */
    short int   facing;         /* Facing, in degrees */
    short int   type;           /* Type of thing */
    short int   attr;           /* Thing attributes */
};
typedef struct s_thing THING;

/***************************************************************************/

char        contents[10][9] =   /* Valid sub-entry names for a level */
{"THINGS", "LINEDEFS", "SIDEDEFS", "VERTEXES", "SEGS", "SSECTORS",
"NODES", "SECTORS", "REJECT", "BLOCKMAP"};

char        itemline[] = "\t%-20s %5ld %5ld %5ld %5ld + %5d %5d %5d %5d\n";     /* Line of text for an item */

/***************************************************************************/

int         isbrief = 0;        /* If non-zero, is the skill level we're
                                 * printing brief stats on */
int         battr = 0;          /* Matching attribute byte for brief printout */

int         battrarry[5] = {0x01, 0x01, 0x02, 0x04, 0x04};      /* Attribute bytes vs.
                                                                 * skill level */

FILE       *savefile;           /* WAD file whose things we're looking at */
long        savefilepos;        /* Save position in WAD file (where we were
                                 * before we started looking at THINGS,
                                 * LINEDEFS, or SECTORS) */

long        thoff = 0, thlen = 0;       /* Position and length of thing data */
long        ldoff = 0, ldlen = 0;       /* Position and length of linedef
                                         * data */
long        seoff = 0, selen = 0;       /* Position and length of sector data */
long        rjlen = 0;          /* Length of reject data */

THING      *th;                 /* Thing data for a level */
int         numth = 0;          /* Number of things */

char        buf[256];

/***************************************************************************/

long        countth(int type, int attr)
{
    /* Returns the number of things of the specified type with at least one
     * matching attribute bit.  If the multi-player bit in <attr> is on,
     * matches either single or multi-player; else matches single-player
     * only. <attr>=(-1) matches any skill or multi-player. */

    THING      *t = th;         /* Current thing */

    int         askill = attr & 0x07;   /* Skill bits of attribute */
    int         amp = attr & 0x10;      /* Multi-player bits of attribute */

    long        n = 0;          /* Number of matches */
    int         i;

    for (i = 0; i < numth; i++, t++) {  /* Search all things */
        if (t->type != type)
            continue;                   /* Wrong object */

        if (attr < 0)
            n++;                        /* Match all objects of this type */
        else if (t->attr & askill) {
            if (amp)
                n++;                    /* Want single or multi */
            else if (!(t->attr & 0x10))
                n++;                    /* Want single only */
        }
    }

    return n;                           /* Return match count */
}

/***************************************************************************/

long       *countdiff(int type, long *dest)
{
    /* Counts the number of things of the specified type at each difficulty
     * level.  dest[0]=L12, dest[1]=L3, dest[2]=L45, dest[3]=multiplayer
     * only. Returns a pointer to the destination array, or NULL if error. */

    THING      *t = th;
    int         i;

    for (i = 0; i < 6; i++)
        dest[i] = 0;                    /* Clear destination array */

    for (i = 0; i < numth; i++) {       /* Search all things */
        if (t->type == type) {          /* Matched type */
            if (t->attr & 0x00010) {    /* Multi-player only */
                if (t->attr & 0x0001)
                    dest[3]++;
                if (t->attr & 0x0002)
                    dest[4]++;
                if (t->attr & 0x0004)
                    dest[5]++;
            } else {                    /* Single-player only */
                if (t->attr & 0x0001)
                    dest[0]++;
                if (t->attr & 0x0002)
                    dest[1]++;
                if (t->attr & 0x0004)
                    dest[2]++;
            }
        }
        t++;                            /* Advance to next thing */
    }

    for (i = 0; i < 3; i++)
        dest[i + 3] += dest[i];         /* Single-player stuff is present in
                                         * multi-player too */

    return dest;                        /* Return match count */
}

/***************************************************************************/

long       *addcountweighted(int type, long *dest, int weight)
{
    /* Similar to countdiff(), but adds <weight> times the counts vs.
     * difficulty to the destination array instead of just setting it to the
     * match counts. Returns NULL if error. */

    long        nd[6];
    int         i;

    countdiff(type, nd);                /* Count the matches */

    for (i = 0; i < 6; i++)
        dest[i] += weight * nd[i];      /* Add a weighted result */

    return dest;
}

/***************************************************************************/
/***************************************************************************/

int         loadthings(FILE * f, long offs, long len)
{
    /* Loads a map's THINGS data at offset <offs>, length <len>.  Returns
     * zero if error. */

    savefile = f;               /* Save WAD file handle */
    savefilepos = ftell(f);     /* Save position in WAD file */

    fseek(f, offs, SEEK_SET);
    th = (THING *) malloc(len);
    if (!th) {
        fprintf(stderr, "Not enough memory to hold THINGS\n");
        return 0;
    }
    fread(th, 1, len, f);
    numth = len / 10;

    return 1;                           /* Success */
}

int         freethings(void)
{
    /* Frees a map's THINGS data loaded by loadthings().  Returns zero if
     * error. */

    free(th);
    numth = 0;

    fseek(savefile, savefilepos, SEEK_SET);

    return 1;                           /* Success */
}

/***************************************************************************/

void        printitem(char *desc, long *arry, int double1)
{
    /* Prints an item's presence at all skill levels.  If double1, doubles
     * the numbers in columns 1 and 5.  */

    int         i;

    for (i = 0; i < 6; i++)
        if (arry[i])
            break;

    if (i == 6)
        return;                         /* Item is not present at any skill
                                         * level */

    printf("\t%-20.20s", desc);         /* Print description */

    for (i = 0; i < 6; i++) {
        if (i == 3)
            printf(" |");
        if (i == 0 || i == 3)
            printf(" %5ld", (double1 ? 2 * arry[i] : arry[i]));
        printf(" %5ld", arry[i]);
    }
    printf("\n");
}


/***************************************************************************/

void        prtth(int type, char *desc)
{
    /* Prints the description and number of matching things at each
     * difficulty level. */

    long        nd[6] = {0, 0, 0, 0, 0, 0};     /* Numbers at each difficulty
                                                 * level */

    countdiff(type, nd);
    printitem(desc, nd, 0);
}

/***************************************************************************/

/* If there is at least one thing with id <no>, print the description of the
 * thing and the number present at each skill level. */
#define PRINTITEM(desc,arry) printitem(desc,arry,0)
#define PRINTITEM2(desc,arry) printitem(desc,arry,1)

/* Reset an array */
#define RESET(arry) for(i=0; i<6; i++) arry[i]=0

int         countthings(void)
{
    /* Counts the things in a level's THINGS data.  Returns zero if error. */

    long        nc[6] = {0, 0, 0, 0, 0, 0};     /* Number of things counted
                                                 * at each skill level */
    long        wdam[6] = {0, 0, 0, 0, 0, 0};   /* Total weapon damage at
                                                 * each skill level */
    float       dratio[6] = {0, 0, 0, 0, 0, 0}; /* Damage ratio for
                                                 * difficulty */
    int         n, i;

    /** Player starts **/

    printf("    Play modes:\n");

    if (countth(1, -1))                 /* Find player 1 start */
        printf("\tSingle player\n");
    n = 0;

    for (i = 1; i <= 4; i++) {          /* Find player 1-4 starts */
        if (countth(i, -1))
            n++;
    }
    if (n > 1)
        printf("\tCooperative (%d player)\n", n);

    n = countth(11, -1);                /* Find deathmatch starts */
    if (n)
        printf("\tDeathmatch (%d starts)\n", n);

    /** Monsters **/

    printf("    Bosses:\n");
    prtth(3003, "Baron");
    prtth(16, "Cyberdemon");
    prtth(7, "Spiderdemon");
    prtth(88, "Final Boss");

    printf("    Monsters:\n");
    prtth(3004, "Trooper");
    prtth(9, "Sergeant");
    prtth(65, "Chaingun guy");
    prtth(84, "SS Nazi");
    prtth(3001, "Imp");
    prtth(3002, "Demon");
    prtth(58, "Spectre");
    prtth(3006, "Lost soul");
    prtth(3005, "Cacodemon");
    prtth(64, "Archvile");
    prtth(66, "Revenant");
    prtth(67, "Mancubis");
    prtth(68, "Arachnotron");
    prtth(69, "Mini Baron");
    prtth(71, "Pain Elemental");

    printf("    Weapons:\n");
    prtth(2001, "Shotgun");
    prtth(82, "Super shotgun");
    prtth(2002, "Chaingun");
    prtth(2003, "Rocket launcher");
    prtth(2004, "Plasma gun");
    prtth(2006, "BFG-9000");
    prtth(2005, "Chainsaw");

    printf("    Equipment:\n");
    prtth(8, "Backpack");
    prtth(2022, "Invulnerability");
    prtth(2023, "Berserk");
    prtth(2024, "Invisibility");
    prtth(2025, "Radiation suit");
    prtth(2026, "Computer map");
    prtth(2045, "Lite amp goggles");

    printf("    Expendibles:\n");

    RESET(nc);
    addcountweighted(2002, nc, 20);     /* Chainguns */
    addcountweighted(2007, nc, 10);     /* Clips */
    addcountweighted(2048, nc, 50);     /* Boxes of ammo */
    addcountweighted(8, nc, 10);        /* Backpacks */
    addcountweighted(3004, nc, 5);      /* Troopers */
    PRINTITEM2("Bullets", nc);
    for (i = 0; i < 6; i++)
        wdam[i] += nc[i];               /* Accumulate damage */

    RESET(nc);
    addcountweighted(2001, nc, 8);      /* Shotguns */
    addcountweighted(82, nc, 16);       /* Super shotguns */
    addcountweighted(2008, nc, 4);      /* Shells */
    addcountweighted(2049, nc, 20);     /* Boxes of shells */
    addcountweighted(8, nc, 4);         /* Backpacks */
    addcountweighted(9, nc, 4);         /* Sergeants */
    PRINTITEM2("Shells", nc);
    for (i = 0; i < 6; i++)
        wdam[i] += 7 * nc[i];           /* Accumulate damage */

    RESET(nc);
    addcountweighted(2003, nc, 2);      /* Rocket launchers */
    addcountweighted(2010, nc, 1);      /* Rockets */
    addcountweighted(2046, nc, 5);      /* Boxes of rockets */
    addcountweighted(8, nc, 1);         /* Backpacks */
    PRINTITEM2("Rockets", nc);
    for (i = 0; i < 6; i++)
        wdam[i] += 20 * nc[i];          /* Accumulate damage */

    RESET(nc);
    addcountweighted(2004, nc, 40);     /* Plasma guns */
    addcountweighted(2006, nc, 40);     /* BFG-9000's */
    addcountweighted(17, nc, 20);       /* Cell packs */
    addcountweighted(2047, nc, 100);    /* Cell charges */
    addcountweighted(8, nc, 20);        /* Backpacks */
    PRINTITEM2("Cells", nc);
    for (i = 0; i < 6; i++)
        wdam[i] += 2 * nc[i];           /* Accumulate damage */

    RESET(nc);
    addcountweighted(2018, nc, 100);    /* Armor */
    addcountweighted(2019, nc, 200);    /* Super armor */
    addcountweighted(83, nc, 200);      /* Megasphere */
    addcountweighted(2015, nc, 1);      /* Armor bonuses */
    PRINTITEM2("Armor points", nc);

    RESET(nc);
    addcountweighted(2011, nc, 10);     /* Stimpacks */
    addcountweighted(2012, nc, 25);     /* Medikits */
    addcountweighted(2013, nc, 100);    /* Soul spheres */
    addcountweighted(83, nc, 200);      /* Megasphere */
    addcountweighted(2023, nc, 100);    /* Berserk strength */
    addcountweighted(2014, nc, 1);      /* Health bonuses */
    PRINTITEM2("Health points", nc);

    prtth(2035, "Barrels");

    /* Calculate difficulty based on damage we can do vs. damage required to
     * kill all the monsters */

    printf("    Difficulty:\n");

    RESET(nc);
    addcountweighted(3004, nc, 2);      /* Troopers */
    addcountweighted(9, nc, 3);         /* Sergeants */
    addcountweighted(3001, nc, 6);      /* Imps */
    addcountweighted(3002, nc, 15);     /* Demons */
    addcountweighted(58, nc, 15);       /* Spectres */
    addcountweighted(3006, nc, 10);     /* Lost souls */
    addcountweighted(3005, nc, 40);     /* Cacodemons */
    addcountweighted(3003, nc, 100);    /* Barons */
    addcountweighted(16, nc, 400);      /* Cyberdemons */
    addcountweighted(7, nc, 300);       /* Spiderdemons */
    addcountweighted(64, nc, 70);       /* Archviles */
    addcountweighted(65, nc, 7);        /* Chaingun guys */
    addcountweighted(66, nc, 30);       /* Revenants */
    addcountweighted(67, nc, 40);       /* Mancubi */
    addcountweighted(68, nc, 50);       /* Arachnotrae */
    addcountweighted(69, nc, 50);       /* Mini Barons */
    addcountweighted(71, nc, 40);       /* Pain Elementals */
    addcountweighted(84, nc, 5);        /* SS Nazis */
    addcountweighted(88, nc, 25);       /* Final bosses */
    PRINTITEM("Total monster hp", nc);
    PRINTITEM2("Max ammo damage", wdam);

    for (i = 0; i < 6; i++) {
        if (!wdam[i])
            continue;
        dratio[i] = (wdam[i] ? (float) nc[i] / (float) wdam[i] : 0);    /* Prevent
                                                                         * divide-by-zero */
    }
    printf("\t%-20s %0.3f %0.3f %0.3f %0.3f", "RATIO", 0.5 * dratio[0], dratio[0], dratio[1], dratio[2]);
    printf(" | %0.3f %0.3f %0.3f %0.3f\n", 0.5 * dratio[3], dratio[3], dratio[4], dratio[5]);

    /*** Return success ***/

    return 1;
}

/***************************************************************************/

/* Item count */
#define COB(item) countth(item,battr)
/* Print a character if the item is present */
#define PRB0(item,char) printf("%c",(COB(item)?(char):'.'))
/* Print item count, one digit */
#define PRB1(item) {n=COB(item);printf((n>9?"+ ":"%d "),n);}
/* Print item count, two digits */
#define PRB2(item) {n=COB(item);printf((n>99?"++ ":"%2d "),n);}

int         countbriefly(void)
{
    /* Counts the things in a level, with one-line output format.  Returns
     * zero if error. */

    long        wdam = 0;       /* Total weapon damage */
    long        mhp = 0;        /* Total monster hit points */
    float       dratio;         /* Damage ratio */

    long        l;
    int         n, i;

    /*** Player starts ***/

    n = 0;
    for (i = 1; i <= 4; i++) {          /* Find player 1-4 starts */
        if (countth(i, -1))
            n++;
    }
    printf("%d%lX ", n, countth(11, -1));        /* Find deathmatch
                                                         * starts */

    /*** Monsters ***/

    /* Bosses */
    PRB2(3003);
    PRB1(16);
    PRB1(7);
    PRB1(88);

    /* Monsters */
    PRB2(3004);
    PRB2(9);
    PRB2(65);
    PRB2(84);
    PRB2(3001);
    PRB2(3002);
    PRB2(58);
    PRB2(3006);
    PRB2(3005);
    PRB2(64);
    PRB2(66);
    PRB2(67);
    PRB2(68);
    PRB2(69);
    PRB2(71);

    /* Weapons */
    PRB0(2005, '1');
    putchar('2');
    PRB0(2001, '3');
    PRB0(82, '#');
    PRB0(2002, '4');
    PRB0(2003, '5');
    PRB0(2004, '6');
    PRB0(2006, '7');

    /* Equipment */
    putchar(' ');
    PRB0(8, 'B');
    PRB0(2022, 'V');
    PRB0(2023, 'S');
    PRB0(2024, 'I');
    PRB0(2025, 'R');
    PRB0(2026, 'A');
    PRB0(2045, 'L');

    /*** Calculate damage ratio ***/

    /** Ammo from all sources **/

    l = 20 * COB(2002) + 10 * COB(2007) + 50 * COB(2048) + 10 * COB(8) + 5 * COB(3004);
    wdam += l;                          /* Bullets */

    l = 8 * COB(2001) + 4 * COB(2008) + 20 * COB(2049) + 4 * COB(8) + 4 * COB(9)
     + 16 * COB(82);
    wdam += 7 * l;                      /* Shells */

    l = 2 * COB(2003) + COB(2010) + 5 * COB(2046) + COB(8);
    wdam += 20 * l;                     /* Rockets */

    l = 40 * COB(2004) + 40 * COB(2006) + 20 * COB(17) + 100 * COB(2047) + 20 * COB(8);
    wdam += 2 * l;                      /* Cell packs */

    /** Monster hit points **/

    mhp = 2 * COB(3004) + 3 * COB(9) + 6 * COB(3001) + 15 * COB(3002) + 15 * COB(58)
          + 10 * COB(3006) + 40 * COB(3005) + 100 * COB(3003) + 400 * COB(16) + 300 * COB(7)
           + 70 * COB(64) + 7 * COB(65) + 30 * COB(66) + 60 * COB(67) + 50 * COB(68) + 50 * COB(69)
           + 40 * COB(71) + 5 * COB(84) + 25 * COB(88);

    dratio = (wdam ? (float) mhp / (float) wdam : 0);   /* Make sure we don't
                                                         * divide by zero */
    if (isbrief == 1)
        dratio *= 0.5;                  /* Twice as much ammo in easiest
                                         * level */

    printf(" %0.3f\n", dratio);

    /*** Return success ***/

    return 1;

}

/***************************************************************************/
/***************************************************************************/

int         countlinedefs(FILE * f, long offs, long len)
{
    /* Counts the interesting linedefs in a map's LINEDEFS data.  Returns
     * zero if error. */

    long        savepos = ftell(f);     /* Save position in WAD file */

    long        nlines = len / 14;      /* Total number of linedefs */
    long        ntrig = 0;      /* Number of linedefs which do something */

    short int   ibuf[7];
    long        l;

    /*** Seek to the LINEDEFS data and loop through the linedefs ***/

    fseek(f, offs, SEEK_SET);

    for (l = 0; l < nlines; l++) {
        fread(ibuf, sizeof(short int), 7, f); /* Read a linedef */

        if (ibuf[3])
            ntrig++;
    }

    /*** Print information ***/

    printf("\tTriggers / linedefs    %4ld / %4ld\n", ntrig, nlines);

    /*** Seek to original file position and return success ***/

    fseek(f, savepos, SEEK_SET);
    return 1;                           /* Success */
}

/***************************************************************************/

int         countsectors(FILE * f, long offs, long len)
{
    /* Counts the interesting sectors in a map's SECTORS data.  Returns zero
     * if error. */

    long        savepos = ftell(f);     /* Save position in WAD file */

    long        nsec = len / 26;/* Total number of sectors */
    long        bright = 0L;     /* Average brightness */
    long        nnuke = 0L;      /* Sectors with nukeage */
    long        nsecret = 0L;    /* Secret sectors */

    short int   ibuf[13];
    long        l;

    /*** Seek to the SECTORS data and loop through the sectors ***/

    fseek(f, offs, SEEK_SET);

    for (l = 0; l < nsec; l++) {
        fread(ibuf, sizeof(short int), 13, f);/* Read a sector */

        bright += ibuf[10];             /* Accumulate brightness */

        switch (ibuf[11]) {             /* Check for specials */
          case 4:                       /* -20%, blinking */
          case 5:                       /* -10% */
          case 7:                       /* -5% */
          case 11:                      /* -20%, end of level */
          case 16:                      /* -20% */
            nnuke++;
            break;
          case 9:                       /* Secret */
            nsecret++;
            break;
        }
    }

    /*** Print information ***/

    printf("\tAverage brightness            %4ld (0=dark, 255=bright)\n", bright / nsec);

    printf("\tSecrets                       %4ld\n", nsecret);
    printf("\tNukeage / sectors      %4ld / %4ld\n", nnuke, nsec);

    /*** Seek to original file position and return success ***/

    fseek(f, savepos, SEEK_SET);
    return 1;                           /* Success */
}

/***************************************************************************/
/***************************************************************************/

int         PrintAllStuff(FILE * f)
{
    /* Prints all information for a map.  Returns zero if error. */


    /*** Print things ***/

    if (thlen) {
        if (loadthings(f, thoff, thlen)) {      /* Load things */
            if (isbrief)
                countbriefly(); /* Print brief counts */
            else
                countthings();          /* Print verbose counts */
            freethings();               /* Free array, restore original file
                                         * position */
        }
    }
    /*** If brief info, return now (no extended info) ***/

    if (isbrief)
        return 1;

    /*** Print other info ***/

    printf("    Other info:\n");

    if (selen)
        countsectors(f, seoff, selen);
    if (ldlen)
        countlinedefs(f, ldoff, ldlen);

    printf("\tReject resource                %s\n", (rjlen > 0 ? "YES" : " NO"));

    /*** Return success ***/

    return 1;
}

/***************************************************************************/

int         handlepwad(char *fname)
{
    /* Handles a PWAD file.  Returns zero if error. */

    int         ispwad = 0;     /* Are we a PWAD? (or an IWAD) */

    long        ndirent;        /* Number of entries in WAD directory */
    long        diroffs;        /* Offset of directory in WAD file */

    long        eoffs, elen;    /* Offset and length of a directory entry */
    char        ename[9] = "entrynam";  /* Name of the entry */
    int         mission = 0;    /* Current mission */
    int		episode;	/* Current episode */

    FILE       *f;
    int         i, j;

    /*** Open the PWAD file ***/

    f = fopen(fname, "rb");
    if (!f) {
        fprintf(stderr, "Can't open file %s\n", fname);
        return 0;
    }
    /*** Read the header ***/

    fread(buf, sizeof(char), 4, f);
    if (!strncmp(buf, "IWAD", 4)) {
        ispwad = 0;
    } else if (!strncmp(buf, "PWAD", 4)) {
        ispwad = 1;
    } else {
        printf("%s is not a DOOM WAD file\n", fname);
        fclose(f);
        return 0;
    }

    if (!isbrief) {                     /* Start a new filename in the output */
        printf("%cWAD FILE %s:\n", (ispwad ? 'P' : 'I'), fname);
    }
    fread(&ndirent, sizeof(long), 1, f);/* Number of entries in WAD dir */
    fread(&diroffs, sizeof(long), 1, f);/* Offset of directory in WAD */

    /*** Print the WAD directory ***/

    fseek(f, diroffs, SEEK_SET);        /* Go to the directory */

    for (i = 0; i < ndirent; i++) {

        /** Read entry **/

        fread(&eoffs, sizeof(long), 1, f);      /* Offset of entry's data */
        fread(&elen, sizeof(long), 1, f);       /* Length of entry's data */
        fread(&ename, sizeof(char), 8, f);      /* Name of entry */

        /** If entry is part of a level, print it **/

        for (j = 0; j < 10; j++) {
            if (!strcmp(ename, contents[j]))
                break;                  /* Matched valid contents for a level */
        }

        switch (j) {
          case 10:                      /* Not level contents */

            if (!mission)
                break;

            /** Look at all the stuff we found **/

            PrintAllStuff(f);

            /** Reset status **/

            thlen = ldlen = selen = rjlen = 0;
            mission = 0;                /* No longer in a level */
            break;

          case 0:                       /* THINGS */
            thoff = eoffs;
            thlen = elen;
            break;

          case 1:                       /* LINEDEFS */
            ldoff = eoffs;
            ldlen = elen;
            break;
          case 7:                       /* SECTORS */
            seoff = eoffs;
            selen = elen;
            break;
          case 8:                       /* REJECT */
            rjlen = elen;
            break;
        }

        /** Keep track of which mission we're looking at, Doom][ **/

        if (ename[0] == 'M' && ename[1] == 'A' && ename[2] == 'P' &&
        isdigit(ename[3]) && isdigit(ename[4]) && ename[5] == '\0') {
            mission = atoi(ename + 3);

            if (!isbrief) {
                printf("-------------------------------------------------------------------------------\n");
                printf("MAP %02d              Difficulty: S1    S2    S3   S45 |    M1    M2    M3   M45\n", mission);
                printf("-------------------------------------------------------------------------------\n");
            }
        }

	/** If this isn't a Doom][ mission try Doom **/

	if ((mission == 0) && (ename[0] == 'E') && (ename[2] == 'M')) {
	    mission = atoi(ename + 3);
	    episode = atoi(ename + 1);
            if (!isbrief) {
                printf("-------------------------------------------------------------------------------\n");
                printf("MAP E%dM%d            Difficulty: S1    S2    S3   S45 |    M1    M2    M3   M45\n", episode, mission);
                printf("-------------------------------------------------------------------------------\n");
            }
	}
    }

    /*** Look at all the stuff we found from the last map, if any ***/

    if (mission)
        PrintAllStuff(f);
    thlen = ldlen = selen = rjlen = 0;

    /*** Close the file and return success ***/

    fclose(f);
    return 1;
}

/***************************************************************************/
/***************************************************************************/

int         main(int argc, char *argv[])
{

    /*struct ffblk ff;*/
    int         i;

    /*** Make sure we've been given a filename ***/

    putchar('\n');
    printf("WADWHAT 2.3 by Gregory P. Kwok (gkwok@jps.net)\n");
    printf("Based on WADWHAT 1.1 by Randall R. Spangler (rspangle@micro.caltech.edu)\n");
    printf("===============================================================================\n");

    if (argc < 2) {
        printf("Prints the contents of a WAD file.\n");
        printf("Usage:\n\twadwhat [-Bn[M]] file1\n");
        printf("\n\t\t-Bn\tbrief contents at skill level n\n");
        printf("\t\t-BnM\tbrief contents at skill level n, multiplayer\n");
        return 1;
    }
    /*** Match all wildcards ***/

    for (i = 1; i < argc; i++) {

        /** See if we're an option **/

        if (!strncmp(argv[i], "-B", 2)) {       /* Print briefly */
            isbrief = atoi(argv[i] + 2);/* Extract skill level */
            printf("St Bosses   Monsters                                     Weapons  Equip   RATIO\n");
            printf("cd ba-c-s-f tr-se-ch-ss-im-de-sp-ls-ca-av-re-ma-ar-mb-pe cps2crpb bvsiral -----\n");
            battr = battrarry[isbrief - 1];     /* Matching attributes for
                                                 * brief printout */
            if (toupper(argv[i][3]) == 'M')
                battr |= 0x10;          /* Add in multi-player stuff */

            continue;
        }
        else
          handlepwad(argv[i]);
    }

    /*** Return success ***/

    return 0;
}

/***************************************************************************/
/***************************************************************************/
