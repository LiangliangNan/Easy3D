/*
===============================================================================

  FILE:  lasreader_txt.hpp

  CONTENTS:

    Reads LIDAR points in LAS format through on-the-fly conversion from ASCII.

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2017, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

    7 September 2018 -- replaced calls to _strdup with calls to the LASCopyString macro
   22 July 2018 -- bug fix for parsing classfication to point type 6 (or higher)
   11 January 2017 -- added with<h>eld and scanner channe<l> for the parse string
   11 January 2017 -- added 'k'eypoint and 'o'verlap flags for the parse string
   17 January 2016 -- pre-scaling and pre-offsetting of "extra bytes" attributes
    9 July 2014 -- allowing input from stdin after the 7:1 in the World Cup
    8 April 2011 -- created after starting a google group for LAStools users

===============================================================================
*/
#ifndef LAS_READER_TXT_HPP
#define LAS_READER_TXT_HPP

#include "lasreader.hpp"

#include <stdio.h>

class LASreaderTXT : public LASreader
{
public:

  void set_pts(BOOL pts);
  void set_ptx(BOOL ptx);

  void set_translate_intensity(F32 translate_intensity);
  void set_scale_intensity(F32 scale_intensity);
  void set_translate_scan_angle(F32 translate_scan_angle);
  void set_scale_scan_angle(F32 scale_scan_angle);
  void set_scale_factor(const F64* scale_factor);
  void set_offset(const F64* offset);
  void add_attribute(I32 data_type, const CHAR* name, const CHAR* description=0, F64 scale=1.0, F64 offset=0.0, F64 pre_scale=1.0, F64 pre_offset=0.0, F64 no_data=F64_MAX);
  virtual BOOL open(const CHAR* file_name, U8 point_type=0, const CHAR* parse_string=0, I32 skip_lines=0, BOOL populate_header=FALSE);
  virtual BOOL open(FILE* file, const CHAR* file_name=0, U8 point_type=0, const CHAR* parse_string=0, I32 skip_lines=0, BOOL populate_header=FALSE);

  I32 get_format() const { return LAS_TOOLS_FORMAT_TXT; };

  BOOL seek(const I64 p_index);

  ByteStreamIn* get_stream() const;
  void close(BOOL close_stream=TRUE);
  BOOL reopen(const CHAR* file_name);

  LASreaderTXT();
  virtual ~LASreaderTXT();

protected:
  BOOL read_point_default();

private:
  U8 point_type;
  CHAR* parse_string;
  F32 translate_intensity;
  F32 scale_intensity;
  F32 translate_scan_angle;
  F32 scale_scan_angle;
  F64* scale_factor;
  F64* offset;
  I32 skip_lines;
  BOOL populated_header;
  BOOL ipts;
  BOOL iptx;
  FILE* file;
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
  I32 attribute_starts[32];
  BOOL parse_attribute(const CHAR* l, I32 index);
  BOOL parse(const CHAR* parse_string);
  BOOL check_parse_string(const CHAR* parse_string);
  void populate_scale_and_offset();
  void populate_bounding_box();
  void clean();
};

class LASreaderTXTrescale : public virtual LASreaderTXT
{
public:
  virtual BOOL open(const CHAR* file_name, U8 point_type=0, const CHAR* parse_string=0, I32 skip_lines=0, BOOL populate_header=FALSE);
  LASreaderTXTrescale(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor);

protected:
  F64 scale_factor[3];
};

class LASreaderTXTreoffset : public virtual LASreaderTXT
{
public:
  virtual BOOL open(const CHAR* file_name, U8 point_type=0, const CHAR* parse_string=0, I32 skip_lines=0, BOOL populate_header=FALSE);
  LASreaderTXTreoffset(F64 x_offset, F64 y_offset, F64 z_offset);
protected:
  F64 offset[3];
};

class LASreaderTXTrescalereoffset : public LASreaderTXTrescale, LASreaderTXTreoffset
{
public:
  BOOL open(const CHAR* file_name, U8 point_type=0, const CHAR* parse_string=0, I32 skip_lines=0, BOOL populate_header=FALSE);
  LASreaderTXTrescalereoffset(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor, F64 x_offset, F64 y_offset, F64 z_offset);
};

#endif
