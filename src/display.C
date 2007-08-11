/*-------------------------------------------------------------------------*\
  <display.C> -- Display implementation file

  Date      Programmer  Description
  02/28/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "compressor.h"
#include "display.h"
#include "game.h"
#include "physfsrwops.h"
#include <SDL_image.h>

const char *BTDisplay::allKeys = "allKeys";

BTDisplay::BTDisplay(BTDisplayConfig *c)
 : config(c), xMult(0), yMult(0), status(*this), textPos(0), p3d(0, 0), mainScreen(0), ttffont(0), sfont(&simple8x8)
{
 if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
 {
  printf("Failed - SDL_Init\n");
  exit(0);
 }
 const SDL_VideoInfo *info = SDL_GetVideoInfo();
 xFull = info->current_w;
 yFull = info->current_h;
 if ((xMult == 0) || (yMult == 0))
 {
  xMult = (xFull - 10) / config->width; // Allow for window decoration
  yMult = (yFull - 10) / config->height; // Allow for window decoration
  if (xMult > yMult)
   xMult = yMult;
  else
   yMult = xMult;
  p3d.setMultiplier(xMult, yMult);
 }
 label.x = config->label.x * xMult;
 label.y = config->label.y * yMult;
 label.w = config->label.w * xMult;
 label.h = config->label.h * yMult;
 text.x = config->text.x * xMult;
 text.y = config->text.y * yMult;
 text.w = config->text.w * xMult;
 text.h = config->text.h * yMult;
 if (TTF_Init() == -1)
 {
  printf("Failed - TTF_Init\n");
  exit(0);
 }
 mainScreen = SDL_SetVideoMode(config->width * xMult, config->height * yMult, 32,
   SDL_SWSURFACE /*| (fullScreen ? SDL_FULLSCREEN : 0)*/);
 if (mainScreen == NULL)
 {
  printf("Failed - SDL_SetVideoMode\n");
  exit(0);
 }
/* if (config->font)
  ttffont = TTF_OpenFont("/usr/share/fonts/bitstream-vera/VeraMono.ttf", 6 * ((xMult == yMult) ? yMult : 1));*/
 white.r = 255;
 white.g = 255;
 white.b = 255;
 black.r = 0;
 black.g = 0;
 black.b = 0;
 setBackground(config->background);

 SDL_EventState(SDL_ACTIVEEVENT, SDL_IGNORE);
 SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
 SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);
 SDL_EventState(SDL_MOUSEBUTTONUP, SDL_IGNORE);
 SDL_EnableUNICODE(1);
}

BTDisplay::~BTDisplay()
{
 for (std::vector<BTUIElement*>::iterator elementItr = element.begin(); element.end() != elementItr; ++elementItr)
 {
  delete (*elementItr);
 }
 element.clear();
 SDL_Quit();
}

void BTDisplay::addBarrier(const char *keys)
{
 element.push_back(new BTUIBarrier(keys));
}

void BTDisplay::addChoice(const char *keys, const char *words, alignment a /*= left*/)
{
 int w, h;
 char *tmp = new char[strlen(words)];
 const char *partial;
 if (partial = strchr(words, '\n'))
 {
  do
  {
   memcpy(tmp, words, partial - words);
   words = partial + 1;
   addChoice(keys, tmp, a);
  }
  while (partial = strchr(words, '\n'));
 }
 partial = words;
 while (partial)
 {
  if (!sizeFont(partial, w, h))
  {
   delete [] tmp;
   return;
  }
  const char *end = NULL;
  if (w > text.w)
  {
   const char *sp = partial;
   for (end = partial; *end; ++end)
   {
    if (isspace(*end))
    {
     memcpy(tmp + (sp - partial), sp, end - sp);
     tmp[end - partial] = 0;
     sizeFont(tmp, w, h);
     if (w > text.w)
     {
      end = sp;
      break;
     }
     sp = end;
    }
   }
   if (!(*end))
    end = sp;
   if (end == partial)
    end = NULL;
   else
   {
    tmp[end - partial] = 0;
    while (isspace(*end))
     ++end;
    if (!(*end))
     end = NULL;
   }
  }
  if (NULL == keys)
   element.push_back(new BTUIText((end ? tmp : partial), a));
  else
   element.push_back(new BTUIChoice(keys, (end ? tmp : partial), a));
  partial = end;
 }
 delete [] tmp;
}

