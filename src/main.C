#include "btconst.h"
#include "file.h"
#include "monster.h"
#include "spell.h"
#include "item.h"
#include "map.h"
#include "factory.h"
#include "game.h"
#include "psuedo3d.h"
#include "ikbbuffer.h"
#include <getopt.h>
#include <SDL.h>
#include <iostream>

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

class BTMapTest : public Psuedo3DMap
{
 public:
  int getWallType(int x, int y, int direction)
  {
   if (x < 0)
    x += 22;
   x = x % 22;
   if (y < 0)
    y += 22;
   y = y % 22;
   IShort w = m->getSquare(y, x).getWall(direction);
   if (w == 2)
    return 2;
   else if (w)
    return 1;
   else
    return 0;
  }

  BTMap *m;
};

void testDisplay(BTMap *map)
{
 if (SDL_Init(SDL_INIT_VIDEO) < 0)
 {
  printf("Failed - SDL_Init\n");
  exit(0);
 }
 SDL_Surface *ISDLScreen = SDL_SetVideoMode(320, 200, 32,
   SDL_SWSURFACE /*| (fullScreen ? SDL_FULLSCREEN : 0)*/);
 if (ISDLScreen == NULL)
 {
  printf("Failed - SDL_SetVideoMode\n");
  exit(0);
 }
 IKeybufferStart();
 std::vector<Psuedo3DConfig*> aryConfig;
 Psuedo3DConfig::readXML("walls.xml", aryConfig);
 Psuedo3D p;
 p.setConfig(aryConfig[0]);
 BTMapTest m;
 m.m = map;
 int x = 0, y = 0, direction = 0;
 unsigned char key = ' ';
 while ('q' != key)
 {
  p.draw(&m, x, y, direction);
  SDL_Rect src;
  src.x = 0;
  src.y = 0;
  src.w = 112;
  src.h = 88;
  SDL_BlitSurface(p.getDisplay(), &src, ISDLScreen, &src);
  SDL_UpdateRect(ISDLScreen, 0, 0, 0, 0);
  key = IKeybufferGet();
  switch (key)
  {
   case 0xBD: // up
    x += Psuedo3D::changeXY[direction][0] + 22;
    x = x % 22;
    y += Psuedo3D::changeXY[direction][1] + 22;
    y = y % 22;
    break;
   case 0xBF: // left
    direction += 3;
    direction = direction % 4;
    break;
   case 0xC3: // down
    x += Psuedo3D::changeXY[(direction + 2) % 4][0] + 22;
    x = x % 22;
    y += Psuedo3D::changeXY[(direction + 2) % 4][1] + 22;
    y = y % 22;
    break;
   case 0xC1: // right
    direction += 1;
    direction = direction % 4;
    break;
  }
 }
}

