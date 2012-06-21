/*-------------------------------------------------------------------------*\
  <combatant.C> -- Combatant implementation file

  Date      Programmer  Description
  09/16/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "combatant.h"
#include "game.h"

bool BTCombatant::drainLevel()
{
 BTJobList &jobList = BTGame::getGame()->getJobList();
 if (level > 1)
 {
  if (((level - 1) % jobList[job]->improveToHit) == 0)
   --toHit;
  if ((jobList[job]->improveAc) && (((level - 1) % jobList[job]->improveAc) == 0))
   --ac;
  --level;
  return false;
 }
 else
 {
  status.set(BTSTATUS_DEAD);
  return true;
 }
}

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
