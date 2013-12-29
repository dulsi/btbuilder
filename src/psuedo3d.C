/*-------------------------------------------------------------------------*\
  <psuedo3d.C> -- Psuedo3D implementation file

  Date      Programmer  Description
  01/15/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "psuedo3d.h"
#include <SDL_image.h>
#include "sdlextend.h"

int Psuedo3D::changeXY[4][2] = { {0, -1}, {1, 0}, {0, 1}, {-1, 0} };

Psuedo3D::Psuedo3D(ImageLoader *il, int xM, int yM)
 : config(NULL), imgLoad(il), xMult(xM), yMult(yM), display(NULL), background(NULL), walls(NULL), mapWalls(NULL), mapSpecial(NULL), mapUnknown(NULL)
{
 for (int i = 0; i < CARDINAL_DIRECTIONS; ++i)
 {
  mapArrows[i] = NULL;
 }
}

Psuedo3D::~Psuedo3D()
{
 clear();
 SDL_FreeSurface(display);
}

void Psuedo3D::clear()
{
 for (int i = 0; i < CARDINAL_DIRECTIONS; ++i)
 {
  if (mapArrows[i])
  {
   SDL_FreeSurface(mapArrows[i]);
   mapArrows[i] = NULL;
  }
 }
 if (mapUnknown)
 {
  SDL_FreeSurface(mapUnknown);
  mapUnknown = NULL;
 }
 if (mapSpecial)
 {
  SDL_FreeSurface(mapSpecial);
  mapSpecial = NULL;
 }
 if (mapWalls)
 {
  for (int i = 0; i < config->mapType.size(); ++i)
  {
   for (int j = 0; j < CARDINAL_DIRECTIONS; ++j)
   {
    if (mapWalls[i][j])
    {
     SDL_FreeSurface(mapWalls[i][j]);
     mapWalls[i][j] = NULL;
    }
   }
   delete [] mapWalls[i];
   mapWalls[i] = NULL;
  }
  delete [] mapWalls;
  mapWalls = NULL;
 }
 if (walls)
 {
  for (int i = 0; i < config->wallType.size(); ++i)
  {
   for (int j = 0; j < WALL_DIRECTIONS; ++j)
   {
    if (walls[i][j])
    {
     SDL_FreeSurface(walls[i][j]);
     walls[i][j] = NULL;
    }
   }
   delete [] walls[i];
   walls[i] = NULL;
  }
  delete [] walls;
  walls = NULL;
 }
 if (background)
 {
  SDL_FreeSurface(background);
  background = NULL;
 }
}

void Psuedo3D::draw(Psuedo3DMap *map, int x, int y, int direction)
{
 int i;
 int type;
 SDL_Rect src, dest;
 src.x = src.y = dest.x = dest.y = 0;
 src.h = config->height * yMult;
 src.w = config->width * xMult;
 int light = map->getLight();
 if (light)
  SDL_BlitSurface(background, &src, display, &dest);
 else
 {
  dest.h = src.h;
  dest.w = src.w;
  SDL_FillRect(display, &dest, SDL_MapRGB(display->format, 0, 0, 0));
  return;
 }
 if (light >= 4)
 {
  drawEdge(map, x + (changeXY[direction][0] * 4), y + (changeXY[direction][1] * 4), direction, WALL_EDGE_LEFT5_1, 2);
  drawFront(map, x + (changeXY[direction][0] * 3), y + (changeXY[direction][1] * 3), direction, WALL_FRONT4, 3);
 }
 if (light >= 3)
 {
  drawEdge(map, x + (changeXY[direction][0] * 3), y + (changeXY[direction][1] * 3), direction, WALL_EDGE_LEFT4_1, 2);
  drawFront(map, x + (changeXY[direction][0] * 2), y + (changeXY[direction][1] * 2), direction, WALL_FRONT3, 2);
 }
 if (light >= 2)
 {
  drawEdge(map, x + (changeXY[direction][0] * 2), y + (changeXY[direction][1] * 2), direction, WALL_EDGE_LEFT3_1, 1);
  drawFront(map, x + changeXY[direction][0], y + changeXY[direction][1], direction, WALL_FRONT2, 1);
 }
 if (light >= 1)
 {
  drawEdge(map, x + changeXY[direction][0], y + changeXY[direction][1], direction, WALL_EDGE_LEFT2, 0);
  drawFront(map, x, y, direction, WALL_FRONT1, 1);
  drawEdge(map, x, y, direction, WALL_EDGE_LEFT1, 0);
 }
}

Psuedo3DConfig *Psuedo3D::getConfig()
{
 return config;
}

SDL_Surface *Psuedo3D::getMapWall(int wallType, int direction, bool complete)
{
 int type = config->findMapType(wallType, complete);
 if (type > 0)
  return mapWalls[type - 1][direction];
 return NULL;
}

void Psuedo3D::setConfig(Psuedo3DConfig *configNew)
{
 if (config)
 {
  clear();
  if ((configNew == NULL) || (configNew->height != config->height) || (configNew->width != config->width))
  {
   SDL_FreeSurface(display);
   display = NULL;
  }
 }
 config = configNew;
 if (configNew)
 {
  background = loadImage(config->background);
  walls = new SDL_Surface_ary[config->wallType.size()];
  for (int i = 0; i < config->wallType.size(); ++i)
  {
   walls[i] = new SDL_Surface_ptr[WALL_DIRECTIONS];
   for (int j = 0; j < WALL_DIRECTIONS; ++j)
   {
    if (config->wallType[i]->walls[j])
    {
     walls[i][j] = loadImage(config->wallType[i]->walls[j]);
    }
    else
     walls[i][j] = NULL;
   }
  }
  mapWalls = new SDL_Surface_ary[config->mapType.size()];
  for (int i = 0; i < config->mapType.size(); ++i)
  {
   mapWalls[i] = new SDL_Surface_ptr[CARDINAL_DIRECTIONS];
   for (int j = 0; j < CARDINAL_DIRECTIONS; ++j)
    if (config->mapType[i]->mapWalls[j])
    {
     mapWalls[i][j] = loadImage(config->mapType[i]->mapWalls[j]);
    }
    else
     mapWalls[i][j] = NULL;
  }
  if (config->mapSpecial)
   mapSpecial = loadImage(config->mapSpecial);
  if (config->mapUnknown)
   mapUnknown = loadImage(config->mapUnknown);
  for (int i = 0; i < CARDINAL_DIRECTIONS; ++i)
  {
   if (config->mapArrows[i])
   {
    mapArrows[i] = loadImage(config->mapArrows[i]);
   }
   else
    mapArrows[i] = NULL;
  }
  if (!display)
  {
   Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
   rmask = 0xff000000;
   gmask = 0x00ff0000;
   bmask = 0x0000ff00;
   amask = 0x000000ff;
#else
   rmask = 0x000000ff;
   gmask = 0x0000ff00;
   bmask = 0x00ff0000;
   amask = 0xff000000;
#endif

   display = SDL_CreateRGBSurface(SDL_SWSURFACE, config->width * xMult, config->height * yMult, 32, rmask, gmask, bmask, amask);
   SDL_SetAlpha(display, 0, 0);
  }
 }
}

void Psuedo3D::drawEdge(Psuedo3DMap *map, int x, int y, int direction, int image, int radius)
{
 SDL_Rect src, dest;
 int type;
 int curX, curY;
 for (int i = radius; i >= 0; --i)
 {
  curX = x + (changeXY[(direction + 3) % 4][0] * i);
  curY = y + (changeXY[(direction + 3) % 4][1] * i);
  map->rationalize(curX, curY);
  type = config->findWallType(map->getWallType(curX, curY, (direction + 3) % 4), (((direction == 0) || (direction == 2)) ? curY : curX));
  if ((type > 0) && (walls[type - 1][image - i]))
  {
   src.x = 0;
   src.y = 0;
   src.w = walls[type - 1][image - i]->w;
   src.h = walls[type - 1][image - i]->h;
   dest.x = 0;
   dest.y = 0;
   SDL_BlitSurface(walls[type - 1][image - i], &src, display, &dest);
  }
 }
 for (int i = radius; i >= 0; --i)
 {
  curX = x + (changeXY[(direction + 1) % 4][0] * i);
  curY = y + (changeXY[(direction + 1) % 4][1] * i);
  map->rationalize(curX, curY);
  type = config->findWallType(map->getWallType(curX, curY, (direction + 1) % 4), (((direction == 0) || (direction == 2)) ? curY : curX));
  if ((type > 0) && (walls[type - 1][image + 1 + i]))
  {
   src.x = 0;
   src.y = 0;
   src.w = walls[type - 1][image + 1 + i]->w;
   src.h = walls[type - 1][image + 1 + i]->h;
   dest.x = 0;
   dest.y = 0;
   SDL_BlitSurface(walls[type - 1][image + 1 + i], &src, display, &dest);
  }
 }
}

void Psuedo3D::drawFront(Psuedo3DMap *map, int x, int y, int direction, int image, int radius)
{
 SDL_Rect src, dest;
 int type;
 int curX, curY;
 for (int i = -1 * radius; i <= radius; ++i)
 {
  curX = x + (changeXY[(direction + 1) % 4][0] * i);
  curY = y + (changeXY[(direction + 1) % 4][1] * i);
  map->rationalize(curX, curY);
  type = config->findWallType(map->getWallType(curX, curY, direction), (((direction == 0) || (direction == 2)) ? curX : curY));
  if ((type > 0) && (walls[type - 1][image]))
  {
   int w = ((config->width * xMult) - walls[type - 1][image]->w) / 2;
   int xPos = w + (walls[type - 1][image]->w * i);
   if (xPos >= 0)
   {
    src.x = 0;
    dest.x = xPos;
    if (xPos + walls[type - 1][image]->w <= (config->width * xMult))
     src.w = walls[type - 1][image]->w;
    else
     src.w = (config->width * xMult) - xPos;
   }
   else
   {
    src.x = xPos * -1;
    dest.x = 0;
    src.w = walls[type - 1][image]->w + xPos;
   }
   src.y = 0;
   src.h = walls[type - 1][image]->h;
   dest.y = 0;
   SDL_BlitSurface(walls[type - 1][image], &src, display, &dest);
  }
 }
}

SDL_Surface *Psuedo3D::loadImage(const char *file)
{
 SDL_Surface *img = NULL;
 imgLoad->loadImageOrAnimation(file, &img, NULL);
 if (NULL == img)
 {
  printf((std::string("Failed - Loading image/") + file + std::string("\n")).c_str());
  exit(0);
 }
 return img;
}

