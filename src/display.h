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
#include "sdlextend.h"
#include "SDL_mng.h"
#include <SDL_mixer.h>
#ifndef BTBUILDER_NOTTF
#include <SDL_ttf.h>
#endif

#define BTUI_CHOICE 1
#define BTUI_SELECT 2
#define BTUI_TEXT 3
#define BTUI_MULTICOLUMN 4
#define BTUI_READSTRING 5
#define BTUI_BARRIER 6

#define BTKEY_UP 1
#define BTKEY_DOWN 2
#define BTKEY_LEFT 3
#define BTKEY_RIGHT 4

#define BTMUSICID_ALL 0

class BTUIElement
{
 public:
  virtual ~BTUIElement() {}

  virtual int getType() const = 0;

  SDL_Rect position;
};


class BTMusic
{
 public:
  BTMusic(int id) : musicObj(0), musicId(id) {}
  ~BTMusic();

  Mix_Music *musicObj;
  int musicId;
};

class BTDisplay
{
 public:
  BTDisplay(BTDisplayConfig *c, bool physfs = true);
  ~BTDisplay();

  enum alignment { left, center, right };
  struct selectItem
  {
   selectItem() : first(0), value(0) {}

   char first;
   std::string name;
   int value;
  };

  void addBarrier(const char *keys);
  void addChoice(const char *keys, const char *words, alignment a = left);
  void addText(const char *words, alignment a = left);
  void addColumns(const std::list<std::string>& c);
  void addReadString(const char *prompt, int maxLen, std::string &response);
  void addSelection(selectItem *list, int size, int &start, int &select, int num = 0);
  void clear(SDL_Rect &r, bool update = false);
  void clearElements();
  void clearText();
  void drawFullScreen(const char *file, int delay);
  void drawImage(int pic);
  void drawLabel(const char *name);
  void drawLast(const char *keys, const char *words, alignment a = left);
  void drawMessage(const char *words, int delay);
  void drawText(const char *words, alignment a = left);
  void drawView();
  void drawIcons();
  void drawMap(int x, int y, int xStart, int yStart, int width, int height, bool knowledge);
  void drawStats();
  SDL_Color &getBlack();
  BTDisplayConfig *getConfig();
  void getMultiplier(int &x, int &y);
  SDL_Color &getWhite();
  int playMusic(const char *file, bool physfs = true);
  unsigned int process(const char *specialKeys = NULL, int delay = 0);
  unsigned int readChar(int delay = 0);
  std::string readString(const char *prompt, int max);
  void refresh();
  void setBackground(const char *file, bool physfs = true);
  void setConfig(BTDisplayConfig *c);
  void setPsuedo3DConfig(const char *file);
  void setWallGraphics(int type);
  void stopMusic(int id);

  void drawFont(const char *text, SDL_Rect &dst, SDL_Color c, alignment a);
  bool sizeFont(const char *text, int &w, int &h);
  void drawImage(SDL_Rect &dst, SDL_Surface *img);
  void drawRect(SDL_Rect &dst, SDL_Color c);

 private:
  void drawAnimationFrame();
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
  std::list<BTMusic*> music;
  MNG_Image *animation;
  int animationFrame;
  unsigned long animationTime;
#ifdef BTBUILDER_NOTTF
  void *ttffont;
#else
  TTF_Font *ttffont;
#endif
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

  void moveDown(BTDisplay &d);
  void moveUp(BTDisplay &d);

  BTDisplay::selectItem *list;
  int size;
  int &start;
  int &select;
  int numbered;
};

class BTUIMultiColumn : public BTUIElement
{
 public:
  BTUIMultiColumn(const std::list<std::string> &c) : col(c) {}

  virtual int getType() const { return BTUI_MULTICOLUMN; }

  void draw(int x, int y, int w, int h, BTDisplay& d);
  int maxHeight(BTDisplay &d);

  std::list<std::string> col;
};

#endif
