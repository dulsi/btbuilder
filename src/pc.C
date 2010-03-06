/*-------------------------------------------------------------------------*\
  <pc.C> -- Player character implementation file

  Date      Programmer  Description
  03/11/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "pc.h"
#include "game.h"
#include <algorithm>

void BTEquipment::serialize(ObjectSerializer* s)
{
 s->add("id", &id);
 s->add("equipped", &equipped);
 s->add("known", &known);
 s->add("charges", &charges);
}

BTPc::BTPc()
 : race(0), job(0), picture(-1), monster(BTMONSTER_NONE), rateAttacks(1), save(0), sp(0), maxSp(0), level(1), gold(0), xp(0)
{
 name = new char[1];
 name[0] = 0;
 int i;
 for (i = 0; i < BT_STATS; ++i)
  stat[i] = 10;
 int skills = BTGame::getGame()->getSkillList().size();
 skill = new int[skills];
 for (i = 0; i < skills; ++i)
  skill[i] = 0;
}

BTPc::BTPc(int monsterType, int j)
 : race(-1), job(j), picture(-1), monster(monsterType), rateAttacks(1), save(0), sp(0), maxSp(0), level(1), gold(0), xp(0)
{
 // TO DO: Modify to accept combatant as an optional argument so that
 // spell bind can be implemented.
 BTFactory<BTMonster> &monsterList = BTGame::getGame()->getMonsterList();
 BTJobList &jobList = BTGame::getGame()->getJobList();
 name = new char[strlen(monsterList[monster].getName()) + 1];
 strcpy(name, monsterList[monster].getName());
 picture = monsterList[monster].getPicture();
 ac = monsterList[monster].getAc();
 toHit = jobList[job]->calcToHit(level);
 save = jobList[job]->calcSave(level);
 hp = maxHp = monsterList[monster].getHp().roll();
 status.set(BTSTATUS_NPC);
 int i;
 for (i = 0; i < BT_STATS; ++i)
  stat[i] = 10;
 int skills = BTGame::getGame()->getSkillList().size();
 skill = new int[skills];
 for (i = 0; i < skills; ++i)
  skill[i] = 0;
}

bool BTPc::advanceLevel()
{
 BTJobList &jobList = BTGame::getGame()->getJobList();
 BTXpChartList &xpChartList = BTGame::getGame()->getXpChartList();
 if (jobList[job]->xpChart != -1)
 {
  if (xp >= xpChartList[jobList[job]->xpChart]->getXpNeeded(level))
  {
   ++level;
   if (((level - 1) % jobList[job]->improveToHit) == 0)
    ++toHit;
   if ((jobList[job]->improveRateAttacks) && (((level - 1) % jobList[job]->improveRateAttacks) == 0))
   {
    ++rateAttacks;
    if (rateAttacks > jobList[job]->maxRateAttacks)
     rateAttacks = jobList[job]->improveRateAttacks;
   }
   if (((level - 1) % jobList[job]->improveSave) == 0)
    ++save;
   if ((jobList[job]->improveAc) && (((level - 1) % jobList[job]->improveAc) == 0))
    ++ac;
   int moreHp = BTDice(1, jobList[job]->hp).roll() + ((stat[BTSTAT_CN] > 14) ? stat[BTSTAT_CN] - 14 : 0);
   hp += moreHp;
   maxHp += moreHp;
   for (int i = 0; i < jobList[job]->skill.size(); ++i)
   {
    if (jobList[job]->skill[i]->improve > 0)
    {
     skill[jobList[job]->skill[i]->skill] += BTDice(1, jobList[job]->skill[i]->improve).roll();
     if ((jobList[job]->skill[i]->modifier >= 0) && (stat[jobList[job]->skill[i]->modifier] > 14))
      skill[jobList[job]->skill[i]->skill] += stat[jobList[job]->skill[i]->modifier] - 14;
     if (skill[jobList[job]->skill[i]->skill] > 99)
      skill[jobList[job]->skill[i]->skill] = 99;
    }
   }
   if (jobList[job]->spells)
   {
    int moreSp = BTDice(1, 4).roll() + ((stat[BTSTAT_IQ] > 14) ? stat[BTSTAT_IQ] - 14 : 0);
    sp += moreSp;
    maxSp += moreSp;
   }
   return true;
  }
 }
 return false;
}

void BTPc::equip(int index)
{
 BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
 int type = itemList[item[index].id].getType();
 for (int i = 0; i < BT_ITEMS; ++i)
 {
  if (BTITEM_NONE == item[i].id)
   break;
  if ((item[i].equipped == BTITEM_EQUIPPED) && (type == itemList[item[i].id].getType()))
  {
   unequip(i);
  }
 }
 ac += itemList[item[index].id].getArmorPlus();
 toHit += itemList[item[index].id].getHitPlus();
 item[index].equipped = BTITEM_EQUIPPED;
}

int BTPc::incrementStat()
{
 int s = 0;
 int i = 0; 
 for (; i < BT_STATS; ++i)
 {
  if (stat[i] < BTSTAT_MAX)
   ++s;
 }
 if (s == 0)
  return -1;
 if (s != 1)
  s = BTDice(1, s, -1).roll();
 for (i = 0; i < BT_STATS; ++i)
 {
  if (stat[i] < BTSTAT_MAX)
  {
   --s;
   ++stat[i];
   if ((i == BTSTAT_LK) && (stat[i] > 14))
    ++save;
   if ((i == BTSTAT_DX) && (stat[i] > 14))
    ++ac;
   if (0 == s)
    return i;
  }
 }
 return -1; // Can't get here
}

bool BTPc::isEquipped(int index) const
{
 return (item[index].equipped == BTITEM_EQUIPPED);
}

bool BTPc::isEquipmentEmpty() const
{
 if (BTITEM_NONE == item[0].id)
  return true;
 else
  return false;
}

bool BTPc::isEquipmentFull() const
{
 if (BTITEM_NONE != item[BT_ITEMS - 1].id)
  return true;
 else
  return false;
}

int BTPc::getGold() const
{
 return gold;
}

int BTPc::getHandWeapon() const
{
 BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
 for (int i = 0; i < BT_ITEMS; ++i)
 {
  if ((isEquipped(i)) && (BTITEM_HANDWEAPON == itemList[getItem(i)].getType()))
  {
   return getItem(i);
  }
 }
 return -1;
}

int BTPc::getItem(int index) const
{
 return item[index].id;
}

unsigned int BTPc::getXPNeeded()
{
 BTJobList &jobList = BTGame::getGame()->getJobList();
 BTXpChartList &xpChartList = BTGame::getGame()->getXpChartList();
 if (jobList[job]->xpChart != -1)
 {
  return xpChartList[jobList[job]->xpChart]->getXpNeeded(level) - xp;
 }
 else
  return 4000000000UL;
}

bool BTPc::giveItem(int id, bool known, int charges)
{
 for (int i = 0; i < BT_ITEMS; ++i)
 {
  if (BTITEM_NONE == item[i].id)
  {
   BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
   item[i].id = id;
   item[i].equipped = (itemList[id].canUse(this) ? BTITEM_NOTEQUIPPED : BTITEM_CANNOTEQUIP);
   item[i].known = known;
   item[i].charges = charges;
   return true;
  }
 }
 return false;
}

unsigned int BTPc::giveGold(unsigned int amount)
{
 // TODO: return the amount left
 gold += amount;
 if (gold > 4000000000UL)
  gold = 4000000000UL;
 return 0;
}

void BTPc::giveHP(int amount)
{
 if (!status.isSet(BTSTATUS_DEAD))
 {
  hp += amount;
  if (hp > maxHp)
   hp = maxHp;
 }
}

void BTPc::giveXP(unsigned int amount)
{
 xp += amount;
 if (xp > 4000000000UL)
  xp = 4000000000UL;
}

bool BTPc::hasItem(int id) const
{
 for (int i = 0; i < BT_ITEMS; ++i)
 {
  if (id == item[i].id)
  {
   return true;
  }
 }
 return false;
}

bool BTPc::savingThrow(int difficulty /*= BTSAVE_DIFFICULTY*/) const
{
 int roll = BTDice(1, 20, save).roll();
 if (roll == 20 + save)
  return true;
 else if (roll == 1 + save)
  return false;
 else
  return (roll >= difficulty);
}

