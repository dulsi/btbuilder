#ifndef __DISPLAY_H
#define __DISPLAY_H
/*-------------------------------------------------------------------------*\
  <display.h> -- Display header file

  Date      Programmer  Description
  02/12/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include <istdlib.h>
#include "psuedo3d.h"
#include <SDL_ttf.h>
#include "sdlextend.h"

class BTDisplay
{
 public:
  BTDisplay(int xM = 0, int yM = 0);
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
  void drawFullScreen(const char *file, int delay);
  void drawImage(const char *file);
  void drawLabel(const char *name);
  void drawText(const char *words, alignment a = left);
  void draw2Column(const char *col1, const char *col2);
  void drawView();
  void drawStats();
  std::string readString(int max);
  bool selectList(selectItem *list, int size, int &start, int &select);
  void setWallGraphics(int type);

 private:
  bool sizeFont(const char *text, int &w, int &h);
  void drawFont(const char *text, SDL_Rect &dst, SDL_Color c, alignment a);

  void scrollUp(int h);

 private:
  int xMult, yMult;
  int x3d, y3d;
  SDL_Rect label;
  SDL_Rect text;
  SDL_Rect stats;
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

