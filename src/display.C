/*-------------------------------------------------------------------------*\
  <display.C> -- Display implementation file

  Date      Programmer  Description
  02/28/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "compressor.h"
#include "display.h"
#include "game.h"
#include "physfsrwops.h"
#include "imagetag.h"
#include <SDL_image.h>
#include "boost/filesystem/operations.hpp"

const char *BTDisplay::allKeys = "allKeys";
std::string BTDisplay::displayDir;

BTLabelWidget::BTLabelWidget(BTLabelConfig *c, int xMult, int yMult)
 : config(c)
{
 location.x = config->location.x * xMult;
 location.y = config->location.y * yMult;
 location.w = config->location.w * xMult;
 location.h = config->location.h * yMult;
}

void BTLabelWidget::render(BTBackgroundAndScreen *d, bool refresh /*= false*/)
{
 if (modified || refresh)
 {
  modified = false;
  d->clear(location);
  d->drawFont(text.c_str(), location, d->getColor(config->color), BTAlignment::center);
 }
}

BTTextWidget::BTTextWidget(BTTextConfig *c, int xMult, int yMult)
 : textPos(0), config(c), processor(NULL), modified(false)
{
 location.x = config->location.x * xMult;
 location.y = config->location.y * yMult;
 location.w = config->location.w * xMult;
 location.h = config->location.h * yMult;
}

void BTTextWidget::addElement(BTUIElement *elm)
{
 modified = true;
 processor = NULL;
 element.push_back(elm);
}

void BTTextWidget::clear(BTBackgroundAndScreen *d)
{
 d->clear(location);
 textPos = 0;
}

void BTTextWidget::clearElements()
{
 modified = true;
 processor = NULL;
 for (std::vector<BTUIElement*>::iterator elementItr = element.begin(); element.end() != elementItr; ++elementItr)
 {
  delete (*elementItr);
 }
 element.clear();
}

void BTTextWidget::clearLast()
{
 last = "";
}

void BTTextWidget::drawLast(BTBackgroundAndScreen *d, const char *words)
{
 last = words;
}

