/*
===============================================================================

  FILE:  lasreadermerged.hpp
  
  CONTENTS:
  
    Reads LiDAR points from the LAS format from more than one file.

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
  
     5 September 2018 -- support for reading points from the PLY format
     1 December 2017 -- support extra bytes during '-merged' operations
     3 May 2015 -- header sets file source ID to 0 when merging flightlines 
    20 January 2011 -- created missing Livermore and my Extra Virgin Olive Oil
  
===============================================================================
*/
#ifndef LAS_READER_MERGED_HPP
#define LAS_READER_MERGED_HPP

#include "lasreader_las.hpp"
#include "lasreader_bin.hpp"
#include "lasreader_shp.hpp"
#include "lasreader_asc.hpp"
#include "lasreader_bil.hpp"
#include "lasreader_dtm.hpp"
#include "lasreader_ply.hpp"
#include "lasreader_qfit.hpp"
#include "lasreader_txt.hpp"

class LASreaderMerged : public LASreader
{
public:

  void set_io_ibuffer_size(I32 io_ibuffer_size);
  inline I32 get_io_ibuffer_size() const { return io_ibuffer_size; };
  BOOL add_file_name(const CHAR* file_name);
  void set_scale_factor(const F64* scale_factor);
  void set_offset(const F64* offset);
  void set_files_are_flightlines(const I32 files_are_flightlines);
  void set_apply_file_source_ID(const BOOL apply_file_source_ID);
  void set_translate_intensity(F32 translate_intensity);
  void set_scale_intensity(F32 scale_intensity);
  void set_translate_scan_angle(F32 translate_scan_angle);
  void set_scale_scan_angle(F32 scale_scan_angle);
  void set_parse_string(const CHAR* parse_string);
  void set_skip_lines(I32 skip_lines);
  void set_populate_header(BOOL populate_header);
  void set_keep_lastiling(BOOL keep_lastiling);
  BOOL open();
  BOOL reopen();

  void set_filter(LASfilter* filter);
  void set_transform(LAStransform* transform);

  BOOL inside_tile(const F32 ll_x, const F32 ll_y, const F32 size);
  BOOL inside_circle(const F64 center_x, const F64 center_y, const F64 radius);
  BOOL inside_rectangle(const F64 min_x, const F64 min_y, const F64 max_x, const F64 max_y);

  I32 get_format() const;

  BOOL seek(const I64 p_index){ return FALSE; };

  ByteStreamIn* get_stream() const { return 0; };
  void close(BOOL close_stream=TRUE);

  LASreaderMerged();
  ~LASreaderMerged();

protected:
  BOOL read_point_default();

private:
  BOOL open_next_file();
  void clean();

  LASreader* lasreader;
  LASreaderLAS* lasreaderlas;
  LASreaderBIN* lasreaderbin;
  LASreaderSHP* lasreadershp;
  LASreaderASC* lasreaderasc;
  LASreaderBIL* lasreaderbil;
  LASreaderDTM* lasreaderdtm;
  LASreaderPLY* lasreaderply;
  LASreaderQFIT* lasreaderqfit;
  LASreaderTXT* lasreadertxt;
  BOOL point_type_change;
  BOOL point_size_change;
  BOOL additional_attribute_change;
  BOOL rescale;
  BOOL reoffset;
  F64* scale_factor;
  F64* offset;
  I32 files_are_flightlines;
  BOOL apply_file_source_ID;
  F32 translate_intensity;
  F32 scale_intensity;
  F32 translate_scan_angle;
  F32 scale_scan_angle;
  CHAR* parse_string;
  I32 skip_lines;
  BOOL populate_header;
  BOOL keep_lastiling;
  U32 file_name_current;
  U32 file_name_number;
  U32 file_name_allocated;
  I32 io_ibuffer_size;
  CHAR** file_names;
  F64* bounding_boxes;
};

#endif
