/*
===============================================================================

  FILE:  lasreader.cpp
  
  CONTENTS:
  
    see corresponding header file
  
  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2005-2013, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    see corresponding header file
  
===============================================================================
*/
#include "lasreader.hpp"

#include "lasindex.hpp"
#include "lasfilter.hpp"
#include "lastransform.hpp"

#include "lasreader_las.hpp"
#include "lasreader_bin.hpp"
#include "lasreader_shp.hpp"
#include "lasreader_qfit.hpp"
#include "lasreader_asc.hpp"
#include "lasreader_bil.hpp"
#include "lasreader_dtm.hpp"
#include "lasreader_txt.hpp"
#include "lasreader_ply.hpp"
#include "lasreadermerged.hpp"
#include "lasreaderbuffered.hpp"
#include "lasreaderstored.hpp"
#include "lasreaderpipeon.hpp"

#include <stdlib.h>
#include <string.h>

LASreader::LASreader()
{
  npoints = 0;
  p_count = 0;
  read_simple = &LASreader::read_point_default;
  read_complex = 0;
  index = 0;
  filter = 0;
  transform = 0;
  inside = 0;
  t_ll_x = 0;
  t_ll_y = 0;
  t_size = 0;
  t_ur_x = 0;
  t_ur_y = 0;
  c_center_x = 0;
  c_center_y = 0;
  c_radius = 0;
  c_radius_squared = 0;
  r_min_x = 0;
  r_min_y = 0;
  r_max_x = 0;
  r_max_y = 0;
  orig_min_x = 0;
  orig_min_y = 0;
  orig_max_x = 0;
  orig_max_y = 0;
}
  
LASreader::~LASreader()
{
  if (index) delete index;
}

void LASreader::set_index(LASindex* index)
{
  if (this->index) delete this->index;
  this->index = index;
}

void LASreader::set_filter(LASfilter* filter)
{
  this->filter = filter;
  if (filter && transform)
  {
    read_simple = &LASreader::read_point_filtered_and_transformed;
  }
  else if (filter)
  {
    read_simple = &LASreader::read_point_filtered;
  }
  else if (transform)
  {
    read_simple = &LASreader::read_point_transformed;
  }
  else
  {
    read_simple = &LASreader::read_point_default;
  }
  read_complex = &LASreader::read_point_default;
}

void LASreader::set_transform(LAStransform* transform)
{
  this->transform = transform;
  if (filter && transform)
  {
    read_simple = &LASreader::read_point_filtered_and_transformed;
  }
  else if (filter)
  {
    read_simple = &LASreader::read_point_filtered;
  }
  else if (transform)
  {
    read_simple = &LASreader::read_point_transformed;
  }
  else
  {
    read_simple = &LASreader::read_point_default;
  }
  read_complex = &LASreader::read_point_default;
}

BOOL LASreader::inside_none()
{
  if (filter || transform)
  {
    read_complex = &LASreader::read_point_default;
  }
  else
  {
    read_simple = &LASreader::read_point_default;
  }
  if (inside)
  {
    header.min_x = orig_min_x;
    header.min_y = orig_min_y;
    header.max_x = orig_max_x;
    header.max_y = orig_max_y;
    inside = 0;
  }
  return TRUE;
}

BOOL LASreader::inside_tile(const F32 ll_x, const F32 ll_y, const F32 size)
{
  inside = 1;
  t_ll_x = ll_x;
  t_ll_y = ll_y;
  t_size = size;
  t_ur_x = ll_x + size;
  t_ur_y = ll_y + size;
  orig_min_x = header.min_x;
  orig_min_y = header.min_y;
  orig_max_x = header.max_x;
  orig_max_y = header.max_y;
  header.min_x = ll_x;
  header.min_y = ll_y;
  header.max_x = ll_x + size;
  header.max_y = ll_y + size;
  header.max_x -= header.x_scale_factor;
  header.max_y -= header.y_scale_factor;
  if (((orig_min_x > header.max_x) || (orig_min_y > header.max_y) || (orig_max_x < header.min_x) || (orig_max_y < header.min_y)))
  {
    if (filter || transform)
    {
      read_complex = &LASreader::read_point_none;
    }
    else
    {
      read_simple = &LASreader::read_point_none;
    }
  }
  else if (filter || transform)
  {
    if (index)
    {
      if (index) index->intersect_tile(ll_x, ll_y, size);
      read_complex = &LASreader::read_point_inside_tile_indexed;
    }
    else
    {
      read_complex = &LASreader::read_point_inside_tile;
    }
  }
  else
  {
    if (index)
    {
      if (index) index->intersect_tile(ll_x, ll_y, size);
      read_simple = &LASreader::read_point_inside_tile_indexed;
    }
    else
    {
      read_simple = &LASreader::read_point_inside_tile;
    }
  }
  return TRUE;
}

BOOL LASreader::inside_circle(const F64 center_x, const F64 center_y, const F64 radius)
{
  inside = 2;
  c_center_x = center_x;
  c_center_y = center_y;
  c_radius = radius;
  c_radius_squared = radius*radius;
  orig_min_x = header.min_x;
  orig_min_y = header.min_y;
  orig_max_x = header.max_x;
  orig_max_y = header.max_y;
  header.min_x = center_x - radius;
  header.min_y = center_y - radius;
  header.max_x = center_x + radius;
  header.max_y = center_y + radius;
  if (((orig_min_x > header.max_x) || (orig_min_y > header.max_y) || (orig_max_x < header.min_x) || (orig_max_y < header.min_y)))
  {
    if (filter || transform)
    {
      read_complex = &LASreader::read_point_none;
    }
    else
    {
      read_simple = &LASreader::read_point_none;
    }
  }
  else if (filter || transform)
  {
    if (index)
    {
      if (index) index->intersect_circle(center_x, center_y, radius);
      read_complex = &LASreader::read_point_inside_circle_indexed;
    }
    else
    {
      read_complex = &LASreader::read_point_inside_circle;
    }
  }
  else
  {
    if (index)
    {
      if (index) index->intersect_circle(center_x, center_y, radius);
      read_simple = &LASreader::read_point_inside_circle_indexed;
    }
    else
    {
      read_simple = &LASreader::read_point_inside_circle;
    }
  }
  return TRUE;
}

BOOL LASreader::inside_rectangle(const F64 min_x, const F64 min_y, const F64 max_x, const F64 max_y)
{
  inside = 3;
  r_min_x = min_x;
  r_min_y = min_y;
  r_max_x = max_x;
  r_max_y = max_y;
  orig_min_x = header.min_x;
  orig_min_y = header.min_y;
  orig_max_x = header.max_x;
  orig_max_y = header.max_y;
  header.min_x = min_x;
  header.min_y = min_y;
  header.max_x = max_x;
  header.max_y = max_y;
  if (((orig_min_x > max_x) || (orig_min_y > max_y) || (orig_max_x < min_x) || (orig_max_y < min_y)))
  {
    if (filter || transform)
    {
      read_complex = &LASreader::read_point_none;
    }
    else
    {
      read_simple = &LASreader::read_point_none;
    }
  }
  else if (filter || transform)
  {
    if (index)
    {
      index->intersect_rectangle(min_x, min_y, max_x, max_y);
      read_complex = &LASreader::read_point_inside_rectangle_indexed;
    }
    else
    {
      read_complex = &LASreader::read_point_inside_rectangle;
    }
  }
  else
  {
    if (index)
    {
      index->intersect_rectangle(min_x, min_y, max_x, max_y);
      read_simple = &LASreader::read_point_inside_rectangle_indexed;
    }
    else
    {
      read_simple = &LASreader::read_point_inside_rectangle;
    }
  }
  return TRUE;
}

BOOL LASreader::read_point_inside_tile()
{
  while (read_point_default())
  {
    if (point.inside_tile(t_ll_x, t_ll_y, t_ur_x, t_ur_y)) return TRUE;
  }
  return FALSE;
}

BOOL LASreader::read_point_inside_tile_indexed()
{
  while (index->seek_next((LASreader*)this))
  {
    if (read_point_default() && point.inside_tile(t_ll_x, t_ll_y, t_ur_x, t_ur_y)) return TRUE;
  }
  return FALSE;
}

BOOL LASreader::read_point_inside_circle()
{
  while (read_point_default())
  {
    if (point.inside_circle(c_center_x, c_center_y, c_radius_squared)) return TRUE;
  }
  return FALSE;
}

BOOL LASreader::read_point_inside_circle_indexed()
{
  while (index->seek_next((LASreader*)this))
  {
    if (read_point_default() && point.inside_circle(c_center_x, c_center_y, c_radius_squared)) return TRUE;
  }
  return FALSE;
}

BOOL LASreader::read_point_inside_rectangle()
{
  while (read_point_default())
  {
    if (point.inside_rectangle(r_min_x, r_min_y, r_max_x, r_max_y)) return TRUE;
  }
  return FALSE;
}

BOOL LASreader::read_point_inside_rectangle_indexed()
{
  while (index->seek_next((LASreader*)this))
  {
    if (read_point_default() && point.inside_rectangle(r_min_x, r_min_y, r_max_x, r_max_y)) return TRUE;
  }
  return FALSE;
}

BOOL LASreader::read_point_none()
{
  return FALSE;
}

BOOL LASreader::read_point_filtered()
{
  while ((this->*read_complex)())
  {
    if (!filter->filter(&point)) return TRUE;
  }
  return FALSE;
}

BOOL LASreader::read_point_transformed()
{
  if ((this->*read_complex)())
  {
    transform->transform(&point);
    return TRUE;
  }
  return FALSE;
}

BOOL LASreader::read_point_filtered_and_transformed()
{
  if (read_point_filtered())
  {
    transform->transform(&point);
    return TRUE;
  }
  return FALSE;
}

BOOL LASreadOpener::is_piped() const
{
  return (!file_names && use_stdin);
}

BOOL LASreadOpener::is_inside() const
{
  return (inside_tile != 0 || inside_circle != 0 || inside_rectangle != 0);
}

