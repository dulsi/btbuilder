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
#include <stdint.h>
#include <physfs.h>

class ObjectSerializer;

class XMLObject
{
 public:
  typedef XMLObject* (*create)(const XML_Char *name, const XML_Char **atts);

  virtual ~XMLObject() {}

  virtual std::string createString() { return ""; }

  virtual void serialize(ObjectSerializer* s) = 0;

  virtual void elementData(const XML_Char *name, const XML_Char **atts) {}
  virtual void characterData(const XML_Char *s, int len) {}
};

class XMLArray
{
 public:
  virtual void erase(size_t i) = 0;
  virtual XMLObject *get(size_t i) = 0;
  virtual void push_back(XMLObject *obj) = 0;
  virtual size_t size() const = 0;
};

template<class T>
class XMLVector : public XMLArray, public std::vector<T>
{
 public:
  XMLVector(bool c = true) : clear(c) {}
  ~XMLVector()
  {
   if (clear)
   {
    for (int i = 0; i < size(); ++i)
    {
     if (std::vector<T>::operator[](i))
     {
      delete std::vector<T>::operator[](i);
     }
    }
   }
  }

  virtual XMLObject *get(size_t i) { return (*this)[i]; }
  virtual void push_back(XMLObject *obj) { std::vector<T>::push_back(static_cast<T>(obj)); }
  void push_back(T obj) { std::vector<T>::push_back(obj); }
  typename std::vector<T>::iterator erase(typename std::vector<T>::iterator position) { if (clear) { delete *position; } std::vector<T>::erase(position); }
  typename std::vector<T>::iterator erase(typename std::vector<T>::iterator position, bool clearOveride) { if (clearOveride) { delete *position; } std::vector<T>::erase(position); }
  typename std::vector<T>::iterator erase(typename std::vector<T>::iterator first, typename std::vector<T>::iterator last) { if (clear) { for (typename std::vector<T>::iterator current = first; current != last; current++) delete *current; } std::vector<T>::erase(first, last); }
  void erase(size_t i) { erase(this->begin() + i); }
  virtual size_t size() const { return std::vector<T>::size(); }

 private:
  bool clear;
};

class XMLAttribute
{
 public:
  XMLAttribute(const char *n, const char *v) : name(n), value(v) {}

  std::string name;
  std::string value;
};

class PictureIndex
{
 public:
  PictureIndex() : value(0) {}

  int value;
};

#define XMLTYPE_TYPE   0x0FFF
#define XMLTYPE_DELETE 0x1000
#define XMLTYPE_CREATE     1
#define XMLTYPE_INT        2
#define XMLTYPE_STRING     3
#define XMLTYPE_OBJECT     4
#define XMLTYPE_BOOL       5
#define XMLTYPE_UINT       6
#define XMLTYPE_BITFIELD   7
#define XMLTYPE_INT16      8
#define XMLTYPE_UINT16     9
#define XMLTYPE_STDSTRING  10
#define XMLTYPE_VECTORUINT 11
#define XMLTYPE_VECTORSTRING 12
#define XMLTYPE_PICTURE    13

#define EXTRA_NONE -999

class XMLAction
{
 public:
  XMLAction() : attrib(NULL), type(0), next(NULL), extra(EXTRA_NONE) {}
  ~XMLAction();

  std::string createTag();
  std::string createString();
  int getType() const { return type & XMLTYPE_TYPE; }

  std::string name;
  std::string objnm;
  std::vector<XMLAttribute> *attrib;
  int type;
  int level;
  void *object;
  void *data;
  int extra;
  std::string extraText;
  XMLAction *next;
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

  void add(const char *name, XMLArray *vec, XMLObject::create func, std::vector<XMLAttribute> *atts = NULL) { add(name, NULL, vec, func, atts); }
  void add(const char *name, const char *objnm, XMLArray *vec, XMLObject::create func, std::vector<XMLAttribute> *atts = NULL);
  void add(const char *name, XMLObject* p, std::vector<XMLAttribute> *atts = NULL);
  void add(const char *name, bool *p, std::vector<XMLAttribute> *atts = NULL, bool delFlg = false);
  void add(const char *name, int *p, std::vector<XMLAttribute> *atts = NULL, ValueLookup *lookup = NULL, int extra = EXTRA_NONE, const std::string &extraText = std::string());
  void add(const char *name, unsigned int *p, std::vector<XMLAttribute> *atts = NULL, bool delFlg = false);
  void add(const char *name, int16_t *p, std::vector<XMLAttribute> *atts = NULL);
  void add(const char *name, uint16_t *p, std::vector<XMLAttribute> *atts = NULL);
  void add(const char *name, char **p, std::vector<XMLAttribute> *atts = NULL);
  void add(const char *name, std::string *p, std::vector<XMLAttribute> *atts = NULL, bool delFlg = false);
  void add(const char *name, BitField *p, ValueLookup *lookup, std::vector<XMLAttribute> *atts = NULL);
  void add(const char *name, std::vector<unsigned int> *p, ValueLookup *lookup = NULL, std::vector<XMLAttribute> *atts = NULL);
  void add(const char *name, std::vector<std::string> *p, std::vector<XMLAttribute> *atts = NULL);
  void add(const char *name, PictureIndex *p, std::vector<XMLAttribute> *atts = NULL);

  void addLevel(XMLLevel *newLevel);
  XMLAction *find(const char *name, const char **atts);
  int getLevel();
  XMLLevel *removeLevel();
  void setNamespace(std::string newNS);

 protected:
  std::vector<XMLAction*> action;
  std::list<XMLLevel*> level;
  std::string ns;
};

class XMLSerializer : public ObjectSerializer, public ExpatXMLParser
{
 public:
  XMLSerializer();
  ~XMLSerializer();

  virtual void startElement(const XML_Char *name, const XML_Char **atts);
  virtual void endElement(const XML_Char *name);
  virtual void characterData(const XML_Char *s, int len);

  void write(const char *filename, bool physfs);

 protected:
  void write(PHYSFS_file *physFile, FILE *file, const char *content, size_t len);

 private:
  XMLAction *state;
  std::string content;
};

#endif

