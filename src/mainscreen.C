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
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include <physfs.h>

namespace fs = boost::filesystem;

BTMainScreen::BTMainScreen(const char *a0)
 : argv0(a0), mainConfig(0), display(0)
{
}

BTMainScreen::~BTMainScreen()
{
 if (display)
  delete display;
}

void BTMainScreen::run()
{
 mainConfig = new BTDisplayConfig;
 XMLSerializer parser;
 mainConfig->serialize(&parser);
 parser.parse("data/mainscreen.xml", false);
 display = new BTDisplay(mainConfig, false);
 std::vector<std::string> fileModule;
 XMLVector<BTModule*> module;
 fs::directory_iterator end_iter;
 for (fs::directory_iterator dir_itr("module"); dir_itr != end_iter; ++dir_itr )
 {
  if (fs::is_regular_file(dir_itr->status()))
  {
   fileModule.push_back(dir_itr->path().string());
   BTModule *current = new BTModule;
   module.push_back(current);
   parser.removeLevel();
   current->serialize(&parser);
   parser.parse(dir_itr->path().string().c_str(), false);
  }
 }
 BTDisplay::selectItem *list = new BTDisplay::selectItem[module.size()];
 for (int i = 0; i < module.size(); ++i)
 {
  list[i].name = module[i]->name;
 }
 int start(0);
 int select(0);
 display->addSelection(list, module.size(), start, select);
 unsigned int key = display->process();
 if (key == 13)
 {
  runModule(fileModule[select]);
 }
 delete [] list;
}

void BTMainScreen::runModule(std::string moduleFile)
{
 PHYSFS_init(argv0);
 BTModule module;
 XMLSerializer parser;
 module.serialize(&parser);
 parser.parse(moduleFile.c_str(), false);
 std::string appName("btbuilder");
 appName += PHYSFS_getDirSeparator();
 appName += fs::path(moduleFile).stem().string();
 if (0 == PHYSFS_setSaneConfig("identical", appName.c_str(), NULL, 0, 0))
 {
  // HACK: Something is wrong with PHYSFS_setSaneConfig on windows.
  if (0 == Alternative_setSaneConfig("btbsave"))
   return;
 }
 std::string contentPath("module");
 contentPath += PHYSFS_getDirSeparator();
 contentPath += "content";
 contentPath += PHYSFS_getDirSeparator();
 contentPath += module.content;
 PHYSFS_addToSearchPath(contentPath.c_str(), 0);
 BTGame game(&module);
 BTFactory<BTMonster> &monList(game.getMonsterList());
 BTFactory<BTItem> &itmList(game.getItemList());
 BTFactory<BTSpell> &splList(game.getSpellList());
 BTDisplayConfig config;
 parser.removeLevel();
 config.serialize(&parser);
 parser.parse("data/display.xml", true);
 if (display)
 {
  display->setConfig(&config);
 }
 else
 {
  display = new BTDisplay(&config);
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

int BTMainScreen::Alternative_setSaneConfig(std::string appName)
{
 const char *basedir = PHYSFS_getBaseDir();
 std::string userdir = PHYSFS_getUserDir();
 std::string writedir = userdir + appName;

 if (!PHYSFS_setWriteDir(writedir.c_str()))
 {
  int no_write = 0;
  if ( (PHYSFS_setWriteDir(userdir.c_str())) && (PHYSFS_mkdir(appName.c_str())) )
  {
   if (!PHYSFS_setWriteDir(writedir.c_str()))
    no_write = 1;
  } /* if */
  else
  {
   no_write = 1;
  } /* else */

  if (no_write)
  {
   PHYSFS_setWriteDir(NULL);   /* just in case. */
   return 0;
  } /* if */
 } /* if */
 PHYSFS_addToSearchPath(writedir.c_str(), 0);
 PHYSFS_addToSearchPath(basedir, 1);
 return 1;
}

