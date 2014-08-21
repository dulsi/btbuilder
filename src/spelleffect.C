/*-------------------------------------------------------------------------*\
  <spelleffect.C> -- Spell Effect implementation file

  Date      Programmer  Description
  11/09/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "spelleffect.h"
#include "display.h"
#include "combat.h"
#include "game.h"

#define BTSPELLFLG_DAMAGE 0
#define BTSPELLFLG_KILLED 1
#define BTSPELLFLG_EXCLAMATION 2

BTBaseEffect::BTBaseEffect(int t, int x, int s, int m)
 : type(t), expiration(x), expire(false), first(true), singer(s), musicId(m)
{
}

int BTBaseEffect::apply(BTDisplay &d, BTCombat *combat, int g /*= BTTARGET_NONE*/, int trgt /*= BTTARGET_INDIVIDUAL*/)
{
 if (g != BTTARGET_NONE)
  return 0;
 else
  return maintain(d, combat);
}

int BTBaseEffect::maintain(BTDisplay &d, BTCombat *combat)
{
 return 0;
}

void BTBaseEffect::finish(BTDisplay &d, BTCombat *combat, int g /*= BTTARGET_NONE*/, int trgt /*= BTTARGET_INDIVIDUAL*/)
{
}

void BTBaseEffect::serialize(ObjectSerializer *s)
{
 s->add("type", &type);
 s->add("expiration", &expiration);
 s->add("expire", &expire);
 s->add("first", &first);
 s->add("singer", &singer);
}

bool BTBaseEffect::targets(int g, int who, bool exact /*= true*/)
{
 return false;
}

bool BTBaseEffect::targetsMonsters()
{
 return false;
}

void BTBaseEffect::move(int g, int who, int where)
{
 if ((g == BTTARGET_PARTY) && (who == singer))
 {
  singer = where;
 }
 else if ((g == BTTARGET_PARTY) && (who < where) && (where >= singer) && (who < singer))
 {
  singer--;
 }
 else if ((g == BTTARGET_PARTY) && (who > where) && (where <= singer) && (who > singer))
 {
  singer++;
 }
}

void BTBaseEffect::remove(BTCombat *combat, int g, int who)
{
}

bool BTBaseEffect::isExpired(BTGame *game)
{
 if (expire)
  return true;
 if (game->isExpired(expiration))
 {
  return true;
 }
 else if (singer != BTTARGET_NOSINGER)
 {
  BTParty &party = game->getParty();
  if (!party[singer]->isAlive())
   return true;
  else
   return false;
 }
 else
  return false;
}

BTTargetedEffect::BTTargetedEffect(int t, int x, int s, int m, int g, int trgt)
 : BTBaseEffect(t, x, s, m), group(g), target(trgt)
{
}

void BTTargetedEffect::serialize(ObjectSerializer *s)
{
 s->add("group", &group);
 s->add("target", &target);
 BTBaseEffect::serialize(s);
}

bool BTTargetedEffect::targets(int g, int who, bool exact /*= true*/)
{
 if ((g == group) && (who == target))
  return true;
 else if (!exact)
 {
  if ((g == BTTARGET_ALLMONSTERS) && (group >= BTTARGET_MONSTER))
   return true;
  if ((g == group) && (target == BTTARGET_INDIVIDUAL))
   return true;
  if ((g == group) && (who == BTTARGET_INDIVIDUAL))
   return true;
 }
 return false;
}

bool BTTargetedEffect::targetsMonsters()
{
 return group >= BTTARGET_MONSTER;
}

void BTTargetedEffect::move(int g, int who, int where)
{
 if ((g == group) && (who == target))
 {
  target = where;
 }
 else if ((g == group) && (who < where) && (where >= target) && (who < target))
 {
  target--;
 }
 else if ((g == group) && (who > where) && (where <= target) && (who > target))
 {
  target++;
 }
 BTBaseEffect::move(g, who, where);
}

void BTTargetedEffect::remove(BTCombat *combat, int g, int who)
{
 if ((group == g) && (target != BTTARGET_INDIVIDUAL) && (target > who))
 {
  --target;
 }
 else if ((group > g) && (target == BTTARGET_INDIVIDUAL))
 {
  --group;
 }
}

BTResistedEffect::BTResistedEffect(int t, int x, int s, int m, int g, int trgt)
 : BTTargetedEffect(t, x, s, m, g, trgt)
{
}

void BTResistedEffect::serialize(ObjectSerializer *s)
{
 s->add("resists", &resists, NULL);
 BTTargetedEffect::serialize(s);
}

