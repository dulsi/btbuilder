#ifndef __MODULE_H
#define __MODULE_H
/*-------------------------------------------------------------------------*\
  <module.h> -- Module header file

  Date      Programmer  Description
  03/19/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "xmlserializer.h"

class BTModule : public XMLObject
{
 public:
  BTModule();
  ~BTModule();

  virtual void serialize(ObjectSerializer* s);

  char *name;
  char *author;
  char *content;
  char *item;
  char *monster;
  char *spell;
  char *race;
  char *skill;
  char *song;
  char *xpChart;
  char *job;
  char *title;
  char *wall;
  std::string startMap;
  int startX;
  int startY;
  int startFace;
  unsigned int maxItems;
  unsigned int maxTime;
  unsigned int nightTime;
};

#endif

