/*
===============================================================================

  FILE:  laswriteitemcompressed_v4.cpp
  
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

#include "laswriteitemcompressed_v4.hpp"

#include <assert.h>
#include <string.h>
#include <stdio.h>

/*
===============================================================================
                       LASwriteItemCompressed_POINT14_v4
===============================================================================
*/

typedef struct LASpoint14
{
  I32 X;
  I32 Y;
  I32 Z;
  U16 intensity;
  U8 legacy_return_number : 3;
  U8 legacy_number_of_returns : 3;
  U8 scan_direction_flag : 1;
  U8 edge_of_flight_line : 1;
  U8 legacy_classification : 5;
  U8 legacy_synthetic_flag : 1;
  U8 legacy_keypoint_flag  : 1;
  U8 legacy_withheld_flag  : 1;
  I8 legacy_scan_angle_rank;
  U8 user_data;
  U16 point_source_ID;

  // LAS 1.4 only
  I16 scan_angle;
  U8 legacy_point_type : 2;
  U8 scanner_channel : 2;
  U8 classification_flags : 4;
  U8 classification;
  U8 return_number : 4;
  U8 number_of_returns : 4;

  // LASlib internal use only
  U8 deleted_flag;

  // for 8 byte alignment of the GPS time
  U8 dummy[2];

  // compressed LASzip 1.4 points only
  BOOL gps_time_change;

  F64 gps_time;
  U16 rgb[4];
//  LASwavepacket wavepacket;
} LASpoint14;

#define LASZIP_GPSTIME_MULTI 500
#define LASZIP_GPSTIME_MULTI_MINUS -10
#define LASZIP_GPSTIME_MULTI_CODE_FULL (LASZIP_GPSTIME_MULTI - LASZIP_GPSTIME_MULTI_MINUS + 1)

#define LASZIP_GPSTIME_MULTI_TOTAL (LASZIP_GPSTIME_MULTI - LASZIP_GPSTIME_MULTI_MINUS + 5) 

LASwriteItemCompressed_POINT14_v4::LASwriteItemCompressed_POINT14_v4(ArithmeticEncoder* enc)
{
  /* not used as a encoder. just gives access to outstream */

  assert(enc);
  this->enc = enc;

  /* zero outstreams and encoders */

  outstream_channel_returns_XY = 0;
  outstream_Z = 0;
  outstream_classification = 0;
  outstream_flags = 0;
  outstream_intensity = 0;
  outstream_scan_angle = 0;
  outstream_user_data = 0;
  outstream_point_source = 0;
  outstream_gps_time = 0;

  enc_channel_returns_XY = 0;
  enc_Z = 0;
  enc_classification = 0;
  enc_flags = 0;
  enc_intensity = 0;
  enc_scan_angle = 0;
  enc_user_data = 0;
  enc_point_source = 0;
  enc_gps_time = 0;

  /* mark the four scanner channel contexts as uninitialized */

  U32 c;
  for (c = 0; c < 4; c++)
  {
    contexts[c].m_changed_values[0] = 0;
  }
  current_context = 0;

  /* number of bytes per layer */
  
  num_bytes_channel_returns_XY = 0;
  num_bytes_Z = 0;
  num_bytes_classification = 0;
  num_bytes_flags = 0;
  num_bytes_intensity = 0;
  num_bytes_scan_angle = 0;
  num_bytes_user_data = 0;
  num_bytes_point_source = 0;
  num_bytes_gps_time = 0;
}

LASwriteItemCompressed_POINT14_v4::~LASwriteItemCompressed_POINT14_v4()
{
  U32 c, i;

  /* destroy all initialized scanner channel contexts */

  for (c = 0; c < 4; c++)
  {
    if (contexts[c].m_changed_values[0])
    {
      enc_channel_returns_XY->destroySymbolModel(contexts[c].m_changed_values[0]);
      enc_channel_returns_XY->destroySymbolModel(contexts[c].m_changed_values[1]);
      enc_channel_returns_XY->destroySymbolModel(contexts[c].m_changed_values[2]);
      enc_channel_returns_XY->destroySymbolModel(contexts[c].m_changed_values[3]);
      enc_channel_returns_XY->destroySymbolModel(contexts[c].m_changed_values[4]);
      enc_channel_returns_XY->destroySymbolModel(contexts[c].m_changed_values[5]);
      enc_channel_returns_XY->destroySymbolModel(contexts[c].m_changed_values[6]);
      enc_channel_returns_XY->destroySymbolModel(contexts[c].m_changed_values[7]);
      enc_channel_returns_XY->destroySymbolModel(contexts[c].m_scanner_channel);
      for (i = 0; i < 16; i++)
      {
        if (contexts[c].m_number_of_returns[i]) enc_channel_returns_XY->destroySymbolModel(contexts[c].m_number_of_returns[i]);
        if (contexts[c].m_return_number[i]) enc_channel_returns_XY->destroySymbolModel(contexts[c].m_return_number[i]);
      }
      enc_channel_returns_XY->destroySymbolModel(contexts[c].m_return_number_gps_same);
      delete contexts[c].ic_dX;
      delete contexts[c].ic_dY;
      delete contexts[c].ic_Z;
      for (i = 0; i < 64; i++)
      {
        if (contexts[c].m_classification[i]) enc_classification->destroySymbolModel(contexts[c].m_classification[i]);
        if (contexts[c].m_flags[i]) enc_flags->destroySymbolModel(contexts[c].m_flags[i]);
        if (contexts[c].m_user_data[i]) enc_user_data->destroySymbolModel(contexts[c].m_user_data[i]);
      }
      delete contexts[c].ic_intensity;
      delete contexts[c].ic_scan_angle;
      delete contexts[c].ic_point_source_ID;
      enc_gps_time->destroySymbolModel(contexts[c].m_gpstime_multi);
      enc_gps_time->destroySymbolModel(contexts[c].m_gpstime_0diff);
      delete contexts[c].ic_gpstime;
    }
  }

  /* destroy all encoders and outstreams  */

  if (outstream_channel_returns_XY)
  {
    delete enc_channel_returns_XY;
    delete enc_Z;
    delete enc_classification;
    delete enc_flags;
    delete enc_intensity;
    delete enc_scan_angle;
    delete enc_user_data;
    delete enc_point_source;
    delete enc_gps_time;

    delete outstream_channel_returns_XY;
    delete outstream_Z;
    delete outstream_classification;
    delete outstream_flags;
    delete outstream_intensity;
    delete outstream_scan_angle;
    delete outstream_user_data;
    delete outstream_point_source;
    delete outstream_gps_time;
  }
}

