/******************************************************************************
	PROGRAM:	WARM.C
	WRITTEN BY:	Robert Fenske, Jr. (rfenske@swri.edu)
	CREATED:	May  1994
	DESCRIPTION:	This program is the WAD Auxiliary Resource Manipulator.
			It extracts data from a DOOM/HERETIC/HEXEN-related
			PWAD, IWAD, or VERDA patch file and builds the
			BSP-related structures segs, subsectors, and nodes;
			the blockmap structure; and the reject structure.  It
			can build these structures for all the levels in the
			input file, or for a specific level(s) found in the
			input file.  The main command line arguments are as
			follows:

			[-<levels>] [-n[=<opt>]] [-b] [-r[=<xlist>]] [-z]
			[-q] <input file> [output file]

			where -<levels> specifies one or more particular
			levels within the input file, -n specifies to build
			the nodes, -b specifies to build the blockmap
			(packed), -r specifies to build a full reject, -z
			specifies to generate a zero-filled reject (much
			faster than building a full REJECT), -q specifies to
			limit I/O during processing ("quiet"); input file is
			the input filename, and output file is the optional
			output filename.  If no output file is specified, the
			input file is rewritten with the new data.

			It also contains various functions to manipulate
			various resources in a DOOM/HERETIC/HEXEN-related IWAD,
			PWAD, or VERDA patch file.

			This program has been compiled and run on the following
			operating systems:
				SunOS 4.1.x	cc
				Solaris 2.x	cc
				MS-DOS 6.x	gcc
				OS/2 2.1+3.0	C/Set++
				Linux 1.2.x	gcc
				FreeBSD 2.0	gcc
				HP-UX 9.01	cc
				VMS 5.x		cc
				Windows/NT 3.5	Visual C++
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#if defined(__OS2__)
#include <io.h>
#define isatty		_isatty
#endif
#include "dmglobal.h"

#define SOFTVER		1.6			/* software version */
#define SOFTDATE	"January 1996"		/* software version date */

#define RESOURCES_NEEDED ((long)((1<<THINGS)|(1<<LINES)|(1<<SIDES)|(1<<VERTS)|\
			         (1<<SEGS)|(1<<SSECTS)|(1<<NODES)|(1<<SECTS)))

#define BANNER		"\n\
                       WAD Auxiliary Resource Manipulator\n\
%*sVersion %03.1f of %s by Robert Fenske, Jr (rfenske@swri.edu)\n\
        Ported to OS/2 by Mark K. Mathews (mmathews@genesis.nred.ma.us)\n"

#define help(t)		printf("%*s %s\n",(int)strlen(prog),"",(t))

#define pause()		((void)((isatty(fileno(stdout)) ?\
			 (printf("press Enter to see more options"),\
			  fflush(stdout), getchar()): 0), printf("\n")))

#define level_mark(n,m)	(1 == sscanf((n),"E%*dM%d",&(m)) ||\
			 1 == sscanf((n),"MAP%d",&(m)))

#define is_furniture(t)	\
		((0x001<=(t).item&&(t).item<=0x004) ||/* player starts     */\
		 (t).item==0x00E ||		    /* teleport            */\
		 (0x019<=(t).item&&(t).item<=0x039) ||/* various columns,  */\
		 (0x03B<=(t).item&&(t).item<=0x03F) ||/* skulls, prisoners */\
		 (t).item==0x046 ||		    /* burning barrel      */\
		 (0x048<=(t).item&&(t).item<=0x051) ||/* various hanging   */\
		 (t).item==0x7EC || (t).item==0x7F3)/* light post,barrel   */
#define is_deathmatch(t) ((t).item==0x000B)	/* deathmatch */
#define is_creature(t)	((t).item==0x0007 ||	/* spider demon    */\
			 (t).item==0x0009 ||	/* former sargent  */\
			 (t).item==0x0010 ||	/* cyber demon     */\
			 (t).item==0x003A ||	/* spectre         */\
			 (t).item==0x0040 ||	/* archvile        */\
			 (t).item==0x0041 ||	/* former commando */\
			 (t).item==0x0042 ||	/* revenant        */\
			 (t).item==0x0043 ||	/* mancubus        */\
			 (t).item==0x0044 ||	/* arachnotron     */\
			 (t).item==0x0045 ||	/* hell knight     */\
			 (t).item==0x0047 ||	/* pain elemental  */\
			 (t).item==0x0054 ||	/* Wolfenstein SS  */\
			 (t).item==0x0057 ||	/* spawn spot      */\
			 (t).item==0x0058 ||	/* boss brain      */\
			 (t).item==0x0059 ||	/* boss shooter    */\
			 (t).item==0x0BB9 ||	/* imp             */\
			 (t).item==0x0BBA ||	/* demon           */\
			 (t).item==0x0BBB ||	/* baron of hell   */\
			 (t).item==0x0BBC ||	/* former human    */\
			 (t).item==0x0BBD ||	/* cacodemon       */\
			 (t).item==0x0BBE)	/* lost soul       */
#define is_boss(t)	((t).item==0x0007 ||	/* spider demon    */\
			 (t).item==0x0010 ||	/* cyber demon     */\
			 (t).item==0x0BBB)	/* baron of hell   */
#define is_weapon(t)	((t).item==0x0052 ||	/* super shotgun   */\
			 (t).item==0x07D1 ||	/* shotgun         */\
			 (t).item==0x07D2 ||	/* chain gun       */\
			 (t).item==0x07D3 ||	/* rocket launcher */\
			 (t).item==0x07D4 ||	/* plasma rifle    */\
			 (t).item==0x07D5 ||	/* chainsaw        */\
			 (t).item==0x07D6)	/* BFG9000         */
#define is_key(t)	((t).item==0x0005 ||	/* blue key         */\
			 (t).item==0x0006 ||	/* yellow key       */\
			 (t).item==0x000D ||	/* red key          */\
			 (t).item==0x0026 ||	/* red skull key    */\
			 (t).item==0x0027 ||	/* yellow skull key */\
			 (t).item==0x0028)	/* blue skull key   */
#define is_health(t)	((t).item==0x0053 ||	/* mega sphere   */\
			 (t).item==0x07DB ||	/* stimpak       */\
			 (t).item==0x07DC ||	/* medical kit   */\
			 (t).item==0x07DD ||	/* soul sphere   */\
			 (t).item==0x07DE)	/* health potion */
#define is_armor(t)	((t).item==0x0053 ||	/* mega sphere    */\
			 (t).item==0x07DF ||	/* armor helmet   */\
			 (t).item==0x07E2 ||	/* security armor */\
			 (t).item==0x07E3)	/* combat armor   */

#define Things		((WAD_THING *)iwad->data[e+THINGS])
#define Lines		((WAD_LINE *)iwad->data[e+LINES])
#define Sides		((WAD_SIDE *)iwad->data[e+SIDES])
#define Verts		((WAD_VERT *)iwad->data[e+VERTS])
#define Segs		((WAD_SEGS *)iwad->data[e+SEGS])
#define Ssecs		((WAD_SSECTOR *)iwad->data[e+SSECTS])
#define Nodes		((WAD_NODE *)iwad->data[e+NODES])
#define Sects		((WAD_SECTOR *)iwad->data[e+SECTS])
#define Rejects		((WAD_REJECT *)iwad->data[e+REJECTS])
#define Blockmaps	((WAD_BLOCKMAP *)iwad->data[e+BLKMAPS])
#define Behaviors	((WAD_BEHAVIOR *)iwad->data[e+BEHAVS])

#define NThings		iwad->count[e+THINGS]
#define NLines		iwad->count[e+LINES]
#define NSides		iwad->count[e+SIDES]
#define NVerts		iwad->count[e+VERTS]
#define NSegs		iwad->count[e+SEGS]
#define NSsecs		iwad->count[e+SSECTS]
#define NNodes		iwad->count[e+NODES]
#define NSects		iwad->count[e+SECTS]
#define NRejects	iwad->count[e+REJECTS]
#define NBlockmaps	iwad->count[e+BLKMAPS]
#define NBehaviors	iwad->count[e+BEHAVS]

#if !defined(RAND_MAX)
#define RAND_MAX	(~(1<<(sizeof(int)*8-1)))
#endif
#define randseed(s)	srand(s)
#define randnum()	((double)rand()/(double)RAND_MAX)

#define merge_resource(b,t,i,r,d1,c1,d2,c2) \
			((b) = (char *)blockmem(t,((c1)+(c2))*sizeof(t)),\
			 blockcopy((b),(d1),(c1)*sizeof(t)),\
			 blockcopy(&(b)[(c1)*sizeof(t)],(d2),(c2)*sizeof(t)),\
			 resource_update((i),(r),(b),(c1)+(c2)))
#define add_resource(b,t,d,c,r) \
			((b) = (char *)blockmem(t,(c)),\
			 blockcopy((b),(d),((c)-1)*sizeof(t)),\
			 ((t *)(b))[(c)-1] = (r),\
			 blockfree(d),\
			 (d) = (t *)(b))

#if defined(ANSI_C)				/* prototypes for this file */
int dmit(int ver, WAD_THING *things, long nthings, WAD_SECTOR *sects,
         long nsects);
int _Optlink rand_xy_sort(const void *thing1, const void *thing2);
int _Optlink rand_item_sort(const void *thing1, const void *thing2);
int rand_thing(long seed, WAD_THING *things, long nthings);
int flip(WAD_THING *things, long nthings, WAD_VERT *verts, long nverts,
         WAD_LINE *lines, long nlines, WAD_NODE *nodes, long nnodes,
         WAD_SEGS *segs, long nsegs);
