#ifndef __EXPATCPP_H
#define __EXPATCPP_H
/*-------------------------------------------------------------------------*\
  <expatcpp.h> -- Expat C++ header file

  Date      Programmer  Description
  01/27/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include <expat.h>

class ExpatXMLParser
{
 public:
  ExpatXMLParser();
  ~ExpatXMLParser() { }

  virtual void startElement(const XML_Char *name, const XML_Char **atts) = 0;
  virtual void endElement(const XML_Char *name) = 0;
  virtual void characterData(const XML_Char *s, int len) = 0;

  void parse(const char *filename, bool physfs);

 private:
  static void startElement(void *userData, const XML_Char *name, const XML_Char **atts);
  static void endElement(void *userData, const XML_Char *name);
  static void characterData(void *userData, const XML_Char *s, int len);
};

#endif
