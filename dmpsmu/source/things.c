/*
   DooM PostScript Maps Utility, by Frans P. de Vries.

Derived from:

   Doom Editor Utility, by Brendon Wyber and Raphaël Quinet.

   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README for more information.

   This program comes with absolutely no warranty.

   THINGS.C - Thing class and size routines.
*/


/* the includes */
#include "dmps.h"
#include "things.h"


/*
   get the class of a thing
*/
BCINT GetThingClass( BCINT type)
{
   if (GameVersion < 16) /* DOOM Things */
      switch( type)
      {
      /* Starting points */
      case THING_PLAYER1:
      case THING_PLAYER2:
      case THING_PLAYER3:
      case THING_PLAYER4:
      case THING_DEATHMATCH:
         return CLASS_START;
      case THING_TELEPORT:
         return CLASS_TELEPORT;

      /* Enhancements & spheres */
      case THING_BLUECARD:
      case THING_YELLOWCARD:
      case THING_REDCARD:
      case THING_BLUESKULLKEY:
      case THING_YELLOWSKULLKEY:
      case THING_REDSKULLKEY:
      case THING_GREENARMOR:
      case THING_BLUEARMOR:
      case THING_SOULSPHERE:
      case THING_MEGASPHERE:	/* ][ */
      case THING_RADSUIT:
      case THING_COMPMAP:
      case THING_BLURSPHERE:
      case THING_BERSERK:
      case THING_INVULN:
      case THING_LITEAMP:
      case THING_BACKPACK:
         return CLASS_ENHANCE;

      /* Armor & health bonuses & small ammo */
      case THING_ARMBONUS1:
      case THING_HLTBONUS1:
      case THING_STIMPACK:
      case THING_MEDIKIT:
      case THING_AMMOCLIP:
      case THING_SHELLS:
      case THING_ROCKET:
      case THING_ENERGYCELL:
         return CLASS_BONUS;

      /* Weapons & bulk ammo */
      case THING_CHAINSAW:
      case THING_SHOTGUN:
      case THING_COMBATGUN:	/* ][ */
      case THING_CHAINGUN:
      case THING_LAUNCHER:
      case THING_PLASMAGUN:
      case THING_BFG9000:
      case THING_AMMOBOX:
      case THING_SHELLBOX:
      case THING_ROCKETBOX:
      case THING_ENERGYPACK:
         return CLASS_WEAPON;

      /* Enemies */
      case THING_HUMAN:
      case THING_SERGEANT:
      case THING_COMMANDO:		/* ][ */
      case THING_IMP:
      case THING_DEMON:
      case THING_SPECTRE:
      case THING_LOSTSOUL:
      case THING_CACODEMON:
      case THING_PAINELEM:		/* ][ */
      case THING_MANCUBUS:		/* ][ */
      case THING_REVENANT:		/* ][ */
      case THING_KNIGHT:		/* ][ */
      case THING_BARON:
      case THING_ARACHNO:		/* ][ */
      case THING_ARCHVILE:		/* ][ */
      case THING_CYBERDEMON:
      case THING_SPIDERBOSS:
      case THING_WOLF3DSS:		/* ][ */
         return CLASS_ENEMY;

      /* Specials */
      case THING_BARREL:
      case THING_KEEN:		/* ][ */
         return CLASS_BARREL;
      case THING_BOSSBRAIN:	/* ][ */
      case THING_BOSSSHOOT:	/* ][ */
      case THING_SPAWNSPOT:	/* ][ */
         return CLASS_DECOR;

      /* Decorations, et al */
      case THING_TECHCOLUMN:
      case THING_TGREENPILLAR:
      case THING_TREDPILLAR:
      case THING_SGREENPILLAR:
      case THING_SREDPILLAR:
      case THING_PILLARHEART:
      case THING_PILLARSKULL:
      case THING_EYEINSYMBOL:
      case THING_GREYTREE:
      case THING_BROWNSTUB:
      case THING_BROWNTREE:
      case THING_LAMP:
      case THING_TALLLAMP:		/* ][ */
      case THING_SHORTLAMP:	/* ][ */
      case THING_CANDLE:
      case THING_CANDELABRA:
      case THING_TBLUETORCH:
      case THING_TGREENTORCH:
      case THING_TREDTORCH:
      case THING_SBLUETORCH:
      case THING_SGREENTORCH:
      case THING_SREDTORCH:
      case THING_FLAMECAN:		/* ][ */
      case THING_DEADPLAYER:
      case THING_DEADHUMAN:
      case THING_DEADSERGEANT:
      case THING_DEADIMP:
      case THING_DEADDEMON:
      case THING_DEADCACODEMON:
      case THING_DEADLOSTSOUL:
      case THING_BONES:
      case THING_BONES2:
      case THING_POOLOFBLOOD:
      case THING_POOLOFBLOOD2:	/* ][ */
      case THING_POOLOFBLOOD3:	/* ][ */
      case THING_POOLOFBRAINS:	/* ][ */
      case THING_SKULLTOPPOLE:
      case THING_HEADSKEWER:
      case THING_PILEOFSKULLS:
      case THING_IMPALEDBODY:
      case THING_IMPALEDBODY2:
      case THING_SKULLSINFLAMES:
      case THING_HANGINGSWAYING:
      case THING_HANGINGSWAYING2:
      case THING_HANGINGARMSOUT:
      case THING_HANGINGARMSOUT2:
      case THING_HANGINGONELEG:
      case THING_HANGINGONELEG2:
      case THING_HANGINGTORSO:
      case THING_HANGINGTORSO2:
      case THING_HANGINGLEG:
      case THING_HANGINGLEG2:
      case THING_HANGINGNOGUTS:	/* ][ */
      case THING_HANGINGNOGUTS2:	/* ][ */
      case THING_HANGINGLOOKDN:	/* ][ */
      case THING_HANGINGLOOKUP:	/* ][ */
      case THING_HANGINGTORSO3:	/* ][ */
      case THING_HANGINGTORSO4:	/* ][ */
         return CLASS_DECOR;
      }
   else /* Heretic Things */
      switch( type)
      {
      /* Starting points */
      case THING_PLAYER1:
      case THING_PLAYER2:
      case THING_PLAYER3:
      case THING_PLAYER4:
      case THING_DEATHMATCH:
         return CLASS_START;
      case THING_TELEPORT:
         return CLASS_TELEPORT;

      /* Enhancements & spheres */
      case THING_BLUEKEY:
      case THING_GREENKEY:
      case THING_YELLOWKEY:
      case THING_BAGHOLDING:
      case THING_MYSTICURN:
      case THING_SILVSHIELD:
      case THING_ENCHSHIELD:
      case THING_MAPSCROLL:
      case THING_TOMEPOWER:
      case THING_TIMEBOMB:
      case THING_CHAOSDEV:
      case THING_SHADOWSPHR:
      case THING_RINGINVULN:
      case THING_TORCH:
      case THING_WINGS:
      case THING_MORPHOVUM:
         return CLASS_ENHANCE;

      /* Armor & health bonuses & small ammo */
      case THING_VIAL:
      case THING_FLASK:
      case THING_CRYSTAL:
      case THING_ARROWS:
      case THING_CLAWORB:
      case THING_RUNESSML:
      case THING_FLAMEORBSML:
      case THING_MACESPHSML:
         return CLASS_BONUS;

      /* Weapons & bulk ammo */
      case THING_GAUNTLETS:
      case THING_CROSSBOW:
      case THING_DRAGONCLAW:
      case THING_HELLSTAFF:
      case THING_PHOENIXROD:
      case THING_MACE:
      case THING_CRGEODE:
      case THING_ARQUIVER:
      case THING_ENERGYORB:
      case THING_RUNESLRG:
      case THING_FLAMEORBLRG:
      case THING_MACESPHLRG:
         return CLASS_WEAPON;

      /* Enemies */
      case THING_GARGOYLE:
      case THING_FRGARGOYLE:
      case THING_GOLEM:
      case THING_GOLEMGH:
      case THING_NTGOLEM:
      case THING_NTGOLEMGH:
      case THING_WARRIOR:
      case THING_WARRIORGH:
      case THING_DISCIPLE:
      case THING_SABRECLAW:
      case THING_WEREDRAGON:
      case THING_OPHIDIAN:
      case THING_IRONLICH:
      case THING_MAULOTAUR:
      case THING_DSPARIL:
         return CLASS_ENEMY;

      /* Specials */
      case THING_DSPARILSPOT:
      case THING_GLITTELEPORT:
      case THING_GLITTELEEXIT:
         return CLASS_TELEPORT;
      case THING_POD:
      case THING_PODGENERATOR:
      case THING_VOLCANO:
         return CLASS_BARREL;

      /* Decorations, et al */
      case THING_BROWNBARREL:
      case THING_TBROWNPILLAR:
      case THING_SGREYPILLAR:
      case THING_BLUESPHERE:
      case THING_GREENSPHERE:
      case THING_YELLOWSPHERE:
      case THING_WALLTORCH:
      case THING_SERPENTTORCH:
      case THING_CHANDELIER:
      case THING_FIREBRAZIER:
      case THING_MOSS1STR:
      case THING_MOSS3STR:
      case THING_STALACTSMALL:
      case THING_STALACTLARGE:
      case THING_STALAGMSMALL:
      case THING_STALAGMLARGE:
      case THING_HANGINGCORPSE:
      case THING_HANGINGSKULL1:
      case THING_HANGINGSKULL2:
      case THING_HANGINGSKULL3:
      case THING_HANGINGSKULL4:
      case THING_SNDBELSS:
      case THING_SNDDROPS:
      case THING_SNDFASTFOOT:
      case THING_SNDSLOWFOOT:
      case THING_SNDHEARTBEAT:
      case THING_SNDGROWL:
      case THING_SNDLAUGHTER:
      case THING_SNDMAGIC:
      case THING_SNDSCREAM:
      case THING_SNDSQUISH:
      case THING_SNDWIND:
      case THING_SNDWATERFALL:
         return CLASS_DECOR;
      }
printf("type = %d\n", type);
   return CLASS_UNKNOWN;
}


