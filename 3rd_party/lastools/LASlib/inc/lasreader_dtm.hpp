/*
===============================================================================

  FILE:  lasreader_dtm.hpp
  
  CONTENTS:
  
    Reads a binary DTM raster (*.dtm) as if it was a LiDAR point cloud.

  PROGRAMMERS:

    info@rapidlasso.de  -  https://rapidlasso.de

  COPYRIGHT:

    (c) 2007-2019, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    31 August 2019 -- add RasterLAZ during code sprint after FOSS4G 2019 in Bucharest 
    10 May 2019 -- checking for overflows in X, Y, Z 32 bit integers of fixed-point LAS
    10 October 2013 -- created after returning from INTERGEO 2013 in Essen
  
===============================================================================
*/
#ifndef LAS_READER_DTM_HPP
#define LAS_READER_DTM_HPP

#include "lasreader.hpp"

#include <stdio.h>

class LASreaderDTM : public LASreader
{
public:

  void set_scale_factor(const F64* scale_factor);
  void set_offset(const F64* offset);
  virtual BOOL open(const CHAR* file_name);

  I32 get_format() const { return LAS_TOOLS_FORMAT_BIL; };

  BOOL seek(const I64 p_index);

  ByteStreamIn* get_stream() const;
  void close(BOOL close_stream=TRUE);
  BOOL reopen(const CHAR* file_name);

  LASreaderDTM();
  virtual ~LASreaderDTM();

protected:
  BOOL read_point_default();

private:
  F64* scale_factor;
  F64* offset;
  FILE* file;
  I32 col, row, ncols, nrows;
  F64 ll_x, ll_y;
  F32 xdim, ydim;
  F32 nodata;
  I16 data_type;  // 2 = F32, 1 = I32, 0 = I16, 3 = F64
  I64 overflow_I32_x;
  I64 overflow_I32_y;
  I64 overflow_I32_z;

  void clean();
  void populate_scale_and_offset();
  void populate_bounding_box();
};

class LASreaderDTMrescale : public virtual LASreaderDTM
{
public:
  virtual BOOL open(const CHAR* file_name);
  LASreaderDTMrescale(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor);

protected:
  F64 scale_factor[3];
};

class LASreaderDTMreoffset : public virtual LASreaderDTM
{
public:
  virtual BOOL open(const CHAR* file_name);
  LASreaderDTMreoffset(F64 x_offset, F64 y_offset, F64 z_offset);
protected:
  F64 offset[3];
};

class LASreaderDTMrescalereoffset : public LASreaderDTMrescale, LASreaderDTMreoffset
{
public:
  BOOL open(const CHAR* file_name);
  LASreaderDTMrescalereoffset(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor, F64 x_offset, F64 y_offset, F64 z_offset);
};

#endif
