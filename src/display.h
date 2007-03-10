#ifndef __DISPLAY_H
#define __DISPLAY_H
/*-------------------------------------------------------------------------*\
  <display.h> -- Display header file

  Date      Programmer  Description
  02/12/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include <istdlib.h>
#include "psuedo3d.h"
#include <SDL_ttf.h>
#include "sdlextend.h"

class BTDisplay
{
 public:
  BTDisplay(int xM = 0, int yM = 0);
  ~BTDisplay();

  void clearText();
  void drawFullScreen(const char *file, int delay);
  void drawImage(const char *file);
  void drawLabel(const char *name);
  void drawText(const char *words);
  void drawView();
  void setWallGraphics(int type);

 private:
  int xMult, yMult;
  int x3d, y3d;
  SDL_Rect label;
  SDL_Rect text;
  int textPos;
  Psuedo3D p3d;
  XMLVector<Psuedo3DConfig*> p3dConfig;
  SDL_Surface *mainScreen;
  SDL_Surface *mainBackground;
  TTF_Font *ttffont;
  simpleFont *sfont;
  SDL_Color white, black;
};

#endif

