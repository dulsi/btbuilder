/*-------------------------------------------------------------------------*\
  <shop.C> -- Shop goods implementation file

  Date      Programmer  Description
  09/06/12  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "shop.h"

void BTShopGood::serialize(ObjectSerializer* s)
{
 s->add("id", &id);
 s->add("count", &count);
}

void BTShop::initDefault()
{
 for (int i = 0; i < 9; ++i)
 {
  goods.push_back(new BTShopGood(i, BTITEMCOUNT_UNLIMITED));
 }
}

void BTShop::addItem(int id)
{
 for (int i = 0; i < goods.size(); ++i)
 {
  if (goods[i]->id == id)
  {
   if (goods[i]->count != BTITEMCOUNT_UNLIMITED)
   {
    if (BTITEMCOUNT_MAX < goods[i]->count)
    {
     goods[i]->count++;
    }
   }
   return;
  }
 }
 goods.push_back(new BTShopGood(id, 1));
}

void BTShop::removeItem(int id)
{
 for (int i = 0; i < goods.size(); ++i)
 {
  if (goods[i]->id == id)
  {
   if (goods[i]->count != BTITEMCOUNT_UNLIMITED)
   {
    goods[i]->count--;
    if (0 >= goods[i]->count)
    {
     goods.erase(goods.begin() + i);
    }
   }
   return;
  }
 }
}

void BTShop::serialize(ObjectSerializer* s)
{
 s->add("id", &id);
 s->add("good", &goods, &BTShopGood::create);
}

void BTShop::readXML(const char *filename, XMLVector<BTShop*> &shops)
{
 XMLSerializer parser;
 parser.add("shop", &shops, &BTShop::create);
 parser.parse(filename, true);
}

void BTShop::writeXML(const char *filename, XMLVector<BTShop*> &shops)
{
 XMLSerializer parser;
 parser.add("shop", &shops, &BTShop::create);
 parser.write(filename, true);
}

