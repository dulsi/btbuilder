#ifndef __PSUEDO3DCONFIG_H
#define __PSUEDO3DCONFIG_H
/*-------------------------------------------------------------------------*\
  <psuedo3dconfig.h> -- Psuedo3DConfig header file

  Date      Programmer  Description
  01/15/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "xmlserializer.h"

#define WALL_DIRECTIONS 24

#define WALL_FRONT1 0
#define WALL_FRONT2 1
#define WALL_FRONT3 2
#define WALL_FRONT4 3
#define WALL_EDGE_LEFT1 4
#define WALL_EDGE_RIGHT1 5
#define WALL_EDGE_LEFT2 6
#define WALL_EDGE_RIGHT2 7
#define WALL_EDGE_LEFT3_2 8
#define WALL_EDGE_LEFT3_1 9
#define WALL_EDGE_RIGHT3_1 10
#define WALL_EDGE_RIGHT3_2 11
#define WALL_EDGE_LEFT4_3 12
#define WALL_EDGE_LEFT4_2 13
#define WALL_EDGE_LEFT4_1 14
#define WALL_EDGE_RIGHT4_1 15
#define WALL_EDGE_RIGHT4_2 16
#define WALL_EDGE_RIGHT4_3 17
#define WALL_EDGE_LEFT5_3 18
#define WALL_EDGE_LEFT5_2 19
#define WALL_EDGE_LEFT5_1 20
#define WALL_EDGE_RIGHT5_1 21
#define WALL_EDGE_RIGHT5_2 22
#define WALL_EDGE_RIGHT5_3 23

class Psuedo3DWallType : public XMLObject
{
 public:
  Psuedo3DWallType()
  {
   for (int i = 0; i < WALL_DIRECTIONS; ++i)
    walls[i] = 0;
  }

  ~Psuedo3DWallType()
  {
   for (int i = 0; i < WALL_DIRECTIONS; ++i)
     if (walls[i])
      delete [] walls[i];
  }

  virtual void serialize(XMLSerializer* s);

  static XMLObject *create() { return new Psuedo3DWallType; }

  char *walls[WALL_DIRECTIONS];
};

class Psuedo3DConfig : public XMLObject
{
 public:
  Psuedo3DConfig()
   : height(0), width(0), background(0)
  {
  }

  ~Psuedo3DConfig()
  {
   if (background)
    delete [] background;
  }

  virtual void serialize(XMLSerializer* s);

  static XMLObject *create() { return new Psuedo3DConfig; }
  static void readXML(char *filename, XMLVector<Psuedo3DConfig*> &cfg);

  int height, width;
  char *background;
  XMLVector<Psuedo3DWallType*> wallType;
};

#endif
