/*-------------------------------------------------------------------------*\
  <displayconfig.C> -- Display config implementation file

  Date      Programmer  Description
  04/01/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "displayconfig.h"
#include "xmlserializer.h"

BTDisplayConfig::BTDisplayConfig()
 : x3d(0), y3d(0), background(0), font(0)
{
}

BTDisplayConfig::~BTDisplayConfig()
{
 if (background)
  delete [] background;
 if (font)
  delete [] font;
}

void BTDisplayConfig::serialize(ObjectSerializer* s)
{
 s->add("width", &width);
 s->add("height", &height);
 s->add("background", &background);
 s->add("font", &font);
 s->add("x3d", &x3d);
 s->add("y3d", &y3d);
 s->add("label", &label);
 s->add("text", &text);
 for (int i = 0; i < BT_PARTYSIZE; ++i)
 {
  std::vector<XMLAttribute> *attrib = new std::vector<XMLAttribute>;
  char tmp[10];
  snprintf(tmp, 10, "%d", i + 1);
  attrib->push_back(XMLAttribute("number", tmp));
  s->add("status", &status[i], attrib);
 }
 s->add("statBlock", &statusInfo, &BTStatBlock::create);
 s->add("conditional", &statusInfo, &BTConditional::create);
 s->add("print", &statusInfo, &BTPrint::create);
 s->add("icon", &icon, &BTIcon::create);
}