/*
   get the size of a thing
*/
BCINT GetThingRadius( BCINT type)
{
   if (GameVersion < 16) /* DOOM Things */
      switch( type)
      {
      case THING_SPIDERBOSS:
         return 128;
      case THING_ARACHNO:
         return 64;
      case THING_MANCUBUS:
      case THING_BOSSBRAIN:
      case THING_BOSSSHOOT:
         return 48;
      case THING_CYBERDEMON:
         return 40;
      case THING_BROWNTREE:
      case THING_SPAWNSPOT:
         return 32;
      case THING_CACODEMON:
      case THING_PAINELEM:
         return 31;
      case THING_DEMON:
      case THING_SPECTRE:
         return 30;
      case THING_KNIGHT:
      case THING_BARON:
         return 24;
      case THING_HUMAN:
      case THING_SERGEANT:
      case THING_COMMANDO:
      case THING_IMP:
      case THING_REVENANT:
      case THING_ARCHVILE:
      case THING_WOLF3DSS:
      case THING_BLUECARD:
      case THING_YELLOWCARD:
      case THING_REDCARD:
      case THING_BLUESKULLKEY:
      case THING_YELLOWSKULLKEY:
      case THING_REDSKULLKEY:
      case THING_ARMBONUS1:
      case THING_HLTBONUS1:
      case THING_GREENARMOR:
      case THING_BLUEARMOR:
      case THING_STIMPACK:
      case THING_MEDIKIT:
      case THING_SOULSPHERE:
      case THING_MEGASPHERE:
      case THING_RADSUIT:
      case THING_COMPMAP:
      case THING_BLURSPHERE:
      case THING_BERSERK:
      case THING_INVULN:
      case THING_LITEAMP:
      case THING_CHAINSAW:
      case THING_SHOTGUN:
      case THING_COMBATGUN:
      case THING_CHAINGUN:
      case THING_LAUNCHER:
      case THING_PLASMAGUN:
      case THING_BFG9000:
      case THING_AMMOCLIP:
      case THING_AMMOBOX:
      case THING_SHELLS:
      case THING_SHELLBOX:
      case THING_ROCKET:
      case THING_ROCKETBOX:
      case THING_ENERGYCELL:
      case THING_ENERGYPACK:
      case THING_BACKPACK:
         return 20;
      case THING_BARREL:
         return 10;
      default: /* includes PLAYERs & LOSTSOUL */
         return 16;
      }
   else /* Heretic Things */
      switch( type)
      {
      case THING_DSPARIL:
      case THING_MAULOTAUR:
         return 40;
      case THING_IRONLICH:
      case THING_WEREDRAGON:
         return 32;
      case THING_OPHIDIAN:
      case THING_SABRECLAW:
      case THING_GOLEM:
      case THING_GOLEMGH:
      case THING_NTGOLEM:
      case THING_NTGOLEMGH:
         return 24;
      case THING_WARRIOR:
      case THING_WARRIORGH:
      case THING_DISCIPLE:
      case THING_DSPARILSPOT:
      case THING_BLUEKEY:
      case THING_GREENKEY:
      case THING_YELLOWKEY:
      case THING_GAUNTLETS:
      case THING_CROSSBOW:
      case THING_DRAGONCLAW:
      case THING_HELLSTAFF:
      case THING_PHOENIXROD:
      case THING_MACE:
      case THING_PODGENERATOR:
         return 20;
      case THING_POD:
         return 10;
      default: /* includes PLAYERs & (FR)GARGOYLE */
         return 16;
      }
}

/* end of file */
