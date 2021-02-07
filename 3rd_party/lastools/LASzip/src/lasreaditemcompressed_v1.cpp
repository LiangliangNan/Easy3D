/*
===============================================================================

  FILE:  lasreaditemcompressed_v1.cpp
  
  CONTENTS:
  
    see corresponding header file
  
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
  
    see corresponding header file
  
===============================================================================
*/

#include "lasreaditemcompressed_v1.hpp"
#include "laszip_common_v1.hpp"

#include <assert.h>
#include <string.h>

/*
===============================================================================
                       LASreadItemCompressed_POINT10_v1
===============================================================================
*/

struct LASpoint10
{
  I32 x;
  I32 y;
  I32 z;
  U16 intensity;
  U8 return_number : 3;
  U8 number_of_returns_of_given_pulse : 3;
  U8 scan_direction_flag : 1;
  U8 edge_of_flight_line : 1;
  U8 classification;
  I8 scan_angle_rank;
  U8 user_data;
  U16 point_source_ID;
};

LASreadItemCompressed_POINT10_v1::LASreadItemCompressed_POINT10_v1(ArithmeticDecoder* dec)
{
  U32 i;

  /* set decoder */
  assert(dec);
  this->dec = dec;

  /* create models and integer compressors */
  ic_dx = new IntegerCompressor(dec, 32);  // 32 bits, 1 context
  ic_dy = new IntegerCompressor(dec, 32, 20); // 32 bits, 20 contexts
  ic_z = new IntegerCompressor(dec, 32, 20);  // 32 bits, 20 contexts
  ic_intensity = new IntegerCompressor(dec, 16);
  ic_scan_angle_rank = new IntegerCompressor(dec, 8, 2);
  ic_point_source_ID = new IntegerCompressor(dec, 16);
  m_changed_values = dec->createSymbolModel(64);
  for (i = 0; i < 256; i++)
  {
    m_bit_byte[i] = 0;
    m_classification[i] = 0;
    m_user_data[i] = 0;
  }
}

LASreadItemCompressed_POINT10_v1::~LASreadItemCompressed_POINT10_v1()
{
  U32 i;

  delete ic_dx;
  delete ic_dy;
  delete ic_z;
  delete ic_intensity;
  delete ic_scan_angle_rank;
  delete ic_point_source_ID;
  dec->destroySymbolModel(m_changed_values);
  for (i = 0; i < 256; i++)
  {
    if (m_bit_byte[i]) dec->destroySymbolModel(m_bit_byte[i]);
    if (m_classification[i]) dec->destroySymbolModel(m_classification[i]);
    if (m_user_data[i]) dec->destroySymbolModel(m_user_data[i]);
  }
}

BOOL LASreadItemCompressed_POINT10_v1::init(const U8* item, U32& context)
{
  U32 i;

  /* init state */
  last_x_diff[0] = last_x_diff[1] = last_x_diff[2] = 0;
  last_y_diff[0] = last_y_diff[1] = last_y_diff[2] = 0;
  last_incr = 0;

  /* init models and integer compressors */
  ic_dx->initDecompressor();
  ic_dy->initDecompressor();
  ic_z->initDecompressor();
  ic_intensity->initDecompressor();
  ic_scan_angle_rank->initDecompressor();
  ic_point_source_ID->initDecompressor();
  dec->initSymbolModel(m_changed_values);
  for (i = 0; i < 256; i++)
  {
    if (m_bit_byte[i]) dec->initSymbolModel(m_bit_byte[i]);
    if (m_classification[i]) dec->initSymbolModel(m_classification[i]);
    if (m_user_data[i]) dec->initSymbolModel(m_user_data[i]);
  }

  /* init last item */
  memcpy(last_item, item, 20);

  return TRUE;
}

