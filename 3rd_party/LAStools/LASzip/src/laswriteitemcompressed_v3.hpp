/*
===============================================================================

  FILE:  laswriteitemcompressed_v3.hpp
  
  CONTENTS:
  
    Native extension for compressing the *new* point types 6 to 10 of LAS 1.4 

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
    22 August 2016 -- finalizing at Basecamp in Bonn during FOSS4g hackfest
    23 February 2016 -- created at OSGeo Code Sprint in Paris to prototype

===============================================================================
*/
#ifndef LAS_WRITE_ITEM_COMPRESSED_V3_HPP
#define LAS_WRITE_ITEM_COMPRESSED_V3_HPP

#include "laswriteitem.hpp"
#include "arithmeticencoder.hpp"
#include "integercompressor.hpp"
#include "bytestreamout_array.hpp"

#include "laszip_common_v3.hpp"

class LASwriteItemCompressed_POINT14_v3 : public LASwriteItemCompressed
{
public:

  LASwriteItemCompressed_POINT14_v3(ArithmeticEncoder* enc);

  BOOL init(const U8* item, U32& context);
  BOOL write(const U8* item, U32& context);
  BOOL chunk_sizes();
  BOOL chunk_bytes();

  ~LASwriteItemCompressed_POINT14_v3();

private:

  /* not used as a encoder. just gives access to outstream */

  ArithmeticEncoder* enc;

  ByteStreamOutArray* outstream_channel_returns_XY;
  ByteStreamOutArray* outstream_Z;
  ByteStreamOutArray* outstream_classification;
  ByteStreamOutArray* outstream_flags;
  ByteStreamOutArray* outstream_intensity;
  ByteStreamOutArray* outstream_scan_angle;
  ByteStreamOutArray* outstream_user_data;
  ByteStreamOutArray* outstream_point_source;
  ByteStreamOutArray* outstream_gps_time;

  ArithmeticEncoder* enc_channel_returns_XY;
  ArithmeticEncoder* enc_Z;
  ArithmeticEncoder* enc_classification;
  ArithmeticEncoder* enc_flags;
  ArithmeticEncoder* enc_intensity;
  ArithmeticEncoder* enc_scan_angle;
  ArithmeticEncoder* enc_user_data;
  ArithmeticEncoder* enc_point_source;
  ArithmeticEncoder* enc_gps_time;

  BOOL changed_classification;
  BOOL changed_flags;
  BOOL changed_intensity;
  BOOL changed_scan_angle;
  BOOL changed_user_data;
  BOOL changed_point_source;
  BOOL changed_gps_time;

  U32 num_bytes_channel_returns_XY;
  U32 num_bytes_Z;
  U32 num_bytes_classification;
  U32 num_bytes_flags;
  U32 num_bytes_intensity;
  U32 num_bytes_scan_angle;
  U32 num_bytes_user_data;
  U32 num_bytes_point_source;
  U32 num_bytes_gps_time;

  U32 current_context;
  LAScontextPOINT14 contexts[4];

  BOOL createAndInitModelsAndCompressors(U32 context, const U8* item);
  void write_gps_time(const U64I64F64 gps_time);
};

class LASwriteItemCompressed_RGB14_v3 : public LASwriteItemCompressed
{
public:

  LASwriteItemCompressed_RGB14_v3(ArithmeticEncoder* enc);

  BOOL init(const U8* item, U32& context);
  BOOL write(const U8* item, U32& context);
  BOOL chunk_sizes();
  BOOL chunk_bytes();

  ~LASwriteItemCompressed_RGB14_v3();

private:

  /* not used as a encoder. just gives access to outstream */

  ArithmeticEncoder* enc;

  ByteStreamOutArray* outstream_RGB;

  ArithmeticEncoder* enc_RGB;

  BOOL changed_RGB;

  U32 num_bytes_RGB;

  U32 current_context;
  LAScontextRGB14 contexts[4];

  BOOL createAndInitModelsAndCompressors(U32 context, const U8* item);
};

class LASwriteItemCompressed_RGBNIR14_v3 : public LASwriteItemCompressed
{
public:

  LASwriteItemCompressed_RGBNIR14_v3(ArithmeticEncoder* enc);

  BOOL init(const U8* item, U32& context);
  BOOL write(const U8* item, U32& context);
  BOOL chunk_sizes();
  BOOL chunk_bytes();

  ~LASwriteItemCompressed_RGBNIR14_v3();

private:

  /* not used as a encoder. just gives access to outstream */

  ArithmeticEncoder* enc;

  ByteStreamOutArray* outstream_RGB;
  ByteStreamOutArray* outstream_NIR;

  ArithmeticEncoder* enc_RGB;
  ArithmeticEncoder* enc_NIR;

  BOOL changed_RGB;
  BOOL changed_NIR;

  U32 num_bytes_RGB;
  U32 num_bytes_NIR;

  U32 current_context;
  LAScontextRGBNIR14 contexts[4];

  BOOL createAndInitModelsAndCompressors(U32 context, const U8* item);
};

class LASwriteItemCompressed_WAVEPACKET14_v3 : public LASwriteItemCompressed
{
public:

  LASwriteItemCompressed_WAVEPACKET14_v3(ArithmeticEncoder* enc);

  BOOL init(const U8* item, U32& context);
  BOOL write(const U8* item, U32& context);
  BOOL chunk_sizes();
  BOOL chunk_bytes();

  ~LASwriteItemCompressed_WAVEPACKET14_v3();

private:

  /* not used as a encoder. just gives access to outstream */

  ArithmeticEncoder* enc;

  ByteStreamOutArray* outstream_wavepacket;

  ArithmeticEncoder* enc_wavepacket;

  BOOL changed_wavepacket;

  U32 num_bytes_wavepacket;

  U32 current_context;
  LAScontextWAVEPACKET14 contexts[4];

  BOOL createAndInitModelsAndCompressors(U32 context, const U8* item);
};

class LASwriteItemCompressed_BYTE14_v3 : public LASwriteItemCompressed
{
public:

  LASwriteItemCompressed_BYTE14_v3(ArithmeticEncoder* enc, U32 number);

  BOOL init(const U8* item, U32& context);
  BOOL write(const U8* item, U32& context);
  BOOL chunk_sizes();
  BOOL chunk_bytes();

  ~LASwriteItemCompressed_BYTE14_v3();

private:

  /* not used as a encoder. just gives access to outstream */

  ArithmeticEncoder* enc;

  ByteStreamOutArray** outstream_Bytes;

  ArithmeticEncoder** enc_Bytes;

  U32* num_bytes_Bytes;

  BOOL* changed_Bytes;

  U32 current_context;
  LAScontextBYTE14 contexts[4];

  U32 number;
  BOOL createAndInitModelsAndCompressors(U32 context, const U8* item);
};

#endif
