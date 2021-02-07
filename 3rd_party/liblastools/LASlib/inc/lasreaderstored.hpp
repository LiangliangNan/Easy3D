/*
===============================================================================

  FILE:  lasreaderstored.hpp
  
  CONTENTS:
  
    Reads LiDAR points from another LASreader and stores them in compressed form
    so they can be read from memory on the second read. This is especially used
    for piping LiDAR from one process to another for those modules that perform
    two reading passes over the input.

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
  
     9 December 2017 -- created at Octopus Resort on Waya Island in Fiji
  
===============================================================================
*/
#ifndef LAS_READER_STORED_HPP
#define LAS_READER_STORED_HPP

#include "lasreader.hpp"
#include "laswriter.hpp"
#include "bytestreamin_array.hpp"
#include "bytestreamout_array.hpp"

class LASreaderStored : public LASreader
{
public:

  BOOL open(LASreader* lasreader);
  BOOL reopen();
  LASreader* get_lasreader() const { return lasreader; };

  I32 get_format() const;

  void set_index(LASindex* index);
  LASindex* get_index() const;
  void set_filter(LASfilter* filter);
  void set_transform(LAStransform* transform);

  BOOL inside_tile(const F32 ll_x, const F32 ll_y, const F32 size);
  BOOL inside_circle(const F64 center_x, const F64 center_y, const F64 radius);
  BOOL inside_rectangle(const F64 min_x, const F64 min_y, const F64 max_x, const F64 max_y);

  BOOL seek(const I64 p_index){ return FALSE; };

  ByteStreamIn* get_stream() const { return 0; };
  void close(BOOL close_stream=TRUE);

  LASreaderStored();
  ~LASreaderStored();

protected:
  BOOL read_point_default();

private:
  LASreader* lasreader;
  LASwriter* laswriter;
  ByteStreamInArray* streaminarray;
  ByteStreamOutArray* streamoutarray;
};

#endif
