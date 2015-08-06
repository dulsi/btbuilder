#ifndef __SPECIALFLAG_H
#define __SPECIALFLAG_H
/*-------------------------------------------------------------------------*\
  <specialflag.h> -- Special flag header file

  Date      Programmer  Description
  08/04/15  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "dice.h"
#include "manifest.h"
#include "xmlserializer.h"

class BTSpecialFlag : public XMLObject
{
 public:
  BTSpecialFlag()
  {
  }

  ~BTSpecialFlag() { }

  const std::string &getName() const;

  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTSpecialFlag; }
  static void readXML(const char *filename, XMLVector<BTSpecialFlag*> &flag);

  std::string name;
  std::string detect;
};

class BTSpecialFlagList : public ValueLookup, public XMLVector<BTSpecialFlag*>
{
 public:
  virtual std::string getName(int index);
  virtual int getIndex(std::string name);
  virtual size_t size();
};

#endif

