#ifndef __STATUSBAR_H
#define __STATUSBAR_H
/*-------------------------------------------------------------------------*\
  <statusbar.h> -- Status bar header file

  Date      Programmer  Description
  04/01/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "xmlserializer.h"
#include "pc.h"

class BTDisplay;

class BTStatusBar : public ObjectSerializer
{
 public:
  BTStatusBar(BTDisplay &d) : display(d) {}
  ~BTStatusBar() {}

  void draw();

  virtual int getLevel();

 private:
  BTDisplay &display;
};

#endif
