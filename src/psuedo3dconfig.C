/*-------------------------------------------------------------------------*\
  <psuedo3dconfig.C> -- Psuedo3DConfig implementation file

  Date      Programmer  Description
  01/22/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "psuedo3dconfig.h"
#include "xmlserializer.h"
#include <stdio.h>

void Psuedo3DWallType::serialize(ObjectSerializer* s)
{
 for (int i = 0; i < WALL_DIRECTIONS; ++i)
 {
  std::vector<XMLAttribute> *attrib = new std::vector<XMLAttribute>;
  char tmp[10];
  sprintf(tmp, "%d", i);
  attrib->push_back(XMLAttribute("direction", tmp));
  s->add("wall", &walls[i], attrib);
 }
}

void Psuedo3DConfig::serialize(ObjectSerializer* s)
{
 s->add("height", &height);
 s->add("width", &width);
 s->add("background", &background);
 s->add("walltype", &wallType, &Psuedo3DWallType::create);
}

void Psuedo3DConfig::readXML(const char *filename, XMLVector<Psuedo3DConfig*> &cfg)
{
 XMLSerializer parser;
 parser.add("psuedo3d", &cfg, &Psuedo3DConfig::create);
 parser.parse(filename, true);
}
