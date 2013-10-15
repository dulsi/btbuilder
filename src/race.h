#ifndef __RACE_H
#define __RACE_H
/*-------------------------------------------------------------------------*\
  <race.h> -- Race header file

  Date      Programmer  Description
  02/11/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "dice.h"
#include "xmlserializer.h"

class BTRace : public XMLObject
{
 public:
  BTRace()
  {
   name = new char[1];
   name[0] = 0;
  }

  ~BTRace() { delete [] name; }

  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTRace; }
  static void readXML(const char *filename, XMLVector<BTRace*> &race);

  char *name;
  BTDice stat[BT_STATS];
};

class BTRaceList : public ValueLookup, public XMLVector<BTRace*>
{
 public:
  virtual std::string getName(int index);
  virtual int getIndex(std::string name);
  virtual size_t size();
};

#endif
