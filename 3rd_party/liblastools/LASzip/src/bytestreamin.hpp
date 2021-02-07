/*
===============================================================================

  FILE:  bytestreamin.hpp
  
  CONTENTS:
      
    Abstract base class for input streams with endian handling.

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
  
     2 January 2013 -- new functions for reading a stream of groups of bits  
     1 October 2011 -- added 64 bit file support in MSVC 6.0 at McCafe at Hbf Linz
    10 January 2011 -- licensing change for LGPL release and liblas integration
    12 December 2010 -- created from ByteStreamOutFile after Howard got pushy (-;
  
===============================================================================
*/
#ifndef BYTE_STREAM_IN_HPP
#define BYTE_STREAM_IN_HPP

#include "mydefs.hpp"

class ByteStreamIn
{
public:
/* write single bits                                         */
  inline U32 getBits(U32 num_bits)
  {
    if (num_buffer < num_bits)
    {
      U32 input_bits;
      get32bitsLE((U8*)&input_bits);
      bit_buffer = bit_buffer | (((U64)input_bits) << num_buffer);
      num_buffer = num_buffer + 32;
    }
    U32 new_bits = (U32)(bit_buffer & ((1 << num_bits) - 1));
    bit_buffer = bit_buffer >> num_bits;
    num_buffer = num_buffer - num_bits;
    return new_bits;
  };
/* read a single byte                                        */
  virtual U32 getByte() = 0;
/* read an array of bytes                                    */
  virtual void getBytes(U8* bytes, const U32 num_bytes) = 0;
/* read 16 bit low-endian field                              */
  virtual void get16bitsLE(U8* bytes) = 0;
/* read 32 bit low-endian field                              */
  virtual void get32bitsLE(U8* bytes) = 0;
/* read 64 bit low-endian field                              */
  virtual void get64bitsLE(U8* bytes) = 0;
/* read 16 bit big-endian field                              */
  virtual void get16bitsBE(U8* bytes) = 0;
/* read 32 bit big-endian field                              */
  virtual void get32bitsBE(U8* bytes) = 0;
/* read 64 bit big-endian field                              */
  virtual void get64bitsBE(U8* bytes) = 0;
/* is the stream seekable (e.g. stdin is not)                */
  virtual BOOL isSeekable() const = 0;
/* get current position of stream                            */
  virtual I64 tell() const = 0;
/* seek to this position in the stream                       */
  virtual BOOL seek(const I64 position) = 0;
/* seek to the end of the file                               */
  virtual BOOL seekEnd(const I64 distance=0) = 0;
/* seek to the end of the file                               */
  virtual BOOL skipBytes(const U32 num_bytes) { I64 curr = tell(); return seek(curr + num_bytes); };
/* constructor                                               */
  inline ByteStreamIn() { bit_buffer = 0; num_buffer = 0; };
/* destructor                                                */
  virtual ~ByteStreamIn() {};
private:
  U64 bit_buffer;
  U32 num_buffer;
};

#endif
