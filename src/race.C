/*-------------------------------------------------------------------------*\
  <race.C> -- Race implementation file

  Date      Programmer  Description
  02/11/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "race.h"

void BTRace::serialize(XMLSerializer* s)
{
 s->add("race", &name);
}

void BTRace::readXML(const char *filename, XMLVector<BTRace*> &race)
{
 XMLSerializer parser;
 parser.add("race", &BTRace::create, &race);
 parser.parse(filename);
}
