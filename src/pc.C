/*-------------------------------------------------------------------------*\
  <pc.C> -- Player character implementation file

  Date      Programmer  Description
  03/11/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "pc.h"

void BTEquipment::serialize(XMLSerializer* s)
{
 s->add("type", &type);
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
 if (BTITEM_NONE == item[0].type)
  return true;
 else
  return false;
}

bool BTPc::isEquipmentFull() const
{
 if (BTITEM_NONE != item[BT_ITEMS - 1].type)
  return true;
 else
  return false;
}

int BTPc::getItem(int index) const
{
 return item[index].type;
}

bool BTPc::giveItem(int type, bool known, int charges)
{
 for (int i = 0; i < BT_ITEMS; ++i)
 {
  if (BTITEM_NONE == item[i].type)
  {
   item[i].type = type;
   item[i].equipped = false;
   item[i].known = known;
   item[i].charges = charges;
   return true;
  }
 }
 return false;
}

bool BTPc::takeItem(int type)
{
 bool found = false;
 for (int i = 0; i < BT_ITEMS; ++i)
 {
  if (found)
  {
   item[i - 1].type = item[i].type;
   item[i - 1].equipped = item[i].equipped;
   item[i - 1].known = item[i].known;
   item[i - 1].charges = item[i].charges;
  }
  else if (type == item[i].type)
  {
   found = true;
  }
 }
 if (found)
  item[BT_ITEMS - 1].type = BTITEM_NONE;
 return found;
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

void BTPc::readXML(const char *filename, XMLVector<BTPc*> &pc)
{
 XMLSerializer parser;
 parser.add("pc", &BTPc::create, &pc);
 parser.parse(filename);
}
