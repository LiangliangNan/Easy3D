/*
===============================================================================

  FILE:  laswriteitemcompressed_v1.cpp
  
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

#include "laswriteitemcompressed_v1.hpp"
#include "laszip_common_v1.hpp"

#include <assert.h>
#include <string.h>

/*
===============================================================================
                       LASwriteItemCompressed_POINT10_v1
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

LASwriteItemCompressed_POINT10_v1::LASwriteItemCompressed_POINT10_v1(ArithmeticEncoder* enc)
{
  U32 i;

  /* set encoder */
  assert(enc);
  this->enc = enc;

  /* create models and integer compressors */
  ic_dx = new IntegerCompressor(enc, 32);  // 32 bits, 1 context
  ic_dy = new IntegerCompressor(enc, 32, 20); // 32 bits, 20 contexts
  ic_z = new IntegerCompressor(enc, 32, 20);  // 32 bits, 20 contexts
  ic_intensity = new IntegerCompressor(enc, 16);
  ic_scan_angle_rank = new IntegerCompressor(enc, 8, 2);
  ic_point_source_ID = new IntegerCompressor(enc, 16);
  m_changed_values = enc->createSymbolModel(64);
  for (i = 0; i < 256; i++)
  {
    m_bit_byte[i] = 0;
    m_classification[i] = 0;
    m_user_data[i] = 0;
  }
}

LASwriteItemCompressed_POINT10_v1::~LASwriteItemCompressed_POINT10_v1()
{
  U32 i;
  delete ic_dx;
  delete ic_dy;
  delete ic_z;
  delete ic_intensity;
  delete ic_scan_angle_rank;
  delete ic_point_source_ID;
  enc->destroySymbolModel(m_changed_values);
  for (i = 0; i < 256; i++)
  {
    if (m_bit_byte[i]) enc->destroySymbolModel(m_bit_byte[i]);
    if (m_classification[i]) enc->destroySymbolModel(m_classification[i]);
    if (m_user_data[i]) enc->destroySymbolModel(m_user_data[i]);
  }
}

BOOL LASwriteItemCompressed_POINT10_v1::init(const U8* item, U32& context)
{
  U32 i;

  /* init state */
  last_x_diff[0] = last_x_diff[1] = last_x_diff[2] = 0;
  last_y_diff[0] = last_y_diff[1] = last_y_diff[2] = 0;
  last_incr = 0;

  /* init models and integer compressors */
  ic_dx->initCompressor();
  ic_dy->initCompressor();
  ic_z->initCompressor();
  ic_intensity->initCompressor();
  ic_scan_angle_rank->initCompressor();
  ic_point_source_ID->initCompressor();
  enc->initSymbolModel(m_changed_values);
  for (i = 0; i < 256; i++)
  {
    if (m_bit_byte[i]) enc->initSymbolModel(m_bit_byte[i]);
    if (m_classification[i]) enc->initSymbolModel(m_classification[i]);
    if (m_user_data[i]) enc->initSymbolModel(m_user_data[i]);
  }

  /* init last item */
  memcpy(last_item, item, 20);

  return TRUE;
}