inline BOOL LASwriteItemCompressed_POINT14_v4::createAndInitModelsAndCompressors(U32 context, const U8* item)
{
  I32 i;

  /* should only be called when context is unused */

  assert(contexts[context].unused);

  /* first create all entropy models and integer compressors (if needed) */

  if (contexts[context].m_changed_values[0] == 0)
  {
    /* for the channel_returns_XY layer */

    contexts[context].m_changed_values[0] = enc_channel_returns_XY->createSymbolModel(128);
    contexts[context].m_changed_values[1] = enc_channel_returns_XY->createSymbolModel(128);
    contexts[context].m_changed_values[2] = enc_channel_returns_XY->createSymbolModel(128);
    contexts[context].m_changed_values[3] = enc_channel_returns_XY->createSymbolModel(128);
    contexts[context].m_changed_values[4] = enc_channel_returns_XY->createSymbolModel(128);
    contexts[context].m_changed_values[5] = enc_channel_returns_XY->createSymbolModel(128);
    contexts[context].m_changed_values[6] = enc_channel_returns_XY->createSymbolModel(128);
    contexts[context].m_changed_values[7] = enc_channel_returns_XY->createSymbolModel(128);
    contexts[context].m_scanner_channel = enc_channel_returns_XY->createSymbolModel(3);
    for (i = 0; i < 16; i++)
    {
      contexts[context].m_number_of_returns[i] = 0;
      contexts[context].m_return_number[i] = 0;
    }
    contexts[context].m_return_number_gps_same = enc_channel_returns_XY->createSymbolModel(13);

    contexts[context].ic_dX = new IntegerCompressor(enc_channel_returns_XY, 32, 2);  // 32 bits, 2 context
    contexts[context].ic_dY = new IntegerCompressor(enc_channel_returns_XY, 32, 22); // 32 bits, 22 contexts

    /* for the Z layer */

    contexts[context].ic_Z = new IntegerCompressor(enc_Z, 32, 20);  // 32 bits, 20 contexts

    /* for the classification layer */
    /* for the flags layer */
    /* for the user_data layer */

    for (i = 0; i < 64; i++)
    {
      contexts[context].m_classification[i] = 0;
      contexts[context].m_flags[i] = 0;
      contexts[context].m_user_data[i] = 0;
    }

    /* for the intensity layer */

    contexts[context].ic_intensity = new IntegerCompressor(enc_intensity, 16, 4);

    /* for the scan_angle layer */

    contexts[context].ic_scan_angle = new IntegerCompressor(enc_scan_angle, 16, 2);

    /* for the point_source_ID layer */

    contexts[context].ic_point_source_ID = new IntegerCompressor(enc_point_source, 16);

    /* for the gps_time layer */

    contexts[context].m_gpstime_multi = enc_gps_time->createSymbolModel(LASZIP_GPSTIME_MULTI_TOTAL);
    contexts[context].m_gpstime_0diff = enc_gps_time->createSymbolModel(5);
    contexts[context].ic_gpstime = new IntegerCompressor(enc_gps_time, 32, 9); // 32 bits, 9 contexts
  }

  /* then init entropy models and integer compressors */

  /* for the channel_returns_XY layer */

  enc_channel_returns_XY->initSymbolModel(contexts[context].m_changed_values[0]);
  enc_channel_returns_XY->initSymbolModel(contexts[context].m_changed_values[1]);
  enc_channel_returns_XY->initSymbolModel(contexts[context].m_changed_values[2]);
  enc_channel_returns_XY->initSymbolModel(contexts[context].m_changed_values[3]);
  enc_channel_returns_XY->initSymbolModel(contexts[context].m_changed_values[4]);
  enc_channel_returns_XY->initSymbolModel(contexts[context].m_changed_values[5]);
  enc_channel_returns_XY->initSymbolModel(contexts[context].m_changed_values[6]);
  enc_channel_returns_XY->initSymbolModel(contexts[context].m_changed_values[7]);
  enc_channel_returns_XY->initSymbolModel(contexts[context].m_scanner_channel);
  for (i = 0; i < 16; i++)
  {
    if (contexts[context].m_number_of_returns[i]) enc_channel_returns_XY->initSymbolModel(contexts[context].m_number_of_returns[i]);
    if (contexts[context].m_return_number[i]) enc_channel_returns_XY->initSymbolModel(contexts[context].m_return_number[i]);
  }
  enc_channel_returns_XY->initSymbolModel(contexts[context].m_return_number_gps_same);
  contexts[context].ic_dX->initCompressor();
  contexts[context].ic_dY->initCompressor();
  for (i = 0; i < 12; i++)
  {
    contexts[context].last_X_diff_median5[i].init();
    contexts[context].last_Y_diff_median5[i].init();
  }
  
  /* for the Z layer */

  contexts[context].ic_Z->initCompressor();
  for (i = 0; i < 8; i++)
  {
    contexts[context].last_Z[i] = ((LASpoint14*)item)->Z;
  }

  /* for the classification layer */
  /* for the flags layer */
  /* for the user_data layer */

  for (i = 0; i < 64; i++)
  {
    if (contexts[context].m_classification[i]) enc_classification->initSymbolModel(contexts[context].m_classification[i]);
    if (contexts[context].m_flags[i]) enc_flags->initSymbolModel(contexts[context].m_flags[i]);
    if (contexts[context].m_user_data[i]) enc_user_data->initSymbolModel(contexts[context].m_user_data[i]);
  }

  /* for the intensity layer */

  contexts[context].ic_intensity->initCompressor();
  for (i = 0; i < 8; i++)
  {
    contexts[context].last_intensity[i] = ((LASpoint14*)item)->intensity;
  }

  /* for the scan_angle layer */

  contexts[context].ic_scan_angle->initCompressor();

  /* for the point_source_ID layer */

  contexts[context].ic_point_source_ID->initCompressor();

  /* for the gps_time layer */

  enc_gps_time->initSymbolModel(contexts[context].m_gpstime_multi);
  enc_gps_time->initSymbolModel(contexts[context].m_gpstime_0diff);
  contexts[context].ic_gpstime->initCompressor();
  contexts[context].last = 0, contexts[context].next = 0;
  contexts[context].last_gpstime_diff[0] = 0;
  contexts[context].last_gpstime_diff[1] = 0;
  contexts[context].last_gpstime_diff[2] = 0;
  contexts[context].last_gpstime_diff[3] = 0;
  contexts[context].multi_extreme_counter[0] = 0;
  contexts[context].multi_extreme_counter[1] = 0;
  contexts[context].multi_extreme_counter[2] = 0;
  contexts[context].multi_extreme_counter[3] = 0;
  contexts[context].last_gpstime[0].f64 = ((LASpoint14*)item)->gps_time;
  contexts[context].last_gpstime[1].u64 = 0;
  contexts[context].last_gpstime[2].u64 = 0;
  contexts[context].last_gpstime[3].u64 = 0;

  /* init current context from item */

  memcpy(contexts[context].last_item, item, sizeof(LASpoint14));
  ((LASpoint14*)contexts[context].last_item)->gps_time_change = FALSE;

  contexts[context].unused = FALSE;

  return TRUE;
}

BOOL LASwriteItemCompressed_POINT14_v4::init(const U8* item, U32& context)
{
  /* on the first init create outstreams and encoders */

  if (outstream_channel_returns_XY == 0)
  {
    if (IS_LITTLE_ENDIAN())
    {
      outstream_channel_returns_XY = new ByteStreamOutArrayLE();
      outstream_Z = new ByteStreamOutArrayLE();
      outstream_classification = new ByteStreamOutArrayLE();
      outstream_flags = new ByteStreamOutArrayLE();
      outstream_intensity = new ByteStreamOutArrayLE();
      outstream_scan_angle = new ByteStreamOutArrayLE();
      outstream_user_data = new ByteStreamOutArrayLE();
      outstream_point_source = new ByteStreamOutArrayLE();
      outstream_gps_time = new ByteStreamOutArrayLE();
    }
    else
    {
      outstream_channel_returns_XY = new ByteStreamOutArrayBE();
      outstream_Z = new ByteStreamOutArrayBE();
      outstream_classification = new ByteStreamOutArrayBE();
      outstream_flags = new ByteStreamOutArrayBE();
      outstream_intensity = new ByteStreamOutArrayBE();
      outstream_scan_angle = new ByteStreamOutArrayBE();
      outstream_user_data = new ByteStreamOutArrayBE();
      outstream_point_source = new ByteStreamOutArrayBE();
      outstream_gps_time = new ByteStreamOutArrayBE();
    }

    /* create layer encoders */

    enc_channel_returns_XY = new ArithmeticEncoder();
    enc_Z = new ArithmeticEncoder();
    enc_classification = new ArithmeticEncoder();
    enc_flags = new ArithmeticEncoder();
    enc_intensity = new ArithmeticEncoder();
    enc_scan_angle = new ArithmeticEncoder();
    enc_user_data = new ArithmeticEncoder();
    enc_point_source = new ArithmeticEncoder();
    enc_gps_time = new ArithmeticEncoder();
  }
  else
  {
    /* otherwise just seek back */

    outstream_channel_returns_XY->seek(0);
    outstream_Z->seek(0);
    outstream_classification->seek(0);
    outstream_flags->seek(0);
    outstream_intensity->seek(0);
    outstream_scan_angle->seek(0);
    outstream_user_data->seek(0);
    outstream_point_source->seek(0);
    outstream_gps_time->seek(0);
  }

  /* init layer encoders */

  enc_channel_returns_XY->init(outstream_channel_returns_XY);
  enc_Z->init(outstream_Z);
  enc_classification->init(outstream_classification);
  enc_flags->init(outstream_flags);
  enc_intensity->init(outstream_intensity);
  enc_scan_angle->init(outstream_scan_angle);
  enc_user_data->init(outstream_user_data);
  enc_point_source->init(outstream_point_source);
  enc_gps_time->init(outstream_gps_time);

  /* set changed booleans to FALSE */

  changed_classification = FALSE;
  changed_flags = FALSE;
  changed_intensity = FALSE;
  changed_scan_angle = FALSE;
  changed_user_data = FALSE;
  changed_point_source = FALSE;
  changed_gps_time = FALSE;

  /* mark the four scanner channel contexts as unused */

  U32 c;
  for (c = 0; c < 4; c++)
  {
    contexts[c].unused = TRUE;
  }

  /* set scanner channel as current context */

  current_context = ((LASpoint14*)item)->scanner_channel;
  context = current_context; // the POINT14 writer sets context for all other items

  /* create and init entropy models and integer compressors (and init context from item) */

  createAndInitModelsAndCompressors(current_context, item);

  return TRUE;
}

