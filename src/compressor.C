/*-------------------------------------------------------------------------*\
  <compressor.c> -- Compress/decompress binary file opperations

  Date      Programmer  Description
  12/01/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  Includes
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#include "compressor.h"

extern void SwapBytes(IUByte *value, int size);

BTCompressorReadFile::BTCompressorReadFile()
{
 swap = false;
 data = (IUByte *)IMalloc(4096);
 IMemset(data, 0, 4096);
 where = 0x0FEE;
 length = 0;
}

BTCompressorReadFile::BTCompressorReadFile(const char *filename)
 : file(filename)
{
 swap = false;
 data = (IUByte *)IMalloc(4096);
 IMemset(data, 0, 4096);
 where = 0x0FEE;
 length = 0;
}

BTCompressorReadFile::~BTCompressorReadFile()
{
 free(data);
}

void BTCompressorReadFile::close()
{
 file.close();
}

void BTCompressorReadFile::open(const char *filename)
{
 file.close();
 file.open(filename);
 swap = false;
 where = 0x0FEE;
 length = 0;
}

void BTCompressorReadFile::readByte(IByte &a)
{
 readData(1, 1, reinterpret_cast<IUByte *>(&a));
}

void BTCompressorReadFile::readByteArray(const int size, IByte *a)
{
 readData(size, 1, reinterpret_cast<IUByte *>(a));
}

void BTCompressorReadFile::readShort(IShort &a)
{
 readData(1, 2, reinterpret_cast<IUByte *>(&a));
}

void BTCompressorReadFile::readShortArray(const int size, IShort *a)
{
 readData(size, 2, reinterpret_cast<IUByte *>(a));
}

void BTCompressorReadFile::readLong(ILong &a)
{
 readData(1, 4, reinterpret_cast<IUByte *>(&a));
}

void BTCompressorReadFile::readLongArray(const int size, ILong *a)
{
 readData(size, 4, reinterpret_cast<IUByte *>(a));
}

void BTCompressorReadFile::readUByte(IUByte &a)
{
 readData(1, 1, &a);
}

void BTCompressorReadFile::readUByteArray(const int size, IUByte *a)
{
 readData(size, 1, a);
}

void BTCompressorReadFile::readUShort(IUShort &a)
{
 readData(1, 2, reinterpret_cast<IUByte *>(&a));
}

void BTCompressorReadFile::readUShortArray(const int size, IUShort *a)
{
 readData(size, 2, reinterpret_cast<IUByte *>(a));
}

void BTCompressorReadFile::readULong(IULong &a)
{
 readData(1, 4, reinterpret_cast<IUByte *>(&a));
}

void BTCompressorReadFile::readULongArray(const int size, IULong *a)
{
 readData(size, 4, reinterpret_cast<IUByte *>(a));
}

void BTCompressorReadFile::readData(const int number, const int size, IUByte *a)
{
 int i, k;
 for (i = number * size; i > 0;)
 {
  if (0 == length)
  {
   IUByte format;

   file.readUByte(format);
   try {
    for (k = 0; k < 8; k++, format >>= 1)
    {
     if (format & 0x01)
     {
      file.readUByte(data[(where + length) & 0x0FFF]);
      length++;
     }
     else
     {
      IUByte tmpData[2];
      IUShort location;
      file.readUByteArray(2, tmpData);
      location = tmpData[0] + ((tmpData[1] & 0xF0) << 4);
      tmpData[1] = (tmpData[1] & 0x0F) + 3;
      for (int count = 0; count < tmpData[1]; count++, length++)
      {
       data[(where + length) & 0x0FFF] = data[(location + count) & 0x0FFF];
      }
     }
    }
   }
   catch (FileException e)
   {
   }
  }
  IULong amount = ((i > length) ? length : i);
  if (where + amount > 4096)
  {
   memcpy(a, data + where, 4096 - where);
   memcpy(a + (4096 - where), data, (where + amount) & 0x0FFF);
  }
  else
   memcpy(a, data + where, amount);
  where = (where + amount) & 0x0FFF;
  length -= amount;
  i -= amount;
  a += amount;
 }
 if (swap)
 {
  for (i = 0; i < number; i++)
  {
   SwapBytes(a + (i * size), size);
  }
 }
}

void BTCompressorReadFile::setSwap(bool value)
{
 swap = value;
}

BTCompressorWriteFile::BTCompressorWriteFile()
{
 count = 0;
}

BTCompressorWriteFile::BTCompressorWriteFile(const char *filename)
 : file(filename)
{
 count = 0;
}

BTCompressorWriteFile::~BTCompressorWriteFile()
{
}

void BTCompressorWriteFile::close()
{
 file.close();
}

void BTCompressorWriteFile::open(const char *filename)
{
 file.close();
 file.open(filename);
 count = 0;
}

void BTCompressorWriteFile::writeByte(const IByte &a)
{
 writeData(1, (const IUByte *)&a);
}

void BTCompressorWriteFile::writeByteArray(const int size, const IByte *a)
{
 writeData(size, (const IUByte *)a);
}

void BTCompressorWriteFile::writeShort(const IShort &a)
{
 writeData(2, (const IUByte *)&a);
}

void BTCompressorWriteFile::writeShortArray(const int size, const IShort *a)
{
 writeData(2 * size, (const IUByte *)a);
}

void BTCompressorWriteFile::writeLong(const ILong &a)
{
 writeData(4, (const IUByte *)&a);
}

void BTCompressorWriteFile::writeLongArray(const int size, const ILong *a)
{
 writeData(4 * size, (const IUByte *)a);
}

void BTCompressorWriteFile::writeUByte(const IUByte &a)
{
 writeData(1, &a);
}

void BTCompressorWriteFile::writeUByteArray(const int size, const IUByte *a)
{
 writeData(size, a);
}

void BTCompressorWriteFile::writeUShort(const IUShort &a)
{
 writeData(2, (const IUByte *)&a);
}

void BTCompressorWriteFile::writeUShortArray(const int size, const IUShort *a)
{
 writeData(2 * size, (const IUByte *)a);
}

void BTCompressorWriteFile::writeULong(const IULong &a)
{
 writeData(4, (const IUByte *)&a);
}

void BTCompressorWriteFile::writeULongArray(const int size, const IULong *a)
{
 writeData(4 * size, (const IUByte *)a);
}

void BTCompressorWriteFile::writeData(const int size, const IUByte *a)
{
 IUByte format(0xFF);
 int where = 0;
 while (where < size)
 {
  if (count == 0)
  {
   file.writeUByte(format);
   count++;
  }
  IUShort amount = ((size - where > 9 - count) ? 9 - count : size - where);
  file.writeUByteArray(amount, a + where);
  where += amount;
  count += amount;
  if (9 == count)
   count = 0;
 }
}