int shift(int dx, int dy, int dz, WAD_THING *things, long nthings,
          WAD_VERT *verts, long nverts, WAD_NODE *nodes, long nnodes,
          WAD_SECTOR *sects, long nsects);
int pack(WAD_INFO *iwad, int e);
void _Optlink node_tree_walk(WAD_NODE *node, int nndx, int depth_cur,
                             int *depth_max, int *depth_sum, int *depth_bal);
int unpack(WAD_INFO *iwad, int e);
int emstat(WAD_INFO *iwad, int e);
int rename_resources(WAD_INFO *iwad, char *rename_list);
int directory(WAD_INFO *iwad);
int substitute(WAD_INFO *iwad, char *list);
int extract(WAD_INFO *iwad, char *list);
int merge(WAD_INFO *iwad, char *list);
int merge_raw(WAD_INFO *iwad, char *list);
#endif


/******************************************************************************
	ROUTINE:	main(ac,av)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	Apr. 1994
	DESCRIPTION:	This routine processes the command line arguments and
			executes the appropriate manipulation function.  If any
			if the arguments are incorrect/illegal a series of
			help screens is displayed.
******************************************************************************/
#if defined(ANSI_C)
int main(int ac, char *av[])
#else
int main(ac,av)
int ac;
char *av[];
#endif
{
  static char func[256] = "";			/* manipulating function(s) */
  static char node_flags[256] = "";		/* node generation flags */
  static char rxcpt_list[256] = "";		/* REJECT exception list */
  static char subst_list[256] = "",		/* rsrce substitution list */
              extract_list[256] = "",		/* resource extraction list */
              merge_list[256] = "",		/* WAD merge file list */
              mraw_list[256] = "";		/* raw merge file list */
  static char rename_list[256] = "";		/* resource rename list */
  char *ifile = NULL, *ofile = NULL;		/* input/output filenames */
  char *prog;					/* program name */
  WAD_INFO *iwad, *owad;			/* input/output info blocks */
  boolean show_help = FALSE;			/* whether to display help */
  boolean good = FALSE;				/* whether processing worked */
  int epmp[100];				/* specific ep/map to do */
  int outtype = 0;				/* specified output type */
  int rej_flags = RG_REJDEPTH & 2;		/* reject processing flags */
  int dmver = 1;				/* deathmatch version # */
  long seed;					/* randomize seed */
  int dx = 0, dy = 0, dz = 0;			/* shift by dx, dy, dz */
  int e = -1;					/* current directory entry */
  char delim[2];				/* option option delimeter */
  time_t begintime, totaltime, hours, mins, secs;/* to measure process time */
  int i;
  int a;

  begintime = time(NULL);
  setbuf(stdout,(char *)NULL);			/* make stdout unbuffered */
  for (i = 0; i < numelm(epmp); i++) epmp[i] = TRUE;/* process all levels */
  printf(BANNER,40-(56+(int)strlen(SOFTDATE))/2,"",SOFTVER,SOFTDATE);
  for (a = 1; a < ac; a++) {			/* scan all arguments */
    if (av[a][0] == '-') {			/* optional argument */
      if (0 == strcmp(av[a],"-?"))
        show_help = TRUE;
      else if (0 == strcmp(av[a],"-help"))
        show_help = TRUE;
      else if (2 == sscanf(av[a],"-%*[Ee]%1d%*[Mm]%1d",epmp,epmp) ||
               2 == sscanf(av[a],"-%*[Mm]%*[Aa]%*[Pp]%1d%1d",epmp,epmp)) {
        char *lvlstr = &av[a][1];
        char *lvlmask = "%*[Ee]%1d%*[Mm]%1d-%*[Ee]%1d%*[Mm]%1d";
        int el, ml, eh, mh;
        if (*lvlstr == 'm' || *lvlstr == 'M')
          lvlmask = "%*[Mm]%*[Aa]%*[Pp]%1d%1d-%*[Mm]%*[Aa]%*[Pp]%1d%1d";
        for (i = 0; i < numelm(epmp); i++) epmp[i] = FALSE;
        while (lvlstr != NULL) {
          i = sscanf(lvlstr,lvlmask,&el,&ml,&eh,&mh);
          if (i == 4)
            for (i = el*10+ml; i <= eh*10+mh; i++)
              epmp[i] = TRUE;
          else if (i == 2)
            epmp[el*10+ml] = TRUE;
          else
            show_help = TRUE;
          lvlstr = strstr(lvlstr,",");
          if (lvlstr != NULL) lvlstr++;
        }
      }else if (0 == strcmp(av[a],"-n"))
        strcat(func,"nodes");
      else if (2 == sscanf(av[a],"-n%1[=:]%s",
                           delim,&node_flags[strlen(node_flags)])) {
        if (strspn(node_flags,"sam(0123456789,)") != strlen(node_flags))
          show_help = TRUE;
        else
          strcat(func,"nodes");
      }else if (0 == strcmp(av[a],"-b"))
        strcat(func,"blockmap");
      else if (0 == strcmp(av[a],"-r"))
        strcat(func,"reject");
      else if (2 == sscanf(av[a],"-r%1[=:]%s",delim,rxcpt_list))
        strcat(func,"reject");
      else if (0 == strcmp(av[a],"-z"))
        strcat(func,"reject"),
        rej_flags &= ~RG_REJDEPTH;		/* set depth to zero */
      else if (0 == strcmp(av[a],"-q"))
        strcat(node_flags,"s"),
        rej_flags |= RG_LIMIT_IO;
      else if (2 == sscanf(av[a],"-dmit%1[=:]%d",delim,&dmver))
        strcat(func,"dmit");
      else if (2 == sscanf(av[a],"-rand%1[=:]%ld",delim,&seed))
        strcat(func,"rand");
      else if (0 == strcmp(av[a],"-flip"))
        strcat(func,"flip");
      else if (3 <= sscanf(av[a],"-shift%1[=:]%d,%d,%d",delim,&dx,&dy,&dz))
        strcat(func,"shift");
      else if (0 == strcmp(av[a],"-pack"))
        strcat(func,"Pack");			/* keep capitalized */
      else if (0 == strcmp(av[a],"-unpack"))
        strcat(func,"unpack");
      else if (0 == strcmp(av[a],"-emstat"))
        strcat(func,"emstat");
      else if (2 == sscanf(av[a],"-ren%1[=:]%s",delim,rename_list))
        strcat(func,"rename");
      else if (0 == strcmp(av[a],"-dir"))
        strcat(func,"dir");
      else if (2 == sscanf(av[a],"-s%1[=:]%s",delim,subst_list))
        strcat(func,"substitute");
      else if (2 == sscanf(av[a],"-e%1[=:]%s",delim,extract_list))
        strcat(func,"extract");
      else if (2 == sscanf(av[a],"-m%1[=:]%s",delim,merge_list))
        strcat(func,"merge");
      else if (2 == sscanf(av[a],"-mraw%1[=:]%s",delim,mraw_list))
        strcat(func,"mraw");
      else if (2 == sscanf(av[a],"-otype%1[=:]%d",delim,&outtype))
        strcat(func,"otype");			/* this isn't in help */
      else
        show_help = TRUE;
    }else if (ifile == NULL)
      ifile = ofile = av[a];			/* get input filename */
    else
      ofile = av[a];				/* get output filename */
  }
  prog = strrchr(av[0],'\\');
  if (prog == NULL) prog = strrchr(av[0],'/');
  if (prog == NULL) prog = strrchr(av[0],']');
  if (prog != NULL) prog++;
  else              prog = av[0];
  if (strrchr(prog,'.') != NULL) *strrchr(prog,'.') = '\0';
  if (ifile == NULL || show_help) {		/* show now to do corectly */
    printf("\n%s [options...] <input file> [output file]\n\n",prog);
help("<input file>  PWAD, IWAD, or VERDA patch file");
help("[output file] output file; if none specified, input file is rewritten");
help("              ");
help("-<levels>     specify level(s) to process; <levels> is <lvl>[-<lvl>]");
help("              [,<lvl>[-<lvl>][,...]] where <lvl> is e#m# or map##");
help("-n[=<opt>]    build nodes; <opt> is 1 or both of 'a', 'm[(<S>[,...])]'");
help("              where <S> is sector #; if no sector #s specified applies");
help("              to all sectors; use these options for special effects");
help("-b            build packed blockmap");
help("-r[=<xlist>]  build reject; <xlist> is exception list of the form");
help("              [!]<S>-<S>[,...] where <S> is * (all sectors) or sector");
help("              #; 1 is forced (0 for leading !) for each sector pair;");
help("              invalid pairs are ignored");
help("-z            build zero-filled reject--much faster; same as -r=!*-*");
help("-q            quiet--suppress screen I/O during I/O-intensive process");
help("              ");
help("defaults to -n -b -r if none of the following options is specified;");
help("otherwise, performs specified function");
    pause();
help("-dmit=<#>     \"deathmatch-ize\" level for version 1 or 2 deathmatch");
help("-rand=<#>     randomly swap postions for creatures, weapons, bonuses,");
help("              and deathmatch starts using specified seed");
help("-flip         flip level about Y-axis (make mirror image)");
help("-shift=<dx,dy[,dz]> shift level by dx, dy, dz (useful with later merge");
help("              operation)");
help("-pack         remove redundant level resource data; do not do this if");
help("              intending further major editing");
help("-unpack       reverses pack function; use this if need to edit a");
help("              a packed WAD");
help("-emstat       display level statistics");
help("              ");
help("any combination of these options can specified with any combination");
help("of options on the previous page; dmit and rand will not work with a");
help("HERETIC or HEXEN WAD");
help("              \n\n\n\n\n\n\n");
    pause();
help("-ren=<rlist>  rename resources; <rlist> is <old>-<new>[,...]");
help("-dir          display entire resource directory");
help("-s=<wlist>    substitute resources in input file with resources in");
help("              comma-separated PWAD, IWAD, or VERDA patch files list");
help("-e=<rlist>    extract comma-separated named resources from input file");
help("              to output file; resources E#M#/MAP## get all data for");
help("              level; if starts with !, extract all resources not named");
help("-m=<wlist>    merge comma-separated PWAD, IWAD, or VERDA patch files");
help("              with input file: matching E#M#/MAP## level data is");
help("              merged (which needs new nodes, blockmap, reject), unique");
help("              resources are added");
help("-mraw=<flist> merge comma-separated raw/lump files with input file;");
help("              <flist> is <rfile>-<name>[,...]: each <rfile> is given");
help("              resource name <name> as it is merged");
help("              ");
help("dir, substitute, extract, merge, and raw merge are mutually exclusive");
help("with all other functions and ignore any -e#m# or -map## option;");
help("extract and merge require an output file different from the input file");
help("              ");
    printf(BANNER,40-(57+(int)strlen(SOFTDATE))/2,"",SOFTVER,SOFTDATE);
    return 1;
  }
  if ((strstr(func,"dir") != NULL && strcmp(func,"dir") != 0) ||
      (strstr(func,"substitute") != NULL && strcmp(func,"substitute") != 0) ||
      (strstr(func,"extract") != NULL && strcmp(func,"extract") != 0) ||
      (strstr(func,"merge") != NULL && strcmp(func,"merge") != 0) ||
      (strstr(func,"mraw") != NULL && strcmp(func,"mraw") != 0)) {
    if (strlen(subst_list) > 0)        strcpy(func,"substitute");
    else if (strlen(extract_list) > 0) strcpy(func,"extract");
    else if (strlen(merge_list) > 0)   strcpy(func,"merge");
    else if (strlen(mraw_list) > 0)    strcpy(func,"mraw");
    else                               strcpy(func,"dir");
    fprintf(stderr,
            "%s function is mutually exclusive with other functions\n",func);
    return 1;
  }
  if ((strstr(func,"extract") != NULL ||
       strstr(func,"merge") != NULL || strstr(func,"mraw") != NULL)
      && strcmp(ifile,ofile) == 0) {
    fprintf(stderr,"output file must be different from input file for ");
    fprintf(stderr,"extract or merge functions\n");
    return 1;
  }
  if (strlen(func) == 0)			/* if no specified funcs do  */
    strcpy(func,"nodes,blockmap,reject");	/* default of building these */
  iwad = wad_open(ifile,TRUE,FALSE);		/* open input file */
  if (iwad == NULL) {
    fprintf(stderr,"unable to open %s for reading\n",ifile);
    return 1;
  }
  if (iwad->type == 0) {			/* not a valid file */
    fprintf(stderr,"%s is not a PWAD, IWAD, nor VERDA patch file\n",ifile);
    return 1;
  }
  printf("\nReading %s file %s...",
         iwad->type==DOOM_FILE?"DOOM/HERETIC":
         iwad->type==HEXEN_FILE?"HEXEN":"patch",
         ifile);
  for (a = 0, i = 0; i < iwad->head.count; i++)	/* count # levels */
    if (2==sscanf(iwad->dir[i].name,"E%dM%d",&iwad->ep,&iwad->mp) ||
        2==sscanf(iwad->dir[i].name,"MAP%1d%1d",&iwad->ep,&iwad->mp))
      a++;
  printf("%d level%s\n",a,a==1?"":"s");
  if (strstr(func,"dir") != NULL) {
    printf("Directory for %s...\n",ifile);
    directory(iwad);
  }
  else if (strstr(func,"substitute") != NULL) {
    printf("Substituting resources in %s with...",ifile);
    good = substitute(iwad,subst_list);
    printf(good?"done\n":"failed\n");
  }
  else if (strstr(func,"extract") != NULL) {
    printf("Extracting from %s...",ifile);
    good = extract(iwad,extract_list);
    printf(good?"done\n":"failed\n");
  }
  else if (strstr(func,"merge") != NULL) {
    printf("Merging %s with...",ifile);
    good = merge(iwad,merge_list);
    printf(good?"done\n":"failed\n");
  }
  else if (strstr(func,"mraw") != NULL) {
    printf("Merging %s with raw data...",ifile);
    good = merge_raw(iwad,mraw_list);
    printf(good?"done\n":"failed\n");
  }
  else do {					/* process file until done */
    good = FALSE;
    for (e++; e < iwad->head.count; e++)	/* find next map level */
      if ((2==sscanf(iwad->dir[e].name,"E%dM%d",&iwad->ep,&iwad->mp) ||
           2==sscanf(iwad->dir[e].name,"MAP%1d%1d",&iwad->ep,&iwad->mp))
          &&
          epmp[iwad->ep*10+iwad->mp]) {		/* level is marked "process" */
        good = wad_read(iwad,e,RESOURCES_NEEDED);
        break;
      }
    if (good) {					/* process new level data */
      if (strstr(func,"nodes") != NULL) {
        WAD_LINE *lines = Lines;
        WAD_SIDE *sides = Sides;
        WAD_VERT *verts = Verts;
        WAD_SEGS *segs = Segs;
        WAD_SSECTOR *ssecs = Ssecs;
        WAD_NODE *nodes = Nodes;
        printf("Building BSP for level %s...",iwad->dir[e].name);
        nodes_make(&nodes,&NNodes,&ssecs,&NSsecs,&segs,&NSegs,&verts,&NVerts,
                   &lines,&NLines,&sides,node_flags);
        resource_update(iwad,e+VERTS,verts,NVerts);/* vertices modified */
        resource_update(iwad,e+SEGS,segs,NSegs);/* just created these three */
        resource_update(iwad,e+SSECTS,ssecs,NSsecs);
        resource_update(iwad,e+NODES,nodes,NNodes);
        printf("%ld nodes, %ld segs          \n",NNodes,NSegs);
      }
      if (strstr(func,"blockmap") != NULL) {
        WAD_BLOCKMAP *blockmaps = Blockmaps;
        printf("Building BLOCKMAP for level %s...",iwad->dir[e].name);
        NBlockmaps = blockmap_make(&blockmaps,Lines,NLines,Verts);
        resource_update(iwad,e+BLKMAPS,blockmaps,NBlockmaps);
        printf("%d x %d blocks\n",blockmaps[2],blockmaps[3]);
      }
      if (strstr(func,"reject") != NULL) {
        WAD_REJECT *rejects = Rejects;
        long i, j, ones = 0;
        printf("Building REJECT for level %s...",iwad->dir[e].name);
        if (Blockmaps == NULL) wad_read(iwad,e,1<<BLKMAPS);
        NRejects = reject_make(&rejects,rej_flags,rxcpt_list,
                               Lines,NLines,Sides,Verts,Blockmaps);
        resource_update(iwad,e+REJECTS,rejects,NRejects);
        for (i = 0; i < NRejects; i++)		/* print "zeroed" or not */
          for (j = 0; j < numbits(rejects[0]); j++)
            if (i*numbits(rejects[0])+j < NSects*NSects &&
                rejects[i] & (1<<j)) ones++;
        if (ones) printf("%4.1f%%\n",100.0*ones/(NSects*NSects));
        else      printf("zeroed\n");
      }
      if (strstr(func,"dmit") != NULL) {
        printf("Deathmatch-izing (V%d.0) level %s...",
               dmver,iwad->dir[e].name);
        dmit(dmver,Things,NThings,Sects,NSects);
        resource_update(iwad,e+THINGS,Things,NThings);
        resource_update(iwad,e+SECTS,Sects,NSects);
        printf("done\n");
      }
      if (strstr(func,"rand") != NULL) {
        printf("Randomizing things on level %s...",iwad->dir[e].name);
        rand_thing(seed,Things,NThings);
        resource_update(iwad,e+THINGS,Things,NThings);
        printf("done\n");
      }
      if (strstr(func,"flip") != NULL) {
        WAD_BLOCKMAP *blockmaps;		/* new BLOCKMAP for flip */
        printf("Flipping level %s...",iwad->dir[e].name);
        flip(Things,NThings,Verts,NVerts,Lines,NLines,Nodes,NNodes,Segs,NSegs);
        NBlockmaps = blockmap_make(&blockmaps,Lines,NLines,Verts);
        resource_update(iwad,e+THINGS,Things,NThings);
        resource_update(iwad,e+VERTS,Verts,NVerts);
        resource_update(iwad,e+LINES,Lines,NLines);
        resource_update(iwad,e+NODES,Nodes,NNodes);
        resource_update(iwad,e+SEGS,Segs,NSegs);
        resource_update(iwad,e+BLKMAPS,blockmaps,NBlockmaps);
        printf("done\n");
      }
      if (strstr(func,"shift") != NULL) {
        WAD_BLOCKMAP *blockmaps;		/* new BLOCKMAP for shift */
        printf("Shifting level %s...",iwad->dir[e].name);
        shift(dx,dy,dz,Things,NThings,Verts,NVerts,Nodes,NNodes,Sects,NSects);
        NBlockmaps = blockmap_make(&blockmaps,Lines,NLines,Verts);
        resource_update(iwad,e+THINGS,Things,NThings);
        resource_update(iwad,e+VERTS,Verts,NVerts);
        resource_update(iwad,e+NODES,Nodes,NNodes);
        resource_update(iwad,e+SECTS,Sects,NSects);
        resource_update(iwad,e+BLKMAPS,blockmaps,NBlockmaps);
        printf("done\n");
      }
      if (strstr(func,"Pack") != NULL) {
        printf("Packing level %s...",iwad->dir[e].name);
        pack(iwad,e);
        printf("done\n");
      }
      if (strstr(func,"unpack") != NULL) {
        printf("Unpacking level %s...",iwad->dir[e].name);
        unpack(iwad,e);
        printf("done\n");
      }
      if (strstr(func,"emstat") != NULL) {
        printf("Statistics for level %s...\n",iwad->dir[e].name);
        emstat(iwad,e);
      }
    }
  } while (good);
  if (strstr(func,"rename") != NULL) {
    printf("Renaming resources...");
    good = rename_resources(iwad,rename_list);
    printf(good?"done\n":"failed\n");
  }
  for (e = 0; e < iwad->head.count; e++)	/* see if anything has been */
    if (iwad->changed[e] == TRUE) break;	/* modified                 */
  if (e < iwad->head.count) {			/* have new data to write */
    if ((owad = wad_open(ofile,FALSE,strcmp(ifile,ofile)==0)) == NULL) {
      fprintf(stderr,"unable to open %s for writing\n",ofile);
      return 1;
    }
    owad->type = iwad->type;			/* set output file type */
    if (outtype && strcmp(ifile,ofile)) owad->type = outtype;
    printf("Writing %s file %s...",
           owad->type==DOOM_FILE?"DOOM/HERETIC":
           owad->type==HEXEN_FILE?"HEXEN":"patch",
           ofile);
    good = wad_write(owad,iwad);
    printf(good?"done\n":"failed\n");
    wad_close(owad);
  }
  wad_close(iwad);
  if (strstr(func,"nodes") != NULL ||		/* display processing time */
      strstr(func,"reject") != NULL ||		/* for compute intensive   */
      strstr(func,"Pack") != NULL ||		/* operations              */
      strstr(func,"unpack") != NULL) {
    totaltime = time(NULL) - begintime;
    hours = totaltime/3600;
    mins = (totaltime-(hours*3600))/60;
    secs = totaltime-(hours*3600)-(mins*60);
    printf("Processing completed in ");
    if (hours) printf("%d hour%s, ",(int)hours,hours==1?"":"s");
    if (hours || mins) printf("%d minute%s, ",(int)mins,mins==1?"":"s");
    printf("%d second%s\n",(int)secs,secs==1?"":"s");
  }
  return 0;					/* everything is okay */
}