inline BOOL LASwriteItemCompressed_POINT14_v4::write(const U8* item, U32& context)
{
  // get last

  U8* last_item = contexts[current_context].last_item;

  ////////////////////////////////////////
  // compress returns_XY layer 
  ////////////////////////////////////////

  // create single (3) / first (1) / last (2) / intermediate (0) context from last point return

  I32 lpr = (((LASpoint14*)last_item)->return_number == 1 ? 1 : 0); // first?
  lpr += (((LASpoint14*)last_item)->return_number >= ((LASpoint14*)last_item)->number_of_returns ? 2 : 0); // last?

  // add info whether the GPS time changed in the last return to the context

  lpr += (((LASpoint14*)last_item)->gps_time_change ? 4 : 0);

  // get the (potentially new) context

  U32 scanner_channel = ((LASpoint14*)item)->scanner_channel;

  // if context has changed (and the new context already exists) get last for new context

  if (scanner_channel != current_context)
  {
    if (contexts[scanner_channel].unused == FALSE)
    {
      last_item = contexts[scanner_channel].last_item;
    }
  }

  // determine changed attributes

  BOOL point_source_change = (((LASpoint14*)item)->point_source_ID != ((LASpoint14*)last_item)->point_source_ID);
  BOOL gps_time_change = (((LASpoint14*)item)->gps_time != ((LASpoint14*)last_item)->gps_time);
  BOOL scan_angle_change = (((LASpoint14*)item)->scan_angle != ((LASpoint14*)last_item)->scan_angle);

  // get last and current return counts

  U32 last_n = ((LASpoint14*)last_item)->number_of_returns;
  U32 last_r = ((LASpoint14*)last_item)->return_number;

  U32 n = ((LASpoint14*)item)->number_of_returns;
  U32 r = ((LASpoint14*)item)->return_number;

  // create the 7 bit mask that encodes various changes (its value ranges from 0 to 127)

  I32 changed_values = ((scanner_channel != current_context) << 6) | // scanner channel compared to last point (same = 0 / different = 1)
                       (point_source_change << 5) |                  // point source ID compared to last point from *same* scanner channel (same = 0 / different = 1)
                       (gps_time_change << 4) |                      // GPS time stamp compared to last point from *same* scanner channel (same = 0 / different = 1)
                       (scan_angle_change << 3) |                    // scan angle compared to last point from *same* scanner channel (same = 0 / different = 1)
                       ((n != last_n) << 2);                         // number of returns compared to last point from *same* scanner channel (same = 0 / different = 1)

  // return number compared to last point of *same* scanner channel (same = 0 / plus one mod 16 = 1 / minus one mod 16 = 2 / other difference = 3)

  if (r != last_r)
  {
    if (r == ((last_r + 1) % 16))
    {
      changed_values |= 1;
    }
    else if (r == ((last_r + 15) % 16))
    {
      changed_values |= 2;
    }
    else
    {
      changed_values |= 3;
    }
  }

  // compress the 7 bit mask that encodes changes with last point return context

  enc_channel_returns_XY->encodeSymbol(contexts[current_context].m_changed_values[lpr], changed_values);

  // if scanner channel has changed, record change

  if (changed_values & (1 << 6))
  {
    I32 diff = scanner_channel - current_context;
    if (diff > 0)
    {
      enc_channel_returns_XY->encodeSymbol(contexts[current_context].m_scanner_channel, diff - 1); // curr = last + (sym + 1)
    }
    else
    {
      enc_channel_returns_XY->encodeSymbol(contexts[current_context].m_scanner_channel, diff + 4 - 1); // curr = (last + (sym + 1)) % 4
    }
    // maybe create and init entropy models and integer compressors
    if (contexts[scanner_channel].unused)
    {
      // create and init entropy models and integer compressors (and init context from last item)
      createAndInitModelsAndCompressors(scanner_channel, contexts[current_context].last_item);
      // get last for new context
      last_item = contexts[scanner_channel].last_item;
    }
    // switch context to current scanner channel
    current_context = scanner_channel;
  }
  context = current_context; // the POINT14 writer sets context for all other items

  // if number of returns is different we compress it

  if (changed_values & (1 << 2))
  {
    if (contexts[current_context].m_number_of_returns[last_n] == 0)
    {
      contexts[current_context].m_number_of_returns[last_n] = enc_channel_returns_XY->createSymbolModel(16);
      enc_channel_returns_XY->initSymbolModel(contexts[current_context].m_number_of_returns[last_n]);
    }
    enc_channel_returns_XY->encodeSymbol(contexts[current_context].m_number_of_returns[last_n], n);
  }

  // if return number is different and difference is bigger than +1 / -1 we compress how it is different

  if ((changed_values & 3) == 3)
  {
    if (gps_time_change) // if the GPS time has changed
    {
      if (contexts[current_context].m_return_number[last_r] == 0)
      {
        contexts[current_context].m_return_number[last_r] = enc_channel_returns_XY->createSymbolModel(16);
        enc_channel_returns_XY->initSymbolModel(contexts[current_context].m_return_number[last_r]);
      }
      enc_channel_returns_XY->encodeSymbol(contexts[current_context].m_return_number[last_r], r);
    }
    else // if the GPS time has not changed
    {
      I32 diff = r - last_r;
      if (diff > 1)
      {
        enc_channel_returns_XY->encodeSymbol(contexts[current_context].m_return_number_gps_same, diff - 2); // r = last_r + (sym + 2) with sym = diff - 2
      }
      else
      {
        enc_channel_returns_XY->encodeSymbol(contexts[current_context].m_return_number_gps_same, diff + 16 - 2); // r = (last_r + (sym + 2)) % 16 with sym = diff + 16 - 2
      }
    }
  }

  // get return map m and return level l context for current point

  U32 m = number_return_map_6ctx[n][r];
  U32 l = number_return_level_8ctx[n][r];

  // create single (3) / first (1) / last (2) / intermediate (0) return context for current point

  I32 cpr = (r == 1 ? 2 : 0); // first ?
  cpr += (r >= n ? 1 : 0); // last ?

  U32 k_bits;
  I32 median, diff;

  // compress X coordinate
  median = contexts[current_context].last_X_diff_median5[(m<<1) | gps_time_change].get();
  diff = ((LASpoint14*)item)->X - ((LASpoint14*)last_item)->X;
  contexts[current_context].ic_dX->compress(median, diff, n==1);
  contexts[current_context].last_X_diff_median5[(m<<1) | gps_time_change].add(diff);

  // compress Y coordinate
  k_bits = contexts[current_context].ic_dX->getK();
  median = contexts[current_context].last_Y_diff_median5[(m<<1) | gps_time_change].get();
  diff = ((LASpoint14*)item)->Y - ((LASpoint14*)last_item)->Y;
  contexts[current_context].ic_dY->compress(median, diff, (n==1) + ( k_bits < 20 ? U32_ZERO_BIT_0(k_bits) : 20 ));
  contexts[current_context].last_Y_diff_median5[(m<<1) | gps_time_change].add(diff);

  ////////////////////////////////////////
  // compress Z layer 
  ////////////////////////////////////////

  k_bits = (contexts[current_context].ic_dX->getK() + contexts[current_context].ic_dY->getK()) / 2;
  contexts[current_context].ic_Z->compress(contexts[current_context].last_Z[l], ((LASpoint14*)item)->Z, (n==1) + (k_bits < 18 ? U32_ZERO_BIT_0(k_bits) : 18));
  contexts[current_context].last_Z[l] = ((LASpoint14*)item)->Z;

  ////////////////////////////////////////
  // compress classifications layer 
  ////////////////////////////////////////

  U32 last_classification = ((LASpoint14*)last_item)->classification;
  U32 classification = ((LASpoint14*)item)->classification;

  if (classification != last_classification)
  {
    changed_classification = TRUE;
  }

  I32 ccc = ((last_classification & 0x1F) << 1) + (cpr == 3 ? 1 : 0);
  if (contexts[current_context].m_classification[ccc] == 0)
  {
    contexts[current_context].m_classification[ccc] = enc_classification->createSymbolModel(256);
    enc_classification->initSymbolModel(contexts[current_context].m_classification[ccc]);
  }
  enc_classification->encodeSymbol(contexts[current_context].m_classification[ccc], classification);

  ////////////////////////////////////////
  // compress flags layer 
  ////////////////////////////////////////

  U32 last_flags = (((LASpoint14*)last_item)->edge_of_flight_line << 5) | (((LASpoint14*)last_item)->scan_direction_flag << 4) | ((LASpoint14*)last_item)->classification_flags;
  U32 flags = (((LASpoint14*)item)->edge_of_flight_line << 5) | (((LASpoint14*)item)->scan_direction_flag << 4) | ((LASpoint14*)item)->classification_flags;

  if (flags != last_flags)
  {
    changed_flags = TRUE;
  }

  if (contexts[current_context].m_flags[last_flags] == 0)
  {
    contexts[current_context].m_flags[last_flags] = enc_flags->createSymbolModel(64);
    enc_flags->initSymbolModel(contexts[current_context].m_flags[last_flags]);
  }
  enc_flags->encodeSymbol(contexts[current_context].m_flags[last_flags], flags);

  ////////////////////////////////////////
  // compress intensity layer 
  ////////////////////////////////////////

  if (((LASpoint14*)item)->intensity != ((LASpoint14*)last_item)->intensity)
  {
    changed_intensity = TRUE;
  }
  contexts[current_context].ic_intensity->compress(contexts[current_context].last_intensity[(cpr<<1) | gps_time_change], ((LASpoint14*)item)->intensity, cpr);
  contexts[current_context].last_intensity[(cpr<<1) | gps_time_change] = ((LASpoint14*)item)->intensity;
  
  ////////////////////////////////////////
  // compress scan_angle layer 
  ////////////////////////////////////////

  if (scan_angle_change)
  {
    changed_scan_angle = TRUE;
    contexts[current_context].ic_scan_angle->compress(((LASpoint14*)last_item)->scan_angle, ((LASpoint14*)item)->scan_angle, gps_time_change); // if the GPS time has changed
  }

  ////////////////////////////////////////
  // compress user_data layer 
  ////////////////////////////////////////

  if (((LASpoint14*)item)->user_data != ((LASpoint14*)last_item)->user_data)
  {
    changed_user_data = TRUE;
  }
  if (contexts[current_context].m_user_data[((LASpoint14*)last_item)->user_data/4] == 0)
  {
    contexts[current_context].m_user_data[((LASpoint14*)last_item)->user_data/4] = enc_user_data->createSymbolModel(256);
    enc_user_data->initSymbolModel(contexts[current_context].m_user_data[((LASpoint14*)last_item)->user_data/4]);
  }
  enc_user_data->encodeSymbol(contexts[current_context].m_user_data[((LASpoint14*)last_item)->user_data/4], ((LASpoint14*)item)->user_data);

  ////////////////////////////////////////
  // compress point_source layer 
  ////////////////////////////////////////

  if (point_source_change)
  {
    changed_point_source = TRUE;
    contexts[current_context].ic_point_source_ID->compress(((LASpoint14*)last_item)->point_source_ID, ((LASpoint14*)item)->point_source_ID);
  }

  ////////////////////////////////////////
  // compress gps_time layer 
  ////////////////////////////////////////

  if (gps_time_change) // if the GPS time has changed
  {
    changed_gps_time = TRUE;

    U64I64F64 gps_time;
    gps_time.f64 = ((LASpoint14*)item)->gps_time;

    write_gps_time(gps_time);
  }

  // copy the last item
  memcpy(last_item, item, sizeof(LASpoint14));
  // remember if the last point had a gps_time_change
  ((LASpoint14*)last_item)->gps_time_change = gps_time_change;

  return TRUE;
}

