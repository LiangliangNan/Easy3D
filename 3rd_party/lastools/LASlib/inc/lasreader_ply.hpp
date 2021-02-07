/*
===============================================================================

  FILE:  lasreader_ply.hpp
  
  CONTENTS:
  
    Reads LIDAR points from binary PLY format through on-the-fly conversion.

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2018, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    4 September 2018 -- created after returning to Samara with locks changed
  
===============================================================================
*/
#ifndef LAS_READER_PLY_HPP
#define LAS_READER_PLY_HPP

#include "lasreader.hpp"

#include <stdio.h>

class LASreaderPLY : public LASreader
{
public:

  void set_translate_intensity(F32 translate_intensity);
  void set_scale_intensity(F32 scale_intensity);
  void set_scale_factor(const F64* scale_factor);
  void set_offset(const F64* offset);
  void add_attribute(I32 data_type, const CHAR* name, const CHAR* description=0, F64 scale=1.0, F64 offset=0.0, F64 pre_scale=1.0, F64 pre_offset=0.0, F64 no_data=F64_MAX);
  virtual BOOL open(const CHAR* file_name, U8 point_type=0, BOOL populate_header=FALSE);
  virtual BOOL open(FILE* file, const CHAR* file_name=0, U8 point_type=0, BOOL populate_header=FALSE);

  I32 get_format() const { return LAS_TOOLS_FORMAT_PLY; };

  BOOL seek(const I64 p_index);

  ByteStreamIn* get_stream() const;
  void close(BOOL close_stream=TRUE);
  BOOL reopen(const CHAR* file_name);

  LASreaderPLY();
  virtual ~LASreaderPLY();

protected:
  BOOL read_point_default();

private:
  U8 point_type;
  CHAR* parse_string;
  CHAR* type_string;
  F32 translate_intensity;
  F32 scale_intensity;
  F64* scale_factor;
  F64* offset;
  BOOL populated_header;
  FILE* file;
  ByteStreamIn* streamin;
  bool piped;
  CHAR line[512];
  I32 number_attributes;
  I32 attributes_data_types[32];
  const CHAR* attribute_names[32];
  const CHAR* attribute_descriptions[32];
  F64 attribute_scales[32];
  F64 attribute_offsets[32];
  F64 attribute_pre_scales[32];
  F64 attribute_pre_offsets[32];
  F64 attribute_no_datas[32];
  BOOL parse_header(BOOL quiet);
  BOOL set_attribute(I32 index, F64 value);
  BOOL parse_attribute(const CHAR* l, I32 index);
  BOOL parse(const CHAR* parse_string);
  F64 read_binary_value(CHAR type);
  BOOL read_binary_point();
  void populate_scale_and_offset();
  void populate_bounding_box();
  void clean();
};

class LASreaderPLYrescale : public virtual LASreaderPLY
{
public:
  virtual BOOL open(const CHAR* file_name, U8 point_type=0, BOOL populate_header=FALSE);
  LASreaderPLYrescale(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor);

protected:
  F64 scale_factor[3];
};

class LASreaderPLYreoffset : public virtual LASreaderPLY
{
public:
  virtual BOOL open(const CHAR* file_name, U8 point_type=0, BOOL populate_header=FALSE);
  LASreaderPLYreoffset(F64 x_offset, F64 y_offset, F64 z_offset);
protected:
  F64 offset[3];
};

class LASreaderPLYrescalereoffset : public LASreaderPLYrescale, LASreaderPLYreoffset
{
public:
  BOOL open(const CHAR* file_name, U8 point_type=0, BOOL populate_header=FALSE);
  LASreaderPLYrescalereoffset(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor, F64 x_offset, F64 y_offset, F64 z_offset);
};

#endif
