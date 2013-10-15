/*-------------------------------------------------------------------------*\
  <xpchart.C> -- XpChart implementation file

  Date      Programmer  Description
  08/12/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "xpchart.h"

unsigned int BTXpChart::getXpNeeded(int level)
{
 if (level <= xp.size())
  return xp[level - 1];
 else
  return xp.back() + ((level - xp.size()) * beyond);
}

void BTXpChart::serialize(ObjectSerializer* s)
{
 s->add("name", &name);
 s->add("xp", &xp);
 s->add("beyond", &beyond);
}

void BTXpChart::readXML(const char *filename, XMLVector<BTXpChart*> &xpChart)
{
 XMLSerializer parser;
 parser.add("xpChart", &xpChart, &BTXpChart::create);
 parser.parse(filename, true);
}

std::string BTXpChartList::getName(int index)
{
 if (size() > index)
  return operator[](index)->name;
 else
  return "";
}

int BTXpChartList::getIndex(std::string name)
{
 for (int i = 0; i < size(); ++i)
  if (name == operator[](i)->name)
   return i;
 return -1;
}

size_t BTXpChartList::size()
{
 return XMLVector<BTXpChart*>::size();
}