inline BOOL LASwriteItemCompressed_POINT14_v4::chunk_sizes()
{
  U32 num_bytes = 0;
  ByteStreamOut* outstream = enc->getByteStreamOut();

  // finish the encoders

  enc_channel_returns_XY->done();
  enc_Z->done();
  if (changed_classification)
  {
    enc_classification->done();
  }
  if (changed_flags)
  {
    enc_flags->done();
  }
  if (changed_intensity)
  {
    enc_intensity->done();
  }
  if (changed_scan_angle)
  {
    enc_scan_angle->done();
  }
  if (changed_user_data)
  {
    enc_user_data->done();
  }
  if (changed_point_source)
  {
    enc_point_source->done();
  }
  if (changed_gps_time)
  {
    enc_gps_time->done();
  }

  // output the sizes of all layer (i.e.. number of bytes per layer)

  num_bytes = (U32)outstream_channel_returns_XY->getCurr();
  num_bytes_channel_returns_XY += num_bytes;
  outstream->put32bitsLE(((U8*)&num_bytes));

  num_bytes = (U32)outstream_Z->getCurr();
  num_bytes_Z += num_bytes;
  outstream->put32bitsLE(((U8*)&num_bytes));

  if (changed_classification)
  {
    num_bytes = (U32)outstream_classification->getCurr();
    num_bytes_classification += num_bytes;
  }
  else
  {
    num_bytes = 0;
  }
  outstream->put32bitsLE(((U8*)&num_bytes));

  if (changed_flags)
  {
    num_bytes = (U32)outstream_flags->getCurr();
    num_bytes_flags += num_bytes;
  }
  else
  {
    num_bytes = 0;
  }
  outstream->put32bitsLE(((U8*)&num_bytes));

  if (changed_intensity)
  {
    num_bytes = (U32)outstream_intensity->getCurr();
    num_bytes_intensity += num_bytes;
  }
  else
  {
    num_bytes = 0;
  }
  outstream->put32bitsLE(((U8*)&num_bytes));

  if (changed_scan_angle)
  {
    num_bytes = (U32)outstream_scan_angle->getCurr();
    num_bytes_scan_angle += num_bytes;
  }
  else
  {
    num_bytes = 0;
  }
  outstream->put32bitsLE(((U8*)&num_bytes));

  if (changed_user_data)
  {
    num_bytes = (U32)outstream_user_data->getCurr();
    num_bytes_user_data += num_bytes;
  }
  else
  {
    num_bytes = 0;
  }
  outstream->put32bitsLE(((U8*)&num_bytes));

  if (changed_point_source)
  {
    num_bytes = (U32)outstream_point_source->getCurr();
    num_bytes_point_source += num_bytes;
  }
  else
  {
    num_bytes = 0;
  }
  outstream->put32bitsLE(((U8*)&num_bytes));

  if (changed_gps_time)
  {
    num_bytes = (U32)outstream_gps_time->getCurr();
    num_bytes_gps_time += num_bytes;
  }
  else
  {
    num_bytes = 0;
  }
  outstream->put32bitsLE(((U8*)&num_bytes));

  return TRUE;
}

inline BOOL LASwriteItemCompressed_POINT14_v4::chunk_bytes()
{
  U32 num_bytes = 0;
  ByteStreamOut* outstream = enc->getByteStreamOut();

  // output the bytes of all layers

  num_bytes = (U32)outstream_channel_returns_XY->getCurr();
  outstream->putBytes(outstream_channel_returns_XY->getData(), num_bytes);

  num_bytes = (U32)outstream_Z->getCurr();
  outstream->putBytes(outstream_Z->getData(), num_bytes);

  if (changed_classification)
  {
    num_bytes = (U32)outstream_classification->getCurr();
    outstream->putBytes(outstream_classification->getData(), num_bytes);
  }
  else
  {
    num_bytes = 0;
  }

  if (changed_flags)
  {
    num_bytes = (U32)outstream_flags->getCurr();
    outstream->putBytes(outstream_flags->getData(), num_bytes);
  }
  else
  {
    num_bytes = 0;
  }

  if (changed_intensity)
  {
    num_bytes = (U32)outstream_intensity->getCurr();
    outstream->putBytes(outstream_intensity->getData(), num_bytes);
  }
  else
  {
    num_bytes = 0;
  }

  if (changed_scan_angle)
  {
    num_bytes = (U32)outstream_scan_angle->getCurr();
    outstream->putBytes(outstream_scan_angle->getData(), num_bytes);
  }
  else
  {
    num_bytes = 0;
  }

  if (changed_user_data)
  {
    num_bytes = (U32)outstream_user_data->getCurr();
    outstream->putBytes(outstream_user_data->getData(), num_bytes);
  }
  else
  {
    num_bytes = 0;
  }

  if (changed_point_source)
  {
    num_bytes = (U32)outstream_point_source->getCurr();
    outstream->putBytes(outstream_point_source->getData(), num_bytes);
  }
  else
  {
    num_bytes = 0;
  }

  if (changed_gps_time)
  {
    num_bytes = (U32)outstream_gps_time->getCurr();
    outstream->putBytes(outstream_gps_time->getData(), num_bytes);
  }
  else
  {
    num_bytes = 0;
  }

  return TRUE;
}

void LASwriteItemCompressed_POINT14_v4::write_gps_time(const U64I64F64 gps_time)
{
  if (contexts[current_context].last_gpstime_diff[contexts[current_context].last] == 0) // if the last integer difference was zero
  {
    // calculate the difference between the two doubles as an integer
    I64 curr_gpstime_diff_64 = gps_time.i64 - contexts[current_context].last_gpstime[contexts[current_context].last].i64;
    I32 curr_gpstime_diff = (I32)curr_gpstime_diff_64;
    if (curr_gpstime_diff_64 == (I64)(curr_gpstime_diff))
    {
      enc_gps_time->encodeSymbol(contexts[current_context].m_gpstime_0diff, 0); // the difference can be represented with 32 bits
      contexts[current_context].ic_gpstime->compress(0, curr_gpstime_diff, 0);
      contexts[current_context].last_gpstime_diff[contexts[current_context].last] = curr_gpstime_diff;
      contexts[current_context].multi_extreme_counter[contexts[current_context].last] = 0; 
    }
    else // the difference is huge
    {
      U32 i;
      // maybe the double belongs to another time sequence
      for (i = 1; i < 4; i++)
      {
        I64 other_gpstime_diff_64 = gps_time.i64 - contexts[current_context].last_gpstime[(contexts[current_context].last+i)&3].i64;
        I32 other_gpstime_diff = (I32)other_gpstime_diff_64;
        if (other_gpstime_diff_64 == (I64)(other_gpstime_diff))
        {
          enc_gps_time->encodeSymbol(contexts[current_context].m_gpstime_0diff, i+1); // it belongs to another sequence 
          contexts[current_context].last = (contexts[current_context].last+i)&3;
          write_gps_time(gps_time);
          return;
        }
      }
      // no other sequence found. start new sequence.
      enc_gps_time->encodeSymbol(contexts[current_context].m_gpstime_0diff, 1); 
      contexts[current_context].ic_gpstime->compress((I32)(contexts[current_context].last_gpstime[contexts[current_context].last].u64 >> 32), (I32)(gps_time.u64 >> 32), 8);
      enc_gps_time->writeInt((U32)(gps_time.u64));
      contexts[current_context].next = (contexts[current_context].next+1)&3;
      contexts[current_context].last = contexts[current_context].next;
      contexts[current_context].last_gpstime_diff[contexts[current_context].last] = 0;
      contexts[current_context].multi_extreme_counter[contexts[current_context].last] = 0; 
    }
    contexts[current_context].last_gpstime[contexts[current_context].last].i64 = gps_time.i64;
  }
  else // the last integer difference was *not* zero
  {
    // calculate the difference between the two doubles as an integer
    I64 curr_gpstime_diff_64 = gps_time.i64 - contexts[current_context].last_gpstime[contexts[current_context].last].i64;
    I32 curr_gpstime_diff = (I32)curr_gpstime_diff_64;

    // if the current gpstime difference can be represented with 32 bits
    if (curr_gpstime_diff_64 == (I64)(curr_gpstime_diff))
    {
      // compute multiplier between current and last integer difference
      F32 multi_f = (F32)curr_gpstime_diff / (F32)(contexts[current_context].last_gpstime_diff[contexts[current_context].last]);
      I32 multi = I32_QUANTIZE(multi_f);

      // compress the residual curr_gpstime_diff in dependance on the multiplier
      if (multi == 1)
      {
        // this is the case we assume we get most often for regular spaced pulses
        enc_gps_time->encodeSymbol(contexts[current_context].m_gpstime_multi, 1);
        contexts[current_context].ic_gpstime->compress(contexts[current_context].last_gpstime_diff[contexts[current_context].last], curr_gpstime_diff, 1);
        contexts[current_context].multi_extreme_counter[contexts[current_context].last] = 0; 
      }
      else if (multi > 0)
      {
        if (multi < LASZIP_GPSTIME_MULTI) // positive multipliers up to LASZIP_GPSTIME_MULTI are compressed directly
        {
          enc_gps_time->encodeSymbol(contexts[current_context].m_gpstime_multi, multi);
          if (multi < 10)
            contexts[current_context].ic_gpstime->compress(multi*contexts[current_context].last_gpstime_diff[contexts[current_context].last], curr_gpstime_diff, 2);
          else
            contexts[current_context].ic_gpstime->compress(multi*contexts[current_context].last_gpstime_diff[contexts[current_context].last], curr_gpstime_diff, 3);
        }
        else
        {
          enc_gps_time->encodeSymbol(contexts[current_context].m_gpstime_multi, LASZIP_GPSTIME_MULTI);
          contexts[current_context].ic_gpstime->compress(LASZIP_GPSTIME_MULTI*contexts[current_context].last_gpstime_diff[contexts[current_context].last], curr_gpstime_diff, 4);
          contexts[current_context].multi_extreme_counter[contexts[current_context].last]++;
          if (contexts[current_context].multi_extreme_counter[contexts[current_context].last] > 3)
          {
            contexts[current_context].last_gpstime_diff[contexts[current_context].last] = curr_gpstime_diff;
            contexts[current_context].multi_extreme_counter[contexts[current_context].last] = 0;
          }
        }
      }
      else if (multi < 0)
      {
        if (multi > LASZIP_GPSTIME_MULTI_MINUS) // negative multipliers larger than LASZIP_GPSTIME_MULTI_MINUS are compressed directly
        {
          enc_gps_time->encodeSymbol(contexts[current_context].m_gpstime_multi, LASZIP_GPSTIME_MULTI - multi);
          contexts[current_context].ic_gpstime->compress(multi*contexts[current_context].last_gpstime_diff[contexts[current_context].last], curr_gpstime_diff, 5);
        }
        else
        {
          enc_gps_time->encodeSymbol(contexts[current_context].m_gpstime_multi, LASZIP_GPSTIME_MULTI - LASZIP_GPSTIME_MULTI_MINUS);
          contexts[current_context].ic_gpstime->compress(LASZIP_GPSTIME_MULTI_MINUS*contexts[current_context].last_gpstime_diff[contexts[current_context].last], curr_gpstime_diff, 6);
          contexts[current_context].multi_extreme_counter[contexts[current_context].last]++;
          if (contexts[current_context].multi_extreme_counter[contexts[current_context].last] > 3)
          {
            contexts[current_context].last_gpstime_diff[contexts[current_context].last] = curr_gpstime_diff;
            contexts[current_context].multi_extreme_counter[contexts[current_context].last] = 0;
          }
        }
      }
      else
      {
        enc_gps_time->encodeSymbol(contexts[current_context].m_gpstime_multi, 0);
        contexts[current_context].ic_gpstime->compress(0, curr_gpstime_diff, 7);
        contexts[current_context].multi_extreme_counter[contexts[current_context].last]++;
        if (contexts[current_context].multi_extreme_counter[contexts[current_context].last] > 3)
        {
          contexts[current_context].last_gpstime_diff[contexts[current_context].last] = curr_gpstime_diff;
          contexts[current_context].multi_extreme_counter[contexts[current_context].last] = 0;
        }
      }
    }
    else // the difference is huge
    {
      U32 i;
      // maybe the double belongs to another time sequence
      for (i = 1; i < 4; i++)
      {
        I64 other_gpstime_diff_64 = gps_time.i64 - contexts[current_context].last_gpstime[(contexts[current_context].last+i)&3].i64;
        I32 other_gpstime_diff = (I32)other_gpstime_diff_64;
        if (other_gpstime_diff_64 == (I64)(other_gpstime_diff))
        {
          // it belongs to this sequence 
          enc_gps_time->encodeSymbol(contexts[current_context].m_gpstime_multi, LASZIP_GPSTIME_MULTI_CODE_FULL+i);
          contexts[current_context].last = (contexts[current_context].last+i)&3;
          write_gps_time(gps_time);
          return;
        }
      }
      // no other sequence found. start new sequence.
      enc_gps_time->encodeSymbol(contexts[current_context].m_gpstime_multi, LASZIP_GPSTIME_MULTI_CODE_FULL);
      contexts[current_context].ic_gpstime->compress((I32)(contexts[current_context].last_gpstime[contexts[current_context].last].u64 >> 32), (I32)(gps_time.u64 >> 32), 8);
      enc_gps_time->writeInt((U32)(gps_time.u64));
      contexts[current_context].next = (contexts[current_context].next+1)&3;
      contexts[current_context].last = contexts[current_context].next;
      contexts[current_context].last_gpstime_diff[contexts[current_context].last] = 0;
      contexts[current_context].multi_extreme_counter[contexts[current_context].last] = 0; 
    }
    contexts[current_context].last_gpstime[contexts[current_context].last].i64 = gps_time.i64;
  }
}

