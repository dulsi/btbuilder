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

class BTDisplayConfig : public XMLObject
{
 public:
  BTDisplayConfig();
  ~BTDisplayConfig();

  virtual void serialize(ObjectSerializer* s);

 public:
  int width, height;
  int x3d, y3d;
  SerialRect label;
  SerialRect text;
  SerialRect status[BT_PARTYSIZE];
  char *background;
  char *font;
  XMLVector<BTStatusInfo*> statusInfo;
  XMLVector<BTIcon*> icon;
};

#endif
