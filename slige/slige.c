/*

   SLIGE - a random-level generator for DOOM
           by David M. Chess
           chess@theogeny.com      http://www.davidchess.com/
                                   http://users.aol.com/dmchess/
           chess@us.ibm.com        http://www.research.ibm.com/people/c/chess/

           http://www.doomworld.com/slige/

   August, 2000

   This source code is made available to the world in general,
   with the following requests:

   - If you're doing a port and you find that something or other needs
     twiddling to get it to compile, let me know and I'll stick in some
     ifdefs.  The current code assumes that Microsoft C has a horrible
     rand() function and uses str(n)icmp() instead of str(n)casecmp(),
     whereas everyone else has an ok rand() and str(n)casecmp().  And
     make sure that you compile with tight structure-packing (/Zp1 or
     -fpack_struct, I think) or DOOM won't understand the files.  Also,
     if you want to port to a non-Intel-byte-order platform, you've got
     a bit of work, but I think it's mostly isolated to DumpLevel,
     CloseDump, dump_texture_lmp, and a few others.  Good luck!  *8)

   - The current version should compile without any special switches
     except structure-packing, under both MSVC++ and DJGPP, without
     any warnings (unless you use -O3 in DJGPP, in which case you'll
     get some warnings that don't matter).  It should also compile
     and work under Linux gcc, but I've never tried it myself; if you
     do, write and tell me about it!  It should also compile for the
     RISC OS with the usual C compiler, thanks to Justin Fletcher.
     Anyone wanna do a Mac port?  I'd be glad to give advice...

   - If you make various changes and improvements to it and release
     a modified version yourself:

     - Write me and tell me about it so I can be pleased,
     - Mention me and SLIGE in the docs somewhere, and
     - Please *don't* call your new program "SLIGE".  SLIGE is
       my program.  Call yours "BLIGE" or "EGGISLES" or "MUMFO"
       or something like that.

   - If you want to do lots of wonderful things to the program to
     make it better, and then send me the result to incorporate into
     my next version, I strongly suggest telling me your plans first.
     Otherwise you may hear nothing from me for weeks after sending me
     your source mods (I don't check my mail that often sometimes), and
     then get a disappointing "thanks, but I don't want to take the
     program that direction, I have other plans" sort of reply.  Small
     bug fixes, of course, are always welcome!  All source mods sent
     to me become the property of the world at large.

   Otherwise, enjoy the program, and let me know what you think of the code!

   DC

*/
#define SOURCE_SERIAL (490)
/*

   New stuff (since 485): -huge switch, fixed gate-to-gate-to-arena bug
     (not as elegantly as I could have!), RISCOS source mods from JF,
     -fstart -ffstart -fend -ffend switches, getenv() var for cfg filename,
     pillar gates, attempt at SPARC compatibility (tx to O. Kraus).


   Stuff to do:
     BUG: very rarely, a slige -dm level will still not have four DM starts.
       Try more rooms than just first and last, and/or use a smaller "width"
       to allow putting a DM start on a pickable object?
     fold in Rob Ellwood's ceiling-effect and split_linedef mods,
       when he has a version he likes enough
     simple DM suggestion from some guy: more star-shaped than usual,
       with a big weapon in the middle.
     Wolfenstein-style secret levels (and switch)
     armor-damage model is wrong: green armor only absorbs 1/3 of
       damage taken; only blue armor absorbs 1/2.  Fix!
     BUG: when trigger_box() calls point_sector(), it should check
       the "danger" return (rather than passing in NULL)
     more lamps/lights when night and/or dim?
     very-dark secret-level flavor?
     level flavor like "always big floordeltas"
     implement the "favorite monster" style of secret level (and switch)
     simple multi-level "ramps" in patios, for variety?
     sometimes monsters can be so tightly packed into a small room
       that none can move until you kill one.  Detect/avoid somehow.
     eliminate "ladders", by having random_basic_link() just make sure that
       if there are stairs abs(floordelta)<=linkdepth?
     switches mounted on columns/placards, not just in walls
     nukage in downsecs of falling-core traps, sometimes.  Barrels, too!
     regularize lighting (less ad-hoc, more constants), have dark
       secret levels, perhaps darker style, perhaps dark rooms with
       bright otherstuff, etc.
     secret doors/closets at the ends of ambush closets
     configify lamps and barrels
     configify other objects and monsters and anything else that's left
     unhardcode health's amounts, weapons' damage, etc
     shouldn't "error" and "gateexitsign" just be property bits?
     fixed the bug that sometimes accidentally made the monster hiding
       on top of a pillar be down in the room; but it'd be cool to
       sometimes do this on purpose!
     trees can block patio doors; fix
     BUG: A raised teleporter right next to a step-up doorway can be
       inaccessible.  Fix!
     once in awhile a barrel in a secret, to discourage the "open and
       shoot without looking" trick.  heh heh.
     have use of lights (gridlights, lightstrips, kickplate lights, etc)
       be correlated per level or config or something, not random
     use CEILING+LIGHT in even more lighting effects (link to
       light_recesses, for instance, and maybe use in windows)
     do mancubus-special in MAP07 even if not last_mission
     do arach-special in MAP07 also (somehow)
     place_timely_something() and friends need an "is it a secret?"
       switch, then could be used in various secret places.
     Secret grid-pillars need something more significant on them!
     Biggest-monster mode in DOOM 1 gives lots and lots and lots and
       lots of spectres...  (if BIG is off, I suspect)  Problem?
     Still the occasional level with many too many rooms; heh!
     BUG: the embellishment of the outersec of a rising room
       can decide to triggerbox the key, even though the rising room
       has already boxed it.  Fix.  (Actually both boxes seem to work,
       even though they're exactly coextensive, but still...)
     BUG: have seen at least one extwindow that extended orthogonal to
       the direction it should have extended.  Very odd!
     BUG: still getting a monster stuck in a floor-lamp now and then?  how?
       also perhaps a monster stuck in a grid-room pillar (or was that two
       grid-room monsters stuck to each other?)  (Those may be fixed now.)
       Also once saw a Hell Knight stuck between a new-style "pillar" and
       a wall.  Looked like.
     BUG: Sometimes a room has two different gates, both in the midtile.
       Well, you know what I mean...  This causes trouble.  Fixed?
       Nope; still can happen with a gate to an arena.  Is that OK?
       Nope; it can result in unfinishable levels.  Fixed now, I think!
     More exit-to-secret-level secret kinds.
     More special things about secret levels.  A black-and-starry
       theme using custom patches?
     For co-op, put all the weapons that we assume the player has
       into the start room (with multiplayer-only bit set).  Conflicts
       with deathmatch stuff, so only if not -dm?  Or have -coop switch?
     It would be nice to not give up on a bath whenever a monster might
       get stuck in the edge.  Try *moving* the monsters, or the edges
       of the bath, instead?
     Make arenas more interesting.  powerups/armor?  More scenarii!  Harder!!
     dead-gardens flavor
     More interesting sorts of GATE_GOAL-locked links (like just
       a grated door).  Also have the gate be one-way if the locked
       link is passible in the far-to-near direction.
     More Instaforks!  (Really just pushes, not forks at all)
     twinned links need to have their extra-painted-door bits and
       nukage-link status and stuff correlated, eh?
     read switches from config file
     constructs (computers, at least) hanging from the ceiling?
     more rational facings in grid-rooms (sort of toward the door)
     have the new-pillar probability in the style, and have it sometimes
       be very very high?
     more open-link stuff: gratings, locks, secrets, monsters, etc, etc
     sometimes have open(ish) links block sound, to avoid excess monster
       accumulation?  (Doesn't seem to be a problem?)
     open lifts sometimes activate at the top also
     sometimes blaze autodoors / lifts
     switch-activated lifts of various kinds
     monsters/pickables in sidesectors of an open link
     maybe have a minimum-link-width thing (used only on non-door links,
       maybe?), usually zero, and sometimes (often when bigification is
       high) higher (like 128).  For a non-narrow level, but not hugeness
     put nukage-edge textures (bottom-aligned) around nukage cores (and
       normal nukage pools, eh?)  Need new attribute bit/thing
     put pillars with nukage-pourers (green gunk vents, red demon
       faces, various leaking pipes) in nukage pools
     use step-textures and/or wall textures for/instead of support
       textures in various other kickplate places (below doors?)
     put decorations up on the centers of some pillars (/constructs)
     I suspect it's possible for a sequence of new-style pillars to
       block off the center of the room (thus perhaps making it impossible
       to get a key).  Implement fix if so.
     make the sunrooms and nukage-city effects less correlated (if nukage
       was forced, have sky lower-prob, and vice-versa)
     Other patio things:
       There are often no plants.  That "128" is possibly too big?
       Store floor and wall textures in style? or even level?
       Patios should count toward the level's (the quest's?) room count, eh?
       Patios would really make more sense as a kind of microquest, rather
         than a mere embellishment
       Make sure "roof" height is also big enough to accomodate the
         link->height1, and for that matter the door's height (twice),
         if there's a door
     in homogenize mode, that hack that will always add one more of the
       room's monsters if there's room for *any* monster can produce
       very painful results (the Revenant Brothers, in particular).
       Fix it?  Just separate out the "is there room for monster M in
       the model" check into a routine to call in homgen mode.
     homogenize_monsters, should be per-level, not per-WAD, eh?  or should it?
     once had a really really steep stairway with a monster on it.  The
       stairway was so narrow I couldn't get around the monster, but so
       steep that I couldn't see it to shoot it!  avoid?  (I actually
       managed to sneak around it eventually, but it got in lots of bites.)
       probably just put no link-guards on steep stairs, eh?
     y-offsets pretty good now; put some support texture in when two
       coalignable textures come together at a base-ceiling boundary?
       that's about the only place that y-offset problems remain
     some cool light-effect around pillars (new and old styles)
     find all monster-width assumptions (tough!), generalize or whatever
     merge secret-closet and plaque-closet code sections (diffs: plaques
       are recessed, and are sometimes not openable at all)
     way-cool recessed windows style
     traditional locked nop-DOOR3 sometimes in entry room?
     sometimes put a rising room at the end of tag-switch quests, too?
     allow embellishing far sides of links (w/lightboxes etc?)
     sometimes vertical lightstrips on walls / in corners (won't happen
       in Wod theme, because no "LIGHT" walls anymore; OK?)
     more extensive outdoor areas; need a theory
     non-rectangular rooms (lots of work there!)
     same texture on both sides of a door?
     before giving up finding a linedef to put a link on, split
       all too-big linedefs around the current room
     if a lift can be walked off of, leave out the WR-lower linedef
       at the top end.  sometimes
     raised sniper-closets
     make level-exits look even more obvious in DOOM I somehow?
     sometimes put keys in ambush closets, other special places
     sometimes have the back wall of a closet be a secret door (hint?)
     have big monsters / weapons (only) in later episodes / maps?
     use of has_chainsaw/berserk on !(FLIES|SHOOTS) working OK?
     use has_chaingun information for something.  remove wimp-bonus?
       chainguns tend to be ammo-inefficient; recognize?
     silly to put in the linedef box around an object before checking if
       there's enough room for the secret closet, eh?  fixed?
     have plaque-doors sometimes be triggered also
     more sophisticated backpack counting (depending on the weapon(s)
       the player has)
     model the invis and radsuit better?
     a big boss monster in a big secret room that lowers down when you
       take pickable.  also in the boss' room is the key/switch you need.
       have to make sure player has enough ammo/health first.  details?
     put a single window-grating in the center, not two half-invisible
       ones on the sides (done?)
     don't watermark the first room of non-first levels in a PWAD?
     autodoors and lifts are sort of boring currently (only 24-deep rec's).
       make them more interesting?
     plain-sky ceilings don't work, because if a nearby room has a
       slightly higher non-sky ceiling, or worse is a steep stairway
       goes up out of the room, it looks really dumb.  think about fixes.
       perhaps always raise the ceiling to be at least as high as all
       ceilings of attached rooms?  that simple?
     the support_misaligns stuff looks awful.  think about this some more.
     skyclosets are still silly-looking if you can stand in the end and
       look back at the "building".  Either fix somehow (how?), or use
       point_froms to put the skylight only at the far end of the closet,
       even in deep closets, so you can't see out from the end.  Also,
       the lip of the outer sides of the skycloset walls isn't necessary;
       would look best, I think, with a lip on the near side, and the
       other sides just ending blap like they used to.
     sometimes use a light-wall texture on walls (especially flanking
       doors/links?)  But there aren't that many of them.
     have a "minwall" in config, to usually forbid those skinny little
       4-pel-wide walls and like that.  (done a different way, but we
       might want to think about something >8 as a config option)
     multiple snipers/ambushers in a single (wider) closet?
     OK to have population before embellishment?  Should prevent
       monsters getting stuck in the wall because of swelling, yes?
       also makes haa-model accesses during embellishment more
       logical (since probably one cleans out the wandering monsters
       first, then the embellished?)  probably need both pre-pop and
       post-pop embellishments eventually for something
     random-link still returns too wide sometimes; that whole routine
       needs to be re-thought and rationalized (fixed?)
     use real door-texture widths, not the current hack, to center faces
     niches: often symettrically around doors, with lights/monsters/items
       sometimes with (secret) doors, sometimes non-walkable (lights,
       airholes to the outside, portraits, etc)
     generalize lightstrips to include counters, plaques, computer
       displays, even those little closets with monsters and stuff.
     do basic stairs by first making the core and then splitting it up; this
       will help auto-alignment, and sort of rationalize things also
       (can use stairify(), with a little work).
     swelling a room moves out the apparent corners, and that interferes
       with later find_rec() calls used during population.  Uh-oh!!
       need true distance-from-point-to-line code.  Now that we're
       embellishing after populating, this isn't as bad.
     figure out how alignment works on left sidedefs, for split_linedef()
       (current code OK?)
     style setting for closet height?  variation here is nice, though
     more interesting window-borders (not just random) and widths
       very narrow "windows" can of course have lower floors and
       higher heights, since you can't walk through 'em anyway
       (just like gratings).  Think about how to do in style / link.
       (Slits sort of provide that now.)
     window stuff in Link, not in Style, eh?
     have generate_room_outline sometimes extend the starting
       linedef with another colinear linedef at one or both
       ends, so you can come into a room on a *long* side
     in intersect(), properly check for overlap of colinear segments.
     use texture-size data more in alignment, door-choice, etc, etc
     need a copy_link() that does small perturbations to an
       existing link (like copy_style())
     restore the ability for twinned links to have different
       door types?  Say!  How about allowing the two links
       of a twinned link to be *completely different?  Well,
       they have to have the same total depth and floordelta,
       anyway.  Hmm...
     on the other hand, it'd be logical if the door-types at
       either end of a link were the same.  That means having
       it in the link, not the room's style.  Sensible?  On
       the third hand, they needn't always be the same, eh?
     allow different-size openings at either end of a link?
       (tapered stair-walls, e.g.)
     have special deliverers for weapons if c->weapons_are_special
       (and then sometimes set that TRUE)
     more global (per-level or per-config) restrictions on style;
       never moving jambs, always soundproof doors, doorceilings
       always/never copy room ceiling, etc, etc?  Also pillars
       and windows and gunkage.  (Extend "Nukage City" concept)
     if link.height1==0, use ceiling height of main room, eh?
     pick one (at least) 64-aligned 64x64 square, make it a sector,
       change the ceiling texture to some light, maybe raise or
       lower the ceiling a bit, and change the light level (up,
       usually), and perhaps make it blink.
     need more general object-copiers (for split_linedef etc)
     generalize the idea that a monster can provide a weapon?  (eventually,
       including in config-file etc)
     in basic philosophy, we should think more about making the individual
       room fancy.  as it is, the overall flavor is likely to always
       be a chain of simple rooms.  Which is OK, but limited.  Put
       another way, the monsters standing around in a room are dull;
       the interesting stuff are the monsters in traps, on ledges,
       behind gratings, etc.  Do lots of monster-placement during
       embellishment?
     or another way, it's too easy now, just running through rooms
       killing things.  make it harder!
     one big thing we're missing is to involve the outdoors more;
       multiple rooms overlooking the same courtyard, "bay windows"
       out over pools and gunkage and stuff, etc, etc, etc.

*/

/* Define various compiler and filesystem features here */

/* Define BIGEND for big-endian (i.e. not Intel 80x8x) systems */
#ifdef __sparc
#ifndef BIGEND
#define BIGEND
#endif /* BIGEND */
#endif /* __sparc */

/* If your compiler/architecture does not support unaligned data in structures
   then you'll have to turn this on; at present ARM compilers are the only
   ones known to have this problem.  (You should also check the places
   where this symbol is tested, to make sure the workarounds used for
   RISCOS are also correct for your platform.)           */
#if defined(RISCOS)
#define ALIGN_STRUCTS
#endif

/* Define this if your standard library does not include strcasecmp */
#if defined(RISCOS)
#define NO_STRCASECMP
#endif

/* Define this if your standard library does not include strdup */
#if defined(RISCOS)
#define NO_STRDUP
#endif

/* If you use different extension seperators, declare them here */
#ifdef RISCOS
#define EXT_SEP "/"
#else
#define EXT_SEP "."
#endif

/* Name of the getenv() variable to look in for a different default
   configuration-file name. */
#ifdef RISCOS
#define SLIGE_CONFIG "Slige$ConfigFile" /* In keeping with rest of the OS */
#else
#define SLIGE_CONFIG "SLIGECONFIG"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#ifdef _MSC_VER
#define RAND_SUCKS
#define USE_STRICMP
#endif

typedef unsigned char boolean;
#define TRUE (1==1)
#define FALSE (!TRUE)
#define HUGE_NUMBER (1000000)

#define LEVEL_MAX_BARS (30)
#define LEVEL_MAX_CRUSHERS (2)

#define TLMPSIZE(rows,columns) ((rows+9)*columns + 8)

typedef unsigned char byte;

#ifndef USE_STRICMP
#define stricmp(x,y) strcasecmp(x,y)
#define strnicmp(x,y,n) strncasecmp(x,y,n)
#endif

#ifdef NO_STRCASECMP
#include <ctype.h>
/* These routines from Justin Fletcher, for the RISCOS port */
int strcasecmp (const char *a, const char *b)
{
    const char *p = a, *q = b;
    for (p = a, q = b; *p && *q; p++, q++)
    {
      int diff = tolower(*p) - tolower(*q);
      if (diff) return diff;
    }
    if (*p) return 1;       /* p was longer than q */
    if (*q) return -1;      /* p was shorter than q */
    return 0;               /* Exact match */
}

int strncasecmp (const char *a, const char *b, int n)
{
    const char *p = a, *q = b;

    for (p = a, q = b; /*NOTHING*/; p++, q++)
    {
      int diff;
      if (p == a + n) return 0;     /*   Match up to n characters */
      if (!(*p && *q)) return *p - *q;
      diff = tolower(*p) - tolower(*q);
      if (diff) return diff;
    }
    /*NOTREACHED*/
}
#endif

#ifdef NO_STRDUP
char *strdup(char *str)
{
  char *newstr=malloc(strlen(str)+1);
  if (newstr==NULL) return NULL;
  else return strcpy(newstr,str);
}
#endif

#ifdef OK_TO_USE_REAL_MONSTER_WIDTH
#define MONSTER_WIDTH(m) (m->width)
#else
#define MONSTER_WIDTH(m) (64)
#endif

typedef unsigned long themebits;  /* Bitarray, really */
/* So at most 32 themes in a config file */

typedef unsigned char gamebits;   /* Also bitarray    */
#define DOOM0_BIT (0x01)
#define DOOM1_BIT (0x02)
#define DOOM2_BIT (0x04)
/* This is "clean" doom, with no "GROSS" items */
#define DOOMC_BIT (0x08)
/* "Intrinsic"; i.e. no SLIGE-special textures */
#define DOOMI_BIT (0x10)
/* and that's all */

typedef unsigned long propertybits;  /* Another bitarray */
#define FLOOR (0x01)
#define CEILING (0x02)
#define DOOR (0x04)
#define ERROR_TEXTURE (0x08)
#define WALL (0x10)
#define SUPPORT (0x20)
#define NUKAGE (0x40)
#define JAMB (0x80)
#define RED (0x100)
#define BLUE (0x200)
#define YELLOW (0x400)
#define GRATING (0x800)
#define PLAQUE (0x1000)
#define HALF_PLAQUE (0x2000)
#define LIGHT (0x4000)
#define BIG (0x8000)
#define SWITCH (0x10000)
#define OUTDOOR (0x20000)
#define GATE (0x40000)
#define EXITSWITCH (0x80000)
#define STEP (0x100000)
#define LIFT_TEXTURE (0x200000)
#define VTILES (0x400000)
/* and so on and so on; 32 may well not be enough! */

/* Some thing-only bits; corresponding bits above are texture/flat-only */
#define MONSTER (0x01)
#define AMMO (0x02)
#define HEALTH (0x04)
#define WEAPON (0x08)
#define PICKABLE (0x10)
#define SHOOTS (0x20)
#define EXPLODES (0x40)
#define FLIES (0x80)
#define BOSS (0x100)
#define SPECIAL (0x800)

typedef struct s_theme {
  char *name;
  boolean secret;
  struct s_theme *next;
} theme, *ptheme;

typedef struct s_texture {
  char name[9];   /* Room for the eos */
  char *realname; /* the DOOM name, in case the name name is an alias */
  gamebits gamemask;
  themebits compatible;
  themebits core;
  propertybits props;
  short width;
  short height;
  short y_hint;
  short y_bias;    /* Y offset that a switch needs */
  struct s_texture *subtle;
  struct s_texture *switch_texture;
  boolean used;
  struct s_texture *next;
} texture, *ptexture;

typedef struct s_flat {
  char name[9];  /* Room for the eos */
  gamebits gamemask;
  themebits compatible;
  propertybits props;
  boolean used;
  struct s_flat *next;
} flat, *pflat;

typedef struct s_linedef linedef, *plinedef;
typedef struct s_gate gate, *pgate;

typedef struct s_link {
  int type;
  propertybits bits;
  int height1;     /* Basic height, or zero for "floor-to-ceiling" */
  int width1;      /* Basic width, or zero for "wall-to-wall" */
  int width2;      /* Width of the interalcove-passage, if any */
  int depth1;      /* Depth of doors / arches (overall depth for OPEN) */
  int depth2;      /* Depth of recess sectors */
  int depth3;      /* Length (depth) of the core (if any) */
  int floordelta;  /* Far sector floorheight - near sector floorheight */
  int stepcount;   /* Number of steps to slice depth3 into (minus one) */
  linedef *cld;    /* The inner side of a twinned core, sometimes */
  struct s_link *next;
} link, *plink;

/* Values for link.type */
#define BASIC_LINK 1001
#define OPEN_LINK 1002
#define GATE_LINK 1003

/* Bits for link.bits */
#define LINK_NEAR_DOOR (0x01)
#define LINK_RECESS (0x02)
#define LINK_ALCOVE (0x04)
#define LINK_TWIN (0x08)
#define LINK_CORE (0x10)
#define LINK_LIFT (0x20)
#define LINK_STEPS (0x40)
/* LINK_WINDOW is used only if LINK_TWIN */
#define LINK_WINDOW (0x80)
#define LINK_MAX_CEILING (0x100)
#define LINK_TRIGGERED (0x200)
#define LINK_LAMPS (0x400)
#define LINK_BARS (0x800)
#define LINK_LEFT (0x1000)
#define LINK_LOCK_CORE (0x2000)
#define LINK_FAR_TWINS (0x4000)
#define LINK_DECROOM (0x8000)
#define LINK_FAR_DOOR (0x10000)

#define LINK_ANY_DOOR (LINK_NEAR_DOOR | LINK_FAR_DOOR)

/* The kinds of things that there are */

typedef struct s_genus {
  gamebits gamemask;
  themebits compatible;
  propertybits bits;
  short thingid;
#define ID_PLAYER1 (0x0001)
#define ID_PLAYER2 (0x0002)
#define ID_PLAYER3 (0x0003)
#define ID_PLAYER4 (0x0004)
#define ID_DM      (0x000b)
#define ID_GATEOUT (0x000e)
#define ID_TROOPER (0x0bbc)
#define ID_SERGEANT (0x0009)
#define ID_IMP (0x0bb9)
#define ID_PINK (0x0bba)
#define ID_SPECTRE (0x003a)
#define ID_COMMANDO (0x041)
#define ID_NAZI (0x054)
#define ID_SKULL (0xbbe)
#define ID_HEAD (0xbbd)
#define ID_SKEL (0x042)
#define ID_PAIN (0x047)
#define ID_ARACH (0x044)
#define ID_HELL (0x045)
#define ID_BARON (0x0bbb)
#define ID_MANCUB (0x0043)
#define ID_ARCHIE (0x0040)
#define ID_CYBER (0x10)
#define ID_SPIDERBOSS (0x07)
#define ID_BRAIN (0x58)
  short width;
  short height;
  float ammo_to_kill[3];
  float ammo_provides;     /* For monsters / ammo / weapons */
  float damage[3];         /* damage[0] will be health provided by HEALTHs */
  float altdamage[3];
  boolean marked;
  struct s_genus *next;
} genus, *pgenus;

#define ID_SHOTGUN (0x7d1)
#define ID_SSGUN (0x052)
#define ID_CHAINGUN (0x7d2)
#define ID_CHAINSAW (0x7d5)
#define ID_PLASMA (0x7d4)
#define ID_BFG (0x7d6)
#define ID_CLIP (0x7d7)
#define ID_SHELLS (0x7d8)
#define ID_BULBOX (0x800)
#define ID_SHELLBOX (0x801)
#define ID_CELL (0x7ff)
#define ID_CELLPACK (0x11)
#define ID_BACKPACK (0x08)
#define ID_LAUNCHER (0x7d3)
#define ID_ROCKET (0x7da)
#define ID_ROCKBOX (0x7fe)

#define ID_STIMPACK (0x7DB)
#define ID_MEDIKIT (0x7dc)
#define ID_POTION (0x7de)
#define ID_SOUL (0x7dd)
#define ID_BERSERK (0x7e7)
#define ID_INVIS (0x7e8)
#define ID_SUIT (0x7e9)
#define ID_MAP (0x7ea)

#define ID_HELMET (0x7df)
#define ID_BLUESUIT (0x7e3)
#define ID_GREENSUIT (0x7e2)

#define ID_BLUEKEY (0x028)
#define ID_REDKEY (0x026)
#define ID_YELLOWKEY (0x027)
#define ID_BLUECARD (0x0005)
#define ID_REDCARD (0x00d)
#define ID_YELLOWCARD (0x006)

#define ID_LAMP (0x07ec)
#define ID_ELEC (0x030)
#define ID_TLAMP2 (0x055)
#define ID_LAMP2 (0x056)
#define ID_TALLBLUE (0x002c)
#define ID_SHORTBLUE (0x037)
#define ID_TALLGREEN (0x02d)
#define ID_SHORTGREEN (0x038)
#define ID_TALLRED (0x02e)
#define ID_SHORTRED (0x039)
#define ID_CANDLE (0x022)
#define ID_CBRA (0x023)
#define ID_BARREL (0x07f3)
#define ID_FBARREL (0x0046)
#define ID_SMIT (0x002f)
#define ID_TREE1 (0x002b)
#define ID_TREE2 (0x0036)

/* The style is the dynamic architectural knowledge and stuff. */
/* It changes throughout the run.                              */
typedef struct s_style {
  int theme_number;
  flat *floor0;
  flat *ceiling0;
  flat *ceilinglight;
  flat *doorfloor;
  flat *doorceiling;
  flat *stepfloor;
  flat *nukage1;
  texture *wall0;
  texture *switch0;
  texture *support0;
  texture *doorjamb;
  texture *widedoorface;
  texture *narrowdoorface;
  texture *twdoorface;       /* tall-wide */
  texture *tndoorface;       /* tall-narrow */
  texture *lockdoorface;         /* can be NULL */
  texture *walllight;            /* Can be NULL */
  texture *liftface;        /* can be NULL */
  texture *kickplate;            /* At least 64 tall */
  texture *stepfront;            /* May be quite short */
  texture *grating;
  texture *plaque;
  texture *redface;
  texture *blueface;
  texture *yellowface;
  genus *lamp0;
  genus *shortlamp0;
  short doorlight0;
  short roomlight0;
  short wallheight0;
  short linkheight0;
  short closet_width;
  short closet_depth;
  short closet_light_delta;
  /* Shouldn't all these booleans just be in a properties bitarray? */
  boolean moving_jambs;
  boolean secret_doors;    /* a silly thing */
  boolean soundproof_doors;
  boolean center_pillars;
  boolean paint_recesses;  /* Put keycolors on recesses, not doors? */
  boolean lightboxes;  /* Ephemeral */
  boolean gaudy_locks;
  int auxheight;     /* Height off the ground of lightboxes (etc) */
  short auxspecial;  /* Special light thing for lightboxes (etc) */
  short doortype;  /* Should be part of link? */
  short slifttype;  /* part of link? */
  int sillheight;    /* should be part of link? */
  int windowheight;    /* part of link? */
  int windowborder;     /* part of link? */
  boolean slitwindows;   /* part of link? */
  boolean window_grate;  /* part of link? */
  int window_decor;     /* part of link? */
#define WINDOW_NORMAL (5001)
#define WINDOW_JAMBS (5002)
#define WINDOW_SUPPORT (5003)
#define WINDOW_LIGHT (5004)
  int lightbox_lighting;
#define LIGHTBOX_NORMAL (6001)
#define LIGHTBOX_LIGHTED (6002)
#define LIGHTBOX_DARK (6003)
  boolean light_recesses;
  boolean light_steps;
  boolean light_edges;
  boolean peg_lightstrips;
  int construct_family;
  boolean do_constructs;
  link *link0;
  struct s_style *next;
} style, *pstyle;

/* General linked list of textures, for constructs */
typedef struct s_texture_cell {
  texture *texture;
  boolean marked;
  boolean primary;
  short y_offset1;
  short y_offset2;
  short width;
  struct s_texture_cell *next;
} texture_cell, *ptexture_cell;

/* General linked list of flats */
typedef struct s_flat_cell {
  flat *flat;
  struct s_flat_cell *next;
} flat_cell, *pflat_cell;

/* Things that are basically boxes with sides */
typedef struct s_construct {
  gamebits gamemask;
  themebits compatible;
  int family;    /* What general kind of thing is it? */
  short height;
  texture_cell *texture_cell_anchor;
  flat_cell *flat_cell_anchor;
  boolean marked;
  struct s_construct *next;
} construct, *pconstruct;

typedef struct s_thing {
  short x;
  short y;
  short angle;
  genus *genus;
  short options;
  short number;
  struct s_thing *next;
} thing, *pthing;

/* These are sort of two-natured; they represent both sectors in the */
/* DooM-engine sense, and rooms.  Split the meanings someday. */
typedef struct s_sector {
  short floor_height;
  short ceiling_height;
  flat *floor_flat;
  flat *ceiling_flat;
  short light_level;
  short special;
  short tag;
  short number;            /* Used only during dumping */
  style *style;            /* Style used to create it */
  boolean marked;
  boolean has_key;         /* Has a key been placed in here? */
  boolean has_dm;          /* A DM start in here? */
  boolean has_dm_weapon;      /* Any weapons in here yet in DM? */
  boolean middle_enhanced;      /* Already specially enhanced */
  gate *gate;
  short entry_x, entry_y;
  boolean findrec_data_valid;
  short minx, miny, maxx, maxy;
  struct s_sector *next;
} sector, *psector;

typedef struct s_vertex {
  short x;
  short y;
  short number;
  boolean marked;
  struct s_vertex *next;
} vertex, *pvertex;

typedef struct s_sidedef {
  short x_offset;
  short x_misalign;
  short y_offset;
  short y_misalign;
  texture *upper_texture;
  texture *lower_texture;
  texture *middle_texture;
  sector *sector;
  short number;
  boolean isBoundary;
  struct s_sidedef *next;
} sidedef, *psidedef;

struct s_linedef {
  vertex *from;
  vertex *to;
  short flags;
  short type;   /* Ooh, could even have linedef-type-kind records! */
  short tag;
  sidedef *right;
  sidedef *left;
  short number;
  boolean marked;
  boolean f_misaligned;
  boolean b_misaligned;
  struct s_linedef *group_next;         /* Used during texture-alignment */
  struct s_linedef *group_previous;     /* A group gets aligned together */
  struct s_linedef *next;
};   /* linedef and plinedef defined above; gcc chokes if we do it again! */

/* Linedef flags */
#define IMPASSIBLE (0x01)
#define BLOCK_MONSTERS (0x02)
#define TWO_SIDED (0x04)
#define UPPER_UNPEGGED (0x08)
#define LOWER_UNPEGGED (0x10)
#define SECRET_LINEDEF (0x20)
#define BLOCK_SOUND (0x40)
#define NOT_ON_MAP (0x80)
#define ALREADY_ON_MAP (0x100)

/* Linedef types */
#define LINEDEF_NORMAL (0)
#define LINEDEF_NORMAL_DOOR (1)
#define LINEDEF_NORMAL_S1_DOOR (31)
#define LINEDEF_BLUE_S1_DOOR (32)
#define LINEDEF_RED_S1_DOOR (33)
#define LINEDEF_YELLOW_S1_DOOR (34)
#define LINEDEF_S1_OPEN_DOOR (103)
#define LINEDEF_S1_OPEN_DOOR_BLUE (133)
#define LINEDEF_S1_OPEN_DOOR_RED (135)
#define LINEDEF_S1_OPEN_DOOR_YELLOW (137)
#define LINEDEF_WR_OPEN_DOOR (86)
#define LINEDEF_W1_OPEN_DOOR (2)
#define LINEDEF_GR_OPEN_DOOR (46)
#define LINEDEF_SR_OC_DOOR (63)
#define LINEDEF_WR_OC_DOOR (90)
#define LINEDEF_BLAZE_DOOR (117)
#define LINEDEF_BLAZE_S1_DOOR (118)
#define LINEDEF_S1_BLAZE_O_DOOR (112)
#define LINEDEF_SR_BLAZE_OC_DOOR (114)
#define LINEDEF_S1_END_LEVEL (11)
#define LINEDEF_W1_END_LEVEL (52)
#define LINEDEF_S1_SEC_LEVEL (51)
#define LINEDEF_W1_SEC_LEVEL (124)
#define LINEDEF_WR_FAST_CRUSH (77)
#define LINEDEF_WR_LOWER_LIFT (88)
#define LINEDEF_SR_LOWER_LIFT (62)
#define LINEDEF_WR_TURBO_LIFT (120)
#define LINEDEF_SR_TURBO_LIFT (123)
#define LINEDEF_S1_RAISE_AND_CLEAN_FLOOR (20)
#define LINEDEF_S1_RAISE_FLOOR (18)
#define LINEDEF_W1_RAISE_FLOOR (119)
#define LINEDEF_S1_RAISE_STAIRS (7)
#define LINEDEF_S1_LOWER_FLOOR (23)
#define LINEDEF_SCROLL (48)
#define LINEDEF_TELEPORT (97)
/* and so on and so on */

/* Sector specials */
#define RANDOM_BLINK (1)
#define SYNC_FAST_BLINK (0x0c)
#define SYNC_SLOW_BLINK (0x0d)
#define GLOW_BLINK (0x08)
#define SECRET_SECTOR (0x09)
#define NUKAGE1_SPECIAL (5)
#define DEATH_SECTOR (0x0b)

/* Stuff related to an open PWAD we're generating */

typedef struct s_index_entry {
  char name[9];
  unsigned int offset;
  unsigned int length;
  struct s_index_entry *next;
} index_entry, *pindex_entry;

typedef struct s_dump_record {
  FILE *f;
  unsigned int offset_to_index;
  unsigned int lmpcount;
  index_entry *index_entry_anchor;
} dump_record, *pdump_record, *dumphandle;

typedef struct s_musheader {
  char tag[4];     /* MUS[0x1a] */
  short muslength;
  short headerlength;
  short primchannels;
  short secchannels;
  short patches;
  short dummy;
} musheader, *pmusheader;

typedef struct s_patch {
  short number;
  short x;
  short y;
  struct s_patch *next;
} patch, *ppatch;

typedef struct s_custom_texture {
  char *name;
  short xsize;
  short ysize;
  patch *patch_anchor;
  struct s_custom_texture *next;
} custom_texture, *pcustom_texture;

typedef struct s_texture_lmp {
  char *name;
  custom_texture *custom_texture_anchor;
} texture_lmp, *ptexture_lmp;

/* Health, Armor, and Ammo estimates */

typedef struct s_one_haa {
  float health;
  float armor;
  float ammo;
  boolean can_use_shells;
  boolean can_use_rockets;
  boolean can_use_cells;
  boolean has_chaingun;
  boolean has_chainsaw;
  boolean has_backpack;
  boolean has_berserk;
  boolean has_ssgun;
  boolean shells_pending;
  boolean chaingun_pending;
} one_haa, *pone_haa;

typedef struct s_haa {
  one_haa haas[3];
#define ITYTD (0)
#define HMP (1)
#define UV (2)
} haa, *phaa;

typedef struct s_quest {
  short goal;          /* What kind of quest? */
  short tag;           /* If a linedef/switch, what's the tag? */
  short tag2;          /* Another tag, if needed for GATEs etc. */
  short type;          /* What should we do to the tag? */
                       /* Or what's the ID of the key */
  sector *room;        /* What room will the quest let us into? */
  short count;         /* How many rooms in the quest so far? */
  short minrooms;      /* How many rooms at least should it have? */
  short auxtag;        /* Tag of door to open when taking goal */
  linedef *surprise;   /* Linedef to populate after goal room */
  thing *thing;        /* Thing that closed a closed thing-quest */
  struct s_quest *next;  /* For the quest stack */
} quest, *pquest;

/* Values for quest.goal */
#define LEVEL_END_GOAL 101
#define KEY_GOAL 102
#define SWITCH_GOAL 103
#define NULL_GOAL 104
#define ARENA_GOAL 105
#define GATE_GOAL 106

/* Teleport gates */
struct s_gate {
  short in_tag;
  short out_tag;
  short gate_lock;   /* The linedef-type, if any, to open the gate */
  boolean is_entry;  /* Does one enter the room by it the first time? */
  gate *next;
};

/* The Arena */

typedef struct s_arena {
  propertybits props;
#define ARENA_ROOF (0x01)
#define ARENA_PORCH (0x02)
#define ARENA_LAMPS (0x04)
#define ARENA_ARRIVAL_HOLE (0x08)
#define ARENA_NUKAGE (0x10)
  genus *boss;
  int boss_count;
  genus *weapon;
  genus *ammo;
  flat *floor;
  texture *walls;
  boolean placed_health;
  boolean placed_armor;
  boolean placed_ammo;
  boolean placed_weapon;
  short minx, miny, maxx, maxy;
  sector *innersec;
  sector *outersec;
  short fromtag;
  struct s_arena *next;
} arena, *parena;

typedef struct s_level {
  thing *thing_anchor;
  sector *sector_anchor;
  vertex *vertex_anchor;
  sidedef *sidedef_anchor;
  linedef *linedef_anchor;
  boolean used_red;
  boolean used_blue;
  boolean used_yellow;
  int last_tag_used;
  short sl_tag;       /* Tag for thing to activate to open secret level exit */
  short sl_type;      /* Type for ... */
  sector *sl_open_start;  /* The first room the opener can go in */
  boolean sl_open_ok;     /* Is it time to do the opener yet? */
  sector *sl_exit_sector; /* The room the exit switch is in */
  boolean sl_done;        /* Did we done it yet? */
  sector *first_room;
  sector *goal_room;
  int secret_count;
  int dm_count;
  int dm_rho;
  boolean support_misaligns;
  boolean seen_suit;
  boolean seen_map;
  boolean scrolling_keylights;
  int skyclosets;     /* Percent chance of closets being open to the sky */
  int p_new_pillars;
  int p_stair_lamps;
  int p_force_nukage;
  int p_force_sky;
  int p_deep_baths;
  int p_falling_core;
  int p_barrels;
  int p_extwindow;
  int p_extroom;
  int p_rising_room;
  int p_surprise;
  int p_swcloset;
  int p_rational_facing;
  int p_biggest_monsters;
  int p_open_link;
  int p_s1_door;
  int p_special_room;  /* Should be in Style, maybe? */
  int lift_rho;     /* How common are lifts? */
  int amcl_rho;     /* How common are ambush-closets? */
  int maxkeys;      /* How many key or switch quests, at most, to use */
  int barcount;     /* How many door-bars so far? */
  int crushercount; /* How many left-on crushers so far? */
  int hugeness;     /* A one or a two or whatever */
  boolean skullkeys; /* Use skull (not card) keys? */
  boolean use_gates; /* Allowed to use non-exit teleporters? */
  boolean raise_gates; /* Teleport flats raised a bit? */
  boolean all_wide_links;
  boolean no_doors;
  short outside_light_level;  /* I don't know if it's cloudy or bright */
  short bright_light_level;  /* How bright a bright room is */
  short lit_light_level;     /* How bright a working lamp/light is */
  /* These lists are just for memory-freeing purposes */
  style *style_anchor;
  link *link_anchor;
  gate *gate_anchor;
  arena *arena_anchor;
} level, *plevel;

/* The config is the static architectural knowledge and stuff. */
/* It's read from a config file (parts of it, anyway!).        */
typedef struct s_config {
  char *configfile;    /* Name of the configuration file */
  char *configdata;    /* Contents of the configuration file */
  char *outfile;       /* Name of the output file */
  boolean cwadonly;    /* Do we want just the customization lumps? */
  boolean fcontrol;
  boolean fstart;
  boolean ffstart;
  boolean ffend;
  boolean fend;
  unsigned int ranseed;
  unsigned char themecount;   /* How many (non-secret) themes there are */
  unsigned char sthemecount;  /* How many secret themes there are */
  boolean secret_themes;
  boolean lock_themes;
  boolean major_nukage;
  propertybits required_monster_bits;
  propertybits forbidden_monster_bits;
  short minrooms;
  theme *theme_anchor;
  genus *genus_anchor;
  flat *flat_anchor;
  texture *texture_anchor;
  construct *construct_anchor;
  flat *sky_flat;
  flat *water_flat;
  texture *null_texture;
  texture *error_texture;
  texture *gate_exitsign_texture;
  gamebits gamemask;  /* Which games must we be compatible with? */
  int levelcount;       /* How many levels to produce */
  boolean produce_null_lmps;
  boolean do_music;
  boolean do_slinfo;
  boolean do_seclevels;
  boolean do_dm;
  boolean secret_monsters;
  boolean force_secret;
  boolean force_arena;
  boolean force_biggest;
  boolean weapons_are_special;  /* Can weapons (not) be given out as ammo? */
  boolean recess_switches;
  boolean big_weapons;
  boolean big_monsters;
  boolean gunk_channels;
  boolean clights;
  boolean allow_boring_rooms;
  boolean both_doors;
  boolean doorless_jambs;
  float machoh;       /* Macho-factor for Hurt Me Plenty */
  float machou;       /* Macho-factor for Ultraviolins */
  int p_bigify;       /* Percent chance of maybe expanding rooms */
  int usualammo[3];   /* Usual ammo/armor/health for the three hardnesses */
  int usualarmor[3];
  int usualhealth[3];
  int minhealth[3];   /* Minimal OK healths for the three hardnesses */
  boolean immediate_monsters;  /* OK to have monsters in first room? */
  int p_hole_ends_level;
  int p_gate_ends_level;
  int p_use_steps;
  int p_sync_doors;
  int p_grid_gaps;
  int p_pushquest;
  int rad_newtheme;      /* How likely to use a random theme beyond a lock */
  int norm_newtheme;      /* How likely beyond a non-lock link */
  int rad_vary;           /* How much to vary the style beyond a lock */
  int norm_vary;          /* How much beyon a non-lock link */
  boolean monsters_can_teleport;
  boolean window_airshafts;
  int homogenize_monsters;  /* How likely to have all room monsters == */
  int minlight;     /* How dark is dark? */
  int minhuge;
  /* These are *not* actually static */
  int episode, mission, map;  /* What map/mission we're on now. */
  boolean last_mission;       /* This the last one we're doing? */
} config, *pconfig;

/* Lots and lots and lots of functions */
/* And this isn't even all of 'em! */

config *get_config(int argc, char *arg[]);
void NewLevel(level *l, haa *init_haa, config *c);
void DumpLevel(dumphandle dh,config *c,level *l,int episode,int mission,int map);
void FreeLevel(level *l);
dumphandle OpenDump(config *c);
void CloseDump(dumphandle dh);
quest *starting_quest(level *l,config *c);
haa *starting_haa(void);
style *random_style(level *l,config *c);
boolean enough_quest(level *l,sector *s,quest *ThisQuest,config *c);
boolean rollpercent(int percent);
int roll(int zero_to_this_minus_one);
linedef *starting_linedef(level *l,style *ThisStyle,config *c);
int mark_adequate_linedefs(level *l,sector *s,style *ThisStyle,config *c);
int mark_decent_boundary_linedefs(level *l,sector *s,int minlen);
boolean isAdequate(level *l,linedef *ld,style *ThisStyle,config *c);
linedef *random_marked_linedef(level *l,int i);
void unmark_linedefs(level *l);
void embellish_room(level *l,sector *oldsector, haa *haa,
  style *ThisStyle,quest *ThisQuest,boolean should_watermark,
  boolean edges_only, config *c);
boolean grid_room(level *l,sector *oldsector, haa *haa,
  style *ThisStyle,quest *ThisQuest,boolean first_room,config *c);
void enhance_room(level *l,sector *oldsector, haa *haa,
  style *ThisStyle,quest *ThisQuest,boolean first_room,config *c);
void align_textures(level *l,sector *oldsector,config *c);
void gloabl_align_textures(level *l,config *c);
void populate(level *l,sector *oldsector,config *c,haa *ThisHaa,boolean first);
void gate_populate(level *l,sector *s,haa *haa, boolean first, config *c);
link *random_link(level *l,linedef *ld,style *ThisStyle,quest *ThisQuest,
                   config *c);
link *random_open_link(level *l,linedef *ld,style *ThisStyle,quest *ThisQuest,
                       config *c);
link *random_basic_link(level *l,linedef *ld,style *ThisStyle,quest *ThisQuest,
                        config *c);
link *gate_link(level *l,config *c);
sector *generate_room_outline(level *l,linedef *ld,style *ThisStyle,
                              boolean try_reduction,config *c);
int lengthsquared(linedef *ld);
int distancesquared(int x1, int y1, int x2, int y2);
int infinity_norm(int x1, int y1, int x2, int y2);
boolean empty_rectangle(level *l,int x1, int y1, int x2, int y2,
                                 int x3, int y3, int x4, int y4);
boolean empty_left_side(level *l, linedef *ld, int sdepth);
void close_quest(level *l,sector *s,quest *ThisQuest,haa *haa,config *c);
void maybe_push_quest(level *l,sector *s,quest *ThisQuest,config *c);
linedef *make_parallel(level *l,linedef *ld,int depth,linedef *old);
linedef *lefthand_box_ext(level *l,linedef *ldf1,int depth,
                                  style *ThisStyle,config *c,
                                  linedef **nld1, linedef **nld2);
#define lefthand_box(l,ldf1,depth,ThisStyle,c) (lefthand_box_ext(l,ldf1,depth,ThisStyle,c,NULL,NULL))
int facing_along(int x1, int y1, int x2, int y2);
int facing_right_from(int x1, int y1, int x2, int y2);
int facing_right_from_ld(linedef *ld);
linedef *make_linkto(level *l,linedef *ldf,link *ThisLink,
                              style *ThisStyle,config *c, linedef *old);
void e_bl_inner(level *l,linedef *ldf1,linedef *ldf2,link *ThisLink,
                quest *ThisQuest,style *ThisStyle,style *NewStyle,
                short flipstate, haa *haa,config *c);
void e_ol_inner(level *l,linedef *ldf1,linedef *ldf2,link *ThisLink,
                quest *ThisQuest,style *ThisStyle,style *NewStyle,
                haa *haa,config *c);
void establish_basic_link(level *l,linedef *ldf1,linedef *ldf2,link *ThisLink,
                          quest *ThisQuest,style *ThisStyle,style *NewStyle,
                          haa *haa,config *c);
void establish_open_link(level *l,linedef *ldf1,linedef *ldf2,link *ThisLink,
                         quest *ThisQuest,style *ThisStyle,style *NewStyle,
                         haa *haa,config *c);
void establish_link(level *l,linedef *ldf1,linedef *ldf2,link *ThisLink,
                     quest *ThisQuest,style *ThisStyle,style *NewStyle,
                     haa *haa,config *c);
void stairify(level *l,linedef *ldf1,linedef *ldf2,linedef *lde1,
              linedef *lde2,short nearheight,short farheight,
              quest *ThisQuest, style *ThisStyle, config *c);
void paint_room(level *l,sector *s,style *ThisStyle,config *c);
linedef *split_linedef(level *l, linedef *ld, int len, config *c);
boolean link_fitsq(link *ThisLink,quest *ThisQuest);
boolean link_fitsh(linedef *ldf,link *ThisLink,config *c);
boolean link_fitsv(level *l,linedef *ldf1,linedef *ldf2,link *ThisLink);
void Usage0(void);
void Usage(void);
void Usage2(void);
boolean do_switches(int argc,char *argv[],config *c,char *s,int conly);
boolean read_switches(config *c);
boolean nonswitch_config(config *c);
void load_config(config *c);
void unload_config(config *c);
texture *new_texture(config *c, char *name);
texture *find_texture(config *c, char *name);
genus *find_genus(config *c, int thingid);
genus *new_genus(config *c,int thingid);
flat *new_flat(config *c, char *name);
flat *find_flat(config *c, char *name);
theme *new_theme(config *c, char *name, boolean secret);
gate *new_gate(level *l,short in,short out,short lock,boolean entry,config *c);
void patch_upper(linedef *ld,texture *t,config *c);
void patch_lower(linedef *ld,texture *t,config *c);
linedef *flip_linedef(linedef *ld);
sector *make_box_ext(level *l,linedef *ldf1, linedef *ldf2,
                        style *ThisStyle, config *c,
                        linedef **nld1, linedef **nld2);
#define make_box(l,ld1,ld2,st,c) (make_box_ext(l,ld1,ld2,st,c,NULL,NULL))
thing *place_object(level *l,sector *s,config *c,short thingid,int width,
                       int angle,int ax,int ay,int bits);
thing *place_object_in_region(level *l,int minx, int miny, int maxx, int maxy,
                       config *c,short thingid,int width,
                       int angle,int ax,int ay,int bits);
thing *place_required_pickable(level *l,sector *s,config *c,short id);
genus *timely_monster(haa *haa,config *c,int *levels,boolean biggest,int mno);
genus *timely_monster_ex(haa *haa,config *c,int *levels,boolean biggest,
                         int mno,propertybits req);
void update_haa_for_monster(haa *haa,genus *m,int levels,int mno,config *c);
void ammo_value(short ammotype, haa *haa, int *a0, int *a1, int *a2);
void haa_unpend(haa *haa);
void trigger_box(level *l,thing *t,sector *s,short tag,short type,config *c);
void populate_linedef(level *l,linedef *ldnew2,haa *haa,config *c,boolean s);
void find_rec(level *l, sector *s, int *minx, int *miny, int *maxx, int *maxy);
void mid_tile(level *l, sector *s, short *tlx, short *tly, short *thx, short *thy);
linedef *centerpart(level *l,linedef *ld,linedef **ld2,int width,
                    style *ThisStyle,config *c);
texture *texture_for_key(short key, style *s, config *c);
texture *texture_for_bits(propertybits pb, style *s, config *c);
short type_for_key(short key);
void make_lighted(level *l, sector *s, config *c);
short locked_linedef_for(short type,short key,config *c);
void install_gate(level *l,sector *s,style *ThisStyle,haa *ThisHaa,
                  boolean force_exit_style,config *c);
void frame_innersec_ex(level *l,sector *oldsector,sector *innersec,
                      texture *tm, texture *tu, texture *tl,
                      int x1,int y1,int x2,int y2,
                      int x3,int y3,int x4,int y4,
                      config *c,
                      linedef **l1, linedef **l2, linedef **l3, linedef **l4);
#define frame_innersec(l,s,i,tm,tu,tl,x1,y1,x2,y2,x3,y3,x4,y4,c) frame_innersec_ex(l,s,i,tm,tu,tl,x1,y1,x2,y2,x3,y3,x4,y4,c,NULL,NULL,NULL,NULL)
void parallel_innersec_ex(level *l,sector *oldsector,sector *innersec,
                         texture *tm, texture *tu, texture *tl,
                         int minx,int miny,int maxx,int maxy,config *c,
                         linedef **l1, linedef **l2, linedef **l3, linedef **l4);
#define parallel_innersec(l,o,i,tm,tu,tl,ix,iy,ax,ay,c) parallel_innersec_ex(l,o,i,tm,tu,tl,ix,iy,ax,ay,c,NULL,NULL,NULL,NULL)
boolean install_construct(level *l,sector *oldsector,
                       int minx,int miny,int maxx,int maxy,
                       style *ThisStyle,config *c);

void make_music(dumphandle dh,config *c);
void make_slinfo(dumphandle dh,config *c);
void record_custom_textures(dumphandle dh,config *c);
void record_custom_flats(dumphandle dh,config *c,boolean even_unused);
void record_custom_patches(dumphandle dh,config *c,boolean even_unused);

boolean need_secret_level(config *c);
void make_secret_level(dumphandle dh, haa *haa, config *c);
void secretize_config(config *c);
boolean install_sl_exit(level *l,sector *oldsector,haa *ThisHaa,
                        style *ThisStyle, quest *ThisQuest,
                        boolean opens, config *c);

#define NONE -1
#define VERBOSE 0
#define LOG 1
#define NOTE 2
#define WARNING 3
#define ERROR 4
void announce(int announcetype, char *s);

#define RIGHT_TURN (90)
#define LEFT_TURN (270)
void point_from(int x1, int y1, int x2, int y2, int angle, int len,
                int *x3, int *y3);
unsigned short psi_sqrt(int v);
#define linelen(x) (psi_sqrt(lengthsquared(x)))
boolean no_monsters_stuck_on(level *l,linedef *ld1);

flat *random_ceiling0(config *c, style *s);
flat *random_ceilinglight(config *c, style *s);
flat *random_floor0(config *c, style *s);
flat *random_gate(config *c, style *s);
flat *random_doorceiling(config *c,style *s);
flat *random_doorfloor(config *c,style *s);
flat *random_stepfloor(config *c,style *s);
flat *random_nukage1(config *c,style *s);
flat *random_flat0(propertybits pmask, config *c, style *s);
genus *random_thing0(propertybits pmask,config *c,style *s,int minh,int maxh);
texture *random_texture0(propertybits pmask, config *c, style *s);
texture *random_wall0(config *c,style *s);
texture *random_kickplate(config *c,style *s);
texture *random_stepfront(config *c,style *s);
texture *switch0_for(config *c, style *s);
texture *random_support0(config *c, style *s);
texture *random_doorjamb(config *c, style *s);
texture *random_widedoorface(config *c, style *s);
texture *random_widedoorface_ex(config *c, style *s,boolean hi);
texture *random_narrowdoorface(config *c, style *s);
texture *random_narrowdoorface_ex(config *c, style *s,boolean hi);
texture *random_twdoorface(config *c, style *s);
texture *random_tndoorface(config *c, style *s);
texture *random_lockdoorface(config *c, style *s);
texture *random_grating(config *c, style *s);
texture *random_walllight(config *c, style *s);
texture *random_liftface(config *c, style *s);
texture *random_plaque(config *c, style *s);
genus *random_lamp0(config *c, style *s);
genus *random_shortlamp0(config *c, style *s);
genus *random_barrel(config *c, style *s);
genus *random_plant(config *c, style *s);
texture *random_redface(config*c, style *s);
texture *random_blueface(config*c, style *s);
texture *random_yellowface(config*c, style *s);

void place_monsters(level *l,sector *s,config *c,haa *haa);
void place_timely_something(level *l,haa *haa, config *c,int x, int y);
void place_armor(level *l,sector *s,config *c,haa *haa);
void place_ammo(level *l,sector *s,config *c,haa *haa);
void place_health(level *l,sector *s,config *c,haa *haa);
void place_barrels(level *l,sector *s,config *c,haa *haa);
void place_plants(level *l,int allow,sector *s,config *c);

boolean common_texture(sidedef *sd1, sidedef *sd2);
boolean coalignable(texture *t1, texture *t2);

void global_align_textures(level *l,config *c);
void global_align_linedef(level *l, linedef *ld);
void global_align_forward(level *l, linedef *ld);
void global_align_backward(level *l, linedef *ld);
void global_align_group_backbone_forward(level *l,linedef *ld);
void global_align_group_backbone_backward(level *l,linedef *ld);
void global_align_group_etc_forward(level *l,linedef *ld);
void global_align_group_etc_backward(level *l,linedef *ld);

int global_verbosity = 0;    /* Oooh, a global variable! */
boolean ok_to_roll = FALSE;  /* Stop breaking -seed...   */

#ifdef __sun
#pragma pack(2)
#endif /* __sun */

#ifdef BIGEND

void swapshort(unsigned short *i)
{
  unsigned short int t;

  ((char *) &t)[ 0] = ((char *) i)[ 1];
  ((char *) &t)[ 1] = ((char *) i)[ 0];
  *i = t;
}

void swaplong(unsigned long *l)
{
  unsigned long t;

  ((char *) &t)[ 0] = ((char *) l)[ 3];
  ((char *) &t)[ 1] = ((char *) l)[ 2];
  ((char *) &t)[ 2] = ((char *) l)[ 1];
  ((char *) &t)[ 3] = ((char *) l)[ 0];
  *l = t;
}

void swapint(unsigned int *l)
{
  unsigned int t;

  ((char *) &t)[ 0] = ((char *) l)[ 3];
  ((char *) &t)[ 1] = ((char *) l)[ 2];
  ((char *) &t)[ 2] = ((char *) l)[ 1];
  ((char *) &t)[ 3] = ((char *) l)[ 0];
  *l = t;
}

#else

#define swapshort(s)
#define swapint(i)
#define swaplong(l)

#endif

int main(int argc, char *argv[]) {

  /* A stubby but functional main() */

  level ThisLevel;
  haa *ThisHaa = NULL;
  config *ThisConfig;
  int i;
  dumphandle dh;

  printf("SLIGE (build %d)   -- by Dave Chess, chess@theogeny.com\n"
#ifdef RISCOS
/* Porting credit / blame */
         "Ported to RISC OS by Justin Fletcher, doom@movspclr.co.uk\n"
#endif
         "\n", SOURCE_SERIAL);

  ThisConfig = get_config(argc,argv);
  if (ThisConfig==NULL) {
    Usage();
    return 100;
  }
  if (ThisConfig->cwadonly) {
    dh = OpenDump(ThisConfig);
    if (dh==NULL) return 28;
    record_custom_textures(dh,ThisConfig);
    record_custom_flats(dh,ThisConfig,TRUE);   /* record all flats */
    record_custom_patches(dh,ThisConfig,TRUE);   /* and patches */
    CloseDump(dh);
    printf("\nDone: wrote customization WAD %s.\n",ThisConfig->outfile);
    return 0;
  }
  dh = OpenDump(ThisConfig);
  if (dh==NULL) return 28;
  if (ThisConfig->do_slinfo) make_slinfo(dh,ThisConfig);
  if (ThisConfig->do_music) make_music(dh,ThisConfig);

  for (i=0;i<ThisConfig->levelcount;i++) {
    if ((!ThisHaa) || (ThisConfig->mission==1)) {
      if (ThisHaa) free(ThisHaa);
      ThisHaa = starting_haa();
    }
    if ((i+1)==(ThisConfig->levelcount)) ThisConfig->last_mission = TRUE;
    NewLevel(&ThisLevel,ThisHaa,ThisConfig);
    DumpLevel(dh,ThisConfig,&ThisLevel,ThisConfig->episode,
                                       ThisConfig->mission,
                                       ThisConfig->map);
    if (need_secret_level(ThisConfig))
      make_secret_level(dh,ThisHaa,ThisConfig);
    if (ThisConfig->map) ThisConfig->map++;
    if (ThisConfig->mission) ThisConfig->mission++;
    if (ThisConfig->mission==9) {    /* Around the corner */
      ThisConfig->episode++;
      ThisConfig->mission=1;
    }
    FreeLevel(&ThisLevel);
  }
  if (!(ThisConfig->gamemask&DOOMI_BIT)) {
    record_custom_textures(dh,ThisConfig);
    record_custom_flats(dh,ThisConfig,FALSE);
    record_custom_patches(dh,ThisConfig,FALSE);
  }
  CloseDump(dh);
  printf("\nDone: wrote %s.\n",ThisConfig->outfile);
  return 0;

}

/* Open a file ready to dump multiple levels into */
dumphandle OpenDump(config *c)
{
  dumphandle answer;
  struct {
    char tag[4];
    unsigned long lmpcount;
    unsigned long inxoffset;
  } headerstuff;

  answer = (dumphandle)malloc(sizeof (*answer));
  answer->f = fopen(c->outfile,"wb");
  if (answer->f==NULL) {
    fprintf(stderr,"Error opening <%s>.\n",c->outfile);
    perror("Maybe");
    return NULL;
  }
  memcpy(headerstuff.tag,"PWAD",4);
  headerstuff.lmpcount = 0;  /* To be filled in later */
  headerstuff.inxoffset = 0;
  fwrite(&headerstuff,sizeof(headerstuff),1,answer->f);
  answer->offset_to_index = sizeof(headerstuff);  /* Length of the header */
  answer->index_entry_anchor = 0;
  answer->lmpcount = 0;
  return answer;
}

/* Write out the directory, patch up the header, and close the file */
void CloseDump(dumphandle dh)
{
  struct {
    unsigned long offset;
    unsigned long length;
    char lumpname[8];
  } directory_entry;

  index_entry *ie;

  /* Write the index entries */
  for (ie=dh->index_entry_anchor;ie;ie=ie->next) {
    directory_entry.offset = ie->offset;
    swaplong(&(directory_entry.offset));
    directory_entry.length = ie->length;
    swaplong(&(directory_entry.length));
    memset(directory_entry.lumpname,0,8);
    memcpy(directory_entry.lumpname,ie->name,strlen(ie->name));
    fwrite(&directory_entry,sizeof(directory_entry),1,dh->f);
  }

  /* Go back and patch up the header */
  fseek(dh->f,4,SEEK_SET);
  swapint(&(dh->lmpcount));
  fwrite(&(dh->lmpcount),sizeof(unsigned int),1,dh->f);
  swapint(&(dh->lmpcount));
  swapint(&(dh->offset_to_index));
  fwrite(&(dh->offset_to_index),sizeof(unsigned int),1,dh->f);
  swapint(&(dh->offset_to_index));

  /* and that's all! */
  fclose(dh->f);

}

/* Free up all the allocated structures associated with the */
/* level, so we can start on a new one without burning too  */
/* much memory.                                             */
void FreeLevel(level *l)
{
  linedef *ld, *ldn;
  sidedef *sd, *sdn;
  vertex *v, *vn;
  thing *t, *tn;
  sector *s, *sn;
  link *link, *linkn;
  style *style, *stylen;
  arena *arena, *arenan;
  gate *gate, *gaten;

  for (ld=l->linedef_anchor;ld;ld=ldn) {
    ldn = ld->next;
    free(ld);
  }
  l->linedef_anchor = NULL;
  for (sd=l->sidedef_anchor;sd;sd=sdn) {
    sdn = sd->next;
    free(sd);
  }
  l->sidedef_anchor = NULL;
  for (v=l->vertex_anchor;v;v=vn) {
    vn = v->next;
    free(v);
  }
  l->vertex_anchor = NULL;
  for (t=l->thing_anchor;t;t=tn) {
    tn = t->next;
    free(t);
  }
  l->thing_anchor = NULL;
  for (s=l->sector_anchor;s;s=sn) {
    sn = s->next;
    free(s);
  }
  l->sector_anchor = NULL;
  for (link=l->link_anchor;link;link=linkn)  {
    linkn = link->next;
    free(link);
  }
  l->link_anchor = NULL;
  for (style=l->style_anchor;style;style=stylen) {
    stylen = style->next;
    free(style);
  }
  l->style_anchor = NULL;
  for (arena=l->arena_anchor;arena;arena=arenan) {
    arenan = arena->next;
    free(arena);
  }
  l->arena_anchor = NULL;
  for (gate=l->gate_anchor;gate;gate=gaten) {
    gaten = gate->next;
    free(gate);
  }
  l->gate_anchor = NULL;
}

/* Record the information about a new lmp of the given size */
void RegisterLmp(dumphandle dh,char *s,unsigned int size)
{
  index_entry *ie, *ie2;

  ie = (index_entry *)malloc(sizeof(*ie));
  ie->next = NULL;
  /* This list really has to be in frontwards order! */
  if (dh->index_entry_anchor==NULL) {
    dh->index_entry_anchor = ie;
  } else {
    for (ie2=dh->index_entry_anchor;ie2->next;ie2=ie2->next);
    ie2->next = ie;
  }
  strcpy(ie->name,s);
  ie->offset = dh->offset_to_index;
  ie->length = size;
  dh->offset_to_index+=size;
  dh->lmpcount++;
}

/* Given a dumphandle, a music header, a music buffer, a lump name, */
/* and for some reason a config, do what's necessary to record it   */
/* in the file and index and stuff. */
void record_music(dumphandle dh,musheader *mh,byte *buf,char *s,config *c)
{
  unsigned int lsize;

  lsize = mh->headerlength + mh->muslength;
  RegisterLmp(dh,s,lsize);
  swapshort((unsigned short *)&(mh->muslength));
  swapshort((unsigned short *)&(mh->headerlength));
  swapshort((unsigned short *)&(mh->primchannels));
  swapshort((unsigned short *)&(mh->secchannels));
  swapshort((unsigned short *)&(mh->patches));
  swapshort((unsigned short *)&(mh->dummy));
  fwrite(mh,sizeof(musheader),1,dh->f);  /* Write fixed header */
  swapshort((unsigned short *)&(mh->muslength));
  swapshort((unsigned short *)&(mh->headerlength));
  swapshort((unsigned short *)&(mh->primchannels));
  swapshort((unsigned short *)&(mh->secchannels));
  swapshort((unsigned short *)&(mh->patches));
  swapshort((unsigned short *)&(mh->dummy));
  fwrite(buf,mh->patches*sizeof(short)+mh->muslength,1,dh->f);
}

/* Make the special SLINFO lmp, containing whatever we like */
void make_slinfo(dumphandle dh, config *c)
{
#ifdef RISCOS
  static char slinfo[100]; /* sprintf takes a char *, byte is unsigned */
#else
  static byte slinfo[100];
#endif

  sprintf(slinfo,"SLIGE (%d) %d",SOURCE_SERIAL,c->ranseed);
  RegisterLmp(dh,"SLINFO",(unsigned int)strlen(slinfo)+1);
  fwrite(slinfo,strlen(slinfo)+1,1,dh->f);

}  /* end make_slinfo() */

/* Write out a PWAD containing just the THINGS, LINEDEFS, SIDEDEFS, */
/* VERTEXES, and SECTORS for the given episode/mission.  The user   */
/* will have to run a nodebuilder and reject mapper hisself.        */
void DumpLevel(dumphandle dh,config *c,level *l,int episode,int mission,int map)
{
  unsigned int i;
  sector *pSector;
  thing *pThing;
  vertex *pVertex;
  linedef *pLinedef;
  sidedef *pSidedef;

  struct {
    short x;
    short y;
    short angle;
    short type;
    short options;
  } rawthing;

  struct {
    short floor_height;
    short ceiling_height;
    char floor_flat[8];
    char ceiling_flat[8];
    short light_level;
    short special;
    short tag;
  } rawsector;

  struct {
    short x;
    short y;
  } rawvertex;

  struct {
    short x_offset;
    short y_offset;
    char upper_texture[8];
    char lower_texture[8];
    char middle_texture[8];
    short sector;
  } rawsidedef;

  struct {
    short from;
    short to;
    short flags;
    short type;
    short tag;
    short right;
    short left;
  } rawlinedef;

  char sb[9];

  /* Register the zero-length marker entry */

  if (map==0) {
    sprintf(sb,"E%dM%d",episode,mission);
  } else {
    sprintf(sb,"MAP%02d",map);
  }
  RegisterLmp(dh,sb,0);

  /* Number all items, register in the directory */

  /* Number and count the things, register */
  for (i=0,pThing=l->thing_anchor;pThing!=NULL;i++) {
    pThing->number = i;
    pThing = pThing->next;
  }
  RegisterLmp(dh,"THINGS",i*10);

  /* Count the number of linedefs, register */
  for (i=0,pLinedef=l->linedef_anchor;pLinedef!=NULL;i++) {
    pLinedef->number = i;
    pLinedef = pLinedef->next;
  }
  RegisterLmp(dh,"LINEDEFS",i*14);

  /* Count the number of sidedefs, register */
  for (i=0,pSidedef=l->sidedef_anchor;pSidedef!=NULL;i++) {
    pSidedef->number = i;
    pSidedef = pSidedef->next;
  }
  RegisterLmp(dh,"SIDEDEFS",i*30);

  /* Count the number of vertexes, register */
  for (i=0,pVertex=l->vertex_anchor;pVertex!=NULL;i++) {
    pVertex->number = i;
    pVertex = pVertex->next;
  }
  RegisterLmp(dh,"VERTEXES",i*4);

  if (c->produce_null_lmps) {
    RegisterLmp(dh,"SEGS",0);
    RegisterLmp(dh,"SSECTORS",0);
    RegisterLmp(dh,"NODES",0);
  }

  /* Count the number of sectors, register */
  for (i=0,pSector=l->sector_anchor;pSector!=NULL;i++) {
    pSector->number = i;
    pSector = pSector->next;
  }
  RegisterLmp(dh,"SECTORS",i*26);

  if (c->produce_null_lmps) {
    RegisterLmp(dh,"REJECT",0);
    RegisterLmp(dh,"BLOCKMAP",0);
  }

  /* Now actually write all those lmps */
  for (pThing=l->thing_anchor;pThing!=NULL;pThing=pThing->next) {
    rawthing.x = pThing->x;
    rawthing.y = pThing->y;
    rawthing.angle = pThing->angle;
    rawthing.type = pThing->genus->thingid;
    rawthing.options = pThing->options;
    swapshort((unsigned short *)&(rawthing.x));
    swapshort((unsigned short *)&(rawthing.y));
    swapshort((unsigned short *)&(rawthing.angle));
    swapshort((unsigned short *)&(rawthing.type));
    swapshort((unsigned short *)&(rawthing.options));
#ifdef ALIGN_STRUCTS
    fwrite(&rawthing,sizeof(rawthing)-2,1,dh->f);
#else
    fwrite(&rawthing,sizeof(rawthing),1,dh->f);
#endif
  }

  /* and all the linedefs */
  for (pLinedef=l->linedef_anchor;pLinedef!=NULL;pLinedef=pLinedef->next) {
    rawlinedef.from = (pLinedef->from)->number;
    rawlinedef.to = (pLinedef->to)->number;
    rawlinedef.flags = pLinedef->flags;
    rawlinedef.type = pLinedef->type;
    rawlinedef.tag = pLinedef->tag;
    if (pLinedef->right == NULL) {
      rawlinedef.right = (short)0xFFFF;     /* actually an error, eh? */
    } else {
      rawlinedef.right = (pLinedef->right)->number;
    }
    if (pLinedef->left == NULL) {
      rawlinedef.left = (short)0xFFFF;
    } else {
      rawlinedef.left = (pLinedef->left)->number;
    }
    swapshort((unsigned short *)&(rawlinedef.from));
    swapshort((unsigned short *)&(rawlinedef.to));
    swapshort((unsigned short *)&(rawlinedef.flags));
    swapshort((unsigned short *)&(rawlinedef.type));
    swapshort((unsigned short *)&(rawlinedef.tag));
    swapshort((unsigned short *)&(rawlinedef.right));
    swapshort((unsigned short *)&(rawlinedef.left));
#ifdef ALIGN_STRUCTS
    fwrite(&rawlinedef,sizeof(rawlinedef)-2,1,dh->f);
#else
    fwrite(&rawlinedef,sizeof(rawlinedef),1,dh->f);
#endif
  }

  /* and all the sidedefs */
  for (pSidedef=l->sidedef_anchor;pSidedef!=NULL;pSidedef=pSidedef->next) {
    rawsidedef.x_offset = pSidedef->x_offset;
    rawsidedef.y_offset = pSidedef->y_offset;
    memset(rawsidedef.upper_texture,0,8);
    memcpy(rawsidedef.upper_texture,pSidedef->upper_texture->realname,strlen(pSidedef->upper_texture->realname));
    pSidedef->upper_texture->used = TRUE;
    memset(rawsidedef.lower_texture,0,8);
    memcpy(rawsidedef.lower_texture,pSidedef->lower_texture->realname,strlen(pSidedef->lower_texture->realname));
    pSidedef->lower_texture->used = TRUE;
    memset(rawsidedef.middle_texture,0,8);
    memcpy(rawsidedef.middle_texture,pSidedef->middle_texture->realname,strlen(pSidedef->middle_texture->realname));
    pSidedef->middle_texture->used = TRUE;
    rawsidedef.sector = (pSidedef->sector)->number;
    swapshort((unsigned short *)&(rawsidedef.x_offset));
    swapshort((unsigned short *)&(rawsidedef.y_offset));
    swapshort((unsigned short *)&(rawsidedef.sector));
#ifdef ALIGN_STRUCTS
    fwrite(&rawsidedef,sizeof(rawsidedef)-2,1,dh->f);
#else
    fwrite(&rawsidedef,sizeof(rawsidedef),1,dh->f);
#endif
  }

  /* and all the vertexes */
  for (pVertex=l->vertex_anchor;pVertex!=NULL;pVertex=pVertex->next) {
    rawvertex.x = pVertex->x;
    rawvertex.y = pVertex->y;
    swapshort((unsigned short *)&(rawvertex.x));
    swapshort((unsigned short *)&(rawvertex.y));
    fwrite(&rawvertex,sizeof(rawvertex),1,dh->f);
  }

  /* and finally all the sectors */
  for (pSector=l->sector_anchor;pSector!=NULL;pSector=pSector->next) {

    rawsector.floor_height = pSector->floor_height;
    rawsector.ceiling_height = pSector->ceiling_height;
    memset(rawsector.floor_flat,0,8);
    memcpy(rawsector.floor_flat,pSector->floor_flat->name,strlen(pSector->floor_flat->name));
    pSector->floor_flat->used = TRUE;
    memset(rawsector.ceiling_flat,0,8);
    memcpy(rawsector.ceiling_flat,pSector->ceiling_flat->name,strlen(pSector->ceiling_flat->name));
    pSector->ceiling_flat->used = TRUE;
    rawsector.light_level = pSector->light_level;
    rawsector.special = pSector->special;
    rawsector.tag = pSector->tag;
    swapshort((unsigned short *)&(rawsector.floor_height));
    swapshort((unsigned short *)&(rawsector.ceiling_height));
    swapshort((unsigned short *)&(rawsector.light_level));
    swapshort((unsigned short *)&(rawsector.special));
    swapshort((unsigned short *)&(rawsector.tag));
#ifdef ALIGN_STRUCTS
    fwrite(&rawsector,sizeof(rawsector)-2,1,dh->f);
#else
    fwrite(&rawsector,sizeof(rawsector),1,dh->f);
#endif
  }

}  /* end DumpLevel */

/* Get the next unused tag for the level */
short new_tag(level *l)
{
  return (short)++(l->last_tag_used);
}

/* Get an unused-color key for the level (if any), and use it. */
/* Zero if all are used. */
short new_key(level *l)
{
  if ((!l->used_red)&&rollpercent(33)) {
    l->used_red = TRUE;
    return (l->skullkeys) ? ID_REDKEY : ID_REDCARD;
  } else if ((!l->used_blue)&&rollpercent(50)) {
    l->used_blue = TRUE;
    return (l->skullkeys) ? ID_BLUEKEY : ID_BLUECARD;
  } else if ((!l->used_yellow)) {
    l->used_yellow = TRUE;
    return (l->skullkeys) ? ID_YELLOWKEY : ID_YELLOWCARD;
  } else return 0;
}

/* Remove a vertex from the level.  Frees the memory, but */
/* doesn't do anything about linedefs nor nothin'.        */
void delete_vertex(level *l, vertex *v)
{
  vertex *v1;

  if (v==l->vertex_anchor) {
    l->vertex_anchor = v->next;
  } else {
    for (v1=l->vertex_anchor;v1;v1=v1->next) {
      if (v1->next==v) {
        v1->next=v->next;
        break;
      }
    }
  }
  free(v);  /* oh, that'll help a lot, eh? */
}

/* Add a vertex to the given level at the given place.  Return it. */
vertex *new_vertex(level *l,int x,int y)
{
  vertex *answer;

  answer = (vertex *)malloc(sizeof(*answer));
  answer->x = x;
  answer->y = y;
  answer->marked = 0;
  answer->next = l->vertex_anchor;
  l->vertex_anchor = answer;
  return answer;
}

/* Remove a linedef from the level.  Frees the memory, but */
/* doesn't do anything about sidedefs nor nothin'.         */
void delete_linedef(level *l, linedef *ld)
{
  linedef *ld1;

  if (ld==l->linedef_anchor) {
    l->linedef_anchor = ld->next;
  } else {
    for (ld1=l->linedef_anchor;ld1;ld1=ld1->next) {
      if (ld1->next==ld) {
        ld1->next=ld->next;
        break;
      }
    }
  }
  free(ld);  /* ooohhh, look, he freed something! */
}

/* Add a linedef to the given level between the given vertexes.  No  */
/* sidedefs or anything are filled in.                               */
linedef *new_linedef(level *l,vertex *from,vertex *to)
{
  linedef *answer;

  answer = (linedef *)malloc(sizeof(*answer));
  answer->from = from;
  answer->to = to;
  answer->flags = 0;
  answer->type = LINEDEF_NORMAL;
  answer->tag = 0;
  answer->left = NULL;
  answer->right = NULL;
  answer->group_next = NULL;
  answer->group_previous = NULL;
  answer->next = l->linedef_anchor;
  answer->marked = 0;
  l->linedef_anchor = answer;
  return answer;
}

/* Return a new sector for the given level */
sector *new_sector(level *l,short fh, short ch, flat *ft, flat *ct)
{
  sector *answer;

  if ((ft==NULL) || (ct==NULL))
    announce(WARNING,"Null flat in new_sector.");
  answer = (sector *)malloc(sizeof(*answer));
  answer->floor_height = fh;
  answer->ceiling_height = ch;
  answer->floor_flat = ft;
  answer->ceiling_flat = ct;
  answer->light_level = 0;
  answer->special = 0;
  answer->tag = 0;
  answer->marked = 0;
  answer->style = NULL;
  answer->entry_x = answer->entry_y = 0;
  answer->findrec_data_valid = FALSE;
  answer->has_key = FALSE;
  answer->has_dm = FALSE;
  answer->has_dm_weapon = FALSE;
  answer->middle_enhanced = FALSE;
  answer->gate = NULL;
  answer->next = l->sector_anchor;
  l->sector_anchor = answer;
  return answer;
}

/* Return a new sector just like the old sector (mostly) */
sector *clone_sector(level *l,sector *s)
{
  sector *answer;
  answer = new_sector(l,s->floor_height,s->ceiling_height,
                        s->floor_flat,s->ceiling_flat);
  answer->style = s->style;
  answer->light_level = s->light_level;
  return answer;
}

/* A new sidedef, similarly, with sensible defaults */
sidedef *new_sidedef(level *l, sector *s, config *c)
{
  sidedef *answer;

  if (s==NULL) announce(ERROR,"Null sector passed to new_sidedef!");
  answer = (sidedef *)malloc(sizeof(*answer));
  answer->x_offset = 0;
  answer->x_misalign = 0;
  answer->y_offset = 0;
  answer->y_misalign = 0;
  answer->upper_texture = c->null_texture;
  answer->lower_texture = c->null_texture;
  answer->middle_texture = c->error_texture;
  answer->sector = s;
  answer->isBoundary = 1;            /* Do we use this sensibly? */
  answer->next = l->sidedef_anchor;
  l->sidedef_anchor = answer;
  return answer;
}

/* Put down a new thing as given */
thing *new_thing(level *l, int x, int y, short angle, short type,
                          short options, config *c)
{
  thing *answer;

  if (type==ID_ELEC) announce(VERBOSE,"Tech column");
  if (type==ID_CBRA) announce(VERBOSE,"Candelabra");
  if (type==ID_LAMP2) announce(VERBOSE,"Lamp2");
  if (type==ID_TLAMP2) announce(VERBOSE,"Tlamp2");
  if (type==ID_LAMP) announce(VERBOSE,"Lamp");
  answer = (thing *)malloc(sizeof(*answer));
  answer->x = (short)x;
  answer->y = (short)y;
  answer->angle = angle;
  answer->genus = find_genus(c,type);
  answer->options = options;
  answer->next = l->thing_anchor;
  l->thing_anchor = answer;
  return answer;
}

/* Return a new arena approprite for the level. */
arena *new_arena(level *l, config *c)
{
  arena *answer = (arena *)malloc(sizeof(*answer));
  int bossno;

  answer->boss_count = 1;   /* Default */

  if (c->mission==8) {       /* Do episode-ends canonically */
    if (c->episode==1) {
      bossno = 0;
    } else if (c->episode==2) {
      bossno = 1;
    } else {
      bossno = 2;
    }
  } else if (c->map==7) {
    bossno = 3;
#ifdef USING_SPAWNER
  } else if (c->map==30) {   /* Including the end of DooM II, eventually */
    bossno=666;
#endif
  } else if (c->map) {       /* Otherwise a random DooM II boss, */
    bossno = roll(7);
  } else {                   /* Or a random DooM I boss. */
    bossno = roll(3);
  }

  /*   How can we configify all the monsters and weapons in here??     */

  switch (bossno) {
    case 0:  /* Baron Brothers */
      answer->boss = find_genus(c,ID_BARON);
      answer->boss_count = 2;
      if (rollpercent(75)) {
        answer->weapon = find_genus(c,ID_LAUNCHER);
        answer->ammo = find_genus(c,ID_ROCKBOX);
      } else {
        answer->weapon = find_genus(c,ID_CHAINGUN);
        answer->ammo = find_genus(c,ID_BULBOX);
      }
      break;
    case 1:  /* Cybie */
      answer->boss = find_genus(c,ID_CYBER);
      if (rollpercent(75)) {
        answer->weapon = find_genus(c,ID_LAUNCHER);
        answer->ammo = find_genus(c,ID_ROCKBOX);
      } else {
        answer->weapon = find_genus(c,ID_BFG);
        answer->ammo = find_genus(c,ID_CELLPACK);
      }
      break;
    case 2:  /* Spiderboss */
      answer->boss = find_genus(c,ID_SPIDERBOSS);
      if (rollpercent(75)) {
        answer->weapon = find_genus(c,ID_BFG);
        answer->ammo = find_genus(c,ID_CELLPACK);
      } else {
        answer->weapon = find_genus(c,ID_LAUNCHER);
        answer->ammo = find_genus(c,ID_ROCKBOX);
      }
      break;
    case 3:  /* Two mancubi (for MAP07, random) */
      answer->boss = find_genus(c,ID_MANCUB);
      answer->boss_count = 2;
      if (rollpercent(75)) {
        answer->weapon = find_genus(c,ID_LAUNCHER);
        answer->ammo = find_genus(c,ID_ROCKBOX);
      } else {
        answer->weapon = find_genus(c,ID_PLASMA);
        answer->ammo = find_genus(c,ID_CELLPACK);
      }
      break;
    case 4:  /* Two pains */
      answer->boss = find_genus(c,ID_PAIN);
      answer->boss_count = 2;
      if (rollpercent(50)) {
        answer->weapon = find_genus(c,ID_CHAINGUN);
        answer->ammo = find_genus(c,ID_BULBOX);
      } else {
        answer->weapon = find_genus(c,ID_PLASMA);
        answer->ammo = find_genus(c,ID_CELLPACK);
      }
      break;
    case 5:
    case 6:
      switch (roll(2)) {
        case 0: answer->boss = find_genus(c,ID_ARCHIE); break;
        default: answer->boss = find_genus(c,ID_ARACH); break;
      }
      answer->boss_count = 2;
      if (rollpercent(75)) {
        answer->weapon = find_genus(c,ID_LAUNCHER);
        answer->ammo = find_genus(c,ID_ROCKBOX);
      } else {
        answer->weapon = find_genus(c,ID_PLASMA);
        answer->ammo = find_genus(c,ID_CELLPACK);
      }
      break;
    case 666:
      /* Just what are we going to do here? */
      answer->boss = find_genus(c,ID_BRAIN);
      answer->weapon = find_genus(c,ID_LAUNCHER);
      answer->ammo = find_genus(c,ID_ROCKBOX);
      break;
    default:
      announce(ERROR,"Arena missing a boss?");
  }

  answer->props = 0;
  if (rollpercent(20)) answer->props |= ARENA_ROOF;
  if (rollpercent(20)) answer->props |= ARENA_PORCH;
  if (rollpercent(20)) answer->props |= ARENA_LAMPS;
  if (rollpercent(20)) answer->props |= ARENA_ARRIVAL_HOLE;
  if (rollpercent(10+l->p_force_nukage)) answer->props |= ARENA_NUKAGE;

  if (answer->props & ARENA_ROOF) {
    answer->floor = random_flat0(FLOOR,c,NULL);    /* These NULLs OK? */
    answer->walls = random_texture0(WALL,c,NULL);
  } else {
    answer->floor = random_flat0(OUTDOOR,c,NULL);
    answer->walls = random_texture0(OUTDOOR,c,NULL);
  }
  answer->placed_health = FALSE;
  answer->placed_armor = FALSE;
  answer->placed_ammo = FALSE;
  answer->placed_weapon = FALSE;
  answer->fromtag = 0;
  answer->next = l->arena_anchor;
  l->arena_anchor = answer;
  return answer;
}

/* Between two points */
int distancesquared(int x1, int y1, int x2, int y2)
{
  int xd,yd;

  xd = (x2-x1);
  yd = (y2-y1);

  return xd*xd + yd*yd;
}

/* Between two points, simple DOOM algorithm */
int infinity_norm(int x1, int y1, int x2, int y2)
{
  int xd,yd;

  xd = abs(x2-x1);
  yd = abs(y2-y1);

  if (xd>yd) return xd;
    else return yd;
}

/* length-sqaured of a linedef */
int lengthsquared(linedef *ld)
{
  int xd,yd;

  xd = (ld->to->x) - (ld->from->x);
  yd = (ld->to->y) - (ld->from->y);
  return xd*xd + yd*yd;
}

/* Return a quest for the very start of the game; always level-end, */
/* consult the config for length and stuff.                         */
quest *starting_quest(level *l,config *c)
{
  quest *answer;

  answer = (quest *)malloc(sizeof(*answer));
  answer->goal = LEVEL_END_GOAL;
  answer->room = NULL; /* won't be used, because this is always stack bottom */
  answer->tag = 0;     /* not a linedef goal */
  answer->type = LINEDEF_S1_END_LEVEL;
  answer->count = 0;   /* no rooms yet! */
  answer->minrooms = c->minrooms;
  answer->auxtag = 0;
  answer->thing = NULL;
  answer->surprise = NULL;
  answer->next = NULL;   /* only one so far */

  return answer;
}

/* Return a health/armor/ammo estimate for the game start */
haa *starting_haa(void)
{
  haa *answer;
  int i;

  answer = (haa *)malloc(sizeof(* answer));

  for (i=ITYTD;i<=UV;i++) {
    answer->haas[i].health = (float)100;
    answer->haas[i].ammo = (float)500;
    answer->haas[i].armor = (float)0;
    answer->haas[i].can_use_shells = 0;
    answer->haas[i].shells_pending = 0;
    answer->haas[i].has_chaingun = 0;
    answer->haas[i].chaingun_pending = 0;
    answer->haas[i].has_chainsaw = 0;
    answer->haas[i].has_backpack = 0;
    answer->haas[i].has_berserk = 0;
    answer->haas[i].has_ssgun = 0;
    answer->haas[i].can_use_rockets = 0;
    answer->haas[i].can_use_cells = 0;
  }

  return answer;
}

/* Mark each boundary linedef from the given sector which isn't */
/* already in obvious use, and which is at least minlen long.   */
int mark_decent_boundary_linedefs(level *l,sector *s,int minlen)
{
  linedef *ld;
  sidedef *sd;
  int answer = 0;

  minlen = minlen * minlen;
  for (ld = l->linedef_anchor;ld;ld=ld->next) {
    if (ld->left) continue;
    if (ld->type) continue;
    sd = ld->right;
    if (sd)
      if (sd->sector == s)
        if (sd->isBoundary)
          if (ld->type==0)
            if (lengthsquared(ld)>=minlen) {
              ld->marked = 1;
              answer++;
            }
  }
  return answer;
}

/* Look at each linedef out of the given sector.  Mark it if it's  */
/* reasonable to consider putting a room on the other side of it.  */
/* Return the number of linedefs marked.                           */
int mark_adequate_linedefs(level *l,sector *s,style *ThisStyle,config *c)
{
  linedef *ld;
  sidedef *sd;
  int answer = 0;

  for (ld = l->linedef_anchor;ld;ld=ld->next) {
    sd = ld->right;
    if (sd)
      if (sd->sector == s)
        if (sd->isBoundary)
          if (isAdequate(l,ld,ThisStyle,c)) {
            ld->marked = 1;
            answer++;
          }
  }
  return answer;
}

/* Given that there are i marked linedefs, return a */
/* random one of them.  NULL if zero.               */
linedef *random_marked_linedef(level *l,int i)
{
  linedef *ld;

  if (i==0) return NULL;
  i = roll(i);
  for (ld=l->linedef_anchor;ld;ld=ld->next) {
    if (ld->marked) {
      if (i==0) return ld;
      i--;
    }
  }

  /* Gosh, I *hope* we never get here... */
  announce(ERROR,"Not enough marked linedefs!");
  return NULL;

}

/* Reset all the linedef marks */
void unmark_linedefs(level *l)
{
  linedef *ld;
  for (ld=l->linedef_anchor;ld;ld=ld->next) ld->marked = 0;
}

/* MSCV++'s rand() seems to be utter junk! */
int bigrand(void)
{
#ifdef RAND_SUCKS
  int answer;
  answer = rand() ^ (rand()<<16) ^ (rand()>>16);  /* Help much? */
  if (answer==0) answer = rand();  /* Last-ditch effort! */
  return answer;
#else
  return rand();
#endif
}

/* Roll a zero-origin n-sided die */
int roll(int n)
{
  assert(ok_to_roll);
  if (n<1) return 0;
  return ((bigrand()>>4) % n);
}

/* Return 1 n percent of the time, else 0 */
boolean rollpercent(int n)
{
  return (roll(100)<n);
}

/*
   Calculates the square root of a 32-bit number.
   Found somewhere on the net.
*/
unsigned short psi_sqrt(int v)
{
    register int t = 1L << 30, r = 0, s;

#define PSISTEP(k) s = t + r; r >>= 1; if (s <= v) { v -= s; r |= t; }

    PSISTEP(15); t >>= 2;
    PSISTEP(14); t >>= 2;
    PSISTEP(13); t >>= 2;
    PSISTEP(12); t >>= 2;
    PSISTEP(11); t >>= 2;
    PSISTEP(10); t >>= 2;
    PSISTEP(9); t >>= 2;
    PSISTEP(8); t >>= 2;
    PSISTEP(7); t >>= 2;
    PSISTEP(6); t >>= 2;
    PSISTEP(5); t >>= 2;
    PSISTEP(4); t >>= 2;
    PSISTEP(3); t >>= 2;
    PSISTEP(2); t >>= 2;
    PSISTEP(1); t >>= 2;
    PSISTEP(0);

    return (unsigned short) r;
}

/* Find a flat with the given name, creating one if */
/* it doesn't already exist.                        */
flat *find_flat(config *c, char *name)
{
  flat *t = NULL;

  for (t=c->flat_anchor;t;t=t->next)
    if (!strcmp(name,t->name)) return t;
  return new_flat(c,name);
}

/* Return a new flat with the given name */
flat *new_flat(config *c,char *name)
{
  flat *answer;

  answer = (flat *)malloc(sizeof(*answer));
  memset(answer->name,0,9);
  memcpy(answer->name,name,strlen(name));
  answer->gamemask = DOOM0_BIT | DOOM1_BIT | DOOM2_BIT | DOOMC_BIT | DOOMI_BIT;
  answer->compatible = 0;
  answer->props = 0;
  answer->used = FALSE;
  answer->next = c->flat_anchor;
  c->flat_anchor = answer;
  return answer;
}

/* Return a new gate with the given attributes and stuff */
gate *new_gate(level *l, short intag, short outtag, short lock,
               boolean entry, config *c)
{
  gate *answer = (gate *)malloc(sizeof(*answer));
  answer->in_tag = intag;
  answer->out_tag = outtag;
  answer->gate_lock = lock;
  answer->is_entry = entry;
  answer->next = l->gate_anchor;
  l->gate_anchor = answer;
  return answer;
}

/* Return a new theme with the given name and secretness.  Non-secret */
/* themes go at the start of the list, secret ones at the end */
theme *new_theme(config *c,char *name,boolean secret)
{
  theme *answer = (theme *)malloc(sizeof(*answer));
  theme *t;

  answer->name = strdup(name);
  answer->secret = secret;
  if (!secret) {   /* Stick it at the end of the non-secrets */
    c->themecount++;
    if ((!c->theme_anchor)||c->theme_anchor->secret) {
      answer->next = c->theme_anchor;
      c->theme_anchor = answer;
    } else {
      for (t=c->theme_anchor;(t->next)&&!t->next->secret;t=t->next) {};
      answer->next = t->next;
      t->next = answer;
    }
  } else {         /* Stick it at the very end */
    c->sthemecount++;
    answer->next = NULL;
    if (c->theme_anchor) {
      for (t=c->theme_anchor;t->next;t=t->next) {};
      t->next = answer;
    } else {
      c->theme_anchor = answer;
    }
  }
  return answer;
}

/* Return a new monster-class with the given thingid */
genus *new_monster(config *c, int thingid)
{
  genus *answer;
  int i;

  answer = new_genus(c,thingid);
  answer->bits |= MONSTER;
  answer->bits &= ~PICKABLE;   /* Can't pick up a monster! */
  for (i=0;i<3;i++) {
    answer->ammo_to_kill[i] = (float)1000;   /* Any reason to have defaults? */
    answer->damage[i] = (float)1000;
    answer->altdamage[i] = (float)1000;
  }
  answer->ammo_provides = (float)0;
  return answer;
}

/* Return a new genus with the given thingid */
genus *new_genus(config *c,int thingid)
{
  genus *answer;

  answer = (genus *)malloc(sizeof(*answer));
  /* Default mask */
  answer->gamemask = DOOM0_BIT|DOOM1_BIT|DOOM2_BIT|DOOMC_BIT|DOOMI_BIT;
  answer->compatible = ~(unsigned long)0;     /* Assume all themes OK */
  answer->thingid = thingid;
  answer->width = 65;  /* Sort of sensible default */
  answer->height = 56; /* Just "not tall" */
  answer->marked = 0;
  answer->next = c->genus_anchor;
  answer->bits = PICKABLE;  /* A plausible default? */
  c->genus_anchor = answer;
  return answer;
}

/* Return a monster-class with the given thingid, */
/* creating it first if needed.                   */
genus *find_monster(config *c, int thingid)
{
  genus *g;

  for (g=c->genus_anchor;g;g=g->next) {
    if (g->thingid==thingid) return g;
  }
  return new_monster(c,thingid);
}

/* Return a thing-class with the given thingid, */
/* creating it first if needed.                 */
genus *find_genus(config *c, int thingid)
{
  genus *g;

  for (g=c->genus_anchor;g;g=g->next) {
    if (g->thingid==thingid) return g;
  }
  return new_genus(c,thingid);
}

/* Find a texture with the given name, creating one if */
/* it doesn't already exist.                           */
texture *find_texture(config *c, char *name)
{
  texture *t = NULL;

  for (t=c->texture_anchor;t;t=t->next)
    if (!strcmp(name,t->name)) return t;
  return new_texture(c,name);
}

/* Return a new texture with the given name */
texture *new_texture(config *c,char *name)
{
  texture *answer;

  answer = (texture *)malloc(sizeof(*answer));
  memset(answer->name,0,9);
  memcpy(answer->name,name,strlen(name));
  answer->realname = answer->name;
  answer->gamemask = DOOM0_BIT | DOOM1_BIT | DOOM2_BIT | DOOMC_BIT | DOOMI_BIT;
  answer->compatible = 0;
  answer->core = 0;
  answer->props = 0;     /* Filled in later */
  answer->width = 256;   /* Or some fraction thereof! */
  answer->height = 128;  /* Except sometimes */
  answer->y_hint = 5;
  answer->y_bias = 0;
  answer->subtle = NULL;
  answer->switch_texture = NULL;
  answer->used = FALSE;
  answer->next = c->texture_anchor;
  c->texture_anchor = answer;
  return answer;
}

/* Split the given linedef at the given distance along it. */
/* Return the new (after-splitpoint) linedef.              */
linedef *split_linedef(level *l, linedef *ld, int len, config *c)
{
  double ratio;
  int dx,dy;
  vertex *v;
  linedef *answer;

  assert(len>0);

  ratio = (double)len / (double)linelen(ld);
  dx = (int)(ratio * (double)(ld->to->x - ld->from->x));
  dy = (int)(ratio * (double)(ld->to->y - ld->from->y));
  v = new_vertex(l,ld->from->x+dx,ld->from->y+dy);
  answer = new_linedef(l,v,ld->to);
  ld->to = v;
  answer->flags = ld->flags;
  answer->type = ld->type;
  answer->tag = ld->tag;
  answer->group_previous = ld;
  answer->group_next = ld->group_next;
  if (answer->group_next) answer->group_next->group_previous = answer;
  ld->group_next = answer;
  if (ld->right) {
    answer->right = new_sidedef(l,ld->right->sector,c);
    answer->right->x_offset = ld->right->x_offset + len;
    answer->right->y_offset = ld->right->y_offset;
    answer->right->upper_texture = ld->right->upper_texture;
    answer->right->lower_texture = ld->right->lower_texture;
    answer->right->middle_texture = ld->right->middle_texture;
    answer->right->isBoundary = ld->right->isBoundary;
  } else {
    answer->right = NULL;
  }
  if (ld->left) {
    answer->left = new_sidedef(l,ld->left->sector,c);
    answer->left->x_offset = ld->left->x_offset + len;   /* is that right? */
    answer->left->y_offset = ld->left->y_offset;
    answer->left->upper_texture = ld->left->upper_texture;
    answer->left->lower_texture = ld->left->lower_texture;
    answer->left->middle_texture = ld->left->middle_texture;
    answer->left->isBoundary = ld->left->isBoundary;
  } else {
    answer->left = NULL;
  }
  answer->marked = ld->marked;  /* I suppose */
  return answer;
}

/* Put in any upper textures required */
void patch_upper(linedef *ld,texture *t,config *c)
{
#ifdef TOLERATE_ERRORS
  if (ld->left==NULL) return;
#endif
  if (ld->right->sector->ceiling_height >
      ld->left->sector->ceiling_height) {
    if ((ld->right->upper_texture==NULL) ||
        (ld->right->upper_texture->name[0]=='-')) {
      ld->right->upper_texture = t;
      ld->flags |= UPPER_UNPEGGED;    /* Seems a good default */
    }
  }
  if (ld->left->sector->ceiling_height >
      ld->right->sector->ceiling_height) {
    if ((ld->left->upper_texture==NULL) ||
        (ld->left->upper_texture->name[0]=='-')) {
      ld->left->upper_texture = t;
      ld->flags |= UPPER_UNPEGGED;    /* Seems a good default */
    }
  }
}

/* Put in any lower textures required */
void patch_lower(linedef *ld,texture *t,config *c)
{
#ifdef TOLERATE_ERRORS
  if (ld->left==NULL) return;
#endif
  if (ld->right->sector->floor_height <
      ld->left->sector->floor_height) {
    if ((ld->right->lower_texture==NULL) ||
        (ld->right->lower_texture->name[0]=='-')) {
      ld->right->lower_texture = t;
      ld->flags |= LOWER_UNPEGGED;    /* Seems a good default */
    }
  }
  if (ld->left->sector->floor_height <
      ld->right->sector->floor_height) {
    if ((ld->left->lower_texture==NULL) ||
        (ld->left->lower_texture->name[0]=='-')) {
      ld->left->lower_texture = t;
      ld->flags |= LOWER_UNPEGGED;    /* Seems a good default */
    }
  }
}

/* Flip the linedef end-for-end */
linedef *flip_linedef(linedef *ld)
{
  sidedef *sd;
  vertex *v;

  v = ld->from;
  sd = ld->left;

  ld->from = ld->to;
  ld->left = ld->right;

  ld->to = v;
  ld->right = sd;

  return ld;
}

void Usage0(void)
{
  printf("Usage: slige [switches] [filename" EXT_SEP "ext] [switches]\n\n");
  printf("Produces a (nodeless) PWAD file that can be completed by a\n");
  printf("nodebuilder such as BSP, and then played using the -file\n");
  printf("function of DOOM (or DOOM2).  The default output file is\n");
  printf("SLIGE" EXT_SEP "OUT.  Gets all sorts of data and options and stuff from\n");
  printf("SLIGE" EXT_SEP "CFG (or other file given with the -config switch.)\n\n");
}

void Usage(void)
{
  Usage0();
  printf("For details, say \"slige -?\".\n\n");
  return;
}

/* Remove anything from the config that would be dangerous if left */
/* in, and optionally remove anything that just benignly won't ever */
/* be used and might take up valuable time and space. */
void compact_config(config *c)
{
  texture *t;

  /* NULLify any texture subtles that aren't in this DOOM version */
  for (t=c->texture_anchor;t;t=t->next)
    if (t->subtle)
      if ((t->subtle->gamemask&c->gamemask)!=c->gamemask) t->subtle = NULL;

  /* Now we *could* also remove from the config all textures and flats */
  /* and monsters and stuff that aren't in this DOOM, and then skip the */
  /* checks later on.  Someday... */

}

/* Alter this config to be good and strange for a secret */
/* level.  Add stuff to this over time! */
void secretize_config(config *c)
{
  boolean something_special = FALSE;

  c->minrooms = c->minrooms * 2 / 3;
  if (c->minrooms<8) c->minrooms = 8;
  if (c->minrooms>20) c->minrooms = 20;
  c->allow_boring_rooms = FALSE;
  c->lock_themes = TRUE;
  if (rollpercent(25)) c->force_biggest = TRUE;   /* stub */
  c->big_monsters = TRUE;

  for (;!something_special;) {

    /* Sometimes bizarre theme */
    if (rollpercent(30)) {
      c->secret_themes = TRUE;
      something_special = TRUE;
      announce(VERBOSE,"Bizarre theme");
    }

    /* Sometimes lots and lots of nukage */
    if (rollpercent(30)) {
      c->major_nukage = TRUE;
      something_special = TRUE;
      announce(VERBOSE,"Nukage everywhere");
    }

    /* Sometimes some DooM II nazis */
    if (rollpercent(80)&&!(c->gamemask&(DOOM0_BIT|DOOM1_BIT))) {
      c->forbidden_monster_bits &= ~SPECIAL;
      something_special = TRUE;
      if (rollpercent(50)) {
        c->required_monster_bits |= SPECIAL;
        c->required_monster_bits &= ~BIG;
        announce(VERBOSE,"All nazis");
      } else {
        announce(VERBOSE,"Some nazis");
      }
    }

    /* Sometimes some monster thing */
    if (rollpercent(30) && !something_special) {
      if (rollpercent(50)) {
        c->required_monster_bits |= BIG;
        c->required_monster_bits &= ~SPECIAL;
        c->big_monsters = TRUE;
        announce(VERBOSE,"All big monsters");
        something_special = TRUE;
      } else {
        /* Put in a favorite monster here! */
        /* and set something_special */
        announce(VERBOSE,"Someday a favorite monster");
      }
    }

  }  /* end for */

}

/* Get the configuration data, switches, etc.  Five steps: */
/* 1. Parse the arglist to find out where the config file is, */
/* 2. Read through the config file to get other switches,     */
/* 3. Parse the arglist to get overrides to switches,         */
/* 4. Read the config for non-switches (flats, themes, etc).  */
/* 5. Do postproduction defaults and calculations and such.   */
config *get_config(int argc, char *argv[])
{
  config *answer;
  int i;
  genus *m;

  answer = (config *)malloc(sizeof(*answer));

  /* Set various defaults and stuff */
  if ( (answer->configfile = getenv(SLIGE_CONFIG)) == NULL)
    answer->configfile = strdup("slige" EXT_SEP "cfg"); /* So's we kin free() it */
  else
    answer->configfile = strdup(answer->configfile);
  answer->outfile = NULL;
  answer->cwadonly = FALSE;
  srand((unsigned int)time(NULL));
  for (i=0;i<20;i++) rand();
  answer->ranseed = (unsigned int)bigrand();

  /* Do initial parsing for possible other config file and ranseed */
  if (!do_switches(argc,argv,answer,"Command line",1)) return NULL;

  ok_to_roll = TRUE;

  /* Set other defaults, possibly involving random numbers */
  answer->theme_anchor = NULL;
  answer->flat_anchor = NULL;
  answer->texture_anchor = NULL;
  answer->construct_anchor = NULL;
  answer->genus_anchor = NULL;
  answer->null_texture = NULL;
  answer->error_texture = NULL;
  answer->gate_exitsign_texture = NULL;
  answer->sky_flat = NULL;
  answer->themecount = 0;
  answer->sthemecount = 0;
  answer->secret_themes = FALSE;
  answer->lock_themes = FALSE;
  answer->major_nukage = FALSE;
  answer->required_monster_bits = 0;
  answer->forbidden_monster_bits = SPECIAL;
  answer->minrooms = 18;
  answer->gamemask = DOOM1_BIT;   /* All/Only things supported by DOOM 1.9 */
  answer->episode = 1;
  answer->mission = 1;
  answer->last_mission = FALSE;
  answer->levelcount = 1;
  answer->force_arena = FALSE;
  answer->force_biggest = FALSE;
  answer->do_music = 0;
  answer->secret_monsters = TRUE;
  answer->do_dm = 0;
  answer->do_slinfo = TRUE;
  answer->produce_null_lmps = FALSE;
  answer->do_seclevels = TRUE;
  answer->force_secret = FALSE;
  answer->fcontrol = FALSE;
  answer->fstart = FALSE;
  answer->ffstart = FALSE;
  answer->ffend = FALSE;
  answer->fend = FALSE;
  answer->map = 0;
  answer->minlight = 115;
  answer->minhuge = 1;
  /* Is this the right place for all these? */
  answer->immediate_monsters = rollpercent(10);
  answer->p_hole_ends_level = 0;
  if (rollpercent(8)) answer->p_hole_ends_level = 100;
  if (rollpercent(3)) answer->p_hole_ends_level = roll(100);
  /* These should depend on lastness of the level in the PWAD? */
  answer->p_gate_ends_level = 0;
  if (rollpercent(8)) answer->p_gate_ends_level = 100;
  if (rollpercent(3)) answer->p_gate_ends_level = roll(100);
  answer->p_use_steps = 100;
  if (rollpercent(5)) answer->p_use_steps = roll(100);
  answer->p_sync_doors = 10;
  if (rollpercent(50)) answer->p_sync_doors = 100;
  if (rollpercent(5)) answer->p_sync_doors = roll(100);
  answer->p_grid_gaps = 0;
  if (rollpercent(40)) answer->p_grid_gaps = 1;
  if (rollpercent(10)) answer->p_grid_gaps = roll(20);
  answer->p_pushquest = 10;
  if (rollpercent(40)) answer->p_pushquest = 50;
  if (rollpercent(10)) answer->p_pushquest = roll(90);
  answer->rad_newtheme = 100;
  answer->norm_newtheme = 0;
  answer->rad_vary = 100;
  answer->norm_vary = 25;
  if (rollpercent(15)) {  /* Some older settings */
    announce(VERBOSE,"Old themeing");
    answer->rad_newtheme = 12;
    answer->norm_newtheme = 4;
    answer->rad_vary = 60;
    answer->norm_vary = 20;
  }
  if (rollpercent(15)) {  /* Sometimes never change themes! */
    announce(VERBOSE,"One theme");
    answer->rad_newtheme = 0;
    answer->norm_newtheme = 0;
    answer->rad_vary = 100;        /* But change other stuff more */
    answer->norm_vary = 60;
  }
  answer->monsters_can_teleport = TRUE;
  if (rollpercent(25)) answer->monsters_can_teleport = FALSE;
  answer->window_airshafts = rollpercent(50);
  answer->homogenize_monsters = 0;
  if (rollpercent(8)) answer->homogenize_monsters = 90;
  if (rollpercent(15)) answer->homogenize_monsters = roll(100);
  {
  char s[80];
  sprintf(s,"Homogenization %d.",answer->homogenize_monsters);
  announce(VERBOSE,s);
  }
  answer->weapons_are_special = FALSE;  /* Just mix weapons in with ammo */
  answer->recess_switches = rollpercent(95);
  answer->allow_boring_rooms = rollpercent(20);
  answer->both_doors = rollpercent(50);
  answer->doorless_jambs = rollpercent(10);
  answer->gunk_channels = rollpercent(70);
  answer->clights = rollpercent(50);
  answer->machoh = (float)1;
  answer->machou = (float)1;
  answer->p_bigify = roll(100);   /* Less uniform? */

  /* Initial defaults; at each level, some chance of turning on */
  answer->big_weapons = rollpercent(50);
  if (answer->big_weapons) answer->big_monsters = rollpercent(80);
    else answer->big_monsters = rollpercent(35);

  /* Open or whatever the config file */
  load_config(answer);

  /* Read switch-defaults from the config file */
  if (!read_switches(answer)) return NULL;

  /* Now scan the command line again */
  if (!do_switches(argc,argv,answer,"Command line",0)) return NULL;

  /* And finally read in all the hard stuff from the file */
  if (!nonswitch_config(answer)) return NULL;

  /* Close/free the config stuff */
  unload_config(answer);

  /* Then we set some final defaulty stuff */
  if (answer->outfile==NULL)
    answer->outfile = strdup("slige" EXT_SEP "out");   /* So we can free it */
  if (answer->error_texture==NULL)   /* Use REDWALL if none specified */
    answer->error_texture = find_texture(answer,"REDWALL");  /* OK default? */
  if (answer->sky_flat==NULL)
    answer->sky_flat = find_flat(answer,"F_SKY1");  /* Default */
  if (answer->water_flat==NULL)
    answer->water_flat = find_flat(answer,"FWATER1");  /* Default */
  if (answer->null_texture==NULL)
    answer->null_texture = find_texture(answer,"-");  /* Always, really... */
  if (answer->fcontrol==FALSE) {  /* The normal DOOM engine requires... */
    answer->fstart = FALSE;
    answer->ffstart = TRUE;
    answer->ffend = TRUE;
    answer->fend = TRUE;
  }

  /* And figure some resultants */
  for (m=answer->genus_anchor;m;m=m->next) {   /* Apply macho factors */
    if (!(m->bits&MONSTER)) continue;
    m->ammo_to_kill[HMP] *= answer->machoh;
    m->damage[HMP] *= answer->machoh;
    m->altdamage[HMP] *= answer->machoh;
    m->ammo_to_kill[UV] *= answer->machou;
    m->damage[UV] *= answer->machou;
    m->altdamage[UV] *= answer->machou;
  }

  if (answer->force_secret) secretize_config(answer);

  /* And finally compact out any unneeded/dangerous stuff */
  compact_config(answer);

  return answer;

}

vertex *make_watermark_path(level *l,vertex *v1,vertex *v2,
                            sidedef *rsd, sidedef *lsd)
{
  linedef *ld = new_linedef(l,v1,v2);
  ld->flags = TWO_SIDED;
  ld->left = lsd;
  ld->right = rsd;
  return v2;
}

void watermark_sector(level *l,sector *s, style *ThisStyle, config *c)
{
   /* This isn't a stub except that find_rec is */
   /* Well, and it's not clear what you'd do with a */
   /* non-rectangular sector */
   int minx, miny, maxx, maxy;
   int x1, x2, x3, x4;
   int y1, y2, y3, y4, y5, y6;
   sector *newsector;
   sidedef *lsd, *rsd;
   vertex *v0, *v1;

   /* Make a new sector for the S-shape */
   newsector = new_sector(l,s->floor_height,(short)(s->ceiling_height+16),
                            s->floor_flat,c->sky_flat);
   newsector->light_level = l->outside_light_level;
   newsector->style = ThisStyle;

   /* Figure out the relevant grid-lines */
   find_rec(l,s,&minx,&miny,&maxx,&maxy);
   x1 = minx + 4;
   x4 = maxx - 4;
   y6 = miny + 4;
   y1 = maxy - 4;
   x2 = x1 + (x4-x1)/3;
   x3 = x2 + (x4-x1)/3;
   y5 = y6 + (y1-y6)/5;
   y4 = y5 + (y1-y6)/5;
   y3 = y4 + (y1-y6)/5;
   y2 = y3 + (y1-y6)/5;

   /* Make the two sidedefs that the linedefs will share */
   rsd = new_sidedef(l,newsector,c);
   rsd->middle_texture = c->null_texture;
   rsd->upper_texture = ThisStyle->wall0;
   rsd->isBoundary = FALSE;
   lsd = new_sidedef(l,s,c);
   lsd->middle_texture = c->null_texture;
   lsd->isBoundary = FALSE;

   /* Now make a whole buncha linedefs */

   v0 = new_vertex(l,x1,y1);
   v1 = make_watermark_path(l,v0,new_vertex(l,x4,y1),rsd,lsd);
   v1 = make_watermark_path(l,v1,new_vertex(l,x4,y2),rsd,lsd);
   v1 = make_watermark_path(l,v1,new_vertex(l,x2,y2),rsd,lsd);
   v1 = make_watermark_path(l,v1,new_vertex(l,x2,y3),rsd,lsd);
   v1 = make_watermark_path(l,v1,new_vertex(l,x4,y3),rsd,lsd);
   v1 = make_watermark_path(l,v1,new_vertex(l,x4,y6),rsd,lsd);
   v1 = make_watermark_path(l,v1,new_vertex(l,x1,y6),rsd,lsd);
   v1 = make_watermark_path(l,v1,new_vertex(l,x1,y5),rsd,lsd);
   v1 = make_watermark_path(l,v1,new_vertex(l,x3,y5),rsd,lsd);
   v1 = make_watermark_path(l,v1,new_vertex(l,x3,y4),rsd,lsd);
   v1 = make_watermark_path(l,v1,new_vertex(l,x1,y4),rsd,lsd);
   v1 = make_watermark_path(l,v1,v0,rsd,lsd);

}  /* end watermark_sector() */

/* Do segments AB and CD intersect?  Algorithm from the Net */
boolean intersects(int XA, int YA, int XB, int YB,
                   int XC, int YC, int XD, int YD)
{
  double r,s;
  int r_top, s_top, bottom;

  bottom = (XB-XA)*(YD-YC)-(YB-YA)*(XD-XC);
  r_top = (YA-YC)*(XD-XC)-(XA-XC)*(YD-YC);
  if (bottom==0) {   /* parallel */
    if (r_top!=0) {
      return 0;   /* proper parallel */
    } else {                  /* colinear; hard case */
      return 0;   /* This is wrong, of course!  But rarely... */
    }
  }
  s_top = (YA-YC)*(XB-XA)-(XA-XC)*(YB-YA);
  r = (double)r_top / (double)bottom;
  s = (double)s_top / (double)bottom;
  if (r<0) return 0;
  if (r>1) return 0;
  if (s<0) return 0;
  if (s>1) return 0;
  return 1;
}

/* Fix up any obvious HOMs with an obvious error texture. */
/* Note that you can still get HOMs with lifts and other */
/* floors and ceilings that move during play. */
void global_paint_HOMs(level *l, config *c)
{
  linedef *ld;
  for (ld=l->linedef_anchor;ld;ld=ld->next) {
    if (ld->right)
      if (ld->left) {
        patch_upper(ld,c->error_texture,c);
        patch_lower(ld,c->error_texture,c);
      }
  }
}

/* Return the (int number of) a random theme that exists in */
/* the given configuration.  Need anything fancier here?    */
int random_theme(config *c)
{
  int answer;

  if (c->secret_themes) {
    answer = c->themecount + roll(c->sthemecount);
  } else {
    answer = roll(c->themecount);
#ifdef THEME_INVERTER_HACK
    answer = (answer==2) ? answer : 1-answer;    /* Just an experiment */
#endif
  }
  {
  char s[80];
  sprintf(s,"Theme %d.",answer);
  announce(VERBOSE,s);
  }
  return answer;
}

/* Need anything fancier in these next few things? */

/* Linedef type for an ordinary inter-room non-secret door */
short random_doortype(level *l, config *c, style *s)
{
  short answer;

  answer = LINEDEF_NORMAL_DOOR;
  if (rollpercent(l->p_s1_door)) answer = LINEDEF_NORMAL_S1_DOOR;
  if (!(DOOM0_BIT&c->gamemask) && rollpercent(20)) {
    if (answer==LINEDEF_NORMAL_DOOR) answer=LINEDEF_BLAZE_DOOR;
    if (answer==LINEDEF_NORMAL_S1_DOOR) answer=LINEDEF_BLAZE_S1_DOOR;
    announce(VERBOSE,"Blaze door type");
  }
  return answer;
}

short random_slifttype(config *c, style *s)
{
  short answer;

  answer = LINEDEF_SR_LOWER_LIFT;
  if (!(DOOM0_BIT&c->gamemask) && rollpercent(20))
    answer = LINEDEF_SR_TURBO_LIFT;
  return answer;
}

int random_sillheight(config *c, style *s)
{
  if (s->window_grate) {
    if (rollpercent(50)) return 0;
      else return 4 * roll(13);
  } else return 28 + 4 * roll(6);
}

int random_windowheight(config *c, style *s)
{
  if (s->window_grate) return 64 + roll(16);     /* Does this make any sense? */
    else return 16 + 4 * roll(9);
}

int random_windowborder(config *c, style *s)
{
  return 4 + 4 * roll(6);
}

int random_windowdecor(config*c, style *s)
{
  switch (roll(5)) {
    case 0:
    case 1: return WINDOW_NORMAL;
    case 2: return WINDOW_JAMBS;
    case 3: return WINDOW_SUPPORT;
    default: return WINDOW_LIGHT;
  }
}

int random_lightboxlighting(config*c, style *s)
{
  switch (roll(4)) {
    case 0: return LIGHTBOX_NORMAL;
    case 1:
    case 2: return LIGHTBOX_LIGHTED;
    default: return LIGHTBOX_DARK;
  }
}

/* Various plants etc; should be from the config also of course. */
genus *random_plant(config *c, style *s)
{
  genus *answer;
  int tcount;

  tcount = (c->gamemask & DOOM1_BIT) ? 3 : 4;

  switch (roll(tcount)) {
    case 0: answer = find_genus(c,ID_SMIT);
      answer->bits &= ~PICKABLE;
      answer->width = 33; break;
    case 1: answer = find_genus(c,ID_TREE1);
      answer->bits &= ~PICKABLE;
      answer->width = 33; break;
    case 2: answer = find_genus(c,ID_TREE2);
      answer->bits &= ~PICKABLE;
      answer->width = 65; break;
    case 3:
    default: answer = find_genus(c,ID_FBARREL);
      answer->bits &= ~PICKABLE;
      answer->width = 33; break;
  }
  return answer;
}

/* Can return NULL if there are no explodables that are */
/* compatible with the theme. */
genus *random_barrel(config *c, style *s)
{
  return random_thing0(EXPLODES,c,s,0,10000);
}

/* A lamp or similar decoration, tall or short */
genus *random_lamp0(config *c, style *s)
{
  genus *answer;

  answer = random_thing0(LIGHT,c,s,70,10000);
  if (answer==NULL) answer = random_thing0(LIGHT,c,s,0,10000);
  return answer;
}

/* A lamp or similar decoration, no taller than a player */
genus *random_shortlamp0(config *c, style *s)
{
  return random_thing0(LIGHT,c,s,0,56);
}


/* Return the number of a random construct family that contains at */
/* least one construct compatible with this style's theme.  */
int construct_family_for(config *c, style *s)
{
  construct *cs;
  int tmask = 0x01 << s->theme_number;
#define MAX_COMPATIBLE_FAMILIES (5)
  int compats[MAX_COMPATIBLE_FAMILIES];
  int compat_count = 0;
  boolean already;
  int i;

  for (cs=c->construct_anchor;cs;cs=cs->next) {
    if (!(cs->compatible&tmask)) continue;
    if ( (cs->gamemask & c->gamemask) != c->gamemask ) continue;
    for (already=FALSE,i=0;i<compat_count;i++)
      if (compats[i]==cs->family) already = TRUE;
    if (already) continue;
    compats[compat_count++] = cs->family;
  }

  if (compat_count==0) {
#ifdef REQUIRE_CONSTRUCTS_IN_ALL_THEMES
    announce(WARNING,"No compatible constructs for theme.");
#endif
    return -1;   /* Whatever */
  }

  return(compats[roll(compat_count)]);
}

/* Make a new style that is in the given theme, and copies the */
/* given style.  Vary is a number from 0 to 100 saying how     */
/* noisy the copy should be; not linear!  If vary is 100, old  */
/* can be NULL or whatever.                                    */
style *copy_style(level *l,style *old,int themenumber,int vary,config *c)
{
  style *answer;

  /* Is this the correct sort of nonlinearity? */

  answer = (style *)malloc(sizeof(*answer));
  answer->next = l->style_anchor;
  l->style_anchor = answer;
  answer->theme_number = themenumber;
  if (!rollpercent(vary)) answer->floor0 = old->floor0;
    else answer->floor0 = random_floor0(c,answer);
  if (!rollpercent(vary)) answer->ceiling0 = old->ceiling0;
    else answer->ceiling0 = random_ceiling0(c,answer);
  if (!rollpercent(vary)) answer->ceilinglight = old->ceilinglight;
    else answer->ceilinglight = random_ceilinglight(c,answer);
  if (!rollpercent(vary)) answer->doorfloor = old->doorfloor;
    else answer->doorfloor = random_doorfloor(c,answer);
  if (!rollpercent(vary)) answer->stepfloor = old->stepfloor;
    else answer->stepfloor = random_stepfloor(c,answer);
  if (!rollpercent(vary)) answer->nukage1 = old->nukage1;
    else answer->nukage1 = random_nukage1(c,answer);
  if (!rollpercent(vary)) answer->doorceiling = old->doorceiling;
    else answer->doorceiling = random_doorceiling(c,answer);
  if (!rollpercent(vary)) {
    answer->wall0 = old->wall0;
    answer->switch0 = old->switch0;
  } else {
    answer->wall0 = random_wall0(c,answer);
    answer->switch0 = switch0_for(c,answer);
  }
  if (!rollpercent(vary)) answer->kickplate = old->kickplate;
    else answer->kickplate = random_kickplate(c,answer);
  if (!rollpercent(vary)) answer->stepfront = old->stepfront;
    else answer->stepfront = random_stepfront(c,answer);
  if (!rollpercent(vary)) answer->support0 = old->support0;
    else answer->support0 = random_support0(c,answer);
  if (!rollpercent(vary)) answer->doorjamb = old->doorjamb;
    else answer->doorjamb = random_doorjamb(c,answer);
  if (!rollpercent(vary)) answer->widedoorface = old->widedoorface;
    else answer->widedoorface = random_widedoorface(c,answer);
  if (!rollpercent(vary)) answer->narrowdoorface = old->narrowdoorface;
    else answer->narrowdoorface = random_narrowdoorface(c,answer);
  if (!rollpercent(vary)) answer->twdoorface = old->twdoorface;
    else answer->twdoorface = random_twdoorface(c,answer);
  if (!rollpercent(vary)) answer->tndoorface = old->tndoorface;
    else answer->tndoorface = random_tndoorface(c,answer);
  if (!rollpercent(vary)) answer->lockdoorface = old->lockdoorface;
    else answer->lockdoorface = random_lockdoorface(c,answer);
  if (!rollpercent(vary)) answer->walllight = old->walllight;
    else answer->walllight = random_walllight(c,answer);
  if (!rollpercent(vary)) answer->liftface = old->liftface;
    else answer->liftface = random_liftface(c,answer);
  if (!rollpercent(vary)) answer->plaque = old->plaque;
    else answer->plaque = random_plaque(c,answer);
  if (!rollpercent(vary)) answer->redface = old->redface;
    else answer->redface = random_redface(c,answer);
  if (!rollpercent(vary)) answer->blueface = old->blueface;
    else answer->blueface = random_blueface(c,answer);
  if (!rollpercent(vary)) answer->yellowface = old->yellowface;
    else answer->yellowface = random_yellowface(c,answer);
  if (!rollpercent(vary)) answer->lamp0 = old->lamp0;
    else answer->lamp0 = random_lamp0(c,answer);
  if (!rollpercent(vary)) answer->shortlamp0 = old->shortlamp0;
    else answer->shortlamp0 = random_shortlamp0(c,answer);
  if (!rollpercent(vary)) answer->grating = old->grating;
    else answer->grating = random_grating(c,answer);
  if (!rollpercent(vary)) answer->roomlight0 = old->roomlight0;
   else answer->roomlight0 = c->minlight +
     roll((l->bright_light_level-c->minlight)/2) +
     roll((l->bright_light_level-c->minlight)/2);
  answer->doorlight0 = answer->roomlight0 + 20 - roll(41);
  if (!rollpercent(vary)) {
    answer->wallheight0 = old->wallheight0;
  } else {
    if (rollpercent(20)) {           /* More variety in here */
      answer->wallheight0 = 256;
    } else if (rollpercent(50)) {
      answer->wallheight0 = 128;
    } else {
      answer->wallheight0 = 96;
    }
  }
  if (!rollpercent(vary)) {
    answer->linkheight0 = old->linkheight0;
  } else {
    if (rollpercent(20)) {            /* More variety in here */
      answer->linkheight0 = 128;
    } else if (rollpercent(50)) {
      answer->linkheight0 = 64;
    } else {
      answer->linkheight0 = 72;
    }
    answer->linkheight0 *= l->hugeness;
  }
  if (!rollpercent(vary)) answer->closet_width = old->closet_width;
    else {
      /* Old narrow method */
      answer->closet_width = 64 + roll(4) + roll(4) + roll(4) + roll(4);
      if (rollpercent(50))
        /* Something wider? */
        answer->closet_width = 64 + 16 * roll(5);
    }
  if (!rollpercent(vary)) answer->closet_depth = old->closet_depth;
    else {
      answer->closet_depth = 64 + roll(4) + roll(4) + roll(4) + roll(4);
      if (rollpercent(40)) answer->closet_depth *= 2;
    }
  if (!rollpercent(vary)) answer->closet_light_delta = old->closet_light_delta;
    else answer->closet_light_delta = roll(55) - 35;
  if (!rollpercent(vary)) answer->moving_jambs = old->moving_jambs;
    else answer->moving_jambs = rollpercent(10);
  if (!rollpercent(vary)) answer->secret_doors = old->secret_doors;
    else answer->secret_doors = rollpercent(5);
  /* These have to co-vary, because grating determines heights */
  if (!rollpercent(vary)) {
    answer->window_grate = old->window_grate;
    answer->sillheight = old->sillheight;
    answer->windowheight = old->windowheight;
  } else {
    answer->window_grate = rollpercent(30);
    answer->sillheight = random_sillheight(c,answer);
    answer->windowheight = random_windowheight(c,answer);
  }
  if (!rollpercent(vary)) answer->light_recesses = old->light_recesses;
    else answer->light_recesses = rollpercent(30);
  if (!rollpercent(vary)) answer->do_constructs = old->do_constructs;
    else answer->do_constructs = rollpercent(80);
  if (!rollpercent(vary)) answer->light_steps = old->light_steps;
    else answer->light_steps = rollpercent(10);   /* Too low? */
  if (!rollpercent(vary)) answer->light_edges = old->light_edges;
    else answer->light_edges = rollpercent(20);
  if (!rollpercent(vary)) answer->peg_lightstrips = old->peg_lightstrips;
    else answer->peg_lightstrips = rollpercent(50);
  if (!rollpercent(vary)) answer->construct_family = old->construct_family;
    else answer->construct_family = construct_family_for(c,answer);
  if (!rollpercent(vary)) answer->window_decor = old->window_decor;
    else answer->window_decor = random_windowdecor(c,answer);
  if (!rollpercent(vary)) answer->lightbox_lighting = old->lightbox_lighting;
    else answer->lightbox_lighting = random_lightboxlighting(c,answer);
  if (!rollpercent(vary)) answer->slitwindows = old->slitwindows;
    else answer->slitwindows = rollpercent(20);
  if (!rollpercent(vary)) answer->windowborder = old->windowborder;
    else answer->windowborder = random_windowborder(c,answer);
  if (!rollpercent(vary)) answer->soundproof_doors = old->soundproof_doors;
    else answer->soundproof_doors = rollpercent(30);
  if (!rollpercent(vary)) answer->center_pillars = old->center_pillars;
    else answer->center_pillars = rollpercent(70);
  if (!rollpercent(vary)) answer->paint_recesses = old->paint_recesses;
    else answer->paint_recesses = rollpercent(60);
  if (!rollpercent(vary)) answer->gaudy_locks = old->gaudy_locks;
    else answer->gaudy_locks = rollpercent(10);
  answer->lightboxes = FALSE;  /* Ephemeral; usually FALSE */
  if (!rollpercent(vary)) answer->auxheight = old->auxheight;
    else answer->auxheight = roll(2) * (8 + 8 * roll(8));
  if (!rollpercent(vary)) answer->auxspecial = old->auxspecial;
    else answer->auxspecial = rollpercent(80) ? 0 : RANDOM_BLINK;
  if (!rollpercent(vary)) answer->doortype = old->doortype;
    else answer->doortype = random_doortype(l,c,answer);
  if (!rollpercent(vary)) answer->slifttype = old->slifttype;
    else answer->slifttype = random_slifttype(c,answer);
  if (!rollpercent(vary)) answer->link0 = old->link0;
    else answer->link0 = random_link(l,NULL,answer,NULL,c);

  return answer;
}

/* Return a new style derived from the given one, based on the config */
/* If "radical", choose a whole new theme.  Else don't.               */
/* should be.  It's not linear!                                      */
style *new_style(level *l,style *old, boolean radical, config *c)
{
  int newtheme = radical ? c->rad_newtheme : c->norm_newtheme;
  int vary = radical ? c->rad_vary : c->norm_vary;

  if ((!c->lock_themes)&&rollpercent(newtheme)) {  /* Sometimes entirely new */
    return copy_style(l,old,random_theme(c),100,c);
  } else if (rollpercent(vary)) {  /* Sometimes new, same theme */
    return copy_style(l,old,old->theme_number,100,c);
  } else {   /* else partly new, same theme */
    return copy_style(l,old,old->theme_number,vary,c);
  }
}

/* Return a random style structure according to the configuration */
style *random_style(level *l,config *c)
{
  return copy_style(l,NULL,random_theme(c),100,c);
}

/* Shockingly special-purpose routine that puts some stuff into */
/* a room that contains a gate in the midtile. */
void gate_populate(level *l,sector *s,haa *haa, boolean first, config *c)
{
  int minx,miny,maxx,maxy;
  short tlx, tly, thx, thy;
  genus *m;
  int levels;

  if (first) return;   /* punt! */

  find_rec(l,s,&minx,&miny,&maxx,&maxy);
  mid_tile(l,s,&tlx,&tly,&thx,&thy);

  if (tlx-minx>63) {   /* "63"s are all wrong */
    if (rollpercent(50)) {  /* A monster */
      m = timely_monster(haa,c,&levels,rollpercent(l->p_biggest_monsters),1);
      if (levels)
        if (NULL!=place_object_in_region(l,minx,miny,tlx,maxy,
                   c,m->thingid,MONSTER_WIDTH(m),-1,s->entry_x,s->entry_y,
                   levels)) update_haa_for_monster(haa,m,levels,1,c);
    } else {
      place_timely_something(l,haa,c,(minx+tlx)/2,(miny+maxy)/2);
    }
  }
  if (maxx-thx>63) {   /* "63"s are all wrong */
    if (rollpercent(50)) {  /* A monster */
      m = timely_monster(haa,c,&levels,rollpercent(l->p_biggest_monsters),1);
      if (levels)
        if (NULL!=place_object_in_region(l,thx,miny,maxx,maxy,
                   c,m->thingid,MONSTER_WIDTH(m),-1,s->entry_x,s->entry_y,
                   levels)) update_haa_for_monster(haa,m,levels,1,c);
    } else {
      place_timely_something(l,haa,c,(thx+maxx)/2,(miny+maxy)/2);
    }
  }

  /* Absurd duplication! */
  if (tly-miny>63) {   /* "63"s are all wrong */
    if (rollpercent(50)) {  /* A monster */
      m = timely_monster(haa,c,&levels,rollpercent(l->p_biggest_monsters),1);
      if (levels)
        if (NULL!=place_object_in_region(l,minx,miny,maxx,tly,
                   c,m->thingid,MONSTER_WIDTH(m),-1,s->entry_x,s->entry_y,
                   levels)) update_haa_for_monster(haa,m,levels,1,c);
    } else {
      place_timely_something(l,haa,c,(minx+maxx)/2,(miny+tly)/2);
    }
  }
  if (maxy-thy>63) {   /* "63"s are all wrong */
    if (rollpercent(50)) {  /* A monster */
      m = timely_monster(haa,c,&levels,rollpercent(l->p_biggest_monsters),1);
      if (levels)
        if (NULL!=place_object_in_region(l,minx,thy,maxx,maxy,
                   c,m->thingid,MONSTER_WIDTH(m),-1,s->entry_x,s->entry_y,
                   levels)) update_haa_for_monster(haa,m,levels,1,c);
    } else {
      place_timely_something(l,haa,c,(minx+maxx)/2,(thy+maxy)/2);
    }
  }

  /* And finally do weapon pickups */
  haa_unpend(haa);
}

/* Put monsters and health and armor and stuff in the room */
/* Update the health/ammo/armor estimate structure also    */
void populate(level *l,sector *s,config *c,haa *haa,boolean first_room)
{
  if ((!first_room)||(c->immediate_monsters))
    place_monsters(l,s,c,haa);
  place_health(l,s,c,haa);
  place_ammo(l,s,c,haa);
  place_armor(l,s,c,haa);
  place_barrels(l,s,c,haa);
  return;
}

/* Taking all the given stuff into account, have we put */
/* enough rooms into the current quest yet?             */
/* This routine can also mess with the current quest,   */
/* to do special end-stuff, like arenas.                */
boolean enough_quest(level *l,sector *s,quest *ThisQuest,config *c)
{
#ifndef NOT_DOING_ARENAS
  /* Perhaps an arena? */
  if ( (ThisQuest->goal==LEVEL_END_GOAL) &&
       (s!=l->first_room) &&
       (!s->gate) &&        /* 'cause we're lazy */
       (!c->do_dm) &&
       ( (l->sl_tag!=0) || !need_secret_level(c) ) &&
       ( (l->sl_tag==0) || l->sl_done ) &&
       (ThisQuest->count >= (ThisQuest->minrooms - 5)) ) {
    if ( (c->mission==8) ||
         (c->map==30) ||
         ((c->map==7)&&(c->last_mission)) ||
         (c->last_mission&&(c->force_arena||rollpercent(3*c->levelcount))) ) {
      ThisQuest->goal = ARENA_GOAL;
      return 1;
    }
  }
#endif
  /* Don't stop a GATE_QUEST at an already-gate room */
  if (ThisQuest->goal==GATE_GOAL)
    if (s->gate) return 0;
  /* Otherwise the ordinary check. */
  if (ThisQuest->count >= ThisQuest->minrooms) return 1;
  return 0;
}

/* Process the switches in the given arg array, filling in the  */
/* given config structure.  Use s in error messages.  If conly, */
/* all we're parsing for here are -config and -seed.            */
/* Print msg and return FALSE if error, else return TRUE.       */
boolean do_switches(int argc,char *argv[],config *c,char *s,int conly)
{
  int i;

  if (conly) {  /* config, seed, -v only; imperfect algorithm! Can be fooled. */
    for (i=1;i<argc;i++) {
      if (!stricmp(argv[i],"-config")) {
        if (i<(argc-1)) {   /* If -config is the last arg, just ignore it */
          c->configfile = strdup(argv[++i]);
        }  /* end if enough args */
      } else if (!stricmp(argv[i],"-seed")) {
        c->ranseed = atoi(argv[++i]);
      } else if (!stricmp(argv[i],"-v")) {
        global_verbosity = 1;
      }  /* end if -config arg */
    }  /* end for args */
    /* Now we have the seed, from timer or cmdline, so use it */
    srand(c->ranseed);
    {
    char logstring[60];
    sprintf(logstring,"Seed: %d",c->ranseed);
    announce(LOG,logstring);
    }
  } else {  /* not conly */
    for (i=1;i<argc;i++) {
      if (argv[i][0]!='-') {
        c->outfile = strdup(argv[i]);   /* Just take last if multiple */
      } else if (!stricmp(argv[i],"-?")) {
        Usage2();
        exit(100);
      } else if (!stricmp(argv[i],"-outfile")) {
        if (i<(argc-1)) {   /* If this is the last arg, just ignore it */
          c->outfile = strdup(argv[++i]);
        }  /* end not last arg */
      } else if (!stricmp(argv[i],"-doom")) {
        c->gamemask = DOOM1_BIT;
        if (c->episode==0) c->episode = c->mission = 1;
        c->map = 0;
      } else if (!stricmp(argv[i],"-doom2")) {
        c->gamemask = DOOM2_BIT;
        c->episode = c->mission = 0;
        if (c->map==0) c->map = 1;
      } else if (!stricmp(argv[i],"-nogross")) {
        c->gamemask |= DOOMC_BIT;
      } else if (!stricmp(argv[i],"-v")) {
        global_verbosity = 1;
      } else if (!stricmp(argv[i],"-cwad")) {
        c->cwadonly = TRUE;
      } else if (!stricmp(argv[i],"-nocustom")) {
        c->gamemask |= DOOMI_BIT;
      } else if (!stricmp(argv[i],"-nulls")) {
        c->produce_null_lmps = TRUE;
      } else if (!stricmp(argv[i],"-noslinfo")) {
        c->do_slinfo = FALSE;
      } else if (!stricmp(argv[i],"-noseclevels")) {
        c->do_seclevels = FALSE;
      } else if (!stricmp(argv[i],"-bimo!")) {
        c->force_biggest = TRUE;
      } else if (!stricmp(argv[i],"-bimo")) {
        c->big_monsters = TRUE;
      } else if (!stricmp(argv[i],"-biwe")) {
        c->big_weapons = TRUE;
      } else if (!stricmp(argv[i],"-huge")) {
        c->minhuge = 2;
      } else if (!stricmp(argv[i],"-xsecret")) {
        c->force_secret = TRUE;
      } else if (!stricmp(argv[i],"-fstart")) {
        c->fcontrol = TRUE;
        c->fstart = TRUE;
      } else if (!stricmp(argv[i],"-ffstart")) {
        c->fcontrol = TRUE;
        c->ffstart = TRUE;
      } else if (!stricmp(argv[i],"-fend")) {
        c->fcontrol = TRUE;
        c->fend = TRUE;
      } else if (!stricmp(argv[i],"-ffend")) {
        c->fcontrol = TRUE;
        c->ffend = TRUE;
      } else if (!stricmp(argv[i],"-gross")) {
        c->gamemask &= ~DOOMC_BIT;
      } else if (!stricmp(argv[i],"-music")) {
        c->do_music = TRUE;
      } else if (!stricmp(argv[i],"-nosemo")) {
        c->secret_monsters = FALSE;
      } else if (!stricmp(argv[i],"-dm")) {
        c->do_dm = TRUE;
      } else if (!stricmp(argv[i],"-arena")) {
        c->force_arena = TRUE;
      } else if (!stricmp(argv[i],"-levels")) {
        if (i<(argc-1)) {   /* If this is the last arg, just ignore it */
          c->levelcount = atoi(argv[++i]);
          if (c->levelcount==0) {
            fprintf(stderr,"%s error: invalid -levels arg <%s>.\n",
                                s,argv[i]);
            return FALSE;
          }
        }  /* end if enough args */
      } else if (!stricmp(argv[i],"-minlight")) {
        if (i<(argc-1)) {   /* If this is the last arg, just ignore it */
          c->minlight = atoi(argv[++i]);
          if (c->minlight==0) {
            fprintf(stderr,"%s error: invalid -minlight arg <%s>.\n",
                                s,argv[i]);
            return FALSE;
          }
        }  /* end if enough args */
      } else if (!stricmp(argv[i],"-macho")) {
        int mfac;
        if (i<(argc-1)) {   /* If this is the last arg, just ignore it */
          mfac = atoi(argv[++i]);
          if ((mfac<1) || (mfac>100)) {
            fprintf(stderr,"%s error: -macho must be in [1,100], not <%s>.\n",
                                s,argv[i]);
            return FALSE;
          }
          c->machoh = ((float)100 - (float)mfac/(float)4) / (float)100;
          c->machou = ((float)100 - (float)mfac/(float)2) / (float)100;
        }  /* end if enough args */
      } else if (!stricmp(argv[i],"-restrict")) {
        if (i<(argc-1)) {   /* If this is the last arg, just ignore it */
          int j;
          i++;
          c->gamemask = 0;
          for (j=0;j<(int)strlen(argv[i]);j++) {
            if (argv[i][j]=='C') c->gamemask |= DOOMC_BIT;
              else if (argv[i][j]=='I') c->gamemask |= DOOMI_BIT;
              else if (argv[i][j]=='0') c->gamemask |= DOOM0_BIT | DOOM1_BIT;
              else if (argv[i][j]=='1') c->gamemask |= DOOM1_BIT;
              else if (argv[i][j]=='2') c->gamemask |= DOOM2_BIT;
              else {
                fprintf(stderr,"%s error: invalid -restrict arg <%s>.\n",
                                    s,argv[i]);
                return FALSE;
              }
          }  /* end for bits of next arg */
        }  /* end if enough args */
      } else if ((strlen(argv[i])==5) &&
                 ( (argv[i][1]=='E') || (argv[i][1]=='e') ) &&
                 ( (argv[i][3]=='M') || (argv[i][3]=='m') ) ) {
        if (2!=sscanf(argv[i],"-%*c%d%*c%d",&(c->episode),&(c->mission))) {
          fprintf(stderr,"%s error: Invalid -ExMx switch <%s>.\n",s,argv[i]);
          return FALSE;
        }
        c->map = 0;
      } else if (!strnicmp(argv[i],"-map",4)) {
        c->map = atoi(argv[i]+4);
        if (c->map==0) {
          fprintf(stderr,"%s error: Invalid -MAPxx switch <%s>.\n",s,argv[i]);
          return FALSE;
        }
        c->episode = c->mission = 0;
      } else if (!stricmp(argv[i],"-rooms")) {
        if (i<(argc-1)) {   /* If this is the last arg, just ignore it */
          c->minrooms = atoi(argv[++i]);
        }  /* end if enough args */
      } else if (!stricmp(argv[i],"-config")) {
        if (i<(argc-1)) i++;   /* Ignored in this stage */
      } else if (!stricmp(argv[i],"-seed")) {
        if (i<(argc-1)) i++;   /* Ignored in this stage */
      } else {  /* Unknown switch */
        fprintf(stderr,"%s error: unknown switch <%s>.\n\n",s,argv[i]);
        return FALSE;
      }
    }  /* end for args */
  }  /* end not conly */

  return TRUE;
}

/* Put this object in this sector.  It's a non-obstable object */
thing *place_required_pickable(level *l,sector *s,config *c,short id)
{
  thing *answer;

  answer = place_object(l,s,c,id,48,0,0,0,7);   /* 48 for nice-looking-ness */
  if (answer==NULL)
    answer = place_object(l,s,c,id,1,0,0,0,7); /* This had better work! */
  if (answer==NULL) announce(ERROR,"Important object could not be placed.");
  return answer;
}

thing *place_required_small_pickable(level *l,sector *s,config *c)
{
  short tid;

  if (rollpercent(50)) tid = ID_POTION;
    else tid = ID_HELMET;                 /* More choices? */

  return place_required_pickable(l,s,c,tid);
}

/* This is for sector-specific texture alignment.  Is there */
/* some reason to want to do that?                          */
void align_textures(level *l,sector *oldsector,config *c)
{
  return;
}

/* Do these two sidedefs share any texture(s) that should */
/* be aligned together?                                   */
boolean common_texture(sidedef *sd1, sidedef *sd2)
{
  /* Pass along most of the job to coalignable(). */
  if (sd1->middle_texture->name[0]!='-') {
    if (coalignable(sd1->middle_texture,sd2->middle_texture)) return 1;
    if (coalignable(sd1->middle_texture,sd2->upper_texture)) return 1;
    if (coalignable(sd1->middle_texture,sd2->lower_texture)) return 1;
  }
  if (sd1->upper_texture->name[0]!='-') {
    if (coalignable(sd1->upper_texture,sd2->middle_texture)) return 1;
    if (coalignable(sd1->upper_texture,sd2->upper_texture)) return 1;
    if (coalignable(sd1->upper_texture,sd2->lower_texture)) return 1;
  }
  if (sd1->lower_texture->name[0]!='-') {
    if (coalignable(sd1->lower_texture,sd2->middle_texture)) return 1;
    if (coalignable(sd1->lower_texture,sd2->upper_texture)) return 1;
    if (coalignable(sd1->lower_texture,sd2->lower_texture)) return 1;
  }
  return 0;
}

void global_align_forward(level *l, linedef *ld)
{
  vertex *v;
  linedef *ld2;
  int newoff;

  v = ld->to;
  for (ld2=l->linedef_anchor;ld2;ld2=ld2->next) {
    if (ld2->from==v)
      if (common_texture(ld->right,ld2->right)) {
        newoff = ld->right->x_offset + linelen(ld);
        newoff = newoff % 256;
        if (newoff<0) newoff += 256;
        if (ld2->marked==0) {
          ld2->right->x_offset = newoff;
          ld2->marked = 1;
          global_align_linedef(l,ld2);
        } else {
          if (ld2->right->x_offset!=newoff)
            ld->f_misaligned = 1;
        }
      }  /* end if common texture */
  }  /* end for ld2 */
}

void global_align_backward(level *l, linedef *ld)
{
  vertex *v;
  linedef *ld2;
  int newoff;

  v = ld->from;
  for (ld2=l->linedef_anchor;ld2;ld2=ld2->next) {
    if (ld2->to==v)
      if (common_texture(ld->right,ld2->right)) {
        newoff = ld->right->x_offset - linelen(ld2);
        newoff = newoff % 256;
        if (newoff<0) newoff += 256;
        if (ld2->marked==0) {
          ld2->right->x_offset = newoff;
          ld2->marked = 1;
          global_align_linedef(l,ld2);
        } else {
          if (ld2->right->x_offset!=newoff)
            ld->b_misaligned = 1;
        }
      }  /* end if common texture */
  }  /* end for ld2 */
}

void global_align_linedef(level *l, linedef *ld)
{
  ld->marked = 1;
  global_align_group_backbone_forward(l,ld);
  global_align_group_backbone_backward(l,ld);
  global_align_group_etc_forward(l,ld);
  global_align_group_etc_backward(l,ld);
}

void global_align_group_backbone_forward(level *l,linedef *ld)
{
  linedef *ldnext;
  int newoff;

  if (NULL != (ldnext = ld->group_next)) {
    if (ld->to != ldnext->from) announce(LOG,"Yow forward!");
    if (common_texture(ld->right,ldnext->right)) {
      newoff = ld->right->x_offset + linelen(ld);
      newoff = newoff % 256;
      if (newoff<0) newoff += 256;
      if (ldnext->marked==0) {
        ldnext->right->x_offset = newoff;
        ldnext->marked = 1;
        global_align_group_backbone_forward(l,ldnext);
      } else {
        announce(LOG,"Found a locked linedef in g_a_g_b_f?");
        if (ldnext->right->x_offset!=newoff)
          ldnext->f_misaligned = 1;
      }
    }  /* end if common texture */
  }
}

void global_align_group_etc_forward(level *l,linedef *ld)
{
  linedef *ldnext;

  if (NULL != (ldnext = ld->group_next))
    global_align_group_etc_forward(l,ldnext);
  global_align_forward(l,ld);
}

void global_align_group_etc_backward(level *l,linedef *ld)
{
  linedef *ldnext;

  if (NULL != (ldnext = ld->group_previous))
    global_align_group_etc_backward(l,ldnext);
  global_align_backward(l,ld);
}

void global_align_group_backbone_backward(level *l,linedef *ld)
{
  linedef *ldprev;
  int newoff;

  if (NULL != (ldprev = ld->group_previous)) {
    if (ld->from != ldprev->to) announce(LOG,"Yow backward!");
    if (common_texture(ld->right,ldprev->right)) {
      newoff = ld->right->x_offset - linelen(ldprev);
      newoff = newoff % 256;
      if (newoff<0) newoff += 256;
      if (ldprev->marked==0) {
        ldprev->right->x_offset = newoff;
        ldprev->marked = 1;
        global_align_group_backbone_backward(l,ldprev);
      } else {
        announce(LOG,"Found a locked linedef in g_a_g_b_b?");
        if (ldprev->right->x_offset!=newoff)
          ldprev->b_misaligned = 1;
      }
    }  /* end if common texture */
  }
}

/* Align textures all around the level */
void global_align_textures(level *l,config *c)
{
  /* This complicated knot of recursives seem pretty good. */
  /* It doesn't know about NOSPLITs and stuff yet, and it */
  /* only does horizontal alignment.  Should we do Y here also? */
  linedef *ld1, *ld2;
  int newoff;

  announce(LOG,"Globally aligning...");

#define DONT_MARK_MISALIGNS

  for (ld1=l->linedef_anchor;ld1;ld1=ld1->next) {
    ld1->f_misaligned = 0;
    ld1->b_misaligned = 0;
  }

  unmark_linedefs(l);  /* just in case */
  for (ld1=l->linedef_anchor;ld1;ld1=ld1->next)
    if (ld1->marked==0) global_align_linedef(l,ld1);

  /* Now put in any intentional misalignments, for hints etc */
  for (ld1=l->linedef_anchor;ld1;ld1=ld1->next) {
    if (ld1->right) {
      ld1->right->x_offset += ld1->right->x_misalign;
      ld1->right->y_offset += ld1->right->y_misalign;
    }
  }  /* end for ld1 */

/* Sometimes put in supports in the places we know didn't align */
/* This actually looks pretty terrible!  Think about it more */
  if (l->support_misaligns) {
    announce(LOG,"Prettying up misalignments...");
    for (ld1=l->linedef_anchor;ld1;ld1=ld1->next) {
      if (ld1->right)
        if (ld1->right->sector->style)
          if (ld1->b_misaligned) {
            newoff = linelen(ld1);
            if (newoff>8) split_linedef(l,ld1,8,c);
            if (ld1->right->upper_texture->name[0]!='-')
              ld1->right->upper_texture = ld1->right->sector->style->support0;
            if (ld1->right->lower_texture->name[0]!='-')
              ld1->right->lower_texture = ld1->right->sector->style->support0;
            if (ld1->right->middle_texture->name[0]!='-')
              ld1->right->middle_texture = ld1->right->sector->style->support0;
           }
          if (ld1->f_misaligned) {
            newoff = linelen(ld1);
            if (newoff>8) ld2 = split_linedef(l,ld1,newoff-8,c);
              else ld2 = ld1;
            if (ld2->right->upper_texture->name[0]!='-')
              ld2->right->upper_texture = ld1->right->sector->style->support0;
            if (ld2->right->lower_texture->name[0]!='-')
              ld2->right->lower_texture = ld1->right->sector->style->support0;
            if (ld2->right->middle_texture->name[0]!='-')
              ld2->right->middle_texture = ld1->right->sector->style->support0;
           }
    }  /* end for ld1 */
  }  /* end if supporting */

#ifdef MARK_MISALIGNS
/* And finally for debugging mark the places we know still didn't align */
  for (ld1=l->linedef_anchor;ld1;ld1=ld1->next) {
    if (ld1->right)
      if (ld1->b_misaligned) {
        newoff = linelen(ld1);
        if (newoff>8) split_linedef(l,ld1,8,c);
        if (ld1->right->upper_texture->name[0]!='-')
          ld1->right->upper_texture = c->error_texture;
        if (ld1->right->lower_texture->name[0]!='-')
          ld1->right->lower_texture = c->error_texture;
        if (ld1->right->middle_texture->name[0]!='-')
          ld1->right->middle_texture = c->error_texture;
       }
      if (ld1->f_misaligned) {
        newoff = linelen(ld1);
        if (newoff>8) ld2 = split_linedef(l,ld1,newoff-8,c);
          else ld2 = ld1;
        if (ld2->right->upper_texture->name[0]!='-')
          ld2->right->upper_texture = c->error_texture;  /* or blue! */
        if (ld2->right->lower_texture->name[0]!='-')
          ld2->right->lower_texture = c->error_texture;
        if (ld2->right->middle_texture->name[0]!='-')
          ld2->right->middle_texture = c->error_texture;
       }
  }  /* end for ld1 */
#endif

  return;
}

/* Random other last-minute fixups to a level */
void global_fixups(level *l)
{
  linedef *ld;

  for (ld=l->linedef_anchor;ld;ld=ld->next)
    if (ld->left==NULL) ld->flags |= IMPASSIBLE;
}

/* This just paints all one-sided boundary sidddefs of the sector */
void paint_room(level *l,sector *s,style *ThisStyle,config *c)
{
  linedef *ld;

  for (ld=l->linedef_anchor;ld;ld=ld->next) {
    if (ld->right)
      if (ld->right->sector==s)
        if (ld->right->isBoundary) {
          if (ld->left==NULL) {
            ld->right->middle_texture = ThisStyle->wall0;
          } else {
            patch_upper(ld,ThisStyle->wall0,c);
            patch_lower(ld,ThisStyle->kickplate,c);  /* Or stepfront? */
          }
        }
  }
  s->light_level = ThisStyle->roomlight0;

  return;
}

/* Construct a linedef on the left side of this linedef, */
/* <depth> away from it and pro-parallel to it.          */
/* If old is not NULL, re-use it, just changing its to and from. */
linedef *make_parallel(level *l,linedef *ld,int depth,linedef *old)
{
  vertex *v1, *v2;
  int x,y;

  point_from(ld->from->x, ld->from->y, ld->to->x, ld->to->y,
             LEFT_TURN,depth,&x,&y);
  if (old) {
    old->to->x = x;     /* Assumes no one else is using */
    old->to->y = y;     /* these vertexes.  OK?         */
    x += ld->from->x - ld->to->x;
    y += ld->from->y - ld->to->y;
    old->from->x = x;
    old->from->y = y;
    return old;
  } else {
    v1 = new_vertex(l,x,y);
    x += ld->from->x - ld->to->x;
    y += ld->from->y - ld->to->y;
    v2 = new_vertex(l,x,y);
    return new_linedef(l,v2,v1);
  }
}

/* Given two linedefs, construct a new rhomboid between them, on */
/* the left side of the first and the right side of the second.  */
/* Return the sector.  If the edge1 and edge2 args are not null, */
/* they get the from-joining new linedef, and the to-joining one */
/* respectively.                                                 */
sector *make_box_ext(level *l,linedef *ldf1, linedef *ldf2,
                        style *ThisStyle, config *c,
                        linedef **edge1, linedef **edge2)
{
  sector *answer;
  linedef *ldnew1, *ldnew2;
  sector *oldsec;

  /* Make the orthogonal sides */
  ldnew1 = new_linedef(l,ldf1->from,ldf2->from);
  ldnew2 = new_linedef(l,ldf2->to,ldf1->to);
  if (edge1) *edge1 = ldnew1;
  if (edge2) *edge2 = ldnew2;
  /* Now tie them all to a sector */
  answer = new_sector(l,0,0,c->sky_flat,c->sky_flat);
  answer->style = ThisStyle;
  if (ldf1->right) {
    oldsec = ldf1->right->sector;
    answer->floor_height = oldsec->floor_height;
    answer->ceiling_height = oldsec->ceiling_height;
    answer->floor_flat = oldsec->floor_flat;
    answer->ceiling_flat = oldsec->ceiling_flat;
    answer->light_level = oldsec->light_level;  /* default */
    answer->special = oldsec->special;      /* or zero? */
    ldf1->right->middle_texture = c->null_texture;
    ldf1->flags |= TWO_SIDED;
  }
  ldf1->left = new_sidedef(l,answer,c);
  ldf2->right = new_sidedef(l,answer,c);
  ldnew1->right = new_sidedef(l,answer,c);
  ldnew2->right = new_sidedef(l,answer,c);
  ldf2->right->middle_texture = c->null_texture;   /* Useful? */
  if (ldf2->left)
    ldf2->left->middle_texture = c->null_texture;  /* HOM until filled */
  ldf1->left->middle_texture = c->null_texture;
  ldnew1->right->middle_texture = ThisStyle->wall0;    /* guess  */
  ldnew2->right->middle_texture = ThisStyle->wall0;    /*  also  */
  return answer;
}

/* Given a one-sided linedef, construct a rectangular sector on */
/* the left side of it, of the given depth.  Returns the other  */
/* (parallel) long side.                                        */
linedef *lefthand_box_ext(level *l,linedef *ldf1,int depth,
                              style *ThisStyle,config *c,
                              linedef **nld1, linedef **nld2)
{
  linedef *answer;
  sector *s;

  /* Get the other side of the box */
  answer = make_parallel(l,ldf1,depth,NULL);
  s = make_box_ext(l,ldf1,answer,ThisStyle,c,nld1,nld2);
  return answer;
}

/* Find the corners of the minimal enclosing rectangle around the */
/* given sector.  Or something like that.  Uses a cache in the    */
/* sector record for speed.  How dangerous is that? */
void find_rec(level *l, sector *s, int *minx, int *miny, int *maxx, int *maxy)
{
  /* Not a stub, but many of its callers are */
  linedef *ld;

  if (!s->findrec_data_valid) {
    int lx,ly,hx,hy;
    lx = ly =  HUGE_NUMBER;
    hx = hy = 0-HUGE_NUMBER;
    for (ld=l->linedef_anchor;ld;ld=ld->next) {
      if (ld->right)
        if (ld->right->sector==s) {
//        if (ld->right->isBoundary) {         /* Need to check this? */
            if (ld->to->x>hx) hx = ld->to->x;
            if (ld->to->y>hy) hy = ld->to->y;
            if (ld->to->x<lx) lx = ld->to->x;
            if (ld->to->y<ly) ly = ld->to->y;
//       }
        }
    }
    s->minx = lx;  s->miny = ly;  s->maxx = hx;  s->maxy = hy;
    s->findrec_data_valid = TRUE;
  }

  *minx = s->minx;  *miny = s->miny;  *maxx = s->maxx;  *maxy = s->maxy;
}

void dump_link(linedef *ldf1,linedef *ldf2,link *ThisLink,char *s1)
{
  char s[200];

  sprintf(s,"%s Link",s1);
  if (ldf1) {
    sprintf(s,"%s between (%d,%d)-(%d,%d) and (%d,%d)-(%d,%d).",s,
      ldf1->from->x,ldf1->from->y,ldf1->to->x,ldf1->to->y,
      ldf2->from->x,ldf2->from->y,ldf2->to->x,ldf2->to->y);
  }
  announce(VERBOSE,s);
  announce(VERBOSE,"T W R ND FD C A S L M  h1  w1  w2  d1  d2  d3   fd  sc ");
  sprintf(s,"%1d %1d %1d  %1d  %1d %1d %1d %1d %1d %03d %03d %03d %03d %03d %03d %04d %03d %03d",
      (ThisLink->bits&LINK_TWIN)!=0,
      (ThisLink->bits&LINK_WINDOW)!=0,
      (ThisLink->bits&LINK_RECESS)!=0,
      (ThisLink->bits&LINK_NEAR_DOOR)!=0,
      (ThisLink->bits&LINK_FAR_DOOR)!=0,
      (ThisLink->bits&LINK_CORE)!=0,
      (ThisLink->bits&LINK_ALCOVE)!=0,
      (ThisLink->bits&LINK_STEPS)!=0,
      (ThisLink->bits&LINK_LIFT)!=0,
      (ThisLink->bits&LINK_MAX_CEILING)!=0,
      ThisLink->height1, ThisLink->width1, ThisLink->width2,
      ThisLink->depth1, ThisLink->depth2, ThisLink->depth3,
      ThisLink->floordelta, ThisLink->stepcount);
  announce(VERBOSE,s);
}

/* Push a new (defaulty) quest onto the given stack */
quest *push_quest(quest *old)
{
  quest *answer;

  answer = (quest *)malloc(sizeof(*answer));

  answer->goal = NULL_GOAL;
  answer->tag = 0;
  answer->type = 0;
  answer->count = 0;
  answer->room = NULL;
  answer->minrooms = 0;
  answer->auxtag = 0;
  answer->surprise = NULL;
  answer->next = old;
  return answer;
}

/* Pop the top off the stack, free it, return new top */
quest *pop_quest(quest *current)
{
  quest *answer;

  answer = current->next;
  free(current);
  return answer;
}

/******* Many routines from here on need more work ****************/

void Usage2(void)
{
  Usage0();
  printf("Switches that do something at the moment:\n");
  printf("  -rooms [n]   -seed [nnnnnn]  -outfile [filename" EXT_SEP "ext]\n");
  printf("  -restrict [012C] -ExMx -MAPxx -doom1 -doom2 -levels <x> \n");
  printf("  -minlight <x>  -music  -macho <nn> -noslinfo -nocustom -cwad \n");
  printf("  -arena  -nulls -nosemo -biwe -bimo -bimo! -huge\n");
  printf("  -fstart -ffstart -fend -ffend\n");
#ifdef SWITCHES_IN_CONFIG_FILES
  printf("(-outfile being most useful in the config file; on the\n");
  printf("command line you don't need the switch.)\n");
#endif
  printf("\nM O R E   D E T A I L S   T O   F O L L O W\n\n");
  return;
}

/*
   Anything that calls this routine is probably too simple to
   handle non-rectangular rooms!

                    Is there any vertex that's
   not marked that occurs in the given rectangle, or do any
   of these four points lie in any unmarked sector?  Or,
   assuming that these points are in clockwise or countercwise
   order, do any existing linedefs between unmarked vertexes
   intersect any of these four lines?  Probably pretty suboptimal!
*/
boolean empty_rectangle(level *l, int x1, int y1, int x2, int y2,
                                  int x3, int y3, int x4, int y4)
{
  int minx, maxx, miny, maxy;
  vertex *v;
  sector *s;
  linedef *ld;

  /* Find the enclosing rectangle of these points */
  if (x1>x2) {
    maxx = x1;
  } else {
    maxx = x2;
  }
  if (x3>maxx) maxx = x3;
  if (x4>maxx) maxx = x4;

  if (y1>y2) {
    maxy = y1;
  } else {
    maxy = y2;
  }
  if (y3>maxy) maxy = y3;
  if (y4>maxy) maxy = y4;

  if (x1<x2) {
    minx = x1;
  } else {
    minx = x2;
  }
  if (x3<minx) minx = x3;
  if (x4<minx) minx = x4;

  if (y1<y2) {
    miny = y1;
  } else {
    miny = y2;
  }
  if (y3<miny) miny = y3;
  if (y4<miny) miny = y4;

  /* Look at all unmarked vertexes, see if any */
  /* are within the enclosing rectangle.       */
  for (v=l->vertex_anchor;v;v=v->next) {
    if (v->marked==0)
      if ( ( (v->x <= maxx) && (v->x >= minx) ) &&
           ( (v->y <= maxy) && (v->y >= miny) ) ) return 0;
  }

  /* Now look at all sectors, see if any of these four */
  /* proposed vertexes is inside the rectangular envelope */
  for (s=l->sector_anchor;s;s=s->next) {
    if (s->marked==0) {
      find_rec(l,s,&minx,&miny,&maxx,&maxy);
      if ( (x1 <= maxx) && (x1 >= minx) &&
           (y1 <= maxy) && (y1 >= miny) ) return 0;
      if ( (x2 <= maxx) && (x2 >= minx) &&
           (y2 <= maxy) && (y2 >= miny) ) return 0;
      if ( (x3 <= maxx) && (x3 >= minx) &&
           (y3 <= maxy) && (y3 >= miny) ) return 0;
      if ( (x4 <= maxx) && (x4 >= minx) &&
           (y4 <= maxy) && (y4 >= miny) ) return 0;
    }
  }

  /* Now look at all linedefs, see if any intersects with */
  /* any of the four implied boundary lines.  Doesn't assume */
  /* axis-parallel lines, for a change!  Does assume there are */
  /* only four sides, though.  Need true polygons. */
  for (ld=l->linedef_anchor;ld;ld=ld->next) {
    if (ld->to->marked==0)
      if (ld->from->marked==0) {
        if (intersects(x1,y1,x2,y2,ld->from->x,ld->from->y,ld->to->x,ld->to->y))
          return 0;
        if (intersects(x2,y2,x3,y3,ld->from->x,ld->from->y,ld->to->x,ld->to->y))
          return 0;
        if (intersects(x3,y3,x4,y4,ld->from->x,ld->from->y,ld->to->x,ld->to->y))
          return 0;
        if (intersects(x4,y4,x1,y1,ld->from->x,ld->from->y,ld->to->x,ld->to->y))
          return 0;
      }
  }

  return 1;
}

/* Given a linedef and a point, return the distance from the */
/* linedef to the point, positive if the point is on the right */
/* side, negative if to the left.  May return HUGE_NUMBER if */
/* the point is not in either axis-shadow of the linedef, and */
/* may assume that the linedef is basically axis-parallel. */
int point_from_linedef(level *l, int x, int y, linedef *ld)
{
  int answer = HUGE_NUMBER;
  int candidate;
  int parity = 1;

  /* An utter kludge; why not just do it right? */

  if ((ld->from->x!=ld->to->x)&&(ld->from->x<=x)&&(ld->to->x>=x)) {
    candidate = abs(ld->to->y-y);
    if (candidate<answer) {
      answer = candidate;
      if (ld->to->y<y) parity = -1;
        else parity = 1;
    }
    candidate = abs(ld->from->y-y);
    if (candidate<answer) {
      answer = candidate;
      if (ld->from->y<y) parity = -1;
        else parity = 1;
    }
  }

  if ((ld->to->x!=ld->from->x)&&(ld->to->x<=x)&&(ld->from->x>=x)) {
    candidate = abs(ld->to->y-y);
    if (candidate<answer) {
      answer = candidate;
      if (ld->to->y<y) parity = 1;
        else parity = -1;
    }
    candidate = abs(ld->from->y-y);
    if (candidate<answer) {
      answer = candidate;
      if (ld->from->y<y) parity = 1;
        else parity = -1;
    }
  }

  if ((ld->from->y!=ld->to->y)&&(ld->from->y<=y)&&(ld->to->y>=y)) {
    candidate = abs(ld->to->x-x);
    if (candidate<answer) {
      answer = candidate;
      if (ld->to->x<x) parity = 1;
        else parity = -1;
    }
    candidate = abs(ld->from->x-x);
    if (candidate<answer) {
      answer = candidate;
      if (ld->from->x<x) parity = 1;
        else parity = -1;
    }
  }

  if ((ld->to->y!=ld->from->y)&&(ld->to->y<=y)&&(ld->from->y>=y)) {
    candidate = abs(ld->to->x-x);
    if (candidate<answer) {
      answer = candidate;
      if (ld->to->x<x) parity = -1;
        else parity = 1;
    }
    candidate = abs(ld->from->x-x);
    if (candidate<answer) {
      answer = candidate;
      if (ld->from->x<x) parity = -1;
        else parity = 1;
    }
  }

  return answer * parity;

}

/* Are any non-flying monsters so close to this linedef */
/* that if the sides of the linedef were unwalkable, the */
/* monster would be stuck? */
boolean no_monsters_stuck_on(level *l, linedef *ld)
{
  thing *m;
  int dist;

  for (m=l->thing_anchor;m;m=m->next) {
    if (!(m->genus->bits&MONSTER)) continue;   /* Only monsters */
    if (m->genus->bits&FLIES) continue;   /* Fliers can escape */
    dist = abs(point_from_linedef(l,m->x,m->y,ld));
    if (dist<=(MONSTER_WIDTH(m)/2)) {
#ifdef ALLOW_STUCK_MONSTERS_IN_BATHS
      announce(LOG,"Bath Bug!");
      return TRUE;
#endif
      return FALSE;
    }
  }

  return TRUE;

}

/* Return sector that the given x,y is in, and if dist is */
/* not null return the distance from the nearest wall that */
/* it is.  Or something like that.  NULL if in no sector. */
/* Set <danger>, if non-NULL, if any non-normal linedef is */
/* within 48 of the point. */
sector *point_sector(level *l,int x, int y, int *dist, boolean *danger)
{
  int thisdist, closest;
  linedef *ld, *ldbest;
  sector *answer = NULL;

  if (danger!=NULL) *danger = FALSE;
  closest = HUGE_NUMBER;
  for (ld=l->linedef_anchor;ld;ld=ld->next) {
    thisdist = point_from_linedef(l,x,y,ld);
    if (abs(thisdist)<49)
      if (ld->type!=LINEDEF_NORMAL)
        if (danger!=NULL)
          *danger = TRUE;
    if (abs(thisdist)<closest) {
      if (thisdist>0) {
        answer = ld->right->sector;
        closest = abs(thisdist);
        ldbest = ld;
      } else if (ld->left) {
        /* Actually, if we find that the closest thing is the left side */
        /* of a one-sided linedef, we should set answer to NULL, and */
        /* update closest and ldbest.  But, because sometimes the crude */
        /* point_from_linedef() seriously underestimates distances, we'll */
        /* actually do nothing in that case, on the theory that some */
        /* linedef that gives us a non-NULL answer is *really* the */
        /* closest one.  This is a hack; we should fix point_from_linedef */
        /* instead. */
        answer = ld->left->sector;
        closest = abs(thisdist);
        ldbest = ld;
      }
    }
  }

  if (dist!=NULL) *dist = closest;
  return answer;

}


/* Return a patch array followed by MUS-format pseudo-MIDI */
/* for one piece of music, and fill in the given header */
/* with the data it needs.  Return value is free()able. */
byte *one_piece(musheader *pmh)
{
  byte *answer;
  byte patch = roll(128);

  /* Now this is the very definition of "stub" */
  pmh->tag[0] = 'M';
  pmh->tag[1] = 'U';
  pmh->tag[2] = 'S';
  pmh->tag[3] = 0x1a;
  pmh->primchannels = 1;
  pmh->secchannels = 0;
  pmh->dummy = 0;
  pmh->patches = 1;
  pmh->headerlength = sizeof(musheader)+pmh->patches*sizeof(short);
  pmh->muslength = 17;
  answer = (byte *)malloc( pmh->patches*sizeof(short)+pmh->muslength );
  answer[0] = patch;  /* instrument */
  answer[1] = 0;
  answer[2] = 0x40;  /* Control change, channel zero */
  answer[3] = 0x00;  /* Select patch */
  answer[4] = patch; /* that again */
  answer[5] = 0x40;  /* Control change, channel zero */
  answer[6] = 0x07;  /* volume */
  answer[7] = 0x7f;  /* loud!       */

  answer[8] = 0x80 | 0x10;  /* Play note, channel zero, last event */
  answer[9] = 0x80 | 0x50;  /* Note 80, with volume */
  answer[10] = 0x7f;  /* volume */
  answer[11] = 70;  /* half-second delay */
  answer[12] = 0x00;  /* Note off, channel zero */
  answer[13] = 0x50;  /* Note 80 off */
  answer[14] = 0x80 | 0x10;  /* Play note, channel zero, last event */
  answer[15] = 0x4b;  /* Note 75, no volume */
  answer[16] = 70;  /* delay */

  answer[17] = 0x60;  /* end score */
  answer[18] = 0x4d;  /* maybe? */
  return answer;
}

/* Allocate, initialize, and return a new lmp for custom textures */
texture_lmp *new_texture_lmp(char *name)
{
  texture_lmp *answer = (texture_lmp *)malloc(sizeof(*answer));

  answer->name = strdup(name);
  answer->custom_texture_anchor = NULL;
  return answer;
}

/* Allocate, initialize, register with the texture (but don't */
/* bother returning) a new patch for this texture */
void add_patch(custom_texture *ct,short patchid,short x,short y)
{
  patch *answer = (patch *)malloc(sizeof(*answer));
  patch *p;

  answer->number = patchid;
  answer->x = x;
  answer->y = y;
  answer->next = NULL;
  /* Simplest if these are in frontward order */
  if (ct->patch_anchor==NULL) {
    ct->patch_anchor = answer;
  } else {
    for (p=ct->patch_anchor;p->next;p=p->next){};   /* find last */
    p->next = answer;
  }
}

/* Allocate, initialize, register with the given lmp, and return */
/* a new custom texture record structure thing */
custom_texture *new_custom_texture(texture_lmp *tl,char *name,
                                   short xsize, short ysize)
{
  custom_texture *answer = (custom_texture *)malloc(sizeof(*answer));

  answer->name = strdup(name);
  answer->xsize = xsize;
  answer->ysize = ysize;
  answer->patch_anchor = NULL;
  answer->next = tl->custom_texture_anchor;
  tl->custom_texture_anchor = answer;
  return answer;
}

/* Free up all resources associated with a texture lump */
void free_texture_lmp(texture_lmp *tl)
{
  custom_texture* ctp;
  patch *p;

  /* Free each texture */
  for (;;) {
    ctp = tl->custom_texture_anchor;
    if (ctp==NULL) break;
    tl->custom_texture_anchor = tl->custom_texture_anchor->next;
    /* Free each patch */
    for (;;) {
      p = ctp->patch_anchor;
      if (p==NULL) break;
      ctp->patch_anchor = ctp->patch_anchor->next;
      free(p);
    }
    free(ctp->name);
    free(ctp);
  }
  free(tl->name);
  free(tl);
}

/* Dump the given texture lmp to the given dump-handle */
void dump_texture_lmp(dumphandle dh,texture_lmp *tl)
{
  int texturecount = 0;
  int patchcount;
  int lmpsize,isize,i;
  custom_texture *ct;
  patch *p;
  byte *buf, *tbuf;

  /* First figure entire lmp size.  Four bytes of tcount... */
  lmpsize = 4;

  /* Plus four-plus-22 bytes per texture, plus 10 per patch */
  for (ct=tl->custom_texture_anchor;ct;ct=ct->next) {
    texturecount++;
    lmpsize += 4 + 22;    /* Four bytes index, 22 bytes structure */
    for (p=ct->patch_anchor;p;p=p->next) lmpsize += 10;
  }

  /* Get storage for the lmp itself */
  buf = (byte *)malloc(lmpsize);
  tbuf = buf;

  /* Write in the count */
  *(int *)tbuf = texturecount;
  swapint((unsigned int *)tbuf);
  tbuf += sizeof(int);

  /* Now traverse the textures again, and make the index */
  isize = 4 + 4 * texturecount;
  for (ct=tl->custom_texture_anchor;ct;ct=ct->next) {
    *(int *)tbuf = isize;
    swapint((unsigned int *)tbuf);
    tbuf += sizeof(int);
    isize += 22;    /* Four bytes index, 22 bytes structure */
    for (p=ct->patch_anchor;p;p=p->next) isize += 10;
  }

  /* Now one last time, writing the data itself */
  for (ct=tl->custom_texture_anchor;ct;ct=ct->next) {
    for (i=0;i<(int)strlen(ct->name);i++) {
      *(tbuf++) = (byte)((ct->name)[i]);
    }
    for (i=strlen(ct->name);i<8;i++) {
      *(tbuf++) = (byte)0;
    }
    *(short *)tbuf = 0;  tbuf += sizeof(short);
    *(short *)tbuf = 0;  tbuf += sizeof(short);
    *(short *)tbuf = ct->xsize;  swapshort((unsigned short *)tbuf); tbuf += sizeof(short);
    *(short *)tbuf = ct->ysize;  swapshort((unsigned short *)tbuf); tbuf += sizeof(short);
    *(short *)tbuf = 0;  tbuf += sizeof(short);
    *(short *)tbuf = 0;  tbuf += sizeof(short);
    for (patchcount=0,p=ct->patch_anchor;p;p=p->next) patchcount++;
    *(short *)tbuf = patchcount;  swapshort((unsigned short *)tbuf); tbuf += sizeof(short);
    for (p=ct->patch_anchor;p;p=p->next) {
      *(short *)tbuf = p->x;  swapshort((unsigned short *)tbuf); tbuf += sizeof(short);
      *(short *)tbuf = p->y;  swapshort((unsigned short *)tbuf); tbuf += sizeof(short);
      *(short *)tbuf = p->number;  swapshort((unsigned short *)tbuf); tbuf += sizeof(short);
      *(short *)tbuf = 1;  swapshort((unsigned short *)tbuf); tbuf += sizeof(short);
      *(short *)tbuf = 0;  tbuf += sizeof(short);
    }
  }

  RegisterLmp(dh,tl->name,lmpsize);
  fwrite(buf,lmpsize,1,dh->f);

}

/* Record any custom textures, made from existing patches, that */
/* we might want to show off by using.  Only works in DOOM2, sadly. */
/* In DOOM I, we'd have to recreate the entire TEXTURE2 (or 1) lump, */
/* and then add our stuff to it. */
void record_custom_textures(dumphandle dh, config *c)
{
  texture_lmp *tl;
  custom_texture *ct;

  /* Return if TEXTURE2 not available */
  if (c->gamemask&(DOOM0_BIT|DOOM1_BIT|DOOMI_BIT)) return;

  tl = new_texture_lmp("TEXTURE2");

  ct = new_custom_texture(tl,"GRAYALT",0x80,0x80);
  add_patch(ct,0x87,0,0);
  add_patch(ct,0x8a,0,0x40);
  add_patch(ct,0x8a,0x40,0);
  add_patch(ct,0x87,0x40,0x40);
  ct = new_custom_texture(tl,"TEKVINE",0x100,0x80);
  add_patch(ct,0x19b,0,0);
  add_patch(ct,0x183,0x40,0);
  add_patch(ct,0x19b,0x80,0);
  add_patch(ct,0x183,0xc0,0);
  add_patch(ct,0x35,0,0);    /* Vines! */
  ct = new_custom_texture(tl,"WOODVINE",0x100,0x80);
  add_patch(ct,0x1b1,0,0);    /* WOOD9 */
  add_patch(ct,0x1b1,0x40,0);    /* WOOD9 */
  add_patch(ct,0x1b1,0x80,0);    /* WOOD9 */
  add_patch(ct,0x1b1,0xc0,0);    /* WOOD9 */
  add_patch(ct,0x35,0,0);    /* Vines! */
  ct = new_custom_texture(tl,"WOODLITE",0x100,0x80);
  add_patch(ct,0x1ac,-4,0);   /* Copied from WOOD5 */
  add_patch(ct,0x1ad,124,0);   /* Copied from WOOD5 */
  add_patch(ct,0x1ac,252,0);   /* Copied from WOOD5 */
  add_patch(ct,0x78,32,20);   /* The light overlay */
  ct = new_custom_texture(tl,"DOORSKUL",0x40,0x48);
  add_patch(ct,0x6b,0,0);      /* The door */
  add_patch(ct,0x1ab,21,11);   /* The liddle skull */
  ct = new_custom_texture(tl,"EXITSWIT",0x40,0x80);
  add_patch(ct,0x87,0,0);
  add_patch(ct,0x87,0,64);
  add_patch(ct,0x177,16,70);
  add_patch(ct,0x79,16,104);
  ct = new_custom_texture(tl,"EXITSWIW",0x40,0x80);
  add_patch(ct,0xdd,0,0);
  add_patch(ct,0x173,0x0e,0x40);
  add_patch(ct,0x79,16,104);
  ct = new_custom_texture(tl,"EXITSWIR",0x40,0x80);
  add_patch(ct,0x12d,0,0);
  add_patch(ct,0x173,0x0f,0x42);
  add_patch(ct,0x79,16,104);
  ct = new_custom_texture(tl,"MARBGARG",0x40,0x80);
  add_patch(ct,0x0BD,0,0);      /* MWALL3_1 */
  add_patch(ct,0x1B2,6,31);     /* SW2_4 */

  dump_texture_lmp(dh,tl);
  free_texture_lmp(tl);
}  /* end record_custom_textures */


/* A primitive not-quite-random-field image-writing thing */
void basic_background(byte *fbuf, byte bottom, int range)
{
  int i,j;
  int above, below, left, right, total;

  for (i=0;i<64;i++) {
    for (j=(i&1);j<64;j+=2) {
      fbuf[64*i+j] = bottom + roll(range);
    }
  }

  for (i=0;i<64;i++) {
    for (j=1-(i&1);j<64;j+=2) {
      above = (i==0) ? 63 : i-1;
      below = (i==63) ? 0 : i+1;
      left  = (j==0) ? 63 : j-1;
      right = (j==63) ? 0 : j+1;
      total = fbuf[64*above+j] +
              fbuf[64*below+j] +
              fbuf[64*i+left] +
              fbuf[64*i+right];
      total >>= 2;
      fbuf[64*i+j] = total;
    }
  }
}

/* A primitive not-quite-random-field image-writing thing */
void basic_background2(byte *fbuf, byte bottom, int range)
{
  int i,j;
  int above, below, left, right, total;

  /* The randomly-set large grid */
  for (i=0;i<64;i+=2) {
    for (j=0;j<64;j+=2) {
      fbuf[64*i+j] = bottom + roll(range);
    }
  }

  /* The quinicunx points of the large grid */
  for (i=1;i<64;i+=2) {
    for (j=1;j<64;j+=2) {
      above = (i==0) ? 63 : i-1;
      below = (i==63) ? 0 : i+1;
      left  = (j==0) ? 63 : j-1;
      right = (j==63) ? 0 : j+1;
      total = fbuf[64*above+left] +
              fbuf[64*below+left] +
              fbuf[64*above+right] +
              fbuf[64*below+right] + 2;
      total >>= 2;
      fbuf[64*i+j] = total;
    }
  }

  /* The remaining grid */
  for (i=0;i<64;i++) {
    for (j=1-(i&1);j<64;j+=2) {
      above = (i==0) ? 63 : i-1;
      below = (i==63) ? 0 : i+1;
      left  = (j==0) ? 63 : j-1;
      right = (j==63) ? 0 : j+1;
      total = fbuf[64*above+j] +
              fbuf[64*below+j] +
              fbuf[64*i+left] +
              fbuf[64*i+right] + 2;
      total >>= 2;
      fbuf[64*i+j] = total;
    }
  }
}

/* A primitive not-quite-random-field image-writing thing */
void basic_background3(byte *fbuf, byte bottom, int range)
{
  int i,j;
  int above, below, left, right, total;

  for (i=0;i<64;i+=2) {
    for (j=(i&2);j<64;j+=4) {
      fbuf[64*i+j] = bottom + roll(range);
    }
  }

  for (i=0;i<64;i+=2) {
    for (j=2-(i&2);j<64;j+=4) {
      above = (i<2) ? i + 62 : i-2;
      below = (i>61) ? i - 62 : i+2;
      left  = (j<2) ? j + 62 : j-2;
      right = (j>61) ? j - 62 : j+2;
      total = fbuf[64*above+j] +
              fbuf[64*below+j] +
              fbuf[64*i+left] +
              fbuf[64*i+right];
      total >>= 2;
      total += roll(4) - roll(4);
      if (total<bottom) total = bottom;
      if (total>=bottom+range) total = bottom + range - 1;
      fbuf[64*i+j] = total;
    }
  }

  for (i=1;i<64;i+=2) {
    for (j=1-(i&1);j<64;j+=2) {
      above = (i==0) ? 63 : i-1;
      below = (i==63) ? 0 : i+1;
      left  = (j==0) ? 63 : j-1;
      right = (j==63) ? 0 : j+1;
      total = fbuf[64*above+j] +
              fbuf[64*below+j] +
              fbuf[64*i+left] +
              fbuf[64*i+right];
      total += roll(2) - roll(2);
      if (total<bottom) total = bottom;
      if (total>=bottom+range) total = bottom + range - 1;
      total >>= 2;
      fbuf[64*i+j] = total;
    }
  }
}

byte fbuf[64*64+4];   /* For use in making custom flats and patches; 64x64 */
byte pbuf[TLMPSIZE(0x80,0x40)];      /* Also */

/* Record any custom flats that we might want to show off by using. */
/* This is *much* simpler than textures! */
void record_custom_flats(dumphandle dh, config *c, boolean even_unused)
{
  short i,j,x,x2,y,dx,dy;
  boolean started = FALSE;

  if (even_unused || find_flat(c,"SLGRASS1")->used) {

    if (!started) {
      if (c->fstart) RegisterLmp(dh,"F_START",0);
      if (c->ffstart) RegisterLmp(dh,"FF_START",0);
    }
    started = TRUE;
    announce(VERBOSE,"SLGRASS1");

    basic_background2(fbuf,0x7c,4);
    x = roll(64);
    y = roll(64);
    for (;;) {
      dx = 1 - roll(3);
      dy = 1 - roll(3);
      if (dx && dy) break;
    }
    for (i=512;i;i--) {
      x += dx;
      y += dy;
      if (x<0) x += 64;
      if (x>63) x -= 64;
      if (y<0) y += 64;
      if (y>63) y -= 64;
      fbuf[64*x+y] = 0xbc+roll(4);
      x2 = (x==0) ? 63 : x-1;
      fbuf[64*x2+y] = 0xbc;
      x2 = (x==63) ? 0 : x+1;
      fbuf[64*x2+y] = 0xbf;
      if (roll(8)==0) dx = 1 - roll(3);
      if (roll(8)==0) dy = 1 - roll(3);
      for (;!(dx||dy);) {
        dx = 1 - roll(3);
        dy = 1 - roll(3);
      }
    }

    RegisterLmp(dh,"SLGRASS1",4096);
    fwrite(fbuf,4096,1,dh->f);
  }

  if (even_unused || find_flat(c,"SLSPARKS")->used) {

    if (!started) {
      if (c->fstart) RegisterLmp(dh,"F_START",0);
      if (c->ffstart) RegisterLmp(dh,"FF_START",0);
    }
    started = TRUE;
    announce(VERBOSE,"SLSPARKS");
    memset(fbuf,0,4096);
    for (i=512;i;i--)
      fbuf[roll(64) + 64*roll(64)] = 0xb0 + roll(16);
    RegisterLmp(dh,"SLSPARKS",4096);
    fwrite(fbuf,4096,1,dh->f);
  }

  if (even_unused || find_flat(c,"SLGATE1")->used) {

    if (!started) {
      if (c->fstart) RegisterLmp(dh,"F_START",0);
      if (c->ffstart) RegisterLmp(dh,"FF_START",0);
    }
    started = TRUE;
    announce(VERBOSE,"SLGATE1");

    basic_background2(fbuf,0x9c,4);

    for (i=4;i<60;i++) {
      for (j=4;j<60;j++) {
        dx = abs ( (i<<1) - 63 ) >> 2;
        dy = abs ( (j<<1) - 63 ) >> 2;
        x = 0xcf - (dx+dy)/2;
        x += roll(2);
        x -= roll(2);
        if (x>0xcf) x = 0xcf;
        if (x<0xc0) x = 0xc0;
        fbuf[64*i+j] = (byte)x;
      }
    }

    RegisterLmp(dh,"SLGATE1",4096);
    fwrite(fbuf,4096,1,dh->f);
  }

  if (even_unused || find_flat(c,"SLLITE1")->used) {

    if (!started) {
      if (c->fstart) RegisterLmp(dh,"F_START",0);
      if (c->ffstart) RegisterLmp(dh,"FF_START",0);
    }
    started = TRUE;
    announce(VERBOSE,"SLLITE1");

    basic_background2(fbuf,0x94,4);

    for (i=0;i<4;i++) {
      for (j=0;j<4;j++) {
        for (x=3;x<13;x++) {
          for (y=3;y<13;y++) {
            if ( (x==3 || x == 12) && (y==3 || y ==12) ) continue;
            dx = abs ( (x<<1) - 15 ) >> 2;
            dy = abs ( (y<<1) - 15 ) >> 2;
            if (dy>dx) dx = dy;
            x2 = 0xa1 + 2 * dx;
            x2 += roll(2) - roll(2);
            if (x2>0xa7) x2 = 0xa7;
            if (x2<0xa0) x2 = 0xa0;
            fbuf[64*(16*i+x)+16*j+y] = (byte)x2;
          }
        }
      }
    }

    RegisterLmp(dh,"SLLITE1",4096);
    fwrite(fbuf,4096,1,dh->f);
  }

  if (even_unused || find_flat(c,"SLFLAT01")->used) {

    if (!started) {
      if (c->fstart) RegisterLmp(dh,"F_START",0);
      if (c->ffstart) RegisterLmp(dh,"FF_START",0);
    }
    started = TRUE;
    announce(VERBOSE,"SLFLAT01");

    basic_background2(fbuf,0x6b,5);
    for (i=0;i<4096;i++)
      if (fbuf[i]>0x6d) fbuf[i] = 0x0;
    for (i=0;i<64;i++) {
      fbuf[i] = 0x6b;
      fbuf[64*i] = 0x6b;
      fbuf[63*64+i] = 0x6f;
      fbuf[64*i+63] = 0x6f;
    }

    RegisterLmp(dh,"SLFLAT01",4096);
    fwrite(fbuf,4096,1,dh->f);
  }

  if (started) {
    if (c->ffend) RegisterLmp(dh,"FF_END",0);
    if (c->fend) RegisterLmp(dh,"F_END",0);
  }
}

/* Record any custom/replacement patches that we might want to show off */
/* by using. */
void record_custom_patches(dumphandle dh, config *c, boolean even_unused)
{
  int rows, columns, i, j, lsize;
  byte *p, thispel;
  boolean started = FALSE;



  if (even_unused || FALSE) {

    if (!started) {
      RegisterLmp(dh,"P_START",0);    /* Which?  Both? */
      RegisterLmp(dh,"PP_START",0);
    }
    started = TRUE;

    rows = 0x80;
    columns = 0x40;
    lsize = TLMPSIZE(rows,columns);
    if (lsize>sizeof(pbuf))
      announce(ERROR,"Buffer overflow in r_c_t()");
    p = pbuf;
    /* The picture header */
    *(short *)p = columns;         swapshort((unsigned short *)p); p += sizeof(short);   /* Width */
    *(short *)p = rows;            swapshort((unsigned short *)p); p += sizeof(short);   /* Height */
    *(short *)p = (columns>>1)-1;  swapshort((unsigned short *)p); p += sizeof(short);   /* Width offset */
    *(short *)p = rows-5;          swapshort((unsigned short *)p); p += sizeof(short);   /* Magic */
    /* The pointers to the columns */
    for (i=0;i<columns;i++) {
      *(int *)p = 8 + 4 * (columns) + i * (rows+5); swapint((unsigned int *)p); p += sizeof(int);
    }
    /* The columns themselves */
    for (i=0;i<columns;i++) {
      /* The column header */
      *p = 0;           p++;
      *p = (byte)rows;  p++;
      /* The column itself, including silly bytes */
      for (j=-1;j<rows+1;j++) {
        if (rollpercent(10)) thispel = 0xc0 + roll(16);
          else thispel = 0;
        *p = thispel;  p++;
      }
      /* And finally */
      *p = 0xff; p++;
    }
    /* Whew! */

    RegisterLmp(dh,"WALL51_1",lsize);  /* DOOM I only */
    fwrite(pbuf,lsize,1,dh->f);
  }

  if (even_unused || FALSE) {

    if (!started) {
      RegisterLmp(dh,"P_START",0);    /* Which?  Both? */
      RegisterLmp(dh,"PP_START",0);
    }
    started = TRUE;

    rows = 0x80;
    columns = 0x40;
    lsize = TLMPSIZE(rows,columns);
    if (lsize>sizeof(pbuf))
      announce(ERROR,"Buffer overflow in r_c_t()");
    p = pbuf;
    /* The picture header */
    *(short *)p = columns;         swapshort((unsigned short *)p); p += sizeof(short);   /* Width */
    *(short *)p = rows;            swapshort((unsigned short *)p); p += sizeof(short);   /* Height */
    *(short *)p = (columns>>1)-1;  swapshort((unsigned short *)p); p += sizeof(short);   /* Width offset */
    *(short *)p = rows-5;          swapshort((unsigned short *)p); p += sizeof(short);   /* Magic */
    /* The pointers to the columns */
    for (i=0;i<columns;i++) {
      *(int *)p = 8 + 4 * (columns) + i * (rows+5);  swapint((unsigned int *)p); p += sizeof(int);
    }
    /* The columns themselves */
    for (i=0;i<columns;i++) {
      /* The column header */
      *p = 0;           p++;
      *p = (byte)rows;  p++;
      /* The column itself, including silly bytes */
      for (j=-1;j<rows+1;j++) {
        if (rollpercent(20)) thispel = 0xd0 + roll(16);
          else thispel = 0;
        *p = thispel;  p++;
      }
      /* And finally */
      *p = 0xff;  p++;
    }
    /* Whew! */

    RegisterLmp(dh,"WALL51_2",lsize);
    fwrite(pbuf,lsize,1,dh->f);
  }

  /* Next is the steel-rollup-door patch.  It's currently put into */
  /* the WALL51_3 slot, which means it appears in texture SP_DUDE5 */
  /* (instead of the yucchy dead guy hanging on the wall.)  The    */
  /* internal name for the texture is SLDOOR1.                     */

  if (even_unused || find_texture(c,"SLDOOR1")->used) {

    if (!started) {
      RegisterLmp(dh,"P_START",0);    /* Which?  Both? */
      RegisterLmp(dh,"PP_START",0);
    }
    started = TRUE;


    /* First a little correlated noise for "dirtying" */
    basic_background2(fbuf,0,5);

    /* Then the actual patch */
    rows = 0x80;
    columns = 0x40;
    lsize = TLMPSIZE(rows,columns);
    if (lsize>sizeof(pbuf))
      announce(ERROR,"Buffer overflow in r_c_t()");
    p = pbuf;
    /* The picture header */
    *(short *)p = columns;         swapshort((unsigned short *)p); p += sizeof(short);   /* Width */
    *(short *)p = rows;            swapshort((unsigned short *)p); p += sizeof(short);   /* Height */
    *(short *)p = (columns>>1)-1;  swapshort((unsigned short *)p); p += sizeof(short);   /* Width offset */
    *(short *)p = rows-5;          swapshort((unsigned short *)p); p += sizeof(short);   /* Magic */
    /* The pointers to the columns */
    for (i=0;i<columns;i++) {
      *(int *)p = 8 + 4 * (columns) + i * (rows+5);  swapint((unsigned int *)p);p += sizeof(int);
    }
    /* The columns themselves */
    for (i=0;i<columns;i++) {
      /* The column header */
      *p = 0;           p++;
      *p = (byte)rows;  p++;
      /* The column itself, including silly bytes */
      for (j=-1;j<rows+1;j++) {
        thispel = 0x60 + (j+1)%8;
        if ((j>=0) && (j<rows))
          thispel += 2 - fbuf[ 64*(i) + (j & 63)];
        *p = thispel;  p++;
      }
      /* And finally */
      *p = 0xff;  p++;
    }
    /* Whew! */

    RegisterLmp(dh,"WALL51_3",lsize);
    fwrite(pbuf,lsize,1,dh->f);
  }

  if (started) {
    RegisterLmp(dh,"PP_END",0);
    RegisterLmp(dh,"P_END",0);
  }

}


/* Compose replacements for the music sections used by the */
/* given config, and send them out the dumphandle. */
void make_music(dumphandle dh, config *c)
{
  musheader mh;
  byte *musbuf;

  /* Definitely a stub! */
  if (c->gamemask&DOOM1_BIT) {
    musbuf = one_piece(&mh);
    record_music(dh,&mh,musbuf,"D_INTROA",c);
    free(musbuf);
  }
  if (c->gamemask&DOOM2_BIT) {
    musbuf = one_piece(&mh);
    record_music(dh,&mh,musbuf,"D_DM2TTL",c);
    free(musbuf);
  }

}  /* end stubby make_music() */

/* Should there be a secret level after the current level? */
boolean need_secret_level(config *c)
{
  if (c->do_seclevels==FALSE) return FALSE;
  if (c->map==15) return TRUE;
  if (c->map==31) return TRUE;
  switch (c->episode) {
    case 1: return (c->mission == 3);
    case 2: return (c->mission == 5);
    case 3: return (c->mission == 6);
    case 4: return (c->mission == 2);
    default: return FALSE;
  }
}

/* Make a secret level following the current level.  With luck, */
/* the current level has an exit to it! */
void make_secret_level(dumphandle dh, haa *oldhaa, config *c)
{
  config *SecConfig;
  level SecLevel;
  haa *SecHaa;

  SecConfig = (config *)malloc(sizeof(*SecConfig));
  memcpy(SecConfig,c,sizeof(*c));
  SecHaa = (haa *)malloc(sizeof(*SecHaa));
  memcpy(SecHaa,oldhaa,sizeof(*oldhaa));
  secretize_config(SecConfig);
  if (SecConfig->map==31) SecConfig->map++;
  if (SecConfig->map==15) SecConfig->map = 31;
  if (SecConfig->episode!=0) SecConfig->mission = 9;
  NewLevel(&SecLevel,SecHaa,SecConfig);
  DumpLevel(dh,SecConfig,&SecLevel,SecConfig->episode,
                                   SecConfig->mission,
                                   SecConfig->map);
  if (SecConfig->map==31) {
    SecConfig->map=32;
    SecConfig->secret_themes = TRUE;
    NewLevel(&SecLevel,SecHaa,SecConfig);
    DumpLevel(dh,SecConfig,&SecLevel,SecConfig->episode,
                                     SecConfig->mission,
                                     SecConfig->map);
  }

}

/* Can this link be locked to the given quest?   Note that this is */
/* only called to check if an existing link made up at random can  */
/* be locked, so it can false-negative sometimes safely.           */
boolean link_fitsq(link *ThisLink,quest *ThisQuest)
{
  if (ThisQuest==NULL) return TRUE;  /* Nothing to fit */

  if (ThisQuest->goal==GATE_GOAL) {
    if (ThisLink->type==OPEN_LINK) return TRUE;  /* Easy */
    return FALSE;        /* else punt */
  }

  /* Keys and switches require doors */
  if ( (ThisQuest->goal==KEY_GOAL) || (ThisQuest->goal==SWITCH_GOAL) ) {
    if (!(ThisLink->bits&LINK_NEAR_DOOR)) return FALSE;
    if (!(ThisLink->type==BASIC_LINK)) return FALSE;
  }
  /* Actually because of nukage locks, SWITCH_GOALs don't require */
  /* doors.  Do something about that here. */
  /* For that matter, there are kinds of OPEN_LINK and GATE_LINK */
  /* that can fit a SWITCH_GOAL, also.  So fix that, too. */
  return TRUE;
}

/* Will this link fit along this linedef? */
boolean link_fitsh(linedef *ldf,link *ThisLink,config *c)
{
   int available, required;

   available = linelen(ldf);
   required = ThisLink->width1;

   switch (ThisLink->type) {
   case BASIC_LINK:
     if (required==0) required = 64;  /* Minimum to pass, eh? */
     if ((ThisLink->bits)&LINK_TWIN) available = (available/2)-16;
     if ((ThisLink->bits)&LINK_ALCOVE)
       required = required * 2 + ThisLink->depth3;
     break;
   case OPEN_LINK:
     if (required==0) required = 33;
     required += 66;
     break;
   case GATE_LINK:
     /* No gate-links outgoing from a gate room, eh? */
     if (ldf->right->sector->gate) {
       return FALSE;
     }
     return TRUE;
   default:
     announce(WARNING,"Funny type in link_fitsh");
     return FALSE;
   }

   return (available>=required);
}

/* Make the given linedefs (which currently form an open archway) */
/* into a cool set-of-bars door (appropriate to the quest).  For  */
/* the door/bar sectors, use newsector, or a new one if NULL.     */
/* Note that barwidth must be 32 or less, because the algorithm   */
/* has some quirks that I really ought to fix; if barwidth is too */
/* big, it can recurse forever or something like that.            */
void barify(level *l,linedef *ldf1,linedef *ldf2,quest *ThisQuest,
            int barwidth,sector *newsector,style *ThisStyle,config *c)
{
  linedef *ld1a, *ld1b, *ld2a, *ld2b, *ldedge1, *ldedge2;
  sector *oldsector;
  texture *t1;
  short type1;

  if (linelen(ldf1)<=32) return;  /* Already impassable */
  /* Get a handle on the sectors involved */
  oldsector = ldf1->left->sector;
  if (newsector==NULL) {
    newsector = clone_sector(l,oldsector);
    newsector->ceiling_height = newsector->floor_height;  /* close it! */
    if (ThisQuest)
      if (ThisQuest->goal==SWITCH_GOAL) newsector->tag = ThisQuest->tag;
    announce(VERBOSE,"Multiple");
  }
  /* Then recurse to get the side bars, if needed */
  ld1a = centerpart(l,ldf1,&ld1b,barwidth,ThisStyle,c);
  ld2a = centerpart(l,ldf2,&ld2b,barwidth,ThisStyle,c);
  barify(l,ldf1,ld2b,ThisQuest,barwidth,newsector,ThisStyle,c);
  barify(l,ld1b,ldf2,ThisQuest,barwidth,newsector,ThisStyle,c);
  /* Now frame the center section, the a's, with linedefs */
  ldedge1 = new_linedef(l,ld2a->to,ld1a->from);
  ldedge2 = new_linedef(l,ld1a->to,ld2a->from);
  /* Fix up existing sidedefs */
  ld1a->left->sector = newsector;
  ld1a->flags &= ~UPPER_UNPEGGED;
  ld1a->right->x_offset = 0;
  ld2a->left->sector = newsector;
  ld2a->flags &= ~UPPER_UNPEGGED;
  ld2a->right->x_offset = 0;
  /* And make some new ones */
  ldedge1->left = new_sidedef(l,newsector,c);
  ldedge1->right = new_sidedef(l,oldsector,c);
  ldedge1->flags |= TWO_SIDED;
  ldedge2->left = new_sidedef(l,newsector,c);
  ldedge2->right = new_sidedef(l,oldsector,c);
  ldedge2->flags |= TWO_SIDED;
  /* Decide on a texture for the bar faces */
  t1 = ThisStyle->support0;   /* Or wall0? */
  if (ThisQuest)
    if (ThisQuest->goal==KEY_GOAL)
      t1 = texture_for_key(ThisQuest->type,ThisStyle,c);
  /* and the opening linedef type */
  type1 = ThisStyle->doortype;
  if (ThisQuest) {
    if (ThisQuest->goal==KEY_GOAL) type1 = type_for_key(ThisQuest->type);
      else if (ThisQuest->goal==SWITCH_GOAL)
        type1 = (c->do_dm) ? LINEDEF_NORMAL_S1_DOOR : LINEDEF_NORMAL;
  }
  /* Now fill in all the textures and stuff */
  ld1a->type = type1;
  ld2a->type = type1;
  ld1a->right->upper_texture = t1;
  ld2a->right->upper_texture = t1;
  ldedge1->left->middle_texture = c->null_texture;
  ldedge1->right->middle_texture = c->null_texture;
  ldedge1->right->upper_texture = t1;
  ldedge2->left->middle_texture = c->null_texture;
  ldedge2->right->middle_texture = c->null_texture;
  ldedge2->right->upper_texture = t1;
  /* Record that we did that */
  l->barcount++;

}  /* end barify */

/* Take the given linedefs (which are currently antiparallel with */
/* just the void between them), and put in a nice too-narrow-to-  */
/* pass slit.  Or, sometimes, split the current linedefs in half  */
/* and recurse on the halves, for a set of slits.  Use the given  */
/* sector for the sector in the slits, or make one if NULL.       */
/* Sort of like both barify() and make_window().                  */
boolean slitify(level *l,linedef *ldf1,linedef *ldf2, int slitwidth,
                sector *newsector,style *ThisStyle,config *c)
{
  linedef *ld1a, *ld2a, *ldedge1, *ldedge2;
  sector *nearsector, *farsector;
  short newch, newch2, newfh, newfh2;
  int len = linelen(ldf1);

  /* Get a handle on the sectors involved */
  nearsector = ldf1->right->sector;
  farsector = ldf2->right->sector;
  /* Invent the new one, if needed */
  if (newsector==NULL) {
    newfh = nearsector->floor_height;
    if (farsector->floor_height<newfh) newfh = farsector->floor_height;
    if (rollpercent(30)) {
      newfh2 = newfh + 4 * roll(9);
      if (newfh2 > (nearsector->ceiling_height - 32)) newfh2 = newfh;
      if (newfh2 > (farsector->ceiling_height - 32)) newfh2 = newfh;
      newfh = newfh2;
    }
    newch = nearsector->ceiling_height;
    if (farsector->ceiling_height>newch) newch = farsector->ceiling_height;
    if (rollpercent(30)) {
      newch2 = newfh + 32 + 8 * roll(9);
      if (newch2>newch) newch2 = newch;
      if (newch < (nearsector->floor_height + 32)) newch2 = newch;
      if (newch < (farsector->floor_height + 32)) newch2 = newch;
      newch = newch2;
    }
    newsector = clone_sector(l,nearsector);
    newsector->floor_height = newfh;
    newsector->ceiling_height = newch;
  }
  /* Sometimes just recurse */
  if ((len>(16+slitwidth+slitwidth)) &&
      (rollpercent(60))) {
    ld1a = split_linedef(l,ldf1,len/2,c);
    ld2a = split_linedef(l,ldf2,len/2,c);
    slitify(l,ldf1,ld2a,slitwidth,newsector,ThisStyle,c);
    slitify(l,ld1a,ldf2,slitwidth,newsector,ThisStyle,c);
  } else {
    ld1a = centerpart(l,ldf1,NULL,slitwidth,ThisStyle,c);
    ld2a = centerpart(l,ldf2,NULL,slitwidth,ThisStyle,c);
    ldedge1 = new_linedef(l,ld2a->from,ld1a->to);
    ldedge2 = new_linedef(l,ld1a->from,ld2a->to);
    /* Fix up existing sidedefs */
    ld1a->right->middle_texture = c->null_texture;   /* Or grating? */
    ld1a->flags |= TWO_SIDED;
    ld2a->right->middle_texture = c->null_texture;   /* Or grating? */
    ld2a->flags |= TWO_SIDED;
    /* and make new ones */
    ldedge1->right = new_sidedef(l,newsector,c);
    ldedge1->right->middle_texture = ldf1->right->middle_texture;
    ldedge2->right = new_sidedef(l,newsector,c);
    ldedge2->right->middle_texture = ldf1->right->middle_texture;
    ldedge1->right->y_offset = ldedge2->right->y_offset =
      nearsector->ceiling_height - newsector->ceiling_height;
    ld1a->left = new_sidedef(l,newsector,c);
    ld1a->left->middle_texture = c->null_texture;
    patch_upper(ld1a,ldf1->right->middle_texture,c);
    patch_lower(ld1a,ThisStyle->support0,c);
    ld2a->left = new_sidedef(l,newsector,c);
    ld2a->left->middle_texture = c->null_texture;
    patch_upper(ld2a,ldf2->right->sector->style->wall0,c);
    patch_lower(ld2a,ldf2->right->sector->style->support0,c);
    announce(VERBOSE,"Slit");
  }

  return TRUE;

}  /* end slitify */

/* OK, so you have a single square sector bounded by ldf1 and ldf2 */
/* at the ends, and lde1 and lde2 at the sides.  They point */
/* *counterclockwise* around the outside of the sector.  This */
/* will make it into a flight of climbable stairs from nearheight */
/* at the ldf1 end to farheight at the ldf2 end. */
/* Should level-hugeness do anything in here? */
void stairify(level *l,linedef *ldf1,linedef *ldf2,linedef *lde1,
              linedef *lde2,short nearheight,short farheight,
              quest *ThisQuest, style *ThisStyle, config *c)
{
  linedef *ldn1, *ldn2, *ldl;
  sector *s, *nearsec;
  int len, stepdepth, i;
  int minstepcount, maxstepcount, stepcount, stepheight;
  boolean need_lock = (ThisQuest!=NULL) && (ThisQuest->goal==SWITCH_GOAL);
  texture *front = ThisStyle->kickplate;
  boolean do_edges = FALSE;

  nearsec = ldf1->right->sector;
  len = linelen(lde1);

  /* Need at least enough steps to get up, 24 at a time */
  minstepcount = (farheight-nearheight)/24;
  /* Want steps at least 24 deep to fit in the space */
  maxstepcount = len/24;

  stepcount = minstepcount + roll(1+(maxstepcount-minstepcount));
  stepdepth = len/stepcount;
  stepheight = (farheight-nearheight)/stepcount;

  /* Hack, to avoid having to actually understand the above */
  if (stepheight>24) {
    stepcount = stepcount + 1;
    stepdepth = len/stepcount;
    stepheight = (farheight-nearheight)/stepcount;
  }
  if (stepheight>24)
    announce(ERROR,"Step too high to climb!");
  if (need_lock) {
    announce(LOG,"Locked stairs");
    stepheight = 8;
    stepcount = (farheight-nearheight)/stepheight;
    stepdepth = len/stepcount;
  }

  {
  char s[100];
  sprintf(s,"%d steps from [%d-%d], each %d deep and %d high.\n",
    stepcount,minstepcount,maxstepcount,stepdepth,stepheight);
  announce(VERBOSE,s);
  sprintf(s,"Total: %d deep, %d high.\n",len,farheight-nearheight);
  announce(VERBOSE,s);
  }

  if (ThisStyle->stepfront)
    if (abs(stepheight)<=ThisStyle->stepfront->height)
      front = ThisStyle->stepfront;

  if (ThisStyle->walllight) {
    if (ThisStyle->light_steps) {
      front = ThisStyle->walllight;
    } else if (ThisStyle->light_edges &&
               (linelen(ldf1)>=(64*l->hugeness)) && (stepheight>7) ) {
      do_edges = TRUE;
      announce(VERBOSE,"Step-edge lights");
    }
  }

  if (need_lock) {
    ThisQuest->type = LINEDEF_S1_RAISE_STAIRS;
  }

  ldf1->right->lower_texture = front;
  ldf1->flags &= ~LOWER_UNPEGGED;

  for (i=0;i<(stepcount-1);i++) {   /* Minus one, since one's already made */
    s = clone_sector(l,nearsec);
    s->ceiling_height = ldf2->right->sector->ceiling_height;
    if (need_lock) if (i==0) s->tag = ThisQuest->tag;
    len -= stepdepth;
    ldn1 = split_linedef(l,lde1,len,c);
    ldn2 = lde2;
    lde2 = split_linedef(l,ldn2,stepdepth,c);
    ldn1->left->sector = s;
    ldn2->left->sector = s;
    ldf1->left->sector = s;
    if (do_edges) {
      ldl = split_linedef(l,ldf1,linelen(ldf1)-16*l->hugeness,c);
      ldl->right->lower_texture = ThisStyle->walllight;
      split_linedef(l,ldf1,16*l->hugeness,c);
      ldf1->right->lower_texture = ThisStyle->walllight;
    }
    nearheight += stepheight;
    s->floor_height = nearheight;
    ldf1 = new_linedef(l,ldn1->from,ldn2->to);
    ldf1->right = new_sidedef(l,s,c);
    ldf1->right->lower_texture = front;
    ldf1->flags &= ~LOWER_UNPEGGED;
    ldf1->right->middle_texture = c->null_texture;
    ldf1->left = new_sidedef(l,s,c);  /* s is wrong; fixed above/below */
    ldf1->left->middle_texture = c->null_texture;
    ldf1->flags |= TWO_SIDED;
    if (need_lock) s->floor_height = nearsec->floor_height;
    if (!need_lock) {  /* recalc to avoid top-step-looks-silly bug */
      stepheight = (farheight-nearheight)/(stepcount-(i+1));
      if (abs(stepheight)>front->height) front = ThisStyle->kickplate;
    }
  }
  ldf1->left->sector = ldf2->left->sector;
  patch_lower(ldf1,front,c);
  ldf1->flags &= ~LOWER_UNPEGGED;
  if (do_edges) {
    ldl = split_linedef(l,ldf1,linelen(ldf1)-16*l->hugeness,c);
    ldl->right->lower_texture = ThisStyle->walllight;
    split_linedef(l,ldf1,16*l->hugeness,c);
    ldf1->right->lower_texture = ThisStyle->walllight;
  }
  if (need_lock) {
    ldf2->left->sector->floor_height = nearsec->floor_height;
    ldf2->left->sector->floor_flat = nearsec->floor_flat;
  }
}

/* Make the given sector into a standard door, opened by the */
/* given linedefs.  Doesn't do any flipping, or alter jambs. */
void doorify(sector *s,linedef *ldf1,linedef *ldf2,
             style *ThisStyle,style *NewStyle, config *c)
{
  /* Needs to use style more, but almost done */
  int lensq;

  s->ceiling_height = s->floor_height;
  s->floor_flat = ThisStyle->doorfloor;
  s->ceiling_flat = ThisStyle->doorceiling;
  /* This should be from the style or link or role or something */
  ldf1->type = ThisStyle->doortype;
  lensq = lengthsquared(ldf1);
  /* This "100" should be determined from the width of the style textures */
  if (lensq>(100*100)) {  /* One of the wide textures */
    if (ldf1->right->sector->ceiling_height-s->floor_height>
        ThisStyle->widedoorface->height)
          ldf1->right->upper_texture = ThisStyle->twdoorface;
          else ldf1->right->upper_texture = ThisStyle->widedoorface;
    if (lensq<(128*128))    /* "128" is wrong */
      ldf1->right->x_offset = (128 - linelen(ldf1))/2;    /* All of these */
      else ldf1->right->x_offset = 128 - (linelen(ldf1)%128)/2;
    /* Avoid TFE! */
    if (ldf1->right->upper_texture->height<128)
      if (ldf1->right->sector->ceiling_height-ldf1->right->sector->floor_height>
        ldf1->right->upper_texture->height)
          ldf1->right->upper_texture = ThisStyle->twdoorface;
  } else {
    if (ldf1->right->sector->ceiling_height-s->floor_height>
      ThisStyle->narrowdoorface->height)
        ldf1->right->upper_texture = ThisStyle->tndoorface;
        else ldf1->right->upper_texture = ThisStyle->narrowdoorface;
    if (lensq<(64*64))  /* Also "64" */
      ldf1->right->x_offset = (64-linelen(ldf1))/2;  /* All of these */
      else ldf1->right->x_offset = 64 - (linelen(ldf1)%64)/2;
    /* Avoid TFE! */
    if (ldf1->right->upper_texture->height<128)
      if (ldf1->right->sector->ceiling_height-ldf1->right->sector->floor_height>
        ldf1->right->upper_texture->height)
          ldf1->right->upper_texture = ThisStyle->tndoorface;
  }
  ldf2->type = ldf1->type;
  lensq = lengthsquared(ldf2);
  /* This "100" should be determined from the width of the style textures? */
  if (lensq>(100*100)) {  /* One of the wide textures */
    if (ldf2->right->sector->ceiling_height-s->floor_height>
        NewStyle->widedoorface->height)
          ldf2->right->upper_texture = NewStyle->twdoorface;
          else ldf2->right->upper_texture = NewStyle->widedoorface;
    if (lensq<(128*128))
      ldf2->right->x_offset = (128 - linelen(ldf2))/2;
      else ldf2->right->x_offset = 128 - (linelen(ldf2)%128)/2;
    /* Avoid TFE! */
    if (ldf2->right->upper_texture->height<128)
      if (ldf2->right->sector->ceiling_height-ldf2->right->sector->floor_height>
        ldf2->right->upper_texture->height)
          ldf2->right->upper_texture = NewStyle->twdoorface;
  } else {
    if (ldf2->right->sector->ceiling_height-s->floor_height>
      NewStyle->narrowdoorface->height)
        ldf2->right->upper_texture = NewStyle->tndoorface;
        else ldf2->right->upper_texture = NewStyle->narrowdoorface;
    if (lensq<(64*64))
      ldf2->right->x_offset = (64 - linelen(ldf2))/2;
      else ldf2->right->x_offset = 64 - (linelen(ldf2)%64)/2;
    /* Avoid TFE! */
    if (ldf2->right->upper_texture->height<128)
      if (ldf2->right->sector->ceiling_height-ldf2->right->sector->floor_height>
        ldf2->right->upper_texture->height)
          ldf2->right->upper_texture = NewStyle->tndoorface;
  }
#ifdef SOMETIMES_UNPEG_DOORFACES
  if (!ThisStyle->secret_doors)    /* Doors secret-flavor? */
#endif
  {
    ldf1->flags &= ~UPPER_UNPEGGED;
    ldf2->flags &= ~UPPER_UNPEGGED;
  }
  if (ThisStyle->soundproof_doors) {
    ldf1->flags |= BLOCK_SOUND;
    ldf2->flags |= BLOCK_SOUND;
  }
  /* And in any case avoid stoop silliness */
  ldf1->flags &= ~LOWER_UNPEGGED;
  ldf2->flags &= ~LOWER_UNPEGGED;

}  /* end doorify */

/* Make a window between the given antiparallel linedefs */
boolean make_window_inner(level *l,linedef *ldf1,linedef *ldf2,link *ThisLink,
                                 style *ThisStyle,style *NewStyle,config *c)
{
  linedef *ldnew1, *ldnew2;
  sector *nearsec, *farsec, *newsec;
  texture *t1, *t2;
  int len;

  announce(VERBOSE,"Making a window");

  nearsec = ldf1->right->sector;
  farsec = ldf2->right->sector;
  t1 = ldf1->right->middle_texture;
  t2 = NewStyle->wall0;

  /* Make sure a window is possible */
  if (nearsec->floor_height+ThisStyle->sillheight>farsec->ceiling_height-16)
    return FALSE;
  if (nearsec->floor_height+ThisStyle->sillheight+ThisStyle->windowheight<
    farsec->floor_height+16)
      return FALSE;

  if (ThisStyle->slitwindows) {
    return slitify(l,ldf1,ldf2,16+roll(17),NULL,ThisStyle,c);
  }

  /* Put a little border on it.  Very simple version for now. */
  ldf1 = split_linedef(l,ldf1,ThisStyle->windowborder,c);
  len = linelen(ldf1);
  split_linedef(l,ldf1,len-ThisStyle->windowborder,c);
  ldf2 = split_linedef(l,ldf2,ThisStyle->windowborder,c);
  len = linelen(ldf2);
  split_linedef(l,ldf2,len-ThisStyle->windowborder,c);

  flip_linedef(ldf2);  /* parallelize, for make_box */
  newsec = make_box_ext(l,ldf1,ldf2,ThisStyle,c,&ldnew1,&ldnew2);
  flip_linedef(ldf2);

  newsec->floor_height = nearsec->floor_height + ThisStyle->sillheight;
  newsec->ceiling_height = newsec->floor_height + ThisStyle->windowheight;
  newsec->light_level = ThisStyle->doorlight0;  /* Wrongish */
  newsec->style = ThisStyle;  /* Is this right? */

  /* Various possibilities for window decor */
  switch (ThisStyle->window_decor) {
    case WINDOW_JAMBS:
      ldnew1->right->middle_texture = ThisStyle->doorjamb;
      ldnew2->right->middle_texture = ThisStyle->doorjamb;
      break;
    case WINDOW_SUPPORT:
      ldnew1->right->middle_texture = ThisStyle->support0;
      ldnew2->right->middle_texture = ThisStyle->support0;
      break;
    case WINDOW_LIGHT:
      make_lighted(l,newsec,c);
      if (ThisStyle->walllight) {
        ldnew1->right->middle_texture = ThisStyle->walllight;
        ldnew2->right->middle_texture = ThisStyle->walllight;
        announce(VERBOSE,"Lit window");
      } else {
        ldnew1->right->middle_texture = ThisStyle->support0;
        ldnew2->right->middle_texture = ThisStyle->support0;
      }
      break;
    default:  /* WINDOW_NORMAL */
      ldnew1->right->y_offset =
        ldnew2->right->y_offset =
          nearsec->ceiling_height - newsec->ceiling_height;
      break;
  }
  if (ThisStyle->window_grate) {
    ldf1->right->middle_texture = ThisStyle->grating;
    /* Unpeg, to keep the texture from floating away, eh? */
    ldf1->flags |= LOWER_UNPEGGED;
    ldf1->flags |= TWO_SIDED | IMPASSIBLE;
#ifdef OLD_FUNNY_WINDOWGRATES
    ldf2->right->middle_texture = NewStyle->grating;
    ldf2->flags |= LOWER_UNPEGGED;
    ldf2->flags |= TWO_SIDED | IMPASSIBLE;
#else
    ldf1->left->middle_texture = ThisStyle->grating;
    ldf2->flags |= TWO_SIDED;
#endif
    announce(VERBOSE,"Window grate");
  } else {
    ldf1->flags |= TWO_SIDED | IMPASSIBLE;
    ldf2->flags |= TWO_SIDED | IMPASSIBLE;
  }

  ldf1->left->y_offset = ldf1->right->y_offset =
    ldf2->left->y_offset = ldf2->right->y_offset = 0;

  /* Should windows ever block sound?  Prolly not! */

  /* Prevent texture-bleeding bug.  Will this do it? */
  /* Possibly the LOWER_UNPEGging above did it alread. */
  if (ThisStyle->window_grate) {
    if (newsec->floor_height==nearsec->floor_height)
      newsec->floor_height++;
    if (newsec->floor_height==farsec->floor_height)
      newsec->floor_height++;
  }

  patch_upper(ldf1,t1,c);
  patch_upper(ldf2,t2,c);
  patch_lower(ldf1,t1,c);
  patch_lower(ldf2,t2,c);

  return TRUE;

}  /* end make_window_inner */

/* Make a window between the given antiparallel linedefs, */
/* possibly elaborately. */
boolean make_window(level *l,linedef *ldf1,linedef *ldf2,link *ThisLink,
                           style *ThisStyle,style *NewStyle,config *c)
{
  if ((distancesquared(ldf1->to->x,ldf1->to->y,ldf2->from->x,ldf2->from->y)
      >=(l->hugeness*l->hugeness*96*96)) && c->window_airshafts) {
    linedef *ld1n, *ld2n, *lde1, *lde2;
    sector *newsec;
    short newfh, newch;
    sector *nearsec = ldf1->right->sector;
    sector *farsec = ldf2->right->sector;
    boolean rc1, rc2;
    ld1n = make_parallel(l,ldf1,16*l->hugeness,NULL);
    flip_linedef(ld1n);
    ld2n = make_parallel(l,ldf2,16*l->hugeness,NULL);
    flip_linedef(ld2n);
    lde1 = new_linedef(l,ld1n->to,ld2n->from);
    lde2 = new_linedef(l,ld2n->to,ld1n->from);
    newfh = nearsec->floor_height;
    if (farsec->floor_height<newfh) newfh = farsec->floor_height;
    if (rollpercent(50)) newfh -= 8 * roll(10);
    newch = nearsec->ceiling_height;
    if (farsec->ceiling_height>newch) newch = farsec->ceiling_height;
    newch += 16 + 8 * roll(10);
    newsec = new_sector(l,newfh,newch,random_flat0(OUTDOOR,c,NULL),
                        nearsec->ceiling_flat);
    newsec->style =
      copy_style(l,nearsec->style,nearsec->style->theme_number,0,c);
    newsec->style->roomlight0 = l->outside_light_level;
    /* Do we want to make the walls OUTDOOR here? */
    ld1n->right = new_sidedef(l,newsec,c);
    ld2n->right = new_sidedef(l,newsec,c);
    lde1->right = new_sidedef(l,newsec,c);
    lde2->right = new_sidedef(l,newsec,c);
    paint_room(l,newsec,newsec->style,c);
    place_plants(l,48,newsec,c);    /* Put in some plants for decor */
    rc1 = make_window_inner(l,ldf1,ld1n,ThisLink,ThisStyle,newsec->style,c);
    newsec->style->sillheight += farsec->floor_height - newsec->floor_height;
    rc2 = make_window_inner(l,ld2n,ldf2,ThisLink,newsec->style,NewStyle,c);
    newsec->ceiling_flat = c->sky_flat;
    if (rollpercent(l->p_force_nukage)) {
      newsec->floor_flat = newsec->style->nukage1;
      newsec->special = NUKAGE1_SPECIAL;
    }
    if (rc1||rc2) announce(LOG,"Window airshaft");
    return rc1 || rc2;
  } else {
    return make_window_inner(l,ldf1,ldf2,ThisLink,ThisStyle,NewStyle,c);
  }
}

/* Make a decorative room between the given antiparallel linedefs */
boolean make_decroom(level *l,linedef *ldf1,linedef *ldf2,config *c)
{
  linedef *ldnew1, *ldnew2;
  sector *nearsec, *farsec, *newsec;
  texture *t1;
  int len;
  style *ThisStyle = ldf1->right->sector->style;

  nearsec = ldf1->right->sector;
  farsec = ldf2->right->sector;
  t1 = ldf1->right->middle_texture;

  /* Put a little border on it.  Very simple version for now. */
  /* Don't really need at all if recesses etc, eh? */
  ldf1 = split_linedef(l,ldf1,ThisStyle->windowborder,c);
  len = linelen(ldf1);
  split_linedef(l,ldf1,len-ThisStyle->windowborder,c);
  ldf2 = split_linedef(l,ldf2,ThisStyle->windowborder,c);
  len = linelen(ldf2);
  split_linedef(l,ldf2,len-ThisStyle->windowborder,c);

  flip_linedef(ldf2);  /* parallelize, for make_box */
  newsec = make_box_ext(l,ldf1,ldf2,ThisStyle,c,&ldnew1,&ldnew2);
  flip_linedef(ldf2);

  newsec->floor_height = nearsec->floor_height - 8 * (roll(4));
  newsec->ceiling_height = nearsec->ceiling_height + 32 + 8 * (roll(6));
  newsec->light_level = l->outside_light_level;
  newsec->ceiling_flat = c->sky_flat;
  if (rollpercent(10)||rollpercent(l->p_force_nukage)) {
    newsec->floor_flat = ThisStyle->nukage1;
    newsec->special = NUKAGE1_SPECIAL;  /* Not that you can get in there! */
    announce(LOG,"Intertwin nukage");
  } else {
    newsec->floor_flat = random_flat0(OUTDOOR,c,NULL);
  }
  newsec->style = ThisStyle;

  ldnew1->right->y_offset =
    ldnew2->right->y_offset =
      nearsec->ceiling_height - newsec->ceiling_height;
  ldf1->right->middle_texture = ThisStyle->grating;
  ldf1->left->middle_texture = ThisStyle->grating;
  ldf2->right->middle_texture = ThisStyle->grating;
  ldf2->left->middle_texture = ThisStyle->grating;
  /* Unpeg, to keep the texture from floating away, eh? */
  ldf1->flags |= LOWER_UNPEGGED;
  ldf2->flags |= LOWER_UNPEGGED;

  ldf1->flags |= TWO_SIDED | IMPASSIBLE;
  ldf2->flags |= TWO_SIDED | IMPASSIBLE;
  ldf1->left->y_offset = ldf1->right->y_offset =
    ldf2->left->y_offset = ldf2->right->y_offset = 0;

  patch_upper(ldf1,t1,c);
  patch_upper(ldf2,t1,c);
  patch_lower(ldf1,t1,c);
  patch_lower(ldf2,t1,c);

  len = linelen(ldnew1);
  if (len>31) {  /* Inset a bit */
    linedef *lt1, *lt2;
    /* Unhook from alignment groups, for simplicity */
    if (ldf1->group_previous) {
      ldf1->group_previous->group_next = NULL;
      ldf1->group_previous = NULL;
    }
    if (ldf1->group_next) {
      ldf1->group_next->group_previous = NULL;
      ldf1->group_next = NULL;
    }
    if (ldf2->group_previous) {
      ldf2->group_previous->group_next = NULL;
      ldf2->group_previous = NULL;
    }
    if (ldf2->group_next) {
      ldf2->group_next->group_previous = NULL;
      ldf2->group_next = NULL;
    }
    /* Do it */
    lt1 = split_linedef(l,ldnew1,8,c);    /* 8's should vary */
    ldnew1->right->sector = ldf1->right->sector;
    ldnew1->right->y_offset = ldf1->right->y_offset;
    ldf1->from = ldnew1->to;
    lt2 = split_linedef(l,ldnew2,8,c);
    ldnew2->right->sector = ldf2->right->sector;
    ldnew2->right->y_offset = ldf2->right->y_offset;
    ldf2->from = ldnew2->to;
    lt2 = split_linedef(l,lt2,len-16,c);
    lt2->right->sector = ldf1->right->sector;
    lt2->right->y_offset = ldf1->right->y_offset;
    ldf1->to = lt2->from;
    lt1 = split_linedef(l,lt1,len-16,c);
    lt1->right->sector = ldf2->right->sector;
    lt1->right->y_offset = ldf2->right->y_offset;
    ldf2->to = lt1->from;
  }

  place_plants(l,48,newsec,c);    /* Put in some plants for decor */

  return TRUE;

}  /* end make_decroom */

texture *texture_for_key(short key, style *s, config *c)
{
  switch (key) {
    case ID_BLUEKEY:
    case ID_BLUECARD: return s->blueface;
    case ID_REDKEY:
    case ID_REDCARD: return s->redface;
    case ID_YELLOWKEY:
    case ID_YELLOWCARD: return s->yellowface;
  }
  announce(WARNING,"Unknown key in texture_for_key()");
  return c->error_texture;
}

texture *texture_for_bits(propertybits pb, style *s, config *c)
{
  texture *answer = NULL;

  switch (pb) {
    case BLUE: answer = s->blueface; break;
    case RED: answer = s->redface; break;
    case YELLOW: answer = s->yellowface; break;
    case LIGHT: answer = s->walllight; break;
  }
  if (answer==NULL) answer = s->wall0;
  return answer;
}

short type_for_key(short key)
{
  switch (key) {
    case ID_BLUEKEY:
    case ID_BLUECARD: return LINEDEF_BLUE_S1_DOOR;
    case ID_REDKEY:
    case ID_REDCARD: return LINEDEF_RED_S1_DOOR;
    case ID_YELLOWKEY:
    case ID_YELLOWCARD: return LINEDEF_YELLOW_S1_DOOR;
  }
  announce(WARNING,"Unknown key in type_for_key()");
  return LINEDEF_NORMAL_S1_DOOR;
}

/* Mark the given door of the given level to look like it's locked */
/* with the given key (thingid).                                   */
void mark_door_for_key(level *l,linedef *ldf1,short key,
                       style *ThisStyle,config *c)
{
  linedef *ldf2;
  texture *t1;

  if (ThisStyle->gaudy_locks) {
    announce(VERBOSE,"Gaudy door");
    ldf1->right->upper_texture =
          texture_for_key(key,ThisStyle,c);
  } else {
    t1 = texture_for_key(key,ThisStyle,c);
    ldf2 = split_linedef(l,ldf1,16,c);   /* '16' is wrong, but not bad */
    ldf1->right->upper_texture = t1;
    ldf2 = split_linedef(l,ldf2,linelen(ldf2)-16,c);
    ldf2->right->upper_texture = t1;
  }
}

void mark_door_for_lock(level *l,linedef *ldf1,style *ThisStyle,config *c)
{
  if (ThisStyle->lockdoorface==NULL) return;
  if (ThisStyle->lockdoorface->height!=128)
    if (ThisStyle->lockdoorface->height<
        (ldf1->right->sector->ceiling_height-ldf1->right->sector->floor_height))
          return;
  ldf1->right->upper_texture = ThisStyle->lockdoorface;
  announce(VERBOSE,"Specially marked door");
}

/* Given a linedef type, return the equivalent linedef type, */
/* only locked with the given key.  If there isn't one, return 0 */
short locked_linedef_for(short type,short key,config *c)
{
  switch (type) {
    case LINEDEF_S1_OPEN_DOOR:
      if (DOOM0_BIT & c->gamemask) return 0;  /* Not in ancient DooMs */
      switch (key) {
        case ID_BLUEKEY:
        case ID_BLUECARD:
          return LINEDEF_S1_OPEN_DOOR_BLUE;
        case ID_REDKEY:
        case ID_REDCARD:
          return LINEDEF_S1_OPEN_DOOR_RED;
        case ID_YELLOWKEY:
        case ID_YELLOWCARD:
          return LINEDEF_S1_OPEN_DOOR_YELLOW;
        default:
          announce(ERROR,"Unknown key in l_l_f");
          return 0;
      }
    default: return 0;
  }
}

/* Make the sector look likeit's in the range of a light */
void make_lighted(level *l, sector *s, config *c)
{
  /* Too many hardcoded constants! */
  if (rollpercent(60))
    if (s->light_level<l->lit_light_level)
      s->light_level = l->lit_light_level;
  if (rollpercent(10))
    s->special = RANDOM_BLINK;
}

/* Make a nice box with a thing to the left of the linedef */
linedef *lightbox(level *l,linedef *ld,genus *g,style *ThisStyle,config *c)
{
  int len;
  linedef *ldb;
  int x,y;
  sector *oldsec, *newsec;

  len = linelen(ld);
  if (len<48) return NULL;  /* All these "48"s should vary, eh? */ /* Hugeness? */
  if (!empty_left_side(l,ld,48)) return NULL;
  announce(VERBOSE,"lightbox");
  if (len>48)
    ld = centerpart(l,ld,NULL,48,ThisStyle,c);
  ldb = lefthand_box(l,ld,48,ThisStyle,c);    /* This one too */
  ldb->right->middle_texture = ThisStyle->wall0;
  oldsec = ld->right->sector;
  newsec = ldb->right->sector;
  newsec->special = ThisStyle->auxspecial;
  newsec->floor_height += ThisStyle->auxheight;
  if (newsec->ceiling_height-newsec->floor_height<64)
    newsec->floor_height = newsec->ceiling_height-64;
  patch_lower(ld,ThisStyle->wall0,c);
  point_from(ld->from->x,ld->from->y,
             ld->to->x,ld->to->y,
             LEFT_TURN,24,&x,&y);
  point_from(ld->to->x,ld->to->y,x,y,LEFT_TURN,24,&x,&y);
  if (g->height>(newsec->ceiling_height-newsec->floor_height))
    g = ThisStyle->shortlamp0;
  new_thing(l,x,y,0,g->thingid,7,c);
  if (g->bits&EXPLODES) announce(VERBOSE,"Barrelbox");
  return ld;
}

/* Make a nice bar with lights to the left of the linedef */
/* Actually looks pretty terrible!  Fix before using */
void lightbar(level *l,linedef *ld,propertybits pb,style *ThisStyle,config *c)
{
  int len, wid, dep;
  linedef *ldb,*lde1,*lde2;
  sector *oldsec, *newsec;

  len = linelen(ld);
  if (len<16) return;
  wid = 12 + roll(len-17);
  dep = 8 + 4 * roll(5);
  if (!empty_left_side(l,ld,dep)) return;
  announce(VERBOSE,"lightbar");
  if (len>wid)
    ld = centerpart(l,ld,NULL,wid,ThisStyle,c);
  ldb = lefthand_box_ext(l,ld,dep,ThisStyle,c,&lde1,&lde2);
  ldb->right->middle_texture =
   lde1->right->middle_texture =
   lde2->right->middle_texture = texture_for_bits(pb,ThisStyle,c);
  {
    texture *t = lde1->right->middle_texture;
    if (t!=ThisStyle->wall0)
      if (!(t->props & LIGHT))
        announce(LOG,"Colorbar");
  }
  oldsec = ld->right->sector;
  newsec = ldb->right->sector;
  newsec->special = ThisStyle->auxspecial;
  if (oldsec->light_level<=l->lit_light_level)
    if (rollpercent(60))
      newsec->light_level = oldsec->light_level + 20;
}

/* Return a <width>-long linedef which is the center of */
/* the given linedef.  In ld2, return the linedef that  */
/* is the far part of the triply-split line.            */
linedef *centerpart(level *l,linedef *ld,linedef **ld2,int width,
                    style *ThisStyle,config *c)
{
  int len, border;
  linedef *answer, *answer2;

  len = linelen(ld);
  border = (len - width) / 2;
  border += (len - (width + 2*border));   /* Fix roundoff errors */
  if (border<=0) {
    answer = ld;
    answer2 = ld;
  } else {
    answer = split_linedef(l,ld,border,c);
    answer2 = split_linedef(l,answer,width,c);
  }
  if (ld2) *ld2 = answer2;

  return answer;
}

/* Return a <width>-long linedef which is the center of */
/* the given linedef.  Optionally embellish the borders, */
/* if called for in the style. */
linedef *borderize(level *l,linedef *ld,int width,boolean fancy,
                   style *ThisStyle,propertybits pb,genus *keyg,
                   boolean *painted_door,config *c)
{
  linedef *answer, *ld2;
  sector *nearsec = ld->right->sector;
  sector *lsec;
  linedef *ldt;
  boolean try_keybox = TRUE;

  answer = centerpart(l,ld,&ld2,width,ThisStyle,c);

  /* Now optionally do fancy things to ld and ld2 */
  /* Gotta think of some other fancy things! */
  if (nearsec->ceiling_height - nearsec->floor_height < 88) try_keybox = FALSE;
  if (painted_door) *painted_door = FALSE;
  if (ld!=ld2)
    if (fancy)
      if (ThisStyle->lightboxes)
        if ((linelen(ld)>=64)) {
          short box_light_level = nearsec->light_level;
          short box_special = 0;
          genus *g = keyg;
          if (g==NULL) {
            if (rollpercent(l->p_barrels*2))
              g = random_barrel(c,ThisStyle);
            if (g==NULL) g = ThisStyle->lamp0;
          }
          if (g->bits&LIGHT) {
            if (ThisStyle->lightbox_lighting==LIGHTBOX_LIGHTED)
              if (box_light_level<l->lit_light_level)
                box_light_level = l->lit_light_level;
            if (ThisStyle->lightbox_lighting==LIGHTBOX_DARK)
              box_light_level = c->minlight;
            if (rollpercent(20)) box_special = RANDOM_BLINK;  /* 20? */
          }
          ldt = lightbox(l,ld,g,ThisStyle,c);
          /* Maybe do the cool keybox thing! */
          if (ldt&&keyg&&try_keybox) {
            lsec = ldt->left->sector;
            lsec->floor_height = nearsec->floor_height + 72;
            lsec->ceiling_height = lsec->floor_height + 32;
            patch_upper(ldt,nearsec->style->wall0,c);
            patch_lower(ldt,nearsec->style->wall0,c);
            ldt->flags |= IMPASSIBLE;
            lsec->special = GLOW_BLINK;
            if (lsec->light_level<l->lit_light_level)
              lsec->light_level = l->lit_light_level;
            if (painted_door) *painted_door = TRUE;
            announce(LOG,"Keybox");
          } else if (ldt) {
            ldt->left->sector->light_level = box_light_level;
            ldt->left->sector->special = box_special;
          }
          ldt = lightbox(l,ld2,g,ThisStyle,c);
          if (ldt&&keyg&&try_keybox) {
            lsec = ldt->left->sector;
            lsec->floor_height = nearsec->floor_height + 72;
            lsec->ceiling_height = lsec->floor_height + 32;
            patch_upper(ldt,nearsec->style->wall0,c);
            patch_lower(ldt,nearsec->style->wall0,c);
            ldt->flags |= IMPASSIBLE;
            lsec->special = GLOW_BLINK;
            if (lsec->light_level<l->lit_light_level)
              lsec->light_level = l->lit_light_level;
            if (painted_door) *painted_door = TRUE;
            announce(LOG,"Keybox");
          } else if (ldt) {
            ldt->left->sector->light_level = box_light_level;
            ldt->left->sector->special = box_special;
          }

#ifdef LIGHTBAR_STUFF
        } else if (linelen(ld)>16) {
          /* These actually look very silly; don't use! */
          lightbar(l,ld,pb,ThisStyle,c);
          lightbar(l,ld2,pb,ThisStyle,c);
#endif
        }

  return answer;
}

/* Try sticking a falling-core trap into the core bounded by the */
/* two given linedefs. */
void try_falling_core(level *l,linedef *ld1,linedef *ld2,haa *haa,config *c)
{
  int len, depth;
  boolean room1, room2;
  linedef *ldn1, *ldn2, *ldfar;
  sector *oldsec, *coresec;
  sector *downsec1 = NULL;
  sector *downsec2 = NULL;
  short downspec;

  oldsec = ld1->right->sector;
  depth = l->hugeness * (1 + 16 * ( 4 + roll(6) ));
  len = linelen(ld1)-(16*l->hugeness);
  ld1 = split_linedef(l,ld1,8*l->hugeness,c);
  split_linedef(l,ld1,len,c);
  room1 = empty_left_side(l,ld1,depth);
  ld2 = split_linedef(l,ld2,8*l->hugeness,c);
  split_linedef(l,ld2,len,c);
  room2 = empty_left_side(l,ld2,depth);
  if (!(room1||room2)) return;   /* No room! */
  switch (roll(6)) {
    case 0:
    case 1:
    case 2: downspec = 0; break;
    case 3: downspec = RANDOM_BLINK; break;
    case 4: downspec = SYNC_FAST_BLINK; break;
    case 5: downspec = SYNC_SLOW_BLINK; break;
    default: downspec = 0;
  }
  coresec = clone_sector(l,ld1->right->sector);
  coresec->tag = new_tag(l);
  ldn1 = new_linedef(l,ld2->to,ld1->from);
  ldn2 = new_linedef(l,ld1->to,ld2->from);
  ldn1->right = ldn2->right = new_sidedef(l,coresec,c);
  ldn1->left = ldn2->left = new_sidedef(l,oldsec,c);
  ldn1->left->middle_texture = c->null_texture;
  ldn1->right->middle_texture = c->null_texture;
  ldn1->right->lower_texture =
    ldn2->right->lower_texture = coresec->style->support0;
  ldn1->flags |= TWO_SIDED;
  ldn2->flags |= TWO_SIDED;
  ld1->right->sector = coresec;
  ld2->right->sector = coresec;
  if (room1) {
    ld1->right->upper_texture = ld1->right->middle_texture;
    ld1->right->y_offset = coresec->floor_height - coresec->ceiling_height;
    ldfar = lefthand_box_ext(l,ld1,depth,coresec->style,c,&ldn1,&ldn2);
    downsec1 = ld1->left->sector;
    ld1->right->middle_texture = c->null_texture;
    ld1->left->middle_texture = c->null_texture;
    ld1->flags |= TWO_SIDED | SECRET_LINEDEF;
    ldn1->tag = coresec->tag;
    ldn1->type = LINEDEF_SR_LOWER_LIFT;
    ldn2->tag = coresec->tag;
    ldn2->type = LINEDEF_SR_LOWER_LIFT;
    ldfar->right->middle_texture =
      ldn1->right->middle_texture =
      ldn2->right->middle_texture =
      ld1->left->lower_texture = coresec->style->support0;
    downsec1->floor_height = coresec->floor_height - 128;
    downsec1->ceiling_height = coresec->floor_height;
    downsec1->light_level = c->minlight + roll(40);
    downsec1->special = downspec;
  }
  if (room2) {
    ld2->right->upper_texture = ld2->right->middle_texture;
    ld2->right->y_offset = coresec->floor_height - coresec->ceiling_height;
    ldfar = lefthand_box_ext(l,ld2,depth,coresec->style,c,&ldn1,&ldn2);
    downsec2 = ld2->left->sector;
    ld2->right->middle_texture = c->null_texture;
    ld2->left->middle_texture = c->null_texture;
    ld2->flags |= TWO_SIDED | SECRET_LINEDEF;
    ldn1->tag = coresec->tag;
    ldn1->type = LINEDEF_SR_LOWER_LIFT;
    ldn2->tag = coresec->tag;
    ldn2->type = LINEDEF_SR_LOWER_LIFT;
    ldfar->right->middle_texture =
      ldn1->right->middle_texture =
      ldn2->right->middle_texture =
      ld2->left->lower_texture = coresec->style->support0;
    downsec2->floor_height = coresec->floor_height - 128;
    downsec2->ceiling_height = coresec->floor_height;
    downsec2->light_level = c->minlight + roll(40);
    downsec2->special = downspec;
  }
  /* Make the tripwire */
  split_linedef(l,ld1,len/2,c);
  if (room1) {
    downsec1->entry_x = ld1->to->x;
    downsec1->entry_y = ld1->to->y;
  }
  split_linedef(l,ld2,len/2,c);
  if (room2) {
    downsec2->entry_x = ld2->to->x;
    downsec2->entry_y = ld2->to->y;
  }
  ldn1 = new_linedef(l,ld1->to,ld2->to);
  ldn1->left = ldn1->right = new_sidedef(l,coresec,c);
  ldn1->left->middle_texture = c->null_texture;
  ldn1->flags |= TWO_SIDED;
  if (!(c->gamemask&DOOM0_BIT)) {
    ldn1->type = LINEDEF_WR_TURBO_LIFT;
  } else {
    ldn1->type = LINEDEF_WR_LOWER_LIFT;
  }
  ldn1->tag = coresec->tag;

  /* Monsters and stuff (works?) */
  if (room1) {
    place_monsters(l,downsec1,c,haa);
    place_health(l,downsec1,c,haa);
    place_ammo(l,downsec1,c,haa);
  }
  if (room2) {
    place_monsters(l,downsec2,c,haa);
    place_health(l,downsec2,c,haa);
    place_ammo(l,downsec2,c,haa);
  }

  /* and that's all */
  announce(VERBOSE,"Falling core");

}

/* Implement the given link between the given linedefs. */
/* For OPEN and BASIC links, these are antiparallel. */
void establish_link(level *l,linedef *ldf1,linedef *ldf2,link *ThisLink,
                     quest *ThisQuest,style *ThisStyle,style *NewStyle,
                     haa *haa,config *c)
{
  switch (ThisLink->type) {
    case BASIC_LINK:
      establish_basic_link(l,ldf1,ldf2,ThisLink,ThisQuest,
                           ThisStyle,NewStyle,haa,c);
      break;
    case OPEN_LINK:
      establish_open_link(l,ldf1,ldf2,ThisLink,ThisQuest,
                          ThisStyle,NewStyle,haa,c);
      break;
    case GATE_LINK: {
      short tag1, tag2;
      tag1 = new_tag(l);
      tag2 = new_tag(l);
      ldf1->right->sector->gate = new_gate(l,tag1,tag2,0,FALSE,c);
      ldf2->right->sector->gate = new_gate(l,tag2,tag1,0,TRUE,c);
      if (ThisQuest) {
        if (rollpercent(50)) {
          ThisQuest->type = LINEDEF_S1_OPEN_DOOR;
        } else {
          ThisQuest->type = LINEDEF_S1_LOWER_FLOOR;
        }
        ThisQuest->tag = tag1;
        ldf1->right->sector->gate->gate_lock = ThisQuest->type;
      }
      break;
    }
    default:
      announce(ERROR,"Unknown linktype, sectors not linked.");
      return;
  }
}

/* Implement the given link between the given (antiparallel) linedefs. */
/* Decide which way is up-going, call the inner routine. */
void establish_open_link(level *l,linedef *ldf1,linedef *ldf2,link *ThisLink,
                         quest *ThisQuest,style *ThisStyle,style *NewStyle,
                         haa *haa,config *c)
{
  int newfloor;
  sector *nearsec, *farsec;
  boolean need_lock;

  need_lock = (ThisQuest) && ( (ThisQuest->goal==SWITCH_GOAL) ||
                               (ThisQuest->goal==GATE_GOAL) );

  nearsec = ldf1->right->sector;
  farsec = ldf2->right->sector;
  /* Are these right? */
  farsec->entry_x = (ldf2->from->x + ldf2->to->x) / 2;
  farsec->entry_y = (ldf2->from->y + ldf2->to->y) / 2;

  if (need_lock||rollpercent(65)) {   /* Upward-going */
    /* Get recommendation */
    newfloor = nearsec->ceiling_height - ThisLink->height1;
    /* Rationalize it */
    if (newfloor-nearsec->floor_height<25)
      newfloor = nearsec->floor_height+25;
    if (newfloor-nearsec->floor_height>128)
      newfloor = nearsec->floor_height+128;
    /* Limit step steepness */
    if (ThisLink->bits&LINK_STEPS)
      if (newfloor-nearsec->floor_height>ThisLink->depth1)
        newfloor = nearsec->floor_height+ThisLink->depth1;
    /* OK, now set far sector, and do it */
    farsec->floor_height = newfloor;
    farsec->ceiling_height = farsec->floor_height + NewStyle->wallheight0;
    e_ol_inner(l,ldf1,ldf2,ThisLink,ThisQuest,ThisStyle,NewStyle,haa,c);
  } else {
    newfloor = nearsec->floor_height + ThisLink->height1
                 - NewStyle->wallheight0;
    if (nearsec->floor_height-newfloor<25)
      newfloor = nearsec->floor_height-25;
    if (nearsec->floor_height-newfloor>128)
      newfloor = nearsec->floor_height-128;
    /* Limit step steepness */
    if (ThisLink->bits&LINK_STEPS)
      if (nearsec->floor_height-newfloor>ThisLink->depth1)
        newfloor = nearsec->floor_height-ThisLink->depth1;
    farsec->floor_height = newfloor;
    farsec->ceiling_height = farsec->floor_height + NewStyle->wallheight0;
    e_ol_inner(l,ldf2,ldf1,ThisLink,ThisQuest,NewStyle,ThisStyle,haa,c);
  }

}

/* Implement the given link between the given (antiparallel) linedefs, */
/* always upward-going. */
void e_ol_inner(level *l,linedef *ldf1,linedef *ldf2,link *ThisLink,
                quest *ThisQuest,style *ThisStyle,style *NewStyle,
                haa *haa,config *c)
{
  linedef *ldf1a, *ldf1b, *ldf2a, *ldf2b;
  linedef *lde1, *lde2, *ldes;
  sector *sideseca, *sidesecb, *midsec;
  sector *nearsec, *farsec;
  int midwidth, len, sidefloor, dieroll;
  boolean nukage = FALSE;
  boolean high_sides = FALSE;
  boolean sidesteps = FALSE;

  if (ThisLink->bits&LINK_LIFT)
    announce(VERBOSE,"Open lift");

  if ((!(ThisLink->bits&LINK_LIFT))&&
      (!(ThisLink->bits&LINK_STEPS)))
    announce(WARNING,"Non-lift non-stair open link; oops!");

  if (rollpercent(l->p_force_nukage) || rollpercent(10)) nukage = TRUE;
  if ((ThisLink->bits&LINK_STEPS) && (ThisLink->bits&LINK_ALCOVE)) nukage=FALSE;

  nearsec = ldf1->right->sector;
  farsec = ldf2->right->sector;

  /* If a teleporter-goal, just make a simple air-connection. */
  /* Would(n't) this be more simply expressed as a BASIC link? */
  if (ThisQuest && (ThisQuest->goal==GATE_GOAL)) {
    midsec = clone_sector(l,farsec);
    midsec->floor_height = nearsec->floor_height;
    midsec->floor_flat = nearsec->floor_flat;
    ldf1->right->middle_texture = c->null_texture;
    ldf1->left = new_sidedef(l,midsec,c);
    ldf1->left->middle_texture = c->null_texture;
    ldf1->left->upper_texture = farsec->style->wall0;
    ldf1->flags |= TWO_SIDED | UPPER_UNPEGGED;
    patch_upper(ldf1,ThisStyle->wall0,c);
    patch_lower(ldf1,ThisStyle->support0,c);
    ldf2->right->middle_texture = c->null_texture;
    ldf2->left = new_sidedef(l,midsec,c);
    ldf2->left->middle_texture = c->null_texture;
    ldf2->left->lower_texture = farsec->style->wall0;
    ldf2->flags |= TWO_SIDED;
    patch_upper(ldf2,NewStyle->wall0,c);
    patch_lower(ldf2,NewStyle->support0,c);
    lde1 = new_linedef(l,ldf1->from,ldf2->to);
    lde1->right = new_sidedef(l,midsec,c);
    lde1->right->middle_texture = farsec->style->wall0;
    lde2 = new_linedef(l,ldf2->from,ldf1->to);
    lde2->right = new_sidedef(l,midsec,c);
    lde2->right->middle_texture = farsec->style->wall0;
    if (nukage) {
      ldf1->left->lower_texture = ThisStyle->support0;
      midsec->floor_flat = ThisStyle->nukage1;
      midsec->special = NUKAGE1_SPECIAL;
      if (midsec->light_level<160) midsec->light_level = 160;  /* visible */
      midsec->floor_height -= 8;
    }
    /* Now arrange for the gate and stuff */
    ThisQuest->tag = new_tag(l);
    if (rollpercent(50)) {
      ThisQuest->tag2 = 0;    /* Can be one-way */
    } else {
      ThisQuest->tag2 = new_tag(l);
      ldf2->right->middle_texture = ThisStyle->grating;
      ldf2->left->middle_texture = ThisStyle->grating;
      ldf2->flags |= IMPASSIBLE | LOWER_UNPEGGED;   /* Lower the grating, eh? */
    }
    farsec->gate = new_gate(l,ThisQuest->tag,ThisQuest->tag2,0,TRUE,c);
    announce(LOG,"OL Gate quest");
    return;  /* and that's it */
  }

  /* Otherwise it's (even) more complicated */

  /* Figure how wide */
  len = linelen(ldf1);
  if (len<100) announce(WARNING,"Open link on a too-narrow linedef!");
  midwidth = ThisLink->width1;
  if (midwidth==0) midwidth = linelen(ldf1)/3;
  if (midwidth<64) midwidth = 64;
  if (((len-midwidth)/2)<33) midwidth = len - 66;
  if (midwidth<33) midwidth = 33;

  /* Decide if doing the sideways-step thing */
  if ( (ThisLink->bits&LINK_STEPS) && (ThisLink->bits&LINK_ALCOVE) &&
       (midwidth>=(farsec->floor_height - nearsec->floor_height)) ) {
    sidesteps = TRUE;
  }

  /* Decide about nukage and side heights and stuff */
  dieroll = roll(100);
  if (sidesteps) {
    sidefloor=nearsec->floor_height;
  } else if ((dieroll<50)||nukage) {
    sidefloor = nearsec->floor_height;
  } else if (dieroll<75) {
    nukage = FALSE;
    high_sides = TRUE;
    sidefloor = farsec->floor_height;
    if (sidefloor>nearsec->ceiling_height-57)
      sidefloor=nearsec->ceiling_height-57;
  } else {
    nukage = FALSE;
    sidefloor = farsec->floor_height;
    if (sidefloor>nearsec->ceiling_height-57)
      sidefloor=nearsec->ceiling_height-57;
    if (farsec->floor_height-nearsec->floor_height>48) {
      sidefloor = nearsec->floor_height +
                    roll(1+sidefloor-nearsec->floor_height);
    }
  }

  ldf1->flags |= TWO_SIDED;
  ldf1->right->middle_texture = c->null_texture;
  ldf2->flags |= TWO_SIDED;
  ldf2->right->middle_texture = c->null_texture;

  ldf1a = ldf1;
  ldf1 = split_linedef(l,ldf1,(len-midwidth)/2,c);
  ldf1b = split_linedef(l,ldf1,midwidth,c);
  ldf2b = ldf2;
  ldf2 = split_linedef(l,ldf2,(len-midwidth)/2,c);
  ldf2a = split_linedef(l,ldf2,midwidth,c);

  midsec = clone_sector(l,farsec);
  if (ThisLink->bits&LINK_LIFT) {
    midsec->tag = new_tag(l);
    ldf1->type = NewStyle->slifttype;
    ldf1->tag = midsec->tag;
  }

  ldf1->left = new_sidedef(l,midsec,c);
  ldf1->left->middle_texture = c->null_texture;
  if (nukage&&(ThisLink->bits&LINK_LIFT))
    ldf1->left->lower_texture = ThisStyle->support0;
  ldf2->left = new_sidedef(l,midsec,c);
  ldf2->left->middle_texture = c->null_texture;
  if (ThisLink->bits&LINK_LIFT) {
    ldf2->left->lower_texture = NewStyle->support0;
    ldf2->flags |= LOWER_UNPEGGED;
  }
  patch_upper(ldf1,ThisStyle->wall0,c);
  patch_lower(ldf1,ThisStyle->support0,c);
  if ((ThisLink->bits&LINK_LIFT) &&
      (ThisStyle->liftface) &&
      (farsec->floor_height - nearsec->floor_height <=
       ThisStyle->liftface->height) &&
      (midwidth==ThisStyle->liftface->width) ) {
    ldf1->right->lower_texture = ThisStyle->liftface;
    ldf1->right->x_offset = 0;
    announce(VERBOSE,"Lift texture");
  }
  ldf1->flags &= ~LOWER_UNPEGGED;
  patch_upper(ldf2,NewStyle->wall0,c);
  patch_lower(ldf2,NewStyle->wall0,c);

  flip_linedef(ldf2a);
  sideseca = make_box_ext(l,ldf1a,ldf2a,ThisStyle,c,&lde1,&lde2);
  flip_linedef(ldf2a);
  sideseca->floor_height = sidefloor;
  sideseca->ceiling_height = midsec->ceiling_height;
  sideseca->ceiling_flat = midsec->ceiling_flat;
  lde1->right->middle_texture = NewStyle->wall0;
  lde2->left = new_sidedef(l,midsec,c);
  lde2->flags |= TWO_SIDED;
  lde2->left->middle_texture = c->null_texture;
  lde2->left->lower_texture = NewStyle->support0;
  lde2->right->lower_texture = NewStyle->wall0;
  lde2->right->middle_texture = c->null_texture;
  lde2->right->y_offset = farsec->ceiling_height - farsec->floor_height;
  ldes = lde2;  /* Save for stairification */
  patch_upper(ldf1a,ThisStyle->wall0,c);
  patch_lower(ldf1a,ThisStyle->wall0,c);
  patch_upper(ldf2a,NewStyle->wall0,c);
  patch_lower(ldf2a,NewStyle->wall0,c);
  if (nukage) {
    announce(VERBOSE,"Open nukage");
    sideseca->floor_height -= 8;
    sideseca->floor_flat = ThisStyle->nukage1;
    sideseca->special = NUKAGE1_SPECIAL;
    patch_lower(ldf1a,ThisStyle->support0,c);
    patch_lower(ldf2a,ThisStyle->support0,c);
    nearsec->marked = farsec->marked = TRUE;
    if (c->gunk_channels && empty_left_side(l,lde1,32)) {
      lefthand_box(l,lde1,32,ThisStyle,c)->right->middle_texture =
        ThisStyle->support0;
      lde1->left->sector->ceiling_height =
        lde1->left->sector->floor_height + 8;
      lde1->left->sector->light_level =
        lde1->right->sector->light_level - 20;
      lde1->left->sector->floor_flat = ThisStyle->nukage1;
      patch_upper(lde1,NewStyle->wall0,c);
      announce(VERBOSE,"Channel");
    }
    nearsec->marked = farsec->marked = FALSE;
  }

  flip_linedef(ldf2b);
  sidesecb = make_box_ext(l,ldf1b,ldf2b,ThisStyle,c,&lde1,&lde2);
  flip_linedef(ldf2b);
  sidesecb->floor_height = sidefloor;
  sidesecb->ceiling_height = midsec->ceiling_height;
  sidesecb->ceiling_flat = midsec->ceiling_flat;
  lde2->right->middle_texture = NewStyle->wall0;
  lde1->left = new_sidedef(l,midsec,c);
  lde1->flags |= TWO_SIDED;
  lde1->left->middle_texture = c->null_texture;
  lde1->left->lower_texture = NewStyle->support0;
  lde1->right->lower_texture = NewStyle->wall0;
  lde1->right->middle_texture = c->null_texture;
  lde1->right->y_offset = farsec->ceiling_height - farsec->floor_height;
  patch_upper(ldf1b,ThisStyle->wall0,c);
  patch_lower(ldf1b,ThisStyle->wall0,c);
  patch_upper(ldf2b,NewStyle->wall0,c);
  patch_lower(ldf2b,NewStyle->wall0,c);
  if (nukage) {
    sidesecb->floor_height -= 8;
    sidesecb->floor_flat = ThisStyle->nukage1;
    sidesecb->special = NUKAGE1_SPECIAL;
    patch_lower(ldf1b,ThisStyle->support0,c);
    patch_lower(ldf2b,ThisStyle->support0,c);
    nearsec->marked = farsec->marked = TRUE;
    if (c->gunk_channels && empty_left_side(l,lde2,32)) {
      lefthand_box(l,lde2,32,ThisStyle,c)->right->middle_texture =
        ThisStyle->support0;
      lde2->left->sector->ceiling_height =
        lde2->left->sector->floor_height + 8;
      lde2->left->sector->light_level =
        lde2->right->sector->light_level - 20;
      lde2->left->sector->floor_flat = ThisStyle->nukage1;
      patch_upper(lde2,NewStyle->wall0,c);
      announce(VERBOSE,"Channel");
    }
    nearsec->marked = farsec->marked = FALSE;
  }

  /* Could be more interesting... */
  midsec->light_level = sideseca->light_level = sidesecb->light_level =
    ThisStyle->roomlight0;

  /* Make center into stairs if we need them */
  if ((ThisLink->bits&LINK_STEPS)&&!sidesteps) {
    announce(VERBOSE,"Open stairs");
    /* Maybe stick on some lights */
    if (rollpercent(50)) {                      /* 50 should vary? */
      genus *g = ThisStyle->lamp0;
      if (g->height>(sideseca->ceiling_height-sideseca->floor_height))
        g = ThisStyle->shortlamp0;
      if ( (high_sides&&((len-midwidth)/2 >= 2 * g->width)) ||
           ((len-midwidth)/2 >= g->width + 69) ) {
        announce(VERBOSE,"and lights");
        new_thing(l,
          (ldf1a->to->x + ldf1a->from->x + ldf2a->to->x + ldf2a->from->x)/4,
          (ldf1a->to->y + ldf1a->from->y + ldf2a->to->y + ldf2a->from->y)/4,
          0,g->thingid,7,c);
        new_thing(l,
          (ldf1b->to->x + ldf1b->from->x + ldf2b->to->x + ldf2b->from->x)/4,
          (ldf1b->to->y + ldf1b->from->y + ldf2b->to->y + ldf2b->from->y)/4,
          0,g->thingid,7,c);
        if (rollpercent(70)) {  /* Should be in link/style? */
          if (sideseca->light_level<=l->bright_light_level)
            sideseca->light_level += 20;    /* Lamps light things up */
          if (sidesecb->light_level<=l->bright_light_level)
            sidesecb->light_level += 20;
          if (midsec->light_level>c->minlight)
            midsec->light_level -= 20;        /* and throw shadows! */
        }
      }
    }
    /* Change a few details */
    lde1->right->y_offset = 0;
    lde1->left->lower_texture = NewStyle->wall0;
    lde1->flags |= LOWER_UNPEGGED;
    ldes->right->y_offset = 0;
    ldes->left->lower_texture = NewStyle->wall0;
    ldes->flags |= LOWER_UNPEGGED;
    if (ThisStyle->light_steps && ThisStyle->walllight) {
      ldf1->right->lower_texture = ThisStyle->walllight;
    } else {
      ldf1->right->lower_texture = ThisStyle->kickplate;
    }
    ldf2->left->lower_texture = NewStyle->wall0;  /* In case of lock */
    /* Make the center into stairs */
    stairify(l,ldf1,ldf2,ldes,lde1,nearsec->floor_height,farsec->floor_height,
      ThisQuest,ThisStyle,c);
  }

  /* Or make the center into *sideways* stairs */
  if ((ThisLink->bits&LINK_STEPS)&&sidesteps) {
    announce(NONE,"Open side-stairs");
    ldf1->right->lower_texture = ThisStyle->wall0;
    ldf2->left->lower_texture = NewStyle->wall0;
    ldf1->right->y_offset = 0;
    ldf1->left->lower_texture = NewStyle->wall0;
    ldf1->flags |= LOWER_UNPEGGED;
    if (ThisLink->bits&LINK_LEFT) {
      if (ThisStyle->light_steps && ThisStyle->walllight) {
        ldes->right->lower_texture = ThisStyle->walllight;
      } else {
        ldes->right->lower_texture = ThisStyle->kickplate;
      }
      ldes->right->y_offset = 0;
      lde1->left->lower_texture = NewStyle->wall0;  /* In case of lock? */
      sidesecb->floor_height = farsec->floor_height;
      sidesecb->floor_flat = farsec->floor_flat;
    } else {
      if (ThisStyle->light_steps && ThisStyle->walllight) {
        lde1->right->lower_texture = ThisStyle->walllight;
      } else {
        lde1->right->lower_texture = ThisStyle->kickplate;
      }
      lde1->right->y_offset = 0;
      ldes->left->lower_texture = NewStyle->wall0;  /* In case of lock? */
      sideseca->floor_height = farsec->floor_height;
      sideseca->floor_flat = farsec->floor_flat;
    }
    ldf2->right->y_offset = 0;
    ldf2->left->lower_texture = NewStyle->wall0;
    ldf2->flags |= LOWER_UNPEGGED;
    patch_lower(ldf1a,ThisStyle->wall0,c);
    patch_lower(ldf1b,ThisStyle->wall0,c);
    if (ThisLink->bits&LINK_LEFT) {
      stairify(l,ldes,lde1,ldf2,ldf1,nearsec->floor_height,farsec->floor_height,
        ThisQuest,ThisStyle,c);
    } else {
      stairify(l,lde1,ldes,ldf1,ldf2,nearsec->floor_height,farsec->floor_height,
        ThisQuest,ThisStyle,c);
    }
  }

  /* Bells and whistles! */
  if ((farsec->floor_height-sideseca->floor_height==128) &&
      (linelen(ldf2a)>=128)) {
    if (linelen(ldf2a)>128) {
      ldf2a = centerpart(l,ldf2a,NULL,128,ThisStyle,c);
      ldf2b = centerpart(l,ldf2b,NULL,128,ThisStyle,c);
    }
    ldf2a->left->lower_texture = ThisStyle->plaque;
    ldf2a->left->x_offset = 0;
    ldf2a->left->y_offset = 0;
    ldf2a->flags &= ~LOWER_UNPEGGED;
    ldf2b->left->lower_texture = ThisStyle->plaque;
    ldf2b->left->x_offset = 0;
    ldf2b->left->y_offset = 0;
    ldf2b->flags &= ~LOWER_UNPEGGED;
    announce(VERBOSE,"Open-link plaques");
  }

  /* and so on */

}

/* Implement the given link between the given (antiparallel) linedefs. */
/* Set bits for any ephemera, then call inner recursive routine.  */
void establish_basic_link(level *l,linedef *ldf1,linedef *ldf2,link *ThisLink,
                          quest *ThisQuest,style *ThisStyle,style *NewStyle,
                          haa *haa,config *c)
{
  ThisStyle->lightboxes = rollpercent(5);  /* Should be from style, or? */
  e_bl_inner(l,ldf1,ldf2,ThisLink,ThisQuest,ThisStyle,NewStyle,0,haa,c);
  ThisStyle->lightboxes = FALSE;  /* Just to be neat */
}

/* Implement the given link between the given (antiparallel) linedefs */
/* Potentially recursive, for windows and twinnings.                  */
/* Needs lots of cleaning up and organizing and splitting into */
/* smaller functions!                                          */
void e_bl_inner(level *l,linedef *ldf1,linedef *ldf2,link *ThisLink,
                     quest *ThisQuest,style *ThisStyle,style *NewStyle,
                     short flipstate, haa *haa,config *c)
{
  linedef *ldnew1, *ldnew2;
  int len, border, maxtop;
  texture *t1, *t2;
  sector *nearsec, *farsec, *newsec = NULL;
  boolean need_to_doorify = FALSE;
  linedef *ldflip1a = NULL;
  linedef *ldflip1b = NULL;
  linedef *ldflip2a = NULL;
  linedef *ldflip2b = NULL;
  sector *sflip1 = NULL;
  sector *sflip2 = NULL;
  linedef *ldedge1, *ldedge2;
  int tag1 = 0;
  boolean trigger_lift = FALSE;
  boolean trigger_door = FALSE;
  boolean painted_door = FALSE;
  int mminx, mminy, mmaxx, mmaxy, mangle;
  propertybits effective_left = ThisLink->bits&LINK_LEFT;
  propertybits litecol = LIGHT;

  if ((ThisLink->bits&LINK_CORE)&&(ThisLink->bits&LINK_ANY_DOOR))
    announce(VERBOSE,"Core and door(s)");

  if ((ThisQuest) && (ThisQuest->goal==KEY_GOAL)) {
    switch (ThisQuest->type) {
      case ID_BLUEKEY:
      case ID_BLUECARD: litecol = BLUE; break;
      case ID_REDKEY:
      case ID_REDCARD: litecol = RED; break;
      case ID_YELLOWKEY:
      case ID_YELLOWCARD: litecol = YELLOW; break;
    }
  } else {
    litecol = LIGHT;
  }

  /* The type of a SWITCH_GOAL isn't set until the link's established */
  if (ThisQuest)
    if (ThisQuest->goal==SWITCH_GOAL) {
      if (ThisLink->bits&LINK_LOCK_CORE) {
        ThisQuest->type = LINEDEF_S1_RAISE_AND_CLEAN_FLOOR;
      } else {
        ThisQuest->type = LINEDEF_S1_OPEN_DOOR;
      }
    }

  dump_link(ldf1,ldf2,ThisLink,"Establishing");

  if ((ThisLink->bits&LINK_ALCOVE) &&
      (ThisLink->bits&LINK_TWIN) &&
      (ThisLink->bits&LINK_ANY_DOOR) ) announce(VERBOSE,"Twin door alcoves!");

  nearsec = ldf1->right->sector;
  farsec = ldf2->right->sector;

  /* Figure floor and ceiling heights for new sector */
  farsec->floor_height = nearsec->floor_height + ThisLink->floordelta;
  farsec->ceiling_height = farsec->floor_height + NewStyle->wallheight0;

  /* Make sure we don't overdo the bar thing and crash the engine... */
  if (l->barcount>LEVEL_MAX_BARS) ThisLink->bits &= ~LINK_BARS;

  /* See if we need to force the floordelta toward zero to avoid */
  /* impassable doorways. */
  if (ThisLink->bits&LINK_STEPS) {
    int need;
    /* The clearance we need is 56 plus the step height times */
    /* the number of steps our 64ish-wide shadow is on at once */
    need = 64 + (1+(64/(ThisLink->depth3/(ThisLink->stepcount))))
              * abs(ThisLink->floordelta / (ThisLink->stepcount-1));
    if (ThisLink->bits&LINK_ANY_DOOR) need += 8;  /* Doors don't open all the way */
    if ( (farsec->ceiling_height-farsec->floor_height<need) ||
         (nearsec->ceiling_height-nearsec->floor_height<need) ) {
      /* There's probably something less drastic we can do here... */
      ThisLink->floordelta = 0;
      ThisLink->bits &= ~LINK_STEPS;
      farsec->floor_height = nearsec->floor_height + ThisLink->floordelta;
      farsec->ceiling_height = farsec->floor_height + NewStyle->wallheight0;
    }
  }

  /* If we need to twin, split and recurse, or do the window thing */
  if ((flipstate==0)&&(ThisLink->bits&LINK_TWIN)) {
    len = linelen(ldf1)/2;
    ldnew1 = split_linedef(l,ldf1,len,c);
    ldnew2 = split_linedef(l,ldf2,len,c);
    if (!(ThisLink->bits&LINK_WINDOW)) {  /* make twin links */
      e_bl_inner(l,ldf1,ldnew2,ThisLink,ThisQuest,ThisStyle,NewStyle,
                  1,haa,c);   /* Lefthand one */
      e_bl_inner(l,ldnew1,ldf2,ThisLink,ThisQuest,ThisStyle,NewStyle,
                  2,haa,c);    /* Righthand one */
    } else {  /* Make a window */
      if (rollpercent(50)) {   /* left or right */
        e_bl_inner(l,ldf1,ldnew2,ThisLink,ThisQuest,ThisStyle,NewStyle,3,haa,c);
        make_window(l,ldnew1,ldf2,ThisLink,ThisStyle,NewStyle,c);
      } else {
        /* Note: always establish the link before making the window! */
        e_bl_inner(l,ldnew1,ldf2,ThisLink,ThisQuest,ThisStyle,NewStyle,3,haa,c);
        make_window(l,ldf1,ldnew2,ThisLink,ThisStyle,NewStyle,c);
      }
    }  /* end else a window */
    return;
  }

  /* If this isn't supposed to be passable at all, just winowify. */
  /* Another fun thing would be to make the door, but bar it. */
  if (ThisQuest && (ThisQuest->goal == GATE_GOAL)) {
    make_window(l,ldf1,ldf2,ThisLink,ThisStyle,NewStyle,c);
    /* Now arrange for the gate and stuff */
    ThisQuest->tag = new_tag(l);
    ThisQuest->tag2 = new_tag(l);
    farsec->gate = new_gate(l,ThisQuest->tag,ThisQuest->tag2,0,TRUE,c);
    announce(LOG,"BL Gate quest");
    return;  /* and that's it */
  }

  /* Figure out maxtop, for MAX_CEILING */
  maxtop = nearsec->floor_height + ThisLink->height1;
  if (ThisLink->floordelta>0) maxtop += ThisLink->floordelta;

  /* If not the whole wall, center it, or do alcove things, or etc */
  len = linelen(ldf1);   /* Really assumes lens are == */
  if (ThisLink->width1>len) {
    announce(WARNING,"Link-width > linedef size!  Reducing...");
    ThisLink->width1 = len;
  }
  if ((ThisLink->width1!=0)&&(ThisLink->width1<len)) {
    if (ThisLink->bits&LINK_ALCOVE) {
      border = (len - (ThisLink->width1*2 + ThisLink->depth3))/2;
      if (border<0) {   /* This should never happen */
        announce(WARNING,"A-link width too big!  Reducing...");
        ThisLink->width1 = (len-ThisLink->depth3)/2;
        border = 0;
        /* May not actually help enough... */
        /* But can't dealcove; too complicated! */
      }
      if ((border!=0)&&(flipstate==2)&&rollpercent(50)) {
        effective_left ^= LINK_LEFT;
        announce(VERBOSE,"Flipping twinned alcove");
      }
      if (effective_left) {
        if (border!=0) ldf1 = split_linedef(l,ldf1,border,c);
        split_linedef(l,ldf1,ThisLink->width1,c);
        if (border!=0) ldf2 = split_linedef(l,ldf2,border,c);
        split_linedef(l,ldf2,ThisLink->width1,c);
      } else {
        ldf1 = split_linedef(l,ldf1,(len-(border+ThisLink->width1)),c);
        if (border!=0) split_linedef(l,ldf1,ThisLink->width1,c);
        ldf2 = split_linedef(l,ldf2,(len-(border+ThisLink->width1)),c);
        if (border!=0) split_linedef(l,ldf2,ThisLink->width1,c);
      }
    } else if ( (flipstate==1) && (ThisLink->bits&LINK_FAR_TWINS) ) {
      split_linedef(l,ldf1,ThisLink->width1,c);
      ldf2 = split_linedef(l,ldf2,len-ThisLink->width1,c);
      announce(NONE,"Far twins");
    } else if ( (flipstate==2) && (ThisLink->bits&LINK_FAR_TWINS) ) {
      split_linedef(l,ldf2,ThisLink->width1,c);
      ldf1 = split_linedef(l,ldf1,len-ThisLink->width1,c);
    } else {   /* No alcove or farness; simple centered borders */
      if (ThisQuest && ThisQuest->goal==KEY_GOAL && l->skullkeys) {
        ldf1 = borderize(l,ldf1,ThisLink->width1,TRUE,ThisStyle,
                         litecol,find_genus(c,ThisQuest->type),&painted_door,c);
      } else {
        ldf1 = borderize(l,ldf1,ThisLink->width1,TRUE,ThisStyle,
                         litecol,NULL,NULL,c);
      }
      /* Embellish only near side of the link linedef? */
      ldf2 = borderize(l,ldf2,ThisLink->width1,FALSE,NewStyle,
                       litecol,NULL,NULL,c);
    }  /* end else no-alcove case */
  } else {
    if (ThisLink->bits&LINK_ALCOVE)
      announce(WARNING,"ALCOVE with width zero, or width == linelen");
    /* yikes! */
  }

  /* Now we know where the player will be entering the far room, */
  /* more or less.  Record that for the far sector. */
  farsec->entry_x = (ldf2->from->x + ldf2->to->x) / 2;
  farsec->entry_y = (ldf2->from->y + ldf2->to->y) / 2;

  /* Get any tags we need for triggered links, and put them on */
  /* the linedefs we're currently working on.                  */
  /* Trigger alcoved lifts.                                    */
  if ((ThisLink->bits&LINK_LIFT)&&
      (ThisLink->bits&LINK_ALCOVE)&&
      (!(ThisLink->bits&LINK_ANY_DOOR))&&
      (ThisLink->bits&LINK_TRIGGERED)) {
    trigger_lift = TRUE;
    announce(VERBOSE,"Walking lift");
    tag1 = new_tag(l);
    if (ThisLink->floordelta>0) {
      ldf1->tag = tag1;
      ldf1->type = LINEDEF_WR_LOWER_LIFT;
    } else {
      ldf2->tag = tag1;
      ldf2->type = LINEDEF_WR_LOWER_LIFT;
    }
  }
  /* Trigger deeply-recessed liftless doors also */
  if (((ThisQuest==NULL)||
       ((ThisQuest->goal!=SWITCH_GOAL)&&(ThisQuest->goal!=KEY_GOAL)))&&
      (ThisLink->bits&LINK_ANY_DOOR)&&
      (!(ThisLink->bits&LINK_BARS))&&
      (ThisLink->bits&LINK_RECESS)&&
      (ThisLink->depth2>16)&&
      (ThisLink->bits&LINK_TRIGGERED)) {
    trigger_door = TRUE;
    tag1 = new_tag(l);
    /* Don't always need both of these, but couldn't hurt... */
    ldf1->tag = tag1;
    ldf1->type = LINEDEF_WR_OC_DOOR;   /* Or WR_OPEN_DOOR? */
    ldf2->tag = tag1;
    ldf2->type = LINEDEF_WR_OC_DOOR;   /* Or WR_OPEN_DOOR? */
  }
  /* and deeply-recessed lifts */
  if (((ThisQuest==NULL)||
       ((ThisQuest->goal!=SWITCH_GOAL)&&(ThisQuest->goal!=KEY_GOAL)))&&
      (ThisLink->bits&LINK_LIFT)&&
      (ThisLink->bits&LINK_RECESS)&&
      (!(ThisLink->bits&LINK_ALCOVE))&&
      (!(ThisLink->bits&LINK_ANY_DOOR))&&
      (ThisLink->depth2>16)&&
      (ThisLink->bits&LINK_TRIGGERED)) {
    trigger_lift = TRUE;
    tag1 = new_tag(l);
    if (ThisLink->floordelta>0) {
      ldf1->tag = tag1;
      ldf1->type = LINEDEF_WR_LOWER_LIFT;
    } else {
      ldf2->tag = tag1;
      ldf2->type = LINEDEF_WR_LOWER_LIFT;
    }
  }

  /* Remember these for later */
  t1 = ldf1->right->middle_texture;
  t2 = NewStyle->wall0;
  len = linelen(ldf1);   /* Really assumes lens are == */

  /* If recessed, make recess sectors and stuff */
  if (ThisLink->bits&LINK_RECESS) {
    ldnew1 = lefthand_box_ext(l,ldf1,ThisLink->depth2,ThisStyle,c,&ldedge1,&ldedge2);
    /* The near recess copies the near room */
    ldnew1->right->sector->floor_height =
      ldf1->right->sector->floor_height;
    if (ThisLink->bits&LINK_MAX_CEILING) {
      ldnew1->right->sector->ceiling_height = maxtop;
    } else {
      ldnew1->right->sector->ceiling_height =
        ldf1->right->sector->floor_height + ThisLink->height1;
    }
    if (nearsec->style->ceilinglight) if (c->clights) {  /* Too often? */
      ldnew1->right->sector->ceiling_flat = nearsec->style->ceilinglight;
      announce(VERBOSE,"rcl");
    }
    /* Paint key-color, or adjust y-offsets, for recess edges */
    if ((ThisQuest) && (ThisStyle->paint_recesses) && (!painted_door) &&
        (ThisQuest->goal==KEY_GOAL) &&
        (ThisLink->depth2>=
         (texture_for_key(ThisQuest->type,ThisStyle,c)->width))  ) {
      ldedge1->right->middle_texture =
        ldedge2->right->middle_texture =
          texture_for_key(ThisQuest->type,ThisStyle,c);
      if (l->scrolling_keylights) {
        ldedge1->type = LINEDEF_SCROLL;
        ldedge2->type = LINEDEF_SCROLL;
      }
      /* Make sure the paint is visible! */
      if (ldedge1->right->sector->light_level<l->lit_light_level)
        ldedge1->right->sector->light_level=l->lit_light_level;
      announce(VERBOSE,"painted recess");
      if (rollpercent(75)) {
        painted_door = TRUE;
      } else {
        announce(VERBOSE,"Extra-painted recess");   /* Paint the door, too */
        ldedge1->flags |= LOWER_UNPEGGED;       /* and make it all line up */
        ldedge2->flags |= LOWER_UNPEGGED;
      }
    } else if ((ThisLink->bits&LINK_NEAR_DOOR)
                 && (ThisStyle->light_recesses)
                 && (ThisStyle->walllight!=NULL)) {
      announce(VERBOSE,"Lit recess");
      ldedge1->right->middle_texture =
        ldedge2->right->middle_texture =
          ThisStyle->walllight;
      make_lighted(l,ldedge1->right->sector,c);
    } else {
      ldedge2->right->y_offset = ldedge1->right->y_offset =
        nearsec->ceiling_height - ldnew1->right->sector->ceiling_height;
    }
    patch_upper(ldf1,t1,c);
    /* and the far the far */
    ldnew2 = lefthand_box_ext(l,ldf2,ThisLink->depth2,NewStyle,c,&ldedge1,&ldedge2);
    ldnew2->right->sector->floor_height = farsec->floor_height;
    if (ThisLink->bits&LINK_MAX_CEILING) {
      ldnew2->right->sector->ceiling_height = maxtop;
    } else {
      ldnew2->right->sector->ceiling_height = farsec->floor_height +
         ThisLink->height1;
    }
    ldnew2->right->sector->light_level = NewStyle->doorlight0;
    if (farsec->style->ceilinglight) if (c->clights) { /* Too often? */
      ldnew2->right->sector->ceiling_flat = farsec->style->ceilinglight;
      announce(VERBOSE,"rcl");
    }
    if ((ThisLink->bits&LINK_FAR_DOOR)
          && (NewStyle->light_recesses)
          && (NewStyle->walllight!=NULL)) {
      announce(VERBOSE,"Lit recess");
      ldedge1->right->middle_texture =
        ldedge2->right->middle_texture =
          NewStyle->walllight;
      make_lighted(l,ldedge1->right->sector,c);
    } else {
      ldedge2->right->y_offset = ldedge1->right->y_offset =
        farsec->ceiling_height - ldnew2->right->sector->ceiling_height;
    }
    patch_upper(ldf2,t2,c);
    /* Now we're working inside the recesses */
    ldf1 = ldnew1;
    ldf2 = ldnew2;
  }

  /* If no core or alcoves, make the one arch/door sector */
  if (!(ThisLink->bits&(LINK_CORE|LINK_ALCOVE))) {
    flip_linedef(ldf2);
    newsec = make_box_ext(l,ldf1,ldf2,ThisStyle,c,&ldnew1,&ldnew2);
    flip_linedef(ldf2);
    ldnew2->right->y_offset = ldnew1->right->y_offset =
      (nearsec->ceiling_height - nearsec->floor_height) - ThisLink->height1;
    if ((ThisLink->bits&(LINK_ANY_DOOR)) || c->doorless_jambs) {
      ldnew1->right->middle_texture = ThisStyle->doorjamb;
      ldnew2->right->middle_texture = ThisStyle->doorjamb;
    }

    newsec->floor_height = nearsec->floor_height;
    if (ThisLink->bits&LINK_MAX_CEILING) {
      newsec->ceiling_height = maxtop;
    } else {
#ifdef STRANGE_THINGS
    newsec->ceiling_height = nearsec->ceiling_height;
#else
    newsec->ceiling_height = newsec->floor_height + ThisLink->height1;
#endif
    }
    newsec->light_level = ThisStyle->doorlight0;
    newsec->style = ThisStyle;  /* Is this right? */

    /* This is where the level-change actually happens */
    patch_upper(ldf1,t1,c);
    patch_upper(ldf2,t2,c);
    patch_lower(ldf1,ThisStyle->kickplate,c);   /* or stepfronts? */
    patch_lower(ldf2,NewStyle->kickplate,c);

    ldf1->flags |= TWO_SIDED;
    ldf2->flags |= TWO_SIDED;

  }

  /* If no core, and a door, doorify the middle sector (s) */
  if ( (!(ThisLink->bits&(LINK_CORE|LINK_ALCOVE))) && (ThisLink->bits&LINK_ANY_DOOR) ) {
    if (!(ThisLink->bits&LINK_BARS)) {
      doorify(newsec,ldf1,ldf2,ThisStyle,NewStyle,c);
      if (trigger_door) {
        ldf1->type = LINEDEF_NORMAL;
        ldf2->type = (c->do_dm) ? LINEDEF_NORMAL_S1_DOOR : LINEDEF_NORMAL;
        newsec->tag = tag1;
      }
      if (!ThisStyle->moving_jambs) {
        ldnew1->flags |= LOWER_UNPEGGED;
        ldnew2->flags |= LOWER_UNPEGGED;
      }
      if (ThisQuest) {
        if (ThisQuest->goal==KEY_GOAL) {
          ldf1->type = type_for_key(ThisQuest->type);
          if (!painted_door)
            mark_door_for_key(l,ldf1,ThisQuest->type,ThisStyle,c);
          ldf2->type = type_for_key(ThisQuest->type);   /* Prevent monsters! */
        } else if (ThisQuest->goal==SWITCH_GOAL && !(ThisLink->bits&LINK_LOCK_CORE)) {
          ldf1->type = LINEDEF_NORMAL;
          ldf2->type = (c->do_dm) ? LINEDEF_NORMAL_S1_DOOR : LINEDEF_NORMAL;
          newsec->tag = ThisQuest->tag;
          mark_door_for_lock(l,ldf1,ThisStyle,c);
        }  /* end else if tag goal */
      }  /* end if ThisQuest */
    } else {
      announce(VERBOSE,"Barred door");
      if (ThisLink->bits&LINK_LOCK_CORE)
        barify(l,ldf1,ldf2,NULL,16*l->hugeness,NULL,ThisStyle,c);
          else barify(l,ldf1,ldf2,ThisQuest,16*l->hugeness,NULL,ThisStyle,c);
    }  /* end else barred door */
  }

  /* If a core, and door(s), need to make the door sector(s) */
  if ( ((ThisLink->bits&LINK_CORE)) && (ThisLink->bits&LINK_ANY_DOOR) ) {
    if (ThisLink->bits&LINK_NEAR_DOOR) {
      ldnew1 = lefthand_box_ext(l,ldf1,ThisLink->depth1,ThisStyle,c,&ldedge1,&ldedge2);
      ldedge1->right->middle_texture = ThisStyle->doorjamb;
      ldedge2->right->middle_texture = ThisStyle->doorjamb;
      if (!ThisStyle->moving_jambs) {
        ldedge1->flags |= LOWER_UNPEGGED;
        ldedge2->flags |= LOWER_UNPEGGED;
      }
      /* Does the y offset of the doorjambs actually matter? */
      ldedge2->right->y_offset = ldedge1->right->y_offset =
        (nearsec->ceiling_height - nearsec->floor_height) - ThisLink->height1;
    }
    if (ThisLink->bits&LINK_FAR_DOOR) {
      ldnew2 = lefthand_box_ext(l,ldf2,ThisLink->depth1,NewStyle,c,&ldedge1,&ldedge2);
      ldedge1->right->middle_texture = NewStyle->doorjamb;
      ldedge2->right->middle_texture = NewStyle->doorjamb;
      if (!NewStyle->moving_jambs) {
        ldedge1->flags |= LOWER_UNPEGGED;
        ldedge2->flags |= LOWER_UNPEGGED;
      }
      ldedge2->right->y_offset = ldedge1->right->y_offset =
        (farsec->ceiling_height - farsec->floor_height) - ThisLink->height1;
    }
    /* Now we're working on the other sides of the doors. */
    /* But we can't really doorify them yet, since they have */
    /* no far sectors, and their linedefs need to be flipped */
    /* for the rest of the alg to work.  So record fixup info */
    /* for later. */
    need_to_doorify = TRUE;  /* Need to flip when done */
    if (ThisLink->bits&LINK_NEAR_DOOR) {
      ldflip1a = ldf1;
      ldflip1b = ldnew1;
      sflip1 = ldf1->left->sector;
    }
    if (ThisLink->bits&LINK_FAR_DOOR) {
      ldflip2a = ldf2;
      ldflip2b = ldnew2;
      sflip2 = ldf2->left->sector;
      /* Might as well fix the light-level on the far door while we're here */
      sflip2->light_level = NewStyle->doorlight0;
    }
    /* Now we're working on the far sides of the (future) door(s) */
    if (ThisLink->bits&LINK_NEAR_DOOR) ldf1 = ldnew1;
    if (ThisLink->bits&LINK_FAR_DOOR) ldf2 = ldnew2;
  }

  /* If alcoves, make them now, and take new linedefs */
  if (ThisLink->bits&LINK_ALCOVE) {
    linedef *ldedgeopen, *ldedgeclosed;
    announce(VERBOSE,"Making alcoves");
    ldnew1 = lefthand_box_ext(l,ldf1,ThisLink->width2,ThisStyle,c,&ldedge1,&ldedge2);
    if (effective_left) {
      ldedgeopen = ldedge2;
      ldedgeclosed = ldedge1;
    } else {
      ldedgeopen = ldedge1;
      ldedgeclosed = ldedge2;
    }
    /* The near alcove copies the near room */
    ldnew1->right->middle_texture = ldedgeopen->right->middle_texture;
    ldedgeopen->right->middle_texture = c->null_texture;
    ldedgeopen->flags |= TWO_SIDED;
    ldnew1->right->sector->floor_height =
      ldf1->right->sector->floor_height;
    if (ThisLink->bits&LINK_MAX_CEILING) {
      ldnew1->right->sector->ceiling_height = maxtop;
    } else {
      ldnew1->right->sector->ceiling_height =
        ldf1->right->sector->floor_height + ThisLink->height1;
    }
    ldnew1->right->y_offset = ldedgeclosed->right->y_offset =
      nearsec->ceiling_height - ldnew1->right->sector->ceiling_height;
    patch_upper(ldf1,t1,c);
    ldf1 = ldedgeopen;
    ldnew2 = lefthand_box_ext(l,ldf2,ThisLink->width2,NewStyle,c,&ldedge1,&ldedge2);
    if (effective_left) {
      ldedgeopen = ldedge2;
      ldedgeclosed = ldedge1;
    } else {
      ldedgeopen = ldedge1;
      ldedgeclosed = ldedge2;
    }
    /* and the far the far */
    ldnew2->right->middle_texture = ldedgeopen->right->middle_texture;
    ldedgeopen->right->middle_texture = c->null_texture;
    ldedgeopen->flags |= TWO_SIDED;
    ldnew2->right->sector->floor_height = farsec->floor_height;
    if (ThisLink->bits&LINK_MAX_CEILING) {
      ldnew2->right->sector->ceiling_height = maxtop;
    } else {
      ldnew2->right->sector->ceiling_height = farsec->floor_height +
         ThisLink->height1;
    }
    ldnew2->right->sector->light_level = NewStyle->roomlight0;
    ldedgeclosed->right->y_offset = ldnew2->right->y_offset =
      farsec->ceiling_height - ldnew2->right->sector->ceiling_height;
    patch_upper(ldf2,t2,c);
    ldf2 = ldedgeopen;
    /* Now we're working out from the alcoves */
    len = ThisLink->width2;
  }

  /* Record the area to maybe put a monster in */
  mminx = ldf1->from->x;
  if (ldf1->to->x<mminx) mminx = ldf1->to->x;
  if (ldf2->from->x<mminx) mminx = ldf2->from->x;
  if (ldf2->to->x<mminx) mminx = ldf2->to->x;
  mminy = ldf1->from->y;
  if (ldf1->to->y<mminy) mminy = ldf1->to->y;
  if (ldf2->from->y<mminy) mminy = ldf2->from->y;
  if (ldf2->to->y<mminy) mminy = ldf2->to->y;
  mmaxx = ldf1->from->x;
  if (ldf1->to->x>mmaxx) mmaxx = ldf1->to->x;
  if (ldf2->from->x>mmaxx) mmaxx = ldf2->from->x;
  if (ldf2->to->x>mmaxx) mmaxx = ldf2->to->x;
  mmaxy = ldf1->from->y;
  if (ldf1->to->y>mmaxy) mmaxy = ldf1->to->y;
  if (ldf2->from->y>mmaxy) mmaxy = ldf2->from->y;
  if (ldf2->to->y>mmaxy) mmaxy = ldf2->to->y;
  /* and the facing */
  mangle = facing_right_from_ld(ldf1);

  /* If the core is stairs, put in all but the last */
  if (ThisLink->bits&LINK_STEPS) {
    int i, depth, stepdelta, x, y;
    texture *front = ThisStyle->kickplate;
    genus *g = ThisStyle->lamp0;
    boolean add_lamps = FALSE;
    if (g->height>ThisLink->height1) g = ThisStyle->shortlamp0;
    depth = ThisLink->depth3 / (ThisLink->stepcount+1);
    if ((ThisLink->bits&LINK_LAMPS) &&
        (g->width<=depth) && (g->width*2+64<=len)) {
      add_lamps = TRUE;
      announce(VERBOSE,"stair lamps");
    }
    stepdelta = ThisLink->floordelta / ThisLink->stepcount;
    if (ThisStyle->light_steps && ThisStyle->walllight) {
      front = ThisStyle->walllight;
    } else if (ThisStyle->stepfront &&
               (ThisStyle->stepfront->height>=abs(stepdelta))) {
      front = ThisStyle->stepfront;
    }
    /* Looks just like the recess stuff, mostly */
    for (i=0;i<ThisLink->stepcount;i++) {
      ldnew1 = lefthand_box_ext(l,ldf1,depth,ThisStyle,c,&ldedge1,&ldedge2);
      if ((add_lamps)&&(i&0x01)) {
        point_from(ldedge1->from->x,ldedge1->from->y,ldedge1->to->x,
          ldedge1->to->y,RIGHT_TURN,g->width/2,&x,&y);
        point_from(ldedge1->to->x,ldedge1->to->y,x,y,
          RIGHT_TURN,depth/2,&x,&y);
        new_thing(l,x,y,0,g->thingid,7,c);
        point_from(ldedge2->from->x,ldedge2->from->y,ldedge2->to->x,
          ldedge2->to->y,RIGHT_TURN,g->width/2,&x,&y);
        point_from(ldedge2->to->x,ldedge2->to->y,x,y,
          RIGHT_TURN,depth/2,&x,&y);
        new_thing(l,x,y,0,g->thingid,7,c);
      }
      ldnew1->right->sector->floor_height =
        ldf1->right->sector->floor_height + stepdelta;
      if (ThisLink->bits&LINK_MAX_CEILING)
        ldnew1->right->sector->ceiling_height = maxtop;
        else ldnew1->right->sector->ceiling_height =
               ldnew1->right->sector->floor_height + ThisLink->height1;
      ldnew1->right->sector->floor_flat = ThisStyle->stepfloor;
      ldedge1->right->y_offset =
      ldedge2->right->y_offset =
        nearsec->ceiling_height - ldedge1->right->sector->ceiling_height;
      patch_upper(ldf1,t1,c);
      patch_lower(ldf1,front,c);
      ldf1->flags &= ~LOWER_UNPEGGED;
      ldf1 = ldnew1;
    } /* end for each step */
    /* The core-making step will do the top (bottom) (far) step */
  } /* end if each steps */

  /* If a core, need to make it */
  if (ThisLink->bits&LINK_CORE) {
    flip_linedef(ldf2);
    newsec = make_box_ext(l,ldf1,ldf2,ThisStyle,c,&ldedge1,&ldedge2);
    flip_linedef(ldf2);
    if (ThisLink->bits&LINK_MAX_CEILING) newsec->ceiling_height = maxtop;
    if (newsec->ceiling_height - ldf1->right->sector->floor_height < 64)
      newsec->ceiling_height = ldf1->right->sector->floor_height + 64;
    if (newsec->ceiling_height - ldf2->right->sector->floor_height < 64)
      newsec->ceiling_height = ldf2->right->sector->floor_height + 64;
    ldedge2->right->y_offset = ldedge1->right->y_offset =
      nearsec->ceiling_height - newsec->ceiling_height;
    if ((ThisQuest!=NULL)&&(ThisLink->bits&LINK_LOCK_CORE)) {
      newsec->floor_flat = ThisStyle->nukage1;
      newsec->special = NUKAGE1_SPECIAL;
      newsec->tag = ThisQuest->tag;
      newsec->floor_height -= 24 - roll(ThisLink->floordelta);
      if (newsec->light_level<160) newsec->light_level = 160;  /* Visible */
      patch_lower(ldf1,ThisStyle->kickplate,c);  /* Or stepfront? */
      if (rollpercent(50)) ldf2->flags |= BLOCK_MONSTERS;  /* Can't decide! */
      haa->haas[ITYTD].health -= 10;
      haa->haas[HMP].health -= 5;
      announce(VERBOSE,"Nukage lock");
    } else if (rollpercent(l->p_force_nukage)&&!(ThisLink->bits&LINK_LIFT)
               &&!(ThisLink->bits&LINK_STEPS)&&(ThisLink->depth3>=64)
               &&(ThisLink->depth3<=196)) {
      newsec->floor_flat = ThisStyle->nukage1;
      newsec->special = NUKAGE1_SPECIAL;
      if (ThisLink->floordelta<0) {
        newsec->floor_height += ThisLink->floordelta;  /* Fixed +- bug here */
        newsec->floor_height -= roll(25+ThisLink->floordelta);
      } else {
        newsec->floor_height -= roll(25-ThisLink->floordelta);
      }
      if (newsec->light_level<160) newsec->light_level = 160;  /* Visible */
      patch_lower(ldf1,ThisStyle->kickplate,c);
      haa->haas[ITYTD].health -= 10;
      haa->haas[HMP].health -= 5;
      announce(VERBOSE,"Nukage link");
    } else if ((rollpercent(l->p_falling_core))&&
               (linelen(ldedge1)>=(120*l->hugeness))&&
               (!(ThisLink->bits&LINK_LIFT))&&
               (flipstate==0) ) {             /* Maybe a fun trap */
      try_falling_core(l,ldedge1,ldedge2,haa,c);
    }
    ldf2->flags |= TWO_SIDED;
    patch_upper(ldf1,t1,c);

    /* This is where the level-change actually happens, eh? */
    patch_upper(ldf2,NewStyle->wall0,c);
    patch_lower(ldf2,NewStyle->kickplate,c);  /* or stepfront? */

    /* On the first pass, sometimes save the righthand wall of the */
    /* core, so that the second pass can make a decorative room. */
    if (flipstate==1) {
      if ( (!(ThisLink->bits&(LINK_LIFT|LINK_ALCOVE)))
           && (ThisLink->bits&LINK_DECROOM)
           && (linelen(ldedge2)>63) ) {
        ThisLink->cld = ldedge2;
      } else {
        ThisLink->cld = NULL;
      }
    }
    /* On the second pass, if we did that, make the room */
    if ((flipstate==2)&&ThisLink->cld) {
      if (rollpercent(10)) {
        if (make_window(l,ldedge1,ThisLink->cld,ThisLink,ThisStyle,NewStyle,c))
          announce(LOG,"Intertwin window");
      } else {
        if (make_decroom(l,ldedge1,ThisLink->cld,c))
          announce(LOG,"Intertwin decroom");
      }
    }

  }  /* end if a core */

  /* If the core is a lift, make that happen */
  if (ThisLink->bits&LINK_LIFT) {
    if (trigger_lift) newsec->tag = tag1;
      else newsec->tag = new_tag(l);
    newsec->ceiling_flat = ThisStyle->doorceiling;  /* really? */
    newsec->floor_flat = ThisStyle->doorfloor;  /* really? */
    ldf1->tag = newsec->tag;
    ldf2->tag = newsec->tag;
    if (nearsec->floor_height>farsec->floor_height) {
      newsec->floor_height = nearsec->floor_height;
      ldf1->type = LINEDEF_WR_LOWER_LIFT;
      ldf1->left->lower_texture = ThisStyle->support0;   /* yes? */
      if (!trigger_lift) ldf2->type = NewStyle->slifttype;
      patch_lower(ldf2,NewStyle->support0,c);
      if ( (NewStyle->liftface) &&
           (nearsec->floor_height - farsec->floor_height <=
            NewStyle->liftface->height) &&
           (linelen(ldf2)==NewStyle->liftface->width) ) {
        ldf2->right->lower_texture = NewStyle->liftface;
        ldf2->right->x_offset = 0;
        announce(VERBOSE,"Lift texture");
      }
      ldf2->flags &= ~LOWER_UNPEGGED;    /* Lift-falling must be visible! */
    } else {
      newsec->floor_height = farsec->floor_height;
      if (!trigger_lift) ldf1->type = ThisStyle->slifttype;
      patch_lower(ldf1,ThisStyle->support0,c);
      if ( (ThisStyle->liftface) &&
           (farsec->floor_height - nearsec->floor_height <=
            ThisStyle->liftface->height) &&
           (linelen(ldf1)==ThisStyle->liftface->width) ) {
        ldf1->right->lower_texture = ThisStyle->liftface;
        ldf1->right->x_offset = 0;
        announce(VERBOSE,"Lift texture");
      }
      ldf1->flags &= ~LOWER_UNPEGGED;    /* Lift-falling must be visible! */
      ldf2->type = LINEDEF_WR_LOWER_LIFT;
      ldf2->left->lower_texture = NewStyle->support0;   /* right? */
    }  /* end else lift that way */
    newsec->ceiling_height = newsec->floor_height + ThisLink->height1;
    /* and re-figure the y offsets in the core */
    ldedge2->right->y_offset = ldedge1->right->y_offset =
      nearsec->ceiling_height - newsec->ceiling_height;
    patch_upper(ldf1,ThisStyle->wall0,c);
    patch_upper(ldf2,NewStyle->wall0,c);
  }  /* end if doing a lift */

  /* Maybe put a monster in the link */
  if ((haa!=NULL)&&(ThisLink->bits&LINK_CORE)&&(rollpercent(40))) {
    int levels;
    genus *m = timely_monster(haa,c,&levels,rollpercent(l->p_biggest_monsters),0);
    if (m) {
      if (rollpercent(15)) levels |= 0x08;   /* deaf */
      /* Try to place it */
      if (!rollpercent(l->p_rational_facing)) mangle = 90 * roll(4);
      if (NULL!=place_object_in_region(l,mminx,mminy,mmaxx,mmaxy,
                 c,m->thingid,MONSTER_WIDTH(m),mangle,0,0,levels)) {

        if (m->thingid == ID_SKULL) announce(NONE,"Skull");
        if (m->thingid == ID_HEAD) announce(VERBOSE,"HEAD");
        if (m->thingid == ID_SKEL) announce(VERBOSE,"SKEL");
        if (m->thingid == ID_HELL) announce(VERBOSE,"KNIGHT");
        if (m->thingid == ID_ARCHIE) announce(VERBOSE,"VILE");

        update_haa_for_monster(haa,m,levels,0,c);
        haa_unpend(haa);

        announce(VERBOSE,"Link guard");
      }  /* end if placed it */
    }  /* end if timely one */
  }  /* end if a monster */

  /* Finally, if we have unmade doors, make 'em */
  if (need_to_doorify) {
    if (ThisLink->bits&LINK_NEAR_DOOR) {
      flip_linedef(ldflip1b);
      doorify(sflip1,ldflip1a,ldflip1b,ThisStyle,ThisStyle,c);
      if (trigger_door) {
        ldflip1a->type = LINEDEF_NORMAL;
        ldflip1b->type = LINEDEF_NORMAL_DOOR;  /* S1 door would break! */
        sflip1->tag = tag1;
      }
      if (ThisQuest) {
        if (ThisQuest->goal==KEY_GOAL) {
          ldflip1a->type = type_for_key(ThisQuest->type);
          if (!painted_door)
            mark_door_for_key(l,ldflip1a,ThisQuest->type,ThisStyle,c);
          ldflip1b->type = type_for_key(ThisQuest->type);  /* Prevent monsters! */
        } else if ((ThisQuest->goal==SWITCH_GOAL) && !(ThisLink->bits&LINK_LOCK_CORE)) {
          ldflip1a->type = LINEDEF_NORMAL;
          ldflip1b->type = (c->do_dm) ? LINEDEF_NORMAL_S1_DOOR : LINEDEF_NORMAL;
          sflip1->tag = ThisQuest->tag;
          mark_door_for_lock(l,ldflip1a,ThisStyle,c);
        }  /* end else if tag goal */
      }  /* end else if ThisQuest */
    }
    if (ThisLink->bits&LINK_FAR_DOOR) {
      flip_linedef(ldflip2b);
      doorify(sflip2,ldflip2a,ldflip2b,NewStyle,NewStyle,c);
      if (trigger_door) {
        ldflip2a->type = LINEDEF_NORMAL;
        ldflip2b->type = LINEDEF_NORMAL_DOOR;  /* S1 door would break! */
        sflip2->tag = tag1;
      }
    }
  }  /* end if need to doorify */

  /* Whew! */

}  /* end e_bl_inner() */

/* Return a random linedef in the plain; no sidedefs or anything. */
linedef *starting_linedef(level *l,style *ThisStyle,config *c)
{
  vertex *v1,*v2;

  v1 = new_vertex(l,0,0);  /* Might as well */
  /* Should consult the style/config? */
  v2 = new_vertex(l,0,l->hugeness*64*(2+roll(9)));
  return new_linedef(l,v1,v2);
}

/* Given a ray from the given point to the given point, turn in */
/* the given direction and go the given distance.  Return the   */
/* point that we end up at.                                     */
void point_from(int x1, int y1, int x2, int y2, int angle, int len,
                int *x3, int *y3)
{
  /* Stub; right angles and axis-parallel lines only */
  int newdx, newdy;

  if (x1==x2) {  /* Parallel to the Y-axis */
    newdy = 0;
    if (y2>y1) {  /* Up */
      newdx = 1;
    } else {      /* Down */
      newdx = -1;
    }
  } else {     /* to the X-axis */
    newdx = 0;
    if (x2>x1) {  /* rightward */
      newdy = -1;
    } else {
      newdy = 1;
    }
  }
  if (angle==LEFT_TURN) {
    newdx = 0 - newdx;
    newdy = 0 - newdy;
  }
  *x3 = x2 + len * newdx;
  *y3 = y2 + len * newdy;
  return;
}

/* Print, log, whatever.  Really ought to support real log files. */
void announce(int announcelevel, char *s)
{
  switch (announcelevel) {
    case NONE: return;
    case VERBOSE: if (global_verbosity==0) return; /** fallthrough **/
    case LOG: break;
    case NOTE: printf("NOTE: "); break;
    case WARNING: printf("WARNING: "); break;
    case ERROR: printf("ERROR: "); break;
    default: printf("HEY: "); break;
  }
  printf("%s\n",s);
}

/* Install a switch on the given linedef.  If the actual */
/* switch linedef changes (due to recessing, say), return */
/* that new linedef, else the old one.  Doesn't set the */
/* type or tag, just the texture and stuff.   If xld is */
/* given, it gets the two-sided center part of ld. */
linedef *install_switch(level *l,linedef *ld,boolean recess,boolean fancy,
                        short key, style *ThisStyle,config *c,linedef **xld)
{
  linedef *ld2, *ldedge1, *ldedge2;
  int rdepth = 8;
  texture *tx = NULL;

  if (fancy) {
    ThisStyle->lightboxes = TRUE;
    announce(VERBOSE,"fancy switch");
  }
  ld = borderize(l,ld,64,TRUE,ThisStyle,LIGHT,NULL,NULL,c);
  if (xld) *xld = ld;
  ThisStyle->lightboxes = FALSE;
  if (recess)
    if (key) {
      tx = texture_for_key(key,ThisStyle,c);
      rdepth = tx->width;
      if (rdepth>8)
       if (!empty_left_side(l,ld,rdepth)) rdepth=8;
    }
    if (empty_left_side(l,ld,rdepth)) {
      texture *t1 = ld->right->middle_texture;
      ld2 = lefthand_box_ext(l,ld,rdepth,ThisStyle,c,&ldedge1,&ldedge2);
      ld2->right->sector->ceiling_height =
        ld2->right->sector->floor_height + 72;  /* Should vary? */
      /* Maybe paint/light the recesses */
      if (key) {
        ldedge2->right->middle_texture = ldedge1->right->middle_texture = tx;
      } else if ((ThisStyle->light_recesses)&&(ThisStyle->walllight!=NULL)) {
        announce(VERBOSE,"Lit switch");
        ldedge2->right->middle_texture = ldedge1->right->middle_texture =
          ThisStyle->walllight;
        make_lighted(l,ld2->right->sector,c);
      } else {
        ldedge2->right->y_offset = ldedge1->right->y_offset =
          (ld->right->sector->ceiling_height -
           ld->right->sector->floor_height) - 72;
      }
      patch_upper(ld,t1,c);
      ld = ld2;
    }
  ld->right->middle_texture = ThisStyle->switch0;
  ld->right->x_offset = 0;
  ld->right->y_offset = ThisStyle->switch0->y_bias;
  ld->flags |= LOWER_UNPEGGED;
  return ld;
}

/* Perhaps add a deathmatch start to this sector, if it doesn't */
/* already have one, and we're doing deathmatch stuff. */
boolean maybe_add_dm_start(level *l, sector *s, config *c, boolean force) {

  if (!c->do_dm) return FALSE;
  if (s->has_dm && !force) return FALSE;
  if (place_object(l,s,c,ID_DM,34,-1,s->entry_x,s->entry_y,7)) {
    s->has_dm = TRUE;
    l->dm_count++;
    if (!s->has_dm_weapon)
      if (place_object(l,s,c,ID_SHOTGUN,24,0,0,0,0x17))
        s->has_dm_weapon = TRUE;
    return TRUE;
  }
  return FALSE;
}

/* Does anything involved with closing a quest that has to happen */
/* -after- the room is populated and embellished and stuff.       */
void close_quest_final(level *l, sector *s, quest *q, haa *haa, config *c)
{
  thing *t = q->thing;

  l->goal_room = s;

  maybe_add_dm_start(l,s,c,FALSE);

  if ( t && (q->auxtag) && (q->surprise)) {
    trigger_box(l,t,s,q->auxtag,LINEDEF_WR_OPEN_DOOR,c);
    populate_linedef(l,q->surprise,haa,c,FALSE);
  }

  /* If we've put in the SL exit, but not yet the thing that */
  /* opens it, do one last try at that. */
  if ((q->goal==LEVEL_END_GOAL) && (l->sl_open_ok)) {
    t = place_required_small_pickable(l,s,c);
    if (t) {
      trigger_box(l,t,s,l->sl_tag,l->sl_type,c);
      l->sl_done = TRUE;
      l->sl_open_ok = FALSE;
      announce(VERBOSE,"Did sl triggerbox");
    }
  }

  /* On the other hand, if we haven't even put in the SL exit yet, */
  /* we're really desparate! */
  if (need_secret_level(c) && !l->sl_done &&!l->sl_tag &&
      q->goal==LEVEL_END_GOAL) {
    int i;
    linedef *ldf;
    i = mark_decent_boundary_linedefs(l,s,32);
    ldf = random_marked_linedef(l,i);
    unmark_linedefs(l);
    if (i) {
      if (ldf->right->middle_texture->subtle)
        ldf->right->middle_texture = ldf->right->middle_texture->subtle;
        else ldf->right->middle_texture = s->style->support0;
      ldf->type = LINEDEF_S1_SEC_LEVEL;
      announce(LOG,"Last-ditch SL exit!");
      l->sl_done = TRUE;
    }
  }
}

/* Make one of them instant-death rooms, like at the end */
/* of E1M8, using the linedef in the linkto(), and the */
/* style for the walls 'n' stuff. */
short death_room(level *l,linedef *ld,style *ThisStyle,config *c)
{
  linedef *ldnew;
  link *gatelink = gate_link(l,c);
  sector *newsector;
  int minx,miny,maxx,maxy;
  int x;

  ldnew = make_linkto(l,ld,gatelink,ThisStyle,c,NULL);
  if (ldnew==NULL) return 0;
  for (;linelen(ldnew)<320;) {
    ldnew->to->x = ldnew->from->x + 2 *
      (ldnew->to->x - ldnew->from->x);
    ldnew->to->y = ldnew->from->y + 2 *
      (ldnew->to->y - ldnew->from->y);
  }
  newsector = generate_room_outline(l,ldnew,ThisStyle,FALSE,c);
  newsector->style = ThisStyle;
  paint_room(l,newsector,ThisStyle,c);
  newsector->tag = new_tag(l);
  newsector->special = DEATH_SECTOR;
  newsector->light_level = 80;

  find_rec(l,newsector,&minx,&miny,&maxx,&maxy);
  new_thing(l,(minx+maxx)/2,(miny+maxy)/2,(short)(90*roll(4)),ID_GATEOUT,7,c);

  /* Worry about having *too many* sergeants? */
  for (x=minx+22; x<=maxx-22; x+=44) {
    new_thing(l,x,miny+22,90,ID_SERGEANT,7,c);
    new_thing(l,x,maxy-22,270,ID_SERGEANT,7,c);
  }

  return newsector->tag;

}

/* Simple trial implementation: just an "EXIT" gate to */
/* an e1m8 instant-death room. */
boolean e1m8_gate(level *l,linedef *ld,sector *s,haa *haa,config *c)
{
  short tag = death_room(l,ld,s->style,c);

  if (tag==0) return FALSE;
  s->gate = new_gate(l,0,tag,0,FALSE,c);
  install_gate(l,s,s->style,haa,TRUE,c);
  s->middle_enhanced = TRUE;
  gate_populate(l,s,haa,FALSE,c);  /* safe/correct? */
  return TRUE;

}

/* Put down zero or more of the required powerups in the */
/* arena's prep-room. */
void prepare_arena_gate(level *l,sector *s,arena *a, haa *haa,config *c)
{
  /*STUB!*/
}

/* Actually put down the main linedefs and sectors */
/* and stuff for the arena. */
void install_arena(level *l,arena *a,sector *s,haa *haa,config *c)
{
  int maxx = 0-HUGE_NUMBER;
  vertex *v, *v1, *v2, *v3, *v4;
  vertex *vt1, *vt2;
  int upness, acrossness, border, i, n;
  sector *newsec;
  linedef *ld;
  short ch;
  genus *lamp;

  ch = 128;   /* Too simple */
  newsec = new_sector(l,0,ch,a->floor,c->sky_flat);
  newsec->light_level = c->minlight+roll(100);
  newsec->style = s->style;
  a->outersec = newsec;

  for (v=l->vertex_anchor;v;v=v->next) if (v->x > maxx) maxx = v->x;
  maxx += 256;
  upness = 750 + roll(501);
  acrossness = 3*upness;
  if (a->props&ARENA_PORCH) {
    border = 72 + 32 * roll(11);
  } else {
    border = 50 + roll(200);
  }
  maxx += border+16;
  a->minx = maxx;
  a->maxx = maxx + acrossness;
  a->miny = 0-upness/2;
  a->maxy = upness/2;

  /* Make the outer walls */
  if (a->props&ARENA_PORCH) {
    /* Flat with promenade */
    newsec->ceiling_flat = newsec->style->ceiling0;
    v1 = new_vertex(l,a->minx-(16+border/2),a->miny-(16+border/2));
    v2 = new_vertex(l,a->minx-(16+border/2),a->maxy+(16+border/2));
    v3 = new_vertex(l,a->maxx+(16+border/2),a->maxy+(16+border/2));
    v4 = new_vertex(l,a->maxx+(16+border/2),a->miny-(16+border/2));

    ld = new_linedef(l,v1,v2);
    ld->right = new_sidedef(l,newsec,c);
    ld->right->middle_texture = a->walls;

    ld = new_linedef(l,v2,v3);
    ld->right = new_sidedef(l,newsec,c);
    ld->right->middle_texture = a->walls;

    ld = new_linedef(l,v3,v4);
    ld->right = new_sidedef(l,newsec,c);
    ld->right->middle_texture = a->walls;

    ld = new_linedef(l,v4,v1);
    ld->right = new_sidedef(l,newsec,c);
    ld->right->middle_texture = a->walls;

    if (a->props&ARENA_LAMPS) {
      lamp = newsec->style->lamp0;
      if (lamp->height >= ch) lamp = newsec->style->shortlamp0;
      new_thing(l,v1->x+64,v1->y+64,0,lamp->thingid,7,c);
      new_thing(l,v2->x+64,v2->y-64,0,lamp->thingid,7,c);
      new_thing(l,v3->x-64,v3->y-64,0,lamp->thingid,7,c);
      new_thing(l,v4->x-64,v4->y+64,0,lamp->thingid,7,c);
    }

  } else {
    /* or irregular */
    v1 = new_vertex(l,a->minx-(16+roll(border/2)),a->miny-(16+roll(border/2)));
    v2 = new_vertex(l,a->minx-(16+roll(border/2)),a->maxy+(16+roll(border/2)));
    v3 = new_vertex(l,a->maxx+(16+roll(border/2)),a->maxy+(16+roll(border/2)));
    v4 = new_vertex(l,a->maxx+(16+roll(border/2)),a->miny-(16+roll(border/2)));
    /* left north-south wallset */
    n = 1 + roll(10);  vt1 = v1;
    for (i=1;i<n;i++) {
      vt2 = new_vertex(l,a->minx-(16+roll(border)),a->miny+i*(upness/(n+1)));
      ld = new_linedef(l,vt1,vt2);
      ld->right = new_sidedef(l,newsec,c);
      ld->right->middle_texture = a->walls;
      vt1 = vt2;
    }
    ld = new_linedef(l,vt1,v2);
    ld->right = new_sidedef(l,newsec,c);
    ld->right->middle_texture = a->walls;
    /* top left-right wallset */
    n = 1 + roll(10);  vt1 = v2;
    for (i=1;i<n;i++) {
      vt2 = new_vertex(l,a->minx+i*(acrossness/(n+1)),a->maxy+(16+roll(border)));
      ld = new_linedef(l,vt1,vt2);
      ld->right = new_sidedef(l,newsec,c);
      ld->right->middle_texture = a->walls;
      vt1 = vt2;
    }
    ld = new_linedef(l,vt1,v3);
    ld->right = new_sidedef(l,newsec,c);
    ld->right->middle_texture = a->walls;
    /* right south-north wallset */
    n = 1 + roll(10);  vt1 = v3;
    for (i=1;i<n;i++) {
      vt2 = new_vertex(l,a->maxx+(16+roll(border)),a->maxy-i*(upness/(n+1)));
      ld = new_linedef(l,vt1,vt2);
      ld->right = new_sidedef(l,newsec,c);
      ld->right->middle_texture = a->walls;
      vt1 = vt2;
    }
    ld = new_linedef(l,vt1,v4);
    ld->right = new_sidedef(l,newsec,c);
    ld->right->middle_texture = a->walls;
    /* bottom right-left wallset */
    n = 1 + roll(10);  vt1 = v4;
    for (i=1;i<n;i++) {
      vt2 = new_vertex(l,a->maxx-i*(acrossness/(n+1)),a->miny-(16+roll(border)));
      ld = new_linedef(l,vt1,vt2);
      ld->right = new_sidedef(l,newsec,c);
      ld->right->middle_texture = a->walls;
      vt1 = vt2;
    }
    ld = new_linedef(l,vt1,v1);
    ld->right = new_sidedef(l,newsec,c);
    ld->right->middle_texture = a->walls;
  }

  /* Now the inner sector */

  ch = 256 + 64 * roll(3);  /* Too simple? */
  newsec = new_sector(l,0,ch,a->floor,c->sky_flat);
  newsec->light_level = a->outersec->light_level;
  newsec->style = s->style;
  if (a->props&ARENA_ROOF) {
    newsec->ceiling_flat = newsec->style->ceiling0;
    a->outersec->ceiling_flat = newsec->style->ceiling0;
    newsec->light_level -= 16;
    if (newsec->light_level < c->minlight)
      newsec->light_level = c->minlight;
    if (newsec->light_level>l->bright_light_level)
      newsec->light_level = l->bright_light_level;
  }

  /* Some sector adjustments... */
  if (rollpercent(30) && (a->props&ARENA_PORCH)) {
    flat *light_flat;
    a->outersec->special = RANDOM_BLINK;
    a->outersec->light_level += 20;
    if (a->outersec->light_level > l->bright_light_level)
      a->outersec->light_level = l->bright_light_level;
    light_flat = random_flat0(CEILING|LIGHT,c,NULL);
    if (light_flat) a->outersec->ceiling_flat = light_flat;
  }

  if (a->props&ARENA_NUKAGE) {
    a->outersec->floor_height -= 8;
    if (a->props&ARENA_PORCH) a->outersec->ceiling_height -= 8;
    a->outersec->floor_flat = a->outersec->style->nukage1;
    a->outersec->special = NUKAGE1_SPECIAL;
  } else if ((a->props&ARENA_PORCH) && rollpercent(50)) {
    short d = 8 + 8 * roll(3);
    a->outersec->floor_height += d;
    a->outersec->ceiling_height += d;
  }

  /* Now the inner sector's boundaries */

  v1 = new_vertex(l,a->minx,a->miny);
  v2 = new_vertex(l,a->minx,a->maxy);
  v3 = new_vertex(l,a->maxx,a->maxy);
  v4 = new_vertex(l,a->maxx,a->miny);

  ld = new_linedef(l,v1,v2);
  ld->flags |= TWO_SIDED;
  ld->right = new_sidedef(l,newsec,c);
  ld->right->middle_texture = c->null_texture;
  ld->left = new_sidedef(l,a->outersec,c);
  ld->left->middle_texture = c->null_texture;
  patch_upper(ld,a->walls,c);
  patch_lower(ld,a->walls,c);

  ld = new_linedef(l,v2,v3);
  ld->flags |= TWO_SIDED;
  ld->right = new_sidedef(l,newsec,c);
  ld->right->middle_texture = c->null_texture;
  ld->left = new_sidedef(l,a->outersec,c);
  ld->left->middle_texture = c->null_texture;
  patch_upper(ld,a->walls,c);
  patch_lower(ld,a->walls,c);

  ld = new_linedef(l,v3,v4);
  ld->flags |= TWO_SIDED;
  ld->right = new_sidedef(l,newsec,c);
  ld->right->middle_texture = c->null_texture;
  ld->left = new_sidedef(l,a->outersec,c);
  ld->left->middle_texture = c->null_texture;
  patch_upper(ld,a->walls,c);
  patch_lower(ld,a->walls,c);

  ld = new_linedef(l,v4,v1);
  ld->flags |= TWO_SIDED;
  ld->right = new_sidedef(l,newsec,c);
  ld->right->middle_texture = c->null_texture;
  ld->left = new_sidedef(l,a->outersec,c);
  ld->left->middle_texture = c->null_texture;
  patch_upper(ld,a->walls,c);
  patch_lower(ld,a->walls,c);

  if ((a->props&ARENA_LAMPS)&&(!(a->props&ARENA_PORCH))) {
    lamp = newsec->style->lamp0;
    if (lamp->height >= ch) lamp = newsec->style->shortlamp0;
    new_thing(l,v1->x+2,v1->y+2,0,lamp->thingid,7,c);
    new_thing(l,v2->x+2,v2->y-2,0,lamp->thingid,7,c);
    new_thing(l,v3->x-2,v3->y-2,0,lamp->thingid,7,c);
    new_thing(l,v4->x-2,v4->y+2,0,lamp->thingid,7,c);
  }

  a->innersec = newsec;
}

/* Make the arrival area, where the player first enters */
/* the arena, as well as any powerups he needs. */
void arena_arrival(level *l,arena *a,haa *haa,config *c)
{
  int minx,maxx;
  int cx,cy;
  float na0, na1, na2;   /* Needed ammos */
  int f0,f1,f2;
  int mask = 7;
  sector *newsec;

  minx = a->minx;
  maxx = a->minx + ( a->maxx - a->minx ) / 3;
  cx = ( minx + maxx ) / 2;
  cy = ( a->miny + a->maxy ) / 2;

  /* a simple version */
  new_thing(l,cx,cy,(short)(90*roll(4)),ID_GATEOUT,7,c);
  a->innersec->tag = a->fromtag;
  a->innersec->entry_x = cx;
  a->innersec->entry_y = cy;

  /* except for this */
  if (a->props&ARENA_ARRIVAL_HOLE) {
    linedef *ld1,*ld2,*ld3,*ld4;
    newsec = clone_sector(l,a->innersec);
    newsec->floor_height -= 384;   /* Down in a hole! */
    parallel_innersec_ex(l,a->innersec,newsec,
                         NULL,a->walls,a->walls,
                         cx-31,cy-31,cx+31,cy+31,
                         c,&ld1,&ld2,&ld3,&ld4);
   /* Fix teleport-tag */
    a->innersec->tag = 0;
    newsec->tag = a->fromtag;
    /* Make walls touchable */
    flip_linedef(ld1);
    flip_linedef(ld2);
    flip_linedef(ld3);
    flip_linedef(ld4);
    ld1->type = LINEDEF_S1_RAISE_FLOOR;
    ld1->tag = newsec->tag;
    ld2->type = LINEDEF_S1_RAISE_FLOOR;
    ld2->tag = newsec->tag;
    ld3->type = LINEDEF_S1_RAISE_FLOOR;
    ld3->tag = newsec->tag;
    ld4->type = LINEDEF_S1_RAISE_FLOOR;
    ld4->tag = newsec->tag;
  }

  /* Now weapons and ammo and stuff */
  if (!place_object_in_region(l,minx,a->miny,maxx,a->maxy,
                       c,a->weapon->thingid,24,0,0,0,7))
    if (!place_object_in_region(l,minx,a->miny,maxx,a->maxy,
                         c,a->weapon->thingid,1,0,0,0,7))
      announce(ERROR,"No room for important weapon!");
  place_object_in_region(l,minx,a->miny,maxx,a->maxy,
                         c,ID_SOUL,24,0,0,0,1);
  ammo_value(a->weapon->thingid,haa,&f0,&f1,&f2);
  na0 = (a->boss_count * a->boss->ammo_to_kill[0]) - (float)f0;
  na1 = (a->boss_count * a->boss->ammo_to_kill[1]) - (float)f1;
  na2 = (a->boss_count * a->boss->ammo_to_kill[2]) - (float)f2;
  ammo_value(a->ammo->thingid,haa,&f0,&f1,&f2);
  for (;mask;) {
    if (!place_object_in_region(l,minx,a->miny,maxx,a->maxy,
                         c,a->ammo->thingid,24,0,0,0,mask))
      if (!place_object_in_region(l,minx,a->miny,maxx,a->maxy,
                           c,a->ammo->thingid,1,0,0,0,mask))
        announce(ERROR,"No room for important ammo!");
    na0 -= (float)f0;
    na1 -= (float)f1;
    na2 -= (float)f2;
    if (na0<=0) mask &= ~0x01;
    if (na1<=0) mask &= ~0x02;
    if (na2<=0) mask &= ~0x04;
  }
  if (a->props&ARENA_NUKAGE) {  /* Little stub health */
    place_object_in_region(l,minx,a->miny,maxx,a->maxy,
                           c,ID_MEDIKIT,16,0,0,0,7);
  }

}

/* Make some decorations, ponds, cover, and so on in */
/* the arena.  */
void arena_decor(level *l,arena *a,haa *haa,config *c)
{
  /* STUB; just a pillar in the center */
  int cx,cy,xmult,ymult,zmult;
  sector *newsec;
  texture *tm;
  linedef *ld1, *ld2, *ld3, *ld4;

  if (rollpercent(25)) {
    xmult=1;
    ymult=1;
    zmult=1;
  } else {
    xmult = ( (a->maxx - a->minx) / 3 - 128 ) / 128;
    xmult =  1 + roll(xmult);
    ymult = ( (a->maxy - a->miny) - 128 ) / 128;
    ymult =  1 + roll(ymult);
    zmult = 1 + roll(3);
  }

  if ( (128*zmult) > (a->innersec->ceiling_height - a->innersec->floor_height) )
    zmult = 1;

  newsec = clone_sector(l,a->innersec);
  newsec->floor_height = (short)(a->innersec->floor_height+zmult*128);
  cx = (a->minx + a->maxx ) / 2 - 64 * xmult;
  cy = (a->miny + a->maxy ) / 2 - 64 * ymult;
  if ((a->innersec->style->plaque->props & VTILES)||(zmult==1)) {
    tm = a->innersec->style->plaque;
  } else {
    tm = a->innersec->style->support0;
  }
  parallel_innersec_ex(l,a->innersec,newsec,
                       c->null_texture,a->innersec->style->wall0,tm,
                       cx,cy,cx+128*xmult,cy+128*ymult,c,
                       &ld1,&ld2,&ld3,&ld4);
  ld1->flags &= ~LOWER_UNPEGGED;
  ld2->flags &= ~LOWER_UNPEGGED;
  ld3->flags &= ~LOWER_UNPEGGED;
  ld4->flags &= ~LOWER_UNPEGGED;
  if ((a->props&ARENA_LAMPS)&&rollpercent(50)) {
    genus *lamp;
    lamp = a->innersec->style->lamp0;
    if ( (a->innersec->ceiling_flat != c->sky_flat) &
         (lamp->height < (a->innersec->ceiling_height - newsec->floor_height)))
      lamp = a->innersec->style->shortlamp0;
    if ( (a->innersec->ceiling_flat != c->sky_flat) &
         (lamp->height < (a->innersec->ceiling_height - newsec->floor_height)))
      lamp = NULL;
    if (lamp) {
      new_thing(l,cx+16,cy+16,0,lamp->thingid,7,c);
      new_thing(l,cx+16,cy+128*ymult-16,0,lamp->thingid,7,c);
      new_thing(l,cx+128*xmult-16,cy+128*ymult-16,0,lamp->thingid,7,c);
      new_thing(l,cx+128*xmult-16,cy+16,0,lamp->thingid,7,c);
      if (newsec->light_level <= l->lit_light_level)
        newsec->light_level += 20;
    }
  }
}

/* Put down the main enemy for the arena, his structures, */
/* and whatever ending gates and/or switches are needed. */
void arena_boss(level *l,arena *a,haa *haa,config *c)
{
  /* STUB */
  int cx,cy;
  short facing;
  boolean need_switch = TRUE;

  cx = a->minx + 5 * ( a->maxx - a->minx ) / 6;
  cy = (a->miny + a->maxy ) / 2;
  facing = facing_along(cx,cy,a->innersec->entry_x,a->innersec->entry_y);
  new_thing(l,cx,cy,facing,a->boss->thingid,7,c);
  if (a->boss_count>1)  /* Only 1 and 2 supported! */
    new_thing(l,cx,cy-(a->boss->width+8),facing,a->boss->thingid,7,c);

  if ((c->episode==2)&&(c->mission==8)) need_switch = FALSE;
  if ((c->episode==3)&&(c->mission==8)) need_switch = FALSE;
  if (((c->episode==4)&&(c->mission==8))||(c->map==7)) {
    linedef *ld1, *ld2, *ld3, *ld4;
    sector *newsec;
    need_switch = FALSE;
    cx -= 32;
    cx &= ~(63);
    cy += a->boss->width+72;
    cy &= ~(63);
    newsec = new_sector(l,(short)(a->innersec->floor_height+64),
                          a->innersec->ceiling_height,
                          random_gate(c,a->innersec->style),
                          a->innersec->ceiling_flat);
    newsec->style = a->innersec->style;
    newsec->light_level = 250;
    newsec->special = GLOW_BLINK;
    newsec->tag = 666;
    parallel_innersec_ex(l,a->innersec,newsec,
                         NULL,NULL,a->innersec->style->wall0,
                         cx,cy,cx+64,cy+64,c,
                         &ld1,&ld2,&ld3,&ld4);
    ld1->type = LINEDEF_W1_END_LEVEL;
    ld1->flags &= ~LOWER_UNPEGGED;
    ld2->type = LINEDEF_W1_END_LEVEL;
    ld2->flags &= ~LOWER_UNPEGGED;
    ld3->type = LINEDEF_W1_END_LEVEL;
    ld3->flags &= ~LOWER_UNPEGGED;
    ld4->type = LINEDEF_W1_END_LEVEL;
    ld4->flags &= ~LOWER_UNPEGGED;
  }
  if ((c->episode==1)&&(c->mission==8)) {
    linedef *ld1, *ld2, *ld3, *ld4;
    sector *newsec;
    short tag = death_room(l,NULL,a->innersec->style,c);
    if (tag) {
      need_switch = FALSE;
      cx -= 32;
      cx &= ~(63);
      cy += a->boss->width+72;
      cy &= ~(63);
      newsec = new_sector(l,(short)(a->innersec->floor_height+64),
                            a->innersec->ceiling_height,
                            random_gate(c,a->innersec->style),
                            a->innersec->ceiling_flat);
      newsec->style = a->innersec->style;
      newsec->light_level = 250;
      newsec->special = GLOW_BLINK;
      newsec->tag = 666;
      parallel_innersec_ex(l,a->innersec,newsec,
                           NULL,NULL,a->innersec->style->wall0,
                           cx,cy,cx+64,cy+64,c,
                           &ld1,&ld2,&ld3,&ld4);
      ld1->type = LINEDEF_TELEPORT;
      ld1->tag = tag;
      ld1->flags &= ~LOWER_UNPEGGED;
      ld2->type = LINEDEF_TELEPORT;
      ld2->tag = tag;
      ld2->flags &= ~LOWER_UNPEGGED;
      ld3->type = LINEDEF_TELEPORT;
      ld3->tag = tag;
      ld3->flags &= ~LOWER_UNPEGGED;
      ld4->type = LINEDEF_TELEPORT;
      ld4->tag = tag;
      ld4->flags &= ~LOWER_UNPEGGED;
    }
  }

  if (need_switch) {
    linedef *ld;
    texture *tm;
    cx -= 64;
    cy += a->boss->width+8;
    parallel_innersec_ex(l,a->innersec,NULL,
                         a->innersec->style->wall0,NULL,NULL,
                         cx,cy,cx+128,cy+128,c,
                         NULL,NULL,&ld,NULL);
    /* This next line is a painful hack to get the switch recessed; */
    /* install_switch uses empty_left_side(), which doesn't grok    */
    /* nested enclosing sectors, sigh!                              */
    a->outersec->marked = 1;
    ld = install_switch(l,ld,TRUE,FALSE,0,a->innersec->style,c,NULL);
    a->outersec->marked = 0;
    ld->type = LINEDEF_S1_END_LEVEL;
    tm = random_texture0(EXITSWITCH,c,a->innersec->style);
    if (tm) {
      ld->right->middle_texture = tm;
      ld->right->y_offset = tm->y_bias;
    }
    if (ld->right->sector != a->innersec)
      ld->right->sector->ceiling_flat = a->innersec->style->ceiling0;
  }
}

/* Gate out to a big place to fight bosses. */
/* NOTE: this renders the haa invalid at the moment, and so can only */
/* be used in an Episode 8, or the end of a PWAD. */
void arena_gate(level *l,sector *s,haa *haa,config *c)
{
  arena *ThisArena = new_arena(l,c);
  unsigned int newseed = bigrand();

  if (s->gate) announce(WARNING,"Stacked gates?");

  /* Put in an exit-style outgoing gate */
  s->gate = new_gate(l,0,new_tag(l),0,FALSE,c);
  ThisArena->fromtag = s->gate->out_tag;
  install_gate(l,s,s->style,haa,FALSE,c);   /* Don't want EXIT style, eh? */
  s->middle_enhanced = TRUE;

  /* Now put down some powerups and stuff in s... */
  prepare_arena_gate(l,s,ThisArena,haa,c);

  /* Make the arena... */
  install_arena(l,ThisArena,s,haa,c);

  /* ... put in some decor ... */
  srand(newseed);
  arena_decor(l,ThisArena,haa,c);

  /* ... put in the player arrival zone ... */
  srand(newseed);
  arena_arrival(l,ThisArena,haa,c);

  /* ... and the boss and exit areas. */
  srand(newseed);
  arena_boss(l,ThisArena,haa,c);

  /* and we're done */
  announce(VERBOSE,"Arena");
  return;

}

/* A room that has a big well in the center that eventually rises */
/* while you fight the awful monsters with the big teeth. */
boolean rising_room(level *l,sector *s,config *c,haa *haa,quest *ThisQuest)
{
  int minx, miny, maxx, maxy;
  int xborder, yborder, depth;
  sector *newsec;
  boolean did_trigger = FALSE;
  linedef *ld1,*ld2,*ld3,*ld4;
  thing *t;
  short tid = rollpercent(50) ? ID_POTION : ID_HELMET;

  if (s->gate) return FALSE;

  /* Make sure nice and huge */
  find_rec(l,s,&minx,&miny,&maxx,&maxy);
  if (maxx-minx<320) return FALSE;
  if (maxy-miny<320) return FALSE;

  xborder = (64 + roll((maxx-minx)-320)) / 2;
  yborder = (64 + roll((maxy-miny)-320)) / 2;
  switch(roll(3)) {
    case 0: depth = 256; break;
    case 1: depth = 256 + 32 * roll(33); break;
    default: depth = 256 + 32 * roll(13); break;
  }
  newsec = clone_sector(l,s);
  newsec->floor_height -= depth;
  if (newsec->light_level>160) newsec->light_level = 160;
  newsec->tag = new_tag(l);
  parallel_innersec_ex(l,s,newsec,NULL,s->style->wall0,s->style->support0,
                       minx+xborder,miny+yborder,maxx-xborder,maxy-yborder,
                       c,&ld1,&ld2,&ld3,&ld4);
  s->middle_enhanced = TRUE;

  /* Point the right sides into the well, for pushing and find_rec */
  flip_linedef(ld1);
  flip_linedef(ld2);
  flip_linedef(ld3);
  flip_linedef(ld4);

  if (ThisQuest->goal == KEY_GOAL) {
    t = new_thing(l,(minx+maxx)/2,(miny+maxy)/2,0,ThisQuest->type,7,c);
    ThisQuest->thing = t;   /* For later */
    if (ThisQuest->auxtag==0)
      if (!(c->gamemask&DOOM0_BIT))
        if (rollpercent(80)) {
          did_trigger = TRUE;
          trigger_box(l,t,newsec,newsec->tag,LINEDEF_W1_RAISE_FLOOR,c);
          announce(VERBOSE,"Zlooty");
        }
  }

  if (ThisQuest->goal == NULL_GOAL) {
    ThisQuest->thing = place_required_small_pickable(l,newsec,c);
    if (ThisQuest->auxtag==0)
      if (!(c->gamemask&DOOM0_BIT))
        if (rollpercent(50)) {
          t = new_thing(l,(minx+maxx)/2,(miny+maxy)/2,0,tid,7,c);
          did_trigger = TRUE;
          trigger_box(l,t,newsec,newsec->tag,LINEDEF_W1_RAISE_FLOOR,c);
        }
  }

  if (!did_trigger) {
    ld1->type = LINEDEF_S1_RAISE_FLOOR;
    ld1->tag = newsec->tag;
    ld2->type = LINEDEF_S1_RAISE_FLOOR;
    ld2->tag = newsec->tag;
    ld3->type = LINEDEF_S1_RAISE_FLOOR;
    ld3->tag = newsec->tag;
    ld4->type = LINEDEF_S1_RAISE_FLOOR;
    ld4->tag = newsec->tag;
  }

  /* Put a couple of things on the ledge */
  if (rollpercent(30)) place_timely_something(l,haa,c,minx+16,miny+16+roll((maxy-miny)-31));
  if (rollpercent(30)) place_timely_something(l,haa,c,maxx-16,miny+16+roll((maxy-miny)-31));
  if (rollpercent(30)) place_timely_something(l,haa,c,minx+16+roll((maxx-minx)-31),miny+16);
  if (rollpercent(30)) place_timely_something(l,haa,c,minx+16+roll((maxx-minx)-31),maxy-16);

  /* Now populate the well */
  populate(l,newsec,c,haa,FALSE);

  if (rollpercent(20)) {
    newsec->floor_flat = c->water_flat;       /* Eli's idea */
    announce(VERBOSE,"Water");
  }

  announce(VERBOSE,"Rising room");

  return TRUE;

}


/* Put whatever's required by this quest into this sector */
void close_quest(level *l,sector *s,quest *q,haa *haa,config *c)
{
  linedef *ld;
  int i, j;
  thing *t;
  texture *tm;
  boolean done = FALSE;

  s->has_key = TRUE;

  switch (q->goal) {
    case SWITCH_GOAL:
      /* Could decide to use a walkthrough linedef or whatever */
      /* instead of a switch for a tag goal.                   */
      i = mark_decent_boundary_linedefs(l,s,64);
      ld = random_marked_linedef(l,i);
      unmark_linedefs(l);
      if (ld==NULL) {
        announce(ERROR,"No applicable linedef to put switch on!");
      } else {
        ld = install_switch(l,ld,c->recess_switches,FALSE,0,s->style,c,NULL);
        ld->type = q->type;
        ld->tag = q->tag;
      }
      /* Maybe a potion or whatever for surprises.  Always?? */
      t = place_required_small_pickable(l,s,c);
      q->thing = t;   /* For later */
      break;
    case LEVEL_END_GOAL:
      /* Just alter some non-tiny boundary linedef to be a switch, */
      /* and try to make it obvious via some light stuff.  Or,     */
      /* sometimes, do the floor-hole thing, or a gate, or...      */
      i = mark_decent_boundary_linedefs(l,s,64);
      for (j=0;j<5;j++) {  /* Try a little harder to get recessible! */
        ld = random_marked_linedef(l,i);
        if (empty_left_side(l,ld,16)) break;
      }
      unmark_linedefs(l);
      if (ld==NULL) {
        announce(ERROR,"No applicable linedef to end level on!");
      } else {
        if ((c->episode==1)&&(c->mission==8)) {  /* Try a fun thing! */
          if (e1m8_gate(l,ld,s,haa,c)) {
            announce(VERBOSE,"e1m8 finale");
            break;
          }
        }
        if (rollpercent(c->p_hole_ends_level)) {  /* Try a floor-hole */
          if (empty_left_side(l,ld,128)) {    /* 128?  Hugeness? */
            if (linelen(ld)>192) split_linedef(l,ld,128,c);
            lefthand_box(l,ld,128,s->style,c)->right->middle_texture =
              s->style->wall0;
            ld->type = LINEDEF_W1_END_LEVEL;
            if (ld->left->sector->light_level<160)
              ld->left->sector->light_level=160;
            ld->left->sector->floor_flat = c->sky_flat;
            ld->left->sector->floor_height -= 16;
            announce(VERBOSE,"Hole ends level");
            done = TRUE;
          }
        }
        if ((!done)&&(s->gate==NULL)&&rollpercent(c->p_gate_ends_level)) {
          /* Do an exit gate */
          s->gate = new_gate(l,0,0,0,TRUE,c);
          install_gate(l,s,s->style,haa,FALSE,c);
          gate_populate(l,s,haa,FALSE,c);  /* Some stuff */
          s->middle_enhanced = TRUE;
          if (s->light_level>130) s->light_level=130;   /* To see "EXIT" */
          announce(VERBOSE,"Gate ends level");
          done = TRUE;
        }
        /* Switch with recess, sometimes fancied-up */
        if (!done) {
          ld = install_switch(l,ld,TRUE,rollpercent(10),0,s->style,c,NULL);
          ld->type = q->type;
          ld->tag = q->tag;    /* Will be zero, actually */
          ld->right->sector->special = GLOW_BLINK;
          if (s->light_level>190) s->light_level = 190;  /* So the glow shows */
          ld->right->sector->light_level = 255;
          done = TRUE;
          tm = random_texture0(EXITSWITCH,c,s->style);
          if (tm) {
            ld->right->middle_texture = tm;
            ld->right->y_offset = tm->y_bias;
            announce(VERBOSE,"Custom exit switch");
          }
        }
        if (need_secret_level(c) && !l->sl_done &&!l->sl_tag) {
          /* This sets sl_done if it works */
          install_sl_exit(l,s,haa,s->style,q,TRUE,c);
        }
      }
      break;
    case ARENA_GOAL:
      /* A teleporter to a big arena in which to fight bosses */
      arena_gate(l,s,haa,c);
      break;
    case GATE_GOAL:
      /* A teleporter to, and perhaps from, the goal room. */
      s->gate = new_gate(l,q->tag2,q->tag,0,FALSE,c);
      break;
    case KEY_GOAL:
      if (rollpercent(l->p_rising_room)) {
        done = rising_room(l,s,c,haa,q);
      }
      if (!done) {  /* Simple case */
        t = place_required_pickable(l,s,c,q->type);
        q->thing = t;   /* For later */
      }
      break;
    case NULL_GOAL:
      if (rollpercent(2*l->p_rising_room)) {
        done = rising_room(l,s,c,haa,q);
      }
      if (!done) {  /* Simple case; potion or whatever for surprises */
        t = place_required_small_pickable(l,s,c);
        q->thing = t;   /* For later */
      }
      break;
    default:
      announce(ERROR,"Unfamiliar goal type; quest not ended.");
    }
  return;
}

/* Consider "pushing" the current quest, which really means */
/* putting its goal right here, but guarding it with something */
/* that still has to be quested for. */
void maybe_push_quest(level *l,sector *s,quest *q,config *c)
{
  short newkey;
  linedef *ld;
  short locked_linedef_type;
  int i;

  if (!rollpercent(c->p_pushquest)) return;  /* Do we want to? */
  if (q->goal!=SWITCH_GOAL) return;  /* Do we know how? */

  newkey = new_key(l);   /* Get an unused key */
  if (!newkey) return;

  /* Figure out how to lock it */
  locked_linedef_type = locked_linedef_for(q->type,newkey,c);
  if (!locked_linedef_type) return;

  /* Find a linedef to install the switch on */
  i = mark_decent_boundary_linedefs(l,s,64);
  ld = random_marked_linedef(l,i);
  unmark_linedefs(l);
  if (ld==NULL) return;

  /* Install the switch and hook it up */
  ld = install_switch(l,ld,TRUE,rollpercent(50),newkey,s->style,c,NULL);
  ld->type = locked_linedef_type;
  ld->tag = q->tag;

  /* Now modify the quest */
  q->goal = KEY_GOAL;
  q->type = newkey;
  q->tag = 0;

  /* and we're done... */
  announce(LOG,"Quest push");
  return;
}

/* Construct a linedef suitable for a generate_room_outline for  */
/* the next room.  If old is not NULL, re-use it, just changing  */
/* its to and from. */
/* For most links this will be an antiparallel linedef on the left */
/* side of this one. */
linedef *make_linkto(level *l,linedef *ld,link *ThisLink, style *ThisStyle,
                     config *c, linedef *old)
{
  int depth;

  switch(ThisLink->type) {
  case BASIC_LINK:
    depth = 0;
    /* Account for any recesses */
    if (ThisLink->bits&LINK_RECESS) depth += 2*ThisLink->depth2;
    /* Account for single door/arch, if any */
    if (!(ThisLink->bits&(LINK_CORE|LINK_ALCOVE))) depth += ThisLink->depth1;
    /* Account for double doors around the core, if any */
    if ( ((ThisLink->bits&LINK_CORE)) && (ThisLink->bits&LINK_NEAR_DOOR) )
      depth += ThisLink->depth1;
    if ( ((ThisLink->bits&LINK_CORE)) && (ThisLink->bits&LINK_FAR_DOOR) )
      depth += ThisLink->depth1;
    /* Alcove width */
    if (ThisLink->bits&LINK_ALCOVE) {
      depth += ThisLink->width2;
    } else {  /* Straight-through core */
      if (ThisLink->bits&LINK_CORE) depth += ThisLink->depth3;
    }
    break;
  case OPEN_LINK:
    depth = ThisLink->depth1;
    break;
  case GATE_LINK: {
    linedef *ldnew;
    vertex *v, *v1;
    int newsize;
    int minx = HUGE_NUMBER;
    for (v=l->vertex_anchor;v;v=v->next) if (v->x < minx) minx = v->x;
    minx -= 64;
    if (ld) {
      newsize = linelen(ld);
    } else {
      newsize = 512;
    }
    if (newsize< 256 * l->hugeness) newsize = 256 * l->hugeness;
    if (old) {
      old->from->x = minx;
      old->to->x = minx;
      old->from->y = newsize/2;
      old->to->y = 0-newsize/2;
      ldnew = old;
    } else {
      v = new_vertex(l,minx,newsize/2);
      v1 = new_vertex(l,minx,0-newsize/2);
      ldnew = new_linedef(l,v,v1);
    }
    return ldnew;
  }
  default:
    announce(ERROR,"Funny linktype in make_linkto.");
    depth = ThisLink->depth1;
  }
  return flip_linedef(make_parallel(l,ld,depth,old));
}

/* Given two antiparallel linedefs, does there seem to be an empty */
/* rectangle between their left sides, or whatever else this link  */
/* needs? */
boolean link_fitsv(level *l,linedef *ldf1, linedef *ldf2, link *ThisLink)
{
  boolean answer;

  if (ThisLink->type==GATE_LINK) return TRUE;  /* These don't care */

  ldf1->from->marked = TRUE;
  ldf1->to->marked = TRUE;
  ldf2->from->marked = TRUE;
  ldf2->to->marked = TRUE;
  if (ldf1->right) ldf1->right->sector->marked = 1;
  if (ldf2->right) ldf2->right->sector->marked = 1;
  answer = empty_rectangle(l,ldf1->from->x,ldf1->from->y,
                             ldf1->to->x,ldf1->to->y,
                             ldf2->from->x,ldf2->from->y,
                             ldf2->to->x,ldf2->to->y);
  if (ldf1->right) ldf1->right->sector->marked = 0;
  if (ldf2->right) ldf2->right->sector->marked = 0;
  ldf1->from->marked = FALSE;
  ldf1->to->marked = FALSE;
  ldf2->from->marked = FALSE;
  ldf2->to->marked = FALSE;
  return answer;
}

void mid_tile(level *l, sector *s,
              short *tlx, short *tly, short *thx, short *thy)
{
  int minx,miny,maxx,maxy;

  find_rec(l,s,&minx,&miny,&maxx,&maxy);
  *tlx = (minx + maxx) / 2;
  *tlx = *tlx & 0xFFC0;   /* Round down to 64; down? */
  if (*tlx<=minx) *tlx = minx + 1;
  *tly = (miny + maxy) / 2;
  *tly = *tly & 0xFFC0;   /* Round down to 64; down? */
  if (*tly<=miny) *tly = miny + 1;
  *thx = *tlx + 64;
  if (*thx>=maxx) *thx = maxx - 1;
  *thy = *tly + 64;
  if (*thy>=maxy) *thy = maxy - 1;
}

/* Is it OK to obstruct the middle (as defined by mid_tile()) tile */
/* of this sector?  i.e. might it block a way or a door? */
boolean ok_to_block_mid_tile(level *l, sector *s)
{
  short tlx, tly, thx, thy;
  int minx, miny, maxx, maxy;

  find_rec(l,s,&minx,&miny,&maxx,&maxy);
  mid_tile(l,s,&tlx,&tly,&thx,&thy);
  /* Very strong rectangle assumptions here! */
  if (tlx-minx<33) return FALSE;
  if (tly-miny<33) return FALSE;
  if (maxx-thx<33) return FALSE;
  if (maxy-thy<33) return FALSE;
  return TRUE;
}

/* Given a bare linedef, make a room extending from its right side. */
sector *generate_room_outline(level *l,linedef *ld,style *ThisStyle,
                              boolean try_reduction,config *c)
{
  sector *answer;
  linedef *newld;
  vertex *v1,*v2;
  int x1,y1,x2,y2,len1,len2;

  /* Very simple squarish rooms */
  len1 = linelen(ld);
  if (roll(2)) {
    len2 = len1;
  } else {
    len2 = len1 + l->hugeness * 64 * (4-roll(9));
  }
  if (len2<128) {
    len2 = 128;
  } else if (len2>1600) {
    len2 = 1600;
  }

  /* Bigify.  */
  if (!try_reduction)   /* Not if we're constrained */
    if (rollpercent(c->p_bigify))
      if (len2<512) len2 *= 2;   /* Keep 'em big! */

  for (;;) {   /* Until we find one that fits */
    point_from(ld->from->x,ld->from->y,ld->to->x,ld->to->y,
               RIGHT_TURN,len2,&x1,&y1);
    point_from(ld->to->x,ld->to->y,x1,y1,RIGHT_TURN,len1,&x2,&y2);
    ld->from->marked = 1;
    ld->to->marked = 1;
    if (empty_rectangle(l,ld->from->x,ld->from->y,ld->to->x,ld->to->y,
                       x1,y1,x2,y2)) break;
    if (!try_reduction) return NULL;
    len2 -= 32;  /* If failed, try again with smaller room */
    if (len2<(l->hugeness*64)) {
      announce(VERBOSE,"No possible rectangle fits in the space.");
      ld->to->marked=0;
      ld->from->marked=0;
      return NULL;
    }
  }
  ld->to->marked = 0;
  ld->from->marked = 0;

  printf(".");
  fflush(stdout);
  {
  char s[200];
  sprintf(s,"New room, corners (%d %d) (%d %d) (%d %d) (%d %d).",
    ld->from->x,ld->from->y,ld->to->x,ld->to->y,x1,y1,x2,y2);
  announce(VERBOSE,s);
  }

  answer = new_sector(l,0,ThisStyle->wallheight0,
                        ThisStyle->floor0, ThisStyle->ceiling0);
  answer->style = ThisStyle;
  answer->light_level = ThisStyle->roomlight0;  /* paint_room can override? */
  ld->right = new_sidedef(l,answer,c);

  v1 = ld->to;
  v2 = new_vertex(l,x1,y1);
  newld = new_linedef(l,v1,v2);
  newld->right = new_sidedef(l,answer,c);
  /* If the wall is long, sometimes split it, for more outlinks */
  /* Should use styles here and stuff too (and config)          */
  if (linelen(newld)>(l->hugeness*256))
    if (rollpercent(25))
      split_linedef(l,newld,linelen(newld)/2,c);

  v1 = v2;
  v2 = new_vertex(l,x2,y2);
  newld = new_linedef(l,v1,v2);
  newld->right = new_sidedef(l,answer,c);
  if (linelen(newld)>(l->hugeness*256))
    if (rollpercent(25))
      split_linedef(l,newld,linelen(newld)/2,c);

  v1 = v2;
  v2 = ld->from;
  newld = new_linedef(l,v1,v2);
  newld->right = new_sidedef(l,answer,c);
  if (linelen(newld)>(l->hugeness*256))
    if (rollpercent(25))
      split_linedef(l,newld,linelen(newld)/2,c);

  return answer;
}

/* Return a random link that will fit on this linedef, */
/* and that can be locked for this quest (if any). */
/* Note that ld can be NULL, meaning "don't worry about it" */
link *random_link(level *l,linedef *ld,style *ThisStyle,quest *ThisQuest,
                   config *c)
{
  link *answer = NULL;
  boolean open_ok = TRUE;

  if (ld) if (linelen(ld)<100) open_ok = FALSE;
  if (ThisQuest)
    if (ThisQuest->goal==KEY_GOAL)
      open_ok = FALSE;

  if (l->use_gates)
    if (ThisQuest)
      if (rollpercent(20))  /* Should vary */
        if (ThisQuest->goal==SWITCH_GOAL)
          if (!(ld->right->sector->gate))
            if (ld->right->sector != l->first_room)
              if (ok_to_block_mid_tile(l,ld->right->sector))
                if (!(c->do_dm))
                  return gate_link(l,c);     /* Already in link_anchor */

  if (answer==NULL) {
    if (rollpercent(l->p_open_link)&&open_ok) {
      answer = random_open_link(l,ld,ThisStyle,ThisQuest,c);
    } else {
      answer = random_basic_link(l,ld,ThisStyle,ThisQuest,c);
    }
  }

  answer->next = l->link_anchor;
  l->link_anchor = answer;
  return answer;
}

/* Return a random open link that will fit on this linedef */
/* Note that ld can be NULL, meaning "don't worry about it" */
link *random_open_link(level *l,linedef *ld,style *ThisStyle,quest *ThisQuest,
                       config *c)
{
  int dieroll, len = 0;
  link *answer = (link *)malloc(sizeof (*answer));

  answer->bits = 0;
  answer->type = OPEN_LINK;

  if ((ThisQuest==NULL)&&rollpercent(40)) {    /* 40 should vary */
    answer->bits |= LINK_LIFT;
  } else {
    answer->bits |= LINK_STEPS;
    if (rollpercent(30)) answer->bits |= LINK_ALCOVE;  /* sidesteps */
    if (rollpercent(50)) answer->bits |= LINK_LEFT;
  }

  if (ld) len = linelen(ld);

  /* Primary width; need more variety! */
  dieroll = roll(100);
  if (dieroll<35) {
    answer->width1 = 64 * l->hugeness;
  } else if (dieroll<70) {
    answer->width1 = 128 * l->hugeness;
  } else {
    answer->width1 = 0;     /* Means "about a third of the wall" */
  }

  if (ld) if (answer->width1+66>len) answer->width1 = 0;

  /* Primary depth */
  if (answer->bits&LINK_LIFT) {
    answer->depth1 = l->hugeness * 32 * (1+roll(5));
  } else {
    if ( (answer->bits&LINK_ALCOVE) && rollpercent(50)) {
      answer->depth1 = l->hugeness * 32 * (1+roll(4));
      announce(VERBOSE,"Narrow side-steps?");
    } else {
      answer->depth1 = l->hugeness * 64 * (2+roll(5));    /* Or something */
    }
  }
  if (answer->depth1<33) answer->depth1 = 33;

  /* Suggested height from new floor to existing ceiling */
  answer->height1 = l->hugeness * 16 * (2+roll(7));

  return answer;
}

/* Return a random basic link that will fit on this linedef */
/* Note that ld can be NULL, meaning "don't worry about it" */
/* This routine has grown like kudzu, and needs to be */
/* heavily pruned and organized and fixed. */
link *random_basic_link(level *l,linedef *ld,style *ThisStyle,quest *ThisQuest,
                        config *c)
{
  link *answer;
  int dieroll;
  int len = 0;
  boolean need_door = FALSE;
  boolean nukage_core_trap = FALSE;

  if (ld) len = linelen(ld);

  answer = (link *)malloc(sizeof (*answer));

  /* Should use style and config more here and there */

  answer->type = BASIC_LINK;
  answer->bits = 0;

  if (ThisQuest) {
    if (ThisQuest->goal==KEY_GOAL) need_door = TRUE;
    /* So far the only tags we know of are door-opens and nukage traps */
    if (ThisQuest->goal==SWITCH_GOAL) {
      if (rollpercent(30)||rollpercent(l->p_force_nukage))  /* Huh? */
        need_door = TRUE;
          else nukage_core_trap = TRUE;
    }
  }

  /* Depth of the door sector, if any */
  if (rollpercent(50)) {
    answer->depth1 = 16;
  } else if (rollpercent(50)) {
    answer->depth1 = 8;
  } else if (rollpercent(50)) {
    answer->depth1 = 32;
  } else {
    answer->depth1 = 64;   /* tunneldoor... */
  }
  answer->depth1 *= l->hugeness;

  /* Stairs and lifts will change this walkable default */
  if (rollpercent(50)) {
    answer->floordelta = 0;
  } else {
    answer->floordelta = 24 - 8 * (roll(7));
  }

  /* Primary width default */
  dieroll = roll(100);
  if (dieroll<50) {
    answer->width1 = 64;
  } else if (dieroll<60) {
    answer->width1 = 128;
  } else if (dieroll<80) {
    answer->width1 = 96;
  } else {
    answer->width1 = 0;
  }
  answer->width1 *= l->hugeness;
  if (ld)
    if (len<answer->width1)
      answer->width1 = 0;
  if (l->all_wide_links) answer->width1 = 0;

  answer->height1 = ThisStyle->linkheight0;   /* should vary some? */
  switch(roll(3)) {            /* alcove depth -- needs more variety */
    case 0: answer->width2 = 64; break;
    case 1: answer->width2 = answer->width1; break;
    case 2: answer->width2 = 64 + 8 * (roll(17)); break;
  }
  answer->width2 *= l->hugeness;
  if (answer->width2==0) answer->width2 = 64 * l->hugeness;
  switch (roll(4)) {          /* recess depth -- also */
    case 0: answer->depth2 = 8;  break;
    case 1: answer->depth2 = 4;  break;
    case 2: answer->depth2 = 16; break;
    case 3: answer->depth2 = 20; break;
  }
  if (rollpercent(10)) answer->depth2 *= 2;  /* Nice and deep! */
  answer->depth2 *= l->hugeness;

  /* In case they're needed for cores and stairs and stuff */
  answer->depth3 = 32 * (1+roll(5)) * l->hugeness;
  answer->stepcount = 2+roll(9);

  dieroll = roll(100);
  /* <Half standard, >half whimsical, for now, for fun */
  /* Other numbers are also sorta high, but that gives variety */
  if (dieroll<30) {  /* A standard thing of some kind */
    dieroll = roll(100);
    if (dieroll<20) {  /* Nice recessed door */
      answer->bits = LINK_ANY_DOOR | LINK_RECESS;
    } else if (dieroll<65) {  /* Nice arch */
      answer->bits = 0;
    } else {   /* Simple stairs */
      answer->bits = LINK_CORE | LINK_STEPS;
      answer->depth3 *= 3;
      answer->floordelta = answer->stepcount * (2+roll(20));
    }
  } else {  /* Make something up */
    answer->bits = 0;
    if (roll(2)) answer->bits |= LINK_RECESS;
    if (rollpercent(40)) {
      if (rollpercent(40) || c->both_doors) {
        answer->bits |= LINK_ANY_DOOR;
      } else if (rollpercent(30)) {
        answer->bits |= LINK_NEAR_DOOR;
      } else {
        answer->bits |= LINK_FAR_DOOR;
      }
    }
    if (rollpercent(10)) answer->bits |= LINK_BARS;
    if (answer->width1!=0)    /* Twinning a full-wall link is ugly */
      if ( (!ld) || ( (len/2 - 16) > answer->width1) )
        if (rollpercent(30)) {
          answer->bits |= LINK_TWIN;
          if (rollpercent(60)) answer->bits |= LINK_WINDOW;
        }
    if (rollpercent(30)) answer->bits |= LINK_ALCOVE;
    if ( (ld) && ( (len/2 - 16) < answer->width1) )
      answer->bits &= ~LINK_ALCOVE;
    if ( (ld) && ( (len/4 - 32) < answer->width1) && (answer->bits&LINK_TWIN))
      answer->bits &= ~LINK_ALCOVE;
    if (rollpercent(40)) {
      answer->bits |= LINK_CORE;
      if (rollpercent(40)) {
        answer->bits |= LINK_STEPS;
        answer->depth3 *= 3;
        answer->floordelta = answer->stepcount * (2+roll(20));
      } else if (l->lift_rho&&!need_door) {
        answer->bits |= LINK_LIFT;
        if (!(answer->bits&LINK_ALCOVE))
          answer->bits &= ~LINK_ANY_DOOR;  /* not currently compatible */
        if (rollpercent(50)) {
          answer->floordelta = 32 + 8 * roll(51);  /* Potentially big */
        } else {
          answer->floordelta = 25 + 4 * roll(26);  /* smaller */
        }
        if (answer->depth3<64) answer->depth3 = 64;
      }
    }
  }
  if (l->no_doors) answer->bits &= ~LINK_ANY_DOOR;

  /* Make sure we have a door if we need one (to lock, etc) */
  if (need_door) answer->bits |= LINK_NEAR_DOOR;

  /* Fewer unrecessed and/or really high doors */
  if (answer->bits|LINK_ANY_DOOR) {
    if (rollpercent(75)) answer->bits |= LINK_RECESS;
    if (rollpercent(75))
      if (answer->height1>72) answer->height1 = 72;  /* Hugeness? */
  }

  /* Sometimes up, sometimes down */
  if (roll(2)) answer->floordelta = 0 - answer->floordelta;

  /* More random fun stuff */
  if (rollpercent(l->p_stair_lamps)) answer->bits |= LINK_LAMPS;
  if (rollpercent(50)) answer->bits |= LINK_MAX_CEILING;
  if (rollpercent(50)) answer->bits |= LINK_LEFT;
  if (rollpercent(75)) answer->bits |= LINK_FAR_TWINS;
  if (rollpercent(75)) answer->bits |= LINK_TRIGGERED;
  if (rollpercent(l->p_force_sky)||rollpercent(l->p_force_sky)||
      rollpercent(50)) answer->bits |= LINK_DECROOM;  /* 50? */

  /* If nukage_core_trap, override much of the above! */
  if (nukage_core_trap) {
    /* A relatively simple core */
    answer->bits &= ~(LINK_STEPS | LINK_ALCOVE | LINK_TWIN | LINK_LIFT);
    answer->bits |= LINK_CORE;
    /* At least 128 long */
    if (answer->depth3<128) answer->depth3 = 128;
    /* And going up a bit */
    answer->floordelta = 4 + roll(18);
    answer->bits |= LINK_LOCK_CORE;
  }

  /* If a gate quest, override much of the above also.  All we want */
  /* is a recessed archway-thing, walkable, etc, to make_window() on */
  if ( (ThisQuest) && (ThisQuest->goal == GATE_GOAL) ) {
    answer->bits &= ~(LINK_STEPS | LINK_ALCOVE | LINK_LIFT | LINK_CORE);
    answer->bits &= ~(LINK_ANY_DOOR | LINK_TRIGGERED);
    answer->bits |= LINK_RECESS;
    if (rollpercent(50)) {
      answer->floordelta = 0;
    } else {
      answer->floordelta = 24 - 8 * (roll(7));
    }
  }

  /* Alcoves require either a door or a recess, and a non-whole width, */
  /* and for now at least a tiny core.                                 */
  if (answer->bits&LINK_ALCOVE) {
    if (LINK_ANY_DOOR!=(answer->bits&LINK_ANY_DOOR)) {
      answer->bits |= LINK_RECESS;
      if (answer->depth2<(8*l->hugeness)) answer->depth2 = 8 * l->hugeness;
    }
    if (answer->width1==0) answer->width1 = 64 * l->hugeness;
    if (!(answer->bits&LINK_CORE)) {
      answer->bits |= LINK_CORE;
      answer->depth3 = 4 * l->hugeness;
    }
  }

  /* Some final sanity checks on stair-sector heights and stuff */
  if (answer->bits&LINK_STEPS) {
    int need;
    /* The clearance we need is 56 plus the step height times */
    /* the number of steps our 64ish-wide shadow is on at once */
    /* (plus eight more in case of doors).  Roughly! */
    need = 64 + (1+(64/(answer->depth3/(answer->stepcount)))) * abs(answer->floordelta / (answer->stepcount-1));
    if (answer->bits&LINK_ANY_DOOR) need += 8;  /* Doors don't open all the way */
    if (answer->height1 < need) answer->height1 = need;
  } else if (!(answer->bits&LINK_LIFT)) {
    if (answer->height1+answer->floordelta<64) answer->height1 = 64 - answer->floordelta;
    if (answer->height1-answer->floordelta<64) answer->height1 = answer->floordelta + 64;
  }

  /* From here on down all we do is turn off fancy bits that worry us, */
  /* or make sure core-depth isn't too small. */

  /* Make sure we're not twinning/alcoving on a too-narrow linedef */
  /* Although this should all be covered above already */
  if (ld) {
    if (len<144) answer->bits &= ~(LINK_TWIN|LINK_ALCOVE);
    if (len<(2*answer->width1))
      answer->bits &= ~(LINK_TWIN|LINK_ALCOVE);
    if (answer->bits&LINK_ALCOVE)
      if (!link_fitsh(ld,answer,c))     /* Try the Official Checker! */
        answer->bits &= ~LINK_ALCOVE;
  }

  if (answer->width1==0) answer->bits &= ~LINK_ALCOVE;

  if ( (answer->bits&LINK_LIFT) && (!(answer->bits&LINK_ALCOVE)))
    answer->bits &= ~LINK_ANY_DOOR;  /* not currently compatible */

  /* Only make a window if not too much floordelta */
  if (answer->floordelta+16>ThisStyle->sillheight+ThisStyle->windowheight)
    answer->bits &= ~LINK_WINDOW;
  /* We don't know a ceiling-delta, so guess here */
  if ((56+answer->floordelta)<(ThisStyle->sillheight))
    answer->bits &= ~LINK_WINDOW;

  /* If two doors are too close together, they won't work. */
  /* Could just turn off NEAR or FAR, eh? */
  if ( (answer->bits&LINK_ANY_DOOR)&&
       (answer->bits&LINK_CORE)&&
       !(answer->bits&LINK_ALCOVE)&&
       (answer->depth3<24) ) answer->depth3 = 24;

  return answer;

}  /* end random_link() */

/* Make a cool recessed lightstrip in the given linedef */
void make_lightstrip(level *l, linedef *ld,style *ThisStyle,int ll,int depth,
                     int spec, int fh, int ch, config *c)
{
  linedef *ldnew;
  sector *s;
  texture *t;

  /* should do an empty_area check here */
  t = ld->right->middle_texture;
  ldnew = lefthand_box(l,ld,4,ThisStyle,c);
  /* Have to shorten ldnew a bit here, for tapered edges, to */
  /* avoid colliding with orthogonal doors and stuff, if we're */
  /* not gonna do a full area check.  Use rather silly shortening */
  if (ldnew->to->x>ldnew->from->x) {
    ldnew->to->x-=2;
    ldnew->from->x+=2;
  }
  if (ldnew->to->x<ldnew->from->x) {
    ldnew->to->x+=2;
    ldnew->from->x-=2;
  }
  if (ldnew->to->y>ldnew->from->y) {
    ldnew->to->y-=2;
    ldnew->from->y+=2;
  }
  if (ldnew->to->y<ldnew->from->y) {
    ldnew->to->y+=2;
    ldnew->from->y-=2;
  }
  ldnew->right->middle_texture = ThisStyle->walllight;
  /* Sometimes use bottom of lights. */
  if (!ThisStyle->peg_lightstrips) ldnew->flags |= LOWER_UNPEGGED;
  s = ldnew->right->sector;
  s->light_level = ll;
  s->special = spec;
  s->floor_height = fh;
  s->ceiling_height = ch;
  patch_upper(ld,t,c);
  patch_lower(ld,t,c);
  /* That wasn't so hard! */
}

/* Is there an <sdepth> empty area on the lefthand side */
/* of the linedef?                                      */
boolean empty_left_side(level *l, linedef *ld, int sdepth)
{
  int newx1, newy1, newx2, newy2;
  boolean rc;

  point_from(ld->from->x,ld->from->y,ld->to->x,ld->to->y,
             LEFT_TURN,sdepth,&newx1,&newy1);
  newx2 = newx1 - ld->to->x + ld->from->x;
  newy2 = newy1 - ld->to->y + ld->from->y;
  ld->from->marked = 1;
  ld->to->marked = 1;
  if (ld->right) ld->right->sector->marked = 1;
  rc = empty_rectangle(l,ld->from->x,ld->from->y,ld->to->x,ld->to->y,
                     newx1,newy1,newx2,newy2);
  if (ld->right) ld->right->sector->marked = 0;
  ld->from->marked = 0;
  ld->to->marked = 0;
  return rc;
}

/* Swell the linedef outward a bit; sdepth pels to the left,  */
/* in sno places.  sno must be 2 or 3.                        */
/* Makes boring rectangular rooms a little more interesting */
/* Seems to have some strange bugs */
void swell_linedef(level *l,linedef *ld,style *ThisStyle,config *c,
                   int sno,int sdepth)
{
  int len,newx1,newy1,newx2,newy2;
  linedef *ldnew1, *ldnew2;
  boolean rc;
  char logstring[200];

  rc = empty_left_side(l,ld,sdepth);
  if (!rc) return;  /* oh, well! */

  sprintf(logstring,"Swelling (%d,%d)-(%d,%d)...\n",ld->from->x,ld->from->y,
    ld->to->x,ld->to->y);
  announce(VERBOSE,logstring);

  /* Now split the linedef, and jiggle the result(s) */
  len = linelen(ld)/sno;
  ldnew1 = split_linedef(l,ld,len,c);
  if (sno==3) ldnew2 = split_linedef(l,ldnew1,len,c);
  point_from(ld->from->x,ld->from->y,ld->to->x,ld->to->y,
             LEFT_TURN,sdepth,&newx1,&newy1);
  if (sno==3) point_from(ldnew1->from->x,ldnew1->from->y,
                         ldnew1->to->x,ldnew1->to->y,
                         LEFT_TURN,sdepth,&newx2,&newy2);
  ld->to->x = newx1;
  ld->to->y = newy1;
  sprintf(logstring,"Swol to (%d,%d)-(%d,%d)...\n",ld->from->x,ld->from->y,
    ld->to->x,ld->to->y);
  announce(VERBOSE,logstring);
  if (sno==3) {
    ldnew1->to->x = newx2;
    ldnew1->to->y = newy2;
    sprintf(logstring,"    and (%d,%d)-(%d,%d)...\n",ldnew1->from->x,ldnew1->from->y,
      ldnew1->to->x,ldnew1->to->y);
    announce(VERBOSE,logstring);
  }

}  /* end swell_linedef */

/* Should these textures be aligned as if they were the same? */
boolean coalignable(texture *t1, texture *t2)
{
  if (t1->subtle==t2) return TRUE;
  if (t2->subtle==t1) return TRUE;
  return (t1==t2);
}

/* Is there room on the given level for the given type of object */
/* at the given point, allowing for at the very least the given  */
/* width?                                                        */
boolean room_at(level *l,genus *g,int x,int y,int width,config *c)
{
  thing *t;

  /* Check for requested length */
  for (t=l->thing_anchor;t;t=t->next)
    if (infinity_norm(t->x,t->y,x,y)<width) return FALSE;
  /* If it's not pickable, make sure not stuck-together */
  if (!(g->bits&PICKABLE))
    for (t=l->thing_anchor;t;t=t->next) {
      if (t->genus->bits&PICKABLE) continue;
      /* This is overly conservative; the real check should */
      /* be against g->width/2 + t->genus->width/2, eh?     */
      if (infinity_norm(t->x,t->y,x,y)<g->width) return FALSE;
      if (infinity_norm(t->x,t->y,x,y)<t->genus->width) return FALSE;
    }
  return TRUE;
}

/* Try to put an object with the given thingid and width into the  */
/* given sector.  Use the given appearance bits, and heading       */
/* deafness.  Return the new thing, or NULL if no room to be found */
/* If angle is -1, point it toward ax/ay. */
thing *place_object(level *l,sector *s,config *c,short thingid,int width,
                       int angle,int ax, int ay,int bits)
{
  int minx,miny,maxx,maxy;

  find_rec(l,s,&minx,&miny,&maxx,&maxy);

  if ((maxx-minx)<width) return NULL;
  if ((maxy-miny)<width) return NULL;

  return place_object_in_region(l,minx,miny,maxx,maxy,
                                c,thingid,width,angle,ax,ay,bits);
}

/* Try to put an object with the given thingid and width into the  */
/* given box.  Use the given appearance bits, and heading          */
/* deafness.  Return the new thing, or NULL if no room to be found */
/* If angle is -1, point it toward ax/ay. */
thing *place_object_in_region(level *l,int minx, int miny, int maxx, int maxy,
                       config *c,short thingid,int width,
                       int angle,int ax,int ay,int bits)
{
  /* Stub assumes rectangles and stuff */
  int x,y,i,n,decksize,tangle;
  genus *g;
  thing *answer;
  struct s_deck {
    int x;
    int y;
    int tried;
  } deck[16];

  {
  char s[200];
  sprintf(s,"place_object trying to place a %04x.",thingid);
  announce(NONE,s);
  }

  g = find_genus(c,thingid);

  if (!(g->bits&PICKABLE)) {
    if (maxx-minx<g->width) return NULL;
    if (maxy-miny<g->width) return NULL;
  }

  /* Try the corners */
  deck[0].x = minx + width/2;
  deck[0].y = miny + width/2;
  deck[1].x = maxx - width/2;
  deck[1].y = maxy - width/2;
  deck[2].x = deck[0].x;
  deck[2].y = deck[1].y;
  deck[3].x = deck[1].x;
  deck[3].y = deck[0].y;
  /* And eight random spots */
  for (i=4;i<12;i++) {
    deck[i].x = minx + width/2 + roll((maxx-minx)-width);
    deck[i].y = miny + width/2 + roll((maxy-miny)-width);
  }
  /* And the center area, if there's room */
  if (((maxx-minx)>(width*2)) && ((maxy-miny)>(width*2))) {
    x = minx + (maxx-minx)/2;
    y = miny + (maxy-miny)/2;
    deck[12].x = x - width/2;
    deck[12].y = y - width/2;
    deck[13].x = x + width/2;
    deck[13].y = y + width/2;
    deck[14].x = deck[12].x;
    deck[14].y = deck[13].y;
    deck[15].x = deck[13].x;
    deck[15].y = deck[12].y;
    decksize = 16;
  } else {
    decksize = 12;
  }

  /* Now we *should* shuffle the deck and go through it in order */
  /* until one is OK, but shuffling is expensive, so for now     */
  /* we'll just use probes.                                      */

  for (i=0;i<decksize;i++) deck[i].tried = FALSE;
  for (i=0;i<10;i++) {
    n = roll(decksize);
    if (deck[n].tried) continue;
    x = deck[n].x;
    y = deck[n].y;
    if (room_at(l,g,x,y,width,c)) {   /* Use first point with room we find */
      tangle = (angle==-1) ? facing_along(x,y,ax,ay) : angle;
      if (!rollpercent(l->p_rational_facing)) tangle = 90 * roll(4);
      answer = new_thing(l,x,y,(short)tangle,thingid,(short)bits,c);
      {
      char s[200];
      sprintf(s,"place_object placed it at (%d,%d).",x,y);
      announce(NONE,s);
      }
      return answer;
    }
    deck[n].tried = TRUE;
  }  /* end for ten probes */

  announce(NONE,"place_object failed");
  return NULL;
}

/* Maybe place some explodables.  Should this effect the haa?  Well, */
/* you can get hurt by an exploding one; on the other hand, you can use */
/* one to kill a monster and thus avoid getting hurt.  So punt. */
void place_barrels(level *l,sector *s,config *c,haa *haa)
{
  int i = 0;
  genus *g;

  if (!rollpercent(l->p_barrels)) return;

  g = random_barrel(c,s->style);
  if (g==NULL) return;    /* No barrels in this style! */

  i = 1 + roll(5);  /* Sort of boring, eh? */

  for (;i;i--) {

    if (NULL==place_object(l,s,c,g->thingid,g->width,0,0,0,(short)7)) return;

    announce(VERBOSE,"Barrel");

  }  /* end forever */

}  /* end place_barrels */

/* Maybe place some plants and other lawn decorations. */
void place_plants(level *l,int allow,sector *s,config *c)
{
  genus *g;

  for (;;) {
    g = random_plant(c,s->style);
    if (g==NULL) return;    /* No plants available! */

    if (rollpercent(10)) return;   /* hmmm... */

    if (g->width<=allow)
      /* Next line used to have "allow", not "g->wdith". Why? */
      if (NULL==place_object(l,s,c,g->thingid,g->width,0,0,0,(short)7))
        return;

    announce(VERBOSE,"Plant");

  }  /* end forever */

}  /* end place_plants */

/* Return some random piece of armor, and a note as to which */
/* levels need some. */
int timely_armor(haa *haa, int *rlevels, config *c)
{
  int i, levels, armortype;

  /* See which levels need more */
  levels = 0;
  for (i=0;i<3;i++) {  /* for each hardness level */
    levels >>= 1;
    if (haa->haas[i].armor < c->usualarmor[i]) levels |= 0x04;
  }

  *rlevels = levels;

  if (levels==0) return 0;

  /* Should be less primitive? */
  if (rollpercent(50)) {
    armortype = ID_HELMET;
  } else if (rollpercent(70)) {
    armortype = ID_GREENSUIT;
  } else {
    armortype = ID_BLUESUIT;
  }

  return armortype;

}

/* Update the haa in the obvious way.  Well, almost the obvious */
/* way.  It has to make some assumptions about how optimally */
/* the player will utilize a suit.  Some random parameters in here! */
void update_haa_for_armor(haa *haa,int levels,short armortype)
{

  switch (armortype) {
    case ID_HELMET:
      if (levels&0x01) haa->haas[ITYTD].armor++;
      if (levels&0x02) haa->haas[HMP].armor++;
      if (levels&0x04) haa->haas[UV].armor++;
      break;
    case ID_GREENSUIT:
      if (levels&0x01) {
        haa->haas[ITYTD].armor += 20;
        if (haa->haas[ITYTD].armor<100) haa->haas[ITYTD].armor = (float)100;
      }
      if (levels&0x02) {
        haa->haas[HMP].armor += 30;
        if (haa->haas[HMP].armor<100) haa->haas[HMP].armor = (float)100;
      }
      if (levels&0x04) {
        haa->haas[UV].armor += 50;
        if (haa->haas[UV].armor<100) haa->haas[UV].armor = (float)100;
      }
      break;
    case ID_BLUESUIT:
      if (levels&0x01) {
        haa->haas[ITYTD].armor += 40;
        if (haa->haas[ITYTD].armor<200) haa->haas[ITYTD].armor = (float)200;
      }
      if (levels&0x02) {
        haa->haas[HMP].armor += 60;
        if (haa->haas[HMP].armor<200) haa->haas[HMP].armor = (float)200;
      }
      if (levels&0x04) {
        haa->haas[UV].armor += 100;
        if (haa->haas[UV].armor<200) haa->haas[UV].armor = (float)200;
      }
      break;
    default:
      announce(ERROR,"Odd armortype in u_h_f_armor");
  }

}

/* Maybe place some armor, update the haa */
void place_armor(level *l,sector *s,config *c,haa *haa)
{
  int levels = 0;
  int armortype;

  if (rollpercent(10)) return;   /* Correct? */

  for (;;) {
    announce(NONE,"place_armor looking for needy levels");
    armortype = timely_armor(haa,&levels,c);
    if (levels==0) return;   /* Done if none */
    announce(NONE,"place_armor found some needy levels");
    if (NULL==place_object(l,s,c,(short)armortype,48,0,0,0,(short)levels)) return;
    announce(NONE,"place_armor placed some armor");
    update_haa_for_armor(haa,levels,(short)armortype);
    if (rollpercent(25)) return;   /* Reasonable? */
  }  /* end forever */

}

/* Return some useful kind of ammo or weapon, and what levels */
/* it ought to be given to. */
int timely_ammo(haa *haa, int *rlevels, config *c)
{
  int levels = 0;
  int i, ammotype = 0;
  boolean need_shotgun, need_plasgun, need_launcher;

  need_shotgun = FALSE;
  need_plasgun = FALSE;
  need_launcher = FALSE;

  /* See which levels need more */
  for (i=0;i<3;i++) {  /* for each hardness level */
    levels >>= 1;
    if (haa->haas[i].ammo < c->usualammo[i]) levels |= 0x04;
    if (haa->haas[i].can_use_shells == FALSE) need_shotgun = TRUE;
    if (haa->haas[i].can_use_cells == FALSE) need_plasgun = TRUE;
    if (haa->haas[i].can_use_rockets == FALSE) need_launcher = TRUE;
  }

  *rlevels = levels;

  if (levels==0) return 0;

  /* it would be logical to only put down shells if */
  /* the player-model can use them, only put down */
  /* cells if... etc.  But that's a little complex, */
  /* since we want to put down a single ammo that will */
  /* be useful for all the hardness levels.  We cheat on */
  /* the shotgun, by always putting down one of those if */
  /* any level doesn't have one.   And for the plasma gun */
  /* and the rocket launcher, we always give to all levels, */
  /* if to any, if any at all didn't have one yet. */

  if ( (!c->weapons_are_special) && (need_shotgun) ) {
    if ((!(c->gamemask&(DOOM0_BIT|DOOM1_BIT))) && rollpercent(30)) {
      ammotype = ID_SSGUN;
    } else {
      ammotype = ID_SHOTGUN;
    }
  } else if ( (!c->weapons_are_special) && rollpercent(15) ) {
    int weapcount;
    if (c->gamemask&(DOOM0_BIT|DOOM1_BIT)) weapcount = 4;
      else weapcount = 5;
    switch (roll(weapcount)) {
      case 0: if (c->big_weapons) {
          ammotype = ID_PLASMA;
        } else {
          ammotype = ID_SHOTGUN;
        }
        break;
      case 1: ammotype = ID_SHOTGUN; break;
      case 2: ammotype = ID_CHAINGUN; break;
      case 3: if (c->big_weapons) {
          ammotype = ID_LAUNCHER;
        } else {
          ammotype = ID_SHOTGUN;
        }
        break;
      case 4: ammotype = ID_SSGUN; break;
    }
  } else if (rollpercent(10)) {
    ammotype = ID_CLIP;
  } else if (haa->haas[0].can_use_cells && rollpercent(10)) {
    ammotype = ID_CELL;
  } else if (haa->haas[0].can_use_cells && rollpercent(15)) {
    ammotype = ID_CELLPACK;
  } else if (haa->haas[0].can_use_rockets && rollpercent(12)) {
    ammotype = ID_ROCKET;
  } else if (haa->haas[0].can_use_rockets && rollpercent(15)) {
    ammotype = ID_ROCKBOX;
  } else if (rollpercent(10)) {
    ammotype = ID_BULBOX;
  } else if (rollpercent(60)) {
    ammotype = ID_SHELLS;
  } else {
    ammotype = ID_SHELLBOX;
  }

  if ((ammotype==ID_PLASMA) && (need_plasgun)) {
    levels |= 0x07;   /* All, if any */
  }

  if ((ammotype==ID_LAUNCHER) && (need_launcher)) {
    levels |= 0x07;   /* All, if any */
  }

  *rlevels = levels;   /* In case we just changed them */
  return ammotype;
}

/* How much is that ammo in the window?   Three numbers, one for each */
/* skill level (since value can vary with what weapons you have!) */
void ammo_value(short ammotype,haa *haa,int *f0,int *f1,int *f2)
{
  int answer;
  boolean special_case = FALSE;

  /* These numbers should just be stored in the config, in the genus */
  switch (ammotype) {
    case ID_SSGUN:
    case ID_SHOTGUN: answer = 560;
      special_case = TRUE; break;
    case ID_SHELLS: answer = 280;
      special_case = TRUE; break;
    case ID_SHELLBOX: answer = 1400;
      special_case = TRUE; break;
    case ID_PLASMA: answer = 880; break;
    case ID_BFG: answer = 880; break;   /* but a BFG is better, eh? */
    case ID_CHAINGUN: answer = 200; break;
    case ID_LAUNCHER: answer = 200; break;
    case ID_CLIP: answer = 100; break;
    case ID_BULBOX: answer = 500; break;
    case ID_CELL: answer = 440; break;
    case ID_CELLPACK: answer = 2200; break;
    case ID_ROCKET: answer = 100; break;
    case ID_ROCKBOX: answer = 500; break;
    default:
      announce(ERROR,"Funny ammo type in a_v");
      answer = 0;
  }
  *f0 = *f1 = *f2 = answer;
  if (special_case) {   /* Sort of a hack!  Make more general? */
    if ( (ammotype==ID_SSGUN) || (haa->haas[0].has_ssgun) )
      *f0 = (int) ( (double)answer * 10.0 / 7.0 );
    if ( (ammotype==ID_SSGUN) || (haa->haas[1].has_ssgun) )
      *f1 = (int) ( (double)answer * 10.0 / 7.0 );
    if ( (ammotype==ID_SSGUN) || (haa->haas[2].has_ssgun) )
      *f2 = (int) ( (double)answer * 10.0 / 7.0 );
  }
  return;
}

/* The obvious thing */
void update_haa_for_ammo(haa *haa,int levels,short ammotype)
{
  int a0,a1,a2;

  ammo_value(ammotype,haa,&a0,&a1,&a2);

  if (levels&0x01) haa->haas[ITYTD].ammo += a0;
  if (levels&0x02) haa->haas[HMP].ammo += a1;
  if (levels&0x04) haa->haas[UV].ammo += a2;
  if ((ammotype==ID_SHOTGUN)||(ammotype==ID_SSGUN)) {
    if (levels&0x01) haa->haas[ITYTD].can_use_shells = 1;
    if (levels&0x02) haa->haas[HMP].can_use_shells = 1;
    if (levels&0x04) haa->haas[UV].can_use_shells = 1;
  }
  if (ammotype==ID_CHAINGUN) {
    if (levels&0x01) haa->haas[ITYTD].has_chaingun = 1;
    if (levels&0x02) haa->haas[HMP].has_chaingun = 1;
    if (levels&0x04) haa->haas[UV].has_chaingun = 1;
  }
  if (ammotype==ID_PLASMA) {
    if (levels&0x01) haa->haas[ITYTD].can_use_cells = 1;
    if (levels&0x02) haa->haas[HMP].can_use_cells = 1;
    if (levels&0x04) haa->haas[UV].can_use_cells = 1;
  }
  if (ammotype==ID_LAUNCHER) {
    if (levels&0x01) haa->haas[ITYTD].can_use_rockets = 1;
    if (levels&0x02) haa->haas[HMP].can_use_rockets = 1;
    if (levels&0x04) haa->haas[UV].can_use_rockets = 1;
  }
  if (ammotype==ID_SSGUN) {
    if (levels&0x01) haa->haas[ITYTD].has_ssgun = 1;
    if (levels&0x02) haa->haas[HMP].has_ssgun = 1;
    if (levels&0x04) haa->haas[UV].has_ssgun = 1;
  }
}

/* Is this thingid a weapon?  Should use config! */
boolean is_weapon(short thingid)
{
  switch (thingid) {
    case ID_SHOTGUN:
    case ID_SSGUN:
    case ID_CHAINGUN:
    case ID_CHAINSAW:
    case ID_PLASMA:
    case ID_BFG:
    case ID_LAUNCHER:
      return TRUE;
    default: return FALSE;
  }
}

/* Maybe place some ammo, update the haa */
void place_ammo(level *l,sector *s,config *c,haa *haa)
{
  int levels = 0;
  short ammotype;

  if (c->allow_boring_rooms&&rollpercent(10)) return;

  for (;;) {
    announce(NONE,"place_ammo looking for needy levels");
    ammotype = timely_ammo(haa,&levels,c);
    if (levels==0) return;   /* Done if none */
    announce(NONE,"place_ammo found some needy levels");
    /* The 48 is just to avoid bunching-up and wall-illusions, */
    /* as well as the grab-through-wall effect. */
    if (NULL==place_object(l,s,c,ammotype,48,0,0,0,levels)) return;
    announce(NONE,"place_ammo placed some ammo");
    if (levels==7) if (is_weapon(ammotype)) s->has_dm_weapon = TRUE;
    update_haa_for_ammo(haa,levels,ammotype);
    if (rollpercent(20)) return;   /* Reasonable? */
  }  /* end forever */

}

/* Update the haa in the obvious way */
void update_haa_for_health(haa *haa,int levels,short healthtype)
{
  int amount;

  if (healthtype==ID_BERSERK) {
    announce(VERBOSE,"Put in a berserk pack!");
    if (levels&0x01) {
      if (haa->haas[ITYTD].health<100)
        haa->haas[ITYTD].health=(float)100;
      haa->haas[ITYTD].has_berserk = TRUE;
    }
    if (levels&0x02) {
      if (haa->haas[HMP].health<100)
        haa->haas[HMP].health=(float)100;
      haa->haas[HMP].has_berserk = TRUE;
    }
    if (levels&0x04) {
      if (haa->haas[UV].health<100)
        haa->haas[UV].health=(float)100;
      haa->haas[UV].has_berserk = TRUE;
    }
  } else {
    switch (healthtype) {
      case ID_STIMPACK: amount= 10; break;
      case ID_MEDIKIT:  amount= 25; break;
      case ID_POTION:   amount=  1; break;
      case ID_SOUL:     amount=100; break;
      default: announce(WARNING,"Odd healthtype in u_h_f_h");
        amount=0;
    }
    if (levels&0x01) haa->haas[ITYTD].health += amount;
    if (levels&0x02) haa->haas[HMP].health += amount;
    if (levels&0x04) haa->haas[UV].health += amount;
  }
}

/* Return a random kind of ordinary health-bonus for those levels */
/* that need some health.  If *levels comes back as zero, return */
/* value is undefined. */
short timely_health(haa *haa,int *levels,config *c)
{
  int i;
  boolean berserk_ok = FALSE;
  short healthtype;

  /* See which levels need more */
  for ((*levels)=0,i=0;i<3;i++) {  /* for each hardness level */
    (*levels) >>= 1;
    if (haa->haas[i].health < c->usualhealth[i]) (*levels) |= 0x04;
    if (haa->haas[i].has_berserk==FALSE) berserk_ok = TRUE;
  }

  if ((*levels)==0) return 0;

  if (rollpercent(50)) {
    healthtype = ID_STIMPACK;
  } else if (rollpercent(50)) {
    healthtype = ID_MEDIKIT;
  } else if (rollpercent(90)) {
    healthtype = ID_POTION;
  } else if (berserk_ok&&rollpercent(50)) {
    healthtype = ID_BERSERK;
  } else {
    healthtype = ID_SOUL;
  }
  return healthtype;
}

/* Maybe place some health boni, update the haa */
void place_health(level *l,sector *s,config *c,haa *haa)
{
  int levels = 0;
  short healthtype;

  /* Coming along.  Might want to create effects around SOULs etc, eh? */

  if (c->allow_boring_rooms&&rollpercent(10)) return;

  for (;;) {

    healthtype = timely_health(haa,&levels,c);

    if (levels==0) return;   /* Done if none */

    /* The 48 is just to avoid bunching-up and wall-illusions */
    if (NULL==place_object(l,s,c,healthtype,48,0,0,0,levels)) return;

    update_haa_for_health(haa,levels,healthtype);

    if (rollpercent(20)) return;   /* Reasonable? */

  }  /* end forever */

}  /* end place_health */

/* Probably put some random bonus that the player needs at */
/* the given location, and update the haa accordingly. */
void place_timely_something(level *l,haa *haa, config *c,int x, int y)
{
  int thingtype, levels;

  switch (roll(5)) {
    case 0:  /* Armor */
      thingtype = timely_armor(haa,&levels,c);
      if (levels==0) return;   /* Done if none */
      new_thing(l,x,y,0,(short)thingtype,(short)levels,c);
      update_haa_for_armor(haa,levels,(short)thingtype);
      return;
    case 1:  /* Ammo/weapons */
    case 2:
      thingtype = timely_ammo(haa,&levels,c);
      if (levels==0) return;   /* Done if none */
      new_thing(l,x,y,0,(short)thingtype,(short)levels,c);
      update_haa_for_ammo(haa,levels,(short)thingtype);
      return;
    case 3:  /* Health */
    case 4:
      thingtype = timely_health(haa,&levels,c);
      if (levels==0) return;   /* Done if none */
      new_thing(l,x,y,0,(short)thingtype,(short)levels,c);
      update_haa_for_health(haa,levels,(short)thingtype);
      return;
  }
  return;   /* Unreachable */
}

/* Return the size of monster, and the difficulty levels, that's due */
/* in the current user-model (the haa).                              */
boolean haa_monster_data(haa *haa,config *c, float *monster_size_health,
                         float *monster_size_ammo,int *levels)
{
  float excess_health;
  float available_ammo;
  int i;

  /* Determine what size monster we want */
  *levels = 0;
  *monster_size_health = (float)10000;
  *monster_size_ammo = (float)10000;
  for (i=0;i<3;i++) {
    *levels >>=  1;   /* Shift the bits over */
    excess_health = haa->haas[i].health - c->minhealth[i];
    if (excess_health>0) {
      *levels |= 0x04;   /* Set the bit */
      /* Can take more damage if armored */
      if (excess_health<haa->haas[i].armor) {
        excess_health += excess_health;
      } else {
        excess_health += haa->haas[i].armor;
      }
      /* -Will- take more if no good weapons */
      if (!(haa->haas[i].can_use_shells||haa->haas[i].can_use_cells))
        excess_health /= 2;
      if (excess_health<*monster_size_health)
        *monster_size_health = excess_health;
      available_ammo = haa->haas[i].ammo;
      /* If wimpy weapons, will use more ammo */
      if (!(haa->haas[i].can_use_shells||haa->haas[i].can_use_cells))
        available_ammo /= 2;
      if (haa->haas[i].ammo<*monster_size_ammo)
        *monster_size_ammo = haa->haas[i].ammo;
    }  /* end this level has excess health */
  }  /* end for difficulty levels determining limits */
  *monster_size_health += (float)5;  /* A little leeway */
  if (*levels==0) return FALSE;  /* No excess health anywhere */
  return TRUE;
}

/* Find a monster that fits the given health and ammo allowance, */
/* for the given apearence bits.  If none, return the monster    */
/* that's the easiest to kill.  Never return null!               */
genus *proper_monster(float health,float ammo,int bits,haa *haa,
                      int mno,propertybits require,propertybits forbid,
                      boolean biggest, config *c)
{
  genus *m, *m1, *m0, *mx, *my;
  float damage,ammo0,bx;
  int i,count;
  float hl, am;
  int thisbit;

  {
  char s[200];
  sprintf(s,"proper_monster looking for %f health, %f ammo, levels %d",
    health, ammo, bits);
  announce(NONE,s);
  }

  require |= MONSTER;                         /* Duh! */
  forbid |= BOSS;                             /* No wandering bosses */
  if (!c->big_monsters) forbid |= BIG;

  /* Mark eligible monsters, and find wimpiest and biggest just in case */
  count = 0;
  ammo0 = (float)10000;
  m0 = NULL;
  mx = NULL;
  my = NULL;
  bx = (float)0;
  for (m=c->genus_anchor;m;m=m->next) {
    if ((m->bits & require) != require) continue;
    if ((m->bits & forbid) != 0) continue;
#ifdef IMPOSSIBLE_MONSTERS_IN_CONFIG
    if ((m->gamemask&c->gamemask)!=c->gamemask) continue;
#endif
    if (m0==NULL) m0 = m;
    m->marked = 1;
    for (i=0,thisbit=1;(i<3)&&(m->marked);i++,thisbit<<=1) {
      if (!(thisbit&bits)) continue;
      /* If we don't have any good weapons, we'll take more */
      /* damage and use more ammo, so halve the limits.     */
      /* Include chaingun here, too? */
      if (!(haa->haas[i].can_use_shells||haa->haas[i].can_use_cells)) {
        hl = health / 2;
        am = ammo / 2;     /* This may not be reasonable / necessary */
      } else {
        hl = health;
        am = ammo;
      }
      if (mno) {
        damage = m->damage[i];
      } else {
        damage = m->altdamage[i];
      }
      if (damage>hl) m->marked=0;
      if (m->ammo_to_kill[i]>am) m->marked=0;
      if (m->ammo_to_kill[i]<ammo0) {
        m0 = m;
        ammo0 = m->ammo_to_kill[i];
      }
    }  /* end for levels */
    if (m->marked) {
      count++;
      if (m->ammo_to_kill[0]+m->damage[0] > bx) {
        my = mx;
        mx = m;
        bx = m->ammo_to_kill[0]+m->damage[0];
      }
    }
  }  /* end for m over monsters */
  if (count==0) {   /* Put down the wimpiest monster */
    m = m0;
  } else if (biggest) {     /* Put down the biggest monster */
    m = mx;
    if (my) if (rollpercent(40)) m = my;   /* Or the second-biggest */
  } else {          /* Choose a random one */
    count = 1 + roll(count);
    for (m=c->genus_anchor;m;m=m->next) {
      if (m->marked) count--;
      if (count==0) break;
    }
  }

  /* Unmark monsters */
  for (m1=c->genus_anchor;m1;m1=m1->next) m1->marked = 0;

  return m;
}

/* This says that the current battle is over, so any pending */
/* weapon-pickups can occur.  If we ever want to be kind, we */
/* can defer the ammo to this point as well. */
void haa_unpend(haa *haa)
{
  int i;

  for (i=ITYTD;i<=UV;i++) {
    if (haa->haas[i].shells_pending) {
      haa->haas[i].can_use_shells = TRUE;
      haa->haas[i].shells_pending = FALSE;
    }
    if (haa->haas[i].chaingun_pending) {
      haa->haas[i].has_chaingun = TRUE;
      haa->haas[i].chaingun_pending = FALSE;
    }
  }
}

/* This makes the model assume that, unlike in-room goodies, ammo */
/* from monsters is taken at once.  challenging!   Weapons, on the */
/* other hand, just go into the _pending bits, for haa_unpend(). */
void update_haa_for_monster(haa *haa,genus *m,int levels,int mno,config *c)
{
  int i, thisbit;
  float damage;

  for (i=0,thisbit=1;i<3;i++,thisbit<<=1) {
    if (!(thisbit&levels)) continue;
    if (mno) {
      damage = m->damage[i];
    } else {
      damage = m->altdamage[i];
    }
    if (!(haa->haas[i].can_use_shells||haa->haas[i].can_use_cells))
      damage *= 2;
    if (damage > 2*haa->haas[i].armor) {
      haa->haas[i].health += haa->haas[i].armor;
      haa->haas[i].armor = (float)0;
      haa->haas[i].health -= damage;
    } else {
      haa->haas[i].health -= damage/2;
      haa->haas[i].armor -= damage/2;
    }
    if (haa->haas[i].health<0) announce(VERBOSE,"Health estimate negative?");
    damage = m->ammo_to_kill[i];
    /* Takes more ammo, if no good weapons */
    if (!(haa->haas[i].can_use_shells||haa->haas[i].can_use_cells))
      damage *= 2;
    /* But less ammo if we can saw it or punch it! */
    if (haa->haas[i].has_chainsaw&&!(m->bits&(FLIES|SHOOTS))) {
      damage /= 2;
    } else if (haa->haas[i].has_berserk&&!(m->bits&(FLIES|SHOOTS))) {
      damage *= (float)0.80;
    }
    haa->haas[i].ammo -= damage;
    haa->haas[i].ammo += m->ammo_provides;  /* Should be in stage two? */
    if (haa->haas[i].ammo<0) announce(VERBOSE,"Ammo estimate negative?");
    if (m->thingid == ID_SERGEANT) haa->haas[i].shells_pending = TRUE;
    if (m->thingid == ID_COMMANDO) haa->haas[i].chaingun_pending = TRUE;

  }  /* end for levels adjusting haa */

}   /* end update_haa_for_monster */

/* Return a monster that there's room for in the model now.  */
genus *timely_monster(haa *haa,config *c,int *levels,boolean biggest,
                      int mno)
{
  /* Should just be a macro, eh? */
  return timely_monster_ex(haa,c,levels,biggest,mno,0);  /* no extra reqs */
}

/* Return a monster that there's room for in the model now, with */
/* some required bits set. */
/* Should really take into account the _size_ of the place you're */
/* planning to put the monster, eh? */
genus *timely_monster_ex(haa *haa,config *c,int *levels,boolean biggest,
                      int mno,propertybits req)
{
  float monster_size_health;
  float monster_size_ammo;

  /* Find how big a monster we can tolerate */
  if (!haa_monster_data(haa,c,
                        &monster_size_health,&monster_size_ammo,levels))
    return NULL;   /* Not enough excess health in any level */

  /* Find a monster of that size */
  return proper_monster(monster_size_health,monster_size_ammo,*levels,haa,mno,
                        c->required_monster_bits+req,
                        c->forbidden_monster_bits,
                        biggest,c);
}

/* Maybe add some monsters, update the haa */
void place_monsters(level *l,sector *s,config *c,haa *haa)
{
  int mno,n;
  int levels;
  genus *m, *lastm;
  boolean rc;

  /* Decide on a limit, if any, for the monster loop; should be config/style? */
  if (c->allow_boring_rooms&&rollpercent(20)) return;  /* No monsters at all */
  if (rollpercent(80)) {
    n = 2 + roll(8);  /* N to M monsters */
  } else {
    n = 1000;     /* As many monsters as will fit! */
  }
  n *= l->hugeness;   /* Reasonable? */

  /* The loop itself */
  for (lastm=NULL,mno=0;mno<n;mno++) {

    m = timely_monster(haa,c,&levels,rollpercent(l->p_biggest_monsters),mno);
    if (!m) return;
    if (lastm)
      if (rollpercent(c->homogenize_monsters))
        m = lastm;  /* Yoiks, dangerous! */
    lastm = m;

    if (rollpercent(15)) levels |= 0x08;   /* deaf */

    announce(NONE,"Trying to place a monster");

    /* Try to place it */
    rc = (NULL!=place_object(l,s,c,m->thingid,MONSTER_WIDTH(m),-1,
          s->entry_x,s->entry_y,levels));
    if (!rc) {
      announce(NONE,"Placement failed");
      goto done_monsters;   /* Might as well give up entirely */
    }

    if (m->thingid == ID_SKULL) announce(NONE,"Skull");
    if (m->thingid == ID_HEAD) announce(VERBOSE,"HEAD");
    if (m->thingid == ID_SKEL) announce(VERBOSE,"SKEL");
    if (m->thingid == ID_HELL) announce(VERBOSE,"KNIGHT");
    if (m->thingid == ID_ARCHIE) announce(VERBOSE,"VILE");

    update_haa_for_monster(haa,m,levels,mno,c);

  }  /* end for a long time */

done_monsters:

  /* NOTE: tempting as it is, we don't do this *within* the */
  /* loop, because the model is that the player doesn't pick */
  /* up the objects in the room (including dropped weapons) */
  /* until after everything is dead.  So we don't use the */
  /* new values of these data until we've placed all the */
  /* monsters for a room.  On the other hand, the effects */
  /* of the can_use_x variables are sort of down in the */
  /* noise, so it may not be worth *too* much effort to */
  /* get this exactly right... */
  haa_unpend(haa);

  return;  /* all done */

}  /* end place_monsters */


/* Boy is this primitive!  On the other hand, other checks later */
/* mean that this can probably be somewhat optimistic.           */
boolean isAdequate(level *l,linedef *ld,style *ThisStyle,config *c)
{
  /* Assume all 1S longish linedefs are OK; very dangerous! */
  if (ld->left) return 0;
  if (ld->flags & TWO_SIDED) return 0;
  if (lengthsquared(ld)>=(128*128)) {  /* Why 128? */
    return 1;
  } else {
    return 0;
  }
}

/* Fill in the default config-file data contents stuff */
void load_default_config(config *c)
{
  char *p;
  c->configdata = strdup(    /* So we can free() it */
    "[THEMES] T W T M T B T R ? "
    "t PANEL5 0 1 "
    "t PANCASE2 0 1 "
    "t PANCASE1 0 1 "
    "t PANBORD2 0 1 "
    "t PANBORD1 0 1 "
    "t METAL7 0 1 "
    "t METAL6 0 1 "
    "t METAL2 0 1 "
    "t COMP2 2 "
    "t SILVER2 0 1 "
    "t SILVER1 0 1 "
    "t EXITSIGN X "
    "t ZZWOLF1 o 0 1 "
    "t ZIMMER3 o 0 1 "
    "t ZIMMER5 o 0 1 "
    "t TANROCK5 o 0 1 "
    "t TANROCK4 o 0 1 "
    "t TANROCK2 o 0 1 "
    "t STUCCO o 0 1 "
    "t STONE6 o 0 1 "
    "t ROCK1 o 0 1 "
    "t MODWALL1 o 0 1 "
    "t BSTONE1 o 0 1 "
    "t BRICK5 o 0 1 "
    "t BRICK4 o 0 1 "
    "t ASHWALL7 o 0 1 "
    "t ASHWALL6 o 0 1 "
    "t ASHWALL4 o 0 1 "
    "t ASHWALL2 o 0 1 "
    "t STONE3 o "
    "t SP_ROCK1 o "
    "t GRAYVINE o "
    "t GRAYBIG o "
    "t SLDOOR1 z 64 128 d L c M c B = SP_DUDE5 u "
    "t DOORSKUL z 64 72 d L c M c B 0 1 u "
    "t TEKBRON2 z 64 128 d c M c B 0 1 "
    "t SPCDOOR4 z 64 128 d c M 0 1 "
    "t SPCDOOR3 z 64 128 d c M 0 1 "
    "t SPCDOOR2 z 64 128 d c M 0 1 "
    "t SPCDOOR1 z 64 128 d c M 0 1 "
    "t DOORHI z 64 128 d c M 2 "
    "t DOOR3 z 64 72 d c M "
    "t DOOR1 z 64 72 d c M "
    "t WOODSKUL z 64 128 d c W c B c R 2 "
    "t WOODMET2 z 64 128 d c W c B c R 0 1 "
    "t WOODGARG z 64 128 d c W c B c R "
    "t BIGDOOR4 z 128 128 d c M "
    "t BIGDOOR3 z 128 128 d c M "
    "t BIGDOOR2 z 128 128 d c M "
    "t BIGDOOR1 z 128 96 d c M "
    "t BIGDOOR7 z 128 128 d c W c B c R "
    "t BIGDOOR6 z 128 112 d c W c B c R "
    "t BIGDOOR5 z 128 128 d c W c B c R "
    "t EXITSWIR E c R 0 1 u "
    "t EXITSWIW E c W c B 0 1 u "
    "t EXITSWIT E c M 0 1 u "
    "t BFALL1 z 8 128 l c R 0 1 "
    "t LITEREDL z 8 128 l c R = LITERED 2 "
    "t TEKLITE l c M 0 1 "
    "t LITE4 l c M c B 2 "
    "t LITE5 l c M c B "
    "t LITE3 l c M c B "
    "t SILVER3 p v c M 0 1 "
    "t SPACEW3 p v c M 0 1 "
    "t COMPSTA2 p v H c M "
    "t COMPSTA1 p v H c M "
    "t COMPTALL p v c M "
    "t COMPUTE1 p v H c M 2 "
    "t PLANET1 p v H c M 2 "
    "t PANBOOK p c W 0 1 "
    "t SKIN2 p v c R "
    "t GSTFONT1 p c R "
    "t SKY1 p c W c B "
    "t SKY3 p c B c R "
    "t MARBFAC3 p v c W c B "
    "t MARBFAC2 p v c W c B "
    "t MARBFACE p v c W c B "
    "t BRNBIGC g c M 2 "
    "t MIDSPACE g c M 0 1 "
    "t MIDVINE1 g c W c M c B c R 2 "
    "t MIDBARS1 g c W c M c B c R 0 1 "
    "t MIDGRATE g c W c M c B c R "
    "t LITERED z 8 128 r c M c B 2 "
    "t DOORYEL z 8 128 y c M c B "
    "t DOORRED z 8 128 r c M c B "
    "t LITEBLU4 z 16 128 b c M c B "
    "t LITEBLU1 z 8 128 b c M c B "
    "t DOORBLU z 8 128 b c M c B "
    "t DOORYEL2 z 16 128 y c W c R "
    "t DOORBLU2 z 16 128 b c W c R "
    "t DOORRED2 z 16 128 r c W c R "
    "t STEP6 z 256 16 e c W c M c B "
    "t STEP5 z 256 16 e c W c M c B "
    "t STEP4 z 256 16 e c W c M c B "
    "t STEP3 z 256 8 e c W c M c B "
    "t STEP2 z 256 8 e c W c M c B "
    "t STEP1 z 256 8 e c W c M c B "
    "t FIRELAVA j c R "
    "t DOORTRAK j c W c M c B "
    "t DOORSTOP j c W c M c B "
    "t SKSNAKE2 I c R "
    "t ROCKRED1 I c R "
    "t ZIMMER7 I c B 0 1 "
    "t BRICK10 I o c B 0 1 "
    "t COMPSPAN I c M "
    "t SUPPORT2 I c M c B "
    "t SHAWN2 I c M c B "
    "t ASHWALL3 I o c W 0 1 "
    "t ASHWALL I o c W 2 "
    "t BROWNHUG I o c W c M c B "
    "t SUPPORT3 I c W c B "
    "t METAL z 64 128 I d c W c B c R "
    "t SW1HOT i c R "
    "t SKY3_W w c R = SKY3 "
    "t SP_HOT1 w C R "
    "t REDWALL w ! C R "
    "t FIREMAG1 p c B "
    "t FIREBLU1 w C R S FIREMAG1 @ 0 "
    "t SW1MARB i c B 0 1 "
    "t SW1GSTON i c B "
    "t GSTVINE1 w o c B S GSTVINE2 "
    "t MARBGRAY w c B S GRAY5 0 1 "
    "t GSTONE1 w o c B S GSTGARG "
    "t MARBGARG w c B S MARBLE1 0 1 u "
    "t MARBLE2 w C B S MARBLE3 "
    "t MARBLE3 w C B S MARBLE1 "
    "t MARBLE1 w C B S MARBLE3 @ 0 "
    "t PLAT1 z 128 128 F c M "
    "t GRAYALT w C M s SW1GRAY 0 1 u "
    "t TEKVINE w c M S TEKGREN3 s SW1TEK @ 0 0 1 u "
    "t SPACEW4 w c M s SW1TEK 0 1 "
    "t SW1MET2 Y 64 "
    "t METAL3 0 1 "
    "t METAL5 w c M S METAL3 s SW1MET2 0 1 "
    "t COMPUTE3 w c M s SW1STRTN 2 "
    "t TEKWALL4 w c M S COMPWERD s SW1COMP @ 0 "
    "t TEKWALL1 w c M S COMPWERD s SW1COMP @ 0 "
    "t ICKWALL3 o "
    "t GRAY1 w c M S ICKWALL3 s SW1GRAY "
    "t BROVINE2 w c M s SW1SLAD @ 24 "
    "t BRONZE4 w C M S BRONZE3 s SW1TEK 0 1 "
    "t STARTAN1 w C M S STARTAN2 s SW1STRTN 2 "
    "t STARTAN3 w C M S STARG3 s SW1STRTN "
    "t LITEMET 2 "
    "t METAL1 w c M S LITEMET s SW1METAL "
    "t STARBR2 w c M S STARTAN2 s SW1STRTN "
    "t STARTAN2 w C M S STARBR2 s SW1STRTN "
    "t STARG3 w C M S STARGR1 s SW1STRTN "
    "t STARG2 w C M S STARG1 s SW1STRTN "
    "t STARG1 w C M S STARG2 s SW1STRTN "
    "t SW1DIRT Y 72 "
    "t BROWN144 o "
    "t BROWN96 w C M S BROWN144 s SW1DIRT "
    "t BROWN1 w C M s SW1BRN2 "
    "t BROWNGRN w C M S SLADWALL s SW1BRNGN "
    "t SLADWALL w C M S BROWNGRN s SW1SLAD "
    "t SW1WOOD i c W c B "
    "t SW1SATYR i c W "
    "t SW1LION i c W "
    "t SW1GARG i c W "
    "t WOOD12 w c W @ 3 0 1 "
    "t SLOPPY2 w c W S SLOPPY1 @ 0 0 1 Q "
    "t SKULWALL w c W S SKULWAL3 @ 0 2 Q "
    "t SKINSYMB w c W S SKINMET1 "
    "t SKINMET2 w c W S SKINMET1 "
    "t SKINMET1 w c W S SKINMET2 "
    "t PIPE2 w j c W c M S PIPE4 s SW1WOOD @ 0 "
    "t WOODVINE w c W S WOOD9 @ 0 0 1 u "
    "t WOOD4 w c W @ 64 "
    "t WOOD9 w C W S WOOD7 @ 0 0 1 "
    "t WOOD5 w C W S WOOD1 "
    "t WOOD3 w C W S WOOD1 @ 3 "
    "t WOOD1 w C W S WOOD3 "
    "f FWATER1 W "
    "f F_SKY1 K "
    "f SLGATE1 G c W c M c B c R u "
    "f GATE4 G c W c M c B c R "
    "f GATE3 G c W c M c B c R "
    "f GATE2 G c W c M c B c R "
    "f GATE1 G c W c M c B c R "
    "f SLGRASS1 o u "
    "f SLIME13 o 0 1 "
    "f RROCK19 o 0 1 "
    "f RROCK16 o 0 1 "
    "f RROCK11 o 0 1 "
    "f GRNROCK o 0 1 "
    "f GRASS2 o 0 1 "
    "f GRASS1 o 0 1 "
    "f MFLR8_4 o "
    "f MFLR8_3 o "
    "f MFLR8_2 o "
    "f FLAT5_7 o "
    "f FLAT10 o "
    "f GRNLITE1 U l c B 0 1 "
    "f FLOOR7_2 U c B "
    "f SLLITE1 U l c M u "
    "f TLITE6_6 U l c M "
    "f FLOOR7_1 U o c M "
    "f FLOOR5_2 U c M "
    "f FLOOR5_1 U c M "
    "f CEIL3_1 U c M "
    "f FLOOR5_4 U o c W "
    "f FLOOR4_6 U c W "
    "f CEIL3_3 U c W "
    "f CEIL1_1 U c W "
    "f LAVA1 n c R "
    "f SLSPARKS D c R u "
    "f SFLR6_4 D U c R "
    "f TLITE6_5 U l c R "
    "f FLOOR6_1 D U r c R "
    "f FLOOR1_7 U l c R "
    "f FLOOR1_6 D U r c R "
    "f FLAT5_3 D U r c R "
    "f SLFLAT01 D c B u "
    "f FLAT1 D c B "
    "f DEM1_6 D c B "
    "f DEM1_5 D U c B "
    "f NUKAGE1 n c M c B "
    "f FLOOR4_1 D c M "
    "f FLOOR3_3 D U c M c B "
    "f FLOOR0_2 D c M "
    "f FLOOR0_1 D o c M "
    "f FLAT1_2 D o c M "
    "f FLAT5 D c M "
    "f SLIME09 n c W 0 1 "
    "f BLOOD1 n r c W c B c R "
    "f FLAT8 D c W "
    "f FLAT5_2 D c W "
    "f FLAT5_1 D U c W c B "
    "f FLAT1_1 D o c W "
    "f CEIL5_2 D o c W "
    "f MFLR8_1 D c W "
    "x m 4 h 128 c W c B 0 1 "
    "O FLAT5_1 O CRATOP2 O CEIL5_2 O CEIL3_3 O CEIL1_1 "
    "B PANEL5 ~ 64 "
    "B PANCASE2 ~ 64 "
    "B PANCASE1 ~ 64 "
    "B PANBORD2 ~ 16 "
    "B PANBORD1 ~ 32 "
    "A PANBOOK ~ 64 "
    "x m 3 h 64 c W c M "
    "O CRATOP2 "
    "A CRATWIDE ] 64 64 "
    "A CRATE1 ~ 64 "
    "x m 3 h 64 c W c M "
    "O CRATOP1 "
    "A CRATWIDE "
    "A CRATE2 ~ 64 "
    "x m 3 h 64 c W c M "
    "O CRATOP1 "
    "A CRATELIT ~ 32 "
    "x m 3 h 32 c W c M "
    "O CRATOP1 "
    "A CRATELIT ~ 32 "
    "x m 3 h 16 c W c M "
    "O CRATOP1 "
    "A CRATINY ~ 16 "
    "x m 2 h 128 c M "
    "O CEIL5_1 O FLAT4 O TLITE6_1 "
    "B METAL7 ] 0 64 ~ 64 "
    "B METAL6 ] 0 64 ~ 64 "
    "B METAL5 ] 0 64 ~ 64 "
    "B METAL3 ] 0 64 ~ 64 "
    "B METAL2 ] 0 64 ~ 64 "
    "B COMPWERD ~ 64 "
    "B COMPSPAN ~ 16 "
    "A SPACEW3 ~ 64 "
    "A COMPTALL ~ 256 "
    "A COMP2 ~ 256 "
    "x m 2 h 64 c M "
    "O CEIL5_1 O FLAT4 O TLITE6_1 "
    "B METAL7 ] 0 64 ~ 64 "
    "B METAL6 ] 0 64 ~ 64 "
    "B METAL5 ] 0 64 ~ 64 "
    "B METAL3 ] 0 64 ~ 64 "
    "B METAL2 ] 0 64 ~ 64 "
    "B COMPWERD ~ 64 "
    "B COMPSPAN ~ 16 "
    "A SPACEW3 ] 0 64 ~ 64 "
    "A COMPTALL ] 0 64 ~ 256 "
    "A COMP2 ] 0 64 ~ 256 "
    "x m 1 h 128 c M c B "
    "O FLAT9 O FLAT4 O FLAT23 O FLAT19 O FLAT18 O CRATOP1 O COMP01 "
    "B SILVER2 ~ 64 "
    "B SILVER1 ~ 64 "
    "B SUPPORT2 ~ 16 "
    "B SHAWN2 ~ 16 "
    "A SILVER3 "
    "A COMPUTE1 ] 0 64 "
    "x m 1 h 64 c M c B "
    "O FLAT9 O FLAT4 O FLAT23 O FLAT19 O FLAT18 O CRATOP1 O COMP01 "
    "B SUPPORT2 ~ 16 "
    "B SHAWN2 ~ 16 "
    "A COMPUTE1 ] 0 64 "
    "A COMPSTA2 "
    "A COMPSTA1 "
    ". 2035 c M "
    ". 34 c W c M c B "
    ". 44 c W c B "
    ". 45 c W c B "
    ". 46 c W c B c R "
    ". 55 c W c B "
    ". 56 c W c B "
    ". 57 c W c B c R "
    ". 48 c M "
    ". 2028 c M "
    ". 85 c M "
    ". 86 c M "
    ". 70 c M c W "
    ". 35 c W "
    "# ");
  for (p=c->configdata;*p;p++)
    if (*p==' ') *p = '\0';
  return;
}

/* Make the config-file data accessible */
void load_config(config *c)
{
  FILE *f;
  char thisline[200];
  char *inc, *outc;
  long flen;
  boolean blankmode = TRUE;   /* Strip leading blanks */

  f = fopen(c->configfile,"rb");
  if (f==NULL) {
    fprintf(stderr,"Could not open %s; using default configuration.\n",
      c->configfile);
    load_default_config(c);
  } else {
    fseek(f,0,SEEK_END);
    flen = ftell(f);
    fseek(f,0,SEEK_SET);
    c->configdata = (char *)malloc(5+flen);
    outc = c->configdata;
    printf("Loading %s...\n",c->configfile);
    for (;;) {
      fgets(thisline,190,f);
      if (feof(f)) break;
      if (strlen(thisline)>180) {
        fprintf(stderr,"Line too long in %s: %s\n",c->configfile,thisline);
        exit(110);
      }
      for (inc=thisline;*inc;inc++) {
        if (*inc==';') break;
        if (strchr(" \t\n\r",*inc)) {
          if (!blankmode) *(outc++) = '\0';
          blankmode = TRUE;
        } else {
          *(outc++) = *inc;
          blankmode = FALSE;
        }
      }  /* Done with line */
    }  /* Done reading file */
    if (!blankmode) *(outc++) = '\0';
    *(outc++) = '\0';
    printf("Loaded.\n");
    c->configdata = (char *)realloc(c->configdata,1+(outc-c->configdata));
    fclose(f);
  }
#if 0
  for (inc=c->configdata;*inc;inc+=1+strlen(inc))
    printf("%s ",inc);
#endif
  return;
}

/* Free up config-file resources */
void unload_config(config *c)
{
  if (c->configdata) free(c->configdata);
  c->configdata = NULL;
  return;
}

/* Look through the config's config file, and fill in values for */
/* the switch lines therein.  Return FALSE if error.  These are  */
/* of course overridable by command-line switches.               */
boolean read_switches(config *c)
{
  /* Dis here is a STUB */
  return TRUE;
}

/* Allocate and return a new, empty construct */
construct *new_construct(config *c)
{
  construct *answer = (construct *)malloc(sizeof(*answer));

  answer->height = 64;
  answer->gamemask = DOOM1_BIT|DOOM0_BIT|DOOM2_BIT|DOOMI_BIT|DOOMC_BIT;
  answer->compatible = 0;
  answer->texture_cell_anchor = NULL;
  answer->flat_cell_anchor = NULL;
  answer->family = 0;
  answer->marked = FALSE;
  answer->next = c->construct_anchor;
  c->construct_anchor = answer;
  return answer;
}

flat_cell *add_flat_cell(construct *cn,char *name,config *c)
{
  flat_cell *answer = (flat_cell *)malloc(sizeof(*answer));

  answer->flat = find_flat(c,name);
  answer->next = cn->flat_cell_anchor;
  cn->flat_cell_anchor = answer;
  return answer;
}

texture_cell *add_texture_cell(construct *cn,char *name,boolean primary,
                               short y1, short y2,config *c)
{
  texture_cell *answer = (texture_cell *)malloc(sizeof(*answer));

  answer->texture = find_texture(c,name);
  answer->width = 128;    /* A nicer default, as it happens */
  answer->y_offset1 = y1;
  answer->y_offset2 = y2;
  answer->primary = primary;
  answer->marked = FALSE;
  answer->next = cn->texture_cell_anchor;
  cn->texture_cell_anchor = answer;
  return answer;
}

#ifdef CONFIG_DUMP

#ifndef CONFIG_DUMP_VERBOSE_NOT

void dump_foo_themebits(themebits yes, themebits no, char *tag, config *c)
{
  themebits mask = 1;
  theme *t = c->theme_anchor;

  for (t=c->theme_anchor;t;t=t->next) {
    if ((yes&mask) && !(no&&mask)) printf("%s %s ",tag,t->name);
    mask <<=1;
  }
}

void dump_foo_texture(texture *t,config *c)
{
  printf("Texture %s ",t->name);
  if ((t->width!=256)||(t->height!=128))
    printf("size %d %d ",t->width,t->height);
  if (t->props&WALL) printf("wall ");
  if (t->props&SWITCH) printf("isswitch ");
  if (t->props&LIFT_TEXTURE) printf("lift ");
  if (t->props&SUPPORT) printf("support ");
  if (t->props&JAMB) printf("jamb ");
  if (t->props&STEP) printf("step ");
  if (t->props&GRATING) printf("grating ");
  if (t->props&PLAQUE) printf("plaque ");
  if (t->props&VTILES) printf("vtiles ");
  if (t->props&HALF_PLAQUE) printf("half_plaque ");
  if (t->props&LIGHT) printf("light ");
  if (t->props&EXITSWITCH) printf("exitswitch ");
  if (t->props&DOOR) printf("door ");
  if (t->props&GATE) printf("locked ");
  if (t->props&OUTDOOR) printf("outside ");
  if (t->props&RED) printf("red ");
  if (t->props&BLUE) printf("blue ");
  if (t->props&YELLOW) printf("yellow ");
  if (t==c->error_texture) printf("error ");
  if (t==c->gate_exitsign_texture) printf("gateexitsign ");
  dump_foo_themebits(t->core,0,"core",c);
  dump_foo_themebits(t->compatible,t->core,"comp",c);
  if (t->subtle)
    printf("subtle %s ",t->subtle->name);
  if (t->switch_texture)
    printf("switch %s ",t->switch_texture->name);
  if (t->realname != t->name)
    printf("realname %s ",t->realname);
  if (t->y_bias) printf("ybias %d ",t->y_bias);
  if (5!=t->y_hint) printf("yhint %d ",t->y_hint);
  if (!(t->gamemask&DOOM0_BIT)) printf("noDoom0 ");
  if (!(t->gamemask&DOOM1_BIT)) printf("noDoom1 ");
  if (!(t->gamemask&DOOM2_BIT)) printf("noDoom2 ");
  if (!(t->gamemask&DOOMI_BIT)) printf("custom " );
  if (!(t->gamemask&DOOMC_BIT)) printf("gross ");
  printf("\n");

}

void dump_foo_flat(flat *f,config *c)
{
  printf("Flat %s ",f->name);

  if (f->props&FLOOR) printf("floor ");
  if (f->props&CEILING) printf("ceiling ");
  if (f->props&LIGHT) printf("light ");
  if (f->props&NUKAGE) printf("nukage ");
  if (f->props&OUTDOOR) printf("outside ");
  if (f->props&GATE) printf("gate ");
  if (f->props&RED) printf("red ");
  if (f==c->sky_flat) printf("sky ");
  if (f==c->water_flat) printf("water ");
  dump_foo_themebits(f->compatible,0,"comp",c);
  if (!(f->gamemask&DOOM0_BIT)) printf("noDoom0 ");
  if (!(f->gamemask&DOOM1_BIT)) printf("noDoom1 ");
  if (!(f->gamemask&DOOM2_BIT)) printf("noDoom2 ");
  if (!(f->gamemask&DOOMI_BIT)) printf("custom ");
  if (!(f->gamemask&DOOMC_BIT)) printf("gross ");
  printf("\n");
}

void dump_foo_genus(genus *g,config *c)
{
  if (g->compatible==0) return;
  if (g->compatible==~(unsigned long)0) return;
  printf("Thing %d ",g->thingid);
  dump_foo_themebits(g->compatible,0,"comp",c);
  printf("\n");
}

void dump_foo_construct(construct *x,config *c)
{
  texture_cell *tc;
  flat_cell *fc;
  printf("Construct family %d height %d ",x->family,x->height);

  dump_foo_themebits(x->compatible,0,"comp",c);
  if (!(x->gamemask&DOOM0_BIT)) printf("noDoom0 ");
  if (!(x->gamemask&DOOM1_BIT)) printf("noDoom1 ");
  if (!(x->gamemask&DOOM2_BIT)) printf("noDoom2 ");
  if (!(x->gamemask&DOOMI_BIT)) printf("custom ");
  if (!(x->gamemask&DOOMC_BIT)) printf("gross ");
  printf("\n");
  printf("  ");
  for (fc=x->flat_cell_anchor;fc;fc=fc->next) {
    printf("top %s ",fc->flat->name);
  }
  printf("\n");
  for (tc=x->texture_cell_anchor;tc;tc=tc->next) {
    printf("  %sary %s ",tc->primary?"Prim":"Second",tc->texture->name);
    if (tc->y_offset1 || tc->y_offset2)
      printf("yoffsets %d %d ",tc->y_offset1,tc->y_offset2);
    if (tc->width!=128) printf("width %d ",tc->width);
    printf("\n");
  }
}

#else

void dump_foo_texture(texture *t,config *c)
{
  printf("t %s ",t->name);
  if ((t->width!=256)||(t->height!=128))
    printf("z %d %d ",t->width,t->height);
  if (t->props&WALL) printf("w ");
  if (t->props&SWITCH) printf("i ");
  if (t->props&LIFT_TEXTURE) printf("F ");
  if (t->props&SUPPORT) printf("I ");
  if (t->props&JAMB) printf("j ");
  if (t->props&STEP) printf("e ");
  if (t->props&GRATING) printf("g ");
  if (t->props&PLAQUE) printf("p ");
  if (t->props&VTILES) printf("v ");
  if (t->props&HALF_PLAQUE) printf("H ");
  if (t->props&LIGHT) printf("l ");
  if (t->props&EXITSWITCH) printf("E ");
  if (t->props&DOOR) printf("d ");
  if (t->props&GATE) printf("L ");
  if (t->props&OUTDOOR) printf("o ");
  if (t->props&RED) printf("r ");
  if (t->props&BLUE) printf("b ");
  if (t->props&YELLOW) printf("y ");
  if (t==c->error_texture) printf("! ");
  if (t==c->gate_exitsign_texture) printf("X ");
  dump_foo_themebits(t->core,0,"C",c);
  dump_foo_themebits(t->compatible,t->core,"c",c);
  if (t->subtle)
    printf("S %s ",t->subtle->name);
  if (t->switch_texture)
    printf("s %s ",t->switch_texture->name);
  if (t->realname != t->name)
    printf("= %s ",t->realname);
  if (t->y_bias) printf("Y %d ",t->y_bias);
  if (5!=t->y_hint) printf("@ %d ",t->y_hint);
  if (!(t->gamemask&DOOM0_BIT)) printf("0 ");
  if (!(t->gamemask&DOOM1_BIT)) printf("1 ");
  if (!(t->gamemask&DOOM2_BIT)) printf("2 ");
  if (!(t->gamemask&DOOMI_BIT)) printf("u " );
  if (!(t->gamemask&DOOMC_BIT)) printf("Q ");
  printf("\n");

}

void dump_foo_flat(flat *f,config *c)
{
  printf("f %s ",f->name);

  if (f->props&FLOOR) printf("D ");
  if (f->props&CEILING) printf("U ");
  if (f->props&LIGHT) printf("l ");
  if (f->props&NUKAGE) printf("n ");
  if (f->props&OUTDOOR) printf("o ");
  if (f->props&GATE) printf("G ");
  if (f->props&RED) printf("r ");
  if (f==c->sky_flat) printf("K ");
  if (f==c->water_flat) printf("W ");
  dump_foo_themebits(f->compatible,0,"c",c);
  if (!(f->gamemask&DOOM0_BIT)) printf("0 ");
  if (!(f->gamemask&DOOM1_BIT)) printf("1 ");
  if (!(f->gamemask&DOOM2_BIT)) printf("2 ");
  if (!(f->gamemask&DOOMI_BIT)) printf("u " );
  if (!(f->gamemask&DOOMC_BIT)) printf("G ");
  printf("\n");
}

void dump_foo_genus(genus *g,config *c)
{
  if (g->compatible==0) return;
  printf(". %d ",g->thingid);
  dump_foo_themebits(g->compatible,0,"c",c);
  printf("\n");
}

void dump_foo_construct(construct *x,config *c)
{
  texture_cell *tc;
  flat_cell *fc;
  printf("x m %d h %d ",x->family,x->height);

  dump_foo_themebits(x->compatible,0,"c",c);
  if (!(x->gamemask&DOOM0_BIT)) printf("0 ");
  if (!(x->gamemask&DOOM1_BIT)) printf("1 ");
  if (!(x->gamemask&DOOM2_BIT)) printf("2 ");
  if (!(x->gamemask&DOOMI_BIT)) printf("u ");
  if (!(x->gamemask&DOOMC_BIT)) printf("G ");
  printf("\n");
  for (fc=x->flat_cell_anchor;fc;fc=fc->next) {
    printf("O %s ",fc->flat->name);
  }
  printf("\n");
  for (tc=x->texture_cell_anchor;tc;tc=tc->next) {
    printf("%s %s ",tc->primary?"A":"B",tc->texture->name);
    if (tc->y_offset1 || tc->y_offset2)
      printf("] %d %d ",tc->y_offset1,tc->y_offset2);
    if (tc->width!=128) printf("~ %d ",tc->width);
    printf("\n");
  }
}

#endif


void dump_foo(config *c)
{
  texture *t;
  flat *f;
  construct *x;
  genus *g;
  for (t=c->texture_anchor;t;t=t->next) dump_foo_texture(t,c);
  for (f=c->flat_anchor;f;f=f->next) dump_foo_flat(f,c);
  for (x=c->construct_anchor;x;x=x->next) dump_foo_construct(x,c);
  for (g=c->genus_anchor;g;g=g->next) dump_foo_genus(g,c);
}

#endif

/* Get the hardwired nonswitch-nontheme config stuff (like */
/* monsters and obstables and such.                        */
boolean hardwired_nonswitch_nontheme_config(config *c)
{
  genus *m;

  /* get these obstacles registered as non-pickables */
  m = find_genus(c,ID_LAMP);
  m->bits &= ~PICKABLE;
  m->bits |= LIGHT;
  m->width = 33;
  m = find_genus(c,ID_ELEC);
  m->bits &= ~PICKABLE;
  m->bits |= LIGHT;
  m->width = 33;
  m->height = 127;  /* About */
  m = find_genus(c,ID_LAMP2);
  m->bits &= ~PICKABLE;
  m->bits |= LIGHT;
  m->width = 33;
  m->gamemask = DOOM2_BIT | DOOMC_BIT | DOOMI_BIT;
  m = find_genus(c,ID_TLAMP2);
  m->bits &= ~PICKABLE;
  m->bits |= LIGHT;
  m->width = 33;
  m->height = 72;   /* Very roughly */
  m->gamemask = DOOM2_BIT | DOOMC_BIT | DOOMI_BIT;
  m = find_genus(c,ID_SHORTRED);
  m->bits &= ~PICKABLE;
  m->bits |= LIGHT;
  m->width = 33;
  m = find_genus(c,ID_SHORTBLUE);
  m->bits &= ~PICKABLE;
  m->bits |= LIGHT;
  m->width = 33;
  m = find_genus(c,ID_SHORTGREEN);
  m->bits &= ~PICKABLE;
  m->bits |= LIGHT;
  m->width = 33;
  m = find_genus(c,ID_TALLRED);
  m->bits &= ~PICKABLE;
  m->bits |= LIGHT;
  m->width = 33;
  m->height = 127;  /* sure */
  m = find_genus(c,ID_TALLBLUE);
  m->bits &= ~PICKABLE;
  m->bits |= LIGHT;
  m->width = 33;
  m->height = 127;  /* sure */
  m = find_genus(c,ID_TALLGREEN);
  m->bits &= ~PICKABLE;
  m->bits |= LIGHT;
  m->width = 33;
  m->height = 127;  /* sure */
  m = find_genus(c,ID_CBRA);
  m->bits &= ~PICKABLE;
  m->bits |= LIGHT;
  m->width = 33;
  m->height = 72;  /* about */
  m = find_genus(c,ID_FBARREL);
  m->gamemask = DOOM2_BIT | DOOMC_BIT | DOOMI_BIT;
  m->bits &= ~PICKABLE;
  m->bits |= LIGHT;
  m->width = 33;
  m = find_genus(c,ID_BARREL);
  m->bits &= ~PICKABLE;
  m->bits |= EXPLODES;
  m->width = 33;
  /* pretend the candle is pickable; really just "not blocking" */
  m = find_genus(c,ID_CANDLE);
  m->bits |= PICKABLE;
  m->bits |= LIGHT;
  m->width = 16;
  /* and register the weapons and ammos and healths */
  /* at least the ones that we need for arenas! */
  m = find_genus(c,ID_ROCKBOX);
  m->bits |= AMMO;
  m->ammo_provides = (float)500;
  m = find_genus(c,ID_BULBOX);
  m->bits |= AMMO;
  m->ammo_provides = (float)500;
  m = find_genus(c,ID_CELLPACK);
  m->bits |= AMMO;
  m->ammo_provides = (float)2000;  /* Hoo-hoo!  Same for BFG as plasgun? */
  /* violence and mayhem */
  c->usualammo[ITYTD] = 5000;
  c->usualammo[HMP] = 3500;      /* or 3k? */
  c->usualammo[UV] = 3500;       /* or 2k? or 3k? */
  c->usualarmor[ITYTD] = 100;
  c->usualarmor[HMP] = 50;
  c->usualarmor[UV] = 30;         /* or 20 */
  c->usualhealth[ITYTD] = 80;
  c->usualhealth[HMP] = 65;
  c->usualhealth[UV] = 55;
  c->minhealth[ITYTD] = 50;
  c->minhealth[HMP] = 35;
  c->minhealth[UV] = 20;
  m = find_monster(c,ID_TROOPER);
  m->width = 42;
  m->ammo_provides = (float)100;
  m->ammo_to_kill[ITYTD] = (float)55;
  m->ammo_to_kill[HMP] = (float)35;
  m->ammo_to_kill[UV] = (float)30;
  m->damage[ITYTD] = (float)15;
  m->damage[HMP] = (float)3;
  m->damage[UV] = (float)1;
  m->altdamage[ITYTD] = (float)10;
  m->altdamage[HMP] = (float)1;
  m->altdamage[UV] = (float)1;
  m->bits |= SHOOTS;
  m = find_monster(c,ID_SERGEANT);
  m->width = 42;
  m->ammo_provides = (float)280;
  m->ammo_to_kill[ITYTD] = (float)80;
  m->ammo_to_kill[HMP] = (float)50;
  m->ammo_to_kill[UV] = (float)40;
  m->damage[ITYTD] = (float)25;
  m->damage[HMP] = (float)6;
  m->damage[UV] = (float)2;
  m->altdamage[ITYTD] = (float)20;
  m->altdamage[HMP] = (float)2;
  m->altdamage[UV] = (float)1;
  m->bits |= SHOOTS;
  m = find_monster(c,ID_IMP);
  m->width = 42;
  m->ammo_provides = (float)0;
  m->ammo_to_kill[ITYTD] = (float)160;
  m->ammo_to_kill[HMP] = (float)95;
  m->ammo_to_kill[UV] = (float)80;
  m->damage[ITYTD] = (float)20;
  m->damage[HMP] = (float)6;
  m->damage[UV] = (float)3;
  m->altdamage[ITYTD] = (float)20;
  m->altdamage[HMP] = (float)5;
  m->altdamage[UV] = (float)2;
  m->bits |= SHOOTS;
  m = find_monster(c,ID_PINK);
  m->width = 62;
  m->ammo_provides = (float)0;
  m->ammo_to_kill[ITYTD] = (float)385;
  m->ammo_to_kill[HMP] = (float)236;
  m->ammo_to_kill[UV] = (float)195;
  m->damage[ITYTD] = (float)25;
  m->damage[HMP] = (float)10;
  m->damage[UV] = (float)8;
  m->altdamage[ITYTD] = (float)20;
  m->altdamage[HMP] = (float)8;
  m->altdamage[UV] = (float)4;
  m = find_monster(c,ID_SPECTRE);
  m->width = 62;
  m->ammo_provides = (float)0;
  m->ammo_to_kill[ITYTD] = (float)410;
  m->ammo_to_kill[HMP] = (float)260;
  m->ammo_to_kill[UV] = (float)220;
  m->damage[ITYTD] = (float)25;
  m->damage[HMP] = (float)10;
  m->damage[UV] = (float)8;
  m->altdamage[ITYTD] = (float)25;
  m->altdamage[HMP] = (float)8;
  m->altdamage[UV] = (float) 6;
  m = find_monster(c,ID_SKULL);
  m->width = 34;
  m->bits |= BIG;  /* Well, sort of! */
  m->ammo_provides = (float)0;
  m->ammo_to_kill[ITYTD] = (float)260;
  m->ammo_to_kill[HMP] = (float)165;
  m->ammo_to_kill[UV] = (float)130;
  m->damage[ITYTD] = (float)22;
  m->damage[HMP] = (float)8;
  m->damage[UV] = (float)5;
  m->altdamage[ITYTD] = (float)18;
  m->altdamage[HMP] = (float)5;
  m->altdamage[UV] = (float)2;
  m->bits |= FLIES;
  m = find_monster(c,ID_HEAD);
  m->width = 63;                 /* Or 62 or maybe 64 */
  m->bits |= BIG;
  m->ammo_provides = (float)0;
  m->ammo_to_kill[ITYTD] = (float)1050;
  m->ammo_to_kill[HMP] = (float)630;
  m->ammo_to_kill[UV] = (float)590;
  m->damage[ITYTD] = (float)60;
  m->damage[HMP] = (float)35;
  m->damage[UV] = (float)18;
  m->altdamage[ITYTD] = (float)50;
  m->altdamage[HMP] = (float)20;
  m->altdamage[UV] = (float)10;
  m->bits |= SHOOTS;
  m->bits |= FLIES;

  m = find_monster(c,ID_BARON);
  m->width = 50;                 /* Roughly */
  m->height = 64;
  m->bits |= BIG | BOSS;         /* Not placed randomly */
  m->ammo_provides = (float)0;
  m->ammo_to_kill[ITYTD] = (float)1900;   /* Numbers are all guesses; fix */
  m->ammo_to_kill[HMP] = (float)1600;
  m->ammo_to_kill[UV] = (float)1500;
  m->damage[ITYTD] = (float)80;
  m->damage[HMP] = (float)40;
  m->damage[UV] = (float)25;
  m->altdamage[ITYTD] = (float)70;
  m->altdamage[HMP] = (float)25;
  m->altdamage[UV] = (float)18;
  m->bits |= SHOOTS;

  /* Other bosses; need to fill in data! */
  m = find_monster(c,ID_CYBER);
  m->width = 84;
  m->height = 110;
  m->bits |= BIG | BOSS;
  m->ammo_provides = (float)0;
  m->ammo_to_kill[ITYTD] = (float)8000;   /* Numbers are all guesses; fix */
  m->ammo_to_kill[HMP] = (float)6500;
  m->ammo_to_kill[UV] = (float)6200;
  m = find_monster(c,ID_SPIDERBOSS);
  m->width = 260;
  m->height = 100;
  m->bits |= BIG | BOSS;
  m->ammo_provides = (float)0;
  m->ammo_to_kill[ITYTD] = (float)6000;   /* Numbers are all guesses; fix */
  m->ammo_to_kill[HMP] = (float)5000;
  m->ammo_to_kill[UV] = (float)4500;

  /* DOOM2 monsters */
  if (!(c->gamemask&(DOOM0_BIT|DOOM1_BIT))) {
    m = find_monster(c,ID_NAZI);
    m->gamemask = DOOM2_BIT;
    m->width = 42;
    m->ammo_to_kill[ITYTD] = (float)117;
    m->ammo_to_kill[HMP] = (float)78;
    m->ammo_to_kill[UV] = (float)65;
    m->damage[ITYTD] = (float)40;
    m->damage[HMP] = (float)14;
    m->damage[UV] = (float)7;
    m->altdamage[ITYTD] = (float)27;
    m->altdamage[HMP] = (float)10;
    m->altdamage[UV] = (float)4;
    m->bits |= SHOOTS | SPECIAL;
    m = find_monster(c,ID_COMMANDO);
    m->gamemask = DOOM2_BIT;
    m->width = 42;
    m->ammo_provides = (float)100;
    m->ammo_to_kill[ITYTD] = (float)155;
    m->ammo_to_kill[HMP] = (float)106;
    m->ammo_to_kill[UV] = (float)90;
    m->damage[ITYTD] = (float)60;
    m->damage[HMP] = (float)25;
    m->damage[UV] = (float)15;
    m->altdamage[ITYTD] = (float)40;
    m->altdamage[HMP] = (float)20;
    m->altdamage[UV] = (float)10;
    m->bits |= SHOOTS;
    m = find_monster(c,ID_SKEL);
    m->gamemask = DOOM2_BIT;
    m->width = 42;
    m->bits |= BIG;
    m->ammo_provides = (float)0;
    m->ammo_to_kill[ITYTD] = (float)800;
    m->ammo_to_kill[HMP] = (float)500;
    m->ammo_to_kill[UV] = (float)400;
    m->damage[ITYTD] = (float)125;
    m->damage[HMP] = (float)70;
    m->damage[UV] = (float)40;
    m->altdamage[ITYTD] = (float)100;
    m->altdamage[HMP] = (float)40;
    m->altdamage[UV] = (float)25;
    m->bits |= SHOOTS;
    m = find_monster(c,ID_HELL);
    m->gamemask = DOOM2_BIT;
    m->width = 50;
    m->bits |= BIG;
    m->ammo_provides = (float)0;
    m->ammo_to_kill[ITYTD] = (float)1400;
    m->ammo_to_kill[HMP] = (float)850;
    m->ammo_to_kill[UV] = (float)666;
    m->damage[ITYTD] = (float)140;
    m->damage[HMP] = (float)80;
    m->damage[UV] = (float)50;
    m->altdamage[ITYTD] = (float)120;
    m->altdamage[HMP] = (float)50;
    m->altdamage[UV] = (float)35;
    m->bits |= SHOOTS;

    /* DOOM2 bosses and underbosses; need to fill in data! */
    m = find_monster(c,ID_MANCUB);
    m->gamemask = DOOM2_BIT;
    m->width = 100;
    m->height = 64;
    m->bits |= BIG | BOSS;
    m->ammo_provides = (float)0;
    m->ammo_to_kill[ITYTD] = (float)1200;   /* Numbers are all guesses; fix */
    m->ammo_to_kill[HMP] = (float)1000;
    m->ammo_to_kill[UV] = (float)900;
    m = find_monster(c,ID_ARCHIE);
    m->gamemask = DOOM2_BIT;
    m->width = 42;
    m->height = 56;
    m->bits |= BIG | BOSS;
    m->ammo_provides = (float)0;
    m->ammo_to_kill[ITYTD] = (float)1300;   /* Numbers are all guesses; fix */
    m->ammo_to_kill[HMP] = (float)1100;
    m->ammo_to_kill[UV] = (float)1000;
    m = find_monster(c,ID_PAIN);
    m->gamemask = DOOM2_BIT;
    m->width = 63;
    m->bits |= BIG | BOSS;
    m->ammo_provides = (float)0;
    m->ammo_to_kill[ITYTD] = (float)1900;   /* Numbers are all guesses; fix */
    m->ammo_to_kill[HMP] = (float)1600;
    m->ammo_to_kill[UV] = (float)1500;
    m = find_monster(c,ID_ARACH);
    m->gamemask = DOOM2_BIT;
    m->width = 130;
    m->bits |= BIG | BOSS;
    m->ammo_provides = (float)0;
    m->ammo_to_kill[ITYTD] = (float)1300;   /* Numbers are all guesses; fix */
    m->ammo_to_kill[HMP] = (float)800;
    m->ammo_to_kill[UV] = (float)700;
  }

  return TRUE;
}


/* Absorb a property-word, return the corresponding bit, and */
/* update *r to point to the last string we used (but since all */
/* the properties we handle are one token long, we never actually */
/* change *r). */
propertybits absorb_propertybit(char **r)
{
  char *p;

  p = *r;

  if ((!stricmp(p,"wall")) || (!strcmp(p,"w"))) return WALL;
  if ((!stricmp(p,"isswitch")) || (!strcmp(p,"i"))) return SWITCH;
  if ((!stricmp(p,"lift")) || (!strcmp(p,"F"))) return LIFT_TEXTURE;
  if ((!stricmp(p,"support")) || (!strcmp(p,"I"))) return SUPPORT;
  if ((!stricmp(p,"jamb")) || (!strcmp(p,"j"))) return JAMB;
  if ((!stricmp(p,"step")) || (!strcmp(p,"e"))) return STEP;
  if ((!stricmp(p,"grating")) || (!strcmp(p,"g"))) return GRATING;
  if ((!stricmp(p,"plaque")) || (!strcmp(p,"p"))) return PLAQUE;
  if ((!stricmp(p,"vtiles")) || (!strcmp(p,"v"))) return VTILES;
  if ((!stricmp(p,"half_plaque")) || (!strcmp(p,"H"))) return HALF_PLAQUE;
  if ((!stricmp(p,"light")) || (!strcmp(p,"l"))) return LIGHT;
  if ((!stricmp(p,"exitswitch")) || (!strcmp(p,"E"))) return EXITSWITCH;
  if ((!stricmp(p,"door")) || (!strcmp(p,"d"))) return DOOR;
  if ((!stricmp(p,"locked")) || (!strcmp(p,"L"))) return GATE;
  if ((!stricmp(p,"outside")) || (!strcmp(p,"o"))) return OUTDOOR;
  if ((!stricmp(p,"red")) || (!strcmp(p,"r"))) return RED;
  if ((!stricmp(p,"blue")) || (!strcmp(p,"b"))) return BLUE;
  if ((!stricmp(p,"yellow")) || (!strcmp(p,"y"))) return YELLOW;
  if ((!stricmp(p,"floor")) || (!strcmp(p,"D"))) return FLOOR;
  if ((!stricmp(p,"ceiling")) || (!strcmp(p,"U"))) return CEILING;
  if ((!stricmp(p,"nukage")) || (!strcmp(p,"n"))) return NUKAGE;
  if ((!stricmp(p,"gate")) || (!strcmp(p,"G"))) return GATE;

  return 0;
}

/* Absorb a restriction-word, return the corresponding bit, and */
/* update *r to point to the last string we used (but since all */
/* the restrictions we handle are one token long, we never actually */
/* change *r). */
gamebits absorb_gamebit(char **r)
{
  char *p;

  p = *r;

  if ((!stricmp(p,"nodoom0")) || (!strcmp(p,"0"))) return DOOM0_BIT;
  if ((!stricmp(p,"nodoom1")) || (!strcmp(p,"1"))) return DOOM1_BIT;
  if ((!stricmp(p,"nodoom2")) || (!strcmp(p,"2"))) return DOOM2_BIT;
  if ((!stricmp(p,"gross")) || (!strcmp(p,"Q"))) return DOOMC_BIT;
  if ((!stricmp(p,"custom")) || (!strcmp(p,"u"))) return DOOMI_BIT;

  return 0;
}

/* Absorb a Theme record from the config data, returning the last */
/* string that we actually used. */
char *absorb_theme(char *p, config *c)
{
  char *q, *name;
  theme *t;
  boolean b = FALSE;

  p += 1+strlen(p);  /* That's the name */
  name = p;

  q = p + 1 + strlen(p);
  if ((!stricmp(q,"secret")) || (!strcmp(q,"?"))) {
    p = q;
    b = TRUE;
  }

  t = new_theme(c,name,b);

  return p;
}

/* Return a themebit for the given name, or zero if none */
themebits themebit_for_name(char *name, config *c)
{
  theme *t;
  themebits answer = 1;

  for (t=c->theme_anchor;t;t=t->next) {
    if (!stricmp(t->name,name)) return answer;
    answer <<= 1;
  }
  return 0;
}

/* Absorb a parameter like "thing stringval", returning stringval.  If */
/* neither the short nor long "thing"s given match, return NULL.  Update */
/* *r to point to the last string we used. */
char *absorb_string(char **r,char *ln, char *sn)
{
  /* Needs more error-checking.  Input Is Evil. */
  if (stricmp(*r,ln) && strcmp(*r,sn)) return NULL;
  (*r) += 1+strlen(*r);  /* That's the name */
  return *r;

}

/* Absorb a parameter like "yhint 5", etc etc etc, see above. */
boolean absorb_short(char **r,char *ln,char *sn,short *s)
{
  char *v = absorb_string(r,ln,sn);
  if (v==NULL) return FALSE;
  *s = (short)atoi(v);
  return TRUE;
}

/* Absorb a parameter like "size 5 6", etc etc etc, see above. */
boolean absorb_two_shorts(char **r,char *ln,char *sn,short *s,short *t)
{
  char *v = absorb_string(r,ln,sn);
  if (v==NULL) return FALSE;
  *s = (short)atoi(v);
  *r += 1 + strlen(*r);
  *t = (short)atoi(*r);
  return TRUE;
}

/* Absorb a Texture record from the config data, returning the last */
/* string that we actually used. */
char *absorb_texture(char *p, config *c)
{
  char *q, *name;
  short n,m;
  texture *t;
  propertybits pb;
  gamebits gb;
  themebits tb;

  p += 1+strlen(p);  /* That's the name */
  t = find_texture(c,p);

  q = p;
  for (;;) {
    p=q;
    q = p + 1 + strlen(p);
    if (0 != (pb = absorb_propertybit(&q))) { t->props |= pb; continue; }
    if (0 != (gb = absorb_gamebit(&q))) { t->gamemask &= ~gb; continue; }
    if (NULL != (name = absorb_string(&q,"core","C"))) {
      tb = themebit_for_name(name,c);
      if (tb==0) {
        fprintf(stderr,"Unknown theme <%s> in core.\n",name);
        exit(129);
      }
      t->core |= tb;
      t->compatible |= tb;
      continue;
    }
    if (NULL != (name = absorb_string(&q,"comp","c"))) {
      tb = themebit_for_name(name,c);
      if (tb==0) {
        fprintf(stderr,"Unknown theme <%s> in comp.\n",name);
        exit(129);
      }
      t->compatible |= tb;
      continue;
    }
    if (NULL != (name = absorb_string(&q,"switch","s")))
      { t->switch_texture = find_texture(c,name); continue; }
    if (NULL != (name = absorb_string(&q,"subtle","S")))
      { t->subtle = find_texture(c,name); continue; }
    if (NULL != (name = absorb_string(&q,"realname","=")))
      { t->realname = strdup(name); continue; }
    if (absorb_short(&q,"yhint","@",&m)) { t->y_hint = m; continue; }
    if (absorb_short(&q,"ybias","Y",&m)) { t->y_bias = m; continue; }
    if (absorb_two_shorts(&q,"size","z",&m,&n))
      { t->width = m; t->height = n; continue; }
    if ((!stricmp(q,"error")) || (!stricmp(q,"!"))) {
      c->error_texture = t;
      continue;
    }
    if ((!stricmp(q,"gateexitsign")) || (!stricmp(q,"X"))) {
      c->gate_exitsign_texture = t;
      continue;
    }
    break;
  }
  return p;
}

/* Absorb a Flat record from the config data, returning the last */
/* string that we actually used. */
char *absorb_flat(char *p, config *c)
{
  char *q, *name;
  flat *f;
  propertybits pb;
  gamebits gb;
  themebits tb;

  p += 1+strlen(p);  /* That's the name */
  f = find_flat(c,p);

  q = p;
  for (;;) {
    p=q;
    q = p + 1 + strlen(p);
    if (0 != (pb = absorb_propertybit(&q))) { f->props |= pb; continue; }
    if (0 != (gb = absorb_gamebit(&q))) { f->gamemask &= ~gb; continue; }
    if (NULL != (name = absorb_string(&q,"comp","c"))) {
      tb = themebit_for_name(name,c);
      if (tb==0) {
        fprintf(stderr,"Unknown theme <%s> in comp.\n",name);
        exit(129);
      }
      f->compatible |= tb;
      continue;
    }
    if ((!stricmp(q,"sky")) || (!stricmp(q,"K"))) {
      c->sky_flat = f;
      continue;
    }
    if ((!stricmp(q,"water")) || (!stricmp(q,"W"))) {
      c->water_flat = f;
      continue;
    }
    break;
  }
  return p;
}

/* Absorb a Thing record from the config data, returning the last */
/* string that we actually used. */
char *absorb_thing(char *p, config *c)
{
  char *q, *name;
  genus *g;
  themebits tb;

  p += 1+strlen(p);  /* That's the number */
  g = find_genus(c,atoi(p));
  if (g->compatible==~(unsigned long)0) g->compatible = 0;

  q = p;
  for (;;) {
    p=q;
    q = p + 1 + strlen(p);
    if (NULL != (name = absorb_string(&q,"comp","c"))) {
      tb = themebit_for_name(name,c);
      if (tb==0) {
        fprintf(stderr,"Unknown theme <%s> in comp.\n",name);
        exit(129);
      }
      g->compatible |= tb;
      continue;
    }
    break;
  }
  return p;
}

/* Absorb a cell subrecord of a construct record, returning TRUE if */
/* there is one there, or FALSE if not.  Update r to point to the */
/* last string we actually used. */
boolean absorb_cell(construct *x,char **r,char *ln,char *sn,boolean b,config *c)
{
  char *p, *q, *name;
  texture_cell *tc;
  short o1 = 0;
  short o2 = 0;
  short width = 128;
  short s,t;

  p=*r;
  if (stricmp(p,ln) && strcmp(p,sn)) return FALSE;
  p += 1+strlen(p);  /* That's the name */
  name = p;

  q = p;
  for (;;) {
    p=q;
    q = p + 1 + strlen(p);
    if (absorb_short(&q,"width","~",&s)) { width = s; continue; }
    if (absorb_two_shorts(&q,"yoffsets","]",&s,&t))
      { o1 = s; o2 = t; continue; }
    break;
  }
  *r = p;

  tc = add_texture_cell(x,name,b,o1,o2,c);
  tc->width = width;

  return TRUE;
}

/* Absorb a Construct record from the config data, returning the last */
/* string that we actually used. */
char *absorb_construct(char *p, config *c)
{
  char *q, *name;
  short s;
  construct *x;
  gamebits gb;
  themebits tb;

  x = new_construct(c);

  q = p;
  for (;;) {
    p=q;
    q = p + 1 + strlen(p);
    if (absorb_short(&q,"family","m",&s)) { x->family = s; continue; }
    if (absorb_short(&q,"height","h",&s)) { x->height = s; continue; }
    if (0 != (gb = absorb_gamebit(&q))) { x->gamemask &= ~gb; continue; }
    if (NULL != (name = absorb_string(&q,"comp","c"))) {
      tb = themebit_for_name(name,c);
      if (tb==0) {
        fprintf(stderr,"Unknown theme <%s> in comp.\n",name);
        exit(129);
      }
      x->compatible |= tb;
      continue;
    }
    if (NULL != (name = absorb_string(&q,"top","O")))
      { add_flat_cell(x,name,c); continue; }
    if (absorb_cell(x,&q,"primary","A",TRUE,c)) continue;
    if (absorb_cell(x,&q,"secondary","B",FALSE,c)) continue;
    break;
  }
  return p;
}



/* Look through the config's config file, and fill in values for */
/* all the non-switch lines therein.  Return FALSE if error.     */
boolean nonswitch_config(config *c)
{

#ifndef USE_OLD_HARDWIRED_STUFF

  char *p;

  /* Skip to the "[THEMES]" section */
  for (p=c->configdata;*p;p+=1+strlen(p))
    if (!stricmp("[themes]",p)) break;
  if (!*p) {
    fprintf(stderr,"No [THEMES] section in config file.\n");
    exit(143);
  }
  p+=1+strlen(p);
  for (;*p;p+=1+strlen(p)) {
    if (p[0]=='[') break;  /* End of section */
    if ((!strcmp(p,"T")) || (!stricmp(p,"theme")))
      p = absorb_theme(p,c);
     else if ((!strcmp(p,"t")) || (!stricmp(p,"texture")))
      p = absorb_texture(p,c);
     else if ((!strcmp(p,"f")) || (!stricmp(p,"flat")))
      p = absorb_flat(p,c);
     else if ((!strcmp(p,"x")) || (!stricmp(p,"construct")))
      p = absorb_construct(p,c);
     else if ((!strcmp(p,".")) || (!stricmp(p,"thing")))
      p = absorb_thing(p,c);
     else if ((!strcmp(p,"#")) || (!stricmp(p,"hardwired1")))
      hardwired_nonswitch_nontheme_config(c);
     else {
       fprintf(stderr,"Nonsensical token <%s> in config file.\n",p);
       exit(174);
    }
  }

#else

  /* Read from the config file here! */
  hardwired_nonswitch_config(c);

#endif

#ifdef CONFIG_DUMP
  dump_foo(c);
#endif

  return TRUE;
}

/* Random parts for the style, based on the config, and other */
/* parts of the style.                                        */

/* Return a random thing in the given style and config, */
/* satisfying the given pmask, in the given height-range. */
/* Ignore s if NULL. */
genus *random_thing0(propertybits pmask,config *c,style *s,int minh,int maxh)
{
  int tmask = 0;
  int tcount;
  genus *answer;

  if (s)
    tmask = 0x01 << s->theme_number;

  tcount = 0;
  for (answer=c->genus_anchor;answer;answer=answer->next) {
    if ((answer->bits&pmask)!=pmask) continue;
    if (s) if (!(answer->compatible&tmask)) continue;
    if ( (answer->gamemask & c->gamemask) != c->gamemask ) continue;
    if (answer->height>maxh) continue;
    if (answer->height<minh) continue;
    tcount++;
  }
  if (tcount==0) {
    announce(NONE,"No compatible things for theme");   /* This is OK */
    return NULL;
  }
  tcount = 1 + roll(tcount);
  for (answer=c->genus_anchor;answer;answer=answer->next) {
    if ( ((answer->bits&pmask)==pmask) &&
         ((s==NULL)||(answer->compatible&tmask)) &&
         (answer->height <= maxh) &&
         (answer->height >= minh) &&
         ( (answer->gamemask & c->gamemask) == c->gamemask ) ) {
      tcount--;
      if (tcount==0) return answer;
    }
  }
  return NULL;
}

/* Return a random flat in the given style and config, */
/* satisfying the given pmask.  If s is NULL, ignore it. */
flat *random_flat0(propertybits pmask, config *c, style *s)
{
  int fmask = 0;
  int fcount;
  flat *answer;

  if (s)
    fmask = 0x01 << s->theme_number;

  fcount = 0;
  for (answer=c->flat_anchor;answer;answer=answer->next) {
    if ((answer->props&pmask)!=pmask) continue;
    if (s) if (!(answer->compatible&fmask)) continue;
    if ( (answer->gamemask & c->gamemask) != c->gamemask ) continue;
    fcount++;
  }
  if (fcount==0) {
    announce(NONE,"No compatible flats for theme");   /* This is OK */
    return NULL;
  }
  fcount = 1 + roll(fcount);
  for (answer=c->flat_anchor;answer;answer=answer->next) {
    if ( ((answer->props&pmask)==pmask) &&
         ((s==NULL)||(answer->compatible&fmask)) &&
         ( (answer->gamemask & c->gamemask) == c->gamemask ) ) {
      fcount--;
      if (fcount==0) return answer;
    }
  }
  return NULL;
}

flat *random_floor0(config *c, style *s)
{
  return random_flat0(FLOOR,c,s);
}

flat *random_gate(config *c, style *s)
{
  return random_flat0(GATE,c,s);
}

flat *random_ceiling0(config *c, style *s)
{
  return random_flat0(CEILING,c,s);
}

flat *random_ceilinglight(config *c, style *s)
{
  return random_flat0(CEILING+LIGHT,c,s);
}

flat *random_nukage1(config *c,style *s)
{
  return random_flat0(NUKAGE,c,s);
}

flat *random_doorceiling(config *c,style *s)
{
  if (rollpercent(50)) return s->ceiling0;
    else return random_ceiling0(c,s);
}

flat *random_doorfloor(config *c,style *s)
{
  if (rollpercent(50)) return s->floor0;
    else return random_floor0(c,s);   /* stub */
}

flat *random_stepfloor(config *c,style *s)
{
  return random_doorfloor(c,s);  /* stub stub */
}

/* Return a random texture in the given style and config, */
/* satisfying the given pmask.  If s is NULL, ignore it. */
texture *random_texture0(propertybits pmask, config *c, style *s)
{
  int tmask = 0;
  int tcount;
  texture *answer;

  if (s) {
    tmask = 0x01 << s->theme_number;
  }

  tcount = 0;
  for (answer=c->texture_anchor;answer;answer=answer->next) {
    if ((answer->props&pmask)!=pmask) continue;
    if (s) if (!(answer->compatible&tmask)) continue;
    if ( (answer->gamemask & c->gamemask) != c->gamemask ) continue;
    tcount++;
  }
  if (tcount==0) {
    announce(NONE,"No compatible textures for theme");   /* It's OK! */
    return NULL;
  }
  tcount = 1 + roll(tcount);
  for (answer=c->texture_anchor;answer;answer=answer->next) {
    if ( ((answer->props&pmask)==pmask) &&
         ((s==NULL)||(answer->compatible&tmask)) &&
         ( (answer->gamemask & c->gamemask) == c->gamemask ) ) {
      tcount--;
      if (tcount==0) return answer;
    }
  }
  return NULL;
}

texture *random_support0(config *c, style *s)
{
  return random_texture0(SUPPORT,c,s);
}

texture *random_wall0(config *c, style *s)
{
  int tmask, tcount;
  texture *answer;

  tmask = 0x01 << s->theme_number;

  if (rollpercent(80)) {  /* Use a core wall texture */
    tcount = 0;
    for (answer=c->texture_anchor;answer;answer=answer->next) {
      if (!(answer->props&WALL)) continue;
      if (!(answer->core&tmask)) continue;
      if ( (answer->gamemask & c->gamemask) != c->gamemask ) continue;
      tcount++;
    }
    if (tcount==0) {
      announce(WARNING,"No core wall textures for theme");
      return c->error_texture;
    }
    tcount = 1 + roll(tcount);
    for (answer=c->texture_anchor;answer;answer=answer->next) {
      if ( (answer->props&WALL) && (answer->core&tmask) &&
           ( (answer->gamemask & c->gamemask) == c->gamemask ) ) {
        tcount--;
        if (tcount==0) return answer;
      }
    }
  } else {  /* Use any compatible wall texture */
    return random_texture0(WALL,c,s);
  }

  return NULL;
}

texture *random_kickplate(config *c, style *s)
{
  return random_support0(c,s);
#if 0
  return random_wall0(c,s);   /* stub */
#endif
}

texture *random_stepfront(config *c, style *s)
{
  texture *answer;
  if (!rollpercent(c->p_use_steps))
    answer = random_kickplate(c,s);
    else answer = random_texture0(STEP,c,s);
  if (answer==NULL) answer = random_kickplate(c,s);
  return answer;
}

texture *switch0_for(config *c, style *s)
{
  if (s->wall0->switch_texture) {
    return s->wall0->switch_texture;
  } else {
    return random_texture0(SWITCH,c,s);
  }
}

texture *random_doorjamb(config *c, style *s)
{
  return random_texture0(JAMB,c,s);
}

texture *random_redface(config*c, style *s)
{
  return random_texture0(RED,c,s);
}

texture *random_blueface(config*c, style *s)
{
  return random_texture0(BLUE,c,s);
}

texture *random_yellowface(config*c, style *s)
{
  return random_texture0(YELLOW,c,s);
}

texture *random_walllight(config*c, style *s)
{
  return random_texture0(LIGHT,c,s);
}

texture *random_liftface(config*c, style *s)
{
  return random_texture0(LIFT_TEXTURE,c,s);
}

/* should consult the lists in the config like the others do */


/* Return a door-face that looks good on a wide door. */
texture *random_widedoorface(config *c, style *s)
{
  return random_widedoorface_ex(c,s,FALSE);
}

/* Return a door-face that looks good on a wide door.  If needhigh, */
/* also require 128 high. */
texture *random_widedoorface_ex(config *c, style *s, boolean needhigh)
{
  int tmask = 0;
  int tcount;
  texture *answer;

  tmask = 0x01 << s->theme_number;

  tcount = 0;
  for (answer=c->texture_anchor;answer;answer=answer->next) {
    if (!(answer->props&DOOR)) continue;
    if (answer->props&GATE) continue;
    if ((answer->width)<128) continue;
    if (needhigh && (answer->height<128)) continue;
    if (!(answer->compatible&tmask)) continue;
    if ( (answer->gamemask & c->gamemask) != c->gamemask ) continue;
    tcount++;
  }
  if (tcount==0) {
    announce(ERROR,"No wide doorfaces for theme");   /* Bad! */
    return c->error_texture;
  }
  tcount = 1 + roll(tcount);
  for (answer=c->texture_anchor;answer;answer=answer->next) {
    if ( (answer->props&DOOR) &&
         (!(answer->props&GATE)) &&
         (answer->width>=128) &&
         (!(needhigh && (answer->height<128))) &&
         (answer->compatible&tmask) &&
         ( (answer->gamemask & c->gamemask) == c->gamemask ) ) {
      tcount--;
      if (tcount==0) return answer;
    }
  }
  return c->error_texture;
}

/* Return a door-face that looks good on a narrow door. */
texture *random_narrowdoorface(config *c, style *s)
{
  texture *answer = random_narrowdoorface_ex(c,s,FALSE);
  return answer;
}

/* Return a door-face that looks good on a wide door.  If needhigh, */
/* also require 128 high. */
texture *random_narrowdoorface_ex(config *c, style *s, boolean needhigh)
{
  int tmask = 0;
  int tcount;
  texture *answer;

  tmask = 0x01 << s->theme_number;

  tcount = 0;
  for (answer=c->texture_anchor;answer;answer=answer->next) {
    if (!(answer->props&DOOR)) continue;
    if (answer->props&GATE) continue;
    if ((answer->width)>=128) continue;
    if (needhigh && (answer->height<128)) continue;
    if (!(answer->compatible&tmask)) continue;
    if ( (answer->gamemask & c->gamemask) != c->gamemask ) continue;
    tcount++;
  }
  if (tcount==0) {
    announce(ERROR,"No narrow doorfaces for theme");   /* Bad! */
    return c->error_texture;
  }
  tcount = 1 + roll(tcount);
  for (answer=c->texture_anchor;answer;answer=answer->next) {
    if ( (answer->props&DOOR) &&
         (!(answer->props&GATE)) &&
         (answer->width<128) &&
         (!(needhigh && (answer->height<128))) &&
         (answer->compatible&tmask) &&
         ( (answer->gamemask & c->gamemask) == c->gamemask ) ) {
      tcount--;
      if (tcount==0) return answer;
    }
  }
  return c->error_texture;
}


/* Looks good wide, and is 128 high.  Note this should only be called */
/* after the style's "widedoorface" has been set. */
texture *random_twdoorface(config *c, style *s)
{
  if (s->widedoorface->height>=128) return s->widedoorface;
  return random_widedoorface_ex(c,s,TRUE);

}

/* Looks good narrow, and is 128 high.  Note this should only be called */
/* after the style's "narrowdoorface" has been set. */
texture *random_tndoorface(config *c, style *s)
{
  texture *answer;
  if (s->narrowdoorface->height>=128) answer = s->narrowdoorface;
    else answer = random_narrowdoorface_ex(c,s,TRUE);
  return answer;
}

/* Special texture to use (if it fits) on locked doors */
/* May return NULL for a given style */
texture *random_lockdoorface(config *c, style *s)
{
  int tmask = 0;
  int tcount;
  texture *answer;

  tmask = 0x01 << s->theme_number;

  tcount = 0;
  for (answer=c->texture_anchor;answer;answer=answer->next) {
    if (!(answer->props&DOOR)) continue;
    if (!(answer->props&GATE)) continue;
    if (!(answer->compatible&tmask)) continue;
    if ( (answer->gamemask & c->gamemask) != c->gamemask ) continue;
    tcount++;
  }
  if (tcount==0) {
    announce(NONE,"No locked doorfaces for theme");   /* That's OK */
    return NULL;
  }
  tcount = 1 + roll(tcount);
  for (answer=c->texture_anchor;answer;answer=answer->next) {
    if ( (answer->props&DOOR) &&
         (answer->props&GATE) &&
         (answer->compatible&tmask) &&
         ( (answer->gamemask & c->gamemask) == c->gamemask ) ) {
      tcount--;
      if (tcount==0) return answer;
    }
  }
  return NULL;
}

texture *random_grating(config *c, style *s)
{
  return random_texture0(GRATING,c,s);
}

texture *random_plaque(config *c, style *s)
{
  return random_texture0(PLAQUE,c,s);
}

/* Return the angle (east is zero, north is ninety) of a thing */
/* that's standing in the linedef, facing right from it.       */
int facing_along(int x1, int y1, int x2, int y2)
{
  int answer = facing_right_from(x1,y1,x2,y2);
  return (answer==270) ? 0 : (answer + 90);
}

/* Return the angle (east is zero, north is ninety) of a thing */
/* that's standing in the linedef, facing right from it.       */
int facing_right_from(int x1, int y1, int x2, int y2)
{
  /* Best at right angles and axis-parallel lines */

  if (abs(x1-x2)<abs(y1-y2)) {  /* More parallel to the Y-axis */
    if (y2>y1) {  /* Up */
      return 0;
    } else {      /* Down */
      return 180;
    }
  } else {     /* to the X-axis */
    if (x2>x1) {  /* rightward */
      return 270;
    } else {
      return 90;
    }
  }
}

/* Return the angle (east is zero, north is ninety) of a thing */
/* that's standing in the linedef, facing right from it.       */
int facing_right_from_ld(linedef *ld)
{
  /* could be a macro */
  return facing_right_from(ld->from->x,ld->from->y,ld->to->x,ld->to->y);
}

/* Wall up the given clear-walled sector inside the given outer sector. */
/* Or if innersec is NULL, put a void sector in there, and use the given */
/* texture on the walls.  If the four last pointers are given, returns   */
/* the four linedefs it makes. */
void frame_innersec_ex(level *l,sector *oldsector,sector *innersec,
                      texture *tm, texture *tu, texture *tl,
                      int x1,int y1,int x2,int y2,
                      int x3,int y3,int x4,int y4,
                      config *c,
                      linedef **ld1, linedef **ld2, linedef **ld3, linedef **ld4)
{
  linedef *ld;
  vertex *v0, *v1, *v2;
  short newflags;

  if (innersec) newflags = TWO_SIDED;
    else newflags = 0;
  if (!tm) tm = c->null_texture;

  v1 = new_vertex(l,x1,y1);
  v0 = v1;
  v2 = new_vertex(l,x2,y2);
  ld = new_linedef(l,v2,v1);
  ld->flags |= newflags;
  ld->right = new_sidedef(l,oldsector,c);
  ld->right->isBoundary = 0;
  ld->right->middle_texture = tm;
  if (innersec) {
    ld->left = new_sidedef(l,innersec,c);
    ld->left->middle_texture = tm;
    patch_upper(ld,tu,c);
    patch_lower(ld,tl,c);
  }
  if (ld1) *ld1 = ld;

  v1 = v2;
  v2 = new_vertex(l,x3,y3);
  ld = new_linedef(l,v2,v1);
  ld->flags |= newflags;
  ld->right = new_sidedef(l,oldsector,c);
  ld->right->isBoundary = 0;
  ld->right->middle_texture = tm;
  if (innersec) {
    ld->left = new_sidedef(l,innersec,c);
    ld->left->middle_texture = tm;
    patch_upper(ld,tu,c);
    patch_lower(ld,tl,c);
  }
  if (ld2) *ld2 = ld;

  v1 = v2;
  v2 = new_vertex(l,x4,y4);
  ld = new_linedef(l,v2,v1);
  ld->flags |= newflags;
  ld->right = new_sidedef(l,oldsector,c);
  ld->right->isBoundary = 0;
  ld->right->middle_texture = tm;
  if (innersec) {
    ld->left = new_sidedef(l,innersec,c);
    ld->left->middle_texture = tm;
    patch_upper(ld,tu,c);
    patch_lower(ld,tl,c);
  }
  if (ld3) *ld3 = ld;

  v1 = v2;
  v2 = v0;
  ld = new_linedef(l,v2,v1);
  ld->flags |= newflags;
  ld->right = new_sidedef(l,oldsector,c);
  ld->right->isBoundary = 0;
  ld->right->middle_texture = tm;
  if (innersec) {
    ld->left = new_sidedef(l,innersec,c);
    ld->left->middle_texture = tm;
    patch_upper(ld,tu,c);
    patch_lower(ld,tl,c);
  }
  if (ld4) *ld4 = ld;

}  /* frame_innersec_ex() */

/* The common axis-parallel case of frame_innersec.  If the four pointers */
/* are given, the first and third will be y-parallel, the second and */
/* fourth x-parallel. */
void parallel_innersec_ex(level *l,sector *oldsector,sector *innersec,
                         texture *tm, texture *tu, texture *tl,
                         int minx,int miny,int maxx,int maxy,config *c,
                         linedef **ld1, linedef **ld2,
                         linedef **ld3, linedef **ld4)
{
  frame_innersec_ex(l,oldsector,innersec,tm,tu,tl,
                    minx,miny,minx,maxy,maxx,maxy,maxx,miny,c,
                    ld1,ld2,ld3,ld4);
}

/* Your basic visual room-center embellishments */
/* Square in the middle of the room with higher/lower/no ceiling, */
/* possibly different floor, light level, nukage, etc. */
boolean ceiling_effect(level *l, sector *oldsector,
                       style *ThisStyle, haa *haa, config *c)
{
  int minx, miny, maxx, maxy, offset, deltah;
  genus *g = ThisStyle->lamp0;
  short thing_id;
  int beamsize, maxbeam;
  sector *innersec;
  boolean force_nukage = rollpercent(l->p_force_nukage);
  boolean force_sky = rollpercent(l->p_force_sky);
  boolean force_quad = rollpercent(15);
  boolean edge_lights = FALSE;
  boolean center_light = FALSE;
  texture *upt = oldsector->style->wall0;

  if (g->height>(oldsector->ceiling_height-oldsector->floor_height))
    g = ThisStyle->shortlamp0;
  thing_id = g->thingid;

  /* Only do this sometimes! */
  if (!(rollpercent(5)||force_nukage||force_sky)) return FALSE;

  /* Find the inner sector corners */
  find_rec(l,oldsector,&minx,&miny,&maxx,&maxy);
  offset = maxx - minx;
  if (maxy - miny < offset) offset = maxy - miny;
  if (offset<96) return FALSE;   /* No sense making a really teeny one */
  offset = 16 + roll((offset>>1)-48);
  minx = minx + offset;
  miny = miny + offset;
  maxx = maxx - offset;
  maxy = maxy - offset;

  /* Sometimes do four little effects */
  if ((maxx-minx)<144) force_quad = FALSE;
  if ((maxy-miny)<144) force_quad = FALSE;
  maxbeam = (maxx-minx)-128;
  if (((maxy-miny)-128)<maxbeam) maxbeam = (maxy-miny)-128;
  beamsize = 16+roll(maxbeam-15);
  if (beamsize>64) beamsize = 64;
  /* In that case, almost always force sky or nukage (why?) */
  if (force_quad && !force_nukage && !force_sky) {
    if (rollpercent(45)) force_nukage = TRUE;
      else if (rollpercent(82)) force_sky = TRUE;
  }

  /* With the box effect, sometimes put a Thing or Things, */
  /* if there's room (don't want stuck monsters!)          */
  if (rollpercent(50))
    if (maxx-minx>170)
      if (maxy-miny>170) {
        if (rollpercent(80)) {
          edge_lights = TRUE;
        } else {
          center_light = TRUE;
        }
        if (force_quad && rollpercent(50))
          edge_lights = center_light = TRUE;
        if (offset<(38+8+g->width))
          edge_lights = FALSE;
        if (force_quad && (beamsize<g->width))
          center_light = FALSE;
        if (edge_lights &&
         room_at(l,g,minx-8,miny-8,g->width,c) &&
         room_at(l,g,minx-8,maxy+8,g->width,c) &&
         room_at(l,g,maxx+8,miny-8,g->width,c) &&
         room_at(l,g,maxx+8,maxy+8,g->width,c) ) {
          new_thing(l,minx-8,miny-8,0,thing_id,7,c);
          new_thing(l,minx-8,maxy+8,0,thing_id,7,c);
          new_thing(l,maxx+8,miny-8,0,thing_id,7,c);
          new_thing(l,maxx+8,maxy+8,0,thing_id,7,c);
          announce(VERBOSE,"edgelights");
        }
        if (center_light &&
            room_at(l,g,minx+(maxx-minx)/2,miny+(maxy-miny)/2,g->width,c)) {
          new_thing(l,minx+(maxx-minx)/2,miny+(maxy-miny)/2,0,thing_id,7,c);
          announce(VERBOSE,"centerlight");
        }
      }  /* end if big enough square for lights */

  {
    char s[200];
    sprintf(s,"Ceiling effect between (%d,%d) and (%d,%d).",minx,miny,
                        maxx,maxy);
    announce(VERBOSE,s);
  }

  /* Make the sector itself */
  innersec = clone_sector(l,oldsector);

  if (rollpercent(50)||force_sky) {   /* Ceiling hole */
    innersec->ceiling_height += 16 * (1 + roll(3));
    innersec->light_level = l->outside_light_level - 20;  /* Minus 20? */
    innersec->ceiling_flat = c->sky_flat;
  } else {  /* Just a difference */
    innersec->ceiling_flat = random_ceiling0(c,ThisStyle);
    deltah = 32 - (roll(65));
    /* Don't lower the ceiling too near a wall (door)! */
    if ((offset<64)&&(deltah<0)) deltah = 0 - deltah;
    innersec->ceiling_height += deltah;
    if ((innersec->ceiling_height-innersec->floor_height)<64)
      innersec->ceiling_height = innersec->floor_height + 64;
  }  /* end not a hole */

  /* Fun recessed ceiling lights? */
  if (rollpercent(20))
    if (innersec->ceiling_height > oldsector->ceiling_height)
      if (oldsector->style->walllight!=NULL) {
        if (innersec->ceiling_height < oldsector->ceiling_height+16)
          innersec->ceiling_height = oldsector->ceiling_height+16;
        upt = oldsector->style->walllight;
        if (innersec->ceiling_flat!=c->sky_flat) {
          innersec->light_level = oldsector->light_level + 20;
          if (rollpercent(90))
            innersec->ceiling_flat = oldsector->ceiling_flat;
        }
        announce(VERBOSE,"Indirect lighting");
      }

  /* If no light-level decision made yet, make one */
  if ((innersec->ceiling_flat != c->sky_flat) &&
      (upt == oldsector->style->wall0)) {   /* Poor test! */
    innersec->light_level = oldsector->light_level + roll(41) - 20;
    if (innersec->light_level<100) innersec->light_level = 100;  /* Minlight? */
  }

  /* If not open-air, maybe make the lights blink/flash */
  if (innersec->ceiling_flat != c->sky_flat) {
    if (rollpercent(20)) switch (roll(4)) {
      case 0: innersec->special = RANDOM_BLINK; break;
      case 1: innersec->special = SYNC_FAST_BLINK; break;
      case 2: innersec->special = SYNC_SLOW_BLINK; break;
      case 3: innersec->special = GLOW_BLINK; break;
    }
  }  /* end if light effects */

  if (force_nukage||rollpercent(30)) {   /* Floor thing also */
    innersec->floor_flat = random_floor0(c,ThisStyle);
    deltah = 24 - (roll(49));
    /* Don't raise the floor too near a wall (door)! */
    if ((offset<64)&&(deltah>0)) deltah = 0 - deltah;
    /* and if forced nukage, always lower the floor */
    if (force_nukage&&(deltah>0)) deltah = 0 - deltah;
    innersec->floor_height += deltah;
    if ((innersec->ceiling_height-innersec->floor_height)<64)
      innersec->floor_height = innersec->ceiling_height - 64;
    if ((oldsector->ceiling_height-innersec->floor_height)<64)
      innersec->floor_height = oldsector->ceiling_height - 64;
    if ((innersec->ceiling_height-oldsector->floor_height)<64)
      innersec->ceiling_height = oldsector->ceiling_height + 64;

    /* And maybe some nukage! */
    if (deltah<0)
      if (force_nukage||rollpercent(30)) {
        announce(VERBOSE,"Nukage");
        innersec->floor_flat = ThisStyle->nukage1;
        innersec->special = NUKAGE1_SPECIAL;
        haa->haas[ITYTD].health -= 10;
        haa->haas[HMP].health -= 5;
      }
  } else {
    deltah = 0;  /* Note we didn't do it */
  }

  /* Make the necessary linedefs and sidedefs */
  if (force_quad) {
    int xsize = ((maxx - minx) - beamsize) / 2;
    int ysize = ((maxy - miny) - beamsize) / 2;
    parallel_innersec(l,oldsector,innersec,
                      NULL,upt,oldsector->style->wall0,
                      minx,miny,minx+xsize,miny+ysize,c);
    parallel_innersec(l,oldsector,innersec,
                      NULL,upt,oldsector->style->wall0,
                      minx,maxy-ysize,minx+xsize,maxy,c);
    parallel_innersec(l,oldsector,innersec,
                      NULL,upt,oldsector->style->wall0,
                      maxx-xsize,miny,maxx,miny+ysize,c);
    parallel_innersec(l,oldsector,innersec,
                      NULL,upt,oldsector->style->wall0,
                      maxx-xsize,maxy-ysize,maxx,maxy,c);
  } else if ( (maxx-minx>128) && (maxy-miny>128) &&
              ((maxx-minx)<=(2*(maxy-miny))) &&
              ((maxy-miny)<=(2*(maxx-minx))) &&
              rollpercent(10) ) {
    /* A diamond!  Is this safe?  Not axis-parallel! */
    announce(LOG,"Diamond");
    frame_innersec(l,oldsector,innersec,
                   NULL,upt,oldsector->style->wall0,
                   (minx+maxx)/2,miny,
                   minx,(miny+maxy)/2,
                   (minx+maxx)/2,maxy,
                   maxx,(miny+maxy)/2,
                   c);
  } else {
    /* Just an old fashioned square */
    linedef *ld1, *ld2, *ld3, *ld4;
    boolean fancied = FALSE;
    parallel_innersec_ex(l,oldsector,innersec,
                         NULL,upt,oldsector->style->wall0,
                         minx,miny,maxx,maxy,c,
                         &ld1,&ld2,&ld3,&ld4);
    /* but maybe with fancy stuff! */
    /* Stairs to get out? */
    if ( (deltah<0) && (!fancied) &&
        no_monsters_stuck_on(l,ld1) &&
        no_monsters_stuck_on(l,ld2) &&
        no_monsters_stuck_on(l,ld4) &&
        rollpercent(l->p_deep_baths) ) {
      int xsize = maxx-minx;
      linedef *ld2new, *ld4new, *ldnew;
      sector *newsec;
      deltah = -24;            /* 24? */
      innersec->floor_height = oldsector->floor_height + deltah;
      for (;xsize>=(128*l->hugeness);) {
        xsize -= 48 * l->hugeness;
        ld2new = ld2;
        ld2 = split_linedef(l,ld2,48*l->hugeness,c);  /* 48? */
        ld4new = split_linedef(l,ld4,xsize,c);
        newsec = clone_sector(l,innersec);
        newsec->floor_flat = oldsector->floor_flat;
        innersec->floor_height += deltah;
        ld3->left->sector = newsec;
        ld2new->left->sector = newsec;
        ld4new->left->sector = newsec;
        ldnew = new_linedef(l,ld4new->from,ld2new->to);
        ldnew->left = new_sidedef(l,innersec,c);
        ldnew->right = new_sidedef(l,newsec,c);
        ldnew->flags |= TWO_SIDED;
        ldnew->right->middle_texture = c->null_texture;
        ldnew->left->middle_texture = c->null_texture;
        patch_lower(ldnew,newsec->style->wall0,c);
        ld3 = ldnew;
        fancied = TRUE;
        if (rollpercent(30)) break;
      }  /* end loop-thing */
      if (innersec->floor_flat != ThisStyle->nukage1)
        if (rollpercent(75)) {
          innersec->floor_flat = c->water_flat;
          announce(LOG,"Water pool");
        }
      if (fancied) announce(LOG,"Bath");
    }
    /* Sometimes more layers!  How dangerous is this? */
    if (rollpercent(20) && (!fancied) &&                /* Generalize "20" */
       (maxx-minx>128) && (maxy-miny>128) &&
       (innersec->floor_flat!=ThisStyle->nukage1)) {
      sector *inner2;
      int deltah;
      deltah = 12 + roll(13);                         /* Generalize this? */
      if (rollpercent(50)) deltah = 0 - deltah;
      if ((offset<64)&&(deltah>0)) deltah = 0 - deltah;
      for (;(maxx-minx>128)&&(maxy-miny>128);) {
        if ((innersec->ceiling_height-oldsector->floor_height)-deltah < 64) break;
        if ((innersec->ceiling_height-innersec->floor_height)-deltah < 64) break;
        if ((oldsector->ceiling_height-innersec->floor_height)-deltah < 64) break;
        inner2 = clone_sector(l,innersec);
        inner2->special = innersec->special;
        inner2->floor_height += deltah;
        announce(VERBOSE,"Sunk");
        minx += 32;                                /* Generalize "32"s */
        maxx -= 32;
        miny += 32;
        maxy -= 32;
        parallel_innersec(l,innersec,inner2,
                          NULL,upt,oldsector->style->wall0,
                          minx,miny,maxx,maxy,c);
        innersec = inner2;
      }
    }
  }

  return TRUE;

}  /* end ceiling_effect() */

/* Perhaps place a timely monster just to the right of the center */
/* of the givenly-ended line.  Update the haa if. */
void righthand_monster(level *l,int xa,int ya,int xb,int yb,haa *haa,config *c)
{
  genus *m;
  int x1, y1, x, y, flags;
  short angle;

  /* See if the model wants a monster */
  m = timely_monster(haa,c,&flags,rollpercent(l->p_biggest_monsters),1);   /* 1 correct? */
  if (!m) return;
  /* Figure out where we want it */
  x1 = (xa+xb)/2;
  y1 = (ya+yb)/2;
  point_from(xa,ya,x1,y1,RIGHT_TURN,1+MONSTER_WIDTH(m)/2,&x,&y);
  if (!room_at(l,m,x,y,MONSTER_WIDTH(m),c)) return;
  /* Fill in other details */
  angle = facing_right_from(xa,ya,xb,yb);  /* Correct? */
  if (rollpercent(50)) flags |= 0x08;   /* deaf; how often? */
  /* And finally create it and update the haa */
  new_thing(l,x,y,angle,m->thingid,(short)flags,c);
  update_haa_for_monster(haa,m,flags,1,c);  /* 1 correct? */

}  /* end righthand_monster */

/* Stick in a pillar (or post).  Assumes rectangles lots. */
/* Now with sometimes monsters! */
/* Should shrink the candidate pillar, not just give up, if */
/* it finds an existing thing in the way. */
void do_pillar(level *l,sector *oldsector,style *ThisStyle,haa *haa,config *c)
{
  int minx, miny, maxx, maxy;
  int xsize, ysize, xoff, yoff;
  thing *t;
  texture *t1;

  /* Figure out where we might want to put it */
  find_rec(l,oldsector,&minx,&miny,&maxx,&maxy);
  /* The room has to be >192 in each direction, for now */
  /* 64 for the pillar, and 64 on every side for monster checks */
  if (maxx-minx<=192) return;
  if (maxy-miny<=192) return;
  /* random sizes within allowable range */
  xsize = 64 + roll( (maxx-minx)-192 );
  ysize = 64 + roll( (maxy-miny)-192 );
  if (rollpercent(50))
    if (xsize>127)
      if (ysize>127) {
        xsize = 128;
        ysize = 128;
      }
  /* Now the offsets from min.  Hmm... */
  if (ThisStyle->center_pillars) {
    xoff = 64 + (   ((maxx-minx)-128) - xsize ) / 2;
    yoff = 64 + (   ((maxy-miny)-128) - ysize ) / 2;
  } else {
    xoff = 64 + roll (   ((maxx-minx)-128) - xsize );
    yoff = 64 + roll (   ((maxy-miny)-128) - ysize );
  }
  /* Now we have the corners of the candidate pillar */
  /* but go out an extra 64 for the check */
  minx = minx + xoff - 64;
  miny = miny + yoff - 64;
  maxx = minx + xsize + 128;   /* Should be 64? */
  maxy = miny + ysize + 128;   /* Should be 64? */
  /* Now we need to see if any Thing is in the area */
  for (t=l->thing_anchor;t;t=t->next)
    if ( (t->x>=minx) && (t->x<=maxx) &&
         (t->y>=miny) && (t->y<=maxy) ) {
           announce(VERBOSE,"Too many things for a pillar");
           return;
         }
  /* None!  A miracle.  Define the space. */
  t1 = ThisStyle->wall0;
  if (rollpercent(80)) t1 = random_wall0(c,ThisStyle);
  if (xsize==128)
    if (ysize==128)
      if (128 == oldsector->ceiling_height - oldsector->floor_height)
        t1 = ThisStyle->plaque;
  minx += 64;
  miny += 64;
  maxx -= 64;
  maxy -= 64;
  if (ThisStyle->do_constructs) {
    install_construct(l,oldsector,minx,miny,maxx,maxy,ThisStyle,c);
  } else {
    parallel_innersec(l,oldsector,NULL,t1,NULL,NULL,minx,miny,maxx,maxy,c);
  }
  announce(VERBOSE,"Made a pillar");

  /* Consider putting some monsters around it */
  righthand_monster(l,minx,maxy,minx,miny,haa,c);
  righthand_monster(l,minx,miny,maxx,miny,haa,c);
  righthand_monster(l,maxx,miny,maxx,maxy,haa,c);
  righthand_monster(l,maxx,maxy,minx,maxy,haa,c);
  haa_unpend(haa);

  /* Whew, I guess that's all! */

}  /* end do_pillar */

/* Does this construct fit into this sector, on sides of these */
/* sizes, in this style? */
boolean construct_fits(construct *cs,int xsize, int ysize,sector *s,
                       style *ThisStyle, config *c)
{
  boolean good_primary = FALSE;
  boolean x_fit = FALSE;
  boolean y_fit = FALSE;
  texture_cell *tc;

  /* Needs to be room between the floor and ceiling */
  if ( cs->height > (s->ceiling_height - s->floor_height) ) return FALSE;

  /* Needs to be in the right family */
  if ( cs->family != ThisStyle->construct_family ) return FALSE;

  /* Need to have at least one primary texture that can fit on */
  /* one side, and at least one texture of any kind that can */
  /* fit on each side. */
  for (tc=cs->texture_cell_anchor;tc;tc=tc->next) {
    if ( (tc->texture->gamemask&c->gamemask)==c->gamemask) {
      if (tc->width<=xsize) {
        x_fit = TRUE;
        if (tc->primary) good_primary = TRUE;
      }
      if (tc->width<=ysize) {
        y_fit = TRUE;
        if (tc->primary) good_primary = TRUE;
      }
    }
  }
  return (x_fit && y_fit && good_primary);
}

/* Return a texture-cell from the given construct that */
/* fits the given size.  If accept_secondaries, then do */
texture_cell *fitting_tc(construct *cs, int size, boolean accept_secondaries,
                         config *c)
{
  texture_cell *tc1;
  texture_cell *answer = NULL;
  int ccount;

  for (tc1=cs->texture_cell_anchor;tc1;tc1=tc1->next) tc1->marked=FALSE;
  for (ccount=0,tc1=cs->texture_cell_anchor;tc1;tc1=tc1->next) {
    if ( (accept_secondaries||tc1->primary)&&(tc1->width<=size)&&
         ( (tc1->texture->gamemask&c->gamemask) == c->gamemask) ) {
      tc1->marked=TRUE;
      ccount++;
    }
  }
  if (ccount) {
    ccount = roll(ccount);
    for (tc1=cs->texture_cell_anchor;tc1;tc1=tc1->next)
      if (tc1->marked) if (0==ccount--) break;
    answer = tc1;
  }
  for (tc1=cs->texture_cell_anchor;tc1;tc1=tc1->next) tc1->marked=FALSE;
  return answer;
}

/* Install, if possible, some construct that fits the style, */
/* in the given place. */
boolean install_construct(level *l,sector *oldsector,
                       int minx,int miny,int maxx,int maxy,
                       style *ThisStyle,config *c)
{
  construct *cs, *cs2;
  sector *innersec;
  int ccount = 0;
  boolean floor_to_ceiling, primary_on_x = FALSE;
  linedef *ld1, *ld2, *ld3, *ld4;
  texture_cell *tc1, *tc2, *tc3, *tc4, *tcp;
  flat_cell *fc;
  int xsize, ysize, mult;

  /* Mark just those constructs that fit */
  for (cs=c->construct_anchor;cs;cs=cs->next) cs->marked = FALSE;
  for (cs=c->construct_anchor;cs;cs=cs->next) {
    if (construct_fits(cs,maxx-minx,maxy-miny,oldsector,ThisStyle,c)) {
      cs->marked = TRUE;
      ccount++;
    }
  }
  if (ccount==0) return FALSE;   /* Give up if none */
  /* Otherwise pick a random marked one */
  ccount = roll(ccount);
  for (cs=c->construct_anchor;cs;cs=cs->next) {
    if (cs->marked) {
      if (ccount==0) break;
      ccount--;
    }
  }
  /* Clean up */
  for (cs2=c->construct_anchor;cs2;cs2=cs2->next) cs2->marked = FALSE;

  floor_to_ceiling =
    (oldsector->ceiling_height - oldsector->floor_height == cs->height);

  /* Iff we need an innersec, make sure we have one */
  if (!floor_to_ceiling) {
    innersec = clone_sector(l,oldsector);
    /* Set the top of the object */
    for (ccount=0,fc=cs->flat_cell_anchor;fc;fc=fc->next) ccount++;
    ccount = roll(ccount);
    for (fc=cs->flat_cell_anchor;fc;fc=fc->next) if (0==ccount--) break;
    innersec->floor_flat = fc->flat;
    innersec->light_level = oldsector->light_level;
    innersec->ceiling_height = oldsector->ceiling_height;
    innersec->floor_height = oldsector->floor_height + cs->height;
  } else {
    innersec=NULL;
  }

  /* Pick a primary texture (cell) */
  tcp = NULL;
  if (rollpercent(50)) {    /* Try X first */
    tcp = fitting_tc(cs,maxy-miny,FALSE,c);
    if (tcp!=NULL) primary_on_x = TRUE;
  }
  if (tcp==NULL) {   /* Nothing yet, try Y */
    tcp = fitting_tc(cs,maxx-minx,FALSE,c);
    if (tcp!=NULL) primary_on_x = FALSE;
  }
  if (tcp==NULL) {   /* Nothing yet, try X (again) */
    tcp = fitting_tc(cs,maxy-miny,FALSE,c);
    if (tcp!=NULL) primary_on_x = TRUE;
  }
  if (tcp==NULL) {  /* Impossible! */
    announce(WARNING,"Some impossible error in construct-construction.");
    return FALSE;
  }

  /* Set all four cells */
  if (primary_on_x) {
    if (rollpercent(50)) {
      tc2 = tcp;
      tc4 = fitting_tc(cs,maxy-miny,TRUE,c);
    } else {
      tc4 = tcp;
      tc2 = fitting_tc(cs,maxy-miny,TRUE,c);
    }
    tc1 = fitting_tc(cs,maxx-minx,TRUE,c);
    tc3 = fitting_tc(cs,maxx-minx,TRUE,c);
  } else {
    if (rollpercent(50)) {
      tc1 = tcp;
      tc3 = fitting_tc(cs,maxx-minx,TRUE,c);
    } else {
      tc3 = tcp;
      tc1 = fitting_tc(cs,maxx-minx,TRUE,c);
    }
    tc2 = fitting_tc(cs,maxy-miny,TRUE,c);
    tc4 = fitting_tc(cs,maxy-miny,TRUE,c);
  }

  /* Now decide how large the X and Y dimensions should actually be */
  xsize = tc1->width;
  if (tc3->width>xsize) xsize = tc3->width;
  mult = (maxx-minx)/xsize;
  if (mult>4) mult=4;   /* Not too huge! */
  if (rollpercent(50)) mult = 1 + roll(mult);
  xsize *= mult;
  ysize = tc2->width;
  if (tc4->width>ysize) ysize = tc4->width;
  mult = (maxy-miny)/ysize;
  if (mult>4) mult=4;   /* Not too huge! */
  if (rollpercent(50)) mult = 1 + roll(mult);
  ysize *= mult;

  /* Finally!  Make the sector */
  parallel_innersec_ex(l,oldsector,innersec,NULL,NULL,NULL,
                       minx,miny,minx+xsize,miny+ysize,c,
                       &ld2,&ld1,&ld4,&ld3);

  /* And fix up the linedefs */
  if (floor_to_ceiling) {
    ld1->right->middle_texture = tc1->texture;
    ld2->right->middle_texture = tc2->texture;
    ld3->right->middle_texture = tc3->texture;
    ld4->right->middle_texture = tc4->texture;
  } else {
    ld1->right->middle_texture = c->null_texture;
    ld2->right->middle_texture = c->null_texture;
    ld3->right->middle_texture = c->null_texture;
    ld4->right->middle_texture = c->null_texture;
    ld1->right->lower_texture = tc1->texture;
    ld2->right->lower_texture = tc2->texture;
    ld3->right->lower_texture = tc3->texture;
    ld4->right->lower_texture = tc4->texture;
    ld1->flags &= ~LOWER_UNPEGGED;
    ld2->flags &= ~LOWER_UNPEGGED;
    ld3->flags &= ~LOWER_UNPEGGED;
    ld4->flags &= ~LOWER_UNPEGGED;
  }
  ld1->right->y_offset = rollpercent(50) ? tc1->y_offset1 : tc1->y_offset2;
  ld2->right->y_offset = rollpercent(50) ? tc2->y_offset1 : tc2->y_offset2;
  ld3->right->y_offset = rollpercent(50) ? tc3->y_offset1 : tc3->y_offset2;
  ld4->right->y_offset = rollpercent(50) ? tc4->y_offset1 : tc4->y_offset2;

  announce(VERBOSE,"Construct");
  return TRUE;

} /* end install_construct */

/* Put in a single pillarish thing, with a much cleverer */
/* algorithm than do_pillar.  Use texture t if given,    */
/* else if NULL use a random one, or sometimes use a     */
/* plaque texture.  If innersec is not null, use that    */
/* inside the pillar (else void).                        */
boolean do_new_pillar(level *l,sector *oldsector,sector *innersec,texture *t1,
                      style *ThisStyle, haa *haa,config *c)
{
  int minx, miny, maxx, maxy, tx, ty;
  thing *t;
  vertex *v;
  linedef *ld;
  texture *tm;

  /* Initialize the 64-enclosing range */
  find_rec(l,oldsector,&minx,&miny,&maxx,&maxy);
  /* The room has to be >192 in each direction, for now */
  /* 64 for the pillar, and 64 on every side for monster checks */
  if (maxx-minx<=192) return FALSE;
  if (maxy-miny<=192) return FALSE;
  /* Pick a point we'd like the pillar to contain, */
  /* to guide the following algorithm */
  tx = minx + 1 + roll(maxx-(minx+1));
  ty = miny + 1 + roll(maxy-(miny+1));
  /* If that point is inside some existing pillar, fail */
  if (oldsector!=point_sector(l,tx,ty,NULL,NULL)) return FALSE;
  /* For each vertex, if the vertex is in the current range, */
  /* shrink the range so it misses the vertex, but still */
  /* contains the t point. */
  for (v=l->vertex_anchor;v;v=v->next) {
    if (infinity_norm(tx,ty,v->x,v->y)<64) return FALSE;  /* Failure! */
    if ( (v->x<minx) || (v->x>maxx) || (v->y<miny) || (v->y>maxy) ) continue;
    if (v->x>tx) maxx = v->x - 1;
      else minx = v->x + 1;
    if (v->y>ty) maxy = v->y - 1;
      else miny = v->y + 1;
  }
  /* and the same for each thing, although in fact the requirement */
  /* to be 64 away from even pickables is overconservative */
  for (t=l->thing_anchor;t;t=t->next) {
    if (infinity_norm(tx,ty,t->x,t->y)<64) return FALSE;  /* Failure! */
    if ( (t->x<minx) || (t->x>maxx) || (t->y<miny) || (t->y>maxy) ) continue;
    if (t->x>tx) maxx = t->x - 1;
      else minx = t->x + 1;
    if (t->y>ty) maxy = t->y - 1;
      else miny = t->y + 1;
  }
  /* Now reduce the enclosing range */
  minx = minx + 64;
  maxx = maxx - 64;
  miny = miny + 64;
  maxy = maxy - 64;
  if (minx>=maxx-15) return FALSE;
  if (miny>=maxy-15) return FALSE;
  /* See if the result has any nasty intersections */
  for (ld=l->linedef_anchor;ld;ld=ld->next) {
    if (intersects(minx,miny,minx,maxy,ld->from->x,ld->from->y,ld->to->x,ld->to->y))
      return FALSE;
    if (intersects(minx,maxy,maxx,maxy,ld->from->x,ld->from->y,ld->to->x,ld->to->y))
      return FALSE;
    if (intersects(maxx,maxy,maxx,miny,ld->from->x,ld->from->y,ld->to->x,ld->to->y))
      return FALSE;
    if (intersects(maxx,miny,minx,miny,ld->from->x,ld->from->y,ld->to->x,ld->to->y))
      return FALSE;
  }
  /* If we made it this far, we found room! */
  /* Now decide how much to use (i.e. should sometimes shrink/narrow here) */
  /* and finally make the pillar (or whatever!) */
  /* Perhaps a construct */
  if (ThisStyle->do_constructs) {
    install_construct(l,oldsector,minx,miny,maxx,maxy,ThisStyle,c);
  } else {
    if (t1==NULL) t1 = random_wall0(c,ThisStyle);
    /* Sometimes do a special plaque thing */
    if ((innersec==NULL) && ((maxx-minx)>=128) && ((maxy-miny)>=128) &&
         ( oldsector->ceiling_height - oldsector->floor_height == 128) ) {
      minx = minx + ((maxx-minx)-128)/2;
      maxx = minx + 128;
      miny = miny + ((maxy-miny)-128)/2;
      maxy = miny + 128;
      t1 = ThisStyle->plaque;
      announce(VERBOSE,"Plaque-pillar");
    }
    if (innersec) {
      announce(VERBOSE,"Inner pillar");
      tm = NULL;
    } else {
      tm = t1;
    }
    parallel_innersec(l,oldsector,innersec,tm,t1,t1,minx,miny,maxx,maxy,c);
    announce(VERBOSE,"New pillar");
  }
  /* Consider putting some monsters around it */
  if (rollpercent(50)) righthand_monster(l,minx,maxy,minx,miny,haa,c);
  if (rollpercent(50)) righthand_monster(l,minx,miny,maxx,miny,haa,c);
  if (rollpercent(50)) righthand_monster(l,maxx,miny,maxx,maxy,haa,c);
  if (rollpercent(50)) righthand_monster(l,maxx,maxy,minx,maxy,haa,c);
  haa_unpend(haa);

  return TRUE;

}  /* end do_new_pillar */

/* Put in a bunch of pillarish things, with a much cleverer */
/* algorithm than do_pillar. */
/* No monsters at present */
void do_new_pillars(level *l,sector *oldsector,style *ThisStyle,
                    haa *haa,config *c)
{
   int want, tried, delta;
   texture *t = NULL;
   sector *s = NULL;

   switch(roll(4)) {
     case 0:
     case 1: t = ThisStyle->wall0; break;
     case 2: t = random_wall0(c,ThisStyle); break;
     case 3: t = NULL;
   }

   /* Number of tries should really be sensitive to */
   /* room-size or something, eh? */
   switch(roll(3)) {
     case 0: want = 1; break;
     case 1: want = 20; break;
     case 2: want = 1 + roll(6); break;
     default: want = 40;   /* Impossible! */
   }

   /* Decide if the pillars should have a non-void inside sector */
   if ((ThisStyle->do_constructs==FALSE)&&(rollpercent(100))) {   /* 100? */
     s = clone_sector(l,oldsector);
     if (rollpercent(30)) {
       s->light_level += 30 - roll(61);
       if (s->light_level>240) s->light_level = 240;
       if (s->light_level<80) s->light_level = 80;
     }
     if (rollpercent(80)) {
       delta = roll((s->ceiling_height-32)-s->floor_height);
       if (delta>128) delta = 128;
       s->floor_height += delta;
     } else if (rollpercent(50)) {
       s->ceiling_height -= roll((s->ceiling_height-32)-s->floor_height);
     } else {
       delta = roll((s->ceiling_height-32)-s->floor_height);
       if (delta>128) delta = 128;
       s->floor_height += delta;
       s->ceiling_height -= roll((s->ceiling_height-32)-s->floor_height);
     }
   }

   for (tried=0;(tried<100)&&want;tried++)
     if (do_new_pillar(l,oldsector,s,t,ThisStyle,haa,c)) want--;
}

/* Put some appropriate monster(s) and bonus(es) along the right */
/* side of the given linedef.  Adjust the haa (haa adjustment */
/* assumes that ITYTD doesn't find the bonuses, and HMP only */
/* finds them half the time, if <secret> is true).            */
void populate_linedef(level *l,linedef *ldnew2,haa *haa,config *c,
                      boolean secret)
{
  int x,y,x1,y1;
  short bonustype;
  int bonusamount;
  genus *m;
  int levels, farness, plen;
  short angle;

  point_from(ldnew2->from->x,ldnew2->from->y,ldnew2->to->x,ldnew2->to->y,
             RIGHT_TURN,32,&x1,&y1);  /* "32" should be improved */
  plen = linelen(ldnew2);
  switch (roll(4)) {
    case 1:  farness = plen-32; break;
    case 2:  farness = plen/2; break;
    case 3:  farness = 32+roll(plen-63); break;
    default: farness = 32; break;
  }
  point_from(ldnew2->to->x,ldnew2->to->y,x1,y1,RIGHT_TURN,farness,&x,&y);
  /* pick a prize; stubby */
  bonustype = ID_POTION;   /* Just in case! */
  if (rollpercent(50)) {  /* Health or whatever */
    switch (roll(4)) {
      case 0: bonustype = ID_MEDIKIT; bonusamount = 25; break;
      case 1: bonustype = ID_MEDIKIT; bonusamount = 25; break;
      case 2: bonustype = ID_STIMPACK; bonusamount = 10; break;
      case 3: if ((FALSE==l->seen_suit)&&(rollpercent(l->p_force_nukage))) {
                bonustype = ID_SUIT; bonusamount = 10;   /* Guess */
                l->seen_suit = TRUE;
              } else if ((FALSE==l->seen_map)&&rollpercent(30)) {
                bonustype = ID_MAP; bonusamount = 0;
                l->seen_map = TRUE;
                announce(VERBOSE,"Area map");
              } else {
                bonustype = ID_INVIS; bonusamount = 10;  /* Also guess */
              }
              break;
      default: bonustype = ID_POTION; bonusamount = 1;  /* Impossible */
    }
    /* We assume ITYTD didn't find the closet! */
    haa->haas[1].health += bonusamount/2;  /* and HMP might not have */
    haa->haas[2].health += bonusamount;
    if (!secret) {  /* Unless it's not a secret */
      haa->haas[0].health += bonusamount;
      haa->haas[1].health += bonusamount/2;
    }
  } else {   /* Some ammo or whatever */
    if ((haa->haas[0].can_use_cells)&&(rollpercent(20))) {
      bonustype = ID_CELLPACK;
      bonusamount = 2000;  /* yow! */
    } else if ((haa->haas[0].can_use_rockets)&&(rollpercent(20))) {
      bonustype = ID_ROCKBOX;
      bonusamount = 500;
    } else if ((!(haa->haas[2].has_chainsaw))&&(rollpercent(20))) {
      bonustype = ID_CHAINSAW;
      bonusamount = 0;
      haa->haas[2].has_chainsaw = TRUE;
    } else if (rollpercent(2)) {
      bonustype = ID_CHAINSAW;
      bonusamount = 0;
      haa->haas[2].has_chainsaw = TRUE;
    } else switch (roll(3)) {
      case 1: bonustype = ID_SHELLBOX; bonusamount = 1400; break;
      case 2: bonustype = ID_BACKPACK;
        bonusamount = 380;
        if (haa->haas[1].can_use_rockets) bonusamount += 100;
        if (haa->haas[1].can_use_cells) bonusamount += 400;
        haa->haas[1].has_backpack = TRUE;
        haa->haas[2].has_backpack = TRUE;
        break;
      default: bonustype = ID_BULBOX; bonusamount = 500; break;
    }  /* end switch */
    /* We assume ITYTD didn't find the closet! */
    haa->haas[1].ammo += bonusamount/2;   /* And HMP only prolly did */
    haa->haas[2].ammo += bonusamount;
    if (!secret) {  /* Unless it's not a secret */
      haa->haas[0].ammo += bonusamount;
      haa->haas[1].ammo += bonusamount/2;
    }
    /* Account for chainsaws; primitive */
    if (bonustype==ID_CHAINSAW) {
      haa->haas[1].has_chainsaw = TRUE;   /* OK? */
      haa->haas[2].has_chainsaw = TRUE;
    }
  }  /* end ammo bonuses */
  new_thing(l,x,y,0,bonustype,7,c);  /* Place the bonus */
  /* Now monsters! */
  if ( ((!secret) || c->secret_monsters) && (rollpercent(90))) {
    farness = 32;   /* mwidth here */
    point_from(ldnew2->to->x,ldnew2->to->y,x1,y1,RIGHT_TURN,farness,&x,&y);
    for (;;) {
      m = timely_monster(haa,c,&levels,rollpercent(l->p_biggest_monsters),0);
      if (m) {
        angle = facing_right_from_ld(ldnew2);
        new_thing(l,x,y,angle,m->thingid,(short)levels,c);  /* not deaf */
        update_haa_for_monster(haa,m,levels,0,c);  /* zero?  one? */
        /* Note that for monster purposes, ALL levels find the closet! */
      } else {
        break;
      }
      farness += 64;   /* Should be mwidths stuff here */
      if (farness+32>plen) break;
      point_from(ldnew2->to->x,ldnew2->to->y,x1,y1,RIGHT_TURN,farness,&x,&y);
    }  /* end forever while monsters and space */
    haa_unpend(haa);
  }  /* end roll for having a monster */
}


/* Put a secret closet behind the given linedef, with something  */
/* or other of interest in it.  Doesn't do anything about hints. */
/* If tag is -1, makes the door faces normal doors.  Otherwise, */
/* makes the inner doorface a door, but leaves the outer one */
/* functionless, and tags the door sector with the tag. */
/* h is the height of the closet, unless it's zero in which case */
/* the existing height is used.  Returns NULL for failure, or    */
/* the linedef of the far wall of the closet.  If a haa is */
/* given, will also populate the closet.  Use the given    */
/* ceiling_height (ch) for y-alignment.  */
linedef *secret_closet(level *l,linedef *ld,style *ThisStyle,int h,
                      haa *haa, config *c, boolean inside_sr, int tag, short ch,
                      boolean secret)
{
  linedef *ldnew, *ldnew2, *ldedge1, *ldedge2;
  sector *s;
  short doortype;

  if (!empty_left_side(l,ld,72)) return NULL;   /* Room? */

  doortype = LINEDEF_NORMAL_DOOR;
  if (!(DOOM0_BIT&c->gamemask) && rollpercent(80))
    doortype = LINEDEF_BLAZE_DOOR;

  /* Modify the outermost linedef to be doory */
  ld->right->upper_texture = ld->right->middle_texture;  /* Door face */
  ld->flags |= SECRET_LINEDEF;
  if (tag==-1)
    ld->type = doortype;
  /* Correct? */
  ld->right->y_offset = 128 -
    (ld->right->sector->ceiling_height - ld->right->sector->floor_height);
  /* Make the door sector itself -- "8" should be variable */
  ldnew = lefthand_box_ext(l,ld,8,ThisStyle,c,&ldedge1,&ldedge2);
  if (tag!=-1) ldnew->right->sector->tag = tag;
  ldedge1->flags |= LOWER_UNPEGGED;
  ldedge2->flags |= LOWER_UNPEGGED;
  ldedge1->right->y_offset =
  ldedge2->right->y_offset =
    ch - ldedge1->right->sector->floor_height;
  /* Make the closet sector -- "64" should be variable */
  ldnew2 = lefthand_box_ext(l,ldnew,64,ThisStyle,c,&ldedge1,&ldedge2);
  if (h!=0) ldnew2->right->sector->ceiling_height =
    ldnew2->right->sector->floor_height + h;
  ldedge1->right->y_offset =
  ldedge2->right->y_offset =
  ldnew2->right->y_offset =
    ch - ldnew2->right->sector->ceiling_height;
  /* Finish making the door doory */
  s = ldnew->right->sector;
  flip_linedef(ldnew);
  if (secret) s->special = SECRET_SECTOR;
  if (inside_sr) {
    ldnew->type = doortype;        /* reopenable */
  } else {
    ldnew->type = LINEDEF_NORMAL_S1_DOOR;  /* Triggered doors never close */
  }
  s->ceiling_height = s->floor_height;
  s->light_level = ThisStyle->doorlight0;
  ldnew->right->upper_texture = ThisStyle->support0;
  ld->flags |= BLOCK_SOUND;       /* Always? */
  ldnew->flags |= BLOCK_SOUND;
  /* and polish up the closet */
  ldnew2->right->middle_texture = ThisStyle->wall0;
  s = ldnew2->right->sector;
  if (s->light_level>160) s->light_level = 160;
  if (s->style->ceilinglight) if (c->clights) {
    s->ceiling_flat = s->style->ceilinglight;
    announce(VERBOSE,"ccl");
    make_lighted(l,s,c);
  }

  /* Sometimes a nukage floor on triggered ones, just */
  /* for fun.  Note that these aren't SECRET anymore. */
  if ((tag!=-1) && (rollpercent(10) || rollpercent(l->p_force_nukage))) {
    s->floor_height -= 8;
    patch_lower(ldnew,ldnew->right->upper_texture,c);
    s->floor_flat = ThisStyle->nukage1;
    s->special = NUKAGE1_SPECIAL;
  }

  if (s->special == SECRET_SECTOR) l->secret_count++;

  if (haa) populate_linedef(l,ldnew2,haa,c,secret);

  return ldnew2;

}  /* end secret_closet() */

/* Put a box around the given thing, with the given tag and */
/* type on each of the linedefs.                            */
void trigger_box(level *l,thing *t,sector* oldsector,short tag,short type,
                 config *c)
{
  vertex *v1, *v2, *v3, *v4;
  linedef *ldnew;
  int dist;
  sector *ns;

  /* Incoming sector is just a guess; confirm it */
  ns = point_sector(l,t->x,t->y,&dist,NULL);  /* Should check "danger"! */
  if (ns) {
    oldsector = ns;
  } else {
    /* This shouldn't ever happen anymore, but just in case... */
    announce(WARNING,"point_sector returned NULL in trigger_box");
  }
  if (dist>24) dist = 24;
  if (dist<4) {
    announce(LOG,"Tiny triggerbox");
    dist = 4;
  } else if (dist<24) {
    announce(LOG,"Small triggerbox");
  }

  v1 = new_vertex(l,t->x-dist,t->y-dist);
  v2 = new_vertex(l,t->x+dist,t->y-dist);
  v3 = new_vertex(l,t->x+dist,t->y+dist);
  v4 = new_vertex(l,t->x-dist,t->y+dist);
  ldnew = new_linedef(l,v1,v2);
  ldnew->right = new_sidedef(l,oldsector,c);
  ldnew->left = new_sidedef(l,oldsector,c);
  ldnew->tag = tag;
  ldnew->type = type;
  ldnew->flags |= TWO_SIDED;
  ldnew->right->middle_texture = c->null_texture;
  ldnew->left->middle_texture = c->null_texture;
  ldnew = new_linedef(l,v2,v3);
  ldnew->right = new_sidedef(l,oldsector,c);
  ldnew->left = new_sidedef(l,oldsector,c);
  ldnew->tag = tag;
  ldnew->type = type;
  ldnew->flags |= TWO_SIDED;
  ldnew->right->middle_texture = c->null_texture;
  ldnew->left->middle_texture = c->null_texture;
  ldnew = new_linedef(l,v3,v4);
  ldnew->right = new_sidedef(l,oldsector,c);
  ldnew->left = new_sidedef(l,oldsector,c);
  ldnew->tag = tag;
  ldnew->type = type;
  ldnew->flags |= TWO_SIDED;
  ldnew->right->middle_texture = c->null_texture;
  ldnew->left->middle_texture = c->null_texture;
  ldnew = new_linedef(l,v4,v1);
  ldnew->right = new_sidedef(l,oldsector,c);
  ldnew->left = new_sidedef(l,oldsector,c);
  ldnew->tag = tag;
  ldnew->type = type;
  ldnew->flags |= TWO_SIDED;
  ldnew->right->middle_texture = c->null_texture;
  ldnew->left->middle_texture = c->null_texture;
}

/* Make a small floor-preserving link that fits on the given */
/* linedef.  Suitable for walking out onto the patio.        */
link *random_patio_link(level *l,linedef *ld,style *ThisStyle,config *c)
{
  link *answer = (link *)malloc(sizeof (*answer));

  answer->type = BASIC_LINK;
  answer->bits = 0;
  answer->floordelta = 0;
  if (rollpercent(50)) {
    answer->height1 = 72;
  } else {
    answer->height1 = 64 + 8 * roll(9);
  }
  if (rollpercent(50)) {
    answer->width1 = 64 * l->hugeness;
  } else {
    answer->width1 = 64 + roll(linelen(ld)-79);
  }
  if (rollpercent(50)) {
    answer->depth1 = 16 * l->hugeness;     /* Door/arch depth */
  } else {
    answer->depth1 = (8 + 4 * roll(15)) * l->hugeness;
  }
  if (rollpercent(50)) {
    answer->depth2 = 8 * l->hugeness;     /* Recess depth */
  } else {
    answer->depth2 = 20 * l->hugeness;
  }
  if (rollpercent(50)) {
    answer->depth3 = 16 * l->hugeness;     /* Core depth */
  } else {
    answer->depth3 = (8 + 4 * roll(15)) * l->hugeness;
  }
  if (rollpercent(50)) answer->bits |= LINK_RECESS;
  if (rollpercent(20)) answer->bits |= LINK_CORE;
  if (rollpercent(5)) answer->bits  |= LINK_BARS;
  if (rollpercent(20)) {  /* Single door */
    answer->bits |= LINK_RECESS | LINK_ANY_DOOR;
    answer->bits &= ~LINK_CORE;
  }
  return answer;
}

/* Try to make a little patio out of the given room */
void make_extroom(level *l, sector *oldsector, haa *haa,
                  style *ThisStyle, config *c)
{
  int i, depth, x, y, fenceh, saveh;
  short cthick;
  linedef *ld, *newldf, *ldfar, *lde1, *lde2, *ldt;
  texture *t1;
  link *ThisLink;
  sector *hisec, *losec;
  vertex *v;
  boolean outtex = rollpercent(70);

  fenceh = 96;  /* Should vary */
  i = mark_decent_boundary_linedefs(l,oldsector,256);
  ld = random_marked_linedef(l,i);
  unmark_linedefs(l);
  if (ld!=NULL) {
    if (!empty_left_side(l,ld,256)) return;
    t1 = ld->right->middle_texture;
    ThisLink = random_patio_link(l,ld,ThisStyle,c);
    newldf = make_linkto(l,ld,ThisLink,ThisStyle,c,NULL);
    if (newldf==NULL) return;   /* Shouldn't happen */
    depth = linelen(ld);
    if ((depth<=512)&&rollpercent(25)) depth *= 2;
    flip_linedef(newldf);  /* Just so we can use the lefthand functions */
    for (;;) {
      if (empty_left_side(l,newldf,depth)) break;
      depth -= 64;
      if (depth<128) {
        delete_vertex(l,newldf->from);
        delete_vertex(l,newldf->to);
        delete_linedef(l,newldf);
        return;   /* How'd that happen? */
      }
    }
    ldfar = lefthand_box_ext(l,newldf,depth,ThisStyle,c,&lde1,&lde2);
    flip_linedef(newldf);  /* Fix it */
    ldfar->right->middle_texture = newldf->right->middle_texture = t1;
    if (outtex) ldfar->right->middle_texture = lde1->right->middle_texture =
      lde2->right->middle_texture = random_texture0(OUTDOOR,c,NULL);
    losec = newldf->right->sector;
    losec->floor_height = oldsector->floor_height;
    losec->floor_flat = oldsector->floor_flat;
    losec->light_level = l->outside_light_level;  /* Minus twenty? */
    cthick = 32;
    if (rollpercent(30)) cthick += 8 * roll(10);
    hisec = clone_sector(l,losec);
    losec->ceiling_height = losec->floor_height + fenceh;
    hisec->ceiling_flat = oldsector->ceiling_flat;   /* For e-l; fixed later */
    newldf->right->sector = hisec;
    /* And now the little triangle thing to look good */
    x = (newldf->to->x + newldf->from->x) / 2;
    y = (newldf->to->y + newldf->from->y) / 2;
    point_from(newldf->from->x,newldf->from->y,x,y,
      RIGHT_TURN,32,&x,&y);
    v = new_vertex(l,x,y);
    ldt = new_linedef(l,newldf->to,v);
    ldt->right = new_sidedef(l,hisec,c);
    ldt->right->middle_texture = c->null_texture;
    ldt->left = new_sidedef(l,losec,c);
    ldt->left->middle_texture = c->null_texture;
    ldt->flags |= TWO_SIDED | NOT_ON_MAP;
    ldt = new_linedef(l,v,newldf->from);
    ldt->right = new_sidedef(l,hisec,c);
    ldt->right->middle_texture = c->null_texture;
    ldt->left = new_sidedef(l,losec,c);
    ldt->left->middle_texture = c->null_texture;
    ldt->flags |= TWO_SIDED | NOT_ON_MAP;
    /* Adjust stuff */
    hisec->ceiling_height = oldsector->ceiling_height + cthick;
    if (hisec->ceiling_height<losec->ceiling_height)
      hisec->ceiling_height = losec->ceiling_height + cthick;
    ldfar->right->y_offset = lde1->right->y_offset = lde2->right->y_offset =
      oldsector->ceiling_height - losec->ceiling_height;
    newldf->right->y_offset = oldsector->ceiling_height - hisec->ceiling_height;
    /* Actually make the link */
    saveh = hisec->ceiling_height;
    establish_link(l,ld,newldf,ThisLink,NULL,ThisStyle,ThisStyle,haa,c);
    hisec->ceiling_flat = c->sky_flat;
    hisec->ceiling_height = saveh;
    /* A hack to fix some quasi-HOMs */
    for (ldt=l->linedef_anchor;ldt;ldt=ldt->next)
      if (ldt->left)
        if (ldt->right)
          if (ldt->right->sector==hisec)
            patch_upper(ldt,t1,c);
    if (outtex) hisec->floor_flat = losec->floor_flat =
      random_flat0(OUTDOOR,c,NULL);
    /* Now populate it and stuff */
    populate(l,losec,c,haa,FALSE);
    place_plants(l,128,losec,c);   /* 128? */
    announce(VERBOSE,"Patio");
  }

}  /* end make_extroom */


/* Try to make an external window out of the given room */
void make_extwindow(level *l, sector *oldsector, style *ThisStyle, config *c)
{
  int wlen, wheight, i, depth, ldlen, border;
  linedef *ld, *ldnew;
  texture *t1;
  linedef *e1, *e2;
  short yoff;

  i = mark_decent_boundary_linedefs(l,oldsector,64);
  ld = random_marked_linedef(l,i);
  unmark_linedefs(l);
  if (ld!=NULL) {
    t1 = ld->right->middle_texture;
    ldlen = linelen(ld);
    wlen = 32 + roll(ldlen-31);
    if (wlen>ldlen) wlen = ldlen;
    border = ( ldlen - wlen ) / 2;
    if (border!=0) {
      ld = split_linedef(l,ld,border,c);
      split_linedef(l,ld,wlen,c);
    }
    depth = 40;
    /* The "48" in the next line is the two eight-deep little sectors, */
    /* plus another 32 just so windows won't be placed *too* absurdly. */
    if (empty_left_side(l,ld,depth+48)) {
      wheight = oldsector->ceiling_height - oldsector->floor_height;
      if (wheight>128) wheight = 128;
      wheight = oldsector->floor_height + 48 + roll(wheight-47);
      if (wheight>oldsector->ceiling_height-32)
        wheight=oldsector->ceiling_height-32;
      ldnew = lefthand_box_ext(l,ld,depth,ThisStyle,c,&e1,&e2);
      ldnew->right->sector->light_level = l->outside_light_level;
      ldnew->right->sector->special = 0;
      ldnew->right->sector->floor_height = wheight;
      ldnew->right->sector->ceiling_height = wheight + 32 + roll(97);
      if ((ldnew->right->sector->ceiling_height>oldsector->ceiling_height) ||
          rollpercent(20))
        ldnew->right->sector->ceiling_height = oldsector->ceiling_height;
      if ((ThisStyle->window_grate)&&rollpercent(50)) {
        announce(VERBOSE,"Grated extwindow");
        ld->right->middle_texture = ThisStyle->grating;
        if (ldnew->right->sector->ceiling_height
            -ldnew->right->sector->floor_height<128) {
          /* Do something about ld->right->y_offset? */
        }
      }
      patch_upper(ld,t1,c);
      patch_lower(ld,t1,c);
      yoff = oldsector->ceiling_height - ldnew->right->sector->ceiling_height;
      e1->right->y_offset = e2->right->y_offset = yoff;
      ldnew = lefthand_box_ext(l,ldnew,8,ThisStyle,c,&e1,&e2);
      e1->right->y_offset = e2->right->y_offset = yoff;
      ldnew->right->sector->floor_height = wheight - 4;
      ldnew->right->sector->ceiling_flat = c->sky_flat;
      ldnew = lefthand_box(l,ldnew,8,ThisStyle,c);
      ldnew->right->sector->floor_height = wheight - 16;
      ldnew->right->sector->ceiling_height = wheight - 8;
      ldnew->right->sector->ceiling_flat = c->sky_flat;
      announce(VERBOSE,"Outside Window");
    }  /* end if enough room */
  }  /* end if found a linedef */
}

/* Special room all full of pillars and stuff.  TRUE if works. */
boolean grid_room(level *l,sector *oldsector,haa *haa,style *ThisStyle,
               quest *ThisQuest,boolean first, config *c)
{
  int minx, miny, maxx, maxy;
  int x1,y1,xi,yi;
  int xcount, ycount;
  int xwidth, ywidth;
  int xspace, yspace;
  int secretx = -1;
  int secrety = -1;
  int sx = 0;
  int sy = 0;
  genus *m;
  int levels;
  short facing;
  texture *t;
  boolean trying_constructs;

  find_rec(l,oldsector,&minx,&miny,&maxx,&maxy);
  if (maxx-minx<192) return FALSE;
  if (maxy-miny<192) return FALSE;
  xcount = (maxx-minx) / 96;
  xcount = 2 + roll(xcount-1);
  ycount = (maxy-miny) / 96;
  ycount = 2 + roll(ycount-1);
  if (xcount*ycount>100) {
    xcount /= 2;
    ycount /= 2;
  }
  xspace = (maxx-minx)/xcount;
  xwidth = 30 + roll(xspace-95);
  yspace = (maxy-miny)/ycount;
  ywidth = 30 + roll(yspace-95);
  if (rollpercent(40)) {  /* Square pillars */
    if (xwidth<ywidth) ywidth = xwidth;
    if (ywidth<xwidth) xwidth = ywidth;
  }

  switch (roll(6)) {
    case 0:
    case 1:
    case 2: t = ThisStyle->wall0; break;
    case 3:
    case 4: t = random_wall0(c,ThisStyle); break;
    default: t = ThisStyle->support0; break;
  }
  if ((ThisStyle->walllight!=NULL)&&rollpercent(3)) {
    announce(LOG,"Gridlight");
    t = ThisStyle->walllight;
    oldsector->light_level = 240;   /* Or so */
    switch (roll(3)) {
      case 0: oldsector->special = RANDOM_BLINK; break;
      case 1: oldsector->special = SYNC_FAST_BLINK; break;
      default: oldsector->special = SYNC_SLOW_BLINK; break;
    }
  }

  trying_constructs = ThisStyle->do_constructs && rollpercent(25);
  if (c->secret_monsters&&rollpercent(75)&&
      (xwidth>63)&&(ywidth>63)&&!trying_constructs) {
    secretx = roll(xcount);
    secrety = roll(ycount);
  }
  for (xi=0,x1=minx+(xspace-xwidth)/2;xi<xcount;x1+=xspace,xi++)
    for (yi=0,y1=miny+(yspace-ywidth)/2;yi<ycount;y1+=yspace,yi++) {
      if ( (xi==secretx) && (yi==secrety) ) {
        linedef *ld1, *ld2, *ld3, *ld4;
        texture *tx = ThisStyle->support0;
        sector *newsec = clone_sector(l,oldsector);
        newsec->floor_height = newsec->ceiling_height;
        newsec->ceiling_height += 96;     /* Fixed? */
        if (tx==t) tx = ThisStyle->wall0;
        newsec->tag = new_tag(l);
        newsec->special = SECRET_SECTOR;
        parallel_innersec_ex(l,oldsector,newsec,NULL,ThisStyle->wall0,t,
                           x1,y1,x1+xwidth,y1+ywidth,c,&ld1,&ld2,&ld3,&ld4);
        ld2->flags |= SECRET_LINEDEF;
        ld3->flags |= SECRET_LINEDEF;
        ld4->flags |= SECRET_LINEDEF;
        ld1->flags &= ~(LOWER_UNPEGGED|UPPER_UNPEGGED);  /* Ought to */
        ld2->flags &= ~(LOWER_UNPEGGED|UPPER_UNPEGGED);  /* re-y-align, */
        ld3->flags &= ~(LOWER_UNPEGGED|UPPER_UNPEGGED);  /* also, */
        ld4->flags &= ~(LOWER_UNPEGGED|UPPER_UNPEGGED);  /* eh? */
        if (rollpercent(50)) {
          ld1->right->lower_texture = tx;
          ld1->flags |= SECRET_LINEDEF;
        }
        ld1->tag = newsec->tag;
        ld1->type = ThisStyle->slifttype;
        flip_linedef(ld3);
        ld3->tag = newsec->tag;
        ld3->type = ThisStyle->slifttype;
        ld3->flags &= ~UPPER_UNPEGGED;
        if (linelen(ld3)>64) split_linedef(l,ld3,64,c);
        ld3->right->upper_texture = ThisStyle->switch0;
        ld3->right->x_offset = 0;
        ld3->right->y_offset += ThisStyle->switch0->y_bias;
        sx = x1+xwidth/2;
        sy = y1+ywidth/2;
        announce(VERBOSE,"Secret grid-pillar");
        continue;
      }
      if (rollpercent(c->p_grid_gaps)) continue;
      if (trying_constructs) {
        trying_constructs =
          install_construct(l,oldsector,x1,y1,x1+xwidth,y1+ywidth,ThisStyle,c);
        if (trying_constructs) announce(VERBOSE,"Grid construct");
      }
      if (!trying_constructs)
        parallel_innersec(l,oldsector,NULL,t,NULL,NULL,
                           x1,y1,x1+xwidth,y1+ywidth,c);
      /* Pretty primitive; monster-width assumptions etc. */
      if (xi&&rollpercent(50)) {
        m = timely_monster(haa,c,&levels,rollpercent(l->p_biggest_monsters),1);
        if (m) {
          if (yi<(ycount/2)) facing = 90; else facing = 270;
          levels |= 0x08;  /* deaf */
          new_thing(l,x1-32,y1+(ywidth/2),facing,m->thingid,(short)levels,c);
          update_haa_for_monster(haa,m,levels,1,c);
        }
      }
      if (yi&&rollpercent(50)) {
        m = timely_monster(haa,c,&levels,rollpercent(l->p_biggest_monsters),1);
        if (m) {
          if (xi<(xcount/2)) facing = 0; else facing = 180;
          levels |= 0x08;  /* deaf */
          new_thing(l,x1+(xwidth/2),y1-32,facing,m->thingid,(short)levels,c);
          update_haa_for_monster(haa,m,levels,1,c);
        }
      }
    }  /* end for */
  haa_unpend(haa);

  /* Rather primitive bonus-depositing */
  for (xi=0,x1=minx+(xspace-xwidth)/2;xi<xcount;x1+=xspace,xi++)
    for (yi=0,y1=miny+(yspace-ywidth)/2;yi<ycount;y1+=yspace,yi++)
      if (yi&&rollpercent(30))
        place_timely_something(l,haa,c,x1+(xwidth/2),y1-23);

  if (secretx!=-1) {
    m = timely_monster(haa,c,&levels,rollpercent(l->p_biggest_monsters),0);
    if (m) {
      facing = 180;
      levels |= 0x08;  /* deaf */
      new_thing(l,sx,sy,facing,m->thingid,(short)levels,c);
      update_haa_for_monster(haa,m,levels,1,c);
    }
    /* This doesn't account for the secretness!! */
    /* It'll also sometimes place nothing at all! */
    place_timely_something(l,haa,c,sx,sy);
    haa_unpend(haa);
  }

  announce(VERBOSE,"Grid room");
  return TRUE;
}

/* Install a teleport gate in the room, and any attendant monsters */
/* and stuff.  Even nukage! */
void install_gate(level *l,sector *s,style *ThisStyle,haa *ThisHaa,
                  boolean force_exit_style, config *c)
{
  short lowx,lowy,hix,hiy;
  sector *innersec, *outersec;
  linedef *ld1, *ld2, *ld3, *ld4;
  flat *gateflat = random_gate(c,s->style);
  boolean exit_style = force_exit_style;
  boolean exit_gate = (s->gate->in_tag==0) && (s->gate->out_tag==0);
  texture *front;
  int rise = 0;
  short tag_mask;

  mid_tile(l,s,&lowx,&lowy,&hix,&hiy);

  outersec = s;

  if (rollpercent(l->p_force_nukage)||rollpercent(10)) {
    int minx,miny,maxx,maxy,leeway;
    find_rec(l,s,&minx,&miny,&maxx,&maxy);
    leeway = lowx - minx;
    if (lowy-miny<leeway) leeway = lowy-miny;
    if (maxx-hix<leeway) leeway = maxx-hix;
    if (maxy-hiy<leeway) leeway = maxy-hiy;
    if (leeway>48) {
      leeway = 16 + roll(leeway-48);
      outersec = clone_sector(l,s);
      outersec->floor_height -= 8;
      outersec->floor_flat = s->style->nukage1;
      if (outersec->light_level<160) outersec->light_level = 160;
      outersec->special = NUKAGE1_SPECIAL;
      rise += 8;
      parallel_innersec(l,s,outersec,NULL,s->style->wall0,s->style->support0,
        minx+leeway,miny+leeway,maxx-leeway,maxy-leeway,c);
      if (s->gate->is_entry) {
        announce(VERBOSE,"Nukage arrival");
      } else {
        announce(VERBOSE,"Nukage gate");
      }
      ThisHaa->haas[ITYTD].health -= 10;
      ThisHaa->haas[HMP].health -= 5;
      if (s->gate->is_entry) {
        ThisHaa->haas[ITYTD].health -= 10;
        ThisHaa->haas[HMP].health -= 5;
        ThisHaa->haas[UV].health -= 5;
      }
    }
  }

  innersec = clone_sector(l,s);
  if (s->gate->out_tag || exit_gate) {
    innersec->floor_flat = gateflat;
    if ((innersec->ceiling_flat!=c->sky_flat)&&ThisStyle->ceilinglight) {
      innersec->ceiling_flat = ThisStyle->ceilinglight;
      announce(VERBOSE,"gcl");
    }
    innersec->light_level = 240;       /* Should vary by style or level */
    innersec->special = GLOW_BLINK;    /* Also */
  }
  if (s->gate->in_tag) {
    innersec->tag = s->gate->in_tag;
    new_thing(l,(lowx+hix)/2,(lowy+hiy)/2,(short)(90*roll(4)),ID_GATEOUT,7,c);
    if (s->gate->is_entry) {
      s->entry_x = lowx+32;
      s->entry_y = lowy+32;
    }
  }
  parallel_innersec_ex(l,outersec,innersec,NULL,NULL,NULL,lowx,lowy,hix,hiy,c,
                       &ld1,&ld2,&ld3,&ld4);
  if (s->gate->gate_lock) {
    switch (s->gate->gate_lock) {
      case LINEDEF_S1_OPEN_DOOR:
        if (rollpercent(30)) {  /* Put in style or config or...? */
          innersec->ceiling_height = innersec->floor_height + 32;
          announce(LOG,"Uplocked gate");
        } else {
          innersec->ceiling_height = innersec->floor_height;
          announce(LOG,"Pillar gate");
        }
        break;
      case LINEDEF_S1_LOWER_FLOOR:
        innersec->floor_height += 32;
        rise += 32;
        announce(LOG,"Downlocked gate");
        break;
      default:
        announce(ERROR,"Odd lock-type in install_gate");
    }
    patch_upper(ld1,s->style->wall0,c);
    patch_upper(ld2,s->style->wall0,c);
    patch_upper(ld3,s->style->wall0,c);
    patch_upper(ld4,s->style->wall0,c);
  }
  if (l->raise_gates) {  /* Don't really want to do this for downlocked? */
    rise += 8;
    innersec->floor_height += 8;
  }
  if (c->p_use_steps) front = s->style->stepfront;
    else front = s->style->support0;
  if (rise>front->height) front = s->style->support0;
  patch_lower(ld1,front,c);
  patch_lower(ld2,front,c);
  patch_lower(ld3,front,c);
  patch_lower(ld4,front,c);
  if (c->monsters_can_teleport) tag_mask = 0;
    else tag_mask = BLOCK_MONSTERS;
  if (c->monsters_can_teleport)
    announce(VERBOSE,"Possible teleporting monsters");
  if (s->gate->out_tag) {
    ld1->type = LINEDEF_TELEPORT;
    ld1->flags |= tag_mask;      /* Always? */
    ld1->tag = s->gate->out_tag;
    ld2->type = LINEDEF_TELEPORT;
    ld2->flags |= tag_mask;      /* Always? */
    ld2->tag = s->gate->out_tag;
    ld3->type = LINEDEF_TELEPORT;
    ld3->flags |= tag_mask;      /* Always? */
    ld3->tag = s->gate->out_tag;
    ld4->type = LINEDEF_TELEPORT;
    ld4->flags |= tag_mask;      /* Always? */
    ld4->tag = s->gate->out_tag;
  } else if (0==s->gate->in_tag) {   /* Must be a level-end gate */
    exit_style = TRUE;
    ld1->type = LINEDEF_W1_END_LEVEL;
    ld1->flags |= tag_mask;
    ld2->type = LINEDEF_W1_END_LEVEL;
    ld2->flags |= tag_mask;
    ld3->type = LINEDEF_W1_END_LEVEL;
    ld3->flags |= tag_mask;
    ld4->type = LINEDEF_W1_END_LEVEL;
    ld4->flags |= tag_mask;
  }   /* Otherwise an in-only gate */
  if (exit_style) {
    innersec->floor_height = outersec->floor_height + 16;
    if (c->gate_exitsign_texture) {
      ld1->right->lower_texture =
        ld2->right->lower_texture =
        ld3->right->lower_texture =
        ld4->right->lower_texture = c->gate_exitsign_texture;
    } else {
      ld1->right->lower_texture =
        ld2->right->lower_texture =
        ld3->right->lower_texture =
        ld4->right->lower_texture = ThisStyle->support0;
    }
    s->middle_enhanced = TRUE;
    innersec->ceiling_flat = gateflat;
  }
  if (s->gate->out_tag || exit_gate)
    if (innersec->ceiling_flat->props&LIGHT)
      if (innersec->ceiling_height-innersec->floor_height>=96)
        if (!s->gate->gate_lock) {
          innersec->ceiling_height -= 16;
          ld1->right->upper_texture =
            ld2->right->upper_texture =
            ld3->right->upper_texture =
            ld4->right->upper_texture = ThisStyle->support0;
        }
  ld1->flags &= ~LOWER_UNPEGGED;
  ld2->flags &= ~LOWER_UNPEGGED;
  ld3->flags &= ~LOWER_UNPEGGED;
  ld4->flags &= ~LOWER_UNPEGGED;

}  /* end install_gate */

/* Install the locked/hidden thing that contains the exit that */
/* leads to a Secret Level.  If it works, set the sl_tag and */
/* sl_type things in the level.  If <opens>, make it openable, */
/* and immediately set sl_done. */
boolean install_sl_exit(level *l,sector *oldsector,haa *ThisHaa,
                        style *ThisStyle, quest *ThisQuest,
                        boolean opens, config *c)
{
  int i, tries;
  linedef *ld, *ld2, *ld3;
  short tag;
  sector *newsec;
  boolean found;

  for (found=FALSE,tries=0,ld=NULL;(!found)&&(tries<5);tries++) {
    i = mark_decent_boundary_linedefs(l,oldsector,72);
    ld = random_marked_linedef(l,i);
    unmark_linedefs(l);
    if (ld==NULL) return FALSE;
    if (empty_left_side(l,ld,8)) found = TRUE;
  }
  ld2 = install_switch(l,ld,TRUE,FALSE,0,ThisStyle,c,&ld3);
  if (ld==ld2) {  /* Didn't recess after all?? */
    announce(WARNING,"Silly switch left sitting around?");
    /* Try to fix it */
    ld->right->middle_texture = ThisStyle->wall0;
    ld->type = LINEDEF_NORMAL;
    return FALSE;
  }
  tag = new_tag(l);
  ld2->type = LINEDEF_S1_SEC_LEVEL;
  newsec = ld2->right->sector;
  newsec->special = GLOW_BLINK;
  newsec->light_level = 255;
  newsec->ceiling_height = newsec->floor_height;
  l->sl_tag = tag;
  if (opens) {
    ld3->type = LINEDEF_NORMAL_S1_DOOR;
    announce(VERBOSE,"Openable sl exit");
    l->sl_done = TRUE;
  } else {
    newsec->tag = tag;
    l->sl_type = LINEDEF_W1_OPEN_DOOR;       /* Or S1, eh?  So... */
    ld->flags |= SECRET_LINEDEF;
    if (ThisQuest->goal==LEVEL_END_GOAL) {
      l->sl_open_ok = TRUE;
    } else {
      l->sl_open_ok = FALSE;
      l->sl_open_start = ThisQuest->room;
    }
    l->sl_exit_sector = oldsector;
    announce(VERBOSE,"Installed sl exit");
  }
  return TRUE;
}

/* Try to put a triggerbox around something in this sector, */
/* to open the sl exit thing. */
void try_sl_triggerbox(level *l, sector *oldsector, config *c)
{
  boolean danger;
  thing *t;
  int border;

  for (t=l->thing_anchor;t;t=t->next) {
    if (!(t->genus->bits&PICKABLE)) continue;
    if (oldsector!=point_sector(l,t->x,t->y,&border,&danger)) continue;
    if (border<32) continue;
    if (danger) continue;
    if ((t->options&0x07)!=0x07) continue;
    break;
  }  /* end for things */
  if (t) {
    trigger_box(l,t,oldsector,l->sl_tag,l->sl_type,c);
    l->sl_done = TRUE;
    l->sl_open_ok = FALSE;
    announce(VERBOSE,"Did sl triggerbox");
  }  /* if found a good thing */
}

/* Fancy up the room, put stuff in it, install gates, etc. */
void enhance_room(level *l,sector *oldsector,haa *ThisHaa,style *ThisStyle,
                  quest *ThisQuest,boolean first, config *c)
{
  boolean done_room = FALSE;
  boolean did_dm = FALSE;

  if ((ThisQuest) && (ThisQuest->goal != NULL_GOAL) &&
      (need_secret_level(c)) && (l->sl_tag == 0) &&
      (rollpercent(20)||(ThisQuest->count+4>ThisQuest->minrooms)) ) {
    install_sl_exit(l,oldsector,ThisHaa,ThisStyle,ThisQuest,FALSE,c);
  }

  if ((first)&&(oldsector->gate)) {
    announce(WARNING,"Gate and watermark do not mix!");
  }

  if ((!done_room)&&oldsector->middle_enhanced) {
    /* Someone else did everything else */
    embellish_room(l,oldsector,ThisHaa,ThisStyle,ThisQuest,first,TRUE,c);
    done_room = TRUE;
  }

  if ((!done_room)&&oldsector->gate) {
    install_gate(l,oldsector,ThisStyle,ThisHaa,FALSE,c);
    gate_populate(l,oldsector,ThisHaa,first,c);  /* Very special-purpose! */
    embellish_room(l,oldsector,ThisHaa,ThisStyle,ThisQuest,first,TRUE,c);
    done_room = TRUE;
  }

  if ((!done_room)&&(!first)&&(!oldsector->has_key)
      &&rollpercent(l->p_special_room)) {
    if (grid_room(l,oldsector,ThisHaa,ThisStyle,ThisQuest,first,c)) {
      embellish_room(l,oldsector,ThisHaa,ThisStyle,ThisQuest,first,TRUE,c);
      done_room = TRUE;
    }
  }

  if (!done_room) {  /* Simple randomly-enhanced room */
    populate(l,oldsector,c,ThisHaa,first);  /* or after embellish? */
    embellish_room(l,oldsector,ThisHaa,ThisStyle,ThisQuest,first,FALSE,c);
  }

  if (first || rollpercent(l->dm_rho)) {
    did_dm = maybe_add_dm_start(l,oldsector,c,FALSE);
  }

  if (did_dm) {
    l->dm_rho = 10;
  } else {
    if (l->dm_rho < 80) l->dm_rho += (400 / c->minrooms);
  }

  align_textures(l,oldsector,c);

}  /* end enhance_room() */

/* Fancy-up the room, after all links are established, and after  */
/* populating with Things.  If <first>, do the obvious            */
/* SLIGE-mark to the room, and prolly no monsters.                */
void embellish_room(level *l,sector *oldsector,haa *haa,style *ThisStyle,
                    quest *ThisQuest,boolean first, boolean edges_only,
                    config *c)
{
  /* Just some random fun things; assumes rectangles etc */
  int i, border, ldlen, depth;
  int switch_tag = 0;
  linedef *switch_ld = NULL;
  linedef *ld;
  boolean did_ceiling = FALSE;
  boolean install_closet, switch_closet;

  if (rollpercent(10))
    if (oldsector->special==0)
      oldsector->special = RANDOM_BLINK;

  if (first) {   /* Watermark */

    /* Mark the first room as SLIGE generated */
    watermark_sector(l,oldsector,ThisStyle,c);

    l->first_room = oldsector;     /* for later */

  } else if (!edges_only) {

    did_ceiling = ceiling_effect(l,oldsector,ThisStyle,haa,c);

    if (!did_ceiling)
      if (rollpercent(0))     /* Looks silly often! */
        oldsector->ceiling_flat = c->sky_flat;  /* Just a plain open top */

    if (!did_ceiling)
      if (rollpercent(80))  /* high, because often fails */
        do_pillar(l,oldsector,ThisStyle,haa,c);

    if (!did_ceiling)
      if (rollpercent(l->p_new_pillars))
        do_new_pillars(l,oldsector,ThisStyle,haa,c);

  }  /* end things we can't do if watermarking or edges_only */

  /* Edgy-things we can do even if watermarking follow */

  /* Perhaps triggerbox a thing in here, to open the */
  /* secret-level-exit thing, if any */
  if (l->sl_open_ok && rollpercent(30) && (oldsector!=l->sl_exit_sector))
    try_sl_triggerbox(l,oldsector,c);

  /* One or more ambush closets */
  /* This should really be a separate routine! */
  if ((!first)||(c->immediate_monsters))   /* Not right off! */
    if (rollpercent(l->amcl_rho)) {
      int n, k, clen, depth, yoff, x1, y1;
      linedef *ldnew, *ldedge1, *ldedge2;
      texture *t1;
      boolean sky_thing = rollpercent(l->skyclosets);
      boolean crushing = FALSE;
      n = 1+roll(3);
      for (k=0;k<n;k++) {
        i = mark_decent_boundary_linedefs(l,oldsector,64);
        ld = random_marked_linedef(l,i);
        unmark_linedefs(l);
        if (ld!=NULL) {
          t1 = ld->right->middle_texture;
          clen = ThisStyle->closet_width;
#ifndef WIDE_SKIES_OK
          if (sky_thing) if (clen>72) clen = 72;  /* Wide skyc's look lame */
#endif
          ldlen = linelen(ld);
          if (clen>ldlen) clen = ldlen;
          border = ( ldlen - clen ) / 2;
          if (border!=0) {
            ld = split_linedef(l,ld,border,c);
            split_linedef(l,ld,clen,c);
          }
          depth = ThisStyle->closet_depth;
          if (empty_left_side(l,ld,depth)) {
            genus *m;
            int levels;
            short angle, bonustype;
            sector *innersec, *outersec;
            ldnew = lefthand_box_ext(l,ld,depth,ThisStyle,c,&ldedge1,&ldedge2);
            outersec = ldnew->right->sector;
            ldnew->right->middle_texture = ldedge1->right->middle_texture;
            if (rollpercent(50))
              if (oldsector->ceiling_height-oldsector->floor_height>72) {
                outersec->ceiling_height = outersec->floor_height + 72;
                yoff = (oldsector->ceiling_height - oldsector->floor_height) - 72;
                ldnew->right->y_offset = yoff;
                ldedge1->right->y_offset = yoff;
                ldedge2->right->y_offset = yoff;
                patch_upper(ld,t1,c);
              }  /* end if bigger'n 72 */
            if (sky_thing) {
              int minx,miny,maxx,maxy;
              announce(VERBOSE,"Sky closet");
              innersec = clone_sector(l,outersec);
              innersec->ceiling_height += 16;   /* 8?  16? */
              find_rec(l,outersec,&minx,&miny,&maxx,&maxy);
              minx += 8;
              miny += 8;
              maxx -= 8;
              maxy -= 8;
              parallel_innersec(l,outersec,innersec,
                             NULL,outersec->style->wall0,outersec->style->wall0,
                             minx,miny,maxx,maxy,c);
              innersec->ceiling_flat = c->sky_flat;
              innersec->light_level = l->outside_light_level;
              if (outersec->light_level<l->bright_light_level)
                outersec->light_level = oldsector->light_level +
                  roll(l->bright_light_level-oldsector->light_level);
            } else {  /* Not sky closet; maybe nukage and/or crushing */
              outersec->light_level = oldsector->light_level +
                ThisStyle->closet_light_delta;
              if (outersec->light_level<80) outersec->light_level = 80;
              if (outersec->light_level>240) outersec->light_level = 240;
              if (rollpercent(2+l->p_force_nukage/2)) {
                outersec->floor_height -= 8;
                outersec->floor_flat = ThisStyle->nukage1;
                outersec->special = NUKAGE1_SPECIAL;
                if (outersec->light_level<120) outersec->light_level = 120;
                patch_lower(ld,ThisStyle->support0,c);
                announce(VERBOSE,"Nukage ambush");
              }  /* end nukage */
              if (rollpercent(2)       /* "2"?  Crush! */
                  &&(outersec->ceiling_height-outersec->floor_height<=72)
                  &&(l->crushercount<LEVEL_MAX_CRUSHERS)) {
                l->crushercount++;
                ld->type = LINEDEF_WR_FAST_CRUSH;
                ld->tag = new_tag(l);
                ld->right->upper_texture = ThisStyle->wall0;
                ld->flags &= ~UPPER_UNPEGGED;
                ld->right->y_offset -=
                  outersec->ceiling_height - oldsector->ceiling_height;
                ldnew->flags |= LOWER_UNPEGGED;
                ldedge1->flags |= LOWER_UNPEGGED;
                ldedge2->flags |= LOWER_UNPEGGED;
                ldnew->right->y_offset +=
                  outersec->ceiling_height - outersec->floor_height;
                ldedge1->right->y_offset +=
                  outersec->ceiling_height - outersec->floor_height;
                ldedge2->right->y_offset +=
                  outersec->ceiling_height - outersec->floor_height;
                outersec->tag = ld->tag;
                outersec->ceiling_flat = random_flat0(RED,c,NULL);
                if (outersec->light_level>120) outersec->light_level = 120;
                crushing = TRUE;
                announce(VERBOSE,"Crush ambush");
              } /* end if crushing */
              if (oldsector->light_level - outersec->light_level >= 16) {
                linedef *ldnew2;
                announce(VERBOSE,"shadow");
                innersec = clone_sector(l,outersec);
                innersec->tag = outersec->tag;
                innersec->style = oldsector->style;  /* Why? */
                innersec->light_level = oldsector->light_level;
                if (rollpercent(50)) {
                  split_linedef(l,ldedge1,16+roll(20),c);  /* OK depth? */
                  ldedge1->right->sector = innersec;
                  ldnew2 = new_linedef(l,ldedge1->to,ld->to);
                } else {
                  ldedge2 =
                    split_linedef(l,ldedge2,linelen(ldedge2)-(16+roll(20)),c);
                  ldedge2->right->sector = innersec;
                  ldnew2 = new_linedef(l,ld->from,ldedge2->from);
                }
                ldnew2->flags |= TWO_SIDED | NOT_ON_MAP;
                ldnew2->right = new_sidedef(l,innersec,c);
                ldnew2->right->middle_texture = c->null_texture;
                ldnew2->left = new_sidedef(l,outersec,c);
                ldnew2->left->middle_texture = c->null_texture;
                ld->left->sector = innersec;
              } else if ((outersec->style->ceilinglight) && (c->clights)) {
                outersec->ceiling_flat = outersec->style->ceilinglight;
                announce(VERBOSE,"acl");
              }
            }  /* end general lighting effects */
            /* Put something in the closet */
            point_from(ldnew->from->x,ldnew->from->y,ldnew->to->x,ldnew->to->y,
                   RIGHT_TURN,32,&x1,&y1);
            point_from(ldnew->to->x,ldnew->to->y,x1,y1,RIGHT_TURN,32,&x1,&y1);
            /* Eek, a monster! */
            m = timely_monster(haa,c,&levels,rollpercent(l->p_biggest_monsters),1);
            /* Should check for monster width here!! */
            if (!m) {
              new_thing(l,x1,y1,0,ID_POTION,7,c);  /* Punt.  Stub. */
            } else {
              angle = facing_right_from_ld(ldnew);
              new_thing(l,x1,y1,angle,m->thingid,
                        (short)(levels|0x08),c);  /* Deaf */
              if (rollpercent(50))
                if (m->bits&SHOOTS) ld->flags |= BLOCK_MONSTERS;
              update_haa_for_monster(haa,m,levels,0,c);  /* zero?  one? */
            }  /* end there was a monster */
            /* Maybe some small bonus also */
            if (rollpercent(15) && ld->type==0) {  /* Not if ld is a trap? */
              if (rollpercent(50)) {  /* Health or whatever */
                switch (roll(3)) {
                  case 0: bonustype = ID_MEDIKIT; break;
                  case 1: bonustype = ID_STIMPACK; break;
                  default: bonustype = ID_POTION; break;
                }
                update_haa_for_health(haa,7,bonustype);
              } else {   /* Some ammo or whatever */
                if ((!(haa->haas[2].has_chainsaw))&&(rollpercent(5))) {
                  bonustype = ID_CHAINSAW;
                  haa->haas[0].has_chainsaw = TRUE;
                  haa->haas[1].has_chainsaw = TRUE;
                  haa->haas[2].has_chainsaw = TRUE;
                } else {
                  switch (roll(2)) {   /* What about a cell? / a rocket */
                    case 0: bonustype = ID_CLIP; break;
                    default: bonustype = ID_SHELLS; break;
                  }  /* end switch */
                  update_haa_for_ammo(haa,7,bonustype);
                }
              }  /* end ammo bonuses */
              new_thing(l,x1+1,y1+1,0,bonustype,7,c);  /* Place the bonus */
            }  /* end some small bonus */
          }  /* end if empty space */
        }  /* end if found a linedef */
      }  /* end for k */
    }  /* end if 1/n */

  if (rollpercent(15)) {
    /* One or more wall-plaques, */
    /* with occasional secrets and monsters */
    boolean sync_doors = rollpercent(c->p_sync_doors);
    int sync_tag = -1;
    int sync_count = 0;    /* Just for announcing */
    int yoff, pheight, j, pup;
    linedef *ldnew, *ldedge1, *ldedge2;
    texture *t1, *tplaque;
    pheight = ThisStyle->plaque->height;
    if (ThisStyle->plaque->props&HALF_PLAQUE)
      if (rollpercent(80)) pheight = pheight / 2;
    pup = ((oldsector->ceiling_height-oldsector->floor_height)-pheight)/2;
    pup = roll(pup);
    tplaque = ThisStyle->plaque;
    if (oldsector->ceiling_height-oldsector->floor_height>pheight) {
      for (j=1;;j++) {
        i = mark_decent_boundary_linedefs(l,oldsector,128);  /* 128 is wrong */
        ld = random_marked_linedef(l,i);
        unmark_linedefs(l);
        if (ld!=NULL) {
          t1 = ld->right->middle_texture;
          ldlen = linelen(ld);
          /* Use borderize(TRUE), to get possible lightboxes etc */
          if (rollpercent(5)) {
            ThisStyle->lightboxes = TRUE;
            ThisStyle->auxheight = pup;     /* "pheight" here is a nice bug! */
            announce(VERBOSE,"fancy plaque");
          }
          ld = borderize(l,ld,128,TRUE,ThisStyle,LIGHT,NULL,NULL,c);
          ThisStyle->lightboxes = FALSE;   /* Neaten up */
          depth = 4 + roll(5) + roll(5);
          if (empty_left_side(l,ld,depth)) {
            announce(VERBOSE,"Putting in a plaque");
            ldnew = lefthand_box_ext(l,ld,depth,ThisStyle,c,&ldedge1,&ldedge2);
            ldnew->right->middle_texture = tplaque;
            if (tplaque!=ThisStyle->plaque) announce(VERBOSE,"Multiplaque");
            ldnew->right->sector->floor_height = oldsector->floor_height+pup;
            ldnew->right->sector->ceiling_height =
              ldnew->right->sector->floor_height + pheight;
            /* Maybe light the recesses */
            if ((ThisStyle->light_recesses)&&(ThisStyle->walllight!=NULL)) {
              announce(VERBOSE,"Lit plaque");
              ldedge2->right->middle_texture = ldedge1->right->middle_texture =
                ThisStyle->walllight;
            } else {
              yoff = oldsector->ceiling_height - ldnew->right->sector->ceiling_height;
              ldedge1->right->y_offset = yoff;
              ldedge2->right->y_offset = yoff;
            }
            patch_upper(ld,t1,c);
            patch_lower(ld,t1,c);
            if (rollpercent(60)) {
              ldnew->right->sector->light_level = oldsector->light_level + roll(21);
              if (ldnew->right->sector->light_level>l->bright_light_level)
                ldnew->right->sector->light_level = oldsector->light_level;
              if (rollpercent(40)) switch (roll(4)) {
                case 0: ldnew->right->sector->special = RANDOM_BLINK; break;
                case 1: ldnew->right->sector->special = SYNC_FAST_BLINK; break;
                case 2: ldnew->right->sector->special = SYNC_SLOW_BLINK; break;
                case 3: ldnew->right->sector->special = GLOW_BLINK; break;
              }  /* end switch */
            }  /* end if doing lights */
            if (pup<25)
              if (rollpercent(80)) {   /* Put a secret thingie behind it! */
                if (sync_doors) if (sync_tag==-1) sync_tag = new_tag(l);
                if (NULL!=secret_closet(l,ldnew,ThisStyle,0,haa,c,TRUE,
                                        sync_tag,oldsector->ceiling_height,TRUE)) {
                  announce(VERBOSE,"Plaque closet");
                  if (sync_doors) {
                    ldnew->tag = sync_tag;
                    ldnew->type = LINEDEF_SR_OC_DOOR;
                    if (!(c->gamemask&DOOM0_BIT))
                      ldnew->type = LINEDEF_SR_BLAZE_OC_DOOR;
                    if (sync_count++) announce(VERBOSE,"Synced doors");
                  }
                }
              }  /* end if secret closet */
          }  /* end if empty space */
        }  /* end if found a linedef */
        if (rollpercent(50)) break;
        if (j>4) break;
        tplaque = random_plaque(c,ThisStyle);
        if ((tplaque->height != pheight) &&
            ((!(tplaque->props&HALF_PLAQUE)) || (tplaque->height != 2*pheight)) ) {
          tplaque = ThisStyle->plaque;
        }
      }  /* end for j */
    }  /* end if tall enough room */
  }  /* end if 1/10 */

  /* The other kind(s) of secret closet */
  install_closet = FALSE;
  switch_closet = FALSE;
  if (rollpercent(l->p_surprise)) {
    install_closet = TRUE;
  } else if (rollpercent(l->p_swcloset)) {
    int i = mark_decent_boundary_linedefs(l,oldsector,72);
    switch_ld = random_marked_linedef(l,i);
    unmark_linedefs(l);
    if (switch_ld && empty_left_side(l,switch_ld,8)) {
      install_closet = TRUE;
      switch_closet = TRUE;
    }
  }
  if (install_closet) {
    thing *t = NULL;
    short tag = -1;
    int plen, pheight;
    boolean goal_trigger, had_map;
    linedef *ldc;

    i = mark_decent_boundary_linedefs(l,oldsector,128);
    ld = random_marked_linedef(l,i);
    unmark_linedefs(l);
    if (switch_closet && (ld==switch_ld)) ld = NULL;  /* Give up */
    if (ld!=NULL) {
      ldlen = linelen(ld);
      plen = ldlen - 64;
      if (rollpercent(50))
        if (plen>64)
          plen = plen - roll( 1 + plen-64 );
      if (plen>256) plen=256;
      border = ( ldlen - plen ) / 2;
      if (border!=0) {
        ld = split_linedef(l,ld,border,c);
        split_linedef(l,ld,plen,c);
      }
      goal_trigger = FALSE;
      if (empty_left_side(l,ld,72)) {     /* "72" is from secret_closet() */
        if (((ThisQuest->goal==KEY_GOAL)||(ThisQuest->goal==NULL_GOAL)) &&
            (!switch_closet) &&
            (ThisQuest->auxtag==0)&&(ThisQuest->surprise==NULL)) {
          /* Goal-triggered, if we can */
          goal_trigger=TRUE;
          tag = new_tag(l);
          ThisQuest->auxtag = tag;
        } else if (switch_closet) {    /* Switch-triggered */
          switch_tag = new_tag(l);
          tag = switch_tag;
        } else if (rollpercent(60)) {   /* Immediate-triggered */
          boolean danger;
          for (t=l->thing_anchor;t;t=t->next) {
            if (!(t->genus->bits&PICKABLE)) continue;
            /* Old bug: &border in next line was NULL */
            if (oldsector!=point_sector(l,t->x,t->y,&border,&danger)) continue;
            if (border<32) continue;
            if (danger) continue;
            if ((t->options&0x07)!=0x07) continue;
            break;
          }  /* end for things */
          if (t) {
            tag = new_tag(l);
            trigger_box(l,t,oldsector,tag,LINEDEF_WR_OPEN_DOOR,c);
          }  /* if found a good thing */
        }  /* if triggered closet */
        pheight = 72 + roll( 1 +
          (oldsector->ceiling_height - oldsector->floor_height) - 72);
        had_map = l->seen_map;
        ldc=secret_closet(l,ld,ThisStyle,pheight,(goal_trigger)?NULL:haa,
                           c,(boolean)(tag==-1),tag,oldsector->ceiling_height,
                           (boolean)(tag==-1));
        if (NULL!=ldc) {
          if (switch_closet) {
            switch_ld = install_switch(l,switch_ld,TRUE,FALSE,0,ThisStyle,c,NULL);
            switch_ld->tag = switch_tag;
            if (DOOM0_BIT&c->gamemask) switch_ld->type = LINEDEF_S1_OPEN_DOOR;
              else switch_ld->type = LINEDEF_S1_BLAZE_O_DOOR;
            announce(VERBOSE,"Switch closet");
          }
          ld->right->y_offset = (oldsector->ceiling_height -
            oldsector->floor_height) - 128;  /* 128 should be tex-height */
          ld->flags |= SECRET_LINEDEF;
          if (tag==-1) {  /* Need a subtle hint here */
            boolean hinted=FALSE;
            if (rollpercent(5)) {
              /* Use a barrel or candle */
              genus *g = random_barrel(c,ThisStyle);
              int x = (ld->from->x+ld->to->x)/2;
              int y = (ld->from->y+ld->to->y)/2;
              if ((g==NULL)||rollpercent(50))
                g = find_genus(c,ID_CANDLE);
              point_from(ld->from->x,ld->from->y,x,y,
                RIGHT_TURN,g->width/2,&x,&y);
              if (room_at(l,g,x,y,g->width/2,c)) {
                hinted=TRUE;
                new_thing(l,x,y,0,g->thingid,7,c);
              }
            }
            if ((!hinted) && had_map && rollpercent(15)) {
              /* Make 'em use the map! */
              hinted = TRUE;
            }
            if ((!hinted)&&rollpercent(40)&&
                (ld->right->upper_texture->y_hint!=0)) {
              /* Typical misalign-hint */
              ld->right->y_misalign = ld->right->upper_texture->y_hint;
              hinted = TRUE;
            }
            if ((!hinted)&&rollpercent(90) &&
                (ld->right->upper_texture->subtle!=NULL)) {
              /* Subtly different texture */
              ld->right->upper_texture = ld->right->upper_texture->subtle;
              announce(VERBOSE,"subtle");
              hinted = TRUE;
            }
            if (!hinted) {
              /* Make it just show on the automap (always possible) */
              ld->flags &= ~SECRET_LINEDEF;
              announce(VERBOSE,"Map hint");
              hinted = TRUE;
            }
          }
          if (goal_trigger) {
            ThisQuest->surprise = ldc;
            announce(VERBOSE,"Goal-trigger");
          } else if (tag!=-1) announce(VERBOSE,"Trigger");
        } else if (goal_trigger) {
          ThisQuest->auxtag = 0;
        }  /* end else if secret closet failed and we were goal triggered */
      }  /* end if enough room */
    }  /* end if found a linedef */
  }  /* end if try a closet */

  if (rollpercent(l->p_extroom))
    make_extroom(l,oldsector,haa,ThisStyle,c);

  if (rollpercent(l->p_extwindow)) {  /* Trivial windows to the outside */
    make_extwindow(l,oldsector,ThisStyle,c);
  }  /* end if try a window */

  if (rollpercent(3) && (ThisStyle->walllight!=NULL)) {
    /* Lightstrips on the walls */
    int ll,sdepth,spec,fh,ch;
    announce(VERBOSE,"Doing the lightstrip thing.");
    sdepth = 4 + 4 * roll(2);
    ll = oldsector->light_level;
    if (ll<l->lit_light_level) ll+= 20;
    /* Mess with the light-motion sometimes */
    if (rollpercent(15)) switch (roll(4)) {
      case 0: spec = RANDOM_BLINK; break;
      case 1: spec = SYNC_FAST_BLINK; break;
      case 2: spec = SYNC_SLOW_BLINK; break;
      default: spec = GLOW_BLINK; break;
    } else spec = 0;
    fh = oldsector->floor_height + 8 * roll(8);  /* Boring? */
    ch = fh + roll(24) + 24 * roll(2);
    if (ch>oldsector->ceiling_height) ch = oldsector->ceiling_height;
    for (ld=l->linedef_anchor;ld;ld=ld->next)
      if (ld->type==0)
        if (ld->right)
          if (ld->left==NULL)
            if (ld->right->sector==oldsector)
              if (lengthsquared(ld)>(8*8))
                if (ld->right->isBoundary)
                  make_lightstrip(l,ld,ThisStyle,ll,sdepth,spec,fh,ch,c);
  } else if (rollpercent(20)) {
    /* swell some boundaries; don't do if lightstripped! */
    /* NOTE: produces some non-square lines!   Terrible bugs! */
    /* But not as bad if you populate() first! */
    int sno, sdepth;
    announce(VERBOSE,"Swelling the room boundaries");
    sno = 1 + roll(2);
    sdepth = 4 + 4 * roll(4);
    for (ld=l->linedef_anchor;ld;ld=ld->next)
      if (ld->type==0)
        if (ld->left==NULL)
          if (ld->right)
            if (ld->right->sector==oldsector)
              if (lengthsquared(ld)>(16*16))
                if (lengthsquared(ld)>(sdepth*sdepth))
                  if (ld->right->isBoundary)
                    swell_linedef(l,ld,ThisStyle,c,sno,sdepth);
  }  /* end if swell-embellish */

  return;
}

/* Return a gate-type link.  Not many properties! */
link *gate_link(level *l,config *c)
{
  link *answer = (link *)malloc(sizeof (*answer));

  answer->bits = 0;
  answer->type = GATE_LINK;
  answer->next = l->link_anchor;
  l->link_anchor = answer;
  return answer;
}


/* From the given sector of the given level, make a next room nearby, */
/* and return the linedefs by which they should be joined.  NULL if   */
/* no room could be placed.  The returned linedef is the one in the   */
/* new room; the **ldf one is the one in oldsector.                   */
linedef *make_next_room(level *l,sector *oldsector,boolean radical,config *c,
                         linedef **ldf, link **ThisLink, quest *ThisQuest)
{
  linedef *newldf;
  int i, tries;
  boolean try_reduction;
  sector *newsector;
  style *ThisStyle,*NewStyle;

#ifdef DEBUG_QUEST_STACK
  {
    quest *q = ThisQuest;
    for (;q;q=q->next) printf("%d of %d; ",q->count,q->minrooms);
    printf("\n");
  }
#endif

  ThisStyle = oldsector->style;
  newldf = NULL;
  newsector = NULL;
  NewStyle = new_style(l,ThisStyle,radical,c);
  for (try_reduction=FALSE;;) {
    /* this loop is a hack, eh? */
    for (tries=0;tries<20;tries++) {
      i = mark_adequate_linedefs(l,oldsector,ThisStyle,c);
      *ldf = random_marked_linedef(l,i);
      unmark_linedefs(l);
      if (i==0) return NULL;
      /* Decide on a link-style for this link */
      if (roll(3)&&(link_fitsq(ThisStyle->link0,ThisQuest))) {
        *ThisLink = ThisStyle->link0;  /* Often use style default */
      } else {
        /* Sometimes not */
        *ThisLink = random_link(l,*ldf,ThisStyle,ThisQuest,c);
      }
      /* If we're getting really desparate, maybe use a gate */
      if (l->use_gates&&try_reduction&&(tries>15)&&(ThisQuest==NULL)) {
        i = mark_adequate_linedefs(l,oldsector,ThisStyle,c);
        *ldf = random_marked_linedef(l,i);
        unmark_linedefs(l);
        *ThisLink = gate_link(l,c);
        announce(VERBOSE,"Gate link");
      }
      if (!link_fitsh(*ldf,*ThisLink,c))    /* Fix if doesn't fit */
        *ThisLink = random_link(l,*ldf,ThisStyle,ThisQuest,c);
      if (!link_fitsh(*ldf,*ThisLink,c))
        announce(WARNING,"random_link() returned too wide!!");
      newldf = make_linkto(l,*ldf,*ThisLink,NewStyle,c,newldf);
      if (!link_fitsv(l,*ldf,newldf,*ThisLink)) {
        announce(VERBOSE,"Retrying because link didn't fit...");
        continue;
      }
      newsector = generate_room_outline(l,newldf,NewStyle,try_reduction,c);
      if (newsector) break;
      announce(VERBOSE,"Retrying because new room didn't fit...");
    }  /* end until one works */
    if (newsector) break;
    if (try_reduction) break;
    try_reduction = TRUE;
  }  /* end with and without reduction */
  if (newsector==NULL) {
    if (newldf) {  /* Avoid engine crashes! */
      delete_vertex(l,newldf->from);
      delete_vertex(l,newldf->to);
      delete_linedef(l,newldf);
    }
    newldf = NULL;
  }

  return newldf;

}  /* end make_next_room() */

/* Place the start positions for Players 1-4 in the given sector */
void place_start_things(level *l,sector *s,config *c)
{
  int minx, miny, maxx, maxy;
  short angle;
  boolean rational_angles = rollpercent(90);

  find_rec(l,s,&minx,&miny,&maxx,&maxy);

  /* this is invisible, but ensures that it won't work w/shareware DOOM */
  new_thing(l,(minx+maxx)/2,(miny+maxy)/2,90,0x0017,7,c);  /* Dead skull. */

  /* Now the start positions */
  if (rational_angles) angle = 0; else angle = 90*roll(4);
  new_thing(l,minx+32,miny+32,angle,ID_PLAYER1,7,c);  /* 1-player start */
  /* In the first room, *lie* about where the player comes in */
  s->entry_x = maxx-32;
  s->entry_y = maxy-32;
  if ((maxx-minx<128)||(maxy-miny<128))  {
    announce(WARNING,"Not enough room for co-op start positions");
    return;
  }
  if (rational_angles) angle = 0; else angle = 90*roll(4);
  new_thing(l,minx+32,maxy-32,angle,ID_PLAYER2,7,c);  /* 2-player start */
  if (rational_angles) angle = 180; else angle = 90*roll(4);
  new_thing(l,maxx-32,miny+32,angle,ID_PLAYER3,7,c);  /* 3-player start */
  if (rational_angles) angle = 180; else angle = 90*roll(4);
  new_thing(l,maxx-32,maxy-32,angle,ID_PLAYER4,7,c);  /* 4-player start */

}

/* Set all the fields of the given level to empty things */
void empty_level(level *l, config *c)
{
   int dieroll;

   l->thing_anchor = NULL;
   l->vertex_anchor = NULL;
   l->sector_anchor = NULL;
   l->linedef_anchor = NULL;
   l->sidedef_anchor = NULL;
   l->style_anchor = NULL;
   l->link_anchor = NULL;
   l->arena_anchor = NULL;
   l->gate_anchor = NULL;
   l->used_red = FALSE;
   l->used_blue = FALSE;
   l->used_yellow = FALSE;
   l->last_tag_used = 0;
   l->sl_tag = 0;
   l->sl_type = 0;
   l->sl_done = FALSE;
   l->sl_open_ok = FALSE;
   l->sl_open_start = NULL;
   l->sl_exit_sector = NULL;
   l->first_room = NULL;
   l->goal_room = NULL;
   l->seen_suit = FALSE;
   l->seen_map = FALSE;
   l->scrolling_keylights = rollpercent(5);
   l->support_misaligns = rollpercent(2);   /* Looks crummy! */
   l->skyclosets = 2;
   if (rollpercent(10)) l->skyclosets = roll(100);
   l->lift_rho = 10;
   if (rollpercent(25)) l->lift_rho = 0;
   if (rollpercent(15)) l->lift_rho = roll(100);
   l->amcl_rho = 30;
   if (rollpercent(25)) l->amcl_rho = 0;
   if (rollpercent(15)) l->amcl_rho = roll(100);
   l->p_new_pillars = 30;
   if (rollpercent(10)) l->p_new_pillars = 0;
   if (rollpercent(8)) l->p_new_pillars = 80 + roll(40);
   l->p_stair_lamps = 20;
   if (rollpercent(5)) l->p_stair_lamps = 50 + roll(60);
   l->p_force_sky = 0;
   if (rollpercent(5)) l->p_force_sky = 20 + roll(60);
   if (l->p_force_sky>30) announce(LOG,"Sunrooms");
   l->p_force_nukage = 0;
   if (rollpercent(8)) l->p_force_nukage = 20 + roll(60);
   if (c->major_nukage) l->p_force_nukage = 85;
   if (l->p_force_nukage>30) announce(LOG,"Nukage city!!");
   l->p_deep_baths = 20;
   if (rollpercent(50)) l->p_deep_baths += l->p_force_nukage;
   if (rollpercent(8)) l->p_deep_baths = 75 + roll(30);
   if (rollpercent(8)) l->p_deep_baths = 0;
   l->p_falling_core = 0;
   if (rollpercent(25)) l->p_falling_core = 5;
   if (rollpercent(5)) l->p_falling_core = 5 + roll(30);
   l->p_barrels = 10;
   if (rollpercent(8)) l->p_barrels = 20 + roll(30);
   if (l->p_force_nukage>30)
     if (rollpercent(50))
       l->p_barrels = l->p_force_nukage;
   l->p_extwindow = 8;
   if (rollpercent(5)) l->p_extwindow = 15 + roll(75);
   if (rollpercent(5)) l->p_extwindow = 0;
   l->p_extroom = 2;
   if (rollpercent(5)) l->p_extroom = 15 + roll(75);
   if (rollpercent(10)) l->p_extroom = 0;
   l->p_rising_room = 0;
   if (rollpercent(50)) l->p_rising_room = 6;
   if (rollpercent(5)) l->p_rising_room = 25 + roll(75);
   if (l->p_force_sky>30) {
     if (rollpercent(60))
       l->p_extwindow = l->p_force_sky;
     if (rollpercent(60))
       l->skyclosets = l->p_force_sky;
     if (rollpercent(60))
       l->p_extroom = l->p_force_sky;
   }
   l->p_surprise = 30;
   if (rollpercent(10)) l->p_surprise = 30 + roll(60);
   l->p_swcloset = 0;
   if (rollpercent(20)) l->p_swcloset = 5;
   if (rollpercent(10)) l->p_swcloset = 5 + roll(20);
   l->p_rational_facing = 90;
   if (rollpercent(2)) l->p_rational_facing = roll(100);
   if (rollpercent(10)) l->p_rational_facing = 100;
   {
   char s[80];
   sprintf(s,"p_rational_facing %d.",l->p_rational_facing);
   announce(VERBOSE,s);
   }
   l->p_biggest_monsters = 0;
   if (rollpercent(5)&&(c->big_monsters)) l->p_biggest_monsters = 100;
   if (c->force_biggest) l->p_biggest_monsters = 100;
   if (l->p_biggest_monsters==100) announce(LOG,"Biggest monsters");
   l->p_open_link = 15;
   if (rollpercent(15)) l->p_open_link = 0;
   if (rollpercent(20)) l->p_open_link = roll(100);
   l->p_s1_door = 20;
   if (rollpercent(10)) l->p_s1_door = roll(100);
   if (rollpercent(5)) l->p_s1_door = 100;
   if (l->p_s1_door>95)
     announce(VERBOSE,"Doors stick");
   l->p_special_room = 2 + roll(5);
   if (rollpercent(5)) l->p_special_room = 0;
   if (rollpercent(5)) l->p_special_room = 20 + roll(20);
   l->secret_count = 0;
   l->dm_count = 0;
   l->dm_rho = 10;
   l->first_room = NULL;
   l->skullkeys = rollpercent(50);
   l->use_gates = rollpercent(60);
   l->raise_gates = rollpercent(60);
   l->no_doors =
     l->all_wide_links = FALSE;
   if (rollpercent(15)) switch(roll(6)) {
     case 0:
     case 1:
     case 2: l->all_wide_links =
               l->no_doors = TRUE; break;
     case 3: l->all_wide_links = TRUE; break;
     case 4: l->no_doors = TRUE; break;
     default: break;
   }
   if (l->all_wide_links)
     announce(VERBOSE,"All wide links");
   if (l->no_doors)
     announce(VERBOSE,"No doors");
   l->hugeness = 1;
   if (rollpercent(c->do_dm ? 30 : 8)) l->hugeness = 2;
   if (l->hugeness < c->minhuge) l->hugeness = c->minhuge;
   if (l->hugeness>1) announce(LOG,"Extra hugeness");
   l->outside_light_level = 240;
   if (rollpercent(20)) {
     l->outside_light_level = c->minlight + 5;
     announce(VERBOSE,"Night");
   }
   l->bright_light_level = 220;
   if (rollpercent(20)) {
     l->bright_light_level = c->minlight +
       roll((221-c->minlight)/2);
     announce(VERBOSE,"Dim");
   }
   l->lit_light_level = 220;   /* Always? */
   dieroll = roll(100);
   if (dieroll<30) l->maxkeys = 0;
     else if (dieroll<60) l->maxkeys = 1;
       else if (dieroll<80) l->maxkeys = 2;
         else l->maxkeys = 3;
   l->barcount = 0;
   l->crushercount = 0;
}

/* Make a whole new level, assuming the player starts with the given */
/* amount of health, ammo, and armor, using the given config.        */
void NewLevel(level *l, haa *ThisHaa, config *c)
{
   style *ThisStyle, *NewStyle;    /* basic decors */
   quest *ThisQuest;               /* stack of pending goals */
   link *ThisLink, *ForkLink;      /* Particular instances */
   linedef *ldf, *newldf = NULL;
   sector *oldsector, *newsector = NULL;
   int i, forks, nullforks;
   boolean done_quest = FALSE;
   boolean first_room = TRUE;
   int keys_used = 0;

   empty_level(l,c);

   ThisStyle = random_style(l,c);
   ThisQuest = starting_quest(l,c);

   ldf = starting_linedef(l,ThisStyle,c);
   oldsector = generate_room_outline(l,ldf,ThisStyle,TRUE,c);
   l->first_room = oldsector;

   /* Make starting position(s) in the first sector */
   place_start_things(l,oldsector,c);

   /* first call to embellish() will add the SLIGE watermark */

   NewStyle = NULL;

   for (;;) {

     /* Are we done by virtue of room-count or similar? */
     ThisQuest->count++;
     if (enough_quest(l,oldsector,ThisQuest,c)) done_quest = 1;
       else done_quest = 0;

     if (!done_quest) {
       newldf = make_next_room(l,oldsector,FALSE,c,&ldf,&ThisLink,NULL);
       if (newldf==NULL) {
         done_quest = 1;
         if (ThisQuest->next==NULL) {
           announce(LOG,"Self-collision; may be fewer rooms than expected.");
         } else {
           ThisQuest->next->minrooms +=
             ThisQuest->minrooms - ThisQuest->count;
         }
       } else {
         newsector = newldf->right->sector;
         NewStyle = newsector->style;
       }
     }

     paint_room(l,oldsector,ThisStyle,c);

     if (!done_quest) {
       establish_link(l,ldf,newldf,ThisLink,NULL,ThisStyle,NewStyle,ThisHaa,c);
       maybe_push_quest(l,oldsector,ThisQuest,c);
     } else {
       close_quest(l,oldsector,ThisQuest,ThisHaa,c);
     }

     /*
        forking in here.  design: if we want to fork, do
        another make_next_room.  give up if fails.  if it
        works, do an establish_locked_link, and push the
        new quest onto the stack.  if the fork works, the
        establish_link above will have established a link
        to newsector in the new quest.

        pull out all the meat into a separate function sometime.
     */
     /* Fork some number of times */
     for (nullforks=0,forks=0;forks<4;forks++) {
       linedef *lld1, *lld2;
       boolean radical;
       short newkey;
       if (done_quest) break;
       if (nullforks) break;    /* Only one of these at a time */
       if ((forks==0)&!rollpercent(15)) break;  /* 85% of rooms don't fork */
       if ((forks!=0)&!rollpercent(30)) break;  /* 70% don't fork any more */
       /* This next bit should be in a routine */
       ThisQuest = push_quest(ThisQuest);
       if ((keys_used>=l->maxkeys)||
           (rollpercent(16))||
           (ThisQuest->next->goal==NULL_GOAL)) {
         ThisQuest->goal = NULL_GOAL;
       } else if (rollpercent(60) && (0!=(newkey=new_key(l)))) {
         ThisQuest->goal = KEY_GOAL;
         ThisQuest->type = newkey;
       } else if (rollpercent(10)) {
         ThisQuest->goal = GATE_GOAL;
         /* Everything else decided later */
       } else {
         ThisQuest->goal = SWITCH_GOAL;
         ThisQuest->tag = new_tag(l);
         announce(LOG,"switch quest");
       }
       radical = (ThisQuest->goal != NULL_GOAL);
       lld1 = make_next_room(l,oldsector,radical,c,&lld2,&ForkLink,ThisQuest);
       if (lld1) {
         announce(VERBOSE,"Fork");
         if (forks) announce(LOG,"Multifork");
         establish_link(l,lld2,lld1,ForkLink,ThisQuest,ThisStyle,
                        lld1->right->sector->style,ThisHaa,c);
         ThisQuest->room = lld1->right->sector;
         if (ThisQuest->goal==NULL_GOAL) {
           ThisQuest->minrooms = 1 + roll(4);
           nullforks++;
         } else {
           ThisQuest->minrooms =
             1 + roll(ThisQuest->next->minrooms-ThisQuest->next->count);
         }
         ThisQuest->next->minrooms -= ThisQuest->minrooms;
         if (ThisQuest->next->minrooms<1)
           ThisQuest->next->minrooms = 1;
         if (ThisQuest->goal==KEY_GOAL) {
           announce(LOG,"Key thing");
           keys_used++;
         }
       } else {
         ThisQuest = pop_quest(ThisQuest);  /* Oh, well! */
         break;   /* No sense in trying any more, eh? */
       }
     }  /* end for */

     /* See if it's OK to put in a secret-level exit-opener yet */
     if (oldsector==l->sl_open_start) l->sl_open_ok = TRUE;

     /* Fancy up and fill in the room itself */
     enhance_room(l,oldsector,ThisHaa,ThisStyle,ThisQuest,first_room,c);
     first_room = FALSE;

     /* Now get ready for the next pass */
     if (!done_quest) {
       oldsector = newsector;
       ThisStyle = NewStyle;
     } else {
       close_quest_final(l,oldsector,ThisQuest,ThisHaa,c);
       if (ThisQuest->next==NULL) break;  /* We're done! */
       oldsector = ThisQuest->room;
       ThisStyle = oldsector->style;
       ThisQuest = pop_quest(ThisQuest);
     }

   }  /* end forever */

   /* Hack to avoid ammo-starvation in megawads due to leftbehinds */
   /* Also turn off berserk effect */
   for (i=ITYTD;i<=UV;i++) {
     ThisHaa->haas[i].ammo *= (float)0.75;   /* awful! */
     ThisHaa->haas[i].has_berserk = FALSE;
   }

   /* Sometimes turn on big stuff; probably too simple */
  if (c->big_weapons) c->big_monsters |= rollpercent(50);
    else c->big_monsters |= rollpercent(15);
  if (c->big_monsters) c->big_weapons |= rollpercent(50);
    else c->big_weapons |= rollpercent(15);

   /* Do some final global twiddles */
   global_paint_HOMs(l,c);
   global_align_textures(l,c);
   global_fixups(l);

   /* Warn if we failed to do a secret level */
   if (need_secret_level(c) && !l->sl_done)
     announce(WARNING,"Secret level(s) may be unreachable; durn!");

   /* Add emergency deathmatch starts, if -dm and needed, announce count */
   if (c->do_dm) {
     char s[80];
     for (;l->dm_count<4;) {
       if (maybe_add_dm_start(l,l->first_room,c,TRUE)) continue;
       if (maybe_add_dm_start(l,l->goal_room,c,TRUE)) continue;
       announce(ERROR,"Not enough deathmatch starts!");
       break;
     }
     sprintf(s,"%d deathmatch starts.",l->dm_count);
     announce(LOG,s);
   }

   /* and finally, always have at least one "secret", for the 100% */
   if (l->secret_count==0)
     if (l->first_room)
       l->first_room->special = SECRET_SECTOR;

}

/****** the end of SLIGE.C ********* please come again *********/

/*
    Now admit it; what was more fun than farting around with
    level-editors, wasn't it?
*/
