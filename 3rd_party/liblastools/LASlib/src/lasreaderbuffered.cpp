/*
===============================================================================

  FILE:  lasreaderbuffered.cpp
  
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
#include "lasreaderbuffered.hpp"

#include "lasindex.hpp"
#include "lasfilter.hpp"
#include "lastransform.hpp"

#include <stdlib.h>
#include <string.h>

void LASreaderBuffered::set_scale_factor(const F64* scale_factor)
{
  lasreadopener.set_scale_factor(scale_factor);
  lasreadopener_neighbors.set_scale_factor(scale_factor);
}

void LASreaderBuffered::set_offset(const F64* offset)
{
  lasreadopener.set_offset(offset);
  lasreadopener_neighbors.set_offset(offset);
}

void LASreaderBuffered::set_translate_intensity(F32 translate_intensity)
{
  lasreadopener.set_translate_intensity(translate_intensity);
  lasreadopener_neighbors.set_translate_intensity(translate_intensity);
}

void LASreaderBuffered::set_scale_intensity(F32 scale_intensity)
{
  lasreadopener.set_scale_intensity(scale_intensity);
  lasreadopener_neighbors.set_scale_intensity(scale_intensity);
}

void LASreaderBuffered::set_translate_scan_angle(F32 translate_scan_angle)
{
  lasreadopener.set_translate_scan_angle(translate_scan_angle);
  lasreadopener_neighbors.set_translate_scan_angle(translate_scan_angle);
}

void LASreaderBuffered::set_scale_scan_angle(F32 scale_scan_angle)
{
  lasreadopener.set_scale_scan_angle(scale_scan_angle);
  lasreadopener_neighbors.set_scale_scan_angle(scale_scan_angle);
}

void LASreaderBuffered::set_parse_string(const char* parse_string)
{
  lasreadopener.set_parse_string(parse_string);
  lasreadopener_neighbors.set_parse_string(parse_string);
}

void LASreaderBuffered::set_skip_lines(I32 skip_lines)
{
  lasreadopener.set_skip_lines(skip_lines);
  lasreadopener_neighbors.set_skip_lines(skip_lines);
}

void LASreaderBuffered::set_populate_header(BOOL populate_header)
{
  lasreadopener.set_populate_header(populate_header);
  lasreadopener_neighbors.set_populate_header(populate_header);
}

BOOL LASreaderBuffered::set_file_name(const char* file_name)
{
  // do we have a file name
  if (file_name == 0)
  {
    fprintf(stderr, "ERROR: file name pointer is NULL\n");
    return FALSE;
  }
  // does the file exist
  FILE* file = fopen(file_name, "r");
  if (file == 0)
  {
    fprintf(stderr, "ERROR: file '%s' cannot be opened\n", file_name);
    return FALSE;
  }
  fclose(file);
  // set the file
  lasreadopener.add_file_name(file_name);
  return TRUE;
}

BOOL LASreaderBuffered::add_neighbor_file_name(const char* file_name)
{
  // do we have a file name
  if (file_name == 0)
  {
    fprintf(stderr, "ERROR: file name pointer is NULL\n");
    return FALSE;
  }
  // does the file exist
  FILE* file = fopen(file_name, "r");
  if (file == 0)
  {
    fprintf(stderr, "ERROR: file '%s' cannot be opened\n", file_name);
    return FALSE;
  }
  fclose(file);
  // add the file
  lasreadopener_neighbors.add_file_name(file_name);
  return TRUE;
}

void LASreaderBuffered::set_buffer_size(const F32 buffer_size)
{
  this->buffer_size = buffer_size;
}

BOOL LASreaderBuffered::open()
{
  if (!lasreadopener.active())
  {
    fprintf(stderr, "ERROR: no input name\n");
    return FALSE;
  }

  // open the main file

  lasreader = lasreadopener.open();
  if (lasreader == 0)
  {
    fprintf(stderr, "ERROR: opening '%s'\n", lasreadopener.get_file_name());
    return FALSE;
  }

  // populate the merged header
  header = lasreader->header;

  // unlink pointers in other header so their data does not get deallocated twice
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
    if (!point.init(&header, header.laszip->num_items, header.laszip->items, &header)) return FALSE;
  }
  else
  {
    if (!point.init(&header, header.point_data_format, header.point_data_record_length, &header)) return FALSE;
  }

  // maybe we have to look for neighbors from which we load buffer points

  if (lasreadopener_neighbors.active())
  {
    F64 xyz;

    lasreadopener_neighbors.set_inside_rectangle(header.min_x - buffer_size, header.min_y - buffer_size, header.max_x + buffer_size, header.max_y + buffer_size);

    // store current counts and bounding box in LASoriginal VLR

    header.set_lasoriginal();

    // force identical scale on the neighbors

    lasreadopener_neighbors.set_scale_factor(&header.x_scale_factor);

    // force identical offset on the neighbors

    lasreadopener_neighbors.set_offset(&header.x_offset);

    // open neighbors

    LASreader* lasreader_neighbor = lasreadopener_neighbors.open();
    if (lasreader_neighbor == 0)
    {
      fprintf(stderr, "ERROR: opening neighbor '%s'\n", lasreadopener_neighbors.get_file_name());
      return FALSE;
    }

    // a point type change could be problematic
    if (header.point_data_format != lasreader_neighbor->header.point_data_format)
    {
      if (!point_type_change) fprintf(stderr, "WARNING: files have different point types: %d vs %d\n", header.point_data_format, lasreader_neighbor->header.point_data_format);
      point_type_change = TRUE;
    }
    // a point size change could be problematic
    if (header.point_data_record_length != lasreader_neighbor->header.point_data_record_length)
    {
      if (!point_size_change) fprintf(stderr, "WARNING: files have different point sizes: %d vs %d\n", header.point_data_record_length, lasreader_neighbor->header.point_data_record_length);
      point_size_change = TRUE;
    }

    while (lasreader_neighbor->read_point())
    {
      // copy
      point = lasreader_neighbor->point;
      // copy_point_to_buffer
      copy_point_to_buffer();
      // increment number of points by return
      if (point.return_number == 1)
      {
        header.number_of_points_by_return[0]++;
      }
      else if (point.return_number == 2)
      {
        header.number_of_points_by_return[1]++;
      }
      else if (point.return_number == 3)
      {
        header.number_of_points_by_return[2]++;
      }
      else if (point.return_number == 4)
      {
        header.number_of_points_by_return[3]++;
      }
      else if (point.return_number == 5)
      {
        header.number_of_points_by_return[4]++;
      }
      // grow bounding box
      xyz = point.get_x();
      if (header.min_x > xyz) header.min_x = xyz;
      else if (header.max_x < xyz) header.max_x = xyz;
      xyz = point.get_y();
      if (header.min_y > xyz) header.min_y = xyz;
      else if (header.max_y < xyz) header.max_y = xyz;
      xyz = point.get_z();
      if (header.min_z > xyz) header.min_z = xyz;
      else if (header.max_z < xyz) header.max_z = xyz;
    }
    lasreader_neighbor->close();
    delete lasreader_neighbor;

    if (header.number_of_point_records)
    {
      header.number_of_point_records += buffered_points;
    }
    else
    {
      header.extended_number_of_point_records += buffered_points;
    }

    fprintf(stderr, "LASreaderBuffered: adding %u buffer points.\n", buffered_points);
  }

  // check if the header can support the enlarged bounding box

  // check x

  if ((((header.max_x - header.x_offset) / header.x_scale_factor) > I32_MAX) || (((header.min_x - header.x_offset) / header.x_scale_factor) < I32_MIN))
  {
    // maybe we can fix it by adjusting the offset (and if needed by lowering the resolution via the scale factor)
    F64 x_offset = (F64)I64_QUANTIZE((header.min_x + header.max_x)/2);
    F64 x_scale_factor = header.x_scale_factor;
    while ((((header.max_x - x_offset) / x_scale_factor) > I32_MAX) || (((header.min_x - x_offset) / x_scale_factor) < I32_MIN))
    {
      x_scale_factor *= 10;
    }
    if (header.x_scale_factor != x_scale_factor)
    {
      fprintf(stderr, "WARNING: i changed x_scale_factor from %g to %g to accommodate enlarged bounding box\n", header.x_scale_factor, x_scale_factor);
      header.x_scale_factor = x_scale_factor;
      rescale = TRUE;
    }
    // maybe we changed the resolution ... so do we really need to adjuste the offset
    if ((((header.max_x - header.x_offset) / x_scale_factor) > I32_MAX) || (((header.min_x - header.x_offset) / x_scale_factor) < I32_MIN))
    {
      fprintf(stderr, "WARNING: i changed x_offset from %g to %g to accommodate enlarged bounding box\n", header.x_offset, x_offset);
      header.x_offset = x_offset;
      reoffset = TRUE;
    }
  }
    
  // check y

  if ((((header.max_y - header.y_offset) / header.y_scale_factor) > I32_MAX) || (((header.min_y - header.y_offset) / header.y_scale_factor) < I32_MIN))
  {
    // maybe we can fix it by adjusting the offset (and if needed by lowering the resolution via the scale factor)
    F64 y_offset = (F64)I64_QUANTIZE((header.min_y + header.max_y)/2);
    F64 y_scale_factor = header.y_scale_factor;
    while ((((header.max_y - y_offset) / y_scale_factor) > I32_MAX) || (((header.min_y - y_offset) / y_scale_factor) < I32_MIN))
    {
      y_scale_factor *= 10;
    }
    if (header.y_scale_factor != y_scale_factor)
    {
      fprintf(stderr, "WARNING: i changed y_scale_factor from %g to %g to accommodate enlarged bounding box\n", header.y_scale_factor, y_scale_factor);
      header.y_scale_factor = y_scale_factor;
      rescale = TRUE;
    }
    // maybe we changed the resolution ... so do we really need to adjuste the offset
    if ((((header.max_y - header.y_offset) / y_scale_factor) > I32_MAX) || (((header.min_y - header.y_offset) / y_scale_factor) < I32_MIN))
    {
      fprintf(stderr, "WARNING: i changed y_offset from %g to %g to accommodate enlarged bounding box\n", header.y_offset, y_offset);
      header.y_offset = y_offset;
      reoffset = TRUE;
    }
  }
    
  // check z

  if ((((header.max_z - header.z_offset) / header.z_scale_factor) > I32_MAX) || (((header.min_z - header.z_offset) / header.z_scale_factor) < I32_MIN))
  {
    // maybe we can fix it by adjusting the offset (and if needed by lowering the resolution via the scale factor)
    F64 z_offset = (F64)I64_QUANTIZE((header.min_z + header.max_z)/2);
    F64 z_scale_factor = header.z_scale_factor;
    while ((((header.max_z - z_offset) / z_scale_factor) > I32_MAX) || (((header.min_z - z_offset) / z_scale_factor) < I32_MIN))
    {
      z_scale_factor *= 10;
    }
    if (header.z_scale_factor != z_scale_factor)
    {
      fprintf(stderr, "WARNING: i changed  z_scale_factor from %g to %g to accommodate enlarged bounding box\n", header.z_scale_factor, z_scale_factor);
      header.z_scale_factor = z_scale_factor;
      rescale = TRUE;
    }
    // maybe we changed the resolution ... so do we really need to adjuste the offset
    if ((((header.max_z - header.z_offset) / z_scale_factor) > I32_MAX) || (((header.min_z - header.z_offset) / z_scale_factor) < I32_MIN))
    {
      fprintf(stderr, "WARNING: i changed z_offset from %g to %g to accommodate enlarged bounding box\n", header.z_offset, z_offset);
      header.z_offset = z_offset;
      reoffset = TRUE;
    }
  }

  npoints = (header.number_of_point_records ? header.number_of_point_records : header.extended_number_of_point_records);
  p_count = 0;

  return TRUE;
}

BOOL LASreaderBuffered::reopen()
{
  p_count = 0;
  point_count = 0;
  if (lasreader)
  {
    return lasreadopener.reopen(lasreader);
  }
  return FALSE;
}

BOOL LASreaderBuffered::remove_buffer()
{
  clean_buffer();
  if (header.vlr_lasoriginal) npoints = header.vlr_lasoriginal->number_of_point_records;
  return header.restore_lasoriginal();
}

void LASreaderBuffered::set_filter(LASfilter* filter)
{
  if (lasreader)
  {
    lasreader->set_filter(filter);
  }
  lasreadopener.set_filter(filter);
  lasreadopener_neighbors.set_filter(filter);
  this->filter = filter;
}

void LASreaderBuffered::set_transform(LAStransform* transform)
{
  if (lasreader)
  {
    lasreader->set_transform(transform);
  }
  lasreadopener.set_transform(transform);
  lasreadopener_neighbors.set_transform(transform);
  this->transform = transform;
}

BOOL LASreaderBuffered::inside_tile(const F32 ll_x, const F32 ll_y, const F32 size)
{
  inside = 1;
  t_ll_x = ll_x;
  t_ll_y = ll_y;
  t_size = size;
  t_ur_x = ll_x + size;
  t_ur_y = ll_y + size;
  header.min_x = ll_x;
  header.min_y = ll_y;
  header.max_x = ll_x + size - 0.001f * header.x_scale_factor;
  header.max_y = ll_y + size - 0.001f * header.y_scale_factor;
  return TRUE;
}

BOOL LASreaderBuffered::inside_circle(const F64 center_x, const F64 center_y, const F64 radius)
{
  inside = 2;
  c_center_x = center_x;
  c_center_y = center_y;
  c_radius = radius;
  c_radius_squared = radius*radius;
  header.min_x = center_x - radius;
  header.min_y = center_y - radius;
  header.max_x = center_x + radius;
  header.max_y = center_y + radius;
  return TRUE;
}

BOOL LASreaderBuffered::inside_rectangle(const F64 min_x, const F64 min_y, const F64 max_x, const F64 max_y)
{
  inside = 3;
  r_min_x = min_x;
  r_min_y = min_y;
  r_max_x = max_x;
  r_max_y = max_y;
  header.min_x = min_x;
  header.min_y = min_y;
  header.max_x = max_x;
  header.max_y = max_y;
  return TRUE;
}

I32 LASreaderBuffered::get_format() const
{
  return lasreader->get_format();
}

BOOL LASreaderBuffered::read_point_default()
{
  while (true)
  {
    if (lasreader->read_point())
    {
      point = lasreader->point;
      p_count++;
      return TRUE;
    }
    else if (point_count < buffered_points)
    {
      copy_point_from_buffer();
      p_count++;
      return TRUE;
    }
    lasreader->close();
    point.zero();
    return FALSE;
  }
}

void LASreaderBuffered::close(BOOL close_stream)
{
  if (lasreader) 
  {
    lasreader->close(close_stream);
  }
}


void LASreaderBuffered::clean()
{
/*
  if (lasreader) 
  {
    delete lasreader;
    lasreader = 0;
  }
  point_type_change = FALSE;
  point_size_change = FALSE;
  inside = 0;
*/
}

