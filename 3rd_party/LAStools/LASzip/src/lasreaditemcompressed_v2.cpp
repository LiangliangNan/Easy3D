/*
===============================================================================

  FILE:  lasreaditemcompressed_v2.cpp
  
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

#include "lasreaditemcompressed_v2.hpp"

#include <assert.h>
#include <string.h>

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

LASreadItemCompressed_POINT10_v2::LASreadItemCompressed_POINT10_v2(ArithmeticDecoder* dec)
{
  U32 i;

  /* set decoder */
  assert(dec);
  this->dec = dec;

  /* create models and integer compressors */
  m_changed_values = dec->createSymbolModel(64);
  ic_intensity = new IntegerCompressor(dec, 16, 4);
  m_scan_angle_rank[0] = dec->createSymbolModel(256);
  m_scan_angle_rank[1] = dec->createSymbolModel(256);
  ic_point_source_ID = new IntegerCompressor(dec, 16);
  for (i = 0; i < 256; i++)
  {
    m_bit_byte[i] = 0;
    m_classification[i] = 0;
    m_user_data[i] = 0;
  }
  ic_dx = new IntegerCompressor(dec, 32, 2);  // 32 bits, 2 context
  ic_dy = new IntegerCompressor(dec, 32, 22); // 32 bits, 22 contexts
  ic_z = new IntegerCompressor(dec, 32, 20);  // 32 bits, 20 contexts
}

LASreadItemCompressed_POINT10_v2::~LASreadItemCompressed_POINT10_v2()
{
  U32 i;

  dec->destroySymbolModel(m_changed_values);
  delete ic_intensity;
  dec->destroySymbolModel(m_scan_angle_rank[0]);
  dec->destroySymbolModel(m_scan_angle_rank[1]);
  delete ic_point_source_ID;
  for (i = 0; i < 256; i++)
  {
    if (m_bit_byte[i]) dec->destroySymbolModel(m_bit_byte[i]);
    if (m_classification[i]) dec->destroySymbolModel(m_classification[i]);
    if (m_user_data[i]) dec->destroySymbolModel(m_user_data[i]);
  }
  delete ic_dx;
  delete ic_dy;
  delete ic_z;
}

BOOL LASreadItemCompressed_POINT10_v2::init(const U8* item, U32& context)
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
  dec->initSymbolModel(m_changed_values);
  ic_intensity->initDecompressor();
  dec->initSymbolModel(m_scan_angle_rank[0]);
  dec->initSymbolModel(m_scan_angle_rank[1]);
  ic_point_source_ID->initDecompressor();
  for (i = 0; i < 256; i++)
  {
    if (m_bit_byte[i]) dec->initSymbolModel(m_bit_byte[i]);
    if (m_classification[i]) dec->initSymbolModel(m_classification[i]);
    if (m_user_data[i]) dec->initSymbolModel(m_user_data[i]);
  }
  ic_dx->initDecompressor();
  ic_dy->initDecompressor();
  ic_z->initDecompressor();

  /* init last item */
  memcpy(last_item, item, 20);

  /* but set intensity to zero */ 
  last_item[12] = 0;
  last_item[13] = 0;

  return TRUE;
}