int main(int argc, char *argv[])
{
 char opt;
 static struct option long_options[] =
 {
  {"monster", 1, 0, 'm'},
  {"item", 1, 0, 'i'},
  {"spell", 1, 0, 's'},
  {"ascii", 0, 0, 'a'},
  {0, 0, 0, 0}
 };

 bool utf8 = true;
 monFile = strdup("default.mon");
 itmFile = strdup("default.itm");
 splFile = strdup("default.spl");
 while ((opt = getopt_long(argc,argv,"m:i:s:a", long_options, NULL)) != EOF)
 {
  switch (opt)
  {
   case 'm':
    if (optarg)
    {
     free(monFile);
     monFile = strdup(optarg);
    }
    break;
   case 'i':
    if (optarg)
    {
     free(itmFile);
     itmFile = strdup(optarg);
    }
    break;
   case 's':
    if (optarg)
    {
     free(splFile);
     splFile = strdup(optarg);
    }
    break;
   case 'a':
    utf8 = false;
    break;
   default:
    break;
  }
 }

 BTGame game(itmFile, monFile, splFile);
 BTFactory<BTMonster> &monList(game.getMonsterList());
 BTFactory<BTItem> &itmList(game.getItemList());
 BTFactory<BTSpell> &splList(game.getSpellList());
 int i;
 for (int argx = optind; argx < argc;  argx++)
 {
  if (strcmp(argv[argx], "monster") == 0)
  {
   for (i = 0; i < monList.size(); i++)
   {
    BTMonster &mon(monList[i]);
    printf("Name: %s\n", mon.getName());
    printf("Illusion: %s   Base armor class: %d\n",
      (mon.isIllusion() ? "Yes" : "No"), mon.getAc());
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
  else if (strcmp(argv[argx], "spell") == 0)
  {
   BTSpellListCompare compare;
   BTSortedFactory<BTSpell> sortedSplList(&splList, &compare);
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
      printf("Class:  %s\n", classTypes[caster]);
     }
     level = mon.getLevel();
     printf("Level: %d\n\n", level);
    }
    printf("Name: %s\n", mon.getName());
    printf("Code: %s\n", mon.getCode());
    printf("Points: %d   Range: %d   Extra range: %s\n", mon.getSp(),
      mon.getRange() * 10, effectiveRanges[mon.getEffectiveRange()]);
    printf("Type: %s", spellTypes[mon.getType()]);
    switch (mon.getType())
    {
     case BTSPELLTYPE_SUMMONILLUSION:
     case BTSPELLTYPE_SUMMONMONSTER:
      printf("   Name: %s", monList[mon.getExtra()].getName());
      break;
     case BTSPELLTYPE_ARMORBONUS:
     case BTSPELLTYPE_HITBONUS:
      printf("   Bonus: %d", mon.getExtra());
      break;
     case BTSPELLTYPE_REGENBARD:
      printf("   Amount: %d", mon.getExtra());
      break;
     default:
      break;
    }
    printf("\nTarget: %s\n", areaEffect[mon.getArea()]);
    printf("Dice: %dd%d   Duration: %s\n", mon.getDice().getNumber(),
      mon.getDice().getType(), durations[mon.getDuration()]);
    switch (mon.getType())
    {
     case BTSPELLTYPE_SCRYSIGHT:
     case BTSPELLTYPE_DOORDETECT:
     case BTSPELLTYPE_SUMMONILLUSION:
     case BTSPELLTYPE_SUMMONMONSTER:
     case BTSPELLTYPE_LIGHT:
     case BTSPELLTYPE_TRAPDESTROY:
//     case BTSPELLTYPE_HITBONUS:
      printf("Effect: %s\n", mon.getEffect());
      break;
     default:
      printf("Effect: %s <target>\n", mon.getEffect());
      break;
    }
    printf("\n");
   }
  }
  else if (strcmp(argv[argx], "item") == 0)
  {
   for (i = 0; i < itmList.size(); i++)
   {
    BTItem &mon(itmList[i]);
    printf("Name: %s\n", mon.getName());
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
      ((mon.getSpellCast() == -1) ? "(none)" :
      splList[mon.getSpellCast()].getName()));
    printf("Cause: <member> %s <opponent>\n", mon.getCause());
    printf("Effect: %s <damage>\n", mon.getEffect());
    printf("\n");
   }
  }
  else if (strcmp(argv[argx], "map") == 0)
  {
   argx++;
   if (argx < argc)
   {
    int x, y;
    IShort index;
    BTMap &gameMap = *game.loadMap(argv[argx]);
    printf("Name: %s\n", gameMap.getName());
    printf("Type: %s   Level: %d\n", mapTypes[gameMap.getType()], gameMap.getLevel());
    printf("Monster difficulty: %d   Chance of encounter: %d%%\n", gameMap.getMonsterLevel(), gameMap.getMonsterChance());
    printf("File: %s.map\n\n", gameMap.getFilename());
    for (y = 0; y < 22; y++)
    {
     printf(" ");
     for (x = 0; x < 22; x++)
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
     if ((y > 0) && (gameMap.getSquare(y - 1, 21).getWall(BTDIRECTION_EAST) > 0))
     {
      index += 1 << BTDIRECTION_NORTH;
     }
     if (gameMap.getSquare(y, 21).getWall(BTDIRECTION_NORTH) > 0)
     {
      index += 1 << BTDIRECTION_WEST;
     }
     if (gameMap.getSquare(y, 21).getWall(BTDIRECTION_EAST) > 0)
     {
      index += 1 << BTDIRECTION_SOUTH;
     }
     if (utf8)
      printf("%s \n", cornerWallsUTF8[index]);
     else
      printf("%c \n", cornerWalls[index]);
     printf(" ");
     for (x = 0; x < 22; x++)
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
      printf("%s \n", sideWallsUTF8[gameMap.getSquare(y, 21).getWall(BTDIRECTION_EAST)]);
     else
      printf("%c \n", sideWalls[gameMap.getSquare(y, 21).getWall(BTDIRECTION_EAST)]);
    }
    printf(" ");
    for (x = 0; x < 22; x++)
    {
     index = 0;
     if (gameMap.getSquare(21, x).getWall(BTDIRECTION_WEST) > 0)
     {
      index += 1 << BTDIRECTION_NORTH;
     }
     if ((x > 0) && (gameMap.getSquare(21, x - 1).getWall(BTDIRECTION_SOUTH) > 0))
     {
      index += 1 << BTDIRECTION_WEST;
     }
     if (gameMap.getSquare(21, x).getWall(BTDIRECTION_SOUTH) > 0)
     {
      index += 1 << BTDIRECTION_EAST;
     }
     if (utf8)
      printf("%s%s", cornerWallsUTF8[index], upperWallsUTF8[gameMap.getSquare(21, x).getWall(BTDIRECTION_SOUTH)]);
     else
      printf("%c%c", cornerWalls[index], upperWalls[gameMap.getSquare(21, x).getWall(BTDIRECTION_SOUTH)]);
    }
    index = 0;
    if (gameMap.getSquare(21, 21).getWall(BTDIRECTION_EAST) > 0)
    {
     index += 1 << BTDIRECTION_NORTH;
    }
    if (gameMap.getSquare(21, 21).getWall(BTDIRECTION_SOUTH) > 0)
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
  else if (strcmp(argv[argx], "test") == 0)
  {
   argx++;
   if (argx < argc)
   {
    BTMap *gameMap = game.loadMap(argv[argx]);
    testDisplay(gameMap);
   }
  }
 }
/* catch (FileException e)
 {
 }*/
 return 0;
}