inline void LASreadItemCompressed_POINT10_v1::read(U8* item, U32& context)
{
  // find median difference for x and y from 3 preceding differences
  I32 median_x;
  if (last_x_diff[0] < last_x_diff[1])
  {
    if (last_x_diff[1] < last_x_diff[2])
      median_x = last_x_diff[1];
    else if (last_x_diff[0] < last_x_diff[2])
      median_x = last_x_diff[2];
    else
      median_x = last_x_diff[0];
  }
  else
  {
    if (last_x_diff[0] < last_x_diff[2])
      median_x = last_x_diff[0];
    else if (last_x_diff[1] < last_x_diff[2])
      median_x = last_x_diff[2];
    else
      median_x = last_x_diff[1];
  }

  I32 median_y;
  if (last_y_diff[0] < last_y_diff[1])
  {
    if (last_y_diff[1] < last_y_diff[2])
      median_y = last_y_diff[1];
    else if (last_y_diff[0] < last_y_diff[2])
      median_y = last_y_diff[2];
    else
      median_y = last_y_diff[0];
  }
  else
  {
    if (last_y_diff[0] < last_y_diff[2])
      median_y = last_y_diff[0];
    else if (last_y_diff[1] < last_y_diff[2])
      median_y = last_y_diff[2];
    else
      median_y = last_y_diff[1];
  }

  // decompress x y z coordinates
  I32 x_diff = ic_dx->decompress(median_x);
  ((LASpoint10*)last_item)->x += x_diff;
  // we use the number k of bits corrector bits to switch contexts
  U32 k_bits = ic_dx->getK();
  I32 y_diff = ic_dy->decompress(median_y, (k_bits < 19 ? k_bits : 19));
  ((LASpoint10*)last_item)->y += y_diff;
  k_bits = (k_bits + ic_dy->getK())/2;
  ((LASpoint10*)last_item)->z = ic_z->decompress(((LASpoint10*)last_item)->z, (k_bits < 19 ? k_bits : 19));

  // decompress which other values have changed
  I32 changed_values = dec->decodeSymbol(m_changed_values);

  if (changed_values)
  {
    // decompress the intensity if it has changed
    if (changed_values & 32)
    {
      ((LASpoint10*)last_item)->intensity = (U16)ic_intensity->decompress(((LASpoint10*)last_item)->intensity);
    }

    // decompress the edge_of_flight_line, scan_direction_flag, ... if it has changed
    if (changed_values & 16)
    {
      if (m_bit_byte[last_item[14]] == 0)
      {
        m_bit_byte[last_item[14]] = dec->createSymbolModel(256);
        dec->initSymbolModel(m_bit_byte[last_item[14]]);
      }
      last_item[14] = (U8)dec->decodeSymbol(m_bit_byte[last_item[14]]);
    }

    // decompress the classification ... if it has changed
    if (changed_values & 8)
    {
      if (m_classification[last_item[15]] == 0)
      {
        m_classification[last_item[15]] = dec->createSymbolModel(256);
        dec->initSymbolModel(m_classification[last_item[15]]);
      }
      last_item[15] = (U8)dec->decodeSymbol(m_classification[last_item[15]]);
    }
    
    // decompress the scan_angle_rank ... if it has changed
    if (changed_values & 4)
    {
      last_item[16] = (U8)ic_scan_angle_rank->decompress(last_item[16], k_bits < 3);
    }

    // decompress the user_data ... if it has changed
    if (changed_values & 2)
    {
      if (m_user_data[last_item[17]] == 0)
      {
        m_user_data[last_item[17]] = dec->createSymbolModel(256);
        dec->initSymbolModel(m_user_data[last_item[17]]);
      }
      last_item[17] = (U8)dec->decodeSymbol(m_user_data[last_item[17]]);
    }

    // decompress the point_source_ID ... if it has changed
    if (changed_values & 1)
    {
      ((LASpoint10*)last_item)->point_source_ID = (U16)ic_point_source_ID->decompress(((LASpoint10*)last_item)->point_source_ID);
    }
  }

  // record the difference
  last_x_diff[last_incr] = x_diff;
  last_y_diff[last_incr] = y_diff;
  last_incr++;
  if (last_incr > 2) last_incr = 0;

  // copy the last point
  memcpy(item, last_item, 20);
}

