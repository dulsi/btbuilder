#ifndef __STATUSBAR_H
#define __STATUSBAR_H
/*-------------------------------------------------------------------------*\
  <statusbar.h> -- Status bar header file

  Date      Programmer  Description
  04/01/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "xmlserializer.h"

class BTBackgroundAndScreen;

class BTStatusBar : public ObjectSerializer
{
 public:
  BTStatusBar() {}
  ~BTStatusBar() {}

  void draw(BTBackgroundAndScreen *d);

  virtual int getLevel();
};

#endif
