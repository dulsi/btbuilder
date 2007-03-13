/*-------------------------------------------------------------------------*\
  <pc.C> -- Player character implementation file

  Date      Programmer  Description
  03/11/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "pc.h"

void BTPc::serialize(XMLSerializer* s)
{
 s->add("name", &name);
}

void BTPc::readXML(const char *filename, XMLVector<BTPc*> &pc)
{
 XMLSerializer parser;
 parser.add("pc", &BTPc::create, &pc);
 parser.parse(filename);
}
