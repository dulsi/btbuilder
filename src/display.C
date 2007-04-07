/*-------------------------------------------------------------------------*\
  <display.C> -- Display implementation file

  Date      Programmer  Description
  02/28/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "compressor.h"
#include "display.h"
#include "game.h"
#include "ikbbuffer.h"
#include "physfsrwops.h"
#include <SDL_image.h>

BTDisplay::BTDisplay(BTDisplayConfig *c)
 : config(c), xMult(0), yMult(0), status(*this), textPos(0), p3d(0, 0), mainScreen(0), ttffont(0), sfont(&simple8x8)
{
 if (SDL_Init(SDL_INIT_VIDEO) < 0)
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
}

BTDisplay::~BTDisplay()
{
 SDL_Quit();
}

void BTDisplay::clearText()
{
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
   IKeybufferGet();
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

unsigned char BTDisplay::readChar()
{
 return IKeybufferGet();
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
 while (((key = IKeybufferGet()) != 13) && (key !=  27))
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
  key = IKeybufferGet();
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
