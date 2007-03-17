#ifndef __JOB_H
#define __JOB_H
/*-------------------------------------------------------------------------*\
  <job.h> -- Job header file

  Date      Programmer  Description
  01/26/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "xmlserializer.h"

class BTJob : public XMLObject
{
 public:
  BTJob()
   : thac0(0), improveThac0(0), save(0), improveSave(0), hp(0), advanced(false)
  {
   name = new char[1];
   name[0] = 0;
   abbrev = new char[1];
   abbrev[0] = 0;
  }

  ~BTJob() { delete [] name; delete [] abbrev; }

  virtual void serialize(XMLSerializer* s);

  static XMLObject *create() { return new BTJob; }
  static void readXML(const char *filename, XMLVector<BTJob*> &job);

  char *name;
  char *abbrev;
  int thac0;
  int improveThac0;
  int save;
  int improveSave;
  int hp;
  bool advanced;
};

#endif
