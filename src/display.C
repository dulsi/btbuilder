/*-------------------------------------------------------------------------*\
  <display.C> -- Display implementation file

  Date      Programmer  Description
  02/28/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "display.h"
#include "game.h"
#include "ikbbuffer.h"
#include <SDL_image.h>

BTDisplay::BTDisplay(int xM, int yM)
 : xMult(xM), yMult(yM), x3d(16), y3d(15), textPos(0), p3d(xM, yM), mainScreen(0)
{
 if (SDL_Init(SDL_INIT_VIDEO) < 0)
 {
  printf("Failed - SDL_Init\n");
  exit(0);
 }
 if ((xMult == 0) || (yMult == 0))
 {
  const SDL_VideoInfo *info = SDL_GetVideoInfo();
  xMult = (info->current_w - 10) / 320; // Allow for window decoration
  yMult = (info->current_h - 10) / 200; // Allow for window decoration
  if (xMult > yMult)
   xMult = yMult;
  else
   yMult = xMult;
  p3d.setMultiplier(xMult, yMult);
 }
 label.x = 16 * xMult;
 label.y = 103 * yMult;
 label.w = 112 * xMult;
 label.h = 13 * yMult;
 text.x = 168 * xMult;
 text.y = 8 * yMult;
 text.w = 136 * xMult;
 text.h = 94 * yMult;
 if (TTF_Init() == -1)
 {
  printf("Failed - TTF_Init\n");
  exit(0);
 }
 mainScreen = SDL_SetVideoMode(320 * xMult, 200 * yMult, 32,
   SDL_SWSURFACE /*| (fullScreen ? SDL_FULLSCREEN : 0)*/);
 if (mainScreen == NULL)
 {
  printf("Failed - SDL_SetVideoMode\n");
  exit(0);
 }
 SDL_Surface *img = IMG_Load("image/mainscreen.png");
 if ((xMult > 1) || (yMult > 1))
 {
  mainBackground = simpleZoomSurface(img, xMult, yMult);
  SDL_FreeSurface(img);
 }
 else
  mainBackground = img;
 SDL_BlitSurface(mainBackground, NULL, mainScreen, NULL);
 Psuedo3DConfig::readXML("data/wall.xml", p3dConfig);
 font = TTF_OpenFont("/usr/share/fonts/bitstream-vera/VeraMono.ttf", 8 * ((xMult == yMult) ? yMult : 1));
 white.r = 255;
 white.g = 255;
 white.b = 255;
 black.r = 0;
 black.g = 0;
 black.b = 0;
}

BTDisplay::~BTDisplay()
{
 SDL_Quit();
}

void BTDisplay::clearText()
{
 SDL_BlitSurface(mainBackground, &text, mainScreen, &text);
}

void BTDisplay::drawFullScreen(const char *file, int delay)
{
 SDL_Surface *img = IMG_Load(file);
 // HACK: Bug in SDL's lbm loading code
 if ((img->format->BitsPerPixel == 8) && (img->format->palette->ncolors < 256))
  img->format->palette->ncolors = 256;
 if ((xMult > 1) || (yMult > 1))
 {
  SDL_Surface *img2 = simpleZoomSurface(img, xMult, yMult);
  SDL_FreeSurface(img);
  img = img2;
 }
 SDL_BlitSurface(img, NULL, mainScreen, NULL);
 SDL_UpdateRect(mainScreen, 0, 0, 0, 0);
 SDL_FreeSurface(img);
 if (delay)
  SDL_Delay(delay);
 else
  IKeybufferGet();
 SDL_BlitSurface(mainBackground, NULL, mainScreen, NULL);
}