/*
===============================================================================
                       LASreadItemCompressed_GPSTIME11_v1
===============================================================================
*/

#define LASZIP_GPSTIME_MULTIMAX 512

LASreadItemCompressed_GPSTIME11_v1::LASreadItemCompressed_GPSTIME11_v1(ArithmeticDecoder* dec)
{
  /* set decoder */
  assert(dec);
  this->dec = dec;
  /* create entropy models and integer compressors */
  m_gpstime_multi = dec->createSymbolModel(LASZIP_GPSTIME_MULTIMAX);
  m_gpstime_0diff = dec->createSymbolModel(3);
  ic_gpstime = new IntegerCompressor(dec, 32, 6); // 32 bits, 6 contexts
}

LASreadItemCompressed_GPSTIME11_v1::~LASreadItemCompressed_GPSTIME11_v1()
{
  dec->destroySymbolModel(m_gpstime_multi);
  dec->destroySymbolModel(m_gpstime_0diff);
  delete ic_gpstime;
}

BOOL LASreadItemCompressed_GPSTIME11_v1::init(const U8* item, U32& context)
{
  /* init state */
  last_gpstime_diff = 0;
  multi_extreme_counter = 0;

  /* init models and integer compressors */
  dec->initSymbolModel(m_gpstime_multi);
  dec->initSymbolModel(m_gpstime_0diff);
  ic_gpstime->initDecompressor();

  /* init last item */
  last_gpstime.u64 = *((U64*)item);
  return TRUE;
}

inline void LASreadItemCompressed_GPSTIME11_v1::read(U8* item, U32& context)
{
  I32 multi;
  if (last_gpstime_diff == 0) // if the last integer difference was zero
  {
    multi = dec->decodeSymbol(m_gpstime_0diff);
    if (multi == 1) // the difference can be represented with 32 bits
    {
      last_gpstime_diff = ic_gpstime->decompress(0, 0);
      last_gpstime.i64 += last_gpstime_diff;      
    }
    else if (multi == 2) // the difference is huge
    {
      last_gpstime.u64 = dec->readInt64();
    }
  }
  else
  {
    multi = dec->decodeSymbol(m_gpstime_multi);

    if (multi <  LASZIP_GPSTIME_MULTIMAX-2)
    {
      I32 gpstime_diff;
      if (multi == 1)
      {
        gpstime_diff = ic_gpstime->decompress(last_gpstime_diff, 1);
        last_gpstime_diff = gpstime_diff;
        multi_extreme_counter = 0;
      }
      else if (multi == 0)
      {
        gpstime_diff = ic_gpstime->decompress(last_gpstime_diff/4, 2);
        multi_extreme_counter++;
        if (multi_extreme_counter > 3)
        {
          last_gpstime_diff = gpstime_diff;
          multi_extreme_counter = 0;
        }
      }
      else if (multi < 10)
      {
        gpstime_diff = ic_gpstime->decompress(multi*last_gpstime_diff, 3);
      }
      else if (multi < 50)
      {
        gpstime_diff = ic_gpstime->decompress(multi*last_gpstime_diff, 4);
      }
      else
      {
        gpstime_diff = ic_gpstime->decompress(multi*last_gpstime_diff, 5);
        if (multi == LASZIP_GPSTIME_MULTIMAX-3)
        {
          multi_extreme_counter++;
          if (multi_extreme_counter > 3)
          {
            last_gpstime_diff = gpstime_diff;
            multi_extreme_counter = 0;
          }
        }
      }
      last_gpstime.i64 += gpstime_diff;
    }
    else if (multi <  LASZIP_GPSTIME_MULTIMAX-1)
    {
      last_gpstime.u64 = dec->readInt64();
    }
  }
  *((I64*)item) = last_gpstime.i64;
}

/*
===============================================================================
                       LASreadItemCompressed_RGB12_v1
===============================================================================
*/

