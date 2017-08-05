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
#include <map>

#define BTUI_CHOICE 1
#define BTUI_SELECT 2
#define BTUI_TEXT 3
#define BTUI_MULTICOLUMN 4
#define BTUI_READSTRING 5
#define BTUI_BARRIER 6
#define BTUI_SELECTIMAGE 7

#define BTKEY_UP 1
#define BTKEY_DOWN 2
#define BTKEY_LEFT 4
#define BTKEY_RIGHT 5
#define BTKEY_PGDN 6
#define BTKEY_PGUP 23
#define BTKEY_END 7
#define BTKEY_INS 8
#define BTKEY_DEL 127
#define BTKEY_CTRL_C 3
#define BTKEY_CTRL_X 24
#define BTKEY_CTRL_V 22
#define BTKEY_F1 28

#define BTMUSICID_ALL 0

#define BTSELECTFLAG_UNSELECTABLE 0
#define BTSELECTFLAG_SHOWVALUE 1
#define BTSELECTFLAG_NUMBER 2

#ifndef SDL2LIB
#define SDL_Keycode SDLKey
#define SDLK_KP_0 SDLK_KP0
#define SDLK_KP_1 SDLK_KP1
#define SDLK_KP_2 SDLK_KP2
#define SDLK_KP_3 SDLK_KP3
#define SDLK_KP_4 SDLK_KP4
#define SDLK_KP_5 SDLK_KP5
#define SDLK_KP_6 SDLK_KP6
#define SDLK_KP_7 SDLK_KP7
#define SDLK_KP_8 SDLK_KP8
#define SDLK_KP_9 SDLK_KP9
#define KMOD_GUI 0
#endif

class BTBackgroundAndScreen;

class BTAlignment
{
 public:
  enum alignment { left, center, right };
};

class BTUIElement
{
 public:
  virtual ~BTUIElement() {}

  virtual int getType() const = 0;

  SDL_Rect position;
};

class BTWidget
{
 public:
  BTWidget() {}
  virtual ~BTWidget() {}

  virtual std::string getName() = 0;

  virtual void render(BTBackgroundAndScreen *d, bool refresh = false) = 0;
};

class BTLabelWidget : public BTWidget
{
 public:
  BTLabelWidget(BTLabelConfig *c, int xMult, int yMult);
  virtual ~BTLabelWidget() {}

  std::string getName() { return config->name; }

  std::string getText() { return text; }

  void render(BTBackgroundAndScreen *d, bool refresh = false);

  void setText(const std::string &t) { text = t; modified = true; }

 protected:
  BTLabelConfig *config;
  SDL_Rect location;
  std::string text;
  bool modified;
};

class BTTextWidget : public BTWidget
{
 public:
  BTTextWidget(BTTextConfig *c, int xMult, int yMult);
  virtual ~BTTextWidget() { clearElements(); }

  void addElement(BTUIElement *elm);
  void clear(BTBackgroundAndScreen *d);
  void clearElements();
  void clearLast();
  void drawLast(BTBackgroundAndScreen *d, const char *words);
  void drawText(BTBackgroundAndScreen *d, const char *words, BTAlignment::alignment a = BTAlignment::left);
  std::vector<BTUIElement*>& getElements() { return element; }
  std::string getName() { return config->name; }
  SDL_Rect &getLocation() { return location; }

  unsigned int process(BTBackgroundAndScreen *d, const char *specialKeys = NULL, int *delay = 0, int delayOveride = -1);
  std::string readString(BTBackgroundAndScreen *d, const char *prompt, int max, const std::string &initial);
  void render(BTBackgroundAndScreen *d, bool refresh = false);
  int selectImage(BTBackgroundAndScreen *d, int initial);

 public:
  int textPos;

 protected:
  BTTextConfig *config;
  SDL_Rect location;
  std::vector<BTUIElement*> element;
  BTUIElement *processor;
  std::string last;
  bool modified;
};

class BTMusic
{
 public:
  BTMusic(unsigned int id) : musicObj(0), musicId(id) {}
  ~BTMusic();

  Mix_Music *musicObj;
  unsigned int musicId;
};

