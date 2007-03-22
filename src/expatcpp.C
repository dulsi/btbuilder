/*-------------------------------------------------------------------------*\
  <expatcpp.C> -- Expat C++ implementation file

  Date      Programmer  Description
  01/27/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "expatcpp.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <physfs.h>

#define XMLBUFFER 2048

ExpatXMLParser::ExpatXMLParser()
{
 p = XML_ParserCreate(NULL);
 XML_SetUserData(p, this);
 XML_SetElementHandler(p, startElement, endElement);
 XML_SetCharacterDataHandler(p, characterData);
}

void ExpatXMLParser::parse(const char *filename, bool physfs)
{
 char *buffer = new char[XMLBUFFER];
 if (physfs)
 {
  PHYSFS_file *f = PHYSFS_openRead(filename);
  while (true)
  {
   PHYSFS_sint64 num = PHYSFS_read(f, buffer, 1, XMLBUFFER);
   XML_Parse(p, buffer, num, ((0 == num) || (PHYSFS_eof(f))));
   if ((0 == num) || (PHYSFS_eof(f)))
    break;
  }
  PHYSFS_close(f);
 }
 else
 {
  int fd = open(filename, O_RDONLY);
  if (-1 == fd)
  {
   delete [] buffer;
   return; // Throw exception
  }
  while (true)
  {
   size_t num = read(fd, buffer, XMLBUFFER);
   XML_Parse(p, buffer, num, (0 == num));
   if (0 == num)
    break;
  }
  close(fd);
 }
 delete [] buffer;
}

void ExpatXMLParser::startElement(void *userData, const XML_Char *name, const XML_Char **atts)
{
 ExpatXMLParser *parser = (ExpatXMLParser*)userData;
 parser->startElement(name, atts);
}

void ExpatXMLParser::endElement(void *userData, const XML_Char *name)
{
 ExpatXMLParser *parser = (ExpatXMLParser*)userData;
 parser->endElement(name);
}

void ExpatXMLParser::characterData(void *userData, const XML_Char *s, int len)
{
 ExpatXMLParser *parser = (ExpatXMLParser*)userData;
 parser->characterData(s, len);
}
