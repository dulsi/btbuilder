#ifndef __DISPLAYCONFIG_H
#define __DISPLAYCONFIG_H
/*-------------------------------------------------------------------------*\
  <displayconfig.h> -- Display config header file

  Date      Programmer  Description
  04/01/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "xmlserializer.h"
#include "icon.h"
#include "serialrect.h"
#include "statusinfo.h"
#include "btconst.h"

class BTDisplayExpanded : public XMLObject
{
 public:
  BTDisplayExpanded();
  ~BTDisplayExpanded();

  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTDisplayExpanded; }

 public:
  int xMult, yMult;
  std::string directory;
  char *font;
  int fontsize;
};

class BTDisplayConfig : public XMLObject
{
 public:
  BTDisplayConfig();
  ~BTDisplayConfig();

  virtual void serialize(ObjectSerializer* s);

  BTDisplayExpanded *findExpanded(int xMult, int yMult);

 public:
  int width, height;
  XMLVector<BTDisplayExpanded*> expanded;
  int x3d, y3d;
  SerialRect label;
  SerialRect text;
  SerialRect status[BT_PARTYSIZE];
  char *background;
  char *font;
  int fontsize;
  XMLVector<BTStatusInfo*> statusInfo;
  XMLVector<BTIcon*> icon;
  int xMap, yMap;
  int widthMap, heightMap;
  int mapDisplayMode;
  bool centerMap;
};

#endif