inline BOOL LASwriteItemCompressed_POINT10_v1::write(const U8* item, U32& context)
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

  // compress x y z coordinates
  I32 x_diff = ((LASpoint10*)item)->x - ((LASpoint10*)last_item)->x;
  I32 y_diff = ((LASpoint10*)item)->y - ((LASpoint10*)last_item)->y;

  ic_dx->compress(median_x, x_diff);
  // we use the number k of bits corrector bits to switch contexts
  U32 k_bits = ic_dx->getK();
  ic_dy->compress(median_y, y_diff, (k_bits < 19 ? k_bits : 19));
  k_bits = (k_bits + ic_dy->getK()) / 2;
  ic_z->compress(((LASpoint10*)last_item)->z, ((LASpoint10*)item)->z, (k_bits < 19 ? k_bits : 19));

  // compress which other values have changed
  I32 changed_values = ((((LASpoint10*)last_item)->intensity != ((LASpoint10*)item)->intensity) << 5) |
                       ((last_item[14] != item[14]) << 4) | // bit_byte
                       ((last_item[15] != item[15]) << 3) | // classification
                       ((last_item[16] != item[16]) << 2) | // scan_angle_rank
                       ((last_item[17] != item[17]) << 1) | // user_data
                       ((((LASpoint10*)last_item)->point_source_ID != ((LASpoint10*)item)->point_source_ID));

  enc->encodeSymbol(m_changed_values, changed_values);

  // compress the intensity if it has changed
  if (changed_values & 32)
  {
    ic_intensity->compress(((LASpoint10*)last_item)->intensity, ((LASpoint10*)item)->intensity);
  }

  // compress the edge_of_flight_line, scan_direction_flag, ... if it has changed
  if (changed_values & 16)
  {
    if (m_bit_byte[last_item[14]] == 0)
    {
      m_bit_byte[last_item[14]] = enc->createSymbolModel(256);
      enc->initSymbolModel(m_bit_byte[last_item[14]]);
    }
    enc->encodeSymbol(m_bit_byte[last_item[14]], item[14]);
  }

  // compress the classification ... if it has changed
  if (changed_values & 8)
  {
    if (m_classification[last_item[15]] == 0)
    {
      m_classification[last_item[15]] = enc->createSymbolModel(256);
      enc->initSymbolModel(m_classification[last_item[15]]);
    }
    enc->encodeSymbol(m_classification[last_item[15]], item[15]);
  }
  
  // compress the scan_angle_rank ... if it has changed
  if (changed_values & 4)
  {
    ic_scan_angle_rank->compress(last_item[16], item[16], k_bits < 3);
  }

  // compress the user_data ... if it has changed
  if (changed_values & 2)
  {
    if (m_user_data[last_item[17]] == 0)
    {
      m_user_data[last_item[17]] = enc->createSymbolModel(256);
      enc->initSymbolModel(m_user_data[last_item[17]]);
    }
    enc->encodeSymbol(m_user_data[last_item[17]], item[17]);
  }

  // compress the point_source_ID ... if it has changed
  if (changed_values & 1)
  {
    ic_point_source_ID->compress(((LASpoint10*)last_item)->point_source_ID, ((LASpoint10*)item)->point_source_ID);
  }

  // record the difference
  last_x_diff[last_incr] = x_diff;
  last_y_diff[last_incr] = y_diff;
  last_incr++;
  if (last_incr > 2) last_incr = 0;

  // copy the last item
  memcpy(last_item, item, 20);
  return TRUE;
}

/*
===============================================================================
                       LASwriteItemCompressed_GPSTIME11_v1
===============================================================================
*/

#define LASZIP_GPSTIME_MULTIMAX 512

LASwriteItemCompressed_GPSTIME11_v1::LASwriteItemCompressed_GPSTIME11_v1(ArithmeticEncoder* enc)
{
  /* set encoder */
  assert(enc);
  this->enc = enc;
  /* create entropy models and integer compressors */
  m_gpstime_multi = enc->createSymbolModel(LASZIP_GPSTIME_MULTIMAX);
  m_gpstime_0diff = enc->createSymbolModel(3);
  ic_gpstime = new IntegerCompressor(enc, 32, 6); // 32 bits, 6 contexts
}

LASwriteItemCompressed_GPSTIME11_v1::~LASwriteItemCompressed_GPSTIME11_v1()
{
  enc->destroySymbolModel(m_gpstime_multi);
  enc->destroySymbolModel(m_gpstime_0diff);
  delete ic_gpstime;
}

BOOL LASwriteItemCompressed_GPSTIME11_v1::init(const U8* item, U32& context)
{
  /* init state */
  last_gpstime_diff = 0;
  multi_extreme_counter = 0;

  /* init models and integer compressors */
  enc->initSymbolModel(m_gpstime_multi);
  enc->initSymbolModel(m_gpstime_0diff);
  ic_gpstime->initCompressor();

  /* init last item */
  last_gpstime.u64 = *((U64*)item);
  return TRUE;
}