/*
===============================================================================
                       LASwriteItemCompressed_RGB14_v4
===============================================================================
*/

LASwriteItemCompressed_RGB14_v4::LASwriteItemCompressed_RGB14_v4(ArithmeticEncoder* enc)
{
  /* not used as a encoder. just gives access to outstream */

  assert(enc);
  this->enc = enc;

  /* zero outstreams and encoders */

  outstream_RGB = 0;

  enc_RGB = 0;

  /* zero num_bytes and init booleans */

  num_bytes_RGB = 0;

  changed_RGB = FALSE;

  /* mark the four scanner channel contexts as uninitialized */

  U32 c;
  for (c = 0; c < 4; c++)
  {
    contexts[c].m_byte_used = 0;
  }
  current_context = 0;
}

LASwriteItemCompressed_RGB14_v4::~LASwriteItemCompressed_RGB14_v4()
{
  /* destroy all initialized scanner channel contexts */

  U32 c;
  for (c = 0; c < 4; c++)
  {
    if (contexts[c].m_byte_used)
    {
      enc_RGB->destroySymbolModel(contexts[c].m_byte_used);
      enc_RGB->destroySymbolModel(contexts[c].m_rgb_diff_0);
      enc_RGB->destroySymbolModel(contexts[c].m_rgb_diff_1);
      enc_RGB->destroySymbolModel(contexts[c].m_rgb_diff_2);
      enc_RGB->destroySymbolModel(contexts[c].m_rgb_diff_3);
      enc_RGB->destroySymbolModel(contexts[c].m_rgb_diff_4);
      enc_RGB->destroySymbolModel(contexts[c].m_rgb_diff_5);
    }
  }

  /* destroy all outstreams and encoders */

  if (outstream_RGB)
  {
    delete outstream_RGB;

    delete enc_RGB;
  }
}

inline BOOL LASwriteItemCompressed_RGB14_v4::createAndInitModelsAndCompressors(U32 context, const U8* item)
{
  /* should only be called when context is unused */

  assert(contexts[context].unused);

  /* first create all entropy models (if needed) */

  if (contexts[context].m_byte_used == 0)
  {
    contexts[context].m_byte_used = enc_RGB->createSymbolModel(128);
    contexts[context].m_rgb_diff_0 = enc_RGB->createSymbolModel(256);
    contexts[context].m_rgb_diff_1 = enc_RGB->createSymbolModel(256);
    contexts[context].m_rgb_diff_2 = enc_RGB->createSymbolModel(256);
    contexts[context].m_rgb_diff_3 = enc_RGB->createSymbolModel(256);
    contexts[context].m_rgb_diff_4 = enc_RGB->createSymbolModel(256);
    contexts[context].m_rgb_diff_5 = enc_RGB->createSymbolModel(256);
  }

  /* then init entropy models */

  enc_RGB->initSymbolModel(contexts[context].m_byte_used);
  enc_RGB->initSymbolModel(contexts[context].m_rgb_diff_0);
  enc_RGB->initSymbolModel(contexts[context].m_rgb_diff_1);
  enc_RGB->initSymbolModel(contexts[context].m_rgb_diff_2);
  enc_RGB->initSymbolModel(contexts[context].m_rgb_diff_3);
  enc_RGB->initSymbolModel(contexts[context].m_rgb_diff_4);
  enc_RGB->initSymbolModel(contexts[context].m_rgb_diff_5);

  /* init current context from item */

  memcpy(contexts[context].last_item, item, 6);

  contexts[context].unused = FALSE;

  return TRUE;
}

BOOL LASwriteItemCompressed_RGB14_v4::init(const U8* item, U32& context)
{
  /* on the first init create outstreams and encoders */

  if (outstream_RGB == 0)
  {
    /* create outstreams */

    if (IS_LITTLE_ENDIAN())
    {
      outstream_RGB = new ByteStreamOutArrayLE();
    }
    else
    {
      outstream_RGB = new ByteStreamOutArrayBE();
    }

    /* create layer encoders */

    enc_RGB = new ArithmeticEncoder();
  }
  else
  {
    /* otherwise just seek back */

    outstream_RGB->seek(0);
  }
  
  /* init layer encoders */

  enc_RGB->init(outstream_RGB);

  /* set changed booleans to FALSE */

  changed_RGB = FALSE;

  /* mark the four scanner channel contexts as unused */

  U32 c;
  for (c = 0; c < 4; c++)
  {
    contexts[c].unused = TRUE;
  }

  /* set scanner channel as current context */

  current_context = context; // all other items use context set by POINT14 writer

  /* create and init entropy models and integer compressors (and init contect from item) */

  createAndInitModelsAndCompressors(current_context, item);

  return TRUE;
}

inline BOOL LASwriteItemCompressed_RGB14_v4::write(const U8* item, U32& context)
{
  // get last

  U16* last_item = contexts[current_context].last_item;

  // check for context switch

  if (current_context != context)
  {
    current_context = context; // all other items use context set by POINT14 writer
    if (contexts[current_context].unused)
    {
      createAndInitModelsAndCompressors(current_context, (U8*)last_item);
    }
    last_item = contexts[current_context].last_item;
  }

  // compress

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
  enc_RGB->encodeSymbol(contexts[current_context].m_byte_used, sym);
  if (sym & (1 << 0))
  {
    diff_l = ((int)(((U16*)item)[0]&255)) - (last_item[0]&255);
    enc_RGB->encodeSymbol(contexts[current_context].m_rgb_diff_0, U8_FOLD(diff_l));
  }
  if (sym & (1 << 1))
  {
    diff_h = ((int)(((U16*)item)[0]>>8)) - (last_item[0]>>8);
    enc_RGB->encodeSymbol(contexts[current_context].m_rgb_diff_1, U8_FOLD(diff_h));
  }
  if (sym & (1 << 6))
  {
    if (sym & (1 << 2))
    {
      corr = ((int)(((U16*)item)[1]&255)) - U8_CLAMP(diff_l + (last_item[1]&255));
      enc_RGB->encodeSymbol(contexts[current_context].m_rgb_diff_2, U8_FOLD(corr));
    }
    if (sym & (1 << 4))
    {
      diff_l = (diff_l + (((U16*)item)[1]&255) - (last_item[1]&255)) / 2;
      corr = ((int)(((U16*)item)[2]&255)) - U8_CLAMP(diff_l + (last_item[2]&255));
      enc_RGB->encodeSymbol(contexts[current_context].m_rgb_diff_4, U8_FOLD(corr));
    }
    if (sym & (1 << 3))
    {
      corr = ((int)(((U16*)item)[1]>>8)) - U8_CLAMP(diff_h + (last_item[1]>>8));
      enc_RGB->encodeSymbol(contexts[current_context].m_rgb_diff_3, U8_FOLD(corr));
    }
    if (sym & (1 << 5))
    {
      diff_h = (diff_h + (((U16*)item)[1]>>8) - (last_item[1]>>8)) / 2;
      corr = ((int)(((U16*)item)[2]>>8)) - U8_CLAMP(diff_h + (last_item[2]>>8));
      enc_RGB->encodeSymbol(contexts[current_context].m_rgb_diff_5, U8_FOLD(corr));
    }
  }
  if (sym)
  {
    changed_RGB = TRUE;
  }
  memcpy(last_item, item, 6);

  return TRUE;
}