bool BTResistedEffect::checkResists(BTCombat *combat, int g /*= BTTARGET_NONE*/, int trgt /*= BTTARGET_INDIVIDUAL*/)
{
 if (g == BTTARGET_NONE)
 {
  g = group;
  trgt = target;
 }
 BTGame *game = BTGame::getGame();
 BTFactory<BTMonster> &monList = game->getMonsterList();
 if (BTTARGET_ALLMONSTERS == g)
 {
  int total = 0;
  bool allResists = true;
  for (int i = 0; i < BTCOMBAT_MAXENCOUNTERS; ++i)
  {
   BTMonsterGroup *grp = combat->getMonsterGroup(i);
   if (NULL == grp)
    break;
   int resistance = monList[grp->monsterType].getMagicResistance();
   if (resistance > 0)
   {
    for (int k = 0; k < grp->individual.size(); ++k)
    {
     if (BTDice(1, 100).roll() <= resistance)
      resists.set(total + k);
     else
      allResists = false;
    }
   }
   else
    allResists = false;
   total += grp->individual.size();
  }
  if (allResists)
   return true;
 }
 else if (BTTARGET_PARTY == g)
 {
  BTParty &party = game->getParty();
  if (BTTARGET_INDIVIDUAL == trgt)
  {
   bool allResists = true;
   for (int i = 0; i < party.size(); ++i)
   {
    if (BTMONSTER_NONE != party[i]->monster)
    {
     if (BTDice(1, 100).roll() <= monList[party[i]->monster].getMagicResistance())
      resists.set(i);
     else
      allResists = false;
    }
    else
     allResists = false;
   }
   if (allResists)
    return true;
  }
  else
  {
   if (BTMONSTER_NONE != party[trgt]->monster)
   {
    if (BTDice(1, 100).roll() <= monList[party[trgt]->monster].getMagicResistance())
    {
     if ((group == BTTARGET_PARTY) && (BTTARGET_INDIVIDUAL == target))
      resists.set(trgt);
     else
      resists.set(0);
     return true;
    }
   }
  }
 }
 else if (g >= BTTARGET_MONSTER)
 {
  BTMonsterGroup *grp = combat->getMonsterGroup(g - BTTARGET_MONSTER);
  int resistance = monList[grp->monsterType].getMagicResistance();
  if (resistance > 0)
  {
   if (BTTARGET_INDIVIDUAL == trgt)
   {
    bool allResists = true;
    for (int i = 0; i < grp->individual.size(); ++i)
    {
     if (BTDice(1, 100).roll() <= resistance)
      resists.set(i);
     else
      allResists = false;
    }
    if (allResists)
     return true;
   }
   else if (BTDice(1, 100).roll() <= resistance)
   {
    resists.set(0);
    return true;
   }
  }
 }
 return false;
}

BTAttackEffect::BTAttackEffect(int t, int x, int s, int m, int rng, int erng, int d, int g, int trgt, const BTDice &dam, int sts)
 : BTResistedEffect(t, x, s, m, g, trgt), range(rng), effectiveRange(erng), distance(d), damage(dam), status(sts)
{
}

void BTAttackEffect::serialize(ObjectSerializer *s)
{
 s->add("range", &range);
 s->add("effectiveRange", &effectiveRange);
 s->add("distance", &distance);
 s->add("damage", &damage);
 s->add("status", &status);
 BTResistedEffect::serialize(s);
}

int BTAttackEffect::apply(BTDisplay &d, BTCombat *combat, int g /*= BTTARGET_NONE*/, int trgt /*= BTTARGET_INDIVIDUAL*/)
{
 if (g != BTTARGET_NONE)
 {
  checkResists(combat, g, trgt);
  return 0;
 }
 else
 {
  if (checkResists(combat, group, target))
  {
   displayResists(d, combat);
   throw BTAllResistException();
  }
  return maintain(d, combat);
 }
}

int BTAttackEffect::maintain(BTDisplay &d, BTCombat *combat)
{
 int killed = 0;
 BTGame *game = BTGame::getGame();
 BTFactory<BTMonster> &monList = game->getMonsterList();
 if (BTTARGET_PARTY == group)
 {
  BTParty &party = game->getParty();
  if (distance > (range * (1 + effectiveRange)))
   return 0;
  if (BTTARGET_INDIVIDUAL == target)
  {
   killed += applyToGroup(d, &party);
  }
  else
  {
   if (party[target]->isAlive())
   {
    int activeNum = 0;
    std::string text = BTCombatant::specialAttack(party[target], damage, status, (distance > range), activeNum);
    killed += abs(activeNum);
    d.drawMessage(text.c_str(), game->getDelay());
   }
  }
 }
 else if (BTTARGET_ALLMONSTERS == group)
 {
  int resistOffset = 0;
  for (int i = 0; i < BTCOMBAT_MAXENCOUNTERS; ++i)
  {
   BTMonsterGroup *grp = combat->getMonsterGroup(i);
   if (NULL == grp)
    break;
   killed += applyToGroup(d, grp, resistOffset);
   resistOffset += grp->size();
  }
 }
 else if (group >= BTTARGET_MONSTER)
 {
  BTMonsterGroup *grp = combat->getMonsterGroup(group - BTTARGET_MONSTER);
  if (abs(grp->distance - distance) > (range * (1 + effectiveRange)))
   return 0;
  if (BTTARGET_INDIVIDUAL == target)
  {
   killed += applyToGroup(d, grp);
  }
  else
  {
   if (grp->individual[target].isAlive())
   {
    int activeNum = 0;
    std::string text = BTCombatant::specialAttack(&(grp->individual[target]), damage, status, (abs(grp->distance - distance) > range), activeNum);
    killed += abs(activeNum);
    d.drawMessage(text.c_str(), game->getDelay());
   }
  }
 }
 if (BTTARGET_PARTY == group)
  d.drawStats();
 return killed;
}

