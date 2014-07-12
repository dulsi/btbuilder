/*-------------------------------------------------------------------------*\
  <xmlserializer.C> -- XMLSerializer implementation file

  Date      Programmer  Description
  01/28/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "xmlserializer.h"
#include <string.h>
#include <stdio.h>
#include <typeinfo>

XMLAction::~XMLAction()
{
 if (next) delete next;
 if (attrib) delete attrib;
 if (type & XMLTYPE_DELETE)
 {
  switch (getType())
  {
   case XMLTYPE_UINT:
    delete reinterpret_cast<unsigned int*>(object);
    break;
   case XMLTYPE_STDSTRING:
    delete reinterpret_cast<std::string*>(object);
    break;
   default:
    break;
  }
 }
}

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

std::string XMLAction::createString()
{
 char convert[30];
 std::string content;
 switch(getType())
 {
  case XMLTYPE_BITFIELD:
  {
   BitField *field = reinterpret_cast<BitField*>(object);
   if (data)
   {
    ValueLookup *lookup = reinterpret_cast<ValueLookup*>(data);
    for (int i = 0; i < lookup->size(); ++i)
    {
     if (field->isSet(i))
     {
      if (content.empty())
       content = lookup->getName(i);
      else
      {
       content = "Multiple";
       break;
      }
     }
    }
   }
   break;
  }
  case XMLTYPE_BOOL:
   if (*(reinterpret_cast<bool*>(object)))
    content = "true";
   else
    content = "false";
   break;
  case XMLTYPE_INT:
   if (data)
   {
    if (extra == *(reinterpret_cast<int*>(object)))
     content = extraText;
    else
     content = reinterpret_cast<ValueLookup*>(data)->getName(*(reinterpret_cast<int*>(object)));
   }
   else
   {
    sprintf(convert, "%d", *(reinterpret_cast<int*>(object)));
    content = convert;
   }
   break;
  case XMLTYPE_UINT:
   sprintf(convert, "%u", *(reinterpret_cast<unsigned int*>(object)));
   content = convert;
   break;
  case XMLTYPE_INT16:
   sprintf(convert, "%d", *(reinterpret_cast<int16_t*>(object)));
   content = convert;
   break;
  case XMLTYPE_UINT16:
   sprintf(convert, "%u", *(reinterpret_cast<uint16_t*>(object)));
   content = convert;
   break;
  case XMLTYPE_STRING:
  {
   content = *(reinterpret_cast<char**>(object));
   break;
  }
  case XMLTYPE_STDSTRING:
  {
   content = *reinterpret_cast<std::string*>(object);
   break;
  }
  case XMLTYPE_VECTORUINT:
  {
   std::vector<unsigned int> *val = reinterpret_cast<std::vector<unsigned int> *>(object);
   if (val->size() > 1)
    content = "Multiple";
   if (data)
   {
    if (val->size() == 1)
     content = reinterpret_cast<ValueLookup*>(data)->getName((*val)[0]);
   }
   else
   {
    sprintf(convert, "%u", (*val)[0]);
    content = convert;
   }
   break;
  }
  case XMLTYPE_PICTURE:
   sprintf(convert, "%d", reinterpret_cast<PictureIndex*>(object)->value);
   content = convert;
   break;
  case XMLTYPE_OBJECT:
   content = reinterpret_cast<XMLObject*>(object)->createString();
   break;
  default:
   break;
 }
 return content;
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

void ObjectSerializer::add(const char *name, const char *objnm, XMLArray* vec, XMLObject::create func, std::vector<XMLAttribute> *atts /*= NULL*/)
{
 XMLAction *act = new XMLAction;
 act->name = ns + name;
 if (objnm)
  act->objnm = objnm;
 act->attrib = atts;
 act->type = XMLTYPE_CREATE;
 act->level = getLevel();
 act->object = reinterpret_cast<void*>(vec);
 act->data = reinterpret_cast<void*>(func);
 for (std::vector<XMLAction*>::reverse_iterator itr(action.rbegin()); (action.rend() != itr) && (act->level == (*itr)->level); itr++)
 {
  if ((*itr)->object == act->object)
  {
   act->next = (*itr)->next;
   (*itr)->next = act;
   return;
  }
 }
 action.push_back(act);
}

void ObjectSerializer::add(const char *name, XMLObject *p, std::vector<XMLAttribute> *atts /*= NULL*/)
{
 XMLAction *act = new XMLAction;
 act->name = ns + name;
 act->attrib = atts;
 act->type = XMLTYPE_OBJECT;
 act->level = getLevel();
 act->object = reinterpret_cast<void*>(p);
 action.push_back(act);
}