void BTPc::serialize(ObjectSerializer* s)
{
 int i;
 s->add("name", &name);
 s->add("race", &race, NULL, &BTGame::getGame()->getRaceList());
 s->add("job", &job, NULL, &BTGame::getGame()->getJobList());
 s->add("jobAbbrev", &job, NULL, &BTGame::getGame()->getJobAbbrevList());
 s->add("picture", &picture);
 s->add("monster", &monster);
 for (i = 0; i < BT_STATS; ++i)
 {
  std::vector<XMLAttribute> *attrib = new std::vector<XMLAttribute>;
  char tmp[10];
  snprintf(tmp, 10, "%d", i + 1);
  attrib->push_back(XMLAttribute("number", tmp));
  s->add("stat", &stat[i], attrib);
 }
 s->add("ac", &ac);
 s->add("toHit", &toHit);
 s->add("rateAttacks", &rateAttacks);
 s->add("save", &save);
 s->add("status", &status, &BTStatusLookup::lookup);
 s->add("maxhp", &maxHp);
 s->add("hp", &hp);
 s->add("maxsp", &maxSp);
 s->add("sp", &sp);
 s->add("level", &level);
 s->add("xp", &xp);
 s->add("gold", &gold);
 int skills = BTGame::getGame()->getSkillList().size();
 for (i = 0; i < skills; ++i)
 {
  std::vector<XMLAttribute> *attrib = new std::vector<XMLAttribute>;
  char tmp[10];
  snprintf(tmp, 10, "%d", i + 1);
  attrib->push_back(XMLAttribute("number", tmp));
  s->add("skill", &skill[i], attrib);
 }
 for (i = 0; i < BT_ITEMS; ++i)
 {
  std::vector<XMLAttribute> *attrib = new std::vector<XMLAttribute>;
  char tmp[10];
  snprintf(tmp, 10, "%d", i + 1);
  attrib->push_back(XMLAttribute("number", tmp));
  s->add("item", &item[i], attrib);
 }
}

