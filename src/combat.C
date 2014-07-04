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

static const char *blank = "";

void BTMonsterCombatant::deactivate(int &activeNum)
{
 if (initiative != BTINITIATIVE_INACTIVE)
 {
  initiative = BTINITIATIVE_INACTIVE;
  group->active--;
  --activeNum;
 }
}

int BTMonsterCombatant::getGender() const
{
 BTFactory<BTMonster> &monList = BTGame::getGame()->getMonsterList();
 return monList[group->monsterType].getGender();
}

std::string BTMonsterCombatant::getName() const
{
 BTFactory<BTMonster> &monList = BTGame::getGame()->getMonsterList();
 return monList[group->monsterType].getName();
}

bool BTMonsterCombatant::isIllusion() const
{
 BTFactory<BTMonster> &monList = BTGame::getGame()->getMonsterList();
 return monList[group->monsterType].isIllusion();
}

bool BTMonsterCombatant::savingThrow(int difficulty /*= BTSAVE_DIFFICULTY*/) const
{
 BTFactory<BTMonster> &monList = BTGame::getGame()->getMonsterList();
 return monList[group->monsterType].savingThrow(BTSAVE_DIFFICULTY);
}

BTMonsterGroup::~BTMonsterGroup()
{
}

int BTMonsterGroup::findTarget(int ind /*= BTTARGET_INDIVIDUAL*/)
{
 if ((ind != BTTARGET_INDIVIDUAL) && (individual[ind].hp >= 0))
  return ind;
 int alive = 0;
 std::vector<BTMonsterCombatant>::iterator monster(individual.begin());
 for (; monster != individual.end(); ++monster)
 {
  if (monster->isAlive())
  {
   ++alive;
  }
 }
 if (alive)
 {
  alive = BTDice(1, alive).roll();
  for (monster = individual.begin(); monster != individual.end(); ++monster)
  {
   if (monster->isAlive())
   {
    if (0 == --alive)
     return monster - individual.begin();
   }
  }
 }
 return BTTARGET_INDIVIDUAL;
}

const std::string &BTMonsterGroup::getName() const
{
 BTFactory<BTMonster> &monList = BTGame::getGame()->getMonsterList();
 if (1 == individual.size())
 {
  return monList[monsterType].getName();
 }
 else
 {
  return monList[monsterType].getPluralName();
 }
}

void BTMonsterGroup::push(int d)
{
 distance += d;
 if (distance <= 0)
  distance = 1;
 else if (distance > 9)
  distance = 9;
}

void BTMonsterGroup::setMonsterType(int type, int number /*= 0*/)
{
 BTFactory<BTMonster> &monList = BTGame::getGame()->getMonsterList();
 monsterType = type;
 distance = BTDice(1, monList[monsterType].getStartDistance()).roll();
 if (number == 0)
  number = BTDice(1, monList[monsterType].getMaxAppearing()).roll();
 BTJobList &jobList = BTGame::getGame()->getJobList();
 int toHit = jobList[((monList[type].isIllusion() == 1) ? BTJOB_ILLUSION : BTJOB_MONSTER)]->toHit;
 if (monList[type].getLevel() > 0)
  toHit += (monList[type].getLevel() - 1) / jobList[((monList[type].isIllusion() == 1) ? BTJOB_ILLUSION : BTJOB_MONSTER)]->improveToHit;
 while (number > 0)
 {
  individual.push_back(BTMonsterCombatant(this, monList[type].getLevel(), ((monList[type].isIllusion() == 1) ? BTJOB_ILLUSION : BTJOB_MONSTER), monList[monsterType].getAc(), toHit, monList[monsterType].getHp().roll()));
  --number;
 }
 active = individual.size();
}

BTCombatant* BTMonsterGroup::at(size_t index)
{
 return &individual[index];
}

int BTMonsterGroup::getDistance()
{
 return distance;
}

size_t BTMonsterGroup::size()
{
 return individual.size();
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
 actionList["hide"] = &hide;
 actionList["partyAttack"] = &partyAttack;
 actionList["runAway"] = &runAway;
 actionList["sing"] = &sing;
 actionList["target"] = &target;
 actionList["useItem"] = &useItem;
}

