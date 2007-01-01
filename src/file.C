/*-------------------------------------------------------------------------*\
  <file.c> -- Binary file opperations
    (Least significant byte first version)

  Date      Programmer  Description
  03/20/98  Dennis      Created.
\*-------------------------------------------------------------------------*/

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  Includes
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#include "file.h"

void SwapBytes(IUByte *value, int size)
{
 IUByte first;
 first = *value;
 for (int i = 1; i < size; i++)
 {
  value[i - 1] = value[i];
 }
 value[size - 1] = first;
}

FileException::FileException()
{
}

FileList::FileList(const char *pattern)
{
 if (glob(pattern, 0, NULL, &results))
 {
  results.gl_pathc = 0;
 }
}

FileList::~FileList()
{
 if (results.gl_pathc)
 {
  globfree(&results);
 }
}

IUShort FileList::length()
{
 return results.gl_pathc;
}

char *FileList::operator[](int num)
{
 return results.gl_pathv[num];
}

BinaryReadFile::BinaryReadFile()
{
 file = (FILE *)0;
 swap = false;
}

BinaryReadFile::BinaryReadFile(const char *filename)
{
 file = fopen(filename, "rb");
 swap = false;
}

BinaryReadFile::~BinaryReadFile()
{
 if (file) fclose(file);
}

void BinaryReadFile::close()
{
 if (file) fclose(file);
}

void BinaryReadFile::open(const char *filename)
{
 if (file) close();
 file = fopen(filename, "rb");
 swap = false;
}

void BinaryReadFile::readByte(IByte &a)
{
 size_t ans = fread(&a, 1, 1, file);
 if (ans != 1)
 {
  throw FileException();
 }
}

void BinaryReadFile::readByteArray(const int size, IByte *a)
{
 size_t ans = fread(a, 1, size, file);
 if (ans != size)
 {
  throw FileException();
 }
}

void BinaryReadFile::readShort(IShort &a)
{
 size_t ans = fread(&a, 2, 1, file);
 if (ans != 1)
 {
  throw FileException();
 }
 if (swap)
 {
  SwapBytes((IUByte *)&a, 2);
 }
}

void BinaryReadFile::readShortArray(const int size, IShort *a)
{
 size_t ans = fread(a, 2, size, file);
 if (ans != size)
 {
  throw FileException();
 }
 if (swap)
 {
  for (int i = 0; i < size; i++)
  {
   SwapBytes((IUByte *)(a + i), 2);
  }
 }
}

void BinaryReadFile::readLong(ILong &a)
{
 size_t ans = fread(&a, 4, 1, file);
 if (ans != 1)
 {
  throw FileException();
 }
 if (swap)
 {
  SwapBytes((IUByte *)&a, 4);
 }
}

void BinaryReadFile::readLongArray(const int size, ILong *a)
{
 size_t ans = fread(a, 4, size, file);
 if (ans != size)
 {
  throw FileException();
 }
 if (swap)
 {
  for (int i = 0; i < size; i++)
  {
   SwapBytes((IUByte *)(a + i), 4);
  }
 }
}

void BinaryReadFile::readUByte(IUByte &a)
{
 size_t ans = fread(&a, 1, 1, file);
 if (ans != 1)
 {
  throw FileException();
 }
}

void BinaryReadFile::readUByteArray(const int size, IUByte *a)
{
 size_t ans = fread(a, 1, size, file);
 if (ans != size)
 {
  throw FileException();
 }
}

void BinaryReadFile::readUShort(IUShort &a)
{
 size_t ans = fread(&a, 2, 1, file);
 if (ans != 1)
 {
  throw FileException();
 }
 if (swap)
 {
  SwapBytes((IUByte *)&a, 2);
 }
}

void BinaryReadFile::readUShortArray(const int size, IUShort *a)
{
 size_t ans = fread(a, 2, size, file);
 if (ans != size)
 {
  throw FileException();
 }
 if (swap)
 {
  for (int i = 0; i < size; i++)
  {
   SwapBytes((IUByte *)(a + i), 2);
  }
 }
}

void BinaryReadFile::readULong(IULong &a)
{
 size_t ans = fread(&a, 4, 1, file);
 if (ans != 1)
 {
  throw FileException();
 }
 if (swap)
 {
  SwapBytes((IUByte *)&a, 4);
 }
}

void BinaryReadFile::readULongArray(const int size, IULong *a)
{
 size_t ans = fread(a, 4, size, file);
 if (ans != size)
 {
  throw FileException();
 }
 if (swap)
 {
  for (int i = 0; i < size; i++)
  {
   SwapBytes((IUByte *)(a + i), 4);
  }
 }
}

void BinaryReadFile::seek(IULong where)
{
 clearerr(file);
 fseek(file, where, SEEK_SET);
}

void BinaryReadFile::setSwap(bool value)
{
 swap = value;
}

BinaryWriteFile::BinaryWriteFile()
{
 file = (FILE *)0;
}

BinaryWriteFile::BinaryWriteFile(const char *filename)
{
 file = fopen(filename, "wb");
}

BinaryWriteFile::~BinaryWriteFile()
{
 if (file) fclose(file);
}

void BinaryWriteFile::close()
{
 if (file) fclose(file);
}

void BinaryWriteFile::open(const char *filename)
{
 if (file) close();
 file = fopen(filename, "wb");
}

void BinaryWriteFile::writeByte(const IByte &a)
{
 fwrite(&a, 1, 1, file);
}

void BinaryWriteFile::writeByteArray(const int size, const IByte *a)
{
 fwrite(a, 1, size, file);
}

void BinaryWriteFile::writeShort(const IShort &a)
{
 fwrite(&a, 2, 1, file);
}

void BinaryWriteFile::writeShortArray(const int size, const IShort *a)
{
 fwrite(a, 2, size, file);
}

void BinaryWriteFile::writeLong(const ILong &a)
{
 fwrite(&a, 4, 1, file);
}

void BinaryWriteFile::writeLongArray(const int size, const ILong *a)
{
 fwrite(a, 4, size, file);
}

void BinaryWriteFile::writeUByte(const IUByte &a)
{
 fwrite(&a, 1, 1, file);
}

void BinaryWriteFile::writeUByteArray(const int size, const IUByte *a)
{
 fwrite(a, 1, size, file);
}

void BinaryWriteFile::writeUShort(const IUShort &a)
{
 fwrite(&a, 2, 1, file);
}

void BinaryWriteFile::writeUShortArray(const int size, const IUShort *a)
{
 fwrite(a, 2, size, file);
}

void BinaryWriteFile::writeULong(const IULong &a)
{
 fwrite(&a, 4, 1, file);
}

void BinaryWriteFile::writeULongArray(const int size, const IULong *a)
{
 fwrite(a, 4, size, file);
}

void BinaryWriteFile::seek(const IULong where)
{
 fseek(file, where, SEEK_SET);
}

