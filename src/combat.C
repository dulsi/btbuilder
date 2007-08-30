/*-------------------------------------------------------------------------*\
  <combat.C> -- Combat implementation file

  Date      Programmer  Description
  05/02/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "combat.h"
#include "game.h"
#include "pc.h"
#include <sstream>

#define BTCOMBATSCREEN_OPTION 2
#define BTCOMBATSCREEN_COMBAT 3
#define BTCOMBATSCREEN_WON 4
#define BTCOMBATSCREEN_PARTYATTACK 5
#define BTCOMBATSCREEN_SELECTATTACK 7
#define BTCOMBATSCREEN_TARGETSINGLE 10
#define BTCOMBATSCREEN_TARGETGROUP 11

static char *blank = "";

BTMonsterGroup::~BTMonsterGroup()
{
 if (monsterName)
  delete[] monsterName;
}

int BTMonsterGroup::findTarget(int ind /*= BTTARGET_INDIVIDUAL*/)
{
 if ((ind != BTTARGET_INDIVIDUAL) && (individual[ind].hp >= 0))
  return ind;
 int alive = 0;
 std::vector<BTMonsterInstance>::iterator monster(individual.begin());
 for (; monster != individual.end(); ++monster)
 {
  if (monster->hp >= 0)
  {
   ++alive;
  }
 }
 if (alive)
 {
  alive = BTDice(1, alive).roll();
  for (monster = individual.begin(); monster != individual.end(); ++monster)
  {
   if (monster->hp >= 0)
   {
    if (0 == --alive)
     return monster - individual.begin();
   }
  }
 }
 return BTTARGET_INDIVIDUAL;
}

void BTMonsterGroup::setMonsterType(int type, int number /*= 0*/)
{
 BTFactory<BTMonster> &monList = BTGame::getGame()->getMonsterList();
 monsterType = type;
 distance = BTDice(1, monList[monsterType].getStartDistance()).roll();
 if (number == 0)
  number = BTDice(1, monList[monsterType].getMaxAppearing()).roll();
 while (number > 0)
 {
  individual.push_back(BTMonsterInstance(monList[monsterType].getHp().roll()));
  --number;
 }
 active = individual.size();
 if (1 == active)
 {
  monsterName = new char[strlen(monList[monsterType].getName()) + 1];
  strcpy(monsterName, monList[monsterType].getName());
 }
 else
 {
  monsterName = new char[strlen(monList[monsterType].getName()) + 4];
  strcpy(monsterName, monList[monsterType].getName());
  strcat(monsterName, "(s)");
 }
}

void BTCombatScreen::draw(BTDisplay &d, ObjectSerializer *obj)
{
 static_cast<BTCombat *>(obj)->runCombat(d);
}

int BTCombatScreen::getEscapeScreen()
{
 if (BTGame::getGame()->getCombat().isWinner())
  return BTCOMBATSCREEN_WON;
 else
  return BTScreenSetScreen::getEscapeScreen();
}

XMLObject *BTCombatScreen::create(const XML_Char *name, const XML_Char **atts)
{
 int number = 0;
 int escapeScreen = 0;
 int timeout = 0;
 for (const char **att = atts; *att; att += 2)
 {
  if (0 == strcmp(*att, "number"))
   number = atoi(att[1]);
  else if (0 == strcmp(*att, "escapeScreen"))
  {
   if (0 == strcmp(att[1], "exit"))
    escapeScreen = BTSCREEN_EXIT;
   else
    escapeScreen = atoi(att[1]);
  }
  else if (0 == strcmp(*att, "timeout"))
   timeout = atoi(att[1]);
 }
 return new BTCombatScreen(number, escapeScreen, timeout);
}

BTCombat::BTCombat()
 : won(false), optionState(false), canAdvance(false), canAttack(false)
{
 partyLabel = strdup("PARTY");
 treasurePic = 28;
 treasureLabel = strdup("Treasure!");
 monsterNames = (char *)malloc(400);
 monsterNames[0] = 0;

 actionList["advance"] = &advance;
 actionList["attack"] = &attack;
 actionList["cast"] = &cast;
 actionList["combatOption"] = &combatOption;
 actionList["defend"] = &defend;
 actionList["partyAttack"] = &partyAttack;
 actionList["runAway"] = &runAway;
 actionList["target"] = &target;
 actionList["useItem"] = &useItem;
}