void ObjectSerializer::add(const char *name, bool *p, std::vector<XMLAttribute> *atts /*= NULL*/, bool delFlg /*= false*/)
{
 XMLAction *act = new XMLAction;
 act->name = ns + name;
 act->attrib = atts;
 act->type = XMLTYPE_BOOL;
 if (delFlg)
  act->type |= XMLTYPE_DELETE;
 act->level = getLevel();
 act->object = reinterpret_cast<void*>(p);
 action.push_back(act);
}

void ObjectSerializer::add(const char *name, int *p, std::vector<XMLAttribute> *atts /*= NULL*/, ValueLookup *lookup /*= NULL*/, int extra /*= EXTRA_NONE*/, const std::string &extraText /*= std::string()*/)
{
 XMLAction *act = new XMLAction;
 act->name = ns + name;
 act->attrib = atts;
 act->type = XMLTYPE_INT;
 act->level = getLevel();
 act->object = reinterpret_cast<void*>(p);
 act->data = reinterpret_cast<void*>(lookup);
 act->extra = extra;
 act->extraText = extraText;
 action.push_back(act);
}

void ObjectSerializer::add(const char *name, uint16_t *p, std::vector<XMLAttribute> *atts /*= NULL*/)
{
 XMLAction *act = new XMLAction;
 act->name = ns + name;
 act->attrib = atts;
 act->type = XMLTYPE_UINT16;
 act->level = getLevel();
 act->object = reinterpret_cast<void*>(p);
 action.push_back(act);
}

void ObjectSerializer::add(const char *name, int16_t *p, std::vector<XMLAttribute> *atts /*= NULL*/)
{
 XMLAction *act = new XMLAction;
 act->name = ns + name;
 act->attrib = atts;
 act->type = XMLTYPE_INT16;
 act->level = getLevel();
 act->object = reinterpret_cast<void*>(p);
 action.push_back(act);
}

void ObjectSerializer::add(const char *name, unsigned int *p, std::vector<XMLAttribute> *atts /*= NULL*/, bool delFlg /*= false*/)
{
 XMLAction *act = new XMLAction;
 act->name = ns + name;
 act->attrib = atts;
 act->type = XMLTYPE_UINT;
 if (delFlg)
  act->type |= XMLTYPE_DELETE;
 act->level = getLevel();
 act->object = reinterpret_cast<void*>(p);
 action.push_back(act);
}

void ObjectSerializer::add(const char *name, char **p, std::vector<XMLAttribute> *atts /*= NULL*/)
{
 XMLAction *act = new XMLAction;
 act->name = ns + name;
 act->attrib = atts;
 act->type = XMLTYPE_STRING;
 act->level = getLevel();
 act->object = reinterpret_cast<void*>(p);
 action.push_back(act);
}

void ObjectSerializer::add(const char *name, std::string *p, std::vector<XMLAttribute> *atts /*= NULL*/, bool delFlg /*= false*/)
{
 XMLAction *act = new XMLAction;
 act->name = ns + name;
 act->attrib = atts;
 act->type = XMLTYPE_STDSTRING;
 if (delFlg)
  act->type |= XMLTYPE_DELETE;
 act->level = getLevel();
 act->object = reinterpret_cast<void*>(p);
 action.push_back(act);
}

void ObjectSerializer::add(const char *name, BitField *p, ValueLookup *lookup, std::vector<XMLAttribute> *atts /*= NULL*/)
{
 XMLAction *act = new XMLAction;
 act->name = ns + name;
 act->attrib = atts;
 act->type = XMLTYPE_BITFIELD;
 act->level = getLevel();
 act->object = reinterpret_cast<void*>(p);
 act->data = reinterpret_cast<void*>(lookup);
 action.push_back(act);
}

void ObjectSerializer::add(const char *name, std::vector<unsigned int> *p, ValueLookup *lookup /*= NULL*/, std::vector<XMLAttribute> *atts /*= NULL*/)
{
 XMLAction *act = new XMLAction;
 act->name = ns + name;
 act->attrib = atts;
 act->type = XMLTYPE_VECTORUINT;
 act->level = getLevel();
 act->object = reinterpret_cast<void*>(p);
 act->data = reinterpret_cast<void*>(lookup);
 action.push_back(act);
}

void ObjectSerializer::add(const char *name, std::vector<std::string> *p, std::vector<XMLAttribute> *atts /*= NULL*/)
{
 XMLAction *act = new XMLAction;
 act->name = ns + name;
 act->attrib = atts;
 act->type = XMLTYPE_VECTORSTRING;
 act->level = getLevel();
 act->object = reinterpret_cast<void*>(p);
 action.push_back(act);
}

