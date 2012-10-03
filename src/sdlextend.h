#ifndef __SDLEXTEND_H
#define __SDLEXTEND_H
/*-------------------------------------------------------------------------*\
  <sdlextend.h> -- SDL extended functions header file

  Date      Programmer  Description
  03/09/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include <SDL.h>
#include <SDL_mng.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _simpleFont {
 int w, h;
 unsigned char *bitmap;
} simpleFont;

extern simpleFont simple8x8;

extern SDL_Surface *simpleZoomSurface(SDL_Surface *src, int xMult, int yMult);
extern SDL_Surface *simpleRender_Solid(simpleFont *font, const char *text, SDL_Color c);
extern void simpleZoomAnimation(MNG_Image *animation, int xMult, int yMult);

#ifdef __cplusplus
}
#endif

#endif
