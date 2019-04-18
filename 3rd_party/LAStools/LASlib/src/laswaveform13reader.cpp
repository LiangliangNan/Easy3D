/*
===============================================================================

  FILE:  laswaveform13reader.cpp

  CONTENTS:

    see corresponding header file

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2012, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

    see corresponding header file

===============================================================================
*/
#include "laswaveform13reader.hpp"

#include "bytestreamin_file.hpp"
#include "arithmeticdecoder.hpp"
#include "integercompressor.hpp"

LASwaveform13reader::LASwaveform13reader()
{
  nbits = 0;
  nsamples = 0;
  temporal = 0;
  location = 0;
  XYZt[0] = XYZt[1] = XYZt[2] = 0;
  XYZreturn[0] = XYZreturn[1] = XYZreturn[2] = 0;

  s_count = 0;
  XYZsample[0] = XYZsample[1] = XYZsample[2] = 0;
  sample = 0;

  sampleMin = 0;
  sampleMax = 0;

  samples = 0;

  size = 0;
  compressed = FALSE;
  wave_packet_descr = 0;
  file = 0;
  stream = 0;
  last_position = 0;
  start_of_waveform_data_packet_record = 0;
  dec = 0;
  ic8 = 0;
  ic16 = 0;
}

LASwaveform13reader::~LASwaveform13reader()
{
  if (samples) delete [] samples;
  if (ic8) delete ic8;
  if (ic16) delete ic16;
  if (dec) delete dec;
}

BOOL LASwaveform13reader::is_compressed() const
{
  return compressed;
}

