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

  void edit(BTDisplay &d);
  void editMap(BTDisplay &d, const char *filename);
  void editMapProperties(BTDisplay &d);
  void editSpecial(BTDisplay &d, BTSpecial *special);

  template<typename item>
  int editFactoryList(BTDisplay &d, BTFactory<item> &itemList, const std::string &newItem)
  {
   BTDisplayConfig *oldConfig = d.getConfig();
   BTDisplayConfig config;
   XMLSerializer parser;
   config.serialize(&parser);
   parser.parse("data/specialedit.xml", true);
   d.setConfig(&config);
   int start(0);
   int current(0);
   BTDisplay::selectItem items[itemList.size() + 1];
   for (int i = 0; i < itemList.size(); ++i)
    items[i].name = itemList[i].getName();
   items[itemList.size()].name = newItem;
   d.addSelection(items, itemList.size() + 1, start, current);
   int key = d.process();
   d.clearText();
   d.setConfig(oldConfig);
   if (27 == key)
    return -1;
   else
    return current;
  }

  void editItem(BTDisplay &d, BTItem &item);
  void editMonster(BTDisplay &d, BTMonster &monster);
  void editSerialized(BTDisplay &d, ObjectSerializer &serial, int entries, const char *description[], const char *field[]);

 protected:
  struct operationList
  {
   public:
    operationList(BTSpecialBody *p, BTSpecialOperation *o) : parent(p), op(o) {}

    BTSpecialBody *parent;
    BTSpecialOperation *op;
  };

  void buildOperationList(BTSpecialBody *body, std::vector<BTDisplay::selectItem> &list, std::vector<operationList> &ops, int level = 0);
  BTSpecialOperation *editSpecialOperation(BTDisplay &d, BTSpecialOperation *special);

 protected:
  int currentWall;
  int startSpecial;
  int currentSpecial;
};

#endif

