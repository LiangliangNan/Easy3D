/*
===============================================================================

  FILE:  lasreader_bil.hpp

  CONTENTS:

    Reads a binary BIL raster (*.bil + *.hdr) as if it was a LiDAR point cloud.

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2014, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

     7 September 2018 -- replaced calls to _strdup with calls to the LASCopyString macro
    20 June 2017 -- fixed reading of signed versus unsigned 16 and 8 bit intergers
     3 April 2012 -- created after joining the Spar Europe 2012 Advisory Board

===============================================================================
*/
#ifndef LAS_READER_BIL_HPP
#define LAS_READER_BIL_HPP

#include "lasreader.hpp"

#include <stdio.h>

class LASreaderBIL : public LASreader
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

  LASreaderBIL();
  virtual ~LASreaderBIL();

protected:
  BOOL read_point_default();

private:
  F64* scale_factor;
  F64* offset;
  FILE* file;
  I32 col, row, ncols, nrows, nbands, nbits;
  F64 ulxcenter, ulycenter;
  F32 xdim, ydim;
  F32 nodata;
  BOOL floatpixels;
  BOOL signedpixels;

  void clean();
  BOOL read_hdr_file(const CHAR* file_name);
  BOOL read_blw_file(const CHAR* file_name);
  void populate_scale_and_offset();
  void populate_bounding_box();
};

class LASreaderBILrescale : public virtual LASreaderBIL
{
public:
  virtual BOOL open(const CHAR* file_name);
  LASreaderBILrescale(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor);

protected:
  F64 scale_factor[3];
};

class LASreaderBILreoffset : public virtual LASreaderBIL
{
public:
  virtual BOOL open(const CHAR* file_name);
  LASreaderBILreoffset(F64 x_offset, F64 y_offset, F64 z_offset);
protected:
  F64 offset[3];
};

class LASreaderBILrescalereoffset : public LASreaderBILrescale, LASreaderBILreoffset
{
public:
  BOOL open(const CHAR* file_name);
  LASreaderBILrescalereoffset(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor, F64 x_offset, F64 y_offset, F64 z_offset);
};

#endif
