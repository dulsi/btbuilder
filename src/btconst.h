#ifndef __BTCONST_H
#define __BTCONST_H
/*-------------------------------------------------------------------------*\
  <btconst.h> -- Bt Builder constants header file.

  Date      Programmer  Description
  11/20/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#define BT_SPELLTYPES 32
#define BTSPELLTYPE_ARMORBONUS 23
#define BTSPELLTYPE_HITBONUS   24
#define BTSPELLTYPE_REGENBARD  29
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

#define BT_itemTypes 13
extern char *itemTypes[];

#endif