int BTAttackEffect::applyToGroup(BTDisplay &d, BTCombatantCollection *grp, int resistOffset /*= 0*/)
{
 if (abs(grp->getDistance() - distance) > (range * (1 + effectiveRange)))
  return 0;
 BTGame *game = BTGame::getGame();
 int killed(0);
 for (int i = 0; i < grp->size(); ++i)
 {
  bool saved = resists.isSet(resistOffset + i);
  int activeNum = 0;
  if ((grp->at(i)->isAlive()) && ((first) || (!saved)))
  {
   std::string text = BTCombatant::specialAttack(grp->at(i), damage, status, (abs(grp->getDistance() - distance) > range), activeNum, (first ? &saved : NULL));
   killed += abs(activeNum);
   d.drawMessage(text.c_str(), game->getDelay());
  }
 }
 return killed;
}

void BTAttackEffect::finish(BTDisplay &d, BTCombat *combat, int g /*= BTTARGET_NONE*/, int trgt /*= BTTARGET_INDIVIDUAL*/)
{
 if (g == BTTARGET_NONE)
  return;
 if ((status == BTSTATUS_NONE) || (status == BTSTATUS_LEVELDRAIN) || (status == BTSTATUS_AGED))
  return;
 if (BTTARGET_PARTY == group)
 {
  BTGame *game = BTGame::getGame();
  BTParty &party = game->getParty();
  if (BTTARGET_INDIVIDUAL == target)
  {
   for (int i = 0; i < party.size(); ++i)
   {
    if (party[i]->status.isSet(status))
    {
     party[i]->status.clear(status);
    }
   }
  }
  else
  {
   if (party[target]->status.isSet(status))
   {
    party[target]->status.clear(status);
   }
  }
 }
 else if (BTTARGET_ALLMONSTERS == group)
 {
  for (int i = 0; i < BTCOMBAT_MAXENCOUNTERS; ++i)
  {
   BTMonsterGroup *grp = combat->getMonsterGroup(i);
   if (NULL == grp)
    break;
   for (int k = 0; k < grp->individual.size(); ++k)
   {
    if ((grp->individual[k].isAlive()) && (!grp->individual[k].status.isSet(status)))
    {
     grp->individual[k].status.clear(status);
    }
   }
  }
 }
 else if (group >= BTTARGET_MONSTER)
 {
  BTMonsterGroup *grp = combat->getMonsterGroup(group - BTTARGET_MONSTER);
  if (BTTARGET_INDIVIDUAL == target)
  {
   for (int i = 0; i < grp->individual.size(); ++i)
   {
    if (grp->individual[i].status.isSet(status))
    {
     grp->individual[i].status.clear(status);
    }
   }
  }
  else
  {
   if (grp->individual[target].status.isSet(status))
   {
    grp->individual[target].status.clear(status);
   }
  }
 }
 if (BTTARGET_PARTY == group)
  d.drawStats();
}

void BTAttackEffect::move(int g, int who, int where)
{
 if ((g == group) && (BTTARGET_INDIVIDUAL == target))
 {
  resists.move(who, where);
 }
 BTTargetedEffect::move(g, who, where);
}

void BTAttackEffect::remove(BTCombat *combat, int g, int who)
{
 if ((group == g) && (target == BTTARGET_INDIVIDUAL))
 {
  resists.remove(who);
 }
 else if (group == BTTARGET_ALLMONSTERS)
 {
  int total = 0;
  for (int i = 0; i < g; ++i)
  {
   BTMonsterGroup *grp = combat->getMonsterGroup(i);
   if (NULL == grp)
    break;
   total += grp->individual.size();
  }
  resists.remove(total + who);
 }
 BTTargetedEffect::remove(combat, g, who);
}

void BTAttackEffect::displayResists(BTDisplay &d, BTCombat *combat)
{
 std::string text;
 if (BTTARGET_ALLMONSTERS == group)
 {
  text = "All monsters";
 }
 else if (BTTARGET_PARTY == group)
 {
  BTParty &party = BTGame::getGame()->getParty();
  if (BTTARGET_INDIVIDUAL == target)
   text = "The whole party";
  else
   text = party[target]->name;
 }
 else if (group >= BTTARGET_MONSTER)
 {
  BTFactory<BTMonster> &monList = BTGame::getGame()->getMonsterList();
  BTMonsterGroup *grp = combat->getMonsterGroup(group - BTTARGET_MONSTER);
  if ((BTTARGET_INDIVIDUAL == target) && (1 < grp->individual.size()))
   text = monList[grp->monsterType].getPluralName();
  else
   text = monList[grp->monsterType].getName();
 }
 text += " repelled the spell!";
 d.drawMessage(text.c_str(), BTGame::getGame()->getDelay());
}

BTCureStatusEffect::BTCureStatusEffect(int t, int x, int s, int m, int g, int trgt, int sts)
 : BTTargetedEffect(t, x, s, m, g, trgt), status(sts)
{
}

void BTCureStatusEffect::serialize(ObjectSerializer *s)
{
 s->add("status", &status);
 BTTargetedEffect::serialize(s);
}

