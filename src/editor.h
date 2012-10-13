#ifndef __EDITOR_H
#define __EDITOR_H
/*-------------------------------------------------------------------------*\
  <editor.h> -- Editor header file

  Date      Programmer  Description
  10/10/12  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "game.h"

class BTEditor : public BTCore
{
 public:
  BTEditor(BTModule *m);
  ~BTEditor();

  int getLight();
  int getWallType(int x, int y, int direction);
  int getKnowledge(int x, int y);

  void editMap(BTDisplay &d, const char *filename);

 protected:
  int currentWall;
  int startSpecial;
  int currentSpecial;
};

#endif

