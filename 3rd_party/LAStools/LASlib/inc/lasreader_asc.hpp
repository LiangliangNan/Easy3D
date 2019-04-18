/*
===============================================================================

  FILE:  lasreader_asc.hpp
  
  CONTENTS:
  
    Reads an ESRI ASCII GRID in *.asc format as if it was a LiDAR point cloud.

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
  
    06 December 2013 -- option to deal with European '-comma_not_dot' numbers
    26 March 2012 -- created after forgetting my laptop adaptor in the pre-fab
  
===============================================================================
*/
#ifndef LAS_READER_ASC_HPP
#define LAS_READER_ASC_HPP

#include "lasreader.hpp"

#include <stdio.h>

class LASreaderASC : public LASreader
{
public:

  void set_scale_factor(const F64* scale_factor);
  void set_offset(const F64* offset);
  virtual BOOL open(const CHAR* file_name, BOOL comma_not_point=FALSE);

  I32 get_format() const { return LAS_TOOLS_FORMAT_ASC; };

  BOOL seek(const I64 p_index);

  ByteStreamIn* get_stream() const;
  void close(BOOL close_stream=TRUE);
  BOOL reopen(const CHAR* file_name);

  LASreaderASC();
  virtual ~LASreaderASC();

protected:
  BOOL read_point_default();

private:
  bool piped;
  BOOL comma_not_point;
  F64* scale_factor;
  F64* offset;
  FILE* file;
  CHAR* line;
  I32 header_lines;
  I32 line_size;
  I32 line_curr;
  I32 col, row, ncols, nrows;
  F64 xllcenter, yllcenter;
  F32 cellsize;
  F32 nodata;

  void clean();
  void populate_scale_and_offset();
  void populate_bounding_box();
};

class LASreaderASCrescale : public virtual LASreaderASC
{
public:
  virtual BOOL open(const CHAR* file_name, BOOL comma_not_point=FALSE);
  LASreaderASCrescale(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor);

protected:
  F64 scale_factor[3];
};

class LASreaderASCreoffset : public virtual LASreaderASC
{
public:
  virtual BOOL open(const CHAR* file_name, BOOL comma_not_point=FALSE);
  LASreaderASCreoffset(F64 x_offset, F64 y_offset, F64 z_offset);
protected:
  F64 offset[3];
};

class LASreaderASCrescalereoffset : public LASreaderASCrescale, LASreaderASCreoffset
{
public:
  BOOL open(const CHAR* file_name, BOOL comma_not_point=FALSE);
  LASreaderASCrescalereoffset(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor, F64 x_offset, F64 y_offset, F64 z_offset);
};

#endif
