#ifndef __ISTDLIB_H
#define __ISTDLIB_H
/*-------------------------------------------------------------------------*\
  <istdlib.h> -- Identical Software's Standard Library

  Date      Programmer  Description
  02/28/97  Dennis      Created.
\*-------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#if defined(__MSDOS__) && !defined(ALLEGRO)

#define IFAR far
#define IRandom(x) random(x)
#define IRandomize() randomize()

#if defined(__LARGE__) || defined(__HUGE__) || defined(__COMPACT__)

#include <string.h>

#define IMalloc(x) malloc(x)
#define ICalloc(x,y) calloc(x,y)
#define IRealloc(x,y) realloc(x,y)
#define IFree(x) free(x)
#define IMemset(x,y,z) memset(x,y,z)
#define IMemcpy(x,y,z) memcpy(x,y,z)

#else

#include <alloc.h>

#define IMalloc(x) farmalloc(x)
#define ICalloc(x,y) farcalloc(x,y)
#define IRealloc(x,y) farrealloc(x,y)
#define IFree(x) farfree(x)
#define IMemset(x,y,z) _fmemset(x,y,z)
#define IMemcpy(x,y,z) _fmemcpy(x,y,z)

#endif

typedef unsigned int IUShort;
typedef unsigned long IULong;
typedef signed int IShort;
typedef signed long ILong;

#else

#include <string.h>

#define IFAR
#define IMalloc(x) malloc(x)
#define ICalloc(x,y) calloc(x,y)
#define IRealloc(x,y) realloc(x,y)
#define IFree(x) free(x)
#define IMemset(x,y,z) memset(x,y,z)
#define IMemcpy(x,y,z) memcpy(x,y,z)
#define IRandom(x) (random()%x)
#define IRandomize() (srandom(time(0)))

typedef unsigned short IUShort;
typedef unsigned int IULong;
typedef signed short IShort;
typedef signed int ILong;

#endif

typedef unsigned char IUByte;
typedef signed char IByte;
typedef int IBool;

#define ITRUE  1
#define IFALSE 0

#ifdef __cplusplus
}
#endif

#endif

