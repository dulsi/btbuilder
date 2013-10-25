/*-------------------------------------------------------------------------*\
  <psuedo3dmap.C> -- Psuedo3DMap implementation file

  Date      Programmer  Description
  10/08/12  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "psuedo3dmap.h"

Psuedo3DMap *Psuedo3DMap::map = 0;

Psuedo3DMap::Psuedo3DMap()
 : xPos(0), yPos(0), facing(0)
{
 map = this;
}

int Psuedo3DMap::getFacing()
{
 return facing;
}

int Psuedo3DMap::getX()
{
 return xPos;
}

int Psuedo3DMap::getY()
{
 return yPos;
}

Psuedo3DMap *Psuedo3DMap::getMap()
{
 return map;
}

void Psuedo3DMap::rationalize(int &x, int &y)
{
 if (x < 0)
  x += map->getXSize();
 x = x % map->getXSize();
 if (y < 0)
  y += map->getYSize();
 y = y % map->getYSize();
}