void BTDisplay::addText(const char *words, alignment a /*= left*/)
{
 addChoice(NULL, words, a);
}

void BTDisplay::add2Column(const char *col1, const char *col2)
{
 element.push_back(new BTUI2Column(col1, col2));
}

void BTDisplay::addReadString(const char *prompt, int maxLen, std::string &response)
{
 element.push_back(new BTUIReadString(prompt, maxLen, response));
}

void BTDisplay::addSelection(selectItem *list, int size, int &start, int &select, int num /*= 0*/)
{
 element.push_back(new BTUISelect(list, size, start, select, num));
}

void BTDisplay::clear(SDL_Rect &r)
{
 SDL_BlitSurface(mainBackground, &r, mainScreen, &r);
}

void BTDisplay::clearElements()
{
 for (std::vector<BTUIElement*>::iterator elementItr = element.begin(); element.end() != elementItr; ++elementItr)
 {
  delete (*elementItr);
 }
 element.clear();
}

void BTDisplay::clearText()
{
 clearElements();
 SDL_BlitSurface(mainBackground, &text, mainScreen, &text);
 SDL_UpdateRect(mainScreen, text.x, text.y, text.w, text.h);
 textPos = 0;
}

void BTDisplay::drawFullScreen(const char *file, int delay)
{
 SDL_RWops *f = PHYSFSRWOPS_openRead(file);
 SDL_Surface *img = IMG_Load_RW(f, 1);
 if (img)
 {
  // HACK: Bug in SDL's lbm loading code
  if ((img->format->BitsPerPixel == 8) && (img->format->palette->ncolors < 256))
   img->format->palette->ncolors = 256;
  if ((xMult > 1) || (yMult > 1))
  {
   SDL_Surface *img2 = simpleZoomSurface(img, xMult, yMult);
   SDL_FreeSurface(img);
   img = img2;
  }
  SDL_BlitSurface(img, NULL, mainScreen, NULL);
  SDL_UpdateRect(mainScreen, 0, 0, 0, 0);
  SDL_FreeSurface(img);
  if (delay)
   SDL_Delay(delay);
  else
   readChar();
 }
}

void BTDisplay::drawImage(int pic)
{
 char filename[50];
 const char *dir = PHYSFS_getDirSeparator();
 SDL_Surface *img = NULL;
 snprintf(filename, 50, "image%sslot%d.png", dir, pic);
 if (PHYSFS_exists(filename))
 {
  SDL_RWops *f = PHYSFSRWOPS_openRead(filename);
  img = IMG_Load_RW(f, 1);
 }
 if ((pic >= 45) && (img == NULL))
 {
  snprintf(filename, 50, "PICS%sSLOT%d.PIC", dir, pic);
  if (PHYSFS_exists(filename))
  {
   img = SDL_CreateRGBSurface(SDL_SWSURFACE, 112, 88, 8, 0, 0, 0, 0);
   char *pixels = (char *)img->pixels;
   BTCompressorReadFile file(filename);
   for (int y = 0; y < 88; ++y)
   {
    file.readUByteArray(112, (IUByte *)pixels);
    pixels += img->pitch;
   }
   SDL_Color *colors = img->format->palette->colors;
   img->format->palette->ncolors = 256;
   snprintf(filename, 50, "PICS%sSLOT%d.PAC", dir, pic);
   BTCompressorReadFile palFile(filename);
   for (int c = 0; c < 256; ++c)
   {
    palFile.readUByte((IUByte&)colors[c].r);
    palFile.readUByte((IUByte&)colors[c].g);
    palFile.readUByte((IUByte&)colors[c].b);
    colors[c].r *= 4;
    colors[c].g *= 4;
    colors[c].b *= 4;
   }
  }
 }
 SDL_Rect src, dst;
 dst.x = config->x3d * xMult;
 dst.y = config->y3d * yMult;
 dst.w = p3d.config->width * xMult;
 dst.h = p3d.config->height * yMult;
 if (NULL == img)
 {
  SDL_FillRect(mainScreen, &dst, SDL_MapRGB(mainScreen->format, black.r, black.g, black.b));
  SDL_UpdateRect(mainScreen, dst.x, dst.y, dst.w, dst.h);
  return;
 }
 if ((xMult > 1) || (yMult > 1))
 {
  SDL_Surface *img2 = simpleZoomSurface(img, xMult, yMult);
  SDL_FreeSurface(img);
  img = img2;
 }
 src.x = 0;
 src.y = 0;
 src.w = p3d.config->width * xMult;
 src.h = p3d.config->height * yMult;
 SDL_BlitSurface(img, &src, mainScreen, &dst);
 SDL_FreeSurface(img);
 SDL_UpdateRect(mainScreen, dst.x, dst.y, dst.w, dst.h);
}