BOOL LASwaveform13reader::open(const char* file_name, I64 start_of_waveform_data_packet_record, const LASvlr_wave_packet_descr * const * wave_packet_descr)
{
  if (file_name == 0)
  {
    fprintf(stderr,"ERROR: file name pointer is zero\n");
    return FALSE;
  }

  if (wave_packet_descr == 0)
  {
    fprintf(stderr,"ERROR: wave packet descriptor pointer is zero\n");
    return FALSE;
  }

  // check if compressed or not

  I32 i;
  compressed = FALSE;

  for (i = 0; i < 256; i++)
  {
    if (wave_packet_descr[i])
    {
      compressed = compressed || (wave_packet_descr[i]->getCompressionType() > 0);
    }
  }

  // create file name and open file

  if (start_of_waveform_data_packet_record == 0)
  {
    if (!compressed && (strstr(".wdp", file_name) || strstr(".WDP", file_name)))
    {
      file = fopen(file_name, "rb");
    }
    else if (compressed && (strstr(".wdz", file_name) || strstr(".WDZ", file_name)))
    {
      file = fopen(file_name, "rb");
    }
    else
    {
      char* file_name_temp = LASCopyString(file_name);
      I32 len = (I32)strlen(file_name_temp);
      if ((file_name_temp[len-3] == 'L') || (file_name_temp[len-3] == 'W'))
      {
        file_name_temp[len-3] = 'W';
        file_name_temp[len-2] = 'D';
        file_name_temp[len-1] = (compressed ? 'Z' : 'P');
      }
      else
      {
        file_name_temp[len-3] = 'w';
        file_name_temp[len-2] = 'd';
        file_name_temp[len-1] = (compressed ? 'z' : 'p');
      }
      file = fopen(file_name_temp, "rb");
      free(file_name_temp);
    }
  }
  else
  {
    file = fopen(file_name, "rb");
  }

  if (file == 0)
  {
    fprintf(stderr, "ERROR: cannot open waveform file '%s'\n", file_name);
    return FALSE;
  }

  if (IS_LITTLE_ENDIAN())
  {
    stream = new ByteStreamInFileLE(file);
  }
  else
  {
    stream = new ByteStreamInFileBE(file);
  }

  this->start_of_waveform_data_packet_record = start_of_waveform_data_packet_record;
  this->wave_packet_descr = wave_packet_descr;

  // attempt waveform descriptor cross-check

  I64 position = start_of_waveform_data_packet_record + 60;
  stream->seek(position);

  char magic[25];
  try { stream->getBytes((U8*)magic, 24); } catch(...)
  {
    fprintf(stderr,"ERROR: reading waveform descriptor cross-check\n");
    return FALSE;
  }

  if (strncmp(magic, "LAStools waveform ", 18) == 0)
  {
    // do waveform descriptor cross-check

    U16 i, number;
    try { stream->get16bitsLE((U8*)&number); } catch(...)
    {
      fprintf(stderr,"ERROR: reading number of waveform descriptors\n");
      return FALSE;
    }
    for (i = 0; i < number; i++)
    {
      U16 index;
      try { stream->get16bitsLE((U8*)&index); } catch(...)
      {
        fprintf(stderr,"ERROR: reading index of waveform descriptor %d\n", i);
        return FALSE;
      }
      if (index > 255)
      {
        fprintf(stderr,"ERROR: cross-check - index %d of waveform descriptor %d out-of-range\n", index, i);
        return FALSE;
      }
      if (wave_packet_descr[index] == 0)
      {
        fprintf(stderr,"WARNING: cross-check - waveform descriptor %d with index %d unknown\n", i, index);
        I32 dummy;
        try { stream->get32bitsLE((U8*)&dummy); } catch(...)
        {
          fprintf(stderr,"ERROR: cross-check - reading rest of waveform descriptor %d\n", i);
          return FALSE;
        }
        continue;
      }
      U8 compression;
      try { stream->getBytes(&compression, 1); } catch(...)
      {
        fprintf(stderr,"ERROR: reading compression of waveform descriptor %d\n", i);
        return FALSE;
      }
      if (compression != wave_packet_descr[index]->getCompressionType())
      {
        fprintf(stderr,"ERROR: cross-check - compression %d %d of waveform descriptor %d with index %d is different\n", compression, wave_packet_descr[index]->getCompressionType(), i, index);
        return FALSE;
      }
      U8 nbits;
      try { stream->getBytes(&nbits, 1); } catch(...)
      {
        fprintf(stderr,"ERROR: reading nbits of waveform descriptor %d\n", i);
        return FALSE;
      }
      if (nbits != wave_packet_descr[index]->getBitsPerSample())
      {
        fprintf(stderr,"ERROR: cross-check - nbits %d %d of waveform descriptor %d with index %d is different\n", nbits, wave_packet_descr[index]->getBitsPerSample(), i, index);
        return FALSE;
      }
      U16 nsamples;
      try { stream->get16bitsLE((U8*)&nsamples); } catch(...)
      {
        fprintf(stderr,"ERROR: reading nsamples of waveform descriptor %d\n", i);
        return FALSE;
      }
      if (nsamples != wave_packet_descr[index]->getNumberOfSamples())
      {
        fprintf(stderr,"ERROR: cross-check - nsamples %d %d of waveform descriptor %d with index %d is different\n", nsamples, wave_packet_descr[index]->getNumberOfSamples(), i, index);
        return FALSE;
      }
    }
  }

  last_position = stream->tell();

  // create decompressor

  if (compressed)
  {
    if (dec == 0) dec = new ArithmeticDecoder();
    if (ic8 == 0) ic8 = new IntegerCompressor(dec, 8);
    if (ic16 == 0) ic16 = new IntegerCompressor(dec, 16);
  }
  return TRUE;
}

