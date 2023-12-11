/*
===============================================================================

  FILE:  arithmeticdecoder.cpp

  CONTENTS:

    A modular C++ wrapper for an adapted version of Amir Said's FastAC Code.
    see: http://www.cipr.rpi.edu/~said/FastAC.html

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

    see header file

===============================================================================
*/

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//                                                                           -
//                       ****************************                        -
//                        ARITHMETIC CODING EXAMPLES                         -
//                       ****************************                        -
//                                                                           -
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
// of arithmetic coding. The original version of this code can be found in   -
// Digital Signal Compression: Principles and Practice                       -
// (Cambridge University Press, 2011, ISBN: 9780511984655)                   -
//                                                                           -
// Copyright (c) 2019 by Amir Said (said@ieee.org) &                         -
//                       William A. Pearlman (pearlw@ecse.rpi.edu)           -
//                                                                           -
// Redistribution and use in source and binary forms, with or without        -
// modification, are permitted provided that the following conditions are    -
// met:                                                                      -
//                                                                           -
// 1. Redistributions of source code must retain the above copyright notice, -
// this list of conditions and the following disclaimer.                     -
//                                                                           -
// 2. Redistributions in binary form must reproduce the above copyright      -
// notice, this list of conditions and the following disclaimer in the       -
// documentation and/or other materials provided with the distribution.      -
//                                                                           -
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       -
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED -
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A           -
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER -
// OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,  -
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,       -
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR        -
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    -
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      -
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        -
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              -
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

#include "arithmeticdecoder.hpp"

#include <string.h>
#include <assert.h>

#include "arithmeticmodel.hpp"

ArithmeticDecoder::ArithmeticDecoder()
{
  instream = 0;
}

BOOL ArithmeticDecoder::init(ByteStreamIn* instream, BOOL really_init)
{
  if (instream == 0) return FALSE;
  this->instream = instream;
  length = AC__MaxLength;
  if (really_init)
  {
    value = (instream->getByte() << 24);
    value |= (instream->getByte() << 16);
    value |= (instream->getByte() << 8);
    value |= (instream->getByte());
  }
  return TRUE;
}

void ArithmeticDecoder::done()
{
  instream = 0;
}

ArithmeticBitModel* ArithmeticDecoder::createBitModel()
{
  ArithmeticBitModel* m = new ArithmeticBitModel();
  return m;
}

void ArithmeticDecoder::initBitModel(ArithmeticBitModel* m)
{
  m->init();
}

void ArithmeticDecoder::destroyBitModel(ArithmeticBitModel* m)
{
  delete m;
}

ArithmeticModel* ArithmeticDecoder::createSymbolModel(U32 n)
{
  ArithmeticModel* m = new ArithmeticModel(n, FALSE);
  return m;
}

void ArithmeticDecoder::initSymbolModel(ArithmeticModel* m, U32 *table)
{
  m->init(table);
}

void ArithmeticDecoder::destroySymbolModel(ArithmeticModel* m)
{
  delete m;
}

U32 ArithmeticDecoder::decodeBit(ArithmeticBitModel* m)
{
  assert(m);

  U32 x = m->bit_0_prob * (length >> BM__LengthShift);       // product l x p0
  U32 sym = (value >= x);                                          // decision
                                                    // update & shift interval
  if (sym == 0) {
    length  = x;
    ++m->bit_0_count;
  }
  else {
    value  -= x;                                  // shifted interval base = 0
    length -= x;
  }

  if (length < AC__MinLength) renorm_dec_interval();        // renormalization
  if (--m->bits_until_update == 0) m->update();       // periodic model update

  return sym;                                         // return data bit value
}

