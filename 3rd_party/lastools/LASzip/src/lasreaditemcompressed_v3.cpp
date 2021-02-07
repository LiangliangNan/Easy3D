/*
===============================================================================

  FILE:  lasreaditemcompressed_v3.cpp
  
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

#include "lasreaditemcompressed_v3.hpp"

#include <assert.h>
#include <string.h>

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
  U8 legacy_flags : 3;
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

LASreadItemCompressed_POINT14_v3::LASreadItemCompressed_POINT14_v3(ArithmeticDecoder* dec, const U32 decompress_selective)
{
  /* not used as a decoder. just gives access to instream */

  assert(dec);
  this->dec = dec;

  /* zero instreams and decoders */

  instream_channel_returns_XY = 0;
  instream_Z = 0;
  instream_classification = 0;
  instream_flags = 0;
  instream_intensity = 0;
  instream_scan_angle = 0;
  instream_user_data = 0;
  instream_point_source = 0;
  instream_gps_time = 0;

  dec_channel_returns_XY = 0;
  dec_Z = 0;
  dec_classification = 0;
  dec_flags = 0;
  dec_intensity = 0;
  dec_scan_angle = 0;
  dec_user_data = 0;
  dec_point_source = 0;
  dec_gps_time = 0;

  /* mark the four scanner channel contexts as uninitialized */

  U32 c;
  for (c = 0; c < 4; c++)
  {
    contexts[c].m_changed_values[0] = 0;
  }
  current_context = 0;

  /* zero num_bytes and init booleans */

  num_bytes_channel_returns_XY = 0;
  num_bytes_Z = 0;
  num_bytes_classification = 0;
  num_bytes_flags = 0;
  num_bytes_intensity = 0;
  num_bytes_scan_angle = 0;
  num_bytes_user_data = 0;
  num_bytes_point_source = 0;
  num_bytes_gps_time = 0;

  changed_Z = FALSE;
  changed_classification = FALSE;
  changed_flags = FALSE;
  changed_intensity = FALSE;
  changed_scan_angle = FALSE;
  changed_user_data = FALSE;
  changed_point_source = FALSE;
  changed_gps_time = FALSE;

  requested_Z = (decompress_selective & LASZIP_DECOMPRESS_SELECTIVE_Z ? TRUE : FALSE);
  requested_classification = (decompress_selective & LASZIP_DECOMPRESS_SELECTIVE_CLASSIFICATION ? TRUE : FALSE);
  requested_flags = (decompress_selective & LASZIP_DECOMPRESS_SELECTIVE_FLAGS ? TRUE : FALSE);
  requested_intensity = (decompress_selective & LASZIP_DECOMPRESS_SELECTIVE_INTENSITY ? TRUE : FALSE);
  requested_scan_angle = (decompress_selective & LASZIP_DECOMPRESS_SELECTIVE_SCAN_ANGLE ? TRUE : FALSE);
  requested_user_data = (decompress_selective & LASZIP_DECOMPRESS_SELECTIVE_USER_DATA ? TRUE : FALSE);
  requested_point_source = (decompress_selective & LASZIP_DECOMPRESS_SELECTIVE_POINT_SOURCE ? TRUE : FALSE);
  requested_gps_time = (decompress_selective & LASZIP_DECOMPRESS_SELECTIVE_GPS_TIME ? TRUE : FALSE);

  /* init the bytes buffer to zero */

  bytes = 0;
  num_bytes_allocated = 0;
}

LASreadItemCompressed_POINT14_v3::~LASreadItemCompressed_POINT14_v3()
{
  U32 c, i;

  /* destroy all initialized scanner channel contexts */

  for (c = 0; c < 4; c++)
  {
    if (contexts[c].m_changed_values[0])
    {
      dec_channel_returns_XY->destroySymbolModel(contexts[c].m_changed_values[0]);
      dec_channel_returns_XY->destroySymbolModel(contexts[c].m_changed_values[1]);
      dec_channel_returns_XY->destroySymbolModel(contexts[c].m_changed_values[2]);
      dec_channel_returns_XY->destroySymbolModel(contexts[c].m_changed_values[3]);
      dec_channel_returns_XY->destroySymbolModel(contexts[c].m_changed_values[4]);
      dec_channel_returns_XY->destroySymbolModel(contexts[c].m_changed_values[5]);
      dec_channel_returns_XY->destroySymbolModel(contexts[c].m_changed_values[6]);
      dec_channel_returns_XY->destroySymbolModel(contexts[c].m_changed_values[7]);
      dec_channel_returns_XY->destroySymbolModel(contexts[c].m_scanner_channel);
      for (i = 0; i < 16; i++)
      {
        if (contexts[c].m_number_of_returns[i]) dec_channel_returns_XY->destroySymbolModel(contexts[c].m_number_of_returns[i]);
        if (contexts[c].m_return_number[i]) dec_channel_returns_XY->destroySymbolModel(contexts[c].m_return_number[i]);
      }
      dec_channel_returns_XY->destroySymbolModel(contexts[c].m_return_number_gps_same);
      delete contexts[c].ic_dX;
      delete contexts[c].ic_dY;
      delete contexts[c].ic_Z;
      for (i = 0; i < 64; i++)
      {
        if (contexts[c].m_classification[i]) dec_classification->destroySymbolModel(contexts[c].m_classification[i]);
        if (contexts[c].m_flags[i]) dec_flags->destroySymbolModel(contexts[c].m_flags[i]);
        if (contexts[c].m_user_data[i]) dec_user_data->destroySymbolModel(contexts[c].m_user_data[i]);
      }
      delete contexts[c].ic_intensity;
      delete contexts[c].ic_scan_angle;
      delete contexts[c].ic_point_source_ID;
      dec_gps_time->destroySymbolModel(contexts[c].m_gpstime_multi);
      dec_gps_time->destroySymbolModel(contexts[c].m_gpstime_0diff);
      delete contexts[c].ic_gpstime;
    }
  }

  /* destroy all decoders and instreams */

  if (instream_channel_returns_XY)
  {
    delete dec_channel_returns_XY;
    delete dec_Z;
    delete dec_classification;
    delete dec_flags;
    delete dec_intensity;
    delete dec_scan_angle;
    delete dec_user_data;
    delete dec_gps_time;

    delete instream_channel_returns_XY;
    delete instream_Z;
    delete instream_classification;
    delete instream_flags;
    delete instream_intensity;
    delete instream_scan_angle;
    delete instream_user_data;
    delete instream_gps_time;
  }

  if (bytes) delete [] bytes;
}

