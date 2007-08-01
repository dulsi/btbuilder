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

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTStatBlock; }

 public:
  char *attribute;
  SerialRect position;
  int modifier;
  bool negate;
  int maxValue;
  char *overflow;
  /*BTDisplay::alignment*/int align;
};

class BTPrint : public BTStatusInfo
{
 public:
  BTPrint() : text(0), align(0) {}

  virtual void draw(BTDisplay &d, int x, int y, ObjectSerializer *pc);
  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTPrint; }

 public:
  char *text;
  SerialRect position;
  /*BTDisplay::alignment*/int align;
};

class BTCondition : public XMLObject
{
 public:
  virtual bool compare(ObjectSerializer *pc) const;
  virtual void serialize(ObjectSerializer* s);
  void draw(BTDisplay &d, int x, int y, ObjectSerializer *pc);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTCondition; }

 private:
  XMLVector<BTStatusInfo*> info;
};

class BTCheckBit : public BTCondition
{
 public:
  BTCheckBit() : attribute(0) {}

  virtual bool compare(ObjectSerializer *pc) const;
  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTCheckBit; }

 private:
  char *attribute;
  int bit;
};

class BTConditional : public BTStatusInfo
{
 public:
  BTConditional() {}

  virtual void draw(BTDisplay &d, int x, int y, ObjectSerializer *pc);
  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTConditional; }

 public:
  XMLVector<BTCondition*> condition;
};

class BTAlignmentLookup : public ArrayLookup
{
 public:
  BTAlignmentLookup() : ArrayLookup(3, value) {}

  static BTAlignmentLookup lookup;

 private:
  static char *value[3];
};

#endif