U32 ArithmeticDecoder::decodeSymbol(ArithmeticModel* m)
{
  U32 n, sym, x, y = length;

  if (m->decoder_table) {             // use table look-up for faster decoding

    unsigned dv = value / (length >>= DM__LengthShift);
    unsigned t = dv >> m->table_shift;

    sym = m->decoder_table[t];      // initial decision based on table look-up
    n = m->decoder_table[t+1] + 1;

    while (n > sym + 1) {                      // finish with bisection search
      U32 k = (sym + n) >> 1;
      if (m->distribution[k] > dv) n = k; else sym = k;
    }
                                                           // compute products
    x = m->distribution[sym] * length;
    if (sym != m->last_symbol) y = m->distribution[sym+1] * length;
  }

  else {                                  // decode using only multiplications

    x = sym = 0;
    length >>= DM__LengthShift;
    U32 k = (n = m->symbols) >> 1;
                                                // decode via bisection search
    do {
      U32 z = length * m->distribution[k];
      if (z > value) {
        n = k;
        y = z;                                             // value is smaller
      }
      else {
        sym = k;
        x = z;                                     // value is larger or equal
      }
    } while ((k = (sym + n) >> 1) != sym);
  }

  value -= x;                                               // update interval
  length = y - x;

  if (length < AC__MinLength) renorm_dec_interval();        // renormalization

  ++m->symbol_count[sym];
  if (--m->symbols_until_update == 0) m->update();    // periodic model update

  assert(sym < m->symbols);

  return sym;
}

U32 ArithmeticDecoder::readBit()
{
  U32 sym = value / (length >>= 1);            // decode symbol, change length
  value -= length * sym;                                    // update interval

  if (length < AC__MinLength) renorm_dec_interval();        // renormalization

  if (sym >= 2)
  {
    throw 4711;
  }

  return sym;
}

U32 ArithmeticDecoder::readBits(U32 bits)
{
  assert(bits && (bits <= 32));

  if (bits > 19)
  {
    U32 tmp = readShort();
    bits = bits - 16;
    U32 tmp1 = readBits(bits) << 16;
    return (tmp1|tmp);
  }

  U32 sym = value / (length >>= bits);// decode symbol, change length
  value -= length * sym;                                    // update interval

  if (length < AC__MinLength) renorm_dec_interval();        // renormalization

  if (sym >= (1u<<bits))
  {
    throw 4711;
  }

  return sym;
}

U8 ArithmeticDecoder::readByte()
{
  U32 sym = value / (length >>= 8);            // decode symbol, change length
  value -= length * sym;                                    // update interval

  if (length < AC__MinLength) renorm_dec_interval();        // renormalization

  if (sym >= (1u<<8))
  {
    throw 4711;
  }

  return (U8)sym;
}

U16 ArithmeticDecoder::readShort()
{
  U32 sym = value / (length >>= 16);           // decode symbol, change length
  value -= length * sym;                                    // update interval

  if (length < AC__MinLength) renorm_dec_interval();        // renormalization

  if (sym >= (1u<<16))
  {
    throw 4711;
  }

  return (U16)sym;
}

U32 ArithmeticDecoder::readInt()
{
  U32 lowerInt = readShort();
  U32 upperInt = readShort();
  return (upperInt<<16)|lowerInt;
}

F32 ArithmeticDecoder::readFloat() /* danger in float reinterpretation */
{
  U32I32F32 u32i32f32;
  u32i32f32.u32 = readInt();
  return u32i32f32.f32;
}

U64 ArithmeticDecoder::readInt64()
{
  U64 lowerInt = readInt();
  U64 upperInt = readInt();
  return (upperInt<<32)|lowerInt;
}

F64 ArithmeticDecoder::readDouble() /* danger in float reinterpretation */
{
  U64I64F64 u64i64f64;
  u64i64f64.u64 = readInt64();
  return u64i64f64.f64;
}

ArithmeticDecoder::~ArithmeticDecoder()
{
}

inline void ArithmeticDecoder::renorm_dec_interval()
{
  do {                                          // read least-significant byte
    value = (value << 8) | instream->getByte();
  } while ((length <<= 8) < AC__MinLength);        // length multiplied by 256
}