class BTSound
{
 public:
  BTSound(Mix_Chunk *soundObj, int c) : sound(soundObj), channel(c) {}
  ~BTSound();

  Mix_Chunk *sound;
  int channel;
};

class BTAnimation
{
 public:
  BTAnimation(MNG_AnimationState *ani, bool c) : animation(ani), clear(c) {}

  bool operator==(const BTAnimation &other) { return animation == other.animation; }

  MNG_AnimationState *animation;
  bool clear;
};

class BTDisplay : public ImageLoader
{
 public:
  BTDisplay(BTDisplayConfig *c, int multiplier = 0, bool full = false, bool softRender = false);
  ~BTDisplay();

  struct selectItem
  {
   selectItem() : first(0), value(0) {}
   selectItem(std::string nm) : first(0), name(nm), value(0) {}

   bool operator<(const selectItem& other) const { return name < other.name; }

   BitField flags;
   char first;
   std::string name;
   int value;
  };

  void addAnimation(MNG_AnimationState *animState, bool clear = false);
  void addBackground(const char *file);
  void addBarrier(const char *keys);
  void addChoice(const char *keys, const char *words, BTAlignment::alignment a = BTAlignment::left);
  void addText(const char *words, BTAlignment::alignment a = BTAlignment::left);
  void addColumns(const std::list<std::string>& c);
  void addReadString(const std::string &prompt, int maxLen, std::string &response);
  void addSelection(selectItem *list, int size, int &start, int &select, int num = 0);
  void addSelectImage(int &select);
  void clear(SDL_Surface *scr, SDL_Rect &r);
  void clearElements();
  void clearImage();
  void clearLast();
  void clearText();
  void drawFullScreen(const char *file, int delay);
  void drawImage(int pic);
  void drawLabel(const char *value);
  void drawLabel(const char *name, const char *value);
  void drawLast(const char *keys, const char *words);
  void drawMessage(const char *words, int *delay);
  void drawText(const char *words, BTAlignment::alignment a = BTAlignment::left);
  void drawView();
  void drawIcons();
  void drawMap(bool knowledge);
  void drawStats();
  SDL_Color &getBlack();
  SDL_Color &getColor(const std::string &color);
  BTDisplayConfig *getConfig();
  int getCurrentImage();
  std::string getCurrentLabel();
  void getMultiplier(int &x, int &y);
  Psuedo3D &getPsuedo3D() { return p3d; }
  BTBackgroundAndScreen *getScreen(int i);
  SDL_Color &getWhite();
  BTWidget *getWidget(const std::string &name);
  void playMusic(unsigned int effectID, const char *file, bool physfs = true);
  void playSound(const char *file, bool physfs = true);
  unsigned int process(const char *specialKeys = NULL, int *delay = 0, int delayOveride = -1);
  unsigned int readChar(int delay = 0);
  std::string readString(const char *prompt, int max, const std::string &initial);
  void refresh();
  void removeAnimation(MNG_AnimationState *animState);
  void setConfig(BTDisplayConfig *c);
  void setPsuedo3DConfig(Psuedo3DConfigList *p3dl);
  Psuedo3DConfig *setWallGraphics(int type);
  void splitText(const char *words, const std::string &prefix, std::vector<std::string> &lines);
  void stopMusic(int id);
  void toggleFullScreen();

  void drawFont(const char *text, SDL_Rect &dst, SDL_Color c, BTAlignment::alignment a, SDL_Surface *scr = NULL);
  bool sizeFont(const char *text, int &w, int &h);
  void drawImage(SDL_Rect &dst, SDL_Surface *img);
  void fillRect(SDL_Surface *scr, SDL_Rect &dst, SDL_Color c);
  SDL_Surface *grabImage(SDL_Surface *scr, SDL_Rect &src);
  void moveImage(SDL_Rect &src, SDL_Rect &dst);

  void loadImageOrAnimation(const char *file, SDL_Surface **img, MNG_Image **animation, bool imageWindow, bool physfs = true);

  void render();

  static std::string applyDisplayDir(const std::string &filename);
  static const std::string &getDisplayDir() { return displayDir; }
  static void setDisplayDir(const std::string &dDir) { displayDir = dDir; }