void BTDisplay::drawLabel(const char *name)
{
 int w, h;
 if (TTF_SizeUTF8(font, name, &w, &h) == -1)
  return;
 SDL_Surface *img = TTF_RenderUTF8_Solid(font, name, white);
 if (xMult != yMult)
 {
  SDL_Surface *img2 = simpleZoomSurface(img, xMult, yMult);
  SDL_FreeSurface(img);
  img = img2;
  w *= xMult;
  h *= yMult;
 }
 SDL_Rect src, dst;
 src.x = 0;
 src.y = 0;
 src.w = ((w > label.w) ? label.w : w);
 src.h = ((h > label.h) ? label.h : h);
 dst.x = ((w > label.w) ? label.x : label.x + (label.w / 2) - (w / 2));
 dst.y = ((h > label.h) ? label.y : label.y + (label.h / 2) - (h / 2));
 dst.w = src.w;
 dst.h = src.h;
 SDL_BlitSurface(mainBackground, &label, mainScreen, &label);
 SDL_BlitSurface(img, &src, mainScreen, &dst);
 SDL_UpdateRect(mainScreen, label.x, label.y, label.w, label.h);
 SDL_FreeSurface(img);
}

void BTDisplay::drawText(const char *words)
{
 int w, h;
 char *tmp = new char[strlen(words)];
 const char *partial = words;
 while (partial)
 {
  if (TTF_SizeUTF8(font, partial, &w, &h) == -1)
  {
   delete [] tmp;
   return;
  }
  if (xMult != yMult)
  {
   w *= xMult;
   h *= yMult;
  }
  if (h + textPos > text.h)
  {
   SDL_Rect src, dst;
   src.x = dst.x = text.x;
   src.y = text.y + h;
   src.w = dst.w = text.w;
   src.h = dst.h = text.h - h;
   dst.y = text.y;
   SDL_BlitSurface(mainScreen, &src, mainScreen, &dst);
   src.x = text.x;
   src.y = text.y + text.h - h;
   src.w = text.w;
   src.h = h;
   SDL_BlitSurface(mainBackground, &src, mainScreen, &src);
   SDL_UpdateRect(mainScreen, text.x, text.y, text.w, text.h);
   textPos -= h;
  }
  const char *end = NULL;
  if (w > text.w)
  {
   const char *sp = partial;
   for (end = partial; *end; ++end)
   {
    if (isspace(*end))
    {
     memcpy(tmp + (sp - partial), sp, end - sp);
     tmp[end - partial] = 0;
     TTF_SizeUTF8(font, tmp, &w, &h);
     if (xMult != yMult)
     {
      w *= xMult;
      h *= yMult;
     }
     if (w > text.w)
     {
      end = sp;
      break;
     }
     sp = end;
    }
   }
   if (!(*end))
    end = sp;
   if (end == partial)
    end = NULL;
   else
   {
    while (isspace(*end))
     ++end;
    if (!(*end))
     end = NULL;
   }
  }
  SDL_Surface *img = TTF_RenderUTF8_Solid(font, (end ? tmp : partial), black);
  if (xMult != yMult)
  {
   SDL_Surface *img2 = simpleZoomSurface(img, xMult, yMult);
   SDL_FreeSurface(img);
   img = img2;
  }
  SDL_Rect src, dst;
  src.x = 0;
  src.y = 0;
  src.w = ((img->w > text.w) ? text.w : w);
  src.h = ((img->h > text.h) ? text.h : h);
  dst.x = text.x;
  dst.y = text.y + textPos;
  dst.w = src.w;
  dst.h = src.h;
  textPos += src.h;
  SDL_BlitSurface(img, &src, mainScreen, &dst);
  SDL_UpdateRect(mainScreen, text.x, text.y, text.w, text.h);
  SDL_FreeSurface(img);
  partial = end;
 }
}

void BTDisplay::drawView()
{
 BTGame *g = BTGame::getGame();
 p3d.draw(g, g->getX(), g->getY(), g->getFacing());
 SDL_Rect src, dst;
 src.x = 0;
 src.y = 0;
 src.w = p3d.config->width * xMult;
 src.h = p3d.config->height * yMult;
 dst.x = x3d * xMult;
 dst.y = y3d * yMult;
 dst.w = p3d.config->width * xMult;
 dst.h = p3d.config->height * yMult;
 SDL_BlitSurface(p3d.getDisplay(), &src, mainScreen, &dst);
 SDL_UpdateRect(mainScreen, 0, 0, 0, 0);
}

void BTDisplay::setWallGraphics(int type)
{
 p3d.setConfig(p3dConfig[type]);
}