/******************************************************************************
	ROUTINE:	dmit(ver,things,nthings,sects,nsects)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	Mar. 1994
	DESCRIPTION:	This routine deathmatch-izes the THINGS and SECTORS
			resources.  This won't work very well with a HERETIC
			nor HEXEN WAD.
******************************************************************************/
#if defined(ANSI_C)
int dmit(int ver,WAD_THING *things,long nthings,WAD_SECTOR *sects,
         long nsects)
#else
int dmit(ver,things,nthings,sects,nsects)
int ver;
WAD_THING *things;
long nthings;
WAD_SECTOR *sects;
long nsects;
#endif
{
  static int subst1[][2] = {			/* ver 1 item substitutions */
	{0x0002, 0x07E6},			/* player 2 --> invulner sph */
	{0x0003, 0x07E7},			/* player 3 --> berzerk pk */
	{0x0004, 0x07E8},			/* player 4 --> blur sphere */
	{0x0005, 0x07DD},			/* blue key --> soul sphere */
	{0x0006, 0x07E8},			/* yellow key --> blur sph */
	{0x000A, 0x07DD},			/* explod being --> soul sph */
	{0x000C, 0x07DD},			/* explod being --> soul sph */
	{0x000D, 0x07E6},			/* red key --> invulner sph */
	{0x000F, 0x0008},			/* dead marine --> backpack */
	{0x0012, 0x07E3},			/* dead fmr hu --> cmbt arm */
	{0x0013, 0x07E6},			/* dead fmr sa --> invl sph */
	{0x0014, 0x07E2},			/* dead fmr sa --> sec armor */
	{0x0015, 0x07E8},			/* dead demon --> blur sph */
	{0x0016, 0x07E7},			/* dead caco --> berzerk pk */
	{0x0017, 0x07E7},			/* dead skull --> berzerk pk */
	{0x0018, 0x0008},			/* bloody goo --> backpack */
	{0x0022, 0x07DB},			/* candle --> stimpak */
	{0x0023, 0x07F3},			/* candelabra --> barrel */
	{0x0026, 0x0011},			/* red skull ky --> ener pk */
	{0x0027, 0x0011},			/* yel skull ky --> ener pk */
	{0x0028, 0x0011},			/* blu skull ky --> ener pk */
	{0x07D7, 0x0800},			/* clip --> box of ammo */
	{0x07D8, 0x0801},			/* shells --> box of shells */
	{0x07DA, 0x07FE},			/* rocket --> box o rockets */
	{0x07DE, 0x07DB},			/* bonus health --> stimpak */
	{0x07DF, 0x07E2},			/* bonus armor --> armor */
	{0x07E9, 0x07E3},			/* rad suit --> combat arm */
	{0x07EA, 0x07DD},			/* map --> soul sphere */
	{0x07EC, 0x07DB},			/* light post --> stimpak */
	{0x07FF, 0x0011}			/* energy cell --> ener pk */
  };
  static int subst2[][2] = {			/* ver 1 item substitutions */
	{0x0002, 0x07E6},			/* player 2 --> invulner sph */
	{0x0003, 0x0008},			/* player 3 --> backpack */
	{0x0004, 0x07E8},			/* player 4 --> blur sphere */
	{0x0005, 0x07DD},			/* blue key --> soul sphere */
	{0x0006, 0x07E8},			/* yellow key --> blur sph */
	{0x000A, 0x07DF},			/* explod being --> armor hl */
	{0x000C, 0x07DF},			/* explod being --> armor hl */
	{0x000D, 0x07E6},			/* red key --> invulner sph */
	{0x000F, 0x07DF},			/* dead marine --> armor hel */
	{0x0012, 0x07DF},			/* dead fmr hu --> armor hel */
	{0x0013, 0x07DF},			/* dead fmr sa --> armor hel */
	{0x0014, 0x07DF},			/* dead fmr sa --> armor hel */
	{0x0015, 0x07DF},			/* dead demon --> armor hel */
	{0x0016, 0x07DF},			/* dead caco --> armor hel */
	{0x0017, 0x07DF},			/* dead skull --> armor hel */
	{0x0018, 0x07DF},			/* bloody goo --> armor hel */
	{0x0026, 0x07DF},			/* red skull ky --> armor hl */
	{0x0027, 0x07DF},			/* yel skull ky --> armor hl */
	{0x0028, 0x07DF},			/* blu skull ky --> armor hl */
	{0x07DB, 0x07DE},			/* stimpak --> health pot */
	{0x07DC, 0x07DE},			/* health kit --> hlth pot */
	{0x07E9, 0x07E3},			/* rad suit --> combat arm */
	{0x07EA, 0x07DD}			/* map --> soul sphere */
  };
  int health = 0;
  int caco = 0, troop = 0, sarg = 0, skull = 0;
  int chainsaw = 0, chaingun = 0, plasma = 0, launcher = 0, bfg = 0;
  int i, s;

  for (i = 0; i < nthings; i++) {
    things[i].flag = 0x07;			/* all skills */
    if (ver == 1) {
      for (s = 0; s < numelm(subst1); s++)	/* do ver 1 substitutions */
        if (things[i].item == subst1[s][0])
          things[i].item = subst1[s][1];
    }else {
      for (s = 0; s < numelm(subst2); s++)	/* do ver 2 substitutions */
        if (things[i].item == subst2[s][0])
          things[i].item = subst2[s][1];
      if (things[i].item == 0x07DE)
        if (health++ < 5) things[i].item = 0x07DC;
    }
    if (is_creature(things[i]))
    {
      if      (caco++ < 2)   things[i].item = 0x0BBD;
      else if (troop++ < 15) things[i].item = 0x0BBC;
      else if (sarg++  < 10) things[i].item = 0x0009;
      else if (skull++ < 15) things[i].item = 0x0BBE;
      else                   things[i].item = ver==1? 0x07DC : 0x07DE;
    }
    if (is_weapon(things[i]))
    {
      if      (chainsaw < 1) chainsaw++, things[i].item = 0x07D5;
      else if (chaingun < 1) chaingun++, things[i].item = 0x07D2;
      else if (plasma   < 1) plasma++,   things[i].item = 0x07D4;
      else if (chaingun < 2) chaingun++, things[i].item = 0x07D2;
      else if (plasma   < 2) plasma++,   things[i].item = 0x07D4;
      else if (launcher < 1) launcher++, things[i].item = 0x07D3;
      else if (bfg      < 1) bfg++,      things[i].item = 0x07D6;
      else                               things[i].item = 0x07D1;
    }
  }
  for (i = 0; i < nsects; i++) {
    if (sects[i].property == 0x07) {		/* remove "acid" */
      sects[i].property = 0x00;
      strcpy(sects[i].floor_desc,"FLAT1_1");
    }else if (sects[i].property == 0x05) {	/* remove health hit */
      sects[i].property = 0x00;
      strcpy(sects[i].floor_desc,"FWATER1");
    }
  }
  return TRUE;
}


