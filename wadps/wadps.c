/*
     WAD PostScript map generator. V1.2
     Copyright James Bonfield, 3/3/94

     Modified for PC by Gerhard Karnik (g.karnik@att.com) 3/29/94
     Enhancements (V1.2) by Gerhard Karnik (g.karnik@att.com) 5/12/94

     Ported back to UNIX by Udo Munk (munkudo@aol.com) 5/2/98
     And ported to Cygwin by Udo Munk (munkudo@aol.com) 12/14/98
     Applied a bug fix reported by Oliver Kraus (olikraus@yahoo.com) 1/23/2000

Description
-----------

This program produces PostScript maps of levels, extracted from a Wad
file. The program is written for the UNIX operating system, and so you need a
method of transfering your doom.wad file, (or perhaps having it visible by,
say, mounting your DOS partition).

Thick lines represent solid walls on the map.
Circles represent objects.
Arrows represent the enemy. The direction of the arrow is the direction the
creature is facing.
Gray lines represent secret passages.

USAGE: wadps [-o] [-m] [-dm] [wadfile] [level]

-o              Do not display objects
-m              Do not display monsters
-dm             Display deathmatch items
wadfile         Selects another wad file (default is "doom.wad")
level           Level number (default is all levels), can use "E1M1" or "11"

The postscript will then be displayed to stdout.
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include "sysdep.h"
#include "swapfunc.h"

unsigned nread;         /* throw away variable for read() */
char wadfile[80], level[80];

typedef long int4;
typedef short int2;

struct wad_header {
        int4 magic;
        int4 dir_size;
        int4 dir_off;
};

struct directory {
        int4 start;
        int4 length;
        char name[8];
};

typedef struct linedef_ {
        int2 from_vertex;
        int2 to_vertex;
        int2 attrib;
        int2 type;
        int2 trigger;
        int2 sidedef1;
        int2 sidedef2;
} linedef;

/* linedef attrib bits */
#define LI_IMPASS 0x01
#define LI_SECRET 0x20

typedef struct vertex_ {
        int2 x;
        int2 y;
} vertex;

typedef struct sidedef_ {
        int2 x;
        int2 y;
        char wall_above[8];
        char wall_below[8];
        char wall_part[8];
        int2 sector;
} sidedef;

typedef struct ssector_ {
        int2 num;
        int2 seg;
} ssector;

typedef struct sector_ {
        int2 bot_pos;
        int2 top_pos;
        char bot_texture[8];
        char top_texture[8];
        int2 brightness;
        int2 special;
        int2 trigger;
} sector;

typedef struct thing_ {
        int2 x;
        int2 y;
        int2 angle;
        int2 type;
        int2 attrib;
} thing;

/* thing 'type' bits */
#define TH_SKILL1  0x01
#define TH_SKILL2  0x01
#define TH_SKILL3  0x02
#define TH_SKILL4  0x04
#define TH_MULTI   0x10

typedef int2 blockmap;

struct directory *open_wad(char *file, int *fd, long *size);
long get_index(struct directory *d, long size, char *name, long st);
linedef *read_linedefs(int fd, struct directory *d, long size, long start, long *num);
vertex *read_vertexes(int fd, struct directory *d, long size, long start, long *num);
blockmap *read_blockmap(int fd, struct directory *d, long size, long start, long *num);
sidedef *read_sidedefs(int fd, struct directory *d, long size, long start, long *num);
sector *read_sectors(int fd, struct directory *d, long size, long start, long *num);
thing *read_things(int fd, struct directory *d, long size, long start, long *num);
void usage(void);
int doit(int fd, struct directory *dir, long size, int objs, int mons, int deathmatch);

/*
 * Paper is 8 1/2" by 11"
 * We allow 1/2" border.
 */
#define PAPER_X (72 * 8.5)
#define PAPER_Y (72 * 11)
#define PAPER_BORDER (72 * 0.5)


