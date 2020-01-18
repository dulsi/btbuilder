#ifndef __FLAGNAME_H
#define __FLAGNAME_H
/*-------------------------------------------------------------------------*\
  <flagname.h> -- Flag name header file

  Date      Programmer  Description
  01/06/20  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "xmlserializer.h"
#include "display.h"

class BTFlagName : public XMLObject
{
 public:
  BTFlagName();

  void buildMatchString();
  bool match(std::vector<std::string> words);

  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTFlagName; }
  static void readXML(const char *filename, XMLVector<BTFlagName*> &tag);
  static void writeXML(const char *filename, XMLVector<BTFlagName*> &tag);

  int slot;
  std::string name;
  std::string matchString;
};

class BTFlagNameList : public XMLVector<BTFlagName*>
{
 public:
  BTFlagNameList() : last(NULL) {}
  ~BTFlagNameList() { if (last) delete [] last; }

  BTDisplay::selectItem *search(std::string words, bool blank, int &current, int &sz, int &sel);

 private:
  BTDisplay::selectItem *last;
};
#endif
