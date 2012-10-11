/*-------------------------------------------------------------------------*\
  <editor.C> -- Editor implementation file

  Date      Programmer  Description
  10/10/12  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "editor.h"

BTEditor::BTEditor(BTModule *m)
 : BTCore(m), currentWall(0)
{
}

BTEditor::~BTEditor()
{
}

int BTEditor::getLight()
{
 return 5;
}

int BTEditor::getWallType(int x, int y, int direction)
{
 if (x < 0)
  x += levelMap->getXSize();
 x = x % levelMap->getXSize();
 if (y < 0)
  y += levelMap->getYSize();
 y = y % levelMap->getYSize();
 IShort w = levelMap->getSquare(y, x).getWall(direction);
 if (w == 0)
  return 0;
 int mapType = p3dConfig->findMapType(w, true);
 if (mapType == 0)
  return 0;
 if (-1 != p3dConfig->mapType[mapType - 1]->incompleteType)
  return p3dConfig->mapType[mapType - 1]->incompleteType;
 return w;
}

int BTEditor::getKnowledge(int x, int y)
{
 return BTKNOWLEDGE_FULL;
}

void BTEditor::editMap(BTDisplay &d, const char *filename)
{
 d.setPsuedo3DConfig(module->wall);
 loadMap(filename);
 xPos = 0; yPos = 0; facing = 0;
 p3dConfig = d.setWallGraphics(levelMap->getType());
 unsigned char key = ' ';
 if (currentWall < p3dConfig->mapType.size())
  d.drawText(p3dConfig->mapType[currentWall]->name.c_str());
 else
  d.drawText("Clear");
 while (key != 'q')
 {
  if (levelMap->getSquare(yPos, xPos).getSpecial() > -1)
   d.drawLabel(levelMap->getSpecial(levelMap->getSquare(yPos, xPos).getSpecial())->getName());
  else
   d.drawLabel("");
  d.drawView();
  key = d.readChar(6000);
  switch (key)
  {
   case BTKEY_UP:
    if (yPos > 0)
     yPos--;
    else
     yPos = getYSize() - 1;
    break;
   case BTKEY_LEFT:
    if (xPos > 0)
     xPos--;
    else
     xPos = getXSize() - 1;
    break;
   case BTKEY_DOWN:
    if (yPos < getYSize() - 1)
     yPos++;
    else
     yPos = 0;
    break;
   case BTKEY_RIGHT:
    if (xPos < getXSize() - 1)
     xPos++;
    else
     xPos = 0;
    break;
   case BTKEY_PGDN:
    if (facing < 3)
     facing++;
    else
     facing = 0;
    break;
   case BTKEY_END:
    if (facing > 0)
     facing--;
    else
     facing = 3;
    break;
   case 13:
   {
    int wall = 0;
    if (currentWall < p3dConfig->mapType.size())
     wall = p3dConfig->mapType[currentWall]->type;
    levelMap->getSquare(yPos, xPos).setWall(facing, wall);
    int xOpposite = xPos + Psuedo3D::changeXY[facing][0] + levelMap->getXSize();
    xOpposite = xOpposite % levelMap->getXSize();
    int yOpposite = yPos + Psuedo3D::changeXY[facing][1] + levelMap->getYSize();
    yOpposite = yOpposite % levelMap->getYSize();
    levelMap->getSquare(yOpposite, xOpposite).setWall((facing + 2) % 4, wall);
    break;
   }
   case ' ':
    if (currentWall < p3dConfig->mapType.size())
    {
     currentWall++;
    }
    else
    {
     currentWall = 0;
    }
    if (currentWall < p3dConfig->mapType.size())
     d.drawText(p3dConfig->mapType[currentWall]->name.c_str());
    else
     d.drawText("Clear");
    break;
  }
 }
}
