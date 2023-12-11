/*
===============================================================================

  FILE:  laswriteitemcompressed_v2.hpp
  
  CONTENTS:
  
    Implementation of LASitemWriteCompressed for *all* items (version 2).

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
  
    28 August 2017 -- moving 'context' from global development hack to interface  
    6 September 2014 -- removed inheritance of EntropyEncoder and EntropyDecoder
    5 March 2011 -- created first night in ibiza to improve the RGB compressor

===============================================================================
*/
#ifndef LAS_WRITE_ITEM_COMPRESSED_V2_HPP
#define LAS_WRITE_ITEM_COMPRESSED_V2_HPP

#include "laswriteitem.hpp"
#include "arithmeticencoder.hpp"
#include "integercompressor.hpp"

#include "laszip_common_v2.hpp"

class LASwriteItemCompressed_POINT10_v2 : public LASwriteItemCompressed
{
public:

  LASwriteItemCompressed_POINT10_v2(ArithmeticEncoder* enc);

  BOOL init(const U8* item, U32& context);
  BOOL write(const U8* item, U32& context);

  ~LASwriteItemCompressed_POINT10_v2();

private:
  ArithmeticEncoder* enc;
  U8 last_item[20];
  U16 last_intensity[16];
  StreamingMedian5 last_x_diff_median5[16];
  StreamingMedian5 last_y_diff_median5[16];
  I32 last_height[8];

  ArithmeticModel* m_changed_values;
  IntegerCompressor* ic_intensity;
  ArithmeticModel* m_scan_angle_rank[2];
  IntegerCompressor* ic_point_source_ID;
  ArithmeticModel* m_bit_byte[256];
  ArithmeticModel* m_classification[256];
  ArithmeticModel* m_user_data[256];
  IntegerCompressor* ic_dx;
  IntegerCompressor* ic_dy;
  IntegerCompressor* ic_z;
};

class LASwriteItemCompressed_GPSTIME11_v2 : public LASwriteItemCompressed
{
public:

  LASwriteItemCompressed_GPSTIME11_v2(ArithmeticEncoder* enc);

  BOOL init(const U8* item, U32& context);
  BOOL write(const U8* item, U32& context);

  ~LASwriteItemCompressed_GPSTIME11_v2();

private:
  ArithmeticEncoder* enc;
  U32 last, next;
  U64I64F64 last_gpstime[4];
  I32 last_gpstime_diff[4];
  I32 multi_extreme_counter[4];

  ArithmeticModel* m_gpstime_multi;
  ArithmeticModel* m_gpstime_0diff;
  IntegerCompressor* ic_gpstime;
};

class LASwriteItemCompressed_RGB12_v2 : public LASwriteItemCompressed
{
public:

  LASwriteItemCompressed_RGB12_v2(ArithmeticEncoder* enc);

  BOOL init(const U8* item, U32& context);
  BOOL write(const U8* item, U32& context);

  ~LASwriteItemCompressed_RGB12_v2();

private:
  ArithmeticEncoder* enc;
  U16 last_item[3];

  ArithmeticModel* m_byte_used;
  ArithmeticModel* m_rgb_diff_0;
  ArithmeticModel* m_rgb_diff_1;
  ArithmeticModel* m_rgb_diff_2;
  ArithmeticModel* m_rgb_diff_3;
  ArithmeticModel* m_rgb_diff_4;
  ArithmeticModel* m_rgb_diff_5;
};

class LASwriteItemCompressed_BYTE_v2 : public LASwriteItemCompressed
{
public:

  LASwriteItemCompressed_BYTE_v2(ArithmeticEncoder* enc, U32 number);

  BOOL init(const U8* item, U32& context);
  BOOL write(const U8* item, U32& context);

  ~LASwriteItemCompressed_BYTE_v2();

private:
  ArithmeticEncoder* enc;
  U32 number;
  U8* last_item;

  ArithmeticModel** m_byte;
};

#endif
