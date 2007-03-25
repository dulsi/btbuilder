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
#include "module.h"
#include "monster.h"
#include "race.h"
#include "spell.h"
#include "map.h"
#include "pc.h"

class BTGame : public Psuedo3DMap
{
 public:
  BTGame(BTModule *m);
  ~BTGame();

  BTFactory<BTItem> &getItemList();
  XMLVector<BTJob*> &getJobList();
  BTFactory<BTMonster> &getMonsterList();
  BTRaceList &getRaceList();
  XMLVector<BTPc*> &getRoster();
  BTFactory<BTSpell> &getSpellList();
  BTMap *getMap();
  BTMap *loadMap(const char *filename);
  XMLVector<BTPc*> &getParty();

  int getFacing();
  int getX();
  int getY();
  int getWallType(int x, int y, int direction);
  void setFacing(int f);

  std::string getLastInput(void) const;
  void setLastInput(std::string input);

  void run(BTDisplay &d);
  bool move(BTDisplay &d, int dir);
  void turnLeft(BTDisplay &d);
  void turnRight(BTDisplay &d);
  void turnAround(BTDisplay &d);

  static BTGame *getGame();

 private:
  BTModule *module;
  BTFactory<BTItem> itemList;
  XMLVector<BTJob*> jobList;
  BTFactory<BTMonster> monsterList;
  BTRaceList raceList;
  XMLVector<BTPc*> roster;
  BTFactory<BTSpell> spellList;
  BTMap *levelMap;
  int xPos, yPos, facing;
  XMLVector<BTPc*> party;
  std::string lastInput;

  static BTGame *game;
};

#endif

