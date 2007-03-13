#ifndef __PC_H
#define __PC_H
/*-------------------------------------------------------------------------*\
  <pc.h> -- Player character header file

  Date      Programmer  Description
  03/11/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "xmlserializer.h"

class BTPc : public XMLObject
{
 public:
  BTPc()
  {
   name = new char[1];
   name[0] = 0;
  }

  ~BTPc() { delete [] name; }

  virtual void serialize(XMLSerializer* s);

  static XMLObject *create() { return new BTPc; }
  static void readXML(const char *filename, XMLVector<BTPc*> &pc);

  char *name;
  int race;
  int job;
  int stat[BT_STATS];
};

#endif