int BTCureStatusEffect::maintain(BTDisplay &d, BTCombat *combat)
{
 if (BTTARGET_PARTY == group)
 {
  BTGame *game = BTGame::getGame();
  BTParty &party = game->getParty();
  if (BTTARGET_INDIVIDUAL == target)
  {
   for (int i = 0; i < party.size(); ++i)
   {
    if (status == BTSTATUS_LEVELDRAIN)
    {
     party[i]->restoreLevel();
    }
    else if (status == BTSTATUS_AGED)
    {
     party[i]->youth();
    }
    else if (party[i]->status.isSet(status))
    {
     party[i]->status.clear(status);
    }
   }
  }
  else
  {
   if (status == BTSTATUS_LEVELDRAIN)
   {
    party[target]->restoreLevel();
   }
   else if (status == BTSTATUS_AGED)
   {
    party[target]->youth();
   }
   else if (party[target]->status.isSet(status))
   {
    party[target]->status.clear(status);
   }
  }
 }
 else if (BTTARGET_ALLMONSTERS == group)
 {
  for (int i = 0; i < BTCOMBAT_MAXENCOUNTERS; ++i)
  {
   BTMonsterGroup *grp = combat->getMonsterGroup(i);
   if (NULL == grp)
    break;
   for (int k = 0; k < grp->individual.size(); ++k)
   {
    if (status == BTSTATUS_LEVELDRAIN)
    {
     grp->individual[k].restoreLevel();
    }
    else if (status == BTSTATUS_AGED)
    {
     grp->individual[k].youth();
    }
    else if ((grp->individual[k].isAlive()) && (!grp->individual[k].status.isSet(status)))
    {
     grp->individual[k].status.clear(status);
    }
   }
  }
 }
 else if (group >= BTTARGET_MONSTER)
 {
  BTMonsterGroup *grp = combat->getMonsterGroup(group - BTTARGET_MONSTER);
  if (BTTARGET_INDIVIDUAL == target)
  {
   for (int i = 0; i < grp->individual.size(); ++i)
   {
    if (status == BTSTATUS_LEVELDRAIN)
    {
     grp->individual[i].restoreLevel();
    }
    else if (status == BTSTATUS_AGED)
    {
     grp->individual[i].youth();
    }
    else if (grp->individual[i].status.isSet(status))
    {
     grp->individual[i].status.clear(status);
    }
   }
  }
  else
  {
   if (status == BTSTATUS_LEVELDRAIN)
   {
    grp->individual[target].restoreLevel();
   }
   else if (status == BTSTATUS_AGED)
   {
    grp->individual[target].youth();
   }
   else if (grp->individual[target].status.isSet(status))
   {
    grp->individual[target].status.clear(status);
   }
  }
 }
 d.drawStats();
 return 0;
}

BTHealEffect::BTHealEffect(int t, int x, int s, int m, int g, int trgt, const BTDice &h)
 : BTTargetedEffect(t, x, s, m, g, trgt), heal(h)
{
}

void BTHealEffect::serialize(ObjectSerializer *s)
{
 s->add("heal", &heal);
 BTTargetedEffect::serialize(s);
}

int BTHealEffect::maintain(BTDisplay &d, BTCombat *combat)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 if (BTTARGET_PARTY == group)
 {
  if (BTTARGET_INDIVIDUAL == target)
  {
   for (int i = 0; i < party.size(); ++i)
   {
    if (party[i]->isAlive())
    {
     party[i]->giveHP(heal.roll());
    }
   }
  }
  else
  {
   if (party[target]->isAlive())
    party[target]->giveHP(heal.roll());
  }
  d.drawStats();
 }
 return 0;
}

BTSummonMonsterEffect::BTSummonMonsterEffect(int t, int x, int s, int m, int g, int trgt)
 : BTTargetedEffect(t, x, s, m, g, trgt)
{
}

void BTSummonMonsterEffect::finish(BTDisplay &d, BTCombat *combat, int g /*= BTTARGET_NONE*/, int trgt /*= BTTARGET_INDIVIDUAL*/)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 if (BTTARGET_PARTY == group)
 {
  std::string text = party[target]->name;
  if (party.remove(target, d))
  {
   text += " leaves your party.";
   d.drawMessage(text.c_str(), game->getDelay());
  }
 }
}

BTSummonIllusionEffect::BTSummonIllusionEffect(int t, int x, int s, int m, int g, int trgt)
 : BTTargetedEffect(t, x, s, m, g, trgt)
{
}

void BTSummonIllusionEffect::finish(BTDisplay &d, BTCombat *combat, int g /*= BTTARGET_NONE*/, int trgt /*= BTTARGET_INDIVIDUAL*/)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 if (BTTARGET_PARTY == group)
 {
  std::string text = "The illusionary ";
  text += party[target]->name;
  if (party.remove(target, d))
  {
   text += " disappears!";
   d.drawMessage(text.c_str(), game->getDelay());
  }
 }
}

BTDispellIllusionEffect::BTDispellIllusionEffect(int t, int x, int s, int m, int rng, int erng, int d, int g, int trgt)
 : BTTargetedEffect(t, x, s, m, g, trgt), range(rng), effectiveRange(erng), distance(d)
{
}

void BTDispellIllusionEffect::serialize(ObjectSerializer *s)
{
 s->add("range", &range);
 s->add("effectiveRange", &effectiveRange);
 s->add("distance", &distance);
 BTTargetedEffect::serialize(s);
}

