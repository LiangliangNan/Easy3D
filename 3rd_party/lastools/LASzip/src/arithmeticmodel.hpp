/*
===============================================================================

  FILE:  arithmeticmodel.hpp
  
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
  
    11 April 2019 -- 1024 AC_BUFFER_SIZE to 4096 for propagate_carry() overflow
    10 January 2011 -- licensing change for LGPL release and liblas integration
    8 December 2010 -- unified framework for all entropy coders
    30 October 2009 -- refactoring Amir Said's FastAC code
  
===============================================================================
*/
#ifndef ARITHMETIC_MODEL_HPP
#define ARITHMETIC_MODEL_HPP

#include <stdio.h>
#include <stdlib.h>

#include "mydefs.hpp"

/* this header byte needs to change in case incompatible change happen */
#define AC_HEADER_BYTE 2
#define AC_BUFFER_SIZE 4096

const U32 AC__MinLength = 0x01000000U;   // threshold for renormalization
const U32 AC__MaxLength = 0xFFFFFFFFU;      // maximum AC interval length

                                           // Maximum values for binary models
const U32 BM__LengthShift = 13;     // length bits discarded before mult.
const U32 BM__MaxCount    = 1 << BM__LengthShift;  // for adaptive models

                                          // Maximum values for general models
const U32 DM__LengthShift = 15;     // length bits discarded before mult.
const U32 DM__MaxCount    = 1 << DM__LengthShift;  // for adaptive models

class ArithmeticModel
{
public:
  ArithmeticModel(U32 symbols, BOOL compress);
  ~ArithmeticModel();

  I32 init(U32* table=0);

private:
  void update();
  U32 * distribution, * symbol_count, * decoder_table;
  U32 total_count, update_cycle, symbols_until_update;
  U32 symbols, last_symbol, table_size, table_shift;
  BOOL compress;
  friend class ArithmeticEncoder;
  friend class ArithmeticDecoder;
};

class ArithmeticBitModel
{
public:
  ArithmeticBitModel();
  ~ArithmeticBitModel();

  void init();

private:
  void update();
  U32 update_cycle, bits_until_update;
  U32 bit_0_prob, bit_0_count, bit_count;
  friend class ArithmeticEncoder;
  friend class ArithmeticDecoder;
};

#endif
