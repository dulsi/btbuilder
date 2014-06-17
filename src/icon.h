#ifndef __ICON_H
#define __ICON_H
/*-------------------------------------------------------------------------*\
  <icon.h> -- Icon header file

  Date      Programmer  Description
  01/23/10  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "xmlserializer.h"
#include "serialrect.h"
#include "btconst.h"
#include <SDL.h>
#include <SDL_mng.h>

class BTDisplay;

class BTIcon : public XMLObject
{
 public:
  BTIcon() : image(0), effect(0), party(false), active(false), img(0) { animation.animation = 0; }
  ~BTIcon();

  virtual void draw(BTDisplay &d, unsigned long ticks);
  bool isActive();
  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTIcon; }

 protected:
  char *image;
  SerialRect position;
  int effect;
  bool party;
  bool active;
  SDL_Surface *img;
  MNG_AnimationState animation;
};

class BTFacingIcon : public BTIcon
{
 public:
  BTFacingIcon();
  ~BTFacingIcon();

  void draw(BTDisplay &d, unsigned long ticks);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTFacingIcon; }

 private:
  SDL_Surface *dirImg[BT_DIRECTIONS];
  MNG_AnimationState dirAni[BT_DIRECTIONS];
  int facing;
};

#endif