 private:
  void clearScreens();
  unsigned long drawAnimationFrame();
  BTBackgroundAndScreen *getVisibleScreen();
  void setupKeyMap();
  static Uint32 timerCallback(Uint32 interval, void *param);
  void setupScreens(BTDisplayConfig *c, int xMult, int yMult);

 public:
  static const char *allKeys;
  int mapXStart, mapYStart;

 private:
  int xFull, yFull;
  bool fullScreen;
  bool softRenderer;
  BTDisplayConfig *config;
  BTDisplayExpanded *expanded;
  int xMult, yMult, lockMult;
  BTStatusBar *status;
  Psuedo3D p3d;
  Psuedo3DConfigList *p3dConfig;
#ifdef SDL2LIB
  SDL_Window *mainWindow;
  SDL_Renderer *mainRenderer;
  SDL_Texture *mainTexture;
#endif
  SDL_Surface *mainScreen;
  std::list<BTBackgroundAndScreen*> screen;
  std::list<BTMusic*> music;
  std::list<BTSound*> sound;
  int picture;
  MNG_AnimationState animation;
#ifdef BTBUILDER_NOTTF
  void *ttffont;
#else
  TTF_Font *ttffont;
#endif
  simpleFont *sfont;
  SDL_Color white, black;
  std::map<SDL_Keycode, char> key;
  std::map<SDL_Keycode, char> shiftKey;
  std::map<SDL_Keycode, char> rightAltKey;

  static std::string displayDir;
};

class BTBackgroundAndScreen
{
 public:
  BTBackgroundAndScreen(BTDisplay *d, SDL_Surface *s, bool v);
  ~BTBackgroundAndScreen();

  void addAnimation(MNG_AnimationState *animState, bool clear = false);
  void addWidget(BTWidget *w);
  void clear();
  void clear(SDL_Rect &r);
  void clearWidgets();
  unsigned long drawAnimationFrame(long ticks);
  void drawFont(const char *text, SDL_Rect &dst, SDL_Color c, BTAlignment::alignment a);
  void drawImage(SDL_Surface *img, SDL_Rect &dst);
  void drawMap(bool knowledge);
  void dropScreen();
  void dupeScreen(SDL_Surface *scr);
  SDL_Color &getColor(const std::string &color);
  BTDisplay *getDisplay() { return display; }
  BTWidget *getWidget(const std::string &name);
  void fillRect(SDL_Rect &dst, SDL_Color c);
  bool isVisable() { return visible; }
  void removeAnimation(MNG_AnimationState *animState);
  void render();
  void scrollUp(const SDL_Rect &text, int h);
  void setBackground(const char *file, bool physfs = true);
  void setVisibility(bool v) { visible = v; }

 protected:
  BTDisplay *display;
  SDL_Surface *screen;
  SDL_Surface *background;
  std::vector<BTWidget*> widgets;
  std::list<BTAnimation> activeAnimation;
  bool visible;
};

class BTUIText : public BTUIElement
{
 public:
  BTUIText(const std::string &t, BTAlignment::alignment a) : text(t), align(a) {}

  virtual int getType() const { return BTUI_TEXT; }
  int maxHeight(BTDisplay &d);

  std::string text;
  BTAlignment::alignment align;
};

class BTUIChoice : public BTUIText
{
 public:
  BTUIChoice(const std::string &k, const std::string &t, BTAlignment::alignment a) : BTUIText(t, a), keys(k) {}

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

  void draw(BTBackgroundAndScreen *d);

  void alter(BTDisplay::selectItem *l, int sz);
  void decrement(BTDisplay &d);
  void increment(BTDisplay &d);
  void moveDown(BTDisplay &d);
  void moveUp(BTDisplay &d);
  void pageDown(BTDisplay &d);
  void pageUp(BTDisplay &d);
  void sanitize(BTDisplay &d);

  BTDisplay::selectItem *list;
  int size;
  int &start;
  int &select;
  int numbered;
};

class BTUISelectImage : public BTUIElement
{
 public:
  BTUISelectImage(int &sel) : select(sel) {}

  virtual int getType() const { return BTUI_SELECTIMAGE; }

  int &select;
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
