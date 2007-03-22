#ifndef __COMPRESSOR_H
#define __COMPRESSOR_H
/*-------------------------------------------------------------------------*\
  <compressor.h> -- Compress/decompress binary file opperations.

  Date      Programmer  Description
  12/01/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  Includes
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#include "file.h"

class BTCompressorReadFile
{
 public:
  BTCompressorReadFile();
  BTCompressorReadFile(const char *filename);
  ~BTCompressorReadFile();
  void close();
  void open(const char *filename);
  void readByte(IByte &a);
  void readByteArray(const int size, IByte *a);
  void readShort(IShort &a);
  void readShortArray(const int size, IShort *a);
  void readLong(ILong &a);
  void readLongArray(const int size, ILong *a);
  void readUByte(IUByte &a);
  void readUByteArray(const int size, IUByte *a);
  void readUShort(IUShort &a);
  void readUShortArray(const int size, IUShort *a);
  void readULong(IULong &a);
  void readULongArray(const int size, IULong *a);
  void setSwap(bool value);

 private:
  void readData(const int number, const int size, IUByte *a);

  BinaryReadFile file;
  bool swap;
  IUByte *data;
  IULong where;
  IULong length;
};

class BTCompressorWriteFile
{
 public:
  BTCompressorWriteFile();
  BTCompressorWriteFile(const char *filename);
  ~BTCompressorWriteFile();
  void close();
  void open(const char *filename);
  void writeByte(const IByte &a);
  void writeByteArray(const int size, const IByte *a);
  void writeShort(const IShort &a);
  void writeShortArray(const int size, const IShort *a);
  void writeLong(const ILong &a);
  void writeLongArray(const int size, const ILong *a);
  void writeUByte(const IUByte &a);
  void writeUByteArray(const int size, const IUByte *a);
  void writeUShort(const IUShort &a);
  void writeUShortArray(const int size, const IUShort *a);
  void writeULong(const IULong &a);
  void writeULongArray(const int size, const IULong *a);

 private:
  void writeData(const int size, const IUByte *a);

  int count;
  BinaryWriteFile file;
};

#endif