int BTDispellIllusionEffect::maintain(BTDisplay &d, BTCombat *combat)
{
 int killed = 0;
 BTGame *game = BTGame::getGame();
 BTFactory<BTMonster> &monList = game->getMonsterList();
 if (BTTARGET_PARTY == group)
 {
  BTParty &party = game->getParty();
  if (distance > (range * (1 + effectiveRange)))
   return 0;
  if (BTTARGET_INDIVIDUAL == target)
  {
   killed += applyToGroup(d, &party);
  }
  else
  {
   killed += apply(d, party[target]);
  }
 }
 else if (BTTARGET_ALLMONSTERS == group)
 {
  int resistOffset = 0;
  for (int i = 0; i < BTCOMBAT_MAXENCOUNTERS; ++i)
  {
   BTMonsterGroup *grp = combat->getMonsterGroup(i);
   if (NULL == grp)
    break;
   killed += applyToGroup(d, grp);
   resistOffset += grp->size();
  }
 }
 else if (group >= BTTARGET_MONSTER)
 {
  BTMonsterGroup *grp = combat->getMonsterGroup(group - BTTARGET_MONSTER);
  if (abs(grp->distance - distance) > (range * (1 + effectiveRange)))
   return 0;
  if (BTTARGET_INDIVIDUAL == target)
  {
   killed += applyToGroup(d, grp);
  }
  else
  {
   killed += apply(d, grp->at(target));
  }
 }
 if (BTTARGET_PARTY == group)
  d.drawStats();
 return killed;
}

int BTDispellIllusionEffect::applyToGroup(BTDisplay &d, BTCombatantCollection *grp)
{
 if (abs(grp->getDistance() - distance) > (range * (1 + effectiveRange)))
  return 0;
 int killed(0);
 for (int i = 0; i < grp->size(); ++i)
 {
  killed += apply(d, grp->at(i));
 }
 return killed;
}

int BTDispellIllusionEffect::apply(BTDisplay &d, BTCombatant *target)
{
 int killed(0);
 if ((target->isAlive()) && (target->isIllusion()))
 {
  BTGame *game = BTGame::getGame();
  int activeNum = 0;
  target->deactivate(activeNum);
  target->status.set(BTSTATUS_DEAD);
  killed += abs(activeNum);
  std::string text = "An illusionary ";
  text += target->getName();
  text += " disappears!";
  d.drawMessage(text.c_str(), game->getDelay());
 }
 return killed;
}

BTArmorBonusEffect::BTArmorBonusEffect(int t, int x, int s, int m, int g, int trgt, int b)
 : BTTargetedEffect(t, x, s, m, g, trgt), bonus(b)
{
}

void BTArmorBonusEffect::serialize(ObjectSerializer *s)
{
 s->add("bonus", &bonus);
 BTTargetedEffect::serialize(s);
}

int BTArmorBonusEffect::apply(BTDisplay &d, BTCombat *combat, int g /*= BTTARGET_NONE*/, int trgt /*= BTTARGET_INDIVIDUAL*/)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 if (g == BTTARGET_NONE)
 {
  g = group;
  trgt = target;
 }
 if (BTTARGET_PARTY == g)
 {
  if (BTTARGET_INDIVIDUAL == trgt)
  {
   for (int i = 0; i < party.size(); ++i)
   {
    party[i]->ac += bonus;
   }
  }
  else
  {
   party[trgt]->ac += bonus;
  }
  d.drawStats();
 }
 else if (BTTARGET_ALLMONSTERS == g)
 {
  for (int i = 0; i < BTCOMBAT_MAXENCOUNTERS; ++i)
  {
   BTMonsterGroup *grp = combat->getMonsterGroup(i);
   if (NULL == grp)
    break;
   for (int k = 0; k < grp->individual.size(); ++k)
   {
    grp->individual[k].ac += bonus;
   }
  }
 }
 else if (g >= BTTARGET_MONSTER)
 {
  BTMonsterGroup *grp = combat->getMonsterGroup(g - BTTARGET_MONSTER);
  if (BTTARGET_INDIVIDUAL == trgt)
  {
   for (int i = 0; i < grp->individual.size(); ++i)
   {
    grp->individual[i].ac += bonus;
   }
  }
  else
  {
   grp->individual[trgt].ac += bonus;
  }
 }
 return 0;
}

void BTArmorBonusEffect::finish(BTDisplay &d, BTCombat *combat, int g /*= BTTARGET_NONE*/, int trgt /*= BTTARGET_INDIVIDUAL*/)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 if (g == BTTARGET_NONE)
 {
  g = group;
  trgt = target;
 }
 if (BTTARGET_PARTY == g)
 {
  if (BTTARGET_INDIVIDUAL == trgt)
  {
   for (int i = 0; i < party.size(); ++i)
   {
    party[i]->ac -= bonus;
   }
  }
  else
  {
   party[trgt]->ac -= bonus;
  }
  d.drawStats();
 }
 else if (BTTARGET_ALLMONSTERS == g)
 {
  for (int i = 0; i < BTCOMBAT_MAXENCOUNTERS; ++i)
  {
   BTMonsterGroup *grp = combat->getMonsterGroup(i);
   if (NULL == grp)
    break;
   for (int k = 0; k < grp->individual.size(); ++k)
   {
    grp->individual[k].ac -= bonus;
   }
  }
 }
 else if (g >= BTTARGET_MONSTER)
 {
  BTMonsterGroup *grp = combat->getMonsterGroup(g - BTTARGET_MONSTER);
  if (BTTARGET_INDIVIDUAL == trgt)
  {
   for (int i = 0; i < grp->individual.size(); ++i)
   {
    grp->individual[i].ac -= bonus;
   }
  }
  else
  {
   grp->individual[trgt].ac -= bonus;
  }
 }
}