void usage()
{
        fprintf(stderr, "WAD PostScript map generator V1.2\n");
        fprintf(stderr, "Produces PostScript maps of levels extracted from a WAD file.\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "USAGE: wadps [-o] [-m] [-dm] [wadfile] [level]\n");
        fprintf(stderr, "  -o              Do not display objects\n");
        fprintf(stderr, "  -m              Do not display monsters\n");
        fprintf(stderr, "  -dm             Display deathmatch items\n");
        fprintf(stderr, "  -? or -h        This help screen\n");
        fprintf(stderr, "  wadfile         Selects another wad file (default is \"doom.wad\")\n");
        fprintf(stderr, "  level           Level number (default is all levels), can use \"E1M1\" or \"11\"\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Map legend:\n");
        fprintf(stderr, "  Thick line     = Solid walls on the map\n");
        fprintf(stderr, "  Gray line      = Secret passages\n");
        fprintf(stderr, "  Shaded circle  = Invisibility, radiation suit, computer map, or lite goggles\n");
        fprintf(stderr, "  Arrow          = Monster facing the direction of the arrow\n");
        fprintf(stderr, "  3D box         = Box of ammo\n");
        fprintf(stderr, "  Smiley face    = Invulnerability or Soulsphere\n");
        fprintf(stderr, "  Cross          = Stimpak, medikit, health bonus, or berserk strength\n");
        fprintf(stderr, "  Shirt          = Green armor 100%% or blue armor 200%%\n");
        exit(-1);
}


int main(int argc, char **argv)
{
        struct directory *d;
        int fd, i, ep, lv;
        long size;
        int objs = 1, mons = 1, deathmatch=0;
        int done;
        char path[80];

        for(i=1; i<argc; i++) {
                if(!strcmp(argv[i], "-dm")) deathmatch = 1;
                if(!strcmp(argv[i], "-o"))  objs = 0;
                if(!strcmp(argv[i], "-m"))  mons = 0;
                if(!strcmp(argv[i], "-?"))  usage();
                if(!strcmp(argv[i], "-h"))  usage();
                if(argv[i][0] != '-')        break; /* no more params */
        }

        switch( argc-i ) {
        case 0: /* no more params */
                strcpy(path, "doom.wad");
                strcpy(level, "");
                break;

        case 1: /* only wad name */
                strcpy(path, argv[i]);
                strcpy(level, "");
                break;

        case 2: /* wad name + level */
                strcpy(path, argv[i]);
                strcpy(level, argv[i+1]);

                if(strlen(level) == 2) {        /* convert '11' to 'E1M1' */
                        level[4] = 0;
                        level[3] = level[1];
                        level[2] = 'M';
                        level[1] = level[0];
                        level[0] = 'E';
                }
                break;

        default:
                usage();
        }

#if 0
        if(strlen(path)>12)
                fprintf(stderr, "WARNING: Can only get WADs in current directory.\n");
#endif

	done = 0;

        while(!done) {
		strcpy(wadfile, path);

                if((d = open_wad(wadfile, &fd, &size)) != NULL) {

                        if(!level[0]) { /* do all levels */
                            for(ep=1; ep<=3; ep++)
                                for(lv=1; lv<=9; lv++) {
                                    sprintf(level, "E%dM%d", ep, lv);
                                    doit(fd, d, size, objs, mons, deathmatch);
                                }
                            strcpy(level, "");
                        }
                        else
                            if(doit(fd, d, size, objs, mons, deathmatch) == -1)
                                fprintf(stderr, "Unknown level: %s\n", level);

			close(fd);
                }

		done = 1;
        }

        return 0;
}


int doit(int fd, struct directory *dir, long size, int objs, int mons, int deathmatch)
{
        linedef *linedefs;
        vertex *vertexes;
        blockmap *blocks;
        sidedef *sidedefs;
        sector *sectors;
        thing *things;
        long numline, numvert, numblock, numside, numsect, numthing;
        long lev_index;
        long xorigin, yorigin, xsize, ysize;
        int2 sector;
        double xscale, yscale;
        long i;

        /* find level index */
        lev_index = get_index(dir, size, level, 0);
        if(lev_index == -1) return(-1);  /* level not found */

        /* load relevent arrays for this level */

        linedefs = read_linedefs(fd, dir, size, lev_index, &numline);
        vertexes = read_vertexes(fd, dir, size, lev_index, &numvert);
        blocks   = read_blockmap(fd, dir, size, lev_index, &numblock);
        sidedefs = read_sidedefs(fd, dir, size, lev_index, &numside);
        sectors  = read_sectors(fd, dir, size, lev_index, &numsect);
        things   = read_things(fd, dir, size, lev_index, &numthing);

        /* calculate scaling info */
        xorigin = blocks[0];
        yorigin = blocks[1];
        xsize   = blocks[2] * 0x80;
        ysize   = blocks[3] * 0x80;
        xscale  = (double) (PAPER_Y - 2 * PAPER_BORDER) / xsize;
        yscale  = (double) (PAPER_X - 2 * PAPER_BORDER) / ysize;

        if(xscale > yscale)
                xscale = yscale;
        else
                yscale = xscale;

        /* output postscript header */
        printf("%%!\n");
        printf("newpath\n");
        printf("\n");
        printf("1 setlinecap\n");
        printf("%f %f translate\n", PAPER_BORDER, PAPER_Y - PAPER_BORDER);

        printf("/Times-Roman findfont 14 scalefont setfont\n");
        printf("0 setgray 20 0 moveto (%s - %s) show\n", wadfile, level);

        printf("-90 rotate\n");
        printf("%f %f scale\n", xscale, yscale);
        printf("%ld %ld translate\n", -xorigin, -yorigin);
        printf("%f setlinewidth\n", (double) .5 / xscale);
        printf("\n");
        printf("/Times-Roman findfont %f scalefont setfont\n", 5 / xscale);
        printf("/Text {\n");
        printf("    gsave\n");
        printf("    0 setgray moveto show\n");
        printf("    grestore\n");
        printf("} def\n");
        printf("\n");
        printf("/l {\n");
        printf("    setgray setlinewidth moveto lineto stroke\n");
        printf("} def\n");
        printf("\n");
        printf("/a {\n");
        printf("    0.5 setgray\n");
        printf("    5 0 360 arc stroke\n");
        printf("} def\n");
        printf("\n");
        printf("/m {\n");
        printf("    gsave\n");
        printf("    0 setgray\n");
        printf("    1 setlinewidth\n");
        printf("    moveto rotate\n");
        printf("    0 -13 rmoveto\n");
        printf("    0 25 rlineto\n");
        printf("    -10 -10 rlineto\n");
        printf("    10 10 rmoveto\n");
        printf("    10 -10 rlineto\n");
        printf("    stroke\n");
        printf("    grestore\n");
        printf("} def\n");
        printf("\n");
        printf("/ammo {\n");
        printf("    gsave\n");
        printf("    0 setgray\n");
        printf("    1 setlinewidth\n");
        printf("    moveto\n");
        printf("    18 0 rlineto\n");
        printf("    0 -18 rlineto\n");
        printf("    -18 0 rlineto\n");
        printf("    0 18 rlineto\n");
        printf("    9 9 rlineto\n");
        printf("    18 0 rlineto\n");
        printf("    -9 -9 rlineto\n");
        printf("    9 9 rmoveto\n");
        printf("    0 -18 rlineto\n");
        printf("    -8 -9 rlineto\n");
        printf("    stroke\n");
        printf("    grestore\n");
        printf("} def\n");
        printf("\n");
        printf("/armor {\n");
        printf("    gsave\n");
        printf("    0 setgray\n");
        printf("    1 setlinewidth\n");
        printf("    translate\n");
        printf("    1.5 1.5 scale\n");
        printf("    0 -5 moveto\n");
        printf("    newpath\n");
        printf("    9 10 8 30 150 arc\n");
        printf("    -9 10 8 30 150 arc\n");
        printf("    -10 9 4 150 20 arcn\n");
        printf("    0 -12 rlineto\n");
        printf("    12 0 rlineto\n");
        printf("    0 12 rlineto\n");
        printf("    10 9 4 140 30 arcn\n");
        printf("    closepath\n");
        printf("    stroke\n");
        printf("    grestore\n");
        printf("} def\n");
        printf("\n");
        printf("/health {\n");
        printf("    gsave\n");
        printf("    0 setgray\n");
        printf("    2 setlinewidth\n");
        printf("    moveto\n");
        printf("    8 0 rlineto\n");
        printf("    -16 0 rlineto\n");
        printf("    8 0 rmoveto\n");
        printf("    0 8 rlineto\n");
        printf("    0 -16 rlineto\n");
        printf("    stroke\n");
        printf("    grestore\n");
        printf("} def\n");
        printf("\n");
        printf("/smiley {\n");
        printf("    gsave\n");
        printf("    0 setgray\n");
        printf("    1 setlinewidth\n");
        printf("    translate\n");
        printf("    newpath\n");
        printf("    0 0 15 90 89 arc\n");
        printf("    closepath\n");
        printf("    -6 -9 rmoveto\n");
        printf("    3 0 rlineto\n");
        printf("    6 0 rmoveto\n");
        printf("    3 0 rlineto\n");
        printf("    -7 -6 rmoveto\n");
        printf("    2 0 rlineto\n");
        printf("    -5 -8 rmoveto\n");
        printf("    8 0 rlineto\n");
        printf("    stroke\n");
        printf("    grestore\n");
        printf("} def\n");
        printf("\n");

        /* Display the walls. Secret passages are done in grey. */
        for(i=0; i<numline; i++) {
                sector = sidedefs[linedefs[i].sidedef1].sector;

                if(linedefs[i].type == 11 ||   /* level exit */
                   linedefs[i].type == 51 ||
                   linedefs[i].type == 52)
                        printf("[1 15] 1 setdash\n");

                printf("%d %d %d %d ",
                       vertexes[linedefs[i].to_vertex].x,
                       vertexes[linedefs[i].to_vertex].y,
                       vertexes[linedefs[i].from_vertex].x,
                       vertexes[linedefs[i].from_vertex].y);

                if (linedefs[i].attrib & LI_IMPASS)
                        printf("%f ", 1.2 / xscale);
                else
                        printf("%f ", 0.5 / xscale);

                if (linedefs[i].attrib & LI_SECRET || sectors[sector].special == 9)
                        printf(".5 l\n");
                else
                        printf("0 l\n");

                if(linedefs[i].type == 11 ||   /* level exit */
                   linedefs[i].type == 51 ||
                   linedefs[i].type == 52)
                        printf("[] 0 setdash\n");
        }

        /* Do we need to display objects or monsters? */
        if(objs || mons) {
                printf("    %f setlinewidth\n", 1 / xscale);

                for(i=0; i<numthing; i++) {

                        /* filter out skill level 4 & 5 stuff */
                        if(!(things[i].attrib & (TH_SKILL2 + TH_SKILL3)))
                                continue;

                        /* filter out deathmatch items */
                        if((things[i].attrib & TH_MULTI) && !deathmatch)
                                continue;

                        switch(things[i].type) {

                        /* monsters */
                        case 7:        /* spider    */
                        case 9:        /* sergeant  */
                        case 16:       /* cyber     */
                        case 58:       /* invis     */
                        case 3001:     /* imp       */
                        case 3002:     /* demon     */
                        case 3003:     /* minotaur  */
                        case 3004:     /* human     */
                        case 3005:     /* beholder  */
                        case 3006:     /* skull     */
                                if(mons) {
                                        printf("%d %d %d m\n",
                                               things[i].angle, things[i].x, things[i].y);
                                }
                                break;

                        /* weapons */
                        case 2001:     /* Shotgun           */
                                printf("(ShotG) %d %d Text\n", things[i].x - 50, things[i].y);
                                break;

                        case 2002:     /* Chaingun          */
                                printf("(ChainG) %d %d Text\n", things[i].x - 50, things[i].y);
                                break;

                        case 2003:     /* Rocket launcher   */
                                printf("(Rocket) %d %d Text\n", things[i].x - 50, things[i].y);
                                break;

                        case 2004:     /* Plasma gun        */
                                printf("(Plasma) %d %d Text\n", things[i].x - 50, things[i].y);
                                break;

                        case 2005:     /* Chainsaw          */
                                printf("(ChainS) %d %d Text\n", things[i].x - 50, things[i].y);
                                break;

                        case 2006:     /* BFG9000           */
                                printf("(BFG) %d %d Text\n", things[i].x - 30, things[i].y);
                                break;

                        /* Good Ammo */
                        case 8:        /* Backpack          */
                        case 17:       /* Cell charge pack  */
                        case 2010:     /* 1 rocket          */
                        case 2046:     /* Box of Rockets    */
                        case 2047:     /* Cell charge       */
                        case 2048:     /* Box of Ammo       */
                        case 2049:     /* Box of Shells     */
                                if(objs) {
                                        printf("%d %d ammo\n", things[i].x, things[i].y);
                                }
                                break;

                        case 2022:     /* Invulnerability   */
                        case 2013:     /* Soulsphere, Supercharge, +100% health */
                                if(objs) {
                                        printf("%d %d smiley\n", things[i].x, things[i].y);
                                }
                                break;

                        /* Health */
                        case 2011:     /* Stimpak           */
                        case 2012:     /* Medikit           */
                        case 2023:     /* Berserk Strength  */
                                if(objs) {
                                        printf("%d %d health\n", things[i].x, things[i].y);
                                }
                                break;

                        /* Armour */
                        case 2018:     /* Green armor 100%  */
                        case 2019:     /* Blue armor 200%   */
                                if(objs) {
                                        printf("%d %d armor\n", things[i].x, things[i].y);
                                }
                                break;

                        /* other good stuff */
                        case 2024:     /* Invisibility      */
                        case 2025:     /* Radiation suit    */
                        case 2026:     /* Computer map      */
                        case 2045:     /* Lite goggles      */
                                if(objs) {
                                        printf("%d %d a\n", things[i].x, things[i].y);
                                }
                                break;

                        /* Another type of object */
                        case 2007:     /* Ammo clip         */
                        case 2008:     /* 4 shotgun shells  */
                        case 2014:     /* Health bonus      */
                        case 2015:     /* Armor bonus       */

                        case 5:        /* Blue keycard      */
                        case 6:        /* Yellow keycard    */
                        case 13:       /* Red Keycard       */
                        case 38:       /* Red skullkey      */
                        case 39:       /* Yellow skullkey   */
                        case 40:       /* Blue skullkey     */
                        default:
                                break;
                        }
                }
        }
        puts("showpage");

        free(linedefs);
        free(vertexes);
        free(blocks);
        free(sidedefs);
        free(sectors);
        free(things);

        return(0);
}


struct directory *open_wad(char *file, int *fd, long *size)
{
        struct wad_header h;
        struct directory *d;
        char *s;

	if ((*fd = open(file, O_RDONLY|O_BINARY)) == -1) {
                perror("can't open wad file");
		close(*fd);
                return (struct directory *)0;
        }

	nread = read(*fd, &h, sizeof(h));
        h.dir_size = bswapl(h.dir_size);
        h.dir_off = bswapl(h.dir_off);

        s = (char *)&h.magic;

        if(s[1] != 'W' || s[2] != 'A' || s[3] != 'D') {
                fprintf(stderr, "Not a WAD file: \"%s\"\n", file);
		close(*fd);
                return (struct directory *) 0;
        }

        if(NULL == (d = (struct directory *)calloc(h.dir_size, sizeof(*d)))) {
                fprintf(stderr, "Could not allocated %ld bytes.\n",
                        h.dir_size * sizeof(*d));
		close(*fd);
                return (struct directory *) 0;
        }
        *size = h.dir_size;

        lseek(*fd, h.dir_off, SEEK_SET);
	nread = read(*fd, d, *size * sizeof(*d));
        swap_lump_list(d, h.dir_size);

        return d;
}

long get_index(struct directory *d, long size, char *name, long start)
{
        long i;

        for(i=start; i<size; i++)
                if(strncmp(d[i].name, name, 8) == 0) return i;

        return -1;
}

linedef *read_linedefs(int fd, struct directory *d, long size, long start, long *num)
{
        long index;
        linedef *indexp;

        index = get_index(d, size, "LINEDEFS", start);
        indexp = (linedef *) malloc(d[index].length);
        if(!indexp) {
                printf("Could not allocate %ld bytes\n",d[index].length);
                exit(-1);
        }
        lseek(fd, d[index].start, SEEK_SET);
	nread = read(fd, indexp, d[index].length);
        *num = d[index].length / sizeof(linedef);
        swap_linedef_list(indexp, *num);

        return indexp;
}

vertex *read_vertexes(int fd, struct directory *d, long size, long start, long *num)
{
        long index;
        vertex *indexp;

        index = get_index(d, size, "VERTEXES", start);
        indexp = (vertex *) malloc(d[index].length);
        if(!indexp) {
                printf("Could not allocate %ld bytes\n",d[index].length);
                exit(-1);
        }
        lseek(fd, d[index].start, SEEK_SET);
	nread = read(fd, indexp, d[index].length);
        *num = d[index].length / sizeof(vertex);
        swap_vertex_list(indexp, *num);

        return indexp;
}

blockmap *read_blockmap(int fd, struct directory *d, long size, long start, long *num)
{
        long index;
        blockmap *indexp;

        index = get_index(d, size, "BLOCKMAP", start);
        indexp = (blockmap *) malloc(d[index].length);
        if(!indexp) {
                printf("Could not allocate %ld bytes\n",d[index].length);
                exit(-1);
        }
        lseek(fd, d[index].start, SEEK_SET);
	nread = read(fd, indexp, d[index].length);
        *num = d[index].length / sizeof(blockmap);
        {
          int i;
          for( i = 0; i < *num; i++ )
            swapint(indexp+i);
        }

        return indexp;
}

sidedef *read_sidedefs(int fd, struct directory *d, long size, long start, long *num)
{
        long index;
        sidedef *indexp;

        index = get_index(d, size, "SIDEDEFS", start);
        indexp = (sidedef *) malloc(d[index].length);
        if(!indexp) {
                printf("Could not allocate %ld bytes\n",d[index].length);
                exit(-1);
        }
        lseek(fd, d[index].start, SEEK_SET);
	nread = read(fd, indexp, d[index].length);
        *num = d[index].length / sizeof(sidedef);
        swap_sidedef_list(indexp, *num);

        return indexp;
}

sector *read_sectors(int fd, struct directory *d, long size, long start, long *num)
{
        long index;
        sector *indexp;

        index = get_index(d, size, "SECTORS", start);
        indexp = (sector *) malloc(d[index].length);
        if(!indexp) {
                printf("Could not allocate %ld bytes\n",d[index].length);
                exit(-1);
        }
        lseek(fd, d[index].start, SEEK_SET);
	nread = read(fd, indexp, d[index].length);
        *num = d[index].length / sizeof(sector);
        swap_sector_list(indexp, *num);

        return indexp;
}

thing *read_things(int fd, struct directory *d, long size, long start, long *num)
{
        long index;
        thing *indexp;

        index = get_index(d, size, "THINGS", start);
        indexp = (thing *) malloc(d[index].length);
        if(!indexp) {
                printf("Could not allocate %ld bytes\n",d[index].length);
                exit(-1);
        }
        lseek(fd, d[index].start, SEEK_SET);
	nread = read(fd, indexp, d[index].length);
        *num = d[index].length / sizeof(thing);
        swap_thing_list(indexp, *num);

        return indexp;
}
