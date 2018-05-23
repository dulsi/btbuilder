#ifndef __DURATION_H
#define __DURATION_H
/*-------------------------------------------------------------------------*\
  <duration.h> -- Duration header file

  Date      Programmer  Description
  05/22/18  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "dice.h"
#include "factory.h"
#include "xmlserializer.h"

class BTDuration : public XMLObject
{
 public:
  BTDuration() : type(0), level(0)
  {
  }

  ~BTDuration() { }

  const std::string &getName() const;

  unsigned int duration(int userLevel);
  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTDuration; }
  static void readXML(const char *filename, XMLVector<BTDuration*> &dur);
  static void writeXML(const char *filename, XMLVector<BTDuration*> &dur);

  std::string name;
  int type;
  BTDice baseTime;
  int level;
  BTDice levelTime;
};

class BTDurationList : public ValueLookup, public XMLVector<BTDuration*>
{
 public:
  virtual std::string getName(int index);
  virtual int getIndex(std::string name);
  virtual size_t size();
};

class BTDurationListCompare : public BTSortCompare<BTDuration>
{
 public:
  int Compare(const BTDuration &a, const BTDuration &b) const;
};

#endif