inline void LASreadItemCompressed_POINT10_v2::read(U8* item, U32& context)
{
  U32 r, n, m, l;
  U32 k_bits;
  I32 median, diff;

  // decompress which other values have changed
  I32 changed_values = dec->decodeSymbol(m_changed_values);

  if (changed_values)
  {
    // decompress the edge_of_flight_line, scan_direction_flag, ... if it has changed
    if (changed_values & 32)
    {
      if (m_bit_byte[last_item[14]] == 0)
      {
        m_bit_byte[last_item[14]] = dec->createSymbolModel(256);
        dec->initSymbolModel(m_bit_byte[last_item[14]]);
      }
      last_item[14] = (U8)dec->decodeSymbol(m_bit_byte[last_item[14]]);
    }

    r = ((LASpoint10*)last_item)->return_number;
    n = ((LASpoint10*)last_item)->number_of_returns_of_given_pulse;
    m = number_return_map[n][r];
    l = number_return_level[n][r];

    // decompress the intensity if it has changed
    if (changed_values & 16)
    {
      ((LASpoint10*)last_item)->intensity = (U16)ic_intensity->decompress(last_intensity[m], (m < 3 ? m : 3));
      last_intensity[m] = ((LASpoint10*)last_item)->intensity;
    }
    else
    {
      ((LASpoint10*)last_item)->intensity = last_intensity[m];
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
      I32 val = dec->decodeSymbol(m_scan_angle_rank[((LASpoint10*)last_item)->scan_direction_flag]);
      last_item[16] = U8_FOLD(val + last_item[16]);
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
  else
  {
    r = ((LASpoint10*)last_item)->return_number;
    n = ((LASpoint10*)last_item)->number_of_returns_of_given_pulse;
    m = number_return_map[n][r];
    l = number_return_level[n][r];
  }

  // decompress x coordinate
  median = last_x_diff_median5[m].get();
  diff = ic_dx->decompress(median, n==1);
  ((LASpoint10*)last_item)->x += diff;
  last_x_diff_median5[m].add(diff);

  // decompress y coordinate
  median = last_y_diff_median5[m].get();
  k_bits = ic_dx->getK();
  diff = ic_dy->decompress(median, (n==1) + ( k_bits < 20 ? U32_ZERO_BIT_0(k_bits) : 20 ));
  ((LASpoint10*)last_item)->y += diff;
  last_y_diff_median5[m].add(diff);

  // decompress z coordinate
  k_bits = (ic_dx->getK() + ic_dy->getK()) / 2;
  ((LASpoint10*)last_item)->z = ic_z->decompress(last_height[l], (n==1) + (k_bits < 18 ? U32_ZERO_BIT_0(k_bits) : 18));
  last_height[l] = ((LASpoint10*)last_item)->z;

  // copy the last point
  memcpy(item, last_item, 20);
}

/*
===============================================================================
                       LASreadItemCompressed_GPSTIME11_v2
===============================================================================
*/

#define LASZIP_GPSTIME_MULTI 500
#define LASZIP_GPSTIME_MULTI_MINUS -10
#define LASZIP_GPSTIME_MULTI_UNCHANGED (LASZIP_GPSTIME_MULTI - LASZIP_GPSTIME_MULTI_MINUS + 1)
#define LASZIP_GPSTIME_MULTI_CODE_FULL (LASZIP_GPSTIME_MULTI - LASZIP_GPSTIME_MULTI_MINUS + 2)

#define LASZIP_GPSTIME_MULTI_TOTAL (LASZIP_GPSTIME_MULTI - LASZIP_GPSTIME_MULTI_MINUS + 6) 

LASreadItemCompressed_GPSTIME11_v2::LASreadItemCompressed_GPSTIME11_v2(ArithmeticDecoder* dec)
{
  /* set decoder */
  assert(dec);
  this->dec = dec;
  /* create entropy models and integer compressors */
  m_gpstime_multi = dec->createSymbolModel(LASZIP_GPSTIME_MULTI_TOTAL);
  m_gpstime_0diff = dec->createSymbolModel(6);
  ic_gpstime = new IntegerCompressor(dec, 32, 9); // 32 bits, 9 contexts
}

LASreadItemCompressed_GPSTIME11_v2::~LASreadItemCompressed_GPSTIME11_v2()
{
  dec->destroySymbolModel(m_gpstime_multi);
  dec->destroySymbolModel(m_gpstime_0diff);
  delete ic_gpstime;
}

BOOL LASreadItemCompressed_GPSTIME11_v2::init(const U8* item, U32& context)
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
  dec->initSymbolModel(m_gpstime_multi);
  dec->initSymbolModel(m_gpstime_0diff);
  ic_gpstime->initDecompressor();

  /* init last item */
  last_gpstime[0].u64 = *((U64*)item);
  last_gpstime[1].u64 = 0;
  last_gpstime[2].u64 = 0;
  last_gpstime[3].u64 = 0;
  return TRUE;
}

inline void LASreadItemCompressed_GPSTIME11_v2::read(U8* item, U32& context)
{
  I32 multi;
  if (last_gpstime_diff[last] == 0) // if the last integer difference was zero
  {
    multi = dec->decodeSymbol(m_gpstime_0diff);
    if (multi == 1) // the difference can be represented with 32 bits
    {
      last_gpstime_diff[last] = ic_gpstime->decompress(0, 0);
      last_gpstime[last].i64 += last_gpstime_diff[last];
      multi_extreme_counter[last] = 0; 
    }
    else if (multi == 2) // the difference is huge
    {
      next = (next+1)&3;
      last_gpstime[next].u64 = ic_gpstime->decompress((I32)(last_gpstime[last].u64 >> 32), 8);
      last_gpstime[next].u64 = last_gpstime[next].u64 << 32;
      last_gpstime[next].u64 |= dec->readInt();
      last = next;
      last_gpstime_diff[last] = 0;
      multi_extreme_counter[last] = 0; 
    }
    else if (multi > 2) // we switch to another sequence
    {
      last = (last+multi-2)&3;
      read(item, context);
    }
  }
  else
  {
    multi = dec->decodeSymbol(m_gpstime_multi);
    if (multi == 1)
    {
      last_gpstime[last].i64 += ic_gpstime->decompress(last_gpstime_diff[last], 1);;
      multi_extreme_counter[last] = 0;
    }
    else if (multi < LASZIP_GPSTIME_MULTI_UNCHANGED)
    {
      I32 gpstime_diff;
      if (multi == 0)
      {
        gpstime_diff = ic_gpstime->decompress(0, 7);
        multi_extreme_counter[last]++;
        if (multi_extreme_counter[last] > 3)
        {
          last_gpstime_diff[last] = gpstime_diff;
          multi_extreme_counter[last] = 0;
        }
      }
      else if (multi < LASZIP_GPSTIME_MULTI)
      {
        if (multi < 10)
          gpstime_diff = ic_gpstime->decompress(multi*last_gpstime_diff[last], 2);
        else
          gpstime_diff = ic_gpstime->decompress(multi*last_gpstime_diff[last], 3);
      }
      else if (multi == LASZIP_GPSTIME_MULTI)
      {
        gpstime_diff = ic_gpstime->decompress(LASZIP_GPSTIME_MULTI*last_gpstime_diff[last], 4);
        multi_extreme_counter[last]++;
        if (multi_extreme_counter[last] > 3)
        {
          last_gpstime_diff[last] = gpstime_diff;
          multi_extreme_counter[last] = 0;
        }
      }
      else
      {
        multi = LASZIP_GPSTIME_MULTI - multi;
        if (multi > LASZIP_GPSTIME_MULTI_MINUS)
        {
          gpstime_diff = ic_gpstime->decompress(multi*last_gpstime_diff[last], 5);
        }
        else
        {
          gpstime_diff = ic_gpstime->decompress(LASZIP_GPSTIME_MULTI_MINUS*last_gpstime_diff[last], 6);
          multi_extreme_counter[last]++;
          if (multi_extreme_counter[last] > 3)
          {
            last_gpstime_diff[last] = gpstime_diff;
            multi_extreme_counter[last] = 0;
          }
        }
      }
      last_gpstime[last].i64 += gpstime_diff;
    }
    else if (multi ==  LASZIP_GPSTIME_MULTI_CODE_FULL)
    {
      next = (next+1)&3;
      last_gpstime[next].u64 = ic_gpstime->decompress((I32)(last_gpstime[last].u64 >> 32), 8);
      last_gpstime[next].u64 = last_gpstime[next].u64 << 32;
      last_gpstime[next].u64 |= dec->readInt();
      last = next;
      last_gpstime_diff[last] = 0;
      multi_extreme_counter[last] = 0; 
    }
    else if (multi >=  LASZIP_GPSTIME_MULTI_CODE_FULL)
    {
      last = (last+multi-LASZIP_GPSTIME_MULTI_CODE_FULL)&3;
      read(item, context);
    }
  }
  *((I64*)item) = last_gpstime[last].i64;
}

/*
===============================================================================
                       LASreadItemCompressed_RGB12_v2
===============================================================================
*/

LASreadItemCompressed_RGB12_v2::LASreadItemCompressed_RGB12_v2(ArithmeticDecoder* dec)
{
  /* set decoder */
  assert(dec);
  this->dec = dec;

  /* create models and integer compressors */
  m_byte_used = dec->createSymbolModel(128);
  m_rgb_diff_0 = dec->createSymbolModel(256);
  m_rgb_diff_1 = dec->createSymbolModel(256);
  m_rgb_diff_2 = dec->createSymbolModel(256);
  m_rgb_diff_3 = dec->createSymbolModel(256);
  m_rgb_diff_4 = dec->createSymbolModel(256);
  m_rgb_diff_5 = dec->createSymbolModel(256);
}

LASreadItemCompressed_RGB12_v2::~LASreadItemCompressed_RGB12_v2()
{
  dec->destroySymbolModel(m_byte_used);
  dec->destroySymbolModel(m_rgb_diff_0);
  dec->destroySymbolModel(m_rgb_diff_1);
  dec->destroySymbolModel(m_rgb_diff_2);
  dec->destroySymbolModel(m_rgb_diff_3);
  dec->destroySymbolModel(m_rgb_diff_4);
  dec->destroySymbolModel(m_rgb_diff_5);
}

BOOL LASreadItemCompressed_RGB12_v2::init(const U8* item, U32& context)
{
  /* init state */

  /* init models and integer compressors */
  dec->initSymbolModel(m_byte_used);
  dec->initSymbolModel(m_rgb_diff_0);
  dec->initSymbolModel(m_rgb_diff_1);
  dec->initSymbolModel(m_rgb_diff_2);
  dec->initSymbolModel(m_rgb_diff_3);
  dec->initSymbolModel(m_rgb_diff_4);
  dec->initSymbolModel(m_rgb_diff_5);

  /* init last item */
  memcpy(last_item, item, 6);
  return TRUE;
}

inline void LASreadItemCompressed_RGB12_v2::read(U8* item, U32& context)
{
  U8 corr;
  I32 diff = 0;
  U32 sym = dec->decodeSymbol(m_byte_used);
  if (sym & (1 << 0))
  {
    corr = dec->decodeSymbol(m_rgb_diff_0);
    ((U16*)item)[0] = (U16)U8_FOLD(corr + (last_item[0]&255));
  }
  else 
  {
    ((U16*)item)[0] = last_item[0]&0xFF;
  }
  if (sym & (1 << 1))
  {
    corr = dec->decodeSymbol(m_rgb_diff_1);
    ((U16*)item)[0] |= (((U16)U8_FOLD(corr + (last_item[0]>>8))) << 8);
  }
  else
  {
    ((U16*)item)[0] |= (last_item[0]&0xFF00);
  }
  if (sym & (1 << 6))
  {
    diff = (((U16*)item)[0]&0x00FF) - (last_item[0]&0x00FF);
    if (sym & (1 << 2))
    {
      corr = dec->decodeSymbol(m_rgb_diff_2);
      ((U16*)item)[1] = (U16)U8_FOLD(corr + U8_CLAMP(diff+(last_item[1]&255)));
    }
    else
    {
      ((U16*)item)[1] = last_item[1]&0xFF;
    }
    if (sym & (1 << 4))
    {
      corr = dec->decodeSymbol(m_rgb_diff_4);
      diff = (diff + ((((U16*)item)[1]&0x00FF) - (last_item[1]&0x00FF))) / 2;
      ((U16*)item)[2] = (U16)U8_FOLD(corr + U8_CLAMP(diff+(last_item[2]&255)));
    }
    else
    {
      ((U16*)item)[2] = last_item[2]&0xFF;
    }
    diff = (((U16*)item)[0]>>8) - (last_item[0]>>8);
    if (sym & (1 << 3))
    {
      corr = dec->decodeSymbol(m_rgb_diff_3);
      ((U16*)item)[1] |= (((U16)U8_FOLD(corr + U8_CLAMP(diff+(last_item[1]>>8))))<<8);
    }
    else
    {
      ((U16*)item)[1] |= (last_item[1]&0xFF00);
    }
    if (sym & (1 << 5))
    {
      corr = dec->decodeSymbol(m_rgb_diff_5);
      diff = (diff + ((((U16*)item)[1]>>8) - (last_item[1]>>8))) / 2;
      ((U16*)item)[2] |= (((U16)U8_FOLD(corr + U8_CLAMP(diff+(last_item[2]>>8))))<<8);
    }
    else
    {
      ((U16*)item)[2] |= (last_item[2]&0xFF00);
    }
  }
  else
  {
    ((U16*)item)[1] = ((U16*)item)[0];
    ((U16*)item)[2] = ((U16*)item)[0];
  }
  memcpy(last_item, item, 6);
}

/*
===============================================================================
                       LASreadItemCompressed_BYTE_v2
===============================================================================
*/

LASreadItemCompressed_BYTE_v2::LASreadItemCompressed_BYTE_v2(ArithmeticDecoder* dec, U32 number)
{
  U32 i;

  /* set decoder */
  assert(dec);
  this->dec = dec;
  assert(number);
  this->number = number;

  /* create models and integer compressors */
  m_byte = new ArithmeticModel*[number];
  for (i = 0; i < number; i++)
  {
    m_byte[i] = dec->createSymbolModel(256);
  }

  /* create last item */
  last_item = new U8[number];
}

LASreadItemCompressed_BYTE_v2::~LASreadItemCompressed_BYTE_v2()
{
  U32 i;
  for (i = 0; i < number; i++)
  {
    dec->destroySymbolModel(m_byte[i]);
  }
  delete [] m_byte;
  delete [] last_item;
}

BOOL LASreadItemCompressed_BYTE_v2::init(const U8* item, U32& context)
{
  U32 i;
  /* init state */

  /* init models and integer compressors */
  for (i = 0; i < number; i++)
  {
    dec->initSymbolModel(m_byte[i]);
  }

  /* init last item */
  memcpy(last_item, item, number);
  return TRUE;
}

inline void LASreadItemCompressed_BYTE_v2::read(U8* item, U32& context)
{
  U32 i;
  I32 value;
  for (i = 0; i < number; i++)
  {
    value = last_item[i] + dec->decodeSymbol(m_byte[i]);
    item[i] = U8_FOLD(value);
  }
  memcpy(last_item, item, number);
}