BTCombat::~BTCombat()
{
 free(monsterNames);
 free(treasureLabel);
 free(partyLabel);
}

void BTCombat::addEffect(int spell, unsigned int expire, int group, int target)
{
 spellEffect.push_back(BTSpellEffect(spell, expire, group, target));
}

void BTCombat::addEncounter(int monsterType, int number /*= 0*/)
{
 BTFactory<BTMonster> &monList = BTGame::getGame()->getMonsterList();
 monsters.push_back(BTMonsterGroup());
 BTMonsterGroup &group = monsters.back();
 group.setMonsterType(monsterType, number);
}

void BTCombat::clearEncounters()
{
 monsters.clear();
}

int BTCombat::findScreen(int num)
{
 int i;
 if ((optionState) && ((num == BTCOMBATSCREEN_OPTION) || (num == BTCOMBATSCREEN_COMBAT)))
 {
  BTParty &party = BTGame::getGame()->getParty();
  BTPc* current = getPc();
  if (current == NULL)
  {
   setPc(party[0]);
   canAttack = ((!canAdvance) && (0 < monsters.size()));
  }
  else
  {
   for (i = 0; i < party.size(); ++i)
    if (party[i] == current)
     break;
   if ((num == BTCOMBATSCREEN_OPTION) && (i != 0))
   {
    setPc(party[i - 1]);
    canAttack = ((!canAdvance) && (i - 1 < BT_BACK) && (!monsters.empty()));
   }
   else if (num == BTCOMBATSCREEN_COMBAT)
   {
    if ((i == party.size() - 1) || (!party[i + 1]->isAlive()))
    {
     setPc(NULL);
     optionState = false;
    }
    else
    {
     setPc(party[i + 1]);
     canAttack = ((!canAdvance) && (i + 1 < BT_BACK) && (!monsters.empty()));
    }
   }
  }
 }
 if ((optionState) && (num == BTCOMBATSCREEN_COMBAT))
  return BTScreenSet::findScreen(BTCOMBATSCREEN_OPTION);
 else
  return BTScreenSet::findScreen(num);
}

bool BTCombat::findTarget(BTPc &pc, int range, BTMonsterGroup *&grp, int &target)
{
 int group = pc.combat.getTargetGroup() - BTTARGET_MONSTER;
 std::list<BTMonsterGroup>::iterator itr(monsters.begin());
 for (; itr != monsters.end(); ++itr, --group)
 {
  if (0 == group)
   break;
 }
 if (itr->distance <= range)
 {
  target = itr->findTarget(pc.combat.getTargetIndividual());
  if (BTTARGET_INDIVIDUAL != target)
  {
   grp = &(*itr);
   return true;
  }
 }
 group = pc.combat.getTargetGroup() - BTTARGET_MONSTER;
 for (itr = monsters.begin(); itr != monsters.end(); ++itr, --group)
 {
  if ((0 != group) && (itr->distance <= range))
  {
   target = itr->findTarget(pc.combat.getTargetIndividual());
   if (BTTARGET_INDIVIDUAL != target)
   {
    grp = &(*itr);
    return false;
   }
  }
 }
 return false;
}