/******************************************************************************
	ROUTINE:	rand_xy_sort(thing1,thing2)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	June 1994
	DESCRIPTION:	This routine is the comparison function for qsort().
			It orders the things by their coordinates.
******************************************************************************/
#if defined(ANSI_C)
int _Optlink rand_xy_sort(const void *thing1,
                          const void *thing2)
#else
local int rand_xy_sort(thing1,thing2)
WAD_THING *thing1, *thing2;
#endif
{
  WAD_THING *t1 = (WAD_THING *)thing1;
  WAD_THING *t2 = (WAD_THING *)thing2;
  int xdel = sgn(t1->x - t2->x);
  int ydel = sgn(t1->y - t2->y);

  return xdel != 0 ? xdel : ydel;
}


/******************************************************************************
	ROUTINE:	rand_item_sort(thing1,thing2)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	June 1994
	DESCRIPTION:	This routine is the comparison function for qsort().
			It orders the things by their item types.
******************************************************************************/
#if defined(ANSI_C)
int _Optlink rand_item_sort(const void *thing1,
                            const void *thing2)
#else
local int rand_item_sort(thing1,thing2)
WAD_THING *thing1, *thing2;
#endif
{
  WAD_THING *t1 = (WAD_THING *)thing1;
  WAD_THING *t2 = (WAD_THING *)thing2;

  return t1->item - t2->item;
}


