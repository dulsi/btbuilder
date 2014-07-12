#ifndef __SERIALEDITOR_H
#define __SERIALEDITOR_H
/*-------------------------------------------------------------------------*\
  <serialeditor.h> -- Serialized editor header file

  Date      Programmer  Description
  10/18/13  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "editor.h"

class BTFactoryEditor
{
 public:
  BTFactoryEditor(bool s = false);
  virtual ~BTFactoryEditor();

  template<typename item, typename item1 = item>
  int editFactoryList(BTDisplay &d, BTFactory<item, item1> &itemList, const BTSortCompare<item> &compare, const std::string &newItem)
  {
   BTDisplayConfig *oldConfig = d.getConfig();
   BTDisplayConfig config;
   XMLSerializer parser;
   config.serialize(&parser);
   parser.parse("data/specialedit.xml", true);
   d.setConfig(&config);
   BTSortedFactory<item, item1> sortList(&itemList, (sorted ? &compare : NULL));
   BTDisplay::selectItem items[itemList.size() + 1];
   for (int i = 0; i < itemList.size(); ++i)
    items[i].name = sortList[i].getName();
   items[itemList.size()].name = newItem;
   int sortedCurrent = 0;
   if (current == -1)
    current = sortList.getUnsortedIndex(0);
   else if (current < itemList.size())
    sortedCurrent = sortList.getSortedIndex(current);
   d.addSelection(items, itemList.size() + 1, start, sortedCurrent);
   int key = d.process("c");
   d.clearText();
   if (sortedCurrent != itemList.size())
    current = sortList.getUnsortedIndex(sortedCurrent);
   else
    current = itemList.size();
   d.setConfig(oldConfig);
   if (27 == key)
    return -1;
   else if (('c' == key) && (current != itemList.size()))
   {
    current = itemList.copy(current);
    return current;
   }
   else
    return current;
  }

 protected:
  int start;
  int current;
  bool sorted;
};

class BTSerializedEditor : public BTFactoryEditor
{
 public:
  BTSerializedEditor(int num, const char **d, const char **f, bool s = false);
  virtual ~BTSerializedEditor();

  void edit(BTDisplay &d, ObjectSerializer &serial);

 protected:
  virtual void complete(ObjectSerializer &serial);
  virtual void delSpecialField(BTDisplay &d, ObjectSerializer &serial, int val);
  void editField(BTDisplay &d, ObjectSerializer &serial, const char *text, XMLAction *curField, int modField, int where);
  virtual void initActive(ObjectSerializer &serial, BitField &active);
  virtual void handleObject(BTDisplay &d, XMLObject *obj, int modField);
  virtual void handleSpecialField(BTDisplay &d, ObjectSerializer &serial, int val);
  virtual int setup(ObjectSerializer &serial, BitField &active, std::vector<BTDisplay::selectItem> &items);
  virtual bool updateActive(ObjectSerializer &serial, BitField &active, int modField);

 protected:
  int entries;
  const char **description;
  const char **field;
};

#define FIELDS_MAP 5

class BTMapPropertiesEditor : public BTSerializedEditor
{
 public:
  BTMapPropertiesEditor();

 protected:
  virtual void handleObject(BTDisplay &d, XMLObject *obj, int modField);

 private:
  static const char *mapDescription[FIELDS_MAP];
  static const char *mapField[FIELDS_MAP];
};

#define FIELDS_MONSTERCHANCE 2

class BTMonsterChanceEditor : public BTSerializedEditor
{
 public:
  BTMonsterChanceEditor();

 private:
  static const char *monsterChanceDescription[FIELDS_MONSTERCHANCE];
  static const char *monsterChanceField[FIELDS_MONSTERCHANCE];
};


#define FIELDS_ITEM 14
#define FIELDS_MONSTER 26
#define FIELDS_SPELL 10

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

  virtual void complete(ObjectSerializer &serial);
  virtual void delSpecialField(BTDisplay &d, ObjectSerializer &serial, int val);
  virtual void handleSpecialField(BTDisplay &d, ObjectSerializer &serial, int val);
  virtual int setup(ObjectSerializer &serial, BitField &active, std::vector<BTDisplay::selectItem> &items);
  virtual bool updateActive(ObjectSerializer &serial, BitField &active, int modField);

  struct extraItems
  {
   extraItems(BTManifest *i, const std::string &n, int v) : item(i), name(n), value(v) {}

   BTManifest *item;
   std::string name;
   int value;
  };

  struct spellType
  {
   int type;
   XMLObject::create f;
  };

 private:
  std::vector<extraItems> extra;

  static spellType spellTypes[BT_SPELLTYPES_USED];
  static const char *spellDescription[FIELDS_SPELL];
  static const char *spellField[FIELDS_SPELL];
};

#endif

