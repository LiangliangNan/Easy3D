/*
===============================================================================

  FILE:  lasreaditemcompressed_v1.hpp
  
  CONTENTS:
  
    Implementation of LASitemReadCompressed for *all* items (version 1).

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2017, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    28 August 2017 -- moving 'context' from global development hack to interface  
    6 September 2014 -- removed inheritance of EntropyEncoder and EntropyDecoder
    10 January 2011 -- licensing change for LGPL release and liblas integration
    7 December 2010 -- refactored after getting invited to KAUST in Saudi Arabia
  
===============================================================================
*/
#ifndef LAS_READ_ITEM_COMPRESSED_V1_HPP
#define LAS_READ_ITEM_COMPRESSED_V1_HPP

#include "lasreaditem.hpp"
#include "arithmeticdecoder.hpp"
#include "integercompressor.hpp"

class LASreadItemCompressed_POINT10_v1 : public LASreadItemCompressed
{
public:

  LASreadItemCompressed_POINT10_v1(ArithmeticDecoder* dec);

  BOOL init(const U8* item, U32& context); // context is unused
  void read(U8* item, U32& context);       // context is unused

  ~LASreadItemCompressed_POINT10_v1();

private:
  ArithmeticDecoder* dec;
  U8 last_item[20];

  I32 last_x_diff[3];
  I32 last_y_diff[3];
  I32 last_incr;
  IntegerCompressor* ic_dx;
  IntegerCompressor* ic_dy;
  IntegerCompressor* ic_z;
  IntegerCompressor* ic_intensity;
  IntegerCompressor* ic_scan_angle_rank;
  IntegerCompressor* ic_point_source_ID;
  ArithmeticModel* m_changed_values;
  ArithmeticModel* m_bit_byte[256];
  ArithmeticModel* m_classification[256];
  ArithmeticModel* m_user_data[256];
};

class LASreadItemCompressed_GPSTIME11_v1 : public LASreadItemCompressed
{
public:

  LASreadItemCompressed_GPSTIME11_v1(ArithmeticDecoder* dec);

  BOOL init(const U8* item, U32& context); // context is unused
  void read(U8* item, U32& context);       // context is unused

  ~LASreadItemCompressed_GPSTIME11_v1();

private:
  ArithmeticDecoder* dec;
  U64I64F64 last_gpstime;

  ArithmeticModel* m_gpstime_multi;
  ArithmeticModel* m_gpstime_0diff;
  IntegerCompressor* ic_gpstime;
  I32 multi_extreme_counter;
  I32 last_gpstime_diff;
};

class LASreadItemCompressed_RGB12_v1 : public LASreadItemCompressed
{
public:

  LASreadItemCompressed_RGB12_v1(ArithmeticDecoder* dec);

  BOOL init(const U8* item, U32& context); // context is unused
  void read(U8* item, U32& context);       // context is unused

  ~LASreadItemCompressed_RGB12_v1();

private:
  ArithmeticDecoder* dec;
  U8* last_item;

  ArithmeticModel* m_byte_used;
  IntegerCompressor* ic_rgb;
};

class LASreadItemCompressed_WAVEPACKET13_v1 : public LASreadItemCompressed
{
public:

  LASreadItemCompressed_WAVEPACKET13_v1(ArithmeticDecoder* dec);

  BOOL init(const U8* item, U32& context); // context is unused
  void read(U8* item, U32& context);       // context is unused

  ~LASreadItemCompressed_WAVEPACKET13_v1();

private:
  ArithmeticDecoder* dec;
  U8* last_item;

  I32 last_diff_32;
  U32 sym_last_offset_diff;
  ArithmeticModel* m_packet_index;
  ArithmeticModel* m_offset_diff[4];
  IntegerCompressor* ic_offset_diff;
  IntegerCompressor* ic_packet_size;
  IntegerCompressor* ic_return_point;
  IntegerCompressor* ic_xyz;
};

class LASreadItemCompressed_BYTE_v1 : public LASreadItemCompressed
{
public:

  LASreadItemCompressed_BYTE_v1(ArithmeticDecoder* dec, U32 number);

  BOOL init(const U8* item, U32& context); // context is unused
  void read(U8* item, U32& context);       // context is unused

  ~LASreadItemCompressed_BYTE_v1();

private:
  ArithmeticDecoder* dec;
  U32 number;
  U8* last_item;

  IntegerCompressor* ic_byte;
};

#endif
