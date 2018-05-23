/*-------------------------------------------------------------------------*\
  <duration.C> -- Duration implementation file

  Date      Programmer  Description
  05/22/18  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "game.h"
#include "duration.h"

const std::string &BTDuration::getName() const
{
 return name;
}

unsigned int BTDuration::duration(int userLevel)
{
 unsigned int expire;
 switch(type)
 {
  case BTDURATIONTYPE_COMBAT:
   expire = BTTIME_COMBAT;
   break;
  case BTDURATIONTYPE_PERMANENT:
   expire = BTTIME_PERMANENT;
   break;
  case BTDURATIONTYPE_CONTINUOUS:
   expire = BTTIME_CONTINUOUS;
   break;
  case BTDURATIONTYPE_INDEFINITE:
   expire = BTTIME_INDEFINITE;
   break;
  default:
   expire = baseTime.roll();
   if (level > 0)
   {
    for (int i = userLevel / level; i > 0; i--)
     expire += levelTime.roll();
   }
   break;
 }
 return expire;
}

void BTDuration::serialize(ObjectSerializer* s)
{
 s->add("name", &name);
 s->add("type", &type, NULL, &durationTypeLookup);
 s->add("baseTime", &baseTime);
 s->add("level", &level);
 s->add("levelTime", &levelTime);
}

void BTDuration::readXML(const char *filename, XMLVector<BTDuration*> &dur)
{
 XMLSerializer parser;
 parser.add("duration", &dur, &BTDuration::create);
 parser.parse(filename, true);
}

void BTDuration::writeXML(const char *filename, XMLVector<BTDuration*> &dur)
{
 XMLSerializer parser;
 parser.add("duration", &dur, &BTDuration::create);
 parser.write(filename, true);
}

std::string BTDurationList::getName(int index)
{
 if (size() > index)
  return operator[](index)->name;
 else
  return "";
}

int BTDurationList::getIndex(std::string name)
{
 for (int i = 0; i < size(); ++i)
  if (name == operator[](i)->name)
   return i;
 return -1;
}

size_t BTDurationList::size()
{
 return XMLVector<BTDuration*>::size();
}

int BTDurationListCompare::Compare(const BTDuration &a, const BTDuration &b) const
{
 return strcmp(a.getName().c_str(), b.getName().c_str());
}
