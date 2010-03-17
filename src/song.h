#ifndef __SONG_H
#define __SONG_H
/*-------------------------------------------------------------------------*\
  <song.h> -- Song header file

  Date      Programmer  Description
  03/14/10  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "dice.h"
#include "manifest.h"
#include "xmlserializer.h"

class BTSong : public XMLObject
{
 public:
  BTSong()
  {
   name = new char[1];
   name[0] = 0;
  }

  ~BTSong() { delete [] name; }

  const char *getName() const;

  int play(BTDisplay &d, const char *caster, BTCombat *combat, int casterLevel);
  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTSong; }
  static void readXML(const char *filename, XMLVector<BTSong*> &song);

  char *name;
  XMLVector<BTManifest*> manifest;
};

#endif

