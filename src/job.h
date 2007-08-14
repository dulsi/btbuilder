#ifndef __JOB_H
#define __JOB_H
/*-------------------------------------------------------------------------*\
  <job.h> -- Job header file

  Date      Programmer  Description
  01/26/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "xmlserializer.h"
#include "pc.h"

class BTJob : public XMLObject
{
 public:
  BTJob()
   : picture(-1), toHit(0), improveToHit(0), improveRateAttacks(0), maxRateAttacks(0), save(0), improveSave(0), ac(0), improveAc(0), hp(0), xpChart(-1), spells(false), advanced(false)
  {
   name = new char[1];
   name[0] = 0;
   abbrev = new char[1];
   abbrev[0] = 0;
  }

  ~BTJob() { delete [] name; delete [] abbrev; }

  bool isAllowed(BTPc *pc, bool starting);
  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTJob; }
  static void readXML(const char *filename, XMLVector<BTJob*> &job);

  char *name;
  char *abbrev;
  int picture;
  int toHit;
  int improveToHit;
  int improveRateAttacks;
  int maxRateAttacks;
  int save;
  int improveSave;
  int ac;
  int improveAc;
  int hp;
  int xpChart;
  bool spells;
  bool advanced;
  BitField allowedRace;
};

class BTJobList : public ValueLookup, public XMLVector<BTJob*>
{
 public:
  virtual std::string getName(int index);
  virtual int getIndex(std::string name);
};

#endif