void BTDisplay::drawLabel(const char *name)
{
 int w, h;
 if (!sizeFont(name, w, h))
  return;
 SDL_BlitSurface(mainBackground, &label, mainScreen, &label);
 drawFont(name, label, white, center);
 SDL_UpdateRect(mainScreen, label.x, label.y, label.w, label.h);
}

void BTDisplay::drawChoice(const char *keys, const char *words, alignment a /*= left*/)
{
 int w, h;
 if (!sizeFont(words, w, h))
  return;
 if (h + textPos > text.h)
 {
  scrollUp(h);
 }
 SDL_Rect dst;
 dst.x = text.x;
 dst.y = text.y + textPos;
 dst.w = text.w;
 dst.h = h;
 SDL_BlitSurface(mainBackground, &dst, mainScreen, &dst);
 drawFont(words, dst, black, a);
 SDL_UpdateRect(mainScreen, text.x, text.y, text.w, text.h);
 textPos += h;
}

void BTDisplay::drawLast(const char *keys, const char *words, alignment a /*= left*/)
{
 int w, h;
 if (!sizeFont(words, w, h))
  return;
 SDL_Rect dst;
 dst.x = text.x;
 dst.y = text.y + text.h - h;
 dst.w = text.w;
 dst.h = h;
 SDL_BlitSurface(mainBackground, &dst, mainScreen, &dst);
 drawFont(words, dst, black, a);
 SDL_UpdateRect(mainScreen, text.x, text.y, text.w, text.h);
}

void BTDisplay::drawText(const char *words, alignment a /*= left*/)
{
 int w, h;
 char *tmp = new char[strlen(words)];
 const char *partial = words;
 while (partial)
 {
  if (!sizeFont(partial, w, h))
  {
   delete [] tmp;
   return;
  }
  if (h + textPos > text.h)
  {
   scrollUp(h);
  }
  const char *end = NULL;
  if (w > text.w)
  {
   const char *sp = partial;
   for (end = partial; *end; ++end)
   {
    if (isspace(*end))
    {
     memcpy(tmp + (sp - partial), sp, end - sp);
     tmp[end - partial] = 0;
     sizeFont(tmp, w, h);
     if (w > text.w)
     {
      end = sp;
      break;
     }
     sp = end;
    }
   }
   if (!(*end))
    end = sp;
   if (end == partial)
    end = NULL;
   else
   {
    tmp[end - partial] = 0;
    while (isspace(*end))
     ++end;
    if (!(*end))
     end = NULL;
   }
  }
  SDL_Rect dst;
  dst.x = text.x;
  dst.y = text.y + textPos;
  dst.w = text.w;
  dst.h = h;
  drawFont((end ? tmp : partial), dst, black, a);
  textPos += h;
  SDL_UpdateRect(mainScreen, text.x, text.y, text.w, text.h);
  partial = end;
 }
 delete [] tmp;
}

