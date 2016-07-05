#ifndef __EDITOR_H
#define __EDITOR_H
/*-------------------------------------------------------------------------*\
  <editor.h> -- Editor header file

  Date      Programmer  Description
  10/10/12  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "game.h"

#define BT_SKIPFILES 5

class BTEditor : public BTCore
{
 public:
  BTEditor(BTModule *m);
  ~BTEditor();

  int getLight();
  int getWallType(int x, int y, int direction);
  int getKnowledge(int x, int y);

  void edit(BTDisplay &d);
  void editMap(BTDisplay &d, const char *filename);
  void editSpecial(BTDisplay &d, BTSpecial *special);

 protected:
  struct operationList
  {
   public:
    operationList(BTSpecialBody *p, BTSpecialOperation *o) : parent(p), op(o) {}

    BTSpecialBody *parent;
    BTSpecialOperation *op;
  };

  void buildOperationList(BTDisplay &d, BTSpecialBody *body, std::vector<BTDisplay::selectItem> &list, std::vector<operationList> &ops, int level = 0);
  bool checkSkipFiles(const char *f);
  BTSpecialOperation *editSpecialOperation(BTDisplay &d, BTSpecialOperation *special);
  void toggleMap();

 protected:
  int currentWall;
  int startSpecial;
  int currentSpecial;
  int startStreet;
  int currentStreet;
  BTMap *swapMap;
  BTSpecialOperation *clipboard;
  static const char *skipFiles[];
};

#endif