inline BOOL LASreadItemCompressed_POINT14_v3::createAndInitModelsAndDecompressors(U32 context, const U8* item)
{
  I32 i;

  /* should only be called when context is unused */

  assert(contexts[context].unused);

  /* first create all entropy models and integer decompressors (if needed) */

  if (contexts[context].m_changed_values[0] == 0)
  {
    /* for the channel_returns_XY layer */

    contexts[context].m_changed_values[0] = dec_channel_returns_XY->createSymbolModel(128);
    contexts[context].m_changed_values[1] = dec_channel_returns_XY->createSymbolModel(128);
    contexts[context].m_changed_values[2] = dec_channel_returns_XY->createSymbolModel(128);
    contexts[context].m_changed_values[3] = dec_channel_returns_XY->createSymbolModel(128);
    contexts[context].m_changed_values[4] = dec_channel_returns_XY->createSymbolModel(128);
    contexts[context].m_changed_values[5] = dec_channel_returns_XY->createSymbolModel(128);
    contexts[context].m_changed_values[6] = dec_channel_returns_XY->createSymbolModel(128);
    contexts[context].m_changed_values[7] = dec_channel_returns_XY->createSymbolModel(128);
    contexts[context].m_scanner_channel = dec_channel_returns_XY->createSymbolModel(3);
    for (i = 0; i < 16; i++)
    {
      contexts[context].m_number_of_returns[i] = 0;
      contexts[context].m_return_number[i] = 0;
    }
    contexts[context].m_return_number_gps_same = dec_channel_returns_XY->createSymbolModel(13);

    contexts[context].ic_dX = new IntegerCompressor(dec_channel_returns_XY, 32, 2);  // 32 bits, 2 context
    contexts[context].ic_dY = new IntegerCompressor(dec_channel_returns_XY, 32, 22); // 32 bits, 22 contexts

    /* for the Z layer */

    contexts[context].ic_Z = new IntegerCompressor(dec_Z, 32, 20);  // 32 bits, 20 contexts

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

    contexts[context].ic_intensity = new IntegerCompressor(dec_intensity, 16, 4);

    /* for the scan_angle layer */

    contexts[context].ic_scan_angle = new IntegerCompressor(dec_scan_angle, 16, 2);

    /* for the point_source_ID layer */

    contexts[context].ic_point_source_ID = new IntegerCompressor(dec_point_source, 16);

    /* for the gps_time layer */

    contexts[context].m_gpstime_multi = dec_gps_time->createSymbolModel(LASZIP_GPSTIME_MULTI_TOTAL);
    contexts[context].m_gpstime_0diff = dec_gps_time->createSymbolModel(5);
    contexts[context].ic_gpstime = new IntegerCompressor(dec_gps_time, 32, 9); // 32 bits, 9 contexts
  }

  /* then init entropy models and integer compressors */

  /* for the channel_returns_XY layer */

  dec_channel_returns_XY->initSymbolModel(contexts[context].m_changed_values[0]);
  dec_channel_returns_XY->initSymbolModel(contexts[context].m_changed_values[1]);
  dec_channel_returns_XY->initSymbolModel(contexts[context].m_changed_values[2]);
  dec_channel_returns_XY->initSymbolModel(contexts[context].m_changed_values[3]);
  dec_channel_returns_XY->initSymbolModel(contexts[context].m_changed_values[4]);
  dec_channel_returns_XY->initSymbolModel(contexts[context].m_changed_values[5]);
  dec_channel_returns_XY->initSymbolModel(contexts[context].m_changed_values[6]);
  dec_channel_returns_XY->initSymbolModel(contexts[context].m_changed_values[7]);
  dec_channel_returns_XY->initSymbolModel(contexts[context].m_scanner_channel);
  for (i = 0; i < 16; i++)
  {
    if (contexts[context].m_number_of_returns[i]) dec_channel_returns_XY->initSymbolModel(contexts[context].m_number_of_returns[i]);
    if (contexts[context].m_return_number[i]) dec_channel_returns_XY->initSymbolModel(contexts[context].m_return_number[i]);
  }
  dec_channel_returns_XY->initSymbolModel(contexts[context].m_return_number_gps_same);
  contexts[context].ic_dX->initDecompressor();
  contexts[context].ic_dY->initDecompressor();
  for (i = 0; i < 12; i++)
  {
    contexts[context].last_X_diff_median5[i].init();
    contexts[context].last_Y_diff_median5[i].init();
  }

  /* for the Z layer */

  contexts[context].ic_Z->initDecompressor();
  for (i = 0; i < 8; i++)
  {
    contexts[context].last_Z[i] = ((LASpoint14*)item)->Z;
  }

  /* for the classification layer */
  /* for the flags layer */
  /* for the user_data layer */

  for (i = 0; i < 64; i++)
  {
    if (contexts[context].m_classification[i]) dec_classification->initSymbolModel(contexts[context].m_classification[i]);
    if (contexts[context].m_flags[i]) dec_flags->initSymbolModel(contexts[context].m_flags[i]);
    if (contexts[context].m_user_data[i]) dec_user_data->initSymbolModel(contexts[context].m_user_data[i]);
  }

  /* for the intensity layer */

  contexts[context].ic_intensity->initDecompressor();
  for (i = 0; i < 8; i++)
  {
    contexts[context].last_intensity[i] = ((LASpoint14*)item)->intensity;
  }

  /* for the scan_angle layer */

  contexts[context].ic_scan_angle->initDecompressor();

  /* for the point_source_ID layer */

  contexts[context].ic_point_source_ID->initDecompressor();

  /* for the gps_time layer */

  dec_gps_time->initSymbolModel(contexts[context].m_gpstime_multi);
  dec_gps_time->initSymbolModel(contexts[context].m_gpstime_0diff);
  contexts[context].ic_gpstime->initDecompressor();
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

  /* init current context from last item */

  memcpy(contexts[context].last_item, item, sizeof(LASpoint14));
  ((LASpoint14*)contexts[context].last_item)->gps_time_change = FALSE;

//  fprintf(stderr, "INIT: current_context %d last item %.14g %d %d %d %d %d %d\n", current_context, ((LASpoint14*)item)->gps_time, ((LASpoint14*)item)->X, ((LASpoint14*)item)->Y, ((LASpoint14*)item)->Z, ((LASpoint14*)item)->intensity, ((LASpoint14*)item)->return_number, ((LASpoint14*)item)->number_of_returns);

  contexts[context].unused = FALSE;

  return TRUE;
}

BOOL LASreadItemCompressed_POINT14_v3::chunk_sizes()
{
  /* for layered compression 'dec' only hands over the stream */

  ByteStreamIn* instream = dec->getByteStreamIn();

  /* read bytes per layer */

  instream->get32bitsLE(((U8*)&num_bytes_channel_returns_XY));
  instream->get32bitsLE(((U8*)&num_bytes_Z));
  instream->get32bitsLE(((U8*)&num_bytes_classification));
  instream->get32bitsLE(((U8*)&num_bytes_flags));
  instream->get32bitsLE(((U8*)&num_bytes_intensity));
  instream->get32bitsLE(((U8*)&num_bytes_scan_angle));
  instream->get32bitsLE(((U8*)&num_bytes_user_data));
  instream->get32bitsLE(((U8*)&num_bytes_point_source));
  instream->get32bitsLE(((U8*)&num_bytes_gps_time));

  return TRUE;
}

BOOL LASreadItemCompressed_POINT14_v3::init(const U8* item, U32& context)
{
  /* for layered compression 'dec' only hands over the stream */

  ByteStreamIn* instream = dec->getByteStreamIn();

  /* on the first init create instreams and decoders */

  if (instream_channel_returns_XY == 0)
  {
    /* create instreams */

    if (IS_LITTLE_ENDIAN())
    {
      instream_channel_returns_XY = new ByteStreamInArrayLE();
      instream_Z = new ByteStreamInArrayLE();
      instream_classification = new ByteStreamInArrayLE();
      instream_flags = new ByteStreamInArrayLE();
      instream_intensity = new ByteStreamInArrayLE();
      instream_scan_angle = new ByteStreamInArrayLE();
      instream_user_data = new ByteStreamInArrayLE();
      instream_point_source = new ByteStreamInArrayLE();
      instream_gps_time = new ByteStreamInArrayLE();
    }
    else
    {
      instream_channel_returns_XY = new ByteStreamInArrayBE();
      instream_Z = new ByteStreamInArrayBE();
      instream_classification = new ByteStreamInArrayBE();
      instream_flags = new ByteStreamInArrayBE();
      instream_intensity = new ByteStreamInArrayBE();
      instream_scan_angle = new ByteStreamInArrayBE();
      instream_user_data = new ByteStreamInArrayBE();
      instream_point_source = new ByteStreamInArrayBE();
      instream_gps_time = new ByteStreamInArrayBE();
    }

    /* create decoders */

    dec_channel_returns_XY = new ArithmeticDecoder();
    dec_Z = new ArithmeticDecoder();
    dec_classification = new ArithmeticDecoder();
    dec_flags = new ArithmeticDecoder();
    dec_intensity = new ArithmeticDecoder();
    dec_scan_angle = new ArithmeticDecoder();
    dec_user_data = new ArithmeticDecoder();
    dec_point_source = new ArithmeticDecoder();
    dec_gps_time = new ArithmeticDecoder();
  }

  /* how many bytes do we need to read */
  
  U32 num_bytes = num_bytes_channel_returns_XY;
  if (requested_Z) num_bytes += num_bytes_Z; 
  if (requested_classification) num_bytes += num_bytes_classification; 
  if (requested_flags) num_bytes += num_bytes_flags; 
  if (requested_intensity) num_bytes += num_bytes_intensity;
  if (requested_scan_angle) num_bytes += num_bytes_scan_angle;
  if (requested_user_data) num_bytes += num_bytes_user_data;
  if (requested_point_source) num_bytes += num_bytes_point_source;
  if (requested_gps_time) num_bytes += num_bytes_gps_time;
  
  /* make sure the buffer is sufficiently large */

  if (num_bytes > num_bytes_allocated)
  {
    if (bytes) delete [] bytes;
    bytes = new U8[num_bytes];
    if (bytes == 0) return FALSE;
    num_bytes_allocated = num_bytes;
  }

  /* load the requested bytes and init the corresponding instreams and decoders */

  num_bytes = 0;
  instream->getBytes(bytes, num_bytes_channel_returns_XY);
  instream_channel_returns_XY->init(bytes, num_bytes_channel_returns_XY);
  dec_channel_returns_XY->init(instream_channel_returns_XY);
  num_bytes += num_bytes_channel_returns_XY;

  if (requested_Z)
  {
    if (num_bytes_Z)
    {
      instream->getBytes(&(bytes[num_bytes]), num_bytes_Z);
      instream_Z->init(&(bytes[num_bytes]), num_bytes_Z);
      dec_Z->init(instream_Z);
      num_bytes += num_bytes_Z;
      changed_Z = TRUE;
    }
    else
    {
      instream_Z->init(0, 0);
      changed_Z = FALSE;
    }
  }
  else 
  {
    if (num_bytes_Z)
    {
      instream->skipBytes(num_bytes_Z);
    }
    changed_Z = FALSE;
  }

  if (requested_classification)
  {
    if (num_bytes_classification)
    {
      instream->getBytes(&(bytes[num_bytes]), num_bytes_classification);
      instream_classification->init(&(bytes[num_bytes]), num_bytes_classification);
      dec_classification->init(instream_classification);
      num_bytes += num_bytes_classification;
      changed_classification = TRUE;
    }
    else
    {
      instream_classification->init(0, 0);
      changed_classification = FALSE;
    }
  }
  else 
  {
    if (num_bytes_classification)
    {
      instream->skipBytes(num_bytes_classification);
    }
    changed_classification = FALSE;
  }

  if (requested_flags)
  {
    if (num_bytes_flags)
    {
      instream->getBytes(&(bytes[num_bytes]), num_bytes_flags);
      instream_flags->init(&(bytes[num_bytes]), num_bytes_flags);
      dec_flags->init(instream_flags);
      num_bytes += num_bytes_flags;
      changed_flags = TRUE;
    }
    else
    {
      instream_flags->init(0, 0);
      changed_flags = FALSE;
    }
  }
  else 
  {
    if (num_bytes_flags)
    {
      instream->skipBytes(num_bytes_flags);
    }
    changed_flags = FALSE;
  }

  if (requested_intensity)
  {
    if (num_bytes_intensity)
    {
      instream->getBytes(&(bytes[num_bytes]), num_bytes_intensity);
      instream_intensity->init(&(bytes[num_bytes]), num_bytes_intensity);
      dec_intensity->init(instream_intensity);
      num_bytes += num_bytes_intensity;
      changed_intensity = TRUE;
    }
    else
    {
      instream_intensity->init(0, 0);
      changed_intensity = FALSE;
    }
  }
  else 
  {
    if (num_bytes_intensity)
    {
      instream->skipBytes(num_bytes_intensity);
    }
    changed_intensity = FALSE;
  }

  if (requested_scan_angle)
  {
    if (num_bytes_scan_angle)
    {
      instream->getBytes(&(bytes[num_bytes]), num_bytes_scan_angle);
      instream_scan_angle->init(&(bytes[num_bytes]), num_bytes_scan_angle);
      dec_scan_angle->init(instream_scan_angle);
      num_bytes += num_bytes_scan_angle;
      changed_scan_angle = TRUE;
    }
    else
    {
      instream_scan_angle->init(0, 0);
      changed_scan_angle = FALSE;
    }
  }
  else 
  {
    if (num_bytes_scan_angle)
    {
      instream->skipBytes(num_bytes_scan_angle);
    }
    changed_scan_angle = FALSE;
  }

  if (requested_user_data)
  {
    if (num_bytes_user_data)
    {
      instream->getBytes(&(bytes[num_bytes]), num_bytes_user_data);
      instream_user_data->init(&(bytes[num_bytes]), num_bytes_user_data);
      dec_user_data->init(instream_user_data);
      num_bytes += num_bytes_user_data;
      changed_user_data = TRUE;
    }
    else
    {
      instream_user_data->init(0, 0);
      changed_user_data = FALSE;
    }
  }
  else 
  {
    if (num_bytes_user_data)
    {
      instream->skipBytes(num_bytes_user_data);
    }
    changed_user_data = FALSE;
  }

  if (requested_point_source)
  {
    if (num_bytes_point_source)
    {
      instream->getBytes(&(bytes[num_bytes]), num_bytes_point_source);
      instream_point_source->init(&(bytes[num_bytes]), num_bytes_point_source);
      dec_point_source->init(instream_point_source);
      num_bytes += num_bytes_point_source;
      changed_point_source = TRUE;
    }
    else
    {
      instream_point_source->init(0, 0);
      changed_point_source = FALSE;
    }
  }
  else
  {
    if (num_bytes_point_source)
    {
      instream->skipBytes(num_bytes_point_source);
    }
    changed_point_source = FALSE;
  }

  if (requested_gps_time)
  {
    if (num_bytes_gps_time)
    {
      instream->getBytes(&(bytes[num_bytes]), num_bytes_gps_time);
      instream_gps_time->init(&(bytes[num_bytes]), num_bytes_gps_time);
      dec_gps_time->init(instream_gps_time);
      num_bytes += num_bytes_gps_time;
      changed_gps_time = TRUE;
    }
    else
    {
      instream_gps_time->init(0, 0);
      changed_gps_time = FALSE;
    }
  }
  else
  {
    if (num_bytes_gps_time)
    {
      instream->skipBytes(num_bytes_gps_time);
    }
    changed_gps_time = FALSE;
  }

  /* mark the four scanner channel contexts as unused */

  U32 c;
  for (c = 0; c < 4; c++)
  {
    contexts[c].unused = TRUE;
  }

  /* set scanner channel as current context */

  current_context = ((LASpoint14*)item)->scanner_channel;
  context = current_context; // the POINT14 reader sets context for all other items

  /* create and init models and decompressors */

  createAndInitModelsAndDecompressors(current_context, item);

  return TRUE;
}

