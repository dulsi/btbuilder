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
  Psuedo3DMap();

  virtual int getLight() = 0;
  int getFacing();
  int getX();
  int getY();
  virtual int getXSize() const = 0;
  virtual int getYSize() const = 0;
  virtual int getKnowledge(int x, int y) = 0;
  virtual int getMapType(int x, int y, int direction) = 0;
  virtual int getWallType(int x, int y, int direction) = 0;
  virtual bool hasSpecial(int x, int y) = 0;
  void rationalize(int &x, int &y);

  static Psuedo3DMap *getMap();

 protected:
  int xPos, yPos, facing;

 private:
  static Psuedo3DMap *map;
};

#endif