void BTCombat::initScreen(BTDisplay &d)
{
 add("advance", &canAdvance);
 add("attack", &canAttack);
 add("gold", &gold);
 add("monsters", &monsterNames);
 add("xp", &xp);

 BTFactory<BTMonster> &monList = BTGame::getGame()->getMonsterList();
 BTMonsterGroup *first = NULL;
 monsterNames[0] = 0;
 int len = monsters.size();
 canAdvance = (len > 0);
 int i = 1;
 for (std::list<BTMonsterGroup>::iterator itr(monsters.begin()); itr != monsters.end(); ++itr, ++i)
 {
  if ((NULL == first) || (first->distance > itr->distance))
   first = &(*itr);
  std::vector<XMLAttribute> *attrib = new std::vector<XMLAttribute>;
  char tmp[10];
  snprintf(tmp, 10, "%d", i);
  attrib->push_back(XMLAttribute("number", tmp));
  add("monsterNumber", &itr->active, attrib);
  attrib = new std::vector<XMLAttribute>;
  attrib->push_back(XMLAttribute("number", tmp));
  add("monsterName", &itr->monsterName, attrib);
  attrib = new std::vector<XMLAttribute>;
  attrib->push_back(XMLAttribute("number", tmp));
  add("distance", &itr->distance, attrib);
  if (itr->distance == 1)
   canAdvance = false;
  if (itr != monsters.begin())
  {
   if (i == len)
    strcat(monsterNames, " and ");
   else
    strcat(monsterNames, ", ");
  }
  sprintf(monsterNames + strlen(monsterNames), "%d %s(s) (%d0')", itr->individual.size(), monList[itr->monsterType].getName(), itr->distance);
 }
 if (first)
 {
  setPicture(d, monList[first->monsterType].getPicture(), const_cast<char *>(monList[first->monsterType].getName()));
 }
 else if (won)
 {
  setPicture(d, treasurePic, treasureLabel);
 }
 else
 {
  setPicture(d, BTGame::getGame()->getParty()[0]->picture, partyLabel);
//  optionState = true;
 }
}

void BTCombat::movedPlayer(BTDisplay &d, int who, int where)
{
 BTFactory<BTSpell> &spellList = BTGame::getGame()->getSpellList();
 for (std::list<BTSpellEffect>::iterator itr = spellEffect.begin(); itr != spellEffect.end();)
 {
  if ((BTTARGET_PARTY == itr->group) && (who == itr->target))
  {
   if (where == BTPARTY_REMOVE)
   {
    spellList[itr->spell].finish(d, this, itr->group, itr->target);
    itr = spellEffect.erase(itr);
    continue;
   }
   else
   {
    itr->target = where;
   }
  }
  else if ((BTTARGET_PARTY == itr->group) && (who < where) && (where >= itr->target) && (who < itr->target))
  {
   itr->target--;
  }
  else if ((BTTARGET_PARTY == itr->group) && (who > where) && (where <= itr->target) && (who > itr->target))
  {
   itr->target++;
  }
  ++itr;
 }
}

void BTCombat::open(const char *filename)
{
 XMLSerializer parser;
 parser.add("picture", &picture);
 parser.add("label", &label);
 parser.add("screen", &screen, &BTScreenSetScreen::create);
 parser.add("combat", &screen, &BTCombatScreen::create);
 parser.add("error", &errors, &BTError::create);
 parser.parse(filename, true);
}

void BTCombat::run(BTDisplay &d, bool partyAttack /*= false*/)
{
 setPc(NULL);
 optionState = false;
 won = false;
 round = 0;
 xp = gold = 0;
 if ((monsters.empty()) && (!partyAttack))
  return;
 initScreen(d);
 try
 {
  BTScreenSet::run(d, (partyAttack ? findScreen(BTCOMBATSCREEN_PARTYATTACK) : 0), false);
 }
 catch (const BTCombatError &e)
 {
  BTError *err = NULL;
  for (int i = 0; i < errors.size(); ++i)
  {
   if (0 == strcmp(errors[i]->getType().c_str(), e.error.c_str()))
   {
    err = errors[i];
    break;
   }
  }
  if (err)
   err->draw(d, this);
  else
   d.addText(("Unknown Error: " + e.error).c_str());
  d.process(BTDisplay::allKeys, 1000);
 }
 catch (const BTSpecialFlipGoForward &e)
 {
 }
 d.clearElements();
 clearEncounters();
}