BTHitBonusEffect::BTHitBonusEffect(int t, int x, int s, int m, int g, int trgt, int b)
 : BTTargetedEffect(t, x, s, m, g, trgt), bonus(b)
{
}

void BTHitBonusEffect::serialize(ObjectSerializer *s)
{
 s->add("bonus", &bonus);
 BTTargetedEffect::serialize(s);
}

int BTHitBonusEffect::apply(BTDisplay &d, BTCombat *combat, int g /*= BTTARGET_NONE*/, int trgt /*= BTTARGET_INDIVIDUAL*/)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 if (g == BTTARGET_NONE)
 {
  g = group;
  trgt = target;
 }
 if (BTTARGET_PARTY == g)
 {
  if (BTTARGET_INDIVIDUAL == trgt)
  {
   for (int i = 0; i < party.size(); ++i)
   {
    party[i]->toHit += bonus;
   }
  }
  else
  {
   party[trgt]->toHit += bonus;
  }
  d.drawStats();
 }
 else if (BTTARGET_ALLMONSTERS == g)
 {
  for (int i = 0; i < BTCOMBAT_MAXENCOUNTERS; ++i)
  {
   BTMonsterGroup *grp = combat->getMonsterGroup(i);
   if (NULL == grp)
    break;
   for (int k = 0; k < grp->individual.size(); ++k)
   {
    grp->individual[k].toHit += bonus;
   }
  }
 }
 else if (g >= BTTARGET_MONSTER)
 {
  BTMonsterGroup *grp = combat->getMonsterGroup(g - BTTARGET_MONSTER);
  if (BTTARGET_INDIVIDUAL == trgt)
  {
   for (int i = 0; i < grp->individual.size(); ++i)
   {
    grp->individual[i].toHit += bonus;
   }
  }
  else
  {
   grp->individual[trgt].toHit += bonus;
  }
 }
 return 0;
}

void BTHitBonusEffect::finish(BTDisplay &d, BTCombat *combat, int g /*= BTTARGET_NONE*/, int trgt /*= BTTARGET_INDIVIDUAL*/)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 if (g == BTTARGET_NONE)
 {
  g = group;
  trgt = target;
 }
 if (BTTARGET_PARTY == g)
 {
  if (BTTARGET_INDIVIDUAL == trgt)
  {
   for (int i = 0; i < party.size(); ++i)
   {
    party[i]->toHit -= bonus;
   }
  }
  else
  {
   party[trgt]->toHit -= bonus;
  }
  d.drawStats();
 }
 else if (BTTARGET_ALLMONSTERS == g)
 {
  for (int i = 0; i < BTCOMBAT_MAXENCOUNTERS; ++i)
  {
   BTMonsterGroup *grp = combat->getMonsterGroup(i);
   if (NULL == grp)
    break;
   for (int k = 0; k < grp->individual.size(); ++k)
   {
    grp->individual[k].toHit -= bonus;
   }
  }
 }
 else if (g >= BTTARGET_MONSTER)
 {
  BTMonsterGroup *grp = combat->getMonsterGroup(g - BTTARGET_MONSTER);
  if (BTTARGET_INDIVIDUAL == trgt)
  {
   for (int i = 0; i < grp->individual.size(); ++i)
   {
    grp->individual[i].toHit -= bonus;
   }
  }
  else
  {
   grp->individual[trgt].toHit -= bonus;
  }
 }
}

BTResurrectEffect::BTResurrectEffect(int t, int x, int s, int m, int g, int trgt)
 : BTTargetedEffect(t, x, s, m, g, trgt)
{
}

int BTResurrectEffect::maintain(BTDisplay &d, BTCombat *combat)
{
 if (BTTARGET_PARTY == group)
 {
  if (BTTARGET_INDIVIDUAL == target)
  {
   BTGame *game = BTGame::getGame();
   BTParty &party = game->getParty();
   for (int i = 0; i < party.size(); ++i)
   {
    if (!party[i]->isAlive())
    {
     std::string text = party[i]->name;
     text += " rises from the dead!";
     d.drawMessage(text.c_str(), game->getDelay());
     party[i]->status.clear(BTSTATUS_DEAD);
     party[i]->hp = 1;
    }
   }
   d.drawStats();
  }
  else
  {
   // BTCS either cancels spells on death or doesn't implement this
  }
 }
}

BTDispellMagicEffect::BTDispellMagicEffect(int t, int x, int s, int m, int rng, int erng, int d, int g, int trgt)
 : BTTargetedEffect(t, x, s, m, g, trgt), range(rng), effectiveRange(erng), distance(d)
{
}

void BTDispellMagicEffect::serialize(ObjectSerializer *s)
{
 s->add("range", &range);
 s->add("effectiveRange", &effectiveRange);
 s->add("distance", &distance);
 BTTargetedEffect::serialize(s);
}

