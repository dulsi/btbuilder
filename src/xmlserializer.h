#ifndef __XMLSERIALIZER_H
#define __XMLSERIALIZER_H
/*-------------------------------------------------------------------------*\
  <xmlserializer.h> -- XMLSerializer header file

  Date      Programmer  Description
  01/27/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "expatcpp.h"
#include "bitfield.h"
#include "valuelookup.h"
#include <vector>
#include <list>
#include <string>

class ObjectSerializer;

class XMLObject
{
 public:
  typedef XMLObject* (*create)();

  virtual void serialize(ObjectSerializer* s) = 0;
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
#define XMLTYPE_OBJECT 4
#define XMLTYPE_BOOL 5
#define XMLTYPE_UINT 6
#define XMLTYPE_BITFIELD 7

class XMLAction
{
 public:
  ~XMLAction() { if (attrib) delete attrib; }

  std::string name;
  std::vector<XMLAttribute> *attrib;
  int type;
  int level;
  void *object;
  void *data;
};

class XMLLevel
{
 public:
  XMLAction *state;
  XMLObject *object;
};

class ObjectSerializer
{
 public:
  ObjectSerializer();
  ~ObjectSerializer();

  void add(const char *name, XMLArray* vec, XMLObject::create func, std::vector<XMLAttribute> *atts = NULL);
  void add(const char *name, XMLObject* p, std::vector<XMLAttribute> *atts = NULL);
  void add(const char *name, bool *p, std::vector<XMLAttribute> *atts = NULL);
  void add(const char *name, int *p, std::vector<XMLAttribute> *atts = NULL, ValueLookup *lookup = NULL);
  void add(const char *name, unsigned int *p, std::vector<XMLAttribute> *atts = NULL);
  void add(const char *name, char **p, std::vector<XMLAttribute> *atts = NULL);
  void add(const char *name, BitField *p, ValueLookup *lookup, std::vector<XMLAttribute> *atts = NULL);

  XMLAction* find(const XML_Char *name, const XML_Char **atts);
  virtual int getLevel() = 0;
  void removeLevel();

 private:
  std::vector<XMLAction*> action;
};

class XMLSerializer : public ObjectSerializer, public ExpatXMLParser
{
 public:
  XMLSerializer();
  ~XMLSerializer();

  virtual int getLevel();

  virtual void startElement(const XML_Char *name, const XML_Char **atts);
  virtual void endElement(const XML_Char *name);
  virtual void characterData(const XML_Char *s, int len);

 private:
  std::list<XMLLevel*> level;
  XMLAction *state;
};

#endif
