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
  int monsterType;
  int distance;
  int active;
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
  void initScreen(BTDisplay &d);
  bool isWinner() { return won; }
  virtual void open(const char *filename);
  void run(BTDisplay &d, bool partyAttack = false);
  void runCombat(BTDisplay &d);

  // Actions
  static void advance(BTScreenSet &b, BTDisplay &d, BTScreenItem *item);
  static void attack(BTScreenSet &b, BTDisplay &d, BTScreenItem *item);
  static void combatOption(BTScreenSet &b, BTDisplay &d, BTScreenItem *item);
  static void defend(BTScreenSet &b, BTDisplay &d, BTScreenItem *item);
  static void partyAttack(BTScreenSet &b, BTDisplay &d, BTScreenItem *item);
  static void runAway(BTScreenSet &b, BTDisplay &d, BTScreenItem *item);
  static void useItem(BTScreenSet &b, BTDisplay &d, BTScreenItem *item);

 private:
  bool endRound();

 private:
  bool won;
  bool optionState;
  int round;
  std::list<BTMonsterGroup> monsters;
  char *partyLabel;
  unsigned int xp;
  unsigned int gold;

  char* monsterNames;
  bool canAdvance;
  bool canAttack;
};

#endif
