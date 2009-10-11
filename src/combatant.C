/*-------------------------------------------------------------------------*\
  <combatant.C> -- Combatant implementation file

  Date      Programmer  Description
  09/16/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "combatant.h"

bool BTCombatant::isAlive() const
{
 return (!status.isSet(BTSTATUS_DEAD)) && (!status.isSet(BTSTATUS_STONED));
}

bool BTCombatant::takeHP(int amount)
{
 if (!status.isSet(BTSTATUS_DEAD))
 {
  hp -= amount;
  if (hp < 0)
   status.set(BTSTATUS_DEAD);
 }
 return hp < 0;
}
