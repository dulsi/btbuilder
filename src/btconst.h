#ifndef __BTCONST_H
#define __BTCONST_H
/*-------------------------------------------------------------------------*\
  <btconst.h> -- Bt Builder constants header file.

  Date      Programmer  Description
  11/20/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#define BT_SPELLTYPES 32
#define BTSPELLTYPE_SCRYSIGHT      21
#define BTSPELLTYPE_LIGHT          22
#define BTSPELLTYPE_ARMORBONUS     23
#define BTSPELLTYPE_HITBONUS       24
#define BTSPELLTYPE_TRAPDESTROY    25
#define BTSPELLTYPE_DOORDETECT     26
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

#define BT_COMBATACTIONS 13
extern char *combatActions[];

#define BT_ITEMTYPES 13
extern char *itemTypes[];

#define BTTIMESUSABLE_UNLIMITED 255

#define BT_CONDITIONALCOMMANDS 18
#define BTCONDITION_GROUPFACING 6
#define BTCONDITION_JOBINPARTY  8
#define BTCONDITION_RANDOM      15
#define BTCONDITION_RACEINPARTY 16
extern char *conditionalCommands[];

#define BT_SPECIALCOMMANDS 61
#define BTSPECIALCOMMAND_STOP             1
#define BTSPECIALCOMMAND_SPIN             2
#define BTSPECIALCOMMAND_PRINT            13
#define BTSPECIALCOMMAND_REGENERATESPELLS 14
#define BTSPECIALCOMMAND_TAKESPELLS       15
#define BTSPECIALCOMMAND_HEALHITPOINTS    16
#define BTSPECIALCOMMAND_DAMAGEHITPOINTS  17
#define BTSPECIALCOMMAND_REGENERATESONGS  18
#define BTSPECIALCOMMAND_TAKESONGS        19
#define BTSPECIALCOMMAND_BACKONE          20
#define BTSPECIALCOMMAND_FORWARDONE       21
#define BTSPECIALCOMMAND_GUILD            23
#define BTSPECIALCOMMAND_REVIEW           24
#define BTSPECIALCOMMAND_SHOP             25
#define BTSPECIALCOMMAND_TRAP             26
#define BTSPECIALCOMMAND_DRAWPICTURE      27
#define BTSPECIALCOMMAND_PRINTLABEL       30
#define BTSPECIALCOMMAND_PRESSANYKEY      45
#define BTSPECIALCOMMAND_SETDIRECTION     48
extern char *specialCommands[];

#define BT_DIRECTIONS 4
#define BTDIRECTION_NORTH 0
#define BTDIRECTION_EAST  1
#define BTDIRECTION_SOUTH 2
#define BTDIRECTION_WEST  3
extern char *directions[];

#define BT_MAPTYPES 4
extern char *mapTypes[];

#define BT_STATS 5
#define BTSTAT_CN 3
extern char *statAbbrev[];

#define BT_ITEMS 8
#define BT_PARTYSIZE 7

#define BTITEM_NONE 0x4000

#endif
