#ifndef __GAME_H
#define __GAME_H
/*-------------------------------------------------------------------------*\
  <game.h> -- Game header file

  Date      Programmer  Description
  11/17/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include <file.h>
#include "combat.h"
#include "display.h"
#include "factory.h"
#include "item.h"
#include "job.h"
#include "module.h"
#include "monster.h"
#include "race.h"
#include "spell.h"
#include "spelleffect.h"
#include "status.h"
#include "map.h"
#include "pc.h"
#include "xpchart.h"

class BTPartyDead
{
 public:
  BTPartyDead() {}
};

class BTGame : public Psuedo3DMap
{
 public:
  BTGame(BTModule *m);
  ~BTGame();

  BTFactory<BTItem> &getItemList();
  BTJobList &getJobList();
  BTFactory<BTMonster> &getMonsterList();
  BTRaceList &getRaceList();
  XMLVector<BTPc*> &getRoster();
  BTFactory<BTSpell> &getSpellList();
  BTXpChartList &getXpChartList();
  BTMap *getMap();
  BTMap *loadMap(const char *filename);
  BTParty &getParty();

  int getLight();
  int getFacing();
  int getX();
  int getY();
  int getWallType(int x, int y, int direction);
  void setFacing(int f);

  std::string getLastInput() const;
  void setLastInput(std::string input);

  int getCounter() const;
  void setCounter(int val);

  BTCombat &getCombat();
  BTStatus &getStatus();

  bool getLocalFlag(int index);
  bool getGlobalFlag(int index);
  void setLocalFlag(int index, bool value);
  void setGlobalFlag(int index, bool value);

  void run(BTDisplay &d);
  bool runSpecial(BTDisplay &d, IShort special);
  bool move(BTDisplay &d, int dir);
  void turnLeft(BTDisplay &d);
  void turnRight(BTDisplay &d);
  void turnAround(BTDisplay &d);

  void setTimedSpecial(IShort special, unsigned int expire);
  void clearTimedSpecial();

  void addEffect(int spell, unsigned int expire, int group, int target, BitField &resists);
  void clearEffects(BTDisplay &d);
  void movedPlayer(BTDisplay &d, int who, int where);
  unsigned int getExpiration(unsigned int duration);
  bool isExpired(unsigned int expiration);
  bool isDaytime();
  void nextTurn(BTDisplay &d, BTCombat *combat = NULL);

  int getDelay() const;

  static BTGame *getGame();

 private:
  BTModule *module;
  BTFactory<BTItem> itemList;
  BTJobList jobList;
  BTFactory<BTMonster> monsterList;
  BTRaceList raceList;
  XMLVector<BTPc*> roster;
  BTFactory<BTSpell> spellList;
  BTXpChartList xpChartList;
  BTMap *levelMap;
  int xPos, yPos, facing;
  BTParty party;
  std::list<BTSpellEffect> spellEffect;
  std::string lastInput;
  int counter;
  BTCombat combat;
  BTStatus status;
  BitField local;
  BitField global;
  unsigned int gameTime;
  unsigned int timedExpiration;
  IShort timedSpecial;
  int delay;

  static BTGame *game;
};

#endif

