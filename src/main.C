#include "btconst.h"
#include "mainscreen.h"
#include "file.h"
#include "monster.h"
#include "spell.h"
#include "item.h"
#include "map.h"
#include "factory.h"
#include "game.h"
#include "psuedo3d.h"
#include "job.h"
#include <getopt.h>
#include <SDL.h>
#include <iostream>
#include <physfs.h>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

char *monFile = NULL;
char *itmFile = NULL;
char *splFile = NULL;

IUByte cornerWalls[16] =
{
 /*0000*/ ' ', /*0001*/ ' ', /*0010*/ ' ', /*0011*/0xC0, /*0100*/ ' ',
 /*0101*/0xB3, /*0110*/0xDA, /*0111*/0xC3, /*1000*/ ' ', /*1001*/0xD9,
 /*1010*/0xC4, /*1011*/0xC1, /*1100*/0xBF, /*1101*/0xB4, /*1110*/0xC2,
 /*1111*/0xC5
};

IUByte upperWalls[4] =
{
 /*00*/ ' ', /*01*/0xC4, /*10*/0xD7, /*11*/0xCD
};

IUByte sideWalls[4] =
{
 /*00*/ ' ', /*01*/0xB3, /*10*/0xD8, /*11*/0xBA
};

IUByte cornerWallsUTF8[16][4] =
{
 {/*0000*/ ' ', 0x00, 0x00, 0x00},
 {/*0001*/ ' ', 0x00, 0x00, 0x00},
 {/*0010*/ ' ', 0x00, 0x00, 0x00},
 {/*0011*/0xE2, 0x94, 0x94, 0x00},
 {/*0100*/ ' ', 0x00, 0x00, 0x00},
 {/*0101*/0xE2, 0x94, 0x82, 0x00},
 {/*0110*/0xE2, 0x94, 0x8C, 0x00},
 {/*0111*/0xE2, 0x94, 0x9C, 0x00},
 {/*1000*/ ' ', 0x00, 0x00, 0x00},
 {/*1001*/0xE2, 0x94, 0x98, 0x00},
 {/*1010*/0xE2, 0x94, 0x80, 0x00},
 {/*1011*/0xE2, 0x94, 0xB4, 0x00},
 {/*1100*/0xE2, 0x94, 0x90, 0x00},
 {/*1101*/0xE2, 0x94, 0xA4, 0x00},
 {/*1110*/0xE2, 0x94, 0xAC, 0x00},
 {/*1111*/0xE2, 0x94, 0xBC, 0x00}
};

IUByte upperWallsUTF8[4][4] =
{
 {/*00*/ ' ', 0x00, 0x00, 0x00},
 {/*01*/0xE2, 0x94, 0x80, 0x00},
 {/*10*/0xE2, 0x95, 0xAB, 0x00},
 {/*11*/0xE2, 0x95, 0x90, 0x00}
};

IUByte sideWallsUTF8[4][4] =
{
 {/*00*/ ' ', 0x00, 0x00, 0x00},
 {/*01*/0xE2, 0x94, 0x82, 0x00},
 {/*10*/0xE2, 0x95, 0xAA, 0x00},
 {/*11*/0xE2, 0x95, 0x91, 0x00}
};

#define MODE_STANDARD 1
#define MODE_ITEM     2
#define MODE_MONSTER  3
#define MODE_SPELL    4
#define MODE_MAP      5
#define MODE_EDITMAP  6

