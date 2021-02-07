/*
===============================================================================

  FILE:  arithmeticencoder.cpp
  
  CONTENTS:
      
    A modular C++ wrapper for an adapted version of Amir Said's FastAC Code.
    see: http://www.cipr.rpi.edu/~said/FastAC.html

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2005-2014, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    see header file
  
===============================================================================
*/

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//                                                                           -
// Fast arithmetic coding implementation                                     -
// -> 32-bit variables, 32-bit product, periodic updates, table decoding     -
//                                                                           -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//                                                                           -
// Version 1.00  -  April 25, 2004                                           -
//                                                                           -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//                                                                           -
//                                  WARNING                                  -
//                                 =========                                 -
//                                                                           -
// The only purpose of this program is to demonstrate the basic principles   -
// of arithmetic coding. It is provided as is, without any express or        -
// implied warranty, without even the warranty of fitness for any particular -
// purpose, or that the implementations are correct.                         -
//                                                                           -
// Permission to copy and redistribute this code is hereby granted, provided -
// that this warning and copyright notices are not removed or altered.       -
//                                                                           -
// Copyright (c) 2004 by Amir Said (said@ieee.org) &                         -
//                       William A. Pearlman (pearlw@ecse.rpi.edu)           -
//                                                                           -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//                                                                           -
// A description of the arithmetic coding method used here is available in   -
//                                                                           -
// Lossless Compression Handbook, ed. K. Sayood                              -
// Chapter 5: Arithmetic Coding (A. Said), pp. 101-152, Academic Press, 2003 -
//                                                                           -
// A. Said, Introduction to Arithetic Coding Theory and Practice             -
// HP Labs report HPL-2004-76  -  http://www.hpl.hp.com/techreports/         -
//                                                                           -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "arithmeticencoder.hpp"

#include <string.h>
#include <assert.h>

#include <stdio.h>

FILE* file = 0;

#include "arithmeticmodel.hpp"

ArithmeticEncoder::ArithmeticEncoder()
{
  outstream = 0;

  outbuffer = (U8*)malloc(sizeof(U8)*2*AC_BUFFER_SIZE);
  endbuffer = outbuffer + 2 * AC_BUFFER_SIZE;
}

ArithmeticEncoder::~ArithmeticEncoder()
{
  free(outbuffer);
}

BOOL ArithmeticEncoder::init(ByteStreamOut* outstream)
{
  if (outstream == 0) return FALSE;
  this->outstream = outstream;
  base   = 0;
  length = AC__MaxLength;
  outbyte = outbuffer;
  endbyte = endbuffer;
  return TRUE;
}

void ArithmeticEncoder::done()
{
  U32 init_base = base;                 // done encoding: set final data bytes
  BOOL another_byte = TRUE;

  if (length > 2 * AC__MinLength) {
    base  += AC__MinLength;                                     // base offset
    length = AC__MinLength >> 1;             // set new length for 1 more byte
  }
  else {
    base  += AC__MinLength >> 1;                                // base offset
    length = AC__MinLength >> 9;            // set new length for 2 more bytes
    another_byte = FALSE;
  }

  if (init_base > base) propagate_carry();                 // overflow = carry
  renorm_enc_interval();                // renormalization = output last bytes

  if (endbyte != endbuffer)
  {
    assert(outbyte < outbuffer + AC_BUFFER_SIZE);
    outstream->putBytes(outbuffer + AC_BUFFER_SIZE, AC_BUFFER_SIZE);
  }
  U32 buffer_size = (U32)(outbyte - outbuffer);
  if (buffer_size) outstream->putBytes(outbuffer, buffer_size);

  // write two or three zero bytes to be in sync with the decoder's byte reads
  outstream->putByte(0);
  outstream->putByte(0);
  if (another_byte) outstream->putByte(0);

  outstream = 0;
}

ArithmeticBitModel* ArithmeticEncoder::createBitModel()
{
  ArithmeticBitModel* m = new ArithmeticBitModel();
  return m;
}

void ArithmeticEncoder::initBitModel(ArithmeticBitModel* m)
{
  m->init();
}

void ArithmeticEncoder::destroyBitModel(ArithmeticBitModel* m)
{
  delete m;
}

ArithmeticModel* ArithmeticEncoder::createSymbolModel(U32 n)
{
  ArithmeticModel* m = new ArithmeticModel(n, true);
  return m;
}

void ArithmeticEncoder::initSymbolModel(ArithmeticModel* m, U32* table)
{
  m->init(table);
}

void ArithmeticEncoder::destroySymbolModel(ArithmeticModel* m)
{
  delete m;
}