inline void LASreadItemCompressed_POINT14_v3::read(U8* item, U32& context)
{
  // get last

  U8* last_item = contexts[current_context].last_item;

  ////////////////////////////////////////
  // decompress returns_XY layer 
  ////////////////////////////////////////

  // create single (3) / first (1) / last (2) / intermediate (0) context from last point return

  I32 lpr = (((LASpoint14*)last_item)->return_number == 1 ? 1 : 0); // first?
  lpr += (((LASpoint14*)last_item)->return_number >= ((LASpoint14*)last_item)->number_of_returns ? 2 : 0); // last?

  // add info whether the GPS time changed in the last return to the context

  lpr += (((LASpoint14*)last_item)->gps_time_change ? 4 : 0);

  // decompress which values have changed with last point return context

  I32 changed_values = dec_channel_returns_XY->decodeSymbol(contexts[current_context].m_changed_values[lpr]);

  // if scanner channel has changed

  if (changed_values & (1 << 6))
  {
    U32 diff = dec_channel_returns_XY->decodeSymbol(contexts[current_context].m_scanner_channel); // curr = last + (sym + 1)
    U32 scanner_channel = (current_context + diff + 1) % 4;
    // maybe create and init entropy models and integer compressors
    if (contexts[scanner_channel].unused)
    {
      // create and init entropy models and integer decompressors
      createAndInitModelsAndDecompressors(scanner_channel, contexts[current_context].last_item);
    }
    // switch context to current scanner channel
    current_context = scanner_channel;
    context = current_context; // the POINT14 reader sets context for all other items

    // get last for new context
    last_item = contexts[current_context].last_item;
    ((LASpoint14*)last_item)->scanner_channel = scanner_channel;
  }

  // determine changed attributes

  BOOL point_source_change = (changed_values & (1 << 5) ? TRUE : FALSE);
  BOOL gps_time_change = (changed_values & (1 << 4) ? TRUE : FALSE);
  BOOL scan_angle_change = (changed_values & (1 << 3) ? TRUE : FALSE);

  // get last return counts

  U32 last_n = ((LASpoint14*)last_item)->number_of_returns;
  U32 last_r = ((LASpoint14*)last_item)->return_number;

  // if number of returns is different we decompress it

  U32 n;
  if (changed_values & (1 << 2))
  {
    if (contexts[current_context].m_number_of_returns[last_n] == 0)
    {
      contexts[current_context].m_number_of_returns[last_n] = dec_channel_returns_XY->createSymbolModel(16);
      dec_channel_returns_XY->initSymbolModel(contexts[current_context].m_number_of_returns[last_n]);
    }
    n = dec_channel_returns_XY->decodeSymbol(contexts[current_context].m_number_of_returns[last_n]);
    ((LASpoint14*)last_item)->number_of_returns = n;
  }
  else
  {
    n = last_n;
  }

  // how is the return number different

  U32 r;
  if ((changed_values & 3) == 0) // same return number
  {
    r = last_r;
  }
  else if ((changed_values & 3) == 1) // return number plus 1 mod 16
  {
    r = ((last_r + 1) % 16);
    ((LASpoint14*)last_item)->return_number = r;
  }
  else if ((changed_values & 3) == 2) // return number minus 1 mod 16
  {
    r = ((last_r + 15) % 16);
    ((LASpoint14*)last_item)->return_number = r;
  }
  else
  {
    // the return number difference is bigger than +1 / -1 so we decompress how it is different

    if (gps_time_change) // if the GPS time has changed
    {
      if (contexts[current_context].m_return_number[last_r] == 0)
      {
        contexts[current_context].m_return_number[last_r] = dec_channel_returns_XY->createSymbolModel(16);
        dec_channel_returns_XY->initSymbolModel(contexts[current_context].m_return_number[last_r]);
      }
      r = dec_channel_returns_XY->decodeSymbol(contexts[current_context].m_return_number[last_r]);
    }
    else // if the GPS time has not changed
    {
      I32 sym = dec_channel_returns_XY->decodeSymbol(contexts[current_context].m_return_number_gps_same);
      r = (last_r + (sym + 2)) % 16;
    }
    ((LASpoint14*)last_item)->return_number = r;
  }

  // set legacy return counts and number of returns

  if (n > 7)
  {
    if (r > 6)
    {
      if (r >= n)
      {
        ((LASpoint14*)last_item)->legacy_return_number = 7;
      }
      else
      {
        ((LASpoint14*)last_item)->legacy_return_number = 6;
      }
    }
    else
    {
      ((LASpoint14*)last_item)->legacy_return_number = r;
    }
    ((LASpoint14*)last_item)->legacy_number_of_returns = 7;
  }
  else
  {
    ((LASpoint14*)last_item)->legacy_return_number = r;
    ((LASpoint14*)last_item)->legacy_number_of_returns = n;
  }

  // get return map m and return level l context for current point

  U32 m = number_return_map_6ctx[n][r];
  U32 l = number_return_level_8ctx[n][r];

  // create single (3) / first (1) / last (2) / intermediate (0) return context for current point

  I32 cpr = (r == 1 ? 2 : 0); // first ?
  cpr += (r >= n ? 1 : 0); // last ?

  U32 k_bits;
  I32 median, diff;

  // decompress X coordinate
  median = contexts[current_context].last_X_diff_median5[(m<<1) | gps_time_change].get();
  diff = contexts[current_context].ic_dX->decompress(median, n==1);
  ((LASpoint14*)last_item)->X += diff;
  contexts[current_context].last_X_diff_median5[(m<<1) | gps_time_change].add(diff);

  // decompress Y coordinate
  median = contexts[current_context].last_Y_diff_median5[(m<<1) | gps_time_change].get();
  k_bits = contexts[current_context].ic_dX->getK();
  diff = contexts[current_context].ic_dY->decompress(median, (n==1) + ( k_bits < 20 ? U32_ZERO_BIT_0(k_bits) : 20 ));
  ((LASpoint14*)last_item)->Y += diff;
  contexts[current_context].last_Y_diff_median5[(m<<1) | gps_time_change].add(diff);

  ////////////////////////////////////////
  // decompress Z layer (if changed and requested)
  ////////////////////////////////////////

  if (changed_Z) // if the Z coordinate should be decompressed and changes within this chunk
  {
    k_bits = (contexts[current_context].ic_dX->getK() + contexts[current_context].ic_dY->getK()) / 2;
    ((LASpoint14*)last_item)->Z = contexts[current_context].ic_Z->decompress(contexts[current_context].last_Z[l], (n==1) + (k_bits < 18 ? U32_ZERO_BIT_0(k_bits) : 18));
    contexts[current_context].last_Z[l] = ((LASpoint14*)last_item)->Z;
  }

  ////////////////////////////////////////
  // decompress classifications layer (if changed and requested)
  ////////////////////////////////////////

  if (changed_classification) // if the classification should be decompressed and changes within this chunk
  {
    U32 last_classification = ((LASpoint14*)last_item)->classification;
    I32 ccc = ((last_classification & 0x1F) << 1) + (cpr == 3 ? 1 : 0);
    if (contexts[current_context].m_classification[ccc] == 0)
    {
      contexts[current_context].m_classification[ccc] = dec_classification->createSymbolModel(256);
      dec_classification->initSymbolModel(contexts[current_context].m_classification[ccc]);
    }
    ((LASpoint14*)last_item)->classification = dec_classification->decodeSymbol(contexts[current_context].m_classification[ccc]);

    // legacy copies
    if (((LASpoint14*)last_item)->classification < 32)
    {
      ((LASpoint14*)last_item)->legacy_classification = ((LASpoint14*)last_item)->classification;
    }
  }

  ////////////////////////////////////////
  // decompress flags layer (if changed and requested)
  ////////////////////////////////////////

  if (changed_flags) // if the flags should be decompressed and change within this chunk
  {
    U32 last_flags = (((LASpoint14*)last_item)->edge_of_flight_line << 5) | (((LASpoint14*)last_item)->scan_direction_flag << 4) | ((LASpoint14*)last_item)->classification_flags;
    if (contexts[current_context].m_flags[last_flags] == 0)
    {
      contexts[current_context].m_flags[last_flags] = dec_flags->createSymbolModel(64);
      dec_flags->initSymbolModel(contexts[current_context].m_flags[last_flags]);
    }
    U32 flags = dec_flags->decodeSymbol(contexts[current_context].m_flags[last_flags]);
    ((LASpoint14*)last_item)->edge_of_flight_line = !!(flags & (1 << 5));
    ((LASpoint14*)last_item)->scan_direction_flag = !!(flags & (1 << 4));
    ((LASpoint14*)last_item)->classification_flags = (flags & 0x0F);

    // legacy copies
    ((LASpoint14*)last_item)->legacy_flags = (flags & 0x07);
  }

  ////////////////////////////////////////
  // decompress intensity layer (if changed and requested)
  ////////////////////////////////////////

  if (changed_intensity) // if the intensity should be decompressed and changes within this chunk
  {
    U16 intensity = contexts[current_context].ic_intensity->decompress(contexts[current_context].last_intensity[(cpr<<1) | gps_time_change], cpr);
    contexts[current_context].last_intensity[(cpr<<1) | gps_time_change] = intensity;
    ((LASpoint14*)last_item)->intensity = intensity;
  }

  ////////////////////////////////////////
  // decompress scan_angle layer (if changed and requested)
  ////////////////////////////////////////

  if (changed_scan_angle) // if the scan angle should be decompressed and changes within this chunk
  {
    if (scan_angle_change) // if the scan angle has actually changed
    {
      ((LASpoint14*)last_item)->scan_angle = contexts[current_context].ic_scan_angle->decompress(((LASpoint14*)last_item)->scan_angle, gps_time_change); // if the GPS time has changed
      ((LASpoint14*)last_item)->legacy_scan_angle_rank = I8_CLAMP(I16_QUANTIZE(0.006f*((LASpoint14*)last_item)->scan_angle));
    }
  }

  ////////////////////////////////////////
  // decompress user_data layer (if changed and requested)
  ////////////////////////////////////////

  if (changed_user_data) // if the user data should be decompressed and changes within this chunk
  {
    if (contexts[current_context].m_user_data[((LASpoint14*)last_item)->user_data/4] == 0)
    {
      contexts[current_context].m_user_data[((LASpoint14*)last_item)->user_data/4] = dec_user_data->createSymbolModel(256);
      dec_user_data->initSymbolModel(contexts[current_context].m_user_data[((LASpoint14*)last_item)->user_data/4]);
    }
    ((LASpoint14*)last_item)->user_data = dec_user_data->decodeSymbol(contexts[current_context].m_user_data[((LASpoint14*)last_item)->user_data/4]);
  }

  ////////////////////////////////////////
  // decompress point_source layer (if changed and requested)
  ////////////////////////////////////////

  if (changed_point_source) // if the point source ID should be decompressed and changes within this chunk
  {
    if (point_source_change) // if the point source ID has actually changed
    {
      ((LASpoint14*)last_item)->point_source_ID = contexts[current_context].ic_point_source_ID->decompress(((LASpoint14*)last_item)->point_source_ID);
    }
  }

  ////////////////////////////////////////
  // decompress gps_time layer (if changed and requested)
  ////////////////////////////////////////

  if (changed_gps_time) // if the GPS time should be decompressed and changes within this chunk
  {
    if (gps_time_change) // if the GPS time has actually changed
    {
      read_gps_time();
      ((LASpoint14*)last_item)->gps_time = contexts[current_context].last_gpstime[contexts[current_context].last].f64;
    }
  }

  // copy the last item
  memcpy(item, last_item, sizeof(LASpoint14));
  // remember if the last point had a gps_time_change
  ((LASpoint14*)last_item)->gps_time_change = gps_time_change;
}

