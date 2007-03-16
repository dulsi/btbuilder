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
