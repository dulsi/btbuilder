#ifndef __PSUEDO3DMAP_H
#define __PSUEDO3DMAP_H
/*-------------------------------------------------------------------------*\
  <psuedo3dmap.h> -- Psuedo3DMap header file

  Date      Programmer  Description
  01/16/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

class Psuedo3DMap
{
 public:
  virtual int getLight() = 0;
  virtual int getWallType(int x, int y, int direction) = 0;
};

#endif
