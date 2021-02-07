/*
===============================================================================

  FILE:  lasreadpoint.cpp
  
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

#include "lasreadpoint.hpp"

#include "arithmeticdecoder.hpp"
#include "lasreaditemraw.hpp"
#include "lasreaditemcompressed_v1.hpp"
#include "lasreaditemcompressed_v2.hpp"
#include "lasreaditemcompressed_v3.hpp"
#include "lasreaditemcompressed_v4.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LASreadPoint::LASreadPoint(U32 decompress_selective)
{
  point_size = 0;
  instream = 0;
  num_readers = 0;
  readers = 0;
  readers_raw = 0;
  readers_compressed = 0;
  dec = 0;
  layered_las14_compression = FALSE;
  // used for chunking
  chunk_size = U32_MAX;
  chunk_count = 0;
  current_chunk = 0;
  number_chunks = 0;
  tabled_chunks = 0;
  chunk_totals = 0;
  chunk_starts = 0;
  // used for selective decompression (new LAS 1.4 point types only)
  this->decompress_selective = decompress_selective;
  // used for seeking
  point_start = 0;
  seek_point = 0;
  // used for error and warning reporting
  last_error = 0;
  last_warning = 0;
}

BOOL LASreadPoint::setup(U32 num_items, const LASitem* items, const LASzip* laszip)
{
  U32 i;

  // is laszip exists then we must use its items
  if (laszip)
  {
    if (num_items == 0) return FALSE;
    if (items == 0) return FALSE;
    if (num_items != laszip->num_items) return FALSE;
    if (items != laszip->items) return FALSE;
  }

  // delete old entropy decoder
  if (dec)
  {
    delete dec;
    dec = 0;
    layered_las14_compression = FALSE;
  }

  // is the content compressed?
  if (laszip && laszip->compressor)
  {
    // create new entropy decoder (if requested)
    switch (laszip->coder)
    {
    case LASZIP_CODER_ARITHMETIC:
      dec = new ArithmeticDecoder();
      break;
    default:
      // entropy decoder not supported
      return FALSE;
    }
    // maybe layered compression for LAS 1.4 
    layered_las14_compression = (laszip->compressor == LASZIP_COMPRESSOR_LAYERED_CHUNKED);
  }
 
  // initizalize the readers
  readers = 0;
  num_readers = num_items;

  // disable chunking
  chunk_size = U32_MAX;

  // always create the raw readers
  readers_raw = new LASreadItem*[num_readers];
  for (i = 0; i < num_readers; i++)
  {
    switch (items[i].type)
    {
    case LASitem::POINT10:
      if (IS_LITTLE_ENDIAN())
        readers_raw[i] = new LASreadItemRaw_POINT10_LE();
      else
        readers_raw[i] = new LASreadItemRaw_POINT10_BE();
      break;
    case LASitem::GPSTIME11:
      if (IS_LITTLE_ENDIAN())
        readers_raw[i] = new LASreadItemRaw_GPSTIME11_LE();
      else
        readers_raw[i] = new LASreadItemRaw_GPSTIME11_BE();
       break;
    case LASitem::RGB12:
    case LASitem::RGB14:
      if (IS_LITTLE_ENDIAN())
        readers_raw[i] = new LASreadItemRaw_RGB12_LE();
      else
        readers_raw[i] = new LASreadItemRaw_RGB12_BE();
      break;
    case LASitem::BYTE:
    case LASitem::BYTE14:
      readers_raw[i] = new LASreadItemRaw_BYTE(items[i].size);
      break;
    case LASitem::POINT14:
      if (IS_LITTLE_ENDIAN())
        readers_raw[i] = new LASreadItemRaw_POINT14_LE();
      else
        readers_raw[i] = new LASreadItemRaw_POINT14_BE();
      break;
    case LASitem::RGBNIR14:
      if (IS_LITTLE_ENDIAN())
        readers_raw[i] = new LASreadItemRaw_RGBNIR14_LE();
      else
        readers_raw[i] = new LASreadItemRaw_RGBNIR14_BE();
      break;
    case LASitem::WAVEPACKET13:
    case LASitem::WAVEPACKET14:
      if (IS_LITTLE_ENDIAN())
        readers_raw[i] = new LASreadItemRaw_WAVEPACKET13_LE();
      else
        readers_raw[i] = new LASreadItemRaw_WAVEPACKET13_BE();
      break;
    default:
      return FALSE;
    }
    point_size += items[i].size;
  }

  if (dec)
  {
    readers_compressed = new LASreadItem*[num_readers];
    // seeks with compressed data need a seek point
    if (seek_point)
    {
      delete [] seek_point[0];
      delete [] seek_point;
    }
    seek_point = new U8*[num_items];
    if (!seek_point) return FALSE;
    if (layered_las14_compression)
    {
      // because combo LAS 1.0 - 1.4 point struct has padding
      seek_point[0] = new U8[(point_size*2)];
      // because extended_point_type must be set
      seek_point[0][22] = 1;
    }
    else
    {
      seek_point[0] = new U8[point_size];
    }
    if (!seek_point[0]) return FALSE;
    for (i = 0; i < num_readers; i++)
    {
      switch (items[i].type)
      {
      case LASitem::POINT10:
        if (items[i].version == 1)
          readers_compressed[i] = new LASreadItemCompressed_POINT10_v1(dec);
        else if (items[i].version == 2)
          readers_compressed[i] = new LASreadItemCompressed_POINT10_v2(dec);
        else
          return FALSE;
        break;
      case LASitem::GPSTIME11:
        if (items[i].version == 1)
          readers_compressed[i] = new LASreadItemCompressed_GPSTIME11_v1(dec);
        else if (items[i].version == 2)
          readers_compressed[i] = new LASreadItemCompressed_GPSTIME11_v2(dec);
        else
          return FALSE;
        break;
      case LASitem::RGB12:
        if (items[i].version == 1)
          readers_compressed[i] = new LASreadItemCompressed_RGB12_v1(dec);
        else if (items[i].version == 2)
          readers_compressed[i] = new LASreadItemCompressed_RGB12_v2(dec);
        else
          return FALSE;
        break;
      case LASitem::BYTE:
        if (items[i].version == 1)
          readers_compressed[i] = new LASreadItemCompressed_BYTE_v1(dec, items[i].size);
        else if (items[i].version == 2)
          readers_compressed[i] = new LASreadItemCompressed_BYTE_v2(dec, items[i].size);
        else
          return FALSE;
        break;
      case LASitem::POINT14:
        if ((items[i].version == 3) || (items[i].version == 2)) // version == 2 from lasproto
          readers_compressed[i] = new LASreadItemCompressed_POINT14_v3(dec, decompress_selective);
        else if (items[i].version == 4)
          readers_compressed[i] = new LASreadItemCompressed_POINT14_v4(dec, decompress_selective);
        else
          return FALSE;
        break;
      case LASitem::RGB14:
        if ((items[i].version == 3) || (items[i].version == 2)) // version == 2 from lasproto
          readers_compressed[i] = new LASreadItemCompressed_RGB14_v3(dec, decompress_selective);
        else if (items[i].version == 4)
          readers_compressed[i] = new LASreadItemCompressed_RGB14_v4(dec, decompress_selective);
        else
          return FALSE;
        break;
      case LASitem::RGBNIR14:
        if ((items[i].version == 3) || (items[i].version == 2)) // version == 2 from lasproto
          readers_compressed[i] = new LASreadItemCompressed_RGBNIR14_v3(dec, decompress_selective);
        else if (items[i].version == 4)
          readers_compressed[i] = new LASreadItemCompressed_RGBNIR14_v4(dec, decompress_selective);
        else
          return FALSE;
        break;
      case LASitem::BYTE14:
        if ((items[i].version == 3) || (items[i].version == 2)) // version == 2 from lasproto
          readers_compressed[i] = new LASreadItemCompressed_BYTE14_v3(dec, items[i].size, decompress_selective);
        else if (items[i].version == 4)
          readers_compressed[i] = new LASreadItemCompressed_BYTE14_v4(dec, items[i].size, decompress_selective);
        else
          return FALSE;
        break;
      case LASitem::WAVEPACKET13:
        if (items[i].version == 1)
          readers_compressed[i] = new LASreadItemCompressed_WAVEPACKET13_v1(dec);
        else
          return FALSE;
        break;
      case LASitem::WAVEPACKET14:
        if (items[i].version == 3)
          readers_compressed[i] = new LASreadItemCompressed_WAVEPACKET14_v3(dec, decompress_selective);
        else if (items[i].version == 4)
          readers_compressed[i] = new LASreadItemCompressed_WAVEPACKET14_v4(dec, decompress_selective);
        else
          return FALSE;
        break;
      default:
        return FALSE;
      }
      if (i)
      {
        if (layered_las14_compression)
        {
          // because combo LAS 1.0 - 1.4 point struct has padding
          seek_point[i] = seek_point[i-1]+(2*items[i-1].size);
        }
        else
        {
          seek_point[i] = seek_point[i-1]+items[i-1].size;
        }
      }
    }
    if (laszip->compressor != LASZIP_COMPRESSOR_POINTWISE)
    {
      if (laszip->chunk_size) chunk_size = laszip->chunk_size;
      number_chunks = U32_MAX;
    }
  }
  return TRUE;
}

BOOL LASreadPoint::init(ByteStreamIn* instream)
{
  if (!instream) return FALSE;
  this->instream = instream;

  U32 i;
  for (i = 0; i < num_readers; i++)
  {
    ((LASreadItemRaw*)(readers_raw[i]))->init(instream);
  }

  if (dec)
  {
    chunk_count = chunk_size;
    point_start = 0;
    readers = 0;
  }
  else
  {
    point_start = instream->tell();
    readers = readers_raw;
  }

  return TRUE;
}

BOOL LASreadPoint::seek(const U32 current, const U32 target)
{
  if (!instream->isSeekable()) return FALSE;
  U32 delta = 0;
  if (dec)
  {
    if (point_start == 0)
    {
      init_dec();
      chunk_count = 0;
    }
    if (chunk_starts)
    {
      U32 target_chunk;
      if (chunk_totals)
      {
        target_chunk = search_chunk_table(target, 0, number_chunks);
        chunk_size = chunk_totals[target_chunk+1]-chunk_totals[target_chunk];
        delta = target - chunk_totals[target_chunk];
      }
      else
      {
        target_chunk = target/chunk_size;
        delta = target%chunk_size;
      }
      if (target_chunk >= tabled_chunks)
      {
        if (current_chunk < (tabled_chunks-1))
        {
          dec->done();
          current_chunk = (tabled_chunks-1);
          instream->seek(chunk_starts[current_chunk]);
          init_dec();
          chunk_count = 0;
        }
        delta += (chunk_size*(target_chunk-current_chunk) - chunk_count);
      }
      else if (current_chunk != target_chunk || current > target)
      {
        dec->done();
        current_chunk = target_chunk;
        instream->seek(chunk_starts[current_chunk]);
        init_dec();
        chunk_count = 0;
      }
      else
      {
        delta = target - current;
      }
    }
    else if (current > target)
    {
      dec->done();
      instream->seek(point_start);
      init_dec();
      delta = target;
    }
    else if (current < target)
    {
      delta = target - current;
    }
    while (delta)
    {
      read(seek_point);
      delta--;
    }
  }
  else
  {
    if (current != target)
    {
      instream->seek(point_start+(I64)point_size*target);
    }
  }
  return TRUE;
}

BOOL LASreadPoint::read(U8* const * point)
{
  U32 i;
  U32 context = 0;

  try
  {
    if (dec)
    {
      if (chunk_count == chunk_size)
      {
        if (point_start != 0)
        {
          dec->done();
          current_chunk++;
          // check integrity
          if (current_chunk < tabled_chunks)
          {
            I64 here = instream->tell();
            if (chunk_starts[current_chunk] != here)
            {
              // previous chunk was corrupt
              current_chunk--;
              throw 4711;
            }
          }
        }
        init_dec();
        if (current_chunk == tabled_chunks) // no or incomplete chunk table?
        {
          if (current_chunk == number_chunks)
          {
            number_chunks += 256;
            chunk_starts = (I64*)realloc(chunk_starts, sizeof(I64)*(number_chunks+1));
          }
          chunk_starts[tabled_chunks] = point_start; // needs fixing
          tabled_chunks++;
        }
        else if (chunk_totals) // variable sized chunks?
        {
          chunk_size = chunk_totals[current_chunk+1]-chunk_totals[current_chunk];
        }
        chunk_count = 0;
      }
      chunk_count++;

      if (readers)
      {
        for (i = 0; i < num_readers; i++)
        {
          readers[i]->read(point[i], context);
        }
      }
      else
      {
        for (i = 0; i < num_readers; i++)
        {
          readers_raw[i]->read(point[i], context);
        }
        if (layered_las14_compression)
        {
          // for layered compression 'dec' only hands over the stream
          dec->init(instream, FALSE);
          // read how many points are in the chunk
          U32 count;
          instream->get32bitsLE((U8*)&count);
          // read the sizes of all layers
          for (i = 0; i < num_readers; i++)
          {
            ((LASreadItemCompressed*)(readers_compressed[i]))->chunk_sizes();
          }
          for (i = 0; i < num_readers; i++)
          {
            ((LASreadItemCompressed*)(readers_compressed[i]))->init(point[i], context);
          }
          if (DEBUG_OUTPUT_NUM_BYTES_DETAILS) fprintf(stderr, "\n");
        }
        else
        {
          for (i = 0; i < num_readers; i++)
          {
            ((LASreadItemCompressed*)(readers_compressed[i]))->init(point[i], context);
          }
          dec->init(instream);
        }
        readers = readers_compressed;
      }
    }
    else
    {
      for (i = 0; i < num_readers; i++)
      {
        readers[i]->read(point[i], context);
      }
    }
  }
  catch (I32 exception) 
  {
    // create error string
    if (last_error == 0) last_error = new CHAR[128];
    // report error
    if (exception == EOF)
    {
      // end-of-file
      if (dec)
      {
        sprintf(last_error, "end-of-file during chunk with index %u", current_chunk);
      }
      else
      {
        sprintf(last_error, "end-of-file");
      }
    }
    else
    {
      // decompression error
      sprintf(last_error, "chunk with index %u of %u is corrupt", current_chunk, tabled_chunks);
      // if we know where the next chunk starts ...
      if ((current_chunk+1) < tabled_chunks)
      {
        // ... try to seek to the next chunk
        instream->seek(chunk_starts[(current_chunk+1)]);
        // ... ready for next LASreadPoint::read()
        chunk_count = chunk_size;
      }
    }
    return FALSE;
  }
  return TRUE;
}

BOOL LASreadPoint::check_end()
{
  if (readers == readers_compressed)
  {
    if (dec)
    {
      dec->done();
      current_chunk++;
      // check integrity
      if (current_chunk < tabled_chunks)
      {
        I64 here = instream->tell();
        if (chunk_starts[current_chunk] != here)
        {
          // create error string
          if (last_error == 0) last_error = new CHAR[128];
          // last chunk was corrupt
          sprintf(last_error, "chunk with index %u of %u is corrupt", current_chunk, tabled_chunks);
          return FALSE;
        }
      }
    }
  }
  return TRUE;
}

BOOL LASreadPoint::done()
{
  instream = 0;
  return TRUE;
}

BOOL LASreadPoint::init_dec()
{
  // maybe read chunk table (only if chunking enabled)

  if (number_chunks == U32_MAX)
  {
    if (!read_chunk_table())
    {
      return FALSE;
    }
    current_chunk = 0;
    if (chunk_totals) chunk_size = chunk_totals[1];
  }

  point_start = instream->tell();
  readers = 0;

  return TRUE;
}

BOOL LASreadPoint::read_chunk_table()
{
  // read the 8 bytes that store the location of the chunk table
  I64 chunk_table_start_position;
  try { instream->get64bitsLE((U8*)&chunk_table_start_position); } catch(...)
  {
    return FALSE;
  }

  // this is where the chunks start
  I64 chunks_start = instream->tell();

  // was compressor interrupted before getting a chance to write the chunk table?
  if ((chunk_table_start_position + 8) == chunks_start)
  {
    // no choice but to fail if adaptive chunking was used
    if (chunk_size == U32_MAX)
    {
      return FALSE;
    }
    // otherwise we build the chunk table as we read the file
    number_chunks = 256;
    chunk_starts = (I64*)malloc(sizeof(I64)*(number_chunks+1));
    if (chunk_starts == 0)
    {
      return FALSE;
    }
    chunk_starts[0] = chunks_start;
    tabled_chunks = 1;
    return TRUE;
  }

  // maybe the stream is not seekable
  if (!instream->isSeekable())
  {
    // no choice but to fail if adaptive chunking was used
    if (chunk_size == U32_MAX)
    {
      return FALSE;
    }
    // then we cannot seek to the chunk table but won't need it anyways
    number_chunks = 0;
    tabled_chunks = 0;
    return TRUE;
  }

  if (chunk_table_start_position == -1)
  {
    // the compressor was writing to a non-seekable stream and wrote the chunk table start at the end
    if (!instream->seekEnd(8))
    {
      return FALSE;
    }
    try { instream->get64bitsLE((U8*)&chunk_table_start_position); } catch(...)
    {
      return FALSE;
    }
  }

  // read the chunk table
  try
  {
    instream->seek(chunk_table_start_position);
    U32 version;
    instream->get32bitsLE((U8*)&version);
    if (version != 0)
    {
      throw 1;
    }
    instream->get32bitsLE((U8*)&number_chunks);
    if (chunk_totals) delete [] chunk_totals;
    chunk_totals = 0;
    if (chunk_starts) free(chunk_starts);
    chunk_starts = 0;
    if (chunk_size == U32_MAX)
    {
      chunk_totals = new U32[number_chunks+1];
      if (chunk_totals == 0)
      {
        throw 1;
      }
      chunk_totals[0] = 0;
    }
    chunk_starts = (I64*)malloc(sizeof(I64)*(number_chunks+1));
    if (chunk_starts == 0)
    {
      throw 1;
    }
    chunk_starts[0] = chunks_start;
    tabled_chunks = 1;
    if (number_chunks > 0)
    {
      U32 i;
      dec->init(instream);
      IntegerCompressor ic(dec, 32, 2);
      ic.initDecompressor();
      for (i = 1; i <= number_chunks; i++)
      {
        if (chunk_size == U32_MAX) chunk_totals[i] = ic.decompress((i>1 ? chunk_totals[i-1] : 0), 0);
        chunk_starts[i] = ic.decompress((i>1 ? (U32)(chunk_starts[i-1]) : 0), 1);
        tabled_chunks++;
      }
      dec->done();
      for (i = 1; i <= number_chunks; i++)
      {
        if (chunk_size == U32_MAX) chunk_totals[i] += chunk_totals[i-1];
        chunk_starts[i] += chunk_starts[i-1];
        if (chunk_starts[i] <= chunk_starts[i-1])
        {
          throw 1;
        }
      }
    }
  }
  catch (...)
  {
    // something went wrong while reading the chunk table
    if (chunk_totals) delete [] chunk_totals;
    chunk_totals = 0;
    // no choice but to fail if adaptive chunking was used
    if (chunk_size == U32_MAX)
    {
      return FALSE;
    }
    // did we not even read the number of chunks
    if (number_chunks == U32_MAX)
    {
      // then compressor was interrupted before getting a chance to write the chunk table
      number_chunks = 256;
      chunk_starts = (I64*)malloc(sizeof(I64)*(number_chunks+1));
      if (chunk_starts == 0)
      {
        return FALSE;
      }
      chunk_starts[0] = chunks_start;
      tabled_chunks = 1;
    }
    else
    {
      // otherwise fix as many additional chunk_starts as possible
      U32 i;
      for (i = 1; i < tabled_chunks; i++)
      {
        chunk_starts[i] += chunk_starts[i-1];
      }
    }
    // create warning string
    if (last_warning == 0) last_warning = new CHAR[128];
    // report warning
    sprintf(last_warning, "corrupt chunk table");
  }
  if (!instream->seek(chunks_start))
  {
    return FALSE;
  }
  return TRUE;
}

U32 LASreadPoint::search_chunk_table(const U32 index, const U32 lower, const U32 upper)
{
  if (lower + 1 == upper) return lower;
  U32 mid = (lower+upper)/2;
  if (index >= chunk_totals[mid])
    return search_chunk_table(index, mid, upper);
  else
    return search_chunk_table(index, lower, mid);
}

LASreadPoint::~LASreadPoint()
{
  U32 i;

  if (readers_raw)
  {
    for (i = 0; i < num_readers; i++)
    {
      delete readers_raw[i];
    }
    delete [] readers_raw;
  }

  if (readers_compressed)
  {
    for (i = 0; i < num_readers; i++)
    {
      delete readers_compressed[i];
    }
    delete [] readers_compressed;
  }

  if (dec)
  {
    delete dec;
  }

  if (chunk_totals) delete [] chunk_totals;
  if (chunk_starts) free(chunk_starts);

  if (seek_point)
  {
    delete [] seek_point[0];
    delete [] seek_point;
  }

  if (last_error) delete [] last_error;
  if (last_warning) delete [] last_warning;
}
