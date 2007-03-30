/*-------------------------------------------------------------------------*\
  <pc.C> -- Player character implementation file

  Date      Programmer  Description
  03/11/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "pc.h"
#include "game.h"

void BTEquipment::serialize(ObjectSerializer* s)
{
 s->add("id", &id);
 s->add("equipped", &equipped);
 s->add("known", &known);
 s->add("charges", &charges);
}

BTPc::BTPc()
 : race(0), job(0), picture(-1), monster(-1), ac(0), hp(0), maxHp(0),  sp(0), maxSp(0), level(1), gold(0), xp(0)
{
 name = new char[1];
 name[0] = 0;
 int i;
 for (i = 0; i < BT_STATS; ++i)
  stat[i] = 10;
 int jobs = BTGame::getGame()->getJobList().size();
 spellLvl = new int[jobs];
 for (i = 0; i < jobs; ++i)
  spellLvl[i] = 0;
}

void BTPc::equip(int index)
{
 BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
 int type = itemList[item[index].id].getType();
 for (int i = 0; i < BT_ITEMS; ++i)
 {
  if (BTITEM_NONE == item[i].id)
   break;
  if ((item[i].equipped) && (type == itemList[item[i].id].getType()))
  {
   unequip(i);
  }
 }
 ac += itemList[item[index].id].getArmorPlus();
 item[index].equipped = true;
}

bool BTPc::isAlive() const
{
 return hp > 0;
}

bool BTPc::isEquipped(int index) const
{
 return item[index].equipped;
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

int BTPc::getItem(int index) const
{
 return item[index].id;
}

bool BTPc::giveItem(int id, bool known, int charges)
{
 for (int i = 0; i < BT_ITEMS; ++i)
 {
  if (BTITEM_NONE == item[i].id)
  {
   item[i].id = id;
   item[i].equipped = false;
   item[i].known = known;
   item[i].charges = charges;
   return true;
  }
 }
 return false;
}

unsigned int BTPc::giveGold(unsigned int amount)
{
 gold += amount;
 if (gold > 4000000000UL)
  gold = 4000000000UL;
}

void BTPc::giveXP(unsigned int amount)
{
 xp += amount;
 if (xp > 4000000000UL)
  xp = 4000000000UL;
}

bool BTPc::hasItem(int id)
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

void BTPc::serialize(ObjectSerializer* s)
{
 int i;
 s->add("name", &name);
 s->add("race", &race, NULL, &BTGame::getGame()->getRaceList());
 s->add("job", &job, NULL, &BTGame::getGame()->getJobList());
 s->add("picture", &race);
 s->add("monster", &job);
 for (i = 0; i < BT_STATS; ++i)
 {
  std::vector<XMLAttribute> *attrib = new std::vector<XMLAttribute>;
  char tmp[10];
  snprintf(tmp, 10, "%d", i + 1);
  attrib->push_back(XMLAttribute("number", tmp));
  s->add("stat", &stat[i], attrib);
 }
 s->add("ac", &ac);
 s->add("maxhp", &maxHp);
 s->add("hp", &hp);
 s->add("maxsp", &maxSp);
 s->add("sp", &sp);
 s->add("level", &level);
 s->add("xp", &xp);
 s->add("gold", &gold);
 int jobs = BTGame::getGame()->getJobList().size();
 for (i = 0; i < jobs; ++i)
 {
  std::vector<XMLAttribute> *attrib = new std::vector<XMLAttribute>;
  char tmp[10];
  snprintf(tmp, 10, "%d", i + 1);
  attrib->push_back(XMLAttribute("job", tmp));
  s->add("spellLvl", &spellLvl[i], attrib);
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
 bool found = false;
 for (int i = 0; i < BT_ITEMS; ++i)
 {
  if (found)
  {
   item[i - 1].id = item[i].id;
   item[i - 1].equipped = item[i].equipped;
   item[i - 1].known = item[i].known;
   item[i - 1].charges = item[i].charges;
  }
  else if (id == item[i].id)
  {
   found = true;
  }
 }
 if (found)
  item[BT_ITEMS - 1].id = BTITEM_NONE;
 return found;
}

void BTPc::unequip(int index)
{
 BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
 ac -= itemList[item[index].id].getArmorPlus();
 item[index].equipped = false;
}

void BTPc::readXML(const char *filename, XMLVector<BTPc*> &pc)
{
 XMLSerializer parser;
 parser.add("pc", &pc, &BTPc::create);
 parser.parse(filename, true);
}
