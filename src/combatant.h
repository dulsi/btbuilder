#ifndef __COMBATANT_H
#define __COMBATANT_H
/*-------------------------------------------------------------------------*\
  <combatant.h> -- Combatant header file

  Date      Programmer  Description
  05/02/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "bitfield.h"

class BTCombatant
{
 public:
  BTCombatant() : maxLevel(1), level(1), job(0), ac(0), toHit(0), maxHp(-1), hp(-1), active(true) {}
  BTCombatant(int startLevel, int startJob, int startAc, int startToHit, int startHp) : maxLevel(startLevel), level(startLevel), job(startJob), ac(startAc), toHit(startToHit), maxHp(startHp), hp(startHp), active(true) {}

  virtual bool age();
  virtual bool drainLevel();
  bool isAlive() const;
  virtual void restoreLevel();
  bool takeHP(int amount);
  virtual void youth();

  int maxLevel;
  int level;
  int job;
  int ac;
  int toHit;
  int maxHp, hp;
  BitField status;
  bool active;
};

#endif
