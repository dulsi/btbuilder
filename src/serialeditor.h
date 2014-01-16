#ifndef __SERIALEDITOR_H
#define __SERIALEDITOR_H
/*-------------------------------------------------------------------------*\
  <serialeditor.h> -- Serialized editor header file

  Date      Programmer  Description
  10/18/13  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "editor.h"

class BTSerializedEditor
{
 public:
  BTSerializedEditor(int num, const char **d, const char **f);
  virtual ~BTSerializedEditor();


  template<typename item>
  int editFactoryList(BTDisplay &d, BTFactory<item> &itemList, const std::string &newItem)
  {
   BTDisplayConfig *oldConfig = d.getConfig();
   BTDisplayConfig config;
   XMLSerializer parser;
   config.serialize(&parser);
   parser.parse("data/specialedit.xml", true);
   d.setConfig(&config);
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

  void edit(BTDisplay &d, ObjectSerializer &serial);

 protected:
  virtual void initActive(ObjectSerializer &serial, BitField &active);
  int setup(ObjectSerializer &serial, BitField &active, std::vector<BTDisplay::selectItem> &items);
  virtual bool updateActive(ObjectSerializer &serial, BitField &active, int modField);

 protected:
  int entries;
  const char **description;
  const char **field;
  int start;
  int current;
};

#define FIELDS_MAP 4

class BTMapPropertiesEditor : public BTSerializedEditor
{
 public:
  BTMapPropertiesEditor();

 private:
  static const char *mapDescription[FIELDS_MAP];
  static const char *mapField[FIELDS_MAP];
};

#define FIELDS_ITEM 14
#define FIELDS_MONSTER 26
#define FIELDS_SPELL 18

class BTItemEditor : public BTSerializedEditor
{
 public:
  BTItemEditor();

 private:
  static const char *itemDescription[FIELDS_ITEM];
  static const char *itemField[FIELDS_ITEM];
};

class BTMonsterEditor : public BTSerializedEditor
{
 public:
  BTMonsterEditor();

 protected:
  virtual void initActive(ObjectSerializer &serial, BitField &active);
  virtual bool updateActive(ObjectSerializer &serial, BitField &active, int modField);

 private:
  unsigned int prevXp;

  static const char *monsterDescription[FIELDS_MONSTER];
  static const char *monsterField[FIELDS_MONSTER];
};

class BTSpellEditor : public BTSerializedEditor
{
 public:
  BTSpellEditor();

 protected:
  virtual void initActive(ObjectSerializer &serial, BitField &active);
  virtual bool updateActive(ObjectSerializer &serial, BitField &active, int modField);

 private:
  static const char *spellDescription[FIELDS_SPELL];
  static const char *spellField[FIELDS_SPELL];
};

#endif

