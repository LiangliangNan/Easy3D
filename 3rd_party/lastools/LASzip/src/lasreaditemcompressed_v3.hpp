/*
===============================================================================

  FILE:  lasreaditemcompressed_v3.hpp
  
  CONTENTS:
  
    Native extension for decompressing the *new* point types 6 to 10 of LAS 1.4 

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
  
    30 December 2021 -- fix small memory leak
    19 March 2019 -- set "legacy classification" to zero if "classification > 31"  
    28 August 2017 -- moving 'context' from global development hack to interface  
    19 April 2017 -- support for selective decompression for new LAS 1.4 points 
    22 June 2016 -- created after Island beat Austria 2:1 in the EM2016
  
===============================================================================
*/
#ifndef LAS_READ_ITEM_COMPRESSED_V3_HPP
#define LAS_READ_ITEM_COMPRESSED_V3_HPP

#include "lasreaditem.hpp"
#include "arithmeticdecoder.hpp"
#include "integercompressor.hpp"
#include "bytestreamin_array.hpp"

#include "laszip_common_v3.hpp"
#include "laszip_decompress_selective_v3.hpp"

class LASreadItemCompressed_POINT14_v3 : public LASreadItemCompressed
{
public:

  LASreadItemCompressed_POINT14_v3(ArithmeticDecoder* dec, const U32 decompress_selective=LASZIP_DECOMPRESS_SELECTIVE_ALL);

  BOOL chunk_sizes();
  BOOL init(const U8* item, U32& context); // context is set
  void read(U8* item, U32& context);       // context is set

  ~LASreadItemCompressed_POINT14_v3();

private:

  /* not used as a decoder. just gives access to instream */
  
  ArithmeticDecoder* dec;

  ByteStreamInArray* instream_channel_returns_XY;
  ByteStreamInArray* instream_Z;
  ByteStreamInArray* instream_classification;
  ByteStreamInArray* instream_flags;
  ByteStreamInArray* instream_intensity;
  ByteStreamInArray* instream_scan_angle;
  ByteStreamInArray* instream_user_data;
  ByteStreamInArray* instream_point_source;
  ByteStreamInArray* instream_gps_time;

  ArithmeticDecoder* dec_channel_returns_XY;
  ArithmeticDecoder* dec_Z;
  ArithmeticDecoder* dec_classification;
  ArithmeticDecoder* dec_flags;
  ArithmeticDecoder* dec_intensity;
  ArithmeticDecoder* dec_scan_angle;
  ArithmeticDecoder* dec_user_data;
  ArithmeticDecoder* dec_point_source;
  ArithmeticDecoder* dec_gps_time;

  BOOL changed_Z;
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

  BOOL requested_Z;
  BOOL requested_classification;
  BOOL requested_flags;
  BOOL requested_intensity;
  BOOL requested_scan_angle;
  BOOL requested_user_data;
  BOOL requested_point_source;
  BOOL requested_gps_time;

  U8* bytes;
  U32 num_bytes_allocated;

  U32 current_context;
  LAScontextPOINT14 contexts[4];

  BOOL createAndInitModelsAndDecompressors(U32 context, const U8* item);
  void read_gps_time();
};

class LASreadItemCompressed_RGB14_v3 : public LASreadItemCompressed
{
public:

  LASreadItemCompressed_RGB14_v3(ArithmeticDecoder* dec, const U32 decompress_selective=LASZIP_DECOMPRESS_SELECTIVE_ALL);

  BOOL chunk_sizes();
  BOOL init(const U8* item, U32& context); // context is only read
  void read(U8* item, U32& context);       // context is only read

  ~LASreadItemCompressed_RGB14_v3();

private:

  /* not used as a decoder. just gives access to instream */

  ArithmeticDecoder* dec;

  ByteStreamInArray* instream_RGB;

  ArithmeticDecoder* dec_RGB;

  BOOL changed_RGB;

  U32 num_bytes_RGB;

  BOOL requested_RGB;

  U8* bytes;
  U32 num_bytes_allocated;

  U32 current_context;
  LAScontextRGB14 contexts[4];

  BOOL createAndInitModelsAndDecompressors(U32 context, const U8* item);
};

class LASreadItemCompressed_RGBNIR14_v3 : public LASreadItemCompressed
{
public:

  LASreadItemCompressed_RGBNIR14_v3(ArithmeticDecoder* dec, const U32 decompress_selective=LASZIP_DECOMPRESS_SELECTIVE_ALL);

  BOOL chunk_sizes();
  BOOL init(const U8* item, U32& context); // context is only read
  void read(U8* item, U32& context);       // context is only read

  ~LASreadItemCompressed_RGBNIR14_v3();

private:

  /* not used as a decoder. just gives access to instream */

  ArithmeticDecoder* dec;

  ByteStreamInArray* instream_RGB;
  ByteStreamInArray* instream_NIR;

  ArithmeticDecoder* dec_RGB;
  ArithmeticDecoder* dec_NIR;

  BOOL changed_RGB;
  BOOL changed_NIR;

  U32 num_bytes_RGB;
  U32 num_bytes_NIR;

  BOOL requested_RGB;
  BOOL requested_NIR;

  U8* bytes;
  U32 num_bytes_allocated;

  U32 current_context;
  LAScontextRGBNIR14 contexts[4];

  BOOL createAndInitModelsAndDecompressors(U32 context, const U8* item);
};

class LASreadItemCompressed_WAVEPACKET14_v3 : public LASreadItemCompressed
{
public:

  LASreadItemCompressed_WAVEPACKET14_v3(ArithmeticDecoder* dec, const U32 decompress_selective=LASZIP_DECOMPRESS_SELECTIVE_ALL);

  BOOL chunk_sizes();
  BOOL init(const U8* item, U32& context); // context is only read
  void read(U8* item, U32& context);       // context is only read

  ~LASreadItemCompressed_WAVEPACKET14_v3();

private:

  /* not used as a decoder. just gives access to instream */

  ArithmeticDecoder* dec;

  ByteStreamInArray* instream_wavepacket;

  ArithmeticDecoder* dec_wavepacket;

  BOOL changed_wavepacket;

  U32 num_bytes_wavepacket;

  BOOL requested_wavepacket;

  U8* bytes;
  U32 num_bytes_allocated;

  U32 current_context;
  LAScontextWAVEPACKET14 contexts[4];

  BOOL createAndInitModelsAndDecompressors(U32 context, const U8* item);
};

class LASreadItemCompressed_BYTE14_v3 : public LASreadItemCompressed
{
public:

  LASreadItemCompressed_BYTE14_v3(ArithmeticDecoder* dec, U32 number, const U32 decompress_selective=LASZIP_DECOMPRESS_SELECTIVE_ALL);

  BOOL chunk_sizes();
  BOOL init(const U8* item, U32& context); // context is only read
  void read(U8* item, U32& context);       // context is only read

  ~LASreadItemCompressed_BYTE14_v3();

private:

  /* not used as a decoder. just gives access to instream */

  ArithmeticDecoder* dec;

  ByteStreamInArray** instream_Bytes;

  ArithmeticDecoder** dec_Bytes;

  U32* num_bytes_Bytes;

  BOOL* changed_Bytes;

  BOOL* requested_Bytes;

  U8* bytes;
  U32 num_bytes_allocated;

  U32 current_context;
  LAScontextBYTE14 contexts[4];

  U32 number;
  BOOL createAndInitModelsAndDecompressors(U32 context, const U8* item);
};

#endif
