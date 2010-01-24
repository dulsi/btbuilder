#ifndef __ICON_H
#define __ICON_H
/*-------------------------------------------------------------------------*\
  <icon.h> -- Icon header file

  Date      Programmer  Description
  01/23/10  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "xmlserializer.h"
#include "serialrect.h"
#include <SDL.h>

class BTDisplay;

class BTIcon : public XMLObject
{
 public:
  BTIcon() : image(0), effect(0), img(0) {}
  ~BTIcon();

  void clear(BTDisplay &d);
  void draw(BTDisplay &d);
  bool isActive();
  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTIcon; }

 private:
  char *image;
  SerialRect position;
  int effect;
  SDL_Surface *img;
};

#endif