void BTDisplay::draw2Column(const char *col1, const char *col2)
{
 int w[2], h[2];
 int maxH;
 if ((!sizeFont(col1, w[0], h[0])) || (!sizeFont(col2, w[1], h[1])))
  return;
 maxH = ((h[0] > h[1]) ? h[0] : h[1]);
 if (maxH + textPos > text.h)
 {
  scrollUp(maxH);
 }
 SDL_Rect dst;
 dst.x = text.x;
 dst.y = text.y + textPos;
 dst.w = text.w / 2;
 dst.h = maxH;
 drawFont(col1, dst, black, left);
 dst.x = text.x + text.w / 2;
 dst.y = text.y + textPos;
 dst.w = text.w / 2;
 dst.h = maxH;
 drawFont(col2, dst, black, left);
 textPos += maxH;
 SDL_UpdateRect(mainScreen, text.x, text.y, text.w, text.h);
}

void BTDisplay::drawView()
{
 BTGame *g = BTGame::getGame();
 p3d.draw(g, g->getX(), g->getY(), g->getFacing());
 SDL_Rect src, dst;
 src.x = 0;
 src.y = 0;
 src.w = p3d.config->width * xMult;
 src.h = p3d.config->height * yMult;
 dst.x = config->x3d * xMult;
 dst.y = config->y3d * yMult;
 dst.w = p3d.config->width * xMult;
 dst.h = p3d.config->height * yMult;
 SDL_BlitSurface(p3d.getDisplay(), &src, mainScreen, &dst);
 SDL_UpdateRect(mainScreen, 0, 0, 0, 0);
}

void BTDisplay::drawStats()
{
 int i;
 SDL_Rect dst;
 for (i = 0; i < BT_PARTYSIZE; ++i)
 {
  dst.x = config->status[i].x * xMult;
  dst.y = config->status[i].y * yMult;
  dst.w = config->status[i].w * xMult;
  dst.h = config->status[i].h * yMult;
  SDL_BlitSurface(mainBackground, &dst, mainScreen, &dst);
 }
 status.draw();
 for (i = 0; i < BT_PARTYSIZE; ++i)
 {
  SDL_UpdateRect(mainScreen, config->status[i].x * xMult, config->status[i].y * yMult, config->status[i].w * xMult, config->status[i].h * yMult);
 }
}

SDL_Color &BTDisplay::getBlack()
{
 return black;
}

BTDisplayConfig *BTDisplay::getConfig()
{
 return config;
}

void BTDisplay::getMultiplier(int &x, int &y)
{
 x = xMult;
 y = yMult;
}

SDL_Color &BTDisplay::getWhite()
{
 return white;
}

