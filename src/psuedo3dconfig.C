/*-------------------------------------------------------------------------*\
  <psuedo3dconfig.C> -- Psuedo3DConfig implementation file

  Date      Programmer  Description
  01/22/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "psuedo3dconfig.h"
#include "xmlserializer.h"
#include <stdio.h>
#include <algorithm>

void Psuedo3DWallType::serialize(ObjectSerializer* s)
{
 s->add("type", &type);
 s->add("modulus", &modulus);
 for (int i = 0; i < WALL_DIRECTIONS; ++i)
 {
  std::vector<XMLAttribute> *attrib = new std::vector<XMLAttribute>;
  char tmp[10];
  sprintf(tmp, "%d", i);
  attrib->push_back(XMLAttribute("direction", tmp));
  s->add("wall", &walls[i], attrib);
 }
}

void Psuedo3DMapType::serialize(ObjectSerializer* s)
{
 s->add("type", &type);
 s->add("name", &name);
 s->add("incompleteType", &incompleteType);
 s->add("viewType", &viewType);
 s->add("passable", &passable);
 s->add("invincible", &invincible);
 for (int i = 0; i < CARDINAL_DIRECTIONS; ++i)
 {
  std::vector<XMLAttribute> *attrib = new std::vector<XMLAttribute>;
  char tmp[10];
  sprintf(tmp, "%d", i);
  attrib->push_back(XMLAttribute("direction", tmp));
  s->add("mapWall", &mapWalls[i], attrib);
 }
}

int Psuedo3DConfig::findWallType(int type, int position)
{
 if (type)
 {
  int viewType = type;
  {
   for (int i = 0; i < mapType.size(); ++i)
   {
    if (mapType[i]->type == type)
    {
     if (-1 != mapType[i]->viewType)
      viewType = mapType[i]->viewType;
    }
   }
  }
  int modulus = position % divide;
  for (int i = 0; i < wallType.size(); ++i)
  {
   if ((wallType[i]->type == viewType) && (wallType[i]->modulus.end() != std::find(wallType[i]->modulus.begin(), wallType[i]->modulus.end(), modulus)))
    return i + 1;
  }
 }
 return 0;
}

int Psuedo3DConfig::findMapType(int type, bool complete)
{
 if (type)
 {
  for (int i = 0; i < mapType.size(); ++i)
  {
   if (mapType[i]->type == type)
   {
    if ((!complete) && (-1 != mapType[i]->incompleteType))
     return findMapType(mapType[i]->incompleteType, true);
    else
     return i + 1;
   }
  }
 }
 return 0;
}

void Psuedo3DConfig::serialize(ObjectSerializer* s)
{
 s->add("name", &name);
 s->add("height", &height);
 s->add("width", &width);
 s->add("background", &background);
 s->add("divide", &divide);
 s->add("walltype", &wallType, &Psuedo3DWallType::create);
 s->add("mapHeight", &mapHeight);
 s->add("mapWidth", &mapWidth);
 s->add("maptype", &mapType, &Psuedo3DMapType::create);
 s->add("mapSpecial", &mapSpecial);
 s->add("mapUnknown", &mapUnknown);
 for (int i = 0; i < CARDINAL_DIRECTIONS; ++i)
 {
  std::vector<XMLAttribute> *attrib = new std::vector<XMLAttribute>;
  char tmp[10];
  sprintf(tmp, "%d", i);
  attrib->push_back(XMLAttribute("direction", tmp));
  s->add("mapArrow", &mapArrows[i], attrib);
 }
}

void Psuedo3DConfig::readXML(const char *filename, XMLVector<Psuedo3DConfig*> &cfg)
{
 XMLSerializer parser;
 parser.add("psuedo3d", &cfg, &Psuedo3DConfig::create);
 parser.parse(filename, true);
}


std::string Psuedo3DConfigList::getName(int index)
{
 if (size() > index)
  return operator[](index)->name;
 else
  return "";
}

int Psuedo3DConfigList::getIndex(std::string name)
{
 for (int i = 0; i < size(); ++i)
  if (name.c_str() == operator[](i)->name)
   return i;
 return -1;
}

size_t Psuedo3DConfigList::size()
{
 return XMLVector<Psuedo3DConfig*>::size();
}

