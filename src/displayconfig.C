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
 s->add("imageDirectory", &imageDirectory);
 s->add("font", &font);
 s->add("fontsize", &fontsize);
}

BTDisplayColor::BTDisplayColor()
{
#ifdef SDL2LIB
 rgb.a = 255;
#endif
}

BTDisplayColor::~BTDisplayColor()
{
}

void BTDisplayColor::serialize(ObjectSerializer* s)
{
 s->add("name", &name);
 s->add("r", &rgb.r);
 s->add("g", &rgb.g);
 s->add("b", &rgb.b);
}

BTWidgetConfig::BTWidgetConfig()
{
}

BTWidgetConfig::~BTWidgetConfig()
{
}

void BTWidgetConfig::serialize(ObjectSerializer* s)
{
 s->add("name", &name);
}

BTLabelConfig::BTLabelConfig()
 : color("white")
{
}

BTLabelConfig::~BTLabelConfig()
{
}

void BTLabelConfig::serialize(ObjectSerializer* s)
{
 BTWidgetConfig::serialize(s);
 s->add("location", &location);
 s->add("color", &color);
}

BTTextConfig::BTTextConfig()
{
}

BTTextConfig::~BTTextConfig()
{
}

void BTTextConfig::serialize(ObjectSerializer* s)
{
 BTWidgetConfig::serialize(s);
 s->add("location", &location);
}

BTLayoutConfig::BTLayoutConfig()
 : visible(true)
{
}

BTLayoutConfig::~BTLayoutConfig()
{
}

void BTLayoutConfig::serialize(ObjectSerializer* s)
{
 s->add("name", &name);
 s->add("background", &background);
 s->add("label", &widgets, &BTLabelConfig::create);
 s->add("text", &widgets, &BTTextConfig::create);
 s->add("visible", &visible);
}

BTDisplayConfig::BTDisplayConfig()
 : x3d(0), y3d(0), background(0), font(0), fontsize(6), xMap(0), yMap(0), widthMap(0), heightMap(0), mapDisplayMode(BTMAPDISPLAYMODE_REQUEST), centerMap(true), coordinatesMap(false)
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
 s->add("color", &color, &BTDisplayColor::create);
 s->add("layout", &layout, &BTLayoutConfig::create);
 s->add("status", &status, &SerialRect::create);
 s->add("statBlock", &statusInfo, &BTStatBlock::create);
 s->add("conditional", &statusInfo, &BTConditional::create);
 s->add("print", &statusInfo, &BTPrint::create);
 s->add("statusIcon", &statusInfo, &BTStatusIcon::create);
 s->add("icon", &icon, &BTIcon::create);
 s->add("facingIcon", &icon, &BTFacingIcon::create);
 s->add("xMap", &xMap);
 s->add("yMap", &yMap);
 s->add("widthMap", &widthMap);
 s->add("heightMap", &heightMap);
 s->add("mapDisplayMode", &mapDisplayMode, NULL, &mapDisplayModeLookup);
 s->add("centerMap", &centerMap);
 s->add("coordinatesMap", &coordinatesMap);
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

