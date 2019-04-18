/*
===============================================================================

  FILE:  lasreader_shp.hpp
  
  CONTENTS:
  
    Reads LIDAR points in LAS format through on-the-fly conversion from SHP.

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2013, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    16 December 2011 -- after Silke got Australia mad and didn't call anymore
  
===============================================================================
*/
#ifndef LAS_READER_SHP_HPP
#define LAS_READER_SHP_HPP

#include "lasreader.hpp"

#include <stdio.h>

class LASreaderSHP : public LASreader
{
public:

  void set_scale_factor(const F64* scale_factor);
  void set_offset(const F64* offset);
  virtual BOOL open(const char* file_name);

  I32 get_format() const { return LAS_TOOLS_FORMAT_SHP; };

  BOOL seek(const I64 p_index);

  ByteStreamIn* get_stream() const;
  void close(BOOL close_stream=TRUE);
  BOOL reopen(const char* file_name);

  LASreaderSHP();
  virtual ~LASreaderSHP();

protected:
  BOOL read_point_default();

private:
  F64* scale_factor;
  F64* offset;
  FILE* file;
  I32 shape_type;
  I32* points;
  I32 points_allocated;
  I32 number_of_points;
  I32 point_count;
  bool piped;
  void populate_scale_and_offset();
  void populate_bounding_box();
  void clean();
};

class LASreaderSHPrescale : public virtual LASreaderSHP
{
public:
  virtual BOOL open(const char* file_name);
  LASreaderSHPrescale(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor);

protected:
  F64 scale_factor[3];
};

class LASreaderSHPreoffset : public virtual LASreaderSHP
{
public:
  virtual BOOL open(const char* file_name);
  LASreaderSHPreoffset(F64 x_offset, F64 y_offset, F64 z_offset);
protected:
  F64 offset[3];
};

class LASreaderSHPrescalereoffset : public LASreaderSHPrescale, LASreaderSHPreoffset
{
public:
  BOOL open(const char* file_name);
  LASreaderSHPrescalereoffset(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor, F64 x_offset, F64 y_offset, F64 z_offset);
};

#endif