void LASreadItemCompressed_POINT14_v3::read_gps_time()
{
  I32 multi;
  if (contexts[current_context].last_gpstime_diff[contexts[current_context].last] == 0) // if the last integer difference was zero
  {
    multi = dec_gps_time->decodeSymbol(contexts[current_context].m_gpstime_0diff);
    if (multi == 0) // the difference can be represented with 32 bits
    {
      contexts[current_context].last_gpstime_diff[contexts[current_context].last] = contexts[current_context].ic_gpstime->decompress(0, 0);
      contexts[current_context].last_gpstime[contexts[current_context].last].i64 += contexts[current_context].last_gpstime_diff[contexts[current_context].last];
      contexts[current_context].multi_extreme_counter[contexts[current_context].last] = 0; 
    }
    else if (multi == 1) // the difference is huge
    {
      contexts[current_context].next = (contexts[current_context].next+1)&3;
      contexts[current_context].last_gpstime[contexts[current_context].next].u64 = contexts[current_context].ic_gpstime->decompress((I32)(contexts[current_context].last_gpstime[contexts[current_context].last].u64 >> 32), 8);
      contexts[current_context].last_gpstime[contexts[current_context].next].u64 = contexts[current_context].last_gpstime[contexts[current_context].next].u64 << 32;
      contexts[current_context].last_gpstime[contexts[current_context].next].u64 |= dec_gps_time->readInt();
      contexts[current_context].last = contexts[current_context].next;
      contexts[current_context].last_gpstime_diff[contexts[current_context].last] = 0;
      contexts[current_context].multi_extreme_counter[contexts[current_context].last] = 0; 
    }
    else // we switch to another sequence
    {
      contexts[current_context].last = (contexts[current_context].last+multi-1)&3;
      read_gps_time();
    }
  }
  else
  {
    multi = dec_gps_time->decodeSymbol(contexts[current_context].m_gpstime_multi);
    if (multi == 1)
    {
      contexts[current_context].last_gpstime[contexts[current_context].last].i64 += contexts[current_context].ic_gpstime->decompress(contexts[current_context].last_gpstime_diff[contexts[current_context].last], 1);;
      contexts[current_context].multi_extreme_counter[contexts[current_context].last] = 0;
    }
    else if (multi < LASZIP_GPSTIME_MULTI_CODE_FULL)
    {
      I32 gpstime_diff;
      if (multi == 0)
      {
        gpstime_diff = contexts[current_context].ic_gpstime->decompress(0, 7);
        contexts[current_context].multi_extreme_counter[contexts[current_context].last]++;
        if (contexts[current_context].multi_extreme_counter[contexts[current_context].last] > 3)
        {
          contexts[current_context].last_gpstime_diff[contexts[current_context].last] = gpstime_diff;
          contexts[current_context].multi_extreme_counter[contexts[current_context].last] = 0;
        }
      }
      else if (multi < LASZIP_GPSTIME_MULTI)
      {
        if (multi < 10)
          gpstime_diff = contexts[current_context].ic_gpstime->decompress(multi*contexts[current_context].last_gpstime_diff[contexts[current_context].last], 2);
        else
          gpstime_diff = contexts[current_context].ic_gpstime->decompress(multi*contexts[current_context].last_gpstime_diff[contexts[current_context].last], 3);
      }
      else if (multi == LASZIP_GPSTIME_MULTI)
      {
        gpstime_diff = contexts[current_context].ic_gpstime->decompress(LASZIP_GPSTIME_MULTI*contexts[current_context].last_gpstime_diff[contexts[current_context].last], 4);
        contexts[current_context].multi_extreme_counter[contexts[current_context].last]++;
        if (contexts[current_context].multi_extreme_counter[contexts[current_context].last] > 3)
        {
          contexts[current_context].last_gpstime_diff[contexts[current_context].last] = gpstime_diff;
          contexts[current_context].multi_extreme_counter[contexts[current_context].last] = 0;
        }
      }
      else
      {
        multi = LASZIP_GPSTIME_MULTI - multi;
        if (multi > LASZIP_GPSTIME_MULTI_MINUS)
        {
          gpstime_diff = contexts[current_context].ic_gpstime->decompress(multi*contexts[current_context].last_gpstime_diff[contexts[current_context].last], 5);
        }
        else
        {
          gpstime_diff = contexts[current_context].ic_gpstime->decompress(LASZIP_GPSTIME_MULTI_MINUS*contexts[current_context].last_gpstime_diff[contexts[current_context].last], 6);
          contexts[current_context].multi_extreme_counter[contexts[current_context].last]++;
          if (contexts[current_context].multi_extreme_counter[contexts[current_context].last] > 3)
          {
            contexts[current_context].last_gpstime_diff[contexts[current_context].last] = gpstime_diff;
            contexts[current_context].multi_extreme_counter[contexts[current_context].last] = 0;
          }
        }
      }
      contexts[current_context].last_gpstime[contexts[current_context].last].i64 += gpstime_diff;
    }
    else if (multi ==  LASZIP_GPSTIME_MULTI_CODE_FULL)
    {
      contexts[current_context].next = (contexts[current_context].next+1)&3;
      contexts[current_context].last_gpstime[contexts[current_context].next].u64 = contexts[current_context].ic_gpstime->decompress((I32)(contexts[current_context].last_gpstime[contexts[current_context].last].u64 >> 32), 8);
      contexts[current_context].last_gpstime[contexts[current_context].next].u64 = contexts[current_context].last_gpstime[contexts[current_context].next].u64 << 32;
      contexts[current_context].last_gpstime[contexts[current_context].next].u64 |= dec_gps_time->readInt();
      contexts[current_context].last = contexts[current_context].next;
      contexts[current_context].last_gpstime_diff[contexts[current_context].last] = 0;
      contexts[current_context].multi_extreme_counter[contexts[current_context].last] = 0; 
    }
    else if (multi >=  LASZIP_GPSTIME_MULTI_CODE_FULL)
    {
      contexts[current_context].last = (contexts[current_context].last+multi-LASZIP_GPSTIME_MULTI_CODE_FULL)&3;
      read_gps_time();
    }
  }
}

