/*
===============================================================================

  FILE:  bytestreamout.hpp
  
  CONTENTS:
      
    Abstract base class for output streams with endian handling.

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
  
     2 January 2013 -- new functions for writing a stream of groups of bits  
     1 October 2011 -- added 64 bit file support in MSVC 6.0 at McCafe at Hbf Linz
    10 January 2011 -- licensing change for LGPL release and liblas integration
    12 December 2010 -- created from ByteStreamOutFile after Howard got pushy (-;
  
===============================================================================
*/
#ifndef BYTE_STREAM_OUT_HPP
#define BYTE_STREAM_OUT_HPP

#include "mydefs.hpp"

class ByteStreamOut
{
public:
/* write single bits                                         */
  inline BOOL putBits(U32 bits, U32 num_bits)
  {
    U64 new_bits = bits;
    bit_buffer |= (new_bits << num_buffer);
    num_buffer += num_bits;
    if (num_buffer >= 32)
    {
      U32 output_bits = (U32)(bit_buffer & U32_MAX);
      bit_buffer = bit_buffer >> 32;
      num_buffer = num_buffer - 32;
      return put32bitsLE((U8*)&output_bits);
    }
    return TRUE;
  };
/* called after writing bits before closing or writing bytes */
  inline BOOL flushBits()
  {
    if (num_buffer)
    {
      U32 num_zero_bits = 32 - num_buffer;
      U32 output_bits = (U32)(bit_buffer >> num_zero_bits);
      bit_buffer = 0;
      num_buffer = 0;
      return put32bitsLE((U8*)&output_bits);
    }
    return TRUE;
  };
/* write a single byte                                       */
  virtual BOOL putByte(U8 byte) = 0;
/* write an array of bytes                                   */
  virtual BOOL putBytes(const U8* bytes, U32 num_bytes) = 0;
/* write 16 bit low-endian field                             */
  virtual BOOL put16bitsLE(const U8* bytes) = 0;
/* write 32 bit low-endian field                             */
  virtual BOOL put32bitsLE(const U8* bytes) = 0;
/* write 64 bit low-endian field                             */
  virtual BOOL put64bitsLE(const U8* bytes) = 0;
/* write 16 bit big-endian field                             */
  virtual BOOL put16bitsBE(const U8* bytes) = 0;
/* write 32 bit big-endian field                             */
  virtual BOOL put32bitsBE(const U8* bytes) = 0;
/* write 64 bit big-endian field                             */
  virtual BOOL put64bitsBE(const U8* bytes) = 0;
/* is the stream seekable (e.g. standard out is not)         */
  virtual BOOL isSeekable() const = 0;
/* get current position of stream                            */
  virtual I64 tell() const = 0;
/* seek to this position in the stream                       */
  virtual BOOL seek(const I64 position) = 0;
/* seek to the end of the file                               */
  virtual BOOL seekEnd() = 0;
/* constructor                                               */
  inline ByteStreamOut() { bit_buffer = 0; num_buffer = 0; };
/* destructor                                                */
  virtual ~ByteStreamOut() {};
private:
  U64 bit_buffer;
  U32 num_buffer;
};

#endif
