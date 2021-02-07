/*
===============================================================================

  FILE:  laswriteitemcompressed_v2.cpp
  
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

#include "laswriteitemcompressed_v2.hpp"

#include <assert.h>
#include <string.h>

/*
===============================================================================
                       LASwriteItemCompressed_POINT10_v2
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

LASwriteItemCompressed_POINT10_v2::LASwriteItemCompressed_POINT10_v2(ArithmeticEncoder* enc)
{
  U32 i;

  /* set encoder */
  assert(enc);
  this->enc = enc;

  /* create models and integer compressors */
  m_changed_values = enc->createSymbolModel(64);
  ic_intensity = new IntegerCompressor(enc, 16, 4);
  m_scan_angle_rank[0] = enc->createSymbolModel(256);
  m_scan_angle_rank[1] = enc->createSymbolModel(256);
  ic_point_source_ID = new IntegerCompressor(enc, 16);
  for (i = 0; i < 256; i++)
  {
    m_bit_byte[i] = 0;
    m_classification[i] = 0;
    m_user_data[i] = 0;
  }
  ic_dx = new IntegerCompressor(enc, 32, 2);  // 32 bits, 2 context
  ic_dy = new IntegerCompressor(enc, 32, 22); // 32 bits, 22 contexts
  ic_z = new IntegerCompressor(enc, 32, 20);  // 32 bits, 20 contexts
}

LASwriteItemCompressed_POINT10_v2::~LASwriteItemCompressed_POINT10_v2()
{
  U32 i;

  enc->destroySymbolModel(m_changed_values);
  delete ic_intensity;
  enc->destroySymbolModel(m_scan_angle_rank[0]);
  enc->destroySymbolModel(m_scan_angle_rank[1]);
  delete ic_point_source_ID;
  for (i = 0; i < 256; i++)
  {
    if (m_bit_byte[i]) enc->destroySymbolModel(m_bit_byte[i]);
    if (m_classification[i]) enc->destroySymbolModel(m_classification[i]);
    if (m_user_data[i]) enc->destroySymbolModel(m_user_data[i]);
  }
  delete ic_dx;
  delete ic_dy;
  delete ic_z;
}

BOOL LASwriteItemCompressed_POINT10_v2::init(const U8* item, U32& context)
{
  U32 i;

  /* init state */
  for (i=0; i < 16; i++)
  {
    last_x_diff_median5[i].init();
    last_y_diff_median5[i].init();
    last_intensity[i] = 0;
    last_height[i/2] = 0;
  }

  /* init models and integer compressors */
  enc->initSymbolModel(m_changed_values);
  ic_intensity->initCompressor();
  enc->initSymbolModel(m_scan_angle_rank[0]);
  enc->initSymbolModel(m_scan_angle_rank[1]);
  ic_point_source_ID->initCompressor();
  for (i = 0; i < 256; i++)
  {
    if (m_bit_byte[i]) enc->initSymbolModel(m_bit_byte[i]);
    if (m_classification[i]) enc->initSymbolModel(m_classification[i]);
    if (m_user_data[i]) enc->initSymbolModel(m_user_data[i]);
  }
  ic_dx->initCompressor();
  ic_dy->initCompressor();
  ic_z->initCompressor();

  /* init last item */
  memcpy(last_item, item, 20);

  return TRUE;
}