int main(int argc, char *argv[])
{
 char opt;
 static struct option long_options[] =
 {
  {"item", 0, 0, 'i'},
  {"monster", 0, 0, 'm'},
  {"spell", 0, 0, 's'},
  {"map", 1, 0, 'p'},
  {"editmap", 1, 0, 'e'},
  {"ascii", 0, 0, 'a'},
  {"xml", 1, 0, 'x'},
  {"multiplier", 1, 0, 'u'},
  {0, 0, 0, 0}
 };

 bool utf8 = true;
 int mode = MODE_STANDARD;
 char *mapFile = NULL;
 char *xmlFile = NULL;
 int multiplier = 0;
 std::string libDir(TOSTRING(BTBUILDERDIR));
 while ((opt = getopt_long(argc,argv,"imsap:x:e:l:u:", long_options, NULL)) != EOF)
 {
  switch (opt)
  {
   case 'i':
    mode = MODE_ITEM;
    break;
   case 'm':
    mode = MODE_MONSTER;
    break;
   case 's':
    mode = MODE_SPELL;
    break;
   case 'p':
    mode = MODE_MAP;
    if (optarg)
    {
     mapFile = strdup(optarg);
    }
    break;
   case 'e':
    mode = MODE_EDITMAP;
    if (optarg)
    {
     mapFile = strdup(optarg);
    }
    break;
   case 'l':
    if (optarg)
    {
     libDir = optarg;
    }
    break;
   case 'a':
    utf8 = false;
    break;
   case 'x':
    if (optarg)
    {
     xmlFile = strdup(optarg);
    }
    break;
   case 'u':
    if (optarg)
    {
     multiplier = atol(optarg);
    }
    break;
   default:
    break;
  }
 }

 BTMainScreen mainScreen(argv[0], libDir, multiplier);
 if (optind >= argc)
 {
  if (mode != MODE_STANDARD)
   return 0;
  mainScreen.run();
  return 0;
 }
 else if (mode == MODE_STANDARD)
 {
  std::string moduleFile("module/");
  moduleFile += argv[optind];
  moduleFile += ".xml";
  mainScreen.runModule(moduleFile);
  return 0;
 }
 else if (mode == MODE_EDITMAP)
 {
  std::string moduleFile("module/");
  moduleFile += argv[optind];
  moduleFile += ".xml";
  mainScreen.editModule(moduleFile, mapFile);
  return 0;
 }
 std::string moduleFile("module/");
 moduleFile += argv[optind];
 moduleFile += ".xml";
 BTModule module;
 mainScreen.loadModule(moduleFile, module);
 BTGame game(&module);
 BTFactory<BTMonster> &monList(game.getMonsterList());
 BTFactory<BTItem> &itmList(game.getItemList());
 BTFactory<BTSpell, BTSpell1> &splList(game.getSpellList());
 int i;
 if (mode == MODE_MONSTER)
 {
  if (xmlFile)
  {
   monList.save(xmlFile);
  }
  else
  {
   for (i = 0; i < monList.size(); i++)
   {
    BTMonster &mon(monList[i]);
    printf("Name: %s\n", mon.getName().c_str());
    printf("Illusion: %s   Base armor class: %d\n",
      (mon.isIllusion() ? "Yes" : "No"), (mon.getAc() - 10) * -1);
    printf("Level: %d   Thaumaturgical resistance: %d\n", mon.getLevel(),
      mon.getMagicResistance());
    printf("Starting distance (*10'): %d   Moves per round: %d\n",
      mon.getStartDistance(), mon.getMove());
    printf("Hit points: %dd%d   Combat options: ", mon.getHp().getNumber(),
      mon.getHp().getType());
    IShort combatAction = mon.getCombatAction(0);
    for (int a = 1; a < 4; ++a)
     if (combatAction != mon.getCombatAction(a))
      combatAction = -1;
    if (-1 == combatAction)
     printf("Multiple\n");
    else
     printf("%s\n", combatActions[combatAction]);
    printf("Rate of attacks: %d   Damage dice: %dd%d\n",
      mon.getRateAttacks(), mon.getMeleeDamage().getNumber(),
      mon.getMeleeDamage().getType());
    printf("Extra damage: %s\n", extraDamage[mon.getMeleeExtra()]);
    printf("Attack message: <monster> %s <opponent>\n", mon.getMeleeMessage());
    printf("Extra ranged attack information -\n");
    printf("   Type: %s", rangedTypes[mon.getRangedType()]);
    switch (mon.getRangedType())
    {
     case BTRANGEDTYPE_MAGIC:
      printf(": %s", splList[mon.getRangedSpell()].getCode());
     case BTRANGEDTYPE_FOE:
     case BTRANGEDTYPE_GROUP:
      printf("   Message: <monster> %s <opponent>\n", mon.getRangedMessage());
      printf("   Damage: %dd%d   Extra damage: %s   Range: %d",
        mon.getRangedDamage().getNumber(),  mon.getRangedDamage().getType(),
        extraDamage[mon.getRangedExtra()], mon.getRange());
      break;
     default:
      break;
    }
    printf("\n");
    printf("Upper limit appearing: %d  Gold: %dd%d   Picture number: %d\n",
      mon.getMaxAppearing(), mon.getGold().getNumber(),
      mon.getGold().getType(), mon.getPicture());
    printf("\n");
   }
  }
 }
 else if (mode == MODE_SPELL)
 {
  if (xmlFile)
  {
   splList.save(xmlFile);
  }
  else
  {
   BTSpellListCompare compare;
   BTSortedFactory<BTSpell, BTSpell1> sortedSplList(&splList, &compare);
   IShort caster = -1;
   IShort level = -1;
   for (i = 0; i < sortedSplList.size(); i++)
   {
    BTSpell &mon(sortedSplList[i]);
    if ((caster != mon.getCaster()) || (level != mon.getLevel()))
    {
     if (caster != mon.getCaster())
     {
      caster = mon.getCaster();
      BTJobList& jobList = BTGame::getGame()->getJobList();
      int j;
      for (j = 0; j < jobList.size(); ++j)
      {
       if (jobList[j]->getSkill(caster) != NULL)
        break;
      }
      printf("Class:  %s\n", ((j < jobList.size()) ? BTGame::getGame()->getJobList()[j]->name : "Unknown"));
     }
     level = mon.getLevel();
     printf("Level: %d\n\n", level);
    }
    printf("Name: %s\n", mon.getName().c_str());
    printf("Code: %s\n", mon.getCode());
    printf("Points: %d   Range: %d   Extra range: %s\n", mon.getSp(),
      mon.getRange() * 10, effectiveRanges[mon.getEffectiveRange()]);
    printf("Target: %s\n", areaEffect[mon.getArea()]);
    printf("Effect: %s <target>\n", mon.getEffect());
    printf("Duration: %s\n", durations[mon.getDuration()]);
    printf("%s", mon.describeManifest().data());
/*    printf("Type: %s", spellTypes[mon.getType()]);
    switch (mon.getType())
    {
     case BTSPELLTYPE_SUMMONILLUSION:
     case BTSPELLTYPE_SUMMONMONSTER:
      printf("   Name: %s", monList[mon.getExtra()].getName().c_str());
      break;
     case BTSPELLTYPE_ARMORBONUS:
     case BTSPELLTYPE_HITBONUS:
      printf("   Bonus: %d", mon.getExtra());
      break;
     case BTSPELLTYPE_PUSH:
      printf("   Distance: %d", mon.getExtra());
      break;
     case BTSPELLTYPE_REGENBARD:
      printf("   Amount: %d", mon.getExtra());
      break;
     default:
      break;
    }
    printf("\nDice: %dd%d   Duration: %s\n", mon.getDice().getNumber(),
      mon.getDice().getType(), durations[mon.getDuration()]);
*/    printf("\n");
   }
  }
 }
 else if (mode == MODE_ITEM)
 {
  if (xmlFile)
  {
   itmList.save(xmlFile);
  }
  else
  {
   for (i = 0; i < itmList.size(); i++)
   {
    BTItem &mon(itmList[i]);
    printf("Name: %s\n", mon.getName().c_str());
    printf("Type: %s\n", itemTypes[mon.getType()]);
    printf("Price: %d   User class: Multiple\n", mon.getPrice());
    printf("Armor bonus: %d   Hit bonus: %d\n", mon.getArmorPlus(),
      mon.getHitPlus());
    printf("Damage dice: %dd%d\n", mon.getDamage().getNumber(),
      mon.getDamage().getType());
    printf("Extra special damage: %s   Special damage likelihood: %d%%\n",
      extraDamage[mon.getXSpecial()], mon.getChanceXSpecial());
    if (BTTIMESUSABLE_UNLIMITED == mon.getTimesUsable())
     printf("Times useable: (unlimited)");
    else
     printf("Times useable: %d", mon.getTimesUsable());
    printf("   Spell cast: %s\n",
      ((mon.getSpellCast() == BTITEMCAST_NONE) ? "(none)" :
      splList[mon.getSpellCast()].getName().c_str()));
    printf("Cause: <member> %s <opponent>\n", mon.getCause());
    printf("Effect: %s <damage>\n", mon.getEffect());
    printf("\n");
   }
  }
 }
 else if (mode == MODE_MAP)
 {
  int x, y;
  IShort index;
  if (mapFile)
   game.loadMap(mapFile);
  BTMap &gameMap = *game.getMap();
  if (xmlFile)
  {
   XMLSerializer parser;
   parser.add("map", &gameMap);
   parser.write(xmlFile, true);
  }
  else
  {
   printf("Name: %s\n", gameMap.getName());
   printf("Type: %s   Level: %d\n", game.getPsuedo3DConfigList().getName(gameMap.getType()).c_str(), gameMap.getLevel());
   printf("Monster difficulty: %d   Chance of encounter: %d%%\n", gameMap.getMonsterLevel(), gameMap.getMonsterChance());
   printf("File: %s\n\n", gameMap.getFilename());
   for (y = 0; y < gameMap.getYSize(); y++)
   {
    printf(" ");
    for (x = 0; x < gameMap.getXSize(); x++)
    {
     index = 0;
     if ((y > 0) && (gameMap.getSquare(y - 1, x).getWall(BTDIRECTION_WEST) > 0))
     {
      index += 1 << BTDIRECTION_NORTH;
     }
     if ((x > 0) && (gameMap.getSquare(y, x - 1).getWall(BTDIRECTION_NORTH) > 0))
     {
      index += 1 << BTDIRECTION_WEST;
     }
     if (gameMap.getSquare(y, x).getWall(BTDIRECTION_NORTH) > 0)
     {
      index += 1 << BTDIRECTION_EAST;
     }
     if (gameMap.getSquare(y, x).getWall(BTDIRECTION_WEST) > 0)
     {
      index += 1 << BTDIRECTION_SOUTH;
     }
     if (utf8)
      printf("%s%s", cornerWallsUTF8[index], upperWallsUTF8[gameMap.getSquare(y, x).getWall(BTDIRECTION_NORTH)]);
     else
      printf("%c%c", cornerWalls[index], upperWalls[gameMap.getSquare(y, x).getWall(BTDIRECTION_NORTH)]);
    }
    index = 0;
    if ((y > 0) && (gameMap.getSquare(y - 1, gameMap.getXSize() - 1).getWall(BTDIRECTION_EAST) > 0))
    {
     index += 1 << BTDIRECTION_NORTH;
    }
    if (gameMap.getSquare(y, gameMap.getXSize() - 1).getWall(BTDIRECTION_NORTH) > 0)
    {
     index += 1 << BTDIRECTION_WEST;
    }
    if (gameMap.getSquare(y, gameMap.getXSize() - 1).getWall(BTDIRECTION_EAST) > 0)
    {
     index += 1 << BTDIRECTION_SOUTH;
    }
    if (utf8)
     printf("%s \n", cornerWallsUTF8[index]);
    else
     printf("%c \n", cornerWalls[index]);
    printf(" ");
    for (x = 0; x < gameMap.getXSize(); x++)
    {
     index = gameMap.getSquare(y, x).getSpecial();
     if (utf8)
      printf("%s%c", sideWallsUTF8[gameMap.getSquare(y, x).getWall(BTDIRECTION_WEST)],
        (index < 0 ) ? ' ' : ((index < 26) ? 'A' + index : 'a' - 26 + index));
     else
      printf("%c%c", sideWalls[gameMap.getSquare(y, x).getWall(BTDIRECTION_WEST)],
        (index < 0 ) ? ' ' : ((index < 26) ? 'A' + index : 'a' - 26 + index));
    }
    if (utf8)
     printf("%s \n", sideWallsUTF8[gameMap.getSquare(y, gameMap.getXSize() - 1).getWall(BTDIRECTION_EAST)]);
    else
     printf("%c \n", sideWalls[gameMap.getSquare(y, gameMap.getXSize() - 1).getWall(BTDIRECTION_EAST)]);
   }
   printf(" ");
   for (x = 0; x < gameMap.getXSize(); x++)
   {
    index = 0;
    if (gameMap.getSquare(gameMap.getYSize() - 1, x).getWall(BTDIRECTION_WEST) > 0)
    {
     index += 1 << BTDIRECTION_NORTH;
    }
    if ((x > 0) && (gameMap.getSquare(gameMap.getYSize() - 1, x - 1).getWall(BTDIRECTION_SOUTH) > 0))
    {
     index += 1 << BTDIRECTION_WEST;
    }
    if (gameMap.getSquare(gameMap.getYSize() - 1, x).getWall(BTDIRECTION_SOUTH) > 0)
    {
     index += 1 << BTDIRECTION_EAST;
    }
    if (utf8)
     printf("%s%s", cornerWallsUTF8[index], upperWallsUTF8[gameMap.getSquare(gameMap.getYSize() - 1, x).getWall(BTDIRECTION_SOUTH)]);
    else
     printf("%c%c", cornerWalls[index], upperWalls[gameMap.getSquare(gameMap.getYSize() - 1, x).getWall(BTDIRECTION_SOUTH)]);
   }
   index = 0;
   if (gameMap.getSquare(gameMap.getYSize() - 1, gameMap.getXSize() - 1).getWall(BTDIRECTION_EAST) > 0)
   {
    index += 1 << BTDIRECTION_NORTH;
   }
   if (gameMap.getSquare(gameMap.getYSize() - 1, gameMap.getXSize() - 1).getWall(BTDIRECTION_SOUTH) > 0)
   {
    index += 1 << BTDIRECTION_WEST;
   }
   if (utf8)
    printf("%s \n", cornerWallsUTF8[index]);
   else
    printf("%c \n", cornerWalls[index]);
   printf("\nDefined special squares:\n\n\n");
   for (int i = 0; i < 30; i++)
   {
    const BTSpecial *sp = gameMap.getSpecial(i);
    if (NULL != sp)
    {
     printf("%c. ", (i < 26) ? 'A' + i : 'a' - 26 + i);
     sp->print(stdout);
     printf("\n");
    }
   }
  }
 }
 PHYSFS_deinit();
 return 0;
}