inline BOOL LASwriteItemCompressed_RGB14_v4::chunk_sizes()
{
  U32 num_bytes = 0;
  ByteStreamOut* outstream = enc->getByteStreamOut();

  // finish the encoders

  enc_RGB->done();

  // output the sizes of all layer (i.e.. number of bytes per layer)

  if (changed_RGB)
  {
    num_bytes = (U32)outstream_RGB->getCurr();
    num_bytes_RGB += num_bytes;
  }
  else
  {
    num_bytes = 0;
  }
  outstream->put32bitsLE(((U8*)&num_bytes));

  return TRUE;
}

inline BOOL LASwriteItemCompressed_RGB14_v4::chunk_bytes()
{
  U32 num_bytes = 0;
  ByteStreamOut* outstream = enc->getByteStreamOut();

  // output the bytes of all layers

  if (changed_RGB)
  {
    num_bytes = (U32)outstream_RGB->getCurr();
    outstream->putBytes(outstream_RGB->getData(), num_bytes);
  }

  return TRUE;
}

/*
===============================================================================
                     LASwriteItemCompressed_RGBNIR14_v4
===============================================================================
*/

LASwriteItemCompressed_RGBNIR14_v4::LASwriteItemCompressed_RGBNIR14_v4(ArithmeticEncoder* enc)
{
  /* not used as a encoder. just gives access to outstream */

  assert(enc);
  this->enc = enc;

  /* zero outstreams and encoders */

  outstream_RGB = 0;
  outstream_NIR = 0;

  enc_RGB = 0;
  enc_NIR = 0;

  /* zero num_bytes and init booleans */

  num_bytes_RGB = 0;
  num_bytes_NIR = 0;

  changed_RGB = FALSE;
  changed_NIR = FALSE;

  /* mark the four scanner channel contexts as uninitialized */

  U32 c;
  for (c = 0; c < 4; c++)
  {
    contexts[c].m_rgb_bytes_used = 0;
  }
  current_context = 0;
}

LASwriteItemCompressed_RGBNIR14_v4::~LASwriteItemCompressed_RGBNIR14_v4()
{
  /* destroy all initialized scanner channel contexts */

  U32 c;
  for (c = 0; c < 4; c++)
  {
    if (contexts[c].m_rgb_bytes_used)
    {
      enc_RGB->destroySymbolModel(contexts[c].m_rgb_bytes_used);
      enc_RGB->destroySymbolModel(contexts[c].m_rgb_diff_0);
      enc_RGB->destroySymbolModel(contexts[c].m_rgb_diff_1);
      enc_RGB->destroySymbolModel(contexts[c].m_rgb_diff_2);
      enc_RGB->destroySymbolModel(contexts[c].m_rgb_diff_3);
      enc_RGB->destroySymbolModel(contexts[c].m_rgb_diff_4);
      enc_RGB->destroySymbolModel(contexts[c].m_rgb_diff_5);

      enc_NIR->destroySymbolModel(contexts[c].m_nir_bytes_used);
      enc_NIR->destroySymbolModel(contexts[c].m_nir_diff_0);
      enc_NIR->destroySymbolModel(contexts[c].m_nir_diff_1);
    }
  }

  /* destroy all outstreams and encoders */

  if (outstream_RGB)
  {
    delete outstream_RGB;
    delete outstream_NIR;

    delete enc_RGB;
    delete enc_NIR;
  }
}

inline BOOL LASwriteItemCompressed_RGBNIR14_v4::createAndInitModelsAndCompressors(U32 context, const U8* item)
{
  /* should only be called when context is unused */

  assert(contexts[context].unused);

  /* first create all entropy models (if needed) */

  if (contexts[context].m_rgb_bytes_used == 0)
  {
    contexts[context].m_rgb_bytes_used = enc_RGB->createSymbolModel(128);
    contexts[context].m_rgb_diff_0 = enc_RGB->createSymbolModel(256);
    contexts[context].m_rgb_diff_1 = enc_RGB->createSymbolModel(256);
    contexts[context].m_rgb_diff_2 = enc_RGB->createSymbolModel(256);
    contexts[context].m_rgb_diff_3 = enc_RGB->createSymbolModel(256);
    contexts[context].m_rgb_diff_4 = enc_RGB->createSymbolModel(256);
    contexts[context].m_rgb_diff_5 = enc_RGB->createSymbolModel(256);

    contexts[context].m_nir_bytes_used = enc_RGB->createSymbolModel(4);
    contexts[context].m_nir_diff_0 = enc_RGB->createSymbolModel(256);
    contexts[context].m_nir_diff_1 = enc_RGB->createSymbolModel(256);
  }

  /* then init entropy models */

  enc_RGB->initSymbolModel(contexts[context].m_rgb_bytes_used);
  enc_RGB->initSymbolModel(contexts[context].m_rgb_diff_0);
  enc_RGB->initSymbolModel(contexts[context].m_rgb_diff_1);
  enc_RGB->initSymbolModel(contexts[context].m_rgb_diff_2);
  enc_RGB->initSymbolModel(contexts[context].m_rgb_diff_3);
  enc_RGB->initSymbolModel(contexts[context].m_rgb_diff_4);
  enc_RGB->initSymbolModel(contexts[context].m_rgb_diff_5);

  enc_NIR->initSymbolModel(contexts[context].m_nir_bytes_used);
  enc_NIR->initSymbolModel(contexts[context].m_nir_diff_0);
  enc_NIR->initSymbolModel(contexts[context].m_nir_diff_1);

  /* init current context from item */

  memcpy(contexts[context].last_item, item, 8);

  contexts[context].unused = FALSE;

  return TRUE;
}

BOOL LASwriteItemCompressed_RGBNIR14_v4::init(const U8* item, U32& context)
{
  /* on the first init create outstreams and encoders */

  if (outstream_RGB == 0)
  {
    /* create outstreams */

    if (IS_LITTLE_ENDIAN())
    {
      outstream_RGB = new ByteStreamOutArrayLE();
      outstream_NIR = new ByteStreamOutArrayLE();
    }
    else
    {
      outstream_RGB = new ByteStreamOutArrayBE();
      outstream_NIR = new ByteStreamOutArrayBE();
    }

    /* create layer encoders */

    enc_RGB = new ArithmeticEncoder();
    enc_NIR = new ArithmeticEncoder();
  }
  else
  {
    /* otherwise just seek back */

    outstream_RGB->seek(0);
    outstream_NIR->seek(0);
  }
  
  /* init layer encoders */

  enc_RGB->init(outstream_RGB);
  enc_NIR->init(outstream_NIR);

  /* set changed booleans to FALSE */

  changed_RGB = FALSE;
  changed_NIR = FALSE;

  /* mark the four scanner channel contexts as unused */

  U32 c;
  for (c = 0; c < 4; c++)
  {
    contexts[c].unused = TRUE;
  }

  /* set scanner channel as current context */

  current_context = context; // ll other items use context set by POINT14 writer

  /* create and init entropy models and integer compressors (and init context from item) */

  createAndInitModelsAndCompressors(current_context, item);

  return TRUE;
}

inline BOOL LASwriteItemCompressed_RGBNIR14_v4::write(const U8* item, U32& context)
{
  // get last

  U16* last_item = contexts[current_context].last_item;

  // check for context switch

  if (current_context != context)
  {
    current_context = context; // all other items use context set by POINT14 writer
    if (contexts[current_context].unused)
    {
      createAndInitModelsAndCompressors(current_context, (U8*)last_item);
    }
    last_item = contexts[current_context].last_item;
  }

  // compress

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
  enc_RGB->encodeSymbol(contexts[current_context].m_rgb_bytes_used, sym);
  if (sym & (1 << 0))
  {
    diff_l = ((int)(((U16*)item)[0]&255)) - (last_item[0]&255);
    enc_RGB->encodeSymbol(contexts[current_context].m_rgb_diff_0, U8_FOLD(diff_l));
  }
  if (sym & (1 << 1))
  {
    diff_h = ((int)(((U16*)item)[0]>>8)) - (last_item[0]>>8);
    enc_RGB->encodeSymbol(contexts[current_context].m_rgb_diff_1, U8_FOLD(diff_h));
  }
  if (sym & (1 << 6))
  {
    if (sym & (1 << 2))
    {
      corr = ((int)(((U16*)item)[1]&255)) - U8_CLAMP(diff_l + (last_item[1]&255));
      enc_RGB->encodeSymbol(contexts[current_context].m_rgb_diff_2, U8_FOLD(corr));
    }
    if (sym & (1 << 4))
    {
      diff_l = (diff_l + (((U16*)item)[1]&255) - (last_item[1]&255)) / 2;
      corr = ((int)(((U16*)item)[2]&255)) - U8_CLAMP(diff_l + (last_item[2]&255));
      enc_RGB->encodeSymbol(contexts[current_context].m_rgb_diff_4, U8_FOLD(corr));
    }
    if (sym & (1 << 3))
    {
      corr = ((int)(((U16*)item)[1]>>8)) - U8_CLAMP(diff_h + (last_item[1]>>8));
      enc_RGB->encodeSymbol(contexts[current_context].m_rgb_diff_3, U8_FOLD(corr));
    }
    if (sym & (1 << 5))
    {
      diff_h = (diff_h + (((U16*)item)[1]>>8) - (last_item[1]>>8)) / 2;
      corr = ((int)(((U16*)item)[2]>>8)) - U8_CLAMP(diff_h + (last_item[2]>>8));
      enc_RGB->encodeSymbol(contexts[current_context].m_rgb_diff_5, U8_FOLD(corr));
    }
  }
  if (sym)
  {
    changed_RGB = TRUE;
  }

  sym = ((last_item[3]&0x00FF) != (((U16*)item)[3]&0x00FF)) << 0;
  sym |= ((last_item[3]&0xFF00) != (((U16*)item)[3]&0xFF00)) << 1;
  enc_NIR->encodeSymbol(contexts[current_context].m_nir_bytes_used, sym);
  if (sym & (1 << 0))
  {
    diff_l = ((int)(((U16*)item)[3]&255)) - (last_item[3]&255);
    enc_NIR->encodeSymbol(contexts[current_context].m_nir_diff_0, U8_FOLD(diff_l));
  }
  if (sym & (1 << 1))
  {
    diff_h = ((int)(((U16*)item)[3]>>8)) - (last_item[3]>>8);
    enc_NIR->encodeSymbol(contexts[current_context].m_nir_diff_1, U8_FOLD(diff_h));
  }
  if (sym)
  {
    changed_NIR = TRUE;
  }
  
  memcpy(last_item, item, 8);
  return TRUE;
}

