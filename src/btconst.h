#ifndef __BTCONST_H
#define __BTCONST_H
/*-------------------------------------------------------------------------*\
  <btconst.h> -- Bt Builder constants header file.

  Date      Programmer  Description
  11/20/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#define BT_SPELLTYPES 32
#define BTSPELLTYPE_ARMORBONUS     23
#define BTSPELLTYPE_HITBONUS       24
#define BTSPELLTYPE_SUMMONILLUSION 19
#define BTSPELLTYPE_SUMMONMONSTER  18
#define BTSPELLTYPE_REGENBARD      29
extern char *spellTypes[];

#define BT_EFFECTIVERANGES 2
extern char *effectiveRanges[];

#define BT_AREAEFFECT 4
extern char *areaEffect[];

#define BT_DURATION 6
extern char *durations[];

#define BT_MONSTEREXTRADAMAGE 11
extern char *extraDamage[];

#define BT_RANGEDTYPES 4
#define BTRANGEDTYPE_FOE   1
#define BTRANGEDTYPE_GROUP 2
#define BTRANGEDTYPE_MAGIC 3
extern char *rangedTypes[];

#define BT_ITEMTYPES 13
extern char *itemTypes[];

#define BT_CLASSTYPES 13
extern char *classTypes[];

#define BT_RACETYPES 7
extern char *raceTypes[];

#define BT_CONDITIONALCOMMANDS 18
extern char *conditionalCommands[];

#define BT_SPECIALCOMMANDS 61
extern char *specialCommands[];

#define BT_DIRECTIONS 4
#define BTDIRECTION_NORTH 0
#define BTDIRECTION_EAST  1
#define BTDIRECTION_SOUTH 2
#define BTDIRECTION_WEST  3
extern char *directions[];

#define BT_MAPTYPES 4
extern char *mapTypes[];

#endif
