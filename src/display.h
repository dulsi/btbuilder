#ifndef __DISPLAY_H
#define __DISPLAY_H
/*-------------------------------------------------------------------------*\
  <display.h> -- Display header file

  Date      Programmer  Description
  02/12/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include <istdlib.h>
#include "psuedo3d.h"

class BTDisplay
{
 public:
  BTDisplay(int xM, int yM);
  ~BTDisplay();

  void drawView();
  void setWallGraphics(int type);

 private:
  int xMult, yMult;
  int x3d, y3d;
  Psuedo3D p3d;
  XMLVector<Psuedo3DConfig*> p3dConfig;
  SDL_Surface *mainScreen;
  SDL_Surface *mainBackground;
};

#endif

