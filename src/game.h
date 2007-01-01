#ifndef __GAME_H
#define __GAME_H
/*-------------------------------------------------------------------------*\
  <game.h> -- Game header file

  Date      Programmer  Description
  11/17/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include <istdlib.h>
#include <file.h>
#include "factory.h"
#include "item.h"
#include "monster.h"
#include "spell.h"
#include "map.h"

class BTGame
{
 public:
  BTGame(const char *itmFile, const char *monFile, const char *splFile);
  ~BTGame();

  BTFactory<BTItem> &getItemList();
  BTFactory<BTMonster> &getMonsterList();
  BTFactory<BTSpell> &getSpellList();
  BTMap *getMap();
  BTMap *loadMap(const char *filename);

  static BTGame *getGame();

 private:
  BTFactory<BTItem> itemList;
  BTFactory<BTMonster> monsterList;
  BTFactory<BTSpell> spellList;
  BTMap *levelMap;

  static BTGame *game;
};

#endif