int BTDispellMagicEffect::maintain(BTDisplay &d, BTCombat *combat)
{
 bool oldExpire = expire;
 BTGame *game = BTGame::getGame();
 if (BTTARGET_PARTY == group)
 {
  if (distance > (range * (1 + effectiveRange)))
   return 0;
  game->clearEffectsBySource(d, false, group, target);
 }
 else if (BTTARGET_ALLMONSTERS == group)
 {
  for (int i = 0; i < BTCOMBAT_MAXENCOUNTERS; ++i)
  {
   BTMonsterGroup *grp = combat->getMonsterGroup(i);
   if (NULL == grp)
    break;
   if (abs(grp->distance - distance) > (range * (1 + effectiveRange)))
    continue;
   game->clearEffectsBySource(d, false, i + BTTARGET_MONSTER, target);
  }
 }
 else if (group >= BTTARGET_MONSTER)
 {
  BTMonsterGroup *grp = combat->getMonsterGroup(group - BTTARGET_MONSTER);
  if (abs(grp->distance - distance) > (range * (1 + effectiveRange)))
   return 0;
  game->clearEffectsBySource(d, false, group, target);
 }
 expire = oldExpire;
 return 0;
}

BTPhaseDoorEffect::BTPhaseDoorEffect(int t, int x, int s, int m, int mX, int mY, int f)
 : BTBaseEffect(t, x, s, m), mapX(mX), mapY(mY), facing(f)
{
}

void BTPhaseDoorEffect::serialize(ObjectSerializer *s)
{
 s->add("mapX", &mapX);
 s->add("mapY", &mapY);
 s->add("facing", &facing);
 BTBaseEffect::serialize(s);
}

BTRegenSkillEffect::BTRegenSkillEffect(int t, int x, int s, int m, int g, int trgt, int sk, const BTDice& u)
 : BTTargetedEffect(t, x, s, m, g, trgt), skill(sk), use(u)
{
}

void BTRegenSkillEffect::serialize(ObjectSerializer *s)
{
 s->add("skill", &skill);
 s->add("use", &use);
 BTTargetedEffect::serialize(s);
}

int BTRegenSkillEffect::maintain(BTDisplay &d, BTCombat *combat)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 if (BTTARGET_PARTY == group)
 {
  if (BTTARGET_INDIVIDUAL == target)
  {
   for (int i = 0; i < party.size(); ++i)
   {
    if (party[i]->isAlive())
    {
     party[i]->giveSkillUse(skill, use.roll());
    }
   }
  }
  else
  {
   if (party[target]->isAlive())
    party[target]->giveSkillUse(skill, use.roll());
  }
  d.drawStats();
 }
 return 0;
}

BTPushEffect::BTPushEffect(int t, int x, int s, int m, int g, int trgt, int dis)
 : BTTargetedEffect(t, x, s, m, g, trgt), distance(dis)
{
}

void BTPushEffect::serialize(ObjectSerializer *s)
{
 s->add("distance", &distance);
 BTTargetedEffect::serialize(s);
}

int BTPushEffect::maintain(BTDisplay &d, BTCombat *combat)
{
 if (combat == NULL)
  return 0;
 if (BTTARGET_PARTY == group)
 {
  for (int i = 0; i < BTCOMBAT_MAXENCOUNTERS; ++i)
  {
   BTMonsterGroup *grp = combat->getMonsterGroup(i);
   if (NULL == grp)
    break;
   grp->push(-1 * distance);
  }
 }
 else if (BTTARGET_ALLMONSTERS == group)
 {
  for (int i = 0; i < BTCOMBAT_MAXENCOUNTERS; ++i)
  {
   BTMonsterGroup *grp = combat->getMonsterGroup(i);
   if (NULL == grp)
    break;
   grp->push(distance);
  }
 }
 else if (group >= BTTARGET_MONSTER)
 {
  BTMonsterGroup *grp = combat->getMonsterGroup(group - BTTARGET_MONSTER);
  grp->push(distance);
 }
 return 0;
}

BTAttackRateBonusEffect::BTAttackRateBonusEffect(int t, int x, int s, int m, int g, int trgt, int b)
 : BTTargetedEffect(t, x, s, m, g, trgt), bonus(b)
{
}

void BTAttackRateBonusEffect::serialize(ObjectSerializer *s)
{
 s->add("bonus", &bonus);
 BTTargetedEffect::serialize(s);
}

int BTAttackRateBonusEffect::apply(BTDisplay &d, BTCombat *combat, int g /*= BTTARGET_NONE*/, int trgt /*= BTTARGET_INDIVIDUAL*/)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 if (g == BTTARGET_NONE)
 {
  g = group;
  trgt = target;
 }
 if (BTTARGET_PARTY == g)
 {
  if (BTTARGET_INDIVIDUAL == trgt)
  {
   for (int i = 0; i < party.size(); ++i)
   {
    party[i]->rateAttacks += bonus;
   }
  }
  else
  {
   party[trgt]->rateAttacks += bonus;
  }
  d.drawStats();
 }
 else
 {
  // Doesn't work on monsters
 }
 return 0;
}

void BTAttackRateBonusEffect::finish(BTDisplay &d, BTCombat *combat, int g /*= BTTARGET_NONE*/, int trgt /*= BTTARGET_INDIVIDUAL*/)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 if (g == BTTARGET_NONE)
 {
  g = group;
  trgt = target;
 }
 if (BTTARGET_PARTY == g)
 {
  if (BTTARGET_INDIVIDUAL == trgt)
  {
   for (int i = 0; i < party.size(); ++i)
   {
    party[i]->rateAttacks -= bonus;
   }
  }
  else
  {
   party[trgt]->rateAttacks -= bonus;
  }
  d.drawStats();
 }
 else
 {
  // Doesn't work on monsters
 }
}

