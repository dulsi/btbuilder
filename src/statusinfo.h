#ifndef __STATUSINFO_H
#define __STATUSINFO_H
/*-------------------------------------------------------------------------*\
  <statusinfo.h> -- Status information header file

  Date      Programmer  Description
  04/02/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "xmlserializer.h"
#include "serialrect.h"

class BTDisplay;

class BTStatusInfo : public XMLObject
{
 public:
  BTStatusInfo() {}

  virtual void draw(BTDisplay &d, int x, int y, ObjectSerializer *pc) = 0;
};

class BTStatBlock : public BTStatusInfo
{
 public:
  BTStatBlock() : attribute(0), modifier(0), negate(false), maxValue(-1), overflow(0), align(0) {}

  virtual void draw(BTDisplay &d, int x, int y, ObjectSerializer *pc);
  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create() { return new BTStatBlock; }

 public:
  char *attribute;
  SerialRect position;
  int modifier;
  bool negate;
  int maxValue;
  char *overflow;
  /*BTDisplay::alignment*/int align;
};

class BTAlignmentLookup : public ValueLookup
{
 public:
  virtual std::string getName(int index);
  virtual int getIndex(std::string name);

  static BTAlignmentLookup lookup;

 private:
  static char *value[3];
};

#endif