inline BOOL LASwriteItemCompressed_RGBNIR14_v4::chunk_sizes()
{
  U32 num_bytes = 0;
  ByteStreamOut* outstream = enc->getByteStreamOut();

  // finish the encoders

  enc_RGB->done();
  enc_NIR->done();

  // output the sizes of all layer (i.e.. number of bytes per layer)

  if (changed_RGB)
  {
    num_bytes = (U32)outstream_RGB->getCurr();
    num_bytes_RGB += num_bytes;
  }
  else
  {
    num_bytes = 0;
  }
  outstream->put32bitsLE(((U8*)&num_bytes));

  if (changed_NIR)
  {
    num_bytes = (U32)outstream_NIR->getCurr();
    num_bytes_NIR += num_bytes;
  }
  else
  {
    num_bytes = 0;
  }
  outstream->put32bitsLE(((U8*)&num_bytes));

  return TRUE;
}

inline BOOL LASwriteItemCompressed_RGBNIR14_v4::chunk_bytes()
{
  U32 num_bytes = 0;
  ByteStreamOut* outstream = enc->getByteStreamOut();

  // output the bytes of all layers

  if (changed_RGB)
  {
    num_bytes = (U32)outstream_RGB->getCurr();
    outstream->putBytes(outstream_RGB->getData(), num_bytes);
  }
  else
  {
    num_bytes = 0;
  }

  if (changed_NIR)
  {
    num_bytes = (U32)outstream_NIR->getCurr();
    outstream->putBytes(outstream_NIR->getData(), num_bytes);
  }
  else
  {
    num_bytes = 0;
  }

  return TRUE;
}

/*
===============================================================================
                       LASwriteItemCompressed_WAVEPACKET14_v4
===============================================================================
*/

LASwriteItemCompressed_WAVEPACKET14_v4::LASwriteItemCompressed_WAVEPACKET14_v4(ArithmeticEncoder* enc)
{
  /* not used as a encoder. just gives access to outstream */

  assert(enc);
  this->enc = enc;

  /* zero outstreams and encoders */

  outstream_wavepacket = 0;

  enc_wavepacket = 0;

  /* zero num_bytes and init booleans */

  num_bytes_wavepacket = 0;

  changed_wavepacket = FALSE;

  /* mark the four scanner channel contexts as uninitialized */

  U32 c;
  for (c = 0; c < 4; c++)
  {
    contexts[c].m_packet_index = 0;
  }
  current_context = 0;
}

LASwriteItemCompressed_WAVEPACKET14_v4::~LASwriteItemCompressed_WAVEPACKET14_v4()
{
  /* destroy all initialized scanner channel contexts */

  U32 c;
  for (c = 0; c < 4; c++)
  {
    if (contexts[c].m_packet_index)
    {
      enc_wavepacket->destroySymbolModel(contexts[c].m_packet_index);
      enc_wavepacket->destroySymbolModel(contexts[c].m_offset_diff[0]);
      enc_wavepacket->destroySymbolModel(contexts[c].m_offset_diff[1]);
      enc_wavepacket->destroySymbolModel(contexts[c].m_offset_diff[2]);
      enc_wavepacket->destroySymbolModel(contexts[c].m_offset_diff[3]);
      delete contexts[c].ic_offset_diff;
      delete contexts[c].ic_packet_size;
      delete contexts[c].ic_return_point;
      delete contexts[c].ic_xyz;
    }
  }

  /* destroy all outstreams and encoders */

  if (outstream_wavepacket)
  {
    delete outstream_wavepacket;

    delete enc_wavepacket;
  }
}

inline BOOL LASwriteItemCompressed_WAVEPACKET14_v4::createAndInitModelsAndCompressors(U32 context, const U8* item)
{
  /* should only be called when context is unused */

  assert(contexts[context].unused);

  /* first create all entropy models (if needed) */

  if (contexts[context].m_packet_index == 0)
  {
      contexts[context].m_packet_index = enc_wavepacket->createSymbolModel(256);
      contexts[context].m_offset_diff[0] = enc_wavepacket->createSymbolModel(4);
      contexts[context].m_offset_diff[1] = enc_wavepacket->createSymbolModel(4);
      contexts[context].m_offset_diff[2] = enc_wavepacket->createSymbolModel(4);
      contexts[context].m_offset_diff[3] = enc_wavepacket->createSymbolModel(4);
      contexts[context].ic_offset_diff = new IntegerCompressor(enc_wavepacket, 32);
      contexts[context].ic_packet_size = new IntegerCompressor(enc_wavepacket, 32);
      contexts[context].ic_return_point = new IntegerCompressor(enc_wavepacket, 32);
      contexts[context].ic_xyz = new IntegerCompressor(enc_wavepacket, 32, 3);
  }

  /* then init entropy models */

  enc_wavepacket->initSymbolModel(contexts[context].m_packet_index);
  enc_wavepacket->initSymbolModel(contexts[context].m_offset_diff[0]);
  enc_wavepacket->initSymbolModel(contexts[context].m_offset_diff[1]);
  enc_wavepacket->initSymbolModel(contexts[context].m_offset_diff[2]);
  enc_wavepacket->initSymbolModel(contexts[context].m_offset_diff[3]);
  contexts[context].ic_offset_diff->initCompressor();
  contexts[context].ic_packet_size->initCompressor();
  contexts[context].ic_return_point->initCompressor();
  contexts[context].ic_xyz->initCompressor();

  /* init current context from item */

  contexts[context].last_diff_32 = 0;
  contexts[context].sym_last_offset_diff = 0;
  memcpy(contexts[context].last_item, item, 29);

  contexts[context].unused = FALSE;

  return TRUE;
}

BOOL LASwriteItemCompressed_WAVEPACKET14_v4::init(const U8* item, U32& context)
{
  /* on the first init create outstreams and encoders */

  if (outstream_wavepacket == 0)
  {
    /* create outstreams */

    if (IS_LITTLE_ENDIAN())
    {
      outstream_wavepacket = new ByteStreamOutArrayLE();
    }
    else
    {
      outstream_wavepacket = new ByteStreamOutArrayBE();
    }

    /* create layer encoders */

    enc_wavepacket = new ArithmeticEncoder();
  }
  else
  {
    /* otherwise just seek back */

    outstream_wavepacket->seek(0);
  }
  
  /* init layer encoders */

  enc_wavepacket->init(outstream_wavepacket);

  /* set changed booleans to FALSE */

  changed_wavepacket = FALSE;

  /* mark the four scanner channel contexts as unused */

  U32 c;
  for (c = 0; c < 4; c++)
  {
    contexts[c].unused = TRUE;
  }

  /* set scanner channel as current context */

  current_context = context; // all other items use context set by POINT14 writer

  /* create and init entropy models and integer compressors (and init contect from item) */

  createAndInitModelsAndCompressors(current_context, item);

  return TRUE;
}

