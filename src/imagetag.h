#ifndef __IMAGETAG_H
#define __IMAGETAG_H
/*-------------------------------------------------------------------------*\
  <imagetag.h> -- Image tag header file

  Date      Programmer  Description
  03/16/16  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "xmlserializer.h"
#include "display.h"

class BTImageTag : public XMLObject
{
 public:
  BTImageTag();

  bool match(std::vector<std::string> words);

  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTImageTag; }
  static void readXML(const char *filename, XMLVector<BTImageTag*> &tag);

  std::string name;
  std::string artist;
};

class BTImageTagList : public XMLVector<BTImageTag*>
{
 public:
  BTImageTagList() : last(NULL) {}

  BTDisplay::selectItem *search(std::string words, int &sz);

 private:
  BTDisplay::selectItem *last;
};
#endif
