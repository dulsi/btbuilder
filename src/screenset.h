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
#include <map>

#define BTSCREEN_EXIT -1

class BTElement
{
 public:
  BTElement(const std::string &t) : isText(true), text(t), atts(0) {}
  BTElement(const char *name, const char **a);
  ~BTElement();

  typedef char *char_ptr;

  bool isText;
  std::string text;
  char_ptr *atts;
};

class BTScreenItem : public XMLObject
{
 public:
  virtual void addCol() {}
  virtual void addText(std::string text) {}
  virtual void addStat(const char *name, const char **atts) {}
  virtual std::string getKeys() { return ""; }
  virtual std::string getAction() { return ""; }
  virtual int getScreen(BTPc *pc) { return 0; }

  virtual void draw(BTDisplay &d, ObjectSerializer *obj) = 0;

  virtual void serialize(ObjectSerializer* s) {}
};

class BTLine : public BTScreenItem
{
 public:
  BTLine() {}
  ~BTLine();

  virtual void addCol();
  virtual void addText(std::string text);
  virtual void addStat(const char *name, const char **atts);

  virtual void draw(BTDisplay &d, ObjectSerializer *obj);

  virtual void elementData(const XML_Char *name, const XML_Char **atts);
  virtual void characterData(const XML_Char *s, int len);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTLine; }

 protected:
  std::string eval(std::vector<BTElement*> &line, ObjectSerializer *obj) const;

 protected:
  std::list<std::vector<BTElement*> > element;
};

class BTChoice : public BTLine
{
 public:
  BTChoice() : screen(0) {}

  virtual void addCol();
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

  virtual void addCol();
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
  virtual int buildList(ObjectSerializer *obj);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts);
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
  virtual int buildList(ObjectSerializer *obj);
  virtual int getScreen(BTPc *pc);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts);

  int fullscreen;
};

class BTSelectParty : public BTScreenItem
{
 public:
  BTSelectParty(const char *a, int s) : action(a), screen(s) {}

  virtual std::string getKeys();
  virtual std::string getAction();
  virtual int getScreen(BTPc *pc);

  virtual void draw(BTDisplay &d, ObjectSerializer *obj);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts);

 private:
  std::string action;
  int screen;
};


class BTCan : public BTScreenItem
{
 public:
  BTCan(const char *o) : option(o) {}

  virtual std::string getKeys();
  virtual std::string getAction();
  virtual int getScreen(BTPc *pc);

  void draw(BTDisplay &d, ObjectSerializer *obj);

  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts);

 private:
  std::string option;
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
  BTError(const char *t, bool r) : type(t), retry(r) {}

  const std::string &getType() { return type; }
  bool getRetry() { return retry; }

  static XMLObject *create(const XML_Char *name, const XML_Char **atts);

 private:
  std::string type;
  bool retry;
};

class BTScreenSet : public ObjectSerializer
{
 public:
  typedef void (*action)(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);

  BTScreenSet();
  ~BTScreenSet();

  virtual int getLevel();

  BTPc* getPc();
  bool displayError(BTDisplay &d, const BTSpecialError &e);
  virtual void endScreen(BTDisplay &d) {}
  action findAction(const std::string &actionName);
  virtual int findScreen(int num);
  virtual void initScreen(BTDisplay &d) {}
  virtual void open(const char *filename);
  void run(BTDisplay &d, int start = 0, bool status = true);
  void setPc(BTPc *c);
  void setPicture(BTDisplay &d, int pic, char *l);

  // Actions
  static void addToParty(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static void buy(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static void create(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static void exit(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static void quit(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static void removeFromParty(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static void save(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static void sell(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static void selectParty(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static void setJob(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);
  static void setRace(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key);

 private:
  BTPc *pc;

 protected:
  int picture;
  char *label;
  XMLVector<BTScreenSetScreen*> screen;
  XMLVector<BTError*> errors;
  std::map<std::string, action> actionList;
};

#endif

