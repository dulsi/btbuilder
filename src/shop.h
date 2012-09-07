#ifndef __SHOP_H
#define __SHOP_H
/*-------------------------------------------------------------------------*\
  <shop.h> -- Shops header file

  Date      Programmer  Description
  09/06/12  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "xmlserializer.h"

class BTShopGood : public XMLObject
{
 public:
  BTShopGood() : id(BTITEM_NONE), count(0) {}
  BTShopGood(int i, int c) : id(i), count(c) {}

  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTShopGood; }

  int id;
  int count;
};

class BTShop : public XMLObject
{
 public:
  BTShop() : id(0) {}

  void initDefault();

  void addItem(int id);
  void removeItem(int id);
  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTShop; }

  static void readXML(const char *filename, XMLVector<BTShop*> &shops);
  static void writeXML(const char *filename, XMLVector<BTShop*> &shops);

  int id;
  XMLVector<BTShopGood*> goods;
};

#endif