BOOL LASwaveform13reader::read_waveform(const LASpoint* point)
{
  U32 index = point->wavepacket.getIndex();
  if (index == 0)
  {
    return FALSE;
  }

  if (wave_packet_descr[index] == 0)
  {
    fprintf(stderr, "ERROR: wavepacket is indexing non-existant descriptor %u\n", index);
    return FALSE;
  }

  nbits = wave_packet_descr[index]->getBitsPerSample();
  if ((nbits != 8) && (nbits != 16))
  {
    fprintf(stderr, "ERROR: waveform with %d bits per samples not supported yet\n", nbits);
    return FALSE;
  }

  nsamples = wave_packet_descr[index]->getNumberOfSamples();

//  temporary Optech Fix
//  nsamples = point->wavepacket.getSize();
//  if (nbits == 16) nsamples / 2;

  if (nsamples == 0)
  {
    fprintf(stderr, "ERROR: waveform has no samples\n");
    return FALSE;
  }

  temporal = wave_packet_descr[index]->getTemporalSpacing();
  location = point->wavepacket.getLocation();

  XYZt[0] = point->wavepacket.getXt();
  XYZt[1] = point->wavepacket.getYt();
  XYZt[2] = point->wavepacket.getZt();

  XYZreturn[0] = point->get_x();
  XYZreturn[1] = point->get_y();
  XYZreturn[2] = point->get_z();

  // alloc data

  if (size < ((nbits/8) * nsamples))
  {
    if (samples) delete [] samples;
    samples = new U8[((nbits/8) * nsamples)];
  }

  size = ((nbits/8) * nsamples);

  // read waveform

  I64 position = start_of_waveform_data_packet_record + point->wavepacket.getOffset();
  stream->seek(position);

  if (wave_packet_descr[index]->getCompressionType() == 0)
  {
    try { stream->getBytes(samples, size); } catch(...)
    {
      fprintf(stderr, "ERROR: cannot read %u bytes for waveform with %u samples of %u bits\n", size, nsamples, nbits);
      return FALSE;
    }
  }
  else
  {
    if (nbits == 8)
    {
      stream->getBytes(samples, 1);
      dec->init(stream);
      ic8->initDecompressor();
      for (s_count = 1; s_count < nsamples; s_count++)
      {
        samples[s_count] = ic8->decompress(samples[s_count-1]);
      }
    }
    else
    {
      stream->getBytes(samples, 2);
      dec->init(stream);
      ic16->initDecompressor();
      for (s_count = 1; s_count < nsamples; s_count++)
      {
        ((U16*)samples)[s_count] = ic16->decompress(((U16*)samples)[s_count-1]);
      }
    }
    dec->done();
  }

  s_count = 0;
  return TRUE;
}

BOOL LASwaveform13reader::get_samples()
{
  if (nbits == 8)
  {
    sampleMin = samples[0];
    sampleMax = samples[0];
    for (s_count = 1; s_count < nsamples; s_count++)
    {
      if (samples[s_count] < sampleMin) sampleMin = samples[s_count];
      else if (samples[s_count] > sampleMax) sampleMax = samples[s_count];
    }
  }
  else
  {
    sampleMin = ((U16*)samples)[0];
    sampleMax = ((U16*)samples)[0];
    for (s_count = 1; s_count < nsamples; s_count++)
    {
      if (((U16*)samples)[s_count] < sampleMin) sampleMin = ((U16*)samples)[s_count];
      else if (((U16*)samples)[s_count] > sampleMax) sampleMax = ((U16*)samples)[s_count];
    }
  }
  s_count = 0;
  return (s_count < nsamples);
}

BOOL LASwaveform13reader::has_samples()
{
  if (s_count < nsamples)
  {
    if (nbits == 8)
    {
      sample = samples[s_count];
    }
    else
    {
      sample = ((U16*)samples)[s_count];
    }
    s_count++;
    return TRUE;
  }
  return FALSE;
}

BOOL LASwaveform13reader::get_samples_xyz()
{
  if (nbits == 8)
  {
    sampleMin = samples[0];
    sampleMax = samples[0];
    for (s_count = 1; s_count < nsamples; s_count++)
    {
      if (samples[s_count] < sampleMin) sampleMin = samples[s_count];
      else if (samples[s_count] > sampleMax) sampleMax = samples[s_count];
    }
  }
  else
  {
    sampleMin = ((U16*)samples)[0];
    sampleMax = ((U16*)samples)[0];
    for (s_count = 1; s_count < nsamples; s_count++)
    {
      if (((U16*)samples)[s_count] < sampleMin) sampleMin = ((U16*)samples)[s_count];
      else if (((U16*)samples)[s_count] > sampleMax) sampleMax = ((U16*)samples)[s_count];
    }
  }
  s_count = 0;
  return (s_count < nsamples);
}

BOOL LASwaveform13reader::has_samples_xyz()
{
  if (s_count < nsamples)
  {
    F32 dist = location - s_count*temporal;
    XYZsample[0] = XYZreturn[0] + dist*XYZt[0];
    XYZsample[1] = XYZreturn[1] + dist*XYZt[1];
    XYZsample[2] = XYZreturn[2] + dist*XYZt[2];
    if (nbits == 8)
    {
      sample = samples[s_count];
    }
    else
    {
      sample = ((U16*)samples)[s_count];
    }
    s_count++;
    return TRUE;
  }
  return FALSE;
}

void LASwaveform13reader::close()
{
  if (stream)
  {
    delete stream;
    stream = 0;
  }
  if (file)
  {
    fclose(file);
    file = 0;
  }
}
