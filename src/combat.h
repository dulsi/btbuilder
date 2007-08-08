#ifndef __COMBAT_H
#define __COMBAT_H
/*-------------------------------------------------------------------------*\
  <combat.h> -- Combat header file

  Date      Programmer  Description
  05/02/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include <file.h>
#include "display.h"
#include "map.h"
#include "screenset.h"
#include <map>

class BTCombatError
{
 public:
  BTCombatError(const std::string &e) : error(e) {}

  std::string error;
};

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
  BTMonsterGroup() : monsterName(0), canMove(true) {}
  ~BTMonsterGroup();

  int findTarget(int ind = BTTARGET_INDIVIDUAL);
  void setMonsterType(int type, int number = 0);

  int monsterType;
  char *monsterName;
  int distance;
  int active;
  bool canMove;
  std::vector<BTMonsterInstance> individual;
};

class BTCombatScreen : public BTScreenSetScreen
{
 public:
  BTCombatScreen(int n, int escScr, int t) : BTScreenSetScreen(n, escScr, t) {}

  virtual void draw(BTDisplay &d, ObjectSerializer *obj);
  virtual int getEscapeScreen();

  static XMLObject *create(const XML_Char *name, const XML_Char **atts);
};

class BTCombat : public BTScreenSet
{
 public:
  BTCombat();
  ~BTCombat();

  void addEncounter(int monsterType, int number = 0);
  void clearEncounters();
  void endScreen(BTDisplay &d);
  int findScreen(int num);
  bool findTarget(BTPc &pc, int range, BTMonsterGroup *&grp, int &target);
  void initScreen(BTDisplay &d);
  bool isWinner() { return won; }
  virtual void open(const char *filename);
  void run(BTDisplay &d, bool partyAttack = false);
  void runCombat(BTDisplay &d);
  void runMonsterAction(BTDisplay &d, int &active, BTMonsterGroup &grp, BTMonsterInstance &mon);
  void runPcAction(BTDisplay &d, int &active, BTPc &pc);

  // Actions
  static int advance(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int attack(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int combatOption(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int defend(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int partyAttack(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int runAway(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int target(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int useItem(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);

 private:
  bool endRound();

 private:
  bool won;
  bool optionState;
  int round;
  std::list<BTMonsterGroup> monsters;
  char *partyLabel;
  int treasurePic;
  char *treasureLabel;
  unsigned int xp;
  unsigned int gold;

  char* monsterNames;
  bool canAdvance;
  bool canAttack;
};

#endif
