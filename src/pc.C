/*-------------------------------------------------------------------------*\
  <pc.C> -- Player character implementation file

  Date      Programmer  Description
  03/11/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "pc.h"

void BTEquipment::serialize(XMLSerializer* s)
{
 s->add("id", &id);
 s->add("equipped", &equipped);
 s->add("known", &known);
 s->add("charges", &charges);
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

void BTPc::giveXP(int amount)
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

void BTPc::serialize(XMLSerializer* s)
{
 s->add("name", &name);
 s->add("race", &race);
 s->add("job", &job);
 for (int i = 0; i < BT_STATS; ++i)
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
 s->add("gold", &gold);
 s->add("xp", &xp);
}

void BTPc::setName(const char *nm)
{
 delete [] name;
 name = new char[strlen(nm) + 1];
 strcpy(name, nm);
}

int BTPc::takeGold(int amount)
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

void BTPc::readXML(const char *filename, XMLVector<BTPc*> &pc)
{
 XMLSerializer parser;
 parser.add("pc", &BTPc::create, &pc);
 parser.parse(filename, true);
}
