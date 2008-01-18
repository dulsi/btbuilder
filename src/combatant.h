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
  BTCombatant() : ac(0), toHit(0), maxHp(-1), hp(-1), active(true) {}
  BTCombatant(int startAc, int startToHit, int startHp) : ac(startAc), toHit(startToHit), maxHp(startHp), hp(startHp), active(true) {}

  bool isAlive() const;
  bool takeHP(int amount);

  int ac;
  int toHit;
  int maxHp, hp;
  BitField status;
  bool active;
};

#endif