inline BOOL LASwriteItemCompressed_GPSTIME11_v1::write(const U8* item, U32& context)
{
  U64I64F64 this_gpstime;
  this_gpstime.i64 = *((I64*)item);

  if (last_gpstime_diff == 0) // if the last integer difference was zero
  {
    if (this_gpstime.i64 == last_gpstime.i64)
    {
      enc->encodeSymbol(m_gpstime_0diff, 0); // the doubles have not changed
    }
    else
    {
      // calculate the difference between the two doubles as an integer
      I64 curr_gpstime_diff_64 = this_gpstime.i64 - last_gpstime.i64;
      I32 curr_gpstime_diff = (I32)curr_gpstime_diff_64;
      if (curr_gpstime_diff_64 == (I64)(curr_gpstime_diff))
      {
        enc->encodeSymbol(m_gpstime_0diff, 1); // the difference can be represented with 32 bits
        ic_gpstime->compress(0, curr_gpstime_diff, 0);
        last_gpstime_diff = curr_gpstime_diff;
      }
      else
      {
        enc->encodeSymbol(m_gpstime_0diff, 2); // the difference is huge
        enc->writeInt64(this_gpstime.u64);
      }
      last_gpstime.i64 = this_gpstime.i64;
    }
  }
  else // the last integer difference was *not* zero
  {
    if (this_gpstime.i64 == last_gpstime.i64)
    {
      // if the doubles have not changed use a special symbol
      enc->encodeSymbol(m_gpstime_multi, LASZIP_GPSTIME_MULTIMAX-1);
    }
    else
    {
      // calculate the difference between the two doubles as an integer
      I64 curr_gpstime_diff_64 = this_gpstime.i64 - last_gpstime.i64;
      I32 curr_gpstime_diff = (I32)curr_gpstime_diff_64;
      // if the current gpstime difference can be represented with 32 bits
      if (curr_gpstime_diff_64 == (I64)(curr_gpstime_diff))
      {
        // compute multiplier between current and last integer difference
        I32 multi = (I32)(((F32)curr_gpstime_diff / (F32)last_gpstime_diff) + 0.5f);

        // limit the multiplier into some bounds
        if (multi >= LASZIP_GPSTIME_MULTIMAX-3)
        {
          multi = LASZIP_GPSTIME_MULTIMAX-3;
        }
        else if (multi <= 0)
        {
          multi = 0;
        }
        // compress this multiplier
        enc->encodeSymbol(m_gpstime_multi, multi);
        // compress the residual curr_gpstime_diff in dependance on the multiplier
        if (multi == 1)
        {
          // this is the case we assume we get most often
          ic_gpstime->compress(last_gpstime_diff, curr_gpstime_diff, 1);
          last_gpstime_diff = curr_gpstime_diff;
          multi_extreme_counter = 0;    
        }
        else
        {
          if (multi == 0)
          {
            ic_gpstime->compress(last_gpstime_diff/4, curr_gpstime_diff, 2);
            multi_extreme_counter++;
            if (multi_extreme_counter > 3)
            {
              last_gpstime_diff = curr_gpstime_diff;
              multi_extreme_counter = 0;
            }
          }
          else if (multi < 10)
          {
            ic_gpstime->compress(multi*last_gpstime_diff, curr_gpstime_diff, 3);
          }
          else if (multi < 50)
          {
            ic_gpstime->compress(multi*last_gpstime_diff, curr_gpstime_diff, 4);
          }
          else
          {
            ic_gpstime->compress(multi*last_gpstime_diff, curr_gpstime_diff, 5);
            if (multi == LASZIP_GPSTIME_MULTIMAX-3)
            {
              multi_extreme_counter++;
              if (multi_extreme_counter > 3)
              {
                last_gpstime_diff = curr_gpstime_diff;
                multi_extreme_counter = 0;
              }
            }
          }
        }
      }
      else
      {
        // if difference is so huge ... we simply write the double
        enc->encodeSymbol(m_gpstime_multi, LASZIP_GPSTIME_MULTIMAX-2);
        enc->writeInt64(this_gpstime.u64);
      }
      last_gpstime.i64 = this_gpstime.i64;
    }
  }
  return TRUE;
}

/*
===============================================================================
                       LASwriteItemCompressed_RGB12_v1
===============================================================================
*/

LASwriteItemCompressed_RGB12_v1::LASwriteItemCompressed_RGB12_v1(ArithmeticEncoder* enc)
{
  /* set encoder */
  assert(enc);
  this->enc = enc;

  /* create models and integer compressors */
  m_byte_used = enc->createSymbolModel(64);
  ic_rgb = new IntegerCompressor(enc, 8, 6);

  /* create last item */
  last_item = new U8[6];
}

