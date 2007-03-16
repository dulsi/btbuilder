#ifndef __PC_H
#define __PC_H
/*-------------------------------------------------------------------------*\
  <pc.h> -- Player character header file

  Date      Programmer  Description
  03/11/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "xmlserializer.h"

class BTEquipment : public XMLObject
{
 public:
  virtual void serialize(XMLSerializer* s);

  int type;
  bool equiped, known;
  int charges;
};

class BTPc : public XMLObject
{
 public:
  BTPc()
   : ac(0), sp(0), maxSp(0)
  {
   name = new char[1];
   name[0] = 0;
  }

  ~BTPc() { delete [] name; }

  virtual void serialize(XMLSerializer* s);
  void setName(const char *nm);

  static XMLObject *create() { return new BTPc; }
  static void readXML(const char *filename, XMLVector<BTPc*> &pc);

  char *name;
  int race;
  int job;
  int stat[BT_STATS];
  int ac;
  int hp, maxHp;
  int sp, maxSp;
  int gold;
  int xp;
  BTEquipment item[BT_ITEMS];
};

#endif
