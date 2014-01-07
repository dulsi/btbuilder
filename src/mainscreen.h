#ifndef __MAINSCREEN_H
#define __MAINSCREEN_H
/*-------------------------------------------------------------------------*\
  <mainscreen.h> -- Main screen header file

  Date      Programmer  Description
  01/24/10  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "display.h"
#include "module.h"

class physfsException : public std::exception
{
};

class BTMainScreen
{
 public:
  BTMainScreen(const char *a0, std::string lDir, int mult = 0);
  ~BTMainScreen();

  void run();
  void runModule(std::string moduleFile);
  void editModule(std::string moduleFile, std::string mapFile = std::string());

  void loadModule(std::string moduleFile, BTModule &module);

 protected:
  void init(std::string moduleName = std::string(), std::string contentFile = std::string());
  void loadMainConfig();

 private:
  const char *argv0;
  std::string libDir;
  BTDisplayConfig *mainConfig;
  BTDisplay *display;
  int multiplier;
};

#endif

