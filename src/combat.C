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

static char *blank = "";

void BTCombatScreen::draw(BTDisplay &d, ObjectSerializer *obj)
{
 BTParty &party = BTGame::getGame()->getParty();
 for (int i = 0; i < party.size(); i++)
 {
  std::string text = party[i]->name;
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
    text += "attacking the party";
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
  d.addText(text.c_str());
  d.addText(blank);
  d.process(BTDisplay::allKeys, 1000);
  d.clearElements();
  if (party[i]->combat.action == BTPc::BTPcAction::runAway)
   throw BTCombatError("runAway");
 }
}

XMLObject *BTCombatScreen::create(const XML_Char *name, const XML_Char **atts)
{
 int number = 0;
 int escapeScreen = 0;
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
 }
 return new BTCombatScreen(number, escapeScreen);
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
}

void BTCombat::clearEncounters()
{
 monsters.clear();
}

void BTCombat::endScreen(BTDisplay &d)
{
 endRound();
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
   canAttack = (!canAdvance);
  }
  else
  {
   for (i = 0; i < party.size(); ++i)
    if (party[i] == current)
     break;
   if ((num == BTCOMBATSCREEN_OPTION) && (i != 0))
   {
    setPc(party[i - 1]);
    canAttack = ((!canAdvance) && (i - 1 < BT_BACK));
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
     canAttack = ((!canAdvance) && (i + 1 < BT_BACK));
    }
   }
  }
 }
 if (optionState)
  return BTScreenSet::findScreen(BTCOMBATSCREEN_OPTION);
 else
  return BTScreenSet::findScreen(num);
}

void BTCombat::initScreen(BTDisplay &d)
{
 add("advance", &canAdvance);
 add("attack", &canAttack);
 add("monsters", &monsterNames);

 BTFactory<BTMonster> &monList = BTGame::getGame()->getMonsterList();
 if (0 == monsters.size())
 {
  setPicture(d, BTGame::getGame()->getParty()[0]->picture, partyLabel);
  optionState = true;
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
 if ((0 == monsters.size()) && (!partyAttack))
  return;
 initScreen(d);
 try
 {
  BTScreenSet::run(d, ((0 == monsters.size()) ? findScreen(BTCOMBATSCREEN_OPTION) : 0), false);
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
 d.clearElements();
 clearEncounters();
/* while (true)
 {
  if (0 == monsters.size())
  {
   d.drawImage(BTGame::getGame()->getParty()[0]->picture);
   d.drawLabel("PARTY");
   gAction = BTCombat::fight;
  }
  else
  {
   d.drawImage(monList[monsters.front().monsterType].getPicture());
   d.drawLabel(monList[monsters.front().monsterType].getName());
   gAction = fightOrRun(d);
  }
  if (gAction == BTCombat::runAway)
  {
   clearEncounters();
   return;
  }
  if (endRound())
  {
   if (partyAttack)
   {
    d.addText("Do you wish to enter a round of intra party combat?");
    d.addChoice("Yy", "Yes, or");
    d.addChoice("Nn", "No");
    int key = d.process();
    if ((key == 'Y') || (key == 'y'))
     continue;
   }
   won = true;
   return;
  }
 }*/
}

bool BTCombat::endRound()
{
 for (std::list<BTMonsterGroup>::iterator itr(monsters.begin()); itr != monsters.end();)
 {
  for (std::vector<BTMonsterInstance>::iterator monster(itr->individual.begin()); monster != itr->individual.end();)
  {
   if (monster->hp < 0)
    itr->individual.erase(monster);
   else
   {
    monster->active = true;
    ++monster;
   }
  }
  if (0 == itr->individual.size())
   monsters.erase(itr);
  else
   ++itr;
 }
 if (0 == monsters.size())
  return true;
 return false;
}

void BTCombat::advance(BTScreenSet &b, BTDisplay &d, BTScreenItem *item)
{
 BTParty &party = BTGame::getGame()->getParty();
 for (int i = 0; i < party.size(); ++i)
  party[i]->combat.action = BTPc::BTPcAction::advance;
}

void BTCombat::attack(BTScreenSet &b, BTDisplay &d, BTScreenItem *item)
{
 b.getPc()->combat.action = BTPc::BTPcAction::attack;
}

void BTCombat::combatOption(BTScreenSet &b, BTDisplay &d, BTScreenItem *item)
{
 static_cast<BTCombat&>(b).optionState = true;
}

void BTCombat::defend(BTScreenSet &b, BTDisplay &d, BTScreenItem *item)
{
 b.getPc()->combat.action = BTPc::BTPcAction::defend;
}

void BTCombat::partyAttack(BTScreenSet &b, BTDisplay &d, BTScreenItem *item)
{
 b.getPc()->combat.action = BTPc::BTPcAction::partyAttack;
}

void BTCombat::runAway(BTScreenSet &b, BTDisplay &d, BTScreenItem *item)
{
 BTParty &party = BTGame::getGame()->getParty();
 for (int i = 0; i < party.size(); ++i)
  party[i]->combat.action = BTPc::BTPcAction::runAway;
}

void BTCombat::useItem(BTScreenSet &b, BTDisplay &d, BTScreenItem *item)
{
 b.getPc()->combat.action = BTPc::BTPcAction::useItem;
}