/*
===============================================================================
                       LASreadItemCompressed_RGB14_v3
===============================================================================
*/

LASreadItemCompressed_RGB14_v3::LASreadItemCompressed_RGB14_v3(ArithmeticDecoder* dec, const U32 decompress_selective)
{
  /* not used as a decoder. just gives access to instream */

  assert(dec);
  this->dec = dec;

  /* zero instreams and decoders */

  instream_RGB = 0;

  dec_RGB = 0;

  /* zero num_bytes and init booleans */

  num_bytes_RGB = 0;

  changed_RGB = FALSE;

  requested_RGB = (decompress_selective & LASZIP_DECOMPRESS_SELECTIVE_RGB ? TRUE : FALSE);

  /* init the bytes buffer to zero */

  bytes = 0;
  num_bytes_allocated = 0;

  /* mark the four scanner channel contexts as uninitialized */

  U32 c;
  for (c = 0; c < 4; c++)
  {
    contexts[c].m_byte_used = 0;
  }
  current_context = 0;
}

LASreadItemCompressed_RGB14_v3::~LASreadItemCompressed_RGB14_v3()
{
  /* destroy all initialized scanner channel contexts */

  U32 c;
  for (c = 0; c < 4; c++)
  {
    if (contexts[c].m_byte_used)
    {
      dec_RGB->destroySymbolModel(contexts[c].m_byte_used);
      dec_RGB->destroySymbolModel(contexts[c].m_rgb_diff_0);
      dec_RGB->destroySymbolModel(contexts[c].m_rgb_diff_1);
      dec_RGB->destroySymbolModel(contexts[c].m_rgb_diff_2);
      dec_RGB->destroySymbolModel(contexts[c].m_rgb_diff_3);
      dec_RGB->destroySymbolModel(contexts[c].m_rgb_diff_4);
      dec_RGB->destroySymbolModel(contexts[c].m_rgb_diff_5);
    }
  }

  /* destroy all instreams and decoders */

  if (instream_RGB)
  {
    delete instream_RGB;

    delete dec_RGB;
  }

  if (bytes) delete [] bytes;
}

inline BOOL LASreadItemCompressed_RGB14_v3::createAndInitModelsAndDecompressors(U32 context, const U8* item)
{
  /* should only be called when context is unused */

  assert(contexts[context].unused);

  /* first create all entropy models (if needed) */

  if (contexts[context].m_byte_used == 0)
  {
    contexts[context].m_byte_used = dec_RGB->createSymbolModel(128);
    contexts[context].m_rgb_diff_0 = dec_RGB->createSymbolModel(256);
    contexts[context].m_rgb_diff_1 = dec_RGB->createSymbolModel(256);
    contexts[context].m_rgb_diff_2 = dec_RGB->createSymbolModel(256);
    contexts[context].m_rgb_diff_3 = dec_RGB->createSymbolModel(256);
    contexts[context].m_rgb_diff_4 = dec_RGB->createSymbolModel(256);
    contexts[context].m_rgb_diff_5 = dec_RGB->createSymbolModel(256);
  }

  /* then init entropy models */

  dec_RGB->initSymbolModel(contexts[context].m_byte_used);
  dec_RGB->initSymbolModel(contexts[context].m_rgb_diff_0);
  dec_RGB->initSymbolModel(contexts[context].m_rgb_diff_1);
  dec_RGB->initSymbolModel(contexts[context].m_rgb_diff_2);
  dec_RGB->initSymbolModel(contexts[context].m_rgb_diff_3);
  dec_RGB->initSymbolModel(contexts[context].m_rgb_diff_4);
  dec_RGB->initSymbolModel(contexts[context].m_rgb_diff_5);

  /* init current context from item */

  memcpy(contexts[context].last_item, item, 6);

  contexts[context].unused = FALSE;

  return TRUE;
}

BOOL LASreadItemCompressed_RGB14_v3::chunk_sizes()
{
  /* for layered compression 'dec' only hands over the stream */

  ByteStreamIn* instream = dec->getByteStreamIn();

  /* read bytes per layer */

  instream->get32bitsLE(((U8*)&num_bytes_RGB));

  return TRUE;
}

BOOL LASreadItemCompressed_RGB14_v3::init(const U8* item, U32& context)
{
  /* for layered compression 'dec' only hands over the stream */

  ByteStreamIn* instream = dec->getByteStreamIn();

  /* on the first init create instreams and decoders */

  if (instream_RGB == 0)
  {
    /* create instreams */

    if (IS_LITTLE_ENDIAN())
    {
      instream_RGB = new ByteStreamInArrayLE();
    }
    else
    {
      instream_RGB = new ByteStreamInArrayBE();
    }

    /* create decoders */

    dec_RGB = new ArithmeticDecoder();
  }
  
  /* make sure the buffer is sufficiently large */

  if (num_bytes_RGB > num_bytes_allocated)
  {
    if (bytes) delete [] bytes;
    bytes = new U8[num_bytes_RGB];
    if (bytes == 0) return FALSE;
    num_bytes_allocated = num_bytes_RGB;
  }

  /* load the requested bytes and init the corresponding instreams an decoders */

  if (requested_RGB)
  {
    if (num_bytes_RGB)
    {
      instream->getBytes(bytes, num_bytes_RGB);
      instream_RGB->init(bytes, num_bytes_RGB);
      dec_RGB->init(instream_RGB);
      changed_RGB = TRUE;
    }
    else
    {
      instream_RGB->init(0, 0);
      changed_RGB = FALSE;
    }
  }
  else
  {
    if (num_bytes_RGB)
    {
      instream->skipBytes(num_bytes_RGB);
    }
    changed_RGB = FALSE;
  }

  /* mark the four scanner channel contexts as unused */

  U32 c;
  for (c = 0; c < 4; c++)
  {
    contexts[c].unused = TRUE;
  }

  /* set scanner channel as current context */

  current_context = context; // all other items use context set by POINT14 reader

  /* create and init models and decompressors */

  createAndInitModelsAndDecompressors(current_context, item);

  return TRUE;
}