/******************************************************************************
	ROUTINE:	rand_thing(seed,things,nthings)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	Mar. 1994
	DESCRIPTION:	This routine randomizes the order of the things in
			the THINGS resource.  It sorts the things by item type
			and coordinates first so that the same seed will
			always produce the same new order regardless of the
			original input order.  Deathmatch starts and all
			"furniture" (barrels, lamps, etc) are never moved.
			This won't work with a HERETIC nor HEXEN WAD.
******************************************************************************/
#if defined(ANSI_C)
int rand_thing(long seed, WAD_THING *things, long nthings)
#else
int rand_thing(seed,things,nthings)
long seed;
WAD_THING *things;
long nthings;
#endif
{
  int t1, t2;
  WAD_THING temp;
  WAD_THING *items;
  int i, t;

  qsort((char *)things,(int)nthings,sizeof *things,rand_xy_sort);
  items = blockmem(WAD_THING,nthings);
  for (t = 0; t < nthings; t++) items[t] = things[t];
  qsort((char *)items,(int)nthings,sizeof *items,rand_item_sort);
  randseed((unsigned int)seed);
  for (t = 0; t < nthings; t++) {		/* scramble order here */
    t1 = nthings*randnum(), t2 = nthings*randnum();
    temp = things[t1], things[t1] = things[t2], things[t2] = temp;
    t1 = nthings*randnum(), t2 = nthings*randnum();
    temp = items[t1], items[t1] = items[t2], items[t2] = temp;
  }
  for (i = 0, t = 0; t < nthings; t++)
    if (!is_deathmatch(things[t]) && !is_furniture(things[t])) {
      while (is_deathmatch(items[i]) || is_furniture(items[i]))
        i++;
      things[t].item = items[i  ].item;		/* swapping gets new */
      things[t].flag = items[i++].flag;		/* (x,y) for thing   */
    }
  blockfree(items);
  return TRUE;
}


/******************************************************************************
	ROUTINE:	flip(things,nthings,verts,nverts,lines,nlines,
			     nodes,nnodes,segs,nsegs)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	Apr. 1994
	DESCRIPTION:	This routine flips the level about the Y axis, so
			effectively reverses the level as far as the player is
			concerned.  Note that flipping about the X axis does
			the same thing from the player's persective.
******************************************************************************/
#if defined(ANSI_C)
int flip(WAD_THING *things, long nthings, WAD_VERT *verts, long nverts,
         WAD_LINE *lines, long nlines, WAD_NODE *nodes, long nnodes,
         WAD_SEGS *segs, long nsegs)
#else
int flip(things,nthings,verts,nverts,lines,nlines,nodes,nnodes,segs,nsegs)
WAD_THING *things;
long nthings;
WAD_VERT *verts;
long nverts;
WAD_LINE *lines;
long nlines;
WAD_NODE *nodes;
long nnodes;
WAD_SEGS *segs;
long nsegs;
#endif
{
  unsigned short tang;
  short temp;
  int i;

  for (i = 0; i < nthings; i++) {		/* negate X coordinate */
    things[i].x = -things[i].x;
    things[i].angle = (540 - things[i].angle) % 360;
  }
  for (i = 0; i < nverts; i++)			/* negate X coordinate */
    verts[i].x = -verts[i].x;
  for (i = 0; i < nlines; i++) {		/* swap from and to vertices */
    temp = lines[i].fndx;
    lines[i].fndx = lines[i].tndx;
    lines[i].tndx = temp;
  }
  for (i = 0; i < nnodes; i++) {
    nodes[i].x = -nodes[i].x;			/* negate X coordinate */
    nodes[i].xdel = -nodes[i].xdel;		/* negate X offset */
    temp = -nodes[i].rxmax;			/* swap right and left  */
    nodes[i].rxmax = -nodes[i].lxmin;		/* bounding box X       */
    nodes[i].lxmin = temp;			/* coordinates: min for */
    temp = -nodes[i].rxmin;			/* max and negate       */
    nodes[i].rxmin = -nodes[i].lxmax;
    nodes[i].lxmax = temp;
    temp = nodes[i].rymax;			/* swap right and left */
    nodes[i].rymax = nodes[i].lymax;		/* bounding box Y      */
    nodes[i].lymax = temp;			/* min/max coordinates */
    temp = nodes[i].rymin;
    nodes[i].rymin = nodes[i].lymin;
    nodes[i].lymin = temp;
    temp = nodes[i].nndx[0];			/* swap node subtrees */
    nodes[i].nndx[0] = nodes[i].nndx[1];
    nodes[i].nndx[1] = temp;
  }
  for (i = 0; i < nsegs; i++) {
    temp = segs[i].tndx;			/* swap from and to vertices */
    segs[i].tndx = segs[i].fndx;
    segs[i].fndx = temp;
    tang = -segs[i].angle;			/* negate angle */
    segs[i].angle = tang;
  }
  return TRUE;
}


/******************************************************************************
	ROUTINE:	shift(dx,dy,dz,things,nthings,verts,nverts,
			      nodes,nnodes,sects,nsects)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	Apr. 1994
	DESCRIPTION:	This routine shifts the level by dx, dy, dz.
******************************************************************************/
#if defined(ANSI_C)
int shift(int dx,int dy,int dz,WAD_THING *things, long nthings,
          WAD_VERT *verts, long nverts, WAD_NODE *nodes, long nnodes,
          WAD_SECTOR *sects, long nsects)
#else
int shift(dx,dy,dz,things,nthings,verts,nverts,nodes,nnodes,sects,nsects)
int dx, dy, dz;
WAD_THING *things;
long nthings;
WAD_VERT *verts;
long nverts;
WAD_NODE *nodes;
long nnodes;
WAD_SECTOR *sects;
long nsects;
#endif
{
  int i;

  for (i = 0; i < nthings; i++)			/* shift things coordinates */
    things[i].x += dx, things[i].y += dy;
  for (i = 0; i < nverts; i++)			/* shift vertices coords */
    verts[i].x += dx, verts[i].y += dy;
  for (i = 0; i < nnodes; i++) {		/* shift nodes coordinates */
    nodes[i].x += dx, nodes[i].y += dy,
    nodes[i].rxmin += dx, nodes[i].rymin += dy,
    nodes[i].rxmax += dx, nodes[i].rymax += dy,
    nodes[i].lxmin += dx, nodes[i].lymin += dy,
    nodes[i].lxmax += dx, nodes[i].lymax += dy;
  }
  for (i = 0; i < nsects; i++) {		/* shift sector heights */
    sects[i].floor_ht += dz;
    sects[i].ceil_ht += dz;
  }
  return TRUE;
}