void BTPc::setName(const char *nm)
{
 delete [] name;
 name = new char[strlen(nm) + 1];
 strcpy(name, nm);
}

unsigned int BTPc::takeGold(unsigned int amount)
{
 if (amount > gold)
 {
  int taken = gold;
  gold = 0;
  return taken;
 }
 else
 {
  gold -= amount;
  return amount;
 }
}

bool BTPc::takeItem(int id)
{
 for (int i = 0; i < BT_ITEMS; ++i)
 {
  if (id == item[i].id)
  {
   return takeItemFromIndex(i);
  }
 }
 return false;
}

bool BTPc::takeItemFromIndex(int index)
{
 if (item[index].id == BTITEM_NONE)
  return false;
 if (item[index].equipped == BTITEM_EQUIPPED)
  unequip(index);
 for (int i = index + 1; i < BT_ITEMS; ++i)
 {
  item[i - 1].id = item[i].id;
  item[i - 1].equipped = item[i].equipped;
  item[i - 1].known = item[i].known;
  item[i - 1].charges = item[i].charges;
 }
 item[BT_ITEMS - 1].id = BTITEM_NONE;
 return true;
}

void BTPc::unequip(int index)
{
 BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
 ac -= itemList[item[index].id].getArmorPlus();
 toHit -= itemList[item[index].id].getHitPlus();
 item[index].equipped = BTITEM_NOTEQUIPPED;
}

void BTPc::readXML(const char *filename, XMLVector<BTGroup*> &group, XMLVector<BTPc*> &pc)
{
 XMLSerializer parser;
 parser.add("party", &group, &BTGroup::create);
 parser.add("pc", &pc, &BTPc::create);
 parser.parse(filename, true);
}

