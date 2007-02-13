#ifndef __BTRACE_H
#define __BTRACE_H
/*-------------------------------------------------------------------------*\
  <race.h> -- Race header file

  Date      Programmer  Description
  02/11/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

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

  virtual void serialize(XMLSerializer* s);

  static XMLObject *create() { return new BTRace; }
  static void readXML(const char *filename, XMLVector<BTRace*> &race);

  char *name;
};

#endif