LASwriteItemCompressed_RGB12_v1::~LASwriteItemCompressed_RGB12_v1()
{
  enc->destroySymbolModel(m_byte_used);
  delete ic_rgb;
  delete [] last_item;
}

BOOL LASwriteItemCompressed_RGB12_v1::init(const U8* item, U32& context)
{
  /* init state */

  /* init models and integer compressors */
  enc->initSymbolModel(m_byte_used);
  ic_rgb->initCompressor();

  /* init last item */
  memcpy(last_item, item, 6);
  return TRUE;
}

inline BOOL LASwriteItemCompressed_RGB12_v1::write(const U8* item, U32& context)
{
  U32 sym = ((((U16*)last_item)[0]&0x00FF) != (((U16*)item)[0]&0x00FF)) << 0;
  sym |= ((((U16*)last_item)[0]&0xFF00) != (((U16*)item)[0]&0xFF00)) << 1;
  sym |= ((((U16*)last_item)[1]&0x00FF) != (((U16*)item)[1]&0x00FF)) << 2;
  sym |= ((((U16*)last_item)[1]&0xFF00) != (((U16*)item)[1]&0xFF00)) << 3;
  sym |= ((((U16*)last_item)[2]&0x00FF) != (((U16*)item)[2]&0x00FF)) << 4;
  sym |= ((((U16*)last_item)[2]&0xFF00) != (((U16*)item)[2]&0xFF00)) << 5;
  enc->encodeSymbol(m_byte_used, sym);
  if (sym & (1 << 0)) ic_rgb->compress(((U16*)last_item)[0]&255,((U16*)item)[0]&255, 0);
  if (sym & (1 << 1)) ic_rgb->compress(((U16*)last_item)[0]>>8,((U16*)item)[0]>>8, 1);
  if (sym & (1 << 2)) ic_rgb->compress(((U16*)last_item)[1]&255,((U16*)item)[1]&255, 2);
  if (sym & (1 << 3)) ic_rgb->compress(((U16*)last_item)[1]>>8,((U16*)item)[1]>>8, 3);
  if (sym & (1 << 4)) ic_rgb->compress(((U16*)last_item)[2]&255,((U16*)item)[2]&255, 4);
  if (sym & (1 << 5)) ic_rgb->compress(((U16*)last_item)[2]>>8,((U16*)item)[2]>>8, 5);
  memcpy(last_item, item, 6);
  return TRUE;
}

/*
===============================================================================
                       LASwriteItemCompressed_WAVEPACKET13_v1
===============================================================================
*/

LASwriteItemCompressed_WAVEPACKET13_v1::LASwriteItemCompressed_WAVEPACKET13_v1(ArithmeticEncoder* enc)
{
  /* set encoder */
  assert(enc);
  this->enc = enc;

  /* create models and integer compressors */
  m_packet_index = enc->createSymbolModel(256);
  m_offset_diff[0] = enc->createSymbolModel(4);
  m_offset_diff[1] = enc->createSymbolModel(4);
  m_offset_diff[2] = enc->createSymbolModel(4);
  m_offset_diff[3] = enc->createSymbolModel(4);
  ic_offset_diff = new IntegerCompressor(enc, 32);
  ic_packet_size = new IntegerCompressor(enc, 32);
  ic_return_point = new IntegerCompressor(enc, 32);
  ic_xyz = new IntegerCompressor(enc, 32, 3);

  /* create last item */
  last_item = new U8[28];
}

LASwriteItemCompressed_WAVEPACKET13_v1::~LASwriteItemCompressed_WAVEPACKET13_v1()
{
  enc->destroySymbolModel(m_packet_index);
  enc->destroySymbolModel(m_offset_diff[0]);
  enc->destroySymbolModel(m_offset_diff[1]);
  enc->destroySymbolModel(m_offset_diff[2]);
  enc->destroySymbolModel(m_offset_diff[3]);
  delete ic_offset_diff;
  delete ic_packet_size;
  delete ic_return_point;
  delete ic_xyz;
  delete [] last_item;
}

