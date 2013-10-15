/*-------------------------------------------------------------------------*\
  <race.C> -- Race implementation file

  Date      Programmer  Description
  02/11/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "race.h"
#include <stdio.h>

void BTRace::serialize(ObjectSerializer* s)
{
 s->add("name", &name);
 for (int i = 0; i < BT_STATS; ++i)
 {
  std::vector<XMLAttribute> *attrib = new std::vector<XMLAttribute>;
  char tmp[10];
  snprintf(tmp, 10, "%d", i + 1);
  attrib->push_back(XMLAttribute("number", tmp));
  s->add("stat", &stat[i], attrib);
 }
}

void BTRace::readXML(const char *filename, XMLVector<BTRace*> &race)
{
 XMLSerializer parser;
 parser.add("race", &race, &BTRace::create);
 parser.parse(filename, true);
}

std::string BTRaceList::getName(int index)
{
 if ((size() > index) && (index >= 0))
  return operator[](index)->name;
 else
  return "---";
}

int BTRaceList::getIndex(std::string name)
{
 for (int i = 0; i < size(); ++i)
  if (strcmp(name.c_str(), operator[](i)->name) == 0)
   return i;
 return -1;
}

size_t BTRaceList::size()
{
 return XMLVector<BTRace*>::size();
}

