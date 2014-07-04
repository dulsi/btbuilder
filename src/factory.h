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

template <typename item, typename item1 = item>
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

template<typename item>
class BTSortCompare
{
 public:
  virtual int Compare(const item &a, const item &b) const = 0;
};

template<typename item, typename item1 = item>
class BTSortedFactory
{
 public:
  BTSortedFactory(BTFactory<item, item1> *fact, const BTSortCompare<item> *comp);
  int getSortedIndex(int index);
  int getUnsortedIndex(int index);
  void resort();
  size_t size();
  item &operator[](IShort num);

 private:
  BTFactory<item, item1> *factory;
  const BTSortCompare<item> *compare;
  std::vector<IUShort> sortedItems;
};

template <typename item, typename item1>
BTFactory<item, item1>::BTFactory(const char *e)
 : extension(e)
{
}

template <typename item, typename item1>
BTFactory<item, item1>::~BTFactory()
{
}

template <typename item, typename item1>
int BTFactory<item, item1>::copy(int index)
{
 items.push_back(new item((*this)[index]));
 return items.size() - 1;
}

template <typename item, typename item1>
int BTFactory<item, item1>::find(item *obj)
{
 for (size_t i = 0; i < items.size(); i++)
 {
  if (items[i] == obj)
   return i;
 }
 return items.size();
}

template <typename item, typename item1>
std::string BTFactory<item, item1>::getName(int index)
{
 return items[index]->getName();
}

template <typename item, typename item1>
int BTFactory<item, item1>::getIndex(std::string name)
{
 for (size_t i = 0; i < items.size(); i++)
 {
  if (items[i]->getName() == name)
   return i;
 }
 return -1;
}

#include <stdio.h>
template <typename item, typename item1>
void BTFactory<item, item1>::load(const char *filename)
{
 int len = strlen(filename);
 if ((len > extension.length()) && (strcmp(extension.c_str(), filename + (len - extension.length())) == 0))
 {
  BinaryReadFile f(filename);
  try {
   while (true)
   {
    item1 *r = new item1(f);
    items.push_back(r);
    r->upgrade();
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

template <typename item, typename item1>
void BTFactory<item, item1>::save(const char *filename)
{
 int len = strlen(filename);
 if ((len > extension.length()) && (strcmp(extension.c_str(), filename + (len - extension.length())) == 0))
 {
  try
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
  catch (const FileException &e)
  {
   PHYSFS_delete(filename);
   printf("Failed to write old map file: %s\n", e.what());
   char tmp[len + 1];
   strcpy(tmp, filename);
   strcpy(tmp + len - 3, "xml");
   item::writeXML(tmp, items);
  }
 }
 else
 {
  item::writeXML(filename, items);
 }
}

template <typename item, typename item1>
size_t BTFactory<item, item1>::size()
{
 return items.size();
}

template <typename item, typename item1>
item &BTFactory<item, item1>::operator[](IShort num)
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

template <typename item, typename item1>
BTSortedFactory<item, item1>::BTSortedFactory(BTFactory<item, item1> *fact, const BTSortCompare<item> *comp)
 : factory(fact), compare(comp)
{
 resort();
}

template <typename item, typename item1>
int BTSortedFactory<item, item1>::getSortedIndex(int index)
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

template <typename item, typename item1>
int BTSortedFactory<item, item1>::getUnsortedIndex(int index)
{
 if (index < sortedItems.size())
 {
  return sortedItems[index];
 }
 return sortedItems.size();
}

template <typename item, typename item1>
void BTSortedFactory<item, item1>::resort()
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

template <typename item, typename item1>
size_t BTSortedFactory<item, item1>::size()
{
 return sortedItems.size();
}

template <typename item, typename item1>
item &BTSortedFactory<item, item1>::operator[](IShort num)
{
 if (num >= sortedItems.size())
 {
  throw BTArrayBoundsException();
 }
 return (*factory)[sortedItems[num]];
}

#endif

