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
#include "boost/filesystem/operations.hpp"

const char *BTDisplay::allKeys = "allKeys";

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

BTDisplay::BTDisplay(BTDisplayConfig *c, bool physfs /*= true*/, int multiplier /*= 0*/)
 : fullScreen(false), config(c), xMult(multiplier), yMult(multiplier), lockMult(multiplier), status(*this), textPos(0), p3d(this, 0, 0), mainScreen(0), mainBackground(0), picture(-1), ttffont(0), sfont(&simple8x8), mapXStart(0), mapYStart(0)
{
 animation.animation = 0;
 animation.frame = 0;
 if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) < 0)
 {
  printf("Failed - SDL_Init\n");
  exit(0);
 }
 Mix_Init(MIX_INIT_FLAC | MIX_INIT_OGG);
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
 }
 char *font = config->font;
 int fontsize = config->fontsize;
 expanded = config->findExpanded(xMult, yMult);
 if (expanded)
 {
  xMult = ((xFull - 10) / (config->width * expanded->xMult)) * expanded->xMult; // Allow for window decoration
  yMult = ((yFull - 10) / (config->height * expanded->yMult)) * expanded->yMult; // Allow for window decoration
  if (xMult > yMult)
   xMult = yMult;
  else
   yMult = xMult;
  font = expanded->font;
  fontsize = expanded->fontsize;
 }
 p3d.setMultiplier(xMult, yMult);
 label.x = config->label.x * xMult;
 label.y = config->label.y * yMult;
 label.w = config->label.w * xMult;
 label.h = config->label.h * yMult;
 text.x = config->text.x * xMult;
 text.y = config->text.y * yMult;
 text.w = config->text.w * xMult;
 text.h = config->text.h * yMult;
#ifndef BTBUILDER_NOTTF
 if (TTF_Init() == -1)
 {
  printf("Failed - TTF_Init\n");
  exit(0);
 }
#endif
 mainScreen = SDL_SetVideoMode(config->width * xMult, config->height * yMult, 32,
   SDL_SWSURFACE | (fullScreen ? SDL_FULLSCREEN : 0));
 if (mainScreen == NULL)
 {
  printf("Failed - SDL_SetVideoMode\n");
  exit(0);
 }
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
 setBackground(config->background, physfs);

 SDL_EventState(SDL_ACTIVEEVENT, SDL_IGNORE);
 SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
 SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);
 SDL_EventState(SDL_MOUSEBUTTONUP, SDL_IGNORE);
 SDL_EnableUNICODE(1);

 Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024);
}

BTDisplay::~BTDisplay()
{
 stopMusic(true);
 for (std::vector<BTUIElement*>::iterator elementItr = element.begin(); element.end() != elementItr; ++elementItr)
 {
  delete (*elementItr);
 }
 if (mainBackground)
 {
  SDL_FreeSurface(mainBackground);
 }
 element.clear();
 Mix_Quit();
 SDL_Quit();
}

