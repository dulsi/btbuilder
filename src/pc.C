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

void BTSkillValue::serialize(ObjectSerializer* s)
{
 s->add("index", &skill);
 s->add("value", &value);
 s->add("uses", &uses);
 s->add("history", &history);
}

BTPc::BTPc()
 : race(0), gender(BTGENDER_MALE), picture(-1), monster(BTMONSTER_NONE), rateAttacks(1), save(0), sp(0), maxSp(0), gold(0), xp(0)
{
 name = new char[1];
 name[0] = 0;
 int i;
 for (i = 0; i < BT_STATS; ++i)
  stat[i] = 10;
 item = new BTEquipment[BTGame::getGame()->getModule()->maxItems];
}

BTPc::BTPc(int monsterType, int j, BTCombatant *c /*= NULL*/)
 : race(-1), gender(BTGENDER_MALE), picture(-1), monster(monsterType), rateAttacks(1), save(0), sp(0), maxSp(0), gold(0), xp(0)
{
 BTFactory<BTMonster> &monsterList = BTGame::getGame()->getMonsterList();
 BTJobList &jobList = BTGame::getGame()->getJobList();
 BTXpChartList &xpChartList = BTGame::getGame()->getXpChartList();
 name = new char[monsterList[monster].getName().length() + 1];
 strcpy(name, monsterList[monster].getName().c_str());
 if (c == NULL)
 {
  job = j;
  level = maxLevel = monsterList[monster].getLevel();
 }
 else
 {
  job = c->job;
  level = c->level;
  maxLevel = c->maxLevel;
 }
 if (level == 0)
  level = maxLevel = 1;
 else if ((level > 1) && (jobList[job]->xpChart != -1))
  xp = xpChartList[jobList[job]->xpChart]->getXpNeeded(level - 1);
 gender = monsterList[monster].getGender();
 picture = monsterList[monster].getPicture();
 ac = monsterList[monster].getAc();
 toHit = jobList[job]->calcToHit(level);
 if (c == NULL)
 {
  hp = maxHp = monsterList[monster].getHp().roll();
 }
 else
 {
  hp = c->hp;
  maxHp = c->maxHp;
 }
 save = jobList[job]->calcSave(level);
 status.set(BTSTATUS_NPC);
 int i;
 for (i = 0; i < BT_STATS; ++i)
  stat[i] = 10;
 item = new BTEquipment[BTGame::getGame()->getModule()->maxItems];
 updateSkills();
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
   if ((jobList[job]->improveRateAttacks) && (level - 1 < ((jobList[job]->maxRateAttacks - 1) * jobList[job]->improveRateAttacks) + 1) && (((level - 1) % jobList[job]->improveRateAttacks) == 0))
   {
    ++rateAttacks;
   }
   if (((level - 1) % jobList[job]->improveSave) == 0)
    ++save;
   if ((jobList[job]->improveAc) && (((level - 1) % jobList[job]->improveAc) == 0))
    ++ac;
   if (level > maxLevel)
   {
    int moreHp = BTDice(1, jobList[job]->hp).roll() + ((stat[BTSTAT_CN] > 14) ? stat[BTSTAT_CN] - 14 : 0);
    hp += moreHp;
    maxHp += moreHp;
    for (int i = 0; i < jobList[job]->skill.size(); ++i)
    {
     if ((jobList[job]->skill[i]->improve > 0) && ((level % jobList[job]->skill[i]->improveLevel) == 0))
     {
      for (int k = 0; k < skill.size(); ++k)
      {
       if (skill[k]->skill == jobList[job]->skill[i]->skill)
       {
        unsigned int increase = BTDice(1, jobList[job]->skill[i]->improve).roll();
        if ((jobList[job]->skill[i]->modifier >= 0) && (stat[jobList[job]->skill[i]->modifier] > 14))
         increase += stat[jobList[job]->skill[i]->modifier] - 14;
        skill[k]->value += increase;
        skill[k]->history.push_back(increase);
        break;
       }
      }
     }
    }
    if (jobList[job]->spells)
    {
     int moreSp = BTDice(1, 4).roll() + ((stat[BTSTAT_IQ] > 14) ? stat[BTSTAT_IQ] - 14 : 0);
     sp += moreSp;
     maxSp += moreSp;
    }
    maxLevel = level;
   }
   else
   {
    for (int k = 0; k < skill.size(); ++k)
    {
     if (skill[k]->history.size() >= level - 1)
     {
      skill[k]->value += skill[k]->history[level - 2];
     }
    }
   }
   return true;
  }
 }
 return false;
}

