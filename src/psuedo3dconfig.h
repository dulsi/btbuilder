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

#define CARDINAL_DIRECTIONS 4

class Psuedo3DWallType : public XMLObject
{
 public:
  Psuedo3DWallType()
   : type(-1)
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

  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new Psuedo3DWallType; }

  int type;
  std::vector<unsigned int> modulus;
  char *walls[WALL_DIRECTIONS];
};

class Psuedo3DMapType : public XMLObject
{
 public:
  Psuedo3DMapType()
   : type(-1), incompleteType(-1), viewType(-1), passable(false), invincible(false)
  {
   for (int i = 0; i < CARDINAL_DIRECTIONS; ++i)
    mapWalls[i] = 0;
  }

  ~Psuedo3DMapType()
  {
   for (int i = 0; i < CARDINAL_DIRECTIONS; ++i)
     if (mapWalls[i])
      delete [] mapWalls[i];
  }

  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new Psuedo3DMapType; }

  int type;
  std::string name;
  int incompleteType;
  int viewType;
  bool passable;
  bool invincible;
  char *mapWalls[CARDINAL_DIRECTIONS];
};

class Psuedo3DConfig : public XMLObject
{
 public:
  Psuedo3DConfig()
   : height(0), width(0), background(0), mapSpecial(0), mapUnknown(0)
  {
   for (int i = 0; i < CARDINAL_DIRECTIONS; ++i)
    mapArrows[i] = 0;
  }

  ~Psuedo3DConfig()
  {
   if (background)
    delete [] background;
  }

  int findWallType(int type, int position);
  int findMapType(int type, bool complete);
  virtual void serialize(ObjectSerializer* s);
  bool validate();

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new Psuedo3DConfig; }
  static void readXML(const char *filename, XMLVector<Psuedo3DConfig*> &cfg);

  std::string name;
  int height, width;
  int divide;
  char *background;
  XMLVector<Psuedo3DWallType*> wallType;
  int mapHeight, mapWidth;
  XMLVector<Psuedo3DMapType*> mapType;
  char *mapSpecial;
  char *mapUnknown;
  char *mapArrows[CARDINAL_DIRECTIONS];
};

class Psuedo3DConfigList : public ValueLookup, public XMLVector<Psuedo3DConfig*>
{
 public:
  virtual std::string getName(int index);
  virtual int getIndex(std::string name);
  virtual size_t size();
};

#endif
