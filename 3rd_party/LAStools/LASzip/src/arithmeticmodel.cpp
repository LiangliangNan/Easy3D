/*
===============================================================================

  FILE:  arithmeticmodel.cpp
  
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

#include "arithmeticmodel.hpp"

#include <stdio.h>
#include <stdlib.h>

ArithmeticModel::ArithmeticModel(U32 symbols, BOOL compress)
{
  this->symbols = symbols;
  this->compress = compress;
  distribution = 0;
}

ArithmeticModel::~ArithmeticModel()
{
  if (distribution) delete [] distribution;
}

I32 ArithmeticModel::init(U32* table)
{
  if (distribution == 0)
  {
    if ( (symbols < 2) || (symbols > (1 << 11)) )
    {
      return -1; // invalid number of symbols
    }
    last_symbol = symbols - 1;
    if ((!compress) && (symbols > 16))
    {
      U32 table_bits = 3;
      while (symbols > (1U << (table_bits + 2))) ++table_bits;
      table_size  = 1 << table_bits;
      table_shift = DM__LengthShift - table_bits;
      distribution = new U32[2*symbols+table_size+2];
      decoder_table = distribution + 2 * symbols;
    }
    else // small alphabet: no table needed
    {                                  
      decoder_table = 0;
      table_size = table_shift = 0;
      distribution = new U32[2*symbols];
    }
    if (distribution == 0)
    {
      return -1; // "cannot allocate model memory");
    }
    symbol_count = distribution + symbols;
  }

  total_count = 0;
  update_cycle = symbols;
  if (table)
    for (U32 k = 0; k < symbols; k++) symbol_count[k] = table[k];
  else
    for (U32 k = 0; k < symbols; k++) symbol_count[k] = 1;

  update();
  symbols_until_update = update_cycle = (symbols + 6) >> 1;

  return 0;
}

void ArithmeticModel::update()
{
  // halve counts when a threshold is reached
  if ((total_count += update_cycle) > DM__MaxCount)
  {
    total_count = 0;
    for (U32 n = 0; n < symbols; n++)
    {
      total_count += (symbol_count[n] = (symbol_count[n] + 1) >> 1);
    }
  }
  
  // compute cumulative distribution, decoder table
  U32 k, sum = 0, s = 0;
  U32 scale = 0x80000000U / total_count;

  if (compress || (table_size == 0))
  {
    for (k = 0; k < symbols; k++)
    {
      distribution[k] = (scale * sum) >> (31 - DM__LengthShift);
      sum += symbol_count[k];
    }
  }
  else
  {
    for (k = 0; k < symbols; k++)
    {
      distribution[k] = (scale * sum) >> (31 - DM__LengthShift);
      sum += symbol_count[k];
      U32 w = distribution[k] >> table_shift;
      while (s < w) decoder_table[++s] = k - 1;
    }
    decoder_table[0] = 0;
    while (s <= table_size) decoder_table[++s] = symbols - 1;
  }
  
  // set frequency of model updates
  update_cycle = (5 * update_cycle) >> 2;
  U32 max_cycle = (symbols + 6) << 3;
  if (update_cycle > max_cycle) update_cycle = max_cycle;
  symbols_until_update = update_cycle;
}

ArithmeticBitModel::ArithmeticBitModel()
{
  init();
}

ArithmeticBitModel::~ArithmeticBitModel()
{
}

void ArithmeticBitModel::init()
{
  // initialization to equiprobable model
  bit_0_count = 1;
  bit_count   = 2;
  bit_0_prob  = 1U << (BM__LengthShift - 1);
  // start with frequent updates
  update_cycle = bits_until_update = 4;
}

void ArithmeticBitModel::update()
{
  // halve counts when a threshold is reached
  if ((bit_count += update_cycle) > BM__MaxCount)
  {
    bit_count = (bit_count + 1) >> 1;
    bit_0_count = (bit_0_count + 1) >> 1;
    if (bit_0_count == bit_count) ++bit_count;
  }
  
  // compute scaled bit 0 probability
  U32 scale = 0x80000000U / bit_count;
  bit_0_prob = (bit_0_count * scale) >> (31 - BM__LengthShift);

  // set frequency of model updates
  update_cycle = (5 * update_cycle) >> 2;
  if (update_cycle > 64) update_cycle = 64;
  bits_until_update = update_cycle;
}