unsigned int BTDisplay::process(const char *specialKeys /*= NULL*/, int delay /*= 0*/)
{
 unsigned int key;
 std::vector<BTUIElement*>::iterator top = element.begin();
 for (; top != element.end(); ++top)
 {
  if ((BTUI_SELECT == (*top)->getType()) || (BTUI_BARRIER == (*top)->getType()) || (BTUI_READSTRING == (*top)->getType()))
  {
   break;
  }
  else if (BTUI_2COLUMN == (*top)->getType())
  {
   BTUI2Column *item = static_cast<BTUI2Column*>(*top);
   int maxH = item->maxHeight(*this);
   if (0 == maxH)
    continue;
   if (maxH + textPos > text.h)
   {
    scrollUp(maxH);
   }
   item->draw(text.x, text.y + textPos, text.w, maxH, *this);
   textPos += maxH;
  }
  else if ((BTUI_TEXT == (*top)->getType()) || (BTUI_CHOICE == (*top)->getType()))
  {
   BTUIText *item = static_cast<BTUIText*>(*top);
   int maxH = item->maxHeight(*this);
   if (maxH + textPos > text.h)
   {
    scrollUp(maxH);
   }
   item->position.x = text.x;
   item->position.y = text.y + textPos;
   item->position.w = text.w;
   item->position.h = maxH;
   drawFont(item->text.c_str(), item->position, black, item->align);
   textPos += maxH;
  }
 }
 BTUISelect *select = NULL;
 if (top != element.end())
 {
  std::vector<BTUIElement*>::iterator bottom = --(element.end());
  int bottomPos = text.h;
  for (; bottom != top; --bottom)
  {
   if (BTUI_2COLUMN == (*bottom)->getType())
   {
    BTUI2Column* item = static_cast<BTUI2Column*>(*bottom);
    int maxH = item->maxHeight(*this);
    if (0 == maxH)
     continue;
    bottomPos -= maxH;
    item->draw(text.x, text.y + bottomPos, text.w, maxH, *this);
   }
   else if ((BTUI_TEXT == (*bottom)->getType()) || (BTUI_CHOICE == (*bottom)->getType()))
   {
    BTUIText *item = static_cast<BTUIText*>(*bottom);
    int maxH = item->maxHeight(*this);
    bottomPos -= maxH;
    item->position.x = text.x;
    item->position.y = text.y + bottomPos;
    item->position.w = text.w;
    item->position.h = maxH;
    drawFont(item->text.c_str(), item->position, black, item->align);
   }
  }
  if (BTUI_READSTRING == (*top)->getType())
  {
   BTUIReadString *item = static_cast<BTUIReadString*>(*top);
   SDL_UpdateRect(mainScreen, text.x, text.y, text.w, text.h);
   item->response = readString(item->prompt.c_str(), item->maxLen);
   return 13;
  }
  else if (BTUI_SELECT == (*top)->getType())
  {
   select = static_cast<BTUISelect*>(*top);
   select->position.x = text.x;
   select->position.y = text.y + textPos;
   select->position.w = text.w;
   select->position.h = bottomPos - textPos;
  }
 }
 if (!select)
  SDL_UpdateRect(mainScreen, text.x, text.y, text.w, text.h);
 else if (select->numbered)
 {
   select->draw(*this);
   SDL_UpdateRect(mainScreen, text.x, text.y, text.w, text.h);
 }
 SDL_TimerID timer;
 while (true)
 {
  if ((select) && (!select->numbered))
  {
   select->draw(*this);
   SDL_UpdateRect(mainScreen, text.x, text.y, text.w, text.h);
  }
  if (delay)
   timer = SDL_AddTimer(delay, timerCallback, NULL);
  key = readChar();
  if (delay != 0)
  {
   if (key == 0)
    break;
   else
    SDL_RemoveTimer(timer);
  }
  if (key == 27)
   break;
  if (select)
  {
   if ((key >= '1') && (key <= '9'))
   {
    int offset = key - '1';
    if (select->start + offset < select->size)
    {
     select->select = select->start + offset;
     key = 13;
     break;
    }
    else
     key = 1;
   }
   else if (!select->numbered)
   {
    if (key == BTKEY_UP)
    {
     if (select->select > 0)
      --select->select;
     continue;
    }
    else if (key == BTKEY_DOWN)
    {
     if (select->select + 1 < select->size)
      ++select->select;
     continue;
    }
    else if ((key == 13) && (select->select >= 0))
     break;
   }
  }
  char utf8Key[5];
  // FIXME: Do real utf-8 conversion
  utf8Key[0] = key;
  utf8Key[1] = 0;
  for (top = element.begin(); top != element.end(); top++)
  {
   if (BTUI_CHOICE == (*top)->getType())
   {
    BTUIChoice *item = static_cast<BTUIChoice*>(*top);
    if (item->keys.find(utf8Key) != std::string::npos)
     return key;
   }
   else if (BTUI_BARRIER == (*top)->getType())
   {
    BTUIBarrier *item = static_cast<BTUIBarrier*>(*top);
    if (item->keys.find(utf8Key) != std::string::npos)
     return key;
   }
  }
  if (specialKeys == allKeys)
   return key;
  else if (specialKeys)
  {
   for (int i = 0; specialKeys[i]; ++i)
   {
    if (specialKeys[i] == key)
     return key;
   }
  }
 }
 return key;
}

