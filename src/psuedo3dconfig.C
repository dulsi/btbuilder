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

int Psuedo3DConfig::findWallType(int type, int position)
{
 if (type)
 {
  printf("%d, %d\n", type, position);
  int modulus = position % divide;
  for (int i = 0; i < wallType.size(); ++i)
  {
   if ((wallType[i]->type == type) && (wallType[i]->modulus.end() != std::find(wallType[i]->modulus.begin(), wallType[i]->modulus.end(), modulus)))
    return i + 1;
  }
 }
 return 0;
}

void Psuedo3DConfig::serialize(ObjectSerializer* s)
{
 s->add("height", &height);
 s->add("width", &width);
 s->add("background", &background);
 s->add("divide", &divide);
 s->add("walltype", &wallType, &Psuedo3DWallType::create);
}

void Psuedo3DConfig::readXML(const char *filename, XMLVector<Psuedo3DConfig*> &cfg)
{
 XMLSerializer parser;
 parser.add("psuedo3d", &cfg, &Psuedo3DConfig::create);
 parser.parse(filename, true);
  for (int i = 0; i < cfg[0]->wallType.size(); ++i)
  {
  for (std::vector<unsigned int>::iterator it = cfg[0]->wallType[i]->modulus.begin(); it != cfg[0]->wallType[i]->modulus.end(); ++it) printf("%d,", *it);
  printf("  %d\n", cfg[0]->wallType[i]->type);
  }
  printf("%d\n", cfg[0]->divide);
}
