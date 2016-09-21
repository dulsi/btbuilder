/*-------------------------------------------------------------------------*\
  <specialflag.C> -- Special flag implementation file

  Date      Programmer  Description
  08/04/15  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "game.h"
#include "specialflag.h"

const std::string &BTSpecialFlag::getName() const
{
 return name;
}

void BTSpecialFlag::serialize(ObjectSerializer* s)
{
 s->add("name", &name);
 s->add("detect", &detect);
}

void BTSpecialFlag::readXML(const char *filename, XMLVector<BTSpecialFlag*> &flag)
{
 XMLSerializer parser;
 parser.add("specialFlag", &flag, &BTSpecialFlag::create);
 parser.parse(filename, true);
}

std::string BTSpecialFlagList::getName(int index)
{
 if ((size() > index) && (index >= 0))
  return operator[](index)->getName();
 else
  return "---";
}

int BTSpecialFlagList::getIndex(std::string name)
{
 for (size_t i = 0; i < size(); ++i)
  if (strcmp(name.c_str(), operator[](i)->getName().c_str()) == 0)
   return i;
 return -1;
}

size_t BTSpecialFlagList::size()
{
 return XMLVector<BTSpecialFlag*>::size();
}