/******************************************************************************
	ROUTINE:	pack(iwad,e)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	May  1995
	DESCRIPTION:	This routine packs the input level by removing
			redundant data from the SIDEDEFS and VERTEXES
			resources, and by generating a new, packed BLOCKMAP.
			This routine should also remove any unused SIDEDEFS,
			VERTEXES, and SECTORS, but it currently does not.
	MODIFIED:		Robert Fenske, Jr.	Dec. 1995
			SIDEDEFS on one-sided LINEDEFS that have non-zero
			sector tags are always kept.
******************************************************************************/
#if defined(ANSI_C)
int pack(WAD_INFO *iwad, int e)
#else
int pack(iwad,e)
WAD_INFO *iwad;
int e;
#endif
{
  WAD_BLOCKMAP *blockmaps = Blockmaps;
  char *keep_side;
  int i, j, l;

  for (i = 0; i < NSides; i++) {		/* remove any trailing   */
    for (j = strlen(Sides[i].uwall); j < sizeof(Sides[i].uwall); j++)
      Sides[i].uwall[j] = '\0';			/* junk in texture names */
    for (j = strlen(Sides[i].lwall); j < sizeof(Sides[i].lwall); j++)
      Sides[i].lwall[j] = '\0';
    for (j = strlen(Sides[i].nwall); j < sizeof(Sides[i].nwall); j++)
      Sides[i].nwall[j] = '\0';
  }
  keep_side = blockmem(char,NSides);
  for (l = 0; l < NLines; l++)			/* flag sides on 1-sided    */
    if (Lines[l].lsidndx == -1 &&		/* lines with non-zero tags */
        Lines[l].sect_tag != 0)			/* as not removable         */
      keep_side[Lines[l].rsidndx] = TRUE;
  for (i = 0; i < NSides-1; i++)		/* remove duplicate SIDEDEFS */
    for (j = i+1; j < NSides; j++)
      if (!keep_side[j] &&
          memcmp(&Sides[i],&Sides[j],sizeof Sides[i]) == 0) {/* found dup */
        for (l = j+1; l < NSides; l++)
          Sides[l-1] = Sides[l];
        NSides--;
        for (l = 0; l < NLines; l++) {		/* adjust references */
          if (Lines[l].rsidndx == j) Lines[l].rsidndx = i;
          if (Lines[l].lsidndx == j) Lines[l].lsidndx = i;
          if (Lines[l].rsidndx > j) Lines[l].rsidndx--;
          if (Lines[l].lsidndx > j) Lines[l].lsidndx--;
        }
        j--;
      }
  for (i = 0; i < NVerts; i++)			/* remove deplicate VERTEXES */
    for (j = i+1; j < NVerts; j++)
      if (memcmp(&Verts[i],&Verts[j],sizeof Verts[i]) == 0) {/* found dup */
        for (l = j+1; l < NVerts; l++)
          Verts[l-1] = Verts[l];
        NVerts--;
        for (l = 0; l < NLines; l++) {		/* adjust references */
          if (Lines[l].fndx == j) Lines[l].fndx = i;
          if (Lines[l].tndx == j) Lines[l].tndx = i;
          if (Lines[l].fndx > j) Lines[l].fndx--;
          if (Lines[l].tndx > j) Lines[l].tndx--;
        }
        for (l = 0; l < NSegs; l++) {		/* adjust references */
          if (Segs[l].fndx == j) Segs[l].fndx = i;
          if (Segs[l].tndx == j) Segs[l].tndx = i;
          if (Segs[l].fndx > j) Segs[l].fndx--;
          if (Segs[l].tndx > j) Segs[l].tndx--;
        }
      }
  NBlockmaps = blockmap_make(&blockmaps,Lines,NLines,Verts);/* new BLOCKMAP */
  resource_update(iwad,e+LINES,Lines,NLines);
  resource_update(iwad,e+SIDES,Sides,NSides);
  resource_update(iwad,e+VERTS,Verts,NVerts);
  resource_update(iwad,e+BLKMAPS,blockmaps,NBlockmaps);
  return TRUE;
}


/******************************************************************************
	ROUTINE:	unpack(iwad,e)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	May  1995
	DESCRIPTION:	This routine unpacks the input level, essentially
			reversing the packing processing (see pack()).  The
			only resources this really affects is the LINEDEFS
			and SIDEDEFS.  This unpacking involves making sure that
			each LINEDEF refers to a unique SIDEDEF.  SIDEDEFS
			are copied as necessary to achieve this.
******************************************************************************/
#if defined(ANSI_C)
int unpack(WAD_INFO *iwad, int e)
#else
int unpack(iwad,e)
WAD_INFO *iwad;
int e;
#endif
{
  char *sidemark = blockmem(char,2*NLines);	/* keep track of used sides */
  WAD_SIDE *newsides = blockmem(WAD_SIDE,2*NLines);
  int l;

  blockcopy(newsides,Sides,NSides*sizeof Sides[0]);
  for (l = 0; l < NLines; l++) {		/* scan all LINEDEFS */
    if (sidemark[Lines[l].rsidndx]) {		/* need to duplicate side */
      blockcopy(&newsides[NSides],&Sides[Lines[l].rsidndx],/* make copy */
                sizeof Sides[Lines[l].rsidndx]);
      Lines[l].rsidndx = NSides++;		/* new side is here */
    }
    sidemark[Lines[l].rsidndx] = 1;
    if (Lines[l].lsidndx != -1) {
      if (sidemark[Lines[l].lsidndx]) {		/* need to duplicate side */
        blockcopy(&newsides[NSides],&Sides[Lines[l].lsidndx],/* make copy */
                  sizeof Sides[Lines[l].lsidndx]);
        Lines[l].lsidndx = NSides++;		/* new side is here */
      }
      sidemark[Lines[l].lsidndx] = 1;
    }
  }
  resource_update(iwad,e+LINES,Lines,NLines);
  resource_update(iwad,e+SIDES,newsides,NSides);
  blockfree(sidemark);				/* done with this */
  return TRUE;
}


/******************************************************************************
	ROUTINE:	node_tree_walk(node,nndx,depth_cur,
			               depth_max,depth_sum,depth_bal)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	June 1995
	DESCRIPTION:	This routine performs an in-order traversal of the
			input node tree.  The depth_cur (current depth of the
			input node), depth_max (maximum depth encountered so
			far), depth_sum (runing sum of the depths of the left
			and right subtrees), and depth_bal (ratio of the
			left and right subtree sums) inputs are updated as
			appropriate.
******************************************************************************/
#if defined(ANSI_C)
void _Optlink node_tree_walk(WAD_NODE *node, int nndx, int depth_cur,
                             int *depth_max, int *depth_sum, int *depth_bal)
#else
local void node_tree_walk(node,nndx,depth_cur,depth_max,depth_sum,depth_bal)
WAD_NODE *node;
int nndx;
int depth_cur;
int *depth_max, *depth_sum, *depth_bal;
#endif
{
  int sumr = 0, suml = 0;

  if (!(nndx & 0x8000)) {			/* more tree below */
    node_tree_walk(node,node[nndx].nndx[0],
                   depth_cur+1,depth_max,&sumr,depth_bal);
    node_tree_walk(node,node[nndx].nndx[1],
                   depth_cur+1,depth_max,&suml,depth_bal);
    *depth_sum += sumr + suml;
    *depth_bal += 1000-1000*abs(sumr-suml)/(sumr+suml);
  }else {					/* hit a subsector */
    if (++depth_cur > *depth_max) *depth_max = depth_cur;
    *depth_sum += depth_cur;
  }
}


