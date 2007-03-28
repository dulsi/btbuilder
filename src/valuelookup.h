#ifndef __VALUELOOKUP_H
#define __VALUELOOKUP_H
/*-------------------------------------------------------------------------*\
  <valuelookup.h> -- ValueLookup header file

  Date      Programmer  Description
  03/24/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include <string>

class ValueLookup
{
 public:
  virtual std::string getName(int index) = 0;
  virtual int getIndex(std::string name) = 0;
};

#endif
