/*
===============================================================================

  FILE:  laswaveform13writer.hpp

  CONTENTS:

    Interface to write the Waveform Data Packets that are associated with points
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
#ifndef LAS_WAVEFORM_13_WRITER_HPP
#define LAS_WAVEFORM_13_WRITER_HPP

#include "lasdefinitions.hpp"

class ByteStreamOut;
class LASwaveformDescription;
class ArithmeticEncoder;
class IntegerCompressor;

class LASwaveform13writer
{
public:
  BOOL open(const char* file_name, const LASvlr_wave_packet_descr * const * wave_packet_descr);

  BOOL write_waveform(LASpoint* point, U8* samples);

  void close();

  LASwaveform13writer();
  ~LASwaveform13writer();

private:
  LASwaveformDescription** waveforms;
  FILE* file;
  ByteStreamOut* stream;

  ArithmeticEncoder* enc;
  IntegerCompressor* ic8;
  IntegerCompressor* ic16;
};

#endif