BTCombat::~BTCombat()
{
 free(monsterNames);
 free(treasureLabel);
 free(partyLabel);
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

bool BTCombat::findNextInitiative(int &group, int &individual)
{
 BTParty &party = BTGame::getGame()->getParty();
 int curInitiative = BTINITIATIVE_INACTIVE + 1;
 group = BTTARGET_NONE;
 individual = -1;

 int monGroup = BTTARGET_MONSTER;
 for (std::list<BTMonsterGroup>::iterator itr = monsters.begin(); itr != monsters.end(); ++itr, ++monGroup)
 {
  int monNumber = 0;
  for (std::vector<BTMonsterCombatant>::iterator monster(itr->individual.begin()); monster != itr->individual.end(); ++monster, ++monNumber)
  {
   if (curInitiative <= monster->initiative)
   {
    group = monGroup;
    individual = monNumber;
    curInitiative = monster->initiative;
   }
  }
 }
 for (int i = 0; i < party.size(); i++)
 {
  if (curInitiative <= party[i]->initiative)
  {
   group = BTTARGET_PARTY;
   individual = i;
   curInitiative = party[i]->initiative;
  }
 }
 return (group != BTTARGET_NONE);
}

int BTCombat::findScreen(int num)
{
 static int lastScreen(0);
 int i;
 if ((optionState) && ((num == BTCOMBATSCREEN_OPTION) || (num == BTCOMBATSCREEN_COMBAT)))
 {
  int closest = 9;
  std::list<BTMonsterGroup>::iterator itr(monsters.begin());
  for (; itr != monsters.end(); ++itr)
  {
   if (itr->distance < closest)
    closest = itr->distance;
  }
  BTParty &party = BTGame::getGame()->getParty();
  BTPc* current = getPc();
  if (current == NULL)
  {
   optionState = false;
   num = BTCOMBATSCREEN_COMBAT;
   for (i = 0; i < party.size(); ++i)
   {
    if (!party[i]->status.isSet(BTSTATUS_NPC))
    {
     setPc(party[i]);
     int range = party[i]->hiddenTime();
     bool hidden = true;
     if (0 == range)
     {
      hidden = false;
      range = 1;
     }
     canAttack = ((closest <= range) && (!monsters.empty()) && ((hidden) || (i < BT_BACK)));
     optionState = true;
     break;
    }
    else
     party[i]->combat.action = BTPc::BTPcAction::npc;
   }
  }
  else
  {
   for (i = 0; i < party.size(); ++i)
    if (party[i] == current)
     break;
   if ((num == BTCOMBATSCREEN_OPTION) && (lastScreen == num))
   {
    for (int prev = i - 1; prev >= 0; --prev)
    {
     if (!party[prev]->status.isSet(BTSTATUS_NPC))
     {
      setPc(party[prev]);
      i = prev;
      break;
     }
    }
    int range = party[i]->hiddenTime();
    bool hidden = true;
    if (0 == range)
    {
     hidden = false;
     range = 1;
    }
    canAttack = ((closest <= range) && (!monsters.empty()) && ((hidden) || (i < BT_BACK)));
   }
   else if (num == BTCOMBATSCREEN_COMBAT)
   {
    optionState = false;
    setPc(NULL);
    for (++i; i < party.size(); ++i)
    {
     if (!party[i]->status.isSet(BTSTATUS_NPC))
     {
      if (party[i]->isAlive())
      {
       optionState = true;
       setPc(party[i]);
       int range = party[i]->hiddenTime();
       bool hidden = true;
       if (0 == range)
       {
        hidden = false;
        range = 1;
       }
       canAttack = ((closest <= range) && (!monsters.empty()) && ((hidden) || (i < BT_BACK)));
      }
      break;
     }
     else
      party[i]->combat.action = BTPc::BTPcAction::npc;
    }
   }
  }
 }
 if ((optionState) && (num == BTCOMBATSCREEN_COMBAT))
 {
  lastScreen = BTCOMBATSCREEN_OPTION;
  return BTScreenSet::findScreen(BTCOMBATSCREEN_OPTION);
 }
 else
 {
  lastScreen = num;
  return BTScreenSet::findScreen(num);
 }
}

bool BTCombat::findTarget(BTPc &pc, int range, BTMonsterGroup *&grp, int &target)
{
 int group = pc.combat.getTargetGroup() - BTTARGET_MONSTER;
 if (group < 0)
  pc.combat.setTarget(BTTARGET_MONSTER + (group = BTDice(1, monsters.size(), -1).roll()));
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

bool BTCombat::findTargetPC(int range, int &target, int ignore /*= BT_PARTYSIZE*/)
{
 BTParty &party = BTGame::getGame()->getParty();
 int alive = 0;
 for (target = 0; (target < party.size()) && (target < range); ++target)
 {
  if ((party[target]->isAlive()) && (target != ignore) && (party[target]->hiddenTime() == 0))
  {
   ++alive;
  }
 }
 if (!alive)
  return false;
 alive = BTDice(1, alive).roll();
 for (target = 0; target < party.size(); ++target)
 {
  if ((target != ignore) && (party[target]->hiddenTime() == 0))
  {
   if ((alive == 0) && (party[target]->isAlive()))
    return true;
   else if ((party[target]->isAlive()) && (--alive == 0))
   {
    return true;
   }
  }
 }
 return false;
}

BTMonsterGroup *BTCombat::getMonsterGroup(int group)
{
 std::list<BTMonsterGroup>::iterator itr(monsters.begin());
 for (; itr != monsters.end(); ++itr, --group)
 {
  if (0 == group)
   return &(*itr);
 }
 return NULL;
}

void BTCombat::initScreen(BTDisplay &d)
{
 add("advance", &canAdvance);
 add("attack", &canAttack);
 add("gold", &gold);
 add("monsters", &monsterNames);
 add("xp", &xp);
 if (getPc())
 {
  char tmp[10];
  int hiddenTime = getPc()->hiddenTime();
  if (hiddenTime > 0)
  {
   if (hiddenTime > 9)
    hiddenTime = 9;
   snprintf(tmp, 10, "%d0'", hiddenTime);
   std::string *distance = new std::string(tmp);
   add("hidden", distance, NULL, true);
  }
 }

 BTFactory<BTMonster> &monList = BTGame::getGame()->getMonsterList();
 BTMonsterGroup *first = NULL;
 monsterNames[0] = 0;
 int len = monsters.size();
 canAdvance = (len > 0);
 int i = 1;
 unsigned int range = 1;
 XMLAction* rangeXML = find("range", NULL);
 if ((rangeXML) && (rangeXML->getType() == XMLTYPE_UINT))
 {
  range = *(reinterpret_cast<unsigned int*>(rangeXML->object));
 }
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
  add("monsterName", new std::string(itr->getName()), attrib, true);
  attrib = new std::vector<XMLAttribute>;
  attrib->push_back(XMLAttribute("number", tmp));
  add("distance", &itr->distance, attrib);
  attrib = new std::vector<XMLAttribute>;
  attrib->push_back(XMLAttribute("number", tmp));
  add("inRange", new bool(range >= itr->distance), attrib, true);
  if (itr->distance == 1)
   canAdvance = false;
  if (itr != monsters.begin())
  {
   if (i == len)
    strcat(monsterNames, " and ");
   else
    strcat(monsterNames, ", ");
  }
  sprintf(monsterNames + strlen(monsterNames), "%d %s (%d0')", itr->individual.size(), itr->getName().c_str(), itr->distance);
 }
 if (first)
 {
  setPicture(d, monList[first->monsterType].getPicture(), const_cast<char *>(monList[first->monsterType].getName().c_str()));
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
 previousPic = d.getCurrentImage();
 previousLabel = d.getCurrentLabel();
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
  BTGame *game = BTGame::getGame();
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
  d.process(BTDisplay::allKeys, game->getDelay());
 }
 d.clearText();
 clearEffects(d);
 checkExpiration(d, this);
 clearEncounters();
 if (previousPic != -1)
  d.drawImage(previousPic);
 else
  d.drawView();
 d.drawLabel(previousLabel.c_str());
}

void BTCombat::runCombat(BTDisplay &d)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = BTGame::getGame()->getParty();
 int active = 0;
 int i = 0;
 std::list<BTMonsterGroup>::iterator itr(monsters.begin());
 for (; itr != monsters.end(); ++itr)
 {
  for (i = 0; i < itr->individual.size(); ++i)
   itr->individual[i].rollInitiative();
  active += itr->active;
 }
 for (i = 0; i < party.size(); ++i)
  if (party[i]->initiative != BTINITIATIVE_INACTIVE)
  {
   party[i]->rollInitiative();
   ++active;
  }

 int curGroup;
 int curIndividual;
 while (findNextInitiative(curGroup, curIndividual))
 {
  if (curGroup == BTTARGET_PARTY)
  {
   runPcAction(d, active, curIndividual, *party[curIndividual]);
   if (BTPc::BTPcAction::useSkill != party[curIndividual]->combat.action)
    party[curIndividual]->combat.clearSkillUsed();
  }
  else
  {
   int monGroup = BTTARGET_MONSTER;
   for (itr = monsters.begin(); itr != monsters.end(); ++itr, ++monGroup)
   {
    if (monGroup == curGroup)
    {
     runMonsterAction(d, active, monGroup, curIndividual, *itr, itr->individual[curIndividual]);
     break;
    }
   }
  }
 }
 game->nextTurn(d, this);
 if (endRound(d))
  won = true;
 d.drawStats(); // In case check dead move people around
 d.drawIcons();
}

