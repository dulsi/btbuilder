/*-------------------------------------------------------------------------*\
  <icon.C> -- Icon implementation file

  Date      Programmer  Description
  01/23/10  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "icon.h"
#include "display.h"
#include "game.h"
#include "physfsrwops.h"
#include <SDL_image.h>

BTIcon::~BTIcon()
{
 if (img)
 {
  SDL_FreeSurface(img);
 }
}

void BTIcon::clear(BTDisplay &d)
{
 int xMult, yMult;
 SDL_Rect dst;
 d.getMultiplier(xMult, yMult);
 dst.x = position.x * xMult;
 dst.y = position.y * yMult;
 dst.w = position.w * xMult;
 dst.h = position.h * yMult;
 d.clear(dst, true);
}

void BTIcon::draw(BTDisplay &d)
{
 int xMult, yMult;
 SDL_Rect dst;
 d.getMultiplier(xMult, yMult);
 if (NULL == img)
 {
  SDL_RWops *f = PHYSFSRWOPS_openRead(image);
  img = IMG_Load_RW(f, 1);
  if (position.w != img->w)
  {
   position.w = img->w;
  }
  if (position.h != img->h)
  {
   position.h = img->h;
  }
  if ((xMult > 1) || (yMult > 1))
  {
   SDL_Surface *img2 = simpleZoomSurface(img, xMult, yMult);
   SDL_FreeSurface(img);
   img = img2;
  }
 }
 dst.x = position.x * xMult;
 dst.y = position.y * yMult;
 dst.w = position.w * xMult;
 dst.h = position.h * yMult;
 d.drawImage(dst, img);
}

bool BTIcon::isActive()
{
 return BTGame::getGame()->hasEffectOfType(effect);
}

void BTIcon::serialize(ObjectSerializer* s)
{
 s->add("image", &image);
 s->add("position", &position);
 s->add("effect", &effect);
}

