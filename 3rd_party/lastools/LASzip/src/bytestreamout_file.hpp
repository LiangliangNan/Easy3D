/*
===============================================================================

  FILE:  bytestreamout_file.hpp
  
  CONTENTS:
      
    Class for FILE*-based output streams with endian handling.

  PROGRAMMERS:

    info@rapidlasso.de  -  https://rapidlasso.de

  COPYRIGHT:

    (c) 2007-2022, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the Apache Public License 2.0 published by the Apache Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
     1 October 2011 -- added 64 bit file support in MSVC 6.0 at McCafe at Hbf Linz
    10 January 2011 -- licensing change for LGPL release and liblas integration
    12 December 2010 -- created from ByteStreamOutFile after Howard got pushy (-;
  
===============================================================================
*/
#ifndef BYTE_STREAM_OUT_FILE_H
#define BYTE_STREAM_OUT_FILE_H

#include "bytestreamout.hpp"

#include <stdio.h>

#if defined(_MSC_VER) && (_MSC_VER < 1300)
extern "C" int _cdecl _fseeki64(FILE*, __int64, int);
extern "C" __int64 _cdecl _ftelli64(FILE*);
#endif

class ByteStreamOutFile : public ByteStreamOut
{
public:
  ByteStreamOutFile(FILE* file);
/* replace a closed FILE* with a reopened FILE* in "ab" mode */
  BOOL refile(FILE* file);
/* write a single byte                                       */
  BOOL putByte(U8 byte);
/* write an array of bytes                                   */
  BOOL putBytes(const U8* bytes, U32 num_bytes);
/* is the stream seekable (e.g. standard out is not)         */
  BOOL isSeekable() const;
/* get current position of stream                            */
  I64 tell() const;
/* seek to this position in the stream                       */
  BOOL seek(const I64 position);
/* seek to the end of the file                               */
  BOOL seekEnd();
/* destructor                                                */
  ~ByteStreamOutFile(){};
protected:
  FILE* file;
};

class ByteStreamOutFileLE : public ByteStreamOutFile
{
public:
  ByteStreamOutFileLE(FILE* file);
/* write 16 bit low-endian field                             */
  BOOL put16bitsLE(const U8* bytes);
/* write 32 bit low-endian field                             */
  BOOL put32bitsLE(const U8* bytes);
/* write 64 bit low-endian field                             */
  BOOL put64bitsLE(const U8* bytes);
/* write 16 bit big-endian field                             */
  BOOL put16bitsBE(const U8* bytes);
/* write 32 bit big-endian field                             */
  BOOL put32bitsBE(const U8* bytes);
/* write 64 bit big-endian field                             */
  BOOL put64bitsBE(const U8* bytes);
private:
  U8 swapped[8];
};

class ByteStreamOutFileBE : public ByteStreamOutFile
{
public:
  ByteStreamOutFileBE(FILE* file);
/* write 16 bit low-endian field                             */
  BOOL put16bitsLE(const U8* bytes);
/* write 32 bit low-endian field                             */
  BOOL put32bitsLE(const U8* bytes);
/* write 64 bit low-endian field                             */
  BOOL put64bitsLE(const U8* bytes);
/* write 16 bit big-endian field                             */
  BOOL put16bitsBE(const U8* bytes);
/* write 32 bit big-endian field                             */
  BOOL put32bitsBE(const U8* bytes);
/* write 64 bit big-endian field                             */
  BOOL put64bitsBE(const U8* bytes);
private:
  U8 swapped[8];
};

inline ByteStreamOutFile::ByteStreamOutFile(FILE* file)
{
  this->file = file;
}

inline BOOL ByteStreamOutFile::refile(FILE* file)
{
  if (file == 0) return FALSE;
  this->file = file;
  return TRUE;
}

inline BOOL ByteStreamOutFile::putByte(U8 byte)
{
  return (fputc(byte, file) == byte);
}

inline BOOL ByteStreamOutFile::putBytes(const U8* bytes, U32 num_bytes)
{
  return (fwrite(bytes, 1, num_bytes, file) == num_bytes);
}

