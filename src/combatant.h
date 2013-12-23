#ifndef __COMBATANT_H
#define __COMBATANT_H
/*-------------------------------------------------------------------------*\
  <combatant.h> -- Combatant header file

  Date      Programmer  Description
  05/02/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "bitfield.h"
#include "btconst.h"
#include "dice.h"
#include <string>

class BTCombatant
{
 public:
  BTCombatant() : maxLevel(1), level(1), job(0), ac(0), toHit(0), maxHp(-1), hp(-1), initiative(0) {}
  BTCombatant(int startLevel, int startJob, int startAc, int startToHit, int startHp) : maxLevel(startLevel), level(startLevel), job(startJob), ac(startAc), toHit(startToHit), maxHp(startHp), hp(startHp), initiative(0) {}
  virtual ~BTCombatant() {}

  virtual bool age();
  std::string attack(BTCombatant *defender, bool melee, const std::string &cause, const std::string &effect, const BTDice &damageDice, IShort chanceXSpecial, IShort xSpecial, int &numAttacksLeft, int &activeNum);
  virtual void deactivate(int &activeNum);
  virtual bool drainItem(int amount);
  virtual bool drainLevel();
  virtual int getGender() const = 0;
  virtual std::string getName() const = 0;
  bool isAlive() const;
  virtual bool isIllusion() const = 0;
  virtual void restoreLevel();
  virtual void rollInitiative();
  virtual bool savingThrow(int difficulty = BTSAVE_DIFFICULTY) const = 0;
  bool takeHP(int amount);
  virtual bool takeSP(int amount);
  virtual void useAutoCombatSkill(bool melee, BitField &special);
  virtual void youth();

  static std::string specialAttack(BTCombatant *defender, const BTDice &damageDice, IShort xSpecial, bool farRange, int &activeNum, bool *saved = NULL);

  int maxLevel;
  int level;
  int job;
  int ac;
  int toHit;
  int maxHp, hp;
  BitField status;
  int initiative;
};

class BTCombatantCollection
{
 public:
  virtual BTCombatant* at(size_t index) = 0;
  virtual int getDistance() = 0;
  virtual size_t size() = 0;
};

#endif