bool BTPc::age()
{
 for (int i = 0; i < BT_STATS; ++i)
 {
  if (stat[i] > 1)
  {
   if (stat[i] > 14)
   {
    if (i == BTSTAT_DX)
     ac -= 1;
    if (i == BTSTAT_LK)
     save -= 1;
   }
   stat[i] -= 1;
  }
 }
 status.set(BTSTATUS_AGED);
 return false;
}

std::string BTPc::attack(BTCombatant *defender, int weapon, int &numAttacksLeft, int &activeNum)
{
 BTGame *game = BTGame::getGame();
 BTFactory<BTItem> &itemList = game->getItemList();
 BTFactory<BTMonster> &monList = game->getMonsterList();
 BTDice damageDice(1, 2);
 IShort chanceXSpecial(0);
 IShort xSpecial(BTEXTRADAMAGE_NONE);
 bool melee = true;
 if (-1 == weapon)
 {
  if (BTMONSTER_NONE != monster)
  {
   damageDice = monList[monster].getMeleeDamage();
   chanceXSpecial = 100;
   xSpecial = monList[monster].getMeleeExtra();
  }
  else
  {
/*   XMLVector<BTSkill*> &skill = game->getSkillList();
   for (int i = 0; i < skill.size(); ++i)
   {
    if ((skill[i]->special == BTSKILLSPECIAL_BAREHANDS) && (hasSkillUse(i)))
    {
     BTDice *skillDice = skill[i]->getRoll(getSkill(i));
     if (skillDice)
      damageDice = *skillDice;
     break;
    }
   }*/
  }
 }
 else
 {
  BTItem &itemWeapon = itemList[weapon];
  damageDice = itemWeapon.getDamage();
  chanceXSpecial = itemWeapon.getChanceXSpecial();
  xSpecial = itemWeapon.getXSpecial();
  if ((BTITEM_ARROW == itemWeapon.getType()) || (BTITEM_THROWNWEAPON == itemWeapon.getType()))
   melee = false;
 }
 if (stat[BTSTAT_ST] > 14)
  damageDice.setModifier(damageDice.getModifier() + stat[BTSTAT_ST] - 14);
 std::string cause;
 std::string effect;
 if (-1 == weapon)
 {
  if (BTMONSTER_NONE == monster)
  {
   cause = "punches at";
   effect = "and strikes";
  }
  else
  {
   cause = monList[monster].getMeleeMessage();
   effect = "and hits";
  }
 }
 else
 {
  BTItem &itemWeapon = itemList[weapon];
  cause = itemWeapon.getCause();
  effect = itemWeapon.getEffect();
  if ((effect.length() >= 4) && (0 == strcmp(effect.c_str() + effect.length() - 4, " for")))
   effect.resize(effect.length() - 4);
 }
 return BTCombatant::attack(defender, melee, cause, effect, damageDice, chanceXSpecial, xSpecial, numAttacksLeft, activeNum);
}

