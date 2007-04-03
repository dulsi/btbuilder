#ifndef __DISPLAY_H
#define __DISPLAY_H
/*-------------------------------------------------------------------------*\
  <display.h> -- Display header file

  Date      Programmer  Description
  02/12/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include <istdlib.h>
#include "psuedo3d.h"
#include "displayconfig.h"
#include "statusbar.h"
#include <SDL_ttf.h>
#include "sdlextend.h"

class BTDisplay
{
 public:
  BTDisplay(BTDisplayConfig *c);
  ~BTDisplay();

  enum alignment { left, center, right };
  struct selectItem
  {
   selectItem() : first(0), value(0) {}

   char first;
   const char *name;
   int value;
  };

  void clearText();
  void drawChoice(const char* keys, const char *words, alignment a = left);
  void drawFullScreen(const char *file, int delay);
  void drawImage(int pic);
  void drawLabel(const char *name);
  void drawLast(const char* keys, const char *words, alignment a = left);
  void drawText(const char *words, alignment a = left);
  void draw2Column(const char *col1, const char *col2);
  void drawView();
  void drawStats();
  SDL_Color &getBlack();
  BTDisplayConfig *getConfig();
  void getMultiplier(int &x, int &y);
  unsigned char readChar();
  std::string readString(const char *prompt, int max);
  void refresh();
  bool selectList(selectItem *list, int size, int &start, int &select);
  void setBackground(const char *file);
  void setPsuedo3DConfig(const char *file);
  void setWallGraphics(int type);

  void drawFont(const char *text, SDL_Rect &dst, SDL_Color c, alignment a);

 private:
  bool sizeFont(const char *text, int &w, int &h);

  void scrollUp(int h);

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
};

#endif

