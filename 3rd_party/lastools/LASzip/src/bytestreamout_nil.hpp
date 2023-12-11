/*
===============================================================================

  FILE:  bytestreamout_.hpilp
  
  CONTENTS:
      
    Class for a black hole that only counts the bytes.

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
#ifndef BYTE_STREAM_OUT_NIL_H
#define BYTE_STREAM_OUT_NIL_H

#include "bytestreamout.hpp"

#include <stdio.h>

class ByteStreamOutNil : public ByteStreamOut
{
public:
  ByteStreamOutNil();
/* write a single byte                                       */
  BOOL putByte(U8 byte);
/* write an array of bytes                                   */
  BOOL putBytes(const U8* bytes, U32 num_bytes);
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
/* is the stream seekable (e.g. standard out is not)         */
  BOOL isSeekable() const;
/* get current position of stream                            */
  I64 tell() const;
/* seek to this position in the stream                       */
  BOOL seek(const I64 position);
/* seek to the end of the file                               */
  BOOL seekEnd();
/* destructor                                                */
  ~ByteStreamOutNil(){};
private:
  I64 num_bytes;
};

inline ByteStreamOutNil::ByteStreamOutNil()
{
  num_bytes = 0;
}

inline BOOL ByteStreamOutNil::putByte(U8 byte)
{
  num_bytes++;
  return TRUE;
}

inline BOOL ByteStreamOutNil::putBytes(const U8* bytes, U32 num_bytes)
{
  this->num_bytes += num_bytes;
  return TRUE;
}

inline BOOL ByteStreamOutNil::put16bitsLE(const U8* bytes)
{
  return putBytes(bytes, 2);
}

inline BOOL ByteStreamOutNil::put32bitsLE(const U8* bytes)
{
  return putBytes(bytes, 4);
}

inline BOOL ByteStreamOutNil::put64bitsLE(const U8* bytes)
{
  return putBytes(bytes, 8);
}

inline BOOL ByteStreamOutNil::put16bitsBE(const U8* bytes)
{
  return putBytes(bytes, 2);
}

inline BOOL ByteStreamOutNil::put32bitsBE(const U8* bytes)
{
  return putBytes(bytes, 4);
}

inline BOOL ByteStreamOutNil::put64bitsBE(const U8* bytes)
{
  return putBytes(bytes, 8);
}

inline BOOL ByteStreamOutNil::isSeekable() const
{
  return TRUE;
}

inline I64 ByteStreamOutNil::tell() const
{
  return num_bytes;
}

inline BOOL ByteStreamOutNil::seek(I64 position)
{
  return TRUE;
}

inline BOOL ByteStreamOutNil::seekEnd()
{
  return TRUE;
}

#endif