BOOL LASwriteItemCompressed_WAVEPACKET13_v1::init(const U8* item, U32& context)
{
  /* init state */
  last_diff_32 = 0;
  sym_last_offset_diff = 0;

  /* init models and integer compressors */
  enc->initSymbolModel(m_packet_index);
  enc->initSymbolModel(m_offset_diff[0]);
  enc->initSymbolModel(m_offset_diff[1]);
  enc->initSymbolModel(m_offset_diff[2]);
  enc->initSymbolModel(m_offset_diff[3]);
  ic_offset_diff->initCompressor();
  ic_packet_size->initCompressor();
  ic_return_point->initCompressor();
  ic_xyz->initCompressor();

  /* init last item */
  item++;
  memcpy(last_item, item, 28);
  return TRUE;
}

inline BOOL LASwriteItemCompressed_WAVEPACKET13_v1::write(const U8* item, U32& context)
{
  enc->encodeSymbol(m_packet_index, (U32)(item[0]));
  item++;

  LASwavepacket13 this_item_m = LASwavepacket13::unpack(item);
  LASwavepacket13 last_item_m = LASwavepacket13::unpack(last_item);

  // calculate the difference between the two offsets
  I64 curr_diff_64 = this_item_m.offset - last_item_m.offset;
  I32 curr_diff_32 = (I32)curr_diff_64;

  // if the current difference can be represented with 32 bits
  if (curr_diff_64 == (I64)(curr_diff_32))
  {
    if (curr_diff_32 == 0) // current difference is zero
    {
      enc->encodeSymbol(m_offset_diff[sym_last_offset_diff], 0);
      sym_last_offset_diff = 0;
    }
    else if (curr_diff_32 == (I32)last_item_m.packet_size)
    {
      enc->encodeSymbol(m_offset_diff[sym_last_offset_diff], 1);
      sym_last_offset_diff = 1;
    }
    else //
    {
      enc->encodeSymbol(m_offset_diff[sym_last_offset_diff], 2);
      sym_last_offset_diff = 2;
      ic_offset_diff->compress(last_diff_32, curr_diff_32);
      last_diff_32 = curr_diff_32;
    }
  }
  else
  {
    enc->encodeSymbol(m_offset_diff[sym_last_offset_diff], 3);
    sym_last_offset_diff = 3;

    enc->writeInt64(this_item_m.offset);
  }

  ic_packet_size->compress(last_item_m.packet_size, this_item_m.packet_size);
  ic_return_point->compress(last_item_m.return_point.i32, this_item_m.return_point.i32);
  ic_xyz->compress(last_item_m.x.i32, this_item_m.x.i32, 0);
  ic_xyz->compress(last_item_m.y.i32, this_item_m.y.i32, 1);
  ic_xyz->compress(last_item_m.z.i32, this_item_m.z.i32, 2);

  memcpy(last_item, item, 28);
  return TRUE;
}

/*
===============================================================================
                       LASwriteItemCompressed_BYTE_v1
===============================================================================
*/

LASwriteItemCompressed_BYTE_v1::LASwriteItemCompressed_BYTE_v1(ArithmeticEncoder* enc, U32 number)
{
  /* set encoder */
  assert(enc);
  this->enc = enc;
  assert(number);
  this->number = number;

  /* create models and integer compressors */
  ic_byte = new IntegerCompressor(enc, 8, number);

  /* create last item */
  last_item = new U8[number];
}

LASwriteItemCompressed_BYTE_v1::~LASwriteItemCompressed_BYTE_v1()
{
  delete ic_byte;
  delete [] last_item;
}

BOOL LASwriteItemCompressed_BYTE_v1::init(const U8* item, U32& context)
{
  /* init state */

  /* init models and integer compressors */
  ic_byte->initCompressor();

  /* init last point */
  memcpy(last_item, item, number);
  return TRUE;
}

inline BOOL LASwriteItemCompressed_BYTE_v1::write(const U8* item, U32& context)
{
  U32 i;
  for (i = 0; i < number; i++)
  {
    ic_byte->compress(last_item[i], item[i], i);
  }
  memcpy(last_item, item, number);
  return TRUE;
}

// vim: set ts=2 sw=2 expandtabs