inline BOOL ByteStreamOutFile::isSeekable() const
{
  return (file != stdout);
}

inline I64 ByteStreamOutFile::tell() const
{
#if defined _WIN32 && ! defined (__MINGW32__)
  return _ftelli64(file);
#elif defined (__MINGW32__)
  return (I64)ftello64(file);
#else
  return (I64)ftello(file);
#endif
}

inline BOOL ByteStreamOutFile::seek(I64 position)
{
#if defined _WIN32 && ! defined (__MINGW32__)
  return !(_fseeki64(file, position, SEEK_SET));
#elif defined (__MINGW32__)
  return !(fseeko64(file, (off_t)position, SEEK_SET));
#else
  return !(fseeko(file, (off_t)position, SEEK_SET));
#endif
}

inline BOOL ByteStreamOutFile::seekEnd()
{
#if defined _WIN32 && ! defined (__MINGW32__)
  return !(_fseeki64(file, 0, SEEK_END));
#elif defined (__MINGW32__)
  return !(fseeko64(file, (off_t)0, SEEK_END));
#else
  return !(fseeko(file, (off_t)0, SEEK_END));
#endif
}

inline ByteStreamOutFileLE::ByteStreamOutFileLE(FILE* file) : ByteStreamOutFile(file)
{
}

inline BOOL ByteStreamOutFileLE::put16bitsLE(const U8* bytes)
{
  return putBytes(bytes, 2);
}

inline BOOL ByteStreamOutFileLE::put32bitsLE(const U8* bytes)
{
  return putBytes(bytes, 4);
}

inline BOOL ByteStreamOutFileLE::put64bitsLE(const U8* bytes)
{
  return putBytes(bytes, 8);
}

inline BOOL ByteStreamOutFileLE::put16bitsBE(const U8* bytes)
{
  swapped[0] = bytes[1];
  swapped[1] = bytes[0];
  return putBytes(swapped, 2);
}

inline BOOL ByteStreamOutFileLE::put32bitsBE(const U8* bytes)
{
  swapped[0] = bytes[3];
  swapped[1] = bytes[2];
  swapped[2] = bytes[1];
  swapped[3] = bytes[0];
  return putBytes(swapped, 4);
}

inline BOOL ByteStreamOutFileLE::put64bitsBE(const U8* bytes)
{
  swapped[0] = bytes[7];
  swapped[1] = bytes[6];
  swapped[2] = bytes[5];
  swapped[3] = bytes[4];
  swapped[4] = bytes[3];
  swapped[5] = bytes[2];
  swapped[6] = bytes[1];
  swapped[7] = bytes[0];
  return putBytes(swapped, 8);
}

inline ByteStreamOutFileBE::ByteStreamOutFileBE(FILE* file) : ByteStreamOutFile(file)
{
}

inline BOOL ByteStreamOutFileBE::put16bitsLE(const U8* bytes)
{
  swapped[0] = bytes[1];
  swapped[1] = bytes[0];
  return putBytes(swapped, 2);
}

inline BOOL ByteStreamOutFileBE::put32bitsLE(const U8* bytes)
{
  swapped[0] = bytes[3];
  swapped[1] = bytes[2];
  swapped[2] = bytes[1];
  swapped[3] = bytes[0];
  return putBytes(swapped, 4);
}

inline BOOL ByteStreamOutFileBE::put64bitsLE(const U8* bytes)
{
  swapped[0] = bytes[7];
  swapped[1] = bytes[6];
  swapped[2] = bytes[5];
  swapped[3] = bytes[4];
  swapped[4] = bytes[3];
  swapped[5] = bytes[2];
  swapped[6] = bytes[1];
  swapped[7] = bytes[0];
  return putBytes(swapped, 8);
}

inline BOOL ByteStreamOutFileBE::put16bitsBE(const U8* bytes)
{
  return putBytes(bytes, 2);
}

inline BOOL ByteStreamOutFileBE::put32bitsBE(const U8* bytes)
{
  return putBytes(bytes, 4);
}

inline BOOL ByteStreamOutFileBE::put64bitsBE(const U8* bytes)
{
  return putBytes(bytes, 8);
}

#endif
