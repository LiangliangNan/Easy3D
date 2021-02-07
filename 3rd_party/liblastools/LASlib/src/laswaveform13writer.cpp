/*
===============================================================================

  FILE:  laswaveform13writer.cpp

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
#include "laswaveform13writer.hpp"

#include "bytestreamout_file.hpp"
#include "arithmeticencoder.hpp"
#include "integercompressor.hpp"

class LASwaveformDescription
{
public:
  U8 compression;
  U8 nbits;
  U16 nsamples;
};

LASwaveform13writer::LASwaveform13writer()
{
  waveforms = 0;
  file = 0;
  stream = 0;
  enc = 0;
  ic8 = 0;
  ic16 = 0;
}

LASwaveform13writer::~LASwaveform13writer()
{
  if (waveforms)
  {
    I32 i;
    for (i = 0; i < 256; i++)
    {
      if (waveforms[i]) delete waveforms[i];
    }
    delete [] waveforms;
  }
  if (ic8) delete ic8;
  if (ic16) delete ic16;
  if (enc) delete enc;
}


BOOL LASwaveform13writer::open(const char* file_name, const LASvlr_wave_packet_descr * const * wave_packet_descr)
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

  // copy relevant wave packet descriptions and check if compressed or not

  U16 i, number = 0;
  BOOL compressed = FALSE;

  if (waveforms == 0)
  {
    waveforms = new LASwaveformDescription*[256];
    for (i = 0; i < 256; i++) waveforms[i] = 0;
  }

  for (i = 0; i < 256; i++)
  {
    if (wave_packet_descr[i])
    {
      if (waveforms[i] == 0)
      {
        waveforms[i] = new LASwaveformDescription;
      }
      waveforms[i]->compression = wave_packet_descr[i]->getCompressionType();
      waveforms[i]->nbits = wave_packet_descr[i]->getBitsPerSample();
      waveforms[i]->nsamples = wave_packet_descr[i]->getNumberOfSamples();
      compressed = compressed || (waveforms[i]->compression > 0);
      number++;
    }
    else
    {
      if (waveforms[i])
      {
        delete waveforms[i];
        waveforms[i] = 0;
      }
    }
  }

  // create file name and open file

  char* file_name_temp = LASCopyString(file_name);

  I32 len = (I32)strlen(file_name_temp);
  if (file_name_temp[len-3] == 'L' || file_name_temp[len-3] == 'W')
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
  file = fopen(file_name_temp, "wb");

  if (file == 0)
  {
    fprintf(stderr, "ERROR: cannot open waveform file '%s'\n", file_name_temp);
    free(file_name_temp);
    return FALSE;
  }
  free(file_name_temp);

  // create stream

  if (IS_LITTLE_ENDIAN())
  {
    stream = new ByteStreamOutFileLE(file);
  }
  else
  {
    stream = new ByteStreamOutFileBE(file);
  }

  // write extended variable length header variable after variable (to avoid alignment issues)

  U16 reserved = 0xAABB;
  if (!stream->put16bitsLE((U8*)&reserved))
  {
    fprintf(stderr,"ERROR: writing EVLR reserved\n");
    return FALSE;
  }
  I8 user_id[16];
  memset(user_id, 0, 16);
  strcpy(user_id, "LASF_Spec");
  if (!stream->putBytes((U8*)user_id, 16))
  {
    fprintf(stderr,"ERROR: writing EVLR user_id\n");
    return FALSE;
  }
  U16 record_id = 65535;
  if (!stream->put16bitsLE((U8*)&record_id))
  {
    fprintf(stderr,"ERROR: writing EVLR record_id\n");
    return FALSE;
  }
  I64 record_length_after_header = 0;
  if (!stream->put64bitsLE((U8*)&record_length_after_header))
  {
    fprintf(stderr,"ERROR: writing EVLR record_length_after_header\n");
    return FALSE;
  }
  I8 description[32];
  memset(description, 0, 32);
  sprintf(description, "%s by LAStools (%d)", (compressed ? "compressed" : "created"), LAS_TOOLS_VERSION);
  if (!stream->putBytes((U8*)description, 32))
  {
    fprintf(stderr,"ERROR: writing EVLR description\n");
    return FALSE;
  }

  // write waveform descriptor cross-check

  char magic[25];
  sprintf(magic, "LAStools waveform %d", LAS_TOOLS_VERSION);

  if (!stream->putBytes((U8*)magic, 24))
  {
    fprintf(stderr,"ERROR: writing waveform descriptor cross-check\n");
    return FALSE;
  }

  if (!stream->put16bitsLE((U8*)&number))
  {
    fprintf(stderr,"ERROR: writing number of waveform descriptors\n");
    return FALSE;
  }

  for (i = 0; i < 256; i++)
  {
    if (waveforms[i])
    {
      if (!stream->put16bitsLE((U8*)&i))
      {
        fprintf(stderr,"ERROR: writing index of waveform descriptor %d\n", i);
        return FALSE;
      }
      if (!stream->putByte(waveforms[i]->compression))
      {
        fprintf(stderr,"ERROR: writing compression of waveform descriptor %d\n", i);
        return FALSE;
      }
      if (!stream->putByte(waveforms[i]->nbits))
      {
        fprintf(stderr,"ERROR: writing nbits of waveform descriptor %d\n", i);
        return FALSE;
      }
      if (!stream->put16bitsLE((U8*)&(waveforms[i]->nsamples)))
      {
        fprintf(stderr,"ERROR: writing nsamples of waveform descriptor %d\n", i);
        return FALSE;
      }
    }
  }

  // create compressor

  if (compressed)
  {
    if (enc == 0) enc = new ArithmeticEncoder();
    if (ic8 == 0) ic8 = new IntegerCompressor(enc, 8);
    if (ic16 == 0) ic16 = new IntegerCompressor(enc, 16);
  }

  return TRUE;
}

BOOL LASwaveform13writer::write_waveform(LASpoint* point, U8* samples)
{
  U32 index = point->wavepacket.getIndex();
  if (index == 0)
  {
    return FALSE;
  }

  U32 nbits = waveforms[index]->nbits;
  if ((nbits != 8) && (nbits != 16))
  {
    fprintf(stderr, "ERROR: waveform with %d bits per samples not supported yet\n", nbits);
    return FALSE;
  }

  U32 nsamples = waveforms[index]->nsamples;
  if (nsamples == 0)
  {
    fprintf(stderr, "ERROR: waveform has no samples\n");
    return FALSE;
  }

  // set offset to waveform data

  I64 offset = stream->tell();
  point->wavepacket.setOffset(offset);

  // write waveform

  if (waveforms[index]->compression == 0)
  {
    U32 size = ((nbits/8) * nsamples);
    if (!stream->putBytes(samples, size))
    {
      fprintf(stderr, "ERROR: cannot write %u bytes for waveform with %u samples of %u bits\n", size, nsamples, nbits);
      return FALSE;
    }
    point->wavepacket.setSize(size);
  }
  else
  {
    U32 s_count;
    if (nbits == 8)
    {
      stream->putBytes(samples, 1);
      enc->init(stream);
      ic8->initCompressor();
      for (s_count = 1; s_count < nsamples; s_count++)
      {
        ic8->compress(samples[s_count-1], samples[s_count]);
      }
    }
    else
    {
      stream->putBytes(samples, 2);
      enc->init(stream);
      ic16->initCompressor();
      for (s_count = 1; s_count < nsamples; s_count++)
      {
        ic16->compress(((U16*)samples)[s_count-1], ((U16*)samples)[s_count]);
      }
    }
    enc->done();
    U32 size = (U32)(stream->tell() - offset);
    point->wavepacket.setSize(size);
  }

  return TRUE;
}

void LASwaveform13writer::close()
{
  if (stream->isSeekable())
  {
    I64 record_length_after_header = stream->tell();
    record_length_after_header -= 60;
    stream->seek(18);
    if (!stream->put64bitsLE((U8*)&record_length_after_header))
    {
      fprintf(stderr,"ERROR: updating EVLR record_length_after_header\n");
    }
    stream->seekEnd();
  }
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
