#ifndef __GAME_H
#define __GAME_H
/*-------------------------------------------------------------------------*\
  <game.h> -- Game header file

  Date      Programmer  Description
  11/17/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include <istdlib.h>
#include <file.h>
#include "display.h"
#include "factory.h"
#include "item.h"
#include "job.h"
#include "monster.h"
#include "race.h"
#include "spell.h"
#include "map.h"

class BTGame : public Psuedo3DMap
{
 public:
  BTGame(const char *itmFile, const char *monFile, const char *splFile);
  ~BTGame();

  BTFactory<BTItem> &getItemList();
  XMLVector<BTJob*> &getJobList();
  BTFactory<BTMonster> &getMonsterList();
  XMLVector<BTRace*> &getRaceList();
  BTFactory<BTSpell> &getSpellList();
  BTMap *getMap();
  BTMap *loadMap(const char *filename);

  int getFacing();
  int getX();
  int getY();
  int getWallType(int x, int y, int direction);

  void run(BTDisplay &d);
  void moveForward(BTDisplay &d);
  void turnLeft(BTDisplay &d);
  void turnRight(BTDisplay &d);
  void turnAround(BTDisplay &d);

  static BTGame *getGame();

 private:
  BTFactory<BTItem> itemList;
  XMLVector<BTJob*> jobList;
  BTFactory<BTMonster> monsterList;
  XMLVector<BTRace*> raceList;
  BTFactory<BTSpell> spellList;
  BTMap *levelMap;
  int xPos, yPos, facing;

  static BTGame *game;
};

#endif

