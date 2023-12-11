/*
===============================================================================

  FILE:  arithmeticdecoder.hpp

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

    22 August 2016 -- can be used as init dummy by "native LAS 1.4 compressor"
    13 November 2014 -- integrity check in readBits(), readByte(), readShort()
     6 September 2014 -- removed the (unused) inheritance from EntropyDecoder
    10 January 2011 -- licensing change for LGPL release and liblas integration
     8 December 2010 -- unified framework for all entropy coders
    30 October 2009 -- refactoring Amir Said's FastAC code

===============================================================================
*/
#ifndef ARITHMETIC_DECODER_HPP
#define ARITHMETIC_DECODER_HPP

#include "mydefs.hpp"
#include "bytestreamin.hpp"

class ArithmeticModel;
class ArithmeticBitModel;

class ArithmeticDecoder
{
public:

/* Constructor & Destructor                                  */
  ArithmeticDecoder();
  ~ArithmeticDecoder();

/* Manage decoding                                           */
  BOOL init(ByteStreamIn* instream, BOOL really_init = TRUE);
  void done();

/* Manage an entropy model for a single bit                  */
  ArithmeticBitModel* createBitModel();
  void initBitModel(ArithmeticBitModel* model);
  void destroyBitModel(ArithmeticBitModel* model);

/* Manage an entropy model for n symbols (table optional)    */
  ArithmeticModel* createSymbolModel(U32 n);
  void initSymbolModel(ArithmeticModel* model, U32* table=0);
  void destroySymbolModel(ArithmeticModel* model);

/* Decode a bit with modelling                               */
  U32 decodeBit(ArithmeticBitModel* model);

/* Decode a symbol with modelling                            */
  U32 decodeSymbol(ArithmeticModel* model);

/* Decode a bit without modelling                            */
  U32 readBit();

/* Decode bits without modelling                             */
  U32 readBits(U32 bits);

/* Decode an unsigned char without modelling                 */
  U8 readByte();

/* Decode an unsigned short without modelling                */
  U16 readShort();

/* Decode an unsigned int without modelling                  */
  U32 readInt();

/* Decode a float without modelling                          */
  F32 readFloat();

/* Decode an unsigned 64 bit int without modelling           */
  U64 readInt64();

/* Decode a double without modelling                         */
  F64 readDouble();

/* Only read from instream if ArithmeticDecoder is dummy     */
  ByteStreamIn* getByteStreamIn() const { return instream; };

private:

  ByteStreamIn* instream;

  void renorm_dec_interval();
  U32 value, length;
};

#endif
