#ifndef __DISPLAY_H
#define __DISPLAY_H
/*-------------------------------------------------------------------------*\
  <display.h> -- Display header file

  Date      Programmer  Description
  02/12/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "psuedo3d.h"
#include "displayconfig.h"
#include "statusbar.h"
#include <SDL_ttf.h>
#include "sdlextend.h"

#define BTUI_CHOICE 1
#define BTUI_SELECT 2
#define BTUI_TEXT 3
#define BTUI_2COLUMN 4
#define BTUI_READSTRING 5
#define BTUI_BARRIER 6

#define BTKEY_UP 1
#define BTKEY_DOWN 2
#define BTKEY_LEFT 3
#define BTKEY_RIGHT 4

class BTUIElement
{
 public:
  virtual ~BTUIElement() {}

  virtual int getType() const = 0;

  SDL_Rect position;
};

class BTDisplay
{
 public:
  BTDisplay(BTDisplayConfig *c);
  ~BTDisplay();

  enum alignment { left, center, right };
  struct selectItem
  {
   selectItem() : first(0), name(0), value(0) {}

   char first;
   const char *name;
   int value;
  };

  void addBarrier(const char *keys);
  void addChoice(const char *keys, const char *words, alignment a = left);
  void addText(const char *words, alignment a = left);
  void add2Column(const char *col1, const char *col2);
  void addReadString(const char *prompt, int maxLen, std::string &response);
  void addSelection(selectItem *list, int size, int &start, int &select, int num = 0);
  void clear(SDL_Rect &r);
  void clearElements();
  void clearText();
  void drawChoice(const char *key, const char *words, alignment a = left);
  void drawFullScreen(const char *file, int delay);
  void drawImage(int pic);
  void drawLabel(const char *name);
  void drawLast(const char *keys, const char *words, alignment a = left);
  void drawText(const char *words, alignment a = left);
  void draw2Column(const char *col1, const char *col2);
  void drawView();
  void drawStats();
  SDL_Color &getBlack();
  BTDisplayConfig *getConfig();
  void getMultiplier(int &x, int &y);
  SDL_Color &getWhite();
  unsigned int process(const char *specialKeys = NULL, int delay = 0);
  unsigned int readChar();
  std::string readString(const char *prompt, int max);
  void refresh();
  bool selectList(selectItem *list, int size, int &start, int &select);
  void setBackground(const char *file);
  void setPsuedo3DConfig(const char *file);
  void setWallGraphics(int type);

  void drawFont(const char *text, SDL_Rect &dst, SDL_Color c, alignment a);
  bool sizeFont(const char *text, int &w, int &h);
  void drawRect(SDL_Rect &dst, SDL_Color c);

 private:
  void scrollUp(int h);
  static Uint32 timerCallback(Uint32 interval, void *param);

 public:
  static const char *allKeys;

 private:
  int xFull, yFull;
  BTDisplayConfig *config;
  int xMult, yMult;
  SDL_Rect label;
  SDL_Rect text;
  BTStatusBar status;
  int textPos;
  Psuedo3D p3d;
  XMLVector<Psuedo3DConfig*> p3dConfig;
  SDL_Surface *mainScreen;
  SDL_Surface *mainBackground;
  TTF_Font *ttffont;
  simpleFont *sfont;
  SDL_Color white, black;
  std::vector<BTUIElement*> element;
};

class BTUIText : public BTUIElement
{
 public:
  BTUIText(const std::string &t, BTDisplay::alignment a) : text(t), align(a) {}

  virtual int getType() const { return BTUI_TEXT; }
  int maxHeight(BTDisplay &d);

  std::string text;
  BTDisplay::alignment align;
};

class BTUIChoice : public BTUIText
{
 public:
  BTUIChoice(const std::string &k, const std::string &t, BTDisplay::alignment a) : keys(k), BTUIText(t, a) {}

  virtual int getType() const { return BTUI_CHOICE; }

  std::string keys;
};

class BTUIReadString : public BTUIElement
{
 public:
  BTUIReadString(const std::string &p, int m, std::string &r) : prompt(p), maxLen(m), response(r) {}

  virtual int getType() const { return BTUI_READSTRING; }

  std::string prompt;
  int maxLen;
  std::string &response;
};

class BTUIBarrier : public BTUIElement
{
 public:
  BTUIBarrier(const std::string &k) : keys(k) {}

  virtual int getType() const { return BTUI_BARRIER; }

  std::string keys;
};

class BTUISelect : public BTUIElement
{
 public:
  BTUISelect(BTDisplay::selectItem *l, int sz, int &st, int &sel, int num = 0);

  virtual int getType() const { return BTUI_SELECT; }

  void draw(BTDisplay &d);

  BTDisplay::selectItem *list;
  int size;
  int &start;
  int &select;
  int numbered;
};

class BTUI2Column : public BTUIElement
{
 public:
  BTUI2Column(const std::string &c1, const std::string &c2) : col1(c1), col2(c2) {}

  virtual int getType() const { return BTUI_2COLUMN; }

  void draw(int x, int y, int w, int h, BTDisplay& d);
  int maxHeight(BTDisplay &d);

  std::string col1;
  std::string col2;
};

#endif