I32 LASreadOpener::unparse(CHAR* string) const
{
  I32 n = 0;
  if (inside_tile)
  {
    n = sprintf(string, "-inside_tile %g %g %g ", inside_tile[0], inside_tile[1], inside_tile[2]);
  }
  else if (inside_circle)
  {
    n = sprintf(string, "-inside_circle %lf %lf %lf ", inside_circle[0], inside_circle[1], inside_circle[2]);
  }
  else if (inside_rectangle)
  {
    n = sprintf(string, "-inside_rectangle %lf %lf %lf %lf ", inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
  }
  if (stored)
  {
    n += sprintf(string + n, "-stored ");
  }
  if (files_are_flightlines)
  {
    if (files_are_flightlines == 1)
    {
      n += sprintf(string + n, "-faf ");
    }
    else
    {
      n += sprintf(string + n, "-faf %d ", files_are_flightlines);
    }
  }
  if (apply_file_source_ID)
  {
    n += sprintf(string + n, "-apply_file_source_ID ");
  }
  if (scale_factor)
  {
    if (scale_factor[2] == 0.0)
    {
      if ((scale_factor[0] != 0.0) && (scale_factor[1] != 0.0))
      {
        n += sprintf(string + n, "-rescale_xy %lf %lf ", scale_factor[0], scale_factor[1]);
      }
    }
    else
    {
      if ((scale_factor[0] == 0.0) && (scale_factor[1] == 0.0))
      {
        n += sprintf(string + n, "-rescale_z %lf ", scale_factor[2]);
      }
      else
      {
        n += sprintf(string + n, "-rescale %lf %lf %lf ", scale_factor[0], scale_factor[1], scale_factor[2]);
      }
    }
  }
  if (offset)
  {
    n += sprintf(string + n, "-reoffset %lf %lf %lf ", offset[0], offset[1], offset[2]);
  }
  else if (auto_reoffset)
  {
    n += sprintf(string + n, "-auto_reoffset ");
  }
  if (populate_header)
  {
    n += sprintf(string + n, "-populate ");
  }
  if (io_ibuffer_size != LAS_TOOLS_IO_IBUFFER_SIZE)
  {
    n += sprintf(string + n, "-io_ibuffer %d ", io_ibuffer_size);
  }
  if (temp_file_base)
  {
    n += sprintf(string + n, "-temp_files \"%s\" ", temp_file_base);
  }
  return n;
}

BOOL LASreadOpener::is_buffered() const
{
  return ((buffer_size > 0) && ((file_name_number > 1) || (neighbor_file_name_number > 0)));
}

BOOL LASreadOpener::is_header_populated() const
{
  return (populate_header || (file_name && (strstr(file_name, ".las") || strstr(file_name, ".laz") || strstr(file_name, ".LAS") || strstr(file_name, ".LAZ"))));
}

void LASreadOpener::reset()
{
  file_name_current = 0;
  file_name = 0;
}

LASreader* LASreadOpener::open(const CHAR* other_file_name, BOOL reset_after_other)
{
  if (filter) filter->reset();
  if (transform) transform->reset();

  if (file_names || other_file_name)
  {
    use_stdin = FALSE;
    if (file_name_current == file_name_number && other_file_name == 0) return 0;
    if ((file_name_number > 1) && merged)
    {
      LASreaderMerged* lasreadermerged = new LASreaderMerged();
      lasreadermerged->set_scale_factor(scale_factor);
      lasreadermerged->set_offset(offset);
      lasreadermerged->set_parse_string(parse_string);
      lasreadermerged->set_skip_lines(skip_lines);
      lasreadermerged->set_populate_header(populate_header);
      lasreadermerged->set_keep_lastiling(keep_lastiling);
      lasreadermerged->set_translate_intensity(translate_intensity);
      lasreadermerged->set_scale_intensity(scale_intensity);
      lasreadermerged->set_translate_scan_angle(translate_scan_angle);
      lasreadermerged->set_scale_scan_angle(scale_scan_angle);
      lasreadermerged->set_io_ibuffer_size(io_ibuffer_size);
      for (file_name_current = 0; file_name_current < file_name_number; file_name_current++) lasreadermerged->add_file_name(file_names[file_name_current]);
      if (!lasreadermerged->open())
      {
        fprintf(stderr,"ERROR: cannot open lasreadermerged with %d file names\n", file_name_number);
        delete lasreadermerged;
        return 0;
      }
      if (files_are_flightlines) lasreadermerged->set_files_are_flightlines(files_are_flightlines);
      if (apply_file_source_ID) lasreadermerged->set_apply_file_source_ID(TRUE);
      if (filter) lasreadermerged->set_filter(filter);
      if (transform) lasreadermerged->set_transform(transform);
      if (inside_tile) lasreadermerged->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
      if (inside_circle) lasreadermerged->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
      if (inside_rectangle) lasreadermerged->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
      LASreader* lasreader = 0;
      if (stored)
      {
        LASreaderStored* lasreaderstored = new LASreaderStored();
        if (!lasreaderstored->open(lasreadermerged))
        {
          fprintf(stderr, "ERROR: could not open lasreaderstored with lasreadermerged\n");
          delete lasreaderstored;
          return 0;
        }
        lasreader = lasreaderstored;
      }
      else
      {
        lasreader = lasreadermerged;
      }
      if (pipe_on)
      {
        LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn();
        if (!lasreaderpipeon->open(lasreader))
        {
          fprintf(stderr,"ERROR: cannot open lasreaderpipeon with lasreadermerged\n");
          delete lasreaderpipeon;
          return 0;
        }
        return lasreaderpipeon;
      }
      else
      {
        return lasreader;
      }
    }
    else if ((buffer_size > 0) && ((file_name_number > 1) || (neighbor_file_name_number > 0)))
    {
      U32 i;
      if (other_file_name)
      {
        file_name = other_file_name;
        if (reset_after_other)
        {
          file_name_current = 0;
        }
      }
      else
      {
        file_name = file_names[file_name_current];
        file_name_current++;
      }
      LASreaderBuffered* lasreaderbuffered = new LASreaderBuffered();
      lasreaderbuffered->set_buffer_size(buffer_size);
      lasreaderbuffered->set_scale_factor(scale_factor);
      lasreaderbuffered->set_offset(offset);
      lasreaderbuffered->set_parse_string(parse_string);
      lasreaderbuffered->set_skip_lines(skip_lines);
      lasreaderbuffered->set_populate_header(populate_header);
      lasreaderbuffered->set_translate_intensity(translate_intensity);
      lasreaderbuffered->set_scale_intensity(scale_intensity);
      lasreaderbuffered->set_translate_scan_angle(translate_scan_angle);
      lasreaderbuffered->set_scale_scan_angle(scale_scan_angle);
      lasreaderbuffered->set_file_name(file_name);
      for (i = 0; i < file_name_number; i++)
      {
        if (file_name != file_names[i])
        {
          lasreaderbuffered->add_neighbor_file_name(file_names[i]);
        }
      }
      for (i = 0; i < neighbor_file_name_number; i++)
      {
        if (strcmp(file_name, neighbor_file_names[i]))
        {
          lasreaderbuffered->add_neighbor_file_name(neighbor_file_names[i]);
        }
      }
      if (filter) lasreaderbuffered->set_filter(filter);
      if (transform) lasreaderbuffered->set_transform(transform);
      if (!lasreaderbuffered->open())
      {
        fprintf(stderr,"ERROR: cannot open lasreaderbuffered with %d file names\n", file_name_number+neighbor_file_name_number);
        delete lasreaderbuffered;
        return 0;
      }
      if (inside_tile) lasreaderbuffered->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
      if (inside_circle) lasreaderbuffered->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
      if (inside_rectangle) lasreaderbuffered->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
      LASreader* lasreader = 0;
      if (stored)
      {
        LASreaderStored* lasreaderstored = new LASreaderStored();
        if (!lasreaderstored->open(lasreaderbuffered))
        {
          fprintf(stderr, "ERROR: could not open lasreaderstored with lasreaderbuffered\n");
          delete lasreaderstored;
          return 0;
        }
        lasreader = lasreaderstored;
      }
      else
      {
        lasreader = lasreaderbuffered;
      }
      if (pipe_on)
      {
        LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn();
        if (!lasreaderpipeon->open(lasreader))
        {
          fprintf(stderr,"ERROR: cannot open lasreaderpipeon with lasreaderbuffered\n");
          delete lasreaderpipeon;
          return 0;
        }
        return lasreaderpipeon;
      }
      else
      {
        return lasreader;
      }
    }
    else
    {
      if (other_file_name)
      {
        file_name = other_file_name;
        if (reset_after_other)
        {
          file_name_current = 0;
        }
      }
      else
      {
        file_name = file_names[file_name_current];
        file_name_current++;
      }
      if (files_are_flightlines)
      {
        transform->setPointSource(file_name_current + files_are_flightlines + files_are_flightlines_index);
      }
      if (strstr(file_name, ".las") || strstr(file_name, ".laz") || strstr(file_name, ".LAS") || strstr(file_name, ".LAZ"))
      {
        LASreaderLAS* lasreaderlas;
        if (scale_factor == nullptr && offset == nullptr)
        {
          if (auto_reoffset)
            lasreaderlas = new LASreaderLASreoffset();
          else
            lasreaderlas = new LASreaderLAS();
        }
        else if (scale_factor != nullptr && offset == nullptr)
        {
          if (auto_reoffset)
            lasreaderlas = new LASreaderLASrescalereoffset(scale_factor[0], scale_factor[1], scale_factor[2]);
          else
            lasreaderlas = new LASreaderLASrescale(scale_factor[0], scale_factor[1], scale_factor[2]);
        }
        else if (scale_factor == nullptr && offset != nullptr)
          lasreaderlas = new LASreaderLASreoffset(offset[0], offset[1], offset[2]);
        else
          lasreaderlas = new LASreaderLASrescalereoffset(scale_factor[0], scale_factor[1], scale_factor[2], offset[0], offset[1], offset[2]);
        if (!lasreaderlas->open(file_name, io_ibuffer_size, FALSE, decompress_selective))
        {
          fprintf(stderr,"ERROR: cannot open lasreaderlas with file name '%s'\n", file_name);
          delete lasreaderlas;
          return 0;
        }
        LASindex* index = new LASindex();
        if (index->read(file_name))
          lasreaderlas->set_index(index);
        else
          delete index;
        if (files_are_flightlines)
        {
          lasreaderlas->header.file_source_ID = file_name_current + files_are_flightlines + files_are_flightlines_index;
        }
        else if (apply_file_source_ID)
        {
          transform->setPointSource(lasreaderlas->header.file_source_ID);
        }
        if (filter) lasreaderlas->set_filter(filter);
        if (transform) lasreaderlas->set_transform(transform);
        if (inside_rectangle) lasreaderlas->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
        else if (inside_tile) lasreaderlas->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
        else if (inside_circle) lasreaderlas->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        LASreader* lasreader = 0;
        if (stored)
        {
          LASreaderStored* lasreaderstored = new LASreaderStored();
          if (!lasreaderstored->open(lasreaderlas))
          {
            fprintf(stderr, "ERROR: could not open lasreaderstored with lasreaderlas\n");
            delete lasreaderstored;
            return 0;
          }
          lasreader = lasreaderstored;
        }
        else
        {
          lasreader = lasreaderlas;
        }
        if (pipe_on)
        {
          LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn();
          if (!lasreaderpipeon->open(lasreader))
          {
            fprintf(stderr,"ERROR: cannot open lasreaderpipeon with lasreaderlas\n");
            delete lasreaderpipeon;
            return 0;
          }
          return lasreaderpipeon;
        }
        else
        {
          return lasreader;
        }
      }
      else if (strstr(file_name, ".bin") || strstr(file_name, ".BIN"))
      {
        LASreaderBIN* lasreaderbin;
        if (scale_factor == 0 && offset == 0)
          lasreaderbin = new LASreaderBIN();
        else if (scale_factor != 0 && offset == 0)
          lasreaderbin = new LASreaderBINrescale(scale_factor[0], scale_factor[1], scale_factor[2]);
        else if (scale_factor == 0 && offset != 0)
          lasreaderbin = new LASreaderBINreoffset(offset[0], offset[1], offset[2]);
        else
          lasreaderbin = new LASreaderBINrescalereoffset(scale_factor[0], scale_factor[1], scale_factor[2], offset[0], offset[1], offset[2]);
        if (!lasreaderbin->open(file_name))
        {
          fprintf(stderr,"ERROR: cannot open lasreaderbin with file name '%s'\n", file_name);
          delete lasreaderbin;
          return 0;
        }
        LASindex* index = new LASindex();
        if (index->read(file_name))
          lasreaderbin->set_index(index);
        else
          delete index;
        if (files_are_flightlines) lasreaderbin->header.file_source_ID = file_name_current + files_are_flightlines + files_are_flightlines_index;
        if (filter) lasreaderbin->set_filter(filter);
        if (transform) lasreaderbin->set_transform(transform);
        if (inside_tile) lasreaderbin->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
        if (inside_circle) lasreaderbin->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        if (inside_rectangle) lasreaderbin->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
        LASreader* lasreader = 0;
        if (stored)
        {
          LASreaderStored* lasreaderstored = new LASreaderStored();
          if (!lasreaderstored->open(lasreaderbin))
          {
            fprintf(stderr, "ERROR: could not open lasreaderstored with lasreaderbin\n");
            delete lasreaderstored;
            return 0;
          }
          lasreader = lasreaderstored;
        }
        else
        {
          lasreader = lasreaderbin;
        }
        if (pipe_on)
        {
          LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn();
          if (!lasreaderpipeon->open(lasreader))
          {
            fprintf(stderr,"ERROR: cannot open lasreaderpipeon with lasreaderbin\n");
            delete lasreaderpipeon;
            return 0;
          }
          return lasreaderpipeon;
        }
        else
        {
          return lasreader;
        }
      }
      else if (strstr(file_name, ".shp") || strstr(file_name, ".SHP"))
      {
        LASreaderSHP* lasreadershp;
        if (scale_factor == 0 && offset == 0)
          lasreadershp = new LASreaderSHP();
        else if (scale_factor != 0 && offset == 0)
          lasreadershp = new LASreaderSHPrescale(scale_factor[0], scale_factor[1], scale_factor[2]);
        else if (scale_factor == 0 && offset != 0)
          lasreadershp = new LASreaderSHPreoffset(offset[0], offset[1], offset[2]);
        else
          lasreadershp = new LASreaderSHPrescalereoffset(scale_factor[0], scale_factor[1], scale_factor[2], offset[0], offset[1], offset[2]);
        if (!lasreadershp->open(file_name))
        {
          fprintf(stderr,"ERROR: cannot open lasreadershp with file name '%s'\n", file_name);
          delete lasreadershp;
          return 0;
        }
        if (files_are_flightlines) lasreadershp->header.file_source_ID = file_name_current + files_are_flightlines + files_are_flightlines_index;
        if (filter) lasreadershp->set_filter(filter);
        if (transform) lasreadershp->set_transform(transform);
        if (inside_tile) lasreadershp->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
        if (inside_circle) lasreadershp->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        if (inside_rectangle) lasreadershp->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
        LASreader* lasreader = 0;
        if (stored)
        {
          LASreaderStored* lasreaderstored = new LASreaderStored();
          if (!lasreaderstored->open(lasreadershp))
          {
            fprintf(stderr, "ERROR: could not open lasreaderstored with lasreadershp\n");
            delete lasreaderstored;
            return 0;
          }
          lasreader = lasreaderstored;
        }
        else
        {
          lasreader = lasreadershp;
        }
        if (pipe_on)
        {
          LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn();
          if (!lasreaderpipeon->open(lasreader))
          {
            fprintf(stderr,"ERROR: cannot open lasreaderpipeon with lasreadershp\n");
            delete lasreaderpipeon;
            return 0;
          }
          return lasreaderpipeon;
        }
        else
        {
          return lasreader;
        }
      }
      else if (strstr(file_name, ".asc") || strstr(file_name, ".ASC"))
      {
        LASreaderASC* lasreaderasc;
        if (scale_factor == 0 && offset == 0)
          lasreaderasc = new LASreaderASC();
        else if (scale_factor != 0 && offset == 0)
          lasreaderasc = new LASreaderASCrescale(scale_factor[0], scale_factor[1], scale_factor[2]);
        else if (scale_factor == 0 && offset != 0)
          lasreaderasc = new LASreaderASCreoffset(offset[0], offset[1], offset[2]);
        else
          lasreaderasc = new LASreaderASCrescalereoffset(scale_factor[0], scale_factor[1], scale_factor[2], offset[0], offset[1], offset[2]);
        if (!lasreaderasc->open(file_name, comma_not_point))
        {
          fprintf(stderr,"ERROR: cannot open lasreaderasc with file name '%s'\n", file_name);
          delete lasreaderasc;
          return 0;
        }
        if (files_are_flightlines) lasreaderasc->header.file_source_ID = file_name_current + files_are_flightlines + files_are_flightlines_index;
        if (filter) lasreaderasc->set_filter(filter);
        if (transform) lasreaderasc->set_transform(transform);
        if (inside_tile) lasreaderasc->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
        if (inside_circle) lasreaderasc->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        if (inside_rectangle) lasreaderasc->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
        LASreader* lasreader = 0;
        if (stored)
        {
          LASreaderStored* lasreaderstored = new LASreaderStored();
          if (!lasreaderstored->open(lasreaderasc))
          {
            fprintf(stderr, "ERROR: could not open lasreaderstored with lasreaderasc\n");
            delete lasreaderstored;
            return 0;
          }
          lasreader = lasreaderstored;
        }
        else
        {
          lasreader = lasreaderasc;
        }
        if (pipe_on)
        {
          LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn();
          if (!lasreaderpipeon->open(lasreader))
          {
            fprintf(stderr,"ERROR: cannot open lasreaderpipeon with lasreaderasc\n");
            delete lasreaderpipeon;
            return 0;
          }
          return lasreaderpipeon;
        }
        else
        {
          return lasreader;
        }
      }
      else if (strstr(file_name, ".bil") || strstr(file_name, ".BIL"))
      {
        LASreaderBIL* lasreaderbil;
        if (scale_factor == 0 && offset == 0)
          lasreaderbil = new LASreaderBIL();
        else if (scale_factor != 0 && offset == 0)
          lasreaderbil = new LASreaderBILrescale(scale_factor[0], scale_factor[1], scale_factor[2]);
        else if (scale_factor == 0 && offset != 0)
          lasreaderbil = new LASreaderBILreoffset(offset[0], offset[1], offset[2]);
        else
          lasreaderbil = new LASreaderBILrescalereoffset(scale_factor[0], scale_factor[1], scale_factor[2], offset[0], offset[1], offset[2]);
        if (!lasreaderbil->open(file_name))
        {
          fprintf(stderr,"ERROR: cannot open lasreaderbil with file name '%s'\n", file_name);
          delete lasreaderbil;
          return 0;
        }
        if (files_are_flightlines) lasreaderbil->header.file_source_ID = file_name_current + files_are_flightlines + files_are_flightlines_index;
        if (filter) lasreaderbil->set_filter(filter);
        if (transform) lasreaderbil->set_transform(transform);
        if (inside_tile) lasreaderbil->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
        if (inside_circle) lasreaderbil->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        if (inside_rectangle) lasreaderbil->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
        LASreader* lasreader = 0;
        if (stored)
        {
          LASreaderStored* lasreaderstored = new LASreaderStored();
          if (!lasreaderstored->open(lasreaderbil))
          {
            fprintf(stderr, "ERROR: could not open lasreaderstored with lasreaderbil\n");
            delete lasreaderstored;
            return 0;
          }
          lasreader = lasreaderstored;
        }
        else
        {
          lasreader = lasreaderbil;
        }
        if (pipe_on)
        {
          LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn();
          if (!lasreaderpipeon->open(lasreader))
          {
            fprintf(stderr,"ERROR: cannot open lasreaderpipeon with lasreaderbil\n");
            delete lasreaderpipeon;
            return 0;
          }
          return lasreaderpipeon;
        }
        else
        {
          return lasreader;
        }
      }
      else if (strstr(file_name, ".dtm") || strstr(file_name, ".DTM"))
      {
        LASreaderDTM* lasreaderdtm;
        if (scale_factor == 0 && offset == 0)
          lasreaderdtm = new LASreaderDTM();
        else if (scale_factor != 0 && offset == 0)
          lasreaderdtm = new LASreaderDTMrescale(scale_factor[0], scale_factor[1], scale_factor[2]);
        else if (scale_factor == 0 && offset != 0)
          lasreaderdtm = new LASreaderDTMreoffset(offset[0], offset[1], offset[2]);
        else
          lasreaderdtm = new LASreaderDTMrescalereoffset(scale_factor[0], scale_factor[1], scale_factor[2], offset[0], offset[1], offset[2]);
        if (!lasreaderdtm->open(file_name))
        {
          fprintf(stderr,"ERROR: cannot open lasreaderdtm with file name '%s'\n", file_name);
          delete lasreaderdtm;
          return 0;
        }
        if (files_are_flightlines) lasreaderdtm->header.file_source_ID = file_name_current + files_are_flightlines + files_are_flightlines_index;
        if (filter) lasreaderdtm->set_filter(filter);
        if (transform) lasreaderdtm->set_transform(transform);
        if (inside_tile) lasreaderdtm->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
        if (inside_circle) lasreaderdtm->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        if (inside_rectangle) lasreaderdtm->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
        LASreader* lasreader = 0;
        if (stored)
        {
          LASreaderStored* lasreaderstored = new LASreaderStored();
          if (!lasreaderstored->open(lasreaderdtm))
          {
            fprintf(stderr, "ERROR: could not open lasreaderstored with lasreaderdtm\n");
            delete lasreaderstored;
            return 0;
          }
          lasreader = lasreaderstored;
        }
        else
        {
          lasreader = lasreaderdtm;
        }
        if (pipe_on)
        {
          LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn();
          if (!lasreaderpipeon->open(lasreader))
          {
            fprintf(stderr,"ERROR: cannot open lasreaderpipeon with lasreaderdtm\n");
            delete lasreaderpipeon;
            return 0;
          }
          return lasreaderpipeon;
        }
        else
        {
          return lasreader;
        }
      }
      else if (strstr(file_name, ".ply") || strstr(file_name, ".PLY"))
      {
        LASreaderPLY* lasreaderply = new LASreaderPLY();
        if (translate_intensity != 0.0f) lasreaderply->set_translate_intensity(translate_intensity);
        if (scale_intensity != 1.0f) lasreaderply->set_scale_intensity(scale_intensity);
        lasreaderply->set_scale_factor(scale_factor);
        lasreaderply->set_offset(offset);
        if (!lasreaderply->open(file_name, point_type, populate_header))
        {
          fprintf(stderr,"ERROR: cannot open lasreaderply with file name '%s'\n", file_name);
          delete lasreaderply;
          return 0;
        }
        if (files_are_flightlines) lasreaderply->header.file_source_ID = file_name_current + files_are_flightlines + files_are_flightlines_index;
        if (filter) lasreaderply->set_filter(filter);
        if (transform) lasreaderply->set_transform(transform);
        if (inside_tile) lasreaderply->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
        if (inside_circle) lasreaderply->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        if (inside_rectangle) lasreaderply->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
        LASreader* lasreader = 0;
        if (stored)
        {
          LASreaderStored* lasreaderstored = new LASreaderStored();
          if (!lasreaderstored->open(lasreaderply))
          {
            fprintf(stderr, "ERROR: could not open lasreaderstored with lasreaderply\n");
            delete lasreaderstored;
            return 0;
          }
          lasreader = lasreaderstored;
        }
        else
        {
          lasreader = lasreaderply;
        }
        if (pipe_on)
        {
          LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn();
          if (!lasreaderpipeon->open(lasreader))
          {
            fprintf(stderr,"ERROR: cannot open lasreaderpipeon with lasreaderply\n");
            delete lasreaderpipeon;
            return 0;
          }
          return lasreaderpipeon;
        }
        else
        {
          return lasreader;
        }
      }
      else if (strstr(file_name, ".qi") || strstr(file_name, ".QI"))
      {
        LASreaderQFIT* lasreaderqfit;
        if (scale_factor == 0 && offset == 0)
          lasreaderqfit = new LASreaderQFIT();
        else if (scale_factor != 0 && offset == 0)
          lasreaderqfit = new LASreaderQFITrescale(scale_factor[0], scale_factor[1], scale_factor[2]);
        else if (scale_factor == 0 && offset != 0)
          lasreaderqfit = new LASreaderQFITreoffset(offset[0], offset[1], offset[2]);
        else
          lasreaderqfit = new LASreaderQFITrescalereoffset(scale_factor[0], scale_factor[1], scale_factor[2], offset[0], offset[1], offset[2]);
        if (!lasreaderqfit->open(file_name))
        {
          fprintf(stderr,"ERROR: cannot open lasreaderqfit with file name '%s'\n", file_name);
          delete lasreaderqfit;
          return 0;
        }
        LASindex* index = new LASindex();
        if (index->read(file_name))
          lasreaderqfit->set_index(index);
        else
          delete index;
        if (files_are_flightlines) lasreaderqfit->header.file_source_ID = file_name_current + files_are_flightlines + files_are_flightlines_index;
        if (filter) lasreaderqfit->set_filter(filter);
        if (transform) lasreaderqfit->set_transform(transform);
        if (inside_tile) lasreaderqfit->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
        if (inside_circle) lasreaderqfit->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        if (inside_rectangle) lasreaderqfit->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
        LASreader* lasreader = 0;
        if (stored)
        {
          LASreaderStored* lasreaderstored = new LASreaderStored();
          if (!lasreaderstored->open(lasreaderqfit))
          {
            fprintf(stderr, "ERROR: could not open lasreaderstored with lasreaderqfit\n");
            delete lasreaderstored;
            return 0;
          }
          lasreader = lasreaderstored;
        }
        else
        {
          lasreader = lasreaderqfit;
        }
        if (pipe_on)
        {
          LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn();
          if (!lasreaderpipeon->open(lasreader))
          {
            fprintf(stderr,"ERROR: cannot open lasreaderpipeon with lasreaderqfit\n");
            delete lasreaderpipeon;
            return 0;
          }
          return lasreaderpipeon;
        }
        else
        {
          return lasreader;
        }
      }
      else
      {
        LASreaderTXT* lasreadertxt = new LASreaderTXT();
        if (ipts) lasreadertxt->set_pts(TRUE);
        else if (iptx) lasreadertxt->set_ptx(TRUE);
        if (translate_intensity != 0.0f) lasreadertxt->set_translate_intensity(translate_intensity);
        if (scale_intensity != 1.0f) lasreadertxt->set_scale_intensity(scale_intensity);
        if (translate_scan_angle != 0.0f) lasreadertxt->set_translate_scan_angle(translate_scan_angle);
        if (scale_scan_angle != 1.0f) lasreadertxt->set_scale_scan_angle(scale_scan_angle);
        lasreadertxt->set_scale_factor(scale_factor);
        lasreadertxt->set_offset(offset);
        if (number_attributes)
        {
          for (I32 i = 0; i < number_attributes; i++)
          {
            lasreadertxt->add_attribute(attribute_data_types[i], attribute_names[i], attribute_descriptions[i], attribute_scales[i], attribute_offsets[i], attribute_pre_scales[i], attribute_pre_offsets[i], attribute_no_datas[i]);
          }
        }
        if (!lasreadertxt->open(file_name, point_type, parse_string, skip_lines, populate_header))
        {
          fprintf(stderr,"ERROR: cannot open lasreadertxt with file name '%s'\n", file_name);
          delete lasreadertxt;
          return 0;
        }
        if (files_are_flightlines) lasreadertxt->header.file_source_ID = file_name_current + files_are_flightlines + files_are_flightlines_index;
        if (filter) lasreadertxt->set_filter(filter);
        if (transform) lasreadertxt->set_transform(transform);
        if (inside_tile) lasreadertxt->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
        if (inside_circle) lasreadertxt->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        if (inside_rectangle) lasreadertxt->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
        LASreader* lasreader = 0;
        if (stored)
        {
          LASreaderStored* lasreaderstored = new LASreaderStored();
          if (!lasreaderstored->open(lasreadertxt))
          {
            fprintf(stderr, "ERROR: could not open lasreaderstored with lasreadertxt\n");
            delete lasreaderstored;
            return 0;
          }
          lasreader = lasreaderstored;
        }
        else
        {
          lasreader = lasreadertxt;
        }
        if (pipe_on)
        {
          LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn();
          if (!lasreaderpipeon->open(lasreader))
          {
            fprintf(stderr,"ERROR: cannot open lasreaderpipeon with lasreadertxt\n");
            delete lasreaderpipeon;
            return 0;
          }
          return lasreaderpipeon;
        }
        else
        {
          return lasreader;
        }
      }
    }
  }
  else if (use_stdin)
  {
    use_stdin = FALSE; populate_header = TRUE;
    if (itxt)
    {
      LASreaderTXT* lasreadertxt = new LASreaderTXT();
      if (ipts) lasreadertxt->set_pts(TRUE);
      else if (iptx) lasreadertxt->set_ptx(TRUE);
      if (translate_intensity != 0.0f) lasreadertxt->set_translate_intensity(translate_intensity);
      if (scale_intensity != 1.0f) lasreadertxt->set_scale_intensity(scale_intensity);
      if (translate_scan_angle != 0.0f) lasreadertxt->set_translate_scan_angle(translate_scan_angle);
      if (scale_scan_angle != 1.0f) lasreadertxt->set_scale_scan_angle(scale_scan_angle);
      lasreadertxt->set_scale_factor(scale_factor);
      lasreadertxt->set_offset(offset);
      if (number_attributes)
      {
        for (I32 i = 0; i < number_attributes; i++)
        {
          lasreadertxt->add_attribute(attribute_data_types[i], attribute_names[i], attribute_descriptions[i], attribute_scales[i], attribute_offsets[i], attribute_pre_scales[i], attribute_pre_offsets[i], attribute_no_datas[i]);
        }
      }
      if (!lasreadertxt->open(stdin, 0, point_type, parse_string, skip_lines, FALSE))
      {
        fprintf(stderr,"ERROR: cannot open lasreadertxt with file name '%s'\n", file_name);
        delete lasreadertxt;
        return 0;
      }
      if (files_are_flightlines) lasreadertxt->header.file_source_ID = file_name_current + files_are_flightlines + files_are_flightlines_index;
      if (filter) lasreadertxt->set_filter(filter);
      if (transform) lasreadertxt->set_transform(transform);
      if (inside_tile) lasreadertxt->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
      if (inside_circle) lasreadertxt->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
      if (inside_rectangle) lasreadertxt->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
      LASreader* lasreader = 0;
      if (stored)
      {
        LASreaderStored* lasreaderstored = new LASreaderStored();
        if (!lasreaderstored->open(lasreadertxt))
        {
          fprintf(stderr, "ERROR: could not open lasreaderstored with lasreadertxt\n");
          delete lasreaderstored;
          return 0;
        }
        lasreader = lasreaderstored;
      }
      else
      {
        lasreader = lasreadertxt;
      }
      if (pipe_on)
      {
        LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn();
        if (!lasreaderpipeon->open(lasreader))
        {
          fprintf(stderr,"ERROR: cannot open lasreaderpipeon with lasreadertxt\n");
          delete lasreaderpipeon;
          return 0;
        }
        return lasreaderpipeon;
      }
      else
      {
        return lasreader;
      }
    }
    else
    {
      LASreaderLAS* lasreaderlas;
      if (scale_factor == 0 && offset == 0)
        lasreaderlas = new LASreaderLAS();
      else if (scale_factor != 0 && offset == 0)
        lasreaderlas = new LASreaderLASrescale(scale_factor[0], scale_factor[1], scale_factor[2]);
      else if (scale_factor == 0 && offset != 0)
        lasreaderlas = new LASreaderLASreoffset(offset[0], offset[1], offset[2]);
      else
        lasreaderlas = new LASreaderLASrescalereoffset(scale_factor[0], scale_factor[1], scale_factor[2], offset[0], offset[1], offset[2]);
      if (!lasreaderlas->open(stdin))
      {
        fprintf(stderr,"ERROR: cannot open lasreaderlas from stdin \n");
        delete lasreaderlas;
        return 0;
      }
      if (filter) lasreaderlas->set_filter(filter);
      if (transform) lasreaderlas->set_transform(transform);
      if (inside_tile) lasreaderlas->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
      if (inside_circle) lasreaderlas->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
      if (inside_rectangle) lasreaderlas->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
      LASreader* lasreader = 0;
      if (stored)
      {
        LASreaderStored* lasreaderstored = new LASreaderStored();
        if (!lasreaderstored->open(lasreaderlas))
        {
          fprintf(stderr, "ERROR: could not open lasreaderstored with lasreaderlas\n");
          delete lasreaderstored;
          return 0;
        }
        lasreader = lasreaderstored;
      }
      else
      {
        lasreader = lasreaderlas;
      }
      if (pipe_on)
      {
        LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn();
        if (!lasreaderpipeon->open(lasreader))
        {
          fprintf(stderr,"ERROR: cannot open lasreaderpipeon with lasreaderlas from stdin\n");
          delete lasreaderpipeon;
          return 0;
        }
        return lasreaderpipeon;
      }
      else
      {
        return lasreader;
      }
    }
  }
  else
  {
    return 0;
  }
}

BOOL LASreadOpener::reopen(LASreader* lasreader, BOOL remain_buffered)
{
  if (lasreader == 0)
  {
    fprintf(stderr,"ERROR: pointer to LASreader is NULL\n");
  }

  // make sure the LASreader was closed

  lasreader->close();

  if (filter) filter->reset();
  if (transform) transform->reset();

  if (pipe_on)
  {
    LASreaderPipeOn* lasreaderpipeon = (LASreaderPipeOn*)lasreader;
    lasreaderpipeon->p_count = 0;
    lasreader = lasreaderpipeon->get_lasreader();
  }

  if (stored)
  {
    LASreaderStored* lasreaderstored = (LASreaderStored*)lasreader;
    if (!lasreaderstored->reopen())
    {
      fprintf(stderr, "ERROR: could not reopen lasreaderstored for stored input\n");
      return FALSE;
    }
    return TRUE;
  }
  else if (file_names)
  {
    if ((file_name_number > 1) && merged)
    {
      LASreaderMerged* lasreadermerged = (LASreaderMerged*)lasreader;
      if (!lasreadermerged->reopen())
      {
        fprintf(stderr,"ERROR: cannot reopen lasreadermerged\n");
        return FALSE;
      }
      if (inside_rectangle || inside_tile || inside_circle)
      {
        lasreadermerged->inside_none();
        if (inside_rectangle) lasreadermerged->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
        else if (inside_tile) lasreadermerged->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
        else lasreadermerged->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
      }
      return TRUE;
    }
    else if ((buffer_size > 0) && ((file_name_number > 1) || (neighbor_file_name_number > 0)))
    {
      LASreaderBuffered* lasreaderbuffered = (LASreaderBuffered*)lasreader;
      if (!lasreaderbuffered->reopen())
      {
        fprintf(stderr,"ERROR: cannot reopen lasreaderbuffered\n");
        return FALSE;
      }
      if (inside_rectangle || inside_tile || inside_circle)
      {
        lasreaderbuffered->inside_none();
        if (inside_rectangle) lasreaderbuffered->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
        else if (inside_tile) lasreaderbuffered->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
        else lasreaderbuffered->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
      }
      if (!remain_buffered) lasreaderbuffered->remove_buffer();
      return TRUE;
    }
    else
    {
      if (!file_name) return FALSE;
      if (strstr(file_name, ".las") || strstr(file_name, ".laz") || strstr(file_name, ".LAS") || strstr(file_name, ".LAZ"))
      {
        LASreaderLAS* lasreaderlas = (LASreaderLAS*)lasreader;
        if (!lasreaderlas->open(file_name, io_ibuffer_size, FALSE, decompress_selective))
        {
          fprintf(stderr,"ERROR: cannot reopen lasreaderlas with file name '%s'\n", file_name);
          return FALSE;
        }
        if (inside_rectangle || inside_tile || inside_circle)
        {
          lasreaderlas->inside_none();
          if (inside_rectangle) lasreaderlas->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
          else if (inside_tile) lasreaderlas->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
          else lasreaderlas->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        }
        return TRUE;
      }
      else if (strstr(file_name, ".bin") || strstr(file_name, ".BIN"))
      {
        LASreaderBIN* lasreaderbin = (LASreaderBIN*)lasreader;
        if (!lasreaderbin->open(file_name))
        {
          fprintf(stderr,"ERROR: cannot reopen lasreaderbin with file name '%s'\n", file_name);
          return FALSE;
        }
        if (inside_rectangle || inside_tile || inside_circle)
        {
          lasreaderbin->inside_none();
          if (inside_rectangle) lasreaderbin->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
          else if (inside_tile) lasreaderbin->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
          else lasreaderbin->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        }
        return TRUE;
      }
      else if (strstr(file_name, ".shp") || strstr(file_name, ".SHP"))
      {
        LASreaderSHP* lasreadershp = (LASreaderSHP*)lasreader;
        if (!lasreadershp->reopen(file_name))
        {
          fprintf(stderr,"ERROR: cannot reopen lasreadershp with file name '%s'\n", file_name);
          return FALSE;
        }
        if (inside_rectangle || inside_tile || inside_circle)
        {
          lasreadershp->inside_none();
          if (inside_rectangle) lasreadershp->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
          else if (inside_tile) lasreadershp->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
          else lasreadershp->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        }
        return TRUE;
      }
      else if (strstr(file_name, ".qi") || strstr(file_name, ".QI"))
      {
        LASreaderQFIT* lasreaderqfit = (LASreaderQFIT*)lasreader;
        if (!lasreaderqfit->reopen(file_name))
        {
          fprintf(stderr,"ERROR: cannot reopen lasreaderqfit with file name '%s'\n", file_name);
          return FALSE;
        }
        if (inside_rectangle || inside_tile || inside_circle)
        {
          lasreaderqfit->inside_none();
          if (inside_rectangle) lasreaderqfit->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
          else if (inside_tile) lasreaderqfit->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
          else lasreaderqfit->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        }
        return TRUE;
      }
      else if (strstr(file_name, ".asc") || strstr(file_name, ".ASC"))
      {
        LASreaderASC* lasreaderasc = (LASreaderASC*)lasreader;
        if (!lasreaderasc->reopen(file_name))
        {
          fprintf(stderr,"ERROR: cannot reopen lasreaderasc with file name '%s'\n", file_name);
          return FALSE;
        }
        if (inside_rectangle || inside_tile || inside_circle)
        {
          lasreaderasc->inside_none();
          if (inside_rectangle) lasreaderasc->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
          else if (inside_tile) lasreaderasc->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
          else lasreaderasc->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        }
        return TRUE;
      }
      else if (strstr(file_name, ".bil") || strstr(file_name, ".BIL"))
      {
        LASreaderBIL* lasreaderbil = (LASreaderBIL*)lasreader;
        if (!lasreaderbil->reopen(file_name))
        {
          fprintf(stderr,"ERROR: cannot reopen lasreaderbil with file name '%s'\n", file_name);
          return FALSE;
        }
        if (inside_rectangle || inside_tile || inside_circle)
        {
          lasreaderbil->inside_none();
          if (inside_rectangle) lasreaderbil->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
          else if (inside_tile) lasreaderbil->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
          else lasreaderbil->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        }
        return TRUE;
      }
      else if (strstr(file_name, ".dtm") || strstr(file_name, ".DTM"))
      {
        LASreaderDTM* lasreaderdtm = (LASreaderDTM*)lasreader;
        if (!lasreaderdtm->reopen(file_name))
        {
          fprintf(stderr,"ERROR: cannot reopen lasreaderdtm with file name '%s'\n", file_name);
          return FALSE;
        }
        if (inside_rectangle || inside_tile || inside_circle)
        {
          lasreaderdtm->inside_none();
          if (inside_rectangle) lasreaderdtm->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
          else if (inside_tile) lasreaderdtm->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
          else lasreaderdtm->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        }
        return TRUE;
      }
      else
      {
        LASreaderTXT* lasreadertxt = (LASreaderTXT*)lasreader;
        if (!lasreadertxt->reopen(file_name))
        {
          fprintf(stderr,"ERROR: cannot reopen lasreadertxt with file name '%s'\n", file_name);
          return FALSE;
        }
        if (inside_rectangle || inside_tile || inside_circle)
        {
          lasreadertxt->inside_none();
          if (inside_rectangle) lasreadertxt->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
          else if (inside_tile) lasreadertxt->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
          else lasreadertxt->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
        }
        return TRUE;
      }
    }
  }
  else
  {
    fprintf(stderr,"ERROR: no lasreader input specified\n");
    return FALSE;
  }
}

LASwaveform13reader* LASreadOpener::open_waveform13(const LASheader* lasheader)
{
  if (lasheader->point_data_format < 4) return nullptr;
  if ((lasheader->point_data_format > 5) && (lasheader->point_data_format < 9)) return nullptr;
  if (lasheader->vlr_wave_packet_descr == nullptr) return nullptr;
  if (get_file_name() == nullptr) return nullptr;
  LASwaveform13reader* waveform13reader = new LASwaveform13reader();
  if ((lasheader->global_encoding & 2) && (lasheader->start_of_waveform_data_packet_record > lasheader->offset_to_point_data))
  {
    if (waveform13reader->open(get_file_name(), lasheader->start_of_waveform_data_packet_record, lasheader->vlr_wave_packet_descr))
    {
      return waveform13reader;
    }
  }
  else
  {
    if (waveform13reader->open(get_file_name(), 0, lasheader->vlr_wave_packet_descr))
    {
      return waveform13reader;
    }
  }
  delete waveform13reader;
  return nullptr;
}

void LASreadOpener::usage() const
{
  fprintf(stderr,"Supported LAS Inputs\n");
  fprintf(stderr,"  -i lidar.las\n");
  fprintf(stderr,"  -i lidar.laz\n");
  fprintf(stderr,"  -i lidar1.las lidar2.las lidar3.las -merged\n");
  fprintf(stderr,"  -i *.las - merged\n");
  fprintf(stderr,"  -i flight0??.laz flight1??.laz\n");
  fprintf(stderr,"  -i terrasolid.bin\n");
  fprintf(stderr,"  -i esri.shp\n");
  fprintf(stderr,"  -i nasa.qi\n");
  fprintf(stderr,"  -i lidar.txt -iparse xyzti -iskip 2 (on-the-fly from ASCII)\n");
  fprintf(stderr,"  -i lidar.txt -iparse xyzi -itranslate_intensity 1024\n");
  fprintf(stderr,"  -lof file_list.txt\n");
  fprintf(stderr,"  -stdin (pipe from stdin)\n");
  fprintf(stderr,"  -rescale 0.01 0.01 0.001\n");
  fprintf(stderr,"  -rescale_xy 0.01 0.01\n");
  fprintf(stderr,"  -rescale_z 0.01\n");
  fprintf(stderr,"  -reoffset 600000 4000000 0\n");
  fprintf(stderr,"Fast AOI Queries for LAS/LAZ with spatial indexing LAX files\n");
  fprintf(stderr,"  -inside min_x min_y max_x max_y\n");
  fprintf(stderr,"  -inside_tile ll_x ll_y size\n");
  fprintf(stderr,"  -inside_circle center_x center_y radius\n");
}

BOOL LASreadOpener::parse(int argc, char* argv[])
{
  int i;
  for (i = 1; i < argc; i++)
  {
    if (argv[i][0] == '\0')
    {
      continue;
    }
    else if (strcmp(argv[i],"-h") == 0)
    {
      LASfilter().usage();
      LAStransform().usage();
      usage();
      return TRUE;
    }
    else if (strcmp(argv[i],"-i") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs at least 1 argument: file_name or wild_card\n", argv[i]);
        return FALSE;
      }
      *argv[i]='\0';
      i+=1;
      do
      {
        add_file_name(argv[i], unique);
        *argv[i]='\0';
        i+=1;
      } while ((i < argc) && (*argv[i] != '-') && (*argv[i] != '\0'));
      i-=1;
    }
    else if (strcmp(argv[i],"-unique") == 0)
    {
      unique = TRUE;
      *argv[i]='\0';
    }
    else if (strncmp(argv[i],"-inside", 7) == 0)
    {
      if (strcmp(argv[i],"-inside_tile") == 0)
      {
        if ((i+3) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 3 arguments: ll_x, ll_y, size\n", argv[i]);
          return FALSE;
        }
        set_inside_tile((F32)atof(argv[i+1]), (F32)atof(argv[i+2]), (F32)atof(argv[i+3]));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; i+=3; 
      }
      else if (strcmp(argv[i],"-inside_circle") == 0)
      {
        if ((i+3) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 3 arguments: center_x, center_y, radius\n", argv[i]);
          return FALSE;
        }
        set_inside_circle(atof(argv[i+1]), atof(argv[i+2]), atof(argv[i+3]));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; i+=3;
      }
      else if (strcmp(argv[i],"-inside") == 0 || strcmp(argv[i],"-inside_rectangle") == 0)
      {
        if ((i+4) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 4 arguments: min_x, min_y, max_x, max_y\n", argv[i]);
          return FALSE;
        }
        set_inside_rectangle(atof(argv[i+1]), atof(argv[i+2]), atof(argv[i+3]), atof(argv[i+4]));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; *argv[i+4]='\0'; i+=4; 
      }
      else
      {
        fprintf(stderr,"ERROR: unknown '-inside' option '%s'\n", argv[i]);
        return FALSE;
      }
    }
    else if (strcmp(argv[i],"-comma_not_point") == 0)
    {
      comma_not_point = TRUE;
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-stdin") == 0)
    {
      use_stdin = TRUE;
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-lof") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: list_of_files\n", argv[i]);
        return FALSE;
      }
      if (!add_list_of_files(argv[i+1]), unique)
      {
        fprintf(stderr, "ERROR: cannot load list of files '%s'\n", argv[i+1]);
        return FALSE;
      }
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-rescale") == 0)
    {
      if ((i+3) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 3 arguments: rescale_x rescale_y rescale_z\n", argv[i]);
        return FALSE;
      }
      F64 scale_factor[3];
      scale_factor[0] = atof(argv[i+1]);
      scale_factor[1] = atof(argv[i+2]);
      scale_factor[2] = atof(argv[i+3]);
      set_scale_factor(scale_factor);
      *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; i+=3;
    }
    else if (strcmp(argv[i],"-rescale_xy") == 0)
    {
      if ((i+2) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 2 argument: rescale_x rescale_y\n", argv[i]);
        return FALSE;
      }
      F64 scale_factor[3];
      scale_factor[0] = atof(argv[i+1]);
      scale_factor[1] = atof(argv[i+2]);
      scale_factor[2] = 0;
      set_scale_factor(scale_factor);
      *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
    }
    else if (strcmp(argv[i],"-rescale_z") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: scale\n", argv[i]);
        return FALSE;
      }
      F64 scale_factor[3];
      scale_factor[0] = 0;
      scale_factor[1] = 0;
      scale_factor[2] = atof(argv[i+1]);
      set_scale_factor(scale_factor);
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-reoffset") == 0)
    {
      if ((i+3) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 3 arguments: reoffset_x, reoffset_y, reoffset_z\n", argv[i]);
        return FALSE;
      }
      F64 offset[3];
			offset[0] = atof(argv[i+1]);
			offset[1] = atof(argv[i+2]);
			offset[2] = atof(argv[i+3]);
      set_offset(offset);
      *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; i+=3;
    }
    else if (strcmp(argv[i],"-auto_reoffset") == 0)
    {
      set_auto_reoffset(TRUE);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-files_are_flightlines") == 0 || strcmp(argv[i],"-faf") == 0)
    {
      if (((i+1) < argc) && ('1' <= argv[i+1][0]) && (argv[i+1][0] <= '9'))
      {
        set_files_are_flightlines(atoi(argv[i+1]));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
      }
      else
      {
        set_files_are_flightlines(1);
        *argv[i]='\0';
      }
    }
    else if (strcmp(argv[i],"-faf_index") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: index\n", argv[i]);
        return FALSE;
      }
      set_files_are_flightlines_index(atoi(argv[i+1]));
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-apply_file_source_ID") == 0)
    {
      set_apply_file_source_ID(TRUE);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-itranslate_intensity") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: offset\n", argv[i]);
        return FALSE;
      }
      set_translate_intensity((F32)atof(argv[i+1]));
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-iscale_intensity") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: scale\n", argv[i]);
        return FALSE;
      }
      set_scale_intensity((F32)atof(argv[i+1]));
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-itranslate_scan_angle") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: offset\n", argv[i]);
        return FALSE;
      }
      set_translate_scan_angle((F32)atof(argv[i+1]));
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-iscale_scan_angle") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: scale\n", argv[i]);
        return FALSE;
      }
      set_scale_scan_angle((F32)atof(argv[i+1]));
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-iadd_extra") == 0 || strcmp(argv[i],"-iadd_attribute") == 0)
    {
      if ((i+3) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 3 arguments: data_type name description\n", argv[i]);
        return FALSE;
      }
      if (((i+4) < argc) && (atof(argv[i+4]) != 0.0))
      {
        if (((i+5) < argc) && ((atof(argv[i+5]) != 0.0) || (strcmp(argv[i+5], "0") == 0) || (strcmp(argv[i+5], "0.0") == 0)))
        {
          if (((i+6) < argc) && (atof(argv[i+6]) != 0.0))
          {
            if (((i+7) < argc) && ((atof(argv[i+7]) != 0.0) || (strcmp(argv[i+7], "0") == 0) || (strcmp(argv[i+7], "0.0") == 0)))
            {
              if (((i+8) < argc) && ((atof(argv[i+8]) != 0.0) || (strcmp(argv[i+8], "0") == 0) || (strcmp(argv[i+8], "0.0") == 0)))
              {
                add_attribute(atoi(argv[i+1]), argv[i+2], argv[i+3], atof(argv[i+4]), atof(argv[i+5]), atof(argv[i+6]), atof(argv[i+7]), atof(argv[i+8]));
                *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; *argv[i+4]='\0'; *argv[i+5]='\0'; *argv[i+6]='\0'; *argv[i+7]='\0'; *argv[i+8]='\0'; i+=8;
              }
              else
              {
                add_attribute(atoi(argv[i+1]), argv[i+2], argv[i+3], atof(argv[i+4]), atof(argv[i+5]), atof(argv[i+6]), atof(argv[i+7]));
                *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; *argv[i+4]='\0'; *argv[i+5]='\0'; *argv[i+6]='\0'; *argv[i+7]='\0'; i+=7;
              }
            }
            else
            {
              add_attribute(atoi(argv[i+1]), argv[i+2], argv[i+3], atof(argv[i+4]), atof(argv[i+5]), atof(argv[i+6]));
              *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; *argv[i+4]='\0'; *argv[i+5]='\0'; *argv[i+6]='\0'; i+=6;
            }
          }
          else
          {
            add_attribute(atoi(argv[i+1]), argv[i+2], argv[i+3], atof(argv[i+4]), atof(argv[i+5]));
            *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; *argv[i+4]='\0'; *argv[i+5]='\0'; i+=5;
          }
        }
        else
        {
          add_attribute(atoi(argv[i+1]), argv[i+2], argv[i+3], atof(argv[i+4]));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; *argv[i+4]='\0'; i+=4;
        }
      }
      else
      {
        add_attribute(atoi(argv[i+1]), argv[i+2], argv[i+3]);
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; i+=3;
      }
    }
    else if (strcmp(argv[i],"-iparse") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: string\n", argv[i]);
        return FALSE;
      }
      set_parse_string(argv[i+1]);
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-iskip") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: number_of_lines\n", argv[i]);
        return FALSE;
      }
      set_skip_lines(atoi(argv[i+1]));
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-merged") == 0)
    {
      set_merged(TRUE);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-stored") == 0)
    {
      set_stored(TRUE);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-buffered") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: size\n", argv[i]);
        return FALSE;
      }
      set_buffer_size((F32)atof(argv[i+1]));
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-temp_files") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: base name\n", argv[i]);
        return FALSE;
      }
      temp_file_base = LASCopyString(argv[i+1]);
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-neighbors") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs at least 1 argument: file_name or wild_card\n", argv[i]);
        return FALSE;
      }
      *argv[i]='\0';
      i+=1;
      do
      {
        add_neighbor_file_name(argv[i]);
        *argv[i]='\0';
        i+=1;
      } while ((i < argc) && (*argv[i] != '-') && (*argv[i] != '\0'));
      i-=1;
    }
    else if (strcmp(argv[i],"-neighbors_lof") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs at least 1 argument: file_name\n", argv[i]);
        return FALSE;
      }
      FILE* file = fopen(argv[i+1], "r");
      if (file == 0)
      {
        fprintf(stderr, "ERROR: cannot open '%s'\n", argv[i+1]);
        return FALSE;
      }
      CHAR line[1024];
      while (fgets(line, 1024, file))
      {
        // find end of line
        I32 len = (I32)strlen(line) - 1;
        // remove extra white spaces and line return at the end 
        while (len > 0 && ((line[len] == '\n') || (line[len] == ' ') || (line[len] == '\t') || (line[len] == '\012')))
        {
          line[len] = '\0';
          len--;
        }
#ifdef _WIN32
        add_neighbor_file_name_single(line);
#else
        add_neighbor_file_name(line);
#endif
      }
      fclose(file);
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-pipe_on") == 0)
    {
      set_pipe_on(TRUE);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-populate") == 0)
    {
      set_populate_header(TRUE);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-io_ibuffer") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: size\n", argv[i]);
        return FALSE;
      }
      set_io_ibuffer_size((I32)atoi(argv[i+1]));
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
    }
    else if (strcmp(argv[i],"-do_not_populate") == 0)
    {
      set_populate_header(FALSE);
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-ipts") == 0)
    {
      itxt = TRUE;
      ipts = TRUE;
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-iptx") == 0)
    {
      itxt = TRUE;
      iptx = TRUE;
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-itxt") == 0)
    {
      itxt = TRUE;
      *argv[i]='\0';
    }
  }

  // check that there are only buffered neighbors for single files

  if (neighbor_file_name_number)
  {
    if (file_name_number > 1)
    {
      fprintf(stderr, "ERROR: neighbors only supported for one buffered input file, not for %d\n", file_name_number);
      return FALSE;
    }
    if (buffer_size == 0.0f)
    {
      fprintf(stderr, "ERROR: neighbors only make sense when used with '-buffered 50' or similar\n");
      return FALSE;
    }
  }

  if (filter) filter->clean();
  else filter = new LASfilter();
  if (!filter->parse(argc, argv))
  {
    delete filter;
    return FALSE;
  }
  if (!filter->active())
  {
    delete filter;
    filter = 0;
  }

  if (transform) transform->clean();
  else transform = new LAStransform();
  if (!transform->parse(argc, argv))
  {
    delete transform;
    return FALSE;
  }
  if (!transform->active())
  {
    delete transform;
    transform = 0;
  }
  else if (transform->filtered())
  {
    transform->setFilter(filter);
    filter = 0;
  }

  if (files_are_flightlines || apply_file_source_ID)
  {
    if (transform == 0) transform = new LAStransform();
    transform->setPointSource(0);
  }

  return TRUE;
}

U32 LASreadOpener::get_file_name_number() const
{
  return file_name_number;
}

U32 LASreadOpener::get_file_name_current() const
{
  return file_name_current;
}

const CHAR* LASreadOpener::get_file_name() const
{
  if (file_name)
    return file_name;
  if (file_name_number)
    return file_names[0];
  return 0;
}

const CHAR* LASreadOpener::get_file_name_only() const
{
  const CHAR* file_name_only = 0;
  const CHAR* file_name_curr = get_file_name();

  if (file_name_curr)
  {
    I32 len = (I32)strlen(file_name_curr);
    while ((len > 0) && (file_name_curr[len] != '\\') && (file_name_curr[len] != '/') && (file_name_curr[len] != ':')) len--;
    if (len)
    {
      file_name_only = file_name_curr + len + 1;
    }
    else
    {
      file_name_only = file_name_curr;
    }
  }

  return file_name_only;
}

const CHAR* LASreadOpener::get_file_name(U32 number) const
{
  return file_names[number];
}

I32 LASreadOpener::get_file_format(U32 number) const
{
  if (strstr(file_names[number], ".las") || strstr(file_names[number], ".LAS"))
  {
    return LAS_TOOLS_FORMAT_LAS;
  }
  else if (strstr(file_names[number], ".laz") || strstr(file_names[number], ".LAZ"))
  {
    return LAS_TOOLS_FORMAT_LAZ;
  }
  else if (strstr(file_names[number], ".bin") || strstr(file_names[number], ".BIN"))
  {
    return LAS_TOOLS_FORMAT_BIN;
  }
  else if (strstr(file_names[number], ".shp") || strstr(file_names[number], ".SHP"))
  {
    return LAS_TOOLS_FORMAT_SHP;
  }
  else if (strstr(file_names[number], ".qi") || strstr(file_names[number], ".QI"))
  {
    return LAS_TOOLS_FORMAT_QFIT;
  }
  else if (strstr(file_names[number], ".asc") || strstr(file_names[number], ".ASC"))
  {
    return LAS_TOOLS_FORMAT_ASC;
  }
  else if (strstr(file_names[number], ".bil") || strstr(file_names[number], ".BIL"))
  {
    return LAS_TOOLS_FORMAT_BIL;
  }
  else if (strstr(file_names[number], ".dtm") || strstr(file_names[number], ".DTM"))
  {
    return LAS_TOOLS_FORMAT_DTM;
  }
  else
  {
    return LAS_TOOLS_FORMAT_TXT;
  }
}

void LASreadOpener::set_merged(const BOOL merged)
{
  this->merged = merged;
}

void LASreadOpener::set_stored(const BOOL stored)
{
  this->stored = stored;
}

void LASreadOpener::set_buffer_size(const F32 buffer_size)
{
  this->buffer_size = buffer_size;
}

F32 LASreadOpener::get_buffer_size() const
{
  return buffer_size;
}

void LASreadOpener::set_filter(LASfilter* filter)
{
  this->filter = filter;
}

void LASreadOpener::set_transform(LAStransform* transform)
{
  this->transform = transform;
}

void LASreadOpener::set_auto_reoffset(const BOOL auto_reoffset)
{
  this->auto_reoffset = auto_reoffset;
}

void LASreadOpener::set_files_are_flightlines(const I32 files_are_flightlines)
{
  this->files_are_flightlines = files_are_flightlines;
  if (files_are_flightlines > (I32)(U16_MAX))
  {
    fprintf(stderr, "WARNING: files_are_flightlines start value %d is too large\n", files_are_flightlines);
  }
  else if ((files_are_flightlines + files_are_flightlines_index) > (I32)(U16_MAX))
  {
    fprintf(stderr, "WARNING: files_are_flightlines start value %d plus index %d is too large\n", files_are_flightlines, files_are_flightlines_index);
  }
}

void LASreadOpener::set_files_are_flightlines_index(const I32 files_are_flightlines_index)
{
  this->files_are_flightlines_index = files_are_flightlines_index-1;
  if (files_are_flightlines_index > (I32)(U16_MAX))
  {
    fprintf(stderr, "WARNING: files_are_flightlines_index index value %d is too large\n", files_are_flightlines_index);
  }
  else if ((files_are_flightlines + files_are_flightlines_index) > (I32)(U16_MAX))
  {
    fprintf(stderr, "WARNING: files_are_flightlines start value %d plus index %d is too large\n", files_are_flightlines, files_are_flightlines_index);
  }
}

void LASreadOpener::set_apply_file_source_ID(const BOOL apply_file_source_ID)
{
  this->apply_file_source_ID = apply_file_source_ID;
}

void LASreadOpener::set_io_ibuffer_size(I32 io_ibuffer_size)
{
  this->io_ibuffer_size = io_ibuffer_size;
}

void LASreadOpener::set_file_name(const CHAR* file_name, BOOL unique)
{
  add_file_name(file_name, unique);
}

#ifdef _WIN32
#include <windows.h>
BOOL LASreadOpener::add_file_name(const CHAR* file_name, BOOL unique)
{
  BOOL r = FALSE;
  HANDLE h;
  WIN32_FIND_DATA info;
  h = FindFirstFile(file_name, &info);
  if (h != INVALID_HANDLE_VALUE)
  {
    // find the path
    I32 len = (I32)strlen(file_name);
    while (len && (file_name[len] != '\\') && (file_name[len] != '/') && (file_name[len] != ':')) len--;
    if (len)
    {
      len++;
      CHAR full_file_name[512];
      strncpy(full_file_name, file_name, len);
	    do
	    {
        sprintf(&full_file_name[len], "%s", info.cFileName);
        if (add_file_name_single(full_file_name, unique)) r = TRUE;
  	  } while (FindNextFile(h, &info));
    }
    else
    {
      do
      {
        if (add_file_name_single(info.cFileName, unique)) r = TRUE;
  	  } while (FindNextFile(h, &info));
    }
	  FindClose(h);
  }
  return r;
}
#endif

#ifdef _WIN32
BOOL LASreadOpener::add_file_name_single(const CHAR* file_name, BOOL unique)
#else
BOOL LASreadOpener::add_file_name(const CHAR* file_name, BOOL unique)
#endif
{
  if (unique)
  {
    U32 i;
    for (i = 0; i < file_name_number; i++)
    {
      if (strcmp(file_names[i], file_name) == 0)
      {
        return FALSE;
      }
    }
  }
  if (file_name_number == file_name_allocated)
  {
    if (file_names)
    {
      file_name_allocated *= 2;
      file_names = (CHAR**)realloc(file_names, sizeof(CHAR*)*file_name_allocated);
    }
    else
    {
      file_name_allocated = 16;
      file_names = (CHAR**)malloc(sizeof(CHAR*)*file_name_allocated);
    }
    if (file_names == 0)
    {
      fprintf(stderr, "ERROR: alloc for file_names pointer array failed at %d\n", file_name_allocated);
    }
  }
  file_names[file_name_number] = LASCopyString(file_name);
  file_name_number++;
  return TRUE;
}

BOOL LASreadOpener::add_list_of_files(const CHAR* list_of_files, BOOL unique)
{
  FILE* file = fopen(list_of_files, "r");
  if (file == 0)
  {
    fprintf(stderr, "ERROR: cannot open '%s'\n", list_of_files);
    return FALSE;
  }
  CHAR line[1024];
  while (fgets(line, 1024, file))
  {
    // find end of line
    I32 len = (I32)strlen(line) - 1;
    // remove extra white spaces and line return at the end 
    while (len > 0 && ((line[len] == '\n') || (line[len] == ' ') || (line[len] == '\t') || (line[len] == '\012')))
    {
      line[len] = '\0';
      len--;
    }
    add_file_name(line, unique);
  }
  fclose(file);
  return TRUE;
}

void LASreadOpener::delete_file_name(U32 file_name_id)
{
  if (file_name_id < file_name_number)
  {
    U32 i;
    free(file_names[file_name_id]);
    for (i = file_name_id+1; i < file_name_number; i++)
    {
      file_names[i-1] = file_names[i];
    }
  }
  file_name_number--;
}

BOOL LASreadOpener::set_file_name_current(U32 file_name_id)
{
  if (file_name_id < file_name_number)
  {
    file_name_current = file_name_id;
    file_name = file_names[file_name_current];
    return TRUE;
  }
  return FALSE;
}

#ifdef _WIN32
#include <windows.h>
BOOL LASreadOpener::add_neighbor_file_name(const CHAR* neighbor_file_name, BOOL unique)
{
  BOOL r = FALSE;
  HANDLE h;
  WIN32_FIND_DATA info;
  h = FindFirstFile(neighbor_file_name, &info);
  if (h != INVALID_HANDLE_VALUE)
  {
    // find the path
    I32 len = (I32)strlen(neighbor_file_name);
    while (len && (neighbor_file_name[len] != '\\') && (neighbor_file_name[len] != '/') && (neighbor_file_name[len] != ':')) len--;
    if (len)
    {
      len++;
      CHAR full_neighbor_file_name[512];
      strncpy(full_neighbor_file_name, neighbor_file_name, len);
	    do
	    {
        sprintf(&full_neighbor_file_name[len], "%s", info.cFileName);
        if (add_neighbor_file_name_single(full_neighbor_file_name, unique)) r = TRUE;
  	  } while (FindNextFile(h, &info));
    }
    else
    {
      do
      {
        if (add_neighbor_file_name_single(info.cFileName, unique)) r = TRUE;
  	  } while (FindNextFile(h, &info));
    }
	  FindClose(h);
  }
  return r;
}
#endif

#ifdef _WIN32
BOOL LASreadOpener::add_neighbor_file_name_single(const CHAR* neighbor_file_name, BOOL unique)
#else
BOOL LASreadOpener::add_neighbor_file_name(const CHAR* neighbor_file_name, BOOL unique)
#endif
{
  if (unique)
  {
    U32 i;
    for (i = 0; i < neighbor_file_name_number; i++)
    {
      if (strcmp(neighbor_file_names[i], neighbor_file_name) == 0)
      {
        return FALSE;
      }
    }
  }
  if (neighbor_file_name_number == neighbor_file_name_allocated)
  {
    if (neighbor_file_names)
    {
      neighbor_file_name_allocated *= 2;
      neighbor_file_names = (CHAR**)realloc(neighbor_file_names, sizeof(CHAR*)*neighbor_file_name_allocated);
    }
    else
    {
      neighbor_file_name_allocated = 16;
      neighbor_file_names = (CHAR**)malloc(sizeof(CHAR*)*neighbor_file_name_allocated);
    }
    if (neighbor_file_names == 0)
    {
      fprintf(stderr, "ERROR: alloc for neighbor_file_names pointer array failed at %d\n", neighbor_file_name_allocated);
    }
  }
  neighbor_file_names[neighbor_file_name_number] = LASCopyString(neighbor_file_name);
  neighbor_file_name_number++;
  return TRUE;
}

BOOL LASreadOpener::set_point_type(U8 point_type)
{
  if (point_type > 10)
  {
    return FALSE;
  }
  this->point_type = point_type;
  return TRUE;
}

void LASreadOpener::set_parse_string(const CHAR* parse_string)
{
  if (this->parse_string) free(this->parse_string);
  if (parse_string)
  {
    this->parse_string = LASCopyString(parse_string);
  }
  else
  {
    this->parse_string = 0;
  }
}

const CHAR* LASreadOpener::get_parse_string() const
{
  return parse_string;
}

void LASreadOpener::set_scale_factor(const F64* scale_factor)
{
  if (scale_factor)
  {
    if (this->scale_factor == 0) this->scale_factor = new F64[3];
    this->scale_factor[0] = scale_factor[0];
    this->scale_factor[1] = scale_factor[1];
    this->scale_factor[2] = scale_factor[2];
  }
  else if (this->scale_factor)
  {
    delete [] this->scale_factor;
    this->scale_factor = 0;
  }
}

void LASreadOpener::set_offset(const F64* offset)
{
  if (offset)
  {
    if (this->offset == 0) this->offset = new F64[3];
    this->offset[0] = offset[0];
    this->offset[1] = offset[1];
    this->offset[2] = offset[2];
  }
  else if (this->offset)
  {
    delete [] this->offset;
    this->offset = 0;
  }
}

void LASreadOpener::set_translate_intensity(F32 translate_intensity)
{
  this->translate_intensity = translate_intensity;
}

void LASreadOpener::set_scale_intensity(F32 scale_intensity)
{
  this->scale_intensity = scale_intensity;
}

void LASreadOpener::set_translate_scan_angle(F32 translate_scan_angle)
{
  this->translate_scan_angle = translate_scan_angle;
}

void LASreadOpener::set_scale_scan_angle(F32 scale_scan_angle)
{
  this->scale_scan_angle = scale_scan_angle;
}

void LASreadOpener::add_attribute(I32 data_type, const CHAR* name, const CHAR* description, F64 scale, F64 offset, F64 pre_scale, F64 pre_offset, F64 no_data)
{
  attribute_data_types[number_attributes] = data_type;
  attribute_names[number_attributes] = (name ? LASCopyString(name) : 0);
  attribute_descriptions[number_attributes] = (description ? LASCopyString(description) : 0);
  attribute_scales[number_attributes] = scale;
  attribute_offsets[number_attributes] = offset;
  attribute_pre_scales[number_attributes] = pre_scale;
  attribute_pre_offsets[number_attributes] = pre_offset;
  attribute_no_datas[number_attributes] = no_data;
  number_attributes++;
}

void LASreadOpener::set_skip_lines(I32 skip_lines)
{
  this->skip_lines = skip_lines;
}

void LASreadOpener::set_populate_header(BOOL populate_header)
{
  this->populate_header = populate_header;
}

void LASreadOpener::set_keep_lastiling(BOOL keep_lastiling)
{
  this->keep_lastiling = keep_lastiling;
}

void LASreadOpener::set_pipe_on(BOOL pipe_on)
{
  this->pipe_on = pipe_on;
}

void LASreadOpener::set_decompress_selective(U32 decompress_selective)
{
  this->decompress_selective = decompress_selective;
  if (filter)
  {
    this->decompress_selective |= filter->get_decompress_selective();
  }
  if (transform)
  {
    this->decompress_selective |= transform->get_decompress_selective();
  }
}

void LASreadOpener::set_inside_tile(const F32 ll_x, const F32 ll_y, const F32 size)
{
  if (inside_tile == 0) inside_tile = new F32[3];
  inside_tile[0] = ll_x;
  inside_tile[1] = ll_y;
  inside_tile[2] = size;
}

void LASreadOpener::set_inside_circle(const F64 center_x, const F64 center_y, const F64 radius)
{
  if (inside_circle == 0) inside_circle = new F64[3];
  inside_circle[0] = center_x;
  inside_circle[1] = center_y;
  inside_circle[2] = radius;
}

void LASreadOpener::set_inside_rectangle(const F64 min_x, const F64 min_y, const F64 max_x, const F64 max_y)
{
  if (inside_rectangle == 0) inside_rectangle = new F64[4];
  inside_rectangle[0] = min_x;
  inside_rectangle[1] = min_y;
  inside_rectangle[2] = max_x;
  inside_rectangle[3] = max_y;
}

BOOL LASreadOpener::active() const
{
  return ((file_name_current < file_name_number) || use_stdin);
}

LASreadOpener::LASreadOpener()
{
  io_ibuffer_size = LAS_TOOLS_IO_IBUFFER_SIZE;
  file_names = 0;
  file_name = 0;
  neighbor_file_names = 0;
  merged = FALSE;
  stored = FALSE;
  use_stdin = FALSE;
  comma_not_point = FALSE;
  scale_factor = 0;
  offset = 0;
  buffer_size = 0.0f;
  auto_reoffset = FALSE;
  files_are_flightlines = 0;
  files_are_flightlines_index = -1;
  apply_file_source_ID = FALSE;
  itxt = FALSE;
  ipts = FALSE;
  iptx = FALSE;
  translate_intensity = 0.0f;
  scale_intensity = 1.0f;
  translate_scan_angle = 0.0f;
  scale_scan_angle = 1.0f;
  number_attributes = 0;
  for (I32 i = 0; i < 32; i++)
  {
    attribute_data_types[i] = 0;
    attribute_names[i] = 0;
    attribute_descriptions[i] = 0;
    attribute_scales[i] = 1.0;
    attribute_offsets[i] = 0.0;
    attribute_pre_scales[i] = 1.0;
    attribute_pre_offsets[i] = 0.0;
    attribute_no_datas[i] = F64_MAX;
  }
  point_type = 0;
  parse_string = 0;
  skip_lines = 0;
  populate_header = FALSE;
  keep_lastiling = FALSE;
  pipe_on = FALSE;
  unique = FALSE;
  file_name_number = 0;
  file_name_allocated = 0;
  file_name_current = 0;
  neighbor_file_name_number = 0;
  neighbor_file_name_allocated = 0;
  decompress_selective = LASZIP_DECOMPRESS_SELECTIVE_ALL;
  inside_tile = 0;
  inside_circle = 0;
  inside_rectangle = 0;
  filter = 0;
  transform = 0;
  temp_file_base = 0;
}

LASreadOpener::~LASreadOpener()
{
  if (file_names)
  {
    U32 i;
    for (i = 0; i < file_name_number; i++) free(file_names[i]);
    free(file_names);
  }
  if (neighbor_file_names)
  {
    U32 i;
    for (i = 0; i < neighbor_file_name_number; i++) free(neighbor_file_names[i]);
    free(neighbor_file_names);
  }
  if (parse_string) free(parse_string);
  if (scale_factor) delete [] scale_factor;
  if (offset) delete [] offset;
  if (inside_tile) delete [] inside_tile;
  if (inside_circle) delete [] inside_circle;
  if (inside_rectangle) delete [] inside_rectangle;
  if (filter) delete filter;
  if (transform) delete transform;
  if (temp_file_base) free(temp_file_base);
}
