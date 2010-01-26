#ifndef __MAINSCREEN_H
#define __MAINSCREEN_H
/*-------------------------------------------------------------------------*\
  <mainscreen.h> -- Main screen header file

  Date      Programmer  Description
  01/24/10  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "display.h"

class BTMainScreen
{
 public:
  BTMainScreen(const char *a0);
  ~BTMainScreen();

  void run();
  void runModule(std::string moduleFile);

 protected:
  int Alternative_setSaneConfig(std::string appName);

 private:
  const char *argv0;
  BTDisplayConfig *mainConfig;
  BTDisplay *display;
};

#endif

