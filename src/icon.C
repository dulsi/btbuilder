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
 if (animation)
 {
  IMG_FreeMNG(animation);
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
 if ((NULL == img) && (NULL == animation))
 {
  d.loadImageOrAnimation(image, &img, &animation);
 }
 dst.x = position.x * xMult;
 dst.y = position.y * yMult;
 dst.w = position.w * xMult;
 dst.h = position.h * yMult;
 d.clear(dst, false);
 if (img)
  d.drawImage(dst, img);
 else
  d.drawImage(dst, animation->frame[0]);
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

BTFacingIcon::BTFacingIcon()
{
 for (int i = 0; i < BT_DIRECTIONS; ++i)
 {
  dirImg[i] = 0;
  dirAni[i] = 0;
 }
}

BTFacingIcon::~BTFacingIcon()
{
 for (int i = 0; i < BT_DIRECTIONS; ++i)
 {
  if (dirImg[i])
  {
   SDL_FreeSurface(dirImg[i]);
  }
  if (dirAni[i])
  {
   IMG_FreeMNG(dirAni[i]);
  }
 }
}

void BTFacingIcon::draw(BTDisplay &d)
{
 int xMult, yMult;
 SDL_Rect dst;
 BTIcon::draw(d);
 BTGame *g = BTGame::getGame();
 int facing = g->getFacing();
 d.getMultiplier(xMult, yMult);
 if ((NULL == dirImg[facing]) && (NULL == dirAni[facing]))
 {
  const char *period = strrchr(image, '.');
  std::string filename(image, period - image);
  filename.append(1, '0' + facing);
  filename += period;
  d.loadImageOrAnimation(filename.c_str(), &dirImg[facing], &dirAni[facing]);
 }
 dst.x = position.x * xMult;
 dst.y = position.y * yMult;
 dst.w = position.w * xMult;
 dst.h = position.h * yMult;
 if (dirImg[facing])
  d.drawImage(dst, dirImg[facing]);
 else
  d.drawImage(dst, dirAni[facing]->frame[0]);
}

