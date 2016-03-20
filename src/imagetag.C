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

void BTImageTag::buildMatchString()
{
 for (int i = 0; i < name.size(); i++)
  matchString += std::toupper(name[i]);
 matchString += " ";
 for (int i = 0; i < artist.size(); i++)
  matchString += std::toupper(artist[i]);
}

bool BTImageTag::match(std::vector<std::string> words)
{
 bool foundAll = true;
 for (int i = 0; i < words.size(); ++i)
 {
  if (matchString.find(words[i]) == std::string::npos)
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
 for (int i = 0; i < tag.size(); i++)
  tag[i]->buildMatchString();
}

BTDisplay::selectItem *BTImageTagList::search(std::string words, bool blank, int &current, int &sz, int &sel)
{
 if (!last)
  last = new BTDisplay::selectItem[size()];
 bool found = false;
 int full = size();
 sz = 0;
 if (words == "")
 {
  for (int i = 0; i < full; i++)
  {
   if (((*this)[i]->artist == "") && (!blank))
    continue;
   if (((*this)[i]->name != "") || (i == current))
   {
    if (i == current)
    {
     found = true;
     sel = sz;
    }
    last[sz].name = (*this)[i]->name;
    last[sz].value = i;
    sz++;
   }
  }
 }
 else
 {
  std::stringstream ss(words);
  std::vector<std::string> tokens;
  std::string buf;

  while (ss >> buf)
  {
   for (int i = 0; i < buf.size(); i++)
    buf[i] = std::toupper(buf[i]);
   tokens.push_back(buf);
  }

  for (int i = 0; i < full; i++)
  {
   if (((*this)[i]->artist == "") && (!blank))
    continue;
   if (((*this)[i]->match(tokens)) && ((*this)[i]->name != ""))
   {
    if (i == current)
    {
     found = true;
     sel = sz;
    }
    last[sz].name = (*this)[i]->name;
    last[sz].value = i;
    sz++;
   }
  }
 }
 if (!found)
 {
  if (sz > 0)
   current = last[0].value;
  sel = 0;
 }
 return last;
}
