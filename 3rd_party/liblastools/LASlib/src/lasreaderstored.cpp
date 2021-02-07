/*
===============================================================================

  FILE:  lasreaderstored.cpp
  
  CONTENTS:
  
    see corresponding header file
  
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
  
    see corresponding header file
  
===============================================================================
*/
#include "lasreaderstored.hpp"

#include "lasindex.hpp"
#include "lasfilter.hpp"
#include "lastransform.hpp"

#include "laswriter_las.hpp"
#include "lasreader_las.hpp"

#include <stdlib.h>
#include <string.h>

BOOL LASreaderStored::open(LASreader* lasreader)
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
  lasreader->header.unlink();

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

  // create the stream output array

  if (streamoutarray) delete streamoutarray;
  streamoutarray = 0;

  if (IS_LITTLE_ENDIAN())
    streamoutarray = new ByteStreamOutArrayLE((header.number_of_point_records ? (I64)header.number_of_point_records : header.extended_number_of_point_records)*2);
  else
    streamoutarray = new ByteStreamOutArrayBE((header.number_of_point_records ? (I64)header.number_of_point_records : header.extended_number_of_point_records)*2);

  if (streamoutarray == 0)
  {
    fprintf(stderr, "ERROR: allocating streamoutarray\n");
    return FALSE;
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

  if (!laswriterlas->open(streamoutarray, &header, LASZIP_COMPRESSOR_DEFAULT))
  {
    delete laswriterlas;
    laswriterlas = 0;
    fprintf(stderr, "ERROR: opening laswriterlas to streamoutarray\n");
    return FALSE;
  }

  laswriterlas->set_delete_stream(FALSE);

  laswriter = laswriterlas;

  npoints = (header.number_of_point_records ? header.number_of_point_records : header.extended_number_of_point_records);
  p_count = 0;

  return TRUE;
}

BOOL LASreaderStored::reopen()
{
  if (streaminarray)
  {
    streaminarray->seek(0);
  }
  else
  {
    if (streamoutarray == 0)
    {
      fprintf(stderr, "ERROR: no streamoutarray\n");
      return FALSE;
    }

    if (streamoutarray->getSize() == 0)
    {
      fprintf(stderr, "ERROR: nothing stored in streamoutarray\n");
      return FALSE;
    }

    if (IS_LITTLE_ENDIAN())
      streaminarray = new ByteStreamInArrayLE(streamoutarray->takeData(), streamoutarray->getSize());
    else
      streaminarray = new ByteStreamInArrayBE(streamoutarray->takeData(), streamoutarray->getSize());

    if (streaminarray == 0)
    {
      fprintf(stderr, "ERROR: creating streaminarray\n");
      return FALSE;
    }
  }

  // create the LASreader

  if (lasreader) delete lasreader;
  lasreader = 0;

  LASreaderLAS* lasreaderlas = new LASreaderLAS();

  if (lasreaderlas == 0)
  {
    fprintf(stderr, "ERROR: allocating lasreaderlas\n");
    return FALSE;
  }

  if (!lasreaderlas->open(streaminarray))
  {
    delete lasreaderlas;
    lasreaderlas = 0;
    fprintf(stderr, "ERROR: opening lasreaderlas from streaminarray\n");
    return FALSE;
  }

  lasreaderlas->set_delete_stream(FALSE);

  lasreader = lasreaderlas;

  header = lasreader->header;
  lasreader->header.unlink();

  npoints = (header.number_of_point_records ? header.number_of_point_records : header.extended_number_of_point_records);
  p_count = 0;


  return TRUE;
}

void LASreaderStored::set_index(LASindex* index)
{
  if (lasreader) lasreader->set_index(index);
}

LASindex* LASreaderStored::get_index() const
{
  return (lasreader ? lasreader->get_index() : 0);
}

void LASreaderStored::set_filter(LASfilter* filter)
{
  if (lasreader) lasreader->set_filter(filter);
}

void LASreaderStored::set_transform(LAStransform* transform)
{
  if (lasreader) lasreader->set_transform(transform);
}

BOOL LASreaderStored::inside_tile(const F32 ll_x, const F32 ll_y, const F32 size)
{
  return (lasreader ? lasreader->inside_tile(ll_x, ll_y, size) : FALSE);
}

BOOL LASreaderStored::inside_circle(const F64 center_x, const F64 center_y, const F64 radius)
{
  return (lasreader ? lasreader->inside_circle(center_x, center_y, radius) : FALSE);
}

BOOL LASreaderStored::inside_rectangle(const F64 min_x, const F64 min_y, const F64 max_x, const F64 max_y)
{
  return (lasreader ? lasreader->inside_rectangle(min_x, min_y, max_x, max_y) : FALSE);
}

I32 LASreaderStored::get_format() const
{
  return (lasreader ? lasreader->get_format() : LAS_TOOLS_FORMAT_DEFAULT);
}

BOOL LASreaderStored::read_point_default()
{
  if (lasreader)
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
    lasreader->close();
    delete lasreader;
    lasreader = 0;
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

void LASreaderStored::close(BOOL close_stream)
{
  if (lasreader) 
  {
    lasreader->close(close_stream);
    delete lasreader;
    lasreader = 0;
  }
  if (laswriter)
  {
    laswriter->close();
    delete laswriter;
    laswriter = 0;
  }
}

LASreaderStored::LASreaderStored()
{
  lasreader = 0;
  laswriter = 0;
  streaminarray = 0;
  streamoutarray = 0;
}

LASreaderStored::~LASreaderStored()
{
  if (lasreader || laswriter) close();
  if (lasreader) delete lasreader;
  if (laswriter) delete laswriter;
  if (streaminarray) delete streaminarray;
  if (streamoutarray) delete streamoutarray;
}