void LASreaderBuffered::clean_buffer()
{
  if (buffers)
  {
    U32 i;
    for (i = 0; i < number_of_buffers; i++)
    {
      free(buffers[i]);
    }
    free(buffers);
    buffers = 0;
  }
  number_of_buffers = 0;
  current_buffer = 0;
  buffered_points = 0;
  point_count = 0;
}

BOOL LASreaderBuffered::copy_point_to_buffer()
{
  U32 point_count_in_buffer = (buffered_points % points_per_buffer);
  if (point_count_in_buffer == 0)
  {
    if (buffers == 0)
    {
      size_of_buffers_array = 1024;
      buffers = (U8**)malloc(sizeof(U8*)*size_of_buffers_array);
      number_of_buffers = 0;
    }
    else if (number_of_buffers == size_of_buffers_array)
    {
      size_of_buffers_array *= 2;
      buffers = (U8**)realloc(buffers, sizeof(U8*)*size_of_buffers_array);
    }
    buffers[number_of_buffers] = (U8*)malloc(point.total_point_size * points_per_buffer);
    current_buffer = buffers[number_of_buffers];
    number_of_buffers++;
  }
  point.copy_to(&(current_buffer[point_count_in_buffer*point.total_point_size]));
  buffered_points++;
  return TRUE;
}

BOOL LASreaderBuffered::copy_point_from_buffer()
{
  if (point_count >= buffered_points)
  {
    return FALSE;
  }
  U32 point_count_in_buffer = (point_count % points_per_buffer);
  if (point_count_in_buffer == 0)
  {
    U32 buffer_count = (point_count / points_per_buffer);
    current_buffer = buffers[buffer_count];
  }
  point.copy_from(&(current_buffer[point_count_in_buffer*point.total_point_size]));
  point_count++;
  return TRUE;
}

LASreaderBuffered::LASreaderBuffered() : points_per_buffer(10000)
{
  lasreader = 0;
  lasreadopener_neighbors.set_merged(TRUE);

  buffer_size = 0.0f;
  buffers = 0;
  clean();
  clean_buffer();
}

LASreaderBuffered::~LASreaderBuffered()
{
  lasreadopener.set_filter(0);
  lasreadopener_neighbors.set_filter(0);
  lasreadopener.set_transform(0);
  lasreadopener_neighbors.set_transform(0);
  if (lasreader) delete lasreader;
  clean_buffer();
}