void BTPc::changeJob(int newJob)
{
 XMLVector<BTJob*> &jobList = BTGame::getGame()->getJobList();
 toHit += jobList[newJob]->toHit - jobList[job]->toHit - ((level - 1) / jobList[job]->improveToHit);
 save += jobList[newJob]->save - jobList[job]->save - ((level - 1) / jobList[job]->improveSave);
 ac += jobList[newJob]->ac - jobList[job]->ac;
 if (jobList[job]->improveAc != 0)
  ac -= ((level - 1) / jobList[job]->improveAc);
 job = newJob;
 if (gender == BTGENDER_FEMALE)
  picture = jobList[newJob]->femalePicture;
 else
  picture = jobList[newJob]->malePicture;
 int moreHp = BTDice(1, jobList[job]->hp).roll() + ((stat[BTSTAT_CN] > 14) ? stat[BTSTAT_CN] - 14 : 0);
 hp += moreHp;
 maxHp += moreHp;
 {
  for (int i = 0; i < skill.size(); ++i)
  {
   skill[i]->history.clear();
  }
 }
 for (int k = 0; k < jobList[newJob]->skill.size(); ++k)
 {
  bool found(false);
  for (int i = 0; i < skill.size(); ++i)
  {
   if (skill[i]->skill == jobList[newJob]->skill[k]->skill)
   {
    if ((jobList[newJob]->skill[k]->value == 0) && (jobList[newJob]->skill[k]->improve == 0))
    {
     skill.erase(skill.begin() + i);
    }
    else
    {
     skill[i]->value = jobList[newJob]->skill[k]->value;
     if ((jobList[newJob]->skill[k]->modifier >= 0) && (stat[jobList[newJob]->skill[k]->modifier] > 14))
      skill[i]->value += stat[jobList[newJob]->skill[k]->modifier] - 14;
    }
    found = true;
    break;
   }
  }
  if ((!found) && ((jobList[newJob]->skill[k]->value != 0) || (jobList[newJob]->skill[k]->improve != 0)))
  {
   BTSkillValue *value = new BTSkillValue;
   value->skill = jobList[newJob]->skill[k]->skill;
   value->value = jobList[newJob]->skill[k]->value;
   if ((jobList[newJob]->skill[k]->modifier >= 0) && (stat[jobList[newJob]->skill[k]->modifier] > 14))
    value->value += stat[jobList[newJob]->skill[k]->modifier] - 14;
   skill.push_back(value);
  }
 }
 if (jobList[newJob]->spells)
 {
  int moreSp = BTDice(1, 4).roll() + ((stat[BTSTAT_IQ] > 14) ? stat[BTSTAT_IQ] - 14 : 0);
  if (maxSp == 0)
   moreSp = BTDice(1, 8, 9).roll() + ((stat[BTSTAT_IQ] > 14) ? stat[BTSTAT_IQ] - 14 : 0);
  sp += moreSp;
  maxSp += moreSp;
 }
 maxLevel = level = 1;
 xp = 0;
}

bool BTPc::drainItem(int amount)
{
 BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
 int numItems = 0;
 for (numItems = 0; numItems < BT_ITEMS; ++numItems)
 {
  if (BTITEM_NONE == item[numItems].id)
   break;
 }
 if (numItems == 0)
  return false;
 int i = BTDice(1, numItems, -1).roll();
 if ((item[i].equipped == BTITEM_EQUIPPED) && (BTITEM_ARROW != itemList[item[i].id].getType()) && (BTITEM_BOW != itemList[item[i].id].getType()) && (BTITEM_THROWNWEAPON != itemList[item[i].id].getType()) && (BTITEMCAST_NONE != itemList[item[i].id].getSpellCast()) && (0 < item[i].charges))
 {
  takeItemCharge(i, amount);
  return true;
 }
 return false;
}

