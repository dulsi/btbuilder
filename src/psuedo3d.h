#ifndef __PSUEDO3D_H
#define __PSUEDO3D_H
/*-------------------------------------------------------------------------*\
  <psuedo3d.h> -- Psuedo3D header file

  Date      Programmer  Description
  01/15/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "psuedo3dconfig.h"
#include "psuedo3dmap.h"
#include <SDL.h>

class Psuedo3D
{
 public:
  Psuedo3D();
  ~Psuedo3D();

  void clear();
  void draw(Psuedo3DMap *map, int x, int y, int direction);
  SDL_Surface *getDisplay() { return display; }
  void setConfig(Psuedo3DConfig *configNew);

 protected:
  void drawEdge(Psuedo3DMap *map, int x, int y, int direction, int image, int radius);
  void drawFront(Psuedo3DMap *map, int x, int y, int direction, int image, int radius);

 public:
  Psuedo3DConfig *config;
  SDL_Surface *display;
  SDL_Surface *background;
  SDL_Surface *walls[WALL_TYPES][WALL_DIRECTIONS];

  static int changeXY[4][2];
};

#endif
