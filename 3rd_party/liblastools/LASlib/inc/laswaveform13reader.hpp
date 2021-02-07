/*
===============================================================================

  FILE:  laswaveform13reader.hpp

  CONTENTS:

    Interface to read the Waveform Data Packets that are associated with points
    of type 4 and 5 in LAS 1.3.

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

     7 September 2018 -- replaced calls to _strdup with calls to the LASCopyString macro
    17 October 2011 -- created after bauarbeiter on the roof next door woke me

===============================================================================
*/
#ifndef LAS_WAVEFORM_13_READER_HPP
#define LAS_WAVEFORM_13_READER_HPP

#include "lasdefinitions.hpp"

class ByteStreamIn;
class LASwaveformDescription;
class ArithmeticDecoder;
class IntegerCompressor;

class LASwaveform13reader
{
public:
  U32 nbits;
  U32 nsamples;
  U32 temporal;
  F32 location;
  F32 XYZt[3];
  F64 XYZreturn[3];

  F64 XYZsample[3];

  U32 s_count;
  U32 sample;

  U32 sampleMin;
  U32 sampleMax;

  U8* samples;

  BOOL open(const char* file_name, I64 start_of_waveform_data_packet_record, const LASvlr_wave_packet_descr * const * wave_packet_descr);
  BOOL is_compressed() const;

  BOOL read_waveform(const LASpoint* point);

  BOOL get_samples();
  BOOL has_samples();

  BOOL get_samples_xyz();
  BOOL has_samples_xyz();

  void close();

  LASwaveform13reader();
  ~LASwaveform13reader();

private:
  BOOL compressed;
  U32 size;
  const LASvlr_wave_packet_descr * const * wave_packet_descr;
  FILE* file;
  ByteStreamIn* stream;
  I64 start_of_waveform_data_packet_record;
  I64 last_position;
  ArithmeticDecoder* dec;
  IntegerCompressor* ic8;
  IntegerCompressor* ic16;
};

#endif
