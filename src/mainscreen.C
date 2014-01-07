/*-------------------------------------------------------------------------*\
  <mainscreen.C> -- Main screen implementation file

  Date      Programmer  Description
  01/24/10  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "mainscreen.h"
#include "module.h"
#include "monster.h"
#include "spell.h"
#include "item.h"
#include "map.h"
#include "factory.h"
#include "game.h"
#include "editor.h"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include <physfs.h>

namespace fs = boost::filesystem;

BTMainScreen::BTMainScreen(const char *a0, std::string lDir, int mult /*= 0*/)
 : argv0(a0), libDir(lDir), mainConfig(0), display(0), multiplier(mult)
{
}

BTMainScreen::~BTMainScreen()
{
 if (display)
  delete display;
 if (mainConfig)
  delete mainConfig;
}

void BTMainScreen::run()
{
 loadMainConfig();
 display = new BTDisplay(mainConfig, true, multiplier);
 std::vector<std::string> fileModule;
 XMLVector<BTModule*> module;
 fs::directory_iterator end_iter;
 XMLSerializer parser;
 std::string moduleDir("module/");
 char **files = PHYSFS_enumerateFiles(moduleDir.c_str());
 for (char **i = files; *i != NULL; i++)
 {
  int len = strlen(*i);
  if ((!PHYSFS_isDirectory(*i)) && (len > 4) && (strcmp(".xml", (*i) + (len - 4)) == 0))
  {
   std::string fullName = moduleDir + *i;
   fileModule.push_back(fullName);
   BTModule *current = new BTModule;
   module.push_back(current);
   parser.removeLevel();
   current->serialize(&parser);
   parser.parse(fullName.c_str(), true);
  }
 }
 PHYSFS_freeList(files);
 BTDisplay::selectItem *list = new BTDisplay::selectItem[module.size()];
 for (int i = 0; i < module.size(); ++i)
 {
  list[i].name = module[i]->name;
 }
 PHYSFS_deinit();
 int start(0);
 int select(0);
 unsigned int key = 0;
 while ((key != 'q') && (key != 27))
 {
  display->clearText();
  display->addSelection(list, module.size(), start, select);
  key = display->process("eq");
  if (key == 13)
  {
   runModule(fileModule[select]);
  }
  else if (key == 'e')
  {
   editModule(fileModule[select]);
  }
 }
 delete [] list;
 delete display;
 display = NULL;
}

void BTMainScreen::runModule(std::string moduleFile)
{
 BTModule module;
 loadModule(moduleFile, module);
 BTGame game(&module);
 BTDisplayConfig config;
 XMLSerializer parser;
 config.serialize(&parser);
 parser.parse("data/display.xml", true);
 if (display)
 {
  display->setConfig(&config);
 }
 else
 {
  display = new BTDisplay(&config, multiplier);
 }
 BTGame::getGame()->run(*display);
 if (mainConfig)
 {
  display->setConfig(mainConfig);
 }
 else
 {
  delete display;
  display = NULL;
 }
 PHYSFS_deinit();
}

void BTMainScreen::editModule(std::string moduleFile, std::string mapFile /*= std::string()*/)
{
 BTModule module;
 loadModule(moduleFile, module);
 BTEditor editor(&module);
 if (!display)
 {
  loadMainConfig();
  display = new BTDisplay(mainConfig, true, multiplier);
 }
 if (mapFile.empty())
  editor.edit(*display);
 else
  editor.editMap(*display, mapFile.c_str());
 if (mainConfig)
 {
  display->setConfig(mainConfig);
 }
 else
 {
  delete display;
  display = NULL;
 }
 PHYSFS_deinit();
}

void BTMainScreen::loadModule(std::string moduleFile, BTModule &module)
{
 init();
 XMLSerializer parser;
 module.serialize(&parser);
 if (!PHYSFS_exists(moduleFile.c_str()))
 {
  fprintf(stderr, "Module file '%s' does not exist\n", moduleFile.c_str());
  exit(0);
 }
 parser.parse(moduleFile.c_str(), true);
 std::string moduleName(fs::path(moduleFile).stem().string());
 PHYSFS_deinit();
 init(moduleName, module.content);
}

void BTMainScreen::init(std::string moduleName, std::string contentFile)
{
 PHYSFS_init(argv0);
 const char *basedir = PHYSFS_getBaseDir();
 std::string userdir = PHYSFS_getUserDir();

#ifdef _WIN32
 std::string appName("identical");
#else
 std::string appName(".identical");
#endif
 std::string appNameUnix(appName + "/");
 appName += PHYSFS_getDirSeparator();
 appName += "btbuilder";
 appNameUnix += "btbuilder";
 if (!moduleName.empty())
 {
  appName += PHYSFS_getDirSeparator();
  appName += moduleName;
  appNameUnix += "/";
  appNameUnix += moduleName;
 }
 std::string writedir = userdir + appName;

 if (!PHYSFS_setWriteDir(writedir.c_str()))
 {
  if ( (PHYSFS_setWriteDir(userdir.c_str())) && (PHYSFS_mkdir(appNameUnix.c_str())) )
  {
   if (!PHYSFS_setWriteDir(writedir.c_str()))
    throw physfsException();
  }
  else
  {
   throw physfsException();
  }
 }
 PHYSFS_addToSearchPath(writedir.c_str(), 0);
 if (!contentFile.empty())
 {
  std::string contentPath(libDir);
  if (!contentPath.empty())
   contentPath += PHYSFS_getDirSeparator();
  contentPath += "module";
  contentPath += PHYSFS_getDirSeparator();
  contentPath += "content";
  contentPath += PHYSFS_getDirSeparator();
  contentPath += contentFile;
  PHYSFS_addToSearchPath(contentPath.c_str(), 1);
 }
 if (libDir.empty())
  PHYSFS_addToSearchPath(basedir, 1);
 else
  PHYSFS_addToSearchPath(libDir.c_str(), 1);
}

void BTMainScreen::loadMainConfig()
{
 init();
 mainConfig = new BTDisplayConfig;
 XMLSerializer parser;
 mainConfig->serialize(&parser);
 parser.parse("data/mainscreen.xml", true);
}

