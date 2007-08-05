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

static char *blank = "";

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
 monsterNames = (char *)malloc(400);
 monsterNames[0] = 0;

 actionList["advance"] = &advance;
 actionList["attack"] = &attack;
 actionList["combatOption"] = &combatOption;
 actionList["defend"] = &defend;
 actionList["partyAttack"] = &partyAttack;
 actionList["runAway"] = &runAway;
 actionList["target"] = &target;
 actionList["useItem"] = &useItem;
}

BTCombat::~BTCombat()
{
 free(partyLabel);
}

void BTCombat::addEncounter(int monsterType, int number /*= 0*/)
{
 BTFactory<BTMonster> &monList = BTGame::getGame()->getMonsterList();
 monsters.push_back(BTMonsterGroup());
 BTMonsterGroup &group = monsters.back();
 group.monsterType = monsterType;
 group.distance = BTDice(1, monList[monsterType].getStartDistance()).roll();
 if (number == 0)
  number = BTDice(1, monList[monsterType].getMaxAppearing()).roll();
 while (number > 0)
 {
  group.individual.push_back(BTMonsterInstance(monList[monsterType].getHp().roll()));
  --number;
 }
 group.active = group.individual.size();
 group.canMove = true;
}

void BTCombat::clearEncounters()
{
 monsters.clear();
}

void BTCombat::endScreen(BTDisplay &d)
{
 if (endRound())
  won = true;
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
    canAttack = ((!canAdvance) && (i - 1 < BT_BACK) && (0 < monsters.size()));
   }
   else if (num == BTCOMBATSCREEN_COMBAT)
   {
    if (i == party.size() - 1)
    {
     setPc(NULL);
     optionState = false;
    }
    else
    {
     setPc(party[i + 1]);
     canAttack = ((!canAdvance) && (i + 1 < BT_BACK) && (0 < monsters.size()));
    }
   }
  }
 }
 if ((optionState) && (num == BTCOMBATSCREEN_COMBAT))
  return BTScreenSet::findScreen(BTCOMBATSCREEN_OPTION);
 else
  return BTScreenSet::findScreen(num);
}

void BTCombat::initScreen(BTDisplay &d)
{
 add("advance", &canAdvance);
 add("attack", &canAttack);
 add("gold", &gold);
 add("monsters", &monsterNames);
 add("xp", &xp);

 BTFactory<BTMonster> &monList = BTGame::getGame()->getMonsterList();
 if (0 == monsters.size())
 {
  setPicture(d, BTGame::getGame()->getParty()[0]->picture, partyLabel);
//  optionState = true;
 }
 else
 {
  setPicture(d, monList[monsters.front().monsterType].getPicture(), const_cast<char *>(monList[monsters.front().monsterType].getName()));
 }
 monsterNames[0] = 0;
 int len = monsters.size();
 canAdvance = (len > 0);
 int i = 1;
 for (std::list<BTMonsterGroup>::iterator itr(monsters.begin()); itr != monsters.end(); ++itr, ++i)
 {
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
 if ((0 == monsters.size()) && (!partyAttack))
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
  std::string text;
  int who = IRandom(active);
  for (itr = monsters.begin(); itr != monsters.end(); ++itr)
  {
   if (who < itr->active)
   {
    // Do monster action
    --itr->active;
    for (std::vector<BTMonsterInstance>::iterator monster(itr->individual.begin()); monster != itr->individual.end(); ++monster)
    {
     if (monster->active)
     {
      --who;
      if (-1 == who)
      {
       runMonsterAI(d, active, *itr, *monster);
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
      party[i]->combat.active = false;
      --active;
      text = party[i]->name;
      text += " is ";
      switch (party[i]->combat.action)
      {
       case BTPc::BTPcAction::advance:
        text += "advancing";
        break;
       case BTPc::BTPcAction::attack:
        text += "attacking";
        break;
       case BTPc::BTPcAction::partyAttack:
        text += "attacking ";
        text += party[party[i]->combat.getTargetIndividual()]->name;
        break;
       case BTPc::BTPcAction::defend:
        text += "defending";
        break;
       case BTPc::BTPcAction::useItem:
        text += "using an item";
        break;
       case BTPc::BTPcAction::runAway:
        text += "running";
        break;
       default:
        break;
      }
     }
    }
   }
   d.addText(text.c_str());
   d.addText(blank);
   d.process(BTDisplay::allKeys, 1000);
   d.clearElements();
  }
 }
}

void BTCombat::runMonsterAI(BTDisplay &d, int &active, BTMonsterGroup &grp, BTMonsterInstance &mon)
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

bool BTCombat::endRound()
{
 ++round;
 BTFactory<BTMonster> &monList = BTGame::getGame()->getMonsterList();
 for (std::list<BTMonsterGroup>::iterator itr(monsters.begin()); itr != monsters.end();)
 {
  itr->canMove = true;
  itr->active = 0;
  for (std::vector<BTMonsterInstance>::iterator monster(itr->individual.begin()); monster != itr->individual.end();)
  {
   if (monster->hp < 0)
   {
    xp += monList[itr->monsterType].getXp();
    gold += monList[itr->monsterType].getGold().roll();
    itr->individual.erase(monster);
   }
   else
   {
    monster->active = true;
    ++(itr->active);
    ++monster;
   }
  }
  if (0 == itr->individual.size())
   monsters.erase(itr);
  else
   ++itr;
 }
 BTParty &party = BTGame::getGame()->getParty();
 if (party.checkDead())
  throw BTSpecialError("dead");
 if ((0 == monsters.size()) && (xp > 0))
 {
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
 return false;
}

void BTCombat::advance(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTParty &party = BTGame::getGame()->getParty();
 for (int i = 0; i < party.size(); ++i)
  party[i]->combat.action = BTPc::BTPcAction::advance;
}

void BTCombat::attack(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 b.getPc()->combat.action = BTPc::BTPcAction::attack;
}

void BTCombat::combatOption(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 static_cast<BTCombat&>(b).optionState = true;
}

void BTCombat::defend(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 b.getPc()->combat.action = BTPc::BTPcAction::defend;
}

void BTCombat::partyAttack(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 b.getPc()->combat.action = BTPc::BTPcAction::partyAttack;
}

void BTCombat::runAway(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTParty &party = BTGame::getGame()->getParty();
 for (int i = 0; i < party.size(); ++i)
  party[i]->combat.action = BTPc::BTPcAction::runAway;
}

void BTCombat::target(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTParty &party = BTGame::getGame()->getParty();
 if ((key >= '1') && (key <= '9') && (key - '1' < party.size()))
 {
  b.getPc()->combat.setTarget(BTTARGET_PARTY, key - '1');
 }
}

void BTCombat::useItem(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 b.getPc()->combat.action = BTPc::BTPcAction::useItem;
}
