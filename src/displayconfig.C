/*-------------------------------------------------------------------------*\
  <displayconfig.C> -- Display config implementation file

  Date      Programmer  Description
  04/01/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "displayconfig.h"
#include "xmlserializer.h"

BTDisplayExpanded::BTDisplayExpanded()
 : xMult(1), yMult(1), font(0), fontsize(6)
{
}

BTDisplayExpanded::~BTDisplayExpanded()
{
}

void BTDisplayExpanded::serialize(ObjectSerializer* s)
{
 s->add("xMult", &xMult);
 s->add("yMult", &yMult);
 s->add("directory", &directory);
 s->add("font", &font);
 s->add("fontsize", &fontsize);
}

BTDisplayConfig::BTDisplayConfig()
 : x3d(0), y3d(0), background(0), font(0), fontsize(6), xMap(0), yMap(0), widthMap(0), heightMap(0), mapDisplayMode(BTMAPDISPLAYMODE_REQUEST), centerMap(true)
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
 s->add("expanded", &expanded, &BTDisplayExpanded::create);
 s->add("background", &background);
 s->add("font", &font);
 s->add("fontsize", &fontsize);
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
 s->add("facingIcon", &icon, &BTFacingIcon::create);
 s->add("xMap", &xMap);
 s->add("yMap", &yMap);
 s->add("widthMap", &widthMap);
 s->add("heightMap", &heightMap);
 s->add("mapDisplayMode", &mapDisplayMode, NULL, &mapDisplayModeLookup);
 s->add("centerMap", &centerMap);
}

BTDisplayExpanded *BTDisplayConfig::findExpanded(int xMult, int yMult)
{
 BTDisplayExpanded *result = NULL;
 for (int i = 0; i < expanded.size(); ++i)
 {
  if ((expanded[i]->xMult <= xMult) && (expanded[i]->yMult <= yMult))
  {
   if ((!result) || ((expanded[i]->xMult > result->xMult) && (expanded[i]->xMult > result->xMult)))
    result = expanded[i];
  }
 }
 return result;
}

