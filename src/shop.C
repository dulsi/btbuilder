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