void BTCombat::runCombat(BTDisplay &d)
{
 BTParty &party = BTGame::getGame()->getParty();
 int active = 0;
 int i = 0;
 std::list<BTMonsterGroup>::iterator itr(monsters.begin());
 for (; itr != monsters.end(); ++itr)
 {
  active += itr->active;
 }
 if (BTPc::BTPcAction::advance == party[i]->combat.action)
 {
  for (itr = monsters.begin(); itr != monsters.end(); ++itr)
  {
   --itr->distance;
  }
  for (i = 0; i < party.size(); ++i)
   party[i]->combat.active = false;
  d.addText("The party advances...");
  d.addText(blank);
  d.process(BTDisplay::allKeys, 1000);
  d.clearElements();
 }
 else if (BTPc::BTPcAction::runAway == party[i]->combat.action)
 {
  for (i = 0; i < party.size(); ++i)
   party[i]->combat.active = false;
  throw BTCombatError("runAway");
 }
 else
 {
  for (i = 0; i < party.size(); ++i)
   if (party[i]->combat.active)
    ++active;
 }

 while (active > 0)
 {
  int who = IRandom(active);
  for (itr = monsters.begin(); itr != monsters.end(); ++itr)
  {
   if (who < itr->active)
   {
    --itr->active;
    for (std::vector<BTMonsterInstance>::iterator monster(itr->individual.begin()); monster != itr->individual.end(); ++monster)
    {
     if (monster->active)
     {
      --who;
      if (-1 == who)
      {
       runMonsterAction(d, active, *itr, *monster);
       break;
      }
     }
    }
    break;
   }
   else
    who -= itr->active;
  }
  if (who > -1)
  {
   for (int i = 0; i < party.size(); i++)
   {
    if (party[i]->combat.active)
    {
     --who;
     if (-1 == who)
     {
      runPcAction(d, active, *party[i]);
     }
    }
   }
  }
 }
 BTGame::getGame()->nextTurn(d, this);
 if (endRound(d))
  won = true;
 d.drawStats(); // In case check dead move people around
}

void BTCombat::runMonsterAction(BTDisplay &d, int &active, BTMonsterGroup &grp, BTMonsterInstance &mon)
{
 BTFactory<BTMonster> &monList = BTGame::getGame()->getMonsterList();
 mon.active = false;
 --active;
 int action = monList[grp.monsterType].getCombatAction(round);
 if (BTCOMBATACTION_RANDOM == action)
 {
  if (grp.distance > 1)
  {
   // Move or special attack if available
   int rangedType = monList[grp.monsterType].getRangedType();
   if (BTRANGEDTYPE_NONE == rangedType)
    action = BTCOMBATACTION_MOVEANDATTACK;
   else if (grp.canMove)
   {
    if (BTRANGEDTYPE_MAGIC == rangedType) // WRONG check range of spell
     action = BTCOMBATACTION_MOVEANDATTACK;
    else if (((BTRANGEDTYPE_FOE == rangedType) || (BTRANGEDTYPE_GROUP == rangedType)) && ((grp.distance > monList[grp.monsterType].getRange()) || (BTDice(1, 2).roll() == 1)))
     action = BTCOMBATACTION_MOVEANDATTACK;
    else
    {
     action = BTCOMBATACTION_SPECIALATTACK;
     grp.canMove = false;
    }
   }
   else
    action = BTCOMBATACTION_SPECIALATTACK;
  }
  else
  {
   // Attack or special attack if available
   if ((BTRANGEDTYPE_NONE != monList[grp.monsterType].getRangedType()) && (BTDice(1, 2).roll() == 1))
    action = BTCOMBATACTION_SPECIALATTACK;
   else
    action = BTCOMBATACTION_ATTACK;
  }
 }
 if (BTCOMBATACTION_MOVEANDATTACK == action)
 {
  if (grp.distance > 1)
  {
   grp.distance -= monList[grp.monsterType].getMove();
   if (grp.distance <= 1)
   {
    grp.distance = 1;
    d.addText("Your foes descend upon you!");
   }
   else
    d.addText("Your foes advance!");
   d.addText(blank);
   d.process(BTDisplay::allKeys, 1000);
   d.clearElements();
   for (std::vector<BTMonsterInstance>::iterator monster(grp.individual.begin()); monster != grp.individual.end(); ++monster)
   {
    if (monster->active)
    {
     monster->active = false;
     --active;
    }
   }
  }
  else
   action = BTCOMBATACTION_ATTACK;
 }
 else if (BTCOMBATACTION_DEPENDENTATTACK == action)
 {
  if (grp.distance > 1)
   action = BTCOMBATACTION_SPECIALATTACK;
  else
   action = BTCOMBATACTION_ATTACK;
 }
 if (BTCOMBATACTION_ATTACK == action)
 {
  std::string text = monList[grp.monsterType].getName();
  text += " is attacking";
  d.addText(text.c_str());
  d.addText(blank);
  d.process(BTDisplay::allKeys, 1000);
  d.clearElements();
 }
 else if (BTCOMBATACTION_SPECIALATTACK == action)
 {
  std::string text = monList[grp.monsterType].getName();
  text += " is special attacking";
  d.addText(text.c_str());
  d.addText(blank);
  d.process(BTDisplay::allKeys, 1000);
  d.clearElements();
 }
}