void BTTextWidget::drawText(BTBackgroundAndScreen *d, const char *words, BTAlignment::alignment a /*= left*/)
{
 int w, h;
 char *tmp = new char[strlen(words)];
 const char *partial = words;
 while (partial)
 {
  if (!d->getDisplay()->sizeFont(partial, w, h))
  {
   delete [] tmp;
   return;
  }
  if (h + textPos > location.h)
  {
   d->scrollUp(location, h);
   textPos -= h;
  }
  const char *end = NULL;
  if (w > location.w)
  {
   const char *sp = partial;
   for (end = partial; *end; ++end)
   {
    if (isspace(*end))
    {
     memcpy(tmp + (sp - partial), sp, end - sp);
     tmp[end - partial] = 0;
     d->getDisplay()->sizeFont(tmp, w, h);
     if (w > location.w)
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
  dst.x = location.x;
  dst.y = location.y + textPos;
  dst.w = location.w;
  dst.h = h;
  d->drawFont((end ? tmp : partial), dst, d->getColor("black"), a);
  textPos += h;
  partial = end;
 }
 delete [] tmp;
}

unsigned int BTTextWidget::process(BTBackgroundAndScreen *d, const char *specialKeys /*= NULL*/, int *delay /*= 0*/, int delayOveride /*= -1*/)
{
 unsigned int key;
 d->getDisplay()->render();
 BTUISelect *select = NULL;
 if (processor)
 {
  if (BTUI_READSTRING == processor->getType())
  {
   BTUIReadString *item = static_cast<BTUIReadString*>(processor);
   item->response = readString(d, item->prompt.c_str(), item->maxLen, item->response);
   return 13;
  }
  else if (BTUI_SELECTIMAGE == processor->getType())
  {
   BTUISelectImage *item = static_cast<BTUISelectImage*>(processor);
   item->select = selectImage(d, item->select);
   return 13;
  }
  else if (BTUI_SELECTFLAG == processor->getType())
  {
   BTUISelectFlag *item = static_cast<BTUISelectFlag*>(processor);
   item->select = selectFlag(d, item->select, item->flagName);
   return 13;
  }
  else if (BTUI_SELECT == processor->getType())
  {
   select = static_cast<BTUISelect*>(processor);
  }
 }
 int start = SDL_GetTicks();
 int delayCurrent = ((delayOveride != -1) ? delayOveride : (delay ? *delay : 0));
 while (true)
 {
  d->getDisplay()->render();
  key = d->getDisplay()->readChar(delayCurrent);
  if ((key == 0) || (key == 27))
   break;
  if (select)
  {
   if ((key >= '1') && (key <= '9'))
   {
    int offset = key - '1';
    if ((select->size > 0) && (select->start + offset < select->size))
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
     select->moveUp(*d->getDisplay());
     continue;
    }
    else if (key == BTKEY_DOWN)
    {
     select->moveDown(*d->getDisplay());
     continue;
    }
    else if (key == BTKEY_PGUP)
    {
     select->pageUp(*d->getDisplay());
     continue;
    }
    else if (key == BTKEY_PGDN)
    {
     select->pageDown(*d->getDisplay());
     continue;
    }
    else if (key == BTKEY_LEFT)
    {
     select->decrement(*d->getDisplay());
     continue;
    }
    else if (key == BTKEY_RIGHT)
    {
     select->increment(*d->getDisplay());
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
  for (std::vector<BTUIElement*>::iterator top = element.begin(); top != element.end(); top++)
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
  if ((key == '+') || (key == '-'))
  {
   if (key == '+')
   {
    if ((delay) && ((*delay) > 100))
     *delay = (*delay) - 300;
   }
   if (key == '-')
   {
    if ((delay) && ((*delay) < 4000))
     *delay = (*delay) + 300;
   }
   int end = SDL_GetTicks();
   if (delayCurrent)
   {
    if (end - start > delayCurrent)
     return 0;
    else
     delayCurrent -= end - start;
   }
  }
  else if (specialKeys == BTDisplay::allKeys)
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

std::string BTTextWidget::readString(BTBackgroundAndScreen *d, const char *prompt, int max, const std::string &initial)
{
 std::string s = initial;
 int w, h;
 d->getDisplay()->sizeFont(s.c_str(), w, h);
 if (h + textPos > location.h)
 {
  d->scrollUp(location, h);
  textPos -= h;
 }
 unsigned char key;
 int len = s.length();
 SDL_Rect dst;
 dst.h = h;
 int startPos = textPos;
 std::string full = prompt;
 full += s;
 dst.x = location.x;
 dst.y = location.y + textPos;
 dst.w = location.w;
 d->getDisplay()->drawText(full.c_str());
 int endPos = textPos;
 while (((key = d->getDisplay()->readChar()) != 13) && (key !=  27))
 {
  if (key == 8)
  {
   if (len > 0)
   {
    s.erase(--len);
    full.erase(full.length() - 1);
   }
  }
  else if ((len < max) && (key >= ' ') && (key <= '~'))
  {
   s.push_back(key);
   full.push_back(key);
   ++len;
  }
  dst.h = endPos - startPos;
  d->clear(dst);
  textPos = startPos;
  d->getDisplay()->drawText(full.c_str());
  if (textPos > endPos)
   endPos = textPos;
  d->getDisplay()->render();
 }
 return s;
}

void BTTextWidget::render(BTBackgroundAndScreen *d, bool refresh /*= false*/)
{
 if (modified || refresh)
 {
  modified = false;
  std::vector<BTUIElement*>::iterator elementEnd = element.end();
  std::vector<BTUIElement*>::iterator top = element.begin();
  for (; top != elementEnd; ++top)
  {
   if ((BTUI_SELECT == (*top)->getType()) || (BTUI_BARRIER == (*top)->getType()) || (BTUI_READSTRING == (*top)->getType()) || (BTUI_SELECTIMAGE == (*top)->getType()) || (BTUI_SELECTFLAG == (*top)->getType()))
   {
    break;
   }
   else if (BTUI_MULTICOLUMN == (*top)->getType())
   {
    BTUIMultiColumn *item = static_cast<BTUIMultiColumn*>(*top);
    int maxH = item->maxHeight(*d->getDisplay());
    if (0 == maxH)
     continue;
    if (maxH + textPos > location.h)
    {
     d->scrollUp(location, maxH);
     textPos -= maxH;
    }
    item->draw(location.x, location.y + textPos, location.w, maxH, *d->getDisplay());
    textPos += maxH;
   }
   else if ((BTUI_TEXT == (*top)->getType()) || (BTUI_CHOICE == (*top)->getType()))
   {
    BTUIText *item = static_cast<BTUIText*>(*top);
    int maxH = item->maxHeight(*d->getDisplay());
    if (maxH + textPos > location.h)
    {
     d->scrollUp(location, maxH);
     textPos -= maxH;
    }
    item->position.x = location.x;
    item->position.y = location.y + textPos;
    item->position.w = location.w;
    item->position.h = maxH;
    d->drawFont(item->text.c_str(), item->position, d->getColor("black"), item->align);
    textPos += maxH;
   }
  }
  if (top != elementEnd)
  {
   std::vector<BTUIElement*>::iterator bottom = --(element.end());
   int bottomPos = location.h;
   for (; bottom != top; --bottom)
   {
    if (BTUI_MULTICOLUMN == (*bottom)->getType())
    {
     BTUIMultiColumn* item = static_cast<BTUIMultiColumn*>(*bottom);
     int maxH = item->maxHeight(*d->getDisplay());
     if (0 == maxH)
      continue;
     bottomPos -= maxH;
     item->draw(location.x, location.y + bottomPos, location.w, maxH, *d->getDisplay());
    }
    else if ((BTUI_TEXT == (*bottom)->getType()) || (BTUI_CHOICE == (*bottom)->getType()))
    {
     BTUIText *item = static_cast<BTUIText*>(*bottom);
     int maxH = item->maxHeight(*d->getDisplay());
     bottomPos -= maxH;
     item->position.x = location.x;
     item->position.y = location.y + bottomPos;
     item->position.w = location.w;
     item->position.h = maxH;
     d->drawFont(item->text.c_str(), item->position, d->getColor("black"), item->align);
    }
   }
   if ((BTUI_READSTRING == (*top)->getType()) || (BTUI_SELECTIMAGE == (*top)->getType()) || (BTUI_SELECTFLAG == (*top)->getType()))
   {
    processor = *top;
    return;
   }
   else if (BTUI_SELECT == (*top)->getType())
   {
    BTUISelect *select = static_cast<BTUISelect*>(*top);
    select->position.x = location.x;
    select->position.y = location.y + textPos;
    select->position.w = location.w;
    select->position.h = bottomPos - textPos;
    select->sanitize(*d->getDisplay());
    select->draw(d);
    processor = *top;
   }
  }
 }
 if ((processor) && (BTUI_SELECT == processor->getType()))
 {
  BTUISelect *select = static_cast<BTUISelect*>(processor);
  if (!select->numbered)
  {
   select->draw(d);
  }
 }
 if (last != "")
 {
  int w, h;
  if (!d->getDisplay()->sizeFont(last.c_str(), w, h))
   return;
  SDL_Rect dst;
  dst.x = location.x;
  dst.y = location.y + location.h - h;
  dst.w = location.w;
  dst.h = h;
  d->clear(dst);
  d->drawFont(last.c_str(), dst, d->getColor("black"), BTAlignment::left);
 }
}

int BTTextWidget::selectFlag(BTBackgroundAndScreen *d, int initial, BTFlagNameList *f)
{
 bool blank = true;
 bool enterMode = false;
 std::string s;
 int w, h;
 d->getDisplay()->sizeFont(s.c_str(), w, h);
 if (h + textPos > location.h)
 {
  d->scrollUp(location, h);
  textPos -= h;
 }
 while ((f->size() <= initial) || (f->size() < 16))
 {
  f->push_back(new BTFlagName);
  f->back()->slot = f->size()-1;
 }
 unsigned char key;
 int len = s.length();
 SDL_Rect dst;
 dst.h = h;
 int startPos = textPos;
 std::string full;
 full += s;
 dst.x = location.x;
 dst.y = location.y + textPos;
 dst.w = location.w;
 d->getDisplay()->drawText(full.c_str());
 int endPos = textPos;
 int bottomPos = location.h;
 int current = initial;
 int selected = current;
 int sz = 0;
 int st = 0;
 BTDisplay::selectItem *sl = f->search(s, blank, current, sz, selected);
 BTUISelect *select = new BTUISelect(sl, sz, st, selected);
 select->position.x = location.x;
 select->position.y = location.y + textPos;
 select->position.w = location.w;
 select->position.h = bottomPos - textPos;
 select->sanitize(*d->getDisplay());
 select->draw(d);
 d->getDisplay()->render();
 while ((((key = d->getDisplay()->readChar()) != 13) && (key !=  27)) || enterMode)
 {
  bool searchChange = false;
  if (enterMode)
  {
   if ((key == '\t') || (key == 13))
   {
    (*f)[current]->name = sl[selected].name;
    (*f)[current]->buildMatchString();
    enterMode = false;
   }
   else if (key == 8)
   {
    if (sl[selected].name.length() > 0)
    {
     sl[selected].name.erase(sl[selected].name.length() - 1);
    }
   }
   else if ((sl[selected].name.length() < 50) && (key >= ' ') && (key <= '~'))
   {
    sl[selected].name.push_back(key);
   }
  }
  else
  {
   if (key == 8)
   {
    if (len > 0)
    {
     s.erase(--len);
     full.erase(full.length() - 1);
     searchChange = true;
    }
   }
   else if (key == '\t')
   {
    enterMode = true;
   }
   else if ((key == '+') || (key == BTKEY_UP))
   {
    select->moveUp(*d->getDisplay());
   }
   else if ((key == '-') || (key == BTKEY_DOWN))
   {
    if ((f->size() - 1 == selected) && (s == ""))
    {
     f->push_back(new BTFlagName);
     f->back()->slot = f->size()-1;
     current = selected;
     sl = f->search(s, blank, current, sz, selected);
     select->alter(sl, sz);
     select->sanitize(*d->getDisplay());
    }
    select->moveDown(*d->getDisplay());
   }
   else if (key == BTKEY_PGUP)
   {
    select->pageUp(*d->getDisplay());
   }
   else if (key == BTKEY_PGDN)
   {
    select->pageDown(*d->getDisplay());
   }
   else if (key == BTKEY_F1)
   {
    blank = !blank;
    searchChange = true;
   }
   else if ((len < 50) && (key >= ' ') && (key <= '~'))
   {
    s.push_back(key);
    full.push_back(key);
    ++len;
    searchChange = true;
   }
   else
    continue;
  }
  current = sl[selected].value;
  if (searchChange)
  {
   sl = f->search(s, blank, current, sz, selected);
   select->alter(sl, sz);
   select->sanitize(*d->getDisplay());
  }
  dst.h = endPos - startPos;
  d->clear(dst);
  textPos = startPos;
  d->getDisplay()->drawText(full.c_str());
  if (textPos > endPos)
   endPos = textPos;
  select->draw(d);
  d->getDisplay()->render();
 }
 delete select;
 return ((key == 27) ? initial : current);
}

int BTTextWidget::selectImage(BTBackgroundAndScreen *d, int initial)
{
 BTImageTagList &tagList = BTCore::getCore()->getImageTagList();
 bool blank = true;
 std::string s;
 int w, h;
 d->getDisplay()->sizeFont(s.c_str(), w, h);
 if (h + textPos > location.h)
 {
  d->scrollUp(location, h);
  textPos -= h;
 }
 unsigned char key;
 int len = s.length();
 SDL_Rect dst;
 dst.h = h;
 int startPos = textPos;
 std::string full;
 full += s;
 dst.x = location.x;
 dst.y = location.y + textPos;
 dst.w = location.w;
 d->getDisplay()->drawText(full.c_str());
 int endPos = textPos;
 int bottomPos = location.h;
 int current = initial;
 int selected = current;
 d->getDisplay()->drawImage(current);
 int sz = 0;
 int st = 0;
 BTDisplay::selectItem *sl = tagList.search(s, blank, current, sz, selected);
 BTUISelect *select = new BTUISelect(sl, sz, st, selected);
 select->position.x = location.x;
 select->position.y = location.y + textPos;
 select->position.w = location.w;
 select->position.h = bottomPos - textPos;
 select->sanitize(*d->getDisplay());
 select->draw(d);
 d->getDisplay()->render();
 while (((key = d->getDisplay()->readChar()) != 13) && (key !=  27))
 {
  bool searchChange = false;
  if (key == 8)
  {
   if (len > 0)
   {
    s.erase(--len);
    full.erase(full.length() - 1);
    searchChange = true;
   }
  }
  else if ((key == '+') || (key == BTKEY_UP))
  {
   select->moveUp(*d->getDisplay());
  }
  else if ((key == '-') || (key == BTKEY_DOWN))
  {
   select->moveDown(*d->getDisplay());
  }
  else if (key == BTKEY_PGUP)
  {
   select->pageUp(*d->getDisplay());
  }
  else if (key == BTKEY_PGDN)
  {
   select->pageDown(*d->getDisplay());
  }
  else if (key == BTKEY_F1)
  {
   blank = !blank;
   searchChange = true;
  }
  else if ((len < 50) && (key >= ' ') && (key <= '~'))
  {
   s.push_back(key);
   full.push_back(key);
   ++len;
   searchChange = true;
  }
  else
   continue;
  current = sl[selected].value;
  if (searchChange)
  {
   sl = tagList.search(s, blank, current, sz, selected);
   select->alter(sl, sz);
   select->sanitize(*d->getDisplay());
  }
  dst.h = endPos - startPos;
  d->clear(dst);
  textPos = startPos;
  d->getDisplay()->drawText(full.c_str());
  if (textPos > endPos)
   endPos = textPos;
  select->draw(d);
  d->getDisplay()->drawImage(current);
  d->getDisplay()->render();
 }
 delete select;
 return ((key == 27) ? initial : current);
}

BTMusic::~BTMusic()
{
 if (musicObj)
 {
  Mix_FreeMusic(musicObj);
 }
}

BTSound::~BTSound()
{
 if (sound)
 {
  Mix_FreeChunk(sound);
 }
}

BTDisplay::BTDisplay(BTDisplayConfig *c, int multiplier /*= 0*/, bool full /*= false*/, bool softRender /*= false*/)
 : mapXStart(0), mapYStart(0), fullScreen(full), softRenderer(softRender), config(c), expanded(0), xMult(multiplier), yMult(multiplier), lockMult(multiplier), status(NULL), p3d(this, 0, 0),
#ifdef SDL2LIB
   mainWindow(0), mainRenderer(0), mainTexture(0),
#endif
   mainScreen(0), picture(-1), ttffont(0), sfont(&simple8x8)
{
 animation.animation = 0;
 animation.frame = 0;
 if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) < 0)
 {
  printf("Failed - SDL_Init: %s\n", SDL_GetError());
  exit(0);
 }
 Mix_Init(MIX_INIT_FLAC | MIX_INIT_OGG);
#ifdef SDL2LIB
 SDL_DisplayMode info;
 int success = SDL_GetCurrentDisplayMode(0, &info);
 if (success != 0)
 {
  printf("%s\n", SDL_GetError());
  exit(0);
 }
 xFull = info.w;
 yFull = info.h;
#else
 const SDL_VideoInfo *info = SDL_GetVideoInfo();
 xFull = info->current_w;
 yFull = info->current_h;
#endif
 if ((xMult == 0) || (yMult == 0))
 {
  xMult = (xFull - 10) / config->width; // Allow for window decoration
  yMult = (yFull - 10) / config->height; // Allow for window decoration
  if (xMult > yMult)
   xMult = yMult;
  else
   yMult = xMult;
 }
 expanded = config->findExpanded(xMult, yMult);
 char *font = config->font;
 int fontsize = config->fontsize;
 if (expanded)
 {
  if (multiplier == 0)
  {
   // Do not expand as big as possible when not explicitly set.
   // User with multiple monitors had a problem.
   xMult = expanded->xMult;
   yMult = expanded->yMult;
  }
  else
  {
   xMult = ((config->width * multiplier) / (config->width * expanded->xMult)) * expanded->xMult;
   yMult = ((config->height * multiplier) / (config->height * expanded->yMult)) * expanded->yMult;
   if (xMult > yMult)
    xMult = yMult;
   else
    yMult = xMult;
  }
  font = expanded->font;
  fontsize = expanded->fontsize;
 }
 else if (multiplier == 0)
 {
  // Do not expand as big as possible when not explicitly set.
  // User with multiple monitors had a problem.
  xMult = yMult = 1;
 }
 p3d.setMultiplier(xMult, yMult);
#ifndef BTBUILDER_NOTTF
 if (TTF_Init() == -1)
 {
  printf("Failed - TTF_Init\n");
  exit(0);
 }
#endif
#ifdef SDL2LIB
 mainWindow = SDL_CreateWindow("Bt Builder",
                           SDL_WINDOWPOS_UNDEFINED,
                           SDL_WINDOWPOS_UNDEFINED,
                           ((fullScreen && softRenderer) ? xFull : config->width * xMult),
                           ((fullScreen && softRenderer) ? yFull : config->height * yMult),
                           (fullScreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
 if (mainWindow == NULL)
 {
  printf("Failed - SDL_CreateWindow\n");
  exit(0);
 }

 mainRenderer = SDL_CreateRenderer(mainWindow, -1, (softRenderer ? SDL_RENDERER_SOFTWARE : 0));
 if (mainRenderer == NULL)
 {
  printf("Failed - SDL_CreateRenderer\n");
  exit(0);
 }
 mainTexture = SDL_CreateTexture(mainRenderer,
                             SDL_PIXELFORMAT_ARGB8888,
                             SDL_TEXTUREACCESS_STREAMING,
                             ((fullScreen && softRenderer) ? xFull : config->width * xMult),
                             ((fullScreen && softRenderer) ? yFull : config->height * yMult));
 if (mainTexture == NULL)
 {
  printf("Failed - SDL_CreateTexture\n");
  exit(0);
 }
 mainScreen = SDL_CreateRGBSurface(0, config->width * xMult, config->height * yMult, 32,
                                        0x00FF0000,
                                        0x0000FF00,
                                        0x000000FF,
                                        0xFF000000);
 if (mainScreen == NULL)
 {
  printf("Failed - SDL_CreateRGBSurface\n");
  exit(0);
 }
#else
 mainScreen = SDL_SetVideoMode(config->width * xMult, config->height * yMult, 32,
   SDL_SWSURFACE | (fullScreen ? SDL_FULLSCREEN : 0));
 if (mainScreen == NULL)
 {
  printf("Failed - SDL_SetVideoMode\n");
  exit(0);
 }
#endif
#ifndef BTBUILDER_NOTTF
 if (font)
 {
  SDL_RWops *f = PHYSFSRWOPS_openRead((std::string("fonts/") + font).c_str());
  if (f)
   ttffont = TTF_OpenFontRW(f, 1, fontsize);
 }
#endif
 white.r = 255;
 white.g = 255;
 white.b = 255;
 black.r = 0;
 black.g = 0;
 black.b = 0;
 setupScreens(config, xMult, yMult);
 status = new BTStatusBar;

#ifndef SDL2LIB
 SDL_EventState(SDL_ACTIVEEVENT, SDL_IGNORE);
#endif
 SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
 SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);
 SDL_EventState(SDL_MOUSEBUTTONUP, SDL_IGNORE);

 Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024);
 setupKeyMap();
}

BTDisplay::~BTDisplay()
{
 stopMusic(true);
 for (std::list<BTBackgroundAndScreen*>::iterator itr = screen.begin(); screen.end() != itr; ++itr)
 {
  delete (*itr);
 }
 if (status)
  delete status;
#ifdef SDL2LIB
 if (mainScreen)
 {
  SDL_FreeSurface(mainScreen);
 }
#endif
 Mix_Quit();
 SDL_Quit();
}

void BTDisplay::addAnimation(MNG_AnimationState *animState, bool clear /*= false*/)
{
 getVisibleScreen()->addAnimation(animState, clear);
}

void BTDisplay::addBackground(const char *file)
{
 if (screen.size() == 1)
  screen.front()->dupeScreen(mainScreen);
 SDL_Surface * scr = SDL_CreateRGBSurface(0, config->width * xMult, config->height * yMult, 32,
                                        0x00FF0000,
                                        0x0000FF00,
                                        0x000000FF,
                                        0xFF000000);
 if (scr == NULL)
 {
  printf("Failed - SDL_CreateRGBSurface\n");
  exit(0);
 }
 BTBackgroundAndScreen *newScreen = new BTBackgroundAndScreen(this, scr, true);
 screen.push_back(newScreen);
 newScreen->setBackground(file);
}

void BTDisplay::addBarrier(const char *keys)
{
 BTTextWidget *widget = dynamic_cast<BTTextWidget*>(getWidget("text"));
 if (widget)
  widget->addElement(new BTUIBarrier(keys));
}

void BTDisplay::addChoice(const char *keys, const char *words, BTAlignment::alignment a /*= left*/)
{
 int w, h;
 BTTextWidget *widget = dynamic_cast<BTTextWidget*>(getWidget("text"));
 SDL_Rect &text = widget->getLocation();
 char *tmp = new char[strlen(words)];
 const char *partial;
 if (partial = strchr(words, '\n'))
 {
  do
  {
   memcpy(tmp, words, partial - words);
   tmp[partial - words] = 0;
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
  if (widget)
  {
   if (NULL == keys)
    widget->addElement(new BTUIText((end ? tmp : partial), a));
   else
    widget->addElement(new BTUIChoice(keys, (end ? tmp : partial), a));
  }
  partial = end;
 }
 delete [] tmp;
}

void BTDisplay::addText(const char *words, BTAlignment::alignment a /*= left*/)
{
 addChoice(NULL, words, a);
}

void BTDisplay::addColumns(const std::list<std::string>& c)
{
 BTTextWidget *widget = dynamic_cast<BTTextWidget*>(getWidget("text"));
 if (widget)
  widget->addElement(new BTUIMultiColumn(c));
}

void BTDisplay::addReadString(const std::string &prompt, int maxLen, std::string &response)
{
 BTTextWidget *widget = dynamic_cast<BTTextWidget*>(getWidget("text"));
 if (widget)
  widget->addElement(new BTUIReadString(prompt, maxLen, response));
}

void BTDisplay::addSelection(selectItem *list, int size, int &start, int &select, int num /*= 0*/)
{
 BTTextWidget *widget = dynamic_cast<BTTextWidget*>(getWidget("text"));
 if (widget)
  widget->addElement(new BTUISelect(list, size, start, select, num));
}

void BTDisplay::addSelectFlag(int &select, BTFlagNameList *f)
{
 BTTextWidget *widget = dynamic_cast<BTTextWidget*>(getWidget("text"));
 if (widget)
 {
  widget->addElement(new BTUISelectFlag(select, f));
 }
}

void BTDisplay::addSelectImage(int &select)
{
 BTTextWidget *widget = dynamic_cast<BTTextWidget*>(getWidget("text"));
 if (widget)
  widget->addElement(new BTUISelectImage(select));
}

void BTDisplay::clear(SDL_Surface *scr, SDL_Rect &r)
{
 SDL_BlitSurface(scr, &r, mainScreen, &r);
}

void BTDisplay::clearElements()
{
 BTTextWidget *widget = dynamic_cast<BTTextWidget*>(getWidget("text"));
 if (widget)
  widget->clearElements();
}

void BTDisplay::clearImage()
{
 picture = -1;
 if (animation.animation)
 {
  IMG_FreeMNG(animation.animation);
  animation.animation = NULL;
  getVisibleScreen()->removeAnimation(&animation);
 }
 SDL_Rect dst;
 dst.x = config->x3d * xMult;
 dst.y = config->y3d * yMult;
 dst.w = p3d.config->width * xMult;
 dst.h = p3d.config->height * yMult;
 screen.front()->fillRect(dst, black);
}

void BTDisplay::clearLast()
{
 BTTextWidget *widget = dynamic_cast<BTTextWidget*>(getWidget("text"));
 if (widget)
  widget->clearLast();
}

void BTDisplay::clearText()
{
 BTTextWidget *widget = dynamic_cast<BTTextWidget*>(getWidget("text"));
 if (widget)
 {
  widget->clearElements();
  widget->clear(getVisibleScreen());
 }
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
  render();
  SDL_FreeSurface(img);
  if (delay)
   SDL_Delay(delay);
  else
   readChar();
 }
}

void BTDisplay::drawImage(int pic)
{
 picture = pic;
 char filename[50];
 if (animation.animation)
 {
  IMG_FreeMNG(animation.animation);
  animation.animation = NULL;
  screen.front()->removeAnimation(&animation);
 }
 snprintf(filename, 50, "slot%d.ng", pic);
 SDL_Rect src, dst;
 SDL_Surface *img = NULL;
 loadImageOrAnimation(filename, &img, &animation.animation, true);
 if (animation.animation)
 {
  IMG_SetAnimationState(&animation, -1, 0);
  animation.dst.x = config->x3d * xMult;
  animation.dst.y = config->y3d * yMult;
  animation.dst.w = p3d.config->width * xMult;
  animation.dst.h = p3d.config->height * yMult;
  screen.front()->addAnimation(&animation);
  drawAnimationFrame();
  return;
 }
 if ((pic >= 45) && (img == NULL))
 {
  snprintf(filename, 50, "PICS/SLOT%d.PIC", pic);
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
   snprintf(filename, 50, "PICS/SLOT%d.PAC", pic);
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
   if ((xMult > 1) || (yMult > 1))
   {
    SDL_Surface *img2 = simpleZoomSurface(img, xMult, yMult);
    SDL_FreeSurface(img);
    img = img2;
   }
  }
 }
 dst.x = config->x3d * xMult;
 dst.y = config->y3d * yMult;
 dst.w = p3d.config->width * xMult;
 dst.h = p3d.config->height * yMult;
 if (NULL == img)
 {
  screen.front()->fillRect(dst, black);
  return;
 }
 src.x = 0;
 src.y = 0;
 src.w = p3d.config->width * xMult;
 src.h = p3d.config->height * yMult;
 screen.front()->drawImage(img, dst);
 SDL_FreeSurface(img);
}

void BTDisplay::drawLabel(const char *value)
{
 drawLabel("main", value);
}

void BTDisplay::drawLabel(const char *name, const char *value)
{
 int w, h;
 if (!sizeFont(name, w, h))
  return;
 BTLabelWidget *widget = dynamic_cast<BTLabelWidget*>(getWidget(name));
 if (widget)
 {
  widget->setText(value);
  widget->render(getVisibleScreen());
 }
}

void BTDisplay::drawLast(const char *keys, const char *words)
{
 BTTextWidget *widget = dynamic_cast<BTTextWidget*>(getWidget("text"));
 if (widget)
  widget->drawLast(getVisibleScreen(), words);
}

void BTDisplay::drawMessage(const char *words, int *delay)
{
 addText(words);
 addText("");
 process(BTDisplay::allKeys, delay);
 clearElements();
}

void BTDisplay::drawText(const char *words, BTAlignment::alignment a /*= left*/)
{
 BTTextWidget *widget = dynamic_cast<BTTextWidget*>(getWidget("text"));
 if (widget)
  widget->drawText(getVisibleScreen(), words, a);
}

void BTDisplay::drawView()
{
 picture = -1;
 if (animation.animation)
 {
  IMG_FreeMNG(animation.animation);
  animation.animation = NULL;
  screen.front()->removeAnimation(&animation);
 }
 Psuedo3DMap *m = Psuedo3DMap::getMap();
 p3d.draw(m, m->getX(), m->getY(), m->getFacing());
 SDL_Rect dst;
 dst.x = config->x3d * xMult;
 dst.y = config->y3d * yMult;
 dst.w = p3d.config->width * xMult;
 dst.h = p3d.config->height * yMult;
 screen.front()->drawImage(p3d.getDisplay(), dst);
 if ((config->mapDisplayMode == BTMAPDISPLAYMODE_ALWAYS) ||
  ((config->mapDisplayMode == BTMAPDISPLAYMODE_NO3D) && (p3d.getConfig()->wallType.empty())))
  drawMap(false);
}

void BTDisplay::drawIcons()
{
 for (int i = 0; i < config->icon.size(); ++i)
 {
  // Should cache this to not constantly redraw.
  config->icon[i]->draw(*screen.back(), SDL_GetTicks());
 }
}

void BTDisplay::drawMap(bool knowledge)
{
 screen.front()->drawMap(knowledge);
}

void BTDisplay::drawStats()
{
 int i;
 if (config->statusInfo.size() == 0)
  return;
 status->draw(screen.front());
}

SDL_Color &BTDisplay::getBlack()
{
 return getColor("black");
}

SDL_Color &BTDisplay::getColor(const std::string &color)
{
 for (int i = 0; i < config->color.size(); ++i)
  if (config->color[i]->name == color)
   return config->color[i]->rgb;
 if (color == "white")
  return white;
 return black;
}

BTDisplayConfig *BTDisplay::getConfig()
{
 return config;
}

int BTDisplay::getCurrentImage()
{
 return picture;
}

std::string BTDisplay::getCurrentLabel()
{
 BTLabelWidget *widget = dynamic_cast<BTLabelWidget*>(getWidget("main"));
 if (widget)
 {
  return widget->getText();
 }
 return "";
}

void BTDisplay::getMultiplier(int &x, int &y)
{
 x = xMult;
 y = yMult;
}

BTBackgroundAndScreen *BTDisplay::getScreen(int i)
{
 for (std::list<BTBackgroundAndScreen *>::iterator itr(screen.begin()); itr != screen.end(); ++itr)
 {
  if (i == 0)
   return *itr;
  --i;
 }
 return NULL;
}

SDL_Color &BTDisplay::getWhite()
{
 return white;
}

BTWidget *BTDisplay::getWidget(const std::string &name)
{
 BTWidget *w = NULL;
 for (std::list<BTBackgroundAndScreen *>::iterator itr(screen.begin()); itr != screen.end(); ++itr)
 {
  w = (*itr)->getWidget(name);
  if (NULL != w)
   return w;
 }
 return w;
}

void BTDisplay::playMusic(unsigned int effectID, const char *file, bool physfs /*= true*/)
{
 if ((!music.empty()) && (music.front()->musicObj != NULL))
 {
  Mix_FadeOutMusic(1000);
 }
 if ((file == NULL) || (file[0] == 0))
 {
  music.push_front(new BTMusic(effectID));
  return;
 }
 BTMusic *m = new BTMusic(effectID);
 SDL_RWops *musicFile;
 if (physfs)
  musicFile = PHYSFSRWOPS_openRead(file);
 else
  musicFile = SDL_RWFromFile(file, "rb");
 if (musicFile)
 {
#ifdef SDL2LIB
  m->musicObj = Mix_LoadMUS_RW(musicFile, 0);
#else
  m->musicObj = Mix_LoadMUS_RW(musicFile);
#endif
  if (m->musicObj)
   Mix_FadeInMusic(m->musicObj, -1, 1000);
 }
 music.push_front(m);
}

void BTDisplay::playSound(const char *file, bool physfs /*= true*/)
{
 for (std::list<BTSound*>::iterator itr(sound.begin()); itr != sound.end(); )
 {
  // Look for finished sounds
  if (0 == Mix_Playing((*itr)->channel))
  {
   Mix_FreeChunk((*itr)->sound);
   itr = sound.erase(itr);
  }
  else
   ++itr;
 }
 if ((file == NULL) || (file[0] == 0))
 {
  return ;
 }
 SDL_RWops *soundFile;
 if (physfs)
  soundFile = PHYSFSRWOPS_openRead(file);
 else
  soundFile = SDL_RWFromFile(file, "rb");
 if (soundFile)
 {
  Mix_Chunk *soundObj = Mix_LoadWAV_RW(soundFile, 1);
  if (soundObj)
  {
   sound.push_back(new BTSound(soundObj, Mix_PlayChannel(-1, soundObj, 0)));
  }
 }
 return ;
}

unsigned int BTDisplay::process(const char *specialKeys /*= NULL*/, int *delay /*= 0*/, int delayOveride /*= -1*/)
{
 BTTextWidget *widget = dynamic_cast<BTTextWidget*>(getWidget("text"));
 if (widget)
  return widget->process(screen.back(), specialKeys, delay, delayOveride);
 return 13;
}

unsigned int BTDisplay::readChar(int delay /*= 0*/)
{
 SDL_Event sdlevent;
 SDL_TimerID timer;
 unsigned long animationDelay = 0;
 animationDelay = drawAnimationFrame();
 render();
 if ((animationDelay) && ((delay == 0) || (animationDelay < delay)))
  timer = SDL_AddTimer(animationDelay, timerCallback, NULL);
 else if (delay)
  timer = SDL_AddTimer(delay, timerCallback, NULL);
 while (true)
 {
  SDL_WaitEvent(&sdlevent);
  if (sdlevent.type == SDL_KEYDOWN)
  {
   if ((animationDelay) || (delay))
    SDL_RemoveTimer(timer);
   if ((sdlevent.key.keysym.sym == SDLK_UP) || (sdlevent.key.keysym.sym == SDLK_KP_8))
    return BTKEY_UP;
   else if ((sdlevent.key.keysym.sym == SDLK_DOWN) || (sdlevent.key.keysym.sym == SDLK_KP_2))
    return BTKEY_DOWN;
   else if ((sdlevent.key.keysym.sym == SDLK_LEFT) || (sdlevent.key.keysym.sym == SDLK_KP_4))
    return BTKEY_LEFT;
   else if ((sdlevent.key.keysym.sym == SDLK_RIGHT) || (sdlevent.key.keysym.sym == SDLK_KP_6))
    return BTKEY_RIGHT;
   else if ((sdlevent.key.keysym.sym == SDLK_PAGEDOWN) || (sdlevent.key.keysym.sym == SDLK_KP_3))
    return BTKEY_PGDN;
   else if ((sdlevent.key.keysym.sym == SDLK_PAGEUP) || (sdlevent.key.keysym.sym == SDLK_KP_9))
    return BTKEY_PGUP;
   else if ((sdlevent.key.keysym.sym == SDLK_END) || (sdlevent.key.keysym.sym == SDLK_KP_1))
    return BTKEY_END;
   else if ((sdlevent.key.keysym.sym == SDLK_INSERT) || (sdlevent.key.keysym.sym == SDLK_KP_0))
    return BTKEY_INS;
   else if ((sdlevent.key.keysym.sym == SDLK_KP_PERIOD) || (sdlevent.key.keysym.sym == SDLK_DELETE))
    return BTKEY_DEL;
   else if (sdlevent.key.keysym.sym == SDLK_F1)
    return BTKEY_F1;
   else if (sdlevent.key.keysym.sym == SDLK_F12)
    toggleFullScreen();
   else
   {
    if ((sdlevent.key.keysym.mod & KMOD_RALT) != 0)
    {
     auto itr = rightAltKey.find(sdlevent.key.keysym.sym);
     if (itr != key.end())
     {
      return itr->second;
     }
    }
    else if ((sdlevent.key.keysym.mod & (KMOD_CTRL)) != 0)
    {
     auto itr = ctrlKey.find(sdlevent.key.keysym.sym);
     if (itr != ctrlKey.end())
     {
      return itr->second;
     }
    }
    else if ((sdlevent.key.keysym.mod & (KMOD_ALT | KMOD_GUI)) != 0)
    {
    }
    else if ((((sdlevent.key.keysym.mod & KMOD_CAPS) != 0) && ((sdlevent.key.keysym.mod & KMOD_SHIFT) == 0)) || (((sdlevent.key.keysym.mod & KMOD_CAPS) == 0) && ((sdlevent.key.keysym.mod & KMOD_SHIFT) != 0)))
    {
     auto itr = shiftKey.find(sdlevent.key.keysym.sym);
     if (itr != shiftKey.end())
     {
      return itr->second;
     }
    }
    else
    {
     auto itr = key.find(sdlevent.key.keysym.sym);
     if (itr != key.end())
     {
      return itr->second;
     }
    }
   }
   if ((animationDelay) && ((delay == 0) || (animationDelay < delay)))
    timer = SDL_AddTimer(animationDelay, timerCallback, NULL);
   else if (delay)
    timer = SDL_AddTimer(delay, timerCallback, NULL);
  }
  else if (sdlevent.type == SDL_QUIT)
  {
   Mix_Quit();
   SDL_Quit();
   exit(0);
  }
  else if (sdlevent.type == SDL_USEREVENT)
  {
   if ((animationDelay) && ((delay == 0) || (animationDelay < delay)))
   {
    delay -= animationDelay;
    animationDelay = drawAnimationFrame();
    render();
    if ((animationDelay) && ((delay == 0) || (animationDelay < delay)))
     timer = SDL_AddTimer(animationDelay, timerCallback, NULL);
    else if (delay)
     timer = SDL_AddTimer(delay, timerCallback, NULL);
   }
   else
    return 0;
  }
 }
}

std::string BTDisplay::readString(const char *prompt, int max, const std::string &initial)
{
 BTTextWidget *widget = dynamic_cast<BTTextWidget*>(getWidget("text"));
 if (widget)
  return widget->readString(getVisibleScreen(), prompt, max, initial);
 return "";
}

void BTDisplay::refresh()
{
 for (std::list<BTBackgroundAndScreen*>::iterator itr = screen.begin(); itr != screen.end(); itr++)
 {
  (*itr)->clear();
 }
 drawStats();
 render();
}

void BTDisplay::removeAnimation(MNG_AnimationState *animState)
{
 for (std::list<BTBackgroundAndScreen*>::iterator itr = screen.begin(); itr != screen.end(); itr++)
 {
  (*itr)->removeAnimation(animState);
 }
}

void BTDisplay::setConfig(BTDisplayConfig *c)
{
 picture = -1;
 if (animation.animation)
 {
  IMG_FreeMNG(animation.animation);
  animation.animation = NULL;
  removeAnimation(&animation);
 }
 int newXMult = lockMult;
 int newYMult = lockMult;
 if ((newXMult == 0) || (newYMult == 0))
 {
  newXMult = (xFull - 10) / c->width; // Allow for window decoration
  newYMult = (yFull - 10) / c->height; // Allow for window decoration
  if (newXMult > newYMult)
   newXMult = newYMult;
  else
   newYMult = newXMult;
 }
 char *font = c->font;
 int fontsize = c->fontsize;
 expanded = c->findExpanded(newXMult, newYMult);
 if (expanded)
 {
  if (lockMult == 0)
  {
   // Do not expand as big as possible when not explicitly set.
   // User with multiple monitors had a problem.
   newXMult = expanded->xMult;
   newYMult = expanded->yMult;
  }
  else
  {
   newXMult = ((config->width * lockMult) / (config->width * expanded->xMult)) * expanded->xMult;
   newYMult = ((config->height * lockMult) / (config->height * expanded->yMult)) * expanded->yMult;
   if (newXMult > newYMult)
    newXMult = newYMult;
   else
    newYMult = newXMult;
  }
  font = expanded->font;
  fontsize = expanded->fontsize;
 }
 else if (lockMult == 0)
 {
  // Do not expand as big as possible when not explicitly set.
  // User with multiple monitors had a problem.
  newXMult = newYMult = 1;
 }
 p3d.setMultiplier(newXMult, newYMult);
 clearScreens();
 if ((config->width * xMult != c->width * newXMult) || (config->height * yMult != c->height * newYMult))
 {
#ifdef SDL2LIB
  SDL_FreeSurface(mainScreen);
  if (!(fullScreen && softRenderer))
  {
   SDL_DestroyTexture(mainTexture);
   SDL_DestroyRenderer(mainRenderer);
   SDL_DestroyWindow(mainWindow);
   mainWindow = SDL_CreateWindow("Bt Builder",
                             SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED,
                             ((fullScreen && softRenderer) ? xFull : c->width * newXMult),
                             ((fullScreen && softRenderer) ? yFull : c->height * newYMult),
                             (fullScreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
   if (mainWindow == NULL)
   {
    printf("Failed - SDL_CreateWindow\n");
    exit(0);
   }

   mainRenderer = SDL_CreateRenderer(mainWindow, -1, 0);
   if (mainRenderer == NULL)
   {
    printf("Failed - SDL_CreateRenderer\n");
    exit(0);
   }
   mainTexture = SDL_CreateTexture(mainRenderer,
                               SDL_PIXELFORMAT_ARGB8888,
                               SDL_TEXTUREACCESS_STREAMING,
                               ((fullScreen && softRenderer) ? xFull : c->width * newXMult),
                               ((fullScreen && softRenderer) ? yFull : c->height * newYMult));
   if (mainTexture == NULL)
   {
    printf("Failed - SDL_CreateTexture\n");
    exit(0);
   }
  }
  mainScreen = SDL_CreateRGBSurface(0, c->width * newXMult, c->height * newYMult, 32,
                                         0x00FF0000,
                                         0x0000FF00,
                                         0x000000FF,
                                         0xFF000000);
  if (mainScreen == NULL)
  {
   printf("Failed - SDL_CreateRGBSurface\n");
   exit(0);
  }
#else
  mainScreen = SDL_SetVideoMode(c->width * newXMult, c->height * newYMult, 32,
    SDL_SWSURFACE | (fullScreen ? SDL_FULLSCREEN : 0));
  if (mainScreen == NULL)
  {
   printf("Failed - SDL_SetVideoMode\n");
   exit(0);
  }
#endif
 }
 config = c;
 xMult = newXMult;
 yMult = newYMult;
#ifndef BTBUILDER_NOTTF
 if (font)
 {
  SDL_RWops *f = PHYSFSRWOPS_openRead((std::string("fonts/") + font).c_str());
  if (f)
   ttffont = TTF_OpenFontRW(f, 1, fontsize);
 }
 else if (ttffont)
 {
  TTF_CloseFont(ttffont);
  ttffont = 0;
 }
#endif
 setupScreens(config, xMult, yMult);
}

void BTDisplay::setPsuedo3DConfig(Psuedo3DConfigList *p3dl)
{
 p3dConfig = p3dl;
 p3d.setConfig(NULL);
}

Psuedo3DConfig *BTDisplay::setWallGraphics(int type)
{
 if (p3dConfig->size() > type)
  p3d.setConfig((*p3dConfig)[type]);
 else
  p3d.setConfig((*p3dConfig)[0]);
 return p3d.getConfig();
}

bool BTDisplay::sizeFont(const char *text, int &w, int &h)
{
#ifndef BTBUILDER_NOTTF
 if (ttffont)
 {
  if (0 == *text)
  {
   w = 0;
   h = TTF_FontHeight(ttffont);
  }
  else if (TTF_SizeUTF8(ttffont, text, &w, &h) == -1)
    return false;
  h = h-1;
 }
 else
#endif
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

void BTDisplay::splitText(const char *words, const std::string &prefix, std::vector<std::string> &lines)
{
 BTTextWidget *widget = dynamic_cast<BTTextWidget*>(getWidget("text"));
 SDL_Rect &text = widget->getLocation();
 int w, h, prefixW;
 char *tmp = new char[strlen(words)];
 const char *partial = words;
 sizeFont(prefix.c_str(), prefixW, h);
 while (partial)
 {
  if (!sizeFont(partial, w, h))
  {
   delete [] tmp;
   lines.push_back(std::string(partial));
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
     if (w > text.w - ((partial == words) ? 0 : prefixW))
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
  lines.push_back(((partial == words) ? std::string() : prefix) + std::string((end ? tmp : partial)));
  partial = end;
 }
 delete [] tmp;
}

void BTDisplay::stopMusic(int id)
{
 if ((!music.empty()) && (music.front()->musicObj != NULL) && ((music.front()->musicId == id) || (id == BTMUSICID_ALL)))
 {
  Mix_FadeOutMusic(1000);
 }
 if (id == BTMUSICID_ALL)
 {
  while (!music.empty())
  {
   BTMusic *m = music.front();
   music.pop_front();
   if (m)
    delete m;
  }
 }
 else
 {
  if ((!music.empty()) && (music.front()->musicId == id))
  {
   BTMusic *m = music.front();
   music.pop_front();
   if (m)
    delete m;
   if ((!music.empty()) && (music.front()->musicObj != NULL))
   {
     Mix_FadeInMusic(music.front()->musicObj, -1, 1000);
   }
  }
  else
  {
   for (std::list<BTMusic*>::iterator itr(music.begin()); itr != music.end(); ++itr)
   {
    if ((*itr)->musicId == id)
    {
     delete (*itr);
     music.remove(*itr);
     return;
    }
   }
  }
 }
}

void BTDisplay::toggleFullScreen()
{
 fullScreen = !fullScreen;
 SDL_Rect src, dst;
 dst.x = config->xMap * xMult;
 dst.y = config->yMap * yMult;
 src.w = dst.w = config->width * xMult;
 src.h = dst.h = config->height * yMult;
 src.x = dst.x = 0;
 src.y = dst.y = 0;
#ifdef SDL2LIB
 SDL_DestroyTexture(mainTexture);
 SDL_DestroyRenderer(mainRenderer);
 SDL_DestroyWindow(mainWindow);
 mainWindow = SDL_CreateWindow("Bt Builder",
                           SDL_WINDOWPOS_UNDEFINED,
                           SDL_WINDOWPOS_UNDEFINED,
                           ((fullScreen && softRenderer) ? xFull : config->width * xMult),
                           ((fullScreen && softRenderer) ? yFull : config->height * yMult),
                           (fullScreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
 if (mainWindow == NULL)
 {
  printf("Failed - SDL_CreateWindow\n");
  exit(0);
 }

 mainRenderer = SDL_CreateRenderer(mainWindow, -1, 0);
 if (mainRenderer == NULL)
 {
  printf("Failed - SDL_CreateRenderer\n");
  exit(0);
 }
 mainTexture = SDL_CreateTexture(mainRenderer,
                             SDL_PIXELFORMAT_ARGB8888,
                             SDL_TEXTUREACCESS_STREAMING,
                             ((fullScreen && softRenderer) ? xFull : config->width * xMult),
                             ((fullScreen && softRenderer) ? yFull : config->height * yMult));
 if (mainTexture == NULL)
 {
  printf("Failed - SDL_CreateTexture\n");
  exit(0);
 }
#else
 mainScreen = SDL_SetVideoMode(config->width * xMult, config->height * yMult, 32,
   SDL_SWSURFACE | (fullScreen ? SDL_FULLSCREEN : 0));
 if (mainScreen == NULL)
 {
  printf("Failed - SDL_SetVideoMode\n");
  exit(0);
 }
#endif
 render();
}

void BTDisplay::drawFont(const char *text, SDL_Rect &dst, SDL_Color c, BTAlignment::alignment a, SDL_Surface *screen /*= NULL*/)
{
 if (0 == *text)
  return;
 SDL_Surface *img;
#ifndef BTBUILDER_NOTTF
 if (ttffont)
  img = TTF_RenderUTF8_Solid(ttffont, text, c);
 else
#endif
  img = simpleRender_Solid(sfont, text, c);
 if ((xMult != yMult) || ((ttffont == NULL) && ((xMult > 1) || (yMult > 1))))
 {
  SDL_Surface *img2 = simpleZoomSurface(img, xMult, yMult);
  SDL_FreeSurface(img);
  img = img2;
 }
 SDL_Rect src, f;
 src.x = 0;
 src.y = 0;
 src.w = ((img->w > dst.w) ? dst.w : img->w);
 src.h = ((img->h > dst.h) ? dst.h : img->h);
 switch (a)
 {
  case BTAlignment::left:
   f.x = dst.x;
   f.y = dst.y;
   f.w = src.w;
   f.h = src.h;
   break;
  case BTAlignment::center:
   f.x = ((img->w > dst.w) ? dst.x : dst.x + (dst.w / 2) - (img->w / 2));
   f.y = ((img->h > dst.h) ? dst.y : dst.y + (dst.h / 2) - (img->h / 2));
   f.w = src.w;
   f.h = src.h;
   break;
  case BTAlignment::right:
   f.x = dst.x + dst.w - src.w;
   f.y = dst.y + dst.h - src.h;
   f.w = src.w;
   f.h = src.h;
   break;
 }
 if (screen)
  SDL_BlitSurface(img, &src, screen, &f);
 else
  SDL_BlitSurface(img, &src, mainScreen, &f);
 SDL_FreeSurface(img);
}

void BTDisplay::drawImage(SDL_Rect &dst, SDL_Surface *img)
{
 SDL_Rect src;
 src.x = 0;
 src.y = 0;
 src.w = dst.w;
 src.h = dst.h;
 simpleBlitSurface(img, &src, mainScreen, &dst);
}

void BTDisplay::fillRect(SDL_Surface *scr, SDL_Rect &dst, SDL_Color c)
{
 if (scr)
  SDL_FillRect(scr, &dst, SDL_MapRGB(scr->format, c.r, c.g, c.b));
 else
  SDL_FillRect(mainScreen, &dst, SDL_MapRGB(mainScreen->format, c.r, c.g, c.b));
}

SDL_Surface *BTDisplay::grabImage(SDL_Surface *scr, SDL_Rect &src)
{
 if (scr == NULL)
  scr = mainScreen;
 SDL_Rect dst;
 dst.x = 0;
 dst.y = 0;
 dst.w = src.w;
 dst.h = src.h;
 SDL_Surface *backup = SDL_CreateRGBSurface(SDL_SWSURFACE, src.w, src.h, 32, scr->format->Rmask, scr->format->Gmask, scr->format->Bmask, scr->format->Amask);
 SDL_BlitSurface(scr, &src, backup, &dst);
 return backup;
}

void BTDisplay::moveImage(SDL_Rect &src, SDL_Rect &dst)
{
  SDL_BlitSurface(mainScreen, &src, mainScreen, &dst);
 }

void BTDisplay::loadImageOrAnimation(const char *file, SDL_Surface **img, MNG_Image **animation, bool imageWindow, bool physfs /*= true*/)
{
 if (expanded)
 {
  std::string filename = "image/";
  if ((imageWindow) && ("" != expanded->imageDirectory))
  {
   filename += expanded->imageDirectory;
  }
  else
  {
   filename += expanded->directory;
  }
  filename += "/";
  filename += file;
  bool exists = false;
  if (physfs)
   exists = PHYSFS_exists(filename.c_str());
  else
   exists = boost::filesystem::exists(filename.c_str());
  if (exists)
  {
   SDL_RWops *f = NULL;
   if (physfs)
    f = PHYSFSRWOPS_openRead(filename.c_str());
   else
    f = SDL_RWFromFile(filename.c_str(), "rb");
   if (IMG_isMNG(f))
   {
    if (animation)
    {
     *animation = IMG_LoadMNG_RW(f);
     if (*animation)
     {
      if (((xMult / expanded->xMult) > 1) || ((yMult / expanded->yMult) > 1))
      {
       simpleZoomAnimation(*animation, xMult / expanded->xMult, yMult / expanded->yMult);
      }
      return;
     }
    }
    else
     SDL_RWclose(f);
   }
   else
   {
    *img = IMG_Load_RW(f, 1);
    if (((xMult / expanded->xMult) > 1) || ((yMult / expanded->yMult) > 1))
    {
     SDL_Surface *img2 = simpleZoomSurface(*img, xMult / expanded->xMult, yMult / expanded->yMult);
     SDL_FreeSurface(*img);
     *img = img2;
    }
    return;
   }
  }
 }
 std::string filename = "image/";
 if ((imageWindow) && ("" != p3d.config->directory))
 {
  filename += p3d.config->directory;
  filename += "/";
 }
 filename += file;
 bool exists = false;
 if (physfs)
  exists = PHYSFS_exists(filename.c_str());
 else
  exists = boost::filesystem::exists(filename.c_str());
 if (exists)
 {
  SDL_RWops *f = NULL;
  if (physfs)
   f = PHYSFSRWOPS_openRead(filename.c_str());
  else
   f = SDL_RWFromFile(filename.c_str(), "rb");
  if (IMG_isMNG(f))
  {
   if (animation)
   {
    *animation = IMG_LoadMNG_RW(f);
    if (*animation)
    {
     if ((xMult > 1) || (yMult > 1))
     {
      simpleZoomAnimation(*animation, xMult, yMult);
     }
    }
   }
   else
    SDL_RWclose(f);
  }
  else
  {
   *img = IMG_Load_RW(f, 1);
   if ((xMult > 1) || (yMult > 1))
   {
    SDL_Surface *img2 = simpleZoomSurface(*img, xMult, yMult);
    SDL_FreeSurface(*img);
    *img = img2;
   }
  }
 }
}

void BTDisplay::render()
{
 for (std::list<BTBackgroundAndScreen*>::iterator itr = screen.begin(); itr != screen.end(); ++itr)
 {
  (*itr)->render();
 }
#ifdef SDL2LIB
 if (fullScreen && softRenderer)
 {
  void *pixels;
  int pitch;
  SDL_Rect dest;
  dest.x = 0;
  dest.y = 0;
  dest.w = xFull;
  dest.h = yFull;
  SDL_LockTexture(mainTexture, &dest, &pixels, &pitch);
  double scaleWidth =  ((double)(config->width * xMult)) / (double)xFull;
  double scaleHeight = ((double)(config->height * yMult)) / (double)yFull;
  double curX = 0;
  double curY = 0;
  Uint8 *realLine;
  Uint8 *realPos;
  int cy;
  int cx;

  SDL_LockSurface(mainScreen);
  realPos = (Uint8 *)pixels;
  Uint8 *pixelSource = (Uint8*)mainScreen->pixels;
  for (cy = 0; cy < yFull; cy++, curY += scaleHeight)
  {
   curX = 0;
   realLine = realPos;
   for (cx = 0; cx < xFull; cx++, curX += scaleWidth)
   {
    int pos = ((int)curY) * mainScreen->pitch + (((int)curX) * 4);
    realPos[0] = pixelSource[pos];
    realPos[1] = pixelSource[pos + 1];
    realPos[2] = pixelSource[pos + 2];
    realPos[3] = pixelSource[pos + 3];
    realPos += 4;
   }
   realPos = realLine + pitch;
  }
  SDL_UnlockSurface(mainScreen);
  SDL_UnlockTexture(mainTexture);
 }
 else
  SDL_UpdateTexture(mainTexture, NULL, mainScreen->pixels, mainScreen->pitch);
 SDL_RenderClear(mainRenderer);
 SDL_RenderCopy(mainRenderer, mainTexture, NULL, NULL);
 SDL_RenderPresent(mainRenderer);
#else
 SDL_UpdateRect(mainScreen, 0, 0, 0, 0);
#endif
}

std::string BTDisplay::applyDisplayDir(const std::string &filename)
{
 std::string answer;
 if (0 == strncmp("data/", filename.c_str(), 5))
 {
  answer = "data/";
  answer += displayDir;
  answer += (filename.c_str() + 5);
 }
 else
 {
  answer = displayDir + filename;
 }
 return answer;
}

void BTDisplay::clearScreens()
{
 for (std::list<BTBackgroundAndScreen*>::iterator itr = screen.begin(); itr != screen.end(); ++itr)
 {
  delete *itr;
 }
 screen.clear();
}

unsigned long BTDisplay::drawAnimationFrame()
{
 unsigned long ticks = SDL_GetTicks();
 unsigned long next = 0;
 for (std::list<BTBackgroundAndScreen*>::iterator itr = screen.begin(); itr != screen.end(); itr++)
 {
  unsigned long possible = (*itr)->drawAnimationFrame(ticks);
  if ((next == 0) || (possible < next))
   next = possible;
 }
 return next;
}

BTBackgroundAndScreen *BTDisplay::getVisibleScreen()
{
 BTBackgroundAndScreen *answer = NULL;
 for (std::list<BTBackgroundAndScreen *>::iterator itr = screen.begin(); itr != screen.end(); ++itr)
 {
  if ((*itr)->isVisable())
   answer = *itr;
 }
 return answer;
}

void BTDisplay::setupKeyMap()
{
 key.insert(std::pair<SDL_Keycode, char>(SDLK_BACKSPACE, '\b'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_TAB, '\t'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_RETURN, '\r'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_ESCAPE, '\033'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_SPACE, ' '));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_EXCLAIM, '!'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_QUOTEDBL, '"'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_HASH, '#'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_DOLLAR, '$'));
#ifdef SDL2LIB
 key.insert(std::pair<SDL_Keycode, char>(SDLK_PERCENT, '%'));
#endif
 key.insert(std::pair<SDL_Keycode, char>(SDLK_AMPERSAND, '&'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_QUOTE, '\''));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_LEFTPAREN, '('));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_RIGHTPAREN, ')'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_ASTERISK, '*'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_PLUS, '+'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_COMMA, ','));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_MINUS, '-'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_PERIOD, '.'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_SLASH, '/'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_0, '0'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_1, '1'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_2, '2'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_3, '3'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_4, '4'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_5, '5'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_6, '6'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_7, '7'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_8, '8'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_9, '9'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_COLON, ':'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_SEMICOLON, ';'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_LESS, '<'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_EQUALS, '='));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_GREATER, '>'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_QUESTION, '?'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_AT, '@'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_LEFTBRACKET, '['));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_BACKSLASH, '\\'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_RIGHTBRACKET, ']'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_CARET, '^'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_UNDERSCORE, '_'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_BACKQUOTE, '`'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_a, 'a'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_b, 'b'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_c, 'c'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_d, 'd'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_e, 'e'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_f, 'f'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_g, 'g'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_h, 'h'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_i, 'i'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_j, 'j'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_k, 'k'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_l, 'l'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_m, 'm'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_n, 'n'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_o, 'o'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_p, 'p'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_q, 'q'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_r, 'r'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_s, 's'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_t, 't'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_u, 'u'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_v, 'v'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_w, 'w'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_x, 'x'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_y, 'y'));
 key.insert(std::pair<SDL_Keycode, char>(SDLK_z, 'z'));

 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_BACKSPACE, '\b'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_TAB, '\t'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_RETURN, '\r'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_ESCAPE, '\033'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_SPACE, ' '));
// shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_EXCLAIM, '!'));
// shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_QUOTEDBL, '"'));
// shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_HASH, '#'));
// shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_DOLLAR, '$'));
// shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_PERCENT, '%'));
// shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_AMPERSAND, '&'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_QUOTE, '"'));
// shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_LEFTPAREN, '('));
// shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_RIGHTPAREN, ')'));
// shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_ASTERISK, '*'));
// shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_PLUS, '+'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_COMMA, '<'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_MINUS, '_'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_PERIOD, '>'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_SLASH, '?'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_0, ')'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_1, '!'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_2, '@'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_3, '#'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_4, '$'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_5, '%'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_6, '^'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_7, '&'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_8, '*'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_9, '('));
// shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_COLON, ':'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_SEMICOLON, ':'));
// shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_LESS, '<'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_EQUALS, '+'));
// shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_GREATER, '>'));
// shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_QUESTION, '?'));
// shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_AT, '@'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_LEFTBRACKET, '{'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_BACKSLASH, '|'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_RIGHTBRACKET, '}'));
// shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_CARET, '^'));
// shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_UNDERSCORE, '_'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_BACKQUOTE, '~'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_a, 'A'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_b, 'B'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_c, 'C'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_d, 'D'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_e, 'E'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_f, 'F'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_g, 'G'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_h, 'H'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_i, 'I'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_j, 'J'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_k, 'K'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_l, 'L'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_m, 'M'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_n, 'N'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_o, 'O'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_p, 'P'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_q, 'Q'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_r, 'R'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_s, 'S'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_t, 'T'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_u, 'U'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_v, 'V'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_w, 'W'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_x, 'X'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_y, 'Y'));
 shiftKey.insert(std::pair<SDL_Keycode, char>(SDLK_z, 'Z'));

 rightAltKey.insert(std::pair<SDL_Keycode, char>(SDLK_n, '~'));

 ctrlKey.insert(std::pair<SDL_Keycode, char>(SDLK_c, BTKEY_CTRL_C));
 ctrlKey.insert(std::pair<SDL_Keycode, char>(SDLK_v, BTKEY_CTRL_V));
 ctrlKey.insert(std::pair<SDL_Keycode, char>(SDLK_x, BTKEY_CTRL_X));
}

