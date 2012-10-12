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

FileException::FileException(const std::string &reason)
 : std::runtime_error(reason)
{
}

BinaryReadFile::BinaryReadFile()
{
 file = (PHYSFS_file *)0;
 swap = false;
}

BinaryReadFile::BinaryReadFile(const char *filename)
{
 file = PHYSFS_openRead(filename);
 swap = false;
}

BinaryReadFile::~BinaryReadFile()
{
 if (file) PHYSFS_close(file);
}

void BinaryReadFile::close()
{
 if (file)
 {
  PHYSFS_close(file);
  file = (PHYSFS_file *)0;
 }
}

void BinaryReadFile::open(const char *filename)
{
 if (file) close();
 file = PHYSFS_openRead(filename);
 swap = false;
}

void BinaryReadFile::readByte(IByte &a)
{
 size_t ans = PHYSFS_read(file, &a, 1, 1);
 if (ans != 1)
 {
  throw FileException("Data read failure");
 }
}

void BinaryReadFile::readByteArray(const int size, IByte *a)
{
 size_t ans = PHYSFS_read(file, a, 1, size);
 if (ans != size)
 {
  throw FileException("Data read failure");
 }
}

void BinaryReadFile::readShort(IShort &a)
{
 size_t ans = PHYSFS_read(file, &a, 2, 1);
 if (ans != 1)
 {
  throw FileException("Data read failure");
 }
 if (swap)
 {
  SwapBytes((IUByte *)&a, 2);
 }
}

void BinaryReadFile::readShortArray(const int size, IShort *a)
{
 size_t ans = PHYSFS_read(file, a, 2, size);
 if (ans != size)
 {
  throw FileException("Data read failure");
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
 size_t ans = PHYSFS_read(file, &a, 4, 1);
 if (ans != 1)
 {
  throw FileException("Data read failure");
 }
 if (swap)
 {
  SwapBytes((IUByte *)&a, 4);
 }
}

void BinaryReadFile::readLongArray(const int size, ILong *a)
{
 size_t ans = PHYSFS_read(file, a, 4, size);
 if (ans != size)
 {
  throw FileException("Data read failure");
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
 size_t ans = PHYSFS_read(file, &a, 1, 1);
 if (ans != 1)
 {
  throw FileException("Data read failure");
 }
}

void BinaryReadFile::readUByteArray(const int size, IUByte *a)
{
 size_t ans = PHYSFS_read(file, a, 1, size);
 if (ans != size)
 {
  throw FileException("Data read failure");
 }
}

void BinaryReadFile::readUShort(IUShort &a)
{
 size_t ans = PHYSFS_read(file, &a, 2, 1);
 if (ans != 1)
 {
  throw FileException("Data read failure");
 }
 if (swap)
 {
  SwapBytes((IUByte *)&a, 2);
 }
}

void BinaryReadFile::readUShortArray(const int size, IUShort *a)
{
 size_t ans = PHYSFS_read(file, a, 2, size);
 if (ans != size)
 {
  throw FileException("Data read failure");
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
 size_t ans = PHYSFS_read(file, &a, 4, 1);
 if (ans != 1)
 {
  throw FileException("Data read failure");
 }
 if (swap)
 {
  SwapBytes((IUByte *)&a, 4);
 }
}

void BinaryReadFile::readULongArray(const int size, IULong *a)
{
 size_t ans = PHYSFS_read(file, a, 4, size);
 if (ans != size)
 {
  throw FileException("Data read failure");
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
// clearerr(file);
 PHYSFS_seek(file, where);
}

void BinaryReadFile::setSwap(bool value)
{
 swap = value;
}

BinaryWriteFile::BinaryWriteFile()
{
 file = (PHYSFS_file *)0;
}

BinaryWriteFile::BinaryWriteFile(const char *filename)
{
 file = PHYSFS_openWrite(filename);
}

BinaryWriteFile::~BinaryWriteFile()
{
 if (file) PHYSFS_close(file);
}

void BinaryWriteFile::close()
{
 if (file)
 {
  PHYSFS_close(file);
  file = (PHYSFS_file *)0;
 }
}

void BinaryWriteFile::open(const char *filename)
{
 if (file) close();
 file = PHYSFS_openWrite(filename);
}

void BinaryWriteFile::writeByte(const IByte &a)
{
 PHYSFS_write(file, &a, 1, 1);
}

void BinaryWriteFile::writeByteArray(const int size, const IByte *a)
{
 PHYSFS_write(file, a, 1, size);
}

void BinaryWriteFile::writeShort(const IShort &a)
{
 PHYSFS_write(file, &a, 2, 1);
}

void BinaryWriteFile::writeShortArray(const int size, const IShort *a)
{
 PHYSFS_write(file, a, 2, size);
}

void BinaryWriteFile::writeLong(const ILong &a)
{
 PHYSFS_write(file, &a, 4, 1);
}

void BinaryWriteFile::writeLongArray(const int size, const ILong *a)
{
 PHYSFS_write(file, a, 4, size);
}

void BinaryWriteFile::writeUByte(const IUByte &a)
{
 PHYSFS_write(file, &a, 1, 1);
}

void BinaryWriteFile::writeUByteArray(const int size, const IUByte *a)
{
 PHYSFS_write(file, a, 1, size);
}

void BinaryWriteFile::writeUShort(const IUShort &a)
{
 PHYSFS_write(file, &a, 2, 1);
}

void BinaryWriteFile::writeUShortArray(const int size, const IUShort *a)
{
 PHYSFS_write(file, a, 2, size);
}

void BinaryWriteFile::writeULong(const IULong &a)
{
 PHYSFS_write(file, &a, 4, 1);
}

void BinaryWriteFile::writeULongArray(const int size, const IULong *a)
{
 PHYSFS_write(file, a, 4, size);
}

void BinaryWriteFile::seek(const IULong where)
{
 PHYSFS_seek(file, where);
}