void BTDisplay::addAnimation(MNG_AnimationState *animState, bool clear /*= false*/)
{
 activeAnimation.push_back(BTAnimation(animState, clear));
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

void BTDisplay::addColumns(const std::list<std::string>& c)
{
 element.push_back(new BTUIMultiColumn(c));
}

void BTDisplay::addReadString(const std::string &prompt, int maxLen, std::string &response)
{
 element.push_back(new BTUIReadString(prompt, maxLen, response));
}

void BTDisplay::addSelection(selectItem *list, int size, int &start, int &select, int num /*= 0*/)
{
 element.push_back(new BTUISelect(list, size, start, select, num));
}

void BTDisplay::addSelectImage(int &select)
{
 element.push_back(new BTUISelectImage(select));
}

void BTDisplay::clear(SDL_Rect &r, bool update /*= false*/)
{
 SDL_BlitSurface(mainBackground, &r, mainScreen, &r);
 SDL_UpdateRect(mainScreen, r.x, r.y, r.w, r.h);
}

void BTDisplay::clearElements()
{
 for (std::vector<BTUIElement*>::iterator elementItr = element.begin(); element.end() != elementItr; ++elementItr)
 {
  delete (*elementItr);
 }
 element.clear();
}

void BTDisplay::clearImage()
{
 picture = -1;
 if (animation.animation)
 {
  IMG_FreeMNG(animation.animation);
  animation.animation = NULL;
  removeAnimation(&animation);
 }
 SDL_Rect dst;
 dst.x = config->x3d * xMult;
 dst.y = config->y3d * yMult;
 dst.w = p3d.config->width * xMult;
 dst.h = p3d.config->height * yMult;
 SDL_FillRect(mainScreen, &dst, SDL_MapRGB(mainScreen->format, black.r, black.g, black.b));
 SDL_UpdateRect(mainScreen, dst.x, dst.y, dst.w, dst.h);
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
 picture = pic;
 char filename[50];
 if (animation.animation)
 {
  IMG_FreeMNG(animation.animation);
  animation.animation = NULL;
  removeAnimation(&animation);
 }
 snprintf(filename, 50, "slot%d.ng", pic);
 SDL_Rect src, dst;
 SDL_Surface *img = NULL;
 loadImageOrAnimation(filename, &img, &animation.animation);
 if (animation.animation)
 {
  IMG_SetAnimationState(&animation, -1, 0);
  animation.dst.x = config->x3d * xMult;
  animation.dst.y = config->y3d * yMult;
  animation.dst.w = p3d.config->width * xMult;
  animation.dst.h = p3d.config->height * yMult;
  addAnimation(&animation);
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
  SDL_FillRect(mainScreen, &dst, SDL_MapRGB(mainScreen->format, black.r, black.g, black.b));
  SDL_UpdateRect(mainScreen, dst.x, dst.y, dst.w, dst.h);
  return;
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
 labelText = name;
 SDL_BlitSurface(mainBackground, &label, mainScreen, &label);
 drawFont(name, label, white, center);
 SDL_UpdateRect(mainScreen, label.x, label.y, label.w, label.h);
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

void BTDisplay::drawMessage(const char *words, int *delay)
{
 addText(words);
 addText("");
 process(BTDisplay::allKeys, delay);
 clearElements();
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

void BTDisplay::drawView()
{
 picture = -1;
 if (animation.animation)
 {
  IMG_FreeMNG(animation.animation);
  animation.animation = NULL;
  removeAnimation(&animation);
 }
 Psuedo3DMap *m = Psuedo3DMap::getMap();
 p3d.draw(m, m->getX(), m->getY(), m->getFacing());
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
 if ((config->mapDisplayMode == BTMAPDISPLAYMODE_ALWAYS) ||
  ((config->mapDisplayMode == BTMAPDISPLAYMODE_NO3D) && (p3d.getConfig()->wallType.empty())))
  drawMap(false);
 SDL_UpdateRect(mainScreen, 0, 0, 0, 0);
}

void BTDisplay::drawIcons()
{
 for (int i = 0; i < config->icon.size(); ++i)
 {
  // Should cache this to not constantly redraw.
  config->icon[i]->draw(*this, SDL_GetTicks());
 }
}

void BTDisplay::drawMap(bool knowledge)
{
 // Draw black
 Psuedo3DMap *m = Psuedo3DMap::getMap();
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
  backup = SDL_CreateRGBSurface(SDL_SWSURFACE, dst.w, dst.h, 32, mainScreen->format->Rmask, mainScreen->format->Gmask, mainScreen->format->Bmask, mainScreen->format->Amask);
  SDL_BlitSurface(mainScreen, &dst, backup, &src);
 }
 SDL_FillRect(mainScreen, &dst, SDL_MapRGB(mainScreen->format, black.r, black.g, black.b));
 if (config->centerMap)
 {
  mapXStart = m->getX() - (config->widthMap / 2);
  mapYStart = m->getY() - (config->heightMap / 2);
 }
 else
 {
  int curX = m->getX();
  if (mapXStart > curX)
   mapXStart = curX;
  if (mapXStart + config->widthMap <= curX)
   mapXStart = curX - config->widthMap + 1;
  int curY = m->getY();
  if (mapYStart > curY)
   mapYStart = curY;
  if (mapYStart + config->heightMap <= curY)
   mapYStart = curY - config->heightMap + 1;
 }
 for (int i = 0; i < config->widthMap; ++i)
 {
  for (int k = 0; k < config->heightMap; ++k)
  {
   src.x = 0;
   src.y = 0;
   src.w = p3d.config->mapWidth * xMult;
   src.h = p3d.config->mapHeight * yMult;
   dst.x = (config->xMap + (i * p3d.config->mapWidth)) * xMult;
   dst.y = (config->yMap + (k * p3d.config->mapHeight)) * yMult;
   dst.w = p3d.config->mapWidth * xMult;
   dst.h = p3d.config->mapHeight * yMult;
   int know = m->getKnowledge(mapXStart + i, mapYStart + k);
   if ((mapXStart + i < 0) || (mapYStart + k < 0) || (mapXStart + i >= m->getXSize()) || (mapYStart + k >= m->getYSize()) || ((!knowledge) && (know == BTKNOWLEDGE_NO)))
   {
    SDL_Surface *unknown = p3d.getMapUnknown();
    if (unknown)
    {
     SDL_BlitSurface(unknown, &src, mainScreen, &dst);
    }
   }
   else
   {
    for (int direction = 0; direction < CARDINAL_DIRECTIONS; ++direction)
    {
     SDL_Surface *mapWall = p3d.getMapWall(m->getMapType(mapXStart + i, mapYStart + k, direction), direction, knowledge || (know == BTKNOWLEDGE_FULL));
     if (mapWall)
     {
      SDL_BlitSurface(mapWall, &src, mainScreen, &dst);
     }
    }
    if (m->hasSpecial(mapXStart + i, mapYStart + k))
    {
     SDL_Surface *special = p3d.getMapSpecial();
     if (special)
     {
      SDL_BlitSurface(special, &src, mainScreen, &dst);
     }
    }
    if ((mapYStart + k == m->getY()) && (mapXStart + i == m->getX()))
    {
     SDL_Surface *arrow = p3d.getMapArrow(m->getFacing());
     if (arrow)
     {
      SDL_BlitSurface(arrow, &src, mainScreen, &dst);
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
 SDL_UpdateRect(mainScreen, dst.x, dst.y, dst.w, dst.h);
 if (config->mapDisplayMode == BTMAPDISPLAYMODE_REQUEST)
 {
  unsigned char response = readChar();
  SDL_BlitSurface(backup, &src, mainScreen, &dst);
  SDL_UpdateRect(mainScreen, dst.x, dst.y, dst.w, dst.h);
  SDL_FreeSurface(backup);
 }
}

void BTDisplay::drawStats()
{
 int i;
 SDL_Rect dst;
 if (config->statusInfo.size() == 0)
  return;
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

int BTDisplay::getCurrentImage()
{
 return picture;
}

std::string BTDisplay::getCurrentLabel()
{
 return labelText;
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

int BTDisplay::playMusic(const char *file, bool physfs /*= true*/)
{
 if ((!music.empty()) && (music.front()->musicObj != NULL))
 {
  Mix_FadeOutMusic(1000);
 }
 int musicID = 1;
 for (std::list<BTMusic*>::iterator itr(music.begin()); itr != music.end(); ++itr)
 {
  if (musicID <= (*itr)->musicId)
   musicID = (*itr)->musicId + 1;
 }
 if ((file == NULL) || (file[0] == 0))
 {
  music.push_front(new BTMusic(musicID));
  return musicID;
 }
 BTMusic *m = new BTMusic(musicID);
 SDL_RWops *musicFile;
 if (physfs)
  musicFile = PHYSFSRWOPS_openRead(file);
 else
  musicFile = SDL_RWFromFile(file, "rb");
 if (musicFile)
 {
  m->musicObj = Mix_LoadMUS_RW(musicFile);
  if (m->musicObj)
   Mix_FadeInMusic(m->musicObj, -1, 1000);
 }
 music.push_front(m);
 return musicID;
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
 unsigned int key;
 std::vector<BTUIElement*>::iterator top = element.begin();
 for (; top != element.end(); ++top)
 {
  if ((BTUI_SELECT == (*top)->getType()) || (BTUI_BARRIER == (*top)->getType()) || (BTUI_READSTRING == (*top)->getType()) || (BTUI_SELECTIMAGE == (*top)->getType()))
  {
   break;
  }
  else if (BTUI_MULTICOLUMN == (*top)->getType())
  {
   BTUIMultiColumn *item = static_cast<BTUIMultiColumn*>(*top);
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
   if (BTUI_MULTICOLUMN == (*bottom)->getType())
   {
    BTUIMultiColumn* item = static_cast<BTUIMultiColumn*>(*bottom);
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
   item->response = readString(item->prompt.c_str(), item->maxLen, item->response);
   return 13;
  }
  else if (BTUI_SELECTIMAGE == (*top)->getType())
  {
   BTUISelectImage *item = static_cast<BTUISelectImage*>(*top);
   SDL_UpdateRect(mainScreen, text.x, text.y, text.w, text.h);
   item->select = selectImage(item->select);
   return 13;
  }
  else if (BTUI_SELECT == (*top)->getType())
  {
   select = static_cast<BTUISelect*>(*top);
   select->position.x = text.x;
   select->position.y = text.y + textPos;
   select->position.w = text.w;
   select->position.h = bottomPos - textPos;
   select->sanitize(*this);
  }
 }
 if (!select)
  SDL_UpdateRect(mainScreen, text.x, text.y, text.w, text.h);
 else if (select->numbered)
 {
   select->draw(*this);
   SDL_UpdateRect(mainScreen, text.x, text.y, text.w, text.h);
 }
 int start = SDL_GetTicks();
 int delayCurrent = ((delayOveride != -1) ? delayOveride : (delay ? *delay : 0));
 while (true)
 {
  if ((select) && (!select->numbered))
  {
   select->draw(*this);
   SDL_UpdateRect(mainScreen, text.x, text.y, text.w, text.h);
  }
  key = readChar(delayCurrent);
  if ((key == 0) || (key == 27))
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
     select->moveUp(*this);
     continue;
    }
    else if (key == BTKEY_DOWN)
    {
     select->moveDown(*this);
     continue;
    }
    else if (key == BTKEY_PGUP)
    {
     select->pageUp(*this);
     continue;
    }
    else if (key == BTKEY_PGDN)
    {
     select->pageDown(*this);
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
  else if (specialKeys == allKeys)
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

unsigned int BTDisplay::readChar(int delay /*= 0*/)
{
 SDL_Event sdlevent;
 SDL_TimerID timer;
 unsigned long animationDelay = 0;
 animationDelay = drawAnimationFrame();
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
   if (sdlevent.key.keysym.unicode)
    return sdlevent.key.keysym.unicode;
   else if ((sdlevent.key.keysym.sym == SDLK_UP) || (sdlevent.key.keysym.sym == SDLK_KP8))
    return BTKEY_UP;
   else if ((sdlevent.key.keysym.sym == SDLK_DOWN) || (sdlevent.key.keysym.sym == SDLK_KP2))
    return BTKEY_DOWN;
   else if ((sdlevent.key.keysym.sym == SDLK_LEFT) || (sdlevent.key.keysym.sym == SDLK_KP4))
    return BTKEY_LEFT;
   else if ((sdlevent.key.keysym.sym == SDLK_RIGHT) || (sdlevent.key.keysym.sym == SDLK_KP6))
    return BTKEY_RIGHT;
   else if ((sdlevent.key.keysym.sym == SDLK_PAGEDOWN) || (sdlevent.key.keysym.sym == SDLK_KP3))
    return BTKEY_PGDN;
   else if ((sdlevent.key.keysym.sym == SDLK_PAGEUP) || (sdlevent.key.keysym.sym == SDLK_KP9))
    return BTKEY_PGUP;
   else if ((sdlevent.key.keysym.sym == SDLK_END) || (sdlevent.key.keysym.sym == SDLK_KP1))
    return BTKEY_END;
   else if ((sdlevent.key.keysym.sym == SDLK_INSERT) || (sdlevent.key.keysym.sym == SDLK_KP0))
    return BTKEY_INS;
   else if ((sdlevent.key.keysym.sym == SDLK_KP_PERIOD) || (sdlevent.key.keysym.sym == SDLK_DELETE))
    return BTKEY_DEL;
   else if (sdlevent.key.keysym.sym == SDLK_F12)
    toggleFullScreen();
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
 std::string s = initial;
 int w, h;
 sizeFont(s.c_str(), w, h);
 if (h + textPos > text.h)
  scrollUp(h);
 unsigned char key;
 int len = s.length();
 SDL_Rect dst;
 dst.h = h;
 int startPos = textPos;
 std::string full = prompt;
 full += s;
 dst.x = text.x;
 dst.y = text.y + textPos;
 dst.w = text.w;
 drawText(full.c_str());
 int endPos = textPos;
 while (((key = readChar()) != 13) && (key !=  27))
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
  SDL_BlitSurface(mainBackground, &dst, mainScreen, &dst);
  textPos = startPos;
  drawText(full.c_str());
  if (textPos > endPos)
   endPos = textPos;
  SDL_UpdateRect(mainScreen, text.x, text.y, text.w, endPos - startPos);
 }
 return s;
}

void BTDisplay::refresh()
{
 SDL_BlitSurface(mainBackground, NULL, mainScreen, NULL);
 drawStats();
 SDL_UpdateRect(mainScreen, 0, 0, 0, 0);
}

void BTDisplay::removeAnimation(MNG_AnimationState *animState)
{
 activeAnimation.remove(BTAnimation(animState, false));
}

int BTDisplay::selectImage(int initial)
{
 char sz[50];
 snprintf(sz, 50, "%d", initial);
 std::string s = sz;
 int w, h;
 sizeFont(s.c_str(), w, h);
 if (h + textPos > text.h)
  scrollUp(h);
 unsigned char key;
 int len = s.length();
 SDL_Rect dst;
 dst.h = h;
 int startPos = textPos;
 std::string full;
 full += s;
 dst.x = text.x;
 dst.y = text.y + textPos;
 dst.w = text.w;
 drawText(full.c_str());
 int endPos = textPos;
 int current = initial;
 drawImage(current);
 while (((key = readChar()) != 13) && (key !=  27))
 {
  if (key == 8)
  {
   if (len > 0)
   {
    s.erase(--len);
    full.erase(full.length() - 1);
   }
   current = atol(s.c_str());
  }
  else if ((len < 50) && (key >= '0') && (key <= '9'))
  {
   s.push_back(key);
   full.push_back(key);
   ++len;
   current = atol(s.c_str());
  }
  else if ((key == '+') || (key == BTKEY_UP))
  {
   ++current;
   snprintf(sz, 50, "%d", current);
   full = s = sz;
  }
  else if ((key == '-') || (key == BTKEY_DOWN))
  {
   --current;
   snprintf(sz, 50, "%d", current);
   full = s = sz;
  }
  else
   continue;
  dst.h = endPos - startPos;
  SDL_BlitSurface(mainBackground, &dst, mainScreen, &dst);
  textPos = startPos;
  drawText(full.c_str());
  if (textPos > endPos)
   endPos = textPos;
  SDL_UpdateRect(mainScreen, text.x, text.y, text.w, endPos - startPos);
  drawImage(current);
 }
 return ((key == 27) ? initial : current);
}

void BTDisplay::setBackground(const char *file, bool physfs /*= true*/)
{
 if (mainBackground)
 {
  SDL_FreeSurface(mainBackground);
  mainBackground = NULL;
 }
 loadImageOrAnimation(file, &mainBackground, NULL, physfs);
 SDL_BlitSurface(mainBackground, NULL, mainScreen, NULL);
 SDL_UpdateRect(mainScreen, 0, 0, 0, 0);
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
  newXMult = ((xFull - 10) / (c->width * expanded->xMult)) * expanded->xMult; // Allow for window decoration
  newYMult = ((yFull - 10) / (c->height * expanded->yMult)) * expanded->yMult; // Allow for window decoration
  if (newXMult > newYMult)
   newXMult = newYMult;
  else
   newYMult = newXMult;
  font = expanded->font;
  fontsize = expanded->fontsize;
 }
 p3d.setMultiplier(newXMult, newYMult);
 label.x = c->label.x * newXMult;
 label.y = c->label.y * newYMult;
 label.w = c->label.w * newXMult;
 label.h = c->label.h * newYMult;
 text.x = c->text.x * newXMult;
 text.y = c->text.y * newYMult;
 text.w = c->text.w * newXMult;
 text.h = c->text.h * newYMult;
 if ((config->width * xMult != c->width * newXMult) || (config->height * yMult != c->height * newYMult))
 {
  mainScreen = SDL_SetVideoMode(c->width * newXMult, c->height * newYMult, 32,
    SDL_SWSURFACE | (fullScreen ? SDL_FULLSCREEN : 0));
  if (mainScreen == NULL)
  {
   printf("Failed - SDL_SetVideoMode\n");
   exit(0);
  }
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
 setBackground(c->background);
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
 SDL_Surface *backup = SDL_CreateRGBSurface(SDL_SWSURFACE, dst.w, dst.h, 32, mainScreen->format->Rmask, mainScreen->format->Gmask, mainScreen->format->Bmask, mainScreen->format->Amask);
 SDL_BlitSurface(mainScreen, &dst, backup, &src);
 mainScreen = SDL_SetVideoMode(config->width * xMult, config->height * yMult, 32,
    SDL_SWSURFACE | (fullScreen ? SDL_FULLSCREEN : 0));
 if (mainScreen == NULL)
 {
  printf("Failed - SDL_SetVideoMode\n");
  exit(0);
 }
 SDL_BlitSurface(backup, &src, mainScreen, &dst);
 SDL_UpdateRect(mainScreen, dst.x, dst.y, dst.w, dst.h);
}

void BTDisplay::drawFont(const char *text, SDL_Rect &dst, SDL_Color c, alignment a)
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

void BTDisplay::drawImage(SDL_Rect &dst, SDL_Surface *img)
{
 SDL_Rect src;
 src.x = 0;
 src.y = 0;
 src.w = dst.w;
 src.h = dst.h;
 SDL_BlitSurface(img, &src, mainScreen, &dst);
 SDL_UpdateRect(mainScreen, dst.x, dst.y, dst.w, dst.h);
}

void BTDisplay::drawRect(SDL_Rect &dst, SDL_Color c)
{
 SDL_FillRect(mainScreen, &dst, SDL_MapRGB(mainScreen->format, c.r, c.g, c.b));
}

void BTDisplay::loadImageOrAnimation(const char *file, SDL_Surface **img, MNG_Image **animation, bool physfs /*= true*/)
{
 if (expanded)
 {
  std::string filename = "image/";
  filename += expanded->directory;
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

unsigned long BTDisplay::drawAnimationFrame()
{
 unsigned long ticks = SDL_GetTicks();
 unsigned long next = 0;
 for (std::list<BTAnimation>::iterator itr = activeAnimation.begin(); itr != activeAnimation.end(); itr++)
 {
  SDL_Surface *nextImg = IMG_TimeUpdate(itr->animation, ticks);
  if (nextImg)
  {
   if (itr->clear)
    clear(itr->animation->dst, false);
   drawImage(itr->animation->dst, nextImg);
  }
  unsigned long possible = IMG_TimeToNextFrame(itr->animation, ticks);
  if ((next == 0) || (possible < next))
   next = possible;
 }
 return next;
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
 if (select >= start + lines)
  start = select - lines - 1;
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
   if (!list[i].name.empty())
    d.drawFont(list[i].name.c_str(), dst, d.getBlack(), BTDisplay::left);
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
   if ((list[i].value) && (list[i].flags.isSet(BTSELECTFLAG_SHOWVALUE)))
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
   if (!list[i].name.empty())
    d.drawFont(list[i].name.c_str(), dst, ((select != i) ? d.getBlack() : d.getWhite()), BTDisplay::left);
   dst.y += h;
  }
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
  d.drawFont(itr->c_str(), dst, d.getBlack(), BTDisplay::left);
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