void BTCombat::runMonsterAction(BTDisplay &d, int &active, int monGroup, int monNumber, BTMonsterGroup &grp, BTCombatant &mon)
{
 BTGame *game = BTGame::getGame();
 BTFactory<BTMonster> &monList = game->getMonsterList();
 BTFactory<BTSpell, BTSpell1> &spellList = game->getSpellList();
 BTParty &party = game->getParty();
 mon.initiative = BTINITIATIVE_INACTIVE;
 --active;
 if (mon.status.isSet(BTSTATUS_PARALYZED))
  return;
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
    if ((BTRANGEDTYPE_MAGIC == rangedType) && ((grp.distance > (spellList[monList[grp.monsterType].getRangedSpell()].getRange() * (1 + spellList[monList[grp.monsterType].getRangedSpell()].getEffectiveRange()))) || (BTDice(1, 2).roll() == 1)))
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
 if ((mon.status.isSet(BTSTATUS_INSANE)) && (BTCOMBATACTION_MOVEANDATTACK == action) && (grp.distance > 1))
 {
  // Let the monster move.
 }
 else if ((mon.status.isSet(BTSTATUS_INSANE)) || (mon.status.isSet(BTSTATUS_POSSESSED)))
 {
  action = BTCOMBATACTION_ATTACK;
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
   d.process(BTDisplay::allKeys, game->getDelay());
   d.clearElements();
   for (std::vector<BTMonsterCombatant>::iterator monster(grp.individual.begin()); monster != grp.individual.end(); ++monster)
   {
    if (monster->initiative != BTINITIATIVE_INACTIVE)
    {
     monster->initiative = BTINITIATIVE_INACTIVE;
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
  BTCombatant *defender = NULL;
  int target;
  for (int attacks = monList[grp.monsterType].getRateAttacks(); attacks > 0; )
  {
   int opponents = 0;
   bool attackParty = false;
   if ((mon.status.isSet(BTSTATUS_INSANE)) || (mon.status.isSet(BTSTATUS_POSSESSED)))
   {
    std::list<BTMonsterGroup>::iterator itr(monsters.begin());
    for (; itr != monsters.end(); ++itr)
     if (itr->distance == grp.distance)
      ++opponents;
   }
   if ((!mon.status.isSet(BTSTATUS_POSSESSED)) && (grp.distance <= 1))
    attackParty = true;
   if ((0 == opponents) && (!attackParty))
    break;
   opponents = BTDice(1, opponents + (attackParty ? 1 : 0), (attackParty ? 0 : 1)).roll();
   if (opponents == BTTARGET_PARTY)
   {
    if (findTargetPC(BT_BACK, target))
    {
     defender = party[target];
     std::string text = mon.attack(defender, true, monList[grp.monsterType].getMeleeMessage(), "and hits", monList[grp.monsterType].getMeleeDamage(), 100, monList[grp.monsterType].getMeleeExtra(), attacks, active);
     d.drawStats();
     d.drawMessage(text.c_str(), game->getDelay());
    }
    else
     break;
   }
   else
   {
    --opponents;
    std::list<BTMonsterGroup>::iterator itr(monsters.begin());
    for (; itr != monsters.end(); ++itr)
    {
     if ((itr->distance == grp.distance) && (--opponents == 0))
      break;
    }
    target = itr->findTarget(BTTARGET_INDIVIDUAL);
    if (target != BTTARGET_INDIVIDUAL)
    {
     defender = itr->at(target);
     std::string text = mon.attack(defender, true, monList[grp.monsterType].getMeleeMessage(), "and hits", monList[grp.monsterType].getMeleeDamage(), 100, monList[grp.monsterType].getMeleeExtra(), attacks, active);
     d.drawStats();
     d.drawMessage(text.c_str(), game->getDelay());
    }
    else
     break;
   }
  }
 }
 else if (BTCOMBATACTION_SPECIALATTACK == action)
 {
  switch (monList[grp.monsterType].getRangedType())
  {
   case BTRANGEDTYPE_MAGIC:
   {
    int target = BTTARGET_INDIVIDUAL;
    if (spellList[monList[grp.monsterType].getRangedSpell()].getArea() == BTAREAEFFECT_FOE)
     if (!findTargetPC(BT_PARTYSIZE, target))
      break;
    active -= spellList[monList[grp.monsterType].getRangedSpell()].cast(d, monList[grp.monsterType].getName().c_str(), monGroup, monNumber, true, this, monList[grp.monsterType].getLevel(), grp.distance, BTTARGET_PARTY, target);
    break;
   }
   case BTRANGEDTYPE_FOE:
   {
    int target = BTTARGET_INDIVIDUAL;
    if (!findTargetPC(BT_PARTYSIZE, target))
     break;
    int attacks = 1;
    std::string text = mon.attack(party[target], false, monList[grp.monsterType].getRangedMessage(), "and hits", monList[grp.monsterType].getRangedDamage(), 100, monList[grp.monsterType].getRangedExtra(), attacks, active);
    d.addText(text.c_str());
    d.addText(blank);
    d.process(BTDisplay::allKeys, game->getDelay());
    d.clearElements();
    break;
   }
   case BTRANGEDTYPE_GROUP:
   {
    monList[grp.monsterType].useRangedOnGroup(d, &party, abs(grp.getDistance() - party.getDistance()), active);
    break;
   }
   default:
   {
    std::string text = monList[grp.monsterType].getName();
    text += " has an unknown ranged attack";
    d.addText(text.c_str());
    d.addText(blank);
    d.process(BTDisplay::allKeys, game->getDelay());
    d.clearElements();
    break;
   }
  }
 }
}

void BTCombat::runPcAction(BTDisplay &d, int &active, int pcNumber, BTPc &pc)
{
 BTGame *game = BTGame::getGame();
 BTFactory<BTItem> &itemList = game->getItemList();
 BTSkillList &skillList = game->getSkillList();
 BTFactory<BTMonster> &monList = game->getMonsterList();
 BTParty &party = game->getParty();
 XMLVector<BTSong*> &songList = game->getSongList();
 BTFactory<BTSpell, BTSpell1> &spellList = game->getSpellList();
 std::string text;
 pc.initiative = BTINITIATIVE_INACTIVE;
 --active;
 if (BTPc::BTPcAction::runAway == pc.combat.action)
 {
  int encounterLvl = monList[monsters.begin()->monsterType].getLevel();
  for (int i = 0; i < party.size(); ++i)
   party[i]->initiative = BTINITIATIVE_INACTIVE;
  for (int s = 0; s < skillList.size(); ++s)
  {
   if (skillList[s]->special == BTSKILLSPECIAL_RUN)
   {
    if (pc.useSkill(s, skillList[s]->defaultDifficulty + (encounterLvl / 2) + (monList[monsters.begin()->monsterType].getMove() / 2)))
    {
     throw BTCombatError("runAway");
    }
    else
    {
     text += skillList[s]->common;
     text += " ";
     text += skillList[s]->failure;
     d.drawMessage(text.c_str(), game->getDelay());
    }
   }
  }
 }
 else if (BTPc::BTPcAction::advance == pc.combat.action)
 {
  bool advance = true;
  std::list<BTMonsterGroup>::iterator itr(monsters.begin());
  for (; itr != monsters.end(); ++itr)
  {
   if (itr->distance == 1)
    advance = false;
  }
  if (advance)
  {
   for (itr = monsters.begin(); itr != monsters.end(); ++itr)
   {
    --itr->distance;
   }
   for (int i = 0; i < party.size(); ++i)
    party[i]->initiative = BTINITIATIVE_INACTIVE;
   d.addText("The party advances...");
   d.addText(blank);
   d.process(BTDisplay::allKeys, game->getDelay());
   d.clearElements();
  }
 }
 else
 {
  if (pc.status.isSet(BTSTATUS_PARALYZED))
  {
   return;
  }
  if ((pc.status.isSet(BTSTATUS_INSANE)) || (pc.status.isSet(BTSTATUS_POSSESSED)))
  {
   int group;
   int target;
   int opponents = 0;
   std::list<BTMonsterGroup>::iterator itr(monsters.begin());
   for (; itr != monsters.end(); ++itr)
    if (itr->distance == 1)
     ++opponents;
   int alive = 0;
   int who = 0;
   for (target = 0; target < party.size(); ++target)
   {
    if (party[target]->isAlive())
    {
     if (party[target] == &pc)
      who = target;
     ++alive;
    }
   }
   if ((pc.status.isSet(BTSTATUS_POSSESSED)) || (0 == opponents) || (who <= BT_BACK))
    group = BTTARGET_PARTY;
   else
   {
    opponents = BTDice(1, opponents + ((alive > 1) ? 1 : 0), ((alive > 1) ? 0 : 1)).roll();
    if (opponents == BTTARGET_PARTY)
     group = opponents;
    else
    {
     --opponents;
     group = BTTARGET_MONSTER;
     for (itr = monsters.begin(); itr != monsters.end(); ++itr, ++group)
     {
      if ((itr->distance == 1) && (--opponents == 0))
       break;
     }
    }
   }
   target = BTTARGET_INDIVIDUAL;
   if (BTTARGET_PARTY == group)
   {
    if (!findTargetPC(BT_PARTYSIZE, target, who))
     target = who;
    pc.combat.action = BTPc::BTPcAction::partyAttack;
    pc.combat.setTarget(group, target);
   }
   else
   {
    pc.combat.action = BTPc::BTPcAction::attack;
    pc.combat.setTarget(group);
   }
  }
  if (BTPc::BTPcAction::npc == pc.combat.action)
  {
   pc.combat.action = BTPc::BTPcAction::defend;
   for (int i = 0; i < party.size(); ++i)
   {
    if (party[i] == &pc)
    {
     if ((i < BT_BACK) && (0 < monsters.size()))
     {
      pc.combat.action = BTPc::BTPcAction::attack;
      pc.combat.clearTarget(pc.combat.getTargetGroup());
     }
     break;
    }
   }
  }
  switch (pc.combat.action)
  {
   case BTPc::BTPcAction::attack:
   case BTPc::BTPcAction::partyAttack:
   {
    int handWeapon = pc.getHandWeapon();
    for (int attacks = pc.rateAttacks; attacks > 0; )
    {
     BTCombatant *defender = NULL;
     BTMonsterGroup *grp = NULL;
     int target = BTTARGET_INDIVIDUAL;
     if (BTPc::BTPcAction::attack == pc.combat.action)
     {
      int range = 1;
      if ((pc.combat.skillUsed != -1) && (skillList[pc.combat.skillUsed]->special == BTSKILLSPECIAL_HIDE))
       range = pc.combat.consecutiveUsed;
      if (!findTarget(pc, range, grp, target))
       return;
      defender = &grp->individual[target];
     }
     else
     {
      target = pc.combat.getTargetIndividual();
      if (!party[target]->isAlive())
       return;
      if (party[target]->status.isSet(BTSTATUS_NPC))
       party[target]->status.set(BTSTATUS_INSANE);
      defender = party[target];
     }
     text = pc.attack(defender, handWeapon, attacks, active);
     if (!grp)
      d.drawStats();
     d.drawMessage(text.c_str(), game->getDelay());
    }
    break;
   }
   case BTPc::BTPcAction::defend:
    break;
   case BTPc::BTPcAction::cast:
   {
    BTMonsterGroup *grp = NULL;
    int target = pc.combat.getTargetIndividual();
    if ((spellList[pc.combat.object].getArea() == BTAREAEFFECT_FOE) && (target == BTTARGET_INDIVIDUAL))
    {
     if (!findTarget(pc, BTDISTANCE_MAX, grp, target))
      return;
    }
    pc.sp -= spellList[pc.combat.object].getSp();
    active -= spellList[pc.combat.object].cast(d, pc.name, BTTARGET_PARTY, pcNumber, true, this, pc.level, 0, pc.combat.getTargetGroup(), target);
    break;
   }
   case BTPc::BTPcAction::sing:
   {
    text = pc.name;
    text += " ";
    bool novoice = true;
    for (int i = 0; i < skillList.size(); ++i)
    {
     if ((skillList[i]->special == BTSKILLSPECIAL_SONG) && (pc.hasSkillUse(i)))
     {
      bool instrument(false);
      for (int k = 0; k < BT_ITEMS; ++k)
      {
       if ((pc.item[k].equipped == BTITEM_EQUIPPED) && (itemList[pc.item[k].id].getType() == BTITEM_INSTRUMENT))
       {
        instrument = true;
        break;
       }
      }
      if (!instrument)
      {
       text += "has no equipped instrument!";
       d.drawMessage(text.c_str(), game->getDelay());
      }
      else if (game->getFlags().isSet(BTSPECIALFLAG_SILENCE))
      {
       text += "can't play a tune! No sound can be heard!";
       d.drawMessage(text.c_str(), game->getDelay());
      }
      else
      {
       clearEffectsBySource(d, true);
       checkExpiration(d, NULL);
       pc.giveSkillUse(i, -1);
       songList[pc.combat.object]->play(d, &pc, this);
      }
      novoice = false;
      break;
     }
    }
    if (novoice)
    {
     text += "lost his voice!";
     d.drawMessage(text.c_str(), game->getDelay());
    }
    break;
   }
   case BTPc::BTPcAction::useItem:
   {
    if (BTITEM_NONE != pc.item[pc.combat.object].id)
    {
     if ((BTITEM_ARROW == itemList[pc.item[pc.combat.object].id].getType()) || (BTITEM_THROWNWEAPON == itemList[pc.item[pc.combat.object].id].getType()))
     {
      int target = pc.combat.getTargetIndividual();
      int numAttacks = 1;
      BTCombatant *defender = NULL;
      if (BTTARGET_PARTY == pc.combat.getTargetGroup())
      {
       defender = party[target];
      }
      else
      {
       BTMonsterGroup *grp = getMonsterGroup(pc.combat.getTargetGroup() - BTTARGET_MONSTER);
       if (target == BTTARGET_INDIVIDUAL)
       {
        if (!findTarget(pc, BTDISTANCE_MAX, grp, target))
         return;
       }
       defender = &(grp->individual[target]);
      }
      std::string text = pc.attack(defender, pc.item[pc.combat.object].id, numAttacks, active);
      d.drawMessage(text.c_str(), BTGame::getGame()->getDelay());
     }
     else
     {
      int spellCast = itemList[pc.item[pc.combat.object].id].getSpellCast();
      BTMonsterGroup *grp = NULL;
      int target = pc.combat.getTargetIndividual();
      if ((spellList[spellCast].getArea() == BTAREAEFFECT_FOE) && (target == BTTARGET_INDIVIDUAL))
      {
       if (!findTarget(pc, BTDISTANCE_MAX, grp, target))
        return;
      }
      active -= spellList[spellCast].cast(d, pc.name, BTTARGET_PARTY, pcNumber, true, this, pc.level, 0, pc.combat.getTargetGroup(), target);
     }
     pc.takeItemCharge(pc.combat.object);
    }
    break;
   }
   case BTPc::BTPcAction::useSkill:
    if (!skillList[pc.combat.object]->common.empty())
    {
     text = pc.name;
     text += " ";
     text += skillList[pc.combat.object]->common;
    }
    if (pc.useSkill(pc.combat.object))
    {
     if (!text.empty())
     {
      text += " ";
      text += skillList[pc.combat.object]->success;
     }
     pc.combat.setSkillUsed(pc.combat.object);
    }
    else
    {
     if (!text.empty())
     {
      text += " ";
      text += skillList[pc.combat.object]->failure;
     }
     pc.combat.clearSkillUsed();
    }
    if (!text.empty())
     d.drawMessage(text.c_str(), game->getDelay());
    break;
   case BTPc::BTPcAction::runAway:
   case BTPc::BTPcAction::advance:
   default:
    break;
  }
 }
}

bool BTCombat::endRound(BTDisplay &d)
{
 BTGame *game = BTGame::getGame();
 ++round;
 BTFactory<BTMonster> &monList = game->getMonsterList();
 BTFactory<BTSpell, BTSpell1> &spellList = game->getSpellList();
 int group;
 checkExpiration(d, this);
 maintain(d, this);
 d.drawIcons();
 XMLVector<BTBaseEffect*>::iterator itrEffect = effect.begin();
 group = BTTARGET_MONSTER;
 int alive = 0;
 for (std::list<BTMonsterGroup>::iterator itr(monsters.begin()); itr != monsters.end();)
 {
  itr->canMove = true;
  itr->active = 0;
  bool hasIndvidualSpell = true;
  for (std::vector<BTMonsterCombatant>::iterator monster(itr->individual.begin()); monster != itr->individual.end();)
  {
   if (monster->isAlive())
   {
    if (monster->status.isSet(BTSTATUS_POISONED))
    {
     if (monster->takeHP(1))
     {
      std::string text;
      text = monList[itr->monsterType].getName();
      text += " dies!";
      d.drawMessage(text.c_str(), game->getDelay());
     }
    }
   }
   if (!monster->isAlive())
   {
    xp += monList[itr->monsterType].getXp();
    gold += monList[itr->monsterType].getGold().roll();
    for (itrEffect = effect.begin(); itrEffect != effect.end();)
    {
     if ((*itrEffect)->targets(group, monster - itr->individual.begin()))
     {
      BTBaseEffect *current = *itrEffect;
      itrEffect = effect.erase(itrEffect, false);
      int size = effect.size();
      if ((BTTIME_PERMANENT != current->expiration) && (BTTIME_CONTINUOUS != current->expiration))
       current->finish(d, this);
      delete current;
      if (size != effect.size())
       itrEffect = effect.begin();
     }
     else
      ++itrEffect;
    }
    for (itrEffect = effect.begin(); itrEffect != effect.end(); ++itrEffect)
    {
     (*itrEffect)->remove(this, group, monster - itr->individual.begin());
    }
    monster = itr->individual.erase(monster);
   }
   else
   {
    monster->initiative = BTINITIATIVE_ACTIVE;
    ++(itr->active);
    ++monster;
   }
  }
  if (0 == itr->individual.size())
  {
   for (itrEffect = effect.begin(); itrEffect != effect.end();)
   {
    if ((*itrEffect)->targets(group, BTTARGET_INDIVIDUAL))
    {
     BTBaseEffect *current = *itrEffect;
     itrEffect = effect.erase(itrEffect, false);
     int size = effect.size();
     if ((BTTIME_PERMANENT != current->expiration) && (BTTIME_CONTINUOUS != current->expiration))
      current->finish(d, this);
     delete current;
     if (size != effect.size())
      itrEffect = effect.begin();
    }
    else
     ++itrEffect;
   }
   for (itrEffect = effect.begin(); itrEffect != effect.end(); ++itrEffect)
   {
    (*itrEffect)->remove(this, group, BTTARGET_INDIVIDUAL);
   }
   itr = monsters.erase(itr);
  }
  else
  {
   alive += itr->active;
   ++itr;
   ++group;
  }
 }
 BTParty &party = game->getParty();
 if (party.checkDead(d))
  throw BTSpecialDead();
 if ((monsters.empty()) && (xp > 0) && (!won))
 {
  clearEffects(d);
  alive = 0;
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
   party[i]->combat.clearSkillUsed();
   if (party[i]->isAlive())
   {
    party[i]->giveXP(xp);
    party[i]->giveGold(gold);
   }
  }
  return true;
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
 BTGame *game = BTGame::getGame();
 BTSkillList &skillList = game->getSkillList();
 BTCombat &c = static_cast<BTCombat&>(b);
 b.getPc()->combat.action = BTPc::BTPcAction::attack;
 int target = 0;
 int i = 0;
 int range = 1;
 if ((b.getPc()->combat.skillUsed != -1) && (skillList[b.getPc()->combat.skillUsed]->special == BTSKILLSPECIAL_HIDE))
  range = b.getPc()->combat.consecutiveUsed;
 for (std::list<BTMonsterGroup>::iterator itr(c.monsters.begin()); itr != c.monsters.end(); ++itr, ++i)
 {
  if (range >= itr->distance)
  {
   if (0 == target)
    target = BTTARGET_MONSTER + i;
   else
   {
    if ((b.getPc()->combat.skillUsed != -1) && (skillList[b.getPc()->combat.skillUsed]->special == BTSKILLSPECIAL_HIDE))
     b.add("range", &b.getPc()->combat.consecutiveUsed, NULL);
    return BTCOMBATSCREEN_SELECTATTACK;
   }
  }
 }
 b.getPc()->combat.setTarget(target);
 return 0;
}

int BTCombat::cast(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTCombat &c = static_cast<BTCombat&>(b);
 BTReadString *readString = static_cast<BTReadString*>(item);
 BTFactory<BTSpell, BTSpell1> &spellList = BTGame::getGame()->getSpellList();
 std::string spellCode = readString->getResponse();
 for (int i = 0; i < spellList.size(); ++i)
 {
  if (0 == strcasecmp(spellCode.c_str(), spellList[i].getCode()))
  {
   if (b.getPc()->getSkill(spellList[i].getCaster()) >= spellList[i].getLevel())
   {
    if (b.getPc()->sp < spellList[i].getSp())
     throw BTSpecialError("nosp");
    b.getPc()->combat.action = BTPc::BTPcAction::cast;
    b.getPc()->combat.object = i;
    b.getPc()->combat.type = BTPc::BTPcAction::spell;
    switch (spellList[i].getArea())
    {
     case BTAREAEFFECT_FOE:
      b.add("range", new unsigned int(spellList[i].getRange() * (1 + spellList[i].getEffectiveRange())), NULL, true);
      return BTCOMBATSCREEN_TARGETSINGLE;
     case BTAREAEFFECT_GROUP:
      if (c.monsters.empty())
      {
       b.getPc()->combat.setTarget(BTTARGET_PARTY);
       return 0;
      }
      else
      {
       b.add("range", new unsigned int(spellList[i].getRange() * (1 + spellList[i].getEffectiveRange())), NULL, true);
       return BTCOMBATSCREEN_TARGETGROUP;
      }
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

int BTCombat::hide(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 BTSkillList &skillList = game->getSkillList();
 for (int i = 0; i < skillList.size(); ++i)
 {
  if ((skillList[i]->special == BTSKILLSPECIAL_HIDE) && (b.getPc()->hasSkillUse(i)))
  {
   b.getPc()->combat.action = BTPc::BTPcAction::useSkill;
   b.getPc()->combat.type = BTPc::BTPcAction::skill;
   b.getPc()->combat.object = i;
   return 0;
  }
 }
 throw BTSpecialError("nohide");
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

int BTCombat::sing(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTGame *game = BTGame::getGame();
 BTFactory<BTItem> &itemList = game->getItemList();
 BTSkillList &skillList = game->getSkillList();
 BTFactory<BTSpell, BTSpell1> &spellList = game->getSpellList();
 BTCombat &c = static_cast<BTCombat&>(b);
 BTSelectSong *select = static_cast<BTSelectSong*>(item);
 for (int i = 0; i < skillList.size(); ++i)
 {
  if ((skillList[i]->special == BTSKILLSPECIAL_SONG) && (b.getPc()->hasSkillUse(i)))
  {
   bool instrument(false);
   for (int k = 0; k < BT_ITEMS; ++k)
   {
    if ((b.getPc()->item[k].equipped == BTITEM_EQUIPPED) && (itemList[b.getPc()->item[k].id].getType() == BTITEM_INSTRUMENT))
    {
     instrument = true;
     break;
    }
   }
   if (!instrument)
    throw BTSpecialError("noinstrument");
   b.getPc()->combat.action = BTPc::BTPcAction::sing;
   b.getPc()->combat.object = select->select;
   b.getPc()->combat.type = BTPc::BTPcAction::song;
   return 0;
  }
 }
 throw BTSpecialError("novoice");
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
 BTCombat &c = static_cast<BTCombat&>(b);
 BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
 BTFactory<BTSpell, BTSpell1> &spellList = BTGame::getGame()->getSpellList();
 BTSelectInventory *select = static_cast<BTSelectInventory*>(item);
 if ((select->select == -1) || (b.getPc()->item[select->select].id == BTITEM_NONE))
 {
  return 2; // Should never get here.
 }
 if (BTITEM_ARROW == itemList[b.getPc()->item[select->select].id].getType())
 {
  if (BTITEM_CANNOTEQUIP == b.getPc()->item[select->select].equipped)
   throw BTSpecialError("notbyyou");
  // Determine if you have a bow equipped.
  bool found = false;
  for (int i = 0; i < BT_ITEMS; ++i)
  {
   if (BTITEM_NONE == b.getPc()->item[i].id)
    break;
   if ((BTITEM_EQUIPPED == b.getPc()->item[i].equipped) && (BTITEM_BOW == itemList[b.getPc()->item[i].id].getType()))
   {
    found = true;
    break;
   }
  }
  if (!found)
   throw BTSpecialError("nobow");
  b.add("range", new unsigned int(BTDISTANCE_MAX), NULL, true);
 }
 else if (BTITEM_THROWNWEAPON == itemList[b.getPc()->item[select->select].id].getType())
 {
  // Allow even if not equipped
  if (BTITEM_CANNOTEQUIP == b.getPc()->item[select->select].equipped)
   throw BTSpecialError("notbyyou");
  b.add("range", new unsigned int(BTDISTANCE_MAX), NULL, true);
 }
 else if (BTITEM_EQUIPPED != b.getPc()->item[select->select].equipped)
  throw BTSpecialError("notequipped");
 else if (BTITEM_BOW == itemList[b.getPc()->item[select->select].id].getType())
  throw BTSpecialError("notarrow");
 else
 {
  int spellCast = itemList[b.getPc()->item[select->select].id].getSpellCast();
  if ((!b.getPc()->item[b.getPc()->combat.object].charges == 0) || (spellCast == BTITEMCAST_NONE))
   throw BTSpecialError("notusable");
  b.getPc()->combat.action = BTPc::BTPcAction::useItem;
  b.getPc()->combat.type = BTPc::BTPcAction::item;
  b.getPc()->combat.object = select->select;
  switch (spellList[spellCast].getArea())
  {
   case BTAREAEFFECT_FOE:
    b.add("range", new unsigned int(spellList[spellCast].getRange() * (1 + spellList[spellCast].getEffectiveRange())), NULL, true);
    return BTCOMBATSCREEN_TARGETSINGLE;
   case BTAREAEFFECT_GROUP:
    if (c.monsters.empty())
    {
     b.getPc()->combat.setTarget(BTTARGET_PARTY);
     return 0;
    }
    else
    {
     b.add("range", new unsigned int(spellList[spellCast].getRange() * (1 + spellList[spellCast].getEffectiveRange())), NULL, true);
     return BTCOMBATSCREEN_TARGETGROUP;
    }
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
 b.getPc()->combat.action = BTPc::BTPcAction::useItem;
 b.getPc()->combat.type = BTPc::BTPcAction::item;
 b.getPc()->combat.object = select->select;
 return BTCOMBATSCREEN_TARGETSINGLE;
}