/******************************************************************************
	ROUTINE:	emstat(iwad,e)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	June 1994
	DESCRIPTION:	This routine displays statistics about the input level.
			The THINGS breakdown is not relevant for HERETIC nor
			HEXEN.
******************************************************************************/
#if defined(ANSI_C)
int emstat(WAD_INFO *iwad, int e)
#else
int emstat(iwad,e)
WAD_INFO *iwad;
int e;
#endif
{
  DIR_ENTRY *dir = &iwad->origdir[e];
  int mlv = -1;					/* maximum line vertex */
  short xmin, xmax, ymin, ymax;
  short x, y;
  int one_sided = 0;				/* one-sided line count */
  int creature = 0,				/* creature count */
      weapon = 0,				/* weapon count */
      key = 0,					/* key count */
      health = 0,				/* health item count */
      armor = 0,				/* armor item count */
      dm = 0;					/* deathmatch start count */
  int depth_sum = 0,				/* node tree depth sum */
      depth_max = 0,				/* node tree max depth */
      depth_bal = 0;				/* tree balance sum*1000 */
  int secret = 0,				/* secret sector count */
      special = 0,				/* special line/sector count */
      tagged = 0;				/* tagged line/sector count */
  int ones = 0;					/* # one bits in reject */
  int packed = 0;				/* packed blockmap flag */
  int i, j;

  for (i = 0; i < ALL(iwad->type); i++)		/* get any missing resources */
    if (iwad->data[e+i] == NULL) (void)wad_read(iwad,e,1L<<i);
  xmin = ymin = (short)0x7FFF, xmax = ymax = (short)0x8000;
  for (i = 0; i < NLines; i++) {		/* find map min,max x,y */
    x = Verts[Lines[i].fndx].x, y = Verts[Lines[i].fndx].y;
    if (x < xmin) xmin = x;
    if (y < ymin) ymin = y;
    if (xmax < x) xmax = x;
    if (ymax < y) ymax = y;
    x = Verts[Lines[i].tndx].x, y = Verts[Lines[i].tndx].y;
    if (x < xmin) xmin = x;
    if (y < ymin) ymin = y;
    if (xmax < x) xmax = x;
    if (ymax < y) ymax = y;
    if (mlv < Lines[i].fndx) mlv = Lines[i].fndx;/* find highest line vertex */
    if (mlv < Lines[i].tndx) mlv = Lines[i].tndx;
    one_sided += Lines[i].lsidndx == -1;
  }
  printf("\tMap...........(%d,%d) to (%d,%d)\n",xmin,ymin,xmax,ymax);
  for (i = 0; i < NThings; i++) {		/* count some special things */
    if (is_creature(Things[i]))        creature++;
    else if (is_weapon(Things[i]))     weapon++;
    else if (is_key(Things[i]))        key++;
    else if (is_health(Things[i]))     health++;
    else if (is_armor(Things[i]))      armor++;
    else if (is_deathmatch(Things[i])) dm++;
  }
  printf("\tThings........%ld: %d %s, %d %s, %d %s, %d %s,\n",
         NThings,creature,"creatures",weapon,"weapons",key,"keys",
         health,"health");
  printf("\t              %*s  %d %s, %d %s (%ld bytes)\n",
         (int)(log10((double)max(NThings,1))+1),"",
         armor,"armor",dm,"dm starts",dir[THINGS].nbytes);
  for (i = 0; i < NLines; i++) {		/* count some special lines */
    if (Lines[i].action_flag != 0) special++;
    if (Lines[i].sect_tag != 0) tagged++;
  }
  printf("\tLines.........%ld: %d 1-sided, %ld 2-sided\n",
         NLines,one_sided,NLines-one_sided);
  printf("\t              %*s  %d special, %d tagged (%ld bytes)\n",
         (int)(log10((double)max(NLines,1))+1),"",
         special,tagged,dir[THINGS].nbytes);
  printf("\tSides.........%ld",NSides);
  if (NSides < 2*NLines-one_sided)
    printf(", packed by %ld%%",100L-100L*NSides/(2*NLines-one_sided));
  printf(" (%ld bytes)\n",dir[SIDES].nbytes);
  printf("\tVertices......%ld: %d for lines, %ld for segs (%ld bytes)\n",
         NVerts,mlv+1,NVerts-(mlv+1),dir[VERTS].nbytes);
  for (j = 0, i = 0; i < NSegs; i++) {		/* count # splits by      */
    if (Segs[i].fndx > mlv) j++;		/* counting # vertices >  */
    if (Segs[i].tndx > mlv) j++;		/* highest LINEDEF vertex */
  }
  printf("\tSegs..........%ld: %d from splits (%ld bytes)\n",
         NSegs,j/2,dir[SEGS].nbytes);
  printf("\tSubsectors....%ld (%ld bytes)\n",NSsecs,dir[SSECTS].nbytes);
  node_tree_walk(Nodes,NNodes-1,0,&depth_max,&depth_sum,&depth_bal);
  printf("\tNodes.........%ld: %05.3f bal, %.1f/%d avg/max depth",
         NNodes,0.001*depth_bal/NNodes,(double)depth_sum/NSsecs,depth_max);
  printf(" (%ld bytes)\n",dir[NODES].nbytes);
  special = tagged = 0;
  for (i = 0; i < NSects; i++) {		/* look for special sectors */
    if (Sects[i].property == 9)      secret++;
    else if (Sects[i].property != 0) special++;
    if (Sects[i].line_tag != 0) tagged++;
  }
  printf("\tSectors.......%ld: %d special, %d secret, %d tagged (%ld bytes)\n",
         NSects,special,secret,tagged,dir[SECTS].nbytes);
  for (i = 0; i < NRejects; i++)
    for (j = 0; j < numbits(Rejects[0]); j++)
      if (i*numbits(Rejects[0])+j < NSects*NSects &&
          Rejects[i] & (1<<j)) ones++;
  printf("\tReject........");
  if (ones) printf("%4.1f%% (%ld bytes)\n",
                   100.0*ones/(NSects*NSects),dir[REJECTS].nbytes);
  else      printf("zeroed (%ld bytes)\n",dir[REJECTS].nbytes);
  for (i = 4+1; i < Blockmaps[2]*Blockmaps[3]-1; i++)/* if offsets are not  */
    if (Blockmaps[i+1] <= Blockmaps[i])		/* monotonically increasing */
      packed++;					/* then it must be packed   */
  printf("\tBlockmap......%d x %d",Blockmaps[2],Blockmaps[3]);
  if (packed)
    printf(", packed by %ld%%",
           100*(2*2*packed)/(dir[BLKMAPS].nbytes+2*2*packed));
  printf(" (%ld bytes)\n",dir[BLKMAPS].nbytes);
  return TRUE;
}


/******************************************************************************
	ROUTINE:	rename_resources(iwad,rename_list)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	Mar. 1995
	DESCRIPTION:	This routine renames the resources listed in the
			input list to the new names also in the list.  This
			list is of the form <old>-<new>[,...].  Note this
			means that no <old> name can contain a dash character.
******************************************************************************/
#if defined(ANSI_C)
int rename_resources(WAD_INFO *iwad, char *rename_list)
#else
int rename_resources(iwad,rename_list)
WAD_INFO *iwad;
char *rename_list;
#endif
{
  char old_name[10], new_name[10];
  boolean undone = TRUE;
  int e;

  while (rename_list != NULL &&
         2 == sscanf(rename_list,"%[^-]-%[^,]",old_name,new_name)) {
    printf("%s",old_name);
    undone = TRUE;
    for (e = 0; e < iwad->head.count; e++)	/* scan resource directory */
      if (strncmp(old_name,iwad->dir[e].name,sizeof iwad->dir[e].name)==0) {
        memset(iwad->dir[e].name,0,sizeof iwad->dir[e].name);
        sprintf(iwad->dir[e].name,"%-.*s",
                (int)sizeof(iwad->dir[e].name),new_name);
        iwad->changed[e] = TRUE;
        printf("->%s...",new_name);
        undone = FALSE;
      }
    rename_list = strstr(rename_list,",");
    if (rename_list != NULL) rename_list++;
  }
  if (undone) printf("...");
  return !undone;
}


/******************************************************************************
	ROUTINE:	directory(iwad)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	Sep. 1994
	DESCRIPTION:	This routine displays the resource directory for the
			input file.
******************************************************************************/
#if defined(ANSI_C)
int directory(WAD_INFO *iwad)
#else
int directory(iwad)
WAD_INFO *iwad;
#endif
{
  char name[20];
  char *periods = "....................";
  int n = 0;					/* count of level parts */
  int e;

  for (e = 0; e < iwad->head.count; e++) {	/* scan resource directory */
    sprintf(name,"%s%-.*s",
            n ? "  " : "",			/* indent if part of level */
            (int)sizeof(iwad->dir[e].name),iwad->dir[e].name);
    if (level_mark(iwad->dir[e].name,n)) n = ALL(iwad->type);
    if (n > 0) n--;
    printf("\t%s%.*s%ld bytes",
           name,(int)(strlen(periods)-strlen(name)),periods,
           iwad->origdir[e].nbytes);
    if (iwad->count[e] < iwad->dir[e].nbytes)
      printf(" (%ld)",iwad->count[e]);
    printf("\n");
  }
  return TRUE;
}


/******************************************************************************
	ROUTINE:	substitute(iwad,list)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	June 1994
	DESCRIPTION:	This routine substitutes the resources in the input
			file with any that are found in the input file list.
			Any unique resources in the input list are ignored.
******************************************************************************/
#if defined(ANSI_C)
int substitute(WAD_INFO *iwad, char *list)
#else
int substitute(iwad,list)
WAD_INFO *iwad;
char *list;
#endif
{
  char sfile[256];				/* file to substitute from */
  int n;
  WAD_INFO *winfo;
  int undone = -1;
  int e, i, r;

  while (list != NULL && 1 == sscanf(list,"%[^,],",sfile)) {
    if (undone < 0) undone = 0;
    printf("%s...",sfile);			/* substituting from this */
    winfo = wad_open(sfile,TRUE,FALSE);
    if (winfo == NULL)
      return FALSE;
    undone++;
    for (r = 0; r < winfo->head.count; r++) {
      for (e = 0; e < iwad->head.count; e++)
        if (strncmp(winfo->dir[r].name,iwad->dir[e].name,
                    sizeof(winfo->dir[r].name)) == 0)
          break;				/* found a matching resource */
      if (e < iwad->head.count) {		/* now substitute resource */
        if (level_mark(winfo->dir[r].name,n)) n = ALL(winfo->type);
        else                                  n = 1;
        (void)wad_read(winfo,r,~(~0L<<n));
        for (i = 0; i < n; i++)
          resource_update(iwad,e+i,winfo->data[r+i],winfo->count[r+i]);
        undone--;
        r += n-1;
      }
      if (level_mark(winfo->dir[r].name,n)) r += ALL(winfo->type)-1;
    }
    wad_close(winfo);
    list = strstr(list,",");
    if (list != NULL) list++;
  }
  return !undone;
}


/******************************************************************************
	ROUTINE:	extract(iwad,list)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	June 1994
	DESCRIPTION:	This routine extracts the named resources in the input
			list from the input file.  It manipulates the resource
			directory so that only those extracted resources are
			present in the directory.  If any of the extracted
			resources are E#M# or MAP##, all resources associated
			with that level are extracted.  If the input list has
			a leading exclamation point (!), then all resources
			not in the list are extracted.
******************************************************************************/
#if defined(ANSI_C)
int extract(WAD_INFO *iwad, char *list)
#else
int extract(iwad,list)
WAD_INFO *iwad;
char *list;
#endif
{
  char name[sizeof(iwad->dir[0].name)+1+1];
  char *elist = blockmem(char,strlen(list)+1+1);
  int i, n;
  boolean find = TRUE;				/* positive extract flag */
  int r = 0;					/* # extracted resources */
  int e = 0;

  if (list[0] == '!') {				/* means to extract all    */
    find = !find;				/* resources not specified */
    list = &list[1];
  }
  strcat(strcat(strcpy(elist,","),list),",");	/* bracket list w/commas */
  while (e < iwad->head.count) {		/* scan resource directory */
    sprintf(name,",%-.*s",(int)(sizeof(iwad->dir[e].name)),iwad->dir[e].name);
    if ((strstr(elist,name) != NULL &&		/* found one in list */
         0 <= sscanf(strstr(elist,name),",%*[^,]%n",&n) &&
         n == strlen(name)) == find) {
      printf("%s...",&name[1]);
      if (level_mark(&name[1],i)) n = ALL(iwad->type);/* get all level data */
      else                        n = 1;	/* get this resource */
      for (i = 0; i < n; i++, e++) {
        blockcopy(&iwad->dir[r],&iwad->dir[e],sizeof(iwad->dir[e]));
        if (r == 0)
          iwad->dir[r].offset = sizeof iwad->head;
        else
          iwad->dir[r].offset = iwad->dir[r-1].offset + iwad->dir[r-1].nbytes;
        if (iwad->data[e] == NULL) wad_read(iwad,e,1L<<0);
        iwad->data[r] = iwad->data[e];
        iwad->count[r] = iwad->count[e];
        iwad->changed[r++] = TRUE;
      }
    }else
      if (find == 0 && level_mark(&name[1],i)) e += ALL(iwad->type);
      else                                     e += 1;
  }
  iwad->head.count = r;				/* new count of resources */
  iwad->head.ident[0] = 'P';			/* make sure it's a PWAD */
  blockfree(elist);
  return r > 0;
}