inline BOOL LASwriteItemCompressed_POINT10_v2::write(const U8* item, U32& context)
{
  U32 r = ((LASpoint10*)item)->return_number;
  U32 n = ((LASpoint10*)item)->number_of_returns_of_given_pulse;
  U32 m = number_return_map[n][r];
  U32 l = number_return_level[n][r];
  U32 k_bits;
  I32 median, diff;

  // compress which other values have changed
  I32 changed_values = (((last_item[14] != item[14]) << 5) | // bit_byte
                        ((last_intensity[m] != ((LASpoint10*)item)->intensity) << 4) |
                        ((last_item[15] != item[15]) << 3) | // classification
                        ((last_item[16] != item[16]) << 2) | // scan_angle_rank
                        ((last_item[17] != item[17]) << 1) | // user_data
                        (((LASpoint10*)last_item)->point_source_ID != ((LASpoint10*)item)->point_source_ID));

  enc->encodeSymbol(m_changed_values, changed_values);

  // compress the bit_byte (edge_of_flight_line, scan_direction_flag, returns, ...) if it has changed
  if (changed_values & 32)
  {
    if (m_bit_byte[last_item[14]] == 0)
    {
      m_bit_byte[last_item[14]] = enc->createSymbolModel(256);
      enc->initSymbolModel(m_bit_byte[last_item[14]]);
    }
    enc->encodeSymbol(m_bit_byte[last_item[14]], item[14]);
  }

  // compress the intensity if it has changed
  if (changed_values & 16)
  {
    ic_intensity->compress(last_intensity[m], ((LASpoint10*)item)->intensity, (m < 3 ? m : 3));
    last_intensity[m] = ((LASpoint10*)item)->intensity;
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
    enc->encodeSymbol(m_scan_angle_rank[((LASpoint10*)item)->scan_direction_flag], U8_FOLD(item[16]-last_item[16]));
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

  // compress x coordinate
  median = last_x_diff_median5[m].get();
  diff = ((LASpoint10*)item)->x - ((LASpoint10*)last_item)->x;
  ic_dx->compress(median, diff, n==1);
  last_x_diff_median5[m].add(diff);

  // compress y coordinate
  k_bits = ic_dx->getK();
  median = last_y_diff_median5[m].get();
  diff = ((LASpoint10*)item)->y - ((LASpoint10*)last_item)->y;
  ic_dy->compress(median, diff, (n==1) + ( k_bits < 20 ? U32_ZERO_BIT_0(k_bits) : 20 ));
  last_y_diff_median5[m].add(diff);

  // compress z coordinate
  k_bits = (ic_dx->getK() + ic_dy->getK()) / 2;
  ic_z->compress(last_height[l], ((LASpoint10*)item)->z, (n==1) + (k_bits < 18 ? U32_ZERO_BIT_0(k_bits) : 18));
  last_height[l] = ((LASpoint10*)item)->z;

  // copy the last item
  memcpy(last_item, item, 20);
  return TRUE;
}

/*
===============================================================================
                       LASwriteItemCompressed_GPSTIME11_v2
===============================================================================
*/

#define LASZIP_GPSTIME_MULTI 500
#define LASZIP_GPSTIME_MULTI_MINUS -10
#define LASZIP_GPSTIME_MULTI_UNCHANGED (LASZIP_GPSTIME_MULTI - LASZIP_GPSTIME_MULTI_MINUS + 1)
#define LASZIP_GPSTIME_MULTI_CODE_FULL (LASZIP_GPSTIME_MULTI - LASZIP_GPSTIME_MULTI_MINUS + 2)

#define LASZIP_GPSTIME_MULTI_TOTAL (LASZIP_GPSTIME_MULTI - LASZIP_GPSTIME_MULTI_MINUS + 6) 

LASwriteItemCompressed_GPSTIME11_v2::LASwriteItemCompressed_GPSTIME11_v2(ArithmeticEncoder* enc)
{
  /* set encoder */
  assert(enc);
  this->enc = enc;
  /* create entropy models and integer compressors */
  m_gpstime_multi = enc->createSymbolModel(LASZIP_GPSTIME_MULTI_TOTAL);
  m_gpstime_0diff = enc->createSymbolModel(6);
  ic_gpstime = new IntegerCompressor(enc, 32, 9); // 32 bits, 9 contexts
}

LASwriteItemCompressed_GPSTIME11_v2::~LASwriteItemCompressed_GPSTIME11_v2()
{
  enc->destroySymbolModel(m_gpstime_multi);
  enc->destroySymbolModel(m_gpstime_0diff);
  delete ic_gpstime;
}

BOOL LASwriteItemCompressed_GPSTIME11_v2::init(const U8* item, U32& context)
{
  /* init state */
  last = 0, next = 0;
  last_gpstime_diff[0] = 0;
  last_gpstime_diff[1] = 0;
  last_gpstime_diff[2] = 0;
  last_gpstime_diff[3] = 0;
  multi_extreme_counter[0] = 0;
  multi_extreme_counter[1] = 0;
  multi_extreme_counter[2] = 0;
  multi_extreme_counter[3] = 0;

  /* init models and integer compressors */
  enc->initSymbolModel(m_gpstime_multi);
  enc->initSymbolModel(m_gpstime_0diff);
  ic_gpstime->initCompressor();

  /* init last item */
  last_gpstime[0].u64 = *((U64*)item);
  last_gpstime[1].u64 = 0;
  last_gpstime[2].u64 = 0;
  last_gpstime[3].u64 = 0;
  return TRUE;
}

inline BOOL LASwriteItemCompressed_GPSTIME11_v2::write(const U8* item, U32& context)
{
  U64I64F64 this_gpstime;
  this_gpstime.i64 = *((I64*)item);

  if (last_gpstime_diff[last] == 0) // if the last integer difference was zero
  {
    if (this_gpstime.i64 == last_gpstime[last].i64)
    {
      enc->encodeSymbol(m_gpstime_0diff, 0); // the doubles have not changed
    }
    else
    {
      // calculate the difference between the two doubles as an integer
      I64 curr_gpstime_diff_64 = this_gpstime.i64 - last_gpstime[last].i64;
      I32 curr_gpstime_diff = (I32)curr_gpstime_diff_64;
      if (curr_gpstime_diff_64 == (I64)(curr_gpstime_diff))
      {
        enc->encodeSymbol(m_gpstime_0diff, 1); // the difference can be represented with 32 bits
        ic_gpstime->compress(0, curr_gpstime_diff, 0);
        last_gpstime_diff[last] = curr_gpstime_diff;
        multi_extreme_counter[last] = 0; 
      }
      else // the difference is huge
      {
        U32 i;
        // maybe the double belongs to another time sequence
        for (i = 1; i < 4; i++)
        {
          I64 other_gpstime_diff_64 = this_gpstime.i64 - last_gpstime[(last+i)&3].i64;
          I32 other_gpstime_diff = (I32)other_gpstime_diff_64;
          if (other_gpstime_diff_64 == (I64)(other_gpstime_diff))
          {
            enc->encodeSymbol(m_gpstime_0diff, i+2); // it belongs to another sequence 
            last = (last+i)&3;
            return write(item, context);
          }
        }
        // no other sequence found. start new sequence.
        enc->encodeSymbol(m_gpstime_0diff, 2); 
        ic_gpstime->compress((I32)(last_gpstime[last].u64 >> 32), (I32)(this_gpstime.u64 >> 32), 8);
        enc->writeInt((U32)(this_gpstime.u64));
        next = (next+1)&3;
        last = next;
        last_gpstime_diff[last] = 0;
        multi_extreme_counter[last] = 0; 
      }
      last_gpstime[last].i64 = this_gpstime.i64;
    }
  }
  else // the last integer difference was *not* zero
  {
    if (this_gpstime.i64 == last_gpstime[last].i64)
    {
      // if the doubles have not changed use a special symbol
      enc->encodeSymbol(m_gpstime_multi, LASZIP_GPSTIME_MULTI_UNCHANGED);
    }
    else
    {
      // calculate the difference between the two doubles as an integer
      I64 curr_gpstime_diff_64 = this_gpstime.i64 - last_gpstime[last].i64;
      I32 curr_gpstime_diff = (I32)curr_gpstime_diff_64;

      // if the current gpstime difference can be represented with 32 bits
      if (curr_gpstime_diff_64 == (I64)(curr_gpstime_diff))
      {
        // compute multiplier between current and last integer difference
        F32 multi_f = (F32)curr_gpstime_diff / (F32)(last_gpstime_diff[last]);
        I32 multi = I32_QUANTIZE(multi_f);

        // compress the residual curr_gpstime_diff in dependance on the multiplier
        if (multi == 1)
        {
          // this is the case we assume we get most often for regular spaced pulses
          enc->encodeSymbol(m_gpstime_multi, 1);
          ic_gpstime->compress(last_gpstime_diff[last], curr_gpstime_diff, 1);
          multi_extreme_counter[last] = 0; 
        }
        else if (multi > 0)
        {
          if (multi < LASZIP_GPSTIME_MULTI) // positive multipliers up to LASZIP_GPSTIME_MULTI are compressed directly
          {
            enc->encodeSymbol(m_gpstime_multi, multi);
            if (multi < 10)
              ic_gpstime->compress(multi*last_gpstime_diff[last], curr_gpstime_diff, 2);
            else
              ic_gpstime->compress(multi*last_gpstime_diff[last], curr_gpstime_diff, 3);
          }
          else
          {
            enc->encodeSymbol(m_gpstime_multi, LASZIP_GPSTIME_MULTI);
            ic_gpstime->compress(LASZIP_GPSTIME_MULTI*last_gpstime_diff[last], curr_gpstime_diff, 4);
            multi_extreme_counter[last]++;
            if (multi_extreme_counter[last] > 3)
            {
              last_gpstime_diff[last] = curr_gpstime_diff;
              multi_extreme_counter[last] = 0;
            }
          }
        }
        else if (multi < 0)
        {
          if (multi > LASZIP_GPSTIME_MULTI_MINUS) // negative multipliers larger than LASZIP_GPSTIME_MULTI_MINUS are compressed directly
          {
            enc->encodeSymbol(m_gpstime_multi, LASZIP_GPSTIME_MULTI - multi);
            ic_gpstime->compress(multi*last_gpstime_diff[last], curr_gpstime_diff, 5);
          }
          else
          {
            enc->encodeSymbol(m_gpstime_multi, LASZIP_GPSTIME_MULTI - LASZIP_GPSTIME_MULTI_MINUS);
            ic_gpstime->compress(LASZIP_GPSTIME_MULTI_MINUS*last_gpstime_diff[last], curr_gpstime_diff, 6);
            multi_extreme_counter[last]++;
            if (multi_extreme_counter[last] > 3)
            {
              last_gpstime_diff[last] = curr_gpstime_diff;
              multi_extreme_counter[last] = 0;
            }
          }
        }
        else
        {
          enc->encodeSymbol(m_gpstime_multi, 0);
          ic_gpstime->compress(0, curr_gpstime_diff, 7);
          multi_extreme_counter[last]++;
          if (multi_extreme_counter[last] > 3)
          {
            last_gpstime_diff[last] = curr_gpstime_diff;
            multi_extreme_counter[last] = 0;
          }
        }
      }
      else // the difference is huge
      {
        U32 i;
        // maybe the double belongs to another time sequence
        for (i = 1; i < 4; i++)
        {
          I64 other_gpstime_diff_64 = this_gpstime.i64 - last_gpstime[(last+i)&3].i64;
          I32 other_gpstime_diff = (I32)other_gpstime_diff_64;
          if (other_gpstime_diff_64 == (I64)(other_gpstime_diff))
          {
            // it belongs to this sequence 
            enc->encodeSymbol(m_gpstime_multi, LASZIP_GPSTIME_MULTI_CODE_FULL+i);
            last = (last+i)&3;
            return write(item, context);
          }
        }
        // no other sequence found. start new sequence.
        enc->encodeSymbol(m_gpstime_multi, LASZIP_GPSTIME_MULTI_CODE_FULL);
        ic_gpstime->compress((I32)(last_gpstime[last].u64 >> 32), (I32)(this_gpstime.u64 >> 32), 8);
        enc->writeInt((U32)(this_gpstime.u64));
        next = (next+1)&3;
        last = next;
        last_gpstime_diff[last] = 0;
        multi_extreme_counter[last] = 0; 
      }
      last_gpstime[last].i64 = this_gpstime.i64;
    }
  }
  return TRUE;
}

/*
===============================================================================
                       LASwriteItemCompressed_RGB12_v2
===============================================================================
*/

LASwriteItemCompressed_RGB12_v2::LASwriteItemCompressed_RGB12_v2(ArithmeticEncoder* enc)
{
  /* set encoder */
  assert(enc);
  this->enc = enc;

  /* create models and integer compressors */
  m_byte_used = enc->createSymbolModel(128);
  m_rgb_diff_0 = enc->createSymbolModel(256);
  m_rgb_diff_1 = enc->createSymbolModel(256);
  m_rgb_diff_2 = enc->createSymbolModel(256);
  m_rgb_diff_3 = enc->createSymbolModel(256);
  m_rgb_diff_4 = enc->createSymbolModel(256);
  m_rgb_diff_5 = enc->createSymbolModel(256);
}

LASwriteItemCompressed_RGB12_v2::~LASwriteItemCompressed_RGB12_v2()
{
  enc->destroySymbolModel(m_byte_used);
  enc->destroySymbolModel(m_rgb_diff_0);
  enc->destroySymbolModel(m_rgb_diff_1);
  enc->destroySymbolModel(m_rgb_diff_2);
  enc->destroySymbolModel(m_rgb_diff_3);
  enc->destroySymbolModel(m_rgb_diff_4);
  enc->destroySymbolModel(m_rgb_diff_5);
}

BOOL LASwriteItemCompressed_RGB12_v2::init(const U8* item, U32& context)
{
  /* init state */

  /* init models and integer compressors */
  enc->initSymbolModel(m_byte_used);
  enc->initSymbolModel(m_rgb_diff_0);
  enc->initSymbolModel(m_rgb_diff_1);
  enc->initSymbolModel(m_rgb_diff_2);
  enc->initSymbolModel(m_rgb_diff_3);
  enc->initSymbolModel(m_rgb_diff_4);
  enc->initSymbolModel(m_rgb_diff_5);

  /* init last item */
  memcpy(last_item, item, 6);
  return TRUE;
}

inline BOOL LASwriteItemCompressed_RGB12_v2::write(const U8* item, U32& context)
{
  I32 diff_l = 0;
  I32 diff_h = 0;
  I32 corr;
  U32 sym = ((last_item[0]&0x00FF) != (((U16*)item)[0]&0x00FF)) << 0;
  sym |= ((last_item[0]&0xFF00) != (((U16*)item)[0]&0xFF00)) << 1;
  sym |= ((last_item[1]&0x00FF) != (((U16*)item)[1]&0x00FF)) << 2;
  sym |= ((last_item[1]&0xFF00) != (((U16*)item)[1]&0xFF00)) << 3;
  sym |= ((last_item[2]&0x00FF) != (((U16*)item)[2]&0x00FF)) << 4;
  sym |= ((last_item[2]&0xFF00) != (((U16*)item)[2]&0xFF00)) << 5;
  sym |= (((((U16*)item)[0]&0x00FF) != (((U16*)item)[1]&0x00FF)) || ((((U16*)item)[0]&0x00FF) != (((U16*)item)[2]&0x00FF)) || ((((U16*)item)[0]&0xFF00) != (((U16*)item)[1]&0xFF00)) || ((((U16*)item)[0]&0xFF00) != (((U16*)item)[2]&0xFF00))) << 6;
  enc->encodeSymbol(m_byte_used, sym);
  if (sym & (1 << 0))
  {
    diff_l = ((int)(((U16*)item)[0]&255)) - (last_item[0]&255);
    enc->encodeSymbol(m_rgb_diff_0, U8_FOLD(diff_l));
  }
  if (sym & (1 << 1))
  {
    diff_h = ((int)(((U16*)item)[0]>>8)) - (last_item[0]>>8);
    enc->encodeSymbol(m_rgb_diff_1, U8_FOLD(diff_h));
  }
  if (sym & (1 << 6))
  {
    if (sym & (1 << 2))
    {
      corr = ((int)(((U16*)item)[1]&255)) - U8_CLAMP(diff_l + (last_item[1]&255));
      enc->encodeSymbol(m_rgb_diff_2, U8_FOLD(corr));
    }
    if (sym & (1 << 4))
    {
      diff_l = (diff_l + (((U16*)item)[1]&255) - (last_item[1]&255)) / 2;
      corr = ((int)(((U16*)item)[2]&255)) - U8_CLAMP(diff_l + (last_item[2]&255));
      enc->encodeSymbol(m_rgb_diff_4, U8_FOLD(corr));
    }
    if (sym & (1 << 3))
    {
      corr = ((int)(((U16*)item)[1]>>8)) - U8_CLAMP(diff_h + (last_item[1]>>8));
      enc->encodeSymbol(m_rgb_diff_3, U8_FOLD(corr));
    }
    if (sym & (1 << 5))
    {
      diff_h = (diff_h + (((U16*)item)[1]>>8) - (last_item[1]>>8)) / 2;
      corr = ((int)(((U16*)item)[2]>>8)) - U8_CLAMP(diff_h + (last_item[2]>>8));
      enc->encodeSymbol(m_rgb_diff_5, U8_FOLD(corr));
    }
  }
  memcpy(last_item, item, 6);
  return TRUE;
}

/*
===============================================================================
                       LASwriteItemCompressed_BYTE_v2
===============================================================================
*/

LASwriteItemCompressed_BYTE_v2::LASwriteItemCompressed_BYTE_v2(ArithmeticEncoder* enc, U32 number)
{
  U32 i;

  /* set encoder */
  assert(enc);
  this->enc = enc;
  assert(number);
  this->number = number;

  /* create models and integer compressors */
  m_byte = new ArithmeticModel*[number];
  for (i = 0; i < number; i++)
  {
    m_byte[i] = enc->createSymbolModel(256);
  }

  /* create last item */
  last_item = new U8[number];
}

LASwriteItemCompressed_BYTE_v2::~LASwriteItemCompressed_BYTE_v2()
{
  U32 i;
  for (i = 0; i < number; i++)
  {
    enc->destroySymbolModel(m_byte[i]);
  }
  delete [] m_byte;
  delete [] last_item;
}

BOOL LASwriteItemCompressed_BYTE_v2::init(const U8* item, U32& context)
{
  U32 i;
  /* init state */

  /* init models and integer compressors */
  for (i = 0; i < number; i++)
  {
    enc->initSymbolModel(m_byte[i]);
  }

  /* init last point */
  memcpy(last_item, item, number);
  return TRUE;
}

inline BOOL LASwriteItemCompressed_BYTE_v2::write(const U8* item, U32& context)
{
  U32 i;
  I32 diff;
  for (i = 0; i < number; i++)
  {
    diff = item[i] - last_item[i];
    enc->encodeSymbol(m_byte[i], U8_FOLD(diff));
  }
  memcpy(last_item, item, number);
  return TRUE;
}

