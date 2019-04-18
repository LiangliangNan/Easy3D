/*
===============================================================================

  FILE:  bytestreamin_file.hpp
  
  CONTENTS:
      
    Class for FILE*-based input streams with endian handling.

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2012, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
     1 October 2011 -- added 64 bit file support in MSVC 6.0 at McCafe at Hbf Linz
    10 January 2011 -- licensing change for LGPL release and liblas integration
    12 December 2010 -- created from ByteStreamOutFile after Howard got pushy (-;
  
===============================================================================
*/
#ifndef BYTE_STREAM_IN_FILE_H
#define BYTE_STREAM_IN_FILE_H

#include "bytestreamin.hpp"

#include <stdio.h>

#if defined(_MSC_VER) && (_MSC_VER < 1300)
extern "C" __int64 _cdecl _ftelli64(FILE*);
extern "C" int _cdecl _fseeki64(FILE*, __int64, int);
#endif

class ByteStreamInFile : public ByteStreamIn
{
public:
  ByteStreamInFile(FILE* file);
/* read a single byte                                        */
  U32 getByte();
/* read an array of bytes                                    */
  void getBytes(U8* bytes, const U32 num_bytes);
/* is the stream seekable (e.g. stdin is not)                */
  BOOL isSeekable() const;
/* get current position of stream                            */
  I64 tell() const;
/* seek to this position in the stream                       */
  BOOL seek(const I64 position);
/* seek to the end of the file                               */
  BOOL seekEnd(const I64 distance=0);
/* destructor                                                */
  ~ByteStreamInFile(){};
protected:
  FILE* file;
};

class ByteStreamInFileLE : public ByteStreamInFile
{
public:
  ByteStreamInFileLE(FILE* file);
/* read 16 bit low-endian field                              */
  void get16bitsLE(U8* bytes);
/* read 32 bit low-endian field                              */
  void get32bitsLE(U8* bytes);
/* read 64 bit low-endian field                              */
  void get64bitsLE(U8* bytes);
/* read 16 bit big-endian field                              */
  void get16bitsBE(U8* bytes);
/* read 32 bit big-endian field                              */
  void get32bitsBE(U8* bytes);
/* read 64 bit big-endian field                              */
  void get64bitsBE(U8* bytes);
private:
  U8 swapped[8];
};

class ByteStreamInFileBE : public ByteStreamInFile
{
public:
  ByteStreamInFileBE(FILE* file);
/* read 16 bit low-endian field                              */
  void get16bitsLE(U8* bytes);
/* read 32 bit low-endian field                              */
  void get32bitsLE(U8* bytes);
/* read 64 bit low-endian field                              */
  void get64bitsLE(U8* bytes);
/* read 16 bit big-endian field                              */
  void get16bitsBE(U8* bytes);
/* read 32 bit big-endian field                              */
  void get32bitsBE(U8* bytes);
/* read 64 bit big-endian field                              */
  void get64bitsBE(U8* bytes);
private:
  U8 swapped[8];
};

inline ByteStreamInFile::ByteStreamInFile(FILE* file)
{
  this->file = file;
}

inline U32 ByteStreamInFile::getByte()
{
  int byte = getc(file);
  if (byte == EOF)
  {
    throw EOF;
  }
  return (U32)byte;
}

inline void ByteStreamInFile::getBytes(U8* bytes, const U32 num_bytes)
{
  if (fread(bytes, 1, num_bytes, file) != num_bytes)
  {
    throw EOF;
  }
}

inline BOOL ByteStreamInFile::isSeekable() const
{
  return (file != stdin);
}

inline I64 ByteStreamInFile::tell() const
{
#if defined _WIN32 && ! defined (__MINGW32__)
  return _ftelli64(file);
#elif defined (__MINGW32__)
  return (I64)ftello64(file);
#else
  return (I64)ftello(file);
#endif
}

inline BOOL ByteStreamInFile::seek(const I64 position)
{
  if (tell() != position)
  {
#if defined _WIN32 && ! defined (__MINGW32__)
    return !(_fseeki64(file, position, SEEK_SET));
#elif defined (__MINGW32__)
    return !(fseeko64(file, (off_t)position, SEEK_SET));
#else
    return !(fseeko(file, (off_t)position, SEEK_SET));
#endif
  }
  return TRUE;
}

inline BOOL ByteStreamInFile::seekEnd(const I64 distance)
{
#if defined _WIN32 && ! defined (__MINGW32__)
  return !(_fseeki64(file, -distance, SEEK_END));
#elif defined (__MINGW32__)
  return !(fseeko64(file, (off_t)-distance, SEEK_END));
#else
  return !(fseeko(file, (off_t)-distance, SEEK_END));
#endif
}

inline ByteStreamInFileLE::ByteStreamInFileLE(FILE* file) : ByteStreamInFile(file)
{
}

inline void ByteStreamInFileLE::get16bitsLE(U8* bytes)
{
  getBytes(bytes, 2);
}

inline void ByteStreamInFileLE::get32bitsLE(U8* bytes)
{
  getBytes(bytes, 4);
}

inline void ByteStreamInFileLE::get64bitsLE(U8* bytes)
{
  getBytes(bytes, 8);
}

inline void ByteStreamInFileLE::get16bitsBE(U8* bytes)
{
  getBytes(swapped, 2);
  bytes[0] = swapped[1];
  bytes[1] = swapped[0];
}

inline void ByteStreamInFileLE::get32bitsBE(U8* bytes)
{
  getBytes(swapped, 4);
  bytes[0] = swapped[3];
  bytes[1] = swapped[2];
  bytes[2] = swapped[1];
  bytes[3] = swapped[0];
}

inline void ByteStreamInFileLE::get64bitsBE(U8* bytes)
{
  getBytes(swapped, 8);
  bytes[0] = swapped[7];
  bytes[1] = swapped[6];
  bytes[2] = swapped[5];
  bytes[3] = swapped[4];
  bytes[4] = swapped[3];
  bytes[5] = swapped[2];
  bytes[6] = swapped[1];
  bytes[7] = swapped[0];
}

inline ByteStreamInFileBE::ByteStreamInFileBE(FILE* file) : ByteStreamInFile(file)
{
}

inline void ByteStreamInFileBE::get16bitsLE(U8* bytes)
{
  getBytes(swapped, 2);
  bytes[0] = swapped[1];
  bytes[1] = swapped[0];
}

inline void ByteStreamInFileBE::get32bitsLE(U8* bytes)
{
  getBytes(swapped, 4);
  bytes[0] = swapped[3];
  bytes[1] = swapped[2];
  bytes[2] = swapped[1];
  bytes[3] = swapped[0];
}

inline void ByteStreamInFileBE::get64bitsLE(U8* bytes)
{
  getBytes(swapped, 8);
  bytes[0] = swapped[7];
  bytes[1] = swapped[6];
  bytes[2] = swapped[5];
  bytes[3] = swapped[4];
  bytes[4] = swapped[3];
  bytes[5] = swapped[2];
  bytes[6] = swapped[1];
  bytes[7] = swapped[0];
}

inline void ByteStreamInFileBE::get16bitsBE(U8* bytes)
{
  getBytes(bytes, 2);
}

inline void ByteStreamInFileBE::get32bitsBE(U8* bytes)
{
  getBytes(bytes, 4);
}

inline void ByteStreamInFileBE::get64bitsBE(U8* bytes)
{
  getBytes(bytes, 8);
}

#endif