void ArithmeticEncoder::encodeBit(ArithmeticBitModel* m, U32 sym)
{
  assert(m && (sym <= 1));

  U32 x = m->bit_0_prob * (length >> BM__LengthShift);       // product l x p0
                                                            // update interval
  if (sym == 0) {
    length = x;
    ++m->bit_0_count;
  }
  else {
    U32 init_base = base;
    base += x;
    length -= x;
    if (init_base > base) propagate_carry();               // overflow = carry
  }

  if (length < AC__MinLength) renorm_enc_interval();        // renormalization
  if (--m->bits_until_update == 0) m->update();       // periodic model update
}

void ArithmeticEncoder::encodeSymbol(ArithmeticModel* m, U32 sym)
{
  assert(m && (sym <= m->last_symbol));

  U32 x, init_base = base;
                                                           // compute products
  if (sym == m->last_symbol) {
    x = m->distribution[sym] * (length >> DM__LengthShift);
    base   += x;                                            // update interval
    length -= x;                                          // no product needed
  }
  else {
    x = m->distribution[sym] * (length >>= DM__LengthShift);
    base   += x;                                            // update interval
    length  = m->distribution[sym+1] * length - x;
  }

  if (init_base > base) propagate_carry();                 // overflow = carry
  if (length < AC__MinLength) renorm_enc_interval();        // renormalization

  ++m->symbol_count[sym];
  if (--m->symbols_until_update == 0) m->update();    // periodic model update
}

void ArithmeticEncoder::writeBit(U32 sym)
{
  assert(sym < 2);

  U32 init_base = base;
  base += sym * (length >>= 1);                // new interval base and length

  if (init_base > base) propagate_carry();                 // overflow = carry
  if (length < AC__MinLength) renorm_enc_interval();        // renormalization
}

void ArithmeticEncoder::writeBits(U32 bits, U32 sym)
{
  assert(bits && (bits <= 32) && (sym < (1u<<bits)));

  if (bits > 19)
  {
    writeShort(sym&U16_MAX);
    sym = sym >> 16;
    bits = bits - 16;
  }

  U32 init_base = base;
  base += sym * (length >>= bits);             // new interval base and length

  if (init_base > base) propagate_carry();                 // overflow = carry
  if (length < AC__MinLength) renorm_enc_interval();        // renormalization
}

void ArithmeticEncoder::writeByte(U8 sym)
{
  U32 init_base = base;
  base += (U32)(sym) * (length >>= 8);           // new interval base and length

  if (init_base > base) propagate_carry();                 // overflow = carry
  if (length < AC__MinLength) renorm_enc_interval();        // renormalization
}

void ArithmeticEncoder::writeShort(U16 sym)
{
  U32 init_base = base;
  base += (U32)(sym) * (length >>= 16);          // new interval base and length

  if (init_base > base) propagate_carry();                 // overflow = carry
  if (length < AC__MinLength) renorm_enc_interval();        // renormalization
}

void ArithmeticEncoder::writeInt(U32 sym)
{
  writeShort((U16)(sym & 0xFFFF)); // lower 16 bits
  writeShort((U16)(sym >> 16));    // UPPER 16 bits
}

void ArithmeticEncoder::writeFloat(F32 sym) /* danger in float reinterpretation */
{
  U32I32F32 u32i32f32;
  u32i32f32.f32 = sym;
  writeInt(u32i32f32.u32);
}

void ArithmeticEncoder::writeInt64(U64 sym)
{
  writeInt((U32)(sym & 0xFFFFFFFF)); // lower 32 bits
  writeInt((U32)(sym >> 32));        // UPPER 32 bits
}

void ArithmeticEncoder::writeDouble(F64 sym) /* danger in float reinterpretation */
{
  U64I64F64 u64i64f64;
  u64i64f64.f64 = sym;
  writeInt64(u64i64f64.u64);
}

inline void ArithmeticEncoder::propagate_carry()
{
  U8 * p;
  if (outbyte == outbuffer)
    p = endbuffer - 1;
  else
    p = outbyte - 1;
  while (*p == 0xFFU)
  {
    *p = 0;
    if (p == outbuffer)
      p = endbuffer - 1;
    else
      p--;
    assert(outbuffer <= p);
    assert(p < endbuffer);    
    assert(outbyte < endbuffer);    
  }
  ++*p;
}

inline void ArithmeticEncoder::renorm_enc_interval()
{
  do {                                          // output and discard top byte
    assert(outbuffer <= outbyte);
    assert(outbyte < endbuffer);    
    assert(outbyte < endbyte);    
    *outbyte++ = (U8)(base >> 24);
    if (outbyte == endbyte) manage_outbuffer();
    base <<= 8;
  } while ((length <<= 8) < AC__MinLength);        // length multiplied by 256
}

inline void ArithmeticEncoder::manage_outbuffer()
{
  if (outbyte == endbuffer) outbyte = outbuffer;
  outstream->putBytes(outbyte, AC_BUFFER_SIZE);
  endbyte = outbyte + AC_BUFFER_SIZE;
  assert(endbyte > outbyte);
  assert(outbyte < endbuffer);    
}
