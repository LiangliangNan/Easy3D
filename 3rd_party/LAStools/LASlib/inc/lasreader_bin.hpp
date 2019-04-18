/*
===============================================================================

  FILE:  lasreader_bin.hpp
  
  CONTENTS:
  
    Reads LIDAR from Terrascan's BIN format via on-the-fly conversion as LAS.
    
    Format description: http://www.terrasolid.fi/system/files/tscan.pdf

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
  
    4 September 2011 -- created on Labor Day Sunday far from beloved mountains
  
===============================================================================
*/
#ifndef LAS_READER_BIN_HPP
#define LAS_READER_BIN_HPP

#include "lasreader.hpp"

#include <stdio.h>

class LASreaderBIN : public LASreader
{
public:

  BOOL open(const char* file_name);

  I32 get_format() const { return LAS_TOOLS_FORMAT_BIN; };

  BOOL seek(const I64 p_index);

  ByteStreamIn* get_stream() const;
  void close(BOOL close_stream=TRUE);
  BOOL reopen(const char* file_name);

  LASreaderBIN();
  virtual ~LASreaderBIN();

protected:
  virtual BOOL open(ByteStreamIn* stream);
  BOOL read_point_default();

private:
  FILE* file;
  ByteStreamIn* stream;
  I32 version;
};

class LASreaderBINrescale : public virtual LASreaderBIN
{
public:
  virtual BOOL open(ByteStreamIn* stream);
  LASreaderBINrescale(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor);

protected:
  F64 scale_factor[3];
};

class LASreaderBINreoffset : public virtual LASreaderBIN
{
public:
  virtual BOOL open(ByteStreamIn* stream);
  LASreaderBINreoffset(F64 x_offset, F64 y_offset, F64 z_offset);
protected:
  F64 offset[3];
};

class LASreaderBINrescalereoffset : public LASreaderBINrescale, LASreaderBINreoffset
{
public:
  BOOL open(ByteStreamIn* stream);
  LASreaderBINrescalereoffset(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor, F64 x_offset, F64 y_offset, F64 z_offset);
};

#endif
