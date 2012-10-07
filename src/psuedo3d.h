#ifndef __PSUEDO3D_H
#define __PSUEDO3D_H
/*-------------------------------------------------------------------------*\
  <psuedo3d.h> -- Psuedo3D header file

  Date      Programmer  Description
  01/15/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "psuedo3dconfig.h"
#include "psuedo3dmap.h"
#include "SDL_mng.h"
#include <SDL.h>

class ImageLoader
{
 public:
  virtual void loadImageOrAnimation(const char *file, SDL_Surface **img, MNG_Image **animation, bool physfs = true) = 0;
};

class Psuedo3D
{
 public:
  Psuedo3D(ImageLoader *il, int xM, int yM);
  ~Psuedo3D();

  void clear();
  void draw(Psuedo3DMap *map, int x, int y, int direction);
  Psuedo3DConfig *getConfig();
  SDL_Surface *getDisplay() { return display; }
  SDL_Surface *getMapWall(int mapType, int direction, bool complete);
  SDL_Surface *getMapSpecial() { return mapSpecial; }
  SDL_Surface *getMapUnknown() { return mapUnknown; }
  SDL_Surface *getMapArrow(int direction) { return mapArrows[direction]; }
  void setConfig(Psuedo3DConfig *configNew);
  void setMultiplier(int xM, int yM) { xMult = xM; yMult = yM; }

 protected:
  void drawEdge(Psuedo3DMap *map, int x, int y, int direction, int image, int radius);
  void drawFront(Psuedo3DMap *map, int x, int y, int direction, int image, int radius);
  SDL_Surface *loadImage(const char *file);

 public:
  typedef SDL_Surface *SDL_Surface_ptr;
  typedef SDL_Surface **SDL_Surface_ary;
  Psuedo3DConfig *config;
  ImageLoader *imgLoad;
  int xMult, yMult;
  SDL_Surface *display;
  SDL_Surface *background;
  SDL_Surface ***walls;
  SDL_Surface ***mapWalls;
  SDL_Surface *mapSpecial;
  SDL_Surface *mapUnknown;
  SDL_Surface *mapArrows[CARDINAL_DIRECTIONS];

  static int changeXY[4][2];
};

#endif
