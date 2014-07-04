/*-------------------------------------------------------------------------*\
  <combatant.C> -- Combatant implementation file

  Date      Programmer  Description
  09/16/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "combatant.h"
#include "game.h"

bool BTCombatant::age()
{
 return drainLevel();
}

std::string BTCombatant::attack(BTCombatant *defender, bool melee, const std::string &cause, const std::string &effect, const BTDice &damageDice, IShort chanceXSpecial, IShort xSpecial, int &numAttacksLeft, int &activeNum)
{
 int hits = 0;
 int totalDamage = 0;
 bool dead = false;
 bool totalDrain = false;
 BitField finalSpecial;
 while ((defender->isAlive()) && (numAttacksLeft > 0))
 {
  int roll = BTDice(1, 20).roll();
  if ((1 != roll) && ((20 == roll) || (roll + toHit >= defender->ac)))
  {
   ++hits;
   int damage = 0;
   BitField special;
   damage = damageDice.roll();
   if ((BTEXTRADAMAGE_NONE != xSpecial) && (BTDice(1, 100).roll() <= chanceXSpecial))
    special.set(xSpecial);
   totalDamage += damage;
   if (defender->takeHP(damage))
   {
    dead = true;
    defender->deactivate(activeNum);
   }
   else
   {
    useAutoCombatSkill(melee, special);
    int maxSpecial = special.getMaxSet();
    if (maxSpecial > -1)
    {
     std::string specialText;
     for (int i = 0; i <= maxSpecial; ++i)
     {
      if (!special.isSet(i))
       continue;
      if (defender->savingThrow(BTSAVE_DIFFICULTY))
      {
       special.clear(i);
       continue;
      }
      switch(i)
      {
       case BTEXTRADAMAGE_POISON:
        defender->status.set(BTSTATUS_POISONED);
        break;
       case BTEXTRADAMAGE_LEVELDRAIN:
        if (defender->drainLevel())
        {
         totalDrain = true;
         defender->deactivate(activeNum);
        }
        break;
       case BTEXTRADAMAGE_INSANITY:
        defender->status.set(BTSTATUS_INSANE);
        break;
       case BTEXTRADAMAGE_AGED:
        if (defender->age())
        {
         defender->deactivate(activeNum);
        }
        break;
       case BTEXTRADAMAGE_POSSESSION:
        defender->status.set(BTSTATUS_POSSESSED);
        break;
       case BTEXTRADAMAGE_PARALYSIS:
        defender->status.set(BTSTATUS_PARALYZED);
        break;
       case BTEXTRADAMAGE_STONED:
        defender->status.set(BTSTATUS_STONED);
        defender->deactivate(activeNum);
        break;
       case BTEXTRADAMAGE_CRITICALHIT:
        defender->status.set(BTSTATUS_DEAD);
        defender->deactivate(activeNum);
        break;
       case BTEXTRADAMAGE_ITEMZOT:
        if (!defender->drainItem(5))
        {
         special.clear(BTEXTRADAMAGE_ITEMZOT);
        }
        break;
       case BTEXTRADAMAGE_POINTPHAZE:
        if (!defender->takeSP(5))
        {
         special.clear(BTEXTRADAMAGE_POINTPHAZE);
        }
        break;
       default:
        break;
      }
     }
     finalSpecial |= special;
    }
   }
  }
  --numAttacksLeft;
 }
 std::string text = getName();
 text += " ";
 text += cause;
 text += " ";
 std::string defenderName;
 text += defenderName = defender->getName();
 if (0 < hits)
 {
  text += " ";
  text += effect;
  text += " ";
  char tmp[20];
  if (hits > 1)
  {
   sprintf(tmp, "%d", hits);
   text += tmp;
   text += " times ";
  }
  text += "for ";
  sprintf(tmp, "%d", totalDamage);
  text += tmp;
  text += " points of damage";
  if (dead)
  {
   text += ", killing ";
   text += genderPronouns[defender->getGender()];
  }
  else
  {
   int maxSpecial = finalSpecial.getMaxSet();
   if (maxSpecial > -1)
   {
    std::string specialText;
    for (int i = 0; i <= maxSpecial; ++i)
    {
     if (!finalSpecial.isSet(i))
      continue;
     if ((specialText == "") || (maxSpecial == i))
      specialText += " and";
     else
      specialText += ",";
     switch(i)
     {
      case BTEXTRADAMAGE_POISON:
       specialText += " poisons";
       break;
      case BTEXTRADAMAGE_LEVELDRAIN:
       specialText += " drains a level from ";
       specialText += defenderName;
       if (totalDrain)
       {
        specialText += " totally draining ";
        specialText += genderPronouns[defender->getGender()];
       }
       break;
      case BTEXTRADAMAGE_INSANITY:
       specialText += " inflicts insanity";
       break;
      case BTEXTRADAMAGE_AGED:
       specialText += " withers ";
        specialText += genderPronouns[defender->getGender()];
       break;
      case BTEXTRADAMAGE_POSSESSION:
       specialText += " possesses";
       break;
      case BTEXTRADAMAGE_PARALYSIS:
       specialText += " paralyzes";
       break;
      case BTEXTRADAMAGE_STONED:
       specialText += " stones";
       break;
      case BTEXTRADAMAGE_CRITICALHIT:
       specialText += " critically hits";
       break;
      case BTEXTRADAMAGE_ITEMZOT:
       specialText += " drains an item";
       break;
      case BTEXTRADAMAGE_POINTPHAZE:
       specialText += " absorbs spell points";
       break;
      default:
       break;
     }
    }
    text += specialText;
   }
  }
  if (defender->isAlive())
   text += ".";
  else
   text += "!";
 }
 else
  text += ", but misses!";
 return text;
}

void BTCombatant::deactivate(int &activeNum)
{
 if (initiative != BTINITIATIVE_INACTIVE)
 {
  initiative = BTINITIATIVE_INACTIVE;
  --activeNum;
 }
}

bool BTCombatant::drainItem(int amount)
{
 return false;
}

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

void BTCombatant::restoreLevel()
{
 BTJobList &jobList = BTGame::getGame()->getJobList();
 while (level < maxLevel)
 {
  ++level;
  if (((level - 1) % jobList[job]->improveToHit) == 0)
   ++toHit;
  if ((jobList[job]->improveAc) && (((level - 1) % jobList[job]->improveAc) == 0))
   ++ac;
 }
}

void BTCombatant::rollInitiative()
{
 initiative = BTDice(1, 20).roll() + (level / 2);
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

bool BTCombatant::takeSP(int amount)
{
 return false;
}

void BTCombatant::useAutoCombatSkill(bool melee, BitField &special)
{
}

void BTCombatant::youth()
{
 restoreLevel();
}

std::string BTCombatant::specialAttack(BTCombatant *defender, const BTDice &damageDice, IShort xSpecial, bool farRange, int &activeNum, bool *saved /*= NULL*/)
{
 int totalDamage = 0;
 bool dead = false;
 bool totalDrain = false;
 BitField special;
 totalDamage = damageDice.roll();
 if (farRange)
  totalDamage = totalDamage / 2;
 bool saveResult = false;
 if (saved)
  saveResult = *saved;
 else
  saveResult = defender->savingThrow(BTSAVE_DIFFICULTY);
 if (saveResult)
 {
  totalDamage = totalDamage / 2;
 }
 else if (BTEXTRADAMAGE_NONE != xSpecial)
 {
  special.set(xSpecial);
 }
 if (defender->takeHP(totalDamage))
 {
  dead = true;
  defender->deactivate(activeNum);
 }
 else
 {
  int maxSpecial = special.getMaxSet();
  if (maxSpecial > -1)
  {
   for (int i = 0; i <= maxSpecial; ++i)
   {
    if (!special.isSet(i))
     continue;
    switch(i)
    {
     case BTEXTRADAMAGE_POISON:
      defender->status.set(BTSTATUS_POISONED);
      break;
     case BTEXTRADAMAGE_LEVELDRAIN:
      if (defender->drainLevel())
      {
       totalDrain = true;
       defender->deactivate(activeNum);
      }
      break;
     case BTEXTRADAMAGE_INSANITY:
      defender->status.set(BTSTATUS_INSANE);
      break;
     case BTEXTRADAMAGE_AGED:
      if (defender->age())
      {
       defender->deactivate(activeNum);
      }
      break;
     case BTEXTRADAMAGE_POSSESSION:
      defender->status.set(BTSTATUS_POSSESSED);
      break;
     case BTEXTRADAMAGE_PARALYSIS:
      defender->status.set(BTSTATUS_PARALYZED);
      break;
     case BTEXTRADAMAGE_STONED:
      defender->status.set(BTSTATUS_STONED);
      defender->deactivate(activeNum);
      break;
     case BTEXTRADAMAGE_CRITICALHIT:
      defender->status.set(BTSTATUS_DEAD);
      defender->deactivate(activeNum);
      break;
     case BTEXTRADAMAGE_ITEMZOT:
      if (!defender->drainItem(5))
      {
       special.clear(BTEXTRADAMAGE_ITEMZOT);
      }
      break;
     case BTEXTRADAMAGE_POINTPHAZE:
      if (!defender->takeSP(5))
      {
       special.clear(BTEXTRADAMAGE_POINTPHAZE);
      }
      break;
     default:
      break;
    }
   }
  }
 }
 std::string text;
 std::string defenderName;
 text = defenderName = defender->getName();
 if (saveResult == false)
 {
  if (totalDamage > 0)
  {
   text += " takes ";
   char tmp[20];
   sprintf(tmp, "%d", totalDamage);
   text += tmp;
   text += " points of damage";
  }
  if (dead)
  {
   text += ", killing ";
   text += genderPronouns[defender->getGender()];
  }
  else
  {
   int maxSpecial = special.getMaxSet();
   if (maxSpecial > -1)
   {
    std::string specialText;
    for (int i = 0; i <= maxSpecial; ++i)
    {
     if (!special.isSet(i))
      continue;
     if ((totalDamage > 0) || (specialText != ""))
     {
      if ((specialText == "") || (maxSpecial == i))
       specialText += " and";
      else
       specialText += ",";
     }
     switch(i)
     {
      case BTEXTRADAMAGE_POISON:
       specialText += " is poisoned";
       break;
      case BTEXTRADAMAGE_LEVELDRAIN:
       specialText += " is drained of a level";
       if (totalDrain)
       {
        specialText += " totally draining ";
        specialText += genderPronouns[defender->getGender()];
       }
       break;
      case BTEXTRADAMAGE_INSANITY:
       specialText += " has gone insane";
       break;
      case BTEXTRADAMAGE_AGED:
       specialText += " withers";
       break;
      case BTEXTRADAMAGE_POSSESSION:
       specialText += " is possessed";
       break;
      case BTEXTRADAMAGE_PARALYSIS:
       specialText += " is paralyzed";
       break;
      case BTEXTRADAMAGE_STONED:
       specialText += " is stoned";
       break;
      case BTEXTRADAMAGE_CRITICALHIT:
       specialText += " is killed";
       break;
      case BTEXTRADAMAGE_ITEMZOT:
       specialText += " has an item drained";
       break;
      case BTEXTRADAMAGE_POINTPHAZE:
       specialText += " is drained of spell points";
       break;
      default:
       break;
     }
    }
    text += specialText;
   }
  }
  if (defender->isAlive())
   text += ".";
  else
   text += "!";
 }
 else
 {
  if (totalDamage > 0)
  {
   text += " saves and takes ";
   char tmp[20];
   sprintf(tmp, "%d", totalDamage);
   text += tmp;
   text += " points of damage";
   if (dead)
   {
    text += ", killing ";
    text += genderPronouns[defender->getGender()];
   }
   text += "!";
  }
  else
   text += " saves!";
 }
 return text;
}

