/*
===============================================================================

  FILE:  lasreader_las.hpp
  
  CONTENTS:
  
    Reads LIDAR points from the LAS format

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
  
    10 July 2018 -- user must set seek-ability of istream (hard to determine) 
    19 April 2017 -- support for selective decompression for new LAS 1.4 points 
    1 February 2017 -- better support for OGC WKT strings in VLRs or EVLRs
    13 October 2014 -- changed default IO buffer size with setvbuf() to 262144
    27 August 2014 -- peek bounding box to open many file with lasreadermerged
     9 July 2012 -- fixed crash that occured when input had a corrupt VLRs
     5 November 2011 -- changed default IO buffer size with setvbuf() to 65536
    21 January 2011 -- adapted from lasreader to create abstract reader class
     3 December 2010 -- updated to (somewhat) support LAS format 1.3
     7 September 2008 -- updated to support LAS format 1.2 
    18 February 2007 -- created after repairing 2 vacuum cleaners in the garden
  
===============================================================================
*/
#ifndef LAS_READER_LAS_HPP
#define LAS_READER_LAS_HPP

#include "lasreader.hpp"
#include "laszip_decompress_selective_v3.hpp"

#include <stdio.h>

#ifdef LZ_WIN32_VC6
#include <fstream.h>
#else
#include <istream>
#include <fstream>
using namespace std;
#endif

class LASreadPoint;

class LASreaderLAS : public LASreader
{
public:

  void set_delete_stream(BOOL delete_stream=TRUE) { this->delete_stream = delete_stream; };

  BOOL open(const char* file_name, I32 io_buffer_size=LAS_TOOLS_IO_IBUFFER_SIZE, BOOL peek_only=FALSE, U32 decompress_selective=LASZIP_DECOMPRESS_SELECTIVE_ALL);
  BOOL open(FILE* file, BOOL peek_only=FALSE, U32 decompress_selective=LASZIP_DECOMPRESS_SELECTIVE_ALL);
  BOOL open(istream& stream, BOOL peek_only=FALSE, U32 decompress_selective=LASZIP_DECOMPRESS_SELECTIVE_ALL, BOOL seekable=TRUE);
  virtual BOOL open(ByteStreamIn* stream, BOOL peek_only=FALSE, U32 decompress_selective=LASZIP_DECOMPRESS_SELECTIVE_ALL);

  I32 get_format() const;

  BOOL seek(const I64 p_index);

  ByteStreamIn* get_stream() const;
  void close(BOOL close_stream=TRUE);

  LASreaderLAS();
  virtual ~LASreaderLAS();

protected:
  virtual BOOL read_point_default();

private:
  FILE* file;
  ByteStreamIn* stream;
  BOOL delete_stream;
  LASreadPoint* reader;
  BOOL checked_end;
};

class LASreaderLASrescale : public virtual LASreaderLAS
{
public:
  LASreaderLASrescale(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor, BOOL check_for_overflow=TRUE);

protected:
  virtual BOOL open(ByteStreamIn* stream, BOOL peek_only=FALSE, U32 decompress_selective=LASZIP_DECOMPRESS_SELECTIVE_ALL);
  virtual BOOL read_point_default();
  BOOL rescale_x, rescale_y, rescale_z;
  BOOL check_for_overflow;
  F64 scale_factor[3];
  F64 orig_x_scale_factor, orig_y_scale_factor, orig_z_scale_factor;
};

class LASreaderLASreoffset : public virtual LASreaderLAS
{
public:
  LASreaderLASreoffset(F64 x_offset, F64 y_offset, F64 z_offset);
  LASreaderLASreoffset(); // auto reoffset

protected:
  virtual BOOL open(ByteStreamIn* stream, BOOL peek_only=FALSE, U32 decompress_selective=LASZIP_DECOMPRESS_SELECTIVE_ALL);
  virtual BOOL read_point_default();
  BOOL auto_reoffset;
  BOOL reoffset_x, reoffset_y, reoffset_z;
  F64 offset[3];
  F64 orig_x_offset, orig_y_offset, orig_z_offset;
};

class LASreaderLASrescalereoffset : public LASreaderLASrescale, LASreaderLASreoffset
{
public:
  LASreaderLASrescalereoffset(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor, F64 x_offset, F64 y_offset, F64 z_offset);
  LASreaderLASrescalereoffset(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor); // auto reoffset

protected:
  BOOL open(ByteStreamIn* stream, BOOL peek_only=FALSE, U32 decompress_selective=LASZIP_DECOMPRESS_SELECTIVE_ALL);
  BOOL read_point_default();
};

#endif