inline void LASreadItemCompressed_RGB14_v3::read(U8* item, U32& context)
{
  // get last

  U16* last_item = contexts[current_context].last_item;

  // check for context switch

  if (current_context != context)
  {
    current_context = context; // all other items use context set by POINT14 reader
    if (contexts[current_context].unused)
    {
      createAndInitModelsAndDecompressors(current_context, (U8*)last_item);
      last_item = contexts[current_context].last_item;
    }
  }

  // decompress

  if (changed_RGB)
  {
    U8 corr;
    I32 diff = 0;
    U32 sym = dec_RGB->decodeSymbol(contexts[current_context].m_byte_used);
    if (sym & (1 << 0))
    {
      corr = dec_RGB->decodeSymbol(contexts[current_context].m_rgb_diff_0);
      ((U16*)item)[0] = (U16)U8_FOLD(corr + (last_item[0]&255));
    }
    else 
    {
      ((U16*)item)[0] = last_item[0]&0xFF;
    }
    if (sym & (1 << 1))
    {
      corr = dec_RGB->decodeSymbol(contexts[current_context].m_rgb_diff_1);
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
        corr = dec_RGB->decodeSymbol(contexts[current_context].m_rgb_diff_2);
        ((U16*)item)[1] = (U16)U8_FOLD(corr + U8_CLAMP(diff+(last_item[1]&255)));
      }
      else
      {
        ((U16*)item)[1] = last_item[1]&0xFF;
      }
      if (sym & (1 << 4))
      {
        corr = dec_RGB->decodeSymbol(contexts[current_context].m_rgb_diff_4);
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
        corr = dec_RGB->decodeSymbol(contexts[current_context].m_rgb_diff_3);
        ((U16*)item)[1] |= (((U16)U8_FOLD(corr + U8_CLAMP(diff+(last_item[1]>>8))))<<8);
      }
      else
      {
        ((U16*)item)[1] |= (last_item[1]&0xFF00);
      }
      if (sym & (1 << 5))
      {
        corr = dec_RGB->decodeSymbol(contexts[current_context].m_rgb_diff_5);
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
  else
  {
    memcpy(item, last_item, 6);
  }
}

/*
===============================================================================
                    LASreadItemCompressed_RGBNIR14_v3
===============================================================================
*/

LASreadItemCompressed_RGBNIR14_v3::LASreadItemCompressed_RGBNIR14_v3(ArithmeticDecoder* dec, const U32 decompress_selective)
{
  /* not used as a decoder. just gives access to instream */

  assert(dec);
  this->dec = dec;

  /* zero instreams and decoders */

  instream_RGB = 0;
  instream_NIR = 0;

  dec_RGB = 0;
  dec_NIR = 0;

  /* zero num_bytes and init booleans */

  num_bytes_RGB = 0;
  num_bytes_NIR = 0;

  changed_RGB = FALSE;
  changed_NIR = FALSE;

  requested_RGB = (decompress_selective & LASZIP_DECOMPRESS_SELECTIVE_RGB ? TRUE : FALSE);
  requested_NIR = (decompress_selective & LASZIP_DECOMPRESS_SELECTIVE_NIR ? TRUE : FALSE);

  /* init the bytes buffer to zero */

  bytes = 0;
  num_bytes_allocated = 0;

  /* mark the four scanner channel contexts as uninitialized */

  U32 c;
  for (c = 0; c < 4; c++)
  {
    contexts[c].m_rgb_bytes_used = 0;
    contexts[c].m_nir_bytes_used = 0;
  }
  current_context = 0;
}

LASreadItemCompressed_RGBNIR14_v3::~LASreadItemCompressed_RGBNIR14_v3()
{
  /* destroy all initialized scanner channel contexts */

  U32 c;
  for (c = 0; c < 4; c++)
  {
    if (contexts[c].m_rgb_bytes_used)
    {
      dec_RGB->destroySymbolModel(contexts[c].m_rgb_bytes_used);
      dec_RGB->destroySymbolModel(contexts[c].m_rgb_diff_0);
      dec_RGB->destroySymbolModel(contexts[c].m_rgb_diff_1);
      dec_RGB->destroySymbolModel(contexts[c].m_rgb_diff_2);
      dec_RGB->destroySymbolModel(contexts[c].m_rgb_diff_3);
      dec_RGB->destroySymbolModel(contexts[c].m_rgb_diff_4);
      dec_RGB->destroySymbolModel(contexts[c].m_rgb_diff_5);
    }

    if (contexts[c].m_nir_bytes_used)
    {
      dec_NIR->destroySymbolModel(contexts[c].m_nir_bytes_used);
      dec_NIR->destroySymbolModel(contexts[c].m_nir_diff_0);
      dec_NIR->destroySymbolModel(contexts[c].m_nir_diff_1);
    }
  }

  /* destroy all instreams and decoders */

  if (instream_RGB)
  {
    delete instream_RGB;

    delete dec_RGB;
  }

  if (instream_NIR)
  {
    delete instream_NIR;

    delete dec_NIR;
  }

  if (bytes) delete [] bytes;
}

inline BOOL LASreadItemCompressed_RGBNIR14_v3::createAndInitModelsAndDecompressors(U32 context, const U8* item)
{
  /* should only be called when context is unused */

  assert(contexts[context].unused);

  /* first create all entropy models (if needed) */

  if (requested_RGB)
  {
    if (contexts[context].m_rgb_bytes_used == 0)
    {
      contexts[context].m_rgb_bytes_used = dec_RGB->createSymbolModel(128);
      contexts[context].m_rgb_diff_0 = dec_RGB->createSymbolModel(256);
      contexts[context].m_rgb_diff_1 = dec_RGB->createSymbolModel(256);
      contexts[context].m_rgb_diff_2 = dec_RGB->createSymbolModel(256);
      contexts[context].m_rgb_diff_3 = dec_RGB->createSymbolModel(256);
      contexts[context].m_rgb_diff_4 = dec_RGB->createSymbolModel(256);
      contexts[context].m_rgb_diff_5 = dec_RGB->createSymbolModel(256);
    }

    /* then init entropy models */

    dec_RGB->initSymbolModel(contexts[context].m_rgb_bytes_used);
    dec_RGB->initSymbolModel(contexts[context].m_rgb_diff_0);
    dec_RGB->initSymbolModel(contexts[context].m_rgb_diff_1);
    dec_RGB->initSymbolModel(contexts[context].m_rgb_diff_2);
    dec_RGB->initSymbolModel(contexts[context].m_rgb_diff_3);
    dec_RGB->initSymbolModel(contexts[context].m_rgb_diff_4);
    dec_RGB->initSymbolModel(contexts[context].m_rgb_diff_5);
  }

  if (requested_NIR)
  {
    if (contexts[context].m_nir_bytes_used == 0)
    {
      contexts[context].m_nir_bytes_used = dec_NIR->createSymbolModel(4);
      contexts[context].m_nir_diff_0 = dec_NIR->createSymbolModel(256);
      contexts[context].m_nir_diff_1 = dec_NIR->createSymbolModel(256);
    }

    /* then init entropy models */

    dec_NIR->initSymbolModel(contexts[context].m_nir_bytes_used);
    dec_NIR->initSymbolModel(contexts[context].m_nir_diff_0);
    dec_NIR->initSymbolModel(contexts[context].m_nir_diff_1);
  }

  /* init current context from item */

  memcpy(contexts[context].last_item, item, 8);

  contexts[context].unused = FALSE;

  return TRUE;
}

BOOL LASreadItemCompressed_RGBNIR14_v3::chunk_sizes()
{
  /* for layered compression 'dec' only hands over the stream */

  ByteStreamIn* instream = dec->getByteStreamIn();

  /* read bytes per layer */

  instream->get32bitsLE(((U8*)&num_bytes_RGB));
  instream->get32bitsLE(((U8*)&num_bytes_NIR));

  return TRUE;
}

BOOL LASreadItemCompressed_RGBNIR14_v3::init(const U8* item, U32& context)
{
  /* for layered compression 'dec' only hands over the stream */

  ByteStreamIn* instream = dec->getByteStreamIn();

  /* on the first init create instreams and decoders */

  if (instream_RGB == 0)
  {
    /* create instreams */

    if (IS_LITTLE_ENDIAN())
    {
      instream_RGB = new ByteStreamInArrayLE();
      instream_NIR = new ByteStreamInArrayLE();
    }
    else
    {
      instream_RGB = new ByteStreamInArrayBE();
      instream_NIR = new ByteStreamInArrayBE();
    }

    /* create decoders */

    dec_RGB = new ArithmeticDecoder();
    dec_NIR = new ArithmeticDecoder();
  }
  
  /* how many bytes do we need to read */

  U32 num_bytes = 0;
  if (requested_RGB) num_bytes += num_bytes_RGB;
  if (requested_NIR) num_bytes += num_bytes_NIR;

  /* make sure the buffer is sufficiently large */

  if (num_bytes > num_bytes_allocated)
  {
    if (bytes) delete [] bytes;
    bytes = new U8[num_bytes];
    if (bytes == 0) return FALSE;
    num_bytes_allocated = num_bytes;
  }

  /* load the requested bytes and init the corresponding instreams an decoders */

  num_bytes = 0;

  if (requested_RGB)
  {
    if (num_bytes_RGB)
    {
      instream->getBytes(bytes, num_bytes_RGB);
      num_bytes += num_bytes_RGB;
      instream_RGB->init(bytes, num_bytes_RGB);
      dec_RGB->init(instream_RGB);
      changed_RGB = TRUE;
    }
    else
    {
      instream_RGB->init(0, 0);
      changed_RGB = FALSE;
    }
  }
  else
  {
    if (num_bytes_RGB)
    {
      instream->skipBytes(num_bytes_RGB);
    }
    changed_RGB = FALSE;
  }

  if (requested_NIR)
  {
    if (num_bytes_NIR)
    {
      instream->getBytes(&bytes[num_bytes], num_bytes_NIR);
      instream_NIR->init(&bytes[num_bytes], num_bytes_NIR);
      dec_NIR->init(instream_NIR);
      changed_NIR = TRUE;
    }
    else
    {
      instream_NIR->init(0, 0);
      changed_NIR = FALSE;
    }
  }
  else
  {
    if (num_bytes_NIR)
    {
      instream->skipBytes(num_bytes_NIR);
    }
    changed_NIR = FALSE;
  }

  /* mark the four scanner channel contexts as unused */

  U32 c;
  for (c = 0; c < 4; c++)
  {
    contexts[c].unused = TRUE;
  }

  /* set scanner channel as current context */

  current_context = context; // all other items use context set by POINT14 reader

  /* create and init models and decompressors */

  createAndInitModelsAndDecompressors(current_context, item);

  return TRUE;
}

inline void LASreadItemCompressed_RGBNIR14_v3::read(U8* item, U32& context)
{
  // get last

  U16* last_item = contexts[current_context].last_item;

  // check for context switch

  if (current_context != context)
  {
    current_context = context; // all other items use context set by POINT14 reader
    if (contexts[current_context].unused)
    {
      createAndInitModelsAndDecompressors(current_context, (U8*)last_item);
      last_item = contexts[current_context].last_item;
    }
  }

  // dempress

  ////////////////////////////////////////
  // decompress RGB layer 
  ////////////////////////////////////////

  if (changed_RGB)
  {
    U8 corr;
    I32 diff = 0;
    U32 sym = dec_RGB->decodeSymbol(contexts[current_context].m_rgb_bytes_used);
    if (sym & (1 << 0))
    {
      corr = dec_RGB->decodeSymbol(contexts[current_context].m_rgb_diff_0);
      ((U16*)item)[0] = (U16)U8_FOLD(corr + (last_item[0]&255));
    }
    else 
    {
      ((U16*)item)[0] = last_item[0]&0xFF;
    }
    if (sym & (1 << 1))
    {
      corr = dec_RGB->decodeSymbol(contexts[current_context].m_rgb_diff_1);
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
        corr = dec_RGB->decodeSymbol(contexts[current_context].m_rgb_diff_2);
        ((U16*)item)[1] = (U16)U8_FOLD(corr + U8_CLAMP(diff+(last_item[1]&255)));
      }
      else
      {
        ((U16*)item)[1] = last_item[1]&0xFF;
      }
      if (sym & (1 << 4))
      {
        corr = dec_RGB->decodeSymbol(contexts[current_context].m_rgb_diff_4);
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
        corr = dec_RGB->decodeSymbol(contexts[current_context].m_rgb_diff_3);
        ((U16*)item)[1] |= (((U16)U8_FOLD(corr + U8_CLAMP(diff+(last_item[1]>>8))))<<8);
      }
      else
      {
        ((U16*)item)[1] |= (last_item[1]&0xFF00);
      }
      if (sym & (1 << 5))
      {
        corr = dec_RGB->decodeSymbol(contexts[current_context].m_rgb_diff_5);
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
  else
  {
    memcpy(item, last_item, 6);
  }

  ////////////////////////////////////////
  // decompress NIR layer 
  ////////////////////////////////////////

  if (changed_NIR)
  {
    U8 corr;
    U32 sym = dec_NIR->decodeSymbol(contexts[current_context].m_nir_bytes_used);
    if (sym & (1 << 0))
    {
      corr = dec_NIR->decodeSymbol(contexts[current_context].m_nir_diff_0);
      ((U16*)item)[3] = (U16)U8_FOLD(corr + (last_item[3]&255));
    }
    else 
    {
      ((U16*)item)[3] = last_item[3]&0xFF;
    }
    if (sym & (1 << 1))
    {
      corr = dec_NIR->decodeSymbol(contexts[current_context].m_nir_diff_1);
      ((U16*)item)[3] |= (((U16)U8_FOLD(corr + (last_item[3]>>8))) << 8);
    }
    else
    {
      ((U16*)item)[3] |= (last_item[3]&0xFF00);
    }
    contexts[current_context].last_item[3] = ((U16*)item)[3];
  }
  else
  {
    ((U16*)item)[3] = contexts[current_context].last_item[3];
  }
}

/*
===============================================================================
                       LASreadItemCompressed_WAVEPACKET14_v3
===============================================================================
*/

LASreadItemCompressed_WAVEPACKET14_v3::LASreadItemCompressed_WAVEPACKET14_v3(ArithmeticDecoder* dec, const U32 decompress_selective)
{
  /* not used as a decoder. just gives access to instream */

  assert(dec);
  this->dec = dec;

  /* zero instreams and decoders */

  instream_wavepacket = 0;

  dec_wavepacket = 0;

  /* zero num_bytes and init booleans */

  num_bytes_wavepacket = 0;

  changed_wavepacket = FALSE;

  requested_wavepacket = (decompress_selective & LASZIP_DECOMPRESS_SELECTIVE_WAVEPACKET ? TRUE : FALSE);

  /* init the bytes buffer to zero */

  bytes = 0;
  num_bytes_allocated = 0;

  /* mark the four scanner channel contexts as uninitialized */

  U32 c;
  for (c = 0; c < 4; c++)
  {
    contexts[c].m_packet_index = 0;
  }
  current_context = 0;
}

LASreadItemCompressed_WAVEPACKET14_v3::~LASreadItemCompressed_WAVEPACKET14_v3()
{
  /* destroy all initialized scanner channel contexts */

  U32 c;
  for (c = 0; c < 4; c++)
  {
    if (contexts[c].m_packet_index)
    {
      dec_wavepacket->destroySymbolModel(contexts[c].m_packet_index);
      dec_wavepacket->destroySymbolModel(contexts[c].m_offset_diff[0]);
      dec_wavepacket->destroySymbolModel(contexts[c].m_offset_diff[1]);
      dec_wavepacket->destroySymbolModel(contexts[c].m_offset_diff[2]);
      dec_wavepacket->destroySymbolModel(contexts[c].m_offset_diff[3]);
      delete contexts[c].ic_offset_diff;
      delete contexts[c].ic_packet_size;
      delete contexts[c].ic_return_point;
      delete contexts[c].ic_xyz;
    }
  }

  /* destroy all instreams and decoders */

  if (instream_wavepacket)
  {
    delete instream_wavepacket;

    delete dec_wavepacket;
  }

  if (bytes) delete [] bytes;
}

inline BOOL LASreadItemCompressed_WAVEPACKET14_v3::createAndInitModelsAndDecompressors(U32 context, const U8* item)
{
  /* should only be called when context is unused */

  assert(contexts[context].unused);

  /* first create all entropy models (if needed) */

  if (requested_wavepacket)
  {
    if (contexts[context].m_packet_index == 0)
    {
      contexts[context].m_packet_index = dec_wavepacket->createSymbolModel(256);
      contexts[context].m_offset_diff[0] = dec_wavepacket->createSymbolModel(4);
      contexts[context].m_offset_diff[1] = dec_wavepacket->createSymbolModel(4);
      contexts[context].m_offset_diff[2] = dec_wavepacket->createSymbolModel(4);
      contexts[context].m_offset_diff[3] = dec_wavepacket->createSymbolModel(4);
      contexts[context].ic_offset_diff = new IntegerCompressor(dec_wavepacket, 32);
      contexts[context].ic_packet_size = new IntegerCompressor(dec_wavepacket, 32);
      contexts[context].ic_return_point = new IntegerCompressor(dec_wavepacket, 32);
      contexts[context].ic_xyz = new IntegerCompressor(dec_wavepacket, 32, 3);
    }

    /* then init entropy models */

    dec_wavepacket->initSymbolModel(contexts[context].m_packet_index);
    dec_wavepacket->initSymbolModel(contexts[context].m_offset_diff[0]);
    dec_wavepacket->initSymbolModel(contexts[context].m_offset_diff[1]);
    dec_wavepacket->initSymbolModel(contexts[context].m_offset_diff[2]);
    dec_wavepacket->initSymbolModel(contexts[context].m_offset_diff[3]);
    contexts[context].ic_offset_diff->initDecompressor();
    contexts[context].ic_packet_size->initDecompressor();
    contexts[context].ic_return_point->initDecompressor();
    contexts[context].ic_xyz->initDecompressor();
  }

  /* init current context from item */

  contexts[context].last_diff_32 = 0;
  contexts[context].sym_last_offset_diff = 0;
  memcpy(contexts[context].last_item, item, 29);

  contexts[context].unused = FALSE;

  return TRUE;
}

BOOL LASreadItemCompressed_WAVEPACKET14_v3::chunk_sizes()
{
  /* for layered compression 'dec' only hands over the stream */

  ByteStreamIn* instream = dec->getByteStreamIn();

  /* read bytes per layer */

  instream->get32bitsLE(((U8*)&num_bytes_wavepacket));

  return TRUE;
}

BOOL LASreadItemCompressed_WAVEPACKET14_v3::init(const U8* item, U32& context)
{
  /* for layered compression 'dec' only hands over the stream */

  ByteStreamIn* instream = dec->getByteStreamIn();

  /* on the first init create instreams and decoders */

  if (instream_wavepacket == 0)
  {
    /* create instreams */

    if (IS_LITTLE_ENDIAN())
    {
      instream_wavepacket = new ByteStreamInArrayLE();
    }
    else
    {
      instream_wavepacket = new ByteStreamInArrayBE();
    }

    /* create decoders */

    dec_wavepacket = new ArithmeticDecoder();
  }
  
  /* make sure the buffer is sufficiently large */

  if (num_bytes_wavepacket > num_bytes_allocated)
  {
    if (bytes) delete [] bytes;
    bytes = new U8[num_bytes_wavepacket];
    if (bytes == 0) return FALSE;
    num_bytes_allocated = num_bytes_wavepacket;
  }

  /* load the requested bytes and init the corresponding instreams an decoders */

  if (requested_wavepacket)
  {
    if (num_bytes_wavepacket)
    {
      instream->getBytes(bytes, num_bytes_wavepacket);
      instream_wavepacket->init(bytes, num_bytes_wavepacket);
      dec_wavepacket->init(instream_wavepacket);
      changed_wavepacket = TRUE;
    }
    else
    {
      instream_wavepacket->init(0, 0);
      changed_wavepacket = FALSE;
    }
  }
  else
  {
    if (num_bytes_wavepacket)
    {
      instream->skipBytes(num_bytes_wavepacket);
    }
    changed_wavepacket = FALSE;
  }

  /* mark the four scanner channel contexts as unused */

  U32 c;
  for (c = 0; c < 4; c++)
  {
    contexts[c].unused = TRUE;
  }

  /* set scanner channel as current context */

  current_context = context; // all other items use context set by POINT14 reader

  /* create and init models and decompressors */

  createAndInitModelsAndDecompressors(current_context, item);

  return TRUE;
}

inline void LASreadItemCompressed_WAVEPACKET14_v3::read(U8* item, U32& context)
{
  // get last

  U8* last_item = contexts[current_context].last_item;

  // check for context switch

  if (current_context != context)
  {
    current_context = context; // all other items use context set by POINT14 reader
    if (contexts[current_context].unused)
    {
      createAndInitModelsAndDecompressors(current_context, last_item);
      last_item = contexts[current_context].last_item;
    }
  }

  // decompress

  if (changed_wavepacket)
  {
    item[0] = (U8)(dec_wavepacket->decodeSymbol(contexts[current_context].m_packet_index));

    LASwavepacket13 this_item_m;
    LASwavepacket13 last_item_m = LASwavepacket13::unpack(last_item+1);

    contexts[current_context].sym_last_offset_diff = dec_wavepacket->decodeSymbol(contexts[current_context].m_offset_diff[contexts[current_context].sym_last_offset_diff]);

    if (contexts[current_context].sym_last_offset_diff == 0)
    {
      this_item_m.offset = last_item_m.offset;
    }
    else if (contexts[current_context].sym_last_offset_diff == 1)
    {
      this_item_m.offset = last_item_m.offset + last_item_m.packet_size;
    }
    else if (contexts[current_context].sym_last_offset_diff == 2)
    {
      contexts[current_context].last_diff_32 = contexts[current_context].ic_offset_diff->decompress(contexts[current_context].last_diff_32);
      this_item_m.offset = last_item_m.offset + contexts[current_context].last_diff_32;
    }
    else
    {
      this_item_m.offset = dec_wavepacket->readInt64();
    }

    this_item_m.packet_size = contexts[current_context].ic_packet_size->decompress(last_item_m.packet_size);
    this_item_m.return_point.i32 = contexts[current_context].ic_return_point->decompress(last_item_m.return_point.i32);
    this_item_m.x.i32 = contexts[current_context].ic_xyz->decompress(last_item_m.x.i32, 0);
    this_item_m.y.i32 = contexts[current_context].ic_xyz->decompress(last_item_m.y.i32, 1);
    this_item_m.z.i32 = contexts[current_context].ic_xyz->decompress(last_item_m.z.i32, 2);

    this_item_m.pack(item+1);

    memcpy(last_item, item, 29);
  }
}

/*
===============================================================================
                       LASreadItemCompressed_BYTE14_v3
===============================================================================
*/

LASreadItemCompressed_BYTE14_v3::LASreadItemCompressed_BYTE14_v3(ArithmeticDecoder* dec, U32 number, const U32 decompress_selective)
{
  /* not used as a decoder. just gives access to instream */

  assert(dec);
  this->dec = dec;

  /* must be more than one byte */

  assert(number);
  this->number = number;

  /* zero instream and decoder pointer arrays */

  instream_Bytes = 0;

  dec_Bytes = 0;

  /* create and init num_bytes and booleans arrays */

  num_bytes_Bytes = new U32[number];

  changed_Bytes = new BOOL[number];

  requested_Bytes = new BOOL[number];

  U32 i;
  for (i = 0; i < number; i++)
  {
    num_bytes_Bytes[i] = 0;

    changed_Bytes[i] = FALSE;

    requested_Bytes[i] = (decompress_selective & (LASZIP_DECOMPRESS_SELECTIVE_BYTE0 << i) ? TRUE : FALSE);
  }

  /* init the bytes buffer to zero */

  bytes = 0;
  num_bytes_allocated = 0;

  /* mark the four scanner channel contexts as uninitialized */

  U32 c;
  for (c = 0; c < 4; c++)
  {
    contexts[c].m_bytes = 0;
  }
  current_context = 0;
}

LASreadItemCompressed_BYTE14_v3::~LASreadItemCompressed_BYTE14_v3()
{
  /* destroy all initialized scanner channel contexts */

  U32 c, i;
  for (c = 0; c < 4; c++)
  {
    if (contexts[c].m_bytes)
    {
      for (i = 0; i < number; i++)
      {
        dec_Bytes[i]->destroySymbolModel(contexts[c].m_bytes[i]);
      }
      delete [] contexts[c].m_bytes;
      delete [] contexts[c].last_item;
    }
  }

  /* destroy all instream and decoder arrays */

  if (instream_Bytes)
  {
    for (i = 0; i < number; i++)
    {
      if (instream_Bytes[i])
      {
        delete instream_Bytes[i];
        delete dec_Bytes[i];
      }
    }

    delete [] instream_Bytes;
    delete [] dec_Bytes;
  }

  /* destroy all other arrays */

  if (num_bytes_Bytes) delete [] num_bytes_Bytes;

  if (changed_Bytes) delete [] changed_Bytes;

  if (requested_Bytes) delete [] requested_Bytes;

  if (bytes) delete [] bytes;
}

inline BOOL LASreadItemCompressed_BYTE14_v3::createAndInitModelsAndDecompressors(U32 context, const U8* item)
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
      contexts[context].m_bytes[i] = dec_Bytes[i]->createSymbolModel(256);
      dec_Bytes[i]->initSymbolModel(contexts[context].m_bytes[i]);
    }

    /* create last item */
    contexts[context].last_item = new U8[number];
  }

  /* then init entropy models */

  for (i = 0; i < number; i++)
  {
    dec_Bytes[i]->initSymbolModel(contexts[context].m_bytes[i]);
  }

  /* init current context from item */

  memcpy(contexts[context].last_item, item, number);

  contexts[context].unused = FALSE;

  return TRUE;
}

BOOL LASreadItemCompressed_BYTE14_v3::chunk_sizes()
{
  U32 i;

  /* for layered compression 'dec' only hands over the stream */

  ByteStreamIn* instream = dec->getByteStreamIn();

  for (i = 0; i < number; i++)
  {
    /* read bytes per layer */

    instream->get32bitsLE(((U8*)&(num_bytes_Bytes[i])));
  }

  return TRUE;
}

BOOL LASreadItemCompressed_BYTE14_v3::init(const U8* item, U32& context)
{
  U32 i;

  /* for layered compression 'dec' only hands over the stream */

  ByteStreamIn* instream = dec->getByteStreamIn();

  /* on the first init create instreams and decoders */

  if (instream_Bytes == 0)
  {
    /* create instream pointer array */

    instream_Bytes = new ByteStreamInArray*[number];

    /* create instreams */

    if (IS_LITTLE_ENDIAN())
    {
      for (i = 0; i < number; i++)
      {
        instream_Bytes[i] = new ByteStreamInArrayLE();
      }
    }
    else
    {
      for (i = 0; i < number; i++)
      {
        instream_Bytes[i] = new ByteStreamInArrayBE();
      }
    }

    /* create decoder pointer array */

    dec_Bytes = new ArithmeticDecoder*[number];

    /* create layer decoders */

    for (i = 0; i < number; i++)
    {
      dec_Bytes[i] = new ArithmeticDecoder();
    }
  }

  /* how many bytes do we need to read */

  U32 num_bytes = 0;

  for (i = 0; i < number; i++)
  {
    if (requested_Bytes[i]) num_bytes += num_bytes_Bytes[i];
  }

  /* make sure the buffer is sufficiently large */

  if (num_bytes > num_bytes_allocated)
  {
    if (bytes) delete [] bytes;
    bytes = new U8[num_bytes];
    if (bytes == 0) return FALSE;
    num_bytes_allocated = num_bytes;
  }

  /* load the requested bytes and init the corresponding instreams an decoders */

  num_bytes = 0;
  for (i = 0; i < number; i++)
  {
    if (requested_Bytes[i])
    {
      if (num_bytes_Bytes[i])
      {
        instream->getBytes(&(bytes[num_bytes]), num_bytes_Bytes[i]);
        instream_Bytes[i]->init(&(bytes[num_bytes]), num_bytes_Bytes[i]);
        dec_Bytes[i]->init(instream_Bytes[i]);
        num_bytes += num_bytes_Bytes[i];
        changed_Bytes[i] = TRUE;
      }
      else
      {
        dec_Bytes[i]->init(0, 0);
        changed_Bytes[i] = FALSE;
      }
    }
    else
    {
      if (num_bytes_Bytes[i])
      {
        instream->skipBytes(num_bytes_Bytes[i]);
      }
      changed_Bytes[i] = FALSE;
    }
  }

  /* mark the four scanner channel contexts as unused */

  U32 c;
  for (c = 0; c < 4; c++)
  {
    contexts[c].unused = TRUE;
  }

  /* set scanner channel as current context */

  current_context = context; // all other items use context set by POINT14 reader

  /* create and init models and decompressors */

  createAndInitModelsAndDecompressors(current_context, item);

  return TRUE;
}

inline void LASreadItemCompressed_BYTE14_v3::read(U8* item, U32& context)
{
  // get last

  U8* last_item = contexts[current_context].last_item;

  // check for context switch

  if (current_context != context)
  {
    current_context = context; // all other items use context set by POINT14 reader
    if (contexts[current_context].unused)
    {
      createAndInitModelsAndDecompressors(current_context, (U8*)last_item);
      last_item = contexts[current_context].last_item;
    }
  }

  // decompress

  U32 i;
  for (i = 0; i < number; i++)
  {
    if (changed_Bytes[i])
    {
      I32 value = last_item[i] + dec_Bytes[i]->decodeSymbol(contexts[current_context].m_bytes[i]);
      item[i] = U8_FOLD(value);
      last_item[i] = item[i];
    }
    else
    {
      item[i] = last_item[i];
    }
  }
}
