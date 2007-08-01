#ifndef __FACTORY_H
#define __FACTORY_H
/*-------------------------------------------------------------------------*\
  <factory.h> -- Fatory header file.

  Date      Programmer  Description
  11/23/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "istdlib.h"
#include <vector>

class BTArrayBoundsException
{
};

template <class item>
class BTFactory
{
 public:
  BTFactory(const char *filename);
  ~BTFactory();

  void save(const char *filename);
  IShort size();
  item &operator[](IShort num);

 private:
  std::vector<item*> items;
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
  void resort();
  IShort size();
  item &operator[](IShort num);

 private:
  BTFactory<item> *factory;
  const BTSortCompare<item> *compare;
  std::vector<IUShort> sortedItems;
};

template <class item>
BTFactory<item>::BTFactory(const char *filename)
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

template <class item>
BTFactory<item>::~BTFactory()
{
// Not sure why gcc doesn't accept this
// for (std::vector<item*>::iterator itr(items.begin()); itr != items.end(); itr++)
//  delete *itr;
 for (size_t i = 0; i < items.size(); i++)
 {
  delete items[i];
 }
}

template <class item>
void BTFactory<item>::save(const char *filename)
{
 BinaryReadFile f(filename);
// Not sure why gcc doesn't accept this
// for (std::vector<item_ptr>::iterator itr(items.begin()); itr != items.end(); itr++)
//  itr->write(f);
 for (size_t i; i < items.size(); i++)
 {
  items[i]->write(f);
 }
}

template <class item>
IShort BTFactory<item>::size()
{
 return items.size();
}

template <class item>
item &BTFactory<item>::operator[](IShort num)
{
 if (num >= items.size())
 {
  for (int i = items.size(); i < num; i++)
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
void BTSortedFactory<item>::resort()
{
 IUShort where;
 sortedItems.clear();
 for (IUShort i = 0; i < factory->size(); i++)
 {
  for (where = 0;
    (where < sortedItems.size()) && (compare->Compare((*factory)[i], (*factory)[sortedItems[where]]) >= 0);
    where++)
  {
  }
  if (where == sortedItems.size())
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
IShort BTSortedFactory<item>::size()
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

