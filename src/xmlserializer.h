#ifndef __XMLSERIALIZER_H
#define __XMLSERIALIZER_H
/*-------------------------------------------------------------------------*\
  <xmlserializer.h> -- XMLSerializer header file

  Date      Programmer  Description
  01/27/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "expatcpp.h"
#include <vector>
#include <list>
#include <string>

class XMLSerializer;

class XMLObject
{
 public:
  typedef XMLObject* (*create)();

  virtual void serialize(XMLSerializer* s) = 0;
};

class XMLArray
{
 public:
  virtual void push_back(XMLObject *obj) = 0;
};

template<class T>
class XMLVector : public XMLArray, public std::vector<T>
{
 public:
  virtual void push_back(XMLObject *obj) { std::vector<T>::push_back(static_cast<T>(obj)); }
  void push_back(T obj) { std::vector<T>::push_back(obj); }
};

class XMLAttribute
{
 public:
  XMLAttribute(const char *n, const char *v) : name(n), value(v) {}

  std::string name;
  std::string value;
};

#define XMLTYPE_CREATE 1
#define XMLTYPE_INT 2
#define XMLTYPE_STRING 3

class XMLAction
{
 public:
  ~XMLAction() { if (attrib) delete attrib; }

  std::string name;
  std::vector<XMLAttribute> *attrib;
  int type;
  int level;
  void *object;
  XMLObject::create func;
};

class XMLLevel
{
 public:
  XMLAction *state;
  XMLObject *object;
};

class XMLSerializer : public ExpatXMLParser
{
 public:
  XMLSerializer();
  ~XMLSerializer();

  void add(const char *name, XMLObject::create func, XMLArray* vec, std::vector<XMLAttribute> *atts = NULL);
  void add(const char *name, int *p, std::vector<XMLAttribute> *atts = NULL);
  void add(const char *name, char **p, std::vector<XMLAttribute> *atts = NULL);

  virtual void startElement(const XML_Char *name, const XML_Char **atts);
  virtual void endElement(const XML_Char *name);
  virtual void characterData(const XML_Char *s, int len);

 private:
  std::vector<XMLAction*> action;
  std::list<XMLLevel*> level;
  XMLAction *state;
};

#endif
