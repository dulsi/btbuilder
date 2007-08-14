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
  char *xpChart;
  char *job;
  char *start;
  char *title;
  char *wall;
};

#endif