BTRegenManaEffect::BTRegenManaEffect(int t, int x, int s, int m, int g, int trgt, const BTDice &sp)
 : BTTargetedEffect(t, x, s, m, g, trgt), mana(sp)
{
}

void BTRegenManaEffect::serialize(ObjectSerializer *s)
{
 s->add("mana", &mana);
 BTTargetedEffect::serialize(s);
}

int BTRegenManaEffect::maintain(BTDisplay &d, BTCombat *combat)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 if (BTTARGET_PARTY == group)
 {
  if (BTTARGET_INDIVIDUAL == target)
  {
   for (int i = 0; i < party.size(); ++i)
   {
    if (party[i]->isAlive())
    {
     party[i]->giveSP(mana.roll());
    }
   }
  }
  else
  {
   if (party[target]->isAlive())
    party[target]->giveSP(mana.roll());
  }
  d.drawStats();
 }
 return 0;
}

BTSaveBonusEffect::BTSaveBonusEffect(int t, int x, int s, int m, int g, int trgt, int b)
 : BTTargetedEffect(t, x, s, m, g, trgt), bonus(b)
{
}

void BTSaveBonusEffect::serialize(ObjectSerializer *s)
{
 s->add("bonus", &bonus);
 BTTargetedEffect::serialize(s);
}

int BTSaveBonusEffect::apply(BTDisplay &d, BTCombat *combat, int g /*= BTTARGET_NONE*/, int trgt /*= BTTARGET_INDIVIDUAL*/)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 if (g == BTTARGET_NONE)
 {
  g = group;
  trgt = target;
 }
 if (BTTARGET_PARTY == g)
 {
  if (BTTARGET_INDIVIDUAL == trgt)
  {
   for (int i = 0; i < party.size(); ++i)
   {
    party[i]->save += bonus;
   }
  }
  else
  {
   party[trgt]->save += bonus;
  }
  d.drawStats();
 }
 else
 {
  // Doesn't work on monsters
 }
 return 0;
}

void BTSaveBonusEffect::finish(BTDisplay &d, BTCombat *combat, int g /*= BTTARGET_NONE*/, int trgt /*= BTTARGET_INDIVIDUAL*/)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 if (g == BTTARGET_NONE)
 {
  g = group;
  trgt = target;
 }
 if (BTTARGET_PARTY == g)
 {
  if (BTTARGET_INDIVIDUAL == trgt)
  {
   for (int i = 0; i < party.size(); ++i)
   {
    party[i]->save -= bonus;
   }
  }
  else
  {
   party[trgt]->save -= bonus;
  }
  d.drawStats();
 }
 else
 {
  // Doesn't work on monsters
 }
}

BTScrySightEffect::BTScrySightEffect(int t, int x, int s, int m)
 : BTBaseEffect(t, x, s, m)
{
}

int BTScrySightEffect::maintain(BTDisplay &d, BTCombat *combat)
{
 d.drawView();
 d.drawMap(true);
}

BTSpellBindEffect::BTSpellBindEffect(int t, int x, int s, int m, int g, int trgt)
 : BTResistedEffect(t, x, s, m, g, trgt)
{
}

int BTSpellBindEffect::apply(BTDisplay &d, BTCombat *combat, int g /*= BTTARGET_NONE*/, int trgt /*= BTTARGET_INDIVIDUAL*/)
{
 if (!targetsMonsters())
  return 0;
 if (target == BTTARGET_ALLMONSTERS)
  return 0;
 if (g == BTTARGET_NONE)
 {
  g = group;
  trgt = target;
 }
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 BTMonsterGroup *grp = combat->getMonsterGroup(g - BTTARGET_MONSTER);
 BTFactory<BTMonster> &monsterList = game->getMonsterList();
 if (checkResists(combat, g, trgt))
 {
  std::string text = monsterList[grp->monsterType].getName();
  text += " resists!";
  d.drawMessage(text.c_str(), game->getDelay());
  throw BTAllResistException();
 }
 else
 {
  if (trgt == BTTARGET_INDIVIDUAL)
  {
  }
  else
  {
   if (party.size() >= BT_PARTYSIZE)
   {
    std::string text = "No room in your party. ";
    text += monsterList[grp->monsterType].getName();
    text += " cannot join!";
    d.drawMessage(text.c_str(), game->getDelay());
   }
   else
   {
    BTPc *pc = new BTPc(grp->monsterType, BTJOB_MONSTER, grp->at(trgt));
    party.add(d, pc);
    d.drawStats();
    grp->at(trgt)->status.set(BTSTATUS_DEAD);
    return 1;
   }
  }
 }
 return 0;
}

void BTSpellBindEffect::finish(BTDisplay &d, BTCombat *combat, int g /*= BTTARGET_NONE*/, int trgt /*= BTTARGET_INDIVIDUAL*/)
{
}

BTLightEffect::BTLightEffect(int t, int x, int s, int m, int g, int trgt, int illum)
 : BTTargetedEffect(t, x, s, m, g, trgt), illumination(illum)
{
}

void BTLightEffect::serialize(ObjectSerializer *s)
{
 s->add("illumination", &illumination);
 BTTargetedEffect::serialize(s);
}

