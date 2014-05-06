#ifndef __SCREENSET_H
#define __SCREENSET_H
/*-------------------------------------------------------------------------*\
  <screenset.h> -- Screen set header file

  Date      Programmer  Description
  04/09/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include <file.h>
#include "display.h"
#include "map.h"
#include "pc.h"
#include "shop.h"
#include <map>

#define BTSCREEN_EXIT -1
#define BTSCREEN_ESCAPE -2

typedef char *char_ptr;

class BTSimpleElement : public XMLObject
{
 public:
  BTSimpleElement() {}

  virtual std::string eval(ObjectSerializer *obj) const = 0;
};

class BTElement : public BTSimpleElement
{
 public:
  BTElement(const std::string &t) : isText(true), text(t), atts(0) {}
  BTElement(const char *name, const char **a);
  ~BTElement();

  std::string eval(ObjectSerializer *obj) const;

  virtual void serialize(ObjectSerializer* s) {}

  bool isText;
  std::string text;
  char_ptr *atts;
};

class BTCheckTrueFalse : public XMLObject
{
 public:
  virtual bool check(ObjectSerializer *obj) const = 0;
};

class BTOperator : public BTCheckTrueFalse
{
 public:
  BTOperator(const char *t) : type(t) {}

  bool check(ObjectSerializer *obj) const;

  virtual void serialize(ObjectSerializer* s);
  virtual void elementData(const XML_Char *name, const XML_Char **atts);
  virtual void characterData(const XML_Char *s, int len);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTOperator(name); }

  std::string type;
  XMLVector<BTSimpleElement*> element;
};

class BTCond : public BTCheckTrueFalse
{
 public:
  BTCond(const char *t) : type(t) {}

  bool check(ObjectSerializer *obj) const;

  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTCond(name); }

 protected:
  std::string type;
  XMLVector<BTCheckTrueFalse*> op;
};

class BTColumn : public BTSimpleElement
{
 public:
  BTColumn() {}

  std::string eval(ObjectSerializer *obj) const;

  virtual void serialize(ObjectSerializer* s);
  virtual void elementData(const XML_Char *name, const XML_Char **atts);
  virtual void characterData(const XML_Char *s, int len);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTColumn; }

 protected:
  XMLVector<BTSimpleElement*> element;
};

class BTIf : public BTSimpleElement
{
 public:
  BTIf() : cond("and") {}

  std::string eval(ObjectSerializer *obj) const;

  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTIf; }

 protected:
  BTCond cond;
  BTColumn thenClause;
  BTColumn elseClause;
};

class BTScreenItem : public XMLObject
{
 public:
  virtual std::string getKeys() { return ""; }
  virtual std::string getAction() { return ""; }
  virtual int getScreen(BTPc *pc) { return 0; }

  virtual void draw(BTDisplay &d, ObjectSerializer *obj) = 0;

  virtual void serialize(ObjectSerializer* s) {}
};

class BTBarrier : public BTScreenItem
{
 public:
  virtual void draw(BTDisplay &d, ObjectSerializer *obj);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTBarrier; }
};

class BTLine : public BTScreenItem
{
 public:
  BTLine() : align(BTDisplay::left) {}
  ~BTLine();

  void setAlignment(std::string a);

  virtual void draw(BTDisplay &d, ObjectSerializer *obj);

  virtual void serialize(ObjectSerializer* s);
  virtual void elementData(const XML_Char *name, const XML_Char **atts);
  virtual void characterData(const XML_Char *s, int len);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts);

 protected:
  XMLVector<BTSimpleElement*> element;
  BTDisplay::alignment align;
};

class BTChoice : public BTLine
{
 public:
  BTChoice() : screen(0) {}

  virtual std::string getKeys();
  virtual std::string getAction();
  virtual int getScreen(BTPc *pc);
  void setKeys(std::string k);
  void setAction(std::string a);
  void setScreen(int s);

  virtual void draw(BTDisplay &d, ObjectSerializer *obj);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts);

 protected:
  std::string keys;
  std::string action;
  int screen;
};

class BTReadString : public BTLine
{
 public:
  BTReadString() : screen(0) {}

  virtual std::string getKeys();
  virtual std::string getAction();
  std::string getResponse() { return response; }
  virtual int getScreen(BTPc *pc);
  void setAction(std::string a);
  void setScreen(int s);

  virtual void draw(BTDisplay &d, ObjectSerializer *obj);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts);

 protected:
  std::string action;
  int screen;
  std::string response;
};

class BTSelectCommon : public BTScreenItem
{
 public:
  BTSelectCommon();
  ~BTSelectCommon();

  void clear();

  virtual int buildList(ObjectSerializer *obj) = 0;
  virtual std::string getKeys();
  virtual std::string getAction();
  virtual int getNumber();
  virtual int getScreen(BTPc *pc);
  void setAction(std::string a);
  void setScreen(int s);

  virtual void draw(BTDisplay &d, ObjectSerializer *obj);

  BTDisplay::selectItem *list;
  int size;
  int start;
  int select;
  std::string action;
  int screen;
  bool numbered;
};

class BTSelectRoster : public BTSelectCommon
{
 public:
  BTSelectRoster();

  virtual int buildList(ObjectSerializer *obj);
  virtual int getScreen(BTPc *pc);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts);

  int fullscreen;
};

class BTSelectRace : public BTSelectCommon
{
 public:
  virtual int buildList(ObjectSerializer *obj);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts);
};

class BTSelectJob : public BTSelectCommon
{
 public:
  BTSelectJob();

  virtual int buildList(ObjectSerializer *obj);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts);

  bool starting;
};

class BTSelectGoods : public BTSelectCommon
{
 public:
  virtual int buildList(ObjectSerializer *obj);
  virtual int getScreen(BTPc *pc);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts);

  int shop;
  int fullscreen;
};

class BTSelectInventory : public BTSelectCommon
{
 public:
  BTSelectInventory();

  virtual int buildList(ObjectSerializer *obj);
  virtual int getNumber();
  virtual int getScreen(BTPc *pc);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts);

  int shop;
  int fullscreen;
  bool noerror;
  bool value;
};

class BTSelectParty : public BTScreenItem
{
 public:
  BTSelectParty(const char *a, int s, int w, BitField d) : action(a), screen(s), who(w), disallow(d) {}

  virtual std::string getKeys();
  virtual std::string getAction();
  virtual int getScreen(BTPc *pc);
  void checkDisallow(BTPc *pc);
  int getWho();

  virtual void draw(BTDisplay &d, ObjectSerializer *obj);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts);

 private:
  std::string action;
  int screen;
  int who;
  BitField disallow;
};

class BTSelectSong : public BTSelectCommon
{
 public:
  BTSelectSong();

  virtual int buildList(ObjectSerializer *obj);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts);
};

class BTCan : public BTScreenItem
{
 public:
  BTCan(const char *o, char_ptr *a, const char *f, const char *v, const char *d);
  ~BTCan();

  virtual std::string getKeys();
  virtual std::string getAction();
  virtual int getScreen(BTPc *pc);

  void draw(BTDisplay &d, ObjectSerializer *obj);

  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts);

 private:
  std::string option;
  char_ptr *atts;
  std::string field;
  std::string value;
  std::string defaultValue;
  bool checkValue;
  bool drawn;
  XMLVector<BTScreenItem*> items;
};

class BTScreenSetScreen : public XMLObject
{
 public:
  BTScreenSetScreen(int n, int escScr, int t) : number(n), escapeScreen(escScr), timeout(t) {}

  virtual void draw(BTDisplay &d, ObjectSerializer *obj);
  BTScreenItem *findItem(int key);
  int getNumber() { return number; }
  virtual int getEscapeScreen() { return escapeScreen; }
  int getTimeout() { return timeout; }

  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts);

 private:
  int number;
  int escapeScreen;
  int timeout;
  XMLVector<BTScreenItem*> items;
};

class BTError : public BTLine
{
 public:
  BTError(const char *t, int s) : type(t), screen(s) {}

  const std::string &getType() { return type; }
  int getScreen() { return screen; }

  static XMLObject *create(const XML_Char *name, const XML_Char **atts);

 private:
  std::string type;
  int screen;
};

class BTEffect : public BTLine
{
 public:
  BTEffect(int t, const char *act) : type(t), action(act), processed(false) {}

  virtual std::string getAction();
  int getType() { return type; }
  bool isProcessed() { return processed; }
  void setProcessed() { processed = true; }

  static XMLObject *create(const XML_Char *name, const XML_Char **atts);

 private:
  int type;
  std::string action;
  bool processed;
};

class BTScreenSet : public ObjectSerializer
{
 public:
  typedef int (*action)(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);

  BTScreenSet();
  ~BTScreenSet();

  BTPc* getPc();
  void checkEffects(BTDisplay &d);
  int displayError(BTDisplay &d, const BTSpecialError &e);
  virtual void endScreen(BTDisplay &d) {}
  action findAction(const std::string &actionName);
  virtual int findScreen(int num);
  virtual void initScreen(BTDisplay &d) {}
  virtual void open(const char *filename);
  void run(BTDisplay &d, int start = 0, bool status = true);
  void setEffect(int type);
  void setGroup(BTGroup *g);
  void setPc(BTPc *c, int who = 0);
  void setPicture(BTDisplay &d, int pic, const char *l);

  // Actions
  static int advanceLevel(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int addToParty(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int buy(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int buySkill(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int castNow(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int changeJob(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int create(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int drop(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int dropFromParty(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int equip(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int exit(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int exitAndSave(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int findTraps(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int give(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int identify(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int moveTo(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int openChest(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int poolGold(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int quit(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int requestSkill(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int requestJob(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int removeFromParty(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int removeRoster(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int removeTraps(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int save(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int saveGame(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int saveParty(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int sell(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int selectBard(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int selectMage(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int selectParty(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int selectItem(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int selectRoster(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int setGender(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int setJob(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int setRace(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int singNow(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int tradeGold(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int unequip(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int useNow(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static int useOn(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);

 private:
  BTPc *pc[2];
  BTGroup *grp;

 protected:
  int picture;
  char *label;
  bool building;
  bool clearMagic;
  XMLVector<BTScreenSetScreen*> screen;
  XMLVector<BTError*> errors;
  XMLVector<BTEffect*> effects;
  std::map<std::string, action> actionList;
};

#endif

