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
 std::vector<BTCombatant>::iterator monster(individual.begin());
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
  individual.push_back(BTCombatant(monList[monsterType].getAc(), toHit, monList[monsterType].getHp().roll()));
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

void BTCombat::addEffect(int spell, unsigned int expire, int casterLevel, int distance, int group, int target, BitField& resist)
{
 spellEffect.push_back(BTSpellEffect(spell, expire, casterLevel, distance, group, target, resist));
}

void BTCombat::addEncounter(int monsterType, int number /*= 0*/)
{
 BTFactory<BTMonster> &monList = BTGame::getGame()->getMonsterList();
 monsters.push_back(BTMonsterGroup());
 BTMonsterGroup &group = monsters.back();
 group.setMonsterType(monsterType, number);
}

void BTCombat::addPlayer(BTDisplay &d, int who)
{
 BTFactory<BTSpell> &spellList = BTGame::getGame()->getSpellList();
 for (std::list<BTSpellEffect>::iterator itr = spellEffect.begin(); itr != spellEffect.end();)
 {
  if ((BTTARGET_PARTY == itr->group) && (BTTARGET_INDIVIDUAL == itr->target))
  {
   BitField resists;
   if (spellList[itr->spell].checkResists(this, itr->group, who, resists))
   {
    itr->resists.set(who);
    spellList[itr->spell].displayResists(d, this, itr->group, who);
   }
   else
    spellList[itr->spell].apply(d, false, this, itr->casterLevel, itr->distance, itr->group, who, itr->resists);
  }
  ++itr;
 }
}

void BTCombat::clearEffects(BTDisplay &d)
{
 BTFactory<BTSpell> &spellList = BTGame::getGame()->getSpellList();
 for (std::list<BTSpellEffect>::iterator itr = spellEffect.begin(); itr != spellEffect.end(); itr = spellEffect.begin())
 {
  int spell = itr->spell;
  int casterLevel = itr->casterLevel;
  int distance = itr->distance;
  int group = itr->group;
  int target = itr->target;
  int expiration = itr->expiration;
  BitField resists = itr->resists;
  spellEffect.erase(itr);
  if ((BTTIME_PERMANENT != expiration) && (BTTIME_CONTINUOUS != expiration))
   spellList[spell].finish(d, this, casterLevel, distance, group, target, resists);
 }
}

void BTCombat::clearEncounters()
{
 monsters.clear();
}