void BTCombat::runPcAction(BTDisplay &d, int &active, BTPc &pc)
{
 BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
 BTFactory<BTMonster> &monList = BTGame::getGame()->getMonsterList();
 BTParty &party = BTGame::getGame()->getParty();
 BTFactory<BTSpell> &spellList = BTGame::getGame()->getSpellList();
 std::string text;
 pc.combat.active = false;
 --active;
 switch (pc.combat.action)
 {
  case BTPc::BTPcAction::attack:
  case BTPc::BTPcAction::partyAttack:
  {
   int ac;
   int handWeapon = pc.getHandWeapon();
   for (int attacks = 0; attacks < pc.rateAttacks; )
   {
    BTMonsterGroup *grp = NULL;
    int target = BTTARGET_INDIVIDUAL;
    if (BTPc::BTPcAction::attack == pc.combat.action)
    {
     findTarget(pc, 1, grp, target);
     if (NULL == grp)
      return;
     ac = monList[grp->monsterType].getAc();
    }
    else
    {
     target = pc.combat.getTargetIndividual();
     if (!party[target]->isAlive())
      return;
     ac = party[target]->ac;
    }
    text = pc.name;
    text += " ";
    if (-1 == handWeapon)
    {
     text += "punches at";
    }
    else
    {
     BTItem &item = itemList[handWeapon];
     text += item.getCause();
    }
    text += " ";
    if (BTPc::BTPcAction::attack == pc.combat.action)
     text += monList[grp->monsterType].getName();
    else
     text += party[target]->name;
    ++attacks;
    int roll = BTDice(1, 20).roll();
    if ((1 != roll) && ((20 == roll) || (roll + pc.toHit >= ac)))
    {
     text += " ";
     int damage = 0;
     if (-1 == handWeapon)
     {
      text += "and strikes for";
      damage = BTDice(1, 2).roll();
     }
     else
     {
      BTItem &item = itemList[handWeapon];
      text += item.getEffect();
      damage = item.getDamage().roll();
     }
     text += " ";
     if (pc.stat[BTSTAT_ST] > 14)
      damage += pc.stat[BTSTAT_ST] - 14;
     char tmp[20];
     sprintf(tmp, "%d", damage);
     text += tmp;
     text += " points of damage";
     if (BTPc::BTPcAction::attack == pc.combat.action)
     {
      if ((grp->individual[target].hp -= damage) < 0)
      {
       text += ", killing him!";
       if (grp->individual[target].active)
       {
        grp->individual[target].active = false;
        grp->active--;
        --active;
       }
      }
      else
       text += ".";
     }
     else
     {
      if (party[target]->takeHP(damage))
       text += ", killing him!";
      else
       text += ".";
      d.drawStats();
     }
    }
    else
     text += ", but misses!";
    d.addText(text.c_str());
    d.addText(blank);
    d.process(BTDisplay::allKeys, 1000);
    d.clearElements();
   }
   break;
  }
  case BTPc::BTPcAction::defend:
   break;
  case BTPc::BTPcAction::cast:
   pc.sp -= spellList[pc.combat.object].getSp();
   spellList[pc.combat.object].cast(d, pc.name, this, pc.combat.getTargetGroup(),  pc.combat.getTargetIndividual());
   break;
  case BTPc::BTPcAction::useItem:
  case BTPc::BTPcAction::skill:
   // TODO: Implement
   break;
  case BTPc::BTPcAction::runAway:
  case BTPc::BTPcAction::advance:
  default:
   break;
 }
}

