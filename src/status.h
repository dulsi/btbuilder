#ifndef __STATUS_H
#define __STATUS_H
/*-------------------------------------------------------------------------*\
  <status.h> -- Status header file

  Date      Programmer  Description
  03/27/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "xmlserializer.h"
#include "display.h"
#include "pc.h"

class BTStatus : public ExpatXMLParser, public ObjectSerializer
{
 public:
  BTStatus(BTDisplay &d, BTPc *c);
  ~BTStatus();

  virtual int getLevel();

  virtual void startElement(const XML_Char *name, const XML_Char **atts);
  virtual void endElement(const XML_Char *name);
  virtual void characterData(const XML_Char *s, int len);

 private:
  BTDisplay &display;
  int level;
  std::list<std::string> line;
};

#endif
