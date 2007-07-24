#ifndef __SCREENSET_H
#define __SCREENSET_H
/*-------------------------------------------------------------------------*\
  <screenset.h> -- Screen set header file

  Date      Programmer  Description
  04/09/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include <istdlib.h>
#include <file.h>
#include "display.h"
#include "map.h"
#include <map>

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

class BTScreenSetScreen : public XMLObject
{
 public:
  BTScreenSetScreen(int n, int escScr) : number(n), escapeScreen(escScr) {}

  void draw(BTDisplay &d, ObjectSerializer *obj);
  BTScreenItem *findItem(int key);
  int getNumber() { return number; }
  int getEscapeScreen() { return escapeScreen; }

  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts);

 private:
  int number;
  int escapeScreen;
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
  typedef void (*action)(BTScreenSet &b, BTDisplay &d, BTScreenItem *item);

  BTScreenSet(const char *filename);
  ~BTScreenSet();

  virtual int getLevel();

  bool displayError(BTDisplay &d, const BTSpecialError &e);
  action findAction(const std::string &actionName);
  int findScreen(int num);
  void run(BTDisplay &d);
  void setPc(BTPc *c);

  // Actions
  static void addToParty(BTScreenSet &b, BTDisplay &d, BTScreenItem *item);
  static void buy(BTScreenSet &b, BTDisplay &d, BTScreenItem *item);
  static void create(BTScreenSet &b, BTDisplay &d, BTScreenItem *item);
  static void exit(BTScreenSet &b, BTDisplay &d, BTScreenItem *item);
  static void quit(BTScreenSet &b, BTDisplay &d, BTScreenItem *item);
  static void removeFromParty(BTScreenSet &b, BTDisplay &d, BTScreenItem *item);
  static void save(BTScreenSet &b, BTDisplay &d, BTScreenItem *item);
  static void sell(BTScreenSet &b, BTDisplay &d, BTScreenItem *item);
  static void setJob(BTScreenSet &b, BTDisplay &d, BTScreenItem *item);
  static void setRace(BTScreenSet &b, BTDisplay &d, BTScreenItem *item);

 private:
  BTPc *pc;
  int picture;
  char *label;
  XMLVector<BTScreenSetScreen*> screen;
  XMLVector<BTError*> errors;
  std::map<std::string, action> actionList;
};

#endif

