/*-------------------------------------------------------------------------*\
  <xmlserializer.C> -- XMLSerializer implementation file

  Date      Programmer  Description
  01/28/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "xmlserializer.h"
#include <physfs.h>

std::string XMLAction::createTag()
{
 std::string str(name);
 if (attrib)
 {
  for (std::vector<XMLAttribute>::iterator itr(attrib->begin()); itr != attrib->end(); ++itr)
  {
   str += " ";
   str += itr->name;
   str += "=\"";
   str += itr->value;
   str += "\"";
  }
 }
 return str;
}

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

void ObjectSerializer::add(const char *name, uint16_t *p, std::vector<XMLAttribute> *atts /*= NULL*/)
{
 XMLAction *act = new XMLAction;
 act->name = name;
 act->attrib = atts;
 act->type = XMLTYPE_UINT16;
 act->level = getLevel();
 act->object = reinterpret_cast<void*>(p);
 action.push_back(act);
}

void ObjectSerializer::add(const char *name, int16_t *p, std::vector<XMLAttribute> *atts /*= NULL*/)
{
 XMLAction *act = new XMLAction;
 act->name = name;
 act->attrib = atts;
 act->type = XMLTYPE_INT16;
 act->level = getLevel();
 act->object = reinterpret_cast<void*>(p);
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

void ObjectSerializer::add(const char *name, std::string *p, std::vector<XMLAttribute> *atts /*= NULL*/)
{
 XMLAction *act = new XMLAction;
 act->name = name;
 act->attrib = atts;
 act->type = XMLTYPE_STDSTRING;
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

XMLAction* ObjectSerializer::find(const char *name, const char **atts)
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

void ObjectSerializer::findAll(const char *name, std::list<XMLAction*> &list)
{
 int curLevel = getLevel();
 for (std::vector<XMLAction*>::reverse_iterator itr(action.rbegin()); (action.rend() != itr) && ((*itr)->level == curLevel); itr++)
 {
  if (0 == strcmp((*itr)->name.c_str(), name))
  {
   list.push_front(*itr);
  }
 }
}

void ObjectSerializer::removeLevel()
{
 int curLevel = getLevel();
 if (action.size() > 0)
 {
  for (XMLAction *act(action.back()); (act) && (act->level == curLevel); act = ((action.size() > 0) ? action.back() : NULL))
  {
   action.pop_back();
   delete act;
  }
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
 {
  if ((!level.empty()) && ((XMLTYPE_CREATE == level.back()->state->type) || (XMLTYPE_OBJECT == level.back()->state->type)))
   level.back()->object->elementData(name, atts);
  return;
 }
 if (XMLTYPE_CREATE == act->type)
 {
  XMLLevel *newLevel = new XMLLevel;
  newLevel->state = act;
  newLevel->object = (*reinterpret_cast<XMLObject::create>(act->data))(name, atts);
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
   case XMLTYPE_INT16:
    *(reinterpret_cast<int16_t*>(state->object)) = atoi(s);
    break;
   case XMLTYPE_UINT16:
   {
    std::string str(s, len);
    unsigned int u;
    sscanf(str.c_str(), "%u", &u);
    *(reinterpret_cast<uint16_t*>(state->object)) = u;
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
   case XMLTYPE_STDSTRING:
   {
    std::string str(s, len);
    *(reinterpret_cast<std::string*>(state->object)) = str;
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
 else if ((!level.empty()) && ((XMLTYPE_CREATE == level.back()->state->type) || (XMLTYPE_OBJECT == level.back()->state->type)))
 {
  level.back()->object->characterData(s, len);
 }
}

void XMLSerializer::write(const char *filename, bool physfs)
{
 if (physfs)
 {
  PHYSFS_file *f = PHYSFS_openWrite(filename);
  PHYSFS_write(f, "<data>\n", 1, 7);
  for (std::vector<XMLAction*>::iterator itr(action.begin()); (itr != action.end()) || (!level.empty()); ++itr)
  {
   if (itr == action.end())
   {
    itr = action.end();
    --itr;
    while (*itr != level.back()->state)
     --itr;
    XMLAction *act = (*itr);
    if (XMLTYPE_CREATE == (*itr)->type)
    {
     XMLArray *ary = reinterpret_cast<XMLArray*>(level.back()->state->object);
     PHYSFS_write(f, "</", 1, 2);
     PHYSFS_write(f, (*itr)->name.c_str(), 1, (*itr)->name.length());
     PHYSFS_write(f, ">", 1, 1);
     int i = 0;
     for (; i < ary->size(); ++i)
     {
      if (ary->get(i) == level.back()->object)
       break;
     }
     removeLevel();
     XMLLevel *oldLevel = level.back();
     level.pop_back();
     delete oldLevel;
     if (i + 1 == ary->size())
     {
      itr = action.end();
      --itr;
      while (*itr != act)
       --itr;
      continue;
     }
     else
     {
      std::string tag = act->createTag();
      PHYSFS_write(f, "<", 1, 1);
      PHYSFS_write(f, tag.c_str(), 1, tag.length());
      PHYSFS_write(f, ">", 1, 1);
      XMLLevel *newLevel = new XMLLevel;
      newLevel->state = act;
      newLevel->object = ary->get(i + 1);
      level.push_back(newLevel);
      int size = action.size();
      newLevel->object->serialize(this);
      itr = action.begin() + size - 1;
      continue;
     }
    }
    else
    {
     PHYSFS_write(f, "</", 1, 2);
     PHYSFS_write(f, (*itr)->name.c_str(), 1, (*itr)->name.length());
     PHYSFS_write(f, ">", 1, 1);
     removeLevel();
     XMLLevel *oldLevel = level.back();
     level.pop_back();
     delete oldLevel;
     itr = action.end();
     --itr;
     while (*itr != act)
      --itr;
     continue;
    }
   }
   if (XMLTYPE_CREATE != (*itr)->type)
   {
    std::string tag = (*itr)->createTag();
    PHYSFS_write(f, "<", 1, 1);
    PHYSFS_write(f, tag.c_str(), 1, tag.length());
    PHYSFS_write(f, ">", 1, 1);
   }
   char convert[30];
   std::string content;
   switch((*itr)->type)
   {
    case XMLTYPE_BOOL:
     if (*(reinterpret_cast<bool*>((*itr)->object)))
      content = "true";
     else
      content = "false";
     break;
    case XMLTYPE_INT:
     if ((*itr)->data)
     {
      content = reinterpret_cast<ValueLookup*>((*itr)->data)->getName(*(reinterpret_cast<int*>((*itr)->object)));
     }
     else
     {
      sprintf(convert, "%d", *(reinterpret_cast<int*>((*itr)->object)));
      content = convert;
     }
     break;
    case XMLTYPE_UINT:
     sprintf(convert, "%u", *(reinterpret_cast<unsigned int*>((*itr)->object)));
     content = convert;
     break;
    case XMLTYPE_INT16:
     sprintf(convert, "%d", *(reinterpret_cast<int16_t*>((*itr)->object)));
     content = convert;
     break;
    case XMLTYPE_UINT16:
     sprintf(convert, "%u", *(reinterpret_cast<uint16_t*>((*itr)->object)));
     content = convert;
     break;
    case XMLTYPE_STRING:
    {
     content = *(reinterpret_cast<char**>((*itr)->object));
     break;
    }
    case XMLTYPE_STDSTRING:
    {
     content = *reinterpret_cast<std::string*>((*itr)->object);
     break;
    }
    case XMLTYPE_CREATE:
    {
     XMLArray *ary = reinterpret_cast<XMLArray*>((*itr)->object);
     if (0 != ary->size())
     {
      std::string tag = (*itr)->createTag();
      PHYSFS_write(f, "<", 1, 1);
      PHYSFS_write(f, tag.c_str(), 1, tag.length());
      PHYSFS_write(f, ">", 1, 1);
      XMLLevel *newLevel = new XMLLevel;
      newLevel->state = *itr;
      newLevel->object = ary->get(0);
      level.push_back(newLevel);
      int size = action.size();
      newLevel->object->serialize(this);
      itr = action.begin() + size - 1;
     }
     break;
    }
    case XMLTYPE_OBJECT:
    {
     XMLLevel *newLevel = new XMLLevel;
     newLevel->state = *itr;
     newLevel->object = reinterpret_cast<XMLObject*>((*itr)->object);
     level.push_back(newLevel);
     int size = action.size();
     newLevel->object->serialize(this);
     itr = action.begin() + size - 1;
     break;
    }
    default:
     break;
   }
   if (content.length())
    PHYSFS_write(f, content.c_str(), 1, content.length());
   if ((XMLTYPE_CREATE != (*itr)->type) && (XMLTYPE_OBJECT != (*itr)->type))
   {
    PHYSFS_write(f, "</", 1, 2);
    PHYSFS_write(f, (*itr)->name.c_str(), 1, (*itr)->name.length());
    PHYSFS_write(f, ">", 1, 1);
   }
  }
  PHYSFS_write(f, "</data>\n", 1, 8);
  PHYSFS_close(f);
 }
 else
 {
 }
}