unsigned int BTDisplay::readChar()
{
 SDL_Event sdlevent;
 while (true)
 {
  SDL_WaitEvent(&sdlevent);
  if (sdlevent.type == SDL_KEYDOWN)
  {
   if (sdlevent.key.keysym.unicode)
    return sdlevent.key.keysym.unicode;
   else if (sdlevent.key.keysym.sym == SDLK_UP)
    return BTKEY_UP;
   else if (sdlevent.key.keysym.sym == SDLK_DOWN)
    return BTKEY_DOWN;
   else if (sdlevent.key.keysym.sym == SDLK_LEFT)
    return BTKEY_LEFT;
   else if (sdlevent.key.keysym.sym == SDLK_RIGHT)
    return BTKEY_RIGHT;
  }
  else if (sdlevent.type == SDL_USEREVENT)
  {
   return 0;
  }
 }
}

std::string BTDisplay::readString(const char *prompt, int max)
{
 std::string s;
 int w, h;
 sizeFont(s.c_str(), w, h);
 if (h + textPos > text.h)
  scrollUp(h);
 unsigned char key;
 int len = 0;
 SDL_Rect dst;
 dst.h = h;
 if (0 != *prompt)
 {
  sizeFont(prompt, w, h);
  dst.x = text.x;
  dst.y = text.y + textPos;
  dst.w = w;
  SDL_BlitSurface(mainBackground, &dst, mainScreen, &dst);
  drawFont(prompt, dst, black, left);
 }
 dst.x = text.x + w;
 dst.y = text.y + textPos;
 dst.w = text.w - w;
 SDL_UpdateRect(mainScreen, text.x, text.y, text.w, text.h);
 while (((key = readChar()) != 13) && (key !=  27))
 {
  if (key == 8)
  {
   if (len > 0)
    s.erase(--len);
  }
  else if ((len < max) && (key >= ' ') && (key <= '~'))
  {
   s.push_back(key);
   ++len;
  }
  SDL_BlitSurface(mainBackground, &dst, mainScreen, &dst);
  drawFont(s.c_str(), dst, black, left);
  SDL_UpdateRect(mainScreen, text.x, text.y, text.w, text.h);
 }
 textPos += h;
 return s;
}

void BTDisplay::refresh()
{
 SDL_BlitSurface(mainBackground, NULL, mainScreen, NULL);
 drawStats();
 SDL_UpdateRect(mainScreen, 0, 0, 0, 0);
}

bool BTDisplay::selectList(selectItem *list, int size, int &start, int &select)
{
 int wFirst, wValue, h, hTmp, lines;
 unsigned char key = 0;
 char tmp[20];
 SDL_Rect dst;
 sizeFont("", wFirst, h);
 sizeFont("@", wFirst, hTmp);
 lines = text.h / h - 1;
 while ((key != 13) && (key !=  27))
 {
  clearText();
  dst.y = text.y;
  dst.h = h;
  for (int i = start; i < start + lines; ++i)
  {
   if (i >= size)
    break;
   if (select == i)
   {
    dst.x = text.x;
    dst.w = text.w;
    SDL_FillRect(mainScreen, &dst, SDL_MapRGB(mainScreen->format, black.r, black.g, black.b));
   }
   dst.x = text.x;
   dst.w = wFirst;
   tmp[0] = list[i].first;
   tmp[1] = 0;
   drawFont(tmp, dst, ((select != i) ? black : white), left);
   if (list[i].value)
   {
    snprintf(tmp, 20, "%d", list[i].value);
    sizeFont(tmp, wValue, hTmp);
    dst.w = text.w;
    drawFont(tmp, dst, ((select != i) ? black : white), right);
   }
   else
    wValue = 0;
   dst.x += wFirst;
   dst.w = text.w - wValue - wFirst;
   drawFont(list[i].name, dst, ((select != i) ? black : white), left);
   dst.y += h;
  }
  SDL_UpdateRect(mainScreen, text.x, text.y, text.w, text.h);
  key = readChar();
  if (key == 0xBD) // up
  {
   if (select > 0)
    --select;
  }
  else if (key == 0xC3) // down
  {
   if (select + 1 < size)
    ++select;
  }
 }
 return (key == 13);
}