void ObjectSerializer::add(const char *name, PictureIndex *p, std::vector<XMLAttribute> *atts /*= NULL*/)
{
 XMLAction *act = new XMLAction;
 act->name = ns + name;
 act->attrib = atts;
 act->type = XMLTYPE_PICTURE;
 act->level = getLevel();
 act->object = reinterpret_cast<void*>(p);
 action.push_back(act);
}

void ObjectSerializer::addLevel(XMLLevel *newLevel)
{
  level.push_back(newLevel);
  newLevel->object->serialize(this);
}

XMLAction* ObjectSerializer::find(const char *name, const char **atts)
{
 int curLevel = getLevel();
 for (std::vector<XMLAction*>::reverse_iterator itr(action.rbegin()); (action.rend() != itr) && ((*itr)->level == curLevel); itr++)
 {
  for (XMLAction *act = *itr; act; act = act->next)
  {
   if (0 == strcmp(act->name.c_str(), name))
   {
    bool found(true);
    if ((NULL == atts) && (act->attrib))
     found = false;
    else if (act->attrib)
    {
     found = false;
     for (std::vector<XMLAttribute>::iterator itrAttrib(act->attrib->begin()); itrAttrib != act->attrib->end(); itrAttrib++)
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
    }
    if (found)
     return act;
   }
  }
 }
 return NULL;
}

int ObjectSerializer::getLevel()
{
 return level.size();
}

XMLLevel *ObjectSerializer::removeLevel()
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
 XMLLevel *old = NULL;
 if (!level.empty())
 {
  old = level.back();
  level.pop_back();
 }
 return old;
}

void ObjectSerializer::setNamespace(std::string newNS)
{
 ns = newNS;
 if (ns != "")
  ns += ":";
}

XMLSerializer::XMLSerializer()
: state(0)
{
}

XMLSerializer::~XMLSerializer()
{
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
 if (XMLTYPE_CREATE == act->getType())
 {
  XMLLevel *newLevel = new XMLLevel;
  newLevel->state = act;
  newLevel->object = (*reinterpret_cast<XMLObject::create>(act->data))(name, atts);
  addLevel(newLevel);
 }
 else if (XMLTYPE_OBJECT == act->getType())
 {
  XMLLevel *newLevel = new XMLLevel;
  newLevel->state = act;
  newLevel->object = reinterpret_cast<XMLObject*>(act->object);
  addLevel(newLevel);
 }
 else
 {
  state = act;
  content = "";
 }
}

void XMLSerializer::endElement(const XML_Char *name)
{
 if (state)
 {
  if (0 == strcmp(state->name.c_str(), name))
  {
   switch(state->getType())
   {
    case XMLTYPE_BOOL:
     *(reinterpret_cast<bool*>(state->object)) = ((strncmp(content.c_str(), "true", 4) == 0) ? true : false);
     break;
    case XMLTYPE_INT:
     if (state->data)
     {
      *(reinterpret_cast<int*>(state->object)) = reinterpret_cast<ValueLookup*>(state->data)->getIndex(content);
      if (-1 == *(reinterpret_cast<int*>(state->object)))
       *(reinterpret_cast<int*>(state->object)) = atoi(content.c_str());
     }
     else
      *(reinterpret_cast<int*>(state->object)) = atoi(content.c_str());
     break;
    case XMLTYPE_UINT:
    {
     sscanf(content.c_str(), "%u", reinterpret_cast<unsigned int*>(state->object));
     break;
    }
    case XMLTYPE_INT16:
     *(reinterpret_cast<int16_t*>(state->object)) = atoi(content.c_str());
     break;
    case XMLTYPE_UINT16:
    {
     unsigned int u;
     sscanf(content.c_str(), "%u", &u);
     *(reinterpret_cast<uint16_t*>(state->object)) = u;
     break;
    }
    case XMLTYPE_STRING:
    {
     char *str = *(reinterpret_cast<char**>(state->object));
     if (str)
     {
      delete [] str;
     }
     int len = content.length();
     str = new char[len + 1];
     strncpy(str, content.c_str(), len);
     str[len] = 0;
     *(reinterpret_cast<char**>(state->object)) = str;
     break;
    }
    case XMLTYPE_STDSTRING:
    {
     *(reinterpret_cast<std::string*>(state->object)) = content;
     break;
    }
    case XMLTYPE_BITFIELD:
    {
     if (state->data)
     {
      int index = reinterpret_cast<ValueLookup*>(state->data)->getIndex(content);
      if (-1 != index)
       reinterpret_cast<BitField*>(state->object)->set(index);
     }
     else
     {
      int b = -1;
      sscanf(content.c_str(), "%d", &b);
      if (b != -1)
       reinterpret_cast<BitField*>(state->object)->set(b);
     }
     break;
    }
    case XMLTYPE_VECTORUINT:
    {
     unsigned int u;
     if (state->data)
     {
      int val = reinterpret_cast<ValueLookup*>(state->data)->getIndex(content);
      if (-1 != val)
      {
       reinterpret_cast<std::vector<unsigned int> *>(state->object)->push_back(val);
       break;
      }
     }
     sscanf(content.c_str(), "%u", &u);
     reinterpret_cast<std::vector<unsigned int> *>(state->object)->push_back(u);
     break;
    }
    case XMLTYPE_VECTORSTRING:
    {
     reinterpret_cast<std::vector<std::string> *>(state->object)->push_back(content);
     break;
    }
    case XMLTYPE_PICTURE:
     reinterpret_cast<PictureIndex*>(state->object)->value = atoi(content.c_str());
     break;
    default:
     break;
   };
   state = 0;
  }
 }
 else if (!level.empty())
 {
  if (0 == strcmp(level.back()->state->name.c_str(), name))
  {
   XMLLevel *oldLevel = removeLevel();
   if (XMLTYPE_CREATE == oldLevel->state->getType())
    reinterpret_cast<XMLArray*>(oldLevel->state->object)->push_back(oldLevel->object);
   delete oldLevel;
  }
 }
}

