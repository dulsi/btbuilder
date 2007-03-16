/*-------------------------------------------------------------------------*\
  <race.C> -- Race implementation file

  Date      Programmer  Description
  02/11/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "race.h"

void BTRace::serialize(XMLSerializer* s)
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
 parser.add("race", &BTRace::create, &race);
 parser.parse(filename);
}
