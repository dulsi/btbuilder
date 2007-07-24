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

void BTCombat::run(BTDisplay &d, bool partyAttack /*= false*/)
{
 BTFactory<BTMonster> &monList = BTGame::getGame()->getMonsterList();
 won = false;
 round = 0;
 BTCombat::groupAction gAction;
 if (0 == monsters.size())
  return;
 while (true)
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
 }
}

BTCombat::groupAction BTCombat::fightOrRun(BTDisplay &d)
{
 bool canAdvance(true);
 BTFactory<BTMonster> &monList = BTGame::getGame()->getMonsterList();
 std::stringstream text;
 text << "You are facing ";
 int len = monsters.size();
 int i = 1;
 for (std::list<BTMonsterGroup>::iterator itr(monsters.begin()); itr != monsters.end(); ++itr, ++i)
 {
  if (itr->distance == 1)
   canAdvance = false;
  if (itr != monsters.begin())
  {
   if (i == len)
    text << " and ";
   else
    text << ", ";
  }
  text << itr->individual.size() << " " << monList[itr->monsterType].getName() << "(s) (" << itr->distance << "0')";
 }
 int key;
 do
 {
  d.clearText();
  d.addText(text.str().c_str());
  d.addText("");
  d.addText("Will the party");
  if (canAdvance)
  {
   d.addChoice("Aa", "Advance,");
  }
  d.addChoice("Ff", "Fight or");
  d.addChoice("Rr", "Run");
  key = d.process();
 }
 while (27 == key);
 switch (key)
 {
  case 'A':
  case 'a':
   return BTCombat::advance;
  case 'F':
  case 'f':
   return BTCombat::fight;
  case 'R':
  case 'r':
   return BTCombat::runAway;
 }
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

bool BTCombat::selectAction(BTDisplay &d, int pcNum)
{
 BTParty &party = BTGame::getGame()->getParty();
 std::stringstream text;
 text << "Select battle option for " << party[pcNum]->name;
 while (true)
 {
  d.clearText();
  d.addText(text.str().c_str());
  d.addText("");
  if (pcNum < 4)
  {
   for (std::list<BTMonsterGroup>::iterator itr(monsters.begin()); itr != monsters.end(); ++itr)
   {
    if (itr->distance == 1)
    {
     d.addChoice("Aa", "Attack foes");
     break;
    }
   }
  }
  d.addChoice("Pp", "Party attack");
  d.addChoice("Dd", "Defend");
  d.addChoice("Uu", "Use an item");
  int key = d.process();
  switch (key)
  {
   case 'A':
   case 'a':
    party[pcNum]->combat.action = BTPc::BTPcAction::attack;
    break;
   case 'P':
   case 'p':
    party[pcNum]->combat.action = BTPc::BTPcAction::partyAttack;
    break;
   case 'D':
   case 'd':
    party[pcNum]->combat.action = BTPc::BTPcAction::defend;
    break;
   case 'U':
   case 'u':
    party[pcNum]->combat.action = BTPc::BTPcAction::useItem;
    break;
   case 27:
    return false;
  }
 }
}
