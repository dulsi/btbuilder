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

class BTMonsterGroup;

class BTMonsterCombatant : public BTCombatant
{
 public:
  BTMonsterCombatant(BTMonsterGroup *grp, int startLevel, int startJob, int startAc, int startToHit, int startHp) : BTCombatant(startLevel, startJob, startAc, startToHit, startHp), group(grp) {}

  void deactivate(int &activeNum);
  std::string getName() const;
  bool savingThrow(int difficulty = BTSAVE_DIFFICULTY) const;

 protected:
  BTMonsterGroup *group;
};

class BTMonsterGroup : public BTCombatantCollection
{
 public:
  BTMonsterGroup() : monsterName(0), canMove(true) {}
  ~BTMonsterGroup();

  int findTarget(int ind = BTTARGET_INDIVIDUAL);
  void push(int d);
  void setMonsterType(int type, int number = 0);

  BTCombatant* at(size_t index);
  int getDistance();
  size_t size();

  int monsterType;
  char *monsterName;
  int distance;
  int active;
  bool canMove;
  std::vector<BTMonsterCombatant> individual;
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

  void addEffect(BTBaseEffect *e);
  void addEncounter(int monsterType, int number = 0);
  void addPlayer(BTDisplay &d, int who);
  void clearEffects(BTDisplay &d);
  void clearEncounters();
  int findScreen(int num);
  bool findTarget(BTPc &pc, int range, BTMonsterGroup *&grp, int &target);
  bool findTargetPC(int range, int &target, int ignore = BT_PARTYSIZE);
  bool hasEffectOfType(int type, int group = BTTARGET_NONE, int target = BTTARGET_INDIVIDUAL);
  BTMonsterGroup *getMonsterGroup(int group);
  void initScreen(BTDisplay &d);
  bool isWinner() { return won; }
  void movedPlayer(BTDisplay &d, int who, int where);
  virtual void open(const char *filename);
  void run(BTDisplay &d, bool partyAttack = false);
  void runCombat(BTDisplay &d);
  void runMonsterAction(BTDisplay &d, int &active, int monGroup, int monNumber, BTMonsterGroup &grp, BTCombatant &mon);
  void runPcAction(BTDisplay &d, int &active, int pcNumber, BTPc &pc);

  // Actions
  static int advance(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int attack(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int cast(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int combatOption(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int defend(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int partyAttack(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int runAway(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int sing(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int target(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int useItem(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);

 private:
  void debugActive();
  bool endRound(BTDisplay &d);

 private:
  bool won;
  bool optionState;
  int round;
  std::list<BTMonsterGroup> monsters;
  XMLVector<BTBaseEffect*> effect;
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

