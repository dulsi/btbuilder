/*-------------------------------------------------------------------------*\
  <xmlserializer.C> -- XMLSerializer implementation file

  Date      Programmer  Description
  01/28/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "xmlserializer.h"

XMLSerializer::XMLSerializer()
: state(0)
{
}

XMLSerializer::~XMLSerializer()
{
 for (std::vector<XMLAction*>::iterator itr(action.begin()); itr != action.end(); itr++)
  delete *itr;
 action.clear();
}

void XMLSerializer::add(const char *name, XMLObject::create func, XMLArray* vec, std::vector<XMLAttribute> *atts /*= NULL*/)
{
 XMLAction *act = new XMLAction;
 act->name = name;
 act->attrib = atts;
 act->type = XMLTYPE_CREATE;
 act->level = level.size();
 act->object = reinterpret_cast<void*>(vec);
 act->func = func;
 action.push_back(act);
}

void XMLSerializer::add(const char *name, int *p, std::vector<XMLAttribute> *atts /*= NULL*/)
{
 XMLAction *act = new XMLAction;
 act->name = name;
 act->attrib = atts;
 act->type = XMLTYPE_INT;
 act->level = level.size();
 act->object = reinterpret_cast<void*>(p);
 action.push_back(act);
}

void XMLSerializer::add(const char *name, char **p, std::vector<XMLAttribute> *atts /*= NULL*/)
{
 XMLAction *act = new XMLAction;
 act->name = name;
 act->attrib = atts;
 act->type = XMLTYPE_STRING;
 act->level = level.size();
 act->object = reinterpret_cast<void*>(p);
 action.push_back(act);
}

void XMLSerializer::startElement(const XML_Char *name, const XML_Char **atts)
{
 int curLevel = level.size();
 // Shouldn't already be in a state
 if (state)
  return;
 for (std::vector<XMLAction*>::reverse_iterator itr(action.rbegin()); (action.rend() != itr) && ((*itr)->level == curLevel); itr++)
 {
  if (0 == strcmp((*itr)->name.c_str(), name))
  {
   if ((*itr)->attrib)
   {
    bool found(false);
    for (std::vector<XMLAttribute>::iterator itrAttrib((*itr)->attrib->begin()); itrAttrib != (*itr)->attrib->end(); itrAttrib++)
    {
     found = false;
     for (int i = 0; atts[i]; i += 2)
     {
      if ((0 == strcmp(itrAttrib->name.c_str(), atts[i])) && (0 == strcmp(itrAttrib->value.c_str(), atts[i + 1])))
      {
       found = true;
       break;
      }
     }
     if (!found)
      break;
    }
    if (!found)
     continue;
   }
   if (XMLTYPE_CREATE == (*itr)->type)
   {
    XMLLevel *newLevel = new XMLLevel;
    newLevel->state = *itr;
    newLevel->object = (*((*itr)->func))();
    level.push_back(newLevel);
    newLevel->object->serialize(this);
   }
   else
   {
    state = *itr;
   }
   return;
  }
 }
}

void XMLSerializer::endElement(const XML_Char *name)
{
 if (state)
 {
  if (0 == strcmp(state->name.c_str(), name))
  {
   state = 0;
  }
 }
 else if (!level.empty())
 {
  if (0 == strcmp(level.back()->state->name.c_str(), name))
  {
   int curLevel = level.size();
   for (XMLAction *act(action.back()); (act) && (act->level == curLevel); act = action.back())
   {
    action.pop_back();
    delete act;
   }
   XMLLevel *oldLevel = level.back();
   level.pop_back();
   reinterpret_cast<XMLArray*>(oldLevel->state->object)->push_back(oldLevel->object);
   delete oldLevel;
  }
 }
}

void XMLSerializer::characterData(const XML_Char *s, int len)
{
 if (state)
 {
  switch(state->type)
  {
   case XMLTYPE_INT:
    *(reinterpret_cast<int*>(state->object)) = atoi(s);
    break;
   case XMLTYPE_STRING:
   {
    char *str = *(reinterpret_cast<char**>(state->object));
    if (str)
     delete [] str;
    str = new char[len + 1];
    strncpy(str, s, len);
    str[len] = 0;
    *(reinterpret_cast<char**>(state->object)) = str;
    break;
   }
   default:
    break;
  };
 }
}
