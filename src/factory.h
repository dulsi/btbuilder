#ifndef __FACTORY_H
#define __FACTORY_H
/*-------------------------------------------------------------------------*\
  <factory.h> -- Fatory header file.

  Date      Programmer  Description
  11/23/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "istdlib.h"
#include "valuelookup.h"
#include "xmlserializer.h"

class BTArrayBoundsException
{
};

template <class item>
class BTFactory : public ValueLookup
{
 public:
  BTFactory(const char *e);
  ~BTFactory();

  int copy(int index);
  int find(item *obj);
  std::string getName(int index);
  int getIndex(std::string name);
  void load(const char *filename);
  void save(const char *filename);
  size_t size();
  item &operator[](IShort num);

 private:
  std::string extension;
  XMLVector<item*> items;
};

template<class item>
class BTSortCompare
{
 public:
  virtual int Compare(const item &a, const item &b) const = 0;
};

template<class item>
class BTSortedFactory
{
 public:
  BTSortedFactory(BTFactory<item> *fact, const BTSortCompare<item> *comp);
  int getSortedIndex(int index);
  int getUnsortedIndex(int index);
  void resort();
  size_t size();
  item &operator[](IShort num);

 private:
  BTFactory<item> *factory;
  const BTSortCompare<item> *compare;
  std::vector<IUShort> sortedItems;
};

template <class item>
BTFactory<item>::BTFactory(const char *e)
 : extension(e)
{
}

template <class item>
BTFactory<item>::~BTFactory()
{
}

template <class item>
int BTFactory<item>::copy(int index)
{
 items.push_back(new item((*this)[index]));
 return items.size() - 1;
}

template <class item>
int BTFactory<item>::find(item *obj)
{
 for (size_t i = 0; i < items.size(); i++)
 {
  if (items[i] == obj)
   return i;
 }
 return items.size();
}

template <class item>
std::string BTFactory<item>::getName(int index)
{
 return items[index]->getName();
}

template <class item>
int BTFactory<item>::getIndex(std::string name)
{
 for (size_t i = 0; i < items.size(); i++)
 {
  if (items[i]->getName() == name)
   return i;
 }
 return -1;
}

#include <stdio.h>
template <class item>
void BTFactory<item>::load(const char *filename)
{
 int len = strlen(filename);
 if ((len > extension.length()) && (strcmp(extension.c_str(), filename + (len - extension.length())) == 0))
 {
  BinaryReadFile f(filename);
  try {
   while (true)
   {
    items.push_back(new item(f));
   }
  }
  catch (FileException e)
  {
  }
 }
 else
 {
  item::readXML(filename, items);
 }
}

template <class item>
void BTFactory<item>::save(const char *filename)
{
 int len = strlen(filename);
 if ((len > extension.length()) && (strcmp(extension.c_str(), filename + (len - extension.length())) == 0))
 {
  BinaryWriteFile f(filename);
//  Not sure why gcc doesn't accept this
//  for (std::vector<item_ptr>::iterator itr(items.begin()); itr != items.end(); itr++)
//   itr->write(f);
  for (size_t i = 0; i < items.size(); i++)
  {
   items[i]->write(f);
  }
 }
 else
 {
  item::writeXML(filename, items);
 }
}

template <class item>
size_t BTFactory<item>::size()
{
 return items.size();
}

template <class item>
item &BTFactory<item>::operator[](IShort num)
{
 if (num >= items.size())
 {
  for (int i = items.size(); i <= num; i++)
  {
   items.push_back(new item);
  } 
 }
 return *items[num];
}

template <class item>
BTSortedFactory<item>::BTSortedFactory(BTFactory<item> *fact, const BTSortCompare<item> *comp)
 : factory(fact), compare(comp)
{
 resort();
}

template <class item>
int BTSortedFactory<item>::getSortedIndex(int index)
{
 for (int i = 0; i < sortedItems.size(); ++i)
 {
  if (index == sortedItems[i])
  {
   return i;
  }
 }
 throw BTArrayBoundsException();
}

template <class item>
int BTSortedFactory<item>::getUnsortedIndex(int index)
{
 if (index < sortedItems.size())
 {
  return sortedItems[index];
 }
 return sortedItems.size();
}

template <class item>
void BTSortedFactory<item>::resort()
{
 IUShort where;
 sortedItems.clear();
 for (IUShort i = 0; i < factory->size(); i++)
 {
  if (compare)
  {
   for (where = 0;
     (where < sortedItems.size()) && (compare->Compare((*factory)[i], (*factory)[sortedItems[where]]) >= 0);
     where++)
   {
   }
  }
  if ((NULL == compare) || (where == sortedItems.size()))
  {
   sortedItems.push_back(i);
  }
  else
  {
   sortedItems.push_back(sortedItems.back());
   for (IShort last = sortedItems.size() - 2; last > where; last--)
   {
    sortedItems[last] = sortedItems[last - 1];
   }
   sortedItems[where] = i;
  }
 }
}

template <class item>
size_t BTSortedFactory<item>::size()
{
 return sortedItems.size();
}

template <class item>
item &BTSortedFactory<item>::operator[](IShort num)
{
 if (num >= sortedItems.size())
 {
  throw BTArrayBoundsException();
 }
 return (*factory)[sortedItems[num]];
}

#endif