LASreadItemCompressed_RGB12_v1::LASreadItemCompressed_RGB12_v1(ArithmeticDecoder* dec)
{
  /* set decoder */
  assert(dec);
  this->dec = dec;

  /* create models and integer compressors */
  m_byte_used = dec->createSymbolModel(64);
  ic_rgb = new IntegerCompressor(dec, 8, 6);

  /* create last item */
  last_item = new U8[6];
}

LASreadItemCompressed_RGB12_v1::~LASreadItemCompressed_RGB12_v1()
{
  dec->destroySymbolModel(m_byte_used);
  delete ic_rgb;
  delete [] last_item;
}

BOOL LASreadItemCompressed_RGB12_v1::init(const U8* item, U32& context)
{
  /* init state */

  /* init models and integer compressors */
  dec->initSymbolModel(m_byte_used);
  ic_rgb->initDecompressor();

  /* init last item */
  memcpy(last_item, item, 6);
  return TRUE;
}

inline void LASreadItemCompressed_RGB12_v1::read(U8* item, U32& context)
{
  U32 sym = dec->decodeSymbol(m_byte_used);
  if (sym & (1 << 0)) ((U16*)item)[0] = (U16)ic_rgb->decompress(((U16*)last_item)[0]&255, 0);
  else ((U16*)item)[0] = (U16)(((U16*)last_item)[0]&0xFF);
  if (sym & (1 << 1)) ((U16*)item)[0] |= (((U16)ic_rgb->decompress(((U16*)last_item)[0]>>8, 1)) << 8);
  else ((U16*)item)[0] |= (((U16*)last_item)[0]&0xFF00);
  if (sym & (1 << 2)) ((U16*)item)[1] = (U16)ic_rgb->decompress(((U16*)last_item)[1]&255, 2);
  else ((U16*)item)[1] = (U16)(((U16*)last_item)[1]&0xFF);
  if (sym & (1 << 3)) ((U16*)item)[1] |= (((U16)ic_rgb->decompress(((U16*)last_item)[1]>>8, 3)) << 8);
  else ((U16*)item)[1] |= (((U16*)last_item)[1]&0xFF00);
  if (sym & (1 << 4)) ((U16*)item)[2] = (U16)ic_rgb->decompress(((U16*)last_item)[2]&255, 4);
  else ((U16*)item)[2] = (U16)(((U16*)last_item)[2]&0xFF);
  if (sym & (1 << 5)) ((U16*)item)[2] |= (((U16)ic_rgb->decompress(((U16*)last_item)[2]>>8, 5)) << 8);
  else ((U16*)item)[2] |= (((U16*)last_item)[2]&0xFF00);
  memcpy(last_item, item, 6);
}

/*
===============================================================================
                       LASreadItemCompressed_WAVEPACKET13_v1
===============================================================================
*/

LASreadItemCompressed_WAVEPACKET13_v1::LASreadItemCompressed_WAVEPACKET13_v1(ArithmeticDecoder* dec)
{
  /* set decoder */
  assert(dec);
  this->dec = dec;

  /* create models and integer compressors */
  m_packet_index = dec->createSymbolModel(256);
  m_offset_diff[0] = dec->createSymbolModel(4);
  m_offset_diff[1] = dec->createSymbolModel(4);
  m_offset_diff[2] = dec->createSymbolModel(4);
  m_offset_diff[3] = dec->createSymbolModel(4);
  ic_offset_diff = new IntegerCompressor(dec, 32);
  ic_packet_size = new IntegerCompressor(dec, 32);
  ic_return_point = new IntegerCompressor(dec, 32);
  ic_xyz = new IntegerCompressor(dec, 32, 3);

  /* create last item */
  last_item = new U8[28];
}

