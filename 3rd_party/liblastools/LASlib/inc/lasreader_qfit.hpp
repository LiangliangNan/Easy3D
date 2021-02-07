/*
===============================================================================

  FILE:  lasreader_qfit.hpp
  
  CONTENTS:
  
    Reads LIDAR from NASA's ATM QFIT format via on-the-fly conversion to LAS.
    
    Format description: http://nsidc.org/data/docs/daac/icebridge/ilatm1b/docs/ReadMe.qfit.txt

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
  
     9 August 2016 -- fixed bug for QFIT version 40 or 56 (without pulse width)
    22 December 2011 -- created after my banker keeps me hostage for 2.5 hours
  
===============================================================================
*/
#ifndef LAS_READER_QFIT_HPP
#define LAS_READER_QFIT_HPP

#include "lasreader.hpp"

#include <stdio.h>

class LASreaderQFIT : public LASreader
{
public:

  BOOL open(const char* file_name);

  I32 get_format() const { return LAS_TOOLS_FORMAT_QFIT; };

  BOOL seek(const I64 p_index);

  ByteStreamIn* get_stream() const;
  void close(BOOL close_stream=TRUE);
  BOOL reopen(const char* file_name);

  LASreaderQFIT();
  virtual ~LASreaderQFIT();

protected:
  virtual BOOL open(ByteStreamIn* stream);
  BOOL read_point_default();

private:
  FILE* file;
  ByteStreamIn* stream;
  I32 version;
  BOOL little_endian;
  BOOL endian_swap;
  I32 offset;
  I32 buffer[14];
  BOOL populated_header;
  I32 scan_azimuth_start;
  I32 pitch_start;
  I32 roll_start;
  I32 pulse_width_start;
};

class LASreaderQFITrescale : public virtual LASreaderQFIT
{
public:
  virtual BOOL open(ByteStreamIn* stream);
  LASreaderQFITrescale(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor);

protected:
  F64 scale_factor[3];
};

class LASreaderQFITreoffset : public virtual LASreaderQFIT
{
public:
  virtual BOOL open(ByteStreamIn* stream);
  LASreaderQFITreoffset(F64 x_offset, F64 y_offset, F64 z_offset);
protected:
  F64 offset[3];
};

class LASreaderQFITrescalereoffset : public LASreaderQFITrescale, LASreaderQFITreoffset
{
public:
  BOOL open(ByteStreamIn* stream);
  LASreaderQFITrescalereoffset(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor, F64 x_offset, F64 y_offset, F64 z_offset);
};

#endif
