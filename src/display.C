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
 : xMult(xM), yMult(yM), x3d(16), y3d(15), p3d(xM, yM), mainScreen(0)
{
 if (SDL_Init(SDL_INIT_VIDEO) < 0)
 {
  printf("Failed - SDL_Init\n");
  exit(0);
 }
 mainScreen = SDL_SetVideoMode(320 * xM, 200 * yM, 32,
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
}

BTDisplay::~BTDisplay()
{
 SDL_Quit();
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
