/*
===============================================================================

  FILE:  lasreaderpipeon.cpp
  
  CONTENTS:
  
    see corresponding header file
  
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
  
    see corresponding header file
  
===============================================================================
*/
#include "lasreaderpipeon.hpp"

#include "lasindex.hpp"
#include "lasfilter.hpp"
#include "lastransform.hpp"

#include "laswriter_las.hpp"

#include <stdlib.h>
#include <string.h>

BOOL LASreaderPipeOn::open(LASreader* lasreader)
{
  if (lasreader == 0)
  {
    fprintf(stderr, "ERROR: no lasreader\n");
    return FALSE;
  }

  // store a reference to the actual lasreader

  this->lasreader = lasreader;

  // populate our header from the actual header
  header = lasreader->header;

  // zero the pointers of the other header so they don't get deallocated twice
  lasreader->header.user_data_in_header = 0;
  lasreader->header.vlrs = 0;
  lasreader->header.laszip = 0;
  lasreader->header.vlr_lastiling = 0;
  lasreader->header.vlr_lasoriginal = 0;
  lasreader->header.user_data_after_header = 0;

  // special check for attributes in extra bytes
  if (header.number_attributes)
  {
    header.number_attributes = 0;
    header.init_attributes(lasreader->header.number_attributes, lasreader->header.attributes);
  }

  // initialize the point with the header info

  if (header.laszip)
  {
    if (!point.init(&header, header.laszip->num_items, header.laszip->items)) return FALSE;
  }
  else
  {
    if (!point.init(&header, header.point_data_format, header.point_data_record_length)) return FALSE;
  }

  // create the LASwriter

  if (laswriter) delete laswriter;
  laswriter = 0;

  LASwriterLAS* laswriterlas = new LASwriterLAS();

  if (laswriterlas == 0)
  {
    fprintf(stderr, "ERROR: allocating laswriterlas\n");
    return FALSE;
  }

  if (!laswriterlas->open(stdout, &header))
  {
    delete laswriterlas;
    fprintf(stderr, "ERROR: opening laswriterlas to stdout\n");
    return FALSE;
  }

  laswriter = laswriterlas;

  npoints = (header.number_of_point_records ? header.number_of_point_records : header.extended_number_of_point_records);
  p_count = 0;

  return TRUE;
}

void LASreaderPipeOn::set_index(LASindex* index)
{
  if (lasreader) lasreader->set_index(index);
}

LASindex* LASreaderPipeOn::get_index() const
{
  return (lasreader ? lasreader->get_index() : 0);
}

void LASreaderPipeOn::set_filter(LASfilter* filter)
{
  if (lasreader) lasreader->set_filter(filter);
}

void LASreaderPipeOn::set_transform(LAStransform* transform)
{
  if (lasreader) lasreader->set_transform(transform);
}

BOOL LASreaderPipeOn::inside_tile(const F32 ll_x, const F32 ll_y, const F32 size)
{
  return (lasreader ? lasreader->inside_tile(ll_x, ll_y, size) : FALSE);
}

BOOL LASreaderPipeOn::inside_circle(const F64 center_x, const F64 center_y, const F64 radius)
{
  return (lasreader ? lasreader->inside_circle(center_x, center_y, radius) : FALSE);
}

BOOL LASreaderPipeOn::inside_rectangle(const F64 min_x, const F64 min_y, const F64 max_x, const F64 max_y)
{
  return (lasreader ? lasreader->inside_rectangle(min_x, min_y, max_x, max_y) : FALSE);
}

I32 LASreaderPipeOn::get_format() const
{
  return (lasreader ? lasreader->get_format() : LAS_TOOLS_FORMAT_DEFAULT);
}

BOOL LASreaderPipeOn::read_point_default()
{
  while (true)
  {
    if (lasreader->read_point())
    {
      point = lasreader->point;
      if (laswriter)
      {
        laswriter->write_point(&point);
      }
      p_count++;
      return TRUE;
    }
    if (laswriter)
    {
      laswriter->close();
      delete laswriter;
      laswriter = 0;
    }
    point.zero();
    return FALSE;
  }
}

void LASreaderPipeOn::close(BOOL close_stream)
{
  if (lasreader) 
  {
    lasreader->close(close_stream);
  }
}

LASreaderPipeOn::LASreaderPipeOn()
{
  lasreader = 0;
  laswriter = 0;
}

LASreaderPipeOn::~LASreaderPipeOn()
{
  if (lasreader || laswriter) close();
  if (lasreader) delete lasreader;
}