/******************************************************************************
	ROUTINE:	merge(iwad,list)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	June 1994
	DESCRIPTION:	This routine merges the files in the input list with
			the currently open input file referenced by iwad.
			Any level data matching level data in the input file
			(i.e., levels that have the same E#M#) is combined
			into a single, larger level.  Note that this combined
			level will require the nodes, blockmap, and rejects to
			be rebuilt.  Any unique resources in the input merge
			list are added to the input file as well.  Non-unique
			resources other than level data are not added, e.g. if
			DEMO1 is in one of the merge list files and the input
			file already has a DEMO1, it is not added to the input
			file.
******************************************************************************/
#if defined(ANSI_C)
int merge(WAD_INFO *iwad, char *list)
#else
int merge(iwad,list)
WAD_INFO *iwad;
char *list;
#endif
{
  char mfile[256];				/* file to merge */
  int maxtag;					/* maximum line/sector tag */
  int maxvert;					/* maximum input file vertex */
  int maxwvert;					/* max merge list file vert */
  WAD_LINE *wlines;
  int e, i, n;
  char *buf;
  WAD_INFO *winfo;
  int r;

  while (list != NULL && 1 == sscanf(list,"%[^,],",mfile)) {
    printf("%s...",mfile);			/* merging this file */
    winfo = wad_open(mfile,TRUE,FALSE);
    if (winfo == NULL)
      return FALSE;
    for (r = 0; r < winfo->head.count; r++) {	/* search for unique rsrcs */
      for (e = 0; e < iwad->head.count; e++)
        if (strncmp(winfo->dir[r].name,iwad->dir[e].name,
                    sizeof(winfo->dir[r].name)) == 0)
          break;
      if (e < winfo->head.count &&
          level_mark(winfo->dir[e].name,n)) {	/* found matching level */
        (void)wad_read(iwad,e,~(~0L<<ALL(iwad->type)));/* get all level data */
        (void)wad_read(winfo,r,~(~0L<<ALL(winfo->type)));
        merge_resource(buf,WAD_THING,iwad,e+THINGS,Things,NThings,
                       winfo->data[r+THINGS],winfo->count[r+THINGS]);
        maxtag = maxvert = 0;
        for (i = 0; i < NLines; i++) {
          if (maxvert < Lines[i].fndx) maxvert = Lines[i].fndx;
          if (maxvert < Lines[i].tndx) maxvert = Lines[i].tndx;
          if (maxtag < Lines[i].sect_tag) maxtag = Lines[i].sect_tag;
        }
        maxwvert = 0;
        wlines = (WAD_LINE *)winfo->data[r+LINES];
        for (i = 0; i < winfo->count[r+LINES]; i++) {
          if (maxwvert < wlines[i].fndx) maxwvert = wlines[i].fndx;
          if (maxwvert < wlines[i].tndx) maxwvert = wlines[i].tndx;
          wlines[i].fndx += maxvert+1;
          wlines[i].tndx += maxvert+1;
          if (wlines[i].sect_tag != 0)
            wlines[i].sect_tag += maxtag;
          wlines[i].rsidndx += NSides;
          if (wlines[i].lsidndx != -1) wlines[i].lsidndx += NSides;
        }
        merge_resource(buf,WAD_LINE,iwad,e+LINES,Lines,NLines,
                       winfo->data[r+LINES],winfo->count[r+LINES]);
        for (i = 0; i < winfo->count[r+SIDES]; i++)
          ((WAD_SIDE *)winfo->data[r+SIDES])[i].sectndx += NSects;
        merge_resource(buf,WAD_SIDE,iwad,e+SIDES,Sides,NSides,
                       winfo->data[r+SIDES],winfo->count[r+SIDES]);
        NVerts = maxvert+1;
        merge_resource(buf,WAD_VERT,iwad,e+VERTS,Verts,NVerts,
                       winfo->data[r+VERTS],maxwvert+1);
        resource_update(iwad,e+SEGS,Segs,0L);	/* kill these because they */
        resource_update(iwad,e+SSECTS,Ssecs,0L);/* will have to be rebuilt */
        resource_update(iwad,e+NODES,Nodes,0L);
        for (i = 0; i < winfo->count[r+SECTS]; i++)
          if (((WAD_SECTOR *)winfo->data[r+SECTS])[i].line_tag != 0)
            ((WAD_SECTOR *)winfo->data[r+SECTS])[i].line_tag += maxtag;
        merge_resource(buf,WAD_SECTOR,iwad,e+SECTS,Sects,NSects,
                       winfo->data[r+SECTS],winfo->count[r+SECTS]);
        buf = (char *)blockmem(WAD_REJECT,(NSects*NSects+7)/8);
        resource_update(iwad,e+REJECTS,buf,(NSects*NSects+7L)/8);
        resource_update(iwad,e+BLKMAPS,Blockmaps,0L);/* has to be rebuilt */
      }
      if (e == iwad->head.count) {		/* add in new resource */
       if (level_mark(winfo->dir[r].name,n)) n = ALL(winfo->type);
       else                                  n = 1;
       (void)wad_read(winfo,r,~(~0L<<n));
       for (i = 0; i < n; i++) {
       add_resource(buf,DIR_ENTRY,iwad->dir,iwad->head.count+1,winfo->dir[r+i]);
       add_resource(buf,char *,iwad->data,iwad->head.count+1,winfo->data[r+i]);
       add_resource(buf,long,iwad->count,iwad->head.count+1,winfo->count[r+i]);
       add_resource(buf,boolean,iwad->changed,iwad->head.count+1,TRUE);
       iwad->head.count++;
       }
       r += n-1;
      }
    }
    wad_close(winfo);
    list = strstr(list,",");
    if (list != NULL) list++;
  }
  return TRUE;
}


/******************************************************************************
	ROUTINE:	merge_raw(iwad,list)
	WRITTEN BY:	Robert Fenske, Jr.
	CREATED:	Dec. 1995
	DESCRIPTION:	This routine merges the files in the input list with
			the currently open input file referenced by iwad.
			The input list contains raw/lump file and resource
			name pairs.  Each listed file is read in and inserted
			with the specified resource name into the open WAD
			file.  Any named resource already in the WAD file will
			be overwritten.
******************************************************************************/
#if defined(ANSI_C)
int merge_raw(WAD_INFO *iwad, char *list)
#else
int merge_raw(iwad,list)
WAD_INFO *iwad;
char *list;
#endif
{
  char rfile[256];				/* raw file to merge */
  char rrsrc[256];				/* target resource */
  DIR_ENTRY rdir;
  FILE *fp;
  char *rbuf;					/* raw data buffer */
  char *buf;
  boolean had_merge = FALSE;
  int e;

  while (list != NULL && 2 == sscanf(list,"%[^-]-%[^,],",rfile,rrsrc)) {
    printf("%s-->%s...",rfile,rrsrc);		/* merging this raw file */
    fp = file_open(rfile,TRUE,FALSE);
    if (fp == NULL)
      return FALSE;
    rdir.offset = 0;
    fseek(fp,0,2);
    rdir.nbytes = ftell(fp);			/* how many bytes there are */
    strncpy(rdir.name,rrsrc,sizeof rdir.name);
    rbuf = blockmem(char,rdir.nbytes);
    fseek(fp,0,0);
    fread(rbuf,rdir.nbytes,1,fp);		/* here is the raw data */
    fclose(fp);
    for (e = 0; e < iwad->head.count; e++)	/* search for resource name */
      if (strncmp(rrsrc,iwad->dir[e].name,sizeof(iwad->dir[e].name)) == 0)
        break;
    if (e == iwad->head.count) {		/* add in new resource */
      add_resource(buf,DIR_ENTRY,iwad->dir,iwad->head.count+1,rdir);
      add_resource(buf,char *,iwad->data,iwad->head.count+1,rbuf);
      add_resource(buf,long,iwad->count,iwad->head.count+1,rdir.nbytes);
      add_resource(buf,boolean,iwad->changed,iwad->head.count+1,TRUE);
      iwad->head.count++;
    }else {					/* substitute resource */
      blockfree(iwad->data[e]);
      iwad->dir[e] = rdir;
      iwad->data[e] = rbuf;
      iwad->count[e] = rdir.nbytes;
      iwad->changed[e] = TRUE;
    }
    had_merge = TRUE;
    list = strstr(list,",");
    if (list != NULL) list++;
  }
  return had_merge;
}
