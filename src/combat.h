#ifndef __COMBAT_H
#define __COMBAT_H
/*-------------------------------------------------------------------------*\
  <combat.h> -- Combat header file

  Date      Programmer  Description
  05/02/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include <istdlib.h>
#include <file.h>
#include "display.h"
#include "map.h"
#include <map>

class BTMonsterInstance
{
 public:
  BTMonsterInstance() : maxHp(-1), hp(-1), active(true) {}
  BTMonsterInstance(int startHp) : maxHp(startHp), hp(startHp), active(true) {}

  int maxHp, hp;
  BitField status;
  bool active;
};

class BTMonsterGroup
{
 public:
  int monsterType;
  int distance;
  std::vector<BTMonsterInstance> individual;
};

class BTCombat
{
 public:
  enum groupAction { runAway, fight, advance };

  BTCombat() : won(false) {}

  void addEncounter(int monsterType, int number = 0);
  void clearEncounters();
  void run(BTDisplay &d, bool partyAttack = false);

 private:
  groupAction fightOrRun(BTDisplay &d);
  bool endRound();
  bool selectAction(BTDisplay &d, int pcNum);
  int selectTarget(bool enemies, bool friends);

 private:
  bool won;
  int round;
  std::list<BTMonsterGroup> monsters;
};

#endif
