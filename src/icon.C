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
 if (animation.animation)
 {
  IMG_FreeMNG(animation.animation);
 }
}

void BTIcon::draw(BTDisplay &d, unsigned long ticks)
{
 int xMult, yMult;
 SDL_Rect dst;
 d.getMultiplier(xMult, yMult);
 if ((NULL == img) && (NULL == animation.animation))
 {
  d.loadImageOrAnimation(image, &img, &animation.animation);
 }
 dst.x = position.x * xMult;
 dst.y = position.y * yMult;
 dst.w = position.w * xMult;
 dst.h = position.h * yMult;
 if ((isActive()) && (active == false))
 {
  if (img)
  {
   d.drawImage(dst, img);
  }
  else
  {
   IMG_SetAnimationState(&animation, -1, 0);
   animation.dst = dst;
   d.addAnimation(&animation, true);
  }
  active = true;
 }
 else if ((!isActive()) && (active))
 {
  d.clear(dst, true);
  if (animation.animation)
   d.removeAnimation(&animation);
  active = false;
 }
}

bool BTIcon::isActive()
{
 if (party)
  return BTGame::getGame()->hasEffectOfType(effect, BTTARGET_PARTY, true);
 else
  return BTGame::getGame()->hasEffectOfType(effect);
}

void BTIcon::serialize(ObjectSerializer* s)
{
 s->add("image", &image);
 s->add("position", &position);
 s->add("effect", &effect);
 s->add("party", &party);
}

BTFacingIcon::BTFacingIcon()
 : facing (-1)
{
 for (int i = 0; i < BT_DIRECTIONS; ++i)
 {
  dirImg[i] = 0;
  dirAni[i].animation = 0;
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
  if (dirAni[i].animation)
  {
   IMG_FreeMNG(dirAni[i].animation);
  }
 }
}

void BTFacingIcon::draw(BTDisplay &d, unsigned long ticks)
{
 bool oldActive = active;
 BTIcon::draw(d, ticks);
 if (active)
 {
  int xMult, yMult;
  SDL_Rect dst;
  BTGame *g = BTGame::getGame();
  int newFacing = g->getFacing();
  if (newFacing != facing)
  {
   d.getMultiplier(xMult, yMult);
   dst.x = position.x * xMult;
   dst.y = position.y * yMult;
   dst.w = position.w * xMult;
   dst.h = position.h * yMult;
   if (dirAni[facing].animation)
    d.removeAnimation(&dirAni[facing]);
   d.clear(dst, true);
   if (animation.animation)
    d.drawImage(animation.dst, animation.animation->frame[animation.frame]);
   else
   {
    d.drawImage(dst, img);
   }
   facing = newFacing;
   if ((NULL == dirImg[facing]) && (NULL == dirAni[facing].animation))
   {
    const char *period = strrchr(image, '.');
    std::string filename(image, period - image);
    filename.append(1, '0' + facing);
    filename += period;
    d.loadImageOrAnimation(filename.c_str(), &dirImg[facing], &dirAni[facing].animation);
   }
   if (dirImg[facing])
    d.drawImage(dst, dirImg[facing]);
   else
   {
    IMG_SetAnimationState(&dirAni[facing], -1, 0);
    dirAni[facing].dst = dst;
    d.addAnimation(&dirAni[facing]);
   }
  }
 }
 else
 {
  if (oldActive)
  {
   d.removeAnimation(&dirAni[facing]);
  }
  facing = -1;
 }
}