LASreadItemCompressed_WAVEPACKET13_v1::~LASreadItemCompressed_WAVEPACKET13_v1()
{
  dec->destroySymbolModel(m_packet_index);
  dec->destroySymbolModel(m_offset_diff[0]);
  dec->destroySymbolModel(m_offset_diff[1]);
  dec->destroySymbolModel(m_offset_diff[2]);
  dec->destroySymbolModel(m_offset_diff[3]);
  delete ic_offset_diff;
  delete ic_packet_size;
  delete ic_return_point;
  delete ic_xyz;
  delete [] last_item;
}

BOOL LASreadItemCompressed_WAVEPACKET13_v1::init(const U8* item, U32& context)
{
  /* init state */
  last_diff_32 = 0;
  sym_last_offset_diff = 0;

  /* init models and integer compressors */
  dec->initSymbolModel(m_packet_index);
  dec->initSymbolModel(m_offset_diff[0]);
  dec->initSymbolModel(m_offset_diff[1]);
  dec->initSymbolModel(m_offset_diff[2]);
  dec->initSymbolModel(m_offset_diff[3]);
  ic_offset_diff->initDecompressor();
  ic_packet_size->initDecompressor();
  ic_return_point->initDecompressor();
  ic_xyz->initDecompressor();

  /* init last item */
  item++;
  memcpy(last_item, item, 28);
  return TRUE;
}

inline void LASreadItemCompressed_WAVEPACKET13_v1::read(U8* item, U32& context)
{
  item[0] = (U8)(dec->decodeSymbol(m_packet_index));
  item++;

  LASwavepacket13 this_item_m;
  LASwavepacket13 last_item_m = LASwavepacket13::unpack(last_item);

  sym_last_offset_diff = dec->decodeSymbol(m_offset_diff[sym_last_offset_diff]);

  if (sym_last_offset_diff == 0)
  {
    this_item_m.offset = last_item_m.offset;
  }
  else if (sym_last_offset_diff == 1)
  {
    this_item_m.offset = last_item_m.offset + last_item_m.packet_size;
  }
  else if (sym_last_offset_diff == 2)
  {
    last_diff_32 = ic_offset_diff->decompress(last_diff_32);
    this_item_m.offset = last_item_m.offset + last_diff_32;
  }
  else
  {
    this_item_m.offset = dec->readInt64();
  }

  this_item_m.packet_size = ic_packet_size->decompress(last_item_m.packet_size);
  this_item_m.return_point.i32 = ic_return_point->decompress(last_item_m.return_point.i32);
  this_item_m.x.i32 = ic_xyz->decompress(last_item_m.x.i32, 0);
  this_item_m.y.i32 = ic_xyz->decompress(last_item_m.y.i32, 1);
  this_item_m.z.i32 = ic_xyz->decompress(last_item_m.z.i32, 2);

  this_item_m.pack(item);

  memcpy(last_item, item, 28);
}

/*
===============================================================================
                       LASreadItemCompressed_BYTE_v1
===============================================================================
*/

LASreadItemCompressed_BYTE_v1::LASreadItemCompressed_BYTE_v1(ArithmeticDecoder* dec, U32 number)
{
  /* set decoder */
  assert(dec);
  this->dec = dec;
  assert(number);
  this->number = number;

  /* create models and integer compressors */
  ic_byte = new IntegerCompressor(dec, 8, number);

  /* create last item */
  last_item = new U8[number];
}

LASreadItemCompressed_BYTE_v1::~LASreadItemCompressed_BYTE_v1()
{
  delete ic_byte;
  delete [] last_item;
}

BOOL LASreadItemCompressed_BYTE_v1::init(const U8* item, U32& context)
{
  /* init state */

  /* init models and integer compressors */
  ic_byte->initDecompressor();

  /* init last item */
  memcpy(last_item, item, number);
  return TRUE;
}

inline void LASreadItemCompressed_BYTE_v1::read(U8* item, U32& context)
{
  U32 i;
  for (i = 0; i < number; i++)
  {
    item[i] = (U8)(ic_byte->decompress(last_item[i], i));
  }
  memcpy(last_item, item, number);
}

// vim: set ts=2 sw=2 expandtabs
