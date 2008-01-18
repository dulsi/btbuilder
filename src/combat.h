#ifndef __COMBAT_H
#define __COMBAT_H
/*-------------------------------------------------------------------------*\
  <combat.h> -- Combat header file

  Date      Programmer  Description
  05/02/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include <file.h>
#include "combatant.h"
#include "display.h"
#include "map.h"
#include "screenset.h"
#include "spelleffect.h"
#include <map>

class BTCombatError
{
 public:
  BTCombatError(const std::string &e) : error(e) {}

  std::string error;
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
  std::vector<BTCombatant> individual;
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

  void addEffect(int spell, unsigned int expire, int group, int target, BitField &resist);
  void addEncounter(int monsterType, int number = 0);
  void addPlayer(BTDisplay &d, int who);
  void clearEffects(BTDisplay &d);
  void clearEncounters();
  int findScreen(int num);
  bool findTarget(BTPc &pc, int range, BTMonsterGroup *&grp, int &target);
  bool findTargetPC(int range, int &target, int ignore = BT_PARTYSIZE);
  BTMonsterGroup *getMonsterGroup(int group);
  void initScreen(BTDisplay &d);
  bool isWinner() { return won; }
  void movedPlayer(BTDisplay &d, int who, int where);
  virtual void open(const char *filename);
  void run(BTDisplay &d, bool partyAttack = false);
  void runCombat(BTDisplay &d);
  void runMonsterAction(BTDisplay &d, int &active, BTMonsterGroup &grp, BTCombatant &mon);
  void runPcAction(BTDisplay &d, int &active, BTPc &pc);

  // Actions
  static int advance(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int attack(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int cast(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int combatOption(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int defend(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int partyAttack(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int runAway(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int target(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int useItem(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);

 private:
  bool endRound(BTDisplay &d);

 private:
  bool won;
  bool optionState;
  int round;
  std::list<BTMonsterGroup> monsters;
  std::list<BTSpellEffect> spellEffect;
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