inline BOOL LASwriteItemCompressed_WAVEPACKET14_v4::write(const U8* item, U32& context)
{
  // get last

  U8* last_item = contexts[current_context].last_item;

  // check for context switch

  if (current_context != context)
  {
    current_context = context; // all other items use context set by POINT14 writer
    if (contexts[current_context].unused)
    {
      createAndInitModelsAndCompressors(current_context, (U8*)last_item);
    }
    last_item = contexts[current_context].last_item;
  }

  if (memcmp(item, last_item, 29) != 0)
  {
    changed_wavepacket = TRUE;
  }

  // compress

  enc_wavepacket->encodeSymbol(contexts[current_context].m_packet_index, (U32)(item[0]));

  LASwavepacket13 this_item_m = LASwavepacket13::unpack(item+1);
  LASwavepacket13 last_item_m = LASwavepacket13::unpack(last_item+1);

  // calculate the difference between the two offsets
  I64 curr_diff_64 = this_item_m.offset - last_item_m.offset;
  I32 curr_diff_32 = (I32)curr_diff_64;

  // if the current difference can be represented with 32 bits
  if (curr_diff_64 == (I64)(curr_diff_32))
  {
    if (curr_diff_32 == 0) // current difference is zero
    {
      enc_wavepacket->encodeSymbol(contexts[current_context].m_offset_diff[contexts[current_context].sym_last_offset_diff], 0);
      contexts[current_context].sym_last_offset_diff = 0;
    }
    else if (curr_diff_32 == (I32)last_item_m.packet_size)
    {
      enc_wavepacket->encodeSymbol(contexts[current_context].m_offset_diff[contexts[current_context].sym_last_offset_diff], 1);
      contexts[current_context].sym_last_offset_diff = 1;
    }
    else //
    {
      enc_wavepacket->encodeSymbol(contexts[current_context].m_offset_diff[contexts[current_context].sym_last_offset_diff], 2);
      contexts[current_context].sym_last_offset_diff = 2;
      contexts[current_context].ic_offset_diff->compress(contexts[current_context].last_diff_32, curr_diff_32);
      contexts[current_context].last_diff_32 = curr_diff_32;
    }
  }
  else
  {
    enc_wavepacket->encodeSymbol(contexts[current_context].m_offset_diff[contexts[current_context].sym_last_offset_diff], 3);
    contexts[current_context].sym_last_offset_diff = 3;

    enc_wavepacket->writeInt64(this_item_m.offset);
  }

  contexts[current_context].ic_packet_size->compress(last_item_m.packet_size, this_item_m.packet_size);
  contexts[current_context].ic_return_point->compress(last_item_m.return_point.i32, this_item_m.return_point.i32);
  contexts[current_context].ic_xyz->compress(last_item_m.x.i32, this_item_m.x.i32, 0);
  contexts[current_context].ic_xyz->compress(last_item_m.y.i32, this_item_m.y.i32, 1);
  contexts[current_context].ic_xyz->compress(last_item_m.z.i32, this_item_m.z.i32, 2);

  memcpy(last_item, item, 29);
  return TRUE;
}

inline BOOL LASwriteItemCompressed_WAVEPACKET14_v4::chunk_sizes()
{
  U32 num_bytes = 0;
  ByteStreamOut* outstream = enc->getByteStreamOut();

  // finish the encoders

  enc_wavepacket->done();

  // output the sizes of all layer (i.e.. number of bytes per layer)

  if (changed_wavepacket)
  {
    num_bytes = (U32)outstream_wavepacket->getCurr();
    num_bytes_wavepacket += num_bytes;
  }
  else
  {
    num_bytes = 0;
  }
  outstream->put32bitsLE(((U8*)&num_bytes));

  return TRUE;
}

inline BOOL LASwriteItemCompressed_WAVEPACKET14_v4::chunk_bytes()
{
  U32 num_bytes = 0;
  ByteStreamOut* outstream = enc->getByteStreamOut();

  // output the bytes of all layers

  if (changed_wavepacket)
  {
    num_bytes = (U32)outstream_wavepacket->getCurr();
    outstream->putBytes(outstream_wavepacket->getData(), num_bytes);
  }

  return TRUE;
}

/*
===============================================================================
                      LASwriteItemCompressed_BYTE14_v4
===============================================================================
*/

LASwriteItemCompressed_BYTE14_v4::LASwriteItemCompressed_BYTE14_v4(ArithmeticEncoder* enc, U32 number)
{
  /* not used as a encoder. just gives access to outstream */

  assert(enc);
  this->enc = enc;

  /* must be more than one byte */

  assert(number);
  this->number = number;

  /* zero outstream and encoder pointer arrays */

  outstream_Bytes = 0;

  enc_Bytes = 0;

  /* number of bytes per layer */
  
  num_bytes_Bytes = new U32[number];

  changed_Bytes = new BOOL[number];

  U32 i;
  for (i = 0; i < number; i++)
  {
    num_bytes_Bytes[i] = 0;

    changed_Bytes[i] = FALSE;
  }

  /* mark the four scanner channel contexts as uninitialized */

  U32 c;
  for (c = 0; c < 4; c++)
  {
    contexts[c].m_bytes = 0;
  }
  current_context = 0;
}

LASwriteItemCompressed_BYTE14_v4::~LASwriteItemCompressed_BYTE14_v4()
{
  /* destroy all initialized scanner channel contexts */

  U32 c, i;
  for (c = 0; c < 4; c++)
  {
    if (contexts[c].m_bytes)
    {
      for (i = 0; i < number; i++)
      {
        enc_Bytes[i]->destroySymbolModel(contexts[c].m_bytes[i]);
      }
      delete [] contexts[c].m_bytes;
      delete [] contexts[c].last_item;
    }
  }

  /* destroy all outstream and encoder arrays */

  if (outstream_Bytes)
  {
    for (i = 0; i < number; i++)
    {
      if (outstream_Bytes[i])
      {
        delete outstream_Bytes[i];
        delete enc_Bytes[i];
      }
    }

    delete [] outstream_Bytes;
    delete [] enc_Bytes;
  }

  /* destroy all other arrays */

  if (num_bytes_Bytes) delete [] num_bytes_Bytes;

  if (changed_Bytes) delete [] changed_Bytes;
}

inline BOOL LASwriteItemCompressed_BYTE14_v4::createAndInitModelsAndCompressors(U32 context, const U8* item)
{
  U32 i;

  /* should only be called when context is unused */

  assert(contexts[context].unused);

  /* first create all entropy models and last items (if needed) */

  if (contexts[context].m_bytes == 0)
  {
    contexts[context].m_bytes = new ArithmeticModel*[number];
    for (i = 0; i < number; i++)
    {
      contexts[context].m_bytes[i] = enc_Bytes[i]->createSymbolModel(256);
      enc_Bytes[i]->initSymbolModel(contexts[context].m_bytes[i]);
    }

    /* create last item */
    contexts[context].last_item = new U8[number];
  }

  /* then init entropy models */

  for (i = 0; i < number; i++)
  {
    enc_Bytes[i]->initSymbolModel(contexts[context].m_bytes[i]);
  }

  /* init current context from item */

  memcpy(contexts[context].last_item, item, number);

  contexts[context].unused = FALSE;

  return TRUE;
}

BOOL LASwriteItemCompressed_BYTE14_v4::init(const U8* item, U32& context)
{
  U32 i;

  /* on the first init create outstreams and encoders */

  if (outstream_Bytes == 0)
  {
    /* create outstreams pointer array */

    outstream_Bytes = new ByteStreamOutArray*[number];

    /* create outstreams */

    if (IS_LITTLE_ENDIAN())
    {
      for (i = 0; i < number; i++)
      {
        outstream_Bytes[i] = new ByteStreamOutArrayLE();
      }
    }
    else
    {
      for (i = 0; i < number; i++)
      {
        outstream_Bytes[i] = new ByteStreamOutArrayBE();
      }
    }

    /* create encoder pointer array */

    enc_Bytes = new ArithmeticEncoder*[number];

    /* create layer encoders */

    for (i = 0; i < number; i++)
    {
      enc_Bytes[i] = new ArithmeticEncoder();
    }
  }
  else
  {
    /* otherwise just seek back */

    for (i = 0; i < number; i++)
    {
      outstream_Bytes[i]->seek(0);
    }
  }
  
  /* init layer encoders */

  for (i = 0; i < number; i++)
  {
    enc_Bytes[i]->init(outstream_Bytes[i]);
  }

  /* set changed booleans to FALSE */

  for (i = 0; i < number; i++)
  {
    changed_Bytes[i] = FALSE;
  }

  /* mark the four scanner channel contexts as unused */

  U32 c;
  for (c = 0; c < 4; c++)
  {
    contexts[c].unused = TRUE;
  }

  /* set scanner channel as current context */

  current_context = context; // all other items use context set by POINT14 writer

  /* create and init entropy models and integer compressors (and init context from item)  */

  createAndInitModelsAndCompressors(current_context, item);

  return TRUE;
}

inline BOOL LASwriteItemCompressed_BYTE14_v4::write(const U8* item, U32& context)
{
  // get last

  U8* last_item = contexts[current_context].last_item;

  // check for context switch

  if (current_context != context)
  {
    current_context = context; // all other items use context set by POINT14 writer
    if (contexts[current_context].unused)
    {
      createAndInitModelsAndCompressors(current_context, last_item);
    }
    last_item = contexts[current_context].last_item;
  }

  // compress

  U32 i;
  I32 diff;
  for (i = 0; i < number; i++)
  {
    diff = item[i] - last_item[i];
    enc_Bytes[i]->encodeSymbol(contexts[current_context].m_bytes[i], U8_FOLD(diff));
    if (diff)
    {
      changed_Bytes[i] = TRUE;
      last_item[i] = item[i];
    }
  }
  return TRUE;
}

inline BOOL LASwriteItemCompressed_BYTE14_v4::chunk_sizes()
{
  U32 i;
  U32 num_bytes = 0;
  ByteStreamOut* outstream = enc->getByteStreamOut();

  // output the sizes of all layer (i.e.. number of bytes per layer)

  for (i = 0; i < number; i++)
  {
    // finish the encoders
    enc_Bytes[i]->done();

    if (changed_Bytes[i])
    {
      num_bytes = (U32)outstream_Bytes[i]->getCurr();
      num_bytes_Bytes[i] += num_bytes;
    }
    else
    {
      num_bytes = 0;
    }
    outstream->put32bitsLE(((U8*)&num_bytes));
  }

  return TRUE;
}

inline BOOL LASwriteItemCompressed_BYTE14_v4::chunk_bytes()
{
  U32 i;
  U32 num_bytes = 0;
  ByteStreamOut* outstream = enc->getByteStreamOut();

  // output the bytes of all layers

  for (i = 0; i < number; i++)
  {
    if (changed_Bytes[i])
    {
      num_bytes = (U32)outstream_Bytes[i]->getCurr();
      outstream->putBytes(outstream_Bytes[i]->getData(), num_bytes);
    }
    else
    {
      num_bytes = 0;
    }
  }

  return TRUE;
}