int BTCombat::findScreen(int num)
{
 static int lastScreen(0);
 int i;
 if ((optionState) && ((num == BTCOMBATSCREEN_OPTION) || (num == BTCOMBATSCREEN_COMBAT)))
 {
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
     canAttack = ((!canAdvance) && (0 < monsters.size()));
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
    canAttack = ((!canAdvance) && (i < BT_BACK) && (!monsters.empty()));
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
       canAttack = ((!canAdvance) && (i < BT_BACK) && (!monsters.empty()));
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
  if ((party[target]->isAlive()) && (target != ignore))
  {
   ++alive;
  }
 }
 if (!alive)
  return false;
 alive = BTDice(1, alive).roll();
 for (target = 0; target < party.size(); ++target)
 {
  if (target != ignore)
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
  add("monsterName", &itr->monsterName, attrib);
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
 if (where == BTPARTY_REMOVE)
 {
  for (std::list<BTSpellEffect>::iterator itr = spellEffect.begin(); itr != spellEffect.end();)
  {
   if ((BTTARGET_PARTY == itr->group) && (who == itr->target))
   {
    int spell = itr->spell;
    int expiration = itr->expiration;
    int casterLevel = itr->casterLevel;
    int distance = itr->distance;
    int group = itr->group;
    int target = itr->target;
    BitField resists = itr->resists;
    itr = spellEffect.erase(itr);
    int size = spellEffect.size();
    if ((BTTIME_PERMANENT != expiration) && (BTTIME_CONTINUOUS != expiration))
     spellList[spell].finish(d, this, casterLevel, distance, group, target, resists);
    if (size != spellEffect.size())
     itr = spellEffect.begin();
   }
   else
    ++itr;
  }
 }
 for (std::list<BTSpellEffect>::iterator itr = spellEffect.begin(); itr != spellEffect.end();)
 {
  if ((BTTARGET_PARTY == itr->group) && (BTTARGET_INDIVIDUAL == itr->target))
  {
   if (BTPARTY_REMOVE == where)
    itr->resists.remove(where);
   else
    itr->resists.move(who, where);
  }
  else if ((BTTARGET_PARTY == itr->group) && (where != BTPARTY_REMOVE) && (who == itr->target))
  {
   itr->target = where;
  }
  else if ((BTTARGET_PARTY == itr->group) && (where == BTPARTY_REMOVE) && (who < itr->target))
  {
   itr->target--;
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
 d.clearText();
 clearEffects(d);
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
   party[i]->active = false;
  d.addText("The party advances...");
  d.addText(blank);
  d.process(BTDisplay::allKeys, 1000);
  d.clearElements();
 }
 else if (BTPc::BTPcAction::runAway == party[i]->combat.action)
 {
  for (i = 0; i < party.size(); ++i)
   party[i]->active = false;
  throw BTCombatError("runAway");
 }
 else
 {
  for (i = 0; i < party.size(); ++i)
   if (party[i]->active)
    ++active;
 }

 while (active > 0)
 {
  bool ran = false;
  BTDice whoDie(1, active, -1);
  int who = whoDie.roll();
  for (itr = monsters.begin(); itr != monsters.end(); ++itr)
  {
   if (who < itr->active)
   {
    --itr->active;
    for (std::vector<BTCombatant>::iterator monster(itr->individual.begin()); monster != itr->individual.end(); ++monster)
    {
     if (monster->active)
     {
      --who;
      if (-1 == who)
      {
       ran = true;
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
    if (party[i]->active)
    {
     --who;
     if (-1 == who)
     {
      ran = true;
      runPcAction(d, active, *party[i]);
     }
    }
   }
  }
  if (!ran)
  {
   debugActive();
   break;
  }
 }
 BTGame::getGame()->nextTurn(d, this);
 if (endRound(d))
  won = true;
 d.drawStats(); // In case check dead move people around
 d.drawIcons();
}

void BTCombat::runMonsterAction(BTDisplay &d, int &active, BTMonsterGroup &grp, BTCombatant &mon)
{
 BTGame *game = BTGame::getGame();
 BTFactory<BTMonster> &monList = game->getMonsterList();
 BTFactory<BTSpell> &spellList = game->getSpellList();
 BTParty &party = game->getParty();
 mon.active = false;
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
   for (std::vector<BTCombatant>::iterator monster(grp.individual.begin()); monster != grp.individual.end(); ++monster)
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
  BTCombatant *defender = NULL;
  int target;
  if (grp.distance <= 1)
  {
   for (int attacks = 0; attacks < monList[grp.monsterType].getRateAttacks(); )
   {
    if (findTargetPC(BT_BACK, target))
    {
     defender = party[target];
     std::string text = monList[grp.monsterType].getName();
     text += " ";
     text += monList[grp.monsterType].getMeleeMessage();
     text += " ";
     text += party[target]->name;
     ++attacks;
     int roll = BTDice(1, 20).roll();
     if ((1 != roll) && ((20 == roll) || (roll + mon.toHit >= defender->ac)))
     {
      text += " ";
      int damage = 0;
      int special = 0;
      text += "and hits for";
      damage = monList[grp.monsterType].getMeleeDamage().roll();
      special = monList[grp.monsterType].getMeleeExtra();
      text += " ";
      char tmp[20];
      sprintf(tmp, "%d", damage);
      text += tmp;
      text += " points of damage";
      if (defender->takeHP(damage))
      {
       text += ", killing him!";
       if (defender->active)
       {
        defender->active = false;
        --active;
       }
      }
      else
      {
       if (special)
       {
        if (party[target]->savingThrow(BTSAVE_DIFFICULTY))
         special = 0;
        switch(special)
        {
         case BTEXTRADAMAGE_POSION:
          defender->status.set(BTSTATUS_POISONED);
          text += " and poisons";
          break;
         case BTEXTRADAMAGE_INSANITY:
          defender->status.set(BTSTATUS_INSANE);
          text += " and inflicts insanity";
          break;
         case BTEXTRADAMAGE_POSSESSION:
          defender->status.set(BTSTATUS_POSSESSED);
          text += " and possesses";
          break;
         case BTEXTRADAMAGE_PARALYSIS:
          defender->status.set(BTSTATUS_PARALYZED);
          text += " and paralyzes";
          break;
         default:
          break;
        }
       }
       text += ".";
      }
      d.drawStats();
     }
     else
      text += ", but misses!";
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
    active -= spellList[monList[grp.monsterType].getRangedSpell()].cast(d, monList[grp.monsterType].getName(), true, this, monList[grp.monsterType].getLevel(), grp.distance, BTTARGET_PARTY, target);
    break;
   }
   default:
   {
    std::string text = monList[grp.monsterType].getName();
    text += " is special attacking";
    d.addText(text.c_str());
    d.addText(blank);
    d.process(BTDisplay::allKeys, 1000);
    d.clearElements();
    break;
   }
  }
 }
}

void BTCombat::runPcAction(BTDisplay &d, int &active, BTPc &pc)
{
 BTGame *game = BTGame::getGame();
 BTFactory<BTItem> &itemList = game->getItemList();
 BTSkillList &skillList = game->getSkillList();
 BTFactory<BTMonster> &monList = game->getMonsterList();
 BTParty &party = game->getParty();
 BTFactory<BTSpell> &spellList = game->getSpellList();
 std::string text;
 pc.active = false;
 --active;
 if (BTPc::BTPcAction::runAway == pc.combat.action)
 {
 }
 else if (BTPc::BTPcAction::advance == pc.combat.action)
 {
 }
 else
 {
  if (pc.status.isSet(BTSTATUS_PARALYZED))
   return;
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
    for (int attacks = 0; attacks < pc.rateAttacks; )
    {
     BTCombatant *defender = NULL;
     BTMonsterGroup *grp = NULL;
     int target = BTTARGET_INDIVIDUAL;
     if (BTPc::BTPcAction::attack == pc.combat.action)
     {
      findTarget(pc, 1, grp, target);
      if (NULL == grp)
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
     text = pc.name;
     text += " ";
     if (-1 == handWeapon)
     {
      if (BTMONSTER_NONE == pc.monster)
       text += "punches at";
      else
       text += monList[pc.monster].getMeleeMessage();
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
     if ((1 != roll) && ((20 == roll) || (roll + pc.toHit >= defender->ac)))
     {
      text += " ";
      int damage = 0;
      BitField special;
      if (-1 == handWeapon)
      {
       if (BTMONSTER_NONE == pc.monster)
       {
        text += "and strikes for";
        damage = BTDice(1, 2).roll();
       }
       else
       {
        text += "and hits for";
        damage = monList[pc.monster].getMeleeDamage().roll();
        special.set(monList[pc.monster].getMeleeExtra());
       }
      }
      else
      {
       BTItem &item = itemList[handWeapon];
       text += item.getEffect();
       damage = item.getDamage().roll();
       if (BTDice(1, 100).roll() <= item.getChanceXSpecial())
        special.set(item.getXSpecial());
      }
      text += " ";
      if (pc.stat[BTSTAT_ST] > 14)
       damage += pc.stat[BTSTAT_ST] - 14;
      char tmp[20];
      sprintf(tmp, "%d", damage);
      text += tmp;
      text += " points of damage";
      if (defender->takeHP(damage))
      {
       text += ", killing him!";
       if (defender->active)
       {
        defender->active = false;
        if (grp)
        {
         grp->active--;
        }
        --active;
       }
      }
      else
      {
       for (int i = 0; i < skillList.size(); ++i)
       {
        if ((skillList[i]->use == BTSKILLUSE_AUTOCOMBAT) && (pc.skill[i] > 0) && (BTDice(1, 100).roll() <= pc.skill[i]))
        {
         special.set(skillList[i]->effect);
        }
       }
       int maxSpecial = special.getMaxSet();
       if (maxSpecial > -1)
       {
        std::string specialText;
        for (int i = 0; i <= maxSpecial; ++i)
        {
         if (!special.isSet(i))
          continue;
         if (i != BTEXTRADAMAGE_CRITICALHIT)
         {
          if ((grp) && (monList[grp->monsterType].savingThrow(BTSAVE_DIFFICULTY)))
           continue;
          else if ((!grp) && (party[target]->savingThrow(BTSAVE_DIFFICULTY)))
           continue;
         }
         if ((specialText == "") || (maxSpecial == i))
          specialText += " and";
         else
          specialText += ",";
         switch(i)
         {
          case BTEXTRADAMAGE_POSION:
           defender->status.set(BTSTATUS_POISONED);
           specialText += " poisons";
           break;
          case BTEXTRADAMAGE_INSANITY:
           defender->status.set(BTSTATUS_INSANE);
           specialText += " inflicts insanity";
           break;
          case BTEXTRADAMAGE_POSSESSION:
           defender->status.set(BTSTATUS_POSSESSED);
           specialText += " possesses";
           break;
          case BTEXTRADAMAGE_PARALYSIS:
           defender->status.set(BTSTATUS_PARALYZED);
           specialText += " paralyzes";
           break;
          case BTEXTRADAMAGE_STONED:
           defender->status.set(BTSTATUS_STONED);
           specialText += " stones";
           if (defender->active)
           {
            defender->active = false;
            if (grp)
            {
             grp->active--;
            }
            --active;
           }
           break;
          case BTEXTRADAMAGE_CRITICALHIT:
           specialText += " critically hits";
           defender->status.set(BTSTATUS_DEAD);
           if (defender->active)
           {
            defender->active = false;
            if (grp)
            {
             grp->active--;
            }
            --active;
           }
           break;
          default:
           break;
         }
        }
        text += specialText;
       }
       if (defender->isAlive())
        text += ".";
       else
        text += "!";
      }
      if (!grp)
       d.drawStats();
     }
     else
      text += ", but misses!";
     d.drawMessage(text.c_str(), game->getDelay());
    }
    break;
   }
   case BTPc::BTPcAction::defend:
    break;
   case BTPc::BTPcAction::cast:
   {
    pc.sp -= spellList[pc.combat.object].getSp();
    BTMonsterGroup *grp = NULL;
    int target = pc.combat.getTargetIndividual();
    if ((spellList[pc.combat.object].getArea() == BTAREAEFFECT_FOE) && (target == BTTARGET_INDIVIDUAL))
     findTarget(pc, BTDISTANCE_MAX, grp, target);
    active -= spellList[pc.combat.object].cast(d, pc.name, true, this, pc.level, 0, pc.combat.getTargetGroup(), target);
    break;
   }
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
}

void BTCombat::debugActive()
{
 BTParty &party = BTGame::getGame()->getParty();
 printf("Active Count Error\n");
 for (std::list<BTMonsterGroup>::iterator itr(monsters.begin()); itr != monsters.end(); ++itr)
 {
  int realActive = 0;
  for (std::vector<BTCombatant>::iterator monster(itr->individual.begin()); monster != itr->individual.end(); ++monster)
  {
   if (monster->active)
   {
    ++realActive;
   }
  }
  printf("%s: cache %d, real %d\n", itr->monsterName, itr->active, realActive);
 }
 for (int i = 0; i < party.size(); i++)
 {
  if (party[i]->active)
   printf("%s\n", party[i]->name);
 }
}

bool BTCombat::endRound(BTDisplay &d)
{
 BTGame *game = BTGame::getGame();
 ++round;
 BTFactory<BTMonster> &monList = game->getMonsterList();
 BTFactory<BTSpell> &spellList = game->getSpellList();
 int group;
 std::list<BTSpellEffect>::iterator effect = spellEffect.begin();
 for (effect = spellEffect.begin(); effect != spellEffect.end();)
 {
  if (game->isExpired(effect->expiration))
  {
   int spell = effect->spell;
   int casterLevel = effect->casterLevel;
   int distance = effect->distance;
   group = effect->group;
   int target = effect->target;
   BitField resists = effect->resists;
   effect = spellEffect.erase(effect);
   int size = spellEffect.size();
   spellList[spell].finish(d, this, casterLevel, distance, group, target, resists);
   if (size != spellEffect.size())
    effect = spellEffect.begin();
  }
  else
   ++effect;
 }
 for (effect = spellEffect.begin(); effect != spellEffect.end(); ++effect)
 {
  if (effect->first)
   effect->first = false;
  else if (BTTIME_PERMANENT != effect->expiration)
   spellList[effect->spell].maintain(d, this, effect->casterLevel, effect->distance, effect->group, effect->target, effect->resists);
 }
 group = BTTARGET_MONSTER;
 int alive = 0;
 for (std::list<BTMonsterGroup>::iterator itr(monsters.begin()); itr != monsters.end();)
 {
  itr->canMove = true;
  itr->active = 0;
  bool hasIndvidualSpell = true;
  for (std::vector<BTCombatant>::iterator monster(itr->individual.begin()); monster != itr->individual.end();)
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
    for (std::list<BTSpellEffect>::iterator effect = spellEffect.begin(); effect != spellEffect.end();)
    {
     if ((group == effect->group) && (effect->target == monster - itr->individual.begin()))
     {
      int spell = effect->spell;
      int casterLevel = effect->casterLevel;
      int distance = itr->distance;
      int expiration = effect->expiration;
      BitField resists = effect->resists;
      effect = spellEffect.erase(effect);
      int size = spellEffect.size();
      if ((BTTIME_PERMANENT != expiration) && (BTTIME_CONTINUOUS != expiration))
       spellList[spell].finish(d, this, casterLevel, distance, group, monster - itr->individual.begin(), resists);
      if (size != spellEffect.size())
       effect = spellEffect.begin();
     }
     else
      ++effect;
    }
    for (std::list<BTSpellEffect>::iterator effect = spellEffect.begin(); effect != spellEffect.end(); ++effect)
    {
     if ((group == effect->group) && (effect->target == BTTARGET_INDIVIDUAL))
     {
      effect->resists.remove(monster - itr->individual.begin());
     }
     if (group == BTTARGET_ALLMONSTERS)
     {
      effect->resists.remove(alive + (monster - itr->individual.begin()));
     }
     else if ((group == effect->group) && (effect->target != BTTARGET_INDIVIDUAL) && (effect->target > monster - itr->individual.begin()))
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
     int spell = effect->spell;
     int casterLevel = effect->casterLevel;
     int distance = itr->distance;
     int expiration = effect->expiration;
     BitField resists = effect->resists;
     effect = spellEffect.erase(effect);
     int size = spellEffect.size();
     if ((BTTIME_PERMANENT != expiration) && (BTTIME_CONTINUOUS != expiration))
      spellList[spell].finish(d, this, casterLevel, distance, group, BTTARGET_INDIVIDUAL, resists);
     if (size != spellEffect.size())
      effect = spellEffect.begin();
    }
    else
     ++effect;
   }
   for (std::list<BTSpellEffect>::iterator effect = spellEffect.begin(); effect != spellEffect.end(); ++effect)
   {
    if (group < effect->group)
    {
     effect->group--;
    }
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
  for (effect = spellEffect.begin(); effect != spellEffect.end(); effect = spellEffect.begin())
  {
   int spell = effect->spell;
   int casterLevel = effect->casterLevel;
   int distance = effect->distance;
   int expiration = effect->expiration;
   group = effect->group;
   int target = effect->target;
   BitField resists = effect->resists;
   spellEffect.erase(effect);
   if ((BTTIME_PERMANENT != expiration) && (BTTIME_CONTINUOUS != expiration))
    spellList[spell].finish(d, this, casterLevel, distance, group, target, resists);
  }
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
