/*-------------------------------------------------------------------------*\
  <psuedo3dconfig.C> -- Psuedo3DConfig implementation file

  Date      Programmer  Description
  01/22/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "psuedo3dconfig.h"
#include "xmlserializer.h"

void Psuedo3DWallType::serialize(XMLSerializer* s)
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

void Psuedo3DConfig::serialize(XMLSerializer* s)
{
 s->add("height", &height);
 s->add("width", &width);
 s->add("background", &background);
 s->add("walltype", &Psuedo3DWallType::create, &wallType);
}

void Psuedo3DConfig::readXML(const char *filename, XMLVector<Psuedo3DConfig*> &cfg)
{
 XMLSerializer parser;
 parser.add("psuedo3d", &Psuedo3DConfig::create, &cfg);
 parser.parse(filename);
}
