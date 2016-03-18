/*-------------------------------------------------------------------------*\
  <imagetag.C> -- Image tag implementation file

  Date      Programmer  Description
  03/16/16  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "imagetag.h"
#include <sstream>

BTImageTag::BTImageTag()
{
}

bool BTImageTag::match(std::vector<std::string> words)
{
 bool foundAll = true;
 for (int i = 0; i < words.size(); ++i)
 {
  if ((name.find(words[i]) == std::string::npos) && (artist.find(words[i]) == std::string::npos))
   foundAll = false;
 }
 return foundAll;
}

void BTImageTag::serialize(ObjectSerializer* s)
{
 s->add("name", &name);
 s->add("artist", &artist);
}

void BTImageTag::readXML(const char *filename, XMLVector<BTImageTag*> &tag)
{
 XMLSerializer parser;
 parser.add("imageTag", &tag, &BTImageTag::create);
 parser.parse(filename, true);
}

BTDisplay::selectItem *BTImageTagList::search(std::string words, int &sz)
{
 if (!last)
  last = new BTDisplay::selectItem[size()];
 if (words == "")
 {
  sz = size();
  for (int i = 0; i < sz; i++)
  {
   last[i].name = (*this)[i]->name;
   last[i].value = i;
  }
 }
 else
 {
  std::stringstream ss(words);
  std::vector<std::string> tokens;
  std::string buf;

  while (ss >> buf)
  {
   tokens.push_back(buf);
  }

  int full = size();
  sz = 0;
  for (int i = 0; i < full; i++)
  {
   if ((*this)[i]->match(tokens))
   {
    last[sz].name = (*this)[i]->name;
    last[sz].value = i;
    sz++;
   }
  }
 }
 return last;
}
