/*-------------------------------------------------------------------------*\
  <sdlextend.c> -- SDL extended functions implementation file

  Date      Programmer  Description
  03/09/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "sdlextend.h"
#include <istdlib.h>
#include <igrbasics.h>
//#include <igrtext.h>

extern unsigned char ITextFont8x8[256][8]; // will be in igrtext.h in the next release.

simpleFont simple8x8 = { 8, 8, (unsigned char*)ITextFont8x8 };

SDL_Surface *simpleZoomSurface(SDL_Surface *src, int xMult, int yMult)
{
 int x, xCopy, y, yCopy;
 char *srcPixelRow;
 char *dstPixelRow;
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

 SDL_Surface *src32 = src;
 if (src->format->BitsPerPixel != 32)
 {
  src32 = SDL_CreateRGBSurface(SDL_SWSURFACE, src->w, src->h, 32, rmask, gmask, bmask, amask);
  SDL_BlitSurface(src, NULL, src32, NULL);
 }
 SDL_Surface *dst = SDL_CreateRGBSurface(SDL_SWSURFACE, src->w * xMult, src->h * yMult, 32, src32->format->Rmask, src32->format->Gmask, src32->format->Bmask, src32->format->Amask);
 SDL_LockSurface(src32);
 srcPixelRow = (char *)src32->pixels;
 dstPixelRow = (char *)dst->pixels;
 for (y = 0; y < src->h; y++)
 {
  if (xMult == 1)
  {
   memcpy(dstPixelRow, srcPixelRow, dst->pitch);
  }
  else
  {
   for (x = 0; x < src->w; x++)
   {
    for (xCopy = 0; xCopy < xMult; ++xCopy)
    {
     memcpy(dstPixelRow + ((x * xMult) + xCopy) * 4, srcPixelRow + (x * 4), 4);
    }
   }
  }
  for (yCopy = 1; yCopy < yMult; yCopy++)
  {
   memcpy(dstPixelRow + (dst->pitch * yCopy), dstPixelRow, dst->pitch);
  }
  dstPixelRow += dst->pitch * yMult;
  srcPixelRow += src32->pitch;
 }
 SDL_UnlockSurface(src32);
 if (src != src32)
  SDL_FreeSurface(src32);
 return dst;
}

SDL_Surface *simpleRender_Solid(simpleFont *font, const char *text, SDL_Color c)
{
 int bpc;
 int len, i, k, x, y, w;
 unsigned char bit;
 SDL_Surface *s;
 Uint32 c32;
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

 bpc = font->w * font->h / 8;
 if (font->w * font->h % 8 > 0)
  bpc++;
 len = strlen(text);
 s = SDL_CreateRGBSurface(SDL_SWSURFACE, font->w * len, font->h, 32, rmask, gmask, bmask, amask);
 c32 = SDL_MapRGB(s->format, c.r, c.g, c.b);
 unsigned char *pixels = (unsigned char *)s->pixels;
 x = 0;
 for (i = 0; i < len; i++)
 {
  y = w = 0;
  for (k = 0; k < bpc; k++)
  {
   for (bit = 128; bit > 0; bit = (bit >> 1))
   {
    if ((font->bitmap[((unsigned char)text[i]) * bpc + k]) & bit)
    {
     *((Uint32 *)(pixels + y * s->pitch + (x + w) * 4)) = c32;
    }
    w++;
    if (w >= font->w)
    {
     w = 0;
     ++y;
    }
   }
  }
  x += font->w;
 }
 return s;
}
