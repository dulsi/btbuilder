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
  std::string imageDirectory;
  char *font;
  int fontsize;
};

class BTDisplayColor : public XMLObject
{
 public:
  BTDisplayColor();
  ~BTDisplayColor();

  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTDisplayColor; }

 public:
  std::string name;
  SDL_Color rgb;
};

class BTWidgetConfig : public XMLObject
{
 public:
  BTWidgetConfig();
  ~BTWidgetConfig();

  virtual void serialize(ObjectSerializer* s);

 public:
  std::string name;
};

class BTLabelConfig : public BTWidgetConfig
{
 public:
  BTLabelConfig();
  ~BTLabelConfig();

  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTLabelConfig; }

 public:
  SerialRect location;
  std::string color;
};

class BTTextConfig : public BTWidgetConfig
{
 public:
  BTTextConfig();
  ~BTTextConfig();

  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTTextConfig; }

 public:
  SerialRect location;
};

class BTLayoutConfig : public XMLObject
{
 public:
  BTLayoutConfig();
  ~BTLayoutConfig();

  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTLayoutConfig; }

 public:
  std::string name;
  std::string background;
  XMLVector<BTWidgetConfig*> widgets;
  bool visible;
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
  XMLVector<BTDisplayColor*> color;
  XMLVector<BTLayoutConfig*> layout;
  XMLVector<SerialRect*> status;
  char *background;
  char *font;
  int fontsize;
  XMLVector<BTStatusInfo*> statusInfo;
  XMLVector<BTIcon*> icon;
  int xMap, yMap;
  int widthMap, heightMap;
  int mapDisplayMode;
  bool centerMap;
  bool coordinatesMap;
};

#endif
