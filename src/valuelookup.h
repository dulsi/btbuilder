#ifndef __VALUELOOKUP_H
#define __VALUELOOKUP_H
/*-------------------------------------------------------------------------*\
  <valuelookup.h> -- ValueLookup header file

  Date      Programmer  Description
  03/24/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include <string>

class ValueLookup
{
 public:
  virtual std::string getName(int index) = 0;
  virtual int getIndex(std::string name) = 0;
  virtual size_t size() = 0;
  virtual int getEndIndex();
  virtual int getFirstIndex();
  virtual int getNextIndex(int previous);
};

class ArrayLookup : public ValueLookup
{
 public:
  ArrayLookup(size_t s, char **a) : sz(s), array(a) {}

  virtual std::string getName(int index);
  virtual int getIndex(std::string name);
  virtual size_t size();

 private:
  size_t sz;
  char **array;
};

class PairLookup : public ValueLookup
{
 public:
  PairLookup(size_t s, int *v, char **n) : sz(s), value(v), names(n) {}

  virtual std::string getName(int index);
  virtual int getIndex(std::string nm);
  virtual size_t size();
  virtual int getEndIndex();
  virtual int getFirstIndex();
  virtual int getNextIndex(int previous);

 private:
  int getArrayFromIndex(int index);

 private:
  size_t sz;
  int *value;
  char **names;
};

#endif