void BTDisplay::setBackground(const char *file)
{
 SDL_RWops *f = PHYSFSRWOPS_openRead(file);
 SDL_Surface *img = IMG_Load_RW(f, 1);
 if ((xMult > 1) || (yMult > 1))
 {
  mainBackground = simpleZoomSurface(img, xMult, yMult);
  SDL_FreeSurface(img);
 }
 else
  mainBackground = img;
 SDL_BlitSurface(mainBackground, NULL, mainScreen, NULL);
 SDL_UpdateRect(mainScreen, 0, 0, 0, 0);
}

void BTDisplay::setPsuedo3DConfig(const char *file)
{
 Psuedo3DConfig::readXML(file, p3dConfig);
}

void BTDisplay::setWallGraphics(int type)
{
 p3d.setConfig(p3dConfig[type]);
}

bool BTDisplay::sizeFont(const char *text, int &w, int &h)
{
 if (ttffont)
 {
  if (0 == *text)
  {
   w = 0;
   h = TTF_FontHeight(ttffont);
  }
  else if (TTF_SizeUTF8(ttffont, text, &w, &h) == -1)
    return false;
 }
 else
 {
  w = strlen(text) * sfont->w;
  h = sfont->h;
 }
 if ((xMult != yMult) || ((ttffont == NULL) && ((xMult > 1) || (yMult > 1))))
 {
  w *= xMult;
  h *= yMult;
 }
 return true;
}

void BTDisplay::drawFont(const char *text, SDL_Rect &dst, SDL_Color c, alignment a)
{
 if (0 == *text)
  return;
 SDL_Surface *img;
 if (ttffont)
  img = TTF_RenderUTF8_Solid(ttffont, text, c);
 else
  img = simpleRender_Solid(sfont, text, c);
 if ((xMult != yMult) || ((ttffont == NULL) && ((xMult > 1) || (yMult > 1))))
 {
  SDL_Surface *img2 = simpleZoomSurface(img, xMult, yMult);
  SDL_FreeSurface(img);
  img = img2;
 }
 SDL_Rect src, final;
 src.x = 0;
 src.y = 0;
 src.w = ((img->w > dst.w) ? dst.w : img->w);
 src.h = ((img->h > dst.h) ? dst.h : img->h);
 switch (a)
 {
  case left:
   final.x = dst.x;
   final.y = dst.y;
   final.w = src.w;
   final.h = src.h;
   break;
  case center:
   final.x = ((img->w > dst.w) ? dst.x : dst.x + (dst.w / 2) - (img->w / 2));
   final.y = ((img->h > dst.h) ? dst.y : dst.y + (dst.h / 2) - (img->h / 2));
   final.w = src.w;
   final.h = src.h;
   break;
  case right:
   final.x = dst.x + dst.w - src.w;
   final.y = dst.y + dst.h - src.h;
   final.w = src.w;
   final.h = src.h;
   break;
 }
 SDL_BlitSurface(img, &src, mainScreen, &final);
 SDL_FreeSurface(img);
}

void BTDisplay::drawRect(SDL_Rect &dst, SDL_Color c)
{
 SDL_FillRect(mainScreen, &dst, SDL_MapRGB(mainScreen->format, c.r, c.g, c.b));
}

void BTDisplay::scrollUp(int h)
{
 SDL_Rect src, dst;
 src.x = dst.x = text.x;
 src.y = text.y + h;
 src.w = dst.w = text.w;
 src.h = dst.h = text.h - h;
 dst.y = text.y;
 SDL_BlitSurface(mainScreen, &src, mainScreen, &dst);
 src.x = text.x;
 src.y = text.y + text.h - h;
 src.w = text.w;
 src.h = h;
 SDL_BlitSurface(mainBackground, &src, mainScreen, &src);
 SDL_UpdateRect(mainScreen, text.x, text.y, text.w, text.h);
 textPos -= h;
}

