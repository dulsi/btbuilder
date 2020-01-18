/*-------------------------------------------------------------------------*\
  <flagname.C> -- Flag name implementation file

  Date      Programmer  Description
  01/06/20  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "flagname.h"
#include <sstream>

BTFlagName::BTFlagName()
 : slot(-1)
{
}

void BTFlagName::buildMatchString()
{
 for (size_t i = 0; i < name.size(); i++)
  matchString += std::toupper(name[i]);
}

bool BTFlagName::match(std::vector<std::string> words)
{
 bool foundAll = true;
 for (size_t i = 0; i < words.size(); ++i)
 {
  if (matchString.find(words[i]) == std::string::npos)
   foundAll = false;
 }
 return foundAll;
}

void BTFlagName::serialize(ObjectSerializer* s)
{
 s->add("slot", &slot);
 s->add("name", &name);
}

void BTFlagName::readXML(const char *filename, XMLVector<BTFlagName*> &tag)
{
 XMLSerializer parser;
 XMLVector<BTFlagName*> tagNew(false);
 parser.add("flag", &tagNew, &BTFlagName::create);
 parser.parse(filename, true);
 for (auto itr : tagNew)
 {
  if (itr->slot == -1)
  {
   itr->slot = tag.size();
   tag.push_back(itr);
  }
  else
  {
   if (tag.size() <= itr->slot)
   {
    while (tag.size() < itr->slot)
    {
     tag.push_back(new BTFlagName);
     tag.back()->slot = tag.size()-1;
    }
    tag.push_back(itr);
   }
   else
   {
    delete tag[itr->slot];
    tag[itr->slot] = itr;
   }
  }
 }
 for (size_t i = 0; i < tag.size(); i++)
  tag[i]->buildMatchString();
}

void BTFlagName::writeXML(const char *filename, XMLVector<BTFlagName*> &tag)
{
 XMLSerializer parser;
 parser.add("flag", &tag, &BTFlagName::create);
 parser.write(filename, true);
}

BTDisplay::selectItem *BTFlagNameList::search(std::string words, bool blank, int &current, int &sz, int &sel)
{
 if (last)
  delete [] last;
 last = new BTDisplay::selectItem[size()];
 bool found = false;
 size_t full = size();
 sz = 0;
 if (words == "")
 {
  for (size_t i = 0; i < full; i++)
  {
   if (((*this)[i]->name == "") && (!blank))
    continue;
//   if (((*this)[i]->name != "") || (i == current))
   {
    if (i == current)
    {
     found = true;
     sel = sz;
    }
    last[sz].name = (*this)[i]->name;
    last[sz].value = i;
    last[sz].flags.set(BTSELECTFLAG_SHOWVALUE);
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
   for (size_t i = 0; i < buf.size(); i++)
    buf[i] = std::toupper(buf[i]);
   tokens.push_back(buf);
  }

  for (size_t i = 0; i < full; i++)
  {
   if (((*this)[i]->name == "") && (!blank))
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
    last[sz].flags.set(BTSELECTFLAG_SHOWVALUE);
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