bool BTPc::drainLevel()
{
 BTJobList &jobList = BTGame::getGame()->getJobList();
 BTXpChartList &xpChartList = BTGame::getGame()->getXpChartList();
 bool answer = BTCombatant::drainLevel();
 if (!answer)
 {
  if ((jobList[job]->improveRateAttacks) && ((jobList[job]->maxRateAttacks - 1) * jobList[job]->improveRateAttacks > level) && (((level) % jobList[job]->improveRateAttacks) == 0))
  {
   --rateAttacks;
  }
  if (((level) % jobList[job]->improveSave) == 0)
   --save;
  for (int k = 0; k < skill.size(); ++k)
  {
   if (skill[k]->history.size() >= level)
   {
    skill[k]->value -= skill[k]->history[level - 1];
   }
  }
  if (level > 1)
   xp = xpChartList[jobList[job]->xpChart]->getXpNeeded(level - 1);
  else
   xp = 0;
 }
 return answer;
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

int BTPc::hiddenTime() const
{
 BTSkillList &skillList = BTGame::getGame()->getSkillList();
 if ((combat.skillUsed != -1) && (skillList[combat.skillUsed]->special == BTSKILLSPECIAL_HIDE))
  return combat.consecutiveUsed;
 else
  return 0;
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
  s = BTDice(1, s, 0).roll();
 for (i = 0; i < BT_STATS; ++i)
 {
  if (stat[i] < BTSTAT_MAX)
  {
   --s;
   if (0 == s)
   {
    ++stat[i];
    if (statMax[i] < BTSTAT_MAX)
     ++statMax[i];
    if ((i == BTSTAT_LK) && (stat[i] > 14))
     ++save;
    if ((i == BTSTAT_DX) && (stat[i] > 14))
     ++ac;
    return i;
   }
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

std::string BTPc::getName() const
{
 return name;
}

int BTPc::getGender() const
{
 return gender;
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

int BTPc::getSkill(int skNum) const
{
 for (int i = 0; i < skill.size(); ++i)
 {
  if (skill[i]->skill == skNum)
  {
   return skill[i]->value;
  }
 }
 return 0;
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

void BTPc::giveSP(int amount)
{
 if (!status.isSet(BTSTATUS_DEAD))
 {
  sp += amount;
  if (sp > maxSp)
   sp = maxSp;
 }
}

void BTPc::giveSkillUse(int skNum, int amount)
{
 if (!status.isSet(BTSTATUS_DEAD))
 {
  for (int i = 0; i < skill.size(); ++i)
  {
   if (skill[i]->skill == skNum)
   {
    skill[i]->uses += amount;
    if (skill[i]->uses > skill[i]->value)
     skill[i]->uses = skill[i]->value;
    else if (skill[i]->uses < 0)
     skill[i]->uses = 0;
   }
  }
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

bool BTPc::hasSkillUse(int skNum)
{
 for (int i = 0; i < skill.size(); ++i)
 {
  if (skill[i]->skill == skNum)
  {
   return skill[i]->uses > 0;
  }
 }
 return false;
}

bool BTPc::isIllusion() const
{
 return job == BTJOB_ILLUSION;
}

void BTPc::restoreLevel()
{
 BTJobList &jobList = BTGame::getGame()->getJobList();
 BTXpChartList &xpChartList = BTGame::getGame()->getXpChartList();
 if (jobList[job]->xpChart != -1)
 {
  if (level < maxLevel)
  {
   if (xp < xpChartList[jobList[job]->xpChart]->getXpNeeded(maxLevel))
    xp = xpChartList[jobList[job]->xpChart]->getXpNeeded(maxLevel);
   while (level < maxLevel)
    advanceLevel();
  }
 }
}

void BTPc::rollInitiative()
{
 BTCombatant::rollInitiative();
 if (stat[BTSTAT_LK] > 14)
  initiative += stat[BTSTAT_LK] - 14;
 if (stat[BTSTAT_DX] > 14)
  initiative += stat[BTSTAT_DX] - 14;
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
 s->add("gender", &gender, NULL, &genderLookup);
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
 for (i = 0; i < BT_STATS; ++i)
 {
  std::vector<XMLAttribute> *attrib = new std::vector<XMLAttribute>;
  char tmp[10];
  snprintf(tmp, 10, "%d", i + 1);
  attrib->push_back(XMLAttribute("number", tmp));
  s->add("statMax", &statMax[i], attrib);
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
 s->add("maxLevel", &maxLevel);
 s->add("level", &level);
 s->add("xp", &xp);
 s->add("gold", &gold);
 s->add("skill", &skill, &BTSkillValue::create);
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

void BTPc::setSkill(int skNum, int value, int uses)
{
 for (int i = 0; i < skill.size(); ++i)
 {
  if (skill[i]->skill == skNum)
  {
   skill[i]->value = value;
   skill[i]->uses = uses;
   return ;
  }
 }
 BTSkillValue *val = new BTSkillValue;
 val->skill = skNum;
 val->value = value;
 val->uses = uses;
 skill.push_back(val);
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

void BTPc::takeItemCharge(int index, int amount /*= 1*/)
{
 if (item[index].id == BTITEM_NONE)
  return;
 if ((item[index].charges == 0) || (item[index].charges == BTTIMESUSABLE_UNLIMITED))
  return;
 item[index].charges -= amount;
 if (item[index].charges <= 0)
 {
  BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
  if (itemList[item[index].id].isConsumed())
  {
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
  }
  else
  {
   item[index].charges = 0;
  }
 }
}

bool BTPc::takeSP(int amount)
{
 if (sp > 0)
 {
  if (sp > amount)
   sp -= amount;
  else
   sp = 0;
  return true;
 }
 else
  return false;
}

void BTPc::unequip(int index)
{
 BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
 ac -= itemList[item[index].id].getArmorPlus();
 toHit -= itemList[item[index].id].getHitPlus();
 item[index].equipped = BTITEM_NOTEQUIPPED;
}

void BTPc::useAutoCombatSkill(bool melee, BitField &special)
{
 BTGame *game = BTGame::getGame();
 BTSkillList &skillList = game->getSkillList();
 for (int i = 0; i < skillList.size(); ++i)
 {
  if ((skillList[i]->use == BTSKILLUSE_AUTOCOMBAT) ||
   ((melee) && (skillList[i]->use == BTSKILLUSE_AUTOCOMBATMELEE)) ||
   ((!melee) && (skillList[i]->use == BTSKILLUSE_AUTOCOMBATRANGED)))
  {
   if (((-1 == skillList[i]->after) || (combat.skillUsed == skillList[i]->after)) && (useSkill(i)))
   {
    special.set(skillList[i]->effect);
   }
  }
 }
}

bool BTPc::useSkill(int index, int difficulty /*= BTSKILL_DEFAULTDIFFICULTY*/)
{
 BTSkillList &skillList = BTGame::getGame()->getSkillList();
 if (difficulty == BTSKILL_DEFAULTDIFFICULTY)
  difficulty = skillList[index]->defaultDifficulty;
 for (int i = 0; i < skill.size(); ++i)
 {
  if (skill[i]->skill == index)
  {
   if (0 < skill[i]->value)
   {
    if (skillList[index]->limited)
    {
     if (skill[i]->uses > 0)
      --(skill[i]->uses);
     else
      return false;
    }
    BTDice *dice = skillList[index]->getRoll(skill[i]->value);
    if (dice)
    {
     int roll = dice->roll();
     if ((roll != dice->getMin()) && (roll + skill[i]->value >= difficulty))
     {
      return true;
     }
    }
   }
   return false;
  }
 }
 return false;
}

void BTPc::updateSkills()
{
 BTJobList &jobList = BTGame::getGame()->getJobList();
 for (int i = 0; i < jobList[job]->skill.size(); ++i)
 {
  int value = jobList[job]->skill[i]->value;
  if ((jobList[job]->skill[i]->modifier >= 0) && (stat[jobList[job]->skill[i]->modifier] > 14))
   value += stat[jobList[job]->skill[i]->modifier] - 14;
  if (getSkill(jobList[job]->skill[i]->skill) == 0)
  {
   setSkill(jobList[job]->skill[i]->skill, value, value);
   if (jobList[job]->skill[i]->improve > 0)
   {
    for (int sk = 0; sk < skill.size(); ++sk)
    {
     if (skill[sk]->skill == jobList[job]->skill[i]->skill)
     {
      for (int k = 2; k <= level; ++k)
      {
       if ((k % jobList[job]->skill[i]->improveLevel) == 0)
       {
        unsigned int increase = BTDice(1, jobList[job]->skill[i]->improve).roll();
        if ((jobList[job]->skill[i]->modifier >= 0) && (stat[jobList[job]->skill[i]->modifier] > 14))
         increase += stat[jobList[job]->skill[i]->modifier] - 14;
        skill[sk]->value += increase;
        skill[sk]->history.push_back(increase);
       }
      }
      break;
     }
    }
   }
  }
 }
}

void BTPc::youth()
{
 for (int i = 0; i < BT_STATS; ++i)
 {
  if (stat[i] < statMax[i])
  {
   if (statMax[i] > 14)
   {
    int difference = (statMax[i] - 14) - ((stat[i] > 14) ? (stat[i] - 14) : 0);
    if (i == BTSTAT_DX)
     ac += difference;
    if (i == BTSTAT_LK)
     save += difference;
   }
   stat[i] = statMax[i];
  }
 }
 status.clear(BTSTATUS_AGED);
}

void BTPc::readXML(const char *filename, XMLVector<BTGroup*> &group, XMLVector<BTPc*> &pc)
{
 XMLSerializer parser;
 parser.add("party", &group, &BTGroup::create);
 parser.add("pc", &pc, &BTPc::create);
 parser.parse(filename, true);
 for (int i = 0; i < pc.size(); ++i)
  pc[i]->updateSkills();
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
   operator[](who)->initiative = BTINITIATIVE_INACTIVE;
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
   pc->initiative = BTINITIATIVE_INACTIVE;
   erase(begin() + who);
   push_back(pc);
   --restDead;
  }
  else
  {
   operator[](who)->initiative = BTINITIATIVE_ACTIVE;
   ++who;
  }
 }
 int restStoned = restDead;
 for (who = restStoned - 1; who >= 0; --who)
 {
  if (operator[](who)->status.isSet(BTSTATUS_STONED))
  {
   operator[](who)->initiative = BTINITIATIVE_INACTIVE;
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
   pc->initiative = BTINITIATIVE_INACTIVE;
   erase(begin() + who);
   if (size() == restStoned - 1)
    push_back(pc);
   else
    insert(begin() + restStoned, pc);
   --restStoned;
  }
  else
  {
   operator[](who)->initiative = BTINITIATIVE_ACTIVE;
   ++who;
  }
 }
 return false;
}

void BTParty::giveItem(int itemID, BTDisplay &d)
{
 BTGame *game = BTGame::getGame();
 int who = 0;
 int charges = game->getItemList()[itemID].getTimesUsable();
 for (; who < size(); ++who)
 {
  if ((*this)[who]->giveItem(itemID, true, charges))
   break;
 }
 char tmp[100];
 if (who < size())
 {
  snprintf(tmp, 100, "%s gets %s.", (*this)[who]->name, game->getItemList()[itemID].getName().c_str());
 }
 else
 {
  snprintf(tmp, 100, "No one has room for %s!", game->getItemList()[itemID].getName().c_str());
 }
 d.drawText(tmp);
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

BTCombatant* BTParty::at(size_t index)
{
 return operator[](index);
}

int BTParty::getDistance()
{
 return 0;
}

size_t BTParty::size()
{
 return XMLVector<BTPc*>::size();
}

BTStatusLookup BTStatusLookup::lookup;
char *BTStatusLookup::value[8] = { "dead", "poisoned", "insane", "aged", "possessed", "stoned", "paralyzed", "npc" };