void BTPc::writeXML(const char *filename, XMLVector<BTGroup*> &group, XMLVector<BTPc*> &pc)
{
 XMLSerializer parser;
 parser.add("party", &group, &BTGroup::create);
 parser.add("pc", &pc, &BTPc::create);
 parser.write(filename, true);
}

void BTPc::BTPcAction::clearTarget(int group, int member /*= BTTARGET_INDIVIDUAL*/)
{
 if ((member == BTTARGET_INDIVIDUAL) && (group == getTargetGroup()))
 {
  target = 0;
 }
 else if ((member != BTTARGET_INDIVIDUAL) && (group == getTargetGroup()) && (member == getTargetIndividual()))
  target = 0;
}

void BTPc::BTPcAction::setTarget(int group, int member /*= BTTARGET_INDIVIDUAL*/)
{
 // Need a way to specify a group if target consistency is kept.
// if ((member == BTTARGET_INDIVIDUAL) && (group != getTargetGroup()))
  target = (group << BTTARGET_GROUPSHIFT) + member;
// else if (member != BTTARGET_INDIVIDUAL)
//  target = (group << BTTARGET_GROUPSHIFT) + member;
}

void BTParty::add(BTDisplay &d, BTPc *pc)
{
 push_back(pc);
 BTGame::getGame()->addPlayer(d, size() - 1);
}

bool BTParty::checkDead(BTDisplay &d)
{
 int restDead = size();
 int who;
 for (who = size() - 1; who >= 0; --who)
 {
  if (operator[](who)->status.isSet(BTSTATUS_DEAD))
  {
   operator[](who)->active = false;
   restDead = who;
  }
  else
   break;
 }
 if (restDead == 0)
  return true;
 BTGame *game = BTGame::getGame();
 for (who = 0; who < restDead; )
 {
  if (operator[](who)->status.isSet(BTSTATUS_DEAD))
  {
   game->movedPlayer(d, who, size() - 1);
   BTPc *pc = operator[](who);
   pc->active = false;
   erase(begin() + who);
   push_back(pc);
   --restDead;
  }
  else
  {
   operator[](who)->active = true;
   ++who;
  }
 }
 int restStoned = restDead;
 for (who = restStoned - 1; who >= 0; --who)
 {
  if (operator[](who)->status.isSet(BTSTATUS_STONED))
  {
   operator[](who)->active = false;
   restStoned = who;
  }
  else
   break;
 }
 if (restStoned == 0)
  return true;
 for (who = 0; who < restStoned; )
 {
  if (operator[](who)->status.isSet(BTSTATUS_STONED))
  {
   game->movedPlayer(d, who, restStoned - 1);
   BTPc *pc = operator[](who);
   pc->active = false;
   erase(begin() + who);
   if (size() == restStoned - 1)
    push_back(pc);
   else
    insert(begin() + restStoned, pc);
   --restStoned;
  }
  else
  {
   operator[](who)->active = true;
   ++who;
  }
 }
 return false;
}

void BTParty::moveTo(int who, int where, BTDisplay &d)
{
 BTPc *pc = (*this)[who];
 erase(begin() + who);
 if (where > who)
 {
  if (where == size())
   push_back(pc);
  else
   insert(begin() + where, pc);
 }
 else
 {
  insert(begin() + where, pc);
 }
 BTGame::getGame()->movedPlayer(d, who, where);
}

bool BTParty::remove(int who, BTDisplay &d)
{
 if (!removing.isSet(who))
 {
  XMLVector<BTPc*> &roster = BTGame::getGame()->getRoster();
  removing.set(who);
  BTPc *pc = (*this)[who];
  BTGame::getGame()->movedPlayer(d, who, BTPARTY_REMOVE);
  erase(begin() + who);
  if (roster.end() == std::find(roster.begin(), roster.end(), pc))
  {
   delete pc;
  }
  d.drawStats();
  removing.clear(who);
  return true;
 }
 else
  return false;
}

BTStatusLookup BTStatusLookup::lookup;
char *BTStatusLookup::value[8] = { "dead", "poisoned", "insane", "aged", "possessed", "stoned", "paralyzed", "npc" };