void BTDisplay::setupScreens(BTDisplayConfig *c, int xMult, int yMult)
{
 for (int w = 0; w < c->layout.size(); ++w)
 {
  SDL_Surface *scr = NULL;
  if (c->layout.size() > 1)
  {
   scr = SDL_CreateRGBSurface(0, c->width * xMult, c->height * yMult, 32,
                              0x00FF0000,
                              0x0000FF00,
                              0x000000FF,
                              0xFF000000);
  }
  screen.push_back(new BTBackgroundAndScreen(this, scr, c->layout[w]->visible));
  screen.back()->setBackground(c->layout[w]->background.c_str(), true);
  for (int i = 0; i < c->layout[w]->widgets.size(); ++i)
  {
   if (dynamic_cast<BTLabelConfig*>(c->layout[w]->widgets[i]))
   {
    screen.back()->addWidget(new BTLabelWidget(dynamic_cast<BTLabelConfig*>(c->layout[w]->widgets[i]), xMult, yMult));
   }
   else if (dynamic_cast<BTTextConfig*>(c->layout[w]->widgets[i]))
   {
    screen.back()->addWidget(new BTTextWidget(dynamic_cast<BTTextConfig*>(c->layout[w]->widgets[i]), xMult, yMult));
   }
  }
 }
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

BTBackgroundAndScreen::BTBackgroundAndScreen(BTDisplay *d, SDL_Surface *s, bool v)
 : display(d), screen(s), background(NULL), visible(v)
{
}

BTBackgroundAndScreen::~BTBackgroundAndScreen()
{
 clearWidgets();
 if (screen)
 {
  SDL_FreeSurface(screen);
 }
 if (background)
 {
  SDL_FreeSurface(background);
 }
}

void BTBackgroundAndScreen::addAnimation(MNG_AnimationState *animState, bool clear /*= false*/)
{
 activeAnimation.push_back(BTAnimation(animState, clear));
}

void BTBackgroundAndScreen::addWidget(BTWidget *w)
{
 widgets.push_back(w);
}

void BTBackgroundAndScreen::clear()
{
 SDL_Rect r;
 r.x = 0;
 r.y = 0;
 r.h = background->h;
 r.w = background->w;

 clear(r);
}

void BTBackgroundAndScreen::clear(SDL_Rect &r)
{
 if (screen)
  simpleBlitSurface(background, &r, screen, &r);
 else
  display->clear(background, r);
}

void BTBackgroundAndScreen::clearWidgets()
{
 for (std::vector<BTWidget*>::iterator widgetItr = widgets.begin(); widgets.end() != widgetItr; ++widgetItr)
 {
  delete (*widgetItr);
 }
 widgets.clear();
}

SDL_Color &BTBackgroundAndScreen::getColor(const std::string &color)
{
 return display->getColor(color);
}

unsigned long BTBackgroundAndScreen::drawAnimationFrame(long ticks)
{
 unsigned long next = 0;
 for (std::list<BTAnimation>::iterator itr = activeAnimation.begin(); itr != activeAnimation.end(); itr++)
 {
  SDL_Surface *nextImg = IMG_TimeUpdate(itr->animation, ticks);
  if (nextImg)
  {
   if (itr->clear)
    clear(itr->animation->dst);
   SDL_Rect src;
   src.x = 0;
   src.y = 0;
   src.w = itr->animation->dst.w;
   src.h = itr->animation->dst.h;
   if (screen)
    SDL_BlitSurface(nextImg, &src, screen, &itr->animation->dst);
   else
    display->drawImage(itr->animation->dst, nextImg);
  }
  unsigned long possible = IMG_TimeToNextFrame(itr->animation, ticks);
  if ((next == 0) || (possible < next))
   next = possible;
 }
 return next;
}

void BTBackgroundAndScreen::drawFont(const char *text, SDL_Rect &dst, SDL_Color c, BTAlignment::alignment a)
{
 display->drawFont(text, dst, c, a, screen);
}

void BTBackgroundAndScreen::drawImage(SDL_Surface *img, SDL_Rect &dst)
{
 SDL_Rect src;
 src.x = 0;
 src.y = 0;
 src.w = dst.w;
 src.h = dst.h;
 if (screen)
  SDL_BlitSurface(img, &src, screen, &dst);
 else
  display->drawImage(dst, img);
}

void BTBackgroundAndScreen::drawMap(bool knowledge)
{
 // Draw black
 Psuedo3DMap *m = Psuedo3DMap::getMap();
 Psuedo3D &p3d = display->getPsuedo3D();
 int xMult, yMult;
 BTDisplayConfig *config = display->getConfig();
 display->getMultiplier(xMult, yMult);
 SDL_Rect src, dst;
 dst.x = config->xMap * xMult;
 dst.y = config->yMap * yMult;
 src.w = dst.w = config->widthMap * p3d.config->mapWidth * xMult;
 src.h = dst.h = config->heightMap * p3d.config->mapHeight * yMult;
 src.x = 0;
 src.y = 0;
 SDL_Surface *backup = NULL;
 if (config->mapDisplayMode == BTMAPDISPLAYMODE_REQUEST)
 {
  backup = display->grabImage(screen, dst);
 }
 display->fillRect(screen, dst, display->getBlack());
 if (config->centerMap)
 {
  display->mapXStart = m->getX() - (config->widthMap / 2);
  display->mapYStart = m->getY() - (config->heightMap / 2);
 }
 else
 {
  int curX = m->getX();
  if (display->mapXStart > curX)
   display->mapXStart = curX;
  if (display->mapXStart + config->widthMap <= curX)
   display->mapXStart = curX - config->widthMap + 1;
  int curY = m->getY();
  if (display->mapYStart > curY)
   display->mapYStart = curY;
  if (display->mapYStart + config->heightMap <= curY)
   display->mapYStart = curY - config->heightMap + 1;
 }
 if (config->coordinatesMap)
 {
  for (int k = 0; k < config->heightMap; ++k)
  {
   char sz[3] = {0, 0, 0};
   int coordinate = m->getYSize() - (k + display->mapYStart) - 1;
   if (coordinate < 100)
   {
    if (coordinate < 0)
    {
     sz[0] = 0;
     sz[1] = 0;
    }
    else if (coordinate < 10)
    {
     sz[0] = '0' + (coordinate);
     sz[1] = 0;
    }
    else
    {
     sz[0] = '0' + ((coordinate) / 10);
     sz[1] = '0' + ((coordinate) % 10);
     sz[2] = 0;
    }
   }
   SDL_Rect dst;
   dst.x = (config->xMap - 2 - (2 * p3d.config->mapWidth)) * xMult;
   dst.y = (config->yMap + (k * p3d.config->mapHeight)) * yMult;
   dst.w = 2 * p3d.config->mapWidth * xMult;
   dst.h = p3d.config->mapHeight * yMult;
   clear(dst);
   drawFont(sz, dst, display->getBlack(), BTAlignment::right);
  }
  for (int i = 0; i < config->widthMap; ++i)
  {
   char sz[2] = {0, 0};
   int coordinate = i + display->mapXStart;
   if (coordinate < 100)
   {
    if (coordinate < 10)
    {
     sz[0] = '0' + (coordinate);
     sz[1] = 0;
    }
    else
    {
     sz[0] = '0' + ((coordinate) / 10);
     sz[1] = 0;
    }
   }
   SDL_Rect dst;
   dst.x = (config->xMap + (i * p3d.config->mapWidth)) * xMult;
   dst.y = (config->yMap + (config->heightMap * p3d.config->mapHeight) + 2) * yMult;
   dst.w = p3d.config->mapWidth * xMult;
   dst.h = p3d.config->mapHeight * yMult;
   clear(dst);
   drawFont(sz, dst, display->getBlack(), BTAlignment::right);
   sz[0] = 0;
   if (coordinate < 100)
   {
    if (coordinate >= 10)
    {
     sz[0] = '0' + ((coordinate) % 10);
     sz[1] = 0;
    }
   }
   dst.y = (config->yMap + ((config->heightMap + 1) * p3d.config->mapHeight) + 2) * yMult;
   clear(dst);
   if (sz[0])
    drawFont(sz, dst, display->getBlack(), BTAlignment::right);
  }
 }
 for (int i = 0; i < config->widthMap; ++i)
 {
  for (int k = 0; k < config->heightMap; ++k)
  {
   dst.x = (config->xMap + (i * p3d.config->mapWidth)) * xMult;
   dst.y = (config->yMap + (k * p3d.config->mapHeight)) * yMult;
   dst.w = p3d.config->mapWidth * xMult;
   dst.h = p3d.config->mapHeight * yMult;
   int know = m->getKnowledge(display->mapXStart + i, display->mapYStart + k);
   if ((display->mapXStart + i < 0) || (display->mapYStart + k < 0) || (display->mapXStart + i >= m->getXSize()) || (display->mapYStart + k >= m->getYSize()) || ((!knowledge) && (know == BTKNOWLEDGE_NO)))
   {
    SDL_Surface *unknown = p3d.getMapUnknown();
    if (unknown)
    {
     drawImage(unknown, dst);
    }
   }
   else
   {
    for (int direction = 0; direction < CARDINAL_DIRECTIONS; ++direction)
    {
     SDL_Surface *mapWall = p3d.getMapWall(m->getMapType(display->mapXStart + i, display->mapYStart + k, direction), direction, knowledge || (know == BTKNOWLEDGE_FULL));
     if (mapWall)
     {
      drawImage(mapWall, dst);
     }
    }
    if (m->hasSpecial(display->mapXStart + i, display->mapYStart + k))
    {
     SDL_Surface *special = p3d.getMapSpecial();
     if (special)
     {
      drawImage(special, dst);
     }
    }
    if ((display->mapYStart + k == m->getY()) && (display->mapXStart + i == m->getX()))
    {
     SDL_Surface *arrow = p3d.getMapArrow(m->getFacing());
     if (arrow)
     {
      drawImage(arrow, dst);
     }
    }
   }
  }
 }
 dst.x = config->xMap * xMult;
 dst.y = config->yMap * yMult;
 src.w = dst.w = config->widthMap * p3d.config->mapWidth * xMult;
 src.h = dst.h = config->heightMap * p3d.config->mapHeight * yMult;
 src.x = 0;
 src.y = 0;
 if (config->mapDisplayMode == BTMAPDISPLAYMODE_REQUEST)
 {
  display->render();
  unsigned char response = display->readChar();
  drawImage(backup, dst);
  SDL_FreeSurface(backup);
 }
}

void BTBackgroundAndScreen::dropScreen()
{
 if (screen)
 {
  SDL_FreeSurface(screen);
  screen = NULL;
 }
}

void BTBackgroundAndScreen::dupeScreen(SDL_Surface *scr)
{
 if (screen == NULL)
 {
  screen = SDL_CreateRGBSurface(SDL_SWSURFACE, scr->w, scr->h, 32, scr->format->Rmask, scr->format->Gmask, scr->format->Bmask, scr->format->Amask);
  if (screen == NULL)
  {
   printf("Failed - SDL_CreateRGBSurface\n");
   exit(0);
  }
  SDL_BlitSurface(scr, NULL, screen, NULL);
 }
}

void BTBackgroundAndScreen::fillRect(SDL_Rect &dst, SDL_Color c)
{
 display->fillRect(screen, dst, c);
}

BTWidget *BTBackgroundAndScreen::getWidget(const std::string &name)
{
 for (std::vector<BTWidget*>::iterator itr(widgets.begin()); itr != widgets.end(); ++itr)
 {
  if (name == (*itr)->getName())
   return *itr;
 }
 return NULL;
}

void BTBackgroundAndScreen::removeAnimation(MNG_AnimationState *animState)
{
 activeAnimation.remove(BTAnimation(animState, false));
}

void BTBackgroundAndScreen::render()
{
 if (!visible)
  return;
 for (std::vector<BTWidget*>::iterator itr(widgets.begin()); itr != widgets.end(); ++itr)
 {
  (*itr)->render(this);
 }
 if ((screen) && (visible))
 {
  SDL_Rect r;
  r.x = 0;
  r.y = 0;
  r.h = screen->h;
  r.w = screen->w;
  display->drawImage(r, screen);
 }
}

void BTBackgroundAndScreen::scrollUp(const SDL_Rect &text, int h)
{
 SDL_Rect src, dst;
 src.x = dst.x = text.x;
 src.y = text.y + h;
 src.w = dst.w = text.w;
 src.h = dst.h = text.h - h;
 dst.y = text.y;
 if (screen)
  SDL_BlitSurface(screen, &src, screen, &dst);
 else
  display->moveImage(src, dst);
 src.x = text.x;
 src.y = text.y + text.h - h;
 src.w = text.w;
 src.h = h;
 clear(src);
}

void BTBackgroundAndScreen::setBackground(const char *file, bool physfs /*= true*/)
{
 if (background)
 {
  SDL_FreeSurface(background);
  background = NULL;
 }
 display->loadImageOrAnimation(file, &background, NULL, false, physfs);
#ifdef SDLLIB
 if (background->format->BitsPerPixel != 32)
 {
  SDL_Surface *src32;
  Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  rmask = 0xff000000;
  gmask = 0x00ff0000;
  bmask = 0x0000ff00;
  amask = 0x000000ff;
#else
  rmask = 0x000000ff;
  gmask = 0x0000ff00;
  bmask = 0x00ff0000;
  amask = 0xff000000;
#endif
  src32 = SDL_CreateRGBSurface(SDL_SWSURFACE, background->w, background->h, 32, rmask, gmask, bmask, amask);
  SDL_BlitSurface(background, NULL, src32, NULL);
  SDL_FreeSurface(background);
  background = src32;
 }
#endif
 clear();
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

void BTUISelect::draw(BTBackgroundAndScreen *d)
{
 int wFirst, wValue, h, hTmp, lines;
 char tmp[20];
 SDL_Rect dst;
 d->getDisplay()->sizeFont("", wFirst, h);
 lines = position.h / h;
 if (select >= start + lines)
  start = select - lines - 1;
 d->clear(position);
 dst.y = position.y;
 dst.h = h;
 if (numbered)
 {
  d->getDisplay()->sizeFont("1) ", wFirst, hTmp);
  int i = 0;
  for (; i < size; ++i)
  {
   dst.x = position.x;
   dst.w = wFirst;
   tmp[0] = '1' + i;
   tmp[1] = ')';
   tmp[2] = list[i].first;
   tmp[3] = 0;
   d->drawFont(tmp, dst, d->getDisplay()->getBlack(), BTAlignment::left);
   dst.x += wFirst;
   dst.w = position.w - wFirst;
   if (!list[i].name.empty())
    d->drawFont(list[i].name.c_str(), dst, d->getDisplay()->getBlack(), BTAlignment::left);
   dst.y += h;
  }
  for (; i < numbered; ++i)
  {
   dst.x = position.x;
   dst.w = wFirst;
   tmp[0] = '1' + i;
   tmp[1] = ')';
   tmp[2] = 0;
   d->drawFont(tmp, dst, d->getDisplay()->getBlack(), BTAlignment::left);
   dst.y += h;
  }
 }
 else if (size > 0)
 {
  d->getDisplay()->sizeFont("@", wFirst, hTmp);
  for (int i = start; i < start + lines; ++i)
  {
   if (i >= size)
    break;
   if (select == i)
   {
    dst.x = position.x;
    dst.w = position.w;
    d->fillRect(dst, d->getDisplay()->getBlack());
   }
   dst.x = position.x;
   dst.w = wFirst;
   tmp[0] = list[i].first;
   tmp[1] = 0;
   d->drawFont(tmp, dst, ((select != i) ? d->getDisplay()->getBlack() : d->getDisplay()->getWhite()), BTAlignment::left);
   if (((list[i].value) && (list[i].flags.isSet(BTSELECTFLAG_SHOWVALUE))) || (list[i].flags.isSet(BTSELECTFLAG_NUMBER)))
   {
    snprintf(tmp, 20, "%d", list[i].value);
    d->getDisplay()->sizeFont(tmp, wValue, hTmp);
    dst.w = position.w;
    d->drawFont(tmp, dst, ((select != i) ? d->getDisplay()->getBlack() : d->getDisplay()->getWhite()), BTAlignment::right);
   }
   else
    wValue = 0;
   dst.x += wFirst;
   dst.w = position.w - wValue - wFirst;
   if (!list[i].name.empty())
    d->drawFont(list[i].name.c_str(), dst, ((select != i) ? d->getDisplay()->getBlack() : d->getDisplay()->getWhite()), BTAlignment::left);
   dst.y += h;
  }
 }
}

void BTUISelect::alter(BTDisplay::selectItem *l, int sz)
{
 list = l;
 size = sz;
}

void BTUISelect::decrement(BTDisplay &d)
{
 if (list[select].flags.isSet(BTSELECTFLAG_NUMBER))
 {
  list[select].value--;
 }
}

void BTUISelect::increment(BTDisplay &d)
{
 if (list[select].flags.isSet(BTSELECTFLAG_NUMBER))
 {
  list[select].value++;
 }
}

void BTUISelect::moveDown(BTDisplay &d)
{
 int wFirst, h, lines;
 d.sizeFont("", wFirst, h);
 lines = position.h / h;
 if (select + 1 < size)
 {
  ++select;
  while ((select + 1 < size) && (list[select].flags.isSet(BTSELECTFLAG_UNSELECTABLE)))
   ++select;
  if (start + lines <= select)
   start = select - lines + 1;
 }
}

void BTUISelect::moveUp(BTDisplay &d)
{
 if (select > 0)
 {
  --select;
  while ((select > 0) && (list[select].flags.isSet(BTSELECTFLAG_UNSELECTABLE)))
   --select;
  if (start > select)
   start = select;
 }
}

void BTUISelect::pageDown(BTDisplay &d)
{
 int wFirst, h, lines;
 d.sizeFont("", wFirst, h);
 lines = position.h / h;
 select += lines;
 if (select >= size)
  select = size - 1;
 start += lines;
 if (start + lines >= size)
  start = size - lines;
 if (start < 0)
  start = 0;
 if (start + (lines / 2) > select)
 {
  while ((select > 0) && (list[select].flags.isSet(BTSELECTFLAG_UNSELECTABLE)))
   --select;
 }
 else
 {
  while ((select + 1 < size) && (list[select].flags.isSet(BTSELECTFLAG_UNSELECTABLE)))
   ++select;
 }
}

void BTUISelect::pageUp(BTDisplay &d)
{
 int wFirst, h, lines;
 d.sizeFont("", wFirst, h);
 lines = position.h / h;
 if (select - lines < 0)
  select = 0;
 else
  select -= lines;
 if (start - lines < 0)
  start = 0;
 else
  start -= lines;
 if (start + (lines / 2) > select)
 {
  while ((select > 0) && (list[select].flags.isSet(BTSELECTFLAG_UNSELECTABLE)))
   --select;
 }
 else
 {
  while ((select + 1 < size) && (list[select].flags.isSet(BTSELECTFLAG_UNSELECTABLE)))
   ++select;
 }
}

void BTUISelect::sanitize(BTDisplay &d)
{
 int wFirst, h, lines;
 d.sizeFont("", wFirst, h);
 lines = position.h / h;
 if (start > select)
  start = select;
 else if (start + lines <= select)
 {
  start = select - (lines - 1);
 }
}

void BTUIMultiColumn::draw(int x, int y, int w, int h, BTDisplay& d)
{
 SDL_Rect dst;
 dst.x = x;
 dst.y = y;
 dst.w = w / col.size();
 dst.h = h;
 for (std::list<std::string>::iterator itr = col.begin(); itr != col.end(); ++itr)
 {
  d.drawFont(itr->c_str(), dst, d.getBlack(), BTAlignment::left);
  dst.x += w / col.size();
 }
}

int BTUIMultiColumn::maxHeight(BTDisplay &d)
{
 int w, h;
 int hFinal = 0;
 for (std::list<std::string>::iterator itr = col.begin(); itr != col.end(); ++itr)
 {
  if ((d.sizeFont(itr->c_str(), w, h)) && (h > hFinal))
   hFinal = h;
 }
 return hFinal;
}