bool BTCombat::endRound(BTDisplay &d)
{
 BTGame *game = BTGame::getGame();
 ++round;
 BTFactory<BTMonster> &monList = game->getMonsterList();
 BTFactory<BTSpell> &spellList = game->getSpellList();
 int group = BTTARGET_MONSTER;
 std::list<BTSpellEffect>::iterator effect = spellEffect.begin();
 for (; effect != spellEffect.end(); ++effect)
 {
  if (!game->isExpired(effect->expiration))
  {
   if (effect->first)
    effect->first = false;
   else if (BTTIME_PERMANENT != effect->expiration)
    spellList[effect->spell].maintain(d, this, effect->group, effect->target);
  }
 }
 for (std::list<BTMonsterGroup>::iterator itr(monsters.begin()); itr != monsters.end();)
 {
  itr->canMove = true;
  itr->active = 0;
  bool hasIndvidualSpell = true;
  for (std::vector<BTMonsterInstance>::iterator monster(itr->individual.begin()); monster != itr->individual.end();)
  {
   if (monster->hp < 0)
   {
    xp += monList[itr->monsterType].getXp();
    gold += monList[itr->monsterType].getGold().roll();
    for (std::list<BTSpellEffect>::iterator effect = spellEffect.begin(); effect != spellEffect.end();)
    {
     if ((group == effect->group) && (effect->target == monster - itr->individual.begin()))
     {
      if ((BTTIME_PERMANENT != effect->expiration) && (BTTIME_CONTINUOUS != effect->expiration))
       spellList[effect->spell].finish(d, this, effect->group, effect->target);
      effect = spellEffect.erase(effect);
     }
     else if ((group == effect->group) && (effect->target > monster - itr->individual.begin()))
     {
      effect->target--;
     }
    }
    monster = itr->individual.erase(monster);
   }
   else
   {
    monster->active = true;
    ++(itr->active);
    ++monster;
   }
  }
  if (0 == itr->individual.size())
  {
   for (std::list<BTSpellEffect>::iterator effect = spellEffect.begin(); effect != spellEffect.end();)
   {
    if (group == effect->group)
    {
     if ((BTTIME_PERMANENT != effect->expiration) && (BTTIME_CONTINUOUS != effect->expiration))
      spellList[effect->spell].finish(d, this, effect->group, effect->target);
     effect = spellEffect.erase(effect);
    }
    else if (group < effect->group)
    {
     effect->group--;
    }
   }
   itr = monsters.erase(itr);
  }
  else
  {
   ++itr;
   ++group;
  }
 }
 BTParty &party = game->getParty();
 if (party.checkDead(d))
  throw BTSpecialError("dead");
 if ((monsters.empty()) && (xp > 0) && (!won))
 {
  for (effect = spellEffect.begin(); effect != spellEffect.end();)
  {
   if ((BTTIME_PERMANENT != effect->expiration) && (BTTIME_CONTINUOUS != effect->expiration))
    spellList[effect->spell].finish(d, this, effect->group, effect->target);
   effect = spellEffect.erase(effect);
  }
  int alive = 0;
  int i;
  for (i = 0; i < party.size(); ++i)
  {
   if (party[i]->isAlive())
    ++alive;
  }
  xp = xp / alive;
  gold = gold / alive;
  for (i = 0; i < party.size(); ++i)
  {
   if (party[i]->isAlive())
   {
    party[i]->giveXP(xp);
    party[i]->giveGold(gold);
   }
  }
  return true;
 }
 for (effect = spellEffect.begin(); effect != spellEffect.end();)
 {
  if (!game->isExpired(effect->expiration))
  {
   if ((BTTIME_PERMANENT != effect->expiration) && (BTTIME_CONTINUOUS != effect->expiration))
    spellList[effect->spell].finish(d, this, effect->group, effect->target);
   effect = spellEffect.erase(effect);
  }
  else
   ++effect;
 }
 return false;
}

