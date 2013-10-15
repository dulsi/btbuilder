#ifndef __XPCHART_H
#define __XPCHART_H
/*-------------------------------------------------------------------------*\
  <xpchart.h> -- XpChart header file

  Date      Programmer  Description
  08/12/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "xmlserializer.h"

class BTXpChart : public XMLObject
{
 public:
  BTXpChart()
   : beyond(0)
  {
  }

  unsigned int getXpNeeded(int level);
  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTXpChart; }
  static void readXML(const char *filename, XMLVector<BTXpChart*> &xpChart);

  std::string name;
  std::vector<unsigned int> xp;
  unsigned int beyond;
};

class BTXpChartList : public ValueLookup, public XMLVector<BTXpChart*>
{
 public:
  virtual std::string getName(int index);
  virtual int getIndex(std::string name);
  virtual size_t size();
};

#endif
