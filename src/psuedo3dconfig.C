/*-------------------------------------------------------------------------*\
  <psuedo3dconfig.C> -- Psuedo3DConfig implementation file

  Date      Programmer  Description
  01/22/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "psuedo3dconfig.h"
#include <expat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define XMLBUFFER 2048

#define STATE_NONE 0
#define STATE_HEIGHT 1
#define STATE_WIDTH 2
#define STATE_BACKGROUND 3
#define STATE_WALLTYPE 5
#define STATE_WALL 4

class Psuedo3DConfigXML
{
 public:
  Psuedo3DConfigXML(std::vector<Psuedo3DConfig*> &cfg);
  ~Psuedo3DConfigXML() { XML_ParserFree(p); }
  void parse(char *filename);

  static void startElement(void *userData, const XML_Char *name, const XML_Char **atts);
  static void endElement(void *userData, const XML_Char *name);
  static void characterData(void *userData, const XML_Char *s, int len);

  XML_Parser p;
  std::vector<Psuedo3DConfig*> &ary;
  int state;
  int type;
  int direction;
  Psuedo3DConfig *current;
};

Psuedo3DConfigXML::Psuedo3DConfigXML(std::vector<Psuedo3DConfig*> &cfg)
: ary(cfg), state(STATE_NONE), type(0), direction(0), current(NULL)
{
 p = XML_ParserCreate(NULL);
 XML_SetUserData(p, this);
 XML_SetElementHandler(p, startElement, endElement);
 XML_SetCharacterDataHandler(p, characterData);
}

void Psuedo3DConfigXML::parse(char *filename)
{
 char *buffer = new char[XMLBUFFER];
 int fd = open(filename, O_RDONLY);
 while (true)
 {
  size_t num = read(fd, buffer, XMLBUFFER);
  XML_Parse(p, buffer, num, (0 == num));
  if (0 == num)
   break;
 }
 close(fd);
 delete [] buffer;
}

void Psuedo3DConfigXML::startElement(void *userData, const XML_Char *name, const XML_Char **atts)
{
 Psuedo3DConfigXML *configXML = (Psuedo3DConfigXML*)userData;
 if (configXML->current)
 {
  if (configXML->state == STATE_NONE)
  {
   if (strcmp(name, "height") == 0)
    configXML->state = STATE_HEIGHT;
   else if (strcmp(name, "width") == 0)
    configXML->state = STATE_WIDTH;
   else if (strcmp(name, "background") == 0)
    configXML->state = STATE_BACKGROUND;
   else if (strcmp(name, "walltype") == 0)
   {
    configXML->state = STATE_WALLTYPE;
    for (int attNum = 0; atts[attNum]; attNum += 2)
    {
     if (strcmp(atts[attNum], "type") == 0)
      configXML->type = atoi(atts[attNum + 1]);
    }
   }
  }
  else if ((configXML->state == STATE_WALLTYPE) && (strcmp(name, "wall") == 0))
  {
   configXML->state = STATE_WALL;
   for (int attNum = 0; atts[attNum]; attNum += 2)
   {
    if (strcmp(atts[attNum], "direction") == 0)
     configXML->direction = atoi(atts[attNum + 1]);
   }
  }
 }
 else
 {
  if (strcmp(name, "psuedo3d") == 0)
  {
   configXML->current = new Psuedo3DConfig;
  }
 }
}

void Psuedo3DConfigXML::endElement(void *userData, const XML_Char *name)
{
 Psuedo3DConfigXML *configXML = (Psuedo3DConfigXML*)userData;
 if (((configXML->state == STATE_HEIGHT) && (strcmp(name, "height") == 0)) ||
  ((configXML->state == STATE_WIDTH) && (strcmp(name, "width") == 0)) ||
  ((configXML->state == STATE_BACKGROUND) && (strcmp(name, "background") == 0)) ||
  ((configXML->state == STATE_WALLTYPE) && (strcmp(name, "walltype") == 0)))
 {
  configXML->state = STATE_NONE;
 }
 else if ((configXML->state == STATE_WALL) && (strcmp(name, "wall") == 0))
 {
  configXML->state = STATE_WALLTYPE;
 }
 else if ((configXML->current) && (configXML->state == STATE_NONE) && (strcmp(name, "psuedo3d") == 0))
 {
  configXML->ary.push_back(configXML->current);
  configXML->current = NULL;
 }
}

void Psuedo3DConfigXML::characterData(void *userData, const XML_Char *s, int len)
{
 Psuedo3DConfigXML *configXML = (Psuedo3DConfigXML*)userData;
 switch (configXML->state)
 {
  case STATE_HEIGHT:
   configXML->current->height = atoi(s);
   break;
  case STATE_WIDTH:
   configXML->current->width = atoi(s);
   break;
  case STATE_BACKGROUND:
   if (configXML->current->background)
    delete [] configXML->current->background;
   configXML->current->background = new char[len + 1];
   strncpy(configXML->current->background, s, len);
   configXML->current->background[len] = 0;
   break;
  case STATE_WALL:
   if (configXML->current->walls[configXML->type][configXML->direction])
    delete [] configXML->current->walls[configXML->type][configXML->direction];
   configXML->current->walls[configXML->type][configXML->direction] = new char[len + 1];
   strncpy(configXML->current->walls[configXML->type][configXML->direction], s, len);
   configXML->current->walls[configXML->type][configXML->direction][len] = 0;
   break;
  default:
   break;
 }
}

void Psuedo3DConfig::readXML(char *filename, std::vector<Psuedo3DConfig*> &cfg)
{
 Psuedo3DConfigXML parser(cfg);
 parser.parse(filename);
}