Uint32 BTDisplay::timerCallback(Uint32 interval, void *param)
{
 SDL_Event event;
 SDL_UserEvent userevent;

 userevent.type = SDL_USEREVENT;
 userevent.code = 0;
 userevent.data1 = NULL;
 userevent.data2 = NULL;

 event.type = SDL_USEREVENT;
 event.user = userevent;

 SDL_PushEvent(&event);
 return 0;
}

int BTUIText::maxHeight(BTDisplay &d)
{
 int w, h;
 if (!d.sizeFont(text.c_str(), w, h))
  return 0;
 return h;
}

BTUISelect::BTUISelect(BTDisplay::selectItem *l, int sz, int &st, int &sel, int num /*= 0*/)
 : list(l), size(sz), start(st), select(sel), numbered(num)
{
 if (size > 9)
  numbered = 0;
}

void BTUISelect::draw(BTDisplay &d)
{
 int wFirst, wValue, h, hTmp, lines;
 char tmp[20];
 SDL_Rect dst;
 d.sizeFont("", wFirst, h);
 lines = position.h / h;
 d.clear(position);
 dst.y = position.y;
 dst.h = h;
 if (numbered)
 {
  d.sizeFont("1) ", wFirst, hTmp);
  int i = 0;
  for (; i < size; ++i)
  {
   dst.x = position.x;
   dst.w = wFirst;
   tmp[0] = '1' + i;
   tmp[1] = ')';
   tmp[2] = list[i].first;
   tmp[3] = 0;
   d.drawFont(tmp, dst, d.getBlack(), BTDisplay::left);
   dst.x += wFirst;
   dst.w = position.w - wFirst;
   if (list[i].name)
    d.drawFont(list[i].name, dst, d.getBlack(), BTDisplay::left);
   dst.y += h;
  }
  for (; i < numbered; ++i)
  {
   dst.x = position.x;
   dst.w = wFirst;
   tmp[0] = '1' + i;
   tmp[1] = ')';
   tmp[2] = 0;
   d.drawFont(tmp, dst, d.getBlack(), BTDisplay::left);
   dst.y += h;
  }
 }
 else if (size > 0)
 {
  d.sizeFont("@", wFirst, hTmp);
  for (int i = start; i < start + lines; ++i)
  {
   if (i >= size)
    break;
   if (select == i)
   {
    dst.x = position.x;
    dst.w = position.w;
    d.drawRect(dst, d.getBlack());
   }
   dst.x = position.x;
   dst.w = wFirst;
   tmp[0] = list[i].first;
   tmp[1] = 0;
   d.drawFont(tmp, dst, ((select != i) ? d.getBlack() : d.getWhite()), BTDisplay::left);
   if (list[i].value)
   {
    snprintf(tmp, 20, "%d", list[i].value);
    d.sizeFont(tmp, wValue, hTmp);
    dst.w = position.w;
    d.drawFont(tmp, dst, ((select != i) ? d.getBlack() : d.getWhite()), BTDisplay::right);
   }
   else
    wValue = 0;
   dst.x += wFirst;
   dst.w = position.w - wValue - wFirst;
   if (list[i].name)
    d.drawFont(list[i].name, dst, ((select != i) ? d.getBlack() : d.getWhite()), BTDisplay::left);
   dst.y += h;
  }
 }
}

void BTUI2Column::draw(int x, int y, int w, int h, BTDisplay& d)
{
 SDL_Rect dst;
 dst.x = x;
 dst.y = y;
 dst.w = w / 2;
 dst.h = h;
 d.drawFont(col1.c_str(), dst, d.getBlack(), BTDisplay::left);
 dst.x = x + w / 2;
 dst.y = y;
 dst.w = w / 2;
 dst.h = h;
 d.drawFont(col2.c_str(), dst, d.getBlack(), BTDisplay::left);
}

int BTUI2Column::maxHeight(BTDisplay &d)
{
 int w[2], h[2];
 if ((!d.sizeFont(col1.c_str(), w[0], h[0])) || (!d.sizeFont(col2.c_str(), w[1], h[1])))
  return 0;
 return ((h[0] > h[1]) ? h[0] : h[1]);
}
