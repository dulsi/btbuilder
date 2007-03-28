/*-------------------------------------------------------------------------*\
  <xmlserializer.C> -- XMLSerializer implementation file

  Date      Programmer  Description
  01/28/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "xmlserializer.h"

ObjectSerializer::ObjectSerializer()
{
}

ObjectSerializer::~ObjectSerializer()
{
 for (std::vector<XMLAction*>::iterator itr(action.begin()); itr != action.end(); itr++)
  delete *itr;
 action.clear();
}

void ObjectSerializer::add(const char *name, XMLArray* vec, XMLObject::create func, std::vector<XMLAttribute> *atts /*= NULL*/)
{
 XMLAction *act = new XMLAction;
 act->name = name;
 act->attrib = atts;
 act->type = XMLTYPE_CREATE;
 act->level = getLevel();
 act->object = reinterpret_cast<void*>(vec);
 act->data = reinterpret_cast<void*>(func);
 action.push_back(act);
}

void ObjectSerializer::add(const char *name, XMLObject *p, std::vector<XMLAttribute> *atts /*= NULL*/)
{
 XMLAction *act = new XMLAction;
 act->name = name;
 act->attrib = atts;
 act->type = XMLTYPE_OBJECT;
 act->level = getLevel();
 act->object = reinterpret_cast<void*>(p);
 action.push_back(act);
}

void ObjectSerializer::add(const char *name, bool *p, std::vector<XMLAttribute> *atts /*= NULL*/)
{
 XMLAction *act = new XMLAction;
 act->name = name;
 act->attrib = atts;
 act->type = XMLTYPE_BOOL;
 act->level = getLevel();
 act->object = reinterpret_cast<void*>(p);
 action.push_back(act);
}

void ObjectSerializer::add(const char *name, int *p, std::vector<XMLAttribute> *atts /*= NULL*/, ValueLookup *lookup /*= NULL*/)
{
 XMLAction *act = new XMLAction;
 act->name = name;
 act->attrib = atts;
 act->type = XMLTYPE_INT;
 act->level = getLevel();
 act->object = reinterpret_cast<void*>(p);
 act->data = reinterpret_cast<void*>(lookup);
 action.push_back(act);
}

void ObjectSerializer::add(const char *name, unsigned int *p, std::vector<XMLAttribute> *atts /*= NULL*/)
{
 XMLAction *act = new XMLAction;
 act->name = name;
 act->attrib = atts;
 act->type = XMLTYPE_UINT;
 act->level = getLevel();
 act->object = reinterpret_cast<void*>(p);
 action.push_back(act);
}

void ObjectSerializer::add(const char *name, char **p, std::vector<XMLAttribute> *atts /*= NULL*/)
{
 XMLAction *act = new XMLAction;
 act->name = name;
 act->attrib = atts;
 act->type = XMLTYPE_STRING;
 act->level = getLevel();
 act->object = reinterpret_cast<void*>(p);
 action.push_back(act);
}

void ObjectSerializer::add(const char *name, BitField *p, ValueLookup *lookup, std::vector<XMLAttribute> *atts /*= NULL*/)
{
 XMLAction *act = new XMLAction;
 act->name = name;
 act->attrib = atts;
 act->type = XMLTYPE_BITFIELD;
 act->level = getLevel();
 act->object = reinterpret_cast<void*>(p);
 act->data = reinterpret_cast<void*>(lookup);
 action.push_back(act);
}

XMLAction* ObjectSerializer::find(const XML_Char *name, const XML_Char **atts)
{
 int curLevel = getLevel();
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
   return *itr;
  }
 }
 return NULL;
}

void ObjectSerializer::removeLevel()
{
 int curLevel = getLevel();
 for (XMLAction *act(action.back()); (act) && (act->level == curLevel); act = action.back())
 {
  action.pop_back();
  delete act;
 }
}

XMLSerializer::XMLSerializer()
: state(0)
{
}

XMLSerializer::~XMLSerializer()
{
}

int XMLSerializer::getLevel()
{
 return level.size();
}

void XMLSerializer::startElement(const XML_Char *name, const XML_Char **atts)
{
 // Shouldn't already be in a state
 if (state)
  return;
 XMLAction *act = find(name, atts);
 if (NULL == act)
  return;
 if (XMLTYPE_CREATE == act->type)
 {
  XMLLevel *newLevel = new XMLLevel;
  newLevel->state = act;
  newLevel->object = (*reinterpret_cast<XMLObject::create>(act->data))();
  level.push_back(newLevel);
  newLevel->object->serialize(this);
 }
 else if (XMLTYPE_OBJECT == act->type)
 {
  XMLLevel *newLevel = new XMLLevel;
  newLevel->state = act;
  newLevel->object = reinterpret_cast<XMLObject*>(act->object);
  level.push_back(newLevel);
  newLevel->object->serialize(this);
 }
 else
 {
  state = act;
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
   removeLevel();
   XMLLevel *oldLevel = level.back();
   level.pop_back();
   if (XMLTYPE_CREATE == oldLevel->state->type)
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
   case XMLTYPE_BOOL:
    *(reinterpret_cast<bool*>(state->object)) = ((strncmp(s, "true", 4) == 0) ? true : false);
    break;
   case XMLTYPE_INT:
    if (state->data)
    {
     std::string str(s, len);
     *(reinterpret_cast<int*>(state->object)) = reinterpret_cast<ValueLookup*>(state->data)->getIndex(str);
    }
    else
     *(reinterpret_cast<int*>(state->object)) = atoi(s);
    break;
   case XMLTYPE_UINT:
   {
    std::string str(s, len);
    sscanf(str.c_str(), "%u", reinterpret_cast<unsigned int*>(state->object));
    break;
   }
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
   case XMLTYPE_BITFIELD:
   {
    std::string str(s, len);
    int index = reinterpret_cast<ValueLookup*>(state->data)->getIndex(str);
    if (-1 != index)
     reinterpret_cast<BitField*>(state->object)->set(index);
    break;
   }
   default:
    break;
  };
 }
}