int BTCombat::advance(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTParty &party = BTGame::getGame()->getParty();
 for (int i = 0; i < party.size(); ++i)
  party[i]->combat.action = BTPc::BTPcAction::advance;
 return 0;
}

int BTCombat::attack(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTCombat &c = static_cast<BTCombat&>(b);
 b.getPc()->combat.action = BTPc::BTPcAction::attack;
 int target = 0;
 int i = 0;
 for (std::list<BTMonsterGroup>::iterator itr(c.monsters.begin()); itr != c.monsters.end(); ++itr, ++i)
 {
  if (1 == itr->distance)
  {
   if (0 == target)
    target = BTTARGET_MONSTER + i;
   else
    return BTCOMBATSCREEN_SELECTATTACK;
  }
 }
 b.getPc()->combat.setTarget(target);
 return 0;
}

int BTCombat::cast(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTCombat &c = static_cast<BTCombat&>(b);
 BTReadString *readString = static_cast<BTReadString*>(item);
 BTFactory<BTSpell> &spellList = BTGame::getGame()->getSpellList();
 std::string spellCode = readString->getResponse();
 for (int i = 0; i < spellList.size(); ++i)
 {
  if (0 == strcasecmp(spellCode.c_str(), spellList[i].getCode()))
  {
   if (b.getPc()->skill[spellList[i].getCaster()] >= spellList[i].getLevel())
   {
    if (b.getPc()->sp < spellList[i].getSp())
     throw BTSpecialError("nosp");
    b.getPc()->combat.action = BTPc::BTPcAction::cast;
    b.getPc()->combat.object = i;
    b.getPc()->combat.type = BTPc::BTPcAction::spell;
    switch (spellList[i].getArea())
    {
     case BTAREAEFFECT_FOE:
      return BTCOMBATSCREEN_TARGETSINGLE;
     case BTAREAEFFECT_GROUP:
      if (c.monsters.empty())
      {
       b.getPc()->combat.setTarget(BTTARGET_PARTY);
       return 0;
      }
      else
       return BTCOMBATSCREEN_TARGETGROUP;
     case BTAREAEFFECT_NONE:
      b.getPc()->combat.clearTarget(b.getPc()->combat.getTargetGroup());
      return 0;
     case BTAREAEFFECT_ALL:
      b.getPc()->combat.setTarget(BTTARGET_ALLMONSTERS);
      return 0;
     default:
      return 0;
    }
   }
   else
    throw BTSpecialError("dontknow");
  }
 }
 throw BTSpecialError("nospell");
}

int BTCombat::combatOption(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 static_cast<BTCombat&>(b).optionState = true;
 return 0;
}

int BTCombat::defend(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 b.getPc()->combat.action = BTPc::BTPcAction::defend;
 return 0;
}

int BTCombat::partyAttack(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 b.getPc()->combat.action = BTPc::BTPcAction::partyAttack;
 return 0;
}

int BTCombat::runAway(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTParty &party = BTGame::getGame()->getParty();
 for (int i = 0; i < party.size(); ++i)
  party[i]->combat.action = BTPc::BTPcAction::runAway;
 return 0;
}

int BTCombat::target(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTCombat &c = static_cast<BTCombat&>(b);
 BTParty &party = BTGame::getGame()->getParty();
 if ((key >= '1') && (key <= '9') && (key - '1' < party.size()))
 {
  b.getPc()->combat.setTarget(BTTARGET_PARTY, key - '1');
 }
 else if (key == 'p')
 {
  b.getPc()->combat.setTarget(BTTARGET_PARTY);
 }
 else if ((key >= 'a') && (key <= 'd') && (key - 'a' < c.monsters.size()))
 {
  b.getPc()->combat.setTarget(BTTARGET_MONSTER + (key - 'a'));
 }
 return 0;
}

int BTCombat::useItem(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 b.getPc()->combat.action = BTPc::BTPcAction::useItem;
 b.getPc()->combat.type = BTPc::BTPcAction::item;
 return 0;
}
