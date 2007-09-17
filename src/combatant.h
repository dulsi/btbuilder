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
  BTCombatant() : maxHp(-1), hp(-1), active(true) {}
  BTCombatant(int startHp) : maxHp(startHp), hp(startHp), active(true) {}

  bool isAlive() const;
  bool takeHP(int amount);

  int maxHp, hp;
  BitField status;
  bool active;
};

#endif
