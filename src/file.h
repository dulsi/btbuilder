#ifndef __FILE_H
#define __FILE_H
/*-------------------------------------------------------------------------*\
  <file.h> -- Platform independant binary file opperations.

  Date      Programmer  Description
  03/20/98  Dennis      Created.
\*-------------------------------------------------------------------------*/

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  Includes
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#include "istdlib.h"
#include <physfs.h>
#include <stdexcept>

/* Error Note:
       At this time very little if any error checking is done in the binary
  file functions.  Eventually exceptions will be thrown in the event of an
  error.
*/

class FileException : public std::runtime_error
{
 public:
  FileException(const std::string &reason);
};

class BinaryReadFile
{
 public:
  BinaryReadFile();
  BinaryReadFile(const char *filename);
  ~BinaryReadFile();
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
  void seek(IULong where);
  void setSwap(bool value);

 private:
  PHYSFS_file *file;
  bool swap;
};

class BinaryWriteFile
{
 public:
  BinaryWriteFile();
  BinaryWriteFile(const char *filename);
  ~BinaryWriteFile();
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
  void seek(const IULong where);

 private:
  PHYSFS_file *file;
};

#endif

