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

  void buildMatchString();
  bool match(std::vector<std::string> words);

  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTImageTag; }
  static void readXML(const char *filename, XMLVector<BTImageTag*> &tag);

  std::string name;
  std::string artist;
  std::string matchString;
};

class BTImageTagList : public XMLVector<BTImageTag*>
{
 public:
  BTImageTagList() : last(NULL) {}
  ~BTImageTagList() { if (last) delete [] last; }

  BTDisplay::selectItem *search(std::string words, bool blank, int &current, int &sz, int &sel);

 private:
  BTDisplay::selectItem *last;
};
#endif