void XMLSerializer::characterData(const XML_Char *s, int len)
{
 if (state)
 {
  std::string str(s, len);
  content += str;
 }
 else if ((!level.empty()) && ((XMLTYPE_CREATE == level.back()->state->getType()) || (XMLTYPE_OBJECT == level.back()->state->getType())))
 {
  level.back()->object->characterData(s, len);
 }
}

void XMLSerializer::write(const char *filename, bool physfs)
{
 PHYSFS_file *physFile = NULL;
 FILE *file = NULL;
 if (strcmp(filename, "-") == 0)
 {
  file = stdout;
 }
 else if (physfs)
 {
  physFile = PHYSFS_openWrite(filename);
 }
 else
 {
  file = fopen(filename, "w");
 }
 write(physFile, file, "<data>\n", 7);
 void *prevObject = NULL;
 for (std::vector<XMLAction*>::iterator itr(action.begin()); (itr != action.end()) || (!level.empty()); ++itr)
 {
  if (itr == action.end())
  {
   itr = action.end();
   --itr;
   while ((*itr)->object != level.back()->state->object)
   {
    --itr;
   }
   XMLAction *act = level.back()->state;
   if (XMLTYPE_CREATE == act->getType())
   {
    XMLArray *ary = reinterpret_cast<XMLArray*>(act->object);
    write(physFile, file, "</", 2);
    write(physFile, file, act->name.c_str(), act->name.length());
    write(physFile, file, ">", 1);
    int i = 0;
    for (; i < ary->size(); ++i)
    {
     if (ary->get(i) == level.back()->object)
      break;
    }
    XMLLevel *oldLevel = removeLevel();
    delete oldLevel;
    if (i + 1 == ary->size())
    {
     itr = action.end();
     --itr;
     while ((*itr)->object != act->object)
      --itr;
     continue;
    }
    else
    {
     XMLLevel *newLevel = new XMLLevel;
     newLevel->object = ary->get(i + 1);
     std::string objnm = typeid(*(newLevel->object)).name();
     for (act = *itr; (act) && (!act->objnm.empty()) && (act->objnm != objnm); act = act->next)
     {
     }
     if (act)
     {
      std::string tag = act->createTag();
      write(physFile, file, "<", 1);
      write(physFile, file, tag.c_str(), tag.length());
      write(physFile, file, ">", 1);
      newLevel->state = act;
      int size = action.size();
      addLevel(newLevel);
      itr = action.begin() + size - 1;
     }
     else
      delete newLevel;
     continue;
    }
   }
   else
   {
    write(physFile, file, "</", 2);
    write(physFile, file, (*itr)->name.c_str(), (*itr)->name.length());
    write(physFile, file, ">", 1);
    XMLLevel *oldLevel = removeLevel();
    delete oldLevel;
    itr = action.end();
    --itr;
    while (*itr != act)
     --itr;
    continue;
   }
  }
  if (prevObject == (*itr)->object)
   continue;
  prevObject = (*itr)->object;
  int type = (*itr)->getType();
  if ((XMLTYPE_CREATE != type) && (XMLTYPE_BITFIELD != type) && (XMLTYPE_VECTORUINT != type))
  {
   std::string tag = (*itr)->createTag();
   write(physFile, file, "<", 1);
   write(physFile, file, tag.c_str(), tag.length());
   write(physFile, file, ">", 1);
  }
  char convert[30];
  std::string content;
  switch(type)
  {
   case XMLTYPE_BOOL:
   case XMLTYPE_INT:
   case XMLTYPE_UINT:
   case XMLTYPE_INT16:
   case XMLTYPE_UINT16:
   case XMLTYPE_STRING:
   case XMLTYPE_STDSTRING:
   case XMLTYPE_PICTURE:
    content = (*itr)->createString();
    break;
   case XMLTYPE_BITFIELD:
   {
    BitField *b = reinterpret_cast<BitField*>((*itr)->object);
    if ((*itr)->data)
    {
     ValueLookup *lookup = reinterpret_cast<ValueLookup*>((*itr)->data);
     for (int i = b->getMaxSet(); i >= 0; --i)
     {
      if (b->isSet(i))
      {
       content += "<" + (*itr)->createTag() + ">" + lookup->getName(i) + "</" + (*itr)->name + ">";
      }
     }
    }
    else
    {
     for (int i = b->getMaxSet(); i >= 0; --i)
     {
      if (b->isSet(i))
      {
       sprintf(convert, "%d", i);
       content += "<" + (*itr)->createTag() + ">" + convert + "</" + (*itr)->name + ">";
      }
     }
    }
    break;
   }
   case XMLTYPE_VECTORUINT:
   {
    std::vector<unsigned int> *v = reinterpret_cast<std::vector<unsigned int> *>((*itr)->object);
    for (int i = 0; i < v->size(); ++i)
    {
     sprintf(convert, "%u", (*v)[i]);
     content += "<" + (*itr)->createTag() + ">" + convert + "</" + (*itr)->name + ">";
    }
    break;
   }
   case XMLTYPE_VECTORSTRING:
   {
    std::vector<std::string> *v = reinterpret_cast<std::vector<std::string> *>((*itr)->object);
    for (int i = 0; i < v->size(); ++i)
    {
     content += "<" + (*itr)->createTag() + ">" + (*v)[i] + "</" + (*itr)->name + ">";
    }
    break;
   }
   case XMLTYPE_CREATE:
   {
    XMLArray *ary = reinterpret_cast<XMLArray*>((*itr)->object);
    if (0 != ary->size())
    {
     XMLLevel *newLevel = new XMLLevel;
     newLevel->object = ary->get(0);
     std::string objnm = typeid(*(newLevel->object)).name();
     XMLAction *act = *itr;
     for (; (act) && (!act->objnm.empty()) && (act->objnm != objnm); act = act->next)
     {
     }
     if (act)
     {
      std::string tag = act->createTag();
      write(physFile, file, "<", 1);
      write(physFile, file, tag.c_str(), tag.length());
      write(physFile, file, ">", 1);
      newLevel->state = act;
      int size = action.size();
      addLevel(newLevel);
      itr = action.begin() + size - 1;
     }
     else
      delete newLevel;
    }
    break;
   }
   case XMLTYPE_OBJECT:
   {
    XMLLevel *newLevel = new XMLLevel;
    newLevel->state = *itr;
    newLevel->object = reinterpret_cast<XMLObject*>((*itr)->object);
    int size = action.size();
    addLevel(newLevel);
    itr = action.begin() + size - 1;
    break;
   }
   default:
    break;
  }
  if (content.length())
   write(physFile, file, content.c_str(), content.length());
  if ((XMLTYPE_CREATE != type) && (XMLTYPE_OBJECT != type) && (XMLTYPE_BITFIELD != type) && (XMLTYPE_VECTORUINT != type))
  {
   write(physFile, file, "</", 2);
   write(physFile, file, (*itr)->name.c_str(), (*itr)->name.length());
   write(physFile, file, ">", 1);
  }
 }
 write(physFile, file, "</data>\n", 8);
 if (strcmp(filename, "-") == 0)
 {
 }
 else if (physfs)
 {
  PHYSFS_close(physFile);
 }
 else
 {
  fclose(file);
 }
}

void XMLSerializer::write(PHYSFS_file *physFile, FILE *file, const char *content, size_t len)
{
 if (physFile)
 {
  PHYSFS_write(physFile, content, 1, len);
 }
 else if (file)
 {
  fwrite(content, 1, len, file);
 }
}

